#pragma once
#ifndef INCLUDED_NEWLANG_TERM_
#define INCLUDED_NEWLANG_TERM_

#include "pch.h"

#include "warning_push.h"
#include "location.hh"
#include "parser.h"
#include "warning_pop.h"

#include "variable.h"
#include "version.h"

namespace newlang {

#define NL_TERMS(_) \
        _(NONE) \
        \
        _(SPACE) \
        _(INDENT) \
        _(COMMENT) \
        _(CRLF) \
        \
        _(DOC_BEFORE) \
        _(DOC_AFTER) \
        \
        _(BLOCK) \
        _(BLOCK_TRY) \
        _(BLOCK_PLUS) \
        _(BLOCK_MINUS) \
        \
        _(INT_PLUS) \
        _(INT_MINUS) \
        _(INT_REPEAT) \
        \
        _(NAME) \
        _(LOCAL) \
        _(GLOBAL) \
        _(MACRO) \
        _(MODULE) \
        _(NATIVE) \
        \
        _(TYPE) \
        _(INTEGER) \
        _(NUMBER) \
        _(COMPLEX) \
        _(RATIONAL) \
        \
        _(STRWIDE) \
        _(STRCHAR) \
        _(TEMPLATE) \
        _(EVAL) \
        \
        _(EMPTY) \
        _(ARGS) \
        _(ARGUMENT) \
        _(NEWLANG) \
        _(TYPENAME) \
        _(UNKNOWN) \
        _(SYMBOL) \
        _(NAMESPACE) \
        _(PARENT) \
        _(ESCAPE) \
        \
        _(MACRO_SEQ) \
        _(MACRO_STR) \
        _(MACRO_DEL) \
        _(MACRO_TOSTR) \
        _(MACRO_CONCAT) \
        _(MACRO_ARGUMENT) \
        _(MACRO_ARGCOUNT) \
        \
        _(CREATE) \
        _(CREATE_OR_ASSIGN) \
        _(ASSIGN) \
        _(PUREFUNC) \
        _(PURE_CREATE) \
        _(APPEND) \
        _(SWAP) \
        \
        _(FUNCTION) \
        _(ITERATOR) \
        \
        _(FOLLOW) \
        _(WHILE) \
        _(DOWHILE) \
        _(MATCHING) \
        _(WITH) \
        _(TAKE) \
        \
        _(RANGE) \
        _(ELLIPSIS) \
        _(FILLING) \
        \
        _(INDEX) \
        _(FIELD) \
        \
        _(TENSOR) \
        _(DICT) \
        _(CLASS) \
        _(OPERATOR) \
        _(SOURCE)

    enum class TermID : uint8_t {
        END = 0,
#define DEFINE_ENUM(name) name,
        NL_TERMS(DEFINE_ENUM)
#undef DEFINE_ENUM
    };

    inline const char* toString(TermID type) {
        switch (type) {
            case TermID::END:
                return "END";

#define DEFINE_CASE(name) \
  case TermID::name:     \
    return #name;
                NL_TERMS(DEFINE_CASE)
#undef DEFINE_CASE

            default:
                LOG_ERROR("UNKNOWN TERM TYPE %d", static_cast<int> (type));
                return "UNKNOWN TYPE ";
        }
    }

    size_t IndexArg(TermPtr term);
    std::string ParserMessage(std::string &buffer, int row, int col, const char *format, ...);

    inline static bool IsAnyCreate(TermID id) {
        return id == TermID::CREATE || id == TermID::CREATE_OR_ASSIGN || id == TermID::ASSIGN || id == TermID::PUREFUNC || id == TermID::PURE_CREATE;
    }

    class Term : public Variable<Term>, public std::enable_shared_from_this<Term> {
    public:

        static TermPtr Create(Term * term) {
            return std::make_shared<Term>(term);
        }

        static TermPtr Create(parser::token_type lex_type, TermID id, const char *text, size_t len = std::string::npos, location *loc = nullptr, std::shared_ptr<std::string> source = nullptr) {
            return std::make_shared<Term>(lex_type, id, text, (len == std::string::npos ? strlen(text) : len), loc, source);
        }

        TermPtr Clone() {
            TermPtr result = Term::Create(m_lexer_type, m_id, m_text.c_str());
            *result.get() = *this;
            return result;
        }

        Term(Term *term) {
            *this = *term;
        }

        Term(parser::token_type lex_type, TermID id, const char *text, size_t len, location *loc, std::shared_ptr<std::string> source = nullptr) {
            m_lexer_type = lex_type;
            m_ref.reset();
            if (text && len) {
                m_text.assign(text, std::min(strlen(text), len));
            }
            if (loc) {
                m_line = loc->begin.line;
                m_col = loc->begin.column;
            } else {
                m_line = 0;
                m_col = 0;
            }
            m_source = source;
            m_is_call = false;
            m_is_const = false;
            m_bracket_depth = 0;
            
            m_ref_restrict = RefType::RefNone;
            m_ref_type = RefType::RefNone;
            
            SetTermID(id);
        }

        virtual ~Term() {
            LOG_DUMP("DeleteVar %d \"%s\" (%p)", static_cast<int> (m_id), m_text.c_str(), (void *) this);
            clear_();
        }

        inline TermID getTermID() {
            return m_id;
        }

        inline void SetTermID(TermID id) {
            m_id = id;
            if (m_id == TermID::BLOCK || m_id == TermID::BLOCK_TRY ||
                    m_id == TermID::BLOCK_PLUS || m_id == TermID::BLOCK_MINUS) {
                m_is_call = true;
            }
        }

        inline std::string & getName() {
            return m_name;
        }

        inline const std::string & getName() const {
            return m_name;
        }

        inline void SetName(std::string & name) {
            m_name = name;
        }

        inline std::string & getText() {
            return m_text;
        }

        inline const std::string & getText() const {
            return m_text;
        }

        inline bool isRef() {
            return !!m_ref;
        }

        inline bool isCall() {
            return m_is_call;
        }

        inline bool IsCreate() {
            switch (m_id) {
                case TermID::APPEND:
                case TermID::CREATE:
                case TermID::CREATE_OR_ASSIGN:
                case TermID::ASSIGN:
                case TermID::PUREFUNC:
                case TermID::PURE_CREATE:
                    return true;
            }
            return false;
        }

        inline bool isMacro() {
            return m_id == TermID::MACRO_DEL || (IsCreate() && m_left && m_left->m_id == TermID::MACRO_SEQ);
        }

        inline bool isReturn() {
            return m_id == TermID::INT_PLUS || m_id == TermID::INT_MINUS;
        }

        inline const std::string GetFullName() {
            std::string result(m_text);
            //result.insert(isType(result) ? 1 : 0, m_ns_block);
            return result;
        }

        inline bool IsFunction() {
            return m_id == TermID::FUNCTION || m_id == TermID::PUREFUNC;
        }

        inline bool IsVariable() {
            return m_id == TermID::CREATE;
        }

        inline bool IsObject() {
            return !IsFunction() && !IsVariable();
        }

        inline bool IsScalar() {
            switch (m_id) {
                case TermID::INTEGER:
                case TermID::NUMBER:
                    return true;
            }
            return false;
        }

        inline bool IsString() {
            switch (m_id) {
                case TermID::STRWIDE:
                case TermID::STRCHAR:
                    return true;
            }
            return false;
        }

        inline bool IsLiteral() {
            switch (m_id) {
                case TermID::INTEGER:
                case TermID::NUMBER:
                case TermID::STRWIDE:
                case TermID::STRCHAR:
                    return true;

            }
            return false;
        }

        inline bool IsCalculated() {
            switch (m_id) {
                case TermID::ARGUMENT:
                case TermID::ARGS:
                case TermID::NAME:
                case TermID::CREATE:
                case TermID::RANGE:
                case TermID::TENSOR:
                case TermID::DICT:
                case TermID::OPERATOR:
                case TermID::EVAL:
                    return true;
                default:
                    return IsLiteral() || IsVariable() || IsFunction() || isCall();
            }
        }

        inline bool IsBlock() {
            switch (m_id) {
                case TermID::BLOCK:
                case TermID::BLOCK_TRY:
                case TermID::BLOCK_PLUS:
                case TermID::BLOCK_MINUS:
                    return true;
            }
            return false;
        }

        bool is_variable_args() {
            return size() && at(size() - 1).second && at(size() - 1).second->getTermID() == TermID::ELLIPSIS;
        }

        void dump_items_(std::string &str) const {
            bool first = true;
            for (auto elem : * this) {
                if (first) {
                    first = false;
                } else {
                    str.append(", ");
                }
                if (!elem.first.empty()) {
                    str.append(elem.first);

                    if (elem.second->GetType()) {
                        str += elem.second->GetType()->asTypeString();
                    }

                    str.append("=");
                }
                //                LOG_DEBUG("%s %s", newlang::toString(elem.second->getTermID()), elem.second->m_text.c_str());
                str.append(elem.second->toString(true));
            }
        }

        std::string toString(bool nested = false) {
            std::string result(m_ref ? "&" : "");
            result += m_name;
            if (Left()) {
                if (!result.empty()) {
                    result += "=";
                }
                ASSERT(this != Left().get());
                result += Left()->toString();
            }

            TermPtr temp;
            bool test;
            switch (m_id) {
                case TermID::END:// name=<END>
                    result += "<END>";
                    return result;

                case TermID::SPACE:
                case TermID::INDENT:
                case TermID::COMMENT:
                case TermID::CRLF:
                case TermID::FIELD:
                    return m_text;

                case TermID::MACRO_STR:
                    result = "@@@";
                    result += m_text;
                    result += "@@@";
                    return result;


                case TermID::ARGS:
                case TermID::ARGUMENT:
                case TermID::ITERATOR:
                    result += m_text;
                    if (size()) {
                        result += "(";
                        dump_items_(result);
                        result += ")";
                    }
                    return result;

                case TermID::INT_PLUS:
                case TermID::INT_MINUS:
                    result = m_text;
                    if (Right()) {
                        result += " ";
                        result += Right()->toString();
                    }
                    return result;

                case TermID::INDEX:
                    result = "";
                    if (size()) {
                        result += "[";
                        dump_items_(result);
                        result += "]";
                    }
                    return result;


                case TermID::NONE:
                case TermID::PARENT:
                case TermID::MODULE:
                case TermID::NEWLANG:
                case TermID::NATIVE:
                case TermID::MACRO:
                case TermID::LOCAL:
                case TermID::GLOBAL:
                case TermID::NAME: // name=(1,second="two",3,<EMPTY>,5)
                    //                result(m_is_ref ? "&" : "");
                    ASSERT(m_dims.empty());

                    result = "";
                    temp = shared_from_this();
                    if (temp->Left()) {
                        result = temp->Left()->toString();
                    }
                    while (!nested && temp->Right()) {
                        ASSERT(this != temp->Right().get());
                        if (temp->Right()->Left()) {
                            if (this == temp->Right()->Left().get()) {
                                break;
                            }
                            ASSERT(this != temp->Right()->Left().get());
                        }
                        result += temp->Right()->toString();
                        temp = temp->Right();
                    }

                    result.insert(0, m_text);
                    //                    result.insert(0, m_namespace);

                    if (m_ref) {
                        result.insert(0, m_ref->m_text);
                    }
                    if (m_is_const) {
                        result += "^";
                    }
                    //                    if (!m_name.empty()) {
                    //                        if (GetType()) {
                    //                            result = m_name + GetType()->asTypeString() + "=" + result;
                    //                            //                        } else {
                    //                            //                            result = m_name + "=" + result;
                    //                        }
                    //                    }
                    //                    LOG_DEBUG("3 %s", result.c_str());
                    if (size()) {
                        result += "(";
                        dump_items_(result);
                        result += ")";
                    } else if (m_is_call) {
                        result += "()";
                    }
                    if (m_name.empty() && GetType()) {
                        result += GetType()->asTypeString();
                    }
                    return result;

                case TermID::STRCHAR:// name:="string"
                case TermID::STRWIDE:// name:="string"
                    //                    if (!result.empty()) {
                    //                        result += "=";
                    //                    }
                    result = m_id == TermID::STRWIDE ? "\"" : "'";
                    result += m_text;
                    result += m_id == TermID::STRWIDE ? "\"" : "'";
                    if (size()) {
                        result += "(";
                        dump_items_(result);
                        result += ")";
                    }
                    return result;

                case TermID::EVAL:
                    //                    if (!result.empty()) {
                    //                        result += "=";
                    //                    }
                    result = "`";
                    result += m_text;
                    result += "`";
                    return result;

                case TermID::INTEGER:// name:=123
                case TermID::NUMBER: // name:=123.0
                    //                    test = result.empty();
                    //                    if (GetType() && !m_name.empty()) {
                    //                        result += GetType()->asTypeString();
                    //                    }
                    //                    if (!test) {
                    //                        result += "=";
                    //                    }
                    result = m_text;
                    if (GetType() && m_name.empty()) {
                        result += GetType()->asTypeString();
                    }
                    return result;

                case TermID::ASSIGN:
                case TermID::CREATE:
                case TermID::CREATE_OR_ASSIGN:
                case TermID::PURE_CREATE:
                    //            case TermID::APPEND:
                    if (m_id == TermID::ASSIGN) {
                        result += m_text;
                    } else {
                        result += " " + m_text + " ";
                    }
                    if (m_right) {
                        result += m_right->toString();
                    }
                    if (!nested) {
                        result += ";";
                    }
                    return result;

                case TermID::APPEND:
                    result = Left()->toString();
                    //                result += "[]";
                    if (!m_name.empty()) {
                        result += "." + m_name;
                    }
                    result += " " + m_text + " ";
                    result += m_right->toString();
                    if (!nested) {
                        result += ";";
                    }
                    return result;
                case TermID::RANGE:
                    ASSERT(size() == 2 || size() == 3);
                    result = at(0).second->toString();
                    result += "..";
                    result += at(1).second->toString();
                    if (size() == 3) {
                        result += "..";
                        result += at(2).second->toString();
                        //                    result += ")";
                    }
                    return result;
                case TermID::FUNCTION:
                case TermID::PUREFUNC:

                    result += " " + m_text + " ";
                    //                    result.insert(0, m_namespace);
                    //                result += "{";
                    result += m_right->toString(true);
                    if (!result.empty() && result[result.size() - 1] != ';') {
                        result += ";";
                    }
                    for (int i = 0; i < m_right->size(); i++) {
                        result += m_right->at(i).second->toString();
                    }
                    //                result += "};";
                    return result;

                case TermID::TENSOR:
                    result += "[";
                    dump_items_(result);
                    result += ",";
                    result += "]";
                    if (GetType()) {
                        result += GetType()->asTypeString();
                    }
                    return result;

                case TermID::DICT:
                    result += "(";
                    dump_items_(result);
                    result += ",";
                    result += ")";
                    if (!m_class.empty()) {
                        result += m_class;
                    }
                    if (GetType()) {
                        result += GetType()->asTypeString();
                    }
                    return result;

                case TermID::TYPE:
                    result += m_text;
                    if (m_dims.size()) {
                        result += "[";
                        for (int i = 0; i < m_dims.size(); i++) {
                            if (i) {
                                result += ",";
                            }
                            result += m_dims[i]->toString();
                        }
                        result += "]";
                    }


                    if (m_is_call) {
                        result += "(";
                        dump_items_(result);
                        result += ")";
                    }
                    return result;
                case TermID::SOURCE: // name:={% function code %}
                    result += "{%";
                    result += m_text;
                    result += "%}";
                    if (m_right) {
                        result += m_right->toString();
                    }
                    return result;

                case TermID::WHILE: // [cond] <->  repeat;
                    result = "[" + result + "]";
                    result += m_text;
                    ASSERT(m_right);
                    result += m_right->toString();
                    result += ";";
                    return result;

                case TermID::DOWHILE:
                    result += m_text + "[";
                    ASSERT(m_right);
                    result += m_right->toString();
                    result += "];";
                    return result;

                case TermID::FOLLOW: // (cond) -> seq;

                    if (m_follow.empty()) {
                        result.insert(0, "[");
                        result += "]-->{";
                        ASSERT(m_right);
                        result += m_right->toString();
                        result += "}";
                        if (m_right->m_right) {
                            result += "-->";
                            result += m_right->m_right->toString();
                        }
                    } else {
                        result.clear();
                        for (size_t i = 0; i < m_follow.size(); i++) {
                            if (!result.empty()) {
                                result += ",\n ";
                            }
                            if (m_follow[i]->m_left) {
                                result += "[";
                                result += m_follow[i]->m_left->toString();
                                result += "]";
                            } else {
                                result += " ";
                            }
                            if (m_follow[i]->m_right) {
                                result += "-->";
                                result += m_follow[i]->m_right->toString();
                                if (!(m_follow[i]->IsBlock() || m_follow[i]->getTermID() == TermID::SOURCE)) {
                                    result += ";";
                                }
                                //                            result += "}";
                            } else {
                                if (nested || (!nested && m_follow[i]->m_left)) {
                                    result += "-->";
                                }
                                result += m_follow[i]->toString(true);
                                if (!(m_follow[i]->IsBlock() || m_follow[i]->getTermID() == TermID::SOURCE)) {
                                    result += ";";
                                }
                                if (nested || (!nested && m_follow[i]->m_left)) {
                                    //                                result += "}";
                                }
                            }
                        }
                        //                    if (!nested) {
                        //                        result += ";";
                        //                    }
                    }

                    return result;
                case TermID::BLOCK:
                case TermID::BLOCK_TRY:
                case TermID::BLOCK_PLUS:
                case TermID::BLOCK_MINUS:
                    result.clear();
                    //                    if (m_id == TermID::CALL_BLOCK || m_id == TermID::CALL_TRY) {
                    //                        result += "(";
                    //                        dump_items_(result);
                    //                        result += ")";
                    //                    }
                    if (m_id == TermID::BLOCK) {
                        result = "{";
                    } else if (m_id == TermID::BLOCK_TRY) {
                        result = "{*";
                    } else if (m_id == TermID::BLOCK_PLUS) {
                        result = "{+";
                    } else if (m_id == TermID::BLOCK_MINUS) {
                        result = "{-";
                    } else {
                        LOG_ABORT("Unknown block type %s (%d)", newlang::toString(m_id), static_cast<uint8_t> (m_id));
                    }

                    if (!m_class.empty()) {
                        result.insert(0, " ");
                        result.insert(0, m_class);
                    }

                    for (size_t i = 0; i < m_block.size(); i++) {
                        if (i) {
                            result += " ";
                        }
                        result += m_block[i]->toString(true);
                        result += ";";
                    }

                    if (m_id == TermID::BLOCK) {
                        result += "}";
                    } else if (m_id == TermID::BLOCK_TRY) {
                        result += "*}";
                    } else if (m_id == TermID::BLOCK_PLUS) {
                        result += "+}";
                    } else if (m_id == TermID::BLOCK_MINUS) {
                        result += "-}";
                    } else {
                        LOG_ABORT("Unknown block type %s (%d)", newlang::toString(m_id), (int) m_id);
                    }

                    result += m_class;
                    return result;

                case TermID::OPERATOR:
                    result += " ";
                    result += m_text;
                    result += " ";
                    if (m_right) {
                        result += m_right->toString();
                    }
                    return result;

                case TermID::ELLIPSIS:
                    if (m_left) {
                        result = m_left->toString();
                        result += " ";
                    }
                    result += m_text;
                    if (m_right) {
                        result += m_right->toString();
                    }
                    return result;

                case TermID::FILLING:
                    result += "...";
                    if (m_right) {
                        result += m_right->toString();
                    }
                    result += "...";
                    return result;

                case TermID::MACRO_DEL:
                case TermID::MACRO_SEQ:
                    result = m_text;
                    result += " ";

                    for (size_t i = 0; i < m_macro_seq.size(); i++) {
                        if (i) {
                            result += " ";
                        }
                        if (m_macro_seq[i]->getTermID() == TermID::NAME) {
                            result += m_macro_seq[i]->toString();
                        } else {
                            result += m_macro_seq[i]->m_text;
                        }
                    }
                    result += " ";
                    result += m_text;
                    return result;

                case TermID::SYMBOL:
                case TermID::UNKNOWN:
                case TermID::RATIONAL:
                case TermID::COMPLEX:
                case TermID::MACRO_ARGCOUNT:
                case TermID::MACRO_ARGUMENT:
                    return m_text;

                case TermID::ESCAPE:
                    result = "@\\";
                    result += m_text;
                    return result;

                case TermID::EMPTY:
                    return result + "=";


                case TermID::CLASS:
                    result.clear();

                    bool comma = false;
                    for (auto &elem : m_base) {
                        if (comma) {
                            result += ", ";
                        } else {
                            comma = true;
                        }
                        result += elem->GetFullName();
                        result += "(";
                        elem->dump_items_(result);
                        result += ")";
                    }

                    result += "{";
                    for (size_t i = 0; i < m_block.size(); i++) {
                        if (i) {
                            result += " ";
                        }
                        result += m_block[i]->toString(true);
                        //                        if (m_block[i]->getTermID() != TermID::SOURCE) {
                        //                            result += ";";
                        //                        }
                    }
                    result += "}";

                    return result;

            }
            LOG_RUNTIME("Fail toString() type %s, text:'%s'", newlang::toString(m_id), m_text.c_str());
        }

        inline std::ostream & Print(std::ostream &out = std::cout, const char *delimiter = nullptr) {
            std::string str;
            out << toString();
            return out;
        }

        inline TermPtr Left(size_t pos = 1) {
            if (pos) {
                if (m_left) {
                    if (pos == 1) {

                        return m_left;
                    }
                    return m_left->Left(pos - 1);
                }
                return nullptr;
            }
            return shared_from_this();
        }

        inline TermPtr Right(size_t pos = 1) {
            if (pos) {
                if (m_right) {
                    if (pos == 1) {

                        return m_right;
                    }
                    return m_right->Right(pos - 1);
                }
                return nullptr;
            }
            return shared_from_this();
        }

        inline TermPtr Begin() {
            if (m_left) {
                return m_left->Begin();
            }
            return shared_from_this();
        }

        inline TermPtr End() {
            if (m_right) {
                return m_right->End();
            }
            return shared_from_this();
        }

        void SetSource(std::shared_ptr<std::string> source) {
            if (m_source == source) {
                return;
            }
            m_source = source;
            if (m_type && m_type.get() != this) {
                m_type->SetSource(source);
            }
            for (int i = 0; i < size(); i++) {
                at(i).second->SetSource(m_source);
            }
            for (auto &elem : m_block) {
                if (elem.get() != this) {
                    elem->SetSource(m_source);
                }
            }

            TermPtr next = shared_from_this()->Right();
            while (next && !m_source) {
                next->SetSource(m_source);
                next = next->Right();
            }
            next = shared_from_this()->Left();
            while (next && !m_source) {

                next->SetSource(m_source);
                next = next->Left();
            }
        }

        inline TermPtr Append(TermPtr item, int side = RIGHT) {
            ASSERT(side);
            if (!item->m_source) {
                item->m_source = m_source;
            }
            if (item) {
                if (side == RIGHT) {
                    m_right = item;
                } else {
                    m_left = item;
                }
            }
            return item;
        }

        inline bool AppendText(const std::string & s) {
            m_text.append(s);

            return true;
        }

        inline bool AppendText(TermPtr txt) {
            m_text.append(txt->getText());
            return true;
        }

        static bool ListToVector(TermPtr &args, std::vector<TermPtr> &vect) {
            TermPtr next = args;
            TermPtr prev;

            vect.clear();
            while (next) {
                if (next->getTermID() != TermID::END) {
                    vect.push_back(next);
                }
                prev = next;
                next = next->m_list;
                prev->m_list.reset();
            }
            return true;
        }

        inline bool SetArgs(TermPtr args) {
            TermPtr next = args;
            TermPtr prev;
            m_is_call = true;

            if (args) {
                args->SetSource(m_source);
            }
            while (next) {
                if (next->getTermID() != TermID::END) {

                    if (isSystemName(next->getName())) {
                        if (!m_sys_prop) {
                            m_sys_prop = Term::Create(parser::token_type::UNKNOWN, TermID::DICT, "");
                        }
                        m_sys_prop->push_back(next, next->getName());
                    } else {
                        if (m_sys_prop) {
                            NL_PARSER(next, "Cannot pass arguments after any system attributes!");
                        }
                        push_back(next, next->getName());
                    }
                }
                prev = next;
                next = next->m_list;
                prev->m_list.reset();
            }
            return true;
        }

        inline TermPtr AppendList(TermPtr item) {
            TermPtr next = shared_from_this();

            while (true) {
                if (next->m_list) {
                    next = next->m_list;
                } else {
                    next->m_list = item;

                    break;
                }
            }
            return item;
        }

        inline TermPtr AppendSequenceTerm(TermPtr item) {
            TermPtr next = shared_from_this();

            while (true) {
                if (next->m_sequence) {
                    next = next->m_sequence;
                } else {
                    next->m_sequence = item;
                    break;
                }
            }
            return item;
        }

        inline bool ConvertSequenceToBlock(TermID id, bool force = true) {

            if (!force && !m_sequence && !IsBlock()) {
                return true;
            }

            TermPtr next = shared_from_this();
            TermPtr prev;

            next = Clone();
            m_id = id;
            //            m_is_call = force;
            m_block.clear();
            m_sequence.reset();
            m_class.clear();

            while (next && next->getTermID() != TermID::END) {
                m_block.push_back(next);
                prev = next;
                next = next->m_sequence;
                prev->m_sequence.reset();
            }
            return true;
        }

        inline bool InsertText(const std::string & s) {
            m_text.insert(0, s);
            return true;
        }

        inline bool Reduce(int direction = RIGHT) {
            ASSERT(direction == RIGHT);
            if (!m_right) {
                return false;
            }

            LOG_DEBUG("Reduce %s <- %s", m_text.c_str(), m_right->m_text.c_str());

            m_right->clear_();

            return true;
        }

        void clear_() override {
            Clear(true);
        }

        void Clear(bool clear_iterator_name) {
            if (m_left && m_right) {
                m_left->m_right = m_right;
                m_right->m_left = m_left;
                m_left.reset();
                m_right.reset();
            } else if (m_left) {
                m_left->m_right.reset();
                m_left.reset();
            } else if (m_right) {

                m_right->m_left.reset();
                m_right.reset();
            }
            m_id = TermID::END;
            m_text.clear();
            m_line = 0;
            m_col = 0;

            m_name.clear();
            m_text.clear();
            m_block.clear();
            m_follow.clear();
            m_source.reset();
            m_docs.clear();
            m_macro_id.clear();
            m_macro_seq.clear();
        }

        inline TermPtr First() {
            if (m_left) {

                return m_left->First();
            }
            return shared_from_this();
        }

        inline TermPtr Last() {
            if (m_right) {
                return m_right->Last();
            }
            return shared_from_this();
        }

        inline size_t Count(int direction = RIGHT, TermID tok = TermID::END, const char *text = nullptr) {
            size_t result = 0;
            if (!direction && (tok != TermID::END || m_id == tok)) {
                result++;
            }
            if (m_right && direction >= RIGHT) {
                result++;
                result += m_right->Count(direction, tok, text);
            } else if (m_left && direction <= LEFT) {

                result++;
                result += m_left->Count(direction, tok, text);
            }
            return result;
        }

        inline TermPtr Find(TermID tok, const char *text = nullptr, int direction = RIGHT, TermPtr end = nullptr) {
            if (m_id == tok && (!text || std::string::npos != m_text.find(text))) {
                return shared_from_this();
            }
            if (end && this == end.get()) {
                return nullptr;
            }
            if (direction >= RIGHT && m_right) {
                return m_right->Find(tok, text, direction, end);
            } else if (direction <= LEFT && m_left) {

                return m_left->Find(tok, text, direction, end);
            }
            return nullptr;
        }

        inline void BreakLeft() {
            if (m_left) {

                m_left->m_right.reset();
                m_left.reset();
            }
        }

        inline void BreakRight() {
            if (m_right) {

                m_right->m_left.reset();
                m_right.reset();
            }
        }

        inline int GetLine() {

            return m_line;
        }

        inline int GetCol() {

            return m_col;
        }

        inline BlockType & BlockCode() {
            return m_block;
        }

        void InsertFollow(TermPtr & term) {
            term->m_source = m_source;
            m_follow.insert(m_follow.begin(), term);
        }

        void AppendFollow(TermPtr & term) {
            term->m_source = m_source;
            m_follow.push_back(term);
        }

        void MakeRef(TermPtr ref) {
            if (m_id != TermID::NAME || Left() || Right()) {
                LOG_RUNTIME("Cannon make referens value for %s!", toString().c_str());
            }
            m_ref = ref;
        }

        std::string asTypeString() const {
            std::string result = m_text;
            if (m_ref) {
                result.insert(1, m_ref->m_text);
            }
            if (size()) {
                result += "(";
                dump_items_(result);
                result += ")";
            }
            if (m_dims.size()) {
                result += "[";
                bool first = true;
                for (size_t i = 0; i < m_dims.size(); i++) {
                    if (!first) {
                        result += ",";
                    }
                    result += m_dims[i]->toString(true);
                    first = false;
                }
                result += "]";
            }
            return result;
        }

        void SetType(TermPtr type) {
            if (type) {
                ASSERT(!type->m_list);
                ASSERT(type->m_type_allowed.empty());
                ASSERT(m_type_allowed.empty());
                m_type_allowed.push_back(type);
                m_type = type;
                m_type_name = m_type->asTypeString();
                // Check type
                if (m_id == TermID::INTEGER) {
                    ObjType type_val = typeFromLimit(parseInteger(m_text.c_str()), ObjType::Bool);
                    if (!canCastLimit(type_val, typeFromString(m_type_name))) {
                        NL_PARSER(type, "Error cast '%s' to integer type '%s'", m_text.c_str(), m_type_name.c_str());
                    }
                } else if (m_id == TermID::NUMBER) {
                    ObjType type_val = typeFromLimit(parseDouble(m_text.c_str()), ObjType::Float64);
                    if (!canCastLimit(type_val, typeFromString(m_type_name))) {
                        NL_PARSER(type, "Error cast '%s' to numeric type '%s'", m_text.c_str(), m_type_name.c_str());
                    }
                } else if (m_id == TermID::COMPLEX) {
                    ObjType type_val = typeFromLimit(parseComplex(m_text.c_str()), ObjType::Complex64);
                    if (!canCastLimit(type_val, typeFromString(m_type_name))) {
                        NL_PARSER(type, "Error cast '%s' to complex type '%s'", m_text.c_str(), m_type_name.c_str());
                    }
                }
            } else {
                if (m_type) {
                    m_type.reset();
                }
                // Default type
                if (m_id == TermID::INTEGER) {
                    m_type_name = newlang::toString(typeFromLimit(parseInteger(m_text.c_str()), ObjType::Bool));
                } else if (m_id == TermID::NUMBER) {
                    m_type_name = newlang::toString(typeFromLimit(parseDouble(m_text.c_str()), ObjType::Float64));
                } else if (m_id == TermID::COMPLEX) {
                    m_type_name = newlang::toString(typeFromLimit(parseComplex(m_text.c_str()), ObjType::Complex64));
                } else if (m_id == TermID::STRCHAR) {
                    m_type_name = newlang::toString(ObjType::StrChar);
                } else if (m_id == TermID::STRWIDE) {
                    m_type_name = newlang::toString(ObjType::StrWide);
                } else if (m_id == TermID::DICT) {
                    m_type_name = newlang::toString(ObjType::Dictionary);
                }
            }
        }

        inline TermPtr GetType() {
            return m_type;
        }

        inline bool TestConst() {
            if (isConst(m_text)) {
                m_text.resize(m_text.size() - 1);
                m_is_const = true;
            }
            return m_is_const;
        }

        BlockType GetMacroId();

        //    SCOPE(protected) :
        static BlockType MakeMacroId(const BlockType &seq);

        TermID m_id;
        SourceType m_source;
        int m_line;
        int m_col;
        parser::token_type m_lexer_type;
        parser::location_type m_lexer_loc;

        // Связи для связаного списка
        static const int LEFT = -1;
        static const int RIGHT = 1;

        TermPtr m_left;
        TermPtr m_right;
        BlockType m_base;
        TermPtr m_list;
        TermPtr m_sequence;

        std::string m_name;
        std::string m_text;
        std::string m_class;
        BlockType m_dims;
        BlockType m_docs;
        BlockType m_type_allowed;

        TermPtr m_sys_prop;
        int m_bracket_depth;

        BlockType m_block;
        BlockType m_follow;
        BlockType m_macro_id;
        BlockType m_macro_seq;

        RefType m_ref_restrict;
        RefType m_ref_type;
        TermPtr m_ref;
        bool m_is_call;
        bool m_is_const;

        /// Символьное описание потребуется для работы с пользовательскими типами данных.
        /// Итоговый тип может отличаться от указанного в исходнике для совместимых типов.
        std::string m_type_name;

        std::weak_ptr<Obj> m_obj;

        SCOPE(private) :

        /// Тип данных, который хранится в виде термина из исходного файла. 
        /// Нужен для отображения сообщений (позиция в исходнике)
        /// Приватная область видимости для использовая SetType для проверки совместимости типов
        TermPtr m_type;
    };

    std::ostream & operator<<(std::ostream &out, newlang::TermPtr & var);
    std::ostream & operator<<(std::ostream &out, newlang::TermPtr var);
}
#endif // INCLUDED_NEWLANG_TERM_
