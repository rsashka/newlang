#ifndef NEWLANG_PARSER_H_
#define NEWLANG_PARSER_H_

#include "pch.h"

#include <core/variable.h>

#include <core/warning_push.h>
#include "parser.yy.h"
#include <core/warning_pop.h>

namespace newlang {

/** The Driver class brings together all components. It creates an instance of
 * the Parser and Scanner classes and connects them. Then the input stream is
 * fed into the scanner object and the parser gets it's token
 * sequence. Furthermore the driver object is available in the grammar rules as
 * a parameter. Therefore the driver class contains a reference to the
 * structure into which the parsed data is saved. */
class Parser {
public:

    /// construct a new parser driver context
    //    Parser(class CalcContext& calc);

    Parser(TermPtr &ast);

    /// enable debug output in the flex scanner
    bool trace_scanning;

    /// enable debug output in the bison parser
    bool trace_parsing;

    /// stream name (file or input stream) used for error messages.
    std::string streamname;

    std::istringstream m_stream;


    /** Invoke the scanner and parser for a stream.
     * @param in	input stream
     * @param sname	stream name for error messages
     * @return		true if successfully parsed
     */
    bool parse_stream(std::istream& in, const std::string_view sname = "stream input");

    /** Invoke the scanner and parser on an input string.
     * @param input	input string
     * @param sname	stream name for error messages
     * @return		true if successfully parsed
     */
    bool parse_string(const std::string_view input, const std::string_view sname = "string stream");

    /** Invoke the scanner and parser on a file. Use parse_stream with a
     * std::ifstream if detection of file reading errors is required.
     * @param filename	input file name
     * @return		true if successfully parsed
     */
    bool parse_file(const std::string_view filename);

    // To demonstrate pure handling of parse errors, instead of
    // simply dumping them on the standard error output, we will pass
    // them to the driver using the following two member functions.

    /** Error handling with associated line number. This can be modified to
     * output the error e.g. to a dialog box. */
    void error(const class location& l, const std::string& m);

    /** General error handling. This can be modified to output the error
     * e.g. to a dialog box. */
    void error(const std::string_view m);

    /** Pointer to the current lexer instance, this is used to connect the
     * parser to the scanner. It is used in the yylex macro. */
    class Scanner* lexer;

    /** Reference to the calculator context filled during parsing of the
     * expressions. */
    //    class CalcContext& calc;

    TermPtr Parse(const std::string_view str);
    static TermPtr ParseString(const std::string_view str);

    void AstAddTerm(TermPtr &term);

    typedef std::map<std::string, std::string> MacrosStore;

    inline static const std::string MACROS_START = "\\\\";
    inline static const std::string MACROS_END = "\\\\\\";

    static inline bool ParseNameMacro(const std::string &body, std::string &name, std::string &args) {
        size_t len = 0;
        // имя макроса должно быть в самом начале строки без пробелов
        if (body.empty() || body[0] != '\\') {
            return false; // Нет имени макроса
        }
        size_t pos = 1;
        while (pos < body.size()) {
            if (std::isspace(static_cast<unsigned char> (body[pos])) || body[pos] == '(') {
                name = body.substr(0, pos);
                break;
            }
            pos++;
        }

        if (pos == 0) {
            LOG_DEBUG("Macro name not found!");
            return false; // Нет имени макроса
        }

        // после имени без пробела жет быть открывающая скобка
        if (body[pos] != '(') {
            args.clear();
            return true; // Открывающей скобки нет, значит нет аргументов
        }
        size_t start = pos;
        while (pos < body.size()) {
            if (body[pos] == ')') {
                args = body.substr(start, pos);
                return true;
            }
            pos++;
        }
        LOG_DEBUG("Fail parse macro args!");
        return false; // Нет закрывающей скобки, косяк в аргументах
    }

    static size_t ExtractMacros(std::string &text, MacrosStore &store) {
        /*
         * Сперва искать начало определения макроса \\, потом завершение определения макроса \\\
         * Вырезать тело определения макроса из строки парсинга и добавить макрос в хранилище.
         */

        size_t start = text.find(MACROS_START); // Начало макроса
        size_t stop = std::string::npos;
        if (start != std::string::npos) {
            stop = text.find(MACROS_END, start + MACROS_START.size() + 1);

            if (stop == std::string::npos) {
                LOG_RUNTIME("Macro termination symbol not found! Start at %d '%s'", (int) start, text.c_str());
                // throw Interrupt(ParserMessage(buffer, row, col, "%s", msg), Interrupt::Parser);
            }

            std::string body = text.substr(start + MACROS_START.size(), stop - MACROS_START.size());
            std::string name;
            std::string args;
            if (!ParseNameMacro(body, name, args)) {
                LOG_RUNTIME("Fail parse name macro! '%s'", body.c_str());
                // throw Interrupt(ParserMessage(buffer, row, col, "%s", msg), Interrupt::Parser);
            }

            auto found = store.find(name);
            if (found != store.end()) {
                std::string f_name;
                std::string f_args;
                VERIFY(ParseNameMacro(found->second, f_name, f_args));
                if (f_args.empty() == args.empty()) {
                    LOG_RUNTIME("Macro %s arguments are duplicated! %s %s", name.c_str(), args.c_str(), f_args.c_str());
                    // throw Interrupt(ParserMessage(buffer, row, col, "%s", msg), Interrupt::Parser);
                }
            }
            store[name] = body;

            // Заменить определение макроса пробелами, кроме переводов строк, чтобы не съезжала позиция парсинга
            std::string fill(stop + MACROS_END.size() - start, ' ');

            ASSERT(fill.size() > body.size());
            for (size_t i = 0; i < body.size(); i++) {
                if (body[i] == '\n') {
                    fill[i] = '\n';
                }
            }
            text.replace(start, stop + MACROS_END.size(), fill);
        }
        return store.size();
    }

    static bool ExpandMacros(std::string &text, MacrosStore &store) {
        /*
         * Искать макросы и заменять их в строке на развернутые определения из хранилища.
         */
        return false;
    }


private:
    TermPtr &m_ast;

};

} // namespace example

#endif // NEWLANG_PARSER_H_
