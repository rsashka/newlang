#include "pch.h"
#include <term.h>
#include <runtime.h>

using namespace newlang;

size_t newlang::IndexArg(TermPtr term) {
    ASSERT(term);
    ASSERT(term->m_id == TermID::ARGUMENT);
    ASSERT(!term->m_text.empty());
    char * pEnd = nullptr;
    size_t index = std::strtoul(term->getText().data() + 1, &pEnd, 10);
    if (term->m_text.data() + term->getText().size() != pEnd) {
        LOG_RUNTIME("Float error '%s' or out of range!", term->getText().c_str());
    }
    return index;
}

BlockType Term::MakeMacroId(const BlockType &seq) {

    BlockType result;
    size_t pos = 0;
    TermPtr term;
    size_t done;

    while (pos < seq.size()) {

        if (Macro::deny_chars_from_macro.find(seq[pos]->m_text[0]) != std::string::npos) {
            NL_PARSER(seq[pos], "Symbol '%c' in lexem sequence not allowed!", seq[pos]->m_text[0]);
        }

        done = Parser::ParseTerm(term, seq, pos);
        if (done) {
            result.push_back(term);
            pos += done;
        } else {
            LOG_RUNTIME("Fail convert %s", seq[pos]->toString().c_str());
        }
    }
    //    std::vector<std::string> result;
    //    for (auto &elem : m_macro_id) {
    //        result.push_back(MacroBuffer::toHashTermName(elem->m_text));
    //    }
    return result;
}

BlockType Term::GetMacroId() {
    if (!isMacro()) {
        LOG_RUNTIME("Term '%s' as %s not a macro!", toString().c_str(), newlang::toString(m_id));
    }

    if (m_macro_id.empty()) {

        if (m_id == TermID::MACRO_DEL) {
            ASSERT(m_macro_seq.size());

            m_macro_id = MakeMacroId(m_macro_seq);

        } else {
            ASSERT(isCreate());
            ASSERT(m_left);
            ASSERT(m_left->m_id == TermID::MACRO_SEQ);
            ASSERT(m_left->m_macro_seq.size());

            m_macro_id = MakeMacroId(m_left->m_macro_seq);
            m_left->m_macro_id = m_macro_id;
        }

        ASSERT(m_macro_id.size());
    }
    return m_macro_id;
}

/*
 * 
 * 
 */
//ObjPtr NameList::NameGet(const char *name, bool is_raise) {
//
//    TermPtr ret = NameFind(name);
//
//    if (ret) {
//        return ret;//->obj;
////        if (at::holds_alternative<ObjPtr>(ret->obj)) {
////            return at::get<ObjPtr>(ret->obj).lock();
////        } else if (at::holds_alternative<std::vector < ObjPtr >> (ret->obj)) {
////            return at::get<std::vector < ObjPtr >> (ret->obj)[0].lock();
////        }
//        if (is_raise) {
//            NL_PARSER(ret->item, "Global name not implemented! '%s'", name);
//        }
//    } else {
//        if (is_raise) {
//            LOG_RUNTIME("Global Name '%s' not found!", name);
//        }
//    }
//    return nullptr;
//}

//std::string NameList::Dump() {
//    std::string result;
//
//    for (auto &elem : * this) {
//        result += '\n';
//        result += elem.first;
//    }
//
//    return result;
//}

TermPtr Term::CreateNone() {
    TermPtr result = Term::Create(parser::token_type::END, TermID::NONE, "_");
    result->m_obj = Obj::CreateNone();
    return result;
}

TermPtr Term::CreateNil() {
    TermPtr result = Term::Create(parser::token_type::END, TermID::NONE, "_");
    result->m_obj = nullptr;
    return result;
}

TermPtr Term::CreateDict() {
    TermPtr result = Term::Create(parser::token_type::END, TermID::DICT, "");
    return result;
}

TermPtr Term::CreateName(std::string name) {
    TermPtr result = Term::Create(parser::token_type::NAME, TermID::NAME, name.c_str());
    return result;
}

/*
 * 
 * 
 */
bool Term::CheckTermEq(const TermPtr &term1, const TermPtr &term2, bool type, RuntimePtr rt) {
    if (term1 == nullptr && term2 == nullptr) {
        return true;
    }
    if (!!term1 != !!term2) {
        if (type) {
            return true;
        }
        return false;
    }
    if (term1->m_text.compare(term2->m_text) != 0) {
        return false;
    }
    if (term1->isCall() != term2->isCall()) {
        return false;
    }
    if (term1->m_dims.size() != term2->m_dims.size()) {
        return false;
    }
    for (int i = 0; i < term1->m_dims.size(); i++) {
        if (!CheckTermEq(term1->m_dims[i], term2->m_dims[i], false, rt)) {
            return false;
        }
    }
    if (term1->size() != term2->size()) {
        return false;
    }
    for (int i = 0; i < term1->size(); i++) {
        if (term1->at(i).first.compare(term2->at(i).first) != 0) {
            return false;
        }
        if (!CheckTermEq(term1->at(i).second, term2->at(i).second, false, rt)) {
            return false;
        }
    }
    return CheckTermEq(term1->m_type, term2->m_type, true, rt);
}

//bool Term::CheckArgsCall(TermPtr &term, RuntimePtr rt) {
//    ASSERT(term);
//    if (rt) {
//        GlobItem * ret = rt->NameFind(term->m_text.c_str());
//        if (ret) {
//            return CheckArgsProto(term, ret->proto);
//        }
//        NL_PARSER(term, "Prototype name '%s' not found!", term->m_text.c_str());
//    }
//    return CheckArgsProto(term, nullptr);
//}
//
//bool Term::CheckArgsProto(TermPtr &term, const TermPtr proto) {
//
//    ASSERT(term);
//    std::string name(term->m_text);
//
//    if (proto) {
//
//        if (isNativeName(proto->m_text)) {
//            if (!RunTime::GetDirectAddressFromLibrary(nullptr, proto->m_text.c_str())) {
//                NL_PARSER(proto, "Native name '%s' not found!", proto->m_text.c_str());
//            }
//            if (proto->Right()) {
//                // printf(fmt:FmtStr, ...) := %printf ...; 
//                if (proto->Right()->getTermID() != TermID::ELLIPSIS) {
//                    NL_PARSER(proto->Right(), "Unexpected term '%s' in native name!", proto->Right()->m_text.c_str());
//                }
//                return true;
//            }
//        } else if (isLocalAnyName(proto->m_text.c_str())) {
//            NL_PARSER(proto, "Unexpected use name '%s' as prototype!", proto->m_text.c_str());
//        }
//
//        // printf(fmt:FmtStr, ...) := %printf(fmt:FmtStr, ...); 
//        return CheckCompareArgs_(term, proto);
//
//    } else {
//
//        if (isNativeName(term->m_text)) {
//            NL_PARSER(term, "Cannot use a native name as LValue !");
//        } else if (isMacroName(term->m_text)) {
//            NL_PARSER(term, "Macro name must be expanded!");
//        } else if (!isLocalAnyName(term->m_text.c_str())) {
//            NL_PARSER(term, "Unexpected term name!");
//        }
//
//    }
//    return true;
//}
//
//bool Term::CheckCompareArgs_(const TermPtr &term, const TermPtr &proto) {
//
//    return false;
//
//    //      
//    //    void Obj::ConvertToArgs_(Obj *in, bool check_valid, Context * ctx)
//    //
//    //    ASSERT(in);
//    //
//    //    //    bool named = false;
//    //    bool is_ellipsis = false;
//    //    if (check_valid && size()) {
//    //        if (at(size() - 1).first.compare("...") == 0) {
//    //            is_ellipsis = true;
//    //            Variable::erase(size() - 1);
//    //        }
//    //    }
//    //    for (int i = 0; i < in->size(); i++) {
//    //
//    //        if (isSystemName(in->name(i))) {
//    //            continue;
//    //        }
//    //
//    //        if (in->name(i).empty()) {
//    //            //            if(check_valid && named) {
//    //            //                LOG_RUNTIME("Position %d requires a named argument!", (int) i + 1);
//    //            //            }
//    //            ObjType base_type = ObjType::None;
//    //            if (i < size()) {
//    //                if (ctx) {
//    //                    if ((*m_prototype)[i].second->m_type_name.empty()) {
//    //                        base_type = ObjType::Any;
//    //                    } else {
//    //                        bool has_error = false;
//    //                        base_type = ctx->m_runtime->BaseTypeFromString((*m_prototype)[i].second->m_type_name, &has_error);
//    //                        if (has_error && (*m_prototype)[i].second->getTermID() == TermID::ELLIPSIS) {
//    //                            base_type = ObjType::Any;
//    //                        }
//    //                    }
//    //                } else if (!(*m_prototype)[i].second->m_type_name.empty()) {
//    //                    base_type = typeFromString((*m_prototype)[i].second->m_type_name, ctx);
//    //                } else {
//    //                    base_type = ObjType::Any;
//    //                }
//    //            } else {
//    //                base_type = ObjType::Any;
//    //            }
//    //
//    //
//    //            if (i < size()) {
//    //                if (check_valid && at(i).second && at(i).second->getType() != ObjType::None) {
//    //                    if (!canCast((*in)[i].second->getType(), at(i).second->getType())) {
//    //                        LOG_RUNTIME("Fail cast value '%s' to type '%s'", newlang::toString((*in)[i].second->getType()),
//    //                                newlang::toString(at(i).second->getType()));
//    //                    }
//    //                }
//    //                if (!at(i).second) {
//    //                    at(i).second = Obj::CreateNone();
//    //                }
//    //                if (m_prototype && i < m_prototype->size()) {
//    //                    at(i).second->m_is_reference = (*m_prototype)[i].second->isRef();
//    //                    ObjType limit_type = (*in)[i].second->getTypeAsLimit();
//    //                    if (!canCast(limit_type, base_type)) {
//    //                        // Строку с одним символом можно преобразовать в арифметичсекий тип
//    //                        if (!(isArithmeticType(base_type) && (*in)[i].second->is_string_type() && (*in)[i].second->size() == 1)) {
//    //                            LOG_RUNTIME("Fail cast value %s%s to type %s",
//    //                                    (*in)[i].second->toString().c_str(),
//    //                                    newlang::toString((*in)[i].second->getType()),
//    //                                    (*m_prototype)[i].second->m_type_name.c_str());
//    //                        }
//    //                    }
//    //                }
//    //
//    //                //                LOG_DEBUG("%s", (*in)[i].second->toString().c_str());
//    //
//    //                at(i).second->op_assign((*in)[i].second->toType(base_type));
//    //            } else {
//    //                if (check_valid && !is_ellipsis && m_prototype && i >= m_prototype->size()) {
//    //                    LOG_RUNTIME("Positional args overflow. Ptrototype '%s'!",
//    //                            m_prototype ? m_prototype->toString().c_str() : "Prototype not exists!");
//    //                }
//    //                push_back(in->at(i).second->toType(base_type), in->at(i).first);
//    //            }
//    //        } else {
//    //            //            named = true;
//    //            auto found = find(in->name(i));
//    //            if (found != end()) {
//    //                if (check_valid && (*found).second && (*found).second->getType() != (*in)[i].second->getType() && (*found).second->getType() != ObjType::None) {
//    //                    LOG_RUNTIME("Different type arg '%s' and '%s'", (*found).second->toString().c_str(),
//    //                            (*in)[i].second->toString().c_str());
//    //                }
//    //                //@todo  Проверка ограничений размер данных при указаном типе
//    //                if (!(*found).second) {
//    //                    (*found).second = Obj::CreateNone();
//    //                }
//    //                (*found).second->op_assign((*in)[i].second);
//    //            } else {
//    //                for (int pos = 0; pos < size(); pos++) {
//    //                    if (!at(pos).first.empty() && at(pos).first.compare(in->at(i).first) == 0) {
//    //                        at(pos).second->op_assign(in[i]);
//    //                        goto done;
//    //                    }
//    //                }
//    //                if (check_valid && !is_ellipsis) {
//    //                    LOG_RUNTIME("Named arg '%s' not found!", in->name(i).c_str());
//    //                }
//    //                push_back(in->at(i));
//    //done:
//    //                ;
//    //            }
//    //        }
//    //    }
//    //    if (check_valid) {
//    //
//    //        CheckArgsValid();
//    //    }
//}

//struct TraversingParam {
//    TermPtr root;
//    std::vector < NodeHandlerFunc *> handlers;
//    void * obj;
//};
//
//void TraversingNodesExecuter(TermPtr &term, TraversingParam &param) {
//    ASSERT(term);
//    ASSERT(!term->m_list);
//    ASSERT(!term->m_sequence);
//
//    for (auto &func : param.handlers) {
//        (*func)(term, param.obj);
//    }
//    if (term->m_left) {
//        TraversingNodesExecuter(term->m_left, param);
//    }
//    if (term->m_right) {
//        TraversingNodesExecuter(term->m_right, param);
//    }
//    for (auto &item : term->m_block) {
//        TraversingNodesExecuter(item, param);
//    }
//    for (auto &item : term->m_follow) {
//        TraversingNodesExecuter(item, param);
//    }
//}
//
//void Term::TraversingNodes(TermPtr &ast, NodeHandlerList h, void * obj) {
//
//    TraversingParam param;
//
//    param.root = ast;
//    param.handlers = h;
//    param.obj = obj;
//
//    TraversingNodesExecuter(ast, param);
//}

bool ScopeBlock::NameMacroExpand(TermPtr term) {
    return false;
}

void ScopeBlock::PushScope(TermPtr ns, TermStorage * storage) {
    Block block;
    if (ns) {
        block.scope_name = ns;
        if (ns->m_text.rfind("::") != ns->m_text.size() - 2) {
            ns->m_text += "::";
        }
    } else {
        block.scope_name = Term::CreateName(std::to_string(m_block_num));
        block.scope_name->m_text += "::";
        m_block_num++;
    }

    block.storage = storage;
    m_stack.push_back(block);
}

std::string ScopeBlock::ExpandNamespace(std::string name) {
    size_t pos = name.find("@::");
    if (pos != std::string::npos) {
        name = name.replace(pos, 3, GetNamespace());
    }
    return name;
}

std::string ScopeBlock::MakeNamespace(std::vector<Block>& stack, size_t skip, bool is_global) {
    std::string result;
    auto iter = stack.rbegin();
    iter += skip;
    while (iter != stack.rend()) {
        if (result.find("::") == 0) {
            break;
        }
        ASSERT(iter->scope_name);
        if (!is_global || !isdigit(iter->scope_name->m_text[0])) {
            // The namespace is always padded with ::
            result.insert(0, iter->scope_name->m_text);
        }
        iter++;
    }
//    if (!result.empty() && (result.size() < 2 || result.rfind("::") != result.size() - 2)) {
//        result += "::";
//    }
    return result;
}

std::string ScopeBlock::GetNamespace(bool is_global) {
    std::string result;
    auto iter = m_stack.rbegin();
    while (iter != m_stack.rend()) {
        if (result.find("::") == 0) {
            break;
        }
        ASSERT(iter->scope_name);
        // An unnamed block of code has a numeric name, 
        // and identifiers with a number at the beginning 
        //  of the name are prohibited by the language syntax.
        if (!is_global || !isdigit(iter->scope_name->m_text[0])) {
            // The namespace is always padded with ::
            result.insert(0, iter->scope_name->m_text);
        }
        iter++;
    }
//    if (!result.empty() && (result.size() < 2 || result.rfind("::") != result.size() - 2)) {
//        result += "::";
//    }
    return result;
}

std::string ScopeBlock::CreateVarName(const std::string_view name) {

    std::string result(NormalizeName(name));

    if (!isGlobalScope(name)) {
        result = result.insert(0, GetNamespace(isTypeName(name)));
    }
    return result;
}

bool ScopeBlock::AddName(const TermPtr var, const char * alt_name) {

    ASSERT(var);

    std::string name;
    if (alt_name) {
        name = alt_name;
    } else {
        name = var->m_int_name;
    }
    if (name.empty()) {
        LOG_RUNTIME("Internal name of '%s' not exist!", var->toString().c_str());
    }
    if (!m_stack.empty()) {
        if (m_stack.back().vars.find(name) != m_stack.back().vars.end()) {
            NL_MESSAGE(LOG_LEVEL_INFO, var, "Var '%s' exist!", name.c_str());
            return false;
        }
        m_stack.back().vars.insert({name, var});
    }

    TermStorage *stor = getStorage_();

    if (stor) {
        if (stor->find(name) != stor->end()) {
            NL_MESSAGE(LOG_LEVEL_INFO, var, "Var '%s' exist!", name.c_str());
            return false;
        }
        stor->insert({name, var});
    }

    if (m_stack.empty() || isStaticName(name)) {
        //        if (this->find(name) != this->end()) {
        //            NL_MESSAGE(LOG_LEVEL_INFO, var, "Var '%s' exist!", name.c_str());
        //            return false;
        //        }
        //        this->insert({name, var});
    }
    //    if ((m_stack.empty() || isStaticName(name)) && m_module) {
    //        return m_module->AddName(var);
    //    }
    return true;
}

TermPtr ScopeBlock::FindVar(std::string int_name, bool local_only) {

    int_name = NormalizeName(int_name);
    ASSERT(isInternalName(int_name));

    TermStorage *stor = getStorage_();
    if (stor && stor != m_module) {
        if (stor->find(int_name) != stor->end()) {
            return stor->find(int_name)->second;
        }
    } else if (m_module && !local_only && m_module->find(int_name) != m_module->end()) {
        return m_module->find(int_name)->second;
    }
    return nullptr;
}

TermPtr ScopeBlock::LookupVar(std::string name, bool local_only) {

    TermPtr found;
    if ((found = FindVar(name, local_only))) {
        return found;
    }

    bool full_search = isTrivialName(name);
    if (!full_search) {
        name = NormalizeName(name);
    }
    std::string temp;
    for (size_t skip = 0; skip < m_stack.size(); skip++) {
        // Check local name
        temp = name;
        if (full_search) {
            temp += "$";
        }
        temp.insert(0, MakeNamespace(m_stack, skip, false));
        if ((found = FindVar(temp))) {
            return found;
        }
        if (full_search) {

            // Check static name local object
            temp = name;
            temp += "::";
            temp.insert(0, MakeNamespace(m_stack, skip, false));
            if ((found = FindVar(temp))) {
                return found;
            }
        }

        // Check static name
        temp = name;
        if (full_search) {
            temp += "::";
        }
        temp.insert(0, MakeNamespace(m_stack, skip, true));
        if ((found = FindVar(temp))) {
            return found;
        }


        if (full_search) {
            // Check type name
            temp = name;
            temp += ":::";
            temp.insert(0, MakeNamespace(m_stack, skip, true));
            if ((found = FindVar(temp))) {
                return found;
            }
        }

        //        if (isGlobalScope(temp)) {
        //            break;
        //        }
    }
    return nullptr;
}

bool ScopeBlock::LookupBlock(TermPtr & term) {
    if (!term || term->m_text.compare("::") == 0) {
        return true;
    }
    for (auto &elem : m_stack) {
        if (elem.scope_name && elem.scope_name->m_text.compare(term->m_text) == 0) {
            return true;
        }
    }
    NL_MESSAGE(LOG_LEVEL_INFO, term, "Lookup block '%s' fail!%s", term->m_text.c_str(), GetOfferBlock().c_str());
    return false;
}

std::string ScopeBlock::GetOfferBlock() {
    if (m_stack.empty()) {
        return "";
    }
    std::string result = " Possible block identifiers: '";

    std::string list_block;
    for (auto &elem : m_stack) {
        if (elem.scope_name) {
            if (!list_block.empty()) {
                list_block += ", ";
            }
            list_block += elem.scope_name->m_text;
        }
    }
    result += list_block;
    result += "'";
    return result;
}

std::string ScopeBlock::Dump() {
    std::string result("Storage: ");
    if (m_module) {
        result += m_module->Dump();
    }

    auto iter = m_stack.begin();
    while (iter != m_stack.end()) {
        result += "Stack [";
        if (iter->scope_name) {
            result += iter->scope_name->m_text;
        }
        result += "]: ";

        std::string list;
        auto iter_list = iter->vars.begin();
        while (iter_list != iter->vars.end()) {

            if (!list.empty()) {
                list += ", ";
            }

            list += iter_list->first;
            iter_list++;
        }

        //        result += "(";
        result += list;
        result += "\n";
        iter++;
    }

    return result;

}

//void ScopeBlock::CloneFrom(const ScopeBlock& obj) {
//    clear();
//    for (auto &elem : obj) {
//        insert({elem.first, Term::Create(elem.second.get())});
//    }
//    m_root = obj.m_root;
//    m_stack.clear();
//    std::map<std::string, TermPtr> temp_vars;
//    for (auto &elem : obj.m_stack) {
//        temp_vars.clear();
//        for (auto &var : elem.vars) {
//            temp_vars.insert({var.first, Term::Create(var.second.get())});
//        }
//        m_stack.push_back({Term::Create(elem.scope_name.get()), temp_vars});
//    }
//}
//
//ScopeBlock::ScopeBlock(const ScopeBlock& obj) {
//    CloneFrom(obj);
//}
//
//ScopeBlock& ScopeBlock::operator=(const ScopeBlock& obj) {
//    CloneFrom(obj);
//    return *this;
//}
