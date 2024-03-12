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

bool AstAnalysis::CheckError(bool result) {
    if (!result) {
        m_diag->m_error_count++;
        if (m_diag->m_error_limit && m_diag->m_error_count >= m_diag->m_error_limit) {
            LOG_CUSTOM_ERROR(ParserError, "fatal error: too many errors emitted %d, stopping now [-nlc-error-limit=]", m_diag->m_error_count);
        }
    }
    return result;
}

bool AstAnalysis::Analyze(TermPtr &term, TermPtr &module) {
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
    return true;
}

TermPtr AstAnalysis::LockupName(TermPtr &term, ScopeStack &stack) {
    if (isReservedName(term->m_text)) {
        term->m_int_name = term->m_text;
        return term;
    }

    TermPtr result = stack.LookupName(term->m_text, &m_rt);
    //    if (!result) {
    //        if (isGlobalScope(term->m_text)) {
    //            result = this->GlobFindProto(term->m_text.c_str());
    //        }
    //    }
    if (result) {
        if (!CheckType(result, term)) {
            return nullptr;
        } else {
            term->m_int_name = result->m_int_name;
        }
    }
    return result;
}

bool AstAnalysis::RecursiveAnalyzer(TermPtr &term, ScopeStack & stack) {
    TermPtr found;
    if (term->isLiteral()) {

        return true;

    } else if (term->getTermID() == TermID::STATIC || term->getTermID() == TermID::LOCAL || term->getTermID() == TermID::NAME
            || term->getTermID() == TermID::NAMESPACE || term->getTermID() == TermID::FIELD || term->getTermID() == TermID::MODULE
            || term->getTermID() == TermID::FUNCTION || term->getTermID() == TermID::TYPE) {

        found = LockupName(term, stack);
        if (CheckError(!!found)) {

            ASSERT(!found->m_int_name.empty());
            term->m_int_name = found->m_int_name;

        } else {
            NL_MESSAGE(LOG_LEVEL_INFO, term, "NameLookup fail for '%s'! %s", term->m_text.c_str(), stack.Dump().c_str());
        }

        if (term->isCall()) {
            return CheckError(CheckCall(found, term, stack));
        }
        return true;

    } else if (term->isInterrupt()) {

        return CheckError(stack.LookupBlock(term));

    } else if (term->isCreate()) {

        return CheckError(CreateOp_(term, stack));

    } else if (term->getTermID() == TermID::OP_MATH) {

        return CheckError(
                RecursiveAnalyzer(term->m_left, stack)
                && RecursiveAnalyzer(term->m_right, stack)
                && CheckType(term->m_left, term->m_right)
                && UpcastOpType(term)
                );

    } else if (term->isBlock()) {

        ScopePush block_scope(stack, term->m_namespace);
        for (auto &elem : term->m_block) {
            RecursiveAnalyzer(elem, stack);
        }
        return true;

    } else if (term->getTermID() == TermID::FOLLOW) {

        ASSERT(term->m_right);
        return CheckError(RecursiveAnalyzer(term->m_right, stack));

    } else if (term->getTermID() == TermID::RANGE) {

        return CheckError(CkeckRange_(term, stack));

    } else if (term->getTermID() == TermID::ITERATOR) {

        return CheckError(Iterator_(term, stack));

    } else {
        NL_MESSAGE(LOG_LEVEL_INFO, term, "AstRecursiveAnalyzer fo type '%s' not implemented!", toString(term->getTermID()));
    }
    return false;
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

    TermPtr found = LockupName(term->m_left, stack);

    if (term->m_text.compare("!?") == 0 || term->m_text.compare("?!") == 0) {

        return !!found && CheckError(CheckType(term, getDefaultType(ObjType::Dictionary)));

    } else if (term->m_text.compare("?") == 0) {
    } else if (term->m_text.compare("!") == 0) {
    } else if (term->m_text.compare("??") == 0) {
    } else if (term->m_text.compare("!!") == 0) {
    } else {
        NL_MESSAGE(LOG_LEVEL_INFO, term, "Unknown iterator '%s'!", term->m_text.c_str());
        return false;
    }
    NL_MESSAGE(LOG_LEVEL_INFO, term, "Iterator '%s' not implemented!", term->m_text.c_str());

    return false;
}

bool AstAnalysis::CheckType(TermPtr &left, const TermPtr right) {

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

bool AstAnalysis::CreateOp_(TermPtr &op, ScopeStack & stack) {

    ASSERT(op);
    ASSERT(op->m_left);
    ASSERT(op->m_right);

    if (!op->m_left->isCall()) {
        // Создание переменной
        TermPtr term = op;
        TermPtr var;
        std::string int_name;
        bool name_exist;
        while (term->m_left) {

            term = op->m_left;
            if (!term->m_type && op->m_right->m_type) {
                term->m_type = op->m_right->m_type;
            }
            int_name = stack.CreateVarName(term->m_text);
            var = LockupName(term, stack);

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
            }

            if (op->m_right->isNamed()) {
                TermPtr r_var = LockupName(op->m_right, stack);
                if (!r_var) {
                    NL_MESSAGE(LOG_LEVEL_INFO, op->m_right, "Name '%s' not exist!", op->m_right->m_text.c_str());
                    return false;
                }
                op->m_right->m_int_name = r_var->m_int_name;
            }

            if (op->m_right->getTermID() == TermID::NATIVE) {
                return CheckNative_(term, op->m_right);
            } else {
                return CheckType(term, op->m_right);
            }



        }
        return true;

    } else {
        // Создание функции
        if (op->m_left->m_left) {
            NL_MESSAGE(LOG_LEVEL_INFO, op->m_left->m_left, "Multiple function creation is not supported!");
            return false;
        }

        TermPtr proto = op->m_left;

        proto->m_id = TermID::FUNCTION;
        proto->m_int_name = stack.CreateVarName(proto->m_text);

        ScopePush block_args(stack, proto, nullptr, true);

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
                proto->at(i).first = proto->at(i).second->m_text;
                all->push_back({stack.CreateVarName(proto->at(i).second->m_text), proto->at(i).second});

                TermPtr none = Term::CreateNil();
                none->m_int_name = stack.CreateVarName(proto->at(i).second->m_text);
                if (!stack.AddName(none)) {
                    return false;
                }
            } else {
                proto->at(i).second->m_int_name = stack.CreateVarName(proto->at(i).first);
                all->push_back({stack.CreateVarName(proto->at(i).first), proto->at(i).second});
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

        if (!stack.AddName(proto)) {
            return false;
        }

        ScopePush block(stack, proto, &proto->m_int_vars, true);

        TermPtr none = Term::CreateNil();
        none->m_int_name = stack.CreateVarName("$0");

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
            return CheckStrFormat(proto->m_text, call, 0);
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

                if (!canCast(proto->at(pos).second, call->at(pos).second)) {
                    ASSERT(proto->at(pos).second->GetType());
                    ASSERT(call->at(pos).second->GetType());
                    NL_MESSAGE(LOG_LEVEL_INFO, proto->at(pos).second, "Fail cast from '%s' to '%s'!",
                            call->at(pos).second->GetType()->m_text.c_str(), proto->at(pos).second->GetType()->m_text.c_str());
                    return false;
                }


                if (proto->at(pos).second->GetType()) {
                    if (proto->at(pos).second->GetType()->m_text.compare(":FmtChar") == 0 || proto->at(pos).second->GetType()->m_text.compare(":FmtWide") == 0) {
                        if (!CheckStrPrintf(call->at(pos).second->m_text, call, pos + 1)) {
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
        } else if (pos > call->size()) {

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

        if (arg_pos + 1 != call->size()) {
            NL_MESSAGE(LOG_LEVEL_INFO, fill_obj, "Filling is supported for the last argument only!");
            return false;
        }

        if (!call->m_dims.size()) {
            NL_MESSAGE(LOG_LEVEL_INFO, call, "Object has no dimensions!");
            return false;
        }

        int64_t full_size = 1;
        for (int dim_index = 0; dim_index < call->m_dims.size(); dim_index++) {

            ObjPtr size = Context::EvalTerm(call->m_dims[dim_index], nullptr);
            if (!size || !size->is_integer()) {
                NL_MESSAGE(LOG_LEVEL_INFO, call->m_dims[dim_index], "Dimension index support integer value only!");
                return false;
            }
            full_size *= size->GetValueAsInteger();
        }

        if (full_size <= 0) {
            NL_MESSAGE(LOG_LEVEL_INFO, fill_obj, "Items count '%ld' error for all dimensions!", full_size);
            return false;
        }

        if (fill_obj->size()) {
            NL_MESSAGE(LOG_LEVEL_INFO, fill_obj, "Argument in function for filling not implemented!");
            return false;
        }

        if (fill_obj->m_type) {
            if (call->m_type) {
                if (!canCast(fill_obj->m_type, call->m_type)) {
                    NL_MESSAGE(LOG_LEVEL_INFO, fill_obj, "Fail cast type '%s' to '%s' type!", fill_obj->m_type->m_text.c_str(), call->m_type->m_text.c_str());
                    return false;
                }
            } else {
                call->m_type = fill_obj->m_type;
            }
        }

        return RecursiveAnalyzer(fill_obj, stack);

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
                if (!canCast(arg->m_type, call->m_type)) {
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

bool AstAnalysis::CheckStrFormat(const std::string_view format, TermPtr args, int start) {
    ASSERT(args);
    NL_MESSAGE(LOG_LEVEL_INFO, args, "Check std::vformat not implemented!");

    return false;
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
