
#include "contrib/logger/logger.h"
#include "core/types.h"
#include "core/variable.h"
#include "pch.h"

#include <core/context.h>
#include <core/newlang.h>
#include <core/object.h>
#include <core/term.h>

using namespace newlang;

std::ostream &operator<<(std::ostream &out, newlang::Obj &var) {
    out << var.toString().c_str();
    return out;
}

std::ostream &operator<<(std::ostream &out, newlang::ObjPtr var) {
    if (var) {
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
    if (has_error) {
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
    snprintf(m_buffer_message, sizeof (m_buffer_message), "Interrupt%s%s%s!", m_obj ? " data: '" : "", m_obj ? m_obj->toString().c_str() : "", m_obj ? "'" : "");
}

const char* Interrupt::what() const noexcept {
    return &m_buffer_message[0];
}

int64_t Obj::size(int64_t dim) const {
    if (is_tensor()) {
        if (is_scalar()) {
            if (dim != 0) {
                LOG_RUNTIME("Scalar has zero dimension!");
            }
            return 0;
        }
        return m_value.size(dim);
    }
    ASSERT(dim == 0);
    if (m_var_type_current == ObjType::StrChar) {
        return m_str.size();
    } else if (m_var_type_current == ObjType::StrWide) {
        return m_wstr.size();
    }
    return Variable::size();
}

int64_t Obj::resize_(int64_t size, ObjPtr fill, const std::string name) {

    if (is_string_type()) {

        if (size >= 0) {
            // Размер положительный, просто изменить число элементов добавив или удалив последние
            if (m_var_type_current == ObjType::StrChar) {
                m_str.resize(size, ' ');
                return m_str.size();
            } else if (m_var_type_current == ObjType::StrWide) {
                m_wstr.resize(size, L' ');
                return m_wstr.size();
            }
        } else {
            // Если размер отрицательный - добавить или удалить вначале
            size = -size;
            if (m_data.size() > size) {
                if (m_var_type_current == ObjType::StrChar) {
                    m_str.erase(0, size);
                    return m_str.size();

                } else if (m_var_type_current == ObjType::StrWide) {
                    m_str.erase(0, size);
                    return m_wstr.size();
                }
            } else if (m_data.size() < size) {
                if (m_var_type_current == ObjType::StrChar) {
                    m_str.insert(0, size, ' ');
                    return m_str.size();

                } else if (m_var_type_current == ObjType::StrWide) {
                    m_wstr.insert(0, size, L' ');
                    return m_wstr.size();

                }
            }
        }

    } else if (is_dictionary_type()) {
        return Variable::resize(size, fill ? fill : Obj::CreateNone(), name);
    } else if (is_tensor()) {
        std::vector<int64_t> sizes;
        for (int i = 0; i < m_value.dim(); i++) {
            sizes.push_back(m_value.size(i));
        }

        if (sizes.empty()) { // Scalar

            LOG_RUNTIME("Method resize for SCALAR type '%s' not implemented!", newlang::toString(m_var_type_current));

        } else if (size == 0 || sizes[0] == size) {
            // Tensor size OK - do nothing            
        } else if (size > 0) { // Increase tensor size

            // The size is positive, just change the number of elements by adding or removing the last
            ASSERT(sizes.size() == 1);

            sizes[0] = size;
            m_value.resize_(at::IntArrayRef(sizes));

        } else { // Decrease tensor size
            // If the size is negative - add or remove elements first
            size = -size;
            if (sizes[0] == size) {
                // Tensor size OK - do nothing            
            } else if (sizes[0] > size) {

                ASSERT(sizes.size() == 1);

                at::Tensor ind = torch::arange(sizes[0] - size - 1, sizes[0] - 1, at::ScalarType::Long);
                at::Tensor any = torch::zeros(sizes[0] - size, at::ScalarType::Long);
                //                LOG_DEBUG("arange %s    %s", TensorToString(ind).c_str(), TensorToString(any).c_str());

                ind = at::cat({any, ind});
                //                LOG_DEBUG("cat %s", TensorToString(ind).c_str());

                //                LOG_DEBUG("m_value %s", TensorToString(m_value).c_str());
                m_value.index_copy_(0, ind, m_value.clone());
                //                LOG_DEBUG("index_copy_ %s", TensorToString(m_value).c_str());

                sizes[0] = size;
                m_value.resize_(at::IntArrayRef(sizes));
                //                LOG_DEBUG("resize_ %s", TensorToString(m_value).c_str());

            } else { // sizes[0] < size
                ASSERT(sizes.size() == 1);
                
                m_value = at::cat({torch::zeros(size - sizes[0], m_value.scalar_type()), m_value});
          }
        }

        if (size == 0) {
            m_value.reset();
            m_var_is_init = false;
        }
        return size;
    }
    LOG_RUNTIME("Method resize for type '%s' not implemented!", newlang::toString(m_var_type_current));
}

const Variable<ObjPtr>::PairType & Obj::at(int64_t index) const {
    if (m_var_type_current == ObjType::StrChar) {
        if (index < m_str.size()) {
            m_str_pair = pair(CreateString(std::string(1, m_str[index])));
            return m_str_pair;
        }
        LOG_RUNTIME("Index '%lu' not exists in byte string '%s'!", static_cast<unsigned long> (index), m_str.c_str());
    } else if (m_var_type_current == ObjType::StrWide) {
        if (index < m_wstr.size()) {
            m_str_pair = pair(CreateString(std::wstring(1, m_wstr[index])));
            return m_str_pair;
        }
        LOG_RUNTIME("Index '%lu' not exists in byte string '%s'!", static_cast<unsigned long> (index), "WIDE");

    } else if (is_tensor()) {
        torch::Tensor t = m_value.index({(int) index});
        m_str_pair = pair(Obj::CreateTensor(t));
        return m_str_pair;
    }
    return Variable::at(index);
}

Variable<ObjPtr>::PairType & Obj::at(int64_t index) {
    if (m_var_type_current == ObjType::StrChar) {
        if (index < m_str.size()) {
            m_str_pair = pair(CreateString(std::string(1, m_str[index])));
            return m_str_pair;
        }
        LOG_RUNTIME("Index '%lu' not exists in byte string '%s'!", static_cast<unsigned long> (index), m_str.c_str());
    } else if (m_var_type_current == ObjType::StrWide) {
        if (index < m_wstr.size()) {
            m_str_pair = pair(CreateString(std::wstring(1, m_wstr[index])));
            return m_str_pair;
        }
        LOG_RUNTIME("Index '%lu' not exists in byte string '%s'!", static_cast<unsigned long> (index), "WIDE");

    } else if (is_tensor()) {
        torch::Tensor t = m_value.index({(int) index});
        m_str_pair = pair(Obj::CreateTensor(t));
        return m_str_pair;
    }
    return Variable::at(index);
}

const ObjPtr Obj::index_get(const std::vector<Index> &index) const {
    if (m_var_type_current == ObjType::StrChar) {
        if (index.size() != 1 || !index[0].is_integer()) {
            LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
        }
        if (index[0].integer() < m_str.size()) {
            return CreateString(std::string(1, m_str[index[0].integer()]));
        }
        LOG_RUNTIME("Index '%s' not exists in byte string '%s'!", IndexToString(index).c_str(), m_str.c_str());
    } else if (m_var_type_current == ObjType::StrWide) {
        if (index.size() != 1 || !index[0].is_integer()) {
            LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
        }
        if (index[0].integer() < m_wstr.size()) {
            return CreateString(std::wstring(1, m_wstr[index[0].integer()]));
        }
        LOG_RUNTIME("Index '%s' not exists in byte string '%s'!", IndexToString(index).c_str(), "WIDE");

    } else if (is_tensor()) {
        torch::Tensor t = m_value.index(index);
        return Obj::CreateTensor(t);
    }

    if (index.size() != 1 || !index[0].is_integer()) {
        LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
    }
    return Variable::at(index[0].integer()).second;
}

ObjPtr Obj::index_set_(const std::vector<Index> &index, const ObjPtr value) {
    if (m_var_type_current == ObjType::StrChar) {
        if (index.size() != 1 || !index[0].is_integer()) {
            LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
        }
        if (index[0].integer() < m_str.size()) {
            m_str.erase(index[0].integer(), 1);
            m_str.insert(index[0].integer(), value->toType(ObjType::StrChar)->m_str);
            m_var_is_init = true;
            return shared();
        }
        LOG_RUNTIME("Index '%s' not exists in byte string '%s'!", IndexToString(index).c_str(), m_str.c_str());
    } else if (m_var_type_current == ObjType::StrWide) {
        if (index.size() != 1 || !index[0].is_integer()) {
            LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
        }
        if (index[0].integer() < m_wstr.size()) {
            m_wstr.erase(index[0].integer(), 1);
            m_wstr.insert(index[0].integer(), value->toType(ObjType::StrWide)->m_wstr);
            m_var_is_init = true;
            return shared();
        }
        LOG_RUNTIME("Index '%s' not exists in byte string '%s'!", IndexToString(index).c_str(), "WIDE");

    } else if (is_tensor()) {

        m_value.index_put_(index, value->toTensor());
        m_var_is_init = true;
        return shared();

    } else if (is_dictionary_type()) {
        if (index.size() != 1 || !index[0].is_integer()) {
            LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
        }
        (*at(index[0].integer()).second) = value;
        return shared();
        LOG_RUNTIME("Index '%s' not exists in object '%s'!", IndexToString(index).c_str(), toString().c_str());
    }
    LOG_RUNTIME("Don`t set index '%s' in object '%s'!", IndexToString(index).c_str(), toString().c_str());
}

ObjPtr Obj::op_set_index(size_t index, std::string value) {
    if (m_var_type_current == ObjType::StrChar) {
        if (index < m_str.size()) {
            m_str.erase(index, 1);
            m_str.insert(index, value);
            m_var_is_init = true;
            return shared();
        }
        LOG_RUNTIME("Index '%lu' not exists in byte string '%s'!", static_cast<unsigned long> (index), m_str.c_str());
    } else if (m_var_type_current == ObjType::StrWide) {
        if (index < m_wstr.size()) {
            m_wstr.erase(index, 1);
            m_wstr.insert(index, utf8_decode(value));
            m_var_is_init = true;
            return shared();
        }
        LOG_RUNTIME("Index '%lu' not exists in byte string '%s'!", static_cast<unsigned long> (index), "WIDE");
    }
    //    at(index).second.set_(value);
    (*at(index).second) = value;
    return Variable::operator[](index);
}

bool Obj::exist(ObjPtr &find, bool strong) {
    for (auto &elem : * this) {
        if (strong && find->op_accurate(elem.second)) {
            return true;
        } else if (!strong && find->op_equal(elem.second)) {
            return true;
        }
    }
    return false;
}

/*
 * Вычисление итогового типа объетка для арифметических типов.
 * Арифметические данные могут быть тензор или ссылка на рельный тип.
 * Тензор может сменить тип, если он не указан для конкретной переменной.
 * Реальный тип данных изменить тип не может.
 */

ObjType newlang::getSummaryTensorType(ObjPtr &obj, ObjType start) {
    ObjType result = ObjType::None;
    if (!obj->getName().empty()) {
        LOG_RUNTIME("Tensor does not support named data or dimensions '%s'!", obj->getName().c_str());
    }
    if (obj->is_dictionary_type()) {
        for (size_t i = 0; i < obj->size(); i++) {
            result = getSummaryTensorType(obj->at(i).second, result);
        }
        return result;
    } else if (obj->is_arithmetic_type() || obj->is_bool_type()) {
        if (start >= std::max(obj->m_var_type_current, obj->m_var_type_fixed)) {
            return start;
        } else {
            return std::max(obj->m_var_type_current, obj->m_var_type_fixed);
        }
    }
    LOG_RUNTIME("Tensor support arithmetic data type only '%s'!", obj->toString().c_str());
}

ObjPtr Obj::operator+=(Obj value) {
    if (is_tensor()) {
        if (value.is_tensor()) {
            testResultIntegralType(value.m_var_type_current, true);
            m_value.add_(value.m_value);
            return shared();
        }
    }
    switch (m_var_type_current) {
        case ObjType::StrChar:
        case ObjType::StrWide:
            switch (value.m_var_type_current) {
                case ObjType::None:
                    return shared();
                case ObjType::StrChar:
                    m_str += value.m_str;
                    return shared();
                case ObjType::StrWide:
                    m_wstr += value.m_wstr;
                    return shared();
            }
            break;

        case ObjType::Class:
        case ObjType::Dictionary:
            if (value.m_var_type_current == ObjType::None) {
                return shared();
            } else if (value.m_var_type_current == ObjType::Class || value.m_var_type_current == ObjType::Dictionary) {
                for (size_t i = 0; i < value.size(); i++) {
                    push_back(value.at(i));
                }
                return shared();
            }
            break;
    }
    LOG_RUNTIME("Operator '+' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

ObjPtr Obj::operator-=(Obj value) {
    if (m_var_type_current == ObjType::None) {
        m_var_type_current = value.m_var_type_current;
    } else if (is_tensor() && value.is_tensor()) {
        testResultIntegralType(value.m_var_type_current, true);
        if (is_bool_type()) {
            toType_(ObjType::Char);
        }
        if (value.is_bool_type()) {
            value.toType_(ObjType::Char);
        }
        m_value.sub_(value.m_value);
        return shared();
    }
    switch (m_var_type_current) {
        case ObjType::Class:
        case ObjType::Dictionary:
            if (value.m_var_type_current == ObjType::None) {
                return shared();
            } else if (value.m_var_type_current == ObjType::Class || value.m_var_type_current == ObjType::Dictionary) {
                for (size_t i = 0; i < value.size(); i++) {
                    auto found = select(value.name(i));
                    if (!found.complete()) {
                        erase(found);
                    }
                }
                return shared();
            }
            break;
    }
    LOG_RUNTIME("Operator '-' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

ObjPtr Obj::operator*=(Obj value) {
    if (m_var_type_current == ObjType::None) {
        m_var_type_current = value.m_var_type_current;
    } else if (is_tensor() && value.is_tensor()) {
        testResultIntegralType(value.m_var_type_current, true);
        m_value.mul_(value.m_value);
        return shared();
    }

    switch (m_var_type_current) {

        case ObjType::Class:
        case ObjType::Dictionary:
            if (value.m_var_type_current == ObjType::None) {
                Variable::clear_();
                return shared();
            } else if (value.m_var_type_current == ObjType::Class || value.m_var_type_current == ObjType::Dictionary) {
                op_bit_and_set(value, false);
                return shared();
            }
            break;

        case ObjType::StrChar:
            if (value.is_integer()) {
                m_str = repeat(m_str, value.GetValueAsInteger());
                return shared();
            } else if (value.is_string_type()) {
                m_str += value.GetValueAsString();
                return shared();
            }
        case ObjType::StrWide:
            if (value.is_integer()) {
                m_wstr = repeat(m_wstr, value.GetValueAsInteger());
                return shared();
            } else if (value.is_string_type()) {
                m_wstr += utf8_decode(value.GetValueAsString());
                return shared();
            }

    }
    LOG_RUNTIME("Operator '*' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

ObjPtr Obj::operator/=(Obj value) {
    if (is_tensor() && value.is_tensor()) {
        testResultIntegralType(ObjType::Double, false);
        m_value.div_(value.m_value);
        return shared();
    }
    LOG_RUNTIME("Operator '/' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

ObjPtr Obj::op_div_ceil_(Obj & value) {
    if (is_tensor() && value.is_tensor()) {
        ObjType type = m_var_type_current;
        testResultIntegralType(ObjType::Float, false);
        m_value.div_(value.m_value, "floor");
        m_value = m_value.toType(toTorchType(type));
        return shared();
    }
    LOG_RUNTIME("Operator '//' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

ObjPtr Obj::operator%=(Obj value) {
    if (is_tensor() && value.is_tensor()) {
        testResultIntegralType(value.m_var_type_current, false);
        m_value.fmod_(value.m_value);
        return shared();
    }
    LOG_RUNTIME("Operator '%%' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

size_t Obj::ItemValueCount(ObjPtr &find, bool strong) {
    size_t result = 0;
    for (auto &elem : * this) {
        if (strong && find->op_accurate(elem.second)) {
            result++;
        } else if (!strong && find->op_equal(elem.second)) {
            result++;
        }
    }
    return result;
}

void Obj::CloneDataTo(Obj & clone) const {

    NL_CHECK(!isLocalType(m_var_type_current), "Local object not clonable!");

    if (&clone != this) { // Не клонировать сам в себя
        clone.m_var_type_current = m_var_type_current;
        clone.m_var_type_fixed = m_var_type_fixed;
        clone.m_var_is_init = m_var_is_init;
        //        clone.m_var_type_name = m_var_type_name;

        if (m_dimensions) {
            clone.m_dimensions = m_dimensions->Clone();
        }

        clone.m_var_name = m_var_name;
        clone.m_str = m_str;
        clone.m_wstr = m_wstr;

        clone.m_class_parents = m_class_parents;
        clone.m_class_name = m_class_name;
        //        clone.m_ctx = m_ctx;
        clone.m_is_const = false;

        clone.m_ref_count = m_ref_count;
        clone.m_func_ptr = m_func_ptr;
        *const_cast<TermPtr *> (&clone.m_func_proto) = m_func_proto;
        if (is_tensor() && m_var_is_init) {
            clone.m_value = m_value.clone();
        }
    }
}

void Obj::ClonePropTo(Obj & clone) const {

    NL_CHECK(!isLocalType(m_var_type_current), "Local object not clonable!");

    for (int i = 0; i < Variable<ObjPtr>::size(); i++) {
        if (Variable<ObjPtr>::at(i).second) {
            if (Variable<ObjPtr>::at(i).second->m_is_reference || Variable<ObjPtr>::at(i).second->m_is_reference) {
                clone.push_back(Variable<ObjPtr>::at(i));
            } else {
                clone.push_back(Variable<ObjPtr>::at(i).second->Clone(nullptr), name(i));
            }
        } else {
            if (name(i).empty()) {
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

void newlang::calcTensorDims(ObjPtr &obj, std::vector<int64_t> &dims) {
    if (obj->is_dictionary_type()) {
        dims.push_back(obj->size());
        if (obj->size()) {
            calcTensorDims(obj->at(0).second, dims);
        }
    }
}

ObjPtr Obj::GetIndex(ObjPtr obj, TermPtr index_arg) {
    ASSERT(index_arg->size() == 1);
    TermPtr index = index_arg->at(0).second;
    ASSERT(index);
    if (index->getTermID() != TermID::INTEGER) {
        LOG_RUNTIME("Fail index type %s '%s'", newlang::toString(index->getTermID()), index->toString().c_str());
    }
    return GetIndex(obj, std::stoi(index->m_text.c_str()));
}

std::string Obj::toString(bool deep) const {
    std::string result(m_is_reference ? "&" : "");
    result += m_var_name;
    if (!result.empty() && m_var_type_current == ObjType::Class && !deep && m_is_reference) {
        return result;
    }
    if (!m_var_name.empty()) {
        result.append("=");
    }
    size_t dot, last;

    std::stringstream ss;

    if (m_var_type_current == ObjType::None) {
        if (m_func_proto && m_func_proto->GetType()) {
            result += m_func_proto->GetType()->toString();
        } else if (m_var_type_fixed != ObjType::None) {
            result += newlang::toString(m_var_type_fixed);
        }
        result += "_";
        return result;
    } else if (is_tensor()) {
        if (is_scalar()) {
            result += GetValueAsString();
        } else {
            result += "[";
            dump_tensor_(result);
            result += ",";
            result += "]";
        }
        return result;
    } else if (isSimpleType(m_var_type_current)) {
        result += GetValueAsString();
        return result;
    } else {
        switch (m_var_type_current) {

            case ObjType::None: // name:=<EMPTY>
                result = "_";
                return result;

            case ObjType::StrChar:
                result += "'";
                result += m_str;
                result.append("'");
                return result;

            case ObjType::StrWide: // name:='string' or name:="string"
                result += "\"";
                result += utf8_encode(m_wstr);
                result.append("\"");
                return result;

            case ObjType::Range: // name:=(1,second="two",3,<EMPTY>,5)
                result = at("start")->GetValueAsString();
                result += "..";
                result += at("stop")->GetValueAsString();
                result += "..";
                result += at("step")->GetValueAsString();
                return result;

            case ObjType::Struct:
            case ObjType::Union:
            case ObjType::Enum:

            case ObjType::Return:
            case ObjType::Error:
            case ObjType::Continue:
            case ObjType::Break:
                if (m_class_name.empty()) {
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
                ss << m_func_ptr;
                result += ss.str();
                result += ":Pointer";
                return result;


            case ObjType::Type:
                result += newlang::toString(m_var_type_fixed);
                if (m_dimensions && m_dimensions->size()) {
                    result += "[";
                    for (int i = 0; i < m_dimensions->size(); i++) {
                        if (i) {
                            result += ",";
                        }
                        result += (*m_dimensions)[i]->toString();
                    }
                    result += "]";
                }


                if (size()) {
                    result += "(";
                    dump_dict_(result);
                    result += ")";
                }
                return result;

            case ObjType::NativeFunc:
            case ObjType::FUNCTION: // name:={function code}
            case ObjType::TRANSPARENT: // name=>{function code}
                ASSERT(m_func_proto);
                result += m_func_proto->m_text;
                result += "(";
                m_func_proto->dump_items_(result);
                result += ")";
                if (!m_func_proto->m_type_name.empty()) {
                    result += m_func_proto->m_type_name;
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
                ASSERT(m_func_proto);
                result += m_func_proto->m_text;
                result += "(";
                m_func_proto->dump_items_(result);
                result += ")";
                if (!m_func_proto->m_type_name.empty()) {
                    result += m_func_proto->m_type_name;
                }

                if (m_var_type_current == ObjType::EVAL_FUNCTION) {
                    result += ":=";
                } else if (m_var_type_current == ObjType::SimplePureFunc) {
                    result += ":-";
                } else if (m_var_type_current == ObjType::SimplePureAND) {
                    result += "::&&=";
                } else if (m_var_type_current == ObjType::SimplePureOR) {
                    result += "::||=";
                } else if (m_var_type_current == ObjType::SimplePureXOR) {
                    result += "::^^=";
                } else {
                    LOG_RUNTIME("Fail function type");
                }

                if (m_block_source->getTermID() != TermID::BLOCK) {
                    result += "{";
                }
                if (m_block_source) {
                    result += m_block_source->toString();
                    if (m_block_source->getTermID() != TermID::BLOCK) {
                        result += ";";
                    }
                }
                if (m_block_source->getTermID() != TermID::BLOCK) {
                    result.append("}");
                }
                return result;

            case ObjType::Class: // name:=@term(id=123, ...) name=base(id=123, ... )
                result += m_class_name;
                result += "(";

                if (!empty()) {
                    dump_dict_(result);
                }
                result += ")";
                return result;

            case ObjType::Ellipsis:
                result += "...";
                return result;
        }
    }
    LOG_RUNTIME("Unknown type '%s' (%d)", newlang::toString(m_var_type_current), (int) m_var_type_current);
}

void TensorToString_(const torch::Tensor &tensor, c10::IntArrayRef shape, std::vector<Index> &ind, const int64_t pos,
        std::stringstream & str) {
    std::string intend;
    ASSERT(pos < ind.size());
    str << "[";
    if (shape.size() > 1 && pos + 1 < ind.size()) {
        str << "\n";
        intend = std::string((pos + 1) * 2, ' ');
        str << intend;
    }
    if (pos + 1 < ind.size()) {
        bool comma = false;
        for (ind[pos] = 0; ind[pos].integer() < shape[pos]; ind[pos] = ind[pos].integer() + 1) {
            if (comma) {
                str << ", ";
            } else {
                comma = true;
            }
            TensorToString_(tensor, shape, ind, pos + 1, str);
        }
    } else {
        bool comma = false;
        for (ind[pos] = 0; ind[pos].integer() < shape[pos]; ind[pos] = ind[pos].integer() + 1) {
            if (comma) {
                str << ", ";
            } else {
                comma = true;
            }
            if (tensor.is_floating_point()) {
                str << tensor.index(ind).item<double>();
            } else if (tensor.is_complex()) {
                ASSERT(!"Not implemented!");
            } else {
                str << tensor.index(ind).item<int64_t>();
            }
        }
    }
    str << ",";
    if (!intend.empty()) {
        str << "\n";
    }
    str << "]";
}

std::string newlang::TensorToString(const torch::Tensor & tensor) {
    std::string result;
    std::stringstream ss;

    if (tensor.dim() == 0) {
        if (tensor.is_floating_point()) {
            ss << tensor.item<double>();
            ss >> result;
        } else if (tensor.is_complex()) {
            ASSERT(!"Not implemented!");
            //            return std::to_string(tensor.item<std::complex<double>>());
        } else {
            result = std::to_string(tensor.item<int64_t>());
        }
        return result;
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
    size_t dot, last;
    std::stringstream ss;

    TEST_INIT_();

    switch (m_var_type_current) {
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
            return TensorToString(m_value);

        case ObjType::StrChar:
            return m_str;

        case ObjType::StrWide:
            return utf8_encode(m_wstr);

        case ObjType::NativeFunc:
        case ObjType::FUNCTION:
        case ObjType::TRANSPARENT:
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
            if (!m_var_name.empty()) {
                result += ": ";
            }
            temp = m_str;
            trim(temp, "\n");
            result += temp;
            return result;

        case ObjType::Pointer:
            ss << m_func_ptr;
            result += ss.str();
            if (m_class_name.empty()) {
                result += ":Pointer";
            } else {
                result += m_class_name;
            }
            return result;

        case ObjType::Range:
            result += toString();
            return result;
    }
    LOG_RUNTIME("Data type '%s' %d incompatible to string!", newlang::toString(m_var_type_current), (int) m_var_type_current);
}

ObjPtr Obj::CreateFunc(std::string prototype, FunctionType *func_addr, ObjType type) {
    ASSERT(func_addr);
    ASSERT(type == ObjType::FUNCTION || type == ObjType::TRANSPARENT);


    TermPtr proto = Parser::ParseString(std::string(prototype + ":={}"));
    proto = proto->Left();

    ObjPtr result = Obj::CreateType(type, proto->m_text.c_str(), type);

    * const_cast<TermPtr *> (&result->m_func_proto) = proto;
    result->m_func_ptr = (void *) func_addr;

    return result;
}

ObjPtr Obj::CreateFunc(Context *ctx, TermPtr proto, ObjType type, const std::string var_name) {
    ASSERT(type == ObjType::FUNCTION || type == ObjType::TRANSPARENT);
    ObjPtr result = std::make_shared<Obj>(type, var_name.c_str(), proto);
    Obj local;
    Obj args(ctx, proto, false, &local);
    args.ClonePropTo(*result);
    *const_cast<TermPtr *> (&result->m_func_proto) = proto;
    if (!result->CheckArgs()) {
        LOG_RUNTIME("Fail create function '%s'!", proto->toString().c_str());
    }
    return result;
}

Obj::Obj(Context *ctx, const TermPtr term, bool as_value, Obj * local_vars) {

    if (!term) {
        NL_CHECK(term, "Fail term!");
    }

    m_namespace = term->m_namespace;
    m_is_reference = term->m_is_ref;
    m_var_name = term->m_name.empty() ? term->m_text : term->m_name;
    m_var_type_current = ObjType::Dictionary;
    m_func_abi = FFI_DEFAULT_ABI;
    m_dimensions = nullptr;

    *const_cast<TermPtr *> (&m_func_proto) = term;

    for (size_t i = 0; i < term->size(); i++) {
        if (term->name(i).empty()) {
            if (as_value) {
                // Не именованный аргумент
                push_back(Context::CreateRVal(ctx, (*term)[i], local_vars));
            } else {
                // Обязательный аргумент без значения по умолчанию
                push_back(pair(nullptr, term->at(i).second->getText()));
            }
        } else {
            push_back(Context::CreateRVal(ctx, (*term)[i], local_vars), term->name(i));
        }
    }
}

bool Obj::CheckArgs() const {
    bool has_error = false;
    bool named = false;
    for (size_t start = 0; start < size(); start++) {
        if (at(start).first.empty()) {
            // Аргумент не имеет имени
            LOG_ERROR("Argument %d has no name!", (int) start + 1);
            has_error = true;
        }
        if (at(start).second == nullptr) {
            if (named) {
                // Обязательный аргумент после параметров со значениями по умолчанию
                LOG_ERROR("Rrequired argument %d after parameters with default values!", (int) start + 1);
                has_error = true;
            }
        } else {
            named = true;
            for (size_t i = start + 1; i < size(); i++) {
                if (at(start).first.compare(at(i).first) == 0) {
                    LOG_ERROR("Duplicate named argument '%s'!", at(start).first.c_str());
                    has_error = true;
                }
            }
        }
    }
    return !has_error;
}

ObjPtr Obj::Call(Context *ctx, Obj * args) {
    if (is_string_type()) {
        ObjPtr result = Clone();
        result->m_str = format(result->m_str, args);
        return result;
    } else if (is_function() || m_var_type_current == ObjType::Type) {
        Obj local;
        ObjPtr param;
        if (m_func_proto) {
            param = std::make_shared<Obj>(ctx, m_func_proto, false, &local);
        } else {
            param = Obj::CreateDict();
        }
        param->ConvertToArgs_(*args, true, ctx);
        param->push_front(pair(shared(), "$0")); // Self

        if (ctx) {
            ctx->RegisterInContext(param);
        }

        ObjPtr result;
        if (m_var_type_current == ObjType::FUNCTION) {
            result = (*reinterpret_cast<FunctionType *> (m_func_ptr))(ctx, *param.get()); // Непосредственно вызов функции
        } else if (m_var_type_current == ObjType::TRANSPARENT || (m_var_type_current == ObjType::Type && m_func_ptr)) {
            result = (*reinterpret_cast<TransparentType *> (m_func_ptr))(ctx, *param.get()); // Непосредственно вызов функции
        } else if (m_var_type_current == ObjType::NativeFunc) {
            result = CallNative(ctx, *param.get());
        } else if (m_var_type_current == ObjType::EVAL_FUNCTION || m_var_type_current == ObjType::SimplePureFunc) {
            result = Context::CallBlock(ctx, m_block_source, param.get());
        } else if (m_var_type_current == ObjType::SimplePureAND) {
            result = Context::EvalBlockAND(ctx, m_block_source, param.get());
        } else if (m_var_type_current == ObjType::SimplePureOR) {
            result = Context::EvalBlockOR(ctx, m_block_source, param.get());
        } else if (m_var_type_current == ObjType::SimplePureXOR) {
            result = Context::EvalBlockXOR(ctx, m_block_source, param.get());
        } else {
            LOG_RUNTIME("Call by name not implemted '%s'!", toString().c_str());
        }

        if (ctx) {
            ctx->pop_front();
        }
        return result;


    } else if (is_dictionary_type()) {

        ObjPtr result = Clone(); // Копия текущего объекта
        result->ConvertToArgs_(*args, false, ctx); // С обновленными полями, переданными в аргументах
        result->m_class_parents.push_back(shared()); // Текущйи объект становится базовым классом для вновь создаваемого
        return result;

    } else if (args->size() > 1) {
        LOG_RUNTIME("Unsupported operation for data type %d '%s'", (int) m_var_type_current, toString().c_str());
    }
    return Clone();
}

// Обновить параметры для вызова функции или элементы у словаря при создании копии

void Obj::ConvertToArgs_(Obj &in, bool check_valid, Context * ctx) {
    bool named = false;
    bool is_ellipsis = false;
    if (check_valid && size()) {
        if (at(size() - 1).first.compare("...") == 0) {
            is_ellipsis = true;
            erase(size() - 1);
        }
    }
    for (size_t i = 0; i < in.size(); i++) {

        if (isInternalName(in.name(i))) {
            continue;
        }

        if (in.name(i).empty()) {
            //            if(check_valid && named) {
            //                LOG_RUNTIME("Position %d requires a named argument!", (int) i + 1);
            //            }
            if (i < size()) {
                if (check_valid && at(i).second && at(i).second->getType() != ObjType::None) {
                    if (!canCast(in[i]->getType(), at(i).second->getType())) {
                        LOG_RUNTIME("Fail cast value '%s' to type '%s'", newlang::toString(in[i]->getType()),
                                newlang::toString(at(i).second->getType()));
                    }
                }
                if (!at(i).second) {
                    at(i).second = Obj::CreateNone();
                }
                if (m_func_proto && i < m_func_proto->size()) {
                    at(i).second->m_is_reference = (*m_func_proto)[i]->isRef();
                    ObjType base_type = ObjType::None;
                    if (ctx) {
                        base_type = typeFromString((*m_func_proto)[i]->m_type_name, ctx);
                    } else {
                        base_type = ctx->BaseTypeFromString((*m_func_proto)[i]->m_type_name);
                    }
                    ObjType limit_type = in[i]->getTypeAsLimit();
                    if (!canCast(limit_type, base_type)) {
                        LOG_RUNTIME("Fail cast value '%s' to type '%s'",
                                in[i]->toString().c_str(), (*m_func_proto)[i]->m_type_name.c_str());
                    }
                }
                at(i).second->op_assign(in[i]);
            } else {
                if (check_valid && !is_ellipsis && m_func_proto && i >= m_func_proto->size()) {
                    LOG_RUNTIME("Positional args overflow. Ptrototype '%s'!",
                            m_func_proto ? m_func_proto->toString().c_str() : "Prototype not exists!");
                }
                push_back(in.at(i));
            }
        } else {
            named = true;
            auto find = select(in.name(i));
            if (find != end()) {
                if (check_valid && *find && (*find)->getType() != in[i]->getType() && (*find)->getType() != ObjType::None) {
                    LOG_RUNTIME("Different type arg '%s' and '%s'", (*find)->toString().c_str(),
                            in[i]->toString().c_str());
                }
                //@todo  Проверка ограничений размер данных при указаном типе
                if (!*find) {
                    *find = Obj::CreateNone();
                }
                (*find)->op_assign(in[i]);
            } else {
                for (size_t pos = 0; pos < size(); pos++) {
                    if (!at(pos).first.empty() && at(pos).first.compare(in.at(i).first) == 0) {
                        at(pos).second->op_assign(in[i]);
                        goto done;
                    }
                }
                if (check_valid && !is_ellipsis) {
                    LOG_RUNTIME("Named arg '%s' not found!", in.name(i).c_str());
                }
                push_back(in.at(i));
done:
                ;
            }
        }
    }
    if (check_valid) {

        CheckArgsValid();
    }
}

void Obj::CheckArgsValid() const {
    bool named = false;
    for (size_t i = 0; i < Variable::size(); i++) {
        //        if(!at(i).second) {
        //
        //            LOG_RUNTIME("Argument %d '%s' missed!", (int) i + 1, at(i).first.c_str());
        //        }
    }
    //    if(!CheckArgs_()) {
    //        LOG_RUNTIME("Fail arguments!");
    //    }
}

/*
 *
 *
 *
 *
 */
int Obj::op_compare(Obj & value) {
    if (this == &value) {
        return 0;
    }
    if (is_scalar() && value.is_scalar()) {
        if (is_floating() || value.is_floating()) {
            if (GetValueAsNumber() < value.GetValueAsNumber()) {
                return -1;
            } else if (GetValueAsNumber() > value.GetValueAsNumber()) {
                return 1;
            };
            return 0;
        } else if (is_complex() || value.is_complex()) {
            // Будет ошибка сравнения комплексных значений
        } else {
            if (GetValueAsInteger() < value.GetValueAsInteger()) {
                return -1;
            } else if (GetValueAsInteger() > value.GetValueAsInteger()) {
                return 1;
            };
            return 0;
        }

    } else if ((is_string_type() && value.is_string_type())) {
        switch (m_var_type_current) {
            case ObjType::StrChar:
                return m_str.compare(value.GetValueAsString());

            case ObjType::StrWide:
                return m_wstr.compare(value.GetValueAsStringWide());
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
    if (this == &value) {
        return true;
    } else if (is_tensor()) {
        // Арифметические типы данных сравниваются как тензоры
        torch::Dtype summary_type = toTorchType(
                static_cast<ObjType> (std::max(static_cast<uint8_t> (m_var_type_current), static_cast<uint8_t> (value.m_var_type_current))));
        try {
            if (m_value.dim() == 0 || value.m_value.dim() == 0) {
                if (m_value.dim() == 0 && value.m_value.dim() == 0) {

                    ObjType type = fromTorchType(summary_type);

                    if (isIntegralType(type, true)) {
                        return GetValueAsInteger() == value.GetValueAsInteger();
                    } else if (isFloatingType(type)) {
                        return GetValueAsNumber() == value.GetValueAsNumber();
                    } else {
                        LOG_RUNTIME("Fail compare type '%s'!", newlang::toString(type));
                    }
                }
                return false;
            }

            return m_value.toType(summary_type).equal(value.toTensor().toType(summary_type));
        } catch (std::exception e) {
            LOG_RUNTIME("Fail compare"); //, e.what());
        }
    } else if (is_bool_type()) {
        return GetValueAsBoolean() == value.GetValueAsBoolean();
    } else if (is_string_type()) {
        return GetValueAsString().compare(value.GetValueAsString()) == 0;
    } else if (is_dictionary_type() && value.is_dictionary_type()) {
        if (size() != value.size()) {
            return false;
        }
        for (size_t i = 0; i < size(); i++) {
            if (name(i).compare(value.name(i)) != 0) {
                return false;
            }
            if (!at(i).second->op_equal(value[i])) {

                return false;
            }
        }
        return true;
    }
    return false; // оставшиеся типы равны только если идентичны сами объекты (первое условие)
}

bool Obj::op_accurate(Obj & value) {
    if (this == &value || (is_none_type() && value.is_none_type())) {
        return true;
    } else if ((is_bool_type() && value.is_bool_type()) || (is_arithmetic_type() && value.is_arithmetic_type()) ||
            (is_string_type() && value.is_string_type()) || (is_dictionary_type() && value.is_dictionary_type())) {

        return op_equal(value);
    }
    return false;
}

ObjPtr Obj::op_bit_and_set(Obj &obj, bool strong) {
    if (m_var_type_current == ObjType::Long) {
        if (m_var_type_current == obj.m_var_type_current) {
            m_value.bitwise_and_(obj.m_value);
            //            m_values.integer &= obj.m_values.integer;
            return shared();
        }
    } else if (m_var_type_current == ObjType::None || obj.m_var_type_current == ObjType::None) {
        Variable::clear_();
        return shared();
    } else if (m_var_type_current == ObjType::Dictionary || m_var_type_current == ObjType::Class) {
        if (obj.m_var_type_current == ObjType::Dictionary || obj.m_var_type_current == ObjType::Class) {
            size_t pos = 0;
            while (pos < size()) {
                if (!obj.exist(at(pos).second, strong)) {
                    erase(pos);
                } else {
                    pos++;
                }
            }
            return shared();
        }
    } else if (is_tensor() && obj.is_tensor()) {
        size_t pos = 0;
        while (pos < size()) {
            if (!obj.exist(at(pos).second, strong)) {
                erase(pos);
            } else {

                pos++;
            }
        }
        return shared();
    }
    LOG_RUNTIME("Incompatible types %d and %d for '&' operator!", (int) m_var_type_current, (int) obj.m_var_type_current);
}

bool Obj::op_class_test(ObjPtr obj, Context * ctx) {
    if (obj->is_string_type()) {
        return op_class_test(obj->GetValueAsString().c_str(), ctx);
    } else if (!obj->m_class_name.empty()) {
        return op_class_test(obj->m_class_name.c_str(), ctx);
    } else if (obj->is_type_name()) {
        return op_class_test(newlang::toString(obj->m_var_type_fixed), ctx);
    } else {
        return op_class_test(newlang::toString(obj->m_var_type_current), ctx);
    }
}

bool Obj::op_class_test(const char *name, Context * ctx) {

    ASSERT(name || *name);

    if (!m_class_name.empty() && m_class_name.compare(name) == 0) {
        return true;
    }
    for (auto &elem : m_class_parents) {
        if (elem->op_class_test(name, ctx)) {
            return true;
        }
    }

    bool has_error = false;
    ObjType type = typeFromString(name, ctx, &has_error);
    if (has_error) {
        LOG_DEBUG("Type name %s not found!", name);
        return false;
    }

    ObjType check_type = m_var_type_current;
    if (m_var_type_current == ObjType::Type || !m_var_is_init && m_var_type_current == ObjType::None) {
        check_type = m_var_type_fixed;
    }

    return isContainsType(type, check_type);

}

bool Obj::op_duck_test_prop(Obj *base, Obj *value, bool strong) {
    if (!value) {
        return !strong; // Пустой объект равен любому при нечетком сравнении
    }
    if (!base || base->m_var_type_current == ObjType::None) {
        // Итина при пустом текущем может быть только если образец тоже пустой
        return value->m_var_type_current == ObjType::None;
    }
    ObjPtr field;
    for (size_t i = 0; i < value->size(); i++) {
        if (value->name(i).empty()) {
            field = (*base)[i];
        } else {
            field = (*base)[value->name(i)];
        }
        if (!field) {
            return false;
        }
        if (strong || !((*value)[i]->getType() != ObjType::None)) {
            for (auto &elem : *value) {
                if (!field->op_duck_test(elem.second, strong)) {

                    return false;
                }
            }
        }
    }
    return true;
}

ObjPtr Obj::op_pow_(Obj & obj) {
    if (obj.is_arithmetic_type()) {
        if (is_tensor()) {
            m_value.pow_(obj.toTensor());
            return shared();
        } else if (is_arithmetic_type()) {
            if (is_integer()) {
                SetValue_(static_cast<int64_t> (pow(GetValueAsInteger(), obj.GetValueAsInteger()) + 0.5));
                return shared();
            } else if (isFloatingType(m_var_type_current)) {
                SetValue_(pow(GetValueAsNumber(), obj.GetValueAsNumber()));
                return shared();
            }
        } else if (m_var_type_current == ObjType::StrChar && obj.is_integer()) {
            m_str = repeat(m_str, obj.GetValueAsInteger());
            return shared();
        } else if (m_var_type_current == ObjType::StrWide && obj.is_integer()) {
            m_wstr = repeat(m_wstr, obj.GetValueAsInteger());
            return shared();
        }
    }
    LOG_RUNTIME("Unsupported power operator for '%s' and '%s'!", toString().c_str(), obj.toString().c_str());
}

bool Obj::op_duck_test(Obj *value, bool strong) {

    if (!value || value->m_var_type_current == ObjType::None) {
        // Пустой объект совместим с любым объектом,
        // а при строгом сравнении только с таким же пустым
        return strong ? m_var_type_current == value->m_var_type_current : true;
    }

    if (strong) {
        if (value->is_simple()) {
            if (m_var_type_current == value->m_var_type_current || (is_string_type() && value->is_string_type())) {
                return true;
            }
            return false;
        }
        return op_duck_test_prop(this, value, strong);
    }
    if (value->m_var_type_current == ObjType::Long || value->m_var_type_current == ObjType::Double) {
        return (m_var_type_current == ObjType::Long || m_var_type_current == ObjType::Double);
    } else if (is_string_type() && value->is_string_type()) {
        return true;
    } else if (is_function() && value->is_function()) {
        return true;
    } else if (value->m_var_type_current == ObjType::Dictionary || value->m_var_type_current == ObjType::Class) {
        if (m_var_type_current == ObjType::Dictionary || m_var_type_current == ObjType::Class) {

            return op_duck_test_prop(this, value, strong);
        }
        return false;
    }
    return m_var_type_current == value->m_var_type_current;
}

std::string Obj::format(std::string format, Obj * args) {
    if (args && !args->empty()) {
        std::string name;
        std::string place;
        std::wstring wname;
        for (size_t i = 0; i < args->size(); i++) {

            if (isInternalName(args->name(i))) {
                continue;
            }

            // Заменить номер аргумента
            name = "\\$" + std::to_string(i + 1);
            place = (*args)[i]->GetValueAsString();
            format = std::regex_replace(format, std::regex(name), place);

            if (!args->name(i).empty()) {

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

ObjPtr Obj::toShape_(ObjPtr dims) {
    std::vector<int64_t> array = dims->toIntVector(true);
    if (is_tensor()) {
        m_value.resize_(array);
        return shared();
    }

    if (array.size() > 1) {
        LOG_RUNTIME("More than one dimension is not supported!");
    } else if (array.size() == 0) {
        // Ноль измерений не у тензора только у None
        toType_(ObjType::None);
        return shared();

    } else if (size() == array[0]) {
        // Требуемый размер
        return shared();
    }

    if (m_var_type_current == ObjType::StrChar) {
        m_str.resize(array[0]);
    } else if (m_var_type_current == ObjType::StrWide) {
        m_wstr.resize(array[0]);
    } else if (is_dictionary_type()) {

        if (size() > array[0]) {
            m_data.resize(array[0]); // Уменьшить размер
        } else {
            while (size() < array[0]) {
                push_back(Obj::CreateNone());
            }
        }
    } else {

        LOG_RUNTIME("Reshaping is not supported!");
    }
    return shared();
}

ObjPtr Obj::toType_(Obj * type) {
    ASSERT(type);
    if (type->m_var_type_current != ObjType::Type) {
        LOG_RUNTIME("Fail type object '%s'!", type->toString().c_str());
    }

    toType_(type->m_var_type_fixed); //, &ref
    return shared();
}

void Obj::toType_(ObjType target) {
    if (m_var_type_current == target) {
        // Конвертировать не нужно
        return;

    } else if (is_none_type() || target == ObjType::None) {
        // Любой тип при конвертации в пустой, просто очистить данные
        clear_();
        m_var_type_current = target;
        return;

    } else if (is_tensor()) {
        // Из тензора конвертировать в другой тип
        if (isTensor(target)) {
            m_value = m_value.toType(toTorchType(target));
            m_var_type_current = target;
            Variable::clear_();
            return;

        } else if (isString(target)) {
            if (target == ObjType::StrChar) {
                // В байтовую строку конвертируются только байтовый скаляр или одномерный байтовый тензор
                if (!(m_value.dtype().toScalarType() == at::ScalarType::Byte || m_value.dtype().toScalarType() == at::ScalarType::Char)) {
                    LOG_RUNTIME("Convert to byte string can 1-byte tensor only!");
                }

                if (m_value.dim() == 0) {
                    m_str.resize(1);
                    m_str[0] = m_value.item().toInt();
                } else if (m_value.dim() == 1) {
                    m_str.clear();
                    for (int i = 0; i < m_value.size(0); i++) {
                        m_str += m_value.index({i}).item().toChar();
                    }
                } else {
                    LOG_RUNTIME("Convert to string single dimension tensor only!");
                }
                m_var_type_current = target;
                return;


            } else { // ObjType::StrWide
                ASSERT(target == ObjType::StrWide);

                // В символьную строку конвертируется любой целочисленный скаляр или одномерный тензор
                if (!(m_value.dtype().toScalarType() == at::ScalarType::Byte || m_value.dtype().toScalarType() == at::ScalarType::Char ||
                        m_value.dtype().toScalarType() == at::ScalarType::Short || m_value.dtype().toScalarType() == at::ScalarType::Int)) {
                    LOG_RUNTIME("Convert to wide string can 1..4 byte tensor only!");
                }

                STATIC_ASSERT(sizeof (wchar_t) == sizeof (int32_t));

                if (m_value.dim() == 0) {
                    m_wstr.resize(1);
                    m_wstr[0] = m_value.item().toInt();
                } else if (m_value.dim() == 1) {
                    m_wstr.clear();
                    for (int i = 0; i < m_value.size(0); i++) {
                        m_wstr += m_value.index({i}).item().toInt();
                    }
                } else {
                    LOG_RUNTIME("Convert to string single dimension tensor only!");
                }

                m_var_type_current = target;
                return;
            }
        } else if (isDictionary(target)) {
            m_var_type_current = target;
            ConvertTensorToDict(m_value, *this);
            m_value.reset();
            return;
        }
    } else if (is_string_type()) {
        // Из строки в другой тип данных
        if (isString(target)) {
            // Строки хранятся в байтовом представлении и их ненужно конвертировать
            m_var_type_current = target;
            return;
        } else if (isTensor(target)) {
            // Сконвертировать строку в тензор
            torch::Tensor std_data;
            std::wstring_convert < std::codecvt_utf8<wchar_t>, wchar_t> converter;
            std::wstring temp;
            if (m_var_type_current == ObjType::StrChar) {
                // Байтовая строка конвертируются в одномерный байтовый тензор
                std_data = torch::from_blob((void *) m_str.data(),{(int) m_str.size()}, torch::Dtype::Char);
            } else { // ObjType::StrWide
                ASSERT(m_var_type_current == ObjType::StrWide);
                STATIC_ASSERT(sizeof (wchar_t) == sizeof (int32_t));
                std_data = torch::from_blob((void *) m_wstr.data(),{(int) m_wstr.size()}, torch::Dtype::Int);
            }
            if (isGenericType(target) && !isContainsType(target, fromTorchType(std_data.scalar_type()))) {
                m_value = std_data.toType(toTorchType(target));
            } else {
                m_value = std_data.clone();
            }

            m_str.clear();
            m_wstr.clear();
            m_var_type_current = target;
            return;

        } else if (isDictionary(target)) {

            torch::Tensor temp;
            ConvertValueToTensor(this, temp);

            m_var_type_current = target;
            ConvertTensorToDict(temp, *this);
            m_str.clear();
            m_wstr.clear();
            return;
        }

    } else if (is_dictionary_type()) {
        // Из словаря в другой тип данных
        if (isString(target)) {
            // Допустимо (обратная реализация чуть выше)
            LOG_RUNTIME("Not implemented!!!");
        } else if (isDictionary(target)) {
            // Словрь в словарь - ничего не делаем
            m_var_type_current = target;
            return;
        } else if (isTensor(target)) {

            ConvertDictToTensor(*this, m_value, target);

            m_var_type_current = fromTorchType(m_value.scalar_type());
            return;

        }
    } else if (m_var_type_current == ObjType::Range) {
        // Из диапазона в другой тип данных
        if (isDictionary(target) || isTensor(target)) {
            // В словарь

            ObjPtr temp = Obj::CreateDict();

            ConvertRangeToDict(this, *temp.get());

            Variable::clear_();

            if (isTensor(target)) {
                // В тензор
                ConvertDictToTensor(*temp.get(), m_value, target);
                m_var_type_current = target;

            } else {
                m_var_type_current = ObjType::Dictionary;
                temp->ClonePropTo(*this);
            }
            return;
        }
    }
    // Остальные варианты предобразований выполнить нельзя
    LOG_RUNTIME("Can`t convert type '%s'(%d) to type '%s'(%d)!",
            newlang::toString(m_var_type_current), (int) m_var_type_current, newlang::toString(target), (int) target);
}

int64_t newlang::ConcatData(Obj *dest, Obj &src, ConcatMode mode) {
    int64_t size = 0;
    ASSERT(dest);

    if (!dest->m_var_is_init) {

        ObjPtr temp = src.toType(dest->m_var_type_current);
        temp->m_var_type_fixed = dest->m_var_type_current;

        dest->clear_();
        temp->CloneDataTo(*dest);
        temp->ClonePropTo(*dest);
        dest->m_var_is_init = true;

    } else if (dest->is_string_type()) {

        if (dest->m_var_type_current == ObjType::StrChar) {
            std::string add = src.GetValueAsString();
            dest->m_str.append(add);
            size = add.size();
        } else if (dest->m_var_type_current == ObjType::StrWide) {
            std::wstring add = src.GetValueAsStringWide();
            dest->m_wstr.append(add);
            size = add.size();
        } else {
            LOG_RUNTIME("Unknown string type %s!", dest->toString().c_str());
        }

    } else if (dest->is_dictionary_type()) {

        ObjPtr temp = src.toType(ObjType::Dictionary);
        for (int i = 0; i < temp->size(); i++) {
            dest->push_back(temp->at(i).second, temp->at(i).first);
            size++;
        }

    } else if (dest->is_tensor()) {

        if (dest->m_var_type_current == src.m_var_type_current) {
            if (dest->m_value.dim() == 0) {
                dest->m_value.resize_(1);
            }
            if (src.m_value.dim() == 0) {
                src.m_value.resize_(1);
            }
            dest->m_value = torch::cat({dest->m_value, src.m_value});
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
    if (obj && (obj->is_dictionary_type() || (obj->is_tensor() && !obj->is_scalar()))) {
        if (!obj->size()) {
            LOG_RUNTIME("Cannot tensor shape from empty dictionary!");
        }
        shape.push_back(obj->size());
        if (obj->at(0).second) {
            ShapeFromDict(obj->at(0).second.get(), shape);
        }
    }
}

std::vector<int64_t> newlang::TensorShapeFromDict(const Obj * obj) {
    std::vector<int64_t> shape;
    ShapeFromDict(obj, shape);
    return shape;
}

torch::Tensor newlang::ConvertToTensor(Obj *data, at::ScalarType type, bool reshape) {
    ASSERT(data);

    if (data->is_tensor()) {
        // Прообразование один в один
        if (type == at::ScalarType::Undefined) {
            if (data->is_scalar()) {
                return data->m_value.reshape({1});
            }
            return data->m_value.clone();
        } else {
            if (data->is_scalar()) {
                return data->m_value.reshape({1}).toType(type);
            }
            return data->m_value.clone().toType(type);
        }
    } else if (data->m_var_type_current == ObjType::StrChar) {
        if (type == at::ScalarType::Undefined) {
            type = at::ScalarType::Char;
        }
        // В байтовую строку конвертируются только байтовый скаляр или одномерный байтовый тензор
        return torch::from_blob((void *) data->m_str.data(), data->m_str.size(), type).clone();
    } else if (data->m_var_type_current == ObjType::StrWide) {
        if (type == at::ScalarType::Undefined) {
            type = at::ScalarType::Int;
        }
        // В символьную строку конвертируется любой целочисленный скаляр или одномерный тензор
        return torch::from_blob((void *) data->m_wstr.data(), {
            (int) data->m_wstr.size()
        }, type).clone();
    } else if (data->is_range()) {

        ASSERT(data->at("start"));
        ASSERT(data->at("stop"));
        ASSERT(data->at("step"));
        ASSERT(data->at("start")->is_arithmetic_type() || data->at("start")->is_bool_type());
        ASSERT(data->at("stop")->is_arithmetic_type() || data->at("stop")->is_bool_type());
        ASSERT(data->at("step")->is_arithmetic_type() || data->at("step")->is_bool_type());

        ObjPtr dict = Obj::CreateDict();
        if (data->at("start")->is_floating() || data->at("stop")->is_floating() || data->at("step")->is_floating()) {
            double value = data->at("start")->GetValueAsNumber();
            double stop = data->at("stop")->GetValueAsNumber();
            double step = data->at("step")->GetValueAsNumber();
            for (; value < stop; value += step) {
                dict->push_back(Obj::CreateValue(value, ObjType::None));
            }
            type = toTorchType(ObjType::Double);
        } else {
            int64_t value = data->at("start")->GetValueAsInteger();
            int64_t stop = data->at("stop")->GetValueAsInteger();
            int64_t step = data->at("step")->GetValueAsInteger();
            for (; value < stop; value += step) {
                dict->push_back(Obj::CreateValue(value, ObjType::None));
            }
        }
        return ConvertToTensor(dict.get(), type, reshape);

    } else if (data->is_dictionary_type()) {
        if (type == at::ScalarType::Undefined) {
            type = at::ScalarType::Char;
        }
        std::vector<int64_t> shape;

        if (reshape) {
            shape = TensorShapeFromDict(data);
            ASSERT(shape.size());
        }

        // Из словаря
        torch::Tensor result = ConvertToTensor(data->index_get({0}).get(), type, false);
        if (result.dim() == 0) {
            result.resize_(1);
        }
        for (int i = 1; i < data->size(); i++) {
            torch::Tensor temp = ConvertToTensor(data->index_get({i}).get(), type, false);
            if (temp.dim() == 0) {
                temp.resize_(1);
            }
            result = torch::cat({result, temp});
        }

        if (reshape) {
            return result.reshape(shape);
        }
        return result;

    } else if (data->is_none_type() || (data->is_dictionary_type() && data->size() == 0)) {
        // Пустое значение или пустой словарь не конвертируются
        LOG_RUNTIME("Not implemented!");

    } else if (data->is_function()) {
        // Как преобразовать функцию? И нужно ли это делать?

        LOG_RUNTIME("Not implemented!");
    }
    // Остальные варианты преобразований выполнить нельзя (Error + служебные)
    LOG_RUNTIME("Can`t convert type %d to tensor!", (int) data->m_var_type_current);
}

at::TensorOptions newlang::ConvertToTensorOptions(const Obj * obj) {
    if (!obj || obj->is_none_type()) {

        return at::TensorOptions();
    }
    LOG_RUNTIME("Not implemented!");
}

at::DimnameList newlang::ConvertToDimnameList(const Obj * obj) {
    if (!obj || obj->is_none_type()) {

        return {};
    }
    LOG_RUNTIME("Not implemented!");
}

ObjPtr Obj::CallNative(Context *ctx, Obj args) {

    ffi_cif m_cif;
    std::vector<ffi_type *> m_args_type;
    std::vector<void *> m_args_ptr;

    union VALUE {
        const void *ptr;
        //        ObjPtr obj;
        size_t size;
        int64_t integer;
        double number;
        bool boolean;
    };
    std::vector<VALUE> m_args_val;
    VALUE temp;

    ASSERT(m_var_type_current == ObjType::NativeFunc);
    ASSERT(m_func_proto);

    if (!m_func_ptr) {
        NL_CHECK(m_module_name.empty() || ctx, "You cannot load a module without access to the runtime context!");
        m_func_ptr = ctx->m_runtime->GetProcAddress(m_func_mangle_name.empty() ? m_func_proto->m_text.c_str() : m_func_mangle_name.c_str(),
                m_module_name.empty() ? nullptr : m_module_name.c_str());
    }
    NL_CHECK(m_func_ptr, "Fail load func name '%s' (%s) or fail load module '%s'!", m_func_proto->m_text.c_str(),
            m_func_mangle_name.empty() ? m_func_proto->m_text.c_str() : m_func_mangle_name.c_str(),
            m_module_name.empty() ? "none" : m_module_name.c_str());

    bool is_ellipsis = (m_func_proto->size() && (*m_func_proto)[m_func_proto->size() - 1]->getTermID() == TermID::ELLIPSIS);
    size_t check_count = is_ellipsis ? m_func_proto->size() - 1 : m_func_proto->size();

    // Пропустить нулевой аргумент для нативных функций
    for (size_t i = 1; i < args.size(); i++) {

        ASSERT(args[i]);
        if (args[i]->m_is_reference) {
            LOG_RUNTIME("Argument REFERENCE! %s", args[i]->toString().c_str());
        }

        size_t pind = i - 1; // Индекс прототипа на единицу меньше из-за пустого нулевого аргумента

        ObjType type = args[i]->getTypeAsLimit();
        switch (type) {
            case ObjType::Bool:
                if (pind < check_count) {
                    NL_CHECK(!(*m_func_proto)[pind]->m_type_name.empty(), "Undefined type arg '%s'", (*m_func_proto)[pind]->toString().c_str());
                    NL_CHECK(canCast(type, typeFromString((*m_func_proto)[pind]->m_type_name, ctx)), "Fail cast from '%s' to '%s'",
                            (*m_func_proto)[pind]->m_type_name.c_str(), newlang::toString(type));
                }
                m_args_type.push_back(&ffi_type_uint8);
                temp.boolean = args[i]->GetValueAsBoolean();
                m_args_val.push_back(temp);
                break;

            case ObjType::Char:
                if (pind < check_count) {
                    NL_CHECK(!(*m_func_proto)[pind]->m_type_name.empty(), "Undefined type arg '%s'", (*m_func_proto)[pind]->toString().c_str());
                    NL_CHECK(canCast(type, typeFromString((*m_func_proto)[pind]->m_type_name, ctx)), "Fail cast from '%s' to '%s'",
                            (*m_func_proto)[pind]->m_type_name.c_str(), newlang::toString(type));
                }
                m_args_type.push_back(&ffi_type_sint8);
                temp.integer = args[i]->GetValueAsInteger();
                m_args_val.push_back(temp);
                break;

            case ObjType::Short:
                if (pind < check_count) {
                    NL_CHECK(!(*m_func_proto)[pind]->m_type_name.empty(), "Undefined type arg '%s'", (*m_func_proto)[pind]->toString().c_str());
                    NL_CHECK(canCast(type, typeFromString((*m_func_proto)[pind]->m_type_name, ctx)), "Fail cast from '%s' to '%s'",
                            (*m_func_proto)[pind]->m_type_name.c_str(), newlang::toString(type));
                }
                m_args_type.push_back(&ffi_type_sint16);
                temp.integer = args[i]->GetValueAsInteger();
                m_args_val.push_back(temp);
                break;

            case ObjType::Int:
                if (pind < check_count) {
                    NL_CHECK(!(*m_func_proto)[pind]->m_type_name.empty(), "Undefined type arg '%s'", (*m_func_proto)[pind]->toString().c_str());
                    NL_CHECK(canCast(type, typeFromString((*m_func_proto)[pind]->m_type_name, ctx)), "Fail cast from '%s' to '%s'",
                            (*m_func_proto)[pind]->m_type_name.c_str(), newlang::toString(type));
                }
                m_args_type.push_back(&ffi_type_sint32);
                temp.integer = args[i]->GetValueAsInteger();
                m_args_val.push_back(temp);
                break;

            case ObjType::Long:
                if (pind < check_count) {
                    NL_CHECK(!(*m_func_proto)[pind]->m_type_name.empty(), "Undefined type arg '%s'", (*m_func_proto)[pind]->toString().c_str());
                    NL_CHECK(canCast(type, typeFromString((*m_func_proto)[pind]->m_type_name, ctx)), "Fail cast from '%s' to '%s'",
                            (*m_func_proto)[pind]->m_type_name.c_str(), newlang::toString(type));
                }
                m_args_type.push_back(&ffi_type_sint64);
                temp.integer = args[i]->GetValueAsInteger();
                m_args_val.push_back(temp);
                break;

            case ObjType::Float:
                if (pind < check_count) {
                    NL_CHECK(!(*m_func_proto)[pind]->m_type_name.empty(), "Undefined type arg '%s'", (*m_func_proto)[pind]->toString().c_str());
                    NL_CHECK(canCast(type, typeFromString((*m_func_proto)[pind]->m_type_name, ctx)), "Fail cast from '%s' to '%s'",
                            (*m_func_proto)[pind]->m_type_name.c_str(), newlang::toString(type));
                }
                m_args_type.push_back(&ffi_type_float);
                temp.number = args[i]->GetValueAsNumber();
                m_args_val.push_back(temp);
                break;

            case ObjType::Double:
                if (pind < check_count) {
                    NL_CHECK(!(*m_func_proto)[pind]->m_type_name.empty(), "Undefined type arg '%s'", (*m_func_proto)[pind]->toString().c_str());
                    NL_CHECK(canCast(type, typeFromString((*m_func_proto)[pind]->m_type_name, ctx)), "Fail cast from '%s' to '%s'",
                            (*m_func_proto)[pind]->m_type_name.c_str(), newlang::toString(type));
                }
                m_args_type.push_back(&ffi_type_double);
                temp.number = args[i]->GetValueAsNumber();
                m_args_val.push_back(temp);
                break;

            case ObjType::StrWide:
            case ObjType::StrChar:
                ASSERT(type == ObjType::StrChar);
                if (pind < check_count) {
                    NL_CHECK(!(*m_func_proto)[pind]->m_type_name.empty(), "Undefined type arg '%s'", (*m_func_proto)[pind]->toString().c_str());
                    NL_CHECK(canCast(type, typeFromString((*m_func_proto)[pind]->m_type_name, ctx)), "Fail cast from '%s' to '%s'",
                            (*m_func_proto)[pind]->m_type_name.c_str(), newlang::toString(type));
                }
                m_args_type.push_back(&ffi_type_pointer);
                temp.ptr = args[i]->m_str.c_str();
                m_args_val.push_back(temp);
                break;

            case ObjType::Pointer:
                if (pind < check_count) {
                    NL_CHECK(!(*m_func_proto)[pind]->m_type_name.empty(), "Undefined type arg '%s'", (*m_func_proto)[pind]->toString().c_str());
                    NL_CHECK(canCast(type, typeFromString((*m_func_proto)[pind]->m_type_name, ctx)), "Fail cast from '%s' to '%s'",
                            (*m_func_proto)[pind]->m_type_name.c_str(), newlang::toString(type));
                }
                m_args_type.push_back(&ffi_type_pointer);
                temp.ptr = args[i]->m_func_ptr;
                m_args_val.push_back(temp);
                break;

            default:
                LOG_RUNTIME("Native arg '%s' not implemented!", args[i]->toString().c_str());
        }
        if (pind < check_count && (*m_func_proto)[pind]->GetType() && (*m_func_proto)[pind]->GetType()->m_text.compare("Format") == 0) {
            NL_CHECK(ParsePrintfFormat(args, i), "Fail format string or type args!");
        }
    }

    for (size_t i = 0; i < m_args_val.size(); i++) {
        m_args_ptr.push_back((void *) &m_args_val[i]);
    }

    NL_CHECK(!m_func_proto->m_type_name.empty(), "Undefined return type '%s'", m_func_proto->toString().c_str());

    VALUE res_value;
    ffi_type *result_ffi_type = nullptr;

    ObjType type = ctx->BaseTypeFromString(m_func_proto->m_type_name);

    switch (type) {
        case ObjType::Bool:
            result_ffi_type = &ffi_type_uint8;
            break;

        case ObjType::Char:
            result_ffi_type = &ffi_type_sint8;
            break;

        case ObjType::Short:
            result_ffi_type = &ffi_type_sint16;
            break;

        case ObjType::Int:
            result_ffi_type = &ffi_type_sint32;
            break;

        case ObjType::Long:
            result_ffi_type = &ffi_type_sint64;
            break;

        case ObjType::Float:
            result_ffi_type = &ffi_type_float;
            break;

        case ObjType::Double:
            result_ffi_type = &ffi_type_double;
            break;

        case ObjType::Pointer:
        case ObjType::StrChar:
        case ObjType::StrWide:
            result_ffi_type = &ffi_type_pointer;
            break;

        default:
            LOG_RUNTIME("Native return type '%s' not implemented!", m_func_proto->m_type_name.c_str());
    }

    ASSERT(m_func_abi == FFI_DEFAULT_ABI); // Нужны другие типы вызовов ???
    if (ffi_prep_cif(&m_cif, m_func_abi, m_args_type.size(), result_ffi_type, m_args_type.data()) == FFI_OK) {

        ffi_call(&m_cif, FFI_FN(m_func_ptr), &res_value, m_args_ptr.data());

        if (result_ffi_type == &ffi_type_uint8) {
            // Возвращаемый тип может быть как Byte, так и Bool
            return Obj::CreateValue(static_cast<uint8_t> (res_value.integer), typeFromString(m_func_proto->m_type_name));
        } else if (result_ffi_type == &ffi_type_sint8) {
            return Obj::CreateValue(static_cast<int8_t> (res_value.integer), ObjType::Char);
        } else if (result_ffi_type == &ffi_type_sint16) {
            return Obj::CreateValue(static_cast<int16_t> (res_value.integer), ObjType::Short);
        } else if (result_ffi_type == &ffi_type_sint32) {
            return Obj::CreateValue(static_cast<int32_t> (res_value.integer), ObjType::Int);
        } else if (result_ffi_type == &ffi_type_sint64) {
            return Obj::CreateValue(res_value.integer, ObjType::Long);
        } else if (result_ffi_type == &ffi_type_float) {
            return Obj::CreateValue(res_value.number, ObjType::Float);
        } else if (result_ffi_type == &ffi_type_double) {
            return Obj::CreateValue(res_value.number, ObjType::Double);
        } else if (result_ffi_type == &ffi_type_pointer) {
            if (type == ObjType::StrChar) {
                return Obj::CreateString(reinterpret_cast<const char *> (res_value.ptr));
            } else if (type == ObjType::StrWide) {
                return Obj::CreateString(reinterpret_cast<const wchar_t *> (res_value.ptr));
            } else if (type == ObjType::Pointer) {
                ObjPtr result = ctx->GetTypeFromString(m_func_proto->m_type_name);
                result->m_func_ptr = (void *) res_value.ptr;
                result->m_var_is_init = true;
                return result;
            } else {
                LOG_RUNTIME("Error result type '%s' or not implemented!", m_func_proto->m_type_name.c_str());
            }
        } else {
            LOG_RUNTIME("Native return type '%s' not implemented!", m_func_proto->m_type_name.c_str());
        }
    }

    LOG_RUNTIME("Fail native call '%s'!", toString().c_str());

    return Obj::CreateNone();
}

bool newlang::ParsePrintfFormat(Obj args, size_t start) {

    if (args.size() <= start || !args[start]) {
        LOG_WARNING("Missing format string!");
        return false;
    }
    if (!args[start]->is_string_type()) {
        LOG_WARNING("Argument Format '%s' not string type!", args[start]->toString().c_str());
        return false;
    }

    std::string format = args[start]->GetValueAsString();
    size_t count = parse_printf_format(format.c_str(), 0, nullptr);
    std::vector<int> types(count);

    parse_printf_format(format.c_str(), types.size(), types.data());
    bool result = true;
    unsigned i = 0;
    unsigned aind = start + 1;
    ObjType cast;
    while (i < types.size()) {

        if (aind < args.size()) {
            //            if(types[i] & PA_FLAG_PTR == PA_FLAG_PTR) {
            //                LOG_WARNING("Pointer arg '%u' not suppotred!", i);
            //                result = false;
            //                i++;
            //                aind++;
            //                continue;
            //            }
            switch (types[i] & ~PA_FLAG_MASK) {
                case PA_INT:
                    if (types[i] & PA_FLAG_MASK == 0) {
                        cast = ObjType::Int;
                    } else if (types[i] & PA_FLAG_LONG == PA_FLAG_LONG) {
                        cast = ObjType::Long;
                    } else if (types[i] & PA_FLAG_SHORT == PA_FLAG_SHORT) {
                        cast = ObjType::Short;
                    }
                    if (!canCast(args[aind]->m_var_type_current, cast)) {
                        LOG_WARNING("Cast '%s' to '%s' not supported!", newlang::toString(args[aind]->m_var_type_current),
                                newlang::toString(cast));
                        result = false;
                    }
                    break;
                case PA_CHAR:
                    if (types[i] & PA_FLAG_MASK) {
                        LOG_WARNING("format modifier arg '%s' %u not supported!", newlang::toString(args[aind]->m_var_type_current), i);
                        result = false;
                    }
                    cast = ObjType::Char;
                    if (!canCast(args[aind]->m_var_type_current, cast)) {
                        LOG_WARNING("Cast '%s' to '%s' not supported!", newlang::toString(args[aind]->m_var_type_current),
                                newlang::toString(cast));
                        result = false;
                    }
                    break;
                case PA_STRING:
                    if (types[i] & PA_FLAG_MASK) {
                        LOG_WARNING("format modifier arg '%s' %u not supported!", newlang::toString(args[aind]->m_var_type_current), i);
                        result = false;
                    }
                    cast = ObjType::StrChar;
                    if (!canCast(args[aind]->m_var_type_current, cast)) {
                        LOG_WARNING("Cast '%s' to '%s' not supported!", newlang::toString(args[aind]->m_var_type_current),
                                newlang::toString(cast));
                        result = false;
                    }
                    break;
                case PA_FLOAT:
                case PA_DOUBLE:
                    if (types[i] & PA_FLAG_MASK) {
                        LOG_WARNING("format modifier arg '%s' %u not supported!", newlang::toString(args[aind]->m_var_type_current), i);
                        result = false;
                    }
                    cast = ObjType::Double;
                    if (!canCast(args[aind]->m_var_type_current, cast)) {
                        LOG_WARNING("Cast '%s' to '%s' not supported!", newlang::toString(args[aind]->m_var_type_current),
                                newlang::toString(cast));
                        result = false;
                    }
                    break;
                default:
                    LOG_WARNING("Arg '%u' not supported!", i);
                    // PA_WCHAR,     /* wide char */
                    // PA_WSTRING,   /* const wchar_t *, wide character string */
                    // PA_POINTER,   /* void * */
                    result = false;
            }
        } else {
            LOG_WARNING("Missing argument %u", i);
            return false;
        }
        i++;
        aind++;
    }
    if (aind != args.size()) {
        LOG_WARNING("Extra arguments more %u", i);
        return false;
    }
    return result;
}

void newlang::ConvertRangeToDict(Obj *from, Obj & to) {

    to.m_var_is_init = false;

    ASSERT(from);
    ASSERT(from->is_range());
    ASSERT(from->at("start"));
    ASSERT(from->at("stop"));
    ASSERT(from->at("step"));

    ASSERT(to.m_var_type_current == ObjType::Dictionary || to.m_var_type_current == ObjType::None);

    if (!to.is_dictionary_type()) {
        to.m_var_type_current = ObjType::Dictionary;
    }

    ObjPtr value = (*from)["start"]->Clone();
    if ((*value) < (*from)["stop"]) {
        ASSERT((*from)["step"]->GetValueAsNumber() > 0);
        while ((*value) < (*from)["stop"]) {
            to.push_back(value->Clone());
            (*value) += (*from)["step"];
        }
    } else {
        ASSERT((*from)["step"]->GetValueAsNumber() < 0);
        while ((*value) > (*from)["stop"]) {
            to.push_back(value->Clone());
            (*value) += (*from)["step"];
        }
    }

    to.m_var_is_init = true;
}

void newlang::ConvertStringToTensor(const std::string &from, torch::Tensor &to, ObjType type) {
    ASSERT(!from.empty());
    ASSERT(type == ObjType::None || type == ObjType::Char || type == ObjType::Tensor);
    to = torch::from_blob((void *) from.data(),{(int64_t) from.size()}, at::ScalarType::Char).clone();
}

void newlang::ConvertStringToTensor(const std::wstring &from, torch::Tensor &to, ObjType type) {
    ASSERT(!from.empty());
    ASSERT(type == ObjType::None || type == ObjType::Int || type == ObjType::Tensor);
    STATIC_ASSERT(sizeof (wchar_t) == sizeof (int));
    to = torch::from_blob((void *) from.data(),{(int64_t) from.size()}, at::ScalarType::Int).clone();
}

template <typename T> void ConvertTensorToStringTemplate(const torch::Tensor &from, T &to, std::vector<Index> *index) {

    if (from.dim() == 0) {

        ASSERT(index == nullptr);
        to = from.toType(at::ScalarType::Char).item<int>();
        return;
    }

    std::vector<Index> dims({0});
    if (index == nullptr) {
        to.clear();
        index = &dims;
    }

    int64_t pos = index->size();
    if (pos == from.dim()) {
        for (int i = 0; i < from.size(pos - 1); i++) {
            (*index)[pos - 1] = i;
            to += from.index(*index).toType(at::ScalarType::Char).item<int>();
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
    if (!to.is_dictionary_type()) {
        to.m_var_type_current = ObjType::Dictionary;
    }

    if (from.dim() == 0) {
        ASSERT(index == nullptr);
        to.push_back(Obj::CreateTensor(from));
        return;
    }

    std::vector<Index> dims({0});
    if (index == nullptr) {
        index = &dims;
    }

    int64_t pos = index->size();
    if (pos == from.dim()) {
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

void newlang::ConvertDictToTensor(Obj &from, torch::Tensor &to, ObjType type) {

    torch::Tensor temp;
    for (size_t i = 0; i < from.size(); i++) {
        ConvertValueToTensor(from.at(i).second.get(), temp, type);

        if (temp.dim() == 0) {
            temp = temp.reshape({1});
        }

        if (to.dim() != 1 || to.size(0) == 0) {
            to = temp.clone();
        } else {
            to = torch::cat({to, temp});
        }
    }
    //    if (dims) {
    //        to = to.reshape(*dims);
    //    }
}

void newlang::ConvertValueToTensor(Obj *from, torch::Tensor &to, ObjType type) {
    ASSERT(from);
    if (from->is_tensor()) {
        to = from->m_value.clone();
        if (type == ObjType::None || type == ObjType::Tensor) {
            return;
        }
        to = to.toType(toTorchType(type));
        //        if (dims) {
        //            to = to.reshape(*dims);
        //        }
    } else if (from->is_range()) {
        ObjPtr temp = Obj::CreateNone();
        ConvertRangeToDict(from, *temp.get());
        ConvertDictToTensor(*temp.get(), to, type);
    } else if (from->getType() == ObjType::StrChar) {
        ConvertStringToTensor(from->m_str, to, type);
    } else if (from->getType() == ObjType::StrWide) {
        ConvertStringToTensor(from->m_wstr, to, type);
    } else if (from->is_dictionary_type()) {
        ConvertDictToTensor(*from, to, type);
    } else {
        LOG_RUNTIME("Fail convert object type %s to tensor (%s)!", newlang::toString(from->getType()), from->toString().c_str());
    }
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
    * const_cast<TermPtr *> (&result->m_func_proto) = proto->Left();

    result->m_func_ptr = (void *) BaseTypeConstructor;
    result->m_is_const = true;
    return result;
}

ObjPtr Obj::BaseTypeConstructor(const Context *ctx, Obj & args) {

    if (args.empty() || !args[0]) {
        LOG_RUNTIME("Self simple type not defined!");
    }
    ASSERT(args[0]->getType() == ObjType::Type);

    ObjPtr result = nullptr;
    if (isArithmeticType(args[0]->m_var_type_fixed)) {
        result = ConstructorSimpleType_(ctx, args);
    } else if (args[0]->m_var_type_fixed == ObjType::Dictionary) {
        result = ConstructorDictionary_(ctx, args);
    } else if (args[0]->m_var_type_fixed == ObjType::Class) {
        result = ConstructorClass_(ctx, args);
    } else if (args[0]->m_var_type_fixed == ObjType::Struct || args[0]->m_var_type_fixed == ObjType::Union) {
        result = ConstructorStruct_(ctx, args);
    } else if (args[0]->m_var_type_fixed == ObjType::Enum) {
        result = ConstructorEnum_(ctx, args);
    } else if (args[0]->m_var_type_fixed == ObjType::Return) {
        result = ConstructorReturn_(ctx, args);
    } else if (args[0]->m_var_type_fixed == ObjType::Break || args[0]->m_var_type_fixed == ObjType::Continue) {
        result = ConstructorInterraption_(ctx, args, args[0]->m_var_type_fixed);
    } else if (args[0]->m_var_type_fixed == ObjType::Error || args[0]->m_var_type_fixed == ObjType::ErrorParser
            || args[0]->m_var_type_fixed == ObjType::ErrorRunTime || args[0]->m_var_type_fixed == ObjType::ErrorSignal) {
        result = ConstructorError_(ctx, args);
    } else {

        result = args[0]->Clone();
        if (result) {
            result->m_is_const = false;
        }
    }

    if (!result) {
        LOG_RUNTIME("Create type '%s' error or not implemented!", newlang::toString(args[0]->m_var_type_fixed));
    }

    result->m_class_name = args[0]->m_class_name;

    return result;
}

ObjPtr Obj::ConstructorSimpleType_(const Context *ctx, Obj & args) {

    ASSERT(!args.empty() && args[0]);
    ASSERT(args[0]->getType() == ObjType::Type);

    // Переданы значения для приведения типов
    // Но само значение пока не установлено
    ObjPtr result = args[0]->Clone();
    if (args.size() == 1) {
        // Копия существующего типа с возможностью редактирования
        result->m_is_const = false;
        return result;
    }

    std::vector<int64_t> dims;

    if (result->m_dimensions) {
        // Размерность указана
        for (size_t i = 0; i < result->m_dimensions->size(); i++) {
            Index ind = (*result->m_dimensions)[i]->toIndex();
            if (ind.is_integer()) {
                dims.push_back(ind.integer());
            } else if (ind.is_boolean()) {
                dims.push_back(ind.boolean());
            } else {
                LOG_RUNTIME("Non fixed dimension not implemented!");
            }
        }
    }


    if (args.size() == 2) {
        // Передано единственное значение (нулевой аргумент - сам объект, т.е. :Тип(Значение) )

        ObjPtr convert;

        // Если обобщенный тип данных, а сами данные принадлежат обощенному типу
        if (isGenericType(result->m_var_type_fixed) && isContainsType(result->m_var_type_fixed, args[1]->getType())) {
            convert = args[1]->Clone();
        } else {
            convert = args[1]->toType(result->m_var_type_fixed);
        }
        convert->m_var_type_fixed = result->m_var_type_fixed;
        convert.swap(result);

    } else {

        // Для списка значений сперва формируется словарь, а после он конвертируется в нужный тип данных

        result->m_var_type_current = ObjType::Dictionary;

        ObjPtr prev = nullptr;
        for (int i = 1; i < args.size(); i++) {

            if (args[i]->getType() == ObjType::Ellipsis) {
                if (!prev) {
                    LOG_RUNTIME("There is no previous item to repeat!");
                }
                if (i + 1 != args.size()) {
                    LOG_RUNTIME("Ellipsis is not the last element!");
                }
                if (dims.empty()) {
                    LOG_RUNTIME("Object has no dimensions!");
                }
                int64_t full_size = 1;
                for (int i = 0; i < dims.size(); i++) {
                    full_size *= dims[i];
                }
                if (full_size <= 0) {
                    LOG_RUNTIME("Items count error for all dimensions!");
                }

                for (int64_t i = result->size(); i < full_size; i++) {
                    result->op_concat_(prev, ConcatMode::Append);
                }

                break;

            } else {
                prev = args[i];
            }

            result->op_concat_(prev, ConcatMode::Append);
        }

        if (args[0]->m_var_type_fixed != ObjType::Dictionary) {
            result = result->toType(args[0]->m_var_type_fixed);
            result->m_var_type_fixed = result->m_var_type_current;
        }
    }


    if (!dims.empty()) {

        if (isString(result->getType()) || isDictionary(result->getType())) {
            if (dims.size() != 1) {
                LOG_RUNTIME("Fail size for type '%s'!", newlang::toString(result->getType()));
            }
            result->resize_(dims[0], nullptr);
        } else if (isTensor(result->getType())) {
            if (dims.size() == 1 && dims[0] == 0) {
                // Скаляр
                if (args.size() == 2 && args[0]->m_var_type_fixed == ObjType::Bool) {

                    result->m_value = torch::scalar_tensor(result->empty() ? 0 : 1, at::ScalarType::Bool);
                    return result;

                } else if (result->size() != 0) {
                    LOG_RUNTIME("Only one value is required for a scalar!");
                }
                dims.clear();
            }
            result->m_value = result->m_value.reshape(dims);
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

    ASSERT(!args.empty() && args[0]);
    ASSERT(args[0]->getType() == ObjType::Type);

    ObjPtr result = Obj::CreateDict();
    for (int i = 1; i < args.size(); i++) {
        result->push_back(args[i], args.name(i));
    }
    result->m_var_is_init = true;
    return result;
}

ObjPtr Obj::ConstructorClass_(const Context *ctx, Obj & args) {
    ObjPtr result = ConstructorDictionary_(ctx, args);
    result->m_var_type_fixed = ObjType::Class;
    for (int i = 0; i < result->size(); i++) {
        if (result->name(i).empty()) {
            LOG_RUNTIME("Field pos %d has no name!", i);
        }
        if (!result->select(result->name(i)).complete()) {
            LOG_RUNTIME("Field name '%s' at index %d already exists!", result->name(i).c_str(), i);
        }
    }
    return result;
}

ObjPtr Obj::ConstructorStruct_(const Context *ctx, Obj & args) {
    ObjPtr result = ConstructorClass_(ctx, args);
    result->m_var_type_fixed = ObjType::Struct;

    if (!result->size()) {
        LOG_RUNTIME("Empty Struct not allowed!");
    }

    for (int i = 0; i < result->size(); i++) {
        if (!(*result)[i]) {
            LOG_RUNTIME("Field '%s' at pos %d not defined!", result->name(i).c_str(), i);
        }
        if (!(*result)[i] || !isSimpleType((*result)[i]->getType()) || isGenericType((*result)[i]->getType())) {
            LOG_RUNTIME("Field '%s' at pos %d not simple type! (%s)", result->name(i).c_str(), i, newlang::toString((*result)[i]->getType()));
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
        if (args.name(i).empty()) {
            if (args[i] && args[i]->is_string_type()) {
                enum_name = args[i]->GetValueAsString();
            } else {
                LOG_RUNTIME("Field pos %d has no name!", i);
            }
        } else {
            enum_name = args.name(i);

            if (args[i] && (args[i]->is_integer() || args[i]->is_bool_type())) {
                val_int = args[i]->GetValueAsInteger();
            } else if (!args[i] || !args[i]->is_none_type()) {
                LOG_RUNTIME("Field value '%s' %d must integer type!", args.name(i).c_str(), i);
            }
        }

        if (!result->select(enum_name).complete()) {
            LOG_RUNTIME("Field value '%s' at index %d already exists!", enum_name.c_str(), i);
        }


        enum_value = Obj::CreateValue(val_int, ObjType::None); // , type
        enum_value->m_var_type_fixed = enum_value->m_var_type_current;
        enum_value->m_is_const = true;
        result->push_back(enum_value, enum_name);
        val_int += 1;
    }

    result->m_is_const = true;

    return result;
}

ObjPtr Obj::ConstructorError_(const Context *ctx, Obj & args) {
    ObjPtr result = ConstructorDictionary_(ctx, args);
    result->m_var_type_current = ObjType::Error;
    result->m_var_type_fixed = ObjType::Error;
    if (!result->size()) {
        LOG_RUNTIME("Argument for type ':Error' required!");
    }
    return result;
}

ObjPtr Obj::ConstructorReturn_(const Context *ctx, Obj & args) {
    ObjPtr result = ConstructorDictionary_(ctx, args);
    result->m_var_type_current = ObjType::Return;
    result->m_var_type_fixed = ObjType::Return;
    if (result->size() == 0) {
        result->push_back(Obj::Arg(Obj::CreateNone()));
    }
    if (result->size() != 1) {
        LOG_RUNTIME("Multiple argument for type ':Return'!");
    }
    return result;
}

ObjPtr Obj::ConstructorInterraption_(const Context* ctx, Obj& args, ObjType type) {
    ObjPtr result = ConstructorDictionary_(ctx, args);
    result->m_var_type_current = type;
    result->m_var_type_fixed = type;
    if (result->size()) {
        LOG_RUNTIME("Argument for type %s not allowed!", newlang::toString(type));
    }
    return result;
}

