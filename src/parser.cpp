#include "pch.h"

#include "parser.h"
#include "lexer.h"

#include <term.h>

using namespace newlang;

const std::string Parser::MACROS_START = "\\\\";
const std::string Parser::MACROS_END = "\\\\\\";

Parser::Parser(TermPtr &ast) : m_ast(ast) {
    m_ast = Term::Create(TermID::END, "");
    m_ast->m_parser = this;
    m_macro = nullptr;
}

bool Parser::parse_stream(std::istream& in, const std::string sname) {
    streamname = sname;

    Init(m_macro);

    Scanner scanner(&in);
    this->lexer = &scanner;

    parser parser(*this);
    parser.set_debug_level(trace_parsing);
    return (parser.parse() == 0);
}

template <typename TP>
std::time_t to_time_t(TP tp) {
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
            + system_clock::now());
    return system_clock::to_time_t(sctp);
}

bool Parser::parse_file(const std::string filename) {
    std::ifstream in(filename);

    m_file_name = filename;

    llvm::sys::fs::file_status fs;
    std::error_code ec = llvm::sys::fs::status(filename, fs);
    if(!ec) {
        time_t temp = llvm::sys::toTimeT(fs.getLastModificationTime());
        struct tm * timeinfo;
        timeinfo = localtime(&temp);
        m_file_time = asctime(timeinfo);
    }

    auto md5 = llvm::sys::fs::md5_contents(filename);
    if(md5) {
        llvm::SmallString<32> hash;
        llvm::MD5::stringifyResult(*md5, hash);
        m_md5 = hash.c_str();
    }

    if(!in.good()) return false;
    return parse_stream(in, filename.c_str());
}

bool Parser::parse_string(const std::string input, const std::string sname) {
    std::istringstream iss(input.c_str());
    return parse_stream(iss, sname.c_str());
}

TermPtr Parser::Parse(const std::string input, MacrosStore *store, MacroBuffer *macro) {

    Init(macro);

    std::string parse_string = ParseAllMacros(input, store);

    m_ast = Term::Create(TermID::END, "");
    m_ast->SetSource(std::make_shared<std::string>(input));
    m_ast->m_parser = this;
    m_stream.str(parse_string);
    Scanner scanner(&m_stream, &std::cout, m_ast->m_source);
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

TermPtr Parser::ParseString(const std::string str, MacrosStore *store, MacroBuffer *macro) {
    TermPtr ast = Term::Create(TermID::END, "");
    Parser p(ast);
    return p.Parse(str, store, macro);
}

void Parser::error(const class location& l, const std::string& m) {
    std::cerr << l << ": " << m << std::endl;
}

void Parser::error(const std::string &m) {
    std::cerr << m << std::endl;
}

void newlang::parser::error(const parser::location_type& l, const std::string& msg) {
    std::string buffer(driver.m_stream.str());
    ParserException(msg.c_str(), buffer, l.begin.line, l.begin.column);
}

void Parser::AstAddTerm(TermPtr &term) {
    ASSERT(m_ast);
    ASSERT(m_ast->m_source);
    term->m_parser = this;
    term->m_source = m_ast->m_source;
    if(m_ast->m_id == TermID::END) {
        m_ast = term;
        m_ast->ConvertSequenceToBlock(TermID::BLOCK, false);
    } else if(!m_ast->IsBlock()) {
        m_ast->ConvertSequenceToBlock(TermID::BLOCK, true);
    } else {
        m_ast->m_block.push_back(term);
    }
}

void Parser::MacroLevelBegin(TermPtr &term) {
    m_macro_level++;
}

void Parser::MacroLevelEnd(TermPtr &term) {
    ASSERT(m_macro_level);
    m_macro_level--;
    if(m_macro) {
        m_macro->Clear(m_macro_level);
    }
}

void Parser::MacroTerm(TermPtr &term) {
    if(!m_macro) {
        Warning(term, "MacroBufferEmpty", "Buffer for macros not initialized!");
        return;
    }

    if(term->GetTokenID() == TermID::MACRO_DEL) {
        if(!m_macro->Remove(term)) {
            Warning(term, "MacroNotFound", "Macros not found!");
        }
        return;
    } else if(m_macro->isExist(term)) {
        Warning(term, "MacroDuplicate", "Macros duplicate!");
        return;
    }

    m_macro->Append(term, m_macro_level);
    lexer->m_is_macro = false;
}

void Parser::Warning(TermPtr &term, const char *id, const char *message) {
    std::string empty;
    std::string str = newlang::ParserMessage(term->m_source ? *term->m_source : empty, term->m_line, term->m_col, "%s", message);

    utils::Logger::LogLevelType save = utils::Logger::Instance()->GetLogLevel();
    utils::Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
    LOG_INFO("%s", str.c_str());
    utils::Logger::Instance()->SetLogLevel(save);
}

std::string MacroBuffer::toHash(const TermPtr & term) {
    std::string result;
    for (auto &elem : term->m_follow) {
        if(!result.empty()) {
            result += "+";
        }
        if(elem->isMacro()) {
            result += elem->m_text;
        } else {
            result += elem->toString();
        }
    }
    return result;
}

void MacroBuffer::Append(const TermPtr &term, size_t level) {
    MacroToken item;
    item.level = level;
    item.term = term;

    if(term->m_text.compare("alias_name") == 0) {
        std::cout << toHash(term) << term->toString() << "\n";
    }
    insert(std::pair<std::string, MacroToken>(toHash(term), item));
}

bool MacroBuffer::Remove(const TermPtr &term) {
    auto iter = find(toHash(term));
    if(iter != end()) {
        erase(iter);
        return true;
    }
    return false;
}

std::string MacroBuffer::Dump() {
    std::string result;
    auto iter = begin();
    while(iter != end()) {
        if(!result.empty()) {
            result += ", ";
        }
        result += iter->first + "->'" + iter->second.term->toString() + "'";
        iter++;
    }
    return result;
}

std::string MacroBuffer::Convert(LexerTokenType &buffer) {

    if(!buffer.empty()) {

        // Рекурсивное раскрытие алиасов только для последнего термина в буфере
        auto iter = find(buffer[buffer.size() - 1].term->m_text);
        while(iter != end() && iter->second.term->GetTokenID() == TermID::ALIAS) {
            std::ptrdiff_t index = std::distance(begin(), iter);

            ASSERT(iter->second.term->Right());

            buffer[buffer.size() - 1].term = iter->second.term->Right();
            iter = find(buffer[buffer.size() - 1].term->m_text);
            // Для исключения зацикливания раскрытия макросов (когда один раскрывается в другой или в сам себя)
            if(iter != end() && index <= std::distance(begin(), iter)) {
                break;
            }
        }

        // Раскрытие макроса с изменением буфера у лексера
        for (auto &elem : * this) {
            if(elem.second.term->GetTokenID() == TermID::MACRO) {
                for (int i = 0; i < elem.second.term->m_follow.size() && i < buffer.size(); i++) {

                    if(elem.second.term->m_follow[i]->m_text.find("$") == 0) {
                        ASSERT(!"Сделать замену аргументам в макросе!!!");
                        //
                    } else if(buffer[buffer.size() - 1 - i].term->m_text.compare(elem.second.term->m_follow[i]->m_text) != 0) {
                        break;
                    }

                    if(i + 1 == elem.second.term->m_follow.size()) {
                        ASSERT(buffer.size() >= elem.second.term->m_follow.size());
                        buffer.resize(buffer.size() - elem.second.term->m_follow.size());

                        ASSERT(elem.second.term->Right());
                        //                        std::cout << "->" << elem.second.term->Right()->m_text << "!!!\n";
                        return elem.second.term->Right()->m_text;
                    }
                }
            } else {
                ASSERT(elem.second.term->GetTokenID() == TermID::ALIAS);
            }
        }
    }
    return std::string();
}

parser::token_type Parser::NewToken(TermPtr * yylval, parser::location_type * yylloc) {

    parser::token_type result;

    ASSERT(yylval);
    ASSERT(yylloc);

    size_t max_count = 20;

    if(!m_is_done) {

        m_is_runing = true;

        LexerToken tok;
        while(1) {

            tok.term = Term::Create(TermID::END, "", 0);
            tok.type = lexer->lex(&tok.term, &tok.loc);


            if(tok.type == parser::token_type::END) {
                if(lexer->m_data.empty()) {
                    m_is_done = true;
                } else {
                    lexer->yypop_buffer_state();
                    delete lexer->m_data[lexer->m_data.size() - 1].iss;
                    lexer->m_data.pop_back();
                    continue;
                }
            }

            m_prep_buff.push_back(tok);

            if(m_macro && !lexer->m_is_macro) { // Нельзя раскрывать макросы в определениях других макросов

                std::string data = m_macro->Convert(m_prep_buff);

                // Если нужно раскрыть и распарсить макрос
                if(!data.empty()) {


                    if(lexer->m_data.size() > 100) {
                        LOG_RUNTIME("Macro expansion '%s' stack overflow?", tok.term->toString().c_str());
                    }

                    lexer->m_data.push_back({data, new std::istringstream(data)});
                    lexer->yypush_buffer_state(lexer->yy_create_buffer(lexer->m_data[lexer->m_data.size() - 1].iss, data.size()));
                    continue;
                }
            }

            if(m_is_done || m_prep_buff.size() >= max_count) {
                break;
            }
        }

        //LOG_DEBUG("LexerToken count %d", (int) m_prep_buff.size());

    }


    if(!m_prep_buff.empty()) {
        *yylval = m_prep_buff.at(0).term;
        *yylloc = m_prep_buff.at(0).loc;
        result = m_prep_buff.at(0).type;
        m_prep_buff.pop_front();
        return result;
    }

    *yylval = nullptr;

    return parser::token_type::END;
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
