
#include "contrib/logger/logger.h"
#include "types.h"
#include "variable.h"
#include "pch.h"

#include <context.h>
#include <newlang.h>
#include <object.h>
#include <term.h>

using namespace newlang;

template <>
const std::string Iterator<Obj>::FIND_KEY_DEFAULT = "(.|\\n)*";

std::ostream &operator<<(std::ostream &out, newlang::Obj &var) {
    out << var.toString().c_str();
    return out;
}

std::ostream &operator<<(std::ostream &out, newlang::ObjPtr var) {
    if(var) {
        out << var->toString().c_str();
    } else {
        out << "<NOT OBJECT>";
    }
    return out;
}

ObjPtr Interrupt::CreateErrorMessage(const std::string message, const std::string error_name) {
    bool has_error = false;
    ObjType type = typeFromString(error_name, nullptr, &has_error);

    std::string str(message);
    ObjPtr result;
    if(has_error) {
        str += " Extra error: Type name interrupt '" + error_name + "' is not a base type!";
        result = Obj::CreateType(ObjType::Error);
    } else {
        result = Obj::CreateType(type);
    }

    result->m_var_is_init = true;
    result->push_back(Obj::Arg(Obj::CreateString(str)));
    return result;
}

Interrupt::Interrupt(const std::string error_message, const std::string type_name) : Interrupt(CreateErrorMessage(error_message, type_name)) {
}

Interrupt::Interrupt(ObjPtr obj) : m_obj(obj) {
    std::string temp = m_obj->toString();
    size_t pos = temp.find("\n')");
    if(pos == temp.size() - 3) {
        temp = temp.replace(pos, 1, "");
    }
    snprintf(m_buffer_message, sizeof (m_buffer_message), "Interrupt%s%s%s!", temp.empty() ? "" : " data: \"", temp.empty() ? "" : temp.c_str(), temp.empty() ? "" : "\"");
}

const char* Interrupt::what() const noexcept {
    return &m_buffer_message[0];
}

int64_t Obj::size(int64_t dim) const {
    if(is_tensor()) {
        if(is_scalar()) {
            if(dim != 0) {
                LOG_RUNTIME("Scalar has zero dimension!");
            }
            return 0;
        }
        return m_tensor.size(dim);
    }
    ASSERT(dim == 0);
    if(m_var_type_current == ObjType::StrChar) {
        return m_value.size();
    } else if(m_var_type_current == ObjType::StrWide) {
        return m_string.size();
    }
    return Variable::size();
}

int64_t Obj::resize_(int64_t new_size, ObjPtr fill, const std::string name) {

    if(is_string_type()) {

        if(new_size >= 0) {
            // Размер положительный, просто изменить число элементов добавив или удалив последние
            if(m_var_type_current == ObjType::StrChar) {
                m_value.resize(new_size, ' ');
                return m_value.size();
            } else if(m_var_type_current == ObjType::StrWide) {
                m_string.resize(new_size, L' ');
                return m_string.size();
            }
        } else {
            // Если размер отрицательный - добавить или удалить вначале
            new_size = -new_size;
            if(static_cast<int64_t> (size()) > new_size) {
                if(m_var_type_current == ObjType::StrChar) {
                    m_value.erase(0, new_size);
                    return m_value.size();

                } else if(m_var_type_current == ObjType::StrWide) {
                    m_value.erase(0, new_size);
                    return m_string.size();
                }
            } else if(static_cast<int64_t> (size()) < new_size) {
                if(m_var_type_current == ObjType::StrChar) {
                    m_value.insert(0, new_size, ' ');
                    return m_value.size();

                } else if(m_var_type_current == ObjType::StrWide) {
                    m_string.insert(0, new_size, L' ');
                    return m_string.size();

                }
            }
        }

    } else if(is_dictionary_type()) {
        return Variable::resize(new_size, fill ? fill : Obj::CreateNone(), name);
    } else if(is_tensor()) {
        std::vector<int64_t> sizes;
        for (int i = 0; i < m_tensor.dim(); i++) {
            sizes.push_back(m_tensor.size(i));
        }

        if(sizes.empty()) { // Scalar

            LOG_RUNTIME("Method resize for SCALAR type '%s' not implemented!", newlang::toString(m_var_type_current));

        } else if(new_size == 0 || sizes[0] == new_size) {
            // Tensor size OK - do nothing            
        } else if(new_size > 0) { // Increase tensor size

            // The size is positive, just change the number of elements by adding or removing the last
            ASSERT(sizes.size() == 1);

            sizes[0] = new_size;
            m_tensor.resize_(at::IntArrayRef(sizes));

        } else { // Decrease tensor size
            // If the size is negative - add or remove elements first
            new_size = -new_size;
            if(sizes[0] == new_size) {
                // Tensor size OK - do nothing            
            } else if(sizes[0] > new_size) {

                ASSERT(sizes.size() == 1);

                at::Tensor ind = torch::arange(sizes[0] - new_size - 1, sizes[0] - 1, at::ScalarType::Long);
                at::Tensor any = torch::zeros(sizes[0] - new_size, at::ScalarType::Long);
                //                LOG_DEBUG("arange %s    %s", TensorToString(ind).c_str(), TensorToString(any).c_str());

                ind = at::cat({any, ind});
                //                LOG_DEBUG("cat %s", TensorToString(ind).c_str());

                //                LOG_DEBUG("m_value %s", TensorToString(m_value).c_str());
                m_tensor.index_copy_(0, ind, m_tensor.clone());
                //                LOG_DEBUG("index_copy_ %s", TensorToString(m_value).c_str());

                sizes[0] = new_size;
                m_tensor.resize_(at::IntArrayRef(sizes));
                //                LOG_DEBUG("resize_ %s", TensorToString(m_value).c_str());

            } else { // sizes[0] < size
                ASSERT(sizes.size() == 1);

                m_tensor = at::cat({torch::zeros(new_size - sizes[0], m_tensor.scalar_type()), m_tensor});
            }
        }

        if(new_size == 0) {
            m_tensor.reset();
            m_var_is_init = false;
        }
        return new_size;
    }
    LOG_RUNTIME("Method resize for type '%s' not implemented!", newlang::toString(m_var_type_current));
}

const Variable<Obj>::PairType & Obj::at(int64_t index) const {
    if(m_var_type_current == ObjType::StrChar) {
        if(index < static_cast<int64_t> (m_value.size())) {
            m_str_pair = pair(CreateString(std::string(1, m_value[index])));
            return m_str_pair;
        }
        LOG_RUNTIME("Index '%ld' not exists in byte string '%s'!", index, m_value.c_str());
    } else if(m_var_type_current == ObjType::StrWide) {
        if(index < static_cast<int64_t> (m_string.size())) {
            m_str_pair = pair(CreateString(std::wstring(1, m_string[index])));
            return m_str_pair;
        }
        LOG_RUNTIME("Index '%ld' not exists in byte string '%s'!", index, "WIDE");

    } else if(is_tensor()) {
        ASSERT(!is_scalar());
        torch::Tensor t = m_tensor.index({index});
        m_str_pair = pair(Obj::CreateTensor(t));
        return m_str_pair;
    }
    return Variable::at(index);
}

Variable<Obj>::PairType & Obj::at(int64_t index) {
    if(m_var_type_current == ObjType::StrChar) {
        if(index < static_cast<int64_t> (m_value.size())) {
            m_str_pair = pair(CreateString(std::string(1, m_value[index])));
            return m_str_pair;
        }
        LOG_RUNTIME("Index '%ld' not exists in byte string '%s'!", index, m_value.c_str());
    } else if(m_var_type_current == ObjType::StrWide) {
        if(index < static_cast<int64_t> (m_string.size())) {
            m_str_pair = pair(CreateString(std::wstring(1, m_string[index])));
            return m_str_pair;
        }
        LOG_RUNTIME("Index '%ld' not exists in byte string '%s'!", index, "WIDE");

    } else if(is_tensor()) {
        ASSERT(!is_scalar());
        ASSERT(m_tensor.defined());
        torch::Tensor t = m_tensor.index({(int) index});
        m_str_pair = pair(Obj::CreateTensor(t));
        return m_str_pair;
        //        }
    }
    return Variable::at(index);
}

const ObjPtr Obj::index_get(const std::vector<Index> &index) const {
    if(m_var_type_current == ObjType::StrChar || m_var_type_current == ObjType::FmtChar) {
        if(index.size() != 1 || !index[0].is_integer()) {
            LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
        }
        int64_t pos = index[0].integer();
        if(pos < 0) {
            pos = m_value.size() + pos; // Позиция с конца строки
        }
        if(pos < static_cast<int64_t> (m_value.size())) {
            return CreateString(std::string(1, m_value[pos]));
        }
        LOG_RUNTIME("Index '%s' not exists in byte string '%s'!", IndexToString(index).c_str(), m_value.c_str());
    } else if(m_var_type_current == ObjType::StrWide || m_var_type_current == ObjType::FmtWide) {
        if(index.size() != 1 || !index[0].is_integer()) {
            LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
        }
        int64_t pos = index[0].integer();
        if(pos < 0) {
            pos = m_string.size() + pos; // Позиция с конца строки
        }
        if(pos < static_cast<int64_t> (m_string.size())) {
            return CreateString(std::wstring(1, m_string[pos]));
        }
        LOG_RUNTIME("Index '%s' not exists in WIDE string '%s'!", IndexToString(index).c_str(), utf8_encode(m_string).c_str());

    } else if(is_tensor()) {
        ASSERT(!is_scalar());
        ASSERT(m_tensor.defined());
        torch::Tensor t = m_tensor.index(index);
        return Obj::CreateTensor(t);
    }

    if(index.size() != 1 || !index[0].is_integer()) {
        LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
    }
    return Variable::at(index[0].integer()).second;
}

ObjPtr Obj::index_set_(const std::vector<Index> &index, const ObjPtr value) {
    if(m_var_type_current == ObjType::StrChar) {
        if(index.size() != 1 || !index[0].is_integer()) {
            LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
        }
        int64_t pos = index[0].integer();
        if(pos < 0) {
            pos = m_value.size() + pos; // Позиция с конца строки
        }
        if(pos < static_cast<int64_t> (m_value.size())) {
            m_value.erase(pos, 1);
            m_value.insert(pos, value->toType(ObjType::StrChar)->m_value);
            m_var_is_init = true;
            return shared();
        }
        LOG_RUNTIME("Index '%s' not exists in byte string '%s'!", IndexToString(index).c_str(), m_value.c_str());
    } else if(m_var_type_current == ObjType::StrWide) {
        if(index.size() != 1 || !index[0].is_integer()) {
            LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
        }
        int64_t pos = index[0].integer();
        if(pos < 0) {
            pos = m_value.size() + pos; // Позиция с конца строки
        }
        if(pos < static_cast<int64_t> (m_string.size())) {
            m_string.erase(pos, 1);
            m_string.insert(pos, value->toType(ObjType::StrWide)->m_string);
            m_var_is_init = true;
            return shared();
        }
        LOG_RUNTIME("Index '%s' not exists in byte string '%s'!", IndexToString(index).c_str(), "WIDE");

    } else if(is_tensor()) {
        ASSERT(!is_scalar());
        ASSERT(m_tensor.defined());

        ObjPtr temp = value->toType(fromTorchType(m_tensor.scalar_type()));
        if(temp->is_scalar()) {
            if(temp->is_integral()) {
                m_tensor.index_put_(index, temp->GetValueAsInteger());
            } else {
                ASSERT(temp->is_floating());
                m_tensor.index_put_(index, temp->GetValueAsNumber());
            }
        } else {
            ASSERT(temp->m_tensor.defined());
            m_tensor.index_put_(index, temp->m_tensor);
        }
        m_var_is_init = true;
        return shared();

    } else if(is_dictionary_type()) {
        if(index.size() != 1 || !index[0].is_integer()) {
            LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
        }
        (*at(index[0].integer()).second) = value;
        return shared();
        LOG_RUNTIME("Index '%s' not exists in object '%s'!", IndexToString(index).c_str(), toString().c_str());
    }
    LOG_RUNTIME("Don`t set index '%s' in object '%s'!", IndexToString(index).c_str(), toString().c_str());
}

ObjPtr Obj::op_set_index(int64_t index, std::string value) {
    if(m_var_type_current == ObjType::StrChar) {
        if(index < static_cast<int64_t> (m_value.size())) {
            m_value.erase(index, 1);
            m_value.insert(index, value);
            m_var_is_init = true;
            return shared();
        }
        LOG_RUNTIME("Index '%ld' not exists in byte string '%s'!", index, m_value.c_str());
    } else if(m_var_type_current == ObjType::StrWide) {
        if(index < static_cast<int64_t> (m_string.size())) {
            m_string.erase(index, 1);
            m_string.insert(index, utf8_decode(value));
            m_var_is_init = true;
            return shared();
        }
        LOG_RUNTIME("Index '%ld' not exists in byte string '%s'!", index, "WIDE");
    }
    //    at(index).second.set_(value);
    (*at(index).second) = value;
    return Variable::operator[](index).second;
}

bool Obj::exist(ObjPtr &find, bool strong) {
    for (auto &elem : * this) {
        if(strong && find->op_accurate(elem.second)) {
            return true;
        } else if(!strong && find->op_equal(elem.second)) {
            return true;
        }
    }
    return false;
}

ObjType newlang::getSummaryTensorType(Obj *obj, ObjType start) {

    ObjType result = ObjType::None;
    if(!obj) {
        return result;
    }
    if(obj->is_dictionary_type()) {
        for (int i = 0; i < obj->size(); i++) {
            result = getSummaryTensorType(obj->at(i).second.get(), result);
        }
        return result;
    } else if(obj->is_arithmetic_type()) {
        if(isGenericType(obj->m_var_type_fixed)) {
            return std::max(obj->m_var_type_current, start);
        } else {
            if(start >= std::max(obj->m_var_type_current, obj->m_var_type_fixed)) {
                return start;
            } else {
                return std::max(obj->m_var_type_current, obj->m_var_type_fixed);
            }
        }
    }
    LOG_RUNTIME("Tensor support arithmetic data type only '%s'!", obj->toString().c_str());
}

ObjPtr Obj::operator+=(Obj value) {
    if(is_tensor()) {
        if(value.is_tensor()) {
            testResultIntegralType(value.m_var_type_current, true);
            if(is_scalar() && value.is_scalar()) {
                if(is_floating()) {
                    ASSERT(at::holds_alternative<double>(m_var));
                    m_var = GetValueAsNumber() + value.GetValueAsNumber();
                    m_var_type_current = ObjType::Double;
                } else if(is_integral() && value.is_integral()) {
                    ASSERT(at::holds_alternative<int64_t>(m_var));
                    m_var = GetValueAsInteger() + value.GetValueAsInteger();
                    m_var_type_current = typeFromLimit(GetValueAsInteger());
                } else {
                    LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
                }
            } else if(value.is_scalar()) {
                if(value.is_floating()) {
                    m_tensor.add_(value.GetValueAsNumber());
                } else if(value.is_integral()) {
                    m_tensor.add_(value.GetValueAsInteger());
                } else {
                    LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
                }
            } else {
                ASSERT(!is_scalar() && !value.is_scalar());
                m_tensor.add_(value.m_tensor);
            }
            return shared();
        }
    }
    switch(m_var_type_current) {
        case ObjType::StrChar:
        case ObjType::StrWide:
            switch(value.m_var_type_current) {
                case ObjType::None:
                    return shared();
                case ObjType::StrChar:
                    m_value += value.m_value;
                    return shared();
                case ObjType::StrWide:
                    m_string += value.m_string;
                    return shared();
            }
            break;

        case ObjType::Class:
        case ObjType::Dictionary:
            if(value.m_var_type_current == ObjType::None) {
                return shared();
            } else if(value.m_var_type_current == ObjType::Class || value.m_var_type_current == ObjType::Dictionary) {
                for (int i = 0; i < value.size(); i++) {
                    push_back(value.at(i).second);
                }
                return shared();
            }
            break;

        case ObjType::Fraction:
            if(value.m_var_type_current == ObjType::Fraction) {
                m_fraction->operator+=(*(value.m_fraction.get()));
            } else {
                m_fraction->operator+=(*(value.toType(ObjType::Fraction)->m_fraction.get()));
            }
            return shared();

    }
    LOG_RUNTIME("Operator '+' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

ObjPtr Obj::operator-=(Obj value) {
    if(m_var_type_current == ObjType::None) {
        m_var_type_current = value.m_var_type_current;
    } else if(is_tensor() && value.is_tensor()) {
        testResultIntegralType(value.m_var_type_current, true);
        if(is_scalar() && value.is_scalar()) {
            if(is_floating()) {
                ASSERT(at::holds_alternative<double>(m_var));
                m_var = GetValueAsNumber() - value.GetValueAsNumber();
                m_var_type_current = ObjType::Double;
            } else if(is_integral() && value.is_integral()) {
                ASSERT(at::holds_alternative<int64_t>(m_var));
                m_var = GetValueAsInteger() - value.GetValueAsInteger();
                m_var_type_current = typeFromLimit(GetValueAsInteger());
            } else {
                LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
            }
        } else if(value.is_scalar()) {
            if(value.is_floating()) {
                m_tensor.sub_(value.GetValueAsNumber());
            } else if(value.is_integral()) {
                m_tensor.sub_(value.GetValueAsInteger());
            } else {
                LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
            }
        } else {
            ASSERT(!is_scalar() && !value.is_scalar());
            m_tensor.sub_(value.m_tensor);
        }
        return shared();
    }
    switch(m_var_type_current) {
        case ObjType::Class:
        case ObjType::Dictionary:
            if(value.m_var_type_current == ObjType::None) {
                return shared();
            } else if(value.m_var_type_current == ObjType::Class || value.m_var_type_current == ObjType::Dictionary) {
                for (int i = 0; i < value.size(); i++) {
                    auto found = find(value.name(i));
                    if(found != end()) {
                        ListType::erase(found);
                    }
                }
                return shared();
            }
            break;
        case ObjType::Fraction:
            if(value.m_var_type_current == ObjType::Fraction) {
                m_fraction->operator-=(*(value.m_fraction.get()));
            } else {
                m_fraction->operator-=(*(value.toType(ObjType::Fraction)->m_fraction.get()));
            }
            return shared();
    }
    LOG_RUNTIME("Operator '-' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

ObjPtr Obj::operator*=(Obj value) {
    if(m_var_type_current == ObjType::None) {
        m_var_type_current = value.m_var_type_current;
    } else if(is_tensor() && value.is_tensor()) {
        testResultIntegralType(value.m_var_type_current, true);
        if(is_scalar() && value.is_scalar()) {
            if(is_floating()) {
                ASSERT(at::holds_alternative<double>(m_var));
                m_var = GetValueAsNumber() * value.GetValueAsNumber();
                m_var_type_current = ObjType::Double;
            } else if(is_integral() && value.is_integral()) {
                ASSERT(at::holds_alternative<int64_t>(m_var));
                m_var = GetValueAsInteger() * value.GetValueAsInteger();
                m_var_type_current = typeFromLimit(GetValueAsInteger());
            } else {
                LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
            }
        } else if(value.is_scalar()) {
            if(value.is_floating()) {
                m_tensor.mul_(value.GetValueAsNumber());
            } else if(value.is_integral()) {
                m_tensor.mul_(value.GetValueAsInteger());
            } else {
                LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
            }
        } else {
            ASSERT(!is_scalar() && !value.is_scalar());
            m_tensor.mul_(value.m_tensor);
        }
        return shared();
    }

    switch(m_var_type_current) {

        case ObjType::Class:
        case ObjType::Dictionary:
            if(value.m_var_type_current == ObjType::None) {
                Variable::clear_();
                return shared();
            } else if(value.m_var_type_current == ObjType::Class || value.m_var_type_current == ObjType::Dictionary) {
                op_bit_and_set(value, false);
                return shared();
            }
            break;

        case ObjType::StrChar:
            if(value.is_integral()) {
                m_value = repeat(m_value, value.GetValueAsInteger());
                return shared();
            } else if(value.is_string_type()) {
                m_value += value.GetValueAsString();
                return shared();
            }
        case ObjType::StrWide:
            if(value.is_integral()) {
                m_string = repeat(m_string, value.GetValueAsInteger());
                return shared();
            } else if(value.is_string_type()) {
                m_string += utf8_decode(value.GetValueAsString());
                return shared();
            }

        case ObjType::Fraction:
            if(value.m_var_type_current == ObjType::Fraction) {
                m_fraction->operator*=(*(value.m_fraction.get()));
            } else {
                m_fraction->operator*=(*(value.toType(ObjType::Fraction)->m_fraction.get()));
            }
            return shared();

    }
    LOG_RUNTIME("Operator '*' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

ObjPtr Obj::operator/=(Obj value) {
    if(is_tensor() && value.is_tensor()) {
        testResultIntegralType(ObjType::Double, true);
        if(is_scalar() && value.is_scalar()) {
            if(is_floating()) {
                ASSERT(at::holds_alternative<double>(m_var));
                m_var = GetValueAsNumber() / value.GetValueAsNumber();
                m_var_type_current = ObjType::Double;
            } else if(is_integral() && value.is_integral()) {
                ASSERT(at::holds_alternative<int64_t>(m_var));
                m_var = GetValueAsInteger() / value.GetValueAsInteger();
                m_var_type_current = typeFromLimit(GetValueAsInteger());
            } else {
                LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
            }
        } else if(value.is_scalar()) {
            if(value.is_floating()) {
                m_tensor.div_(value.GetValueAsNumber());
            } else if(value.is_integral()) {
                m_tensor.div_(value.GetValueAsInteger());
            } else {
                LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
            }
        } else {
            ASSERT(!is_scalar() && !value.is_scalar());
            m_tensor.div_(value.m_tensor);
        }
        return shared();
    } else if(m_var_type_current == ObjType::Fraction) {
        if(value.m_var_type_current == ObjType::Fraction) {
            m_fraction->operator/=(*(value.m_fraction.get()));
        } else {
            m_fraction->operator/=(*(value.toType(ObjType::Fraction)->m_fraction.get()));
        }
        return shared();
    }
    LOG_RUNTIME("Operator '/' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

ObjPtr Obj::op_div_ceil_(Obj & value) {
    if(is_tensor() && value.is_tensor()) {
        ObjType type = m_var_type_current;
        testResultIntegralType(ObjType::Float, false);
        if(is_scalar() && value.is_scalar()) {
            if(is_floating()) {
                ASSERT(at::holds_alternative<double>(m_var));
                m_var = floor(GetValueAsNumber() / value.GetValueAsNumber());
                m_var_type_current = ObjType::Double;
            } else if(is_integral() && value.is_integral()) {
                ASSERT(at::holds_alternative<int64_t>(m_var));
                m_var = static_cast<int64_t> (GetValueAsInteger() / value.GetValueAsInteger());
                m_var_type_current = typeFromLimit(GetValueAsInteger());
            } else {
                LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
            }
        } else if(value.is_scalar()) {
            if(value.is_floating()) {
                m_tensor.div_(value.GetValueAsNumber(), "floor");
            } else if(value.is_integral()) {
                m_tensor.div_(value.GetValueAsInteger(), "floor");
            } else {
                LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
            }
        } else {
            ASSERT(!is_scalar() && !value.is_scalar());
            m_tensor.div_(value.m_tensor, "floor");
            m_tensor = m_tensor.toType(toTorchType(type));
        }
        //        ObjType type = m_var_type_current;
        //        testResultIntegralType(ObjType::Float, false);
        //        m_tensor.div_(value.m_tensor, "floor");
        //        m_tensor = m_tensor.toType(toTorchType(type));
        return shared();

    } else if(m_var_type_current == ObjType::Fraction) {
        //        if(value.m_var_type_current == ObjType::Fraction) {
        //            m_fraction->operator/=(value.m_fraction);
        //        } else {
        //            m_fraction->operator/=(value.toType(ObjPtr::Fraction)->m_fraction);
        //        }
        //        return shared();
    }
    LOG_RUNTIME("Operator '//' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

ObjPtr Obj::operator%=(Obj value) {
    if(is_tensor() && value.is_tensor()) {
        testResultIntegralType(value.m_var_type_current, true);
        if(is_scalar() && value.is_scalar()) {
            if(is_floating()) {
                ASSERT(at::holds_alternative<double>(m_var));
                m_var = fmod(GetValueAsNumber(), value.GetValueAsNumber());
            } else if(is_integral() && value.is_integral()) {
                ASSERT(at::holds_alternative<int64_t>(m_var));
                m_var = GetValueAsInteger() % value.GetValueAsInteger();
            } else {
                LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
            }
        } else if(value.is_scalar()) {
            if(value.is_floating()) {
                m_tensor.fmod_(value.GetValueAsNumber());
            } else if(value.is_integral()) {
                m_tensor.fmod_(value.GetValueAsInteger());
            } else {
                LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
            }
        } else {
            ASSERT(!is_scalar() && !value.is_scalar());
            m_tensor.fmod_(value.m_tensor);
        }
        return shared();
    } else if(m_var_type_current == ObjType::Fraction) {
        //        if(value.m_var_type_current == ObjType::Fraction) {
        //            m_fraction->operator*=(value.m_fraction);
        //        } else {
        //            m_fraction->operator*=(value.toType(ObjPtr::Fraction)->m_fraction);
        //        }
        //        return shared();
    }
    LOG_RUNTIME("Operator '%%' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

size_t Obj::ItemValueCount(ObjPtr &find, bool strong) {
    size_t result = 0;
    for (auto &elem : * this) {
        if(strong && find->op_accurate(elem.second)) {
            result++;
        } else if(!strong && find->op_equal(elem.second)) {
            result++;
        }
    }
    return result;
}

void Obj::CloneDataTo(Obj & clone) const {

    NL_CHECK(!isLocalType(m_var_type_current), "Local object not clonable!");

    if(&clone != this) { // Не клонировать сам в себя
        clone.m_var_type_current = m_var_type_current;
        clone.m_var_type_fixed = m_var_type_fixed;
        clone.m_var_is_init = m_var_is_init;
        //        clone.m_var_type_name = m_var_type_name;

        if(m_dimensions) {
            clone.m_dimensions = m_dimensions->Clone();
        } else {
            clone.m_dimensions = nullptr;
        }

        clone.m_var_name = m_var_name;
        clone.m_value = m_value;
        clone.m_string = m_string;

        if(m_fraction) {
            clone.m_fraction = std::make_shared<Fraction>(*m_fraction.get());
        } else {
            clone.m_fraction = nullptr;
        }
        clone.m_iterator = m_iterator;

        clone.m_class_parents = m_class_parents;
        clone.m_class_name = m_class_name;
        clone.m_is_const = m_is_const;

        clone.m_var = m_var;
        if(m_prototype) {
            *const_cast<TermPtr *> (&clone.m_prototype) = m_prototype;
        }
        if(m_tensor.defined()) {
            clone.m_tensor = m_tensor.clone();
        }
    }
}

void Obj::ClonePropTo(Obj & clone) const {

    NL_CHECK(!isLocalType(m_var_type_current), "Local object not clonable!");

    for (int i = 0; i < Variable<Obj>::size(); i++) {
        if(Variable<Obj>::at(i).second) {
            if(Variable<Obj>::at(i).second->m_is_reference || Variable<Obj>::at(i).second->m_is_reference) {
                clone.push_back(Variable<Obj>::at(i));
            } else {
                clone.push_back(Variable<Obj>::at(i).second->Clone(nullptr), name(i));
            }
        } else {
            if(name(i).empty()) {
                LOG_RUNTIME("Null arg %d without name! %s", i, toString().c_str());
            }
            // Объекта может не быть у обязательных параметров функций
            clone.push_back(nullptr, name(i));
        }
    }
}

void Obj::SetTermProp(Term & term) {
    m_namespace = term.m_namespace;
}

//void newlang::calcTensorDims(ObjPtr &obj, std::vector<int64_t> &dims) {
//    if(obj->is_dictionary_type()) {
//        dims.push_back(obj->size());
//        if(obj->size()) {
//            calcTensorDims(obj->at(0).second, dims);
//        }
//    }
//}

ObjPtr Obj::GetIndex(ObjPtr obj, TermPtr index_arg) {
    ASSERT(index_arg->size() == 1);
    TermPtr index = index_arg->at(0).second;
    ASSERT(index);
    if(index->getTermID() != TermID::INTEGER) {
        LOG_RUNTIME("Fail index type %s '%s'", newlang::toString(index->getTermID()), index->toString().c_str());
    }
    return GetIndex(obj, std::stoi(index->m_text.c_str()));
}

std::string Obj::toString(bool deep) const {
    std::string result(m_is_reference ? "&" : "");
    result += m_var_name;
    if(!result.empty() && m_var_type_current == ObjType::Class && !deep && m_is_reference) {
        return result;
    }
    if(!m_var_name.empty()) {
        result.append("=");
    }

    std::stringstream ss;

    if(m_var_type_current == ObjType::None) {
        if(m_prototype && m_prototype->GetType()) {
            result += m_prototype->GetType()->toString();
        } else if(m_var_type_fixed != ObjType::None) {
            result += newlang::toString(m_var_type_fixed);
        }
        result += "_";
        return result;
    } else if(is_tensor()) {
        if(is_scalar()) {
            result += GetValueAsString();
        } else {
            ASSERT(m_tensor.defined());
            result += TensorToString(m_tensor);
        }
        return result;
    } else if(isSimpleType(m_var_type_current)) {
        result += GetValueAsString();
        return result;
    } else {
        switch(m_var_type_current) {

            case ObjType::None: // name:=<EMPTY>
                result = "_";
                return result;

            case ObjType::StrChar:
                result += "'";
                result += m_value;
                result.append("'");
                return result;

            case ObjType::StrWide: // name:='string' or name:="string"
                result += "\"";
                result += utf8_encode(m_string);
                result.append("\"");
                return result;

            case ObjType::Range: // name:=(1,second="two",3,<EMPTY>,5)
                result = at("start").second->GetValueAsString();
                result += "..";
                result += at("stop").second->GetValueAsString();
                result += "..";
                result += at("step").second->GetValueAsString();
                return result;

            case ObjType::Struct:
            case ObjType::Union:
            case ObjType::Enum:

            case ObjType::Return:
            case ObjType::Error:
            case ObjType::ErrorParser:
            case ObjType::ErrorRunTime:
            case ObjType::ErrorSignal:
            case ObjType::Continue:
            case ObjType::Break:
                if(m_class_name.empty()) {
                    result += newlang::toString(m_var_type_current);
                } else {
                    result += m_class_name;
                }
                result += "(";
                dump_dict_(result);
                result += ")";
                return result;

            case ObjType::Dictionary: // name:=(1,second="two",3,<EMPTY>,5)
                result += "(";
                dump_dict_(result);
                result += ",";
                result += ")";
                return result;

            case ObjType::Pointer:
                ASSERT(at::holds_alternative<void *>(m_var));
                ss << at::get<void *>(m_var);
                result += ss.str();
                result += ":Pointer";
                return result;


            case ObjType::Type:
                result += newlang::toString(m_var_type_fixed);
                if(m_dimensions && m_dimensions->size()) {
                    result += "[";
                    for (int i = 0; i < m_dimensions->size(); i++) {
                        if(i) {
                            result += ",";
                        }
                        result += (*m_dimensions)[i].second->toString();
                    }
                    result += "]";
                }


                if(size()) {
                    result += "(";
                    dump_dict_(result);
                    result += ")";
                }
                return result;

            case ObjType::NativeFunc:
            case ObjType::Function: // name:={function code}
            case ObjType::PureFunc: // name=>{function code}
                ASSERT(m_prototype);
                result += m_prototype->m_text;
                result += "(";
                m_prototype->dump_items_(result);
                result += ")";
                if(!m_prototype->m_type_name.empty()) {
                    result += m_prototype->m_type_name;
                }

            case ObjType::BLOCK:
                result += "{}";
                return result;

            case ObjType::BLOCK_TRY:
                result += "{{}}";
                return result;

            case ObjType::EVAL_FUNCTION: // name=>{function code}
            case ObjType::SimplePureFunc:
            case ObjType::SimplePureAND:
            case ObjType::SimplePureOR:
            case ObjType::SimplePureXOR:
                ASSERT(m_prototype);
                result += m_prototype->m_text;
                result += "(";
                m_prototype->dump_items_(result);
                result += ")";
                if(!m_prototype->m_type_name.empty()) {
                    result += m_prototype->m_type_name;
                }

                if(m_var_type_current == ObjType::EVAL_FUNCTION) {
                    result += ":=";
                } else if(m_var_type_current == ObjType::SimplePureFunc) {
                    result += ":-";
                } else if(m_var_type_current == ObjType::SimplePureAND) {
                    result += "::&&=";
                } else if(m_var_type_current == ObjType::SimplePureOR) {
                    result += "::||=";
                } else if(m_var_type_current == ObjType::SimplePureXOR) {
                    result += "::^^=";
                } else {
                    LOG_RUNTIME("Fail function type");
                }

                if(m_sequence->getTermID() != TermID::BLOCK) {
                    result += "{";
                }
                if(m_sequence) {
                    result += m_sequence->toString();
                    if(m_sequence->getTermID() != TermID::BLOCK) {
                        result += ";";
                    }
                }
                if(m_sequence->getTermID() != TermID::BLOCK) {
                    result.append("}");
                }
                return result;

            case ObjType::Class: // name:=@term(id=123, ...) name=base(id=123, ... )
                result += m_class_name;
                result += "(";

                if(!empty()) {
                    dump_dict_(result);
                }
                result += ")";
                return result;

            case ObjType::Ellipsis:
                result += "...";
                return result;

            case ObjType::Fraction:
                ASSERT(m_fraction);
                result += m_fraction->GetAsString();
                return result;

            case ObjType::Iterator:
            case ObjType::IteratorEnd:
                return newlang::toString(m_var_type_current);
        }
    }
    LOG_RUNTIME("Unknown type '%s' (%d)", newlang::toString(m_var_type_current), (int) m_var_type_current);
}

void TensorToString_(const torch::Tensor &tensor, c10::IntArrayRef shape, std::vector<Index> &ind, const int64_t pos,
        std::stringstream & str) {
    std::string intend;
    ASSERT(pos < static_cast<int64_t> (ind.size()));
    str << "[";
    if(shape.size() > 1 && pos + 1 < static_cast<int64_t> (ind.size())) {
        str << "\n";
        intend = std::string((pos + 1) * 2, ' ');
        str << intend;
    }
    if(pos + 1 < static_cast<int64_t> (ind.size())) {
        bool comma = false;
        for (ind[pos] = 0; ind[pos].integer() < shape[pos]; ind[pos] = ind[pos].integer() + 1) {
            if(comma) {
                str << ", ";
            } else {
                comma = true;
            }
            TensorToString_(tensor, shape, ind, pos + 1, str);
        }
    } else {
        bool comma = false;
        for (ind[pos] = 0; ind[pos].integer() < shape[pos]; ind[pos] = ind[pos].integer() + 1) {
            if(comma) {
                str << ", ";
            } else {
                comma = true;
            }
            if(tensor.is_floating_point()) {
                str << tensor.index(ind).item<double>();
            } else if(tensor.is_complex()) {
                ASSERT(!"Not implemented!");
            } else {
                str << tensor.index(ind).item<int64_t>();
            }
        }
    }
    str << ",";
    if(!intend.empty()) {
        str << "\n";
    }
    str << "]";
}

std::string newlang::TensorToString(const torch::Tensor & tensor) {
    std::string result;
    std::stringstream ss;

    if(!tensor.dim()) {
        ASSERT(tensor.dim());
    }

    c10::IntArrayRef shape = tensor.sizes(); // Кол-во эментов в каждом измерении
    std::vector<Index> ind(shape.size(), 0); // Счетчик обхода всех элементов тензора
    TensorToString_(tensor, shape, ind, 0, ss);
    result = ss.str();
    result += newlang::toString(fromTorchType(tensor.scalar_type()));

    return result;
}

std::string Obj::GetValueAsString() const {
    std::string result;
    std::string temp;
    std::stringstream ss;

    if(!m_var_is_init) {
        LOG_RUNTIME("Object not initialized '%s'!", toString().c_str());
    }

    switch(m_var_type_current) {
        case ObjType::None:
            return result;

        case ObjType::Tensor:
        case ObjType::Bool:
        case ObjType::Char:
        case ObjType::Short:
        case ObjType::Int:
        case ObjType::Long:
        case ObjType::Integer:
        case ObjType::Float:
        case ObjType::Double:
        case ObjType::Number:
        case ObjType::Complex:
        case ObjType::ComplexFloat:
        case ObjType::ComplexDouble:
            if(is_scalar()) {
                if(is_integral()) {
                    return std::to_string(GetValueAsInteger());
                } else if(is_floating()) {
                    ss << GetValueAsNumber();
                    return ss.str();
                } else {
                    ASSERT(!"Not implemented!");
                }
            } else {
                return TensorToString(m_tensor);
            }

        case ObjType::StrChar:
        case ObjType::FmtChar:
            return m_value;

        case ObjType::StrWide:
        case ObjType::FmtWide:
            return utf8_encode(m_string);

        case ObjType::NativeFunc:
        case ObjType::Function:
        case ObjType::PureFunc:
        case ObjType::EVAL_FUNCTION:
        case ObjType::SimplePureFunc:
        case ObjType::SimplePureAND:
        case ObjType::SimplePureOR:
        case ObjType::SimplePureXOR:
            return m_var_name + "={}";

        case ObjType::Class:
        case ObjType::Dictionary:
            return toString();

        case ObjType::Error:
            result = m_var_name;
            if(!m_var_name.empty()) {
                result += ": ";
            }
            temp = m_value;
            trim(temp, "\n");
            result += temp;
            return result;

        case ObjType::Pointer:
            ASSERT(at::holds_alternative<void *>(m_var));
            ss << at::get<void *>(m_var);
            result += ss.str();
            if(m_class_name.empty()) {
                result += ":Pointer";
            } else {
                result += m_class_name;
            }
            return result;

        case ObjType::Range:
            result += toString();
            return result;

        case ObjType::Fraction:
            ASSERT(m_fraction);
            result += m_fraction->GetAsString();
            return result;
    }
    LOG_RUNTIME("Data type '%s' %d incompatible to string!", newlang::toString(m_var_type_current), (int) m_var_type_current);
}

ObjPtr Obj::CreateFunc(std::string prototype, FunctionType *func_addr, ObjType type) {
    ASSERT(func_addr);
    ASSERT(type == ObjType::Function || type == ObjType::PureFunc);


    TermPtr proto = Parser::ParseString(std::string(prototype + ":={}"));
    proto = proto->Left();

    ObjPtr result = Obj::CreateType(type, type);

    * const_cast<TermPtr *> (&result->m_prototype) = proto;
    result->m_var = (void *) func_addr;

    return result;
}

ObjPtr Obj::CreateFunc(Context *ctx, TermPtr proto, ObjType type, const std::string var_name) {
    ASSERT(type == ObjType::Function || type == ObjType::PureFunc);
    ObjPtr result = std::make_shared<Obj>(type, var_name.c_str(), proto);
    Obj local;
    Obj args(ctx, proto, false, &local);
    args.ClonePropTo(*result);
    *const_cast<TermPtr *> (&result->m_prototype) = proto;
    if(!result->CheckArgs()) {
        LOG_RUNTIME("Fail create function '%s'!", proto->toString().c_str());
    }
    return result;
}

Obj::Obj(Context *ctx, const TermPtr term, bool as_value, Obj * local_vars) {

    if(!term) {
        NL_CHECK(term, "Fail term!");
    }

    m_namespace = term->m_namespace;
    m_is_reference = term->m_is_ref;
    m_var_name = term->m_name.empty() ? term->m_text : term->m_name;
    m_var_type_current = ObjType::Dictionary;
    m_dimensions = nullptr;
    m_var_is_init = false;
    m_is_const = false;
    m_var = at::monostate();
    ASSERT(!m_tensor.defined());

    *const_cast<TermPtr *> (&m_prototype) = term;

    for (int i = 0; i < term->size(); i++) {
        if(term->name(i).empty()) {
            if(as_value) {
                // Не именованный аргумент
                push_back(Context::CreateRVal(ctx, (*term)[i].second, local_vars));
            } else {
                // Обязательный аргумент без значения по умолчанию
                push_back(pair(nullptr, term->at(i).second->getText()));
            }
        } else {
            push_back(Context::CreateRVal(ctx, (*term)[i].second, local_vars), term->name(i));
        }
    }
}

bool Obj::CheckArgs() const {
    bool has_error = false;
    bool named = false;
    for (int start = 0; start < size(); start++) {
        if(at(start).first.empty()) {
            // Аргумент не имеет имени
            LOG_ERROR("Argument %d has no name!", (int) start + 1);
            has_error = true;
        }
        if(at(start).second == nullptr) {
            if(named) {
                // Обязательный аргумент после параметров со значениями по умолчанию
                LOG_ERROR("Rrequired argument %d after parameters with default values!", (int) start + 1);
                has_error = true;
            }
        } else {
            named = true;
            for (int64_t i = start + 1; i < size(); i++) {
                if(at(start).first.compare(at(i).first) == 0) {
                    LOG_ERROR("Duplicate named argument '%s'!", at(start).first.c_str());
                    has_error = true;
                }
            }
        }
    }
    return !has_error;
}

ObjPtr Obj::Call(Context *ctx, Obj * args) {
    if(is_string_type()) {
        ObjPtr result = Clone();
        result->m_value = format(result->m_value, args);
        return result;
    } else if(is_function() || m_var_type_current == ObjType::Type) {
        Obj local;
        ObjPtr param;
        if(m_prototype) {
            param = std::make_shared<Obj>(ctx, m_prototype, false, &local);
        } else {
            param = Obj::CreateDict();
        }
        param->ConvertToArgs_(args, true, ctx);
        param->push_front(pair(shared(), "$0")); // Self

        if(ctx) {
            ctx->RegisterInContext(param);
        }

        ObjPtr result;
        if(m_var_type_current == ObjType::Function) {
            ASSERT(at::holds_alternative<void *>(m_var));
            result = (*reinterpret_cast<FunctionType *> (at::get<void *>(m_var)))(ctx, *param.get()); // Непосредственно вызов функции
        } else if(m_var_type_current == ObjType::PureFunc || (m_var_type_current == ObjType::Type)) {
            ASSERT(at::holds_alternative<void *>(m_var));
            result = (*reinterpret_cast<TransparentType *> (at::get<void *>(m_var)))(ctx, *param.get()); // Непосредственно вызов функции
        } else if(m_var_type_current == ObjType::NativeFunc) {
            result = CallNative(ctx, *param.get());
        } else if(m_var_type_current == ObjType::EVAL_FUNCTION || m_var_type_current == ObjType::SimplePureFunc) {
            result = Context::CallBlock(ctx, m_sequence, param.get(), false);
        } else if(m_var_type_current == ObjType::SimplePureAND) {
            result = Context::EvalBlockAND(ctx, m_sequence, param.get());
        } else if(m_var_type_current == ObjType::SimplePureOR) {
            result = Context::EvalBlockOR(ctx, m_sequence, param.get());
        } else if(m_var_type_current == ObjType::SimplePureXOR) {
            result = Context::EvalBlockXOR(ctx, m_sequence, param.get());
        } else {
            LOG_RUNTIME("Call by name not implemted '%s'!", toString().c_str());
        }

        if(ctx) {
            ctx->pop_front();
        }
        return result;


    } else if(is_dictionary_type()) {

        ObjPtr result = Clone(); // Копия текущего объекта
        result->ConvertToArgs_(args, false, ctx); // С обновленными полями, переданными в аргументах
        result->m_class_parents.push_back(shared()); // Текущйи объект становится базовым классом для вновь создаваемого
        return result;

    } else if(args->size() > 1) {
        LOG_RUNTIME("Unsupported operation for data type %d '%s'", (int) m_var_type_current, toString().c_str());
    }
    return Clone();
}

// Обновить параметры для вызова функции или элементы у словаря при создании копии

void Obj::ConvertToArgs_(Obj *in, bool check_valid, Context * ctx) {

    ASSERT(in);

    //    bool named = false;
    bool is_ellipsis = false;
    if(check_valid && size()) {
        if(at(size() - 1).first.compare("...") == 0) {
            is_ellipsis = true;
            erase(size() - 1);
        }
    }
    for (int i = 0; i < in->size(); i++) {

        if(isInternalName(in->name(i))) {
            continue;
        }

        if(in->name(i).empty()) {
            //            if(check_valid && named) {
            //                LOG_RUNTIME("Position %d requires a named argument!", (int) i + 1);
            //            }
            if(i < size()) {
                if(check_valid && at(i).second && at(i).second->getType() != ObjType::None) {
                    if(!canCast((*in)[i].second->getType(), at(i).second->getType())) {
                        LOG_RUNTIME("Fail cast value '%s' to type '%s'", newlang::toString((*in)[i].second->getType()),
                                newlang::toString(at(i).second->getType()));
                    }
                }
                if(!at(i).second) {
                    at(i).second = Obj::CreateNone();
                }
                if(m_prototype && i < m_prototype->size()) {
                    at(i).second->m_is_reference = (*m_prototype)[i].second->isRef();
                    ObjType base_type = ObjType::None;
                    if(ctx) {
                        base_type = ctx->BaseTypeFromString((*m_prototype)[i].second->m_type_name);
                    } else {
                        base_type = typeFromString((*m_prototype)[i].second->m_type_name, ctx);
                    }
                    ObjType limit_type = (*in)[i].second->getTypeAsLimit();
                    if(!canCast(limit_type, base_type)) {
                        // Строку с одним символом можно преобразовать в арифметичсекий тип
                        if(!(isArithmeticType(base_type) && (*in)[i].second->is_string_type() && (*in)[i].second->size() == 1)) {
                            LOG_RUNTIME("Fail cast value '%s' to type '%s'",
                                    (*in)[i].second->toString().c_str(), (*m_prototype)[i].second->m_type_name.c_str());
                        }
                    }
                }
                at(i).second->op_assign((*in)[i].second);
            } else {
                if(check_valid && !is_ellipsis && m_prototype && i >= m_prototype->size()) {
                    LOG_RUNTIME("Positional args overflow. Ptrototype '%s'!",
                            m_prototype ? m_prototype->toString().c_str() : "Prototype not exists!");
                }
                push_back(in->at(i));
            }
        } else {
            //            named = true;
            auto found = find(in->name(i));
            if(found != end()) {
                if(check_valid && (*found).second && (*found).second->getType() != (*in)[i].second->getType() && (*found).second->getType() != ObjType::None) {
                    LOG_RUNTIME("Different type arg '%s' and '%s'", (*found).second->toString().c_str(),
                            (*in)[i].second->toString().c_str());
                }
                //@todo  Проверка ограничений размер данных при указаном типе
                if(!(*found).second) {
                    (*found).second = Obj::CreateNone();
                }
                (*found).second->op_assign((*in)[i].second);
            } else {
                for (int pos = 0; pos < size(); pos++) {
                    if(!at(pos).first.empty() && at(pos).first.compare(in->at(i).first) == 0) {
                        at(pos).second->op_assign(in[i]);
                        goto done;
                    }
                }
                if(check_valid && !is_ellipsis) {
                    LOG_RUNTIME("Named arg '%s' not found!", in->name(i).c_str());
                }
                push_back(in->at(i));
done:
                ;
            }
        }
    }
    if(check_valid) {

        CheckArgsValid();
    }
}

void Obj::CheckArgsValid() const {
    //    bool named = false;
    //    for (int i = 0; i < Variable::size(); i++) {
    //        //        if(!at(i).second) {
    //        //
    //        //            LOG_RUNTIME("Argument %d '%s' missed!", (int) i + 1, at(i).first.c_str());
    //        //        }
    //    }
    //    //    if(!CheckArgs_()) {
    //    //        LOG_RUNTIME("Fail arguments!");
    //    //    }
}

/*
 *
 *
 *
 *
 */
int Obj::op_compare(Obj & value) {
    if(this == &value) {
        return 0;
    }
    if(is_scalar() && value.is_scalar()) {
        if(is_floating() || value.is_floating()) {
            if(GetValueAsNumber() < value.GetValueAsNumber()) {
                return -1;
            } else if(GetValueAsNumber() > value.GetValueAsNumber()) {
                return 1;
            };
            return 0;
        } else if(is_complex() || value.is_complex()) {
            // Будет ошибка сравнения комплексных значений
        } else {
            if(GetValueAsInteger() < value.GetValueAsInteger()) {
                return -1;
            } else if(GetValueAsInteger() > value.GetValueAsInteger()) {
                return 1;
            };
            return 0;
        }

    } else if((is_string_type() && value.is_string_type())) {
        switch(m_var_type_current) {
            case ObjType::StrChar:
                return m_value.compare(value.GetValueAsString());

            case ObjType::StrWide:
                return m_string.compare(value.GetValueAsStringWide());
        }
    }
    LOG_RUNTIME("Fail compare type %s and %s", newlang::toString(m_var_type_current), newlang::toString(value.m_var_type_current));
}

///*
//    0 == false // true
//    0 === false // false, так как разные типы
//    1 == "1" // true, происходит автоматическая конвертация
//    1 === "1" // false, так как разные типы
//    null == undefined // true
//    null === undefined // false
//    '0' == false // true
//    '0' === false // false
// */
//

bool Obj::op_equal(Obj & value) {
    if(this == &value) {
        return true;
    } else if(is_tensor()) {
        ObjType summary_type = static_cast<ObjType> (std::max(
                static_cast<uint8_t> (m_var_type_current),
                static_cast<uint8_t> (value.m_var_type_current)));
        try {
            if(is_scalar() || value.is_scalar()) {
                if(is_scalar() && value.is_scalar()) {
                    if(isIntegralType(summary_type, true)) {
                        return GetValueAsInteger() == value.GetValueAsInteger();
                    } else if(isFloatingType(summary_type)) {
                        return GetValueAsNumber() == value.GetValueAsNumber();
                    } else {
                        LOG_RUNTIME("Fail compare type '%s'!", newlang::toString(summary_type));
                    }
                }
                return false;
            }
            torch::Dtype summary_torch_type = toTorchType(static_cast<ObjType> (summary_type));
            return m_tensor.toType(summary_torch_type).equal(value.toType(summary_type)->m_tensor);

        } catch (std::exception e) {
            LOG_RUNTIME("Fail compare"); //, e.what());
        }
    } else if(is_bool_type()) {
        return GetValueAsBoolean() == value.GetValueAsBoolean();
    } else if(is_string_type()) {
        return GetValueAsString().compare(value.GetValueAsString()) == 0;
    } else if(is_dictionary_type() && value.is_dictionary_type()) {
        if(size() != value.size()) {
            return false;
        }
        for (int64_t i = 0; i < static_cast<int64_t> (size()); i++) {
            if(name(i).compare(value.name(i)) != 0) {
                return false;
            }
            if(!at(i).second->op_equal(value[i].second)) {

                return false;
            }
        }
        return true;
    }
    return false; // оставшиеся типы равны только если идентичны сами объекты (первое условие)
}

bool Obj::op_accurate(Obj & value) {
    if(this == &value || (is_none_type() && value.is_none_type())) {
        return true;
    } else if((is_bool_type() && value.is_bool_type()) || (is_arithmetic_type() && value.is_arithmetic_type()) ||
            (is_string_type() && value.is_string_type()) || (is_dictionary_type() && value.is_dictionary_type())) {

        return op_equal(value);
    }
    return false;
}

ObjPtr Obj::op_bit_and_set(Obj &obj, bool strong) {
    if(m_var_type_current == ObjType::Long) {
        if(m_var_type_current == obj.m_var_type_current) {
            m_tensor.bitwise_and_(obj.m_tensor);
            //            m_values.integer &= obj.m_values.integer;
            return shared();
        }
    } else if(m_var_type_current == ObjType::None || obj.m_var_type_current == ObjType::None) {
        Variable::clear_();
        return shared();
    } else if(m_var_type_current == ObjType::Dictionary || m_var_type_current == ObjType::Class) {
        if(obj.m_var_type_current == ObjType::Dictionary || obj.m_var_type_current == ObjType::Class) {
            int pos = 0;
            while(pos < size()) {
                if(!obj.exist(at(pos).second, strong)) {
                    erase(pos);
                } else {
                    pos++;
                }
            }
            return shared();
        }
    } else if(is_tensor() && obj.is_tensor()) {
        int pos = 0;
        while(pos < size()) {
            if(!obj.exist(at(pos).second, strong)) {
                erase(pos);
            } else {

                pos++;
            }
        }
        return shared();
    }
    LOG_RUNTIME("Incompatible types %d and %d for '&' operator!", (int) m_var_type_current, (int) obj.m_var_type_current);
}

bool Obj::op_class_test(ObjPtr obj, Context * ctx) const {
    if(obj->is_string_type()) {
        return op_class_test(obj->GetValueAsString().c_str(), ctx);
    } else if(!obj->m_class_name.empty()) {
        return op_class_test(obj->m_class_name.c_str(), ctx);
    } else if(obj->is_type_name()) {
        return op_class_test(newlang::toString(obj->m_var_type_fixed), ctx);
    } else {
        return op_class_test(newlang::toString(obj->m_var_type_current), ctx);
    }
}

bool Obj::op_class_test(const char *name, Context * ctx) const {

    ASSERT(name || *name);

    if(!m_class_name.empty() && m_class_name.compare(name) == 0) {
        return true;
    }
    for (auto &elem : m_class_parents) {
        if(elem->op_class_test(name, ctx)) {
            return true;
        }
    }

    bool has_error = false;
    ObjType type = typeFromString(name, ctx, &has_error);
    if(has_error) {
        LOG_DEBUG("Type name %s not found!", name);
        return false;
    }

    ObjType check_type = m_var_type_current;
    if(m_var_type_current == ObjType::Type || (!m_var_is_init && m_var_type_current == ObjType::None)) {
        check_type = m_var_type_fixed;
    }

    if(isContainsType(type, check_type)) {
        return true;
    }

    std::string class_name = newlang::toString(check_type);
    return !class_name.empty() && class_name.compare(name) == 0;
}

bool Obj::op_duck_test_prop(Obj *base, Obj *value, bool strong) {
    if(!value) {
        return !strong; // Пустой объект равен любому при нечетком сравнении
    }
    if(!base || base->m_var_type_current == ObjType::None) {
        // Итина при пустом текущем может быть только если образец тоже пустой
        return value->m_var_type_current == ObjType::None;
    }
    ObjPtr field;
    for (int i = 0; i < value->size(); i++) {
        if(value->name(i).empty()) {
            field = (*base)[i].second;
        } else {
            field = (*base)[value->name(i)].second;
        }
        if(!field) {
            return false;
        }
        if(strong || !((*value)[i].second->getType() != ObjType::None)) {
            for (auto &elem : *value) {
                if(!field->op_duck_test(elem.second, strong)) {

                    return false;
                }
            }
        }
    }
    return true;
}

ObjPtr Obj::op_pow_(Obj & obj) {
    if(is_tensor()) {
        ASSERT(obj.is_arithmetic_type());
        if(is_scalar()) {
            double temp = pow(GetValueAsNumber(), obj.GetValueAsNumber());
            if(is_integral()) {
                if(temp > static_cast<double> (std::numeric_limits<int64_t>::max())) {
                    LOG_ERROR("Integer overflow!");
                }
                m_var = static_cast<int64_t> (llround(temp));
            } else {
                m_var = temp;
            }
        } else {
            ASSERT(m_tensor.defined());
            m_tensor.pow_(obj.GetValueAsNumber());
        }
        return shared();
    } else if(is_fraction()) {

        ASSERT(m_fraction);
        ObjPtr temp = obj.toType(ObjType::Fraction);
        ASSERT(temp);
        ASSERT(temp->m_fraction);

        m_fraction->op_pow_(*temp->m_fraction.get());
        return shared();

    } else if(m_var_type_current == ObjType::StrChar && obj.is_integral()) {
        m_value = repeat(m_value, obj.GetValueAsInteger());
        return shared();
    } else if(m_var_type_current == ObjType::StrWide && obj.is_integral()) {
        m_string = repeat(m_string, obj.GetValueAsInteger());
        return shared();
    } else if(m_var_type_current == ObjType::Fraction) {
        //        if(value.m_var_type_current == ObjType::Fraction) {
        //            m_fraction->op_pow_(value.m_fraction);
        //        } else {
        //            m_fraction->op_pow_(value.toType(ObjPtr::Fraction)->m_fraction);
        //        }
        //        return shared();
    }
    LOG_RUNTIME("Unsupported power operator for '%s' and '%s'!", toString().c_str(), obj.toString().c_str());
}

bool Obj::op_duck_test(Obj *value, bool strong) {

    if(!value || value->m_var_type_current == ObjType::None) {
        // Пустой объект совместим с любым объектом,
        // а при строгом сравнении только с таким же пустым
        return strong ? m_var_type_current == value->m_var_type_current : true;
    }

    if(strong) {
        if(value->is_simple()) {
            if(m_var_type_current == value->m_var_type_current || (is_string_type() && value->is_string_type())) {
                return true;
            }
            return false;
        }
        return op_duck_test_prop(this, value, strong);
    }
    if(value->m_var_type_current == ObjType::Long || value->m_var_type_current == ObjType::Double) {
        return (m_var_type_current == ObjType::Long || m_var_type_current == ObjType::Double);
    } else if(is_string_type() && value->is_string_type()) {
        return true;
    } else if(is_function() && value->is_function()) {
        return true;
    } else if(value->m_var_type_current == ObjType::Dictionary || value->m_var_type_current == ObjType::Class) {
        if(m_var_type_current == ObjType::Dictionary || m_var_type_current == ObjType::Class) {

            return op_duck_test_prop(this, value, strong);
        }
        return false;
    }
    return m_var_type_current == value->m_var_type_current;
}

std::string Obj::format(std::string format, Obj * args) {
    if(args && !args->empty()) {
        std::string name;
        std::string place;
        std::wstring wname;
        for (int i = 0; i < args->size(); i++) {

            if(isInternalName(args->name(i))) {
                continue;
            }

            // Заменить номер аргумента
            name = "\\$" + std::to_string(i + 1);
            place = (*args)[i].second->GetValueAsString();
            format = std::regex_replace(format, std::regex(name), place);

            if(!args->name(i).empty()) {

                std::wstring wplace = utf8_decode(place);
                std::wstring temp = utf8_decode(format);

                wname = L"\\$\\{" + utf8_decode(args->name(i)) + L"\\}";
                temp = std::regex_replace(temp, std::wregex(wname), wplace);

                wname = L"\\$" + utf8_decode(args->name(i)); // + L"\\b"; // Иначе перестает работать UTF8
                temp = std::regex_replace(temp, std::wregex(wname), wplace);

                format = utf8_encode(temp);

            }
        }
    }
    return format;
}

int64_t newlang::ConcatData(Obj *dest, Obj &src, ConcatMode mode) {
    int64_t size = 0;
    ASSERT(dest);

    if(!dest->m_var_is_init) {

        ObjPtr temp = src.toType(dest->m_var_type_current);
        temp->m_var_type_fixed = dest->m_var_type_current;

        dest->clear_();
        temp->CloneDataTo(*dest);
        temp->ClonePropTo(*dest);
        dest->m_var_is_init = true;

    } else if(dest->is_string_type()) {

        if(dest->m_var_type_current == ObjType::StrChar) {
            std::string add = src.GetValueAsString();
            dest->m_value.append(add);
            size = add.size();
        } else if(dest->m_var_type_current == ObjType::StrWide) {
            std::wstring add = src.GetValueAsStringWide();
            dest->m_string.append(add);
            size = add.size();
        } else {
            LOG_RUNTIME("Unknown string type %s!", dest->toString().c_str());
        }

    } else if(dest->is_dictionary_type()) {

        ObjPtr temp = src.toType(ObjType::Dictionary);
        for (int i = 0; i < temp->size(); i++) {
            dest->push_back(temp->at(i).second, temp->at(i).first);
            size++;
        }

    } else if(dest->is_tensor()) {

        if(dest->m_var_type_current == src.m_var_type_current) {
            if(dest->m_tensor.dim() == 0) {
                dest->m_tensor.resize_(1);
            }
            if(src.m_tensor.dim() == 0) {
                src.m_tensor.resize_(1);
            }
            dest->m_tensor = torch::cat({dest->m_tensor, src.m_tensor});
            //            size += src.m_value.si
        } else {
            ObjPtr temp = src.toType(dest->m_var_type_current);
            size += ConcatData(dest, *(temp.get()), mode);
        }

    } else {
        LOG_RUNTIME("Unknown data type %s!", dest->toString().c_str());
    }

    return size;
}

void ShapeFromDict(const Obj *obj, std::vector<int64_t> &shape) {
    if(obj && (obj->is_dictionary_type() || (obj->is_tensor() && !obj->is_scalar()))) {
        if(!obj->size()) {
            LOG_RUNTIME("Cannot tensor shape from empty dictionary!");
        }
        shape.push_back(obj->size());
        if(obj->at(0).second) {
            ShapeFromDict(obj->at(0).second.get(), shape);
        }
    }
}

std::vector<int64_t> newlang::TensorShapeFromDict(const Obj * obj) {
    std::vector<int64_t> shape;
    ShapeFromDict(obj, shape);
    return shape;
}

ObjPtr Obj::CallNative(Context *ctx, Obj args) {

    if(!ctx || !ctx->m_runtime) {
        LOG_RUNTIME("Fail context for call native!");
    }


    std::vector<LLVMTypeRef> arg_types;
    std::vector<LLVMGenericValueRef> arg_generic;

    ObjType type_result = ctx->BaseTypeFromString(m_prototype->m_type_name);
    LLVMTypeRef return_llvm_type = toLLVMType(type_result);

    bool is_ellipsis = (m_prototype->size() && (*m_prototype)[m_prototype->size() - 1].second->getTermID() == TermID::ELLIPSIS);
    size_t check_count = is_ellipsis ? m_prototype->size() - 1 : m_prototype->size();

    bool pointer_exist = false;
    // Пропустить нулевой аргумент для нативных функций
    for (int i = 1; i < args.size(); i++) {

        ASSERT(args[i].second);
        if(args[i].second->m_is_reference) {
            LOG_RUNTIME("Argument REFERENCE! %s", args[i].second->toString().c_str());
        }

        size_t pind = i - 1; // Индекс прототипа на единицу меньше из-за пустого нулевого аргумента

        ObjType type = args[i].second->getTypeAsLimit();
        if(pind < check_count) {
            NL_CHECK(!(*m_prototype)[pind].second->m_type_name.empty(), "Undefined type arg '%s'", (*m_prototype)[pind].second->toString().c_str());

            ObjType proto_type = typeFromString((*m_prototype)[pind].second->m_type_name, ctx);
            if(!canCast(type, proto_type)) {
                if(!((type == ObjType::StrChar || type == ObjType::FmtChar) && proto_type == ObjType::Char) ||
                        ((type == ObjType::StrWide || type == ObjType::FmtWide) && proto_type == ObjType::Int)) {
                    LOG_RUNTIME("Fail cast from '%s' to '%s'", (*m_prototype)[pind].second->m_type_name.c_str(), newlang::toString(type));
                }
            }
        }

        ObjType check_type;
        if(pind < check_count) {
            std::string temp_str = (*m_prototype)[pind].second->m_type_name;
            check_type = typeFromString((*m_prototype)[pind].second->m_type_name, ctx);
        } else {
            check_type = args[i].second->getType();
        }

        if(args[i].second->is_string_type()) {
            pointer_exist = true;
        }

        if(check_type == ObjType::Bool && m_namespace.empty()) {
            // В чистом С (для пустого m_namespace) для логического типа используется тип int
            check_type = ObjType::Int;
        }
        if((check_type == ObjType::FmtWide || check_type == ObjType::StrWide) ||
                ((check_type == ObjType::FmtChar || check_type == ObjType::StrChar) &&
                (args[i].second->getType() == ObjType::StrWide || args[i].second->getType() == ObjType::FmtWide))) {
            LOG_RUNTIME("Convert wide characters as native function arguments not supported!");
        }

        LLVMTypeRef temp = toLLVMType(check_type);
        arg_types.push_back(temp);
        arg_generic.push_back(args[i].second->GetGenericValueRef(temp));

        if(pind < check_count && (*m_prototype)[pind].second->GetType()) {
            if((*m_prototype)[pind].second->GetType()->m_text.compare(newlang::toString(ObjType::FmtChar)) == 0) {
                NL_CHECK(ParsePrintfFormat(&args, i), "Fail format string or type args!");
            }
        }
    }

    auto module = LLVMModuleCreateWithName("call_native");
    LLVMExecutionEngineRef interpreter;
    LLVMCreateExecutionEngineForModule(&interpreter, module, nullptr);

    //    std::vector<LLVMTypeRef> arg_types{LLVMPointerType(LLVMInt8Type(), 0)};
    LLVMTypeRef func_res_type = LLVMFunctionType(return_llvm_type, arg_types.data(), static_cast<unsigned int> (arg_types.size()), is_ellipsis);

    // Declare a function bar in IR, we will define this function with IR
    LLVMValueRef wrap = LLVMAddFunction(module, "call_wrap", func_res_type);

    // Create the code block of the function bar
    auto entry = LLVMAppendBasicBlock(wrap, "entry");
    LLVMPositionBuilderAtEnd(ctx->m_llvm_builder, entry);



    std::vector<LLVMValueRef> args_param;
    // Пропустить нулевой аргумент для нативных функций
    for (int i = 1; i < args.size(); i++) {
        // Индекс прототипа на единицу меньше из-за пустого нулевого аргумента
        args_param.push_back(LLVMGetParam(wrap, i - 1));
    }

    //    LLVMValueRef param_call[] = {LLVMGetParam(wrap, 0)};
    LLVMValueRef func_call = LLVMAddFunction(module, "m_func_ptr", func_res_type); // Имя функции может пересечся с сужествующими при связывании?????
    LLVMValueRef func_ret = LLVMBuildCall2(ctx->m_llvm_builder, func_res_type, func_call, args_param.data(), static_cast<unsigned int> (args_param.size()), m_prototype->m_text.c_str());

    //return value 
    LLVMBuildRet(ctx->m_llvm_builder, func_ret);


    char *error = nullptr;
    if(LLVMVerifyModule(module, LLVMReturnStatusAction, &error)) {
        LOG_RUNTIME("LLVMVerifyModule  %s", error ? error : "");
    }
    if(error) {
        LLVMDisposeMessage(error);
        error = nullptr;
    }



#ifdef UNITTEST
    char *dump = LLVMPrintValueToString(wrap);
    LOG_DEBUG("LLVM DUMP %s:\n%s\r\r", toString().c_str(), dump);
    LLVMDisposeMessage(dump);
#endif

    LLVMExecutionEngineRef engine;
    if(LLVMCreateExecutionEngineForModule(&engine, module, &error)) {
        LOG_RUNTIME("Failed to create execution engine '%s'!", error ? error : "");
    }
    if(error) {
        LLVMDisposeMessage(error);
        error = nullptr;
    }


    // Map the global function in the external C++ code to the IR code, only the declaration in the IR code
    ASSERT(at::holds_alternative<void *>(m_var));
    LLVMAddGlobalMapping(engine, func_call, at::get<void *>(m_var));


    ObjPtr result = nullptr;
#ifdef _MSC_VER
    bool skip_call = true;
#else
    bool skip_call = false;
#endif
    if(skip_call || (skip_call && pointer_exist)) {
        LOG_WARNING("LLVM reported error on Windows: \"MCJIT::runFunction does not support full-featured argument passing!!!!\"");
        result = Obj::CreateNone();
    } else {
        //    std::vector<LLVMGenericValueRef> exec_args{LLVMCreateGenericValueOfPointer((void *) "РАБОТАЕТ!!!!")};
        LLVMGenericValueRef exec_res = LLVMRunFunction(engine, wrap, static_cast<unsigned int> (arg_generic.size()), arg_generic.data());
        result = Obj::CreateFromGenericValue(type_result, exec_res, return_llvm_type);
        LLVMDisposeGenericValue(exec_res);
    }

    for (auto &elem : arg_generic) {
        LLVMDisposeGenericValue(elem);
    }
    return result;
}

bool newlang::ParsePrintfFormat(Obj *args, int start) {

    if(!args) {
        LOG_WARNING("Missing object!");
        return false;
    }
    if(args->size() <= static_cast<int64_t> (start) || !(*args)[start].second) {
        LOG_WARNING("Missing format string!");
        return false;
    }
    if(!(*args)[start].second->is_string_type()) {
        LOG_WARNING("Argument Format '%s' not string type!", (*args)[start].second->toString().c_str());
        return false;
    }

    std::string format = (*args)[start].second->GetValueAsString();


    static const std::string flags_list = "-+0123456789.lLh"; // '#', '*'
    static const std::string types_list = "diufFeEgGxXaAoscp"; //, 'n'

    static std::string valid_chars;
    if(valid_chars.empty()) {
        valid_chars.append(flags_list);
        valid_chars.append(types_list);
    }

    bool result = true;
    int aind = start + 1;

    size_t pos = 0;
    while(pos < format.length()) {
        pos = format.find_first_of('%', pos);
        if(pos == format.npos) {
            break;
        }
        pos++;
        if(format[pos] == '%' || !format[pos]) {
            continue;
        }

        if(aind >= args->size()) {

            LOG_WARNING("Missing argument %u", (int) pos);
            return false;

        } else {

            pos = format.find_first_not_of(valid_chars, pos);
            if(pos == format.npos) {
                pos = format.length();
            }
            pos--;

            ObjType cast = ObjType::None;
            switch(format[pos]) {

                case 'a': //%a	Шестнадцатеричное в виде 0xh.hhhhp+d (только С99)
                case 'A': //%A	Шестнадцатеричное в виде 0Xh.hhhhP+d (только С99)
                case 'd': //%d	Десятичное целое со знаком
                case 'i': //%i	Десятичное целое со знаком
                case 'o': //%o	Восьмеричное без знака
                case 'u': //%u	Десятичное целое без знака
                case 'x': //%x	Шестнадцатеричное без знака (буквы на нижнем регистре)
                case 'X': //%X	Шестнадцатеричное без знака (буквы на верхнем регистре)
                    cast = ObjType::Int;
                    if(pos && (format[pos - 1] == 'l' || format[pos - 1] == 'L')) {
                        cast = ObjType::Long;
                    } else if(pos && format[pos - 1] == 'h') {
                        cast = ObjType::Short;
                    }
                    if(!canCast((*args)[aind].second->m_var_type_current, cast)) {
                        LOG_WARNING("Cast '%s' to '%s' not supported!", newlang::toString((*args)[aind].second->m_var_type_current), newlang::toString(cast));
                        result = false;
                    }
                    break;


                case 'e'://%e	Экспоненциальное представление ('е' на нижнем регистре)
                case 'E'://%E	Экспоненциальное представление ('Е' на верхнем регистре)
                case 'f'://%f	Десятичное с плавающей точкой
                case 'g'://%g	В зависимости от того, какой вывод будет короче, используется %е или %f
                case 'G'://%G	В зависимости от того, какой вывод будет короче, используется %Е или %F

                    cast = ObjType::Double;
                    if(!canCast((*args)[aind].second->m_var_type_current, cast)) {
                        LOG_WARNING("Cast '%s' to '%s' not supported!", newlang::toString((*args)[aind].second->m_var_type_current), newlang::toString(cast));
                        result = false;
                    }
                    break;

                case 'c':
                    cast = ObjType::Char;
                    if(pos && (format[pos - 1] == 'l' || format[pos - 1] == 'L')) {
                        cast = ObjType::Int;
                    }
                    if(!canCast((*args)[aind].second->m_var_type_current, cast)) {
                        LOG_WARNING("Cast '%s' to '%s' not supported!", newlang::toString((*args)[aind].second->m_var_type_current), newlang::toString(cast));
                        result = false;
                    }
                    break;

                case 's':
                    cast = ObjType::StrChar;
                    if(pos && (format[pos - 1] == 'l' || format[pos - 1] == 'L')) {
                        cast = ObjType::StrWide;
                    }
                    if(!canCast((*args)[aind].second->m_var_type_current, cast)) {
                        LOG_WARNING("Cast '%s' to '%s' not supported!", newlang::toString((*args)[aind].second->m_var_type_current), newlang::toString(cast));
                        result = false;
                    }
                    break;


                case 'p':
                    cast = ObjType::Pointer;
                    if(!canCast((*args)[aind].second->m_var_type_current, cast)) {
                        LOG_WARNING("Cast '%s' to '%s' not supported!", newlang::toString((*args)[aind].second->m_var_type_current), newlang::toString(cast));
                        result = false;
                    }
                    break;

                default:
                    LOG_WARNING("Format modifier '%c' at pos %d in '%s' not supported!", format[pos], (int) pos, format.c_str());
                    result = false;
            }
        }
        pos++;
        aind++; // Следующий аргумент
    }

    if(aind != args->size()) {
        LOG_WARNING("Extra arguments more %d", aind);
        return false;
    }
    return result;
}

/*
 * 
 * 
 */

void newlang::ConvertStringToTensor(const std::string &from, torch::Tensor &to, ObjType type) {
    if(from.empty()) {
        LOG_RUNTIME("Fail convert empty string to tensor!");
    }
    to = torch::from_blob((void *) from.c_str(),{(int64_t) from.size()}, at::ScalarType::Char).clone().toType(toTorchType(type));
}

void newlang::ConvertStringToTensor(const std::wstring &from, torch::Tensor &to, ObjType type) {
    if(from.empty()) {
        LOG_RUNTIME("Fail convert empty string to tensor!");
    }
    if(sizeof (wchar_t) == sizeof (int32_t)) {
        to = torch::from_blob((void *) from.c_str(),{(int64_t) from.size()}, torch::Dtype::Int).clone().toType(toTorchType(type));
    } else if(sizeof (wchar_t) == sizeof (int16_t)) {
        to = torch::from_blob((void *) from.c_str(),{(int64_t) from.size()}, torch::Dtype::Short).clone().toType(toTorchType(type));
    } else {
        LOG_RUNTIME("Unsupport wchar_t size '%d'!!!", (int) sizeof (wchar_t));
    }
}

template <typename T> void ConvertTensorToStringTemplate(const torch::Tensor &from, T &to, std::vector<Index> *index) {

    ASSERT(from.dim()); // Скаляры хранятся не тензорами, а нативными типами

    std::vector<Index> dims;
    if(index == nullptr) {
        to.clear();
        dims.push_back(Index(0));
        index = &dims;
    }

    int64_t pos = index->size();
    if(pos == from.dim()) {
        at::ScalarType torch_type;
        switch(sizeof (to[0])) {
            case 1:
                torch_type = at::ScalarType::Char;
                break;
            case 2:
                torch_type = at::ScalarType::Short;
                break;
            case 4:
                torch_type = at::ScalarType::Int;
                break;
            default:
                LOG_RUNTIME("Unsupported char size! %d", (int) sizeof (to[0]));
        }
        for (int i = 0; i < from.size(pos - 1); i++) {
            (*index)[pos - 1] = i;
            to += from.index(*index).toType(torch_type).item<int>();
        }
    } else {
        index->push_back(0);
        for (int64_t i = 0; i < from.size(pos - 1); i++) {
            (*index)[pos - 1] = i;
            ConvertTensorToString(from, to, index);
        }
    }
}

void newlang::ConvertTensorToString(const torch::Tensor &from, std::string &to, std::vector<Index> *index) {
    ConvertTensorToStringTemplate<std::string>(from, to, index);
}

void newlang::ConvertTensorToString(const torch::Tensor &from, std::wstring &to, std::vector<Index> *index) {
    ConvertTensorToStringTemplate<std::wstring>(from, to, index);
}

void newlang::ConvertTensorToDict(const torch::Tensor &from, Obj &to, std::vector<Index> *index) {

    to.m_var_is_init = false;
    ASSERT(to.m_var_type_current == ObjType::Dictionary || to.m_var_type_current == ObjType::None);
    if(!to.is_dictionary_type()) {
        to.m_var_type_current = ObjType::Dictionary;
    }

    ASSERT(from.dim()); // Скаляры хранятся не тензорами, а нативными типами

    std::vector<Index> dims;
    if(index == nullptr) {
        dims.push_back(Index(0));
        index = &dims;
    }

    int64_t pos = index->size();
    if(pos == from.dim()) {
        for (int i = 0; i < from.size(pos - 1); i++) {
            (*index)[pos - 1] = i;
            to.push_back(Obj::CreateTensor(from.index(*index)));
        }
    } else {
        index->push_back(0);
        for (int64_t i = 0; i < from.size(pos - 1); i++) {
            (*index)[pos - 1] = i;
            ConvertTensorToDict(from, to, index);
        }
    }

    to.m_var_is_init = true;
}

/*
 * Варианты преобраования типов
 * Range -> Dict
 * String -> Tensor
 * Tensor -> String
 * Tensor -> Dict
 * Dict -> Tensor
 */
void Obj::toType_(ObjType type) {
    if(m_var_type_current == type) {
        return;
    } else if(type == ObjType::None) {
        clear_();
        return;
    } else if(is_tensor() && isTensor(type)) {

        // Изменить тип тензора
        if(isGenericType(type)) {
            m_var_type_fixed = type;
        }

        if(is_scalar()) {
            if((isGenericType(type) && isContainsType(type, m_var_type_current)) ||
                    (is_integral() && isIntegralType(type, true)) || (is_floating() && isFloatingType(type))) {
                // Ничего менять ненужно
            } else if(isIntegralType(type, true)) {
                m_var = GetValueAsInteger();
            } else if(isFloatingType(type)) {
                m_var = GetValueAsNumber();
            } else {
                LOG_RUNTIME("Unknown convert value!");
            }
            if(!isGenericType(type)) {
                m_var_type_current = type;
            }

        } else {
            ASSERT(m_tensor.defined());
            if(!isGenericType(type)) {
                m_tensor = m_tensor.toType(toTorchType(type));
                m_var_type_current = fromTorchType(m_tensor.scalar_type());
            }
        }

        return;

    } else if(is_range() && isDictionary(type)) {


        ObjPtr value = at("start").second;
        ObjPtr stop = at("stop").second;
        ObjPtr step = at("step").second;

        ASSERT(value);
        ASSERT(stop);
        ASSERT(step);

        Variable::clear_();
        m_var_type_current = type;

        if((*value) < stop) {
            ASSERT(step->GetValueAsNumber() > 0);
            while((*value) < stop) {
                push_back(value->Clone());
                (*value) += step;
            }
        } else {
            ASSERT(step->GetValueAsNumber() < 0);
            while((*value) > stop) {
                push_back(value->Clone());
                (*value) += step;
            }
        }
        return;

    } else if(is_range() && isTensor(type)) {

        toType_(ObjType::Dictionary);
        toType_(type);
        return;

    } else if(is_string_char_type() && isTensor(type)) {

        if(isGenericType(type)) {
            m_var_type_fixed = type;
            m_var_type_current = ObjType::Char;
        } else {
            m_var_type_current = type;
        }

        if(m_value.size() == 1) {
            // Скаляр хранится в нативном типе
            if(isIntegralType(m_var_type_current, true)) {
                m_var = static_cast<int64_t> (m_value[0]);
            } else {
                ASSERT(isFloatingType(m_var_type_current));
                m_var = static_cast<double> (m_value[0]);
            }
        } else {
            ConvertStringToTensor(m_value, m_tensor, m_var_type_current);
        }
        m_value.clear();
        return;

    } else if(is_string_wide_type() && isTensor(type)) {

        if(isGenericType(type)) {
            m_var_type_fixed = type;
            if(sizeof (wchar_t) == 4) {
                m_var_type_current = ObjType::Int;
            } else {
                ASSERT(sizeof (wchar_t) == 2);
                m_var_type_current = ObjType::Short;
            }
        } else {
            m_var_type_current = type;
        }

        if(m_string.size() == 1) {
            // Скаляр хранится в нативном типе
            if(isIntegralType(m_var_type_current, true)) {
                m_var = static_cast<int64_t> (m_string[0]);
            } else {
                ASSERT(isFloatingType(m_var_type_current));
                m_var = static_cast<double> (m_string[0]);
            }
        } else {
            ConvertStringToTensor(m_string, m_tensor, m_var_type_current);
        }
        m_string.clear();
        return;

    } else if(is_tensor() && isStringChar(type)) {

        if(is_scalar()) {
            int64_t char_val = GetValueAsInteger();
            if((char_val < 0 && char_val < std::numeric_limits<char>::min()) ||
                    (char_val > std::numeric_limits<uint8_t>::max())) {
                LOG_ERROR("Single char overflow! %ld", char_val);
            }
            if(char_val < 0) {
                // По стандарту char - знаковый тип
                m_value.assign(1, static_cast<char> (char_val));
            } else {
                // Но часто про это забывают и забивают
                m_value.assign(1, static_cast<uint8_t> (char_val));
            }
            m_var = at::monostate();
        } else {
            ASSERT(!is_scalar());
            if(static_cast<uint8_t> (m_var_type_current) > static_cast<uint8_t> (ObjType::Char)) {
                LOG_ERROR("Possible data loss when converting tensor %s to a byte string!", newlang::toString(m_var_type_current));
            }
            ConvertTensorToString(m_tensor, m_value);
            m_tensor.reset();
        }
        m_var_type_current = type;
        return;

    } else if(is_tensor() && isStringWide(type)) {

        if(is_scalar()) {
            int64_t char_val = GetValueAsInteger();
            if((char_val < std::numeric_limits<wchar_t>::min()) ||
                    (char_val > std::numeric_limits<wchar_t>::max())) {
                LOG_ERROR("Single wchar_t overflow! %ld", char_val);
            }
            m_string.assign(1, static_cast<wchar_t> (char_val));
            m_var = at::monostate();
        } else {
            ASSERT(!is_scalar());
            ASSERT(sizeof (wchar_t) == 2 || sizeof (wchar_t) == 4);
            if((sizeof (wchar_t) == 2 && static_cast<uint8_t> (m_var_type_current) > static_cast<uint8_t> (ObjType::Short)) ||
                    (sizeof (wchar_t) == 4 && static_cast<uint8_t> (m_var_type_current) > static_cast<uint8_t> (ObjType::Int))) {
                LOG_ERROR("Possible data loss when converting tensor %s to a wide string!", newlang::toString(m_var_type_current));
            }
            ConvertTensorToString(m_tensor, m_string);
            m_tensor.reset();
        }
        m_var_type_current = type;
        return;

    } else if(is_tensor() && type == ObjType::Fraction) {

        if(!is_scalar()) {
            LOG_RUNTIME("Convert tensor to fraction support for scalar only!");
        }
        if(is_integral()) {
            m_fraction = std::make_shared<Fraction>(GetValueAsInteger());
            m_var = at::monostate();
        } else {
            LOG_RUNTIME("Convert value '%s' to fraction not implemented!", toString().c_str());
        }
        m_var_type_current = type;
        return;

    } else if(is_tensor() && isDictionary(type)) {

        if(is_scalar() && is_integral()) {
            ASSERT(at::holds_alternative<int64_t>(m_var));
            push_back(Obj::CreateValue(at::get<int64_t>(m_var)));
            m_var = at::monostate();
        } else if(is_scalar() && is_floating()) {
            ASSERT(at::holds_alternative<double>(m_var));
            push_back(Obj::CreateValue(at::get<double>(m_var)));
            m_var = at::monostate();
        } else {
            ASSERT(!is_scalar());
            if(!m_tensor.defined()) {
                ASSERT(m_tensor.defined());
            }
            ConvertTensorToDict(m_tensor, *this);
            m_tensor.reset();
        }
        m_var_type_current = type;
        return;

    } else if(is_dictionary_type() && isTensor(type)) {

        ASSERT(!m_tensor.defined());

        if(isGenericType(type)) {
            m_var_type_fixed = type;
        }

        if(!size()) {
            LOG_RUNTIME("Fail convert empty dictionary to tensor!");
        }

        /*
         * Все элементы словаря переводятся в требуемый тип и определется минимально 
         * возможный тип по его размерности среди всех элементов сконвертированных словаря.
         */

        for (int i = 0; i < size(); i++) {
            at(i).second->toType_(type);
        }
        ObjType summary_type = getSummaryTensorType(this, ObjType::None);

        /*
         * Создается итоговый тензор требуемого типа и в него последоватльно добаляются все элементы словаря.
         */
        torch::Dtype summary_torch_type = toTorchType(static_cast<ObjType> (summary_type));
        if(at(0).second->is_scalar()) {
            if(at(0).second->is_integral()) {
                m_tensor = torch::full({1}, at(0).second->GetValueAsInteger(), summary_torch_type);
            } else {
                if(!at(0).second->is_floating()) {
                    ASSERT(at(0).second->is_floating());
                }
                m_tensor = torch::full({1}, at(0).second->GetValueAsNumber(), summary_torch_type);
            }
        } else {
            m_tensor = at(0).second->m_tensor.toType(summary_torch_type);
        }

        for (int i = 1; i < size(); i++) {

            ObjPtr temp = at(i).second;
            if(!temp) {
                LOG_RUNTIME("Fail convert nullptr to tensor at index %d!", i);
            }

            torch::Tensor temp_tensor;
            if(temp->is_scalar()) {
                ASSERT(!temp->m_tensor.defined());
                if(temp->is_integral()) {
                    temp_tensor = torch::full({1}, temp->GetValueAsInteger(), summary_torch_type);
                } else {
                    if(!temp->is_floating()) {
                        ASSERT(temp->is_floating());
                    }
                    temp_tensor = torch::full({1}, temp->GetValueAsNumber(), summary_torch_type);
                }
            } else {
                ASSERT(temp->m_tensor.defined());
                temp_tensor = temp->m_tensor.toType(summary_torch_type);
            }

            m_tensor = torch::cat({m_tensor, temp_tensor});
        }

        Variable::clear_();

        m_var_type_current = summary_type;

        return;

    }
    LOG_RUNTIME("Can`t convert object type %s to %s!", newlang::toString(m_var_type_current), newlang::toString(type));
}

/*
 * ТИПЫ ДАННЫХ (без аргументов)
 * 
 * :type_int := :Int; # Синоним типа Int во время компиляции (тип не может быть изменен)
 * :type_int := :Int(); # Копия типа Int во время выполнения (тип может быть изменен после Mutable)
 * var_type := :Int; # Тип в переменной, которую можно передавать как аргумент в функции
 * 
 * 
 * ЗНАЧЕНИЯ УКАЗАННЫХ ТИПОВ  (при наличии аргументов)
 * 
 * scalar_int := :Int(0); # Преобразование типа во время выполнения с автоматической размерностью (скаляр)
 * scalar_int := :Int[0](0); # Преобразование типа во время выполнения с указанием размерности (скаляр)
 * scalar_int := :Int[0]([0,]); # Преобразование типа во время выполнения с указанием размерности (скаляр)
 * 
 * tensor_int := :Int([0,]); # Преобразование типа во время выполнения с автоматической размерностью (тензор)
 * tensor_int := :Int[1](0); # Преобразование типа во время выполнения с указанием размерности (тензор)
 * tensor_int := :Int[...](0); # Преобразование типа во время выполнения с произвольной размернотью (тензор)
 */

ObjPtr Obj::CreateBaseType(ObjType type) {

    ObjPtr result = std::make_shared<Obj>(ObjType::Type);
    result->m_class_name = newlang::toString(type);
    result->m_var_type_fixed = type;

    std::string func_proto(result->m_class_name);
    func_proto += "(...)";
    func_proto += result->m_class_name;
    func_proto += ":-{}";

    TermPtr proto = Parser::ParseString(func_proto);
    ASSERT(proto->Left());
    * const_cast<TermPtr *> (&result->m_prototype) = proto->Left();

    result->m_var = (void *) BaseTypeConstructor;
    result->m_is_const = true;

    return result;
}

ObjPtr Obj::BaseTypeConstructor(const Context *ctx, Obj & args) {

    if(args.empty() || !args[0].second) {
        LOG_RUNTIME("Self simple type not defined!");
    }
    ASSERT(args[0].second->getType() == ObjType::Type);

    ObjPtr result = nullptr;
    if(isArithmeticType(args[0].second->m_var_type_fixed)) {
        result = ConstructorSimpleType_(ctx, args);
    } else if(args[0].second->m_var_type_fixed == ObjType::Dictionary) {
        result = ConstructorDictionary_(ctx, args);
    } else if(args[0].second->m_var_type_fixed == ObjType::Pointer && args.size() > 1) {
        result = ConstructorNative_(ctx, args);
    } else if(args[0].second->m_var_type_fixed == ObjType::Class) {
        result = ConstructorClass_(ctx, args);
    } else if(args[0].second->m_var_type_fixed == ObjType::Struct || args[0].second->m_var_type_fixed == ObjType::Union) {
        result = ConstructorStruct_(ctx, args);
    } else if(args[0].second->m_var_type_fixed == ObjType::Enum) {
        result = ConstructorEnum_(ctx, args);
    } else if(args[0].second->m_var_type_fixed == ObjType::Return) {
        result = ConstructorReturn_(ctx, args);
    } else if(args[0].second->m_var_type_fixed == ObjType::Break || args[0].second->m_var_type_fixed == ObjType::Continue) {
        result = ConstructorInterraption_(ctx, args, args[0].second->m_var_type_fixed);
    } else if(args[0].second->m_var_type_fixed == ObjType::Error || args[0].second->m_var_type_fixed == ObjType::ErrorParser
            || args[0].second->m_var_type_fixed == ObjType::ErrorRunTime || args[0].second->m_var_type_fixed == ObjType::ErrorSignal) {
        result = ConstructorError_(ctx, args);
    } else {

        result = args[0].second->Clone();
        if(result) {
            result->m_is_const = false;
        }
    }

    if(!result) {
        LOG_RUNTIME("Create type '%s' error or not implemented!", newlang::toString(args[0].second->m_var_type_fixed));
    }

    result->m_class_name = args[0].second->m_class_name;

    return result;
}

ObjPtr Obj::ConstructorSimpleType_(const Context *ctx, Obj & args) {

    ASSERT(!args.empty() && args[0].second);
    ASSERT(args[0].second->getType() == ObjType::Type);

    // Переданы значения для приведения типов
    // Но само значение пока не установлено
    ObjPtr result = args[0].second->Clone();
    if(args.size() == 1) {
        // Копия существующего типа с возможностью редактирования
        result->m_is_const = false;
        return result;
    }

    std::vector<int64_t> dims;

    if(result->m_dimensions) {
        // Размерность указана
        for (int i = 0; i < result->m_dimensions->size(); i++) {
            Index ind = (*result->m_dimensions)[i].second->toIndex();
            if(ind.is_integer()) {
                dims.push_back(ind.integer());
            } else if(ind.is_boolean()) {
                dims.push_back(ind.boolean());
            } else {
                LOG_RUNTIME("Non fixed dimension not implemented!");
            }
        }
    }


    if(args.size() == 2) {
        // Передано единственное значение (нулевой аргумент - сам объект, т.е. :Тип(Значение) )

        ObjPtr convert;

        // Если обобщенный тип данных, а сами данные принадлежат обощенному типу
        if(isGenericType(result->m_var_type_fixed) && isContainsType(result->m_var_type_fixed, args[1].second->getType())) {
            convert = args[1].second->Clone();
        } else {
            convert = args[1].second->toType(result->m_var_type_fixed);
        }
        convert->m_var_type_fixed = result->m_var_type_fixed;
        convert.swap(result);

    } else {

        // Для списка значений сперва формируется словарь, а после он конвертируется в нужный тип данных

        result->m_var_type_current = ObjType::Dictionary;

        ObjPtr prev = nullptr;
        for (int i = 1; i < args.size(); i++) {

            if(args[i].second->getType() == ObjType::Ellipsis) {
                if(!prev) {
                    LOG_RUNTIME("There is no previous item to repeat!");
                }
                if(i + 1 != args.size()) {
                    LOG_RUNTIME("Ellipsis is not the last element!");
                }
                if(dims.empty()) {
                    LOG_RUNTIME("Object has no dimensions!");
                }
                int64_t full_size = 1;
                for (int j = 0; j < dims.size(); j++) {
                    full_size *= dims[j];
                }
                if(full_size <= 0) {
                    LOG_RUNTIME("Items count error for all dimensions!");
                }

                for (int64_t j = result->size(); j < full_size; j++) {
                    result->op_concat_(prev, ConcatMode::Append);
                }

                break;

            } else {
                prev = args[i].second;
            }

            result->op_concat_(prev, ConcatMode::Append);
        }

        if(args[0].second->m_var_type_fixed != ObjType::Dictionary) {
            result = result->toType(args[0].second->m_var_type_fixed);
            result->m_var_type_fixed = result->m_var_type_current;
        }
    }


    if(!dims.empty()) {

        if(isString(result->getType()) || isDictionary(result->getType())) {
            if(dims.size() != 1) {
                LOG_RUNTIME("Fail size for type '%s'!", newlang::toString(result->getType()));
            }
            result->resize_(dims[0], nullptr);
        } else if(isTensor(result->getType())) {
            if(dims.size() == 1 && dims[0] == 0) {
                // Скаляр
                if(args.size() == 2 && args[0].second->m_var_type_fixed == ObjType::Bool) {
                    result->m_var = static_cast<int64_t> (result->empty() ? 0 : 1);
                    result->m_tensor.reset();
                    return result;
                } else if(result->size() != 0) {
                    LOG_RUNTIME("Only one value is required for a scalar!");
                }
                dims.clear();
            } else {
                result->m_tensor = result->m_tensor.reshape(dims);
            }
        } else {

            LOG_RUNTIME("Fail esing dimensions for type '%s'!", newlang::toString(result->getType()));
        }
    }
    return result;
}

/*
 * :Class(One=0, Two=_, Three=3); # Все аргументы имеют имена
 */

ObjPtr Obj::ConstructorDictionary_(const Context *ctx, Obj & args) {

    ASSERT(!args.empty() && args[0].second);
    ASSERT(args[0].second->getType() == ObjType::Type);

    ObjPtr result = Obj::CreateDict();
    for (int i = 1; i < args.size(); i++) {
        result->push_back(args[i].second, args.name(i));
    }
    result->m_var_is_init = true;

    return result;
}

ObjPtr Obj::ConstructorNative_(const Context *ctx_const, Obj & args) {
    if(args.size() < 2) {
        LOG_RUNTIME("Empty argument list!");
    }
    if(!args.at(1).second->is_string_type()) {
        LOG_RUNTIME("First argument not a string!");
    }
    //@todo Передача дополнительных аргументов? args["module"]->GetValueAsString().c_str(), args["lazzy"]->GetValueAsBoolean()
    Context *ctx = const_cast<Context *> (ctx_const);
    return ctx->CreateNative(args.at(1).second->GetValueAsString().c_str());
}

ObjPtr Obj::ConstructorClass_(const Context *ctx, Obj & args) {
    ObjPtr result = ConstructorDictionary_(ctx, args);
    result->m_var_type_fixed = ObjType::Class;
    result->m_var_is_init = true;
    for (int i = 1; i < result->size(); i++) {
        if(result->name(i).empty()) {
            LOG_RUNTIME("Field pos %d has no name!", i);
        }
        for (int pos = 0; pos < i; pos++) {
            if(result->name(pos).compare(result->name(i)) == 0) {
                LOG_RUNTIME("Field name '%s' at index %d already exists!", result->name(i).c_str(), i);
            }
        }
    }
    return result;
}

ObjPtr Obj::ConstructorStruct_(const Context *ctx, Obj & args) {
    ObjPtr result = ConstructorClass_(ctx, args);
    result->m_var_type_fixed = ObjType::Struct;

    if(!result->size()) {
        LOG_RUNTIME("Empty Struct not allowed!");
    }

    for (int i = 0; i < result->size(); i++) {
        if(!(*result)[i].second) {
            LOG_RUNTIME("Field '%s' at pos %d not defined!", result->name(i).c_str(), i);
        }
        if(!(*result)[i].second || !isSimpleType((*result)[i].second->getType()) || isGenericType((*result)[i].second->getType())) {

            LOG_RUNTIME("Field '%s' at pos %d not simple type! (%s)", result->name(i).c_str(), i, newlang::toString((*result)[i].second->getType()));
        }
    }
    return result;
}

/*
 * :Enum(One=0, Two=_, "Three", Ten=10);
 */

ObjPtr Obj::ConstructorEnum_(const Context *ctx, Obj & args) {
    ObjPtr result = Obj::CreateDict();
    result->m_var_type_fixed = ObjType::Enum;

    int64_t val_int = 0;
    ObjPtr enum_value;
    std::string enum_name;

    for (int i = 1; i < args.size(); i++) {
        if(args.name(i).empty()) {
            if(args[i].second && args[i].second->is_string_type()) {
                enum_name = args[i].second->GetValueAsString();
            } else {
                LOG_RUNTIME("Field pos %d has no name!", i);
            }
        } else {
            enum_name = args.name(i);

            if(args[i].second && (args[i].second->is_integral())) {
                val_int = args[i].second->GetValueAsInteger();
            } else if(!args[i].second || !args[i].second->is_none_type()) {
                LOG_RUNTIME("Field value '%s' %d must integer type!", args.name(i).c_str(), i);
            }
        }

        if(result->find(enum_name) != result->end()) {
            LOG_RUNTIME("Field value '%s' at index %d already exists!", enum_name.c_str(), i);
        }


        enum_value = Obj::CreateValue(val_int, ObjType::None); // , type
        enum_value->m_var_type_fixed = enum_value->m_var_type_current;
        enum_value->m_is_const = true;
        result->push_back(enum_value, enum_name);
        val_int += 1;
    }

    result->m_is_const = true;
    result->m_var_is_init = true;

    return result;
}

ObjPtr Obj::ConstructorError_(const Context *ctx, Obj & args) {
    ObjPtr result = ConstructorDictionary_(ctx, args);
    result->m_var_type_current = ObjType::Error;
    result->m_var_type_fixed = ObjType::Error;
    if(!result->size()) {

        LOG_RUNTIME("Argument for type ':Error' required!");
    }
    return result;
}

ObjPtr Obj::ConstructorReturn_(const Context *ctx, Obj & args) {
    ObjPtr result = ConstructorDictionary_(ctx, args);
    result->m_var_type_current = ObjType::Return;
    result->m_var_type_fixed = ObjType::Return;
    if(result->size() == 0) {
        result->push_back(Obj::Arg(Obj::CreateNone()));
    }
    if(result->size() != 1) {

        LOG_RUNTIME("Multiple argument for type ':Return'!");
    }
    return result;
}

ObjPtr Obj::ConstructorInterraption_(const Context* ctx, Obj& args, ObjType type) {
    ObjPtr result = ConstructorDictionary_(ctx, args);
    result->m_var_type_current = type;
    result->m_var_type_fixed = type;
    if(result->size()) {
        LOG_RUNTIME("Argument for type %s not allowed!", newlang::toString(type));
    }
    return result;
}

template<>
const Iterator<Obj>::IterPairType Iterator<Obj>::m_interator_end = IterObj::pair(Obj::CreateType(ObjType::IteratorEnd, ObjType::IteratorEnd, true));

template<>
ObjPtr Iterator<Obj>::read_and_next(int64_t count) {
    ObjPtr result;

    if(count == 0) {
        result = (*(*this)).second;
        (*this)++;
    } else if(count < 0) {

        result = Obj::CreateDict();
        for (int64_t i = 0; i < -count; i++) {
            if(*this != this->end()) {
                result->push_back(*(*this));
                (*this)++;
            } else {
                result->push_back(Obj::CreateType(ObjType::IteratorEnd));
            }
        }

    } else {
        result = Obj::CreateDict();
        while(*this != this->end() && result->size() < count) {
            result->push_back(*(*this));
            (*this)++;
        }
    }
    return result;
}

ObjPtr Obj::MakeIterator(const std::string filter, bool check_create) {
    ObjPtr result = CreateType(ObjType::Iterator, ObjType::Iterator, true);
    if(!is_indexing()) {
        if(getType() == ObjType::Iterator && !check_create) {
            return shared();
        }
        LOG_RUNTIME("Can't create iterator from '%s'!", toString().c_str());
    }
    result->m_iterator = std::make_shared<Iterator < Obj >> (shared(), filter);
    return result;
}

ObjPtr Obj::MakeIterator(Obj * args) {
    ObjPtr result = CreateType(ObjType::Iterator, ObjType::Iterator, true);
    if(!is_indexing()) {
        if(getType() == ObjType::Iterator) {
            return shared();
        }
        LOG_RUNTIME("Can't create iterator from '%s'!", toString().c_str());
    }

    if(!args || args->size() == 0) {
        result->m_iterator = std::make_shared<Iterator < Obj >> (shared());
    } else if(args->size() == 1 && args->at(0).second && args->at(0).second->is_string_type()) {
        result->m_iterator = std::make_shared<Iterator < Obj >> (shared(), args->GetValueAsString());
    } else if(args->size() >= 1 && args->at(0).second && args->at(0).second->is_function()) {
        ASSERT(false);
        //        ObjPtr func = args->at(0).second;
        //        ObjPtr func_arg = args->Clone();
        //        func_arg->resize_(-(func_arg->size() - 1)); // Удалить первый элемент
        //        result->m_iterator = std::make_shared<Iterator < Obj >> (shared(), func.get(), func_arg.get());
    } else {
        LOG_RUNTIME("Invalid arguments for iterator create! '%s'", args->toString().c_str());
    }
    return result;
}

ObjPtr Obj::IteratorReset() {
    if(m_var_type_current != ObjType::Iterator) {
        LOG_RUNTIME("Method available an iterator only!");
    }
    ASSERT(m_iterator);
    m_iterator->reset();
    return shared();
}

ObjPtr Obj::IteratorNext(int64_t count) {
    if(m_var_type_current != ObjType::Iterator) {
        LOG_RUNTIME("Method available an iterator only!");
    }
    ASSERT(m_iterator);
    return m_iterator->read_and_next(count);
}

//void Obj::SetHelp(const std::string &help) {
//    Context::HelpInfo help_info = Context::;
//    std::shared_ptr<std::string> m_help = help;
//}

