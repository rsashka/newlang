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
            ASSERT(IsCreate());
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
ObjPtr GlobNameList::GlobalNameGet(const char *name, bool is_raise) {

    GlobNameItem * ret = GlobalNameFind(name);

    if (ret) {
        if (!at::holds_alternative<ObjWeak>(ret->obj)) {

            if (at::holds_alternative<ObjWeak>(ret->obj)) {
                return at::get<ObjWeak>(ret->obj).lock();
            }
            ASSERT(at::holds_alternative<std::vector < ObjWeak >> (ret->obj));
            return at::get<std::vector < ObjWeak >> (ret->obj)[0].lock();
        }
        if (is_raise) {
            NL_PARSER(ret->proto, "Global name not implemented! '%s'", name);
        }
    }
    if (is_raise) {
        LOG_RUNTIME("Global Name '%s' not found!", name);
    }
    return nullptr;
}

/*
 * 
 * 
 */
bool Term::CheckArgsCall(TermPtr &term, RuntimePtr rt) {
    ASSERT(term);
    if (rt) {
        GlobNameItem * ret = rt->GlobalNameFind(term->m_text.c_str());
        if (ret) {
            return CheckArgsProto(term, ret->proto);
        }
        NL_PARSER(term, "Prototype name '%s' not found!", term->m_text.c_str());
    }
    return CheckArgsProto(term, nullptr);
}

bool Term::CheckArgsProto(TermPtr &term, const TermPtr proto) {

    ASSERT(term);
    std::string name(term->m_text);

    if (proto) {

        if (isNativeName(proto->m_text)) {
            if (!RunTime::GetDirectAddressFromLibrary(nullptr, proto->m_text.c_str())) {
                NL_PARSER(proto, "Native name '%s' not found!", proto->m_text.c_str());
            }
            if (proto->Right()) {
                // printf(fmt:FmtStr, ...) := %printf ...; 
                if (proto->Right()->getTermID() != TermID::ELLIPSIS) {
                    NL_PARSER(proto->Right(), "Unexpected term '%s' in native name!", proto->Right()->m_text.c_str());
                }
                return true;
            }
        } else if (isLocalAnyName(proto->m_text.c_str())) {
            NL_PARSER(proto, "Unexpected use name '%s' as prototype!", proto->m_text.c_str());
        }

        // printf(fmt:FmtStr, ...) := %printf(fmt:FmtStr, ...); 
        return CheckCompareArgs_(term, proto);

    } else {

        if (isNativeName(term->m_text)) {
            NL_PARSER(term, "Cannot use a native name as LValue !");
        } else if (isMacroName(term->m_text)) {
            NL_PARSER(term, "Macro name must be expanded!");
        } else if (!isLocalAnyName(term->m_text.c_str())) {
            NL_PARSER(term, "Unexpected term name!");
        }

    }
    return true;
}

bool Term::CheckCompareArgs_(const TermPtr &term, const TermPtr &proto) {

    return false;

    //      
    //    void Obj::ConvertToArgs_(Obj *in, bool check_valid, Context * ctx)
    //
    //    ASSERT(in);
    //
    //    //    bool named = false;
    //    bool is_ellipsis = false;
    //    if (check_valid && size()) {
    //        if (at(size() - 1).first.compare("...") == 0) {
    //            is_ellipsis = true;
    //            Variable::erase(size() - 1);
    //        }
    //    }
    //    for (int i = 0; i < in->size(); i++) {
    //
    //        if (isSystemName(in->name(i))) {
    //            continue;
    //        }
    //
    //        if (in->name(i).empty()) {
    //            //            if(check_valid && named) {
    //            //                LOG_RUNTIME("Position %d requires a named argument!", (int) i + 1);
    //            //            }
    //            ObjType base_type = ObjType::None;
    //            if (i < size()) {
    //                if (ctx) {
    //                    if ((*m_prototype)[i].second->m_type_name.empty()) {
    //                        base_type = ObjType::Any;
    //                    } else {
    //                        bool has_error = false;
    //                        base_type = ctx->m_runtime->BaseTypeFromString((*m_prototype)[i].second->m_type_name, &has_error);
    //                        if (has_error && (*m_prototype)[i].second->getTermID() == TermID::ELLIPSIS) {
    //                            base_type = ObjType::Any;
    //                        }
    //                    }
    //                } else if (!(*m_prototype)[i].second->m_type_name.empty()) {
    //                    base_type = typeFromString((*m_prototype)[i].second->m_type_name, ctx);
    //                } else {
    //                    base_type = ObjType::Any;
    //                }
    //            } else {
    //                base_type = ObjType::Any;
    //            }
    //
    //
    //            if (i < size()) {
    //                if (check_valid && at(i).second && at(i).second->getType() != ObjType::None) {
    //                    if (!canCast((*in)[i].second->getType(), at(i).second->getType())) {
    //                        LOG_RUNTIME("Fail cast value '%s' to type '%s'", newlang::toString((*in)[i].second->getType()),
    //                                newlang::toString(at(i).second->getType()));
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
    //                            LOG_RUNTIME("Fail cast value %s%s to type %s",
    //                                    (*in)[i].second->toString().c_str(),
    //                                    newlang::toString((*in)[i].second->getType()),
    //                                    (*m_prototype)[i].second->m_type_name.c_str());
    //                        }
    //                    }
    //                }
    //
    //                //                LOG_DEBUG("%s", (*in)[i].second->toString().c_str());
    //
    //                at(i).second->op_assign((*in)[i].second->toType(base_type));
    //            } else {
    //                if (check_valid && !is_ellipsis && m_prototype && i >= m_prototype->size()) {
    //                    LOG_RUNTIME("Positional args overflow. Ptrototype '%s'!",
    //                            m_prototype ? m_prototype->toString().c_str() : "Prototype not exists!");
    //                }
    //                push_back(in->at(i).second->toType(base_type), in->at(i).first);
    //            }
    //        } else {
    //            //            named = true;
    //            auto found = find(in->name(i));
    //            if (found != end()) {
    //                if (check_valid && (*found).second && (*found).second->getType() != (*in)[i].second->getType() && (*found).second->getType() != ObjType::None) {
    //                    LOG_RUNTIME("Different type arg '%s' and '%s'", (*found).second->toString().c_str(),
    //                            (*in)[i].second->toString().c_str());
    //                }
    //                //@todo  Проверка ограничений размер данных при указаном типе
    //                if (!(*found).second) {
    //                    (*found).second = Obj::CreateNone();
    //                }
    //                (*found).second->op_assign((*in)[i].second);
    //            } else {
    //                for (int pos = 0; pos < size(); pos++) {
    //                    if (!at(pos).first.empty() && at(pos).first.compare(in->at(i).first) == 0) {
    //                        at(pos).second->op_assign(in[i]);
    //                        goto done;
    //                    }
    //                }
    //                if (check_valid && !is_ellipsis) {
    //                    LOG_RUNTIME("Named arg '%s' not found!", in->name(i).c_str());
    //                }
    //                push_back(in->at(i));
    //done:
    //                ;
    //            }
    //        }
    //    }
    //    if (check_valid) {
    //
    //        CheckArgsValid();
    //    }
}

struct TraversingParam {
    TermPtr root;
    std::vector < NodeHandlerFunc *> handlers;
    void * obj;
};

void TraversingNodesExecuter(TermPtr &term, TraversingParam &param) {
    ASSERT(term);
    ASSERT(!term->m_list);
    ASSERT(!term->m_sequence);

    for (auto &func : param.handlers) {
        (*func)(term, param.obj);
    }
    if (term->m_left) {
        TraversingNodesExecuter(term->m_left, param);
    }
    if (term->m_right) {
        TraversingNodesExecuter(term->m_right, param);
    }
    for (auto &item : term->m_block) {
        TraversingNodesExecuter(item, param);
    }
    for (auto &item : term->m_follow) {
        TraversingNodesExecuter(item, param);
    }
}

void Term::TraversingNodes(TermPtr &ast, NodeHandlerList h, void * obj) {

    TraversingParam param;

    param.root = ast;
    param.handlers = h;
    param.obj = obj;

    TraversingNodesExecuter(ast, param);
}

