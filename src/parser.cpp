#include "pch.h"

#include "parser.h"
#include "lexer.h"

#include <term.h>

using namespace newlang;

const std::string Parser::MACROS_START = "\\\\";
const std::string Parser::MACROS_END = "\\\\\\";

Parser::Parser(TermPtr &ast) : m_ast(ast) {
    m_ast = Term::Create(TermID::END, "");
}

bool Parser::parse_stream(std::istream& in, const std::string sname) {
    streamname = sname;

    Scanner scanner(&in);
    this->lexer = &scanner;

    parser parser(*this);
    parser.set_debug_level(trace_parsing);
    return (parser.parse() == 0);
}

bool Parser::parse_file(const std::string filename) {
    std::ifstream in(filename);
    if(!in.good()) return false;
    return parse_stream(in, filename.c_str());
}

bool Parser::parse_string(const std::string input, const std::string sname) {
    std::istringstream iss(input.c_str());
    return parse_stream(iss, sname.c_str());
}

TermPtr Parser::Parse(const std::string input, MacrosStore *store) {

    std::string parse_string = ParseAllMacros(input, store);

    m_ast = Term::Create(TermID::END, "");
    m_ast->SetSource(std::make_shared<std::string>(input));
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

TermPtr Parser::ParseString(const std::string str, MacrosStore *store) {
    TermPtr ast = Term::Create(TermID::END, "");
    Parser p(ast);
    return p.Parse(str, store);
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
