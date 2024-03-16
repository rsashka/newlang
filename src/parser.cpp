#include "pch.h"

#include "parser.h"
#include "lexer.h"
#include "builtin.h"
#include "system.h"

#include <term.h>

using namespace newlang;

int Parser::m_counter = 0;

Parser::Parser(MacroPtr macro, PostLexerType *postlex, DiagPtr diag, bool pragma_enable, RuntimePtr rt) {
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
    m_diag = diag ? diag : Diag::Init();
    m_annotation = Term::Create(parser::token_type::ARGS, TermID::ARGS, "");
    m_no_macro = false;
    m_enable_pragma = pragma_enable;
    m_name_module = "\\\\__main__";

    m_ast = nullptr;
}

TermPtr Parser::ParseFile(const std::string filename) {

    llvm::SmallVector<char> path;
    if (!llvm::sys::fs::real_path(filename, path)) {
        m_filename = llvm::StringRef(path.data(), path.size());
    } else {
        m_filename = filename;
    }

    llvm::sys::fs::file_status fs;
    std::error_code ec = llvm::sys::fs::status(filename, fs);
    if (!ec) {
        time_t temp = llvm::sys::toTimeT(fs.getLastModificationTime());
        struct tm * timeinfo;
        timeinfo = localtime(&temp);
        m_file_time = asctime(timeinfo);
        m_file_time = m_file_time.substr(0, 24); // Remove \n on the end line
    }

    auto md5 = llvm::sys::fs::md5_contents(filename);
    if (md5) {
        llvm::SmallString<32> hash;
        llvm::MD5::stringifyResult(*md5, hash);
        m_file_md5 = hash.c_str();
    }

    int fd = open(filename.c_str(), O_RDONLY);
    if (fd < 0) {
        LOG_RUNTIME("Error open file '%s'", filename.c_str());
    }

    struct stat sb;
    fstat(fd, &sb);

    std::string data;
    data.resize(sb.st_size);

    read(fd, const_cast<char*> (data.data()), sb.st_size);
    close(fd);

    return Parse(data);
}

TermPtr Parser::Parse(const std::string input) {

    m_ast = Term::Create(parser::token_type::END, TermID::END, "");
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
    Parser p(macro, postlex, diag, true, rt);
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
        m_ast->ConvertSequenceToBlock(TermID::BLOCK, false);
    } else if (!m_ast->isBlock()) {
        m_ast->ConvertSequenceToBlock(TermID::BLOCK, true);
        m_ast->m_namespace = nullptr;
    } else {
        ASSERT(m_ast->isBlock());
        m_ast->m_block.push_back(term);
    }
}

TermPtr Parser::MacroEval(const TermPtr &term) {

    if (!m_macro) {
        m_diag->Emit(Diag::DIAG_MACRO_NOT_FOUND, term);
        return term;
    }

    if (term->m_bracket_depth) {
        NL_PARSER(term, "Macro definitions allowed at the top level only, not inside conditions, namespace or any brackets!");
    }

    TermPtr result = m_macro->EvalOpMacros(term);

    return result;
}

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

        utils::Logger::LogLevelType save = utils::Logger::Instance()->GetLogLevel();
        utils::Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
        if (term->m_text.compare(__PRAGMA_MESSAGE__) == 0) {
            LOG_INFO("note: %s", message.c_str());
        } else if (term->m_text.compare(__PRAGMA_WARNING__) == 0) {
            LOG_WARNING("warn: %s", message.c_str());
        } else {
            ASSERT(term->m_text.compare(__PRAGMA_ERROR__) == 0);
            LOG_RUNTIME("error: %s", message.c_str());
        }
        utils::Logger::Instance()->SetLogLevel(save);

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
                m_annotation->push_back(Term::Create(parser::token_type::INTEGER, TermID::INTEGER, "1", 1, &term->m_lexer_loc, term->m_source), name);
            } else {
                //                iter->second =
                m_annotation->push_back(Term::Create(parser::token_type::INTEGER, TermID::INTEGER, "1", 1, &term->m_lexer_loc, term->m_source), name);
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

    if (!m_rt) {
        NL_PARSER(term, "Runtime environment for eval static assert is not available!");
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
        message += std::to_string(col);
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
    throw Error(ParserMessage(buffer, row, col, "%s", msg), ":ErrorParser");
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
        VERIFY(RegisterPredefMacro("@__FUNC_BLOCK__", "Full namespace function name"));



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

        char buf[sizeof "Jul 27 2012"];
        strftime(buf, sizeof buf, "%b %e %Y", localtime(&m_timestamp));

        term->m_text = buf;
        term->m_id = str_type;
        term->m_lexer_type = str_token;
        return term->m_lexer_type;

    } else if (text.compare("@__TIME__") == 0) {

        char buf[sizeof "07:07:09"];
        strftime(buf, sizeof buf, "%T", localtime(&m_timestamp));

        term->m_text = buf;
        term->m_id = str_type;
        term->m_lexer_type = str_token;
        return term->m_lexer_type;

    } else if (text.compare("@__TIMESTAMP__") == 0) {

        term->m_text = asctime(localtime(&m_timestamp));
        term->m_text = term->m_text.substr(0, 24); // Remove \n on the end line
        term->m_id = str_type;
        term->m_lexer_type = str_token;
        return term->m_lexer_type;

    } else if (text.compare("@__TIMESTAMP_ISO__") == 0) {

        char buf[sizeof "2011-10-08T07:07:09Z"];
        strftime(buf, sizeof buf, "%FT%TZ", localtime(&m_timestamp));

        term->m_text = buf;
        term->m_id = str_type;
        term->m_lexer_type = str_token;
        return term->m_lexer_type;

    } else if (text.compare("@__CLASS__") == 0 || text.compare("@__NAMESPACE__") == 0 || text.compare("@__FUNC_BLOCK__") == 0 ||
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

bool Parser::CheckLoadModule(TermPtr & term) {
    if (!CheckCharModuleName(term->m_text.c_str())) {
        NL_PARSER(term, "Module name - backslash, underscore, lowercase English letters or number!");
    }
    if (m_rt && !m_rt->CheckLoadModule(term)) {
        NL_PARSER(term, "Fail load module '%s'!", term->toString().c_str());
    }
    return true;
}
