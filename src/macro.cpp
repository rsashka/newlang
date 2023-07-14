#include "pch.h"

#include "lexer.h"
#include "builtin.h"

#include <term.h>

using namespace newlang;

int MacroBuffer::m_counter = 0;
const std::string MacroBuffer::deny_chars_from_macro(";@:&^#?!{}+-%*~`\"',/|\\()[]<>");

std::string MacroBuffer::toHash(const TermPtr &term) {
    if (term->isMacro()) {
        ASSERT(!term->GetMacroId().empty());
        return toHashTermName(term->GetMacroId()[0]->m_text);
    }
    return toHashTermName(term->m_text);
}

void MacroBuffer::Push(const TermPtr term) {
    //    ASSERT(!m_diag_stack.empty());
    //    m_diag_stack.push_back(m_diag_stack[m_diag_stack.size() - 1]);
}

void MacroBuffer::Pop(const TermPtr term) {
    //    if(m_diag_stack.empty()) {
    //        if(term) {
    //            NL_PARSER(term, "Empty stack diags at '%s'!", term->toString().c_str());
    //        } else {
    //            LOG_RUNTIME("Empty stack diags!");
    //        }
    //    }
    //    m_diag_stack.pop_back();
}

TermPtr MacroBuffer::ParseTerm(const char *proto, MacroBuffer *macro, bool pragma_enable) {
    try {
        // Термин или термин + тип парсятся без ошибок
        Parser p(macro, nullptr, nullptr, pragma_enable);
        return p.Parse(proto);
    } catch (std::exception &) {
        std::string func(proto);
        try {
            func += ":={}";
            Parser p(macro, nullptr, nullptr, pragma_enable);
            return p.Parse(func)->Left();
        } catch (std::exception &e) {
            LOG_RUNTIME("Fail parsing prototype '%s' as '%s'!", func.c_str(), e.what());
        }
    }
}

size_t MacroBuffer::SkipBrackets(const BlockType& buffer, const size_t offset) {

    if (offset >= buffer.size()) {
        return 0;
    }

    std::string br_end;
    if (buffer[offset]->m_text.compare("(") == 0) {
        br_end = ")";
    } else if (buffer[offset]->m_text.compare("<") == 0) {
        br_end = ">";
    } else if (buffer[offset]->m_text.compare("[") == 0) {
        br_end = "]";
    } else {
        return 0;
    }

    size_t shift = 1;
    int count = 1;
    while (offset + shift < buffer.size()) {
        if (buffer[offset]->m_text.compare(buffer[offset + shift]->m_text) == 0) {
            count++; // Next level bracket
        } else if (br_end.compare(buffer[offset + shift]->m_text) == 0) {
            count--; // // Leave level bracket
            if (count == 0) {
                return shift + 1;
            }
        }
        shift++;
    }
    NL_PARSER(buffer[offset], "Closed bracket '%s' not found!", br_end.c_str());
}

size_t MacroBuffer::ParseTerm(TermPtr &result, const BlockType &buffer, size_t offset, bool pragma_enable) {

    if (offset >= buffer.size()) {
        LOG_RUNTIME("Fail skip count %d or buffer size %d!", (int) offset, (int) buffer.size());
    }

    /* term
     * func()
     * 
     * term: type
     * func(): type
     * 
     * term: type[]
     * func(): type[]
     *      
     */

    std::string source = buffer[offset]->toString();
    offset++;
    size_t skip = SkipBrackets(buffer, offset);

    if (skip) {
        /* 
         * term
         * func()
         */
        for (size_t i = 0; i < skip; i++) {
            if (buffer[offset + i]) {
                source += buffer[offset + i]->toString();
            }
        }
        offset += skip;
    }

    if (offset + 1 < buffer.size() && buffer[offset + 1]->m_text.compare(":") == 0) {
        offset++;
        source += buffer[offset]->toString();

        /* 
         * term: type
         * func(): type
         */

        if (offset + 1 >= buffer.size()) {
            NL_PARSER(buffer[offset + 1], "Typename missing!");
        }

        offset++;
        source += buffer[offset]->toString();

        skip = SkipBrackets(buffer, offset + 1);
        if (skip) {
            /* 
             * term: type[]
             * func(): type[]
             *      
             */
            for (size_t i = 0; i < skip; i++) {
                source += buffer[offset + i]->toString();
            }

            offset += (skip + 1);
        }
    }
    //    LOG_DEBUG("ParseTerm: '%s' - %d", source.c_str(), (int) offset);
    result = ParseTerm(source.c_str(), nullptr, pragma_enable);
    return offset;
}

std::string MacroBuffer::GetMacroMaping(const std::string str, const char *separator) {

    std::string result;

    // Итератор для списка макросов, один из которых может соответствовать текущему буферу (по первому термину буфера)
    iterator found = this->map::find(str);

    for (auto iter = found->second.begin(); found != end() && iter != found->second.end(); ++iter) {

        if (!result.empty() && separator) {
            result += separator;
        }

        result += (*iter)->toString();
    }
    return result;
}

bool MacroBuffer::RegisterPredefMacro(const char * name, const char * desc) {
    if (m_predef_macro.find(name) != m_predef_macro.end()) {
        LOG_ERROR("Predef macro '%s' redefined!", name);
        return false;
    }
    m_predef_macro.insert({name, desc});
    return true;
}

parser::token_type MacroBuffer::ExpandPredefMacro(TermPtr & term, Parser * parser) {

    if (m_predef_macro.empty()) {

        VERIFY(RegisterPredefMacro("__NLC_VER__", "Version NewLang Compiler."));
        VERIFY(RegisterPredefMacro("__NLC_SOURCE_GIT__", "Git source code identifier of the current compiler version."));
        VERIFY(RegisterPredefMacro("__NLC_DATE_BUILD__", "Date build of the current compiler version."));
        VERIFY(RegisterPredefMacro("__NLC_SOURCE_BUILD__", "Git source code identifier and date build of the current compiler version."));

        VERIFY(RegisterPredefMacro("__FILE__", "Current file name"));
        VERIFY(RegisterPredefMacro("__FILE_NAME__", "Current file name"));

        VERIFY(RegisterPredefMacro("__LINE__", "Line number in current file"));
        VERIFY(RegisterPredefMacro("__FILE_LINE__", "Line number in current file"));

        VERIFY(RegisterPredefMacro("__FILE_MD5__", "MD5 hash for current file"));
        VERIFY(RegisterPredefMacro("__FILE_TIMESTAMP__", "Timestamp current file"));

        VERIFY(RegisterPredefMacro("__DATE__", "Current date"));
        VERIFY(RegisterPredefMacro("__TIME__", "Current time"));
        // определяется как строковый литерал, содержащий дату и время последнего изменения текущего исходного файла 
        //в сокращенной форме с постоянной длиной, которые возвращаются функцией asctime библиотеки CRT, 
        //например: Fri 19 Aug 13:32:58 2016. Этот макрос определяется всегда.
        VERIFY(RegisterPredefMacro("__TIMESTAMP__", "Current timestamp"));
        VERIFY(RegisterPredefMacro("__TIMESTAMP_ISO__", "Current timestamp as ISO format")); // 2013-07-06T00:50:06Z

        //Развертывается до целочисленного литерала, начинающегося с 0. 
        //Значение увеличивается на 1 каждый раз, когда используется в файле исходного кода или во включенных заголовках файла исходного кода. 
        VERIFY(RegisterPredefMacro("__COUNTER__", "Monotonically increasing counter from zero"));
    }


    if (!term) {
        LOG_RUNTIME("Environment variable not defined!");
    }
    if (term->m_id != TermID::NAME) {
        return term->m_lexer_type;
    }

    std::string_view text = term->m_text;
    if (text.find("@") == 0) {
        text.remove_prefix(1);
    }

    ASSERT(!m_predef_macro.empty());
    if (m_predef_macro.find(text.begin()) == m_predef_macro.end()) {
        return term->m_lexer_type;
    }


    const TermID str_type = TermID::STRWIDE;
    const parser::token_type str_token = parser::token_type::STRWIDE;

    if (text.compare("__COUNTER__") == 0) {

        term->m_id = TermID::INTEGER;
        term->m_text = std::to_string(m_counter);
        m_counter++;
        return parser::token_type::INTEGER;

    } else if (text.compare("__NLC_VER__") == 0) {

        term->m_id = TermID::INTEGER;
        term->m_text = std::to_string(VERSION);
        return parser::token_type::INTEGER;

    } else if (text.compare("__NLC_SOURCE_GIT__") == 0) {
        term->m_text = GIT_SOURCE;
        term->m_id = str_type;
        return str_token;

    } else if (text.compare("__NLC_DATE_BUILD__") == 0) {
        term->m_text = DATE_BUILD_STR;
        term->m_id = str_type;
        return str_token;

    } else if (text.compare("__NLC_SOURCE_BUILD__") == 0) {
        term->m_text = SOURCE_FULL_ID;
        term->m_id = str_type;
        return str_token;

    } else if (text.compare("__LINE__") == 0 || text.compare("__FILE_LINE__") == 0) {

        term->m_id = TermID::INTEGER;
        term->m_text = std::to_string(term->m_line);
        return parser::token_type::INTEGER;

    } else if (text.compare("__FILE__") == 0 || text.compare("__FILE_NAME__") == 0) {

        term->m_id = str_type;
        if (parser && !parser->m_file_name.empty()) {
            term->m_text = parser->m_file_name;
        } else {
            term->m_text = "File name undefined!!!";
        }
        return str_token;

    } else if (text.compare("__FILE_TIMESTAMP__") == 0) {

        term->m_id = str_type;
        if (parser && !parser->m_file_time.empty()) {
            term->m_text = parser->m_file_time;
        } else {
            term->m_text = "??? ??? ?? ??:??:?? ????";
        }
        return str_token;

    } else if (text.compare("__FILE_MD5__") == 0) {

        term->m_id = str_type;
        if (parser && !parser->m_file_md5.empty()) {
            term->m_text = parser->m_file_md5;
        } else {
            term->m_text = "?????????????????????????????????";
        }
        return str_token;



    } else if (text.compare("__DATE__") == 0) {

        time_t t = std::time(NULL);
        if (parser) {
            t = parser->m_timestamp;
        }
        char buf[sizeof "Jul 27 2012"];
        strftime(buf, sizeof buf, "%b %e %Y", localtime(&t));

        term->m_text = buf;
        term->m_id = str_type;
        return str_token;

    } else if (text.compare("__TIME__") == 0) {

        time_t t = std::time(NULL);
        if (parser) {
            t = parser->m_timestamp;
        }
        char buf[sizeof "07:07:09"];
        strftime(buf, sizeof buf, "%T", localtime(&t));

        term->m_text = buf;
        term->m_id = str_type;
        return str_token;

    } else if (text.compare("__TIMESTAMP__") == 0) {

        time_t t = std::time(NULL);
        if (parser) {
            t = parser->m_timestamp;
        }
        term->m_text = asctime(localtime(&t));
        term->m_text = term->m_text.substr(0, 24); // Remove \n on the end line
        term->m_id = str_type;
        return str_token;

    } else if (text.compare("__TIMESTAMP_ISO__") == 0) {

        time_t t = std::time(NULL);
        if (parser) {
            t = parser->m_timestamp;
        }
        char buf[sizeof "2011-10-08T07:07:09Z"];
        strftime(buf, sizeof buf, "%FT%TZ", localtime(&t));

        term->m_text = buf;
        term->m_id = str_type;
        return str_token;
    }

    NL_PARSER(term, "Predef macro '%s' not implemented!", term->toString().c_str());
}

//bool MacroBuffer::CheckOpMacros(const TermPtr & term) {
//    if (term && (term->getTermID() == TermID::MACRO_DEL)) {
//        return true;
//    } else if (term && IsAnyCreate(term->getTermID())) {
//        if (term->Left() && term->Left()->getTermID() == TermID::MACRO_SEQ) {
//            if (term->Left()->m_list) {
//                NL_PARSER(term->Left(), "Define macro for list not allowed!");
//            }
//            return true;
//        }
//    }
//    return false;
//}

//TermPtr MacroBuffer::CreateMacroFromOp(const TermPtr & term) {
//
//    ASSERT(term);
//    if (!IsAnyCreate(term->getTermID())) {
//        NL_PARSER(term, "Operand '%s' not assign or create macro!", term->m_text.c_str());
//    }
//    ASSERT(term->Left());
//    if (term->Left()->getTermID() != TermID::MACRO_SEQ) {
//        NL_PARSER(term, "Operand '%s' not a lexem sequence!", term->Left()->toString().c_str());
//    }
//
//
//    TermPtr result = term->Left();
//    result->m_right = term->Right();
//
//    ASSERT(result->m_macro_seq.size());
//
//
//    result->m_text = result->GetMacroId()[0]->m_text; // For cache
//
//
//    term->m_left.reset();
//    term->m_right.reset();
//    //@todo Check syntax macros body?
//    return result;
//}

TermPtr MacroBuffer::EvalOpMacros(const TermPtr & term) {

    ASSERT(term);

    if (term->getTermID() == TermID::MACRO_DEL) {
        if (!Remove(term)) {
            LOG_WARNING("Macro '%s' not found!", toHash(term).c_str());
        }
        return term;
    }

    if (!term->Left()) {
        ASSERT(term->Left());
    }
    if (term->Left()->getTermID() != TermID::MACRO_SEQ) {
        NL_PARSER(term, "Operand '%s' not a macros!", term->Left()->toString().c_str());
    }

    if (term->Right()->getTermID() == TermID::MACRO_DEL || term->Right()->getTermID() == TermID::END) {
        NL_PARSER(term, "For remove macro use operator @@@@ %s @@@@;)", term->Left()->m_text.c_str());
    }


    TermPtr macro = GetMacroById(term->GetMacroId());

    if (macro) {

        if (term->getTermID() == TermID::CREATE || term->getTermID() == TermID::PURE_CREATE) {
            NL_PARSER(term, "Macros '%s' already exists!", term->Left()->toString().c_str());
        }

        // Итератор для списка макросов, один из которых может соответствовать текущему буферу (по первому термину буфера)
        MacroBuffer::iterator found = map::find(toHash(term));
        for (auto iter = found->second.begin(); found != end() && iter != found->second.end(); ++iter) {
            if (MacroBuffer::IdentityMacro(term->GetMacroId(), *iter) || MacroBuffer::IdentityMacro(iter->get()->GetMacroId(), term)) {

                if (term->getTermID() == TermID::CREATE || term->getTermID() == TermID::PURE_CREATE || (iter->get() != macro.get())) {
                    LOG_RUNTIME("Macro duplication '%s' and '%s'!", term->Left()->toString().c_str(), (*iter)->toString().c_str());
                }
            }
        }
        
        macro->m_right = term->Right();

    } else {

        if (term->getTermID() == TermID::ASSIGN) {
            NL_PARSER(term, "Macros '%s' not exists!", term->Left()->toString().c_str());
        }

        // Итератор для списка макросов, один из которых может соответствовать текущему буферу (по первому термину буфера)
        MacroBuffer::iterator found = map::find(toHash(term->Left()));
        for (auto iter = found->second.begin(); found != end() && iter != found->second.end(); ++iter) {
            if (MacroBuffer::IdentityMacro(term->GetMacroId(), *iter) || MacroBuffer::IdentityMacro(iter->get()->GetMacroId(), term)) {
                LOG_RUNTIME("Macro duplication '%s' and '%s'!", term->Left()->toString().c_str(), (*iter)->toString().c_str());
            }
        }

        macro = term;

        iterator iter = map::find(toHash(macro));
        if (iter == end()) {
            BlockType vect{macro};
            insert(std::make_pair(toHash(macro), vect));
        } else {
            iter->second.push_back(macro);
        }
    }


    return macro;
}

bool MacroBuffer::Remove(const TermPtr & term) {

    BlockType list = term->GetMacroId();
    ASSERT(!list.empty());


    if (term->m_id == TermID::MACRO_DEL && list.size() == 1 && list[0]->m_text.compare("_") == 0) {
        clear();
        return true;
    }

    iterator found = map::find(toHash(term));

    if (found != end()) {
        for (BlockType::iterator iter = found->second.begin(); iter != found->second.end(); ++iter) {

            BlockType names = (*iter)->GetMacroId();

            //        for (auto &elem : names) {
            //            LOG_DEBUG("%s ", elem.c_str());
            //        }


            if (names.size() != list.size()) {
                continue;
            }

            for (int pos = 0; pos < list.size(); pos++) {
                if (!CompareTermName(list[pos]->m_text.c_str(), names[pos]->m_text.c_str())) {
                    goto skip_remove;
                }
            }

            found->second.erase(iter);

            if (found->second.empty()) {
                erase(list[0]->m_text.c_str());
            }

            return true;

skip_remove:
            ;
        }
    }
    return false;
}

std::string MacroBuffer::Dump() {
    std::string result;
    auto iter = begin();
    while (iter != end()) {
        if (!result.empty()) {
            result += ", ";
        }

        for (int pos = 0; pos < iter->second.size(); pos++) {

            std::string str;
            for (auto &elem : iter->second[pos]->GetMacroId()) {
                if (!str.empty()) {
                    str += " ";
                }
                str += elem->m_text;
                if (iter->second[pos]->isCall()) {
                    str += "(";
                }
            }
            result += iter->first + "->'" + str + "'";
            if (pos + 1 < iter->second.size()) {
                result += "; ";
            }
        }

        iter++;
    }
    return result;
}

std::string MacroBuffer::Dump(const MacroArgsType & var) {
    std::string result;
    auto iter = var.begin();
    while (iter != var.end()) {
        if (!result.empty()) {
            result += ", ";
        }

        std::string str;
        for (int pos = 0; pos < iter->second.size(); pos++) {
            if (!str.empty()) {
                str += " ";
            }
            str += iter->second[pos]->toString();
        }

        result += iter->first + "->'" + str + "'";
        iter++;
    }
    return result;
}

std::string MacroBuffer::Dump(const BlockType & arr) {
    std::string result;
    auto iter = arr.begin();
    while (iter != arr.end()) {
        if (!result.empty()) {
            result += " ";
        }
        result += (*iter)->toString();
        iter++;
    }
    return result;
}

bool MacroBuffer::CompareTermName(const std::string & term_name, const std::string & macro_name) {
    //    LOG_DEBUG("%s == %s", term_name.c_str(), macro_name.c_str());
    if (isLocal(macro_name)) {
        // Шаблон соответствует любому термину входного буфера
        return true;
    }
    if (isMacro(term_name)) {
        // Если термин в буфере - имя макроса
        if (isMacro(macro_name)) {
            return macro_name.compare(term_name) == 0;
        }
        // Префикс макроса не учавтствует в сравнении
        return macro_name.compare(&term_name.c_str()[1]) == 0;

    } else if (isMacro(macro_name)) {
        // Если термин в буфере - имя макроса
        if (isMacro(term_name)) {
            return macro_name.compare(term_name) == 0;
        }
        // Префикс макроса не учавтствует в сравнении
        return term_name.compare(&macro_name.c_str()[1]) == 0;

    } else if (isLocalAny(term_name.c_str())) {
        // Любой другой термин не подходит
        return false;
    }
    // Без префиксов оба термина
    return term_name.compare(macro_name) == 0;
}

/*
 * < first second >
 * first != first second 
 * first second == first second 
 * first second ( ) == first second 
 * 
 * < first second ( ) >
 * first != first second ( )
 * first second != first second ( ) <- Exception
 * first second ( ) == first second  ( )
 * 
 * func name()
 * virtual func name()
 * override func name()
 * final func name()
 * 
 * override virtual func name()
 * virtual override func name()
 * override final func name()
 * virtual final func name()
 * final override virtual func name()
 * final virtual override func name()
 * 
 * 
 * * func name()
 * virtual func name()
 * override func name()
 * final func name()
 * 
 * 
 * Классификация аннотаций Java
 * Аннотации можно классифицировать по количеству передаваемых в них параметров: без параметров, с одним параметром и с несколькими параметрами.
 * Маркерные аннотации - Маркерные аннотации не содержат никаких членов или данных. Для определения наличия аннотации можно использовать метод isAnnotationPresent().
 * Аннотации с одним значением содержат только один атрибут, который принято называть value.
 * Полные аннотации (из нескольких пар "имя-значение". Например, Company(name = "ABC", city = "XYZ"). )
 * 
 * Аннотации в NewLang всегда сожержат только один атрибут, маркертные имитируются установкой значения 1, 
 * а полные аннотации не поддериватся (по крайнемй мера пока)
 * 
 * Прагмы обработываются и доступны только во время компиляции,
 * а Аннотации доступна и во время компиляции и во время выполнения в виде системых?????? свойств объектов.
 * 
 * Прагмы применяются для всего текущего окружения (экземпляра компилятора),
 * а аннотации только к одному создаваемому объекту (к следующей операции создания объекта/присвоения значения);
 * 
 * 
 * @@ override @@ := @@ @__ANNOTATION_SET__(override, 1) @__PRAGMA_UNEXPECTED__( (, <, [, {, {+, {-, {*) @@
 * @@ nothrow @@ := @@ @__ANNOTATION_SET__(nothrow, 1) @__PRAGMA_UNEXPECTED__( (, <, [, {, {+, {-, {*) @@
 * @@ func $name (...) @@ := @@ @$name ( @$* )  @__ANNOTATION_IIF__(override, =, ::=) 
 *                                              @__ANNOTATION_IF_EXPECTED__(nothrow, {*) 
 *                                              @__ANNOTATION_ELSE_EXPECTED__(nothrow, {, {+, {-, {*)  @@
 * 
 * @@ __name__ @@ ::= @@ . @__PRAGMA_NO_MACRO__()  __name__ @@ # -> \.__name__
 * 
 * @__PRAGMA_INDENT_BLOCK__( +} )
 * 
 * 
 * func name() { # ->   name() ::= {
 * }; 
 * 
 * @override
 * func name() { # ->   name() = {
 * }; 
 *  
 * @override
 * \\nothrow
 * func name() { # ->   name() = {*
 * *}; 
 * 
 * 
 * 
 * 
 * @@ property(name, value) @@ := @@ @__ANNOTATION_SET__(@$name, @$value) @@
 * 
 * @property(name, "ABC")
 * @property(city, "XYZ")
 * 
 * @override -> @__ANNOTATION_SET__(override, 1) 
 * @virtual  -> @__ANNOTATION_SET__(virtual, 1)  @__PRAGMA_PROP_SET__(virtual)
 * @final    -> @__ANNOTATION_SET__(final, 1)    @__PRAGMA_PROP_SET__(final)
 * @const    -> @__ANNOTATION_SET__(const, 1)    @__PRAGMA_PROP_SET__(const)
 * @data(1,2,3)    -> @__ANNOTATION_SET__(data, 1,2,3)
 * @data([1,2,3,])    -> @__ANNOTATION_SET__(data, [1,2,3,])
 * @data((1,2,3,))    -> @__ANNOTATION_SET__(data, (1,2,3,))
 * 
 * @Company( name="ABC", city="XYZ" ) -> name="ABC"; city="XYZ";
 * 
 * func name()
 * 
 * @__ANNOTATION_IIF__()
 * @__PRAGMA_PROP_CHECK__(override, = , ::=)
 * @__PRAGMA_PROP_CHECK__(const, ^)
 * @__PRAGMA_PROP_TEST__(const, ^)
 * @__PRAGMA_PROP_TEST__(const, ^)
 * 
 */
bool MacroBuffer::IdentityMacro(const BlockType &buffer, const TermPtr & macro) {

    if (!macro || !macro->isMacro()) {// || buffer.size() < macro->m_macro_id.size()) {
        return false;
    }


    int buff_offset = 0;
    int macro_offset = 0;
    while (buff_offset < buffer.size() && macro_offset < macro->GetMacroId().size()) {

        if (buffer[buff_offset]->getTermID() == TermID::END) {
            return false;
        }

        //        LOG_DEBUG("TermID: %s, '%s'  '%s'", toString(buffer[buff_offset]->getTermID()),
        //                buffer[buff_offset]->m_text.c_str(), macro->m_macro_id[macro_offset]->m_text.c_str());
        // Текст термина сравнивается только для опредленных терминов
        if (!CompareTermName(buffer[buff_offset]->m_text, macro->GetMacroId()[macro_offset]->m_text)) {
            return false;
        } else {

            buff_offset++;

            if (macro->GetMacroId()[macro_offset]->isCall()) {
                // Пропускаем скобки и все что находится между ними
                size_t skip = SkipBrackets(buffer, buff_offset);
                if (!skip) {
                    return false;
                }
                buff_offset += skip;

            }
        }

        macro_offset++;

        if (macro_offset == macro->GetMacroId().size()) {
            //            LOG_DEBUG("Macro '%s' done for %d lexem!", macro->toString().c_str(), buff_offset);
            return true;
        }

        //        buff_offset++;
    }
    // Нужен следующий термин для сопоставления
    return false;
}

void MacroBuffer::InsertArg_(MacroArgsType & args, std::string name, BlockType & buffer, size_t pos) {

    if (args.find(name) != args.end()) {
        LOG_RUNTIME("Duplicate arg %s!", name.c_str());
    }

    BlockType vect;
    if (pos == static_cast<size_t> (-1)) {
        for (auto &elem : buffer) {
            vect.push_back(elem->Clone());
        }
    } else {
        if (pos >= buffer.size()) {
            LOG_RUNTIME("No data for input buffer! Pos %d for size %d!", (int) pos, (int) buffer.size());
        }
        vect.push_back(buffer[pos]->Clone());
    }
    args.insert(std::make_pair(name, vect));
}

BlockType MacroBuffer::SymbolSeparateArg_(const BlockType &buffer, size_t pos, std::vector<std::string> sym, std::string & error) {
    error.clear();
    BlockType result;
    size_t skip;
    while (pos < buffer.size()) {
        if (buffer[pos]->getTermID() == TermID::SYMBOL) {
            for (auto &elem : sym) {
                if (buffer[pos]->m_text.compare(elem) == 0) {
                    return result;
                }
            }

        }

        skip = SkipBrackets(buffer, pos);
        for (int i = 0; skip && i < skip - 1; i++) {
            result.push_back(buffer[pos]->Clone());
            pos++;
        }

        result.push_back(buffer[pos]->Clone());
        pos++;
    }

    for (auto &elem : sym) {
        if (!error.empty()) {
            error += " or ";
        }
        error += "'";
        error += elem;
        error += "'";
    }

    error.insert(0, "Expected symbol ");
    error += "!";

    return result;
}

size_t MacroBuffer::ExtractArgs(BlockType &buffer, const TermPtr &term, MacroArgsType & args) {

    ASSERT(term);

    if (!term->isMacro()) { //((term->getTermID() == TermID::MACRO_SEQ || term->getTermID() == TermID::MACRO_STR || term->getTermID() == TermID::MACRO_DEL))) {
        LOG_RUNTIME("Term is not a macro! '%s'", term->toString().c_str());
    }

    args.clear();

    size_t pos_buf = 0; //Позиция во входном буфере
    int pos_id = 0; // Позиция в идентификаторе макроса

    std::string arg_name; // Имя аргумента
    BlockType args_all; // Последовательность терминов для @$...

    size_t arg_count = 0; // Кол-во аругментов
    size_t arg_offset = 0; // Позиция аргумента во входном буфере

    bool arg_ellipsys = false;
    bool all_args_done = false; // Маркер анализа аргументов (могту быть только у одного термина)

    // Перебор идентификатора макроса
    while (pos_id < term->GetMacroId().size()) {

        // Имя локальной переменной в идентификаторе макроса - это шаблон для замещения при его последующем раскрытии тела макроса
        if (isLocal(term->GetMacroId()[pos_id]->m_text)) {

            InsertArg_(args, term->GetMacroId()[pos_id]->m_text, buffer, pos_buf);

            // Для термина с аргументами
        } else if (term->GetMacroId()[pos_id]->isCall()) {

            // Аругменты поддерживаются только у одного термина
            if (all_args_done) {
                LOG_RUNTIME("Support single term call only!");
            }
            all_args_done = true;


            // Пропустить открывающую скобку
            pos_buf++;
            if (pos_buf >= buffer.size() || buffer[pos_buf]->getTermID() != TermID::SYMBOL || buffer[pos_buf]->m_text.compare("(") != 0) {
                LOG_RUNTIME("Expected '('!");
            }
            pos_buf++;

            std::string error_str;
            BlockType arg_seq;
            while (1) {
                // Последовательность терминов в качестве аргумента, включая вложенные скобки
                arg_seq = SymbolSeparateArg_(buffer, pos_buf,{")", ","}, error_str);

                if (!error_str.empty()) {
                    LOG_RUNTIME("%s", error_str.c_str());
                }

                if (!arg_seq.empty()) {

                    args_all.insert(args_all.end(), arg_seq.begin(), arg_seq.end());

                    pos_buf += arg_seq.size();

                    ASSERT(pos_buf < buffer.size());

                    arg_count++;

                    // Порядковый номер аргумента @$1, @$2 и т.д.
                    arg_name = "@$";
                    arg_name += std::to_string(arg_count);
                    InsertArg_(args, arg_name, arg_seq);

                    if (arg_count - 1 < term->GetMacroId()[pos_id]->size()) {

                        // Именованные аругменты из прототипа макроса
                        arg_name = term->GetMacroId()[pos_id]->at(arg_count - 1).second->m_text;

                        if (arg_name.compare("...") == 0) {

                            if (arg_ellipsys) {
                                NL_PARSER(arg_seq[0], "Fail ellipsys args in prototype '%s'!", term->GetMacroId()[pos_id]->toString().c_str());
                            }
                            arg_ellipsys = true;

                        } else {

                            //                            if(arg_name.find("$") == 0) {
                            //                                arg_name.insert(0, "@");
                            //                            } else {
                            arg_name.insert(0, "@$");
                            //                            }
                            InsertArg_(args, arg_name, arg_seq);
                        }

                    } else {
                        // @todo Сделать проверку аргументов у маркосов
                        // if(!arg_ellipsys) {
                        //      NL_PARSER(buffer[pos + arg_offset - 1], "Extra args for prototype '%s'!", term->GetMacroId()[i]->toString().c_str());
                        // }
                    }

                } else if (buffer[pos_buf]->m_text == ",") {
                    args_all.push_back(buffer[pos_buf]);
                    pos_buf++;
                } else if (buffer[pos_buf]->m_text == ")") {
                    break;
                } else {
                    LOG_RUNTIME("Unexpected symbol %s ?????????", buffer[pos_buf]->m_text.c_str());
                }
            }
        }

        pos_buf++;
        pos_id++;
    }



    BlockType cnt{Term::Create(parser::token_type::INTEGER, TermID::INTEGER, std::to_string(arg_count).c_str())};
    arg_name = "@$#";
    InsertArg_(args, arg_name, cnt);

    arg_name = "@$...";
    InsertArg_(args, arg_name, args_all);
    //    LOG_DEBUG("args_all: %s", Dump(args_all).c_str());

    // As dictionary
    arg_name = "@$*";
    args_all.insert(args_all.begin(), Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, "("));
    args_all.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, ","));
    args_all.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, ")"));

    InsertArg_(args, arg_name, args_all);

    std::string ttt;
    for (size_t j = 0; j < term->GetMacroId().size(); j++) {
        if (j) {
            ttt += " ";
        }
        ttt += MacroBuffer::toHashTermName(term->GetMacroId()[j]->m_text);
    }
    //    LOG_DEBUG("m_macro_id: '%s'  args: %s", ttt.c_str(), Dump(args).c_str());


    if (((pos_id == term->GetMacroId().size()) //term->getTermID() == TermID::MACRO
            || (term->getTermID() == TermID::MACRO_SEQ && pos_id == term->GetMacroId().size()))
            && pos_buf + arg_offset <= buffer.size()) {
        ASSERT(pos_buf + arg_offset <= buffer.size());

        return pos_buf + arg_offset;
    }

    NL_PARSER(buffer[0], "Input buffer empty for extract args macros %s (%d+%d)=%d!", term->toString().c_str(), (int) pos_buf, (int) arg_offset, (int) buffer.size());
}

BlockType MacroBuffer::ExpandMacros(const TermPtr &macro, MacroArgsType & args) {
    ASSERT(macro);
    ASSERT(macro->Right());

    BlockType result;
    BlockType seq = macro->Right()->m_macro_seq;

    for (int i = 0; i < seq.size(); i++) {

        if (seq[i]->getTermID() == TermID::MACRO_TOSTR) {

            if (i + 1 >= seq.size()) {
                NL_PARSER(seq[i], "Next element to string not found!");
            }
            result.insert(result.end(), seq[i + 1]->Clone());

            if (seq[i]->m_text.compare("@#\"") == 0) {
                (*result.rbegin())->m_id = TermID::STRWIDE;
            } else if (seq[i]->m_text.compare("@#'") == 0) {
                (*result.rbegin())->m_id = TermID::STRCHAR;
            } else {
                ASSERT(seq[i]->m_text.compare("@#") == 0);
                //@todo Set string type default by global settings
                (*result.rbegin())->m_id = TermID::STRWIDE;
            }
            i++;


        } else if (seq[i]->getTermID() == TermID::MACRO_CONCAT) {
            ASSERT(seq[i]->m_text.compare("@##") == 0);

            if (result.empty() || i + 1 >= seq.size()) {
                NL_PARSER(seq[i], "Concat elements not exist!");
            }
            (*result.rbegin())->m_text.append(seq[i + 1]->m_text);
            i++;

        } else if (seq[i]->m_text.find("@$") == 0) {

            auto iter = args.find(seq[i]->m_text);
            if (iter == args.end()) {
                LOG_RUNTIME("Argument name '%s' not found!", seq[i]->m_text.c_str());
            }

            if (seq[i]->m_text.compare("@$...") == 0) {

                // Remove last comma if argument list is empty/
                if (iter->second.empty() && result.rbegin() != result.rend() && (*result.rbegin())->m_text.compare(",") == 0) {
                    result.erase(std::prev(result.end()));
                }
            }

            for (auto & elem : iter->second) {
                result.push_back(elem->Clone());
            }

        } else {
            result.insert(result.end(), seq[i]->Clone());
        }
    }
    return result;
}

std::string ReplaceAll(std::string str, const std::string& from, const std::string & to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

std::string MacroBuffer::ExpandString(const TermPtr &macro, MacroArgsType & args) {
    ASSERT(macro);
    ASSERT(macro->Right());
    if (macro->Right()->m_id != TermID::MACRO_STR) {
        LOG_RUNTIME("Fail convert term type %s as macros string!", toString(macro->Right()->m_id));
    }

    std::string body(macro->Right()->m_text);

    for (auto &elem : args) {

        std::string text;
        for (auto &lex : elem.second) {

            text += lex->toString();
            text += " ";
        }

        //        LOG_DEBUG("Replace: '%s' -> '%s' in \"%s\"", elem.first.c_str(), text.c_str(), body.c_str());
        body = ReplaceAll(body, elem.first, text);
    }

    return body;
}

TermPtr MacroBuffer::GetMacroById(const BlockType block) {
    std::vector<std::string> list;
    for (auto &elem : block) {
        list.push_back(elem->m_text);
    }
    return GetMacro(list);
}

TermPtr MacroBuffer::GetMacro(std::vector<std::string> list) {
    if (list.empty()) {
        return nullptr;
    }
    iterator found = map::find(toHashTermName(list[0]));

    if (found != end()) {
        for (BlockType::iterator iter = found->second.begin(); iter != found->second.end(); ++iter) {

            BlockType names = (*iter)->GetMacroId();

            //            for (auto &elem : names) {
            //                LOG_DEBUG("%s ", elem.c_str());
            //            }


            if (names.size() != list.size()) {
                continue;
            }

            for (int pos = 0; pos < list.size(); pos++) {
                if (!CompareTermName(list[pos].c_str(), names[pos]->m_text.c_str())) {

                    goto skip_step;
                }
            }
            return *iter;
skip_step:
            ;
        }
    }
    return nullptr;
}

parser::token_type Parser::GetNextToken(TermPtr * yylval, parser::location_type * yylloc) {

    parser::token_type result;

    ASSERT(yylval);
    ASSERT(yylloc);

    /*
     * Новая логика работы парсера.
     * 
     * Термины считываются из лексера до символа ';' или до конца файла.
     * Каждый термин буфера проверяется на макрос и если находится - заменяется.
     * Для обычного макроса просто заменяются токены и тела макроса,
     * а для текстового макроса используется отдельный буфер до завершения работы лексичсекского анализатора,
     * после чего этот буфер вставляется на место макроса в основонм бефере данных.
     * После обработки всего буфера его элеменеты передаются в парсер для обработки.
     */

    parser::location_type loc;
    parser::token_type type;
    bool lexer_complete;


go_parse_string:

    m_is_runing = true;

    TermPtr term;
    bool is_escape = false;

    if (m_macro_analisys_buff.empty() || lexer->m_macro_iss) {

        lexer_complete = false;

        while (!m_is_lexer_complete) {

next_escape_token:

            term = Term::Create(parser::token_type::END, TermID::END, "", 0);
            type = lexer->lex(&term, &loc);
            term->m_lexer_loc = loc;

            ASSERT(type == term->m_lexer_type);

            if (is_escape) {

                if (type == parser::token_type::END) {
                    NL_PARSER(term, "Unexpected end of file '%s'", term->toString().c_str());
                }
                is_escape = false;
                term->m_id = TermID::ESCAPE;
                term->m_lexer_type = parser::token_type::ESCAPE;
                type = parser::token_type::ESCAPE;

            } else if (type == parser::token_type::ESCAPE) {

                is_escape = true;
                goto next_escape_token;
            }

            if (m_macro) {
                type = m_macro->ExpandPredefMacro(term, this);
            }


            if (type == parser::token_type::END) {

                if (lexer->m_macro_iss == nullptr) {
                    m_is_lexer_complete = true;
                } else {
                    lexer->yypop_buffer_state();
                    *lexer->m_loc = lexer->m_macro_loc; // restore

                    delete lexer->m_macro_iss;
                    lexer->m_macro_iss = nullptr;

                    lexer->source_string = lexer->source_base;

                    continue;
                }
                //                if(lexer->m_data.empty()) {
                //                    m_is_lexer_complete = true;
                //                } else {
                //                    lexer->yypop_buffer_state();
                //                    *lexer->m_loc = lexer->m_data[lexer->m_data.size() - 1].loc;
                //
                //                    delete lexer->m_data[lexer->m_data.size() - 1].iss;
                //                    lexer->m_data.pop_back();
                //
                //                    if(lexer->m_data.empty()) {
                //                        lexer->source_string = lexer->source_base;
                //                    } else {
                //                        lexer->source_string = lexer->m_data[lexer->m_data.size() - 1].data;
                //                    }
                //                    continue;
                //                }
            } else if (lexer->m_macro_count == 1) {

                ASSERT(type == parser::token_type::MACRO_SEQ);

                if (lexer->m_macro_del) {
                    NL_PARSER(term, "Invalid token '%s' at given position!", term->m_text.c_str());
                }

                lexer->m_macro_count = 2;
                lexer->m_macro_body = term;
                continue;

            } else if (lexer->m_macro_count == 3) {

                ASSERT(lexer->m_macro_body);
                ASSERT(type == parser::token_type::MACRO_SEQ);

                lexer->m_macro_count = 0;
                term.swap(lexer->m_macro_body);
                lexer->m_macro_body = nullptr;

            } else if (lexer->m_macro_count == 2 || lexer->m_macro_del == 2) {

                ASSERT(lexer->m_macro_body);

                if (lexer->m_macro_del && type != parser::token_type::NAME) {
                    NL_PARSER(term, "Invalid token '%s' at given position!", term->m_text.c_str());
                }

                lexer->m_macro_body->m_macro_seq.push_back(term);
                continue;

            } else if (lexer->m_macro_del == 1) {

                ASSERT(type == parser::token_type::MACRO_DEL);

                if (lexer->m_macro_count) {
                    NL_PARSER(term, "Invalid token '%s' at given position!", term->m_text.c_str());
                }

                lexer->m_macro_del = 2;
                lexer->m_macro_body = term;
                continue;

            } else if (lexer->m_macro_del == 3) {

                ASSERT(type == parser::token_type::MACRO_DEL);
                if (!lexer->m_macro_body) {
                    ASSERT(lexer->m_macro_body);
                }

                if (lexer->m_macro_body->m_macro_seq.empty()) {
                    NL_PARSER(term, "Empty sequence not allowed!");
                }

                lexer->m_macro_del = 0;

                if (lexer->m_macro_body) {
                    term.swap(lexer->m_macro_body);
                }
                lexer->m_macro_body = nullptr;
            }

            m_macro_analisys_buff.push_back(term);
            if (term->m_text.compare(";") == 0) {
                lexer_complete = true;
                break;
            }

            //            if(lexer_complete && lexer->m_macro_iss == nullptr) {
            //                break;
            //            }
        }

        if (lexer->m_macro_del || lexer->m_macro_count) {
            TermPtr bag_position;

            bag_position = (m_macro_analisys_buff.size() > 1) ? m_macro_analisys_buff[m_macro_analisys_buff.size() - 2] : term;
            if (!bag_position) {
                ASSERT(bag_position);
            }

            if (term->m_lexer_loc.begin.filename) {
                NL_PARSER(bag_position, "Incomplete syntax near '%s' in file %s!", bag_position->m_text.c_str(), term->m_lexer_loc.begin.filename->c_str());
            } else {
                NL_PARSER(bag_position, "Incomplete syntax near '%s'!", bag_position->m_text.c_str());
            }
        }
    }


    size_t counter = 0;
    TermPtr pragma;
    while (lexer->m_macro_count == 0 && !m_macro_analisys_buff.empty()) {

        // Обработка команд парсера @__PRAGMA ... __
        if (m_enable_pragma && PragmaCheck(m_macro_analisys_buff[0])) {

            size_t size;
            size = MacroBuffer::ParseTerm(pragma, m_macro_analisys_buff, 0, false);

            ASSERT(size);
            ASSERT(pragma);

            m_macro_analisys_buff.erase(m_macro_analisys_buff.begin(), m_macro_analisys_buff.begin() + size);

            while (!m_macro_analisys_buff.empty() && m_macro_analisys_buff[0]->m_text.compare(";") == 0) {
                m_macro_analisys_buff.erase(m_macro_analisys_buff.begin());
            }


            PragmaEval(pragma, m_macro_analisys_buff);
            continue;
        }

        // Обработка команды проверка следующего термина @__PRAGMA_EXPECTED__
        if (m_expected) {
            for (int i = 0; i < m_expected->size(); i++) {
                if (m_macro_analisys_buff[0]->m_text.compare(m_expected->at(i).second->m_text) == 0) {
                    m_expected.reset();
                    break;
                }
            }
            if (m_expected) {
                std::string msg;
                for (int i = 0; i < m_expected->size(); i++) {
                    if (!msg.empty()) {
                        msg += ", ";
                    }
                    msg += "'";
                    msg += m_expected->at(i).second->m_text;
                    msg += "'";
                }
                NL_PARSER(m_macro_analisys_buff[0], "Term %s expected!", msg.c_str());
            }
        }

        // Обработка команды проверка следующего термина @__PRAGMA_UNEXPECTED__
        if (m_unexpected) {
            for (int i = 0; i < m_unexpected->size(); i++) {
                if (m_macro_analisys_buff[0]->m_text.compare(m_unexpected->at(i).second->m_text) == 0) {
                    NL_PARSER(m_macro_analisys_buff[0], "Term '%s' unexpected!", m_macro_analisys_buff[0]->m_text.c_str());
                }
            }
            m_unexpected.reset();
        }

        if (m_no_macro) {
            m_no_macro = false;
            break;
        }

        if (m_macro) {

            // Макрос должне начинаться всегда с термина
            if (m_macro_analisys_buff[0]->getTermID() != TermID::NAME) {
                break;
            }

            TermPtr macro_done = nullptr;

            // Итератор для списка макросов, один из которых может соответствовать текущему буферу (по первому термину буфера)
            MacroBuffer::iterator found = m_macro->map::find(MacroBuffer::toHash(m_macro_analisys_buff[0]));

            if (found == m_macro->end()) {

                //                // Если макрос не найден - ошибка 
                //                if(isMacro(m_macro_analisys_buff[0]->m_text)) {
                //                    LOG_RUNTIME("Macro '%s' not found!", m_macro_analisys_buff[0]->toString().c_str());
                //                }

                break;
            }

            macro_done.reset();
            // Перебрать все макросы и сравнить с буфером
            for (auto iter = found->second.begin(); iter != found->second.end(); ++iter) {

                if (MacroBuffer::IdentityMacro(m_macro_analisys_buff, *iter)) {

                    if (macro_done) {
                        LOG_RUNTIME("Macro duplication %s and '%s'!", macro_done->toString().c_str(), (*iter)->toString().c_str());
                    }
                    macro_done = *iter;
                }
            }

            ASSERT(found != m_macro->end());

            if (macro_done) {

                counter++;
                if (counter > 100) {
                    LOG_RUNTIME("Macro expansion '%s' stack overflow?", macro_done->toString().c_str());
                }

                //                compare_macro = true; // Раскрывать макросы в теле раскрытого макроса

                ASSERT(m_macro_analisys_buff.size() >= macro_done->m_macro_seq.size());
                ASSERT(macro_done->Right());

                MacroBuffer::MacroArgsType macro_args;
                size_t size_remove = MacroBuffer::ExtractArgs(m_macro_analisys_buff, macro_done, macro_args);

                //                LOG_DEBUG("buffer '%s' DumpArgs: %s", MacroBuffer::Dump(m_macro_analisys_buff).c_str(), MacroBuffer::Dump(macro_args).c_str());


                ASSERT(size_remove);
                ASSERT(size_remove <= m_macro_analisys_buff.size());

                std::string temp = "";
                for (auto &elem : m_macro_analisys_buff) {
                    if (!temp.empty()) {
                        temp += " ";
                    }
                    temp += elem->m_text;
                }
                //                LOG_DEBUG("From: %s (remove %d)", temp.c_str(), (int) size_remove);

                m_macro_analisys_buff.erase(m_macro_analisys_buff.begin(), m_macro_analisys_buff.begin() + size_remove);

                if (macro_done->Right()->getTermID() == TermID::MACRO_STR) {

                    std::string macro_str = MacroBuffer::ExpandString(macro_done, macro_args);
                    lexer->source_string = std::make_shared<std::string>(MacroBuffer::ExpandString(macro_done, macro_args));
                    lexer->m_macro_iss = new std::istringstream(*lexer->source_string);
                    lexer->m_macro_loc = *lexer->m_loc; // save
                    lexer->m_loc->initialize();
                    lexer->yypush_buffer_state(lexer->yy_create_buffer(lexer->m_macro_iss, lexer->source_string->size()));

                    //                    if(lexer->m_data.size() > 100) {
                    //                        LOG_RUNTIME("Macro expansion '%s' stack overflow?", macro_done->toString().c_str());
                    //                    }
                    //
                    //                    std::string macro_str = MacroBuffer::ExpandString(macro_done, macro_args);
                    //                    lexer->source_string = std::make_shared<std::string>(MacroBuffer::ExpandString(macro_done, macro_args));
                    //                    lexer->m_data.push_back({lexer->source_string, new std::istringstream(*lexer->source_string), *lexer->m_loc});
                    //                    lexer->m_loc->initialize();
                    //                    lexer->yypush_buffer_state(lexer->yy_create_buffer(lexer->m_data[lexer->m_data.size() - 1].iss, lexer->source_string->size()));

                    m_is_lexer_complete = false;
                    goto go_parse_string;

                } else {

                    ASSERT(macro_done->Right());
                    BlockType macro_block = MacroBuffer::ExpandMacros(macro_done, macro_args);
                    m_macro_analisys_buff.insert(m_macro_analisys_buff.begin(), macro_block.begin(), macro_block.end());

                    std::string temp = "";
                    for (auto &elem : m_macro_analisys_buff) {
                        if (!temp.empty()) {
                            temp += " ";
                        }
                        temp += elem->m_text;
                    }
                    //                    LOG_DEBUG("To: %s", temp.c_str());
                }

                continue;

            } else {
                //                if(m_macro_analisys_buff[0]->getTermID() == TermID::MACRO) { // || found != m_macro->end()

                LOG_RUNTIME("Macro mapping '%s' not found!\nThe following macro mapping are available:\n%s",
                        m_macro_analisys_buff[0]->toString().c_str(),
                        m_macro->GetMacroMaping(MacroBuffer::toHash(m_macro_analisys_buff[0]), "\n").c_str());

                //                }
            }
        }

        break;

    }

    //LOG_DEBUG("LexerToken count %d", (int) m_prep_buff.size());

    if (!m_macro_analisys_buff.empty()) {

        //        LOG_DEBUG("%d  %s", (int)m_prep_buff.at(0)->m_lexer_type, m_prep_buff.at(0)->m_text.c_str());

        *yylval = m_macro_analisys_buff.at(0);
        *yylloc = m_macro_analisys_buff.at(0)->m_lexer_loc;
        result = m_macro_analisys_buff.at(0)->m_lexer_type;

        //        LOG_DEBUG("Token (%d=%s): '%s'", result, toString((*yylval)->m_id), (*yylval)->m_text.c_str());

        if (m_postlex) {

            m_postlex->push_back((*yylval)->m_text);

            // Раскрыть последовательность токенов, т.к. они собираются в термин в лексере, а не парсере
            if ((*yylval)->getTermID() == TermID::MACRO_SEQ || (*yylval)->getTermID() == TermID::MACRO_DEL) {
                for (int i = 0; i < (*yylval)->m_macro_seq.size(); i++) {
                    m_postlex->push_back((*yylval)->m_macro_seq[i]->m_text);
                }
                m_postlex->push_back((*yylval)->m_text);
            }

        }

        m_macro_analisys_buff.erase(m_macro_analisys_buff.begin());
        return result;
    }

    *yylval = nullptr;
    return parser::token_type::END;
}

