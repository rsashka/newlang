//#include "pch.h"

#include "warning_push.h"
#include <torch/torch.h>
#include <ATen/ATen.h>

//#include <llvm-c/Types.h>
//#include <llvm-c/ExecutionEngine.h>
#include "warning_pop.h"

#include <codecvt>

#include "object.h"
#include "context.h"
#include "runtime.h"

using namespace newlang;


std::string TensorToString(const torch::Tensor &tensor);
ObjType fromTorchType(at::ScalarType t);

/* Для конвертирования словаря в тензор для вывода общего типа данных для всех элементов */
ObjType getSummaryTensorType(Obj *obj, ObjType start = ObjType::None);
void ConvertStringToTensor(const std::string &from, torch::Tensor &to, ObjType type = ObjType::None);
void ConvertStringToTensor(const std::wstring &from, torch::Tensor &to, ObjType type = ObjType::None);
void ConvertTensorToString(const torch::Tensor &from, std::string &to, std::vector<Index> *index = nullptr);
void ConvertTensorToString(const torch::Tensor &from, std::wstring &to, std::vector<Index> *index = nullptr);
void ConvertTensorToDict(const torch::Tensor &from, Obj &to, std::vector<Index> *index = nullptr);

ObjType GetTensorType(torch::Tensor & val);

at::indexing::Slice toSlice(Obj &obj);

at::indexing::Slice toSlice(Obj &obj) {
    NL_CHECK(obj.is_range(), "Convert to slice supported for range only!");

    ASSERT(obj.size() == 3
            && obj.Variable<Obj>::at("start").second
            && obj.Variable<Obj>::at("stop").second
            && obj.Variable<Obj>::at("step").second);

    NL_CHECK(obj.Variable<Obj>::at("start").second->is_integer(), "Slice value start support integer type only!");
    NL_CHECK(obj.Variable<Obj>::at("stop").second->is_integer(), "Slice value stop support integer type only!");
    NL_CHECK(obj.Variable<Obj>::at("step").second->is_integer(), "Slice value step support integer type only!");

    return at::indexing::Slice(
            obj.Variable<Obj>::at("start").second->GetValueAsInteger(),
            obj.Variable<Obj>::at("stop").second->GetValueAsInteger(),
            obj.Variable<Obj>::at("step").second->GetValueAsInteger());
}


Index toIndex(Obj &obj);

ObjPtr CreateTensor(torch::Tensor tensor) {
    ObjType check_type = GetTensorType(tensor);
    if (!isTensor(check_type)) {
        LOG_RUNTIME("Unsupport torch type %s (%d)!", at::toString(tensor.dtype().toScalarType()), (int) tensor.dtype().toScalarType());
    }
    ObjPtr result;
    if (tensor.dim() == 0) {
        if (tensor.is_floating_point()) {
            result = Obj::CreateValue(tensor.item<double>(), check_type);
        } else {
            ASSERT(!tensor.is_complex());
            result = Obj::CreateValue(tensor.item<int64_t>(), check_type);
        }
    } else {

        result = Obj::CreateType(check_type);
        ASSERT(result->m_tensor);
        *result->m_tensor = tensor;
        result->m_var_is_init = true;
    }
    return result;
}

torch::ScalarType toTorchType(ObjType t) {
    switch (t) {
        case ObjType::Bool:
            return at::ScalarType::Bool;
        case ObjType::Int8:
        case ObjType::Byte:
        case ObjType::Char:
            return at::ScalarType::Char;
        case ObjType::Int16:
        case ObjType::Word:
            return at::ScalarType::Short;
        case ObjType::Int32:
        case ObjType::DWord:
            return at::ScalarType::Int;
        case ObjType::Int64:
        case ObjType::DWord64:
        case ObjType::Integer:
            return at::ScalarType::Long;
        case ObjType::Float32:
        case ObjType::Single:
        case ObjType::Tensor:
            return at::ScalarType::Float;
        case ObjType::Float64:
        case ObjType::Double:
        case ObjType::Number:
            return at::ScalarType::Double;
        case ObjType::Complex16:
            return at::ScalarType::ComplexHalf;
        case ObjType::Complex32:
            return at::ScalarType::ComplexFloat;
        case ObjType::Complex64:
        case ObjType::Complex:
            return at::ScalarType::ComplexDouble;
    }
    LOG_RUNTIME("Can`t convert type '%s' to torch scalar type!", toString(t));
}

ObjType fromTorchType(torch::Dtype t) {
    switch (t) {
        case at::ScalarType::Bool:
            return ObjType::Bool;
        case at::ScalarType::Byte:
        case at::ScalarType::Char:
        case at::ScalarType::QInt8:
        case at::ScalarType::QUInt8:
            return ObjType::Int8;
        case at::ScalarType::Short:
            return ObjType::Int16;
        case at::ScalarType::Int:
        case at::ScalarType::QInt32:
            return ObjType::Int32;
        case at::ScalarType::Long:
            return ObjType::Int64;
        case at::ScalarType::BFloat16:
        case at::ScalarType::Half:
            return ObjType::Float16;
        case at::ScalarType::Float:
            return ObjType::Float32;
        case at::ScalarType::Double:
            return ObjType::Float64;
        case at::ScalarType::ComplexHalf:
            return ObjType::Complex16;
        case at::ScalarType::ComplexFloat:
            return ObjType::Complex32;
        case at::ScalarType::ComplexDouble:
            return ObjType::Complex64;
    }
    LOG_RUNTIME("Can`t convert type '%s' to ObjType!", at::toString(t));
}

std::string DimToString(const Dimension dim) {
    std::stringstream ss;
    ss << dim;

    return ss.str();
}

std::string IndexToString(const std::vector<Index> &index) {
    std::stringstream ss;

    ss << "[";
    for (int i = 0; i < index.size(); i++) {
        ss << index[i];
    }
    ss << "]";

    return ss.str();
}

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

std::string newlang::utf8_encode(const std::wstring wstr) {
    std::string utf8line;

    if (wstr.empty()) {
        return utf8line;
    }
    utf8line = std::wstring_convert < std::codecvt_utf8<wchar_t>>().to_bytes(wstr.c_str());
    return utf8line;
}

// Convert an UTF8 string to a wide Unicode String

std::wstring newlang::utf8_decode(const std::string str) {
    std::wstring wide_line;

    if (str.empty()) {
        return wide_line;
    }
    wide_line = std::wstring_convert < std::codecvt_utf8<wchar_t>>().from_bytes(str.c_str());
    return wide_line;
}

Obj::Obj(ObjType type, const char *var_name, TermPtr func_proto, ObjType fixed, bool init) :
m_var_type_current(type), m_var_name(var_name ? var_name : ""), m_prototype(func_proto) {
    m_is_const = false;
    m_check_args = false;
    m_dimensions = nullptr;
    m_is_reference = false;
    m_var_type_fixed = fixed;
    m_var_is_init = init;
    m_is_const = false;
    m_var = std::monostate();
    m_tensor = std::make_shared<torch::Tensor>();
}

bool Obj::empty() const {
    if (is_none_type()) {
        return true;
    } else if (m_var_type_current == ObjType::StrChar) {
        return !m_var_is_init || m_value.empty();
    } else if (m_var_type_current == ObjType::StrWide) {
        return !m_var_is_init || m_string.empty();
    } else if (is_tensor_type()) {
        return !m_var_is_init || at::_is_zerotensor(*m_tensor);
    }
    return Variable<Obj>::empty();
}

void Obj::erase(const size_t from, const size_t to) {
    if (!is_indexing()) {
        LOG_RUNTIME("Operator erase(from, to) for object type %s not implemented!", newlang::toString(m_var_type_current));
    }
    if (is_tensor_type()) {
        // For expand operator (val, tensor := ... tensor)
        int64_t new_size = m_tensor->size(0) - 1;
        if ((from == 0 && to == 1) || (from == 0 && to == 0 && new_size == 0)) {
            if (new_size > 0) {
                std::vector<int64_t> sizes(1);
                sizes[0] = new_size + 1;

                at::Tensor ind = torch::arange(sizes[0] - new_size - 1, sizes[0] - 1, at::ScalarType::Long);
                at::Tensor any = torch::zeros(sizes[0] - new_size, at::ScalarType::Long);
                //                LOG_DEBUG("arange %s    %s", TensorToString(ind).c_str(), TensorToString(any).c_str());

                ind = at::cat({any, ind});
                //                LOG_DEBUG("cat %s", TensorToString(ind).c_str());

                //                LOG_DEBUG("m_value %s", TensorToString(m_value).c_str());
                m_tensor->index_copy_(0, ind, m_tensor->clone());
                //                LOG_DEBUG("index_copy_ %s", TensorToString(m_value).c_str());

                sizes[0] = new_size;
                m_tensor->resize_(at::IntArrayRef(sizes));

            } else {
                m_tensor->reset();
                m_var_type_current = ObjType::None;
            }
        } else {
            LOG_RUNTIME("Operator erase(%ld, %ld) for object type %s not implemented!", from, to, newlang::toString(m_var_type_current));
        }
    } else {
        Variable<Obj>::erase(from, to);
    }
}

int64_t Obj::size(int64_t dim) const {
    if (is_tensor_type()) {
        if (is_scalar()) {
            if (dim != 0) {
                LOG_RUNTIME("Scalar has zero dimension!");
            }
            return 0;
        }
        return m_tensor->size(dim);
    }
    ASSERT(dim == 0);
    if (m_var_type_current == ObjType::StrChar) {
        return m_value.size();
    } else if (m_var_type_current == ObjType::StrWide) {
        return m_string.size();
    }
    return Variable::size();
}

int64_t Obj::resize_(int64_t new_size, ObjPtr fill, const std::string name) {

    if (is_string_type()) {

        if (new_size >= 0) {
            // Размер положительный, просто изменить число элементов добавив или удалив последние
            if (m_var_type_current == ObjType::StrChar) {
                m_value.resize(new_size, ' ');
                return m_value.size();
            } else if (m_var_type_current == ObjType::StrWide) {
                m_string.resize(new_size, L' ');
                return m_string.size();
            }
        } else {
            // Если размер отрицательный - добавить или удалить вначале
            new_size = -new_size;
            if (static_cast<int64_t> (size()) > new_size) {
                if (m_var_type_current == ObjType::StrChar) {
                    m_value.erase(0, new_size);
                    return m_value.size();

                } else if (m_var_type_current == ObjType::StrWide) {
                    m_value.erase(0, new_size);
                    return m_string.size();
                }
            } else if (static_cast<int64_t> (size()) < new_size) {
                if (m_var_type_current == ObjType::StrChar) {
                    m_value.insert(0, new_size, ' ');
                    return m_value.size();

                } else if (m_var_type_current == ObjType::StrWide) {
                    m_string.insert(0, new_size, L' ');
                    return m_string.size();

                }
            }
        }

    } else if (is_dictionary_type()) {
        return Variable::resize(new_size, fill ? fill : Obj::CreateNone(), name);
    } else if (is_tensor_type()) {
        std::vector<int64_t> sizes;
        for (int i = 0; i < m_tensor->dim(); i++) {
            sizes.push_back(m_tensor->size(i));
        }

        if (sizes.empty()) { // Scalar

            LOG_RUNTIME("Method resize for SCALAR type '%s' not implemented!", newlang::toString(m_var_type_current));

        } else if (new_size == 0 || sizes[0] == new_size) {
            // Tensor size OK - do nothing            
        } else if (new_size > 0) { // Increase tensor size

            // The size is positive, just change the number of elements by adding or removing the last
            ASSERT(sizes.size() == 1);

            sizes[0] = new_size;
            m_tensor->resize_(at::IntArrayRef(sizes));

        } else { // Decrease tensor size
            // If the size is negative - add or remove elements first
            new_size = -new_size;
            if (sizes[0] == new_size) {
                // Tensor size OK - do nothing            
            } else if (sizes[0] > new_size) {

                ASSERT(sizes.size() == 1);

                at::Tensor ind = torch::arange(sizes[0] - new_size - 1, sizes[0] - 1, at::ScalarType::Long);
                at::Tensor any = torch::zeros(sizes[0] - new_size, at::ScalarType::Long);
                //                LOG_DEBUG("arange %s    %s", TensorToString(ind).c_str(), TensorToString(any).c_str());

                ind = at::cat({any, ind});
                //                LOG_DEBUG("cat %s", TensorToString(ind).c_str());

                //                LOG_DEBUG("m_value %s", TensorToString(m_value).c_str());
                m_tensor->index_copy_(0, ind, m_tensor->clone());
                //                LOG_DEBUG("index_copy_ %s", TensorToString(m_value).c_str());

                sizes[0] = new_size;
                m_tensor->resize_(at::IntArrayRef(sizes));
                //                LOG_DEBUG("resize_ %s", TensorToString(m_value).c_str());

            } else { // sizes[0] < size
                ASSERT(sizes.size() == 1);

                *m_tensor = at::cat({torch::zeros(new_size - sizes[0], m_tensor->scalar_type()), *m_tensor});
            }
        }

        if (new_size == 0) {
            m_tensor->reset();
            m_var_type_current = ObjType::None;
        }
        return new_size;
    }
    LOG_RUNTIME("Method resize for type '%s' not implemented!", newlang::toString(m_var_type_current));
}

const Variable<Obj>::PairType & Obj::at(int64_t index) const {
    if (m_var_type_current == ObjType::StrChar) {
        if (index < static_cast<int64_t> (m_value.size())) {
            m_str_pair = pair(CreateString(std::string(1, m_value[index])));
            return m_str_pair;
        }
        LOG_RUNTIME("Index '%ld' not exists in byte string '%s'!", index, m_value.c_str());
    } else if (m_var_type_current == ObjType::StrWide) {
        if (index < static_cast<int64_t> (m_string.size())) {
            m_str_pair = pair(CreateString(std::wstring(1, m_string[index])));
            return m_str_pair;
        }
        LOG_RUNTIME("Index '%ld' not exists in byte string '%s'!", index, "WIDE");

    } else if (is_tensor_type()) {
        ASSERT(!is_scalar());
        torch::Tensor t = m_tensor->index({index});
        m_str_pair = pair(CreateTensor(t));
        return m_str_pair;
    }
    return Variable::at(index);
}

Variable<Obj>::PairType & Obj::at(int64_t index) {
    if (m_var_type_current == ObjType::StrChar) {
        if (index < static_cast<int64_t> (m_value.size())) {
            m_str_pair = pair(CreateString(std::string(1, m_value[index])));
            return m_str_pair;
        }
        LOG_RUNTIME("Index '%ld' not exists in byte string '%s'!", index, m_value.c_str());
    } else if (m_var_type_current == ObjType::StrWide) {
        if (index < static_cast<int64_t> (m_string.size())) {
            m_str_pair = pair(CreateString(std::wstring(1, m_string[index])));
            return m_str_pair;
        }
        LOG_RUNTIME("Index '%ld' not exists in byte string '%s'!", index, "WIDE");

    } else if (is_tensor_type()) {
        ASSERT(!is_scalar());
        ASSERT(m_tensor->defined());
        torch::Tensor t = m_tensor->index({(int) index});
        m_str_pair = pair(CreateTensor(t));
        return m_str_pair;
        //        }
    }
    return Variable::at(index);
}

const ObjPtr Obj::index_get(const std::vector<Index> &index) const {
    if (m_var_type_current == ObjType::StrChar || m_var_type_current == ObjType::FmtChar) {
        if (index.size() != 1 || !index[0].is_integer()) {
            LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
        }
        int64_t pos = index[0].integer().expect_int();
        if (pos < 0) {
            pos = m_value.size() + pos; // Позиция с конца строки
        }
        if (pos < static_cast<int64_t> (m_value.size())) {
            return CreateString(std::string(1, m_value[pos]));
        }
        LOG_RUNTIME("Index '%s' not exists in byte string '%s'!", IndexToString(index).c_str(), m_value.c_str());
    } else if (m_var_type_current == ObjType::StrWide || m_var_type_current == ObjType::FmtWide) {
        if (index.size() != 1 || !index[0].is_integer()) {
            LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
        }
        int64_t pos = index[0].integer().expect_int();
        if (pos < 0) {
            pos = m_string.size() + pos; // Позиция с конца строки
        }
        if (pos < static_cast<int64_t> (m_string.size())) {
            return CreateString(std::wstring(1, m_string[pos]));
        }
        LOG_RUNTIME("Index '%s' not exists in WIDE string '%s'!", IndexToString(index).c_str(), utf8_encode(m_string).c_str());

    } else if (is_tensor_type()) {
        ASSERT(!is_scalar());
        ASSERT(m_tensor->defined());
        torch::Tensor t = m_tensor->index(index);
        return CreateTensor(t);
    }

    if (index.size() != 1 || !index[0].is_integer()) {
        LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
    }
    return Variable::at(index[0].integer().expect_int()).second;
}

ObjPtr Obj::index_set_(const std::vector<Index> &index, const ObjPtr value) {
    if (m_var_type_current == ObjType::StrChar) {
        if (index.size() != 1 || !index[0].is_integer()) {
            LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
        }
        int64_t pos = index[0].integer().expect_int();
        if (pos < 0) {
            pos = m_value.size() + pos; // Позиция с конца строки
        }
        if (pos < static_cast<int64_t> (m_value.size())) {
            m_value.erase(pos, 1);
            m_value.insert(pos, value->toType(ObjType::StrChar)->m_value);
            m_var_is_init = true;
            return shared();
        }
        LOG_RUNTIME("Index '%s' not exists in byte string '%s'!", IndexToString(index).c_str(), m_value.c_str());
    } else if (m_var_type_current == ObjType::StrWide) {
        if (index.size() != 1 || !index[0].is_integer()) {
            LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
        }
        int64_t pos = index[0].integer().expect_int();
        if (pos < 0) {
            pos = m_value.size() + pos; // Позиция с конца строки
        }
        if (pos < static_cast<int64_t> (m_string.size())) {
            m_string.erase(pos, 1);
            m_string.insert(pos, value->toType(ObjType::StrWide)->m_string);
            m_var_is_init = true;
            return shared();
        }
        LOG_RUNTIME("Index '%s' not exists in byte string '%s'!", IndexToString(index).c_str(), "WIDE");

    } else if (is_tensor_type()) {
        ASSERT(!is_scalar());
        ASSERT(m_tensor->defined());

        ObjPtr temp = value->toType(fromTorchType(m_tensor->scalar_type()));
        if (temp->is_scalar()) {
            if (temp->is_integral()) {
                m_tensor->index_put_(index, temp->GetValueAsInteger());
            } else {
                ASSERT(temp->is_floating());
                m_tensor->index_put_(index, temp->GetValueAsNumber());
            }
        } else {
            ASSERT(temp->m_tensor->defined());
            m_tensor->index_put_(index, *temp->m_tensor);
        }
        m_var_is_init = true;
        return shared();

    } else if (is_dictionary_type()) {
        if (index.size() != 1 || !index[0].is_integer()) {
            LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
        }
        (*at(index[0].integer().expect_int()).second) = value;
        return shared();
        LOG_RUNTIME("Index '%s' not exists in object '%s'!", IndexToString(index).c_str(), toString().c_str());
    }
    LOG_RUNTIME("Don`t set index '%s' in object '%s'!", IndexToString(index).c_str(), toString().c_str());
}

ObjPtr Obj::op_set_index(int64_t index, std::string value) {
    if (m_var_type_current == ObjType::StrChar) {
        if (index < static_cast<int64_t> (m_value.size())) {
            m_value.erase(index, 1);
            m_value.insert(index, value);
            m_var_is_init = true;
            return shared();
        }
        LOG_RUNTIME("Index '%ld' not exists in byte string '%s'!", index, m_value.c_str());
    } else if (m_var_type_current == ObjType::StrWide) {
        if (index < static_cast<int64_t> (m_string.size())) {
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
        if (strong && find->op_accurate(elem.second)) {
            return true;
        } else if (!strong && find->op_equal(elem.second)) {
            return true;
        }
    }
    return false;
}

ObjType getSummaryTensorType(Obj *obj, ObjType start) {

    ObjType result = start;
    if (!obj) {
        return result;
    }
    if (obj->is_dictionary_type() || obj->is_range()) {
        for (int i = 0; i < obj->size(); i++) {
            result = getSummaryTensorType(obj->at(i).second.get(), result);
        }
        return result;
    } else if (obj->is_arithmetic_type()) {
        if (isGenericType(obj->m_var_type_fixed)) {
            return std::max(obj->m_var_type_current, start);
        } else if (start == ObjType::Rational || obj->m_var_type_current == ObjType::Rational || obj->m_var_type_fixed == ObjType::Rational) {
            return ObjType::Rational;
        } else {
            if (start >= std::max(obj->m_var_type_current, obj->m_var_type_fixed)) {
                return start;
            } else {
                return std::max(obj->m_var_type_current, obj->m_var_type_fixed);
            }
        }
    }
    LOG_RUNTIME("Tensor support arithmetic data type only '%s'!", obj->toString().c_str());
}

void Obj::clear_(bool clear_iterator_name) {

    m_value.clear();
    m_string.clear();
    m_var_type_current = ObjType::None;

    m_class_parents.clear();
    m_var_is_init = false;
    if (m_tensor) {
        m_tensor->reset();
    }
    m_rational.set_(0);
    m_var = std::monostate();
    //        m_value.reset(); //????????????????
    //        m_items.clear();
}

ObjPtr Obj::operator-() {
    if (is_arithmetic_type()) {
        if (is_tensor_type()) {
            (*m_tensor) = -(*m_tensor);
        } else if (is_integer()) {
            SetValue_(-GetValueAsInteger());
        } else if (isFloatingType(m_var_type_current)) {
            SetValue_(-GetValueAsNumber());
        } else {
            LOG_RUNTIME("Unary minus for object '%s' not supported!", toString().c_str());
        }
        return shared();
    }
    LOG_RUNTIME("Unary minus for object '%s' not supported!", toString().c_str());
}

ObjPtr Obj::operator--() {
    if (is_tensor_type()) {
        m_tensor->sub_(torch::ones_like(*m_tensor));
        return shared();
    }
    LOG_RUNTIME("Object '%s' not numeric!", toString().c_str());
}

ObjPtr & Obj::operator-(ObjPtr & obj) {
    if (is_tensor_type()) {
        *obj->m_tensor = torch::zeros_like(*obj->m_tensor) - *obj->m_tensor;
        return obj;
    }
    LOG_RUNTIME("Object '%s' not numeric!", obj->toString().c_str());
}


//префиксная версия возвращает значение после инкремента

ObjPtr Obj::operator++() {
    if (is_tensor_type()) {
        m_tensor->add_(torch::ones_like(*m_tensor));

        return shared();
    }
    LOG_RUNTIME("Object '%s' not numeric!", toString().c_str());
}

ObjPtr Obj::operator+=(Obj value) {
    if (is_tensor_type()) {
        if (value.is_tensor_type()) {
            testResultIntegralType(value.m_var_type_current, true);
            if (is_scalar() && value.is_scalar()) {
                if (is_floating()) {
                    ASSERT(std::holds_alternative<double>(m_var));
                    m_var = GetValueAsNumber() + value.GetValueAsNumber();
                    m_var_type_current = ObjType::Float64;
                } else if (is_integral() && value.is_integral()) {
                    ASSERT(std::holds_alternative<int64_t>(m_var));
                    m_var = GetValueAsInteger() + value.GetValueAsInteger();
                    m_var_type_current = typeFromLimit(GetValueAsInteger());
                } else {
                    LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
                }
            } else if (value.is_scalar()) {
                if (value.is_floating()) {
                    m_tensor->add_(value.GetValueAsNumber());
                } else if (value.is_integral()) {
                    m_tensor->add_(value.GetValueAsInteger());
                } else {
                    LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
                }
            } else {
                ASSERT(!is_scalar() && !value.is_scalar());
                m_tensor->add_(*value.m_tensor);
            }
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
                    m_value += value.m_value;
                    return shared();
                case ObjType::StrWide:
                    m_string += value.m_string;
                    return shared();
            }
            break;

        case ObjType::Class:
        case ObjType::Dictionary:
            if (value.m_var_type_current == ObjType::None) {
                return shared();
            } else if (value.m_var_type_current == ObjType::Class || value.m_var_type_current == ObjType::Dictionary) {
                for (int i = 0; i < value.size(); i++) {
                    push_back(value.at(i).second);
                }
                return shared();
            }
            break;

        case ObjType::Rational:
            if (value.m_var_type_current == ObjType::Rational) {
                m_rational.operator+=(value.m_rational);
            } else {
                m_rational.operator+=(value.toType(ObjType::Rational)->m_rational);
            }
            return shared();

    }
    LOG_RUNTIME("Operator '+' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

ObjPtr Obj::operator-=(Obj value) {
    if (m_var_type_current == ObjType::None) {
        m_var_type_current = value.m_var_type_current;
    } else if (is_tensor_type() && value.is_tensor_type()) {
        testResultIntegralType(value.m_var_type_current, true);
        if (is_scalar() && value.is_scalar()) {
            if (is_floating()) {
                ASSERT(std::holds_alternative<double>(m_var));
                m_var = GetValueAsNumber() - value.GetValueAsNumber();
                m_var_type_current = ObjType::Float64;
            } else if (is_integral() && value.is_integral()) {
                ASSERT(std::holds_alternative<int64_t>(m_var));
                m_var = GetValueAsInteger() - value.GetValueAsInteger();
                m_var_type_current = typeFromLimit(GetValueAsInteger());
            } else {
                LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
            }
        } else if (value.is_scalar()) {
            if (value.is_floating()) {
                m_tensor->sub_(value.GetValueAsNumber());
            } else if (value.is_integral()) {
                m_tensor->sub_(value.GetValueAsInteger());
            } else {
                LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
            }
        } else {
            ASSERT(!is_scalar() && !value.is_scalar());
            m_tensor->sub_(*value.m_tensor);
        }
        return shared();
    }
    switch (m_var_type_current) {
        case ObjType::Class:
        case ObjType::Dictionary:
            if (value.m_var_type_current == ObjType::None) {
                return shared();
            } else if (value.m_var_type_current == ObjType::Class || value.m_var_type_current == ObjType::Dictionary) {
                for (int i = 0; i < value.size(); i++) {
                    auto found = find(value.name(i));
                    if (found != end()) {
                        ListType::erase(found);
                    }
                }
                return shared();
            }
            break;
        case ObjType::Rational:
            if (value.m_var_type_current == ObjType::Rational) {
                m_rational.operator-=(value.m_rational);
            } else {
                m_rational.operator-=(value.toType(ObjType::Rational)->m_rational);
            }
            return shared();
    }
    LOG_RUNTIME("Operator '-' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

ObjPtr Obj::operator*=(Obj value) {
    if (m_var_type_current == ObjType::None) {
        m_var_type_current = value.m_var_type_current;
    } else if (is_tensor_type() && value.is_tensor_type()) {
        testResultIntegralType(value.m_var_type_current, true);
        if (is_scalar() && value.is_scalar()) {
            if (is_floating()) {
                ASSERT(std::holds_alternative<double>(m_var));
                m_var = GetValueAsNumber() * value.GetValueAsNumber();
                m_var_type_current = ObjType::Float64;
            } else if (is_integral() && value.is_integral()) {
                ASSERT(std::holds_alternative<int64_t>(m_var));
                m_var = GetValueAsInteger() * value.GetValueAsInteger();
                m_var_type_current = typeFromLimit(GetValueAsInteger());
            } else {
                LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
            }
        } else if (value.is_scalar()) {
            if (value.is_floating()) {
                m_tensor->mul_(value.GetValueAsNumber());
            } else if (value.is_integral()) {
                m_tensor->mul_(value.GetValueAsInteger());
            } else {
                LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
            }
        } else {
            ASSERT(!is_scalar() && !value.is_scalar());
            m_tensor->mul_(*value.m_tensor);
        }
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
            if (value.is_integral()) {
                m_value = repeat(m_value, value.GetValueAsInteger());
                return shared();
            } else if (value.is_string_type()) {
                m_value += value.GetValueAsString();
                return shared();
            }
        case ObjType::StrWide:
            if (value.is_integral()) {
                m_string = repeat(m_string, value.GetValueAsInteger());
                return shared();
            } else if (value.is_string_type()) {
                m_string += utf8_decode(value.GetValueAsString());
                return shared();
            }

        case ObjType::Rational:
            if (value.m_var_type_current == ObjType::Rational) {
                m_rational.operator*=(value.m_rational);
            } else {
                m_rational.operator*=(value.toType(ObjType::Rational)->m_rational);
            }
            return shared();

    }
    LOG_RUNTIME("Operator '*' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

ObjPtr Obj::operator/=(Obj value) {
    if (is_tensor_type() && value.is_tensor_type()) {
        testResultIntegralType(ObjType::Float64, true);
        if (is_scalar() && value.is_scalar()) {
            if (is_floating()) {
                ASSERT(std::holds_alternative<double>(m_var));
                m_var = GetValueAsNumber() / value.GetValueAsNumber();
                m_var_type_current = ObjType::Float64;
            } else if (is_integral() && value.is_integral()) {
                ASSERT(std::holds_alternative<int64_t>(m_var));
                m_var = GetValueAsInteger() / value.GetValueAsInteger();
                m_var_type_current = typeFromLimit(GetValueAsInteger());
            } else {
                LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
            }
        } else if (value.is_scalar()) {
            if (value.is_floating()) {
                m_tensor->div_(value.GetValueAsNumber());
            } else if (value.is_integral()) {
                m_tensor->div_(value.GetValueAsInteger());
            } else {
                LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
            }
        } else {
            ASSERT(!is_scalar() && !value.is_scalar());
            ASSERT(value.m_tensor);
            m_tensor->div_(*value.m_tensor);
        }
        return shared();
    } else if (m_var_type_current == ObjType::Rational) {
        if (value.m_var_type_current == ObjType::Rational) {
            m_rational.operator/=(value.m_rational);
        } else {
            m_rational.operator/=(value.toType(ObjType::Rational)->m_rational);
        }
        return shared();
    }
    LOG_RUNTIME("Operator '/' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

ObjPtr Obj::op_div_ceil_(Obj & value) {
    if (is_tensor_type() && value.is_tensor_type()) {
        ObjType type = m_var_type_current;
        //        testResultIntegralType(ObjType::Float32, false);
        if (is_scalar() && value.is_scalar()) {
            if (is_floating()) {
                ASSERT(std::holds_alternative<double>(m_var));
                m_var = floor(GetValueAsNumber() / value.GetValueAsNumber());
                m_var_type_current = ObjType::Float64;
            } else if (is_integral() && value.is_integral()) {
                ASSERT(std::holds_alternative<int64_t>(m_var));

                int64_t num = GetValueAsInteger();
                int64_t den = value.GetValueAsInteger();
                if (0 < (num^den)) {
                    m_var = static_cast<int64_t> (num / den);
                } else {
                    ldiv_t res = std::ldiv(num, den);
                    m_var = static_cast<int64_t> ((res.rem) ? res.quot - 1 : res.quot);
                }
                m_var_type_current = typeFromLimit(GetValueAsInteger());
            } else {
                LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
            }
        } else if (value.is_scalar()) {
            if (value.is_floating()) {
                m_tensor->div_(value.GetValueAsNumber(), "floor");
            } else if (value.is_integral()) {
                m_tensor->div_(value.GetValueAsInteger(), "floor");
            } else {
                LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
            }
        } else {
            ASSERT(!is_scalar() && !value.is_scalar());
            ASSERT(value);
            m_tensor->div_(*value.m_tensor, "floor");
            (*m_tensor) = m_tensor->toType(toTorchType(type));
        }
        //        ObjType type = m_var_type_current;
        //        testResultIntegralType(ObjType::Float32, false);
        //        m_tensor->div_(value.m_tensor, "floor");
        //        m_tensor = m_tensor->toType(toTorchType(type));
        return shared();

    } else if (m_var_type_current == ObjType::Rational) {
        if (value.m_var_type_current == ObjType::Rational) {
            m_rational.op_div_ceil_(value.m_rational);
        } else {
            m_rational.op_div_ceil_(value.toType(ObjType::Rational)->m_rational);
        }
        return shared();
    }
    LOG_RUNTIME("Operator '//' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

ObjPtr Obj::operator%=(Obj value) {
    if (is_tensor_type() && value.is_tensor_type()) {
        testResultIntegralType(value.m_var_type_current, true);
        if (is_scalar() && value.is_scalar()) {
            if (is_floating()) {
                ASSERT(std::holds_alternative<double>(m_var));
                m_var = fmod(GetValueAsNumber(), value.GetValueAsNumber());
            } else if (is_integral() && value.is_integral()) {
                ASSERT(std::holds_alternative<int64_t>(m_var));
                m_var = GetValueAsInteger() % value.GetValueAsInteger();
            } else {
                LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
            }
        } else if (value.is_scalar()) {
            if (value.is_floating()) {
                m_tensor->fmod_(value.GetValueAsNumber());
            } else if (value.is_integral()) {
                m_tensor->fmod_(value.GetValueAsInteger());
            } else {
                LOG_RUNTIME("Fail convert '%s' to type %s!", value.toString().c_str(), newlang::toString(m_var_type_current));
            }
        } else {
            ASSERT(!is_scalar() && !value.is_scalar());
            ASSERT(value.m_tensor);
            m_tensor->fmod_(*value.m_tensor);
        }
        return shared();
    } else if (m_var_type_current == ObjType::Rational) {
        if (value.m_var_type_current == ObjType::Rational) {
            m_rational.operator%=(value.m_rational);
        } else {
            m_rational.operator%=(value.toType(ObjType::Rational)->m_rational);
        }
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
        } else {
            clone.m_dimensions = nullptr;
        }

        clone.m_var_name = m_var_name;
        clone.m_ctx = m_ctx;
        clone.m_value = m_value;
        clone.m_string = m_string;
        clone.m_return_obj = m_return_obj;

        clone.m_rational = *m_rational.clone();
        clone.m_iterator = m_iterator;
        if (m_iter_range_value) {
            clone.m_iter_range_value = m_iter_range_value->Clone();
        }

        clone.m_class_parents = m_class_parents;
        clone.m_class_name = m_class_name;
        clone.m_is_const = m_is_const;

        clone.m_var = m_var;
        if (m_prototype) {
            *const_cast<TermPtr *> (&clone.m_prototype) = m_prototype;
        }
        ASSERT(m_tensor);
        if (m_tensor->defined()) {
            *clone.m_tensor = m_tensor->clone();
        }
    }
}

void Obj::ClonePropTo(Obj & clone) const {

    NL_CHECK(!isLocalType(m_var_type_current), "Local object not clonable!");

    for (int i = 0; i < Variable<Obj>::size(); i++) {
        if (Variable<Obj>::at(i).second) {
            //            if (Variable<Obj>::at(i).second->m_is_reference || Variable<Obj>::at(i).second->m_is_reference) {
            //                clone.Variable<Obj>::push_back(Variable<Obj>::at(i));
            //            } else {
            clone.Variable<Obj>::push_back(Variable<Obj>::at(i).second->Clone(nullptr), name(i));
            //            }
        } else {
            if (name(i).empty()) {
                LOG_RUNTIME("Null arg %d without name! %s", i, toString().c_str());
            }
            // Объекта может не быть у обязательных параметров функций
            clone.Variable<Obj>::push_back(nullptr, name(i));
        }
    }
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

    std::stringstream ss;

    if (m_var_type_current == ObjType::None) {
//        if (m_prototype && m_prototype->GetType()) {
//            result += m_prototype->GetType()->toString();
//        } else if (m_var_type_fixed != ObjType::None) {
//            result += newlang::toString(m_var_type_fixed);
//        }
        result += "_";
        return result;
    } else if (is_tensor_type()) {
        if (is_scalar()) {
            result += GetValueAsString();
        } else {
            ASSERT(m_tensor);
            ASSERT(m_tensor->defined());
            result += TensorToString(*m_tensor);
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

                //            case ObjType::Return:
            case ObjType::RetPlus:
            case ObjType::RetMinus:
            case ObjType::RetRepeat:
                if (m_class_name.empty()) {
                    result += newlang::toString(m_var_type_current);
                } else {
                    result += m_class_name;
                }
                result += "(";
                result += m_return_obj->toString();
                result += ")";
                return result;


            case ObjType::Struct:
            case ObjType::Union:
            case ObjType::Enum:

            case ObjType::Error:
            case ObjType::ErrorParser:
            case ObjType::ErrorRunTime:
            case ObjType::ErrorSignal:
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

            case ObjType::Module:
            case ObjType::Dictionary: // name:=(1,second="two",3,<EMPTY>,5)
                result += "(";
                dump_dict_(result);
                result += ",";
                result += ")";
                result += m_class_name;
                return result;

            case ObjType::Pointer:
                if (std::holds_alternative<void *>(m_var)) {
                    ss << std::get<void *>(m_var);
                    //                } else if (m_var_type_fixed == ObjType::None || m_var_type_current == ObjType::None) {
                    //                    ss << "nullptr";
                } else {
                    LOG_RUNTIME("Fail convert object to pointer!");
                }
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
                        result += (*m_dimensions)[i].second->toString();
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
            case ObjType::Function: // name:={function code}
            case ObjType::PureFunc: // name=>{function code}
//                ASSERT(m_prototype);
//                result += m_prototype->m_text;
//                result += "(";
//                m_prototype->dump_items_(result);
//                result += ")";
//                if (m_prototype->m_type) {
//                    result += m_prototype->m_type->asTypeString();
//                }

            case ObjType::BLOCK:
                result += "{ }";
                return result;

            case ObjType::BLOCK_TRY:
                result += "{* *}";
                return result;

            case ObjType::BLOCK_PLUS:
                result += "{+ +}";
                return result;

            case ObjType::BLOCK_MINUS:
                result += "{- -}";
                return result;

            case ObjType::EVAL_FUNCTION: // name=>{function code}
//                ASSERT(m_prototype);
//                result += m_prototype->m_text;
//                result += "(";
//                m_prototype->dump_items_(result);
//                result += ")";
//                if (m_prototype->m_type) {
//                    result += m_prototype->m_type->asTypeString();
//                }
//
//                if (m_var_type_current == ObjType::EVAL_FUNCTION) {
//                    result += ":=";
//                } else if (m_var_type_current == ObjType::PureFunc) {
//                    result += ":-";
//                } else {
//                    LOG_RUNTIME("Fail function type");
//                }
//
//                if (m_sequence->getTermID() != TermID::BLOCK) {
//                    result += "{";
//                }
//                if (m_sequence) {
//                    result += m_sequence->toString();
//                    if (m_sequence->getTermID() != TermID::BLOCK) {
//                        result += ";";
//                    }
//                }
//                if (m_sequence->getTermID() != TermID::BLOCK) {
//                    result.append("}");
//                }
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

            case ObjType::Rational:
                result += m_rational.GetAsString();
                return result;

            case ObjType::Iterator:
            case ObjType::IteratorEnd:
                return newlang::toString(m_var_type_current);

                //            case ObjType::Context:
                //            {
                //                Context * ctx = (Context *)this;
                //
                //                ObjPtr temp = Obj::CreateType(ObjType::Dictionary, ObjType::Dictionary, true);
                //                temp->m_var_name = "$$";
                //
                //                auto iter = ctx->begin();
                //                while (iter != ctx->end()) {
                //                    if (!iter->second.expired()) {
                //                        temp->push_back(Obj::CreateString(iter->first));
                //                        iter++;
                //                    } else {
                //                        iter = ctx->ListType::erase(iter);
                //                    }
                //                }
                //
                //                return temp->toString();
                //            }
        }
    }
    LOG_RUNTIME("Unknown type '%s' (%d)", newlang::toString(m_var_type_current), (int) m_var_type_current);
}

void TensorToString_(const torch::Tensor &tensor, c10::IntArrayRef shape, std::vector<Index> &ind, const int64_t pos,
        std::stringstream & str) {
    std::string intend;
    ASSERT(pos < static_cast<int64_t> (ind.size()));
    str << "[";
    if (shape.size() > 1 && pos + 1 < static_cast<int64_t> (ind.size())) {
        str << "\n";
        intend = std::string((pos + 1) * 2, ' ');
        str << intend;
    }
    if (pos + 1 < static_cast<int64_t> (ind.size())) {
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

std::string TensorToString(const torch::Tensor & tensor) {
    std::string result;
    std::stringstream ss;

    if (!tensor.dim()) {
        LOG_RUNTIME("!tensor.dim() %s", tensor.toString().c_str());
        //        ASSERT(tensor.dim());
    }

    c10::IntArrayRef shape = tensor.sizes(); // Кол-во эментов в каждом измерении
    std::vector<Index> ind(shape.size(), 0); // Счетчик обхода всех элементов тензора
    TensorToString_(tensor, shape, ind, 0, ss);
    result = ss.str();
    result += newlang::toString(fromTorchType(tensor.scalar_type()));

    return result;
}

bool Obj::GetValueAsBoolean() const {
    TEST_INIT_();
    if (m_var_type_current == ObjType::IteratorEnd) {
        return false;
    }
    if (is_scalar()) {
        return GetValueAsInteger();
        //return m_tensor->toType(at::ScalarType::Bool).item<double>();
    } else if (isSimpleType(m_var_type_current)) {
        // Error: Boolean value of Tensor with more than one value is ambiguous
        ASSERT(m_tensor);
        return !at::_is_zerotensor(*m_tensor);
    } else {
        switch (m_var_type_current) {
            case ObjType::StrWide:
                return !m_string.empty();
            case ObjType::StrChar:
                return !m_value.empty();
            case ObjType::None:
                return false;

            case ObjType::Rational:
                return m_rational.GetAsBoolean();

            case ObjType::Dictionary:
            case ObjType::Class:
                if (size()) {
                    return true;
                }
                for (auto &elem : m_class_parents) {
                    if (elem->GetValueAsBoolean()) {
                        return true;
                    }
                }
                return false;

            case ObjType::Iterator:
                ASSERT(m_iterator);
                ASSERT(m_iterator->m_iter_obj);
                if (m_iterator->m_iter_obj->getType() == ObjType::Range) {
                    if (m_iterator->m_iter_obj->m_iter_range_value && m_iterator->m_iter_obj->m_iter_range_value->m_var_type_current != ObjType::IteratorEnd) {
                        return true;
                    }
                } else {
                    if ((*m_iterator) != m_iterator->end()) {
                        return m_iterator->data().second->GetValueAsBoolean();
                    }
                }
                return false;

            default:
                LOG_RUNTIME("Type cast to bool %s", toString().c_str());
        }
        return true;
    }
}

int64_t Obj::GetValueAsInteger() const {
    TEST_INIT_();

    switch (m_var_type_current) {
        case ObjType::Bool:
            if (std::holds_alternative<int64_t>(m_var)) {
                return std::get<int64_t>(m_var);
            } else if (std::holds_alternative<bool *>(m_var)) {
                return *std::get<bool *>(m_var);
            }
        case ObjType::Int8:
        case ObjType::Char:
        case ObjType::Byte:
            if (std::holds_alternative<int64_t>(m_var)) {
                return std::get<int64_t>(m_var);
            } else if (std::holds_alternative<int8_t *>(m_var)) {
                return *std::get<int8_t *>(m_var);
            }
        case ObjType::Int16:
        case ObjType::Word:
            if (std::holds_alternative<int64_t>(m_var)) {
                return std::get<int64_t>(m_var);
            } else if (std::holds_alternative<int16_t *>(m_var)) {
                return *std::get<int16_t *>(m_var);
            }
        case ObjType::Int32:
        case ObjType::DWord:
            if (std::holds_alternative<int64_t>(m_var)) {
                return std::get<int64_t>(m_var);
            } else if (std::holds_alternative<int32_t *>(m_var)) {
                return *std::get<int32_t *>(m_var);
            }
        case ObjType::Int64:
        case ObjType::DWord64:
            if (std::holds_alternative<int64_t>(m_var)) {
                return std::get<int64_t>(m_var);
            } else if (std::holds_alternative<int64_t *>(m_var)) {
                return *std::get<int64_t *>(m_var);
            }
        case ObjType::Integer:
            if (std::holds_alternative<int64_t>(m_var)) {
                return std::get<int64_t>(m_var);
                //                    } else if (m_tensor->dim()==0) {
                //                        return m_tensor->item<int64_t>();
            }
            ASSERT(!is_scalar());
            LOG_RUNTIME("Can`t convert tensor to scalar!");

        case ObjType::Float16:
        case ObjType::Float32:
        case ObjType::Float64:
        case ObjType::Single:
        case ObjType::Double:
        case ObjType::Number:
            return static_cast<int64_t> (GetValueAsNumber());

        case ObjType::Rational:
            return m_rational.GetAsInteger();

        case ObjType::StrWide:
        case ObjType::FmtWide:
            if (m_string.size() == 1) {
                return m_string[0];
            }
        case ObjType::StrChar:
        case ObjType::FmtChar:
            if (m_value.size() == 1) {
                return m_value[0];
            }

        case ObjType::Iterator:
            ASSERT(m_iterator);
            return m_iterator->data().second->GetValueAsInteger();

        default:
            if (m_var_type_current == ObjType::Pointer || m_var_type_fixed == ObjType::Pointer) {
                ASSERT(std::holds_alternative<void *>(m_var));
                return reinterpret_cast<int64_t> (std::get<void *>(m_var));
            }
    }
    LOG_RUNTIME("Data type incompatible %s", toString().c_str());
}

Obj::operator float() const {
    double result = GetValueAsNumber();
    if (result > (double) std::numeric_limits<float>::max()) {
        LOG_RUNTIME("Value1 '%s' %.20f %.20f %.20f is out of range of the casting type float!", GetValueAsString().c_str(), result, std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest());
    }

    //    __asm__ volatile ( "; //SOURCE: __FLT_MAX__ ");
    if (result < -__FLT_MAX__) {//(double) std::numeric_limits<float>::lowest()) {
        LOG_RUNTIME("Value2 '%s' %.20f %.20f %.20f is out of range of the casting type float!", GetValueAsString().c_str(), result, std::numeric_limits<float>::max(), std::numeric_limits<float>::lowest());
    }
    LOG_DEBUG("operator float() '%s' %.20f", GetValueAsString().c_str(), result);
    return result;
}

Obj::operator double() const {
    return GetValueAsNumber();
}

double Obj::GetValueAsNumber() const {
    TEST_INIT_();

    switch (m_var_type_current) {

        case ObjType::Single:
        case ObjType::Float16:
        case ObjType::Float32:
            if (std::holds_alternative<double>(m_var)) {
                LOG_DEBUG("1std::get<double>(m_var) %.20f", std::get<double>(m_var));
                return std::get<double>(m_var);
            } else if (std::holds_alternative<float *>(m_var)) {
                LOG_DEBUG("1std::get<float *>(m_var) %.20f", *std::get<float *>(m_var));
                return *std::get<float *>(m_var);
            }
        case ObjType::Double:
        case ObjType::Float64:
            if (std::holds_alternative<double>(m_var)) {
                LOG_DEBUG("2std::get<double>(m_var) %.20f", std::get<double>(m_var));
                return std::get<double>(m_var);
            } else if (std::holds_alternative<double *>(m_var)) {
                LOG_DEBUG("2std::get<float *>(m_var) %.20f", *std::get<float *>(m_var));
                return *std::get<double *>(m_var);
            }
        case ObjType::Number:
            if (std::holds_alternative<double>(m_var)) {
                return std::get<double>(m_var);
            }
            if (is_scalar()) {
                ASSERT(!is_scalar());
            }
            LOG_RUNTIME("Can`t convert tensor to scalar!");

        case ObjType::Rational:
            return m_rational.GetAsNumber();

        case ObjType::Iterator:
            ASSERT(m_iterator);
            return m_iterator->data().second->GetValueAsNumber();

        default:
            if (is_simple_type() || is_string_type()) {
                LOG_DEBUG("3is_simple_type() || is_string_type() %.20f", static_cast<double> (GetValueAsInteger()));
                return static_cast<double> (GetValueAsInteger());
            }
    }
    LOG_RUNTIME("Data type incompatible %s", toString().c_str());
}

std::string Obj::GetValueAsString() const {
    std::string result;
    std::string temp;
    std::stringstream ss;

    if (!m_var_is_init) {
        LOG_RUNTIME("Object not initialized name:'%s' type:%s, fix:%s!",
                m_var_name.c_str(), newlang::toString(m_var_type_current), newlang::toString(m_var_type_fixed));
    }

    switch (m_var_type_current) {
        case ObjType::None:
            return result;

        case ObjType::Tensor:
        case ObjType::Bool:
        case ObjType::Int8:
        case ObjType::Int16:
        case ObjType::Int32:
        case ObjType::Int64:
        case ObjType::Char:
        case ObjType::Byte:
        case ObjType::Word:
        case ObjType::DWord:
        case ObjType::DWord64:
        case ObjType::Integer:
        case ObjType::Float16:
        case ObjType::Float32:
        case ObjType::Float64:
        case ObjType::Single:
        case ObjType::Double:
        case ObjType::Number:
        case ObjType::Complex:
        case ObjType::Complex16:
        case ObjType::Complex32:
        case ObjType::Complex64:
            if (is_scalar()) {
                if (is_integral()) {
                    return std::to_string(GetValueAsInteger());
                } else if (is_floating()) {
                    ss << GetValueAsNumber();
                    return ss.str();
                } else {
                    ASSERT(!"Not implemented!");
                }
            } else {
                return TensorToString(*m_tensor);
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
            return m_var_name + "={ }";

        case ObjType::Class:
        case ObjType::Dictionary:
            return toString();

        case ObjType::Error:
            result = m_var_name;
            if (!m_var_name.empty()) {
                result += ": ";
            }
            temp = m_value;
            trim(temp, "\n");
            result += temp;
            return result;

        case ObjType::Pointer:
            ASSERT(std::holds_alternative<void *>(m_var));
            ss << std::get<void *>(m_var);
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

        case ObjType::Rational:
            result += m_rational.GetAsString();
            return result;

        case ObjType::Iterator:
        case ObjType::IteratorEnd:
            return newlang::toString(m_var_type_current);
    }
    LOG_RUNTIME("Data type '%s' %d incompatible to string!", newlang::toString(m_var_type_current), (int) m_var_type_current);
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
    } else if (is_rational()) {

        if (value.getType() == ObjType::Rational) {
            return m_rational.op_compare(value.m_rational);
        } else {
            return m_rational.op_compare(*value.GetValueAsRational());
        }

    } else if ((is_string_type() && value.is_string_type())) {
        switch (m_var_type_current) {
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
    if (this == &value) {
        return true;
    } else if (is_tensor_type()) {
        ObjType summary_type = static_cast<ObjType> (std::max(
                static_cast<uint8_t> (m_var_type_current),
                static_cast<uint8_t> (value.m_var_type_current)));
        try {
            if (is_scalar() || value.is_scalar()) {
                if (is_scalar() && value.is_scalar()) {
                    if (isIntegralType(summary_type, true)) {
                        return GetValueAsInteger() == value.GetValueAsInteger();
                    } else if (isFloatingType(summary_type)) {
                        return GetValueAsNumber() == value.GetValueAsNumber();
                    } else {
                        LOG_RUNTIME("Fail compare type '%s'!", newlang::toString(summary_type));
                    }
                }
                return false;
            }
            torch::Dtype summary_torch_type = toTorchType(static_cast<ObjType> (summary_type));
            return m_tensor->toType(summary_torch_type).equal(*value.toType(summary_type)->m_tensor);

        } catch (std::exception e) {
            LOG_RUNTIME("Fail compare"); //, e.what());
        }
    } else if (is_bool_type()) {
        return GetValueAsBoolean() == value.GetValueAsBoolean();
    } else if (is_string_type()) {
        return GetValueAsString().compare(value.GetValueAsString()) == 0;
    } else if (is_rational()) {

        if (value.getType() == ObjType::Rational) {
            return m_rational.op_equal(value.m_rational);
        } else {
            return m_rational.op_equal(*value.GetValueAsRational());
        }

    } else if (is_dictionary_type() && value.is_dictionary_type()) {
        if (size() != value.size()) {
            return false;
        }
        for (int64_t i = 0; i < static_cast<int64_t> (size()); i++) {
            if (name(i).compare(value.name(i)) != 0) {
                return false;
            }
            if (!at(i).second->op_equal(value[i].second)) {

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
    if (m_var_type_current == ObjType::Int64) {
        if (m_var_type_current == obj.m_var_type_current) {
            m_tensor->bitwise_and_(*obj.m_tensor);
            //            m_values.integer &= obj.m_values.integer;
            return shared();
        }
    } else if (m_var_type_current == ObjType::None || obj.m_var_type_current == ObjType::None) {
        Variable::clear_();
        return shared();
    } else if (m_var_type_current == ObjType::Dictionary || m_var_type_current == ObjType::Class) {
        if (obj.m_var_type_current == ObjType::Dictionary || obj.m_var_type_current == ObjType::Class) {
            int pos = 0;
            while (pos < size()) {
                if (!obj.exist(at(pos).second, strong)) {
                    erase(pos);
                } else {
                    pos++;
                }
            }
            return shared();
        }
    } else if (is_tensor_type() && obj.is_tensor_type()) {
        int pos = 0;
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

bool Obj::op_class_test(ObjPtr obj, Context * ctx) const {
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

//bool Obj::op_class_test(const char *name, Context * ctx) const {
//
//    ASSERT(name || *name);
//
//    if (!m_class_name.empty() && m_class_name.compare(name) == 0) {
//        return true;
//    }
//    for (auto &elem : m_class_parents) {
//        if (elem->op_class_test(name, ctx)) {
//            return true;
//        }
//    }
//
//    bool has_error = false;
//    ObjType type = RunTime::BaseTypeFromString(m_ctx ? m_ctx->m_runtime.get() : nullptr, name, &has_error);
//    if (has_error) {
//        LOG_DEBUG("Type name %s not found!", name);
//        return false;
//    }
//
//    ObjType check_type = m_var_type_current;
//    if (m_var_type_current == ObjType::Type || (!m_var_is_init && m_var_type_current == ObjType::None)) {
//        check_type = m_var_type_fixed;
//    }
//
//    if (isContainsType(type, check_type)) {
//        return true;
//    }
//
//    std::string class_name = newlang::toString(check_type);
//    return !class_name.empty() && class_name.compare(name) == 0;
//}

bool Obj::op_duck_test_prop(Obj *base, Obj *value, bool strong) {
    if (!value) {
        return !strong; // Пустой объект равен любому при нечетком сравнении
    }
    if (!base || base->m_var_type_current == ObjType::None) {
        // Итина при пустом текущем может быть только если образец тоже пустой
        return value->m_var_type_current == ObjType::None;
    }
    ObjPtr field;
    for (int i = 0; i < value->size(); i++) {
        if (value->name(i).empty()) {
            field = (*base)[i].second;
        } else {
            field = (*base)[value->name(i)].second;
        }
        if (!field) {
            return false;
        }
        if (strong || !((*value)[i].second->getType() != ObjType::None)) {
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
    if (is_tensor_type()) {
        ASSERT(obj.is_arithmetic_type());
        if (is_scalar()) {
            double temp = pow(GetValueAsNumber(), obj.GetValueAsNumber());
            if (is_integral()) {
                if (temp > static_cast<double> (std::numeric_limits<int64_t>::max())) {
                    LOG_ERROR("Integer overflow!");
                }
                m_var = static_cast<int64_t> (llround(temp));
            } else {
                m_var = temp;
            }
        } else {
            ASSERT(m_tensor->defined());
            m_tensor->pow_(obj.GetValueAsNumber());
        }
        return shared();
    } else if (is_rational()) {

        if (obj.getType() == ObjType::Rational) {
            m_rational.op_pow_(obj.m_rational);
        } else {
            m_rational.op_pow_(*obj.GetValueAsRational());
        }
        return shared();

    } else if (m_var_type_current == ObjType::StrChar && obj.is_integral()) {
        m_value = repeat(m_value, obj.GetValueAsInteger());
        return shared();
    } else if (m_var_type_current == ObjType::StrWide && obj.is_integral()) {
        m_string = repeat(m_string, obj.GetValueAsInteger());
        return shared();
    } else if (m_var_type_current == ObjType::Rational) {
        //        if(value.m_var_type_current == ObjType::Rational) {
        //            m_rational->op_pow_(value.m_rational);
        //        } else {
        //            m_rational->op_pow_(value.toType(ObjPtr::Rational)->m_rational);
        //        }
        //        return shared();
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
        if (value->is_simple_type()) {
            if (m_var_type_current == value->m_var_type_current || (is_string_type() && value->is_string_type())) {
                return true;
            }
            return false;
        }
        return op_duck_test_prop(this, value, strong);
    }
    if (value->m_var_type_current == ObjType::Int64 || value->m_var_type_current == ObjType::Float64) {
        return (m_var_type_current == ObjType::Int64 || m_var_type_current == ObjType::Float64);
    } else if (is_string_type() && value->is_string_type()) {
        return true;
    } else if (is_function_type() && value->is_function_type()) {
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
        for (int i = 0; i < args->size(); i++) {

            if (isSystemName(args->name(i))) {
                continue;
            }

            // Заменить номер аргумента
            name = "\\$" + std::to_string(i + 1);
            place = (*args)[i].second->GetValueAsString();
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
            dest->m_value.append(add);
            size = add.size();
        } else if (dest->m_var_type_current == ObjType::StrWide) {
            std::wstring add = src.GetValueAsStringWide();
            dest->m_string.append(add);
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

    } else if (dest->is_tensor_type()) {

        if (dest->m_var_type_current == src.m_var_type_current) {
            if (dest->m_tensor->dim() == 0) {
                dest->m_tensor->resize_(1);
            }
            if (src.m_tensor->dim() == 0) {
                src.m_tensor->resize_(1);
            }
            *dest->m_tensor = torch::cat({*(dest->m_tensor), *src.m_tensor});
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
    if (obj && (obj->is_dictionary_type() || (obj->is_tensor_type() && !obj->is_scalar()))) {
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

/*
 * Так как под виндой не получается передавать аргументы в функции при вызове LLVMRunFunction вернул libffi.
 */

bool Obj::is_scalar() const {
    return is_tensor_type() && m_tensor && !m_tensor->defined();
}

/*
 * 
 * 
 */

void ConvertStringToTensor(const std::string &from, torch::Tensor &to, ObjType type) {
    if (from.empty()) {
        LOG_RUNTIME("Fail convert empty string to tensor!");
    }
    to = torch::from_blob((void *) from.c_str(),{(int64_t) from.size()}, at::ScalarType::Char).clone().toType(toTorchType(type));
}

void ConvertStringToTensor(const std::wstring &from, torch::Tensor &to, ObjType type) {
    if (from.empty()) {
        LOG_RUNTIME("Fail convert empty string to tensor!");
    }
    if (sizeof (wchar_t) == sizeof (int32_t)) {
        to = torch::from_blob((void *) from.c_str(),{(int64_t) from.size()}, torch::Dtype::Int).clone().toType(toTorchType(type));
    } else if (sizeof (wchar_t) == sizeof (int16_t)) {
        to = torch::from_blob((void *) from.c_str(),{(int64_t) from.size()}, torch::Dtype::Short).clone().toType(toTorchType(type));
    } else {
        LOG_RUNTIME("Unsupport wchar_t size '%d'!!!", (int) sizeof (wchar_t));
    }
}

template <typename T> void ConvertTensorToStringTemplate(const torch::Tensor &from, T &to, std::vector<Index> *index) {

    ASSERT(from.dim()); // Скаляры хранятся не тензорами, а нативными типами

    std::vector<Index> dims;
    if (index == nullptr) {
        to.clear();
        dims.push_back(Index(0));
        index = &dims;
    }

    int64_t pos = index->size();
    if (pos == from.dim()) {
        at::ScalarType torch_type;
        switch (sizeof (to[0])) {
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

void ConvertTensorToString(const torch::Tensor &from, std::string &to, std::vector<Index> *index) {
    ConvertTensorToStringTemplate<std::string>(from, to, index);
}

void ConvertTensorToString(const torch::Tensor &from, std::wstring &to, std::vector<Index> *index) {
    ConvertTensorToStringTemplate<std::wstring>(from, to, index);
}

void ConvertTensorToDict(const torch::Tensor &from, Obj &to, std::vector<Index> *index) {

    to.m_var_is_init = false;
    ASSERT(to.m_var_type_current == ObjType::Dictionary || to.m_var_type_current == ObjType::None);
    if (!to.is_dictionary_type()) {
        to.m_var_type_current = ObjType::Dictionary;
    }

    ASSERT(from.dim()); // Скаляры хранятся не тензорами, а нативными типами

    std::vector<Index> dims;
    if (index == nullptr) {
        dims.push_back(Index(0));
        index = &dims;
    }

    int64_t pos = index->size();
    if (pos == from.dim()) {
        for (int i = 0; i < from.size(pos - 1); i++) {
            (*index)[pos - 1] = i;
            to.push_back(CreateTensor(from.index(*index)));
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
    if (m_var_type_current == type || type == ObjType::Any || (is_string_char_type() && isString(type))) {
        return;
    } else if (type == ObjType::None) {
        clear_();
        return;
    } else if ((is_none_type() || m_var_type_current == ObjType::Type || m_var_type_fixed == ObjType::Pointer) && type == ObjType::Pointer) {

        m_var_type_current = type;
        return;

    } else if (is_tensor_type() && isTensor(type)) {

        // Изменить тип тензора
        if (isGenericType(type)) {
            m_var_type_fixed = type;
        }

        if (is_scalar()) {
            if ((isGenericType(type) && isContainsType(type, m_var_type_current)) ||
                    (is_integral() && isIntegralType(type, true)) || (is_floating() && isFloatingType(type))) {
                // Ничего менять ненужно
            } else if (isIntegralType(type, true)) {
                m_var = GetValueAsInteger();
            } else if (isFloatingType(type)) {
                m_var = GetValueAsNumber();
            } else {
                LOG_RUNTIME("Unknown convert value!");
            }
            if (!isGenericType(type)) {
                m_var_type_current = type;
            }

        } else {
            ASSERT(m_tensor->defined());
            if (!isGenericType(type)) {
                *m_tensor = m_tensor->toType(toTorchType(type));
                m_var_type_current = fromTorchType(m_tensor->scalar_type());
            }
        }

        return;

    } else if (is_range() && isDictionary(type)) {


        ObjPtr iter = IteratorMake();
        ObjPtr temp = iter->IteratorNext(std::numeric_limits<int64_t>::max());
        Variable::clear_();
        for (auto &elem : *temp) {
            Variable::push_back(elem);
        }
        m_var_type_current = type;
        m_var_type_fixed = ObjType::None;
        return;

    } else if (is_range() && isTensor(type)) {

        toType_(ObjType::Dictionary);
        toType_(type);
        return;

    } else if (is_string_char_type() && isTensor(type)) {

        if (isGenericType(type)) {
            m_var_type_fixed = type;
            m_var_type_current = ObjType::Int8;
        } else {
            m_var_type_current = type;
        }

        if (m_value.size() == 1) {
            // Скаляр хранится в нативном типе
            if (isIntegralType(m_var_type_current, true)) {
                m_var = static_cast<int64_t> (m_value[0]);
            } else {
                ASSERT(isFloatingType(m_var_type_current));
                m_var = static_cast<double> (m_value[0]);
            }
        } else {
            ConvertStringToTensor(m_value, *m_tensor, m_var_type_current);
        }
        m_value.clear();
        return;

    } else if (is_string_wide_type() && isTensor(type)) {

        if (isGenericType(type)) {
            m_var_type_fixed = type;
            if (sizeof (wchar_t) == 4) {
                m_var_type_current = ObjType::Int32;
            } else {
                ASSERT(sizeof (wchar_t) == 2);
                m_var_type_current = ObjType::Int16;
            }
        } else {
            m_var_type_current = type;
        }

        if (m_string.size() == 1) {
            // Скаляр хранится в нативном типе
            if (isIntegralType(m_var_type_current, true)) {
                m_var = static_cast<int64_t> (m_string[0]);
            } else {
                ASSERT(isFloatingType(m_var_type_current));
                m_var = static_cast<double> (m_string[0]);
            }
        } else {
            ConvertStringToTensor(m_string, *m_tensor, m_var_type_current);
        }
        m_string.clear();
        return;

    } else if (is_tensor_type() && isStringChar(type)) {

        if (is_scalar()) {
            int64_t char_val = GetValueAsInteger();
            if ((char_val < 0 && char_val < std::numeric_limits<char>::min()) ||
                    (char_val > std::numeric_limits<uint8_t>::max())) {
                LOG_ERROR("Single char overflow! %ld", char_val);
            }
            if (char_val < 0) {
                // По стандарту char - знаковый тип
                m_value.assign(1, static_cast<char> (char_val));
            } else {
                // Но часто про это забывают и забивают
                m_value.assign(1, static_cast<uint8_t> (char_val));
            }
            m_var = std::monostate();
        } else {
            ASSERT(!is_scalar());
            if (static_cast<uint8_t> (m_var_type_current) > static_cast<uint8_t> (ObjType::Int8)) {
                LOG_ERROR("Possible data loss when converting tensor %s to a byte string!", newlang::toString(m_var_type_current));
            }
            ConvertTensorToString(*m_tensor, m_value);
            m_tensor->reset();
        }
        m_var_type_current = type;
        return;

    } else if (is_tensor_type() && isStringWide(type)) {

        if (is_scalar()) {
            int64_t char_val = GetValueAsInteger();
            if ((char_val < std::numeric_limits<wchar_t>::min()) ||
                    (char_val > std::numeric_limits<wchar_t>::max())) {
                LOG_ERROR("Single wchar_t overflow! %ld", char_val);
            }
            m_string.assign(1, static_cast<wchar_t> (char_val));
            m_var = std::monostate();
        } else {
            ASSERT(!is_scalar());
            ASSERT(sizeof (wchar_t) == 2 || sizeof (wchar_t) == 4);
            if ((sizeof (wchar_t) == 2 && static_cast<uint8_t> (m_var_type_current) > static_cast<uint8_t> (ObjType::Int16)) ||
                    (sizeof (wchar_t) == 4 && static_cast<uint8_t> (m_var_type_current) > static_cast<uint8_t> (ObjType::Int32))) {
                LOG_ERROR("Possible data loss when converting tensor %s to a wide string!", newlang::toString(m_var_type_current));
            }
            ASSERT(m_tensor);
            ConvertTensorToString(*m_tensor, m_string);
            m_tensor->reset();
        }
        m_var_type_current = type;
        return;

    } else if (is_tensor_type() && type == ObjType::Rational) {

        if (!is_scalar()) {
            LOG_RUNTIME("Convert tensor to rational support for scalar only!");
        }
        if (is_integral()) {
            m_rational = GetValueAsInteger();
            m_var = std::monostate();
        } else {
            LOG_RUNTIME("Convert value '%s' to rational not implemented!", toString().c_str());
        }
        m_var_type_current = type;
        return;

    } else if (is_tensor_type() && isDictionary(type)) {

        if (is_scalar() && is_integral()) {
            ASSERT(std::holds_alternative<int64_t>(m_var));
            push_back(Obj::CreateValue(std::get<int64_t>(m_var)));
            m_var = std::monostate();
        } else if (is_scalar() && is_floating()) {
            ASSERT(std::holds_alternative<double>(m_var));
            push_back(Obj::CreateValue(std::get<double>(m_var)));
            m_var = std::monostate();
        } else {
            ASSERT(!is_scalar());
            ASSERT(m_tensor);
            if (!m_tensor->defined()) {
                ASSERT(m_tensor->defined());
            }
            ConvertTensorToDict(*m_tensor, *this);
            m_tensor->reset();
        }
        m_var_type_current = type;
        return;

    } else if (is_dictionary_type() && isTensor(type)) {

        ASSERT(!m_tensor->defined());

        if (isGenericType(type)) {
            m_var_type_fixed = type;
        }

        if (!size()) {
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
        at::ScalarType summary_torch_type = toTorchType(static_cast<ObjType> (summary_type));
        if (at(0).second->is_scalar()) {
            if (at(0).second->is_integral()) {
                *m_tensor = torch::full({1}, at(0).second->GetValueAsInteger(), summary_torch_type);
            } else {
                if (!at(0).second->is_floating()) {
                    ASSERT(at(0).second->is_floating());
                }
                *m_tensor = torch::full({1}, at(0).second->GetValueAsNumber(), summary_torch_type);
            }
        } else {
            *m_tensor = at(0).second->m_tensor->toType(summary_torch_type);
        }

        for (int i = 1; i < size(); i++) {

            ObjPtr temp = at(i).second;
            if (!temp) {
                LOG_RUNTIME("Fail convert nullptr to tensor at index %d!", i);
            }

            torch::Tensor temp_tensor;
            if (temp->is_scalar()) {
                ASSERT(!temp->m_tensor->defined());
                if (temp->is_integral()) {
                    temp_tensor = torch::full({1}, temp->GetValueAsInteger(), summary_torch_type);
                } else {
                    if (!temp->is_floating()) {
                        ASSERT(temp->is_floating());
                    }
                    temp_tensor = torch::full({1}, temp->GetValueAsNumber(), summary_torch_type);
                }
            } else {
                ASSERT(temp->m_tensor->defined());
                temp_tensor = temp->m_tensor->toType(summary_torch_type);
            }

            *m_tensor = torch::cat({*m_tensor, temp_tensor});
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
 * :type_int := :Int32; # Синоним типа Int32 во время компиляции (тип не может быть изменен)
 * :type_int := :Int32(); # Копия типа Int32 во время выполнения (тип может быть изменен после Mutable)
 * var_type := :Int32; # Тип в переменной, которую можно передавать как аргумент в функции
 * 
 * 
 * ЗНАЧЕНИЯ УКАЗАННЫХ ТИПОВ  (при наличии аргументов)
 * 
 * scalar_int := :Int32(0); # Преобразование типа во время выполнения с автоматической размерностью (скаляр)
 * scalar_int := :Int32[0](0); # Преобразование типа во время выполнения с указанием размерности (скаляр)
 * scalar_int := :Int32[0]([0,]); # Преобразование типа во время выполнения с указанием размерности (скаляр)
 * 
 * tensor_int := :Int32([0,]); # Преобразование типа во время выполнения с автоматической размерностью (тензор)
 * tensor_int := :Int32[1](0); # Преобразование типа во время выполнения с указанием размерности (тензор)
 * tensor_int := :Int32[...](0); # Преобразование типа во время выполнения с произвольной размернотью (тензор)
 */

ObjPtr Obj::CreateBaseType(ObjType type) {

    ObjPtr result = std::make_shared<Obj>(ObjType::Type);
    result->m_class_name = newlang::toString(type);
    result->m_var_type_fixed = type;

    std::string func_proto(result->m_class_name);
    func_proto += "(...)";
    func_proto += result->m_class_name;
    func_proto += ":-{ }";
    result->m_class_name = newlang::NormalizeName(newlang::toString(type));

//    TermPtr proto = Parser::ParseString(func_proto, nullptr);
//    ASSERT(proto->Left());
//    * const_cast<TermPtr *> (&result->m_prototype) = proto->Left();

    result->m_var = (void *) BaseTypeConstructor;
    result->m_is_const = true;

    return result;
}

ObjPtr Obj::BaseTypeConstructor(Context *ctx, Obj & args) {

    if (args.empty() || !args[0].second) {
        LOG_RUNTIME("Self simple type not defined!");
    }
    ASSERT(args[0].second->getType() == ObjType::Type);

    ObjPtr result = nullptr;
    if (isArithmeticType(args[0].second->m_var_type_fixed)) {
        result = ConstructorSimpleType_(ctx, args);
    } else if (args[0].second->m_var_type_fixed == ObjType::Dictionary) {
        result = ConstructorDictionary_(ctx, args);
    } else if (args[0].second->m_var_type_fixed == ObjType::Pointer && args.size() > 1) {
        result = ConstructorNative_(ctx, args);
    } else if (args[0].second->m_var_type_fixed == ObjType::Class) {
        result = ConstructorClass_(ctx, args);
    } else if (args[0].second->m_var_type_fixed == ObjType::Struct || args[0].second->m_var_type_fixed == ObjType::Union) {
        result = ConstructorStruct_(ctx, args);
    } else if (args[0].second->m_var_type_fixed == ObjType::Enum) {
        result = ConstructorEnum_(ctx, args);
        //    } else if (args[0].second->m_var_type_fixed == ObjType::Return) {
        //        result = ConstructorReturn_(ctx, args);
        //    } else if (args[0].second->m_var_type_fixed == ObjType::Thread) {
        //        result = ConstructorThread_(ctx, args);
        //    } else if (args[0].second->m_var_type_fixed == ObjType::Sys) {
        //        result = ConstructorSystem_(ctx, args);
        //    } else if (args[0].second->m_var_type_fixed == ObjType::Break || args[0].second->m_var_type_fixed == ObjType::Continue) {
        //        result = ConstructorInterraption_(ctx, args, args[0].second->m_var_type_fixed);
        //    } else if (args[0].second->m_var_type_fixed == ObjType::Error || args[0].second->m_var_type_fixed == ObjType::ErrorParser
        //            || args[0].second->m_var_type_fixed == ObjType::ErrorRunTime || args[0].second->m_var_type_fixed == ObjType::ErrorSignal) {
        //        result = ConstructorError_(ctx, args);
    } else if (args[0].second->m_var_type_fixed == ObjType::StrChar && args.size() > 1) {
        result = Obj::CreateString("");
        for (int i = 1; i < args.size(); i++) {
            result->op_concat_(args[i].second, ConcatMode::Append);
        }
    } else if (args[0].second->m_var_type_fixed == ObjType::StrWide && args.size() > 1) {
        result = Obj::CreateString(L"");
        for (int i = 1; i < args.size(); i++) {
            result->op_concat_(args[i].second, ConcatMode::Append);
        }
    } else if (args.size() == 1) { // Клонировать тип
        result = args[0].second->Clone();
        if (result) {
            result->m_is_const = false;
        }
    }

    if (!result) {
        LOG_RUNTIME("Create type '%s' error or not implemented!", newlang::toString(args[0].second->m_var_type_fixed));
    }

    //    result->m_class_name = args[0].second->m_class_name;

    return result;
}

ObjPtr Obj::ConstructorSimpleType_(Context *ctx, Obj & args) {

    ASSERT(!args.empty() && args[0].second);
    ASSERT(args[0].second->getType() == ObjType::Type);

    // Переданы значения для приведения типов
    // Но само значение пока не установлено
    ObjPtr result = args[0].second->Clone();
    if (args.size() == 1) {
        // Копия существующего типа с возможностью редактирования
        result->m_is_const = false;
        return result;
    }

    //    std::vector<int64_t> dims;
    //
    //    bool dim_auto = false;
    //    if (result->m_dimensions) {
    //        // Размерность указана
    //        for (int i = 0; i < result->m_dimensions->size(); i++) {
    //            ObjPtr ind = result->m_dimensions->at(i).second;
    //            ASSERT(ind);
    //            if (ind->GetType())->isind.is_integer()) {
    //                dims.push_back(ind.integer());
    //            } else if (ind.is_boolean()) {
    //                dims.push_back(ind.boolean());
    //            } else if (ind.is_none()) {
    //
    //            } else if (isEllipsis(ind->GetType())) {
    //                if (dim_auto) {
    //                    LOG_RUNTIME("Only one dimension of any size is possible!");
    //                }
    //                dim_auto = true;
    //            } else {
    //                LOG_RUNTIME("Non fixed dimension not implemented!");
    //            }
    //        }
    //    }

    ObjPtr first_dim = result->m_dimensions && result->m_dimensions->size() ? result->m_dimensions->at(0).second : nullptr;

    bool to_scalar = false;

    if (!first_dim || (first_dim->is_integral() && first_dim->GetValueAsInteger() == 0)) {
        if (result->m_var_type_fixed == ObjType::Bool) {
            bool value = false;
            if (args.size() > 2) {
                value = true;
            } else {
                value = args[1].second->GetValueAsBoolean();
            }
            result->m_var_type_current = result->m_var_type_fixed;
            result->m_var = static_cast<int64_t> (value);
            result->m_var_is_init = true;
            result->m_tensor->reset();
            result->m_dimensions = nullptr;
            return result;
        }
        to_scalar = true;
    }


    if (args.size() == 2) {
        // Передано единственное значение (нулевой аргумент - сам объект, т.е. :Тип(Значение) )

        ObjPtr convert;

        // Если обобщенный тип данных, а сами данные принадлежат обощенному типу
        if (isGenericType(result->m_var_type_fixed) && isContainsType(result->m_var_type_fixed, args[1].second->getType())) {
            convert = args[1].second->Clone();
        } else {
            convert = args[1].second->toType(result->m_var_type_fixed);
        }
        convert->m_var_type_fixed = result->m_var_type_fixed;
        convert.swap(result);
        convert->m_dimensions.swap(result->m_dimensions);

    } else {

        // Для списка значений сперва формируется словарь, а после он конвертируется в нужный тип данных

        result->m_var_type_current = ObjType::Dictionary;

        ObjPtr prev = nullptr;
        for (int i = 1; i < args.size(); i++) {

            if (args[i].second->getType() == ObjType::Ellipsis) {
                if (!prev) {
                    LOG_RUNTIME("There is no previous item to repeat!");
                }
                if (i + 1 != args.size()) {
                    LOG_RUNTIME("Ellipsis is not the last element!");
                }
                if (!result->m_dimensions || !result->m_dimensions->size()) {
                    LOG_RUNTIME("Object has no dimensions!");
                }
                int64_t full_size = 1;
                for (int j = 0; j < result->m_dimensions->size(); j++) {
                    if (!result->m_dimensions->at(i).second->is_integral()) {
                        LOG_RUNTIME("Type '%s' error for dimensions!", newlang::toString(result->m_dimensions->at(i).second->getType()));
                    }
                    full_size *= result->m_dimensions->at(i).second->GetValueAsInteger();
                }
                if (full_size <= 0) {
                    LOG_RUNTIME("Items count error for all dimensions!");
                }

                for (int64_t j = result->size(); j < full_size; j++) {
                    result->op_concat_(prev, ConcatMode::Append);
                }

                break;

            } else {
                prev = args[i].second;
            }
            prev->m_dimensions = result->m_dimensions;
            result->op_concat_(prev, ConcatMode::Append);
        }

        if (args[0].second->m_var_type_fixed != ObjType::Dictionary) {
            result = result->toType(args[0].second->m_var_type_fixed);
            result->m_var_type_fixed = result->m_var_type_current;
        }
    }


    if (to_scalar) {
        // To Scalar
        if (result->is_scalar()) {
            // All ok
        } else if (result->size() == 1) {

            if (result->is_arithmetic_type()) {
                if (result->is_integral()) {
                    result->m_var = static_cast<int64_t> (result->at(0).second->GetValueAsInteger());
                } else if (result->is_floating()) {
                    result->m_var = static_cast<double> (result->at(0).second->GetValueAsNumber());
                } else {
                    LOG_RUNTIME("Convert type '%s' to scalar not implemented!", newlang::toString(result->getType()));
                }
            } else {
                LOG_RUNTIME("Convert type '%s' to scalar not implemented!", newlang::toString(result->getType()));
            }

            if (result->is_dictionary_type() && result->size()) {
                result->erase(0);
            }
            result->m_dimensions.reset();
            result->m_tensor->reset();

        } else {
            LOG_RUNTIME("Conversion to scalar is not possible!");
        }

        //        result->m_var_type_fixed = result->getType();

    } else {
        ASSERT(first_dim);
        if (isString(result->getType()) || isDictionary(result->getType())) {
            if (result->m_dimensions->size() != 1) {
                LOG_RUNTIME("Fail dimension size for type '%s'!", newlang::toString(result->getType()));
            }
            if (!first_dim->is_any_size()) {
                result->resize_(first_dim->GetValueAsInteger(), nullptr);
            }
        } else if (isTensor(result->getType())) {

            if (result->m_dimensions->size() != 1 || !result->m_dimensions->at(0).second->is_any_size()) {
                std::vector<int64_t> dims;
                for (int i = 0; i < result->m_dimensions->size(); i++) {
                    Index ind = toIndex(*(*result->m_dimensions)[i].second);
                    if (ind.is_integer()) {
                        dims.push_back(ind.integer().expect_int());
                    } else if (ind.is_boolean()) {
                        dims.push_back(ind.boolean());
                    } else {
                        LOG_RUNTIME("Resize dimension '%s' not implemented!", newlang::toString(result->m_dimensions->at(i).second->getType()));
                    }
                }
                *result->m_tensor = result->m_tensor->reshape(dims);
            }

        } else {
            LOG_RUNTIME("Fail use dimensions for type '%s'!", newlang::toString(result->getType()));
        }
    }
    return result;
}

/*
 * :Class(One=0, Two=_, Three=3); # Все аргументы имеют имена
 */

ObjPtr Obj::ConstructorDictionary_(Context *ctx, Obj & args) {

    ASSERT(!args.empty() && args[0].second);
    ASSERT(args[0].second->getType() == ObjType::Type);

    ObjPtr result = Obj::CreateDict();
    for (int i = 1; i < args.size(); i++) {
        result->push_back(args[i].second, args.name(i));
    }
    result->m_var_is_init = true;

    return result;
}

ObjPtr Obj::ConstructorNative_(Context *ctx_const, Obj & args) {
    //    if (args.size() < 2) {
    LOG_RUNTIME("Empty argument list!");
    //    }
    //    if (!args.at(1).second->is_string_type()) {
    //        LOG_RUNTIME("First argument not a string!");
    //    }
    //    //@todo Передача дополнительных аргументов? args["module"]->GetValueAsString().c_str(), args["lazzy"]->GetValueAsBoolean()
    //    Context *ctx = const_cast<Context *> (ctx_const);
    //    return ctx->m_runtime->CreateNative(args.at(1).second->GetValueAsString().c_str());
}

ObjPtr Obj::ConstructorStub_(Context *ctx, Obj & args) {
    return Obj::CreateClass(":Class");
}

/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
 * Различие между классом и словарям в том, что элементы словаря могут добавлятся и удаляться динамически,
 * а у класса состав полей фиуксируется при определении и в последствии они не могут быть добалвены или удалены.
 * Это нужно для возможности работы синтаксического анализатора на этапе компиляции программы.
 * >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 * 
 * Данный конструктор используется для создания классов литералов без дополнительных 
 * вызовов базовых классов и/или дектрукторов
 *
 */
ObjPtr Obj::ConstructorClass_(Context *ctx, Obj & args) {

    //    bool is_check = false;
    //    ObjPtr result = nullptr;
    //    ObjPtr constructor = nullptr;
    //    if (args.size() && !args.at(0).first.empty() && args.at(0).second) {
    //        //LOG_DEBUG("'%s' %s", args.at(0).first.c_str(), args.at(0).second->toString().c_str());
    //        result = args.at(0).second;
    //        is_check = true;
    //
    //        std::string name = MakeConstructorName(args.at(0).second->m_class_name);
    //        constructor = const_cast<Context *> (ctx)->FindTerm(name);
    //
    //    } else {
    //        result = Obj::CreateType(ObjType::Class, ObjType::Class, true);
    //
    //        ASSERT(args.size() == 0);
    //        args.push_back(result);
    //    }
    //
    //    for (int i = 1; i < args.size(); i++) {
    //        if (args.name(i).empty()) {
    //            LOG_RUNTIME("Field pos %d has no name!", i);
    //        }
    //        for (int pos = 0; pos < i; pos++) {
    //            if (args.name(pos).compare(args.name(i)) == 0) {
    //                LOG_RUNTIME("Field name '%s' at index %d already exists!", args.name(i).c_str(), i);
    //            }
    //        }
    //        if (result->find(args.name(i)) != result->end()) {
    //            result->find(args.name(i))->second->SetValue_(args.at(i).second);
    //        } else if (is_check) {
    //            LOG_RUNTIME("Property '%s' not found!", args.name(i).c_str());
    //        } else {
    //            result->push_back(args.at(i).second, args.name(i));
    //        }
    //    }
    //
    //    if (constructor) {
    //        //        result = constructor->Call(const_cast<Context *> (ctx), &args, true, result);
    //    }
    ASSERT(0);
    return nullptr;
}

ObjPtr Obj::ConstructorStruct_(Context *ctx, Obj & args) {
    ObjPtr result = ConstructorClass_(ctx, args);
    result->m_var_type_fixed = ObjType::Struct;

    if (!result->size()) {
        LOG_RUNTIME("Empty Struct not allowed!");
    }

    for (int i = 0; i < result->size(); i++) {
        if (!(*result)[i].second) {
            LOG_RUNTIME("Field '%s' at pos %d not defined!", result->name(i).c_str(), i);
        }
        if (!(*result)[i].second || !isSimpleType((*result)[i].second->getType()) || isGenericType((*result)[i].second->getType())) {

            LOG_RUNTIME("Field '%s' at pos %d not simple type! (%s)", result->name(i).c_str(), i, newlang::toString((*result)[i].second->getType()));
        }
    }
    return result;
}

/*
 * :Enum(One=0, Two=_, "Three", Ten=10);
 */

ObjPtr Obj::ConstructorEnum_(Context *ctx, Obj & args) {
    ObjPtr result = Obj::CreateDict();
    result->m_var_type_fixed = ObjType::Enum;

    int64_t val_int = 0;
    ObjPtr enum_value;
    std::string enum_name;

    for (int i = 1; i < args.size(); i++) {
        if (args.name(i).empty()) {
            if (args[i].second && args[i].second->is_string_type()) {
                enum_name = args[i].second->GetValueAsString();
            } else {
                LOG_RUNTIME("Field pos %d has no name!", i);
            }
        } else {
            enum_name = args.name(i);

            if (args[i].second && (args[i].second->is_integral())) {
                val_int = args[i].second->GetValueAsInteger();
            } else if (!args[i].second || !args[i].second->is_none_type()) {
                LOG_RUNTIME("Field value '%s' %d must integer type!", args.name(i).c_str(), i);
            }
        }

        if (result->find(enum_name) != result->end()) {
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

//ObjPtr Obj::ConstructorError_(Context *ctx, Obj & args) {
//    ObjPtr result = ConstructorDictionary_(ctx, args);
//    result->m_var_type_current = ObjType::Error;
//    result->m_var_type_fixed = ObjType::Error;
//    return result;
//}

//ObjPtr Obj::ConstructorReturn_(Context *ctx, Obj & args) {
//    ObjPtr result = ConstructorDictionary_(ctx, args);
//    result->m_var_type_current = ObjType::Return;
//    result->m_var_type_fixed = ObjType::Return;
//    if (result->size() == 0) {
//        result->push_back(Obj::Arg(Obj::CreateNone()));
//    }
//    if (result->size() != 1) {
//
//        LOG_RUNTIME("Multiple argument for type ':Return'!");
//    }
//    return result;
//}
//
//ObjPtr Obj::ConstructorThread_(Context *ctx, Obj & args) {
//    if (args.size() == 0) {
//        LOG_RUNTIME("Function for thread not defined!");
//    }
//
//    ObjPtr result = ConstructorDictionary_(ctx, args);
//    result->m_var_type_current = ObjType::Thread;
//    result->m_var_type_fixed = ObjType::Thread;
//
//
//    return result;
//}
//
//ObjPtr Obj::ConstructorSystem_(Context *ctx, Obj & args) {
//    //    if (args.size() == 0) {
//    LOG_RUNTIME("Function for System not defined!");
//    //    }
//
//    ObjPtr result = ConstructorDictionary_(ctx, args);
//    result->m_var_type_current = ObjType::Thread;
//    result->m_var_type_fixed = ObjType::Thread;
//
//
//    return result;
//}
//
//ObjPtr Obj::ConstructorInterraption_(Context* ctx, Obj& args, ObjType type) {
//    ObjPtr result = ConstructorDictionary_(ctx, args);
//    result->m_var_type_current = type;
//    result->m_var_type_fixed = type;
//    if (result->size()) {
//        LOG_RUNTIME("Argument for type %s not allowed!", newlang::toString(type));
//    }
//    return result;
//}

template<>
const Iterator<Obj>::IterPairType Iterator<Obj>::m_Iterator_end = IterObj::pair(Obj::CreateType(ObjType::IteratorEnd, ObjType::IteratorEnd, true));
static const ObjPtr zero = Obj::CreateValue(0);

template<>
ObjPtr Iterator<Obj>::read_and_next(int64_t count) {
    ObjPtr result;

    if (count == 0) {
        if (m_iter_obj->m_var_type_current == ObjType::Range) {

            ASSERT(m_iter_obj->m_iter_range_value);

            ObjPtr value = m_iter_obj->m_iter_range_value; //->Clone();
            ObjPtr stop = m_iter_obj->at("stop").second;
            ObjPtr step = m_iter_obj->at("step").second;

            ASSERT(value);
            ASSERT(stop);
            ASSERT(step);

            int up_direction = step->op_compare(*zero);
            ASSERT(up_direction);

            if (up_direction < 0) {
                if (value->op_compare(*stop) > 0) {
                    result = value->Clone();
                    (*value) += step;
                } else {
                    result = Obj::CreateType(ObjType::IteratorEnd, ObjType::IteratorEnd, true);
                }
            } else {
                if (value->op_compare(*stop) < 0) {
                    result = value->Clone();
                    (*value) += step;
                } else {
                    result = Obj::CreateType(ObjType::IteratorEnd, ObjType::IteratorEnd, true);
                }
            }
            //            m_iter_obj->m_iter_range_value = value;

        } else if (m_iter_obj->is_indexing()) {
            result = (*(*this)).second;
            (*this)++;
        } else {
            LOG_RUNTIME("Iterator to type %s not implemented!", newlang::toString(m_iter_obj->m_var_type_current));
        }
    } else if (count < 0) {

        result = Obj::CreateDict();
        result->m_var_is_init = true;

        if (m_iter_obj->m_var_type_current == ObjType::Range) {

            ASSERT(m_iter_obj->m_iter_range_value);

            ObjPtr value = m_iter_obj->m_iter_range_value; //->Clone();
            ObjPtr stop = m_iter_obj->at("stop").second;
            ObjPtr step = m_iter_obj->at("step").second;

            ASSERT(value);
            ASSERT(stop);
            ASSERT(step);

            int up_direction = step->op_compare(*zero);
            ASSERT(up_direction);

            bool next_value = true;
            for (int64_t i = 0; i < -count; i++) {

                if (next_value) {
                    if (up_direction < 0) {
                        if (value->op_compare(*stop) < 0) {
                            next_value = false;
                        } else {
                            result->push_back(value->Clone());
                            (*value) += step;
                        }
                    } else {
                        if (value->op_compare(*stop) > 0) {
                            next_value = false;
                        } else {
                            result->push_back(value->Clone());
                            (*value) += step;
                        }
                    }
                } else {
                    result->push_back(Obj::CreateType(ObjType::IteratorEnd, ObjType::IteratorEnd, true));
                }
            }
            //m_iter_obj->m_iter_range_value = value;

        } else if (m_iter_obj->is_indexing()) {

            for (int64_t i = 0; i < -count; i++) {
                if (*this != this->end()) {
                    result->push_back(*(*this));
                    (*this)++;
                } else {
                    result->push_back(Obj::CreateType(ObjType::IteratorEnd, ObjType::IteratorEnd, true));
                }
            }

        } else {
            LOG_RUNTIME("Iterator to type %s not implemented!", newlang::toString(m_iter_obj->m_var_type_current));
        }

    } else {

        result = Obj::CreateDict();
        result->m_var_is_init = true;

        if (m_iter_obj->m_var_type_current == ObjType::Range) {

            ASSERT(m_iter_obj->m_iter_range_value);

            ObjPtr value = m_iter_obj->m_iter_range_value; //->Clone();
            ObjPtr stop = m_iter_obj->at("stop").second;
            ObjPtr step = m_iter_obj->at("step").second;

            ASSERT(value);
            ASSERT(stop);
            ASSERT(step);

            int up_direction = step->op_compare(*zero);
            ASSERT(up_direction);

            if (up_direction < 0) {
                while (value->op_compare(*stop) > 0) {
                    result->push_back(value->Clone());
                    (*value) += step;
                }
            } else {
                while (value->op_compare(*stop) < 0) {
                    result->push_back(value->Clone());
                    (*value) += step;
                }
            }
            //            m_iter_obj->m_iter_range_value.swap(value);

        } else if (m_iter_obj->is_indexing()) {
            while (*this != this->end() && result->size() < count) {
                result->push_back(*(*this));
                (*this)++;
            }
        } else {
            LOG_RUNTIME("Iterator to type %s not implemented!", newlang::toString(m_iter_obj->m_var_type_current));
        }
    }

    return result;
}

ObjPtr Obj::IteratorMake(const char * filter, bool check_create) {
    ObjPtr result = CreateType(ObjType::Iterator, ObjType::Iterator, true);
    if (!(is_indexing() || m_var_type_current == ObjType::Range)) {
        if (getType() == ObjType::Iterator && !check_create) {
            return shared();
        }
        LOG_RUNTIME("Can't create iterator from '%s'!", toString().c_str());
    }
    result->m_iterator = std::make_shared<Iterator < Obj >> (shared(), filter);
    ASSERT(result->m_iterator->m_iter_obj.get() == this);
    ASSERT(!result->m_iterator->m_iter_obj->m_iter_range_value);
    result->IteratorReset();
    if (is_range()) {
        ASSERT(result->m_iterator->m_iter_obj->m_iter_range_value);
    }
    return result;
}

ObjPtr Obj::IteratorMake(Obj * args) {
    ObjPtr result = CreateType(ObjType::Iterator, ObjType::Iterator, true);
    if (!(is_indexing() || m_var_type_current == ObjType::Range)) {
        //        if(getType() == ObjType::Iterator) {
        //            return shared();
        //        }
        LOG_RUNTIME("Can't create iterator from '%s'!", toString().c_str());
    }

    if (!args || args->size() == 0) {
        result->m_iterator = std::make_shared<Iterator < Obj >> (shared());
    } else if (args->size() == 1 && args->at(0).second && args->at(0).second->is_string_type()) {
        result->m_iterator = std::make_shared<Iterator < Obj >> (shared(), args->GetValueAsString().c_str());
    } else if (args->size() >= 1 && args->at(0).second && args->at(0).second->is_function_type()) {
        ASSERT(false);
        //        ObjPtr func = args->at(0).second;
        //        ObjPtr func_arg = args->Clone();
        //        func_arg->resize_(-(func_arg->size() - 1)); // Удалить первый элемент
        //        result->m_iterator = std::make_shared<Iterator < Obj >> (shared(), func.get(), func_arg.get());
    } else {
        LOG_RUNTIME("Invalid arguments for iterator create! '%s'", args->toString().c_str());
    }
    ASSERT(result->m_iterator->m_iter_obj.get() == this);
    ASSERT(!result->m_iterator->m_iter_obj->m_iter_range_value);
    result->IteratorReset();
    if (is_range()) {
        ASSERT(result->m_iterator->m_iter_obj->m_iter_range_value);
    }
    return result;
}

ObjPtr Obj::IteratorData() {
    if (!(m_var_type_current == ObjType::Iterator || m_var_type_current == ObjType::IteratorEnd)) {
        LOG_RUNTIME("Object '%s' not iterator!", toString().c_str());
    }

    if (m_var_type_current == ObjType::IteratorEnd) {
        return Iterator<Obj>::m_Iterator_end.second;
    }

    ASSERT(m_iterator);
    ASSERT(m_iterator->m_iter_obj);

    if (m_iterator->m_iter_obj->is_range()) {

        ASSERT(m_iterator->m_iter_obj->m_iter_range_value);

        ObjPtr value = m_iterator->m_iter_obj->m_iter_range_value; //->Clone();
        ObjPtr stop = m_iterator->m_iter_obj->at("stop").second;
        ObjPtr step = m_iterator->m_iter_obj->at("step").second;

        ASSERT(value);
        ASSERT(stop);
        ASSERT(step);

        int up_direction = step->op_compare(*zero);
        ASSERT(up_direction);

        if (up_direction < 0) {
            if (value->op_compare(*stop) > 0) {
                return value->Clone();
            }
        } else {
            if (value->op_compare(*stop) < 0) {
                return value->Clone();
            }
        }

        return Iterator<Obj>::m_Iterator_end.second;

    } else if (m_iterator->m_iter_obj->is_indexing()) {
        return m_iterator->data().second;
    }
    LOG_RUNTIME("IteratorData not implemented for object type %s!", newlang::toString(m_iterator->m_iter_obj->m_var_type_current));
}

ObjPtr Obj::IteratorReset() {
    if (m_var_type_current != ObjType::Iterator) {
        LOG_RUNTIME("Method available an iterator only!");
    }
    ASSERT(m_iterator);

    if (m_iterator->m_iter_obj->is_range()) {
        ObjType summary_type = getSummaryTensorType(m_iterator->m_iter_obj.get(), ObjType::None);
        m_iterator->m_iter_obj->m_iter_range_value = m_iterator->m_iter_obj->at("start").second->toType(summary_type);
    } else if (m_iterator->m_iter_obj->is_indexing()) {
        m_iterator->reset();
    } else {
        LOG_RUNTIME("IteratorReset not implemented for object type %s!", newlang::toString(m_iterator->m_iter_obj->m_var_type_current));
    }
    return shared();
}

ObjPtr Obj::IteratorNext(int64_t count) {
    if (m_var_type_current != ObjType::Iterator) {
        LOG_RUNTIME("Method available an iterator only!");
    }
    ASSERT(m_iterator);
    return m_iterator->read_and_next(count);
}

ObjPtr newlang::CheckSystemField(const Obj *obj, std::string name) {
    /*
        Встроенные атрибуты у каждого объекта
     */

    static const char * SYS__NAME__ = "__name__";
    static const char * SYS__FULL_NAME__ = "__full_name__";
    static const char * SYS__TYPE__ = "__type__";
    static const char * SYS__TYPE_FIXED__ = "__type_fixed__";
    static const char * SYS__MOULE__ = "__module__";
    static const char * SYS__CLASS__ = "__class__";
    static const char * SYS__BASE__ = "__base__";
    static const char * SYS__SIZE__ = "__size__";

    static const char * SYS__DOC__ = "__doc__";
    static const char * SYS__STR__ = "__str__";
    static const char * SYS__SOURCE__ = "__source__";

    static const char * MODULE__MD5__ = "__md5__";
    static const char * MODULE__FILE__ = "__file__";
    static const char * MODULE__TIMESTAMP__ = "__timestamp__";
    static const char * MODULE__MAIN__ = "__main__";
    static const char * MODULE__VERSION__ = "__version__";

    if (!isSystemName(name)) {
        return nullptr;
    } else if (!obj || !obj->is_init()) {
        return Obj::CreateString(":Undefined");
    } else if (name.compare(SYS__FULL_NAME__) == 0) {
        return Obj::CreateString(obj->m_var_name);
    } else if (name.compare(SYS__NAME__) == 0) {
        return Obj::CreateString(ExtractName(obj->m_var_name));
    } else if (name.compare(SYS__MOULE__) == 0) {
        return Obj::CreateString(ExtractModuleName(obj->m_var_name.c_str()));
    } else if (name.compare(SYS__TYPE__) == 0) {
        return Obj::CreateString(newlang::toString(obj->m_var_type_current));
    } else if (name.compare(SYS__TYPE_FIXED__) == 0) {
        return Obj::CreateString(newlang::toString(obj->m_var_type_fixed));
    } else if (name.compare(SYS__CLASS__) == 0) {
        return Obj::CreateString(obj->m_class_name);
    } else if (name.compare(SYS__BASE__) == 0) {
        return Obj::CreateDict(obj->m_class_parents);
    } else if (name.compare(SYS__MOULE__) == 0) {
        return Obj::CreateString(obj->m_var_name);
    } else if (name.compare(SYS__SIZE__) == 0) {
        return Obj::CreateValue(obj->size());
//    } else if (name.compare(SYS__DOC__) == 0) {
//        return Obj::CreateString(GetDoc(obj->m_var_name));
    } else if (name.compare(SYS__STR__) == 0) {
        return Obj::CreateString(obj->toString());
    } else if (name.compare(SYS__SOURCE__) == 0 && obj->m_var_type_current != ObjType::Module) {
        // @todo Тут должен возвращаться код для создания аналогичного объекта
        return Obj::CreateString(obj->toString());
    } else {

        if (obj->m_var_type_current == ObjType::Module) {
            //            const Module *mod = static_cast<const Module *> (obj);
            //
            //            if (name.compare(MODULE__MD5__) == 0) {
            //                return Obj::CreateString(mod->m_md5);
            //            } else if (name.compare(MODULE__FILE__) == 0) {
            //                return Obj::CreateString(mod->m_file);
            //            } else if (name.compare(MODULE__TIMESTAMP__) == 0) {
            //                return Obj::CreateString(mod->m_timestamp);
            //            } else if (name.compare(MODULE__VERSION__) == 0) {
            //                return Obj::CreateString(mod->m_version);
            //            } else if (name.compare(MODULE__MAIN__) == 0) {
            //                return Obj::CreateBool(mod->m_is_main);
            //            } else if (name.compare(SYS__SOURCE__) == 0) {
            //                return Obj::CreateString(mod->m_source);
            //            }
        }

        std::string message("Internal field '");
        message += name;
        message += "' not exist!";
        return Obj::CreateString(message);
    }
    return nullptr;
}

ObjPtr Obj::op_call(Obj & args) {

//    return Context::Call(m_ctx, *this, args);
    return nullptr;
}

void Obj::testResultIntegralType(ObjType type, bool upscalint) {
    ObjType new_type = m_var_type_current;

    if (((is_integral() && isIntegralType(type, true)) || (is_floating() && isFloatingType(type)))
            && static_cast<uint8_t> (type) <= static_cast<uint8_t> (m_var_type_current)) {
        // type already including to current type

    } else {

        if (!canCast(type, m_var_type_current)) {
            testConvertType(type);
            new_type = type;
        }
        bool check = false;
        if (upscalint && isIntegralType(m_var_type_current, true) && isIntegralType(type, true)) {
            if (type < ObjType::Int64) {
                new_type = static_cast<ObjType> (static_cast<uint8_t> (type) + 1);
                check = true;
            }
            if (check && m_var_type_fixed != ObjType::None && !canCast(new_type, m_var_type_fixed)) {
                new_type = type; // Тип данных менять нельзя, но сама операция возможна
                LOG_WARNING("Data type '%s' cannot be changed to '%s', loss of precision is possible!", newlang::toString(type), newlang::toString(m_var_type_fixed));
            }
        }
    }
    if (new_type != m_var_type_current) {
        if (is_scalar()) {
            if (isFloatingType(new_type) && isIntegralType(m_var_type_current, true)) {
                // Для скаляров повышение типа с целочисленного на число с плавающий точкой
                m_var = static_cast<double> (GetValueAsInteger());
            } else {
                // Измерение размерности, а не типа - ничего делать ненужно
                ASSERT((isFloatingType(new_type) && isFloatingType(m_var_type_current)) ||
                        (isIntegralType(new_type, true) && isIntegralType(m_var_type_current, true)));
            }
        } else {
            ASSERT(m_tensor);
            ASSERT(m_tensor->defined());
            *m_tensor = m_tensor->toType(toTorchType(new_type));
        }
        m_var_type_current = new_type;
    }
}

ObjType GetTensorType(torch::Tensor & val) {
    switch (val.dtype().toScalarType()) {
        case at::ScalarType::Bool:
            return ObjType::Bool;
        case at::ScalarType::Half:
        case at::ScalarType::BFloat16:
            return ObjType::Float16;
        case at::ScalarType::Float:
            return ObjType::Float32;
        case at::ScalarType::Double:
            return ObjType::Float64;
        case at::ScalarType::Byte:
        case at::ScalarType::Char:
        case at::ScalarType::QInt8:
        case at::ScalarType::QUInt8:
        case at::ScalarType::QUInt4x2:
            return ObjType::Int8;
        case at::ScalarType::Short:
            return ObjType::Int16;
        case at::ScalarType::Int:
        case at::ScalarType::QInt32:
            return ObjType::Int32;
        case at::ScalarType::Long:
            return ObjType::Int64;
        case at::ScalarType::ComplexHalf:
            return ObjType::Complex16;
        case at::ScalarType::ComplexFloat:
            return ObjType::Complex32;
        case at::ScalarType::ComplexDouble:
            return ObjType::Complex64;
    }
    LOG_RUNTIME("Fail tensor type %s", val.toString().c_str());
}

/*
 * From TensorIndexing.h
// There is one-to-one correspondence between Python and C++ tensor index types:
// Python                  | C++
// -----------------------------------------------------
// `None`                  | `at::indexing::None`
// `Ellipsis`              | `at::indexing::Ellipsis`
// `...`                   | `"..."`
// `123`                   | `123`
// `True` / `False`        | `true` / `false`
// `:`                     | `Slice()` / `Slice(None, None)`
// `::`                    | `Slice()` / `Slice(None, None, None)`
// `1:`                    | `Slice(1, None)`
// `1::`                   | `Slice(1, None, None)`
// `:3`                    | `Slice(None, 3)`
// `:3:`                   | `Slice(None, 3, None)`
// `::2`                   | `Slice(None, None, 2)`
// `1:3`                   | `Slice(1, 3)`
// `1::2`                  | `Slice(1, None, 2)`
// `:3:2`                  | `Slice(None, 3, 2)`
// `1:3:2`                 | `Slice(1, 3, 2)`
// `torch.tensor([1, 2])`) | `torch::tensor({1, 2})`
 */
Index toIndex(Obj &obj) {
    if (obj.is_none_type()) {
        return Index(c10::nullopt);
    } else if (obj.is_dictionary_type()) {
        std::vector<int64_t> temp = obj.toIntVector(true);
        if (temp.size()) {
            torch::Tensor tensor = torch::from_blob(temp.data(), temp.size(), torch::Dtype::Long);
            return Index(tensor.clone());
        } else {
            return Index(c10::nullopt);
        }

    } else if (obj.is_scalar()) {
        switch (obj.m_var_type_current) {
            case ObjType::Bool:
                return Index(obj.GetValueAsBoolean());

            case ObjType::Int8:
            case ObjType::Int16:
            case ObjType::Int32:
            case ObjType::Int64:
                return Index(obj.GetValueAsInteger());
            default:
                LOG_RUNTIME("Fail convert scalar type '%s' to Index!", newlang::toString(obj.m_var_type_current));
        }
    } else if (obj.is_tensor_type()) {
        return Index(*obj.m_tensor);
    } else if (obj.is_ellipsis()) {
        return Index(at::indexing::Ellipsis);
    } else if (obj.is_range()) {

        return Index(toSlice(obj));
    }
    LOG_RUNTIME("Fail convert object '%s' to Index!", obj.toString().c_str());
}

void Obj::SetValue_(ObjPtr value) {
    TEST_CONST_();
    if (value->is_none_type()) {
        clear_();
        return;
    } else if ((is_none_type() || is_class_type()) && (value->is_class_type() || (value->is_type_name() && isClass(value->m_var_type_fixed)))) {
        if (is_class_type() && m_class_name.compare(value->m_class_name) != 0) {
            ASSERT(!value->m_class_name.empty());
            LOG_RUNTIME("Fail set value class '%s' as class '%s'!", m_class_name.c_str(), value->m_class_name.c_str());
        }

        std::string old_name = m_var_name;
        value->CloneDataTo(*this);
        value->ClonePropTo(*this);
        m_var_name.swap(old_name);
        m_var_is_init = true;
        return;

    } else if ((is_none_type() || is_tensor_type()) && value->is_tensor_type()) {

        if (value->empty()) {
            m_var = std::monostate();
            m_tensor->reset();
            m_var_is_init = false;
            return;
        }

        if (!canCast(value->m_var_type_current, m_var_type_current)) {
            testConvertType(value->m_var_type_current);
        }

        if (is_none_type()) {

            // Присаеваем данные пустому значению
            ASSERT(std::holds_alternative<std::monostate>(m_var));
            ASSERT(!m_tensor->defined());

            if (value->is_scalar()) {
                m_var = value->m_var;
                //                        if (value->is_integral()) {
                //                            m_var = value->GetValueAsInteger(); // Нужно считывать значение, т.к. может быть ссылка
                //                        } else {
                //                            ASSERT(value->is_floating());
                //                            m_var = value->GetValueAsNumber(); // Нужно считывать значение, т.к. может быть ссылка
                //                        }
            } else {
                ASSERT(m_tensor);
                *m_tensor = value->m_tensor->clone();
            }
            m_var_type_current = value->m_var_type_current;

        } else {

            // текущая переменная уже сожержит данные

            if (is_scalar() && value->is_scalar()) {
                // Два скаляра
                switch (m_var_type_current) {
                    case ObjType::Bool:
                        if (std::holds_alternative<int64_t>(m_var)) {
                            m_var = value->GetValueAsInteger();
                        } else if (std::holds_alternative<bool *>(m_var)) {
                            ASSERT(std::get<bool *>(m_var));
                            *std::get<bool *>(m_var) = value->GetValueAsInteger();
                        }
                        break;
                    case ObjType::Int8:
                    case ObjType::Char:
                    case ObjType::Byte:
                        if (std::holds_alternative<int64_t>(m_var)) {
                            m_var = value->GetValueAsInteger();
                        } else if (std::holds_alternative<int8_t *>(m_var)) {
                            ASSERT(std::get<int8_t *>(m_var));
                            *std::get<int8_t *>(m_var) = static_cast<int8_t> (value->GetValueAsInteger());
                        }
                        break;
                    case ObjType::Int16:
                    case ObjType::Word:
                        if (std::holds_alternative<int64_t>(m_var)) {
                            m_var = value->GetValueAsInteger();
                        } else if (std::holds_alternative<int16_t *>(m_var)) {
                            ASSERT(std::get<int16_t *>(m_var));
                            *std::get<int16_t *>(m_var) = static_cast<int16_t> (value->GetValueAsInteger());
                        }
                        break;
                    case ObjType::Int32:
                    case ObjType::DWord:
                        if (std::holds_alternative<int64_t>(m_var)) {
                            m_var = value->GetValueAsInteger();
                        } else if (std::holds_alternative<int32_t *>(m_var)) {
                            ASSERT(std::get<int32_t *>(m_var));
                            *std::get<int32_t *>(m_var) = static_cast<int32_t> (value->GetValueAsInteger());
                        }
                        break;
                    case ObjType::Int64:
                    case ObjType::DWord64:
                        if (std::holds_alternative<int64_t>(m_var)) {
                            m_var = value->GetValueAsInteger();
                        } else if (std::holds_alternative<int64_t *>(m_var)) {
                            ASSERT(std::get<int64_t *>(m_var));
                            *std::get<int64_t *>(m_var) = value->GetValueAsInteger();
                        }
                        break;
                    case ObjType::Float32:
                    case ObjType::Single:
                        if (std::holds_alternative<double>(m_var)) {
                            m_var = value->GetValueAsNumber();
                        } else if (std::holds_alternative<float *>(m_var)) {
                            ASSERT(std::get<float *>(m_var));
                            *std::get<float *>(m_var) = static_cast<float> (value->GetValueAsNumber());
                        }
                        break;
                    case ObjType::Float64:
                    case ObjType::Double:
                        if (std::holds_alternative<double>(m_var)) {
                            m_var = value->GetValueAsNumber();
                        } else if (std::holds_alternative<double *>(m_var)) {
                            ASSERT(std::get<double *>(m_var));
                            *std::get<double *>(m_var) = value->GetValueAsNumber();
                        }
                        break;
                    default:
                        LOG_RUNTIME("Fail set value type '%s'!", newlang::toString(m_var_type_current));
                }

            } else if (is_scalar() && !value->is_scalar()) {

                m_var = std::monostate();
                ASSERT(m_tensor);
                ASSERT(!m_tensor->defined());
                *m_tensor = value->m_tensor->clone();

            } else if (!is_scalar() && value->is_scalar()) {

                // Установить одно значение для всех элементов тензора
                if (value->is_integral()) {
                    m_tensor->set_(torch::scalar_tensor(value->GetValueAsInteger(), m_tensor->scalar_type()));
                } else {
                    ASSERT(is_floating());
                    m_tensor->set_(torch::scalar_tensor(value->GetValueAsNumber(), m_tensor->scalar_type()));
                }

            } else {
                //  Продублировать значения тензора если они одинакового размера
                ASSERT(m_tensor);
                if (m_tensor->sizes().equals(value->m_tensor->sizes())) {
                    *m_tensor = value->m_tensor->toType(m_tensor->scalar_type()).clone();
                } else {
                    LOG_RUNTIME("Different sizes of tensors!");
                }
            }
        }
        m_var_is_init = true;
        return;

    } else if ((is_none_type() || is_string_type()) && value->is_string_type()) {

        switch (m_var_type_current) {
            case ObjType::None: // @todo Какой тип сроки по умолчанию? Пока байтовые
            case ObjType::StrChar:
            case ObjType::FmtChar:
                SetValue_(value->GetValueAsString());
                return;
            case ObjType::StrWide:
            case ObjType::FmtWide:
                SetValue_(value->GetValueAsStringWide());
                return;
        }

    } else if ((is_none_type() || is_dictionary_type()) && (value->is_dictionary_type() || value->getType() == ObjType::Iterator)) {

        std::string old_name = m_var_name;
        clear_();
        value->CloneDataTo(*this);
        value->ClonePropTo(*this);
        m_var_name.swap(old_name);
        m_var_is_init = true;
        return;

        //            } else if ((is_none_type() || m_var_type_current == ObjType::Pointer) && value->m_var_type_current == ObjType::Pointer) {
        //                //@todo Check tree type !!!
        //
        //                std::string old_name = m_var_name;
        //                value->CloneDataTo(*this);
        //                value->ClonePropTo(*this);
        //                m_var_name.swap(old_name);
        //                m_var_is_init = true;
        //                return;

    } else if (((is_none_type() || m_var_type_current == ObjType::Function) && value->is_function_type()) ||
            ((is_none_type() || m_var_type_current == ObjType::Pointer) && value->m_var_type_current == ObjType::Pointer)) {
        //@todo Check function type args !!!

        std::string old_name = m_var_name;
        value->CloneDataTo(*this);
        value->ClonePropTo(*this);
        m_var_name.swap(old_name);
        m_var_is_init = true;
        return;

    } else if ((is_none_type() && value->getType() == ObjType::Rational) || ((m_var_type_current == ObjType::Rational) && value->is_arithmetic_type())) {

        if (is_none_type()) {
            m_rational = *value->GetValueAsRational();
            m_var_is_init = true;
        } else {
            m_rational.set_(*value->GetValueAsRational());
        }
        m_var = std::monostate();
        m_var_type_current = ObjType::Rational;
        return;

    } else if ((is_none_type() || m_var_type_current == ObjType::Function || m_var_type_current == ObjType::EVAL_FUNCTION) && value->is_block()) {
        //@todo Check function type args !!!

        //            std::string old_name = m_var_name;
        //            TermPtr save_proto = m_func_proto;
        //            TermPtr save_block = m_block_source;
        //            ObjType save_type = m_var_type_current;
        //            value->CloneDataTo(*this);
        //            value->ClonePropTo(*this);
        //            m_var_name.swap(old_name);
        //            *const_cast<TermPtr *> (&m_func_proto) = save_proto;
        m_sequence = value->m_sequence;
        m_var_is_init = value->m_var_is_init;
        //            m_var_type_current = save_type;

        return;

    } else if ((is_none_type() || m_var_type_current == ObjType::Pointer || m_var_type_current == ObjType::NativeFunc) && (value->m_var_type_current == ObjType::Pointer || value->m_var_type_fixed == ObjType::Pointer)) {

        m_var_is_init = value->m_var_is_init;
        ASSERT(std::holds_alternative<void *>(value->m_var));
        m_var = std::get<void *>(value->m_var);
        return;

    }

    LOG_RUNTIME("Set value type '%s' as '%s' not implemented!", newlang::toString(m_var_type_current), value->toString().c_str());
}

IntAny::IntAny(const ObjPtr obj, ObjType type) : Obj(type, nullptr, nullptr, type, true) {
    m_return_obj = obj;
}



ObjType newlang::typeFromLimit(double value, ObjType type_default) {
    if (value >= std::numeric_limits<float>::min() && value < std::numeric_limits<float>::max()) {
        return ObjType::Float32;
    }
    return ObjType::Float64;
}

ObjType newlang::typeFromLimit(int64_t value, ObjType type_default) {
    if (value == 1 || value == 0) {
        return ObjType::Bool;
    } else if (value < std::numeric_limits<int32_t>::min() || value > std::numeric_limits<int32_t>::max()) {
        ASSERT(value > std::numeric_limits<int64_t>::min());
        ASSERT(value < std::numeric_limits<int64_t>::max());
        return ObjType::Int64;
    } else if (value < std::numeric_limits<int16_t>::min() || value > std::numeric_limits<int16_t>::max()) {
        return ObjType::Int32;
    } else if (value < std::numeric_limits<int8_t>::min() ||
            value > std::numeric_limits<int8_t>::max()) { //-127 < ... > 128
        return ObjType::Int16;
    } else {
        return ObjType::Int8;
    }
    return type_default;
}


#pragma message "Переделать сравение"
bool Obj::op_class_test(const char *name, Context * ctx) const {

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
    ObjType type = RunTime::BaseTypeFromString(m_ctx ? m_ctx->m_runtime.get() : nullptr, name, &has_error);
    if (has_error) {
        LOG_DEBUG("Type name %s not found!", name);
        return false;
    }

    ObjType check_type = m_var_type_current;
    if (m_var_type_current == ObjType::Type || (!m_var_is_init && m_var_type_current == ObjType::None)) {
        check_type = m_var_type_fixed;
    }

    if (isContainsType(type, check_type)) {
        return true;
    }

    std::string class_name = newlang::toString(check_type);
    return !class_name.empty() && class_name.compare(name) == 0;
}
