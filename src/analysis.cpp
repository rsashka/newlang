#include "pch.h"

#include "system.h"

#include <term.h>
#include <runtime.h>
#include <builtin.h>
#include <system.h>

using namespace newlang;

bool CheckClearFunction(TermPtr term) {
    if (term && term->getTermID() == TermID::EMBED) {
        return false;
    } else if (term && term->getTermID() == TermID::BLOCK) {
        for (int i = 0; i < (int) term->m_block.size(); i++) {
            if (!CheckClearFunction(term->m_block[i])) {
                return false;
            }
        }
    } else if (term && term->Right()) {
        TermPtr temp = term->Right();
        while (temp) {
            if (term->getTermID() == TermID::EMBED) {
                return false;
            }
            if (term->Right()) {
                temp = term->Right();
            } else {

                break;
            }
        }
    }
    return true;
}

bool AstAnalysis::CheckArgsType_(TermPtr proto, TermPtr value) {
    //    if (!proto || !proto->getType() || proto->getType()->isNone()) {
    //        return true;
    //    }
    //    if (!canCast(value->getType(), proto->getType())) {
    //        NL_PARSER(value, "Fail cast value '%s' to type '%s'", newlang::toString(value->getType()),
    //                newlang::toString(proto->getType()));
    //    }
    //
    //    ObjType limit_type = value->getTypeAsLimit();
    //    if (!canCast(limit_type, value->getType())) {
    //        // Строку с одним символом можно преобразовать в арифметичсекий тип
    //        if (!(isArithmeticType(value->getType()) && value->is_string_type() && value->size() == 1)) {
    //            NL_PARSER(value, "Limit value overflow!");
    //        }
    //    }
    return true;
}

bool AstAnalysis::CheckArgs_(TermPtr proto, TermPtr args) {

    ASSERT(proto); // Прототип функции с аргументами по умолчанию
    ASSERT(args); // Реальные аргументы

    return true;

    //    bool named = false;
    size_t args_count = proto->size();
    bool is_ellipsis = false;
    if (args_count) {
        if (proto->at(args_count - 1).first.compare("...") == 0) {
            is_ellipsis = true;
            args_count--;
        }
    }
    bool is_system_arg = false;
    for (int i = 0; i < args->size(); i++) {

        if (isSystemName(args->name(i))) {
            is_system_arg = true;
            continue;
        }

        if (is_system_arg) {
            NL_PARSER(args->at(i).second, "System arguments must be passed last!");
        }

        if (args->name(i).empty()) {
            //            if(check_valid && named) {
            //                LOG_RUNTIME("Position %d requires a named argument!", (int) i + 1);
            //            }
            ObjType base_type = ObjType::Any;
            if (i < args_count) {
                ASSERT(proto->at(1).second->m_type);
                //if (proto->at(1).second->m_type) {
                base_type = RunTime::BaseTypeFromString(&m_rt, proto->at(i).second->m_type->m_text);
                //}
            }


            if (i < args_count) {
                ASSERT(proto->at(i).second);
                CheckArgsType_(proto->at(i).second, args->at(i).second);
                //                if (proto->at(i).second && at(i).second->getType() != ObjType::None) {
                //                    if (!canCast(args->at(i).second->getType(), proto->at(i).second->getType())) {
                //                        LOG_RUNTIME("Fail cast value '%s' to type '%s'", newlang::toString(args->at(i).second->getType()),
                //                                newlang::toString(proto->at(i).second->getType()));
                //                    }
                //                }
                //                if (!at(i).second) {
                //                    at(i).second = Obj::CreateNone();
                //                }
                //                if (m_prototype && i < m_prototype->size()) {
                //                    at(i).second->m_is_reference = (*m_prototype)[i].second->isRef();
                //                    ObjType limit_type = (*in)[i].second->getTypeAsLimit();
                //                    if (!canCast(limit_type, base_type)) {
                //                        // Строку с одним символом можно преобразовать в арифметичсекий тип
                //                        if (!(isArithmeticType(base_type) && (*in)[i].second->is_string_type() && (*in)[i].second->size() == 1)) {
                //                            LOG_RUNTIME("Fail cast value !!!!!!!!!!!!!!");
                //                            //                            LOG_RUNTIME("Fail cast value %s%s to type %s",
                //                            //                                    (*in)[i].second->toString().c_str(),
                //                            //                                    newlang::toString((*in)[i].second->getType()),
                //                            //                                    (*m_prototype)[i].second->m_type->asTypeString.c_str());
                //                        }
                //                    }
                //                }
                //
                //                LOG_DEBUG("%s", (*in)[i].second->toString().c_str());
                //
                //                at(i).second->op_assign((*in)[i].second->toType(base_type));
            } else {
                if (!is_ellipsis && i >= args_count) {
                    NL_PARSER(args->at(i).second, "Positional args overflow!");
                }
                //                push_back(in->at(i).second->toType(base_type), in->at(i).first);
            }
        } else {
            //            named = true;
            //            auto found = proto->find(args->name(i));
            //            if (found != proto->end()) {
            //
            //                CheckArgsType_(proto->at(i).second, (*found).second);
            //
            //                //                if ((*found).second && (*found).second->getType() != (*in)[i].second->getType() && (*found).second->getType() != ObjType::None) {
            //                //                    LOG_RUNTIME("Different type arg '%s' and '%s'", (*found).second->toString().c_str(),
            //                //                            (*in)[i].second->toString().c_str());
            //                //                }
            //                //                //@todo  Проверка ограничений размер данных при указаном типе
            //                //                if (!(*found).second) {
            //                //                    (*found).second = Obj::CreateNone();
            //                //                }
            //                //                (*found).second->op_assign((*in)[i].second);
            //            } else {
            //                for (int pos = 0; pos < size(); pos++) {
            //                    //                    if (!at(pos).first.empty() && at(pos).first.compare(in->at(i).first) == 0) {
            //                    //                        at(pos).second->op_assign(in[i]);
            //                    goto done;
            //                    //                    }
            //                }
            //                if (!is_ellipsis) {
            //                    NL_PARSER(args->at(i).second, "Named arg '%s' not found!", args->name(i).c_str());
            //                }
            //                //                push_back(in->at(i));
            //done:
            //                ;
            //            }
        }
    }
    for (int i = 0; i < args_count; i++) {
        //        if (!at(i).second) {
        //            LOG_RUNTIME("Argument %d '%s' missed!", (int) i + 1, at(i).first.c_str());
        //        }
    }
}

bool AstAnalysis::UpcastOpType(TermPtr &op) {
    ASSERT(op);
    ASSERT(op->m_left);
    ASSERT(op->m_right);

    //    if (op->m_left->m_type) {
    //        if (op->m_text[0] == '+' || op->m_text[0] == '-' || op->m_text[0] == '*') { // +, +=, -, -=, *, *=
    //        } else if (op->m_text.compare("/") == 0 || op->m_text.compare("/=") == 0) {
    //        } else {
    //            if (!(op->m_text.compare("//") == 0 || op->m_text.compare("//=") == 0
    //                    || op->m_text.compare("%") == 0 || op->m_text.compare("%=") == 0)) {
    //                NL_MESSAGE(LOG_LEVEL_INFO, op, "Unknown operator '%s'!", op->m_text.c_str());
    //                return false;
    //            }
    //        }
    //    }

    return true;
}

TermPtr AstAnalysis::CalcSummaryType(const TermPtr &term, ScopeStack & stack) {
    if (term->m_id == TermID::RANGE) {
        if (term->m_type) {
            if (!canCast(term->at("start").second->m_type, term->m_type)) {
                ASSERT(term->at("start").second->m_type);
                NL_MESSAGE(LOG_LEVEL_INFO, term->at("start").second, "Fail cast type %s to %s.",
                        term->at("start").second->m_type->asTypeString().c_str(), term->m_type->asTypeString().c_str());
                return nullptr;
            }

            if (!canCast(term->at("stop").second->m_type, term->m_type)) {
                ASSERT(term->at("stop").second->m_type);
                NL_MESSAGE(LOG_LEVEL_INFO, term->at("stop").second, "Fail cast type %s to %s.",
                        term->at("stop").second->m_type->asTypeString().c_str(), term->m_type->asTypeString().c_str());
                return nullptr;
            }

            if (!canCast(term->at("step").second->m_type, term->m_type)) {
                ASSERT(term->at("step").second->m_type);
                NL_MESSAGE(LOG_LEVEL_INFO, term->at("step").second, "Fail cast type %s to %s.",
                        term->at("step").second->m_type->asTypeString().c_str(), term->m_type->asTypeString().c_str());
                return nullptr;
            }
            return term->m_type;

        } else {

            ObjType sum = ObjType::None;
            TermPtr sum_type = getDefaultType(sum);

            std::array<TermPtr, 3> all{
                term->at("start").second->m_type,
                term->at("stop").second->m_type,
                term->at("step").second->m_type};

            ObjType temp;
            for (auto &elem : all) {
                if (elem) {
                    ASSERT(!elem->m_text.empty());
                    temp = GetBaseTypeFromString(elem->m_text);
                    if (static_cast<int> (temp) > static_cast<int> (sum)) {
                        sum = temp;
                        sum_type = elem;
                    }
                }
            }

            for (auto &elem : all) {
                if (elem) {
                    if (!canCast(elem, sum_type)) {
                        NL_MESSAGE(LOG_LEVEL_INFO, elem, "Fail cast type %s to %s.",
                                elem->asTypeString().c_str(), sum_type->asTypeString().c_str());
                        return nullptr;
                    }
                }
            }
            return sum_type;
        }
    }
    NL_MESSAGE(LOG_LEVEL_INFO, term, "Summary type for '%s' not implemented!", toString(term->m_id));

    return nullptr;
}

bool AstAnalysis::CheckFollow_(TermPtr &term, ScopeStack & stack) {
    ASSERT(term);

    /*
     * [cond] --> {expr};
     * [cond] --> {expr}, [...] --> {else};
     */

    for (int64_t i = 0; i < static_cast<int64_t> (term->m_follow.size()); i++) {

        TermPtr cond = term->m_follow[i]->Left();
        ASSERT(cond);
        if (cond->getTermID() == TermID::ELLIPSIS) { // else
            if (i + 1 != term->m_follow.size()) {
                NL_MESSAGE(LOG_LEVEL_INFO, cond, "The term 'else' should be the last term in the list of conditions!");
                return false;
            }
        } else {
            if (!RecursiveAnalyzer(cond, stack)) {
                return false;
            }
        }
        if (!RecursiveAnalyzer(term->m_follow[i]->Right(), stack)) {
            return false;
        }
    }
    return true;
}

bool AstAnalysis::CheckWhile_(TermPtr &term, ScopeStack & stack) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    if (!RecursiveAnalyzer(term->Left(), stack)) {
        return false;
    }
    if (!RecursiveAnalyzer(term->Right(), stack)) {
        return false;
    }

    if (!term->m_follow.empty()) {
        // else
        ASSERT(term->m_follow.size() == 1);
        if (!RecursiveAnalyzer(term->m_follow[0], stack)) {
            return false;
        }
    }
    return true;
}

bool AstAnalysis::CheckDoWhile_(TermPtr &term, ScopeStack & stack) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    ASSERT(term->m_follow.empty());

    if (!RecursiveAnalyzer(term->Left(), stack)) {
        return false;
    }
    if (!RecursiveAnalyzer(term->Right(), stack)) {
        return false;
    }
    return true;
}

bool AstAnalysis::CheckMatching_(TermPtr &term, ScopeStack & stack) {
    NL_MESSAGE(LOG_LEVEL_INFO, term, "CheckMatching not implemented!");
    return false;
}

bool AstAnalysis::CheckWith_(TermPtr &term, ScopeStack & stack) {
    NL_MESSAGE(LOG_LEVEL_INFO, term, "CkeckWith not implemented!");
    return false;
}

bool AstAnalysis::CheckTake_(TermPtr &term, ScopeStack & stack) {

    for (auto &elem : *term) {
        RecursiveAnalyzer(elem.second, stack);
    }
    if (!term->m_follow.empty()) {
        ASSERT(term->m_follow.size() == 1);
        RecursiveAnalyzer(term->m_follow[0], stack);
    }
    return true;
}

bool AstAnalysis::CkeckRange_(TermPtr &term, ScopeStack & stack) {
    if (term->find("step") == term->end()) {
        term->push_back(Term::Create(parser::token_type::INTEGER, TermID::INTEGER, "1"), "step");
    }
    if (!RecursiveAnalyzer(term->at("start").second, stack)
            || !RecursiveAnalyzer(term->at("stop").second, stack)
            || !RecursiveAnalyzer(term->at("step").second, stack)) {
        return false;
    }
    TermPtr type = CalcSummaryType(term, stack);

    if (!type) {
        return false;
    }

    if (!term->m_type) {
        term->m_type = type;
    } else {
        if (!canCast(type, term->m_type)) {
            NL_MESSAGE(LOG_LEVEL_INFO, type, "Fail cast type %s to %s.", type->asTypeString().c_str(), term->m_type->asTypeString().c_str());
            return false;
        }
    }
    term->at("start").second->m_type = type;
    term->at("stop").second->m_type = type;
    term->at("step").second->m_type = type;

    return true;
}

bool AstAnalysis::CheckNative_(TermPtr &proto, TermPtr &native) {

    ASSERT(native->getTermID() == TermID::NATIVE);
    if (!native->m_right || native->m_right->m_text.compare("...") != 0) {
        NL_MESSAGE(LOG_LEVEL_INFO, native, "Native name definition is supported with ellipses only!");
        return false;
    }

    if (proto->isCall() != native->isCall()) {
        // Нативная функция с частичным прототипом
        TermPtr from;
        TermPtr to;
        if (native->isCall()) {
            from = native;
            to = proto;
        } else {
            ASSERT(proto->isCall());
            from = proto;
            to = native;
        }
        for (int i = 0; i < from->size(); i++) {
            to->push_back((*from)[i]);
        }
        to->m_is_call = from->m_is_call;
        to->m_type = from->m_type;

    } else if (!proto->isCall() && !native->isCall()) {
        // Нативная переменная
        if (!native->m_type) {
            native->m_type = proto->m_type;
        }

    } else if (proto->isCall() && native->isCall()) {
        NL_MESSAGE(LOG_LEVEL_INFO, native, "Native name definition is supported with ellipses only!");
        return false;
    }

    proto->m_name = native->m_text.substr(1).c_str();
    //    proto->m_obj = CreateNative(proto, nullptr, false, proto->m_name.c_str());

    if (!m_rt.GetNativeAddr(proto->m_name.c_str(), nullptr)) {
        NL_MESSAGE(LOG_LEVEL_INFO, native, "Error getting address native '%s'!", proto->toString().c_str());

        return false;
    }

    return true;
}

bool AstAnalysis::Iterator_(TermPtr &term, ScopeStack &stack) {
    ASSERT(term->getTermID() == TermID::ITERATOR);
    ASSERT(term->m_left);

    for (auto &elem : *term) {
        RecursiveAnalyzer(elem.second, stack);
    }
    return RecursiveAnalyzer(term->m_left, stack);
}

bool AstAnalysis::CheckOpType(TermPtr &op, TermPtr &left, TermPtr const right) {
    if (left && right) {
        if (!right->m_type && !left->m_type) {
            return true;
        }
        if (!left->m_type && right->m_type) {
            left->m_type = right->m_type;
            return true;
        } else if (left->m_type && !right->m_type) {
            // Mark dynamic type check cast ??????????????????????????????????
            return true;
        }
        if (right->m_type && left->m_type) {
            if (!canCast(typeFromString(right->m_type, &m_rt), typeFromString(left->m_type, &m_rt))) {
                if (canCast(typeFromString(left->m_type, &m_rt), typeFromString(right->m_type, &m_rt)) && isDefaultType(left->m_type)) {
                    // UpCast default type
                    left->m_type = right->m_type;
                } else {
                    //                    ObjType l_type = typeFromString(left->m_type, &m_rt);
                    //                    ObjType r_type = typeFromString(right->m_type, &m_rt);
                    //                    if (isString(l_type) && isIntegralType(r_type, true)) {
                    //                        if (op->m_text.compare("*") == 0) {
                    //                            //  'string' * 3
                    //                            return true;
                    //                        } else if (op->m_type) {
                    //                            // str[1] = 13;
                    //                            ObjType str_type = typeFromString(op->m_type, &m_rt);
                    //                            if (isStringChar(str_type) && canCast(r_type, ObjType::Int8)) {
                    //                                return true;
                    //                            } else if (isStringWide(str_type) && canCast(r_type, RunTime::m_wide_char_type)) {
                    //                                return true;
                    //                            }
                    //                        }
                    //                    }
                    NL_MESSAGE(LOG_LEVEL_INFO, right, "Fail cast type %s to %s.", right->m_type->asTypeString().c_str(), left->m_type->asTypeString().c_str());
                    return false;
                }
            }
            return true;
        }
        LOG_TEST("Skip CheckType left '%s' and  right '%s'", left->toString().c_str(), right->toString().c_str());
        LOG_TEST("Skip CheckType %s and %s", toString(left->getTermID()), right->toString().c_str());

        return false;
    }
    return false;
}

TermPtr AstAnalysis::CheckGetValue_(TermPtr &obj, ScopeStack &stack) {
    return nullptr;
}

bool AstAnalysis::CheckSetValue_(TermPtr &obj, TermPtr &value, ScopeStack &stack) {
    return false;
}

TermPtr AstAnalysis::CheckField_(TermPtr &term, TermPtr &field, ScopeStack &stack) {
    NL_MESSAGE(LOG_LEVEL_INFO, term, "CheckField not implemented!");
    return nullptr;
}

bool AstAnalysis::CheckIndex_(TermPtr &term, TermPtr &index, TermPtr &value, ScopeStack &stack) {

    ASSERT(index->size());
    switch (term->m_id) {
        case TermID::DICT:
        case TermID::STRCHAR:
        case TermID::STRWIDE:
            if (index->size() > 1) {
                NL_MESSAGE(LOG_LEVEL_INFO, index, "Strings and dictionaries do not support multiple dimensions!");
            }
        case TermID::TENSOR:
            break;

        case TermID::NAME:
        case TermID::LOCAL:
        case TermID::TYPE:
        case TermID::ARGS:
        case TermID::ARGUMENT:
        case TermID::STATIC:
            break;

        default:
            NL_MESSAGE(LOG_LEVEL_INFO, term, "Term type '%s' not indexable!", toString(term->m_id));
    }

    if (term->isNamed()) {
        TermPtr rvar = LookupName(term, stack);
        if (!rvar) {
            NL_MESSAGE(LOG_LEVEL_INFO, term, "Name '%s' not exist!", term->m_text.c_str());
            return false;
        }
        term->m_int_name = rvar->m_int_name;

        if (term->isCall()) {
            if (!CheckCall(rvar, term, stack)) {
                return false;
            }
        }
    } else {
        NL_MESSAGE(LOG_LEVEL_INFO, term->m_right, "Type rvalue '%s' not implemented!", toString(term->m_id));
        return false;
    }

    for (size_t i = 0; i < index->size(); i++) {
        if (!RecursiveAnalyzer(index->at(i).second, stack)) {
            return false;
        }
        if (index->at(i).second && canCast(index->at(i).second, RunTime::m_integer_type)) {
            continue;
        } else if (index->at(i).second->m_type) {
            NL_MESSAGE(LOG_LEVEL_INFO, index->at(i).second, "Index type '%s' not implemented!", index->at(i).second->m_type->m_text.c_str());
            return false;
        } else {
            NL_MESSAGE(LOG_LEVEL_INFO, index->at(i).second, "Index not implemented!");
            return false;
        }
    }

    ObjType type = typeFromString(term->m_type, &m_rt);
    if (isStringChar(type) && canCast(value, ObjType::Int8)) {
        return true;
    } else if (isStringWide(type) && canCast(value, RunTime::m_wide_char_type)) {
        return true;
    }
    NL_MESSAGE(LOG_LEVEL_INFO, term, "Index type not checked!");
    return true;
}

bool AstAnalysis::CreateOp_(TermPtr &op, ScopeStack & stack) {

    ASSERT(op);
    ASSERT(op->m_left);
    ASSERT(op->m_right);

    if (!op->m_left->isCall()) {
        // Создание переменной
        TermPtr term = op->m_left;
        TermPtr var;
        std::string int_name;
        bool name_exist;
        while (term) {

            int_name = stack.CreateVarName(term->m_text);
            var = LookupName(term, stack);

            if (op->isCreateOnce()) {
                if (var) {
                    NL_MESSAGE(LOG_LEVEL_INFO, term, "Name '%s' already exist!", var->m_text.c_str());
                    return false;
                }
            } else if (op->getTermID() == TermID::ASSIGN) {
                if (!var) {
                    NL_MESSAGE(LOG_LEVEL_INFO, term, "Name '%s' not exist!", term->m_text.c_str());
                    return false;
                }
            } else {
                ASSERT(op->isCreateOverlap());
            }

            if (!var) {
                term->m_int_name = int_name;
                if (!stack.AddName(term)) {
                    return false;
                }
            } else {
                if (!term->m_type) {
                    term->m_type = var->m_type;
                } else {
                    if (!canCast(var, term)) {
                        return false;
                    }
                }
            }


            if (op->m_right->isNamed()) {
                TermPtr r_var = LookupName(op->m_right, stack);
                if (!r_var) {
                    NL_MESSAGE(LOG_LEVEL_INFO, op->m_right, "Name '%s' not exist!", op->m_right->m_text.c_str());
                    return false;
                }
                op->m_right->m_int_name = r_var->m_int_name;

                if (op->m_right->isCall()) {
                    if (!CheckCall(r_var, op->m_right, stack)) {
                        return false;
                    }
                }

            } else if (op->m_right->getTermID() == TermID::NATIVE) {
                if (!CheckNative_(term, op->m_right)) {
                    return false;
                }
            } else if (op->m_right->getTermID() == TermID::ELLIPSIS) {
                if (!op->m_right->m_right) {
                    NL_MESSAGE(LOG_LEVEL_INFO, op->m_right, "Object expansion not found!");
                    return false;
                }
                if (!RecursiveAnalyzer(op->m_right->m_right, stack)) { //  && CheckOpType(op, term, op->m_right->m_right)
                    return false;
                }

            } else {
                if (!RecursiveAnalyzer(op->m_right, stack) && CheckOpType(op, term, op->m_right)) {
                    return false;
                }
            }


            if (term->m_right) {
                if (term->m_right->m_id == TermID::INDEX) {
                    return CheckIndex_(term, term->m_right, op->m_right, stack);
                } else if (term->m_right->m_id == TermID::FIELD) {

                    term = CheckField_(term, term->m_right, stack);
                    if (!term) {
                        return false;
                    }

                } else {
                    NL_MESSAGE(LOG_LEVEL_INFO, term, "Analysis type '%s' not implemented!", toString(term->m_right->m_id));
                    return false;
                }
            }


            if (!term->m_type && op->m_right->m_type) {
                term->m_type = op->m_right->m_type;
            } else if (term->m_type && op->m_right->m_type) {
                if (!canCast(op->m_right, term)) {
                    if (canCast(term, op->m_right) && isDefaultType(term->m_type)) {
                        // UpCast default type
                        term->m_type = op->m_right->m_type;
                        var->m_type = op->m_right->m_type; // // UpCast default type for variable
                    } else {
                        return false;
                    }
                }
            } else if (term->m_type && !op->m_right->m_type) {
                op->m_right->m_type = term->m_type;
            }

            term = term->m_list;
        }
        return true;

    } else {
        // Создание функции
        if (op->m_left->m_left) {
            NL_MESSAGE(LOG_LEVEL_INFO, op->m_left->m_left, "Multiple function creation is not supported!");
            return false;
        }

        if (!(op->m_right->isBlock() || op->m_right->m_id == TermID::NATIVE)) {
            NL_MESSAGE(LOG_LEVEL_INFO, op->m_left->m_left, "Create type '%s' is not implemented!", toString(op->m_right->m_id));
            return false;
        }

        TermPtr proto = op->m_left;

        proto->m_id = TermID::FUNCTION;
        proto->m_int_name = stack.CreateVarName(proto->m_text);

        ScopePush block_func(stack, proto, nullptr, true);

        if (!stack.AddName(proto)) {
            return false;
        }

        ScopePush block_args(stack, proto, &proto->m_int_vars, true);

        //  Add default args self and all args as dict
        TermPtr all = Term::CreateDict();
        TermPtr arg;
        std::string name;
        for (size_t i = 0; i < proto->size(); i++) {

            ASSERT(proto->at(i).second);
            if (proto->at(i).second->getTermID() == TermID::ELLIPSIS) {
                if (i + 1 != proto->size()) {
                    NL_MESSAGE(LOG_LEVEL_INFO, proto->at(i).second, "The ellipsis must be the last argument!");
                    return false;
                }
                break;
            }

            // Named arguments
            if (proto->at(i).first.empty()) {
                if (proto->at(i).second->getTermID() != TermID::NAME) {
                    NL_MESSAGE(LOG_LEVEL_INFO, proto->at(i).second, "Argument name expected!");
                    return false;
                }
                name = proto->at(i).second->m_text;
                proto->at(i).first = name;
                all->push_back({name, proto->at(i).second});

                TermPtr none = Term::CreateNil();
                none->m_int_name = NormalizeName(name);
                if (!stack.AddName(none)) {
                    return false;
                }
            } else {
                proto->at(i).second->m_int_name = NormalizeName(proto->at(i).first);
                all->push_back({proto->at(i).first, proto->at(i).second});
                if (!stack.AddName(proto->at(i).second)) {
                    return false;
                }
            }

            name = "$";
            name += std::to_string(i + 1);
            // Positional arguments
            if (!stack.AddName(proto->at(i).second, name.c_str())) {
                return false;
            }

        }

//        ScopePush block(stack, proto, &proto->m_int_vars, true);

        TermPtr none = Term::CreateName("$0");
        none->m_int_name = NormalizeName(none->m_text);

        if (!stack.AddName(all, "$*") || !stack.AddName(none, "$0")) {
            return false;
        }


        ASSERT(op->m_right);
        if (op->m_right->getTermID() == TermID::NATIVE) {
            return CheckNative_(proto, op->m_right) && stack.FixTransaction();
        } else {
            return RecursiveAnalyzer(op->m_right, stack) && stack.FixTransaction();
        }
    }
}

bool AstAnalysis::CheckCall(TermPtr &proto, TermPtr &call, ScopeStack & stack) {

    ASSERT(proto);
    ASSERT(call);
    ASSERT(call->isCall());

    if (proto->isString() && proto->GetType()) {
        if (proto->GetType()->m_text.compare(":FmtChar") == 0 || proto->GetType()->m_text.compare(":FmtWide") == 0) {
            return CheckStrPrintf(proto->m_text, call, 0);
        } else {
            try {
                MakeFormat(proto->m_text, call, &m_rt);
            } catch (std::exception &ex) {
                NL_MESSAGE(LOG_LEVEL_INFO, call, "%s", ex.what());
                return false;
            }
            return true;
        }
    }

    bool ell_count = false;
    for (int i = 0; call->m_dims && i < call->m_dims->size(); i++) {
        if (call->m_dims->at(i).second->m_id == TermID::ELLIPSIS) {
            if (ell_count) {
                NL_MESSAGE(LOG_LEVEL_INFO, call->m_dims->at(i).second, "Only one dimension of any size is possible!");
                CheckError(false);
            }
            ell_count = true;
        } else if (call->m_dims->at(i).second->isNone()) {
            // Skip dim
        } else {
            RecursiveAnalyzer(call->m_dims->at(i).second, stack);
            if (!(canCast(call->m_dims->at(i).second, ObjType::Integer) || canCast(call->m_dims->at(i).second, ObjType::Range))) {
                NL_MESSAGE(LOG_LEVEL_INFO, call->m_dims->at(i).second, "Unsupported type for tensor dimension!");
                CheckError(false);
            }
        }
    }

    if (proto->isCall()) {
        // Check function args and calc default values
        bool is_system_args = false;
        bool is_named_args = false;
        bool is_ellipsys = proto->is_variable_args();
        int max_args = proto->size() - is_ellipsys;
        int pos = 0;
        for (; pos < max_args; pos++) {

            std::string_view proto_arg_name = proto->at(pos).first;
            TermPtr proto_default_value;

            if (proto->at(pos).second->m_name.empty()) {
                proto_default_value = nullptr;
            } else {
                proto_default_value = proto->at(pos).second;
            }


            if (pos < call->size()) {


                if (!CheckCallArg(call, pos, stack)) {
                    return false;
                }

                // Системые аргументы могут быть только самыми последними в списке
                if (isSystemName(proto_arg_name)) {
                    is_system_args = true;
                } else {
                    if (is_system_args) {
                        NL_MESSAGE(LOG_LEVEL_INFO, proto->at(pos).second, "System argument name expected!");
                        return false;
                    }
                }

                TermPtr value = call->at(pos).second;
                ObjType value_type = typeFromString(value->m_type, &m_rt);

                if (value->m_right) {

                    if (value->m_right->m_id == TermID::INDEX) {
                        if (isString(value_type) || isDictionary(value_type)) {

                            if (value->m_right->size() != 1) {
                                NL_MESSAGE(LOG_LEVEL_INFO, value->m_right, "Strings and dictionaries support single dimensions only!");
                                return false;
                            }

                            if (isStringChar(value_type)) {
                                value_type = ObjType::Int8;
                            } else if (isStringWide(value_type)) {
                                value_type = RunTime::m_wide_char_type;
                            }

                        } else if (value_type == ObjType::Any) {
                            // Skip - not check
                        } else {
                            NL_MESSAGE(LOG_LEVEL_INFO, value->m_right, "Index for type '%s' not implemented!", toString(value_type));
                            return false;
                        }

                    } else if (value->m_right->m_id == TermID::FIELD) {
                        NL_MESSAGE(LOG_LEVEL_INFO, proto->at(pos).second, "System argument name expected!");
                        return false;
                    } else {
                        NL_MESSAGE(LOG_LEVEL_INFO, proto->at(pos).second, "Index type '%s' not implemented!", toString(value->m_right->m_id));
                        return false;
                    }
                }

                if (value_type != ObjType::Any && !canCast(value_type, typeFromString(proto->at(pos).second->m_type, &m_rt))) {
                    ASSERT(proto->at(pos).second->GetType());
                    NL_MESSAGE(LOG_LEVEL_INFO, proto->at(pos).second, "Fail cast from '%s' to '%s'!",
                            toString(value_type), proto->at(pos).second->GetType()->m_text.c_str());
                    return false;
                }


                if (proto->at(pos).second->GetType()) {
                    if (proto->at(pos).second->GetType()->m_text.compare(":FmtChar") == 0 || proto->at(pos).second->GetType()->m_text.compare(":FmtWide") == 0) {
                        if (!CheckStrPrintf(value->m_text, call, pos + 1)) {
                            return false;
                        }
                    }
                }

                // Именованные аргументы идут после обычных
                if (!call->at(pos).first.empty()) {
                    is_named_args = true;

                    if (proto->find(call->at(pos).first) == proto->end()) {
                        NL_MESSAGE(LOG_LEVEL_INFO, call->at(pos).second, "Named argument '%s' not found in prototype '%s'!", call->at(pos).first.c_str(), proto->m_text.c_str());
                        return false;
                    }

                } else {
                    if (is_named_args) {
                        NL_MESSAGE(LOG_LEVEL_INFO, proto->at(pos).second, "Expecting a named argument!");
                        return false;
                    }
                }

            } else {
                // Заполняются значения по умолчанию
                if (!proto_default_value) {
                    NL_MESSAGE(LOG_LEVEL_INFO, proto->at(pos).second, "The argument '%s' has no default value!", proto_arg_name.begin());
                    return false;
                } else {
                    call->push_back(proto_default_value, proto_arg_name.begin());
                }
            }
        }

        if (pos < call->size()) {
            if (!is_ellipsys) {
                NL_MESSAGE(LOG_LEVEL_INFO, proto->at(pos).second, "An unexpected extra argument!");
                return false;
            }
            while (pos < call->size()) {
                if (!RecursiveAnalyzer(call->at(pos).second, stack)) {
                    return false;
                }
                pos++;
            }
        } else if (pos > call->size()) {

            NL_MESSAGE(LOG_LEVEL_INFO, proto->at(pos).second, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
            return false;

        }

    } else {

        for (int i = 0; i < call->size(); i++) {
            if (!CheckCallArg(call, i, stack)) {
                return false;
            }
        }
    }

    // Object clone
    if (!proto->isCall() && !isTypeName(proto->m_int_name)) {
        if (call->size()) {
            NL_MESSAGE(LOG_LEVEL_INFO, call->at(0).second, "Cloning objects with field overrides is not implemented!");

            return false;
        }
    }

    return true;
}

void AstAnalysis::CheckDims(TermPtr &dims, ScopeStack & stack, bool allow_none, bool allow_ellipsis) {
    bool is_ellipsis = false;
    for (int index = 0; dims && index < dims->size(); index++) {
        if (dims->at(index).second->isNone()) {
            // Skip None
            if (!allow_none) {
                NL_MESSAGE(LOG_LEVEL_INFO, dims->at(index).second, "Empty dimension is not allowed!");
                CheckError(false);
            }

        } else if (dims->at(index).second->m_id == TermID::ELLIPSIS) {
            if (!allow_ellipsis) {
                NL_MESSAGE(LOG_LEVEL_INFO, dims->at(index).second, "Unlimited dimension is not allowed!");
                CheckError(false);
            }
            if (is_ellipsis) {
                NL_MESSAGE(LOG_LEVEL_INFO, dims->at(index).second, "Only one dimension of unlimited is possible!");
                CheckError(false);
            }
            is_ellipsis = true;
        } else {
            if (!RecursiveAnalyzer(dims->at(index).second, stack)) {
                CheckError(false);
            }
        }
    }
}

bool AstAnalysis::CheckCallArg(TermPtr &call, size_t arg_pos, ScopeStack & stack) {
    ASSERT(call);
    ASSERT(arg_pos < call->size());

    TermPtr arg = call->at(arg_pos).second;
    ASSERT(arg);

    if (arg->m_id == TermID::FILLING) {

        // Заполнение значений вызовом функции
        // :Tensor[10](1, 2, 3, ... rand() ... );

        ASSERT(!arg->Left());
        ASSERT(arg->Right());

        TermPtr fill_obj = arg->Right();
        ASSERT(fill_obj);

        if (fill_obj->m_text.find("rndа") != std::string::npos) {
            ASSERT(1);
        }
        LOG_DEBUG("%s", fill_obj->m_text.c_str());

        if (!RecursiveAnalyzer(fill_obj, stack)) {
            return false;
        }

        if (arg_pos + 1 != call->size()) {
            NL_MESSAGE(LOG_LEVEL_INFO, fill_obj, "Filling is supported for the last argument only!");
            return false;
        }

        if (!call->m_dims || !call->m_dims->size()) {
            NL_MESSAGE(LOG_LEVEL_INFO, call, "Object has no dimensions!");
            return false;
        }

        CheckDims(call->m_dims, stack, false, false);

        if (fill_obj->size()) {
            NL_MESSAGE(LOG_LEVEL_INFO, fill_obj, "Argument in function for filling not implemented!");
            return false;
        }

        if (fill_obj->m_type) {
            if (call->m_type) {
                if (!canCast(fill_obj, call)) {
                    NL_MESSAGE(LOG_LEVEL_INFO, fill_obj, "Fail cast type '%s' to '%s' type!", fill_obj->m_type->m_text.c_str(), call->m_type->m_text.c_str());
                    return false;
                }
            } else {
                call->m_type = fill_obj->m_type;
            }
        }

        return true;

    } else if (arg->m_id == TermID::ELLIPSIS) {

        if (!arg->m_name.empty()) {
            NL_MESSAGE(LOG_LEVEL_INFO, arg, "Named ellipsys not implemented!");
            return false;
        }

        TermPtr fill_obj = arg->Right();

        if (!fill_obj) {
            // Fill with last values to full size
            return true;
        }

        //        if (!fill_obj->m_type) {
        //            fill_obj->m_type = getDefaultType(ObjType::Dictionary);
        //        } else {
        //            if (!canCast(fill_obj->m_type, ObjType::Dictionary)) {
        //                NL_MESSAGE(LOG_LEVEL_INFO, fill_obj, "Fail cast type '%s' to dictionary type!", fill_obj->m_type->m_text.c_str());
        //                return false;
        //            }
        //        }

        return RecursiveAnalyzer(fill_obj, stack);

    } else if (arg->m_id == TermID::RANGE) {

        if (!RecursiveAnalyzer(arg, stack)) {
            return false;
        }

        if (!arg->m_name.empty()) {
            NL_MESSAGE(LOG_LEVEL_INFO, arg, "Named range not implemented!");
            return false;
        }

        if (arg->m_type) {
            if (call->m_type) {
                if (!canCast(arg, call)) {
                    NL_MESSAGE(LOG_LEVEL_INFO, arg, "Fail cast type '%s' to '%s' type!",
                            arg->m_type->m_text.c_str(), call->m_type->m_text.c_str());
                    return false;
                }
            } else {
                call->m_type = arg->m_type;
            }
        }
        return true;

    } else {

        return RecursiveAnalyzer(arg, stack);
    }
}

/*
 * $template := "{name} {0}"; # std::format equivalent "{1} {0}"
 * $result := $template("шаблон", name = "Строка"); # result = "Строка шаблон"
 */
std::string AstAnalysis::ConvertToVFormat_(const std::string_view format, TermPtr args) {
    if (!args || args->m_id != TermID::DICT) {
        LOG_RUNTIME("ConvertToFormat requires arguments!");
    }
    std::string result;
    int pos = 0;
    while (pos < format.size()) {
        if (format[pos] == '{' && pos + 1 < format.size() && format[pos + 1] == '{') {
            // Escaped text in doubling {{
            result += "{{";
            pos += 1;
        } else if (format[pos] == '}' && pos + 1 < format.size() && format[pos + 1] == '}') {
            // Escaped text in doubling }}
            result += "}}";
            pos += 1;
        } else if (format[pos] == '{') {
            pos += 1;
            int name = pos;
            result += '{';
            while (pos < format.size()) {
                if (format[pos] == '{') {
                    LOG_RUNTIME("Unexpected opening bracket '%s' at position %d!", format.begin(), pos);
                }
                if (format[pos] == '}') {
                    result += '}';
                    goto done;
                }
                if (isalpha(format[pos]) || format[pos] == '_') {
                    while (name < format.size()) {
                        if (isalnum(format[name]) || format[name] == '_') {
                            name++;
                        } else {
                            name -= 1;
                            break;
                        }
                    }

                    ASSERT(name > pos);
                    std::string arg_name(&format[pos], name - pos + 1);

                    auto found = args->find(arg_name);
                    if (found == args->end()) {
                        LOG_RUNTIME("Argument name '%s' not found!", arg_name.c_str());
                    }
                    result += std::to_string(std::distance(args->begin(), found));
                    pos += (name - pos);
                } else {
                    result += format[pos];
                }
                pos++;
            }
            LOG_RUNTIME("Closing bracket in '%s' for position %d not found!", format.begin(), name - 1);
        } else {
            result += format[pos];
        }
done:
        pos++;
    }
    return result;
}

fmt::dynamic_format_arg_store<fmt::format_context> AstAnalysis::MakeFormatArgs(TermPtr args, RunTime * rt) {

    fmt::dynamic_format_arg_store<fmt::format_context> store;

    for (int i = 0; i < args->size(); i++) {

        ObjType type = typeFromString(args->at(i).second->m_type, rt);

        switch (type) {
            case ObjType::Bool:
            case ObjType::Int8:
            case ObjType::Byte:
            case ObjType::Char:
            case ObjType::Int16:
            case ObjType::Word:
            case ObjType::Int32:
            case ObjType::DWord:
            case ObjType::Int64:
            case ObjType::DWord64:
                if (args->at(i).second->m_obj) {
                    store.push_back(args->at(i).second->m_obj->GetValueAsInteger());
                } else {
                    store.push_back(parseInteger(args->at(i).second->m_text.c_str()));
                }
                break;

            case ObjType::Float16:
            case ObjType::Float32:
            case ObjType::Single:
            case ObjType::Float64:
            case ObjType::Double:
                if (args->at(i).second->m_obj) {
                    store.push_back(args->at(i).second->m_obj->GetValueAsNumber());
                } else {
                    store.push_back(parseDouble(args->at(i).second->m_text.c_str()));
                }
                break;

            default:
                if (args->at(i).second->m_obj) {
                    store.push_back(args->at(i).second->m_obj->GetValueAsString());
                } else {
                    store.push_back(args->at(i).second->toString());
                }
                break;
                // LOG_RUNTIME("Support type '%s' not implemented!", toString(type));
        }
    }
    return store;
}

std::string AstAnalysis::MakeFormat(const std::string_view format, TermPtr args, RunTime * rt) {
    ASSERT(args);
    std::string result;
    std::string conv_format;
    try {
        conv_format = ConvertToVFormat_(format, args);
        fmt::dynamic_format_arg_store<fmt::format_context> store = MakeFormatArgs(args, rt);
        result = fmt::vformat(conv_format, store);
    } catch (const std::exception& ex) {
        LOG_TEST("Fail format from '%s' to '%s'", format.begin(), conv_format.c_str());
        LOG_RUNTIME("%s", ex.what());
    }
    return result;
}

bool AstAnalysis::CheckStrPrintf(const std::string_view format, TermPtr args, int start) {

    static const std::string flags_list = "-+0123456789.lLh"; // '#', '*'
    static const std::string types_list = "diufFeEgGxXaAoscp"; //, 'n'

    static std::string valid_chars;
    if (valid_chars.empty()) {
        valid_chars.append(flags_list);
        valid_chars.append(types_list);
    }

    bool result = true;
    int aind = start;

    size_t pos = 0;
    while (pos < format.length()) {
        pos = format.find('%', pos);
        if (pos == format.npos) {
            break;
        }
        pos++;
        if (format[pos] == '%' || !format[pos]) {
            pos++;
            continue;
        }

        if (aind >= args->size()) {

            NL_MESSAGE(LOG_LEVEL_INFO, args, "Argument number '%d' is missing!", (int) pos);
            return false;

        } else {

            if (isSystemName((*args)[aind].first)) {
                NL_MESSAGE(LOG_LEVEL_INFO, (*args)[aind].second, "System argument name not allowed!");
                return false;
            }

            if (!(*args)[aind].first.empty()) {
                NL_MESSAGE(LOG_LEVEL_INFO, (*args)[aind].second, "Named argument '%s' not allowed!", (*args)[aind].first.c_str());
                return false;
            }

            pos = format.find_first_not_of(valid_chars, pos);
            if (pos == format.npos) {
                pos = format.length();
            }
            pos--;

            TermPtr test_type;
            ObjType cast = ObjType::None;
            switch (format[pos]) {

                case 'a': //%a	Шестнадцатеричное в виде 0xh.hhhhp+d (только С99)
                case 'A': //%A	Шестнадцатеричное в виде 0Xh.hhhhP+d (только С99)
                case 'd': //%d	Десятичное целое со знаком
                case 'i': //%i	Десятичное целое со знаком
                case 'o': //%o	Восьмеричное без знака
                case 'u': //%u	Десятичное целое без знака
                case 'x': //%x	Шестнадцатеричное без знака (буквы на нижнем регистре)
                case 'X': //%X	Шестнадцатеричное без знака (буквы на верхнем регистре)
                    cast = ObjType::Int32;
                    if (pos && (format[pos - 1] == 'l' || format[pos - 1] == 'L')) {
                        cast = ObjType::Int64;
                    } else if (pos && format[pos - 1] == 'h') {
                        cast = ObjType::Int16;
                    }

                    //                    if ((*args)[aind].second->getType() == ObjType::Iterator) {
                    //                        ASSERT((*args)[aind].second->m_iterator);
                    //                        test_type = (*args)[aind].second->m_iterator->data().second->getType();
                    //                    } else {
                    test_type = (*args)[aind].second;
                    //                    }

                    if (!canCast(test_type, cast)) {
                        ASSERT(test_type->m_type);
                        NL_MESSAGE(LOG_LEVEL_INFO, (*args)[aind].second, "Fail cast from '%s' to '%s'!", test_type->m_type->m_text.c_str(), newlang::toString(cast));
                        result = false;
                    }
                    break;


                case 'e'://%e	Экспоненциальное представление ('е' на нижнем регистре)
                case 'E'://%E	Экспоненциальное представление ('Е' на верхнем регистре)
                case 'f'://%f	Десятичное с плавающей точкой
                case 'g'://%g	В зависимости от того, какой вывод будет короче, используется %е или %f
                case 'G'://%G	В зависимости от того, какой вывод будет короче, используется %Е или %F

                    cast = ObjType::Float64;
                    if (!canCast((*args)[aind].second, cast)) {
                        ASSERT((*args)[aind].second->m_type);
                        NL_MESSAGE(LOG_LEVEL_INFO, (*args)[aind].second, "Fail cast from '%s' to '%s'!", (*args)[aind].second->m_type->m_text.c_str(), newlang::toString(cast));
                        result = false;
                    }
                    break;

                case 'c':
                    cast = ObjType::Int8;
                    if (pos && (format[pos - 1] == 'l' || format[pos - 1] == 'L')) {
                        cast = ObjType::Int32;
                    }
                    if (!canCast((*args)[aind].second, cast)) {
                        ASSERT((*args)[aind].second->m_type);
                        NL_MESSAGE(LOG_LEVEL_INFO, (*args)[aind].second, "Fail cast from '%s' to '%s'!", (*args)[aind].second->m_type->m_text.c_str(), newlang::toString(cast));
                        result = false;
                    }
                    break;

                case 's':
                    cast = ObjType::StrChar;
                    if (pos && (format[pos - 1] == 'l' || format[pos - 1] == 'L')) {
                        cast = ObjType::StrWide;
                    }
                    if (!canCast((*args)[aind].second, cast)) {
                        ASSERT((*args)[aind].second->m_type);
                        NL_MESSAGE(LOG_LEVEL_INFO, (*args)[aind].second, "Fail cast from '%s' to '%s'!", (*args)[aind].second->m_type->m_text.c_str(), newlang::toString(cast));
                        result = false;
                    }
                    break;


                case 'p':
                    cast = ObjType::Pointer;
                    if (!canCast((*args)[aind].second, cast)) {
                        ASSERT((*args)[aind].second->m_type);
                        NL_MESSAGE(LOG_LEVEL_INFO, (*args)[aind].second, "Fail cast from '%s' to '%s'!", (*args)[aind].second->m_type->m_text.c_str(), newlang::toString(cast));
                        result = false;
                    }
                    break;

                default:
                    NL_MESSAGE(LOG_LEVEL_INFO, args, "Format modifier '%c' at pos %d in '%s' not supported!", format[pos], (int) pos, format.begin());
                    result = false;
            }
        }
        pos++;
        aind++; // Следующий аргумент
    }

    if (aind < args->size()) {
        NL_MESSAGE(LOG_LEVEL_INFO, (*args)[aind].second, "Extra arguments more %d!", aind);

        return false;
    }

    return result;
}

/*
 * 
 * 
 * 
 */
bool AstAnalysis::CheckError(bool result) {
    if (!result) {
        m_diag->m_error_count++;
        if (m_diag->m_error_limit && m_diag->m_error_count >= m_diag->m_error_limit) {

            LOG_CUSTOM_ERROR(ParserError, "fatal error: too many errors emitted %d, stopping now [-nlc-error-limit=]", m_diag->m_error_count);
        }
    }
    return result;
}

bool AstAnalysis::Analyze(TermPtr &term, TermPtr & module) {
    m_diag->m_error_count = 0;
    ScopeStack stack(module->m_int_vars);

    //    if (is_main) {
    //        // Main module name - empty string
    //        term->m_module = Term::Create(parser::token_type::END, TermID::STRCHAR, "");
    //    }
    try {
        RecursiveAnalyzer(term, stack);
        if (m_diag->m_error_count) {
            LOG_CUSTOM_ERROR(ParserError, "fatal error: %d generated. ", m_diag->m_error_count);
        }
    } catch (ParserError err) {

        return false;
    }
    return !m_diag->m_error_count;
}

TermPtr AstAnalysis::LookupName(TermPtr &term, ScopeStack & stack) {
    if (isReservedName(term->m_text)) {
        term->m_int_name = term->m_text;
        return term;
    }

    TermPtr result = stack.LookupName(term->m_text, &m_rt);
    if (!result) {
        if (isGlobalScope(term->m_text)) {
            result = m_rt.GlobFindProto(term->m_text.c_str());
        }
    }

    if (result) {
        ASSERT(!result->m_int_name.empty());
        term->m_int_name = result->m_int_name;
        if (!CheckOpType(term, result, term)) {

            return nullptr;
        }
    }
    return result;
}

bool AstAnalysis::RecursiveAnalyzer(TermPtr term, ScopeStack & stack) {
    TermPtr found;

    switch (term->getTermID()) {
        case TermID::BLOCK:
        case TermID::BLOCK_PLUS:
        case TermID::BLOCK_MINUS:
        case TermID::BLOCK_TRY:
        {
            //            int step = 1;
            //            LOG_TEST("Block '%s'", term->m_namespace ? term->m_namespace->m_text.c_str() : "");

            ScopePush block_scope(stack, term->m_namespace);
            for (auto &elem : term->m_block) {
                //                LOG_TEST("%d: %s", step++, elem->m_text.c_str());
                // Возможные ошибки подсчитываются внутри RecursiveAnalyzer
                RecursiveAnalyzer(elem, stack);
            }
            return true;
        }

        case TermID::NAME:
        case TermID::TYPE:
        case TermID::ARGS:
        case TermID::ARGUMENT:
        case TermID::LOCAL:
        case TermID::STATIC:
        case TermID::MODULE:
        case TermID::NAMESPACE:
        {
            found = LookupName(term, stack);
            if (CheckError(!!found)) {

                ASSERT(!found->m_int_name.empty());
                term->m_int_name = found->m_int_name;

            } else {
                NL_MESSAGE(LOG_LEVEL_INFO, term, "NameLookup fail for '%s'! %s", term->m_text.c_str(), stack.Dump().c_str());
                return false;
            }

            if (term->isCall()) {
                return CheckError(CheckCall(found, term, stack));
            }
            return true;
        }

        case TermID::OP_MATH:
            return CheckError(
                    RecursiveAnalyzer(term->m_left, stack)
                    && RecursiveAnalyzer(term->m_right, stack)
                    && CheckOpType(term, term->m_left, term->m_right)
                    && UpcastOpType(term));

        case TermID::OP_COMPARE:
            return CheckError(
                    RecursiveAnalyzer(term->m_left, stack)
                    && RecursiveAnalyzer(term->m_right, stack));

        case TermID::INT_PLUS:
        case TermID::INT_MINUS:
        case TermID::INT_REPEAT:
        {
            if (term->Right() && !RecursiveAnalyzer(term->Right(), stack)) {
                return false;
            }
            if (term->m_namespace) {
                return CheckError(stack.LookupBlock_(term->m_namespace));
            }
            return true;
        }

        case TermID::EVAL:
        {
            if (!m_rt.m_eval_enable) {
                NL_MESSAGE(LOG_LEVEL_INFO, term, "Used flag '--nlc-no-eval-enable' to disabled the eval statement!");
                return false;
            }
            return true;
        }

        case TermID::ASSIGN:
        case TermID::CREATE_ONCE:
        case TermID::CREATE_OVERLAP:
        case TermID::PURE_OVERLAP:
            return CheckError(CreateOp_(term, stack));

        case TermID::DICT:
        case TermID::TENSOR:
        case TermID::STRWIDE:
        case TermID::STRCHAR:
        case TermID::INTEGER:
        case TermID::NUMBER:
        case TermID::RATIONAL:
        case TermID::END:
            return true;

        case TermID::FOLLOW:
            return CheckError(CheckFollow_(term, stack));
        case TermID::MATCHING:
            return CheckError(CheckMatching_(term, stack));
        case TermID::WHILE:
            return CheckError(CheckWhile_(term, stack));
        case TermID::DOWHILE:
            return CheckError(CheckDoWhile_(term, stack));
        case TermID::RANGE:
            return CheckError(CkeckRange_(term, stack));
        case TermID::WITH:
            return CheckError(CheckWith_(term, stack));
        case TermID::TAKE:
            return CheckError(CheckTake_(term, stack));
        case TermID::ITERATOR:
            return CheckError(Iterator_(term, stack));

        default:
            NL_MESSAGE(LOG_LEVEL_INFO, term, "AstRecursiveAnalyzer for type '%s' not implemented!", toString(term->getTermID()));
    }
    return false;
}
