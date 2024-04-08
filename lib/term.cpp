//#include "pch.h"
#include "term.h"
#include "runtime.h"

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

//BlockType Term::MakeMacroId(const BlockType &seq) {
//
//    BlockType result;
//    size_t pos = 0;
//    TermPtr term;
//    size_t done;
//
//    while (pos < seq.size()) {
//
//        if (Macro::deny_chars_from_macro.find(seq[pos]->m_text[0]) != std::string::npos) {
//            NL_PARSER(seq[pos], "Symbol '%c' in lexem sequence not allowed!", seq[pos]->m_text[0]);
//        }
//
//        done = Parser::ParseTerm(term, seq, pos);
//        if (done) {
//            result.push_back(term);
//            pos += done;
//        } else {
//            LOG_RUNTIME("Fail convert %s", seq[pos]->toString().c_str());
//        }
//    }
//    //    std::vector<std::string> result;
//    //    for (auto &elem : m_macro_id) {
//    //        result.push_back(MacroBuffer::toHashTermName(elem->m_text));
//    //    }
//    return result;
//}
//
//BlockType Term::GetMacroId() {
//    if (!isMacro()) {
//        LOG_RUNTIME("Term '%s' as %s not a macro!", toString().c_str(), newlang::toString(m_id));
//    }
//
//    if (m_macro_id.empty()) {
//
//        if (m_id == TermID::MACRO_DEL) {
//            ASSERT(m_macro_seq.size());
//
//            m_macro_id = MakeMacroId(m_macro_seq);
//
//        } else {
//            ASSERT(isCreate());
//            ASSERT(m_left);
//            ASSERT(m_left->m_id == TermID::MACRO_SEQ);
//            ASSERT(m_left->m_macro_seq.size());
//
//            m_macro_id = MakeMacroId(m_left->m_macro_seq);
//            m_left->m_macro_id = m_macro_id;
//        }
//
//        ASSERT(m_macro_id.size());
//    }
//    return m_macro_id;
//}

/*
 * 
 * 
 */

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

TermPtr Term::CreateName(std::string name, TermID id) {
    TermPtr result = Term::Create(parser::token_type::NAME, id, name.c_str());
    return result;
}

void Term::SetType(TermPtr type) {
    if (type) {

        if (type->m_list) {
            m_id = TermID::BLOCK;
            ListToVector(type, m_block);
        }

        ASSERT(!type->m_list);
        //                ASSERT(type->m_type_allowed.empty());
        //                ASSERT(m_type_allowed.empty());
        //                m_type_allowed.push_back(type);
        m_type = type;
        //                m_type_name = m_type->asTypeString();
        // Check type
        if (m_id == TermID::INTEGER) {
            ObjType type_val = typeFromLimit(parseInteger(m_text.c_str()), ObjType::Bool);
            if (!canCastLimit(type_val, typeFromString(m_type, nullptr))) {
                NL_PARSER(type, "Error cast '%s' to integer type '%s'", m_text.c_str(), m_type->m_text.c_str());
            }
        } else if (m_id == TermID::NUMBER) {
            ObjType type_val = typeFromLimit(parseDouble(m_text.c_str()), ObjType::Float64);
            if (!canCastLimit(type_val, typeFromString(m_type, nullptr))) {
                NL_PARSER(type, "Error cast '%s' to numeric type '%s'", m_text.c_str(), m_type->m_text.c_str());
            }
        } else if (m_id == TermID::COMPLEX) {
            //            ObjType type_val = typeFromLimit(parseComplex(m_text.c_str()), ObjType::Complex64);
            //            if (!canCastLimit(type_val, typeFromString(m_type, nullptr))) {
            NL_PARSER(type, "Error cast '%s' to complex type '%s'", m_text.c_str(), m_type->m_text.c_str());
            //            }
        }
    } else {
        //                if (m_type) {
        //                    m_type.reset();
        //                }
        // Default type
        if (m_id == TermID::INTEGER) {
            m_type = getDefaultType(typeFromLimit(parseInteger(m_text.c_str()), ObjType::Bool));
        } else if (m_id == TermID::NUMBER) {
            m_type = getDefaultType(typeFromLimit(parseDouble(m_text.c_str()), ObjType::Float64));
        } else if (m_id == TermID::COMPLEX) {
            NL_PARSER(type, "Error cast '%s' to complex type '%s'", m_text.c_str(), m_type->m_text.c_str());
//            m_type = getDefaultType(typeFromLimit(parseComplex(m_text.c_str()), ObjType::Complex64));
        } else if (m_id == TermID::STRCHAR) {
            m_type = getDefaultType(ObjType::StrChar);
        } else if (m_id == TermID::STRWIDE) {
            m_type = getDefaultType(ObjType::StrWide);
        } else if (m_id == TermID::DICT) {
            m_type = getDefaultType(ObjType::Dictionary);
        } else if (m_id == TermID::RATIONAL) {
            m_type = getDefaultType(ObjType::Rational);
        } else if (m_id == TermID::RANGE) {
            m_type = getDefaultType(ObjType::Range);
        } else if (m_id == TermID::ITERATOR) {
            m_type = getDefaultType(ObjType::Iterator);
            //                } else if (m_id == TermID::INT_MINUS || m_id == TermID::INT_PLUS || m_id == TermID::INT_REPEAT) {
            //                    m_type = getDefaultType(ObjType::Interruption);
        }
    }
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
    if (!!term1->m_dims != !!term2->m_dims) {
        return false;
    }
    if (term1->m_dims && term2->m_dims && term1->m_dims->size() != term2->m_dims->size()) {
        return false;
    }
    for (int i = 0; term1->m_dims && i < term1->m_dims->size(); i++) {
        if (!CheckTermEq(term1->m_dims->at(i).second, term2->m_dims->at(i).second, false, rt)) {
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

void ScopeStack::PushScope(TermPtr ns, StorageTerm * storage, bool transaction) {

    if (transaction != m_is_transaction) {
        if (!transaction) {
            m_transaction.clear();
        }
        m_is_transaction = transaction;
    }

    ScopeVars block;
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
    push_back(block);
}

std::string ScopeStack::ExpandNamespace(std::string name) {
    size_t pos = name.find("@::");
    if (pos != std::string::npos) {
        name = name.replace(pos, 3, GetNamespace());
    }
    return name;
}

bool ScopeStack::CheckInterrupt(std::string_view name) {
    std::string ns(name);

    if (!(ns.size() > 1 && ns[ns.size() - 1] == ':' && ns[ns.size() - 2] == ':')) {
        ns += "::";
    }
    auto iter = rbegin();
    while (iter != rend()) {
        if (iter->scope_name && iter->scope_name->m_namespace && iter->scope_name->m_namespace->m_text.compare(ns.c_str()) == 0) {
            return iter == rbegin();
        }
        iter++;
    }
    LOG_RUNTIME("Named block '%s' not found!", name.begin());
}

std::string ScopeStack::MakeNamespace(int skip, bool is_global) {
    std::string result;
    auto iter = rbegin();
    if (skip > 0) {
        iter += skip;
    }
    int count = 0;
    while (iter != rend()) {
        if (result.find("::") == 0) {
            break;
        }
        if (skip < 0 && count == -skip) {
            break;
        }
        ASSERT(iter->scope_name);
        if (!is_global || !isdigit(iter->scope_name->m_text[0])) {
            // The namespace is always padded with ::
            result.insert(0, iter->scope_name->m_text);
        }
        iter++;
        count++;
    }
    return result;
}

std::string ScopeStack::GetNamespace(bool is_global) {
    std::string result;
    auto iter = rbegin();
    while (iter != rend()) {
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

std::string ScopeStack::CreateVarName(const std::string_view name) {

    std::string result(NormalizeName(name));

    if (!isGlobalScope(name)) {
        result = result.insert(0, GetNamespace(isTypeName(name)));
    }
    return result;
}

void ScopeStack::RemoveName_(const std::string_view int_name) {
    auto iter = rbegin();
    while (iter != rend()) {
        if (iter->vars.find(int_name.begin()) != iter->vars.end()) {
            iter->vars.erase(iter->vars.find(int_name.begin()));
        }
        iter++;
    }
    if (m_static.find(int_name.begin()) != m_static.end()) {
        m_static.erase(m_static.find(int_name.begin()));
    }
    if (isGlobalScope(int_name)) {
        LOG_RUNTIME("Remove global name in transaction not implemented!");
    }
}

void ScopeStack::RollbackNames_() {
    if (m_is_transaction) {
        for (auto &name : m_transaction) {
            RemoveName_(name);
        }
        m_transaction.clear();
        m_is_transaction = false;
    }
}

bool ScopeStack::FixTransaction() {
    m_is_transaction = false;
    m_transaction.clear();
    return true;
}

bool ScopeStack::AddName(const TermPtr var, const char * alt_name) {

    ASSERT(var);

    std::string name;
    if (alt_name) {
        //        name = GetNamespace();
        name = alt_name;
    } else {
        name = var->m_int_name;
    }
    if (name.empty()) {
        LOG_RUNTIME("Internal name of '%s' not exist!", var->toString().c_str());
    }
    if (!empty()) {
        if (back().vars.find(name) != back().vars.end()) {
            NL_MESSAGE(LOG_LEVEL_INFO, var, "Var '%s' exist!", name.c_str());
            return false;
        }
        back().vars.insert({name, var});
    }
    if (m_is_transaction) {
        m_transaction.push_back(name);
    }
    StorageTerm &stor = getStorage_();
    if (stor.find(name) != stor.end()) {
        NL_MESSAGE(LOG_LEVEL_INFO, var, "Var '%s' exist! (%s)", name.c_str(), stor.Dump().c_str());
        return false;
    }
    stor.insert({name, var});

    //    if (empty() || isStaticName(name)) {
    //        //        if (this->find(name) != this->end()) {
    //        //            NL_MESSAGE(LOG_LEVEL_INFO, var, "Var '%s' exist!", name.c_str());
    //        //            return false;
    //        //        }
    //        //        this->insert({name, var});
    //    }
    //    if ((m_stack.empty() || isStaticName(name)) && m_module) {
    //        return m_module->AddName(var);
    //    }
    return true;
}

TermPtr ScopeStack::FindInternalName(std::string_view int_name, RunTime *rt) {

    //    int_name = NormalizeName(int_name);
    if (!isInternalName(int_name)) {
        LOG_RUNTIME("'%s' is not an internal name!", int_name.begin());
    }


    StorageTerm & stor = getStorage_();
    if (&stor != &m_static && stor.find(int_name.begin()) != stor.end()) {
        return stor.find(int_name.begin())->second;
    }

    auto iter = rbegin();
    while (iter != rend()) {
        if (iter->vars.find(int_name.begin()) != iter->vars.end()) {
            return iter->vars.find(int_name.begin())->second;
        }
        iter++;
    }

    if (m_static.find(int_name.begin()) != m_static.end()) {
        return m_static.find(int_name.begin())->second;
    }

    if ((isGlobalScope(int_name) || isTypeName(int_name)) && rt) {
        return rt->GlobFindProto(int_name);
    }
    return nullptr;
}

TermPtr ScopeStack::LookupName(std::string name, RunTime *rt) {

    if (isInternalName(name)) {
        return FindInternalName(name, rt);
    }

    TermPtr found;
    bool full_search = isTrivialName(name);
    if (!full_search) {
        name = NormalizeName(name);
    }
    std::string temp;
    for (size_t skip = 0; skip < size(); skip++) {
        // Check local name
        temp = name;
        if (full_search) {
            temp += "$";
        }
        temp.insert(0, MakeNamespace(skip, false));
        if ((found = FindInternalName(temp, rt))) {
            return found;
        }
        if (full_search) {

            // Check static name local object
            temp = name;
            temp += "::";
            temp.insert(0, MakeNamespace(skip, false));
            if ((found = FindInternalName(temp, rt))) {
                return found;
            }
        }

        // Check static name
        temp = name;
        if (full_search) {
            temp += "::";
        }
        temp.insert(0, MakeNamespace(skip, true));
        if ((found = FindInternalName(temp, rt))) {
            return found;
        }


        if (full_search) {
            // Check type name
            temp = name;
            temp += ":";
            temp.insert(0, MakeNamespace(skip, true));
            if ((found = FindInternalName(temp, rt))) {
                return found;
            }
        }

        //        if (isGlobalScope(temp)) {
        //            break;
        //        }
    }

    temp = name;
    if (full_search) {
        temp += "$";
        if ((found = FindInternalName(temp, rt))) {
            return found;
        }
        temp = name;
        temp += "::";
        if ((found = FindInternalName(temp, rt))) {
            return found;
        }
        temp = name;
        temp += ":";
        if ((found = FindInternalName(temp, rt))) {
            return found;
        }

        temp = name;
        temp += "::";
        temp.insert(0, "::");
        if ((found = FindInternalName(temp, rt))) {
            return found;
        }
        temp += ":";
        if ((found = FindInternalName(temp, rt))) {
            return found;
        }
    } else {
        if ((found = FindInternalName(temp, rt))) {

            return found;
        }
    }
    return nullptr;
}

bool ScopeStack::LookupBlock_(TermPtr & ns) {
    //    ASSERT(term->isInterrupt());
    if (ns->m_text.empty()) {
        NL_MESSAGE(LOG_LEVEL_INFO, ns, "Lookup block empty!");
        return false;
    }
    ns->m_text = ExpandNamespace(ns->m_text);
    if (ns->m_text.compare("::") == 0) {
        return true;
    }
    for (int count = 0; count < size(); count++) {
        // Check fullname
        if (MakeNamespace(-(count + 1), true).find(ns->m_text) == 0) {
            return true;
        }
    }
    NL_MESSAGE(LOG_LEVEL_INFO, ns, "Lookup block '%s' fail!%s", ns->m_text.c_str(), Dump().c_str());
    return false;
}

std::string ScopeStack::GetOfferBlock() {
    if (empty()) {
        return "";
    }
    std::string result = " Possible block identifiers: '";

    std::string list_block;
    for (auto &elem : * this) {
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

std::string ScopeStack::Dump() {
    std::string result;
#ifdef BUILD_UNITTEST
    result = "Storage: ";
    //    if (m_module) {
    result += m_static.Dump();
    //    }

    auto iter = begin();
    while (iter != end()) {
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
#endif
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

bool StorageTerm::RegisterName(TermPtr term, const std::string_view syn) {
    InternalName name;
    if (syn.empty()) {
        name = term->m_int_name;
    } else {
        name = syn.begin();
    }
    ASSERT(!name.empty());
    if (find(name) != end()) {
        return false;
    }
    insert({name, term});
    return true;
}

StorageTerm::StorageTerm(const StorageTerm &clone) : std::map<InternalName, TermPtr>(clone) {
    for (auto &elem : * this) {
        elem.second = elem.second->Clone();
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
