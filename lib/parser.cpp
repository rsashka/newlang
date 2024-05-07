
#include "parser.h"
#include "lexer.h"
#include "builtin.h"
#include "system.h"

#include "term.h"
#include "context.h"

using namespace newlang;

int Parser::m_counter = 0;

Parser::Parser(MacroPtr macro, PostLexerType *postlex, DiagPtr diag, bool pragma_enable, RunTime *rt) {
    char *source_date_epoch = std::getenv("SOURCE_DATE_EPOCH");
    if (source_date_epoch) {
        std::istringstream iss(source_date_epoch);
        iss >> m_timestamp;
        if (iss.fail() || !iss.eof()) {
            LOG_RUNTIME("Error: Cannot parse SOURCE_DATE_EPOCH (%s) as integer", source_date_epoch);
        }
    } else {
        m_timestamp = std::time(NULL);
    }

    m_rt = rt;
    m_is_runing = false;
    m_is_lexer_complete = false;

    m_filename = "";
    m_file_time = "??? ??? ?? ??:??:?? ????";
    m_file_md5 = "??????????????????????????????";
    m_macro = macro;
    m_postlex = postlex;
    m_diag = diag; // ? diag : Diag::Init();
    m_annotation = Term::Create(TermID::ARGS, "", parser::token_type::ARGS);
    m_no_macro = false;
    m_enable_pragma = pragma_enable;
    m_name_module = "\\\\__main__";

    m_ast = nullptr;
}


TermPtr Parser::Parse(const std::string input) {

    m_ast = Term::Create(TermID::END, "");
    //    m_ast->SetSource(std::make_shared<std::string>(input));
    m_stream.str(input);
    m_stream.clear();
    Scanner scanner(&m_stream, &std::cout, std::make_shared<std::string>(input));
    scanner.ApplyDiags(m_diag);

    lexer = &scanner;

    parser parser(*this);
    if (parser.parse() != 0) {
        return m_ast;
    }

    // Исходники требуются для вывода информации ошибках во время анализа типов
    m_ast->SetSource(m_ast->m_source);
    lexer = nullptr;
    return m_ast;
}

TermPtr Parser::ParseString(const std::string str, MacroPtr macro, PostLexerType *postlex, DiagPtr diag, RuntimePtr rt) {
    Parser p(macro, postlex, diag, true, rt.get());
    return p.Parse(str);
}

void Parser::error(const class location& l, const std::string& m) {
    std::cerr << l << ": " << m << std::endl;
}

void Parser::error(const std::string &m) {
    std::cerr << m << std::endl;
}

void newlang::parser::error(const parser::location_type& l, const std::string& msg) {
    //    std::string buffer(driver.m_stream.str());
    std::string buffer(*(driver.lexer->source_string));
    std::cout << driver.lexer->source_string->c_str() << "\n\n";
    std::cout.flush();
    ParserException(msg.c_str(), buffer, l.begin.line, l.begin.column);
}

TermPtr Parser::GetAst() {
    return m_ast;
}

void Parser::AstAddTerm(TermPtr &term) {
    ASSERT(m_ast);
    term->m_source = lexer->source_string;
    if (m_ast->m_id == TermID::END) {
        m_ast = term;
        m_ast->ConvertSequenceToBlock(TermID::SEQUENCE, false);
    } else if (!m_ast->isBlock()) {
        m_ast->ConvertSequenceToBlock(TermID::SEQUENCE, true);
        m_ast->m_namespace = nullptr;
    } else {
        ASSERT(m_ast->isBlock());
        m_ast->m_block.push_back(term);
    }
}

// stub

__attribute__ ((weak)) TermPtr newlang::ProcessMacro(Parser &, TermPtr &term) {
    return term;
}

__attribute__ ((weak)) ExpandMacroResult newlang::ExpandTermMacro(Parser &parser) {
    return ExpandMacroResult::Break;
}

//TermPtr Parser::MacroEval(const TermPtr &term) {
//
//    if (!m_macro && m_diag) {
//        m_diag->Emit(Diag::DIAG_MACRO_NOT_FOUND, term);
//        return term;
//    }
//
//    if (term->m_bracket_depth) {
//        NL_PARSER(term, "Macro definitions allowed at the top level only, not inside conditions, namespace or any brackets!");
//    }
//
//    if (m_macro) {
//        return m_macro->EvalOpMacros(term);
//    }
//    return term;
//}

bool Parser::PragmaCheck(const TermPtr& term) {
    if (term && term->m_text.size() > 5
            && term->m_text.find("@__") == 0
            && term->m_text.rfind("__") == term->m_text.size() - 2) {
        return !CheckPredefMacro(term);
    }
    return false;
}

bool Parser::PragmaEval(const TermPtr &term, BlockType &buffer, BlockType &seq) {
    /*
     * 
     * https://javarush.com/groups/posts/1896-java-annotacii-chto-ehto-i-kak-ehtim-poljhzovatjhsja
     * https://habr.com/ru/companies/otus/articles/764244/
     * 
        @__PRAGMA_DIAG__(push)
        @__PRAGMA_DIAG__(ignored, "-Wundef")
        @__PRAGMA_DIAG__(warning, "-Wformat" , "-Wundef", "-Wuninitialized")
        @__PRAGMA_DIAG__(error, "-Wuninitialized")
        @__PRAGMA_DIAG__(pop)

     @__PRAGMA_DIAG__(once, "-Wuninitialized") ??????????????

        #pragma message "Compiling " __FILE__ "..."
        @__PRAGMA_MESSAGE__("Compiling ", __FILE__, "...")


    #define DO_PRAGMA(x) _Pragma (#x)
    #define TODO(x) DO_PRAGMA(message ("TODO - " #x))

    @@TODO( ... )@@ := @__PRAGMA_MESSAGE__("TODO - ", @#...)

    @TODO(Remember to fix this)  # note: TODO - Remember to fix this

    \\.__lexer__ignore_space__ = 1;
    \\.__lexer__ignore_indent__ = 1;
    \\.__lexer__ignore_comment__ = 1;
    \\.__lexer__ignore_crlf__ = 1;
     * 
     *  @__PRAGMA_MACRO__(push)
     *  @__PRAGMA_MACRO__(push, if, this)
     *  @__PRAGMA_MACRO__(pop)
     *  @__PRAGMA_MACRO__(pop, if, this)
     * 
     *  @__PRAGMA_MACRO_COND__(ndef, if)  @__PRAGMA_ERROR__("Macro if not defined!")
     *  @__PRAGMA_MACRO_COND__(lt, __VERSION__, 0.5)  @__PRAGMA_ERROR__("This functional supported since version 0.5 only!")
     */

    static const char * __PRAGMA_DIAG__ = "@__PRAGMA_DIAG__";
    static const char * __PRAGMA_TYPE_DEFINE__ = "@__PRAGMA_TYPE_DEFINE__";
    static const char * __PRAGMA_IGNORE__ = "@__PRAGMA_IGNORE__";
    static const char * __PRAGMA_MACRO__ = "@__PRAGMA_MACRO__";
    static const char * __PRAGMA_MACRO_COND__ = "@__PRAGMA_MACRO_COND__";

    static const char * __PRAGMA_MESSAGE__ = "@__PRAGMA_MESSAGE__";
    static const char * __PRAGMA_WARNING__ = "@__PRAGMA_WARNING__";
    static const char * __PRAGMA_ERROR__ = "@__PRAGMA_ERROR__";

    static const char * __PRAGMA_EXPECTED__ = "@__PRAGMA_EXPECTED__";
    static const char * __PRAGMA_UNEXPECTED__ = "@__PRAGMA_UNEXPECTED__";
    static const char * __PRAGMA_FINALIZE__ = "@__PRAGMA_FINALIZE__";

    static const char * __PRAGMA_NO_MACRO__ = "@__PRAGMA_NO_MACRO__";

    static const char * __PRAGMA_LOCATION__ = "@__PRAGMA_LOCATION__";

    static const char * __ANNOTATION_SET__ = "@__ANNOTATION_SET__";
    static const char * __ANNOTATION_CHECK__ = "@__ANNOTATION_CHECK__";
    static const char * __ANNOTATION_IIF__ = "@__ANNOTATION_IIF__";

    static const char * __PRAGMA_STATIC_ASSERT__ = "@__PRAGMA_STATIC_ASSERT__";

    ASSERT(term);
    if (term->m_text.compare(__PRAGMA_DIAG__) == 0) {
        if (term->size() == 0) {
            NL_PARSER(term, "Expected argument in pragma '%s'", term->toString().c_str());
        }

        ASSERT(m_diag);
        Diag::State state;
        if (term->at(0).second->m_text.compare("push") == 0) {

            m_diag->Push(term);
            return true;

        } else if (term->at(0).second->m_text.compare("pop") == 0) {

            m_diag->Pop(term);
            return true;

        } else if (term->at(0).second->m_text.compare(Diag::toString(Diag::State::ignored)) == 0) {
            state = Diag::State::ignored;
        } else if (term->at(0).second->m_text.compare(Diag::toString(Diag::State::warning)) == 0) {
            state = Diag::State::warning;
        } else if (term->at(0).second->m_text.compare(Diag::toString(Diag::State::error)) == 0) {
            state = Diag::State::error;
        } else {
            NL_PARSER(term, "Pragma '%s' not recognized!", term->toString().c_str());
        }

        for (int i = 1; i < term->size(); i++) {
            m_diag->Apply(term->at(i).second->m_text.c_str(), state, term);
        }
    } else if (term->m_text.compare(__PRAGMA_TYPE_DEFINE__) == 0) {

        if (term->size() != 1 || !term->at(1).first.empty()) {
            NL_PARSER(term, "Expected argument in pragma macro '%s'", term->toString().c_str());
        }

        if (term->at(1).first.empty() && term->at(1).second->getTermID() == TermID::INTEGER && m_rt) {
            m_rt->m_typedef_limit = std::stoi(term->at(0).second->getText().c_str()); //parseInteger
            return true;
        }

    } else if (term->m_text.compare(__PRAGMA_MESSAGE__) == 0
            || term->m_text.compare(__PRAGMA_WARNING__) == 0
            || term->m_text.compare(__PRAGMA_ERROR__) == 0) {

        std::string message;
        for (int i = 0; i < term->size(); i++) {
            message += term->at(i).second->m_text;
        }

        Logger::LogLevelType save = Logger::Instance()->GetLogLevel();
        Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
        if (term->m_text.compare(__PRAGMA_MESSAGE__) == 0) {
            LOG_INFO("note: %s", message.c_str());
        } else if (term->m_text.compare(__PRAGMA_WARNING__) == 0) {
            LOG_WARNING("warn: %s", message.c_str());
        } else {
            ASSERT(term->m_text.compare(__PRAGMA_ERROR__) == 0);
            LOG_RUNTIME("error: %s", message.c_str());
        }
        Logger::Instance()->SetLogLevel(save);

    } else if (term->m_text.compare(__PRAGMA_IGNORE__) == 0) {

        LOG_RUNTIME("Pragma @__PRAGMA_IGNORE__ not implemented!");

        static const char * ignore_space = "space";
        static const char * ignore_indent = "indent";
        static const char * ignore_comment = "comment";
        static const char * ignore_crlf = "crlf";

    } else if (term->m_text.compare(__PRAGMA_MACRO__) == 0) {

        if (term->size() == 0) {
            NL_PARSER(term, "Expected argument in pragma macro '%s'", term->toString().c_str());
        }

        Diag::State state;
        if (term->at(0).second->m_text.compare("push") == 0) {

            LOG_RUNTIME("Pragma push not implemented!");
            //            m_macro->Push(term);
            return true;

        } else if (term->at(0).second->m_text.compare("pop") == 0) {

            LOG_RUNTIME("Pragma pop not implemented!");
            //            m_macro->Pop(term);
            return true;

            //        } else if(term->at(0).second->m_text.compare(Diag::toString(Diag::State::ignored)) == 0) {
            //            state = Diag::State::ignored;
            //        } else if(term->at(0).second->m_text.compare(Diag::toString(Diag::State::warning)) == 0) {
            //            state = Diag::State::warning;
            //        } else if(term->at(0).second->m_text.compare(Diag::toString(Diag::State::error)) == 0) {
            //            state = Diag::State::error;
        } else {
            NL_PARSER(term, "Pragma macro '%s' not recognized!", term->toString().c_str());
        }

        //        for (int i = 1; i < term->size(); i++) {
        //            m_diag->Apply(term->at(i).second->m_text.c_str(), state, term);
        //        }
    } else if (term->m_text.compare(__PRAGMA_MACRO_COND__) == 0) {

        if (term->size() == 0) {
            NL_PARSER(term, "Expected argument in pragma macro '%s'", term->toString().c_str());
        }

        LOG_RUNTIME("Pragma @__PRAGMA_MACRO_COND__ not implemented!");


    } else if (term->m_text.compare(__PRAGMA_EXPECTED__) == 0) {

        m_expected = term;

    } else if (term->m_text.compare(__PRAGMA_UNEXPECTED__) == 0) {

        m_unexpected = term;

    } else if (term->m_text.compare(__PRAGMA_FINALIZE__) == 0) {

        if (m_finalize || m_finalize_counter) {
            LOG_RUNTIME("Nested definitions of pragma @__PRAGMA_FINALIZE__ not implemented!");
        }

        m_finalize = term;
        m_finalize_counter = 0;

    } else if (term->m_text.compare(__PRAGMA_NO_MACRO__) == 0) {

        ASSERT(term->size() == 0);

        m_no_macro = true;

    } else if (term->m_text.compare(__PRAGMA_LOCATION__) == 0) {

        // #line 303 "location.hh"
        // Prototype - @__PRAGMA_LOCATION__( pop )
        // Prototype - @__PRAGMA_LOCATION__( push ) or @__PRAGMA_LOCATION__( push, 'filename') or @__PRAGMA_LOCATION__( push, 'filename', line)
        // Prototype - @__PRAGMA_LOCATION__( line ) or @__PRAGMA_LOCATION__( line, 'filename')
        if (term->size() == 1 && term->at(0).first.empty() && term->at(0).second->m_text.compare("pop") == 0) {

            if (m_loc_stack.empty()) {
                NL_PARSER(term, "Empty stack location!");
            }
            m_filename = m_loc_stack[m_loc_stack.size() - 1].filename;
            m_location = m_loc_stack[m_loc_stack.size() - 1].location;
            m_loc_stack.pop_back();
            return true;

        } else if (term->size() >= 1 && term->at(0).first.empty() && term->at(0).second->m_text.compare("push") == 0) {

            if (term->size() == 1) {
                m_loc_stack.push_back({m_filename, m_location});
                return true;
            } else if (term->size() >= 2 && term->at(1).first.empty() && term->at(1).second->isString()) {

                m_loc_stack.push_back({m_filename, m_location});
                m_filename = term->at(1).second->getText();

                if (term->size() == 2) {
                    return true;
                } else if (term->size() == 3 && term->at(2).first.empty() && term->at(2).second->getTermID() == TermID::INTEGER) {
                    m_location.begin.line = std::stoi(term->at(0).second->getText().c_str());
                    m_location.end.line = m_location.begin.line;
                    return true;
                }
            }

        } else if (term->size() >= 1 && term->at(0).first.empty() && term->at(0).second->getTermID() == TermID::INTEGER) {

            m_location.begin.line = std::stoi(term->at(0).second->getText().c_str());
            m_location.end.line = m_location.begin.line;
            if (term->size() == 1) {
                return true;
            } else if (term->size() == 2 && term->at(1).first.empty() && term->at(1).second->isString()) {
                m_filename = term->at(1).second->getText();
                m_location.begin.filename = &m_filename;
                //                m_loc_stack.push_back(m_location);
                return true;
            }
        }
        NL_PARSER(term, "See @__PRAGMA_LOCATION__ for usage and syntax help.");


    } else if (term->m_text.compare(__ANNOTATION_SET__) == 0) {

        if (term->size() == 1) {
            // Set `name` = 1;
            std::string name = term->at(0).second->m_text;
            LOG_DEBUG("NAME: %s", name.c_str());

            auto iter = m_annotation->find(term->at(0).second->m_text);
            if (iter == m_annotation->end()) {
                m_annotation->push_back(Term::Create(TermID::INTEGER, "1", parser::token_type::INTEGER, 1, &term->m_lexer_loc, term->m_source), name);
            } else {
                //                iter->second =
                m_annotation->push_back(Term::Create(TermID::INTEGER, "1", parser::token_type::INTEGER, 1, &term->m_lexer_loc, term->m_source), name);
            }


        } else if (term->size() == 2) {
            // Set `name` = value;
            m_annotation->push_back(term->at(1).second, term->at(0).second->m_text);
        } else {
            NL_PARSER(term, "Annotation args in '%s' not recognized!", term->toString().c_str());
        }

        //        LOG_DEBUG("ANNOT: %s", m_annotation->toString().c_str());

    } else if (term->m_text.compare(__ANNOTATION_CHECK__) == 0) {

        ASSERT(m_annotation);

        LOG_RUNTIME("Pragma __ANNOTATION_CHECK__ not implemented!");

        //        if (term->size() != 3) {
        //            NL_PARSER(term, "Annotation IIF must have three arguments!");
        //        }
        //
        //        //        LOG_DEBUG("Annot %s %d", m_annotation->toString().c_str(), (int) m_annotation->size());
        //
        //        auto iter = m_annotation->find(term->at(0).second->m_text);
        //        if (iter == m_annotation->end() || iter->second->m_text.empty() || iter->second->m_text.compare("0") == 0) {
        //            buffer.insert(buffer.begin(), term->at(2).second);
        //        } else {
        //            buffer.insert(buffer.begin(), term->at(1).second);
        //        }

    } else if (term->m_text.compare(__ANNOTATION_IIF__) == 0) {

        ASSERT(m_annotation);

        if (term->size() != 3) {
            NL_PARSER(term, "Annotation IIF must have three arguments!");
        }

        //        LOG_DEBUG("Annot %s %d", m_annotation->toString().c_str(), (int) m_annotation->size());

        auto iter = m_annotation->find(term->at(0).second->m_text);
        if (iter == m_annotation->end() || iter->second->m_text.empty() || iter->second->m_text.compare("0") == 0) {
            buffer.insert(buffer.begin(), term->at(2).second);
        } else {
            buffer.insert(buffer.begin(), term->at(1).second);
        }

    } else if (term->m_text.compare(__PRAGMA_STATIC_ASSERT__) == 0) {

        return PragmaStaticAssert(term);

    } else {
        NL_PARSER(term, "Uknown pragma '%s'", term->toString().c_str());
    }
    return true;
}

bool Parser::PragmaStaticAssert(const TermPtr &term) {

    if (term->size() < 1 || !term->at(0).first.empty()) {
        NL_PARSER(term, "Agruments '%s' not recognized! See @__PRAGMA_STATIC_ASSERT__ for usage and syntax help.", term->toString().c_str());
    }

    bool done;
    try {
        done = Context::EvalTerm(term->at(0).second, nullptr)->GetValueAsBoolean();
    } catch (...) {
        done = false;
    }
    if (!done) {
        NL_PARSER(term, "StaticAssert '%s' failed!", term->at(0).second->toString().c_str());
    }
    return true;
}

std::string newlang::ParserMessage(std::string &buffer, int row, int col, const char *format, ...) {

    char va_buffer[1024];

    va_list args;
    va_start(args, format);
    vsnprintf(va_buffer, sizeof (va_buffer), format, args);
    va_end(args);

    std::string message(va_buffer);

    if (row) { // Если переданы координаты ошибки
        message += " at line ";
        message += std::to_string(row);
        message += " col ";
        message += std::to_string(col - 1);
        message += "\n";
    }

    // Ищем нужную строку
    size_t pos = 0;
    if (buffer.find("\n") != std::string::npos) {
        int count = 1;
        while (count < row) {
            pos = buffer.find("\n", pos + 1);
            count++;
        }
    }
    // Убираем лишние переводы строк
    std::string tmp = buffer.substr((pos ? pos + 1 : pos), buffer.find("\n", pos + 1));
    tmp = tmp.substr(0, tmp.find("\n", col));

    if (row) { // Если переданы координаты ошибки, показываем место

        // Лексер обрабатывает строки в байтах, а вывод в UTF8
        // поэтому позиция ошибки лексера может не совпадать для многобайтных символов
        std::wstring_convert < std::codecvt_utf8<wchar_t>, wchar_t> converter;
        std::wstring wstr = converter.from_bytes(tmp.substr(0, col));

        message += tmp;
        if (col > 1) {
            message += "\n";
            std::string placeholder(col - 2 - (tmp.substr(0, col).size() - wstr.size()), ' ');
            placeholder += "^   ";
            message += placeholder;
        }
    } else {

        message += tmp;
    }

    return message;
}

void newlang::ParserException(const char *msg, std::string &buffer, int row, int col) {
    throw ParserError(ParserMessage(buffer, row, col, "%s", msg));
}

bool Parser::RegisterPredefMacro(const char * name, const char * desc) {
    if (m_predef_macro.find(name) != m_predef_macro.end()) {
        LOG_ERROR("Predef macro '%s' redefined!", name);
        return false;
    }
    m_predef_macro.insert({name, desc});
    return true;
}

void Parser::InitPredefMacro() {
    if (m_predef_macro.empty()) {

        VERIFY(RegisterPredefMacro("@__NLC_VER__", "Version NewLang Compiler."));
        VERIFY(RegisterPredefMacro("@__NLC_SOURCE_GIT__", "Git source code identifier of the current compiler version."));
        VERIFY(RegisterPredefMacro("@__NLC_DATE_BUILD__", "Date build of the current compiler version."));
        VERIFY(RegisterPredefMacro("@__NLC_SOURCE_BUILD__", "Git source code identifier and date build of the current compiler version."));

        VERIFY(RegisterPredefMacro("@__FILE__", "Current file name"));
        VERIFY(RegisterPredefMacro("@__FILE_NAME__", "Current file name"));

        VERIFY(RegisterPredefMacro("@__CLASS__", "Current class name"));
        VERIFY(RegisterPredefMacro("@__NAMESPACE__", "Current namespace"));
        VERIFY(RegisterPredefMacro("@__FUNCTION__", "Current function name"));
        VERIFY(RegisterPredefMacro("@__FUNCDNAME__", "Decorated of current function name"));
        VERIFY(RegisterPredefMacro("@__FUNCSIG__", "Signature of current function"));
//        VERIFY(RegisterPredefMacro("@__FUNC_BLOCK__", "Full namespace function name"));



        VERIFY(RegisterPredefMacro("@__LINE__", "Line number in current file"));
        VERIFY(RegisterPredefMacro("@__FILE_LINE__", "Line number in current file"));

        VERIFY(RegisterPredefMacro("@__FILE_MD5__", "MD5 hash for current file"));
        VERIFY(RegisterPredefMacro("@__FILE_TIMESTAMP__", "Timestamp current file"));

        VERIFY(RegisterPredefMacro("@__DATE__", "Current date"));
        VERIFY(RegisterPredefMacro("@__TIME__", "Current time"));
        // определяется как строковый литерал, содержащий дату и время последнего изменения текущего исходного файла 
        //в сокращенной форме с постоянной длиной, которые возвращаются функцией asctime библиотеки CRT, 
        //например: Fri 19 Aug 13:32:58 2016. Этот макрос определяется всегда.
        VERIFY(RegisterPredefMacro("@__TIMESTAMP__", "Current timestamp"));
        VERIFY(RegisterPredefMacro("@__TIMESTAMP_ISO__", "Current timestamp as ISO format")); // 2013-07-06T00:50:06Z

        //Развертывается до целочисленного литерала, начинающегося с 0. 
        //Значение увеличивается на 1 каждый раз, когда используется в файле исходного кода или во включенных заголовках файла исходного кода. 
        VERIFY(RegisterPredefMacro("@__COUNTER__", "Monotonically increasing counter from zero"));

        VERIFY(RegisterPredefMacro("@::", "Full name of the current namespace"));
        VERIFY(RegisterPredefMacro("$\\\\", "Full name of the current module name"));
        VERIFY(RegisterPredefMacro("@\\\\", "Root directory with the main program module"));
    }
}

bool Parser::CheckPredefMacro(const TermPtr & term) {
    if (term->m_id != TermID::NAME) {
        return false;
    }

    std::string_view text = term->m_text;
    if (text.find("@") == 0) {
        text.remove_prefix(1);
    }

    InitPredefMacro();
    return m_predef_macro.find(text.begin()) != m_predef_macro.end();
}

std::string Parser::GetCurrentDate(time_t ts) {
    std::string buf("Jul 27 2012");
    strftime(buf.data(), buf.size(), "%b %e %Y", localtime(&ts));
    return buf;
}

std::string Parser::GetCurrentTime(time_t ts) {
    std::string buf("07:07:09");
    strftime(buf.data(), buf.size(), "%T", localtime(&ts));
    return buf;
}

std::string Parser::GetCurrentTimeStamp(time_t ts) {
    std::string result = asctime(localtime(&ts));
    result = result.substr(0, 24); // Remove \n on the end line
    return result;
}

std::string Parser::GetCurrentTimeStampISO(time_t ts) {
    std::string buf("2011-10-08T07:07:09Z");
    strftime(buf.data(), buf.size(), "%FT%TZ", localtime(&ts));
    return buf;
}

parser::token_type Parser::ExpandPredefMacro(TermPtr & term) {

    InitPredefMacro();

    if (!term) {
        LOG_RUNTIME("Environment variable not defined!");
    }
    if (term->m_id != TermID::MACRO) {
        return term->m_lexer_type;
    }

    std::string_view text = term->m_text;
    //    if (text.find("@") == 0) {
    //        text.remove_prefix(1);
    //    }

    ASSERT(!m_predef_macro.empty());
    if (m_predef_macro.find(text.begin()) == m_predef_macro.end()) {
        return term->m_lexer_type;
    }


    const TermID str_type = TermID::STRWIDE;
    const parser::token_type str_token = parser::token_type::STRWIDE;

    if (text.compare("@__COUNTER__") == 0) {

        term->m_id = TermID::INTEGER;
        term->m_text = std::to_string(m_counter);
        m_counter++;
        term->m_lexer_type = parser::token_type::INTEGER;
        return term->m_lexer_type;

    } else if (text.compare("@__NLC_VER__") == 0) {

        term->m_id = TermID::INTEGER;
        term->m_text = std::to_string(VERSION);
        term->m_lexer_type = parser::token_type::INTEGER;
        return term->m_lexer_type;

    } else if (text.compare("@__NLC_SOURCE_GIT__") == 0) {
        term->m_text = GIT_SOURCE;
        term->m_id = str_type;
        term->m_lexer_type = str_token;
        return term->m_lexer_type;

    } else if (text.compare("@__NLC_DATE_BUILD__") == 0) {
        term->m_text = DATE_BUILD_STR;
        term->m_id = str_type;
        term->m_lexer_type = str_token;
        return term->m_lexer_type;

    } else if (text.compare("@__NLC_SOURCE_BUILD__") == 0) {
        term->m_text = SOURCE_FULL_ID;
        term->m_id = str_type;
        term->m_lexer_type = str_token;
        return term->m_lexer_type;

    } else if (text.compare("@__LINE__") == 0 || text.compare("@__FILE_LINE__") == 0) {

        term->m_id = TermID::INTEGER;
        term->m_text = std::to_string(m_location.begin.line);
        term->m_lexer_type = parser::token_type::INTEGER;
        return term->m_lexer_type;

    } else if (text.compare("@__FILE__") == 0 || text.compare("@__FILE_NAME__") == 0) {

        term->m_id = str_type;
        if (!m_filename.empty()) {
            term->m_text = m_filename;
        } else {
            term->m_text = "File name undefined!!!";
        }
        term->m_lexer_type = str_token;
        return term->m_lexer_type;

    } else if (text.compare("@__FILE_TIMESTAMP__") == 0) {

        term->m_id = str_type;
        if (!m_file_time.empty()) {
            term->m_text = m_file_time;
        } else {
            term->m_text = "??? ??? ?? ??:??:?? ????";
        }
        term->m_lexer_type = str_token;
        return term->m_lexer_type;

    } else if (text.compare("@__FILE_MD5__") == 0) {

        term->m_id = str_type;
        if (!m_file_md5.empty()) {
            term->m_text = m_file_md5;
        } else {
            term->m_text = "?????????????????????????????????";
        }
        term->m_lexer_type = str_token;
        return term->m_lexer_type;


    } else if (text.compare("@__DATE__") == 0) {

        term->m_text = GetCurrentDate(m_timestamp);
        term->m_id = str_type;
        term->m_lexer_type = str_token;
        return term->m_lexer_type;

    } else if (text.compare("@__TIME__") == 0) {

        term->m_text = GetCurrentTime(m_timestamp);
        term->m_id = str_type;
        term->m_lexer_type = str_token;
        return term->m_lexer_type;

    } else if (text.compare("@__TIMESTAMP__") == 0) {

        term->m_text = GetCurrentTimeStamp(m_timestamp);
        term->m_text = term->m_text.substr(0, 24); // Remove \n on the end line
        term->m_id = str_type;
        term->m_lexer_type = str_token;
        return term->m_lexer_type;

    } else if (text.compare("@__TIMESTAMP_ISO__") == 0) {

        term->m_text = GetCurrentTimeStampISO(m_timestamp);
        term->m_id = str_type;
        term->m_lexer_type = str_token;
        return term->m_lexer_type;

    } else if (text.compare("@__CLASS__") == 0 || text.compare("@__NAMESPACE__") == 0 || //text.compare("@__FUNC_BLOCK__") == 0 ||
            text.compare("@__FUNCTION__") == 0 || text.compare("@__FUNCDNAME__") == 0 || text.compare("@__FUNCSIG__") == 0) {

        term->m_id = TermID::NAMESPACE;
        term->m_lexer_type = parser::token_type::NAMESPACE;
        return term->m_lexer_type;

    } else if (text.compare("@::") == 0) {

        term->m_id = TermID::NAMESPACE;
        term->m_lexer_type = parser::token_type::NAMESPACE;
        return term->m_lexer_type;

        //        ASSERT(text.compare("@::") != 0);

    } else if (text.compare("@$$") == 0) {

        term->m_id = TermID::NAMESPACE;
        term->m_lexer_type = parser::token_type::NAMESPACE;
        return term->m_lexer_type;

        //        // Внешний блок или функция
        //        ASSERT(text.compare("@$$") != 0);

    } else if (text.compare("@\\\\") == 0) {

        if (m_rt) {
            term->m_text = m_rt->m_exec_dir;
        }
        term->m_id = TermID::NAME;
        term->m_lexer_type = parser::token_type::NAME;
        return term->m_lexer_type;

    } else if (text.compare("$\\\\") == 0) {

        term->m_text = GetCurrentModule();
        term->m_id = TermID::MODULE;
        term->m_lexer_type = parser::token_type::MODULE;
        return term->m_lexer_type;
    }

    NL_PARSER(term, "Predef macro '%s' not implemented!", term->toString().c_str());
}

TermPtr Parser::ParseTerm(const char *proto, MacroPtr macro, bool pragma_enable) {
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

size_t Parser::SkipBrackets(const BlockType& buffer, const size_t offset) {

    if (offset >= buffer.size()) {
        return 0;
    }

    std::string br_end;
    if (buffer[offset]->m_text.compare("(") == 0) {
        br_end = ")";
//    } else if (buffer[offset]->m_text.compare("<") == 0) {
//        br_end = ">";
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

size_t Parser::ParseTerm(TermPtr &result, const BlockType &buffer, size_t offset, bool pragma_enable) {

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
    //        LOG_DEBUG("ParseTerm: '%s' - %d", source.c_str(), (int) offset);
    result = ParseTerm(source.c_str(), nullptr, pragma_enable);
    return offset;
}

TermPtr Parser::LoadIfModule(const TermPtr & term) {
    return term;
}

TermPtr Parser::CheckLoadModule(const TermPtr & term) {
    if (!CheckCharModuleName(term->m_text.c_str())) {
        NL_PARSER(term, "Module name - backslash, underscore, lowercase English letters or number!");
    }
    return term;
}





using namespace newlang;


#define MAKE_TYPE(type_name) {type_name, Term::Create(TermID::TYPE, type_name)}

static const std::map<const std::string, const TermPtr> default_types{
    MAKE_TYPE(":None"),

    MAKE_TYPE(":Bool"),
    MAKE_TYPE(":Int8"),
    MAKE_TYPE(":Int16"),
    MAKE_TYPE(":Int32"),
    MAKE_TYPE(":Int64"),

    MAKE_TYPE(":Float16"),
    MAKE_TYPE(":Float32"),
    MAKE_TYPE(":Float64"),

    MAKE_TYPE(":Complex16"),
    MAKE_TYPE(":Complex32"),
    MAKE_TYPE(":Complex64"),

    MAKE_TYPE(":Rational"),

    MAKE_TYPE(":StrChar"),
    MAKE_TYPE(":StrWide"),

    MAKE_TYPE(":Range"),
    MAKE_TYPE(":Iterator"),

    MAKE_TYPE(":Dictionary"),

    MAKE_TYPE(":Any")};

#undef MAKE_TYPE

static const TermPtr type_default_none = default_types.find(":None")->second;
static const TermPtr type_default_any = default_types.find(":Any")->second;
//static const TermPtr type_default_dict = default_types.find(":Dict")->second;
static const TermPtr term_none = Term::Create(TermID::NAME, "_", parser::token_type::NAME);
static const TermPtr term_ellipsys = Term::Create(TermID::ELLIPSIS, "...", parser::token_type::ELLIPSIS);
static const TermPtr term_required = Term::Create(TermID::NONE, "_", parser::token_type::END);
static const ObjPtr obj_none = Obj::CreateNone();
static const ObjPtr obj_ellipsys = Obj::CreateType(ObjType::Ellipsis, ObjType::Ellipsis, true);

const TermPtr newlang::getNoneTerm() {
    return term_none;
}

const TermPtr newlang::getRequiredTerm() {
    return term_required;
}

const TermPtr newlang::getEllipsysTerm() {
    return term_ellipsys;
}

const ObjPtr newlang::getNoneObj() {
    return obj_none;
}

const ObjPtr newlang::getEllipsysObj() {
    return obj_ellipsys;
}

bool newlang::isDefaultType(const TermPtr & term) {
    if (term) {
        TermPtr def = getDefaultType(term->m_text);
        return def && (term.get() == def.get());
    }
    return true; // Not defined type as type default
}

const TermPtr newlang::getDefaultType(ObjType type) {
    return getDefaultType(toString(type));
}

const TermPtr newlang::getDefaultType(const std::string_view text) {
    if (!text.empty()) {
        auto iter = default_types.find(text.begin());
        if (iter != default_types.end()) {
            return iter->second;
        }
    } else {
        ASSERT(type_default_none->m_text.compare(":None") == 0);
        ASSERT(type_default_any->m_text.compare(":Any") == 0);

        return type_default_any;
    }
    return nullptr;
}

bool newlang::canCast(const TermPtr &from, const ObjType to) {
    if (!from->m_type) {
        return true; // Empty type cast any type
    }
    ObjType test_type = GetBaseTypeFromString(from->m_type->m_text);
    return canCast(test_type, to);
}

bool newlang::canCast(const TermPtr &from, const TermPtr &to) {
    if (!from || !to || !from->m_type || !to->m_type) {
        return true; // Empty type cast any type
    }
    return canCast(GetBaseTypeFromString(from->m_type->m_text), GetBaseTypeFromString(to->m_type->m_text));
}

ObjType newlang::GetBaseTypeFromString(const std::string_view type_arg, bool *has_error) {

    std::string type(type_arg);

    if (type.find("~") != std::string::npos) {
        type.erase(std::remove(type.begin(), type.end(), '~'), type.end());
    }

    if (type.empty()) {
        return ObjType::None;
    } else if (type.compare("_") == 0) {
        return ObjType::None;
    }

#define DEFINE_CASE(name, _)                    \
    else if (type.compare(":"#name) == 0) {     \
        return ObjType:: name;                  \
    }

    NL_TYPES(DEFINE_CASE)
#undef DEFINE_CASE

    if (has_error) {
        *has_error = true;
        return ObjType::None;
    }
    LOG_RUNTIME("Undefined type name '%s'!", type.c_str());
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

            term = Term::Create(TermID::END, "", parser::token_type::END, 0);
            type = lexer->lex(&term, &m_location);
            term->m_lexer_loc = m_location;

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


            //            if (type == parser::token_type::MACRO && term->m_text.compare("@::") == 0) {
            //                
            //                type = parser::token_type::NAMESPACE;
            //                term->m_lexer_type = type;
            //                term->m_id = TermID::NAMESPACE;
            //                term->m_text = m_ns_stack.NamespaceCurrent();
            //                        
            //            } else 

            //            if (type == MACRO_MODULE) {
            //
            //                type = parser::token_type::MODULE;
            //                term->m_lexer_type = type;
            //                term->m_id = TermID::MODULE;
            //                term->m_text = GetCurrentModule();
            //
            //            }


            //            if (m_next_string != static_cast<uint8_t> (TermID::NONE)) {
            //
            //                if (m_next_string == static_cast<uint8_t> (TermID::STRCHAR)) {
            //                    type = parser::token_type::STRCHAR;
            //                } else {
            //                    ASSERT(m_next_string == static_cast<uint8_t> (TermID::STRWIDE));
            //                    type = parser::token_type::STRWIDE;
            //                }
            //
            //                term->m_lexer_type = type;
            //                term->m_id = static_cast<TermID> (m_next_string);
            //
            //                m_next_string = static_cast<uint8_t> (TermID::NONE);
            //
            //            } else if (type == parser::token_type::MACRO_TOSTR && lexer->m_macro_count == 0) {
            //
            //                if (term->m_text.compare("@#\"") == 0) {
            //                    m_next_string = static_cast<uint8_t> (TermID::STRWIDE);
            //                } else if (term->m_text.compare("@#'") == 0) {
            //                    m_next_string = static_cast<uint8_t> (TermID::STRCHAR);
            //                } else {
            //                    ASSERT(term->m_text.compare("@#") == 0);
            //                    //@todo Set string type default by global settings
            //                    m_next_string = static_cast<uint8_t> (TermID::STRWIDE);
            //                }
            //
            //                goto next_escape_token;
            //            }



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

                if (lexer->m_macro_del && !(type == parser::token_type::NAME || type == parser::token_type::LOCAL)) {
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


    TermPtr pragma;
    while (lexer->m_macro_count == 0 && !m_macro_analisys_buff.empty()) {

        if (m_enable_pragma) {

            ExpandPredefMacro(m_macro_analisys_buff[0]);

            // Обработка команд парсера @__PRAGMA ... __
            if (PragmaCheck(m_macro_analisys_buff[0])) {

                size_t size;
                size = Parser::ParseTerm(pragma, m_macro_analisys_buff, 0, false);

                ASSERT(size);
                ASSERT(pragma);

                //                LOG_DEBUG("Pragma '%s' size %d", pragma->toString().c_str(), (int) size);

                BlockType temp(m_macro_analisys_buff.begin(), m_macro_analisys_buff.begin() + size);

                m_macro_analisys_buff.erase(m_macro_analisys_buff.begin(), m_macro_analisys_buff.begin() + size);

                //                while (!m_macro_analisys_buff.empty() && m_macro_analisys_buff[0]->m_text.compare(";") == 0) {
                //                    LOG_DEBUG("Erase '%s' !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!", m_macro_analisys_buff[0]->toString().c_str());
                //                    m_macro_analisys_buff.erase(m_macro_analisys_buff.begin());
                //                }


                PragmaEval(pragma, m_macro_analisys_buff, temp);
                continue;
            }
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

        switch (ExpandTermMacro(*this)) {
            case ExpandMacroResult::Continue:;
                continue;
            case ExpandMacroResult::Goto:;
                goto go_parse_string;
            default:
                break;
        };
        //        if (m_macro) {
        //
        //            // Макрос должне начинаться всегда с термина
        //            if (!(m_macro_analisys_buff[0]->getTermID() == TermID::MACRO || m_macro_analisys_buff[0]->getTermID() == TermID::NAME)) {
        //                break;
        //            }
        //
        //            TermPtr macro_done = nullptr;
        //
        //            // Итератор для списка макросов, один из которых может соответствовать текущему буферу (по первому термину буфера)
        //            Macro::iterator found = m_macro->map::find(Macro::toMacroHash(m_macro_analisys_buff[0]));
        //
        //            if (found == m_macro->end()) {
        //
        //                //                // Если макрос не найден - ошибка 
        //                //                if(isMacro(m_macro_analisys_buff[0]->m_text)) {
        //                //                    LOG_RUNTIME("Macro '%s' not found!", m_macro_analisys_buff[0]->toString().c_str());
        //                //                }
        //
        //                break;
        //            }
        //
        //            macro_done.reset();
        //            // Перебрать все макросы и сравнить с буфером
        //            for (auto iter = found->second.begin(); iter != found->second.end(); ++iter) {
        //
        //                if (Macro::IdentityMacro(m_macro_analisys_buff, *iter)) {
        //
        //                    if (macro_done) {
        //                        LOG_RUNTIME("Macro duplication %s and '%s'!", macro_done->toString().c_str(), (*iter)->toString().c_str());
        //                    }
        //                    macro_done = *iter;
        //                }
        //            }
        //
        //            ASSERT(found != m_macro->end());
        //
        //            if (macro_done) {
        //
        //                counter++;
        //                if (counter > 100) {
        //                    LOG_RUNTIME("Macro expansion '%s' stack overflow?", macro_done->toString().c_str());
        //                }
        //
        //                //                compare_macro = true; // Раскрывать макросы в теле раскрытого макроса
        //
        //                ASSERT(m_macro_analisys_buff.size() >= macro_done->m_macro_seq.size());
        //                ASSERT(macro_done->Right());
        //
        //                Macro::MacroArgsType macro_args;
        //                size_t size_remove = Macro::ExtractArgs(m_macro_analisys_buff, macro_done, macro_args);
        //
        //                //                LOG_TEST_DUMP("buffer '%s' DumpArgs: %s", Macro::Dump(m_macro_analisys_buff).c_str(), Macro::Dump(macro_args).c_str());
        //
        //
        //                ASSERT(size_remove);
        //                ASSERT(size_remove <= m_macro_analisys_buff.size());
        //
        //                std::string temp = "";
        //                for (auto &elem : m_macro_analisys_buff) {
        //                    if (!temp.empty()) {
        //                        temp += " ";
        //                    }
        //                    temp += elem->m_text;
        //                    temp += ":";
        //                    temp += toString(elem->m_id);
        //                }
        //                //                LOG_TEST("From: %s (remove %d)", temp.c_str(), (int) size_remove);
        //
        //                m_macro_analisys_buff.erase(m_macro_analisys_buff.begin(), m_macro_analisys_buff.begin() + size_remove);
        //
        //                if (macro_done->Right()->getTermID() == TermID::MACRO_STR) {
        //
        //                    std::string macro_str = Macro::ExpandString(macro_done, macro_args);
        //                    lexer->source_string = std::make_shared<std::string>(Macro::ExpandString(macro_done, macro_args));
        //                    lexer->m_macro_iss = new std::istringstream(*lexer->source_string);
        //                    lexer->m_macro_loc = *lexer->m_loc; // save
        //                    lexer->m_loc->initialize();
        //                    lexer->yypush_buffer_state(lexer->yy_create_buffer(lexer->m_macro_iss, lexer->source_string->size()));
        //
        //                    //                    if(lexer->m_data.size() > 100) {
        //                    //                        LOG_RUNTIME("Macro expansion '%s' stack overflow?", macro_done->toString().c_str());
        //                    //                    }
        //                    //
        //                    //                    std::string macro_str = MacroBuffer::ExpandString(macro_done, macro_args);
        //                    //                    lexer->source_string = std::make_shared<std::string>(MacroBuffer::ExpandString(macro_done, macro_args));
        //                    //                    lexer->m_data.push_back({lexer->source_string, new std::istringstream(*lexer->source_string), *lexer->m_loc});
        //                    //                    lexer->m_loc->initialize();
        //                    //                    lexer->yypush_buffer_state(lexer->yy_create_buffer(lexer->m_data[lexer->m_data.size() - 1].iss, lexer->source_string->size()));
        //
        //                    m_is_lexer_complete = false;
        //                    goto go_parse_string;
        //
        //                } else {
        //
        //                    ASSERT(macro_done->Right());
        //                    BlockType macro_block = Macro::ExpandMacros(macro_done, macro_args);
        //                    m_macro_analisys_buff.insert(m_macro_analisys_buff.begin(), macro_block.begin(), macro_block.end());
        //
        //                    std::string temp = "";
        //                    for (auto &elem : m_macro_analisys_buff) {
        //                        if (!temp.empty()) {
        //                            temp += " ";
        //                        }
        //                        temp += elem->m_text;
        //                        temp += ":";
        //                        temp += toString(elem->m_id);
        //                    }
        //                    //                    LOG_TEST("To: %s", temp.c_str());
        //                }
        //
        //                continue;
        //
        //            } else {
        //                //                if(m_macro_analisys_buff[0]->getTermID() == TermID::MACRO) { // || found != m_macro->end()
        //
        //                LOG_RUNTIME("Macro mapping '%s' not found!\nThe following macro mapping are available:\n%s",
        //                        m_macro_analisys_buff[0]->toString().c_str(),
        //                        m_macro->GetMacroMaping(Macro::toMacroHash(m_macro_analisys_buff[0]), "\n").c_str());
        //
        //                //                }
        //            }
        //        }

        break;

    }

    //LOG_DEBUG("LexerToken count %d", (int) m_prep_buff.size());

    if (!m_macro_analisys_buff.empty()) {

        //        if (m_macro_analisys_buff[0]->m_id == TermID::END) {
        //            *yylval = nullptr;
        //            return parser::token_type::END;
        //        }

        //        if (m_macro_analisys_buff.at(0)->m_id) {
        //        LOG_DEBUG("%d  %s", (int)m_prep_buff.at(0)->m_lexer_type, m_prep_buff.at(0)->m_text.c_str());

        *yylval = m_macro_analisys_buff.at(0);
        *yylloc = m_macro_analisys_buff.at(0)->m_lexer_loc;
        result = m_macro_analisys_buff.at(0)->m_lexer_type;

        //        LOG_TEST("Token (%d=%s): '%s'", result, toString((*yylval)->m_id), (*yylval)->m_text.c_str());

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
        //        }

        m_macro_analisys_buff.erase(m_macro_analisys_buff.begin());
        return result;
    }

    *yylval = nullptr;

    return parser::token_type::END;
}
