#include "pch.h"

#include <core/variable.h>

using namespace newlang;


//Variable::Variable(Context *ctx, TermPtr term, bool as_value) {
//    if(!term) {
//        LOG_RUNTIME("Prototype not exist!");
//    }
//    m_ellipsis = term->m_ellipsis;
//    for (size_t i = 0; i < term->size(); i++) {
//        if(term->name(i).empty()) {
//            if(as_value) {
//                m_args.push_back(std::pair<std::string, ObjPtr>("", Obj::CreateFrom(ctx, term->at(i))));
//            } else {
//                m_args.push_back(std::pair<std::string, ObjPtr>(term->at(i)->getText(), nullptr));
//            }
//        } else {
//            m_args.push_back(Arg(term->name(i).c_str(), Obj::CreateFrom(ctx, term->at(i))));
//        }
//    }
//    CheckOrder_();
//}
//
//// Клонирование объектов
//
//Variable Variable::Copy(bool validate) const {
//    Args clone(m_ellipsis);
//    for (auto &elem : m_args) {
//        if(elem.second) {
//            clone.m_args.push_back(std::pair<std::string, ObjPtr>(elem.first, (*elem.second)()));
//        } else {
//            clone.m_args.push_back(std::pair<std::string, ObjPtr>(elem.first, nullptr));
//        }
//    }
//    if(validate) {
//
//        clone.CheckValid_();
//    }
//    return clone;
//}
//// Клонирование объектов с установкой новых значений
//
//Variable Variable::Prepare(Args &in) const {
//    Args clone = Copy(false);
//    bool named = false;
//    for (size_t i = 0; i < in.m_args.size(); i++) {
//        if(in.m_args[i].first.empty()) {
//            if(named) {
//                LOG_EXCEPT(std::invalid_argument, "Position %d requires a named argument!", (int) i + 1);
//            }
//            if(i < clone.m_args.size()) {
//                if(clone.m_args[i].second && in.m_args[i].second->getType() != clone.m_args[i].second->getType() && clone.m_args[i].second->getType() != Obj::Type::EMPTY) {
//                    LOG_EXCEPT(std::invalid_argument, "Different type arg '%s' and '%s'", clone.m_args[i].second->toString().c_str(), in.m_args[i].second->toString().c_str());
//                }
//                clone.m_args[i].second = in.m_args[i].second;
//            } else {
//                if(!clone.m_ellipsis) {
//                    LOG_EXCEPT(std::invalid_argument, "Positional args overflow. Limit %d present %d!", (int) m_args.size(), (int) in.m_args.size());
//                }
//                clone.m_args.push_back(in.m_args[i]);
//            }
//        } else {
//            named = true;
//            ObjPtr find = clone.Find(in.m_args[i].first.c_str());
//            if(find) {
//                if(clone.m_args[i].second && find->getType() != in.m_args[i].second->getType() && find->getType() != Obj::Type::EMPTY) {
//                    LOG_EXCEPT(std::invalid_argument, "Different type arg '%s' and '%s'", find->toString().c_str(), in.m_args[i].second->toString().c_str());
//                }
//                find->op_assign(in.m_args[i].second);
//            } else {
//                for (size_t pos = 0; pos < clone.Count(); pos++) {
//                    if(!clone.At(pos).first.empty() && clone.At(pos).first.compare(in.m_args[i].first) == 0) {
//                        clone.At(pos).second = in.m_args[i].second;
//                        goto done;
//                    }
//                }
//                if(!clone.m_ellipsis) {
//                    LOG_EXCEPT(std::invalid_argument, "Named arg '%s' not found!", in.m_args[i].first.c_str());
//                }
//                clone.m_args.push_back(in.m_args[i]);
//done:
//                ;
//            }
//        }
//    }
//    clone.CheckValid_();
//
//    return clone;
//}
//
//// обновления свойств объекта набором текущих параметров
//
//VarPtr Args::UpdateObject(VarPtr obj) {
//
//    bool test_position = true; // Проверяем позиционные аргументы (пока не начались именованные)
//    size_t arg_pos = 0;
//    bool is_ok = true;
//
//    for (size_t i = 0; i < m_args.size(); i++) {
//
//        if(test_position && m_args[i].first.empty()) {
//            // У аргумента нет имени - проверяем позицию аргумента
//            if(arg_pos < obj->size()) {
//                if((*obj)[arg_pos]->getType() != m_args[i].second->getType() && (*obj)[arg_pos]->getType() != Obj::Type::EMPTY) {
//                    is_ok = false;
//                    LOG_ERROR("Different type arg %s and %s", m_args[i].second->toString().c_str(), (*obj)[arg_pos]->toString().c_str());
//                }
//                (*obj)[arg_pos] = m_args[i].second;
//            } else {
//                obj->push_back(m_args[i].second, m_args[i].first);
//            }
//        } else {
//            // У аргумента есть имя - позиционные аргументы закончились
//            test_position = false;
//            if(m_args[i].first.empty()) {
//                LOG_ERROR("Fail unnamed argument %zd (%s)", arg_pos, m_args[i].second->toString().c_str());
//                is_ok = false;
//            } else {
//                auto found = obj->select(m_args[i].first);
//                if(found == obj->end()) {
//                    obj->push_back(m_args[i].second, m_args[i].first);
//                } else {
//                    if((*found)->getType() != m_args[i].second->getType() && (*found)->getType() != Obj::Type::EMPTY) {
//                        LOG_RUNTIME("Different type arg %d and %s", (int) (*found)->getType(), m_args[i].second->toString().c_str());
//                    }
//                    (*obj)[m_args[i].first] = m_args[i].second;
//                }
//            }
//        }
//        arg_pos++;
//    }
//    if(!is_ok) {
//        LOG_RUNTIME("Fail update object %s", obj->toString().c_str());
//    }
//    return obj;
//}
//
//void Variable::CheckOrder_() {
//    bool named = false;
//    for (size_t start = 0; start < m_args.size(); start++) {
//        if(m_args[start].first.empty()) {
//            if(named) {
//                LOG_EXCEPT(std::invalid_argument, "Not named argument %d!", (int) start + 1);
//            }
//        } else {
//            named = true;
//            for (size_t i = start + 1; i < m_args.size(); i++) {
//                if(!m_args[i].first.empty() && m_args[start].first.compare(m_args[i].first) == 0) {
//                    LOG_EXCEPT(std::invalid_argument, "Duplicate named argument '%s'!", m_args[start].first.c_str());
//                }
//            }
//        }
//    }
//}
//
///*
// * Проверить корректность аргументов перед выходом
// */
//void Variable::CheckValid_() {
//    bool named = false;
//    for (size_t i = 0; i < m_args.size(); i++) {
//        if(!m_args[i].second) {
//            LOG_EXCEPT(std::invalid_argument, "Argument '%s' missed!", m_args[i].first.c_str());
//        }
//    }
//}
//
