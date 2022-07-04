#pragma once
#ifndef INCLUDED_NEWLANG_TERM_
#define INCLUDED_NEWLANG_TERM_

#include "pch.h"

#include "warning_push.h"
#include "location.hh"
#include "parser.h"
#include "warning_pop.h"

#include "variable.h"

namespace newlang {

#define NL_TERMS(_) \
        _(BLOCK) \
        _(BLOCK_TRY) \
        _(CALL_BLOCK) \
        _(CALL_TRY) \
        _(TERM) \
        _(TYPE) \
        _(CALL) \
        _(TYPE_CALL) \
        _(INTEGER) \
        _(NUMBER) \
        _(COMPLEX) \
        _(STRWIDE) \
        _(STRCHAR) \
        _(TEMPLATE) \
        _(EVAL) \
        _(COMMENT) \
        \
        _(FRACTION) \
        \
        _(NONE) \
        _(EMPTY) \
        _(TYPENAME) \
        _(UNKNOWN) \
        _(SYMBOL) \
        _(NAMESPACE) \
        _(PARENT) \
        _(MACRO) \
        _(MACRO_BODY) \
        _(NEWLANG) \
        _(ASSIGN) \
        _(CREATE) \
        _(CREATE_OR_ASSIGN) \
        _(APPEND) \
        \
        _(FUNCTION) \
        _(SIMPLE_AND) \
        _(SIMPLE_OR) \
        _(SIMPLE_XOR) \
        _(PUREFUNC) \
        _(ITERATOR) \
        _(ITERATOR_QQ)\
        _(FOLLOW) \
        _(MATCHING) \
        _(WHILE) \
        _(DOWHILE) \
        \
        _(RANGE) \
        _(ELLIPSIS) \
        _(FILLING) \
        _(ARGUMENT) \
        _(ARGS) \
        _(EXIT) \
        \
        _(INDEX) \
        _(FIELD) \
        \
        _(TENSOR) \
        _(DICT) \
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

    class Term : public Variable<TermPtr>, public std::enable_shared_from_this<Term> {
    public:

        static TermPtr Create(Term * term) {
            return std::make_shared<Term>(term);
        }

        static TermPtr Create(TermID id, const char *text, size_t len = std::string::npos, location *loc = nullptr, std::shared_ptr<std::string> source = nullptr) {
            return std::make_shared<Term>(id, text, (len == std::string::npos ? strlen(text) : len), loc, source);
        }

        Term(Term *term) {
            *this = *term;
        }

        Term(TermID id, const char *text, size_t len, location *loc, std::shared_ptr<std::string> source = nullptr) : m_id(id) {
            m_is_ref = false;
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
            return m_is_ref;
        }

        inline const std::string GetFullName() {
            std::string result(m_text);
            result.insert(0, m_namespace);
            return result;
        }

        inline bool IsFunction() {
            return m_id == TermID::FUNCTION || m_id == TermID::PUREFUNC || m_id == TermID::SIMPLE_AND || m_id == TermID::SIMPLE_OR || m_id == TermID::SIMPLE_XOR;
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
                case TermID::CALL:
                case TermID::TERM:
                case TermID::CREATE:
                case TermID::RANGE:
                case TermID::TENSOR:
                case TermID::DICT:
                case TermID::OPERATOR:
                case TermID::EVAL:
                    return true;
                default:
                    return IsLiteral() || IsVariable() || IsFunction();
            }
        }

        inline bool IsBlock() {
            switch (m_id) {
                case TermID::BLOCK:
                case TermID::BLOCK_TRY:
                case TermID::CALL_BLOCK:
                case TermID::CALL_TRY:
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
                str.append(elem.second->toString(true));
            }
        }

        std::string toString(bool nested = false) {
            std::string result(m_is_ref ? "&" : "");
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

                case TermID::ARGS:
                case TermID::ARGUMENT:
                    return m_text;

                case TermID::EXIT:
                    result = m_text;
                    if (Right()) {
                        result += " ";
                        result += Right()->toString();
                    }
                    return result;

                case TermID::FIELD:
                    return std::string("." + m_text);
                case TermID::INDEX:
                    result = "";
                    if (size()) {
                        result += "[";
                        dump_items_(result);
                        result += "]";
                    }
                    return result;
                case TermID::CALL:
                case TermID::TERM: // name=(1,second="two",3,<EMPTY>,5)
                    //                result(m_is_ref ? "&" : "");
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
                    result.insert(0, m_namespace);

                    if (m_is_ref) {
                        result.insert(0, "&");
                    }
                    if (!m_name.empty()) {
                        if (GetType()) {
                            result = m_name + GetType()->asTypeString() + "=" + result;
                        } else {
                            result = m_name + "=" + result;
                        }
                    }
                    if (size()) {
                        result += "(";
                        dump_items_(result);
                        result += ")";
                    } else if (m_id == TermID::CALL) {
                        result += "()";
                    }
                    if (m_name.empty() && GetType()) {
                        result += GetType()->asTypeString();
                    }
                    return result;

                case TermID::STRCHAR:// name:="string"
                case TermID::STRWIDE:// name:="string"
                    if (!result.empty()) {
                        result += "=";
                    }
                    result += m_id == TermID::STRWIDE ? "\"" : "'";
                    result += m_text;
                    result += m_id == TermID::STRWIDE ? "\"" : "'";
                    if (size()) {
                        result += "(";
                        dump_items_(result);
                        result += ")";
                    }
                    return result;

                case TermID::EVAL:
                    if (!result.empty()) {
                        result += "=";
                    }
                    result += "`";
                    result += m_text;
                    result += "`";
                    return result;

                case TermID::INTEGER:// name:=123
                case TermID::NUMBER: // name:=123.0
                    test = result.empty();
                    if (GetType() && !m_name.empty()) {
                        result += GetType()->asTypeString();
                    }
                    if (!test) {
                        result += "=";
                    }
                    result += m_text;
                    if (GetType() && m_name.empty()) {
                        result += GetType()->asTypeString();
                    }
                    return result;

                case TermID::ASSIGN:
                case TermID::CREATE:
                case TermID::CREATE_OR_ASSIGN:
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
                    result.insert(0, m_namespace);
                    //                result += "{";
                    result += m_right->toString(true);
                    if (m_right->GetTokenID() != TermID::SOURCE && !result.empty() && result[result.size() - 1] != ';') {
                        result += ";";
                    }
                    for (int i = 0; i < m_right->size(); i++) {
                        result += m_right->at(i).second->toString();
                    }
                    //                result += "};";
                    return result;

                case TermID::SIMPLE_AND:
                case TermID::SIMPLE_OR:
                case TermID::SIMPLE_XOR:
                    result += " " + m_text + " ";
                    result.insert(0, m_namespace);
                    if (m_right->GetTokenID() == TermID::BLOCK) {
                        for (size_t i = 0; i < m_right->m_block.size(); i++) {
                            if (i) {
                                result += ", ";
                            }
                            result += m_right->m_block[i]->toString();
                        }
                    } else {
                        result += m_right->toString();
                        result += ";";
                    }
                    return result;

                case TermID::ITERATOR:
                    result += m_text;
                    if (size()) {
                        result += "(";
                        dump_items_(result);
                        result += ")";
                    }
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
                    if (!m_class_name.empty()) {
                        result += m_class_name;
                    }
                    if (GetType()) {
                        result += GetType()->asTypeString();
                    }
                    return result;

                case TermID::TYPE:
                case TermID::TYPE_CALL:
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


                    if (m_id == TermID::TYPE_CALL) {
                        result += "(";
                        dump_items_(result);
                        result += ")";
                    }
                    return result;
                case TermID::SOURCE: // name:={{function code}}
                    result += "%{";
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
                    result = result;
                    result += m_text + "[";
                    ASSERT(m_right);
                    result += m_right->toString();
                    result += "];";
                    return result;

                case TermID::FOLLOW: // (cond) -> seq;

                    if (m_follow.empty()) {
                        result.insert(0, "[");
                        result += "]->{";
                        ASSERT(m_right);
                        result += m_right->toString();
                        result += "}";
                        if (m_right->m_right) {
                            result += "->";
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
                                result += "->";
                                result += m_follow[i]->m_right->toString();
                                if (m_follow[i]->m_right->getTermID() != TermID::BLOCK) {
                                    result += ";";
                                }
                                //                            result += "}";
                            } else {
                                if (nested || (!nested && m_follow[i]->m_left)) {
                                    result += "->";
                                }
                                result += m_follow[i]->toString(true);
                                if (m_follow[i]->getTermID() != TermID::BLOCK) {
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
                case TermID::CALL_BLOCK:
                case TermID::CALL_TRY:
                    result.clear();
                    if (m_id == TermID::CALL_BLOCK || m_id == TermID::CALL_TRY) {
                        result += "(";
                        dump_items_(result);
                        result += ")";
                    }
                    result = "{";
                    if (m_id == TermID::BLOCK_TRY) {
                        result += "{";
                    }
                    for (size_t i = 0; i < m_block.size(); i++) {
                        if (i) {
                            result += " ";
                        }
                        result += m_block[i]->toString(true);
                        if (m_block[i]->GetTokenID() != TermID::SOURCE) {
                            result += ";";
                        }
                    }
                    result += "}";
                    if (m_id == TermID::BLOCK_TRY) {
                        result += "}";
                    }
                    result += m_class_name;
                    return result;

                case TermID::OPERATOR:
                    result += m_text;
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

                case TermID::SYMBOL:
                case TermID::FRACTION:
                case TermID::COMPLEX:
                    return m_text;

                case TermID::EMPTY:
                    return result + "=";

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
            while (next) {
                next->SetSource(m_source);
                next = next->Right();
            }
            next = shared_from_this()->Left();
            while (next) {
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

        inline bool SetDims(TermPtr args) {
            TermPtr next = args;
            TermPtr prev;

            m_dims.clear();

            args->SetSource(m_source);
            while (next) {
                if (next->getTermID() != TermID::END) {
                    m_dims.push_back(next);
                }
                prev = next;
                next = next->m_comma_seq;
                prev->m_comma_seq.reset();
            }
            return true;
        }

        inline bool SetArgs(TermPtr args) {
            TermPtr next = args;
            TermPtr prev;

            if (args) {
                args->SetSource(m_source);
            }
            while (next && next->getTermID() != TermID::END) {
                push_back(next, next->getName());
                prev = next;
                next = next->m_comma_seq;
                prev->m_comma_seq.reset();
            }
            return true;
        }

        inline TermPtr AppendCommaTerm(TermPtr item) {
            TermPtr next = shared_from_this();

            while (true) {
                if (next->m_comma_seq) {
                    next = next->m_comma_seq;
                } else {
                    next->m_comma_seq = item;
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

            next = Term::Create(this);
            m_id = id;
            m_block.clear();
            m_sequence.reset();
            m_class_name.clear();

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

        inline TermID GetTokenID() {
            return m_id;
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

        typedef std::vector<TermPtr> BlockType;

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

        void MakeRef() {
            if (m_id != TermID::TERM || Left() || Right()) {
                LOG_RUNTIME("Cannon make referens value for %s!", toString().c_str());
            }
            m_is_ref = true;
        }

        std::string asTypeString() const {
            std::string result = m_text;
            if (m_is_ref) {
                result.insert(1, "&");
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
                m_type = type;
                m_type_name = m_type->asTypeString();
                // Check type
                if (m_id == TermID::INTEGER) {
                    ObjType type_val = typeFromLimit(parseInteger(m_text.c_str()), ObjType::Bool);
                    if (!canCastLimit(type_val, typeFromString(m_type_name))) {
                        NL_PARSER(type, "Error cast '%s' to integer type '%s'", m_text.c_str(), m_type_name.c_str());
                    }
                } else if (m_id == TermID::NUMBER) {
                    ObjType type_val = typeFromLimit(parseDouble(m_text.c_str()), ObjType::Float);
                    if (!canCastLimit(type_val, typeFromString(m_type_name))) {
                        NL_PARSER(type, "Error cast '%s' to numeric type '%s'", m_text.c_str(), m_type_name.c_str());
                    }
                } else if (m_id == TermID::COMPLEX) {
                    ObjType type_val = typeFromLimit(parseComplex(m_text.c_str()), ObjType::ComplexFloat);
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
                    m_type_name = newlang::toString(typeFromLimit(parseDouble(m_text.c_str()), ObjType::Float));
                } else if (m_id == TermID::COMPLEX) {
                    m_type_name = newlang::toString(typeFromLimit(parseComplex(m_text.c_str()), ObjType::ComplexFloat));
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

        //    SCOPE(protected) :

        TermID m_id;
        std::shared_ptr<std::string> m_source;
        int m_line;
        int m_col;

        // Связи для связаного списка
        static const int LEFT = -1;
        static const int RIGHT = 1;

        TermPtr m_left;
        TermPtr m_right;
        TermPtr m_base;
        TermPtr m_comma_seq;
        TermPtr m_sequence;

        std::string m_name;
        std::string m_text;
        std::string m_class_name;
        std::string m_namespace;
        std::vector<TermPtr> m_dims;

        BlockType m_block;
        BlockType m_follow;

        bool m_is_ref;

        /// Символьное описание потребуется для работы с пользовательскими типами данных.
        /// Итоговый тип может отличаться от указанного в исходнике для совместимых типов.
        std::string m_type_name;
    private:
        /// Тип данных, который хранится в виде термина из исходного файла. 
        /// Нужен для отображения сообщений (позиция в исходнике)
        /// Приватная область видимости для использовая SetType для проверки совместимости типов
        TermPtr m_type;
    };

    std::ostream & operator<<(std::ostream &out, newlang::TermPtr & var);
    std::ostream & operator<<(std::ostream &out, newlang::TermPtr var);
}
#endif // INCLUDED_NEWLANG_TERM_
