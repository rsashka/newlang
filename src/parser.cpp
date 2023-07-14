#include "pch.h"

#include "parser.h"
#include "lexer.h"
#include "builtin.h"

#include <term.h>

using namespace newlang;

Parser::Parser(MacroBuffer *macro, PostLexerType *postlex, DiagPtr diag, bool pragma_enable) {
    char *source_date_epoch = std::getenv("SOURCE_DATE_EPOCH");
    if(source_date_epoch) {
        std::istringstream iss(source_date_epoch);
        iss >> m_timestamp;
        if(iss.fail() || !iss.eof()) {
            LOG_RUNTIME("Error: Cannot parse SOURCE_DATE_EPOCH (%s) as integer", source_date_epoch);
        }
    } else {
        m_timestamp = std::time(NULL);
    }

    m_is_runing = false;
    m_is_lexer_complete = false;

    m_file_name = "";
    m_file_time = "??? ??? ?? ??:??:?? ????";
    m_file_md5 = "??????????????????????????????";
    m_macro = macro;
    m_postlex = postlex;
    m_diag = diag ? diag : Diag::Init();
    m_annotation = Term::Create(parser::token_type::ARGS, TermID::ARGS, "");
    m_no_macro = false;
    m_enable_pragma = pragma_enable;

    m_ast = nullptr;
}

TermPtr Parser::ParseFile(const std::string filename) {

    llvm::SmallVector<char> path;
    if(!llvm::sys::fs::real_path(filename, path)) {
        m_file_name = llvm::StringRef(path.data(), path.size());
    } else {
        m_file_name = filename;
    }

    llvm::sys::fs::file_status fs;
    std::error_code ec = llvm::sys::fs::status(filename, fs);
    if(!ec) {
        time_t temp = llvm::sys::toTimeT(fs.getLastModificationTime());
        struct tm * timeinfo;
        timeinfo = localtime(&temp);
        m_file_time = asctime(timeinfo);
        m_file_time = m_file_time.substr(0, 24); // Remove \n on the end line
    }

    auto md5 = llvm::sys::fs::md5_contents(filename);
    if(md5) {
        llvm::SmallString<32> hash;
        llvm::MD5::stringifyResult(*md5, hash);
        m_file_md5 = hash.c_str();
    }

    int fd = open(filename.c_str(), O_RDONLY);
    if(fd < 0) {
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
    Scanner scanner(&m_stream, &std::cout, std::make_shared<std::string>(input));
    scanner.ApplyDiags(m_diag);

    lexer = &scanner;

    parser parser(*this);
    if(parser.parse() != 0) {
        return m_ast;
    }

    // Исходники требуются для вывода информации ошибках во время анализа типов
    m_ast->SetSource(m_ast->m_source);
    lexer = nullptr;
    return m_ast;
}

TermPtr Parser::ParseString(const std::string str, MacroBuffer *macro, PostLexerType *postlex, DiagPtr diag) {
    Parser p(macro, postlex, diag);
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
    if(m_ast->m_id == TermID::END) {
        m_ast = term;
        m_ast->ConvertSequenceToBlock(TermID::BLOCK, false);
    } else if(!m_ast->IsBlock()) {
        m_ast->ConvertSequenceToBlock(TermID::BLOCK, true);
    } else {
        ASSERT(m_ast->IsBlock());
        m_ast->m_block.push_back(term);
    }
}

TermPtr Parser::MacroEval(const TermPtr &term) {

    if(!m_macro) {
        m_diag->Emit(Diag::DIAG_MACRO_NOT_FOUND, term);
        return term;
    }

    TermPtr result = m_macro->EvalOpMacros(term);

    return result;
}

bool Parser::PragmaCheck(const TermPtr& term) {
    if(term && term->m_text.size() > 5
            && term->m_text.find("@__") == 0
            && term->m_text.rfind("__") == term->m_text.size() - 2) {
        return true;
    }
    return false;
}

bool Parser::PragmaEval(const TermPtr &term, BlockType &buffer) {
    /*
     * 
     * https://javarush.com/groups/posts/1896-java-annotacii-chto-ehto-i-kak-ehtim-poljhzovatjhsja
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
    static const char * __PRAGMA_INDENT_BLOCK__ = "@__PRAGMA_INDENT_BLOCK__";
    static const char * __PRAGMA_PROTOTYPE__ = "@__PRAGMA_PROTOTYPE__";

    static const char * __ANNOTATION_SET__ = "@__ANNOTATION_SET__";
    static const char * __ANNOTATION_IIF__ = "@__ANNOTATION_IIF__";

    ASSERT(term);
    if(term->m_text.compare(__PRAGMA_DIAG__) == 0) {
        if(term->size() == 0) {
            NL_PARSER(term, "Expected argument in pragma '%s'", term->toString().c_str());
        }

        Diag::State state;
        if(term->at(0).second->m_text.compare("push") == 0) {

            m_diag->Push(term);
            return true;

        } else if(term->at(0).second->m_text.compare("pop") == 0) {

            m_diag->Pop(term);
            return true;

        } else if(term->at(0).second->m_text.compare(Diag::toString(Diag::State::ignored)) == 0) {
            state = Diag::State::ignored;
        } else if(term->at(0).second->m_text.compare(Diag::toString(Diag::State::warning)) == 0) {
            state = Diag::State::warning;
        } else if(term->at(0).second->m_text.compare(Diag::toString(Diag::State::error)) == 0) {
            state = Diag::State::error;
        } else {
            NL_PARSER(term, "Pragma '%s' not recognized!", term->toString().c_str());
        }

        for (int i = 1; i < term->size(); i++) {
            m_diag->Apply(term->at(i).second->m_text.c_str(), state, term);
        }


    } else if(term->m_text.compare(__PRAGMA_MESSAGE__) == 0
            || term->m_text.compare(__PRAGMA_WARNING__) == 0
            || term->m_text.compare(__PRAGMA_ERROR__) == 0) {

        std::string message;
        for (int i = 0; i < term->size(); i++) {
            message += term->at(i).second->m_text;
        }

        utils::Logger::LogLevelType save = utils::Logger::Instance()->GetLogLevel();
        utils::Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
        if(term->m_text.compare(__PRAGMA_MESSAGE__) == 0) {
            LOG_INFO("note: %s", message.c_str());
        } else if(term->m_text.compare(__PRAGMA_WARNING__) == 0) {
            LOG_WARNING("warn: %s", message.c_str());
        } else {
            ASSERT(term->m_text.compare(__PRAGMA_ERROR__) == 0);
            LOG_RUNTIME("error: %s", message.c_str());
        }
        utils::Logger::Instance()->SetLogLevel(save);

    } else if(term->m_text.compare(__PRAGMA_IGNORE__) == 0) {

        LOG_RUNTIME("Pragma @__PRAGMA_IGNORE__ not implemented!");

        static const char * ignore_space = "space";
        static const char * ignore_indent = "indent";
        static const char * ignore_comment = "comment";
        static const char * ignore_crlf = "crlf";

    } else if(term->m_text.compare(__PRAGMA_MACRO__) == 0) {

        if(term->size() == 0) {
            NL_PARSER(term, "Expected argument in pragma macro '%s'", term->toString().c_str());
        }

        Diag::State state;
        if(term->at(0).second->m_text.compare("push") == 0) {

            m_macro->Push(term);
            return true;

        } else if(term->at(0).second->m_text.compare("pop") == 0) {

            m_macro->Pop(term);
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
    } else if(term->m_text.compare(__PRAGMA_MACRO_COND__) == 0) {

        if(term->size() == 0) {
            NL_PARSER(term, "Expected argument in pragma macro '%s'", term->toString().c_str());
        }

        LOG_RUNTIME("Pragma @__PRAGMA_MACRO_COND__ not implemented!");


    } else if(term->m_text.compare(__PRAGMA_INDENT_BLOCK__) == 0) {

        LOG_RUNTIME("Pragma @__PRAGMA_INDENT_BLOCK__ not implemented!");

    } else if(term->m_text.compare(__PRAGMA_EXPECTED__) == 0) {

        m_expected = term;

    } else if(term->m_text.compare(__PRAGMA_UNEXPECTED__) == 0) {

        m_unexpected = term;

    } else if(term->m_text.compare(__PRAGMA_FINALIZE__) == 0) {

        if(m_finalize || m_finalize_counter) {
            LOG_RUNTIME("Nested definitions of pragma @__PRAGMA_FINALIZE__ not implemented!");
        }

        m_finalize = term;
        m_finalize_counter = 0;

    } else if(term->m_text.compare(__PRAGMA_NO_MACRO__) == 0) {

        ASSERT(term->size() == 0);

        m_no_macro = true;

    } else if(term->m_text.compare(__ANNOTATION_SET__) == 0) {

        if(term->size() == 1) {
            // Set `name` = 1;
            std::string name = term->at(0).second->m_text;
            LOG_DEBUG("NAME: %s", name.c_str());

            auto iter = m_annotation->find(term->at(0).second->m_text);
            if(iter == m_annotation->end()) {
                m_annotation->push_back(Term::Create(parser::token_type::INTEGER, TermID::INTEGER, "1", 1, &term->m_lexer_loc, term->m_source), name);
            } else {
                //                iter->second =
                m_annotation->push_back(Term::Create(parser::token_type::INTEGER, TermID::INTEGER, "1", 1, &term->m_lexer_loc, term->m_source), name);
            }


        } else if(term->size() == 2) {
            // Set `name` = value;
            m_annotation->push_back(term->at(1).second, term->at(0).second->m_text);
        } else {
            NL_PARSER(term, "Annotation args in '%s' not recognized!", term->toString().c_str());
        }

//        LOG_DEBUG("ANNOT: %s", m_annotation->toString().c_str());

    } else if(term->m_text.compare(__ANNOTATION_IIF__) == 0) {

        ASSERT(m_annotation);

        if(term->size() != 3) {
            NL_PARSER(term, "Annotation IIF must have three arguments!");
        }

//        LOG_DEBUG("Annot %s %d", m_annotation->toString().c_str(), (int) m_annotation->size());

        auto iter = m_annotation->find(term->at(0).second->m_text);
        if(iter == m_annotation->end() || iter->second->m_text.empty() || iter->second->m_text.compare("0") == 0) {
            buffer.insert(buffer.begin(), term->at(2).second);
        } else {
            buffer.insert(buffer.begin(), term->at(1).second);
        }

    } else {
        NL_PARSER(term, "Uknown pragma '%s'", term->toString().c_str());
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

    if(row) { // Если переданы координаты ошибки
        message += " at line ";
        message += std::to_string(row);
        message += " col ";
        message += std::to_string(col);
        message += "\n";
    }

    // Ищем нужную строку
    size_t pos = 0;
    if(buffer.find("\n") != std::string::npos) {
        int count = 1;
        while(count < row) {
            pos = buffer.find("\n", pos + 1);
            count++;
        }
    }
    // Убираем лишние переводы строк
    std::string tmp = buffer.substr((pos ? pos + 1 : pos), buffer.find("\n", pos + 1));
    tmp = tmp.substr(0, tmp.find("\n", col));

    if(row) { // Если переданы координаты ошибки, показываем место

        // Лексер обрабатывает строки в байтах, а вывод в UTF8
        // поэтому позиция ошибки лексера може не совпадать для многобайтных символов
        std::wstring_convert < std::codecvt_utf8<wchar_t>, wchar_t> converter;
        std::wstring wstr = converter.from_bytes(tmp.substr(0, col));

        message += tmp + "\n";
        std::string placeholder(col - 1 - (tmp.substr(0, col).size() - wstr.size()), ' ');
        placeholder += "^\n";
        message += placeholder;
    } else {

        message += tmp;
    }

    return message;
}

void newlang::ParserException(const char *msg, std::string &buffer, int row, int col) {
    throw Return(ParserMessage(buffer, row, col, "%s", msg), Return::Parser);
}
