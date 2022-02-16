
#include "pch.h"

#include <core/term.h>
#include <core/object.h>
#include <core/newlang.h>
#include <core/context.h>

using namespace newlang;

std::ostream & operator<<(std::ostream &out, newlang::Object & var) {
    out << var.toString().c_str();
    return out;
}

std::ostream & operator<<(std::ostream &out, newlang::ObjPtr var) {
    if(var) {
        out << var->toString().c_str();
    } else {
        out << "<NOT OBJECT>";
    }
    return out;
}

newlang_exception::newlang_exception(ObjPtr obj) : m_obj(obj), std::runtime_error(obj->GetValueAsString()) {
    if(m_obj) {
        LOG_ERROR("NewLang exception %s!", m_obj->toString().c_str());
    } else {
        LOG_ERROR("'%s'", what());
    }
}

int64_t Object::size(int64_t dim) const {
    if(is_tensor()) {
        return m_value.size(dim);
    }
    ASSERT(dim == 0);
    if(m_var_type_current == ObjType::StrChar) {
        return m_str.size();
    } else if(m_var_type_current == ObjType::StrWide) {
        return m_wstr.size();
        //        std::wstring_convert < std::codecvt_utf8<wchar_t>, wchar_t> converter;
        //        std::wstring temp = converter.from_bytes(m_string);
        //        return temp.size();
    }
    return Variable::size();
}

const Variable<ObjPtr>::PairType & Object::at(size_t index) const {
    //    if(m_var_type == Type::STRBYTE) {
    //        if(index < m_string.size()) {
    //            m_str_pair = pair(CreateString(std::string(1, m_string[index])));
    //            return m_str_pair;
    //        }
    //        LOG_EXCEPT(std::out_of_range, "Index '%lu' not exists in byte string '%s'!", static_cast<unsigned long> (index), m_string.c_str());
    //    } else if(m_var_type == Type::STRCHAR) {
    //        std::wstring_convert < std::codecvt_utf8<wchar_t>, wchar_t> converter;
    //        std::wstring temp = converter.from_bytes(m_string);
    //        if(index < temp.size()) {
    //            m_str_pair = pair(CreateString(std::wstring(1, temp[index])));
    //            return m_str_pair;
    //        }
    //        LOG_EXCEPT(std::out_of_range, "Index '%lu' not exists in char string '%s'!", static_cast<unsigned long> (index), m_string.c_str());
    //    }
    return Variable::at(index);
}

Variable<ObjPtr>::PairType & Object::at(size_t index) {
    if(m_var_type_current == ObjType::StrChar) {
        if(index < m_str.size()) {
            m_str_pair = pair(CreateString(std::string(1, m_str[index])));
            return m_str_pair;
        }
        LOG_CALLSTACK(std::out_of_range, "Index '%lu' not exists in byte string '%s'!", static_cast<unsigned long> (index), m_str.c_str());
    } else if(m_var_type_current == ObjType::StrWide) {
        if(index < m_wstr.size()) {
            m_str_pair = pair(CreateString(std::wstring(1, m_wstr[index])));
            return m_str_pair;
        }
        LOG_CALLSTACK(std::out_of_range, "Index '%lu' not exists in byte string '%s'!", static_cast<unsigned long> (index), "WIDE");
        //        std::wstring_convert < std::codecvt_utf8<wchar_t>, wchar_t> converter;
        //        std::wstring temp = converter.from_bytes(m_string);
        //        if(index < temp.size()) {
        //            m_str_pair = pair(CreateString(std::wstring(1, temp[index])));
        //            return m_str_pair;
        //        }
        //        LOG_CALLSTACK(std::out_of_range, "Index '%lu' not exists in char string '%s'!", static_cast<unsigned long> (index), m_string.c_str());
    } else if(is_tensor()) {
        torch::Tensor t = m_value.index({(int) index});
        m_str_pair = pair(Object::CreateTensor(t));
        return m_str_pair;
    }
    return Variable::at(index);
}

const ObjPtr Object::index_get(const std::vector<Index> &index) const {
    if(m_var_type_current == ObjType::StrChar) {
        if(index.size() != 1 || !index[0].is_integer()) {
            LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
        }
        if(index[0].integer() < m_str.size()) {
            return CreateString(std::string(1, m_str[index[0].integer()]));
        }
        LOG_CALLSTACK(std::out_of_range, "Index '%s' not exists in byte string '%s'!", IndexToString(index).c_str(), m_str.c_str());
    } else if(m_var_type_current == ObjType::StrWide) {
        if(index.size() != 1 || !index[0].is_integer()) {
            LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
        }
        if(index[0].integer() < m_wstr.size()) {
            return CreateString(std::wstring(1, m_wstr[index[0].integer()]));
        }
        LOG_CALLSTACK(std::out_of_range, "Index '%s' not exists in byte string '%s'!", IndexToString(index).c_str(), "WIDE");
        //        std::wstring_convert < std::codecvt_utf8<wchar_t>, wchar_t> converter;
        //        std::wstring temp = converter.from_bytes(m_string);
        //        if(index.is_integer() && index.integer() < temp.size()) {
        //            return CreateString(std::wstring(1, temp[index.integer()]));
        //        }
        //        LOG_CALLSTACK(std::out_of_range, "Index '%s' not exists in char string '%s'!", IndexToString(index).c_str(), m_string.c_str());
    } else if(is_tensor()) {
        torch::Tensor t = m_value.index(index);
        return Object::CreateTensor(t);
    }
    //    if(!index.is_integer()) {
    //        LOG_CALLSTACK(std::out_of_range, "Bad index '%s' in object '%s'!", IndexToString(index).c_str(), m_str.c_str());
    //    }
    if(index.size() != 1 || !index[0].is_integer()) {
        LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
    }
    return Variable::at(index[0].integer()).second;
}

ObjPtr Object::index_set_(const std::vector<Index> & index, const ObjPtr value) {
    if(m_var_type_current == ObjType::StrChar) {
        if(index.size() != 1 || !index[0].is_integer()) {
            LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
        }
        if(index[0].integer() < m_str.size()) {
            m_str.erase(index[0].integer(), 1);
            m_str.insert(index[0].integer(), value->toType(ObjType::StrChar)->m_str);
            m_var_is_init = true;
            return shared();
        }
        LOG_CALLSTACK(std::out_of_range, "Index '%s' not exists in byte string '%s'!", IndexToString(index).c_str(), m_str.c_str());
    } else if(m_var_type_current == ObjType::StrWide) {
        if(index.size() != 1 || !index[0].is_integer()) {
            LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
        }
        if(index[0].integer() < m_wstr.size()) {
            m_wstr.erase(index[0].integer(), 1);
            m_wstr.insert(index[0].integer(), value->toType(ObjType::StrWide)->m_wstr);
            m_var_is_init = true;
            return shared();
        }
        LOG_CALLSTACK(std::out_of_range, "Index '%s' not exists in byte string '%s'!", IndexToString(index).c_str(), "WIDE");

        //        std::wstring_convert < std::codecvt_utf8<wchar_t>, wchar_t> converter;
        //        std::wstring temp = converter.from_bytes(m_string);
        //        if(index.is_integer() && index.integer() < temp.size()) {
        //            std::wstring ins_value = converter.from_bytes(value->toType(ObjType::StrChar)->m_string);
        //            temp.erase(index.integer(), 1);
        //            temp.insert(index.integer(), ins_value);
        //            m_string = converter.to_bytes(temp);
        //            return shared();
        //        }
        //        LOG_CALLSTACK(std::out_of_range, "Index '%s' not exists in char string '%s'!", IndexToString(index).c_str(), m_string.c_str());
    } else if(is_tensor()) {

        m_value.index_put_(index, value->toTensor());
        m_var_is_init = true;
        return shared();

    } else if(is_dictionary_type()) {
        if(index.size() != 1 || !index[0].is_integer()) {
            LOG_RUNTIME("The index must be an integer value '%s'!", IndexToString(index).c_str());
        }
        (*at(index[0].integer()).second) = value;
        return shared();
        LOG_CALLSTACK(std::out_of_range, "Index '%s' not exists in object '%s'!", IndexToString(index).c_str(), toString().c_str());
    }
    LOG_RUNTIME("Don`t set index '%s' in object '%s'!", IndexToString(index).c_str(), toString().c_str());
}

ObjPtr Object::op_set_index(size_t index, std::string value) {
    if(m_var_type_current == ObjType::StrChar) {
        if(index < m_str.size()) {
            m_str.erase(index, 1);
            m_str.insert(index, value);
            m_var_is_init = true;
            return shared();
        }
        LOG_CALLSTACK(std::out_of_range, "Index '%lu' not exists in byte string '%s'!", static_cast<unsigned long> (index), m_str.c_str());
    } else if(m_var_type_current == ObjType::StrWide) {
        if(index < m_wstr.size()) {
            m_wstr.erase(index, 1);
            m_wstr.insert(index, utf8_decode(value));
            m_var_is_init = true;
            return shared();
        }
        LOG_CALLSTACK(std::out_of_range, "Index '%lu' not exists in byte string '%s'!", static_cast<unsigned long> (index), "WIDE");
        //        std::wstring_convert < std::codecvt_utf8<wchar_t>, wchar_t> converter;
        //        std::wstring temp = converter.from_bytes(m_string);
        //        if(index < temp.size()) {
        //            std::wstring ins_value = converter.from_bytes(value);
        //            temp.erase(index, 1);
        //            temp.insert(index, ins_value);
        //            m_string = converter.to_bytes(temp);
        //            return shared();
        //        }
        //        LOG_CALLSTACK(std::out_of_range, "Index '%lu' not exists in char string '%s'!", static_cast<unsigned long> (index), m_string.c_str());
    }
    //    at(index).second.set_(value);
    (*at(index).second) = value;
    return Variable::operator[](index);
}

bool Object::exist(ObjPtr &find, bool strong) {
    for (auto &elem : * this) {
        if(strong && find->op_accurate(elem.second)) {
            return true;
        } else if(!strong && find->op_equal(elem.second)) {
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

ObjType newlang::getSummaryTensorType(ObjPtr & obj, ObjType start) {
    ObjType result = ObjType::None;
    if(!obj->getName().empty()) {
        LOG_RUNTIME("Tensor does not support named data or dimensions '%s'!", obj->getName().c_str());
    }
    if(obj->is_dictionary_type()) {
        for (size_t i = 0; i < obj->size(); i++) {
            result = getSummaryTensorType(obj->at(i).second, result);
        }
        return result;
    } else if(obj->is_arithmetic_type() || obj->is_bool_type()) {
        if(start >= std::max(obj->m_var_type_current, obj->m_var_type_fixed)) {
            return start;
        } else {
            return std::max(obj->m_var_type_current, obj->m_var_type_fixed);
        }
    }
    LOG_RUNTIME("Tensor support arithmetic data type only '%s'!", obj->toString().c_str());
}

std::vector<int64_t> newlang::getTensorSizes(Object *obj) {
    //    ASSERT(obj);
    //    if(!obj->is_dictionary_type()) {
    //        LOG_RUNTIME("Tensor size support for dict or class only '%s'!", obj->toString().c_str());
    //    }
#warning TensorSizes !!!!
    return {2, 4};
}

ObjPtr Object::operator+=(Object value) {
    if(is_tensor()) {
        if(value.is_tensor()) {
            testResultIntegralType(value.m_var_type_current, true);
            m_value.add_(value.m_value);
            return shared();
        }
        //    } else if(is_arithmetic_type()) {
        //        if(value.is_arithmetic_type()) {
        //            if(value.is_tensor()) {
        //                m_value.add_(value.asTensor_());
        //                return shared();
        //            } else if(value.is_integer()) {
        //                m_value.add_(value.GetValueAsInteger());
        //                return shared();
        //            } else if(isFloatingType(value.getType())) {
        //                m_value.add_(value.GetValueAsNumber());
        //                return shared();
        //            }
        //
        //        }
    }
    switch(m_var_type_current) {
        case ObjType::StrChar:
        case ObjType::StrWide:
            switch(value.m_var_type_current) {
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
        case ObjType::Dict:
            if(value.m_var_type_current == ObjType::None) {
                return shared();
            } else if(value.m_var_type_current == ObjType::Class || value.m_var_type_current == ObjType::Dict) {
                for (size_t i = 0; i < value.size(); i++) {
                    push_back(value.at(i));
                }
                return shared();
            }
            break;
    }
    LOG_RUNTIME("Operator '+' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

ObjPtr Object::operator-=(Object value) {
    if(m_var_type_current == ObjType::None) {
        m_var_type_current = value.m_var_type_current;
    } else if(is_tensor() && value.is_tensor()) {
        testResultIntegralType(value.m_var_type_current, true);
        m_value.sub_(value.m_value);
        return shared();
    }
    switch(m_var_type_current) {
        case ObjType::Class:
        case ObjType::Dict:
            if(value.m_var_type_current == ObjType::None) {
                return shared();
            } else if(value.m_var_type_current == ObjType::Class || value.m_var_type_current == ObjType::Dict) {
                for (size_t i = 0; i < value.size(); i++) {
                    auto found = select(value.name(i));
                    if(!found.complete()) {
                        erase(found);
                    }
                }
                return shared();
            }
            break;
    }
    LOG_RUNTIME("Operator '-' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

ObjPtr Object::operator*=(Object value) {
    if(m_var_type_current == ObjType::None) {
        m_var_type_current = value.m_var_type_current;
    } else if(is_tensor() && value.is_tensor()) {
        testResultIntegralType(value.m_var_type_current, true);
        m_value.mul_(value.m_value);
        return shared();
    }

    switch(m_var_type_current) {

        case ObjType::Class:
        case ObjType::Dict:
            if(value.m_var_type_current == ObjType::None) {
                Variable::clear_();
                return shared();
            } else if(value.m_var_type_current == ObjType::Class || value.m_var_type_current == ObjType::Dict) {
                op_bit_and_set(value, false);
                return shared();
            }
            break;

        case ObjType::StrChar:
            if(value.is_integer()) {
                m_str = repeat(m_str, value.GetValueAsInteger());
                return shared();
            } else if(value.is_string_type()) {
                m_str += value.GetValueAsString();
                return shared();
            }
        case ObjType::StrWide:
            if(value.is_integer()) {
                m_wstr = repeat(m_wstr, value.GetValueAsInteger());
                return shared();
            } else if(value.is_string_type()) {
                m_wstr += utf8_decode(value.GetValueAsString());
                return shared();
            }


            //        case ObjType::ARRAY:
            //            if(value.m_var_type == ObjType::EMPTY) {
            //                Variable::clear();
            //                return shared();
            //            } else if(value.m_var_type == ObjType::ARRAY) {
            //                op_bit_and_set(value, false);
            //                return shared();
            //            }
            //            break;
    }
    LOG_RUNTIME("Operator '*' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

ObjPtr Object::operator/=(Object value) {
    if(is_tensor() && value.is_tensor()) {
        testResultIntegralType(ObjType::Double, false);
        m_value.div_(value.m_value);
        return shared();
    }
    LOG_RUNTIME("Operator '/' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

ObjPtr Object::op_div_ceil_(Object & value) {
    if(is_tensor() && value.is_tensor()) {
        ObjType type = m_var_type_current;
        testResultIntegralType(ObjType::Float, false);
        m_value.div_(value.m_value, "floor");
        m_value = m_value.toType(toTorchType(type));
        return shared();
    }
    LOG_RUNTIME("Operator '//' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

ObjPtr Object::operator%=(Object value) {
    if(is_tensor() && value.is_tensor()) {
        testResultIntegralType(value.m_var_type_current, false);
        m_value.fmod_(value.m_value);
        return shared();
    }
    LOG_RUNTIME("Operator '%%' fail for '%s' and '%s'", toString().c_str(), value.toString().c_str());
}

size_t Object::ItemValueCount(ObjPtr &find, bool strong) {
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

void Object::CloneDataTo(Object & clone) const {

    NL_CHECK(!isLocalType(m_var_type_current), "Local object not clonable!");

    if(&clone != this) { // Не клонировать сам в себя
        clone.m_var_type_current = m_var_type_current;
        clone.m_var_type_fixed = m_var_type_fixed;
        clone.m_var_is_init = m_var_is_init;
        clone.m_var_type_name = m_var_type_name;

        clone.m_var_name = m_var_name;
        clone.m_str = m_str;
        clone.m_wstr = m_wstr;

        clone.m_class_base = m_class_base;
        clone.m_class_name = m_class_name;
        //        clone.m_ctx = m_ctx;
        clone.m_is_const = false;

        clone.m_ref_count = m_ref_count;
        clone.m_func_ptr = m_func_ptr;
        *const_cast<TermPtr *> (&clone.m_func_proto) = m_func_proto;
        if(is_tensor()) {
            clone.m_value = m_value.clone();
        }
    }
}

void Object::ClonePropTo(Object & clone) const {

    NL_CHECK(!isLocalType(m_var_type_current), "Local object not clonable!");

    for (int i = 0; i < Variable<ObjPtr>::size(); i++) {
        if(Variable<ObjPtr>::at(i).second) {
            if(Variable<ObjPtr>::at(i).second->m_is_reference || Variable<ObjPtr>::at(i).second->m_is_reference) {
                clone.push_back(Variable<ObjPtr>::at(i));
            } else {
                clone.push_back(Variable<ObjPtr>::at(i).second->Clone(nullptr), name(i));
            }
        } else {
            if(name(i).empty()) {
                LOG_CALLSTACK(std::logic_error, "Null arg %d without name! %s", i, toString().c_str());
            }
            // Объекта может не быть у обязательных параметров функций 
            clone.push_back(nullptr, name(i));
        }
    }
}

void Object::SetTermProp(Term & term) {
    m_namespace = term.m_namespace;
}

void newlang::calcTensorDims(ObjPtr & obj, std::vector<int64_t> &dims) {
    if(obj->is_dictionary_type()) {
        dims.push_back(obj->size());
        if(obj->size()) {
            calcTensorDims(obj->at(0).second, dims);
        }
    }
}

ObjPtr Object::GetIndex(ObjPtr obj, TermPtr index_arg) {
    ASSERT(index_arg->size() == 1);
    TermPtr index = index_arg->at(0).second;
    ASSERT(index);
    if(index->getTermID() != TermID::INTEGER) {
        LOG_RUNTIME("Fail index type %s '%s'", newlang::toString(index->getTermID()), index->toString().c_str());
    }
    return GetIndex(obj, std::stoi(index->m_text.c_str()));
}

std::string Object::toString(bool deep) const {
    std::string result(m_is_reference ? "&" : "");
    result += m_var_name;
    if(!result.empty() && m_var_type_current == ObjType::Class && !deep && m_is_reference) {
        return result;
    }
    if(!m_var_name.empty()) {
        result.append("=");
    }
    size_t dot, last;

    std::stringstream ss;

    if(m_var_type_current == ObjType::None) {
        if(m_func_proto && m_func_proto->GetType()) {
            result += ":";
            result += m_func_proto->GetType()->toString();
        } else if(m_var_type_fixed != ObjType::None) {
            result += ":";
            result += newlang::toString(m_var_type_fixed);
        }
        result += "_";
        return result;
    } else if(is_tensor()) {
        if(is_scalar()) {
            result += GetValueAsString();
        } else {
            result += "[";
            dump_tensor_(result);
            result += ",";
            result += "]";
        }
        return result;
    } else if(isSimpleType(m_var_type_current)) {
        result += GetValueAsString();
        return result;
    } else {
        switch(m_var_type_current) {
            case ObjType::None:// name:=<EMPTY>
                result = "_";
                return result;
            case ObjType::Error:
                result.append(m_str);
                return result;

            case ObjType::StrChar:
                result += "'";
                result += m_str;
                result.append("'");
                return result;

            case ObjType::StrWide:// name:='string' or name:="string"
                result += "\"";
                result += utf8_encode(m_wstr);
                result.append("\"");
                return result;
                //        case Object::Type::STRVAR:// name:=`str$var`
                //            result += "`";
                //            result.append(m_string);
                //            result.append("`");
                //            return result;
                //            case ObjType::Long:// name:=123
                //                result += GetValueAsString();
                //                return result;
                //            case ObjType::Double: // name:=123.0
                //                result += GetValueAsString();
                //                return result;
                //            case ObjType::Bool:// name:=@true
                //                result += GetValueAsString();
                //                return result;
            case ObjType::Range: // name:=(1,second="two",3,<EMPTY>,5)
                result = at("start")->GetValueAsString();
                result += "..";
                result += at("stop")->GetValueAsString();
                result += "..";
                result += at("step")->GetValueAsString();
                return result;

            case ObjType::Dict: // name:=(1,second="two",3,<EMPTY>,5)
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
            case ObjType::NativeFunc:
            case ObjType::FUNCTION: // name:={function code}
            case ObjType::TRANSPARENT: // name=>{function code}
                ASSERT(m_func_proto);
                result += m_func_proto->m_text;
                result += "(";
                m_func_proto->dump_items_(result);
                result += ")";
                if(!m_func_proto->m_type_name.empty()) {
                    result += ":";
                    result += m_func_proto->m_type_name;
                }
                result += "{";
                //                result.append(m_str);
                result.append("}");
                return result;

            case ObjType::EVAL_FUNCTION: // name=>{function code}
            case ObjType::EVAL_TRANSP:
                ASSERT(m_func_proto);
                result += m_func_proto->m_text;
                result += "(";
                m_func_proto->dump_items_(result);
                result += ")";
                if(!m_func_proto->m_type_name.empty()) {
                    result += ":";
                    result += m_func_proto->m_type_name;
                }

                if(m_var_type_current == ObjType::EVAL_FUNCTION) {
                    result += ":=";
                } else {
                    ASSERT(m_var_type_current == ObjType::EVAL_TRANSP);
                    result += ":-";
                }

                result += "{";
                if(m_func_source) {
                    result += m_func_source->toString();
                    if(m_func_source->getTermID() != TermID::BLOCK) {
                        result += ";";
                    }
                }
                result.append("}");
                return result;
            case ObjType::Class: // name:=@term(id=123, ...) name=base(id=123, ... )
                if(m_class_base) {
                    result += m_class_base->getName();
                }
                result += "(";

                if(!empty()) {
                    dump_dict_(result);
                }
                result += ")";
                return result;
        }
    }
    LOG_CALLSTACK(std::logic_error, "Unknown type %s (%d)", newlang::toString(m_var_type_current), (int) m_var_type_current);
}

void TensorToString_(const torch::Tensor &tensor, c10::IntArrayRef shape, std::vector<Index> &ind, const int64_t pos, std::stringstream &str) {
    std::string intend;
    ASSERT(pos < ind.size());
    str << "[";
    if(shape.size() > 1 && pos + 1 < ind.size()) {
        str << "\n";
        intend = std::string((pos + 1) * 2, ' ');
        str << intend;
    }
    if(pos + 1 < ind.size()) {
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

std::string newlang::TensorToString(const torch::Tensor &tensor) {
    std::string result;
    std::stringstream ss;

    if(tensor.dim() == 0) {
        if(tensor.is_floating_point()) {
            ss << tensor.item<double>();
            ss >> result;
        } else if(tensor.is_complex()) {
            ASSERT(!"Not implemented!");
            //            return std::to_string(tensor.item<std::complex<double>>());
        } else {
            result = std::to_string(tensor.item<int64_t>());
        }
        return result;
    }

    c10::IntArrayRef shape = tensor.sizes(); // Кол-во эментов в каждом измерении
    std::vector<Index> ind(shape.size(), 0); // Счетчик обхода всех эелемнтов тензора
    TensorToString_(tensor, shape, ind, 0, ss);
    ss << ":";
    result = ss.str();
    result += newlang::toString(fromTorchType(tensor.scalar_type()));

    return result;
}

std::string Object::GetValueAsString() const {
    std::string result;
    std::string temp;
    size_t dot, last;
    std::stringstream ss;

    TEST_INIT_();

    switch(m_var_type_current) {
        case ObjType::None:
            return result;
        case ObjType::Bool:
            return TensorToString(m_value);
            //            return (m_value.item<bool>() ? "@true" : "@false");

            //        case ObjType::LocalBool:
            //        case ObjType::NativeBool:
            //            ASSERT(m_func_ptr);
            //            return (*reinterpret_cast<bool *> (m_func_ptr) ? "@true" : "@false");


            //        case ObjType::Byte:
        case ObjType::Char:
        case ObjType::Short:
        case ObjType::Int:
        case ObjType::Long:
            return TensorToString(m_value);
            //            return std::to_string(m_value.item<int64_t>());


        case ObjType::Float:
        case ObjType::Double:
            return TensorToString(m_value);
            //            //            result = std::to_string(m_values.number);
            //            ss << m_value.item<double>();
            //            ss >> result;
            //            return result;

        case ObjType::StrChar:
            return m_str;

        case ObjType::StrWide:
            return utf8_encode(m_wstr);



        case ObjType::NativeFunc:
        case ObjType::FUNCTION:
        case ObjType::TRANSPARENT:
        case ObjType::EVAL_FUNCTION:
        case ObjType::EVAL_TRANSP:
            return m_var_name + "={}";

        case ObjType::Class:
        case ObjType::Dict:
            return toString();

        case ObjType::Error:
            result = m_var_name;
            if(!m_var_name.empty()) {
                result += ": ";
            }
            temp = m_str;
            trim(temp, "\n");
            result += temp;
            return result;

        case ObjType::Pointer:
            ss << m_func_ptr;
            result += ss.str();
            if(m_var_type_name.empty()) {
                result += ":Pointer";
            } else {
                result += ":" + m_var_type_name;
            }
            return result;

        case ObjType::Range:
            result += toString();
            return result;
    }
    LOG_CALLSTACK(std::runtime_error, "Data type '%s' %d incompatible to string!", newlang::toString(m_var_type_current), (int) m_var_type_current);
}

ObjPtr Object::CreateLambda(const char *text) {
    TermPtr ast;
    Parser p(ast);
    p.Parse(text);
    return CreateLambda(ast);
}

ObjPtr Object::CreateLambda(TermPtr term) {
    return nullptr;
}

ObjPtr Object::CreateFunc(Context *ctx, TermPtr proto, ObjType type, const std::string var_name) {
    ASSERT(type == ObjType::FUNCTION || type == ObjType::TRANSPARENT);
    ObjPtr result = std::make_shared<Object>(type, var_name.c_str(), proto);
    Object local;
    Object args(ctx, proto, false, local);
    args.ClonePropTo(*result);
    *const_cast<TermPtr *> (&result->m_func_proto) = proto;
    if(!result->CheckArgs()) {
        LOG_RUNTIME("Fail create function '%s'!", proto->toString().c_str());
    }
    return result;
}

Object::Object(Context *ctx, const TermPtr term, bool as_value, Object &local_vars) {

    NL_CHECK(term, "Fail term!");

    m_namespace = term->m_namespace;
    m_is_reference = term->m_is_ref;
    m_var_name = term->m_name.empty() ? term->m_text : term->m_name;
    m_var_type_current = ObjType::Dict;
    m_func_abi = FFI_DEFAULT_ABI;

    *const_cast<TermPtr *> (&m_func_proto) = term;

    for (size_t i = 0; i < term->size(); i++) {
        if(term->name(i).empty()) {
            if(as_value) {
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

bool Object::CheckArgs() const {
    bool has_error = false;
    bool named = false;
    for (size_t start = 0; start < size(); start++) {
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
            for (size_t i = start + 1; i < size(); i++) {
                if(at(start).first.compare(at(i).first) == 0) {
                    LOG_ERROR("Duplicate named argument '%s'!", at(start).first.c_str());
                    has_error = true;
                }
            }
        }
    }
    return !has_error;
}

ObjPtr Object::Call(Context *ctx, Object args) {
    if(is_string_type()) {
        ObjPtr result = Clone();
        result->m_str = format(result->m_str, args);
        return result;
    } else if(is_function()) {
        Object local;
        ObjPtr param = std::make_shared<Object>(ctx, m_func_proto, false, local);
        param->ConvertToArgs_(args, true, ctx);
        param->push_front(pair(Object::CreateNone(), "$0")); // Self

        if(ctx) {
            ctx->RegisterInContext(param);
        }

        if(m_var_type_current == ObjType::FUNCTION) {
            return (*reinterpret_cast<FunctionType *> (m_func_ptr))(ctx, *param.get()); // Непосредственно вызов функции
        } else if(m_var_type_current == ObjType::TRANSPARENT) {
            return (*reinterpret_cast<TransparentType *> (m_func_ptr))(ctx, *param.get()); // Непосредственно вызов функции
        } else if(m_var_type_current == ObjType::NativeFunc) {
            return CallNative(ctx, *param.get());
        } else if(m_var_type_current == ObjType::EVAL_FUNCTION || m_var_type_current == ObjType::EVAL_TRANSP) {
            return Context::EvalBlock(ctx, m_func_source, *param.get());
        } else {
            LOG_RUNTIME("Call by name not implemted '%s'!", toString().c_str());
        }

        //        return (*m_function)(m_ctx, param);

    } else if(is_dictionary_type()) {

        ObjPtr result = Clone(); // Копия текущего объекта
        result->ConvertToArgs_(args, false, ctx); // С обновленными полями, переданными в аргументах
        result->m_class_base = shared(); // Текущйи объект становится базовым классом для внось создаваемого
        return result;

    } else if(args.size() > 1) {
        LOG_CALLSTACK(std::logic_error, "Unsupported operation for data type %d '%s'", (int) m_var_type_current, toString().c_str());
    }
    return Clone();
}

// Обновить параметры для вызова функции или элементы у словаря при создании копии

void Object::ConvertToArgs_(Object &in, bool check_valid, Context *ctx) {
    bool named = false;
    bool is_ellipsis = false;
    if(check_valid && size()) {
        if(at(size() - 1).first.compare("...") == 0) {
            is_ellipsis = true;
            erase(size() - 1);
        }
    }
    for (size_t i = 0; i < in.size(); i++) {
        if(in.name(i).empty()) {
            if(check_valid && named) {
                LOG_CALLSTACK(std::invalid_argument, "Position %d requires a named argument!", (int) i + 1);
            }
            if(i < size()) {
                if(check_valid && at(i).second && at(i).second->getType() != ObjType::None) {
                    NL_CHECK(canCast(in[i]->getType(), at(i).second->getType()),
                            "Fail cast value '%s' to type '%s'", newlang::toString(in[i]->getType()), newlang::toString(at(i).second->getType()));
                }
                if(!at(i).second) {
                    at(i).second = Object::CreateNone();
                }
                if(m_func_proto && i < m_func_proto->size()) {
                    at(i).second->m_is_reference = (*m_func_proto)[i]->isRef();
                    NL_CHECK(canCast(in[i]->getTypeAsLimit(), typeFromString((*m_func_proto)[i]->m_type_name, ctx)),
                            "Fail cast value '%s' to type '%s'", in[i]->GetValueAsString().c_str(), (*m_func_proto)[i]->m_type_name.c_str());
                }
                at(i).second->op_assign(in[i]);
            } else {
                if(check_valid && !is_ellipsis && m_func_proto && i >= m_func_proto->size()) {
                    LOG_CALLSTACK(std::invalid_argument, "Positional args overflow. Ptrototype '%s'!", m_func_proto ? m_func_proto->toString().c_str() : "Prototype not exists!");
                }
                push_back(in.at(i));
            }
        } else {
            named = true;
            auto find = select(in.name(i));
            if(find != end()) {
                if(check_valid && *find && (*find)->getType() != in[i]->getType() && (*find)->getType() != ObjType::None) {
                    LOG_CALLSTACK(std::invalid_argument, "Different type arg '%s' and '%s'", (*find)->toString().c_str(), in[i]->toString().c_str());
                }
                //@todo  Проверка ограничений размер данных при указаном типе
                if(!*find) {
                    *find = Object::CreateNone();
                }
                (*find)->op_assign(in[i]);
            } else {
                for (size_t pos = 0; pos < size(); pos++) {
                    if(!at(pos).first.empty() && at(pos).first.compare(in.at(i).first) == 0) {
                        at(pos).second->op_assign(in[i]);
                        goto done;
                    }
                }
                if(check_valid && !is_ellipsis) {
                    LOG_CALLSTACK(std::invalid_argument, "Named arg '%s' not found!", in.name(i).c_str());
                }
                push_back(in.at(i));
done:
                ;
            }
        }
    }
    if(check_valid) {

        CheckArgsValid();
    }
}

void Object::CheckArgsValid() const {
    bool named = false;
    for (size_t i = 0; i < Variable::size(); i++) {
        if(!at(i).second) {

            LOG_CALLSTACK(std::invalid_argument, "Argument %d '%s' missed!", (int) i + 1, at(i).first.c_str());
        }
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
int Object::op_compare(Object & value) {
    if(this == &value) {
        return 0;
    }
    if((is_string_type() && value.is_string_type()) || (is_arithmetic_type() && value.is_arithmetic_type())) {
        switch(m_var_type_current) {
                //            case Type::INTEGER:
                //                if(value.getType() == Type::INTEGER) {
                //                    return m_values.integer < value.m_values.integer ? -1 : m_values.integer > value.m_values.integer ? 1 : 0;
                //                } else if(value.getType() == Type::NUMBER) {
                //                    return m_values.integer < value.m_values.number ? -1 : m_values.integer > value.m_values.number ? 1 : 0;
                //                }
                //                break;
                //
                //            case Type::NUMBER:
                //                if(value.getType() == Type::NUMBER) {
                //                    return m_values.number < value.m_values.number ? -1 : m_values.number > value.m_values.number ? 1 : 0;
                //                } else if(value.getType() == Type::INTEGER) {
                //                    return m_values.number < value.m_values.integer ? -1 : m_values.number > value.m_values.integer ? 1 : 0;
                //                }
                //                break;

            case ObjType::StrChar:
                return m_str.compare(value.GetValueAsString());

            case ObjType::StrWide:
                return m_wstr.compare(utf8_decode(value.GetValueAsString()));

            default:
                break;
        }
    }
    LOG_RUNTIME("Fail compare type %d", (int) m_var_type_current);
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

bool Object::op_equal(Object & value) {
    if(this == &value) {
        return true;
    } else if(is_tensor()) {
        // Арифметические типы данных сравниваются как тензоры
        torch::Dtype summary_type = toTorchType(static_cast<ObjType> (std::max(static_cast<uint8_t> (m_var_type_current), static_cast<uint8_t> (value.m_var_type_current))));
        try {
            if(m_value.dim() == 0 || value.m_value.dim() == 0) {
                if(m_value.dim() == 0 && value.m_value.dim() == 0) {

                    ObjType type = fromTorchType(summary_type);

                    if(isIntegralType(type, true)) {
                        return GetValueAsInteger() == value.GetValueAsInteger();
                    } else if(isFloatingType(type)) {
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
    } else if(is_bool_type()) {
        return GetValueAsBoolean() == value.GetValueAsBoolean();
    } else if(is_string_type()) {
        return GetValueAsString().compare(value.GetValueAsString()) == 0;
    } else if(is_dictionary_type() && value.is_dictionary_type()) {
        if(size() != value.size()) {
            return false;
        }
        for (size_t i = 0; i < size(); i++) {
            if(name(i).compare(value.name(i)) != 0) {
                return false;
            }
            if(!at(i).second->op_equal(value[i])) {

                return false;
            }
        }
        return true;
    }
    return false; // оставшиеся типы равны только если идентичны сами объекты (первое условие)
}

//is_none_type();
//is_bool_type();
//is_arithmetic_type();
//is_string_type();
//is_dictionary_type();
//is_other_type();

bool Object::op_accurate(Object & value) {
    if(this == &value || (is_none_type() && value.is_none_type())) {
        return true;
    } else if((is_bool_type() && value.is_bool_type())
            || (is_arithmetic_type() && value.is_arithmetic_type())
            || (is_string_type() && value.is_string_type())
            || (is_dictionary_type() && value.is_dictionary_type())) {

        return op_equal(value);
    }
    return false;
}

ObjPtr Object::op_bit_and_set(Object &obj, bool strong) {
    if(m_var_type_current == ObjType::Long) {
        if(m_var_type_current == obj.m_var_type_current) {
            m_value.bitwise_and_(obj.m_value);
            //            m_values.integer &= obj.m_values.integer;
            return shared();
        }
    } else if(m_var_type_current == ObjType::None || obj.m_var_type_current == ObjType::None) {
        Variable::clear_();
        return shared();
    } else if(m_var_type_current == ObjType::Dict || m_var_type_current == ObjType::Class) {
        if(obj.m_var_type_current == ObjType::Dict || obj.m_var_type_current == ObjType::Class) {
            size_t pos = 0;
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
        size_t pos = 0;
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

bool Object::op_class_test(ObjPtr obj) {
    if(obj->is_string_type()) {
        return op_class_test(obj->GetValueAsString().c_str());
    } else if(obj->getType() == ObjType::Class || obj->getType() == ObjType::Dict || !obj->m_class_name.empty()) {

        return op_class_test(obj->m_class_name.c_str());
    }
    LOG_RUNTIME("Class test fail for type %d of object '%s' ", (int) obj->getType(), obj->toString().c_str());
}

bool Object::op_class_test(const char *name) {
    if(!name || !*name) {
        return !m_class_base && m_class_name.empty();
    }
    if(m_var_name.compare(name) == 0) {
        return true;
    }
    if(m_class_name.compare(name) == 0) {
        return true;
    }
    if(m_class_base) {

        return m_class_base->op_class_test(name);
    }
    return false;
}

bool Object::op_duck_test_prop(Object *base, Object *value, bool strong) {
    if(!value) {
        return !strong; // Пустой объект равен любому при нечетком сравнении
    }
    if(!base || base->m_var_type_current == ObjType::None) {
        // Итина при пустом текущем может быть только если образец тоже пустой
        return value->m_var_type_current == ObjType::None;
    }
    ObjPtr field;
    for (size_t i = 0; i < value->size(); i++) {
        if(value->name(i).empty()) {
            field = (*base)[i];
        } else {
            field = (*base)[value->name(i)];
        }
        if(!field) {
            return false;
        }
        if(strong || !((*value)[i]->getType() != ObjType::None)) {
            for (auto &elem : *value) {
                if(!field->op_duck_test(elem.second, strong)) {

                    return false;
                }
            }
            //            if(!field->op_duck_test((*value)[i], strong)) {
            //
            //                return false;
            //            }
        }
    }
    return true;
}

ObjPtr Object::op_pow_(Object & obj) {
    if(obj.is_arithmetic_type()) {
        if(is_tensor()) {
            m_value.pow_(obj.toTensor());
            return shared();
        } else if(is_arithmetic_type()) {
            if(is_integer()) {
                SetValue_(static_cast<int64_t> (pow(GetValueAsInteger(), obj.GetValueAsInteger()) + 0.5));
                return shared();
            } else if(isFloatingType(m_var_type_current)) {
                SetValue_(pow(GetValueAsNumber(), obj.GetValueAsNumber()));
                return shared();
            }
        } else if(m_var_type_current == ObjType::StrChar && obj.is_integer()) {
            m_str = repeat(m_str, obj.GetValueAsInteger());
            return shared();
        } else if(m_var_type_current == ObjType::StrWide && obj.is_integer()) {
            m_wstr = repeat(m_wstr, obj.GetValueAsInteger());
            return shared();
        }
    }
    LOG_RUNTIME("Unsupported power operator for '%s' and '%s'!", toString().c_str(), obj.toString().c_str());
}

bool Object::op_duck_test(Object *value, bool strong) {

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
    } else if(value->m_var_type_current == ObjType::Dict || value->m_var_type_current == ObjType::Class) {
        if(m_var_type_current == ObjType::Dict || m_var_type_current == ObjType::Class) {

            return op_duck_test_prop(this, value, strong);
        }
        return false;
    }
    return m_var_type_current == value->m_var_type_current;
}

std::string Object::format(std::string format, Object args) {
    if(!args.empty()) {
        std::string name;
        std::string place;
        std::wstring wname;
        for (size_t i = 0; i < args.size(); i++) {

            // Заменить номер аргумента
            name = "\\$" + std::to_string(i + 1);
            place = args[i]->GetValueAsString();
            format = std::regex_replace(format, std::regex(name), place);

            if(!args.name(i).empty()) {

                std::wstring wplace = utf8_decode(place);
                std::wstring temp = utf8_decode(format);

                wname = L"\\$\\{" + utf8_decode(args.name(i)) + L"\\}";
                temp = std::regex_replace(temp, std::wregex(wname), wplace);

                wname = L"\\$" + utf8_decode(args.name(i)); // + L"\\b"; // Иначе перестает работать UTF8
                temp = std::regex_replace(temp, std::wregex(wname), wplace);

                format = utf8_encode(temp);

                //                name = "\\$\\{" + args.name(i) + "\\}";
                //                format = std::regex_replace(format, std::regex(name), place);
                //
                //                name = "\\$" + args.name(i) + "\\b";
                //                format = std::regex_replace(format, std::regex(name), place);
            }
        }
    }
    return format;
}

ObjPtr Object::toShape_(ObjPtr dims) {
    std::vector<int64_t> array = dims->toIntVector(true);
    if(is_tensor()) {
        m_value.resize_(array);
        return shared_from_this();
    }

    if(array.size() > 1) {
        LOG_RUNTIME("More than one dimension is not supported!");
    } else if(array.size() == 0) {
        // Ноль измерений не у тензора только у None
        return toType_(ObjType::None);
    } else if(size() == array[0]) {
        // Требуемый размер
        return shared_from_this();
    }

    if(m_var_type_current == ObjType::StrChar) {
        m_str.resize(array[0]);
    } else if(m_var_type_current == ObjType::StrWide) {
        m_wstr.resize(array[0]);
        //        std::wstring_convert < std::codecvt_utf8<wchar_t>, wchar_t> converter;
        //        std::wstring temp = converter.from_bytes(m_string);
        //        temp.resize(array[0]);
        //        m_string = converter.to_bytes(temp);
    } else if(is_dictionary_type()) {

        if(size() > array[0]) {
            m_data.resize(array[0]); // Уменьшить размер
        } else {
            while(size() < array[0]) {
                push_back(Object::CreateNone());
            }
        }
    } else {

        LOG_RUNTIME("Reshaping is not supported!");
    }
    return shared_from_this();
}

ObjPtr Object::toType_(ObjType target) {
    if(m_var_type_current == target) {
        // Конвертировать не нужно
        return shared_from_this();

    } else if(is_none_type() || target == ObjType::None) {
        // Любой тип при конвертации в пустой, просто очистить данные
        clear_();
        m_var_type_current = target;
        return shared_from_this();

    } else if(is_tensor()) {
        // Из тензора конвертировать в другой тип
        if(isTensor(target)) {
            m_value = m_value.toType(toTorchType(target));
            m_var_type_current = target;
            return shared_from_this();
        } else if(isString(target)) {
            if(target == ObjType::StrChar) {
                // В байтовую строку конвертируются только байтовый скаляр или одномерный байтовый тензор
                if(!(m_value.dtype().toScalarType() == at::ScalarType::Byte || m_value.dtype().toScalarType() == at::ScalarType::Char)) {
                    LOG_RUNTIME("Convert to byte string can 1-byte tensor only!");
                }

                if(m_value.dim() == 0) {
                    m_str.resize(1);
                    m_str[0] = m_value.item().toInt();
                } else if(m_value.dim() == 1) {
                    m_str.clear();
                    for (int i = 0; i < m_value.size(0); i++) {
                        m_str += m_value.index({i}).item().toChar();
                    }
                } else {
                    LOG_RUNTIME("Convert to string single dimension tensor only!");
                }
                m_var_type_current = target;
                return shared_from_this();
            } else { // ObjType::StrWide
                ASSERT(target == ObjType::StrWide);

                // В символьную строку конвертируется любой целочисленный скаляр или одномерный тензор
                if(!(m_value.dtype().toScalarType() == at::ScalarType::Byte || m_value.dtype().toScalarType() == at::ScalarType::Char ||
                        m_value.dtype().toScalarType() == at::ScalarType::Short || m_value.dtype().toScalarType() == at::ScalarType::Int)) {
                    LOG_RUNTIME("Convert to wide string can 1..4 byte tensor only!");
                }

                STATIC_ASSERT(sizeof (wchar_t) == sizeof (int32_t));

                if(m_value.dim() == 0) {
                    m_wstr.resize(1);
                    m_wstr[0] = m_value.item().toInt();
                } else if(m_value.dim() == 1) {
                    m_wstr.clear();
                    for (int i = 0; i < m_value.size(0); i++) {
                        m_wstr += m_value.index({i}).item().toInt();
                    }
                } else {
                    LOG_RUNTIME("Convert to string single dimension tensor only!");
                }

                m_var_type_current = target;
                return shared_from_this();
            }
        } else if(isDictionary(target)) {
            LOG_RUNTIME("Not implemented!!!");
        }
    } else if(is_string_type()) {
        // Из строки в другой тип данных
        if(isString(target)) {
            // Строки хранятся в байтовом представлении и их ненужно конвертировать
            m_var_type_current = target;
            return shared_from_this();
        } else if(isTensor(target)) {
            // Сконвертировать строку в тензор
            torch::Tensor std_data;
            std::wstring_convert < std::codecvt_utf8<wchar_t>, wchar_t> converter;
            std::wstring temp;
            if(m_var_type_current == ObjType::StrChar) {
                // Байтовая строка конвертируются в одномерный байтовый тензор
                std_data = torch::from_blob((void *) m_str.data(),{(int) m_str.size()}, torch::Dtype::Char);
            } else { // ObjType::StrWide
                ASSERT(m_var_type_current == ObjType::StrWide);
                STATIC_ASSERT(sizeof (wchar_t) == sizeof (int32_t));
                std_data = torch::from_blob((void *) m_wstr.data(),{(int) m_wstr.size()}, torch::Dtype::Int);
            }
            m_value = std_data.toType(toTorchType(target));
            m_var_type_current = target;
            return shared_from_this();

        } else if(isDictionary(target)) {
            // Строка в словарь, тоже допустимо, причем можно сделать даже именованные эелементы, если потребуется
            LOG_RUNTIME("Not implemented!!!");
        }
    } else if(is_dictionary_type()) {
        // Из словаря в другой тип данных
        if(isString(target)) {
            // Допустимо (обратная реализация чуть выше)
            LOG_RUNTIME("Not implemented!!!");
        } else if(isDictionary(target)) {
            // Словрь в словарь - ничего не делаем
            m_var_type_current = target;
            return shared_from_this();
        } else if(isTensor(target)) {

            //            ConvertToTensor(const Object * data)
            //            // Сконвертировать словарь в тензор
            //
            //            ObjType summary_type = DictionarySummaryType(this);

            LOG_RUNTIME("Not implemented!!!");

            //            Dimension dims = sizes(max_dim);
            //
            //            m_value = torch::zeros(dims, toTorchType(summary_type));
            //
            //            m_var_type = target;
            //            return shared_from_this();
        }
    } else if(m_var_type_current == ObjType::Range) {
        // Из диапазона в другой тип данных
        if(isDictionary(target)) {
            // В словарь
            LOG_RUNTIME("Not implemented!!!");
            //        } else if(is_tensor()isTensor(target)) {
            // В тензор

            LOG_RUNTIME("Not implemented!!!");
        }
    }
    // Остальные варианты предобразований выполнить нельзя
    LOG_RUNTIME("Can`t convert type '%s'(%d) to type '%s'(%d)!", newlang::toString(m_var_type_current), (int) m_var_type_current, newlang::toString(target), (int) target);
}

void ShapeFromDict(const Object *obj, std::vector<int64_t> &shape) {
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

std::vector<int64_t> newlang::TensorShapeFromDict(const Object *obj) {
    std::vector<int64_t> shape;
    ShapeFromDict(obj, shape);
    return shape;
}

torch::Tensor newlang::ConvertToTensor(Object * data, at::ScalarType type, bool reshape) {
    ASSERT(data);

    if(data->is_tensor()) {
        // Прообразование один в один
        if(type == at::ScalarType::Undefined) {
            if(data->is_scalar()) {
                return data->m_value.reshape({1});
            }
            return data->m_value.clone();
        } else {
            if(data->is_scalar()) {
                return data->m_value.reshape({1}).toType(type);
            }
            return data->m_value.clone().toType(type);
        }
    } else if(data->m_var_type_current == ObjType::StrChar) {
        if(type == at::ScalarType::Undefined) {
            type = at::ScalarType::Char;
        }
        // В байтовую строку конвертируются только байтовый скаляр или одномерный байтовый тензор
        return torch::from_blob((void *) data->m_str.data(), data->m_str.size(), type).clone();
    } else if(data->m_var_type_current == ObjType::StrWide) {
        if(type == at::ScalarType::Undefined) {
            type = at::ScalarType::Int;
        }
        // В символьную строку конвертируется любой целочисленный скаляр или одномерный тензор
        return torch::from_blob((void *) data->m_wstr.data(), {
            (int) data->m_wstr.size()
        }, type).clone();
    } else if(data->is_range()) {

        ASSERT(data->at("start"));
        ASSERT(data->at("stop"));
        ASSERT(data->at("step"));
        ASSERT(data->at("start")->is_arithmetic_type() || data->at("start")->is_bool_type());
        ASSERT(data->at("stop")->is_arithmetic_type() || data->at("stop")->is_bool_type());
        ASSERT(data->at("step")->is_arithmetic_type() || data->at("step")->is_bool_type());

        ObjPtr dict = Object::CreateDict();
        if(data->at("start")->is_floating() || data->at("stop")->is_floating() || data->at("step")->is_floating()) {
            double value = data->at("start")->GetValueAsNumber();
            double stop = data->at("stop")->GetValueAsNumber();
            double step = data->at("step")->GetValueAsNumber();
            for (; value < stop; value += step) {
                dict->push_back(Object::CreateValue(value));
            }
            type = toTorchType(ObjType::Double);
        } else {
            int64_t value = data->at("start")->GetValueAsInteger();
            int64_t stop = data->at("stop")->GetValueAsInteger();
            int64_t step = data->at("step")->GetValueAsInteger();
            for (; value < stop; value += step) {
                dict->push_back(Object::CreateValue(value));
            }
        }
        return ConvertToTensor(dict.get(), type, reshape);


    } else if(data->is_dictionary_type()) {
        if(type == at::ScalarType::Undefined) {
            type = at::ScalarType::Char;
        }
        std::vector<int64_t> shape;

        if(reshape) {
            shape = TensorShapeFromDict(data);
            ASSERT(shape.size());
        }

        // Из словаря
        torch::Tensor result = ConvertToTensor(data->index_get({0}).get(), type, false);
        if(result.dim() == 0) {
            result.resize_(1);
        }
        for (int i = 1; i < data->size(); i++) {
            torch::Tensor temp = ConvertToTensor(data->index_get({i}).get(), type, false);
            if(temp.dim() == 0) {
                temp.resize_(1);
            }
            result = torch::cat({result, temp});
        }

        if(reshape) {
            return result.reshape(shape);
        }
        return result;


    } else if(data->is_none_type() || (data->is_dictionary_type() && data->size() == 0)) {
        // Пустое значение или пустой словарь не конвертируются
        LOG_RUNTIME("Not implemented!");

        //    } else if(data->is_ellipsis()) {
        //        // Раскрытие словаря
        //        LOG_RUNTIME("Not implemented!");

    } else if(data->is_function()) {
        // Как преобразовать функцию? И нужно ли это делать?

        LOG_RUNTIME("Not implemented!");

    }
    // Остальные варианты преобразований выполнить нельзя (Error + служебные)
    LOG_RUNTIME("Can`t convert type %d to tensor!", (int) data->m_var_type_current);
}

//at::IntArrayRef newlang::ConvertToIntArrayRef(const Object *obj) {
//    if(!obj || obj->is_none()) {
//        return {};
//    }
//    LOG_RUNTIME("Not implemented!");
//}

at::TensorOptions newlang::ConvertToTensorOptions(const Object * obj) {
    if(!obj || obj->is_none_type()) {

        return at::TensorOptions();
    }
    LOG_RUNTIME("Not implemented!");
}

at::DimnameList newlang::ConvertToDimnameList(const Object * obj) {
    if(!obj || obj->is_none_type()) {

        return{};
    }
    LOG_RUNTIME("Not implemented!");
}

ObjPtr Object::CallNative(Context *ctx, Object args) {

    ffi_cif m_cif;
    std::vector<ffi_type *> m_args_type;
    std::vector<void *> m_args_ptr;

    union VALUE {
        const void * ptr;
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

    if(!m_func_ptr) {
        NL_CHECK(m_module_name.empty() || ctx, "You cannot load a module without access to the runtime context!");
        m_func_ptr = ctx->m_runtime->GetProcAddress(m_func_mangle_name.empty() ? m_func_proto->m_text.c_str() : m_func_mangle_name.c_str(), m_module_name.empty() ? nullptr : m_module_name.c_str());
    }
    NL_CHECK(m_func_ptr, "Fail load func name '%s' (%s) or fail load module '%s'!",
            m_func_proto->m_text.c_str(),
            m_func_mangle_name.empty() ? m_func_proto->m_text.c_str() : m_func_mangle_name.c_str(),
            m_module_name.empty() ? "none" : m_module_name.c_str());

    bool is_ellipsis = (m_func_proto->size() && (*m_func_proto)[m_func_proto->size() - 1]->getTermID() == TermID::ELLIPSIS);
    size_t check_count = is_ellipsis ? m_func_proto->size() - 1 : m_func_proto->size();


    // Пропустить нулевой аргумент для нативных функций
    for (size_t i = 1; i < args.size(); i++) {

        ASSERT(args[i]);
        if(args[i]->m_is_reference) {
            LOG_RUNTIME("Argument REFERENCE! %s", args[i]->toString().c_str());
        }

        size_t pind = i - 1; // Индекс прототипа на единицу меньше из-за пустого нулевого аргумента

        ObjType type = args[i]->m_var_type_current; //static_cast<ObjType>(static_cast<uint8_t>(args[i]->m_var_type) & );
        switch(type) {
            case ObjType::Bool:
                if(pind < check_count) {
                    NL_CHECK(!(*m_func_proto)[pind]->m_type_name.empty(), "Undefined type arg '%s'", (*m_func_proto)[pind]->toString().c_str());
                    NL_CHECK(canCast(type, typeFromString((*m_func_proto)[pind]->m_type_name, ctx)),
                            "Fail cast from '%s' to '%s'", (*m_func_proto)[pind]->m_type_name.c_str(), newlang::toString(type));
                }
                m_args_type.push_back(&ffi_type_uint8);
                temp.boolean = args[i]->GetValueAsBoolean();
                m_args_val.push_back(temp);
                break;

            case ObjType::Char:
                if(pind < check_count) {
                    NL_CHECK(!(*m_func_proto)[pind]->m_type_name.empty(), "Undefined type arg '%s'", (*m_func_proto)[pind]->toString().c_str());
                    NL_CHECK(canCast(type, typeFromString((*m_func_proto)[pind]->m_type_name, ctx)),
                            "Fail cast from '%s' to '%s'", (*m_func_proto)[pind]->m_type_name.c_str(), newlang::toString(type));
                }
                m_args_type.push_back(&ffi_type_sint8);
                temp.integer = args[i]->GetValueAsInteger();
                m_args_val.push_back(temp);
                break;

            case ObjType::Short:
                if(pind < check_count) {
                    NL_CHECK(!(*m_func_proto)[pind]->m_type_name.empty(), "Undefined type arg '%s'", (*m_func_proto)[pind]->toString().c_str());
                    NL_CHECK(canCast(type, typeFromString((*m_func_proto)[pind]->m_type_name, ctx)),
                            "Fail cast from '%s' to '%s'", (*m_func_proto)[pind]->m_type_name.c_str(), newlang::toString(type));
                }
                m_args_type.push_back(&ffi_type_sint16);
                temp.integer = args[i]->GetValueAsInteger();
                m_args_val.push_back(temp);
                break;

            case ObjType::Int:
                if(pind < check_count) {
                    NL_CHECK(!(*m_func_proto)[pind]->m_type_name.empty(), "Undefined type arg '%s'", (*m_func_proto)[pind]->toString().c_str());
                    NL_CHECK(canCast(type, typeFromString((*m_func_proto)[pind]->m_type_name, ctx)),
                            "Fail cast from '%s' to '%s'", (*m_func_proto)[pind]->m_type_name.c_str(), newlang::toString(type));
                }
                m_args_type.push_back(&ffi_type_sint32);
                temp.integer = args[i]->GetValueAsInteger();
                m_args_val.push_back(temp);
                break;

            case ObjType::Long:
                if(pind < check_count) {
                    NL_CHECK(!(*m_func_proto)[pind]->m_type_name.empty(), "Undefined type arg '%s'", (*m_func_proto)[pind]->toString().c_str());
                    NL_CHECK(canCast(type, typeFromString((*m_func_proto)[pind]->m_type_name, ctx)),
                            "Fail cast from '%s' to '%s'", (*m_func_proto)[pind]->m_type_name.c_str(), newlang::toString(type));
                }
                m_args_type.push_back(&ffi_type_sint64);
                temp.integer = args[i]->GetValueAsInteger();
                m_args_val.push_back(temp);
                break;

            case ObjType::Float:
                if(pind < check_count) {
                    NL_CHECK(!(*m_func_proto)[pind]->m_type_name.empty(), "Undefined type arg '%s'", (*m_func_proto)[pind]->toString().c_str());
                    NL_CHECK(canCast(type, typeFromString((*m_func_proto)[pind]->m_type_name, ctx)),
                            "Fail cast from '%s' to '%s'", (*m_func_proto)[pind]->m_type_name.c_str(), newlang::toString(type));
                }
                m_args_type.push_back(&ffi_type_float);
                temp.number = args[i]->GetValueAsNumber();
                m_args_val.push_back(temp);
                break;

            case ObjType::Double:
                if(pind < check_count) {
                    NL_CHECK(!(*m_func_proto)[pind]->m_type_name.empty(), "Undefined type arg '%s'", (*m_func_proto)[pind]->toString().c_str());
                    NL_CHECK(canCast(type, typeFromString((*m_func_proto)[pind]->m_type_name, ctx)),
                            "Fail cast from '%s' to '%s'", (*m_func_proto)[pind]->m_type_name.c_str(), newlang::toString(type));
                }
                m_args_type.push_back(&ffi_type_double);
                temp.number = args[i]->GetValueAsNumber();
                m_args_val.push_back(temp);
                break;

            case ObjType::StrWide:
            case ObjType::StrChar:
                ASSERT(type == ObjType::StrChar);
                if(pind < check_count) {
                    NL_CHECK(!(*m_func_proto)[pind]->m_type_name.empty(), "Undefined type arg '%s'", (*m_func_proto)[pind]->toString().c_str());
                    NL_CHECK(canCast(type, typeFromString((*m_func_proto)[pind]->m_type_name, ctx)),
                            "Fail cast from '%s' to '%s'", (*m_func_proto)[pind]->m_type_name.c_str(), newlang::toString(type));
                }
                m_args_type.push_back(&ffi_type_pointer);
                temp.ptr = args[i]->m_str.c_str();
                m_args_val.push_back(temp);
                break;

            case ObjType::Pointer:
                if(pind < check_count) {
                    NL_CHECK(!(*m_func_proto)[pind]->m_type_name.empty(), "Undefined type arg '%s'", (*m_func_proto)[pind]->toString().c_str());
                    NL_CHECK(canCast(type, typeFromString((*m_func_proto)[pind]->m_type_name, ctx)),
                            "Fail cast from '%s' to '%s'", (*m_func_proto)[pind]->m_type_name.c_str(), newlang::toString(type));
                }
                m_args_type.push_back(&ffi_type_pointer);
                temp.ptr = args[i]->m_func_ptr;
                m_args_val.push_back(temp);
                break;

            default:
                LOG_RUNTIME("Native arg '%s' not implemented!", args[i]->toString().c_str());
        }
        if(pind < check_count && (*m_func_proto)[pind]->GetType() && (*m_func_proto)[pind]->GetType()->m_text.compare("Format") == 0) {
            NL_CHECK(ParsePrintfFormat(args, i), "Fail format string or type args!");
        }
    }


    for (size_t i = 0; i < m_args_val.size(); i++) {
        m_args_ptr.push_back((void *) &m_args_val[i]);
    }


    NL_CHECK(!m_func_proto->m_type_name.empty(), "Undefined return type '%s'", m_func_proto->toString().c_str());

    VALUE res_value;
    ffi_type *m_result_type;

    ObjType type = typeFromString(m_func_proto->m_type_name, ctx);

    switch(type) {
        case ObjType::Bool:
            m_result_type = &ffi_type_uint8;
            break;

        case ObjType::Char:
            m_result_type = &ffi_type_sint8;
            break;

        case ObjType::Short:
            m_result_type = &ffi_type_sint16;
            break;

        case ObjType::Int:
            m_result_type = &ffi_type_sint32;
            break;

        case ObjType::Long:
            m_result_type = &ffi_type_sint64;
            break;

        case ObjType::Float:
            m_result_type = &ffi_type_float;
            break;

        case ObjType::Double:
            m_result_type = &ffi_type_double;
            break;

        case ObjType::Pointer:
        case ObjType::StrChar:
        case ObjType::StrWide:
            m_result_type = &ffi_type_pointer;
            break;

        default:
            LOG_RUNTIME("Native return type '%s' not implemented!", m_func_proto->m_type_name.c_str());
    }


    ASSERT(m_func_abi == FFI_DEFAULT_ABI); // Нужны другие типы вызовов ???
    if(ffi_prep_cif(&m_cif, m_func_abi, m_args_type.size(), m_result_type, m_args_type.data()) == FFI_OK) {

        ffi_call(&m_cif, FFI_FN(m_func_ptr), &res_value, m_args_ptr.data());


        if(m_result_type == &ffi_type_uint8) {
            // Возвращаемый тип может быть как Byte, так и Bool
            return Object::CreateValue(static_cast<uint8_t> (res_value.integer), typeFromString(m_func_proto->m_type_name));
        } else if(m_result_type == &ffi_type_sint8) {
            return Object::CreateValue(static_cast<int8_t> (res_value.integer), ObjType::Char);
        } else if(m_result_type == &ffi_type_sint16) {
            return Object::CreateValue(static_cast<int16_t> (res_value.integer), ObjType::Short);
        } else if(m_result_type == &ffi_type_sint32) {
            return Object::CreateValue(static_cast<int32_t> (res_value.integer), ObjType::Int);
        } else if(m_result_type == &ffi_type_sint64) {
            return Object::CreateValue(res_value.integer, ObjType::Long);
        } else if(m_result_type == &ffi_type_float) {
            return Object::CreateValue(res_value.number, ObjType::Float);
        } else if(m_result_type == &ffi_type_double) {
            return Object::CreateValue(res_value.number, ObjType::Double);
        } else if(m_result_type == &ffi_type_pointer) {
            if(type == ObjType::StrChar) {
                return Object::CreateString(reinterpret_cast<const char *> (res_value.ptr));
            } else if(type == ObjType::StrWide) {
                return Object::CreateString(reinterpret_cast<const wchar_t *> (res_value.ptr));
            } else if(type == ObjType::Pointer) {
                ObjPtr result = Object::CreateType(type, nullptr, type);
                result->m_func_ptr = (void *) res_value.ptr;
                result->m_var_is_init = true;
                return result;
            }
        } else {
            LOG_RUNTIME("Native return type '%s' not implemented!", m_func_proto->m_type_name.c_str());
        }
    }

    LOG_RUNTIME("Fail native call '%s'!", toString().c_str());

    return Object::CreateNone();
}

bool newlang::ParsePrintfFormat(Object args, size_t start) {

    if(args.size() <= start || !args[start]) {
        LOG_WARNING("Missing format string!");
        return false;
    }
    if(!args[start]->is_string_type()) {
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
    while(i < types.size()) {

        if(aind < args.size()) {
            //            if(types[i] & PA_FLAG_PTR == PA_FLAG_PTR) {
            //                LOG_WARNING("Pointer arg '%u' not suppotred!", i);
            //                result = false;
            //                i++;
            //                aind++;
            //                continue;
            //            }
            switch(types[i] & ~PA_FLAG_MASK) {
                case PA_INT:
                    if(types[i] & PA_FLAG_MASK == 0) {
                        cast = ObjType::Int;
                    } else if(types[i] & PA_FLAG_LONG == PA_FLAG_LONG) {
                        cast = ObjType::Long;
                    } else if(types[i] & PA_FLAG_SHORT == PA_FLAG_SHORT) {
                        cast = ObjType::Short;
                    }
                    if(!canCast(args[aind]->m_var_type_current, cast)) {
                        LOG_WARNING("Cast '%s' to '%s' not supported!", newlang::toString(args[aind]->m_var_type_current), newlang::toString(cast));
                        result = false;
                    }
                    break;
                case PA_CHAR:
                    if(types[i] & PA_FLAG_MASK) {
                        LOG_WARNING("format modifier arg '%s' %u not supported!", newlang::toString(args[aind]->m_var_type_current), i);
                        result = false;
                    }
                    cast = ObjType::Char;
                    if(!canCast(args[aind]->m_var_type_current, cast)) {
                        LOG_WARNING("Cast '%s' to '%s' not supported!", newlang::toString(args[aind]->m_var_type_current), newlang::toString(cast));
                        result = false;
                    }
                    break;
                case PA_STRING:
                    if(types[i] & PA_FLAG_MASK) {
                        LOG_WARNING("format modifier arg '%s' %u not supported!", newlang::toString(args[aind]->m_var_type_current), i);
                        result = false;
                    }
                    cast = ObjType::StrChar;
                    if(!canCast(args[aind]->m_var_type_current, cast)) {
                        LOG_WARNING("Cast '%s' to '%s' not supported!", newlang::toString(args[aind]->m_var_type_current), newlang::toString(cast));
                        result = false;
                    }
                    break;
                case PA_FLOAT:
                case PA_DOUBLE:
                    if(types[i] & PA_FLAG_MASK) {
                        LOG_WARNING("format modifier arg '%s' %u not supported!", newlang::toString(args[aind]->m_var_type_current), i);
                        result = false;
                    }
                    cast = ObjType::Double;
                    if(!canCast(args[aind]->m_var_type_current, cast)) {
                        LOG_WARNING("Cast '%s' to '%s' not supported!", newlang::toString(args[aind]->m_var_type_current), newlang::toString(cast));
                        result = false;
                    }
                    break;
                default:
                    LOG_WARNING("Arg '%u' not supported!", i);
                    //PA_WCHAR,     /* wide char */
                    //PA_WSTRING,   /* const wchar_t *, wide character string */
                    //PA_POINTER,   /* void * */
                    result = false;
            }
        } else {
            LOG_WARNING("Missing argument %u", i);
            return false;
        }
        i++;
        aind++;
    }
    if(aind != args.size()) {
        LOG_WARNING("Extra arguments more %u", i);
        return false;
    }
    return result;
}
