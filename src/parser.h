#ifndef NEWLANG_PARSER_H_
#define NEWLANG_PARSER_H_

#include "pch.h"

#include <variable.h>

#include <warning_push.h>
#include "parser.yy.h"
#include <warning_pop.h>

namespace newlang {

    /** The Driver class brings together all components. It creates an instance of
     * the Parser and Scanner classes and connects them. Then the input stream is
     * fed into the scanner object and the parser gets it's token
     * sequence. Furthermore the driver object is available in the grammar rules as
     * a parameter. Therefore the driver class contains a reference to the
     * structure into which the parsed data is saved. */
    class Parser {
    public:

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
        bool parse_stream(std::istream& in, const std::string sname = "stream input");

        /** Invoke the scanner and parser on an input string.
         * @param input	input string
         * @param sname	stream name for error messages
         * @return		true if successfully parsed
         */
        bool parse_string(const std::string input, const std::string sname = "string stream");

        /** Invoke the scanner and parser on a file. Use parse_stream with a
         * std::ifstream if detection of file reading errors is required.
         * @param filename	input file name
         * @return		true if successfully parsed
         */
        bool parse_file(const std::string filename);

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

        void AstAddTerm(TermPtr &term);


        typedef std::vector<std::string> MacrosArgs;
        typedef std::map<std::string, std::string, std::greater<std::string>> MacrosStore;

        inline static const std::string MACROS_START = "\\\\";
        inline static const std::string MACROS_END = "\\\\\\";


        TermPtr Parse(const std::string_view str, MacrosStore *store = nullptr);
        static TermPtr ParseString(const std::string_view str, MacrosStore *store = nullptr);

        static inline std::string ParseMacroName(const std::string &body) {
            // имя макроса должно быть в самом начале строки без пробелов и начинаться на один слешь
            if (body.size() < 3 || body[0] != '\\' || body[1] == '\\') {
                return std::string(); // Нет имени макроса
            }
            for (size_t i = 0; i < body.size(); i++) {
                if (std::isspace(static_cast<unsigned char> (body[i]))) {
                    return body.substr(0, i);
                } else if (body[i] == '(') {
                    return body.substr(0, i + 1); // Для макросов с аргументами имя включает открывающую скобку
                }
            }
            return body; // Макрос без тела
        }

        static inline MacrosArgs ParseMacroArgs(const std::string &body) {
            std::string name = ParseMacroName(body);
            MacrosArgs result;

            if (name.size() < 2 || name[name.size() - 1] != '(') {
                return result; // Нет имени макроса или аргументов
            }

            std::string arg;
            for (size_t i = name.size(); i < body.size(); i++) {
                if (body[i] == ',' || body[i] == ')') {
                    trim(arg);
                    if (!arg.empty()) {
                        if (arg.find(".") != std::string::npos && arg.compare("...") != 0) {
                            LOG_RUNTIME("Macro argument name failure '%s'!", arg.c_str());
                        }
                        result.push_back(arg); // новый аргумент макроса
                        arg.clear();
                    } else if (body[i] == ',') {
                        LOG_RUNTIME("Macro argument missing!");
                    }
                } else {
                    arg.append(1, body[i]); // имя аргумента
                }
                if (body[i] == ')') {
                    return result; // Аругменты закончились
                }
            }
            // Нет закрывающей скобки
            LOG_RUNTIME("Closing bracket not found!");
        }

        /**
         * Искать в строке text начало определения макроса \\, потом завершение определения макроса \\\
         * Вырезать тело определения макроса из строки и добавить макрос в хранилище store.
         * @param text Строка для парсинга
         * @param store Хранилище макросов
         * @param fill Удалять ли макрос из входной строки
         * @return Истина, если была произведена замена, иначе ложь
         */
        static bool ExtractMacros(std::string &text, MacrosStore &store, bool fill = true) {

            ASSERT(MACROS_START.size() == 2);
            ASSERT(MACROS_END.size() == 3);

            size_t start = text.find(MACROS_START); // Начало макроса
            size_t stop = std::string::npos;
            if (start != std::string::npos) {
                stop = text.find(MACROS_END, start + MACROS_START.size() + 1);

                if (stop == std::string::npos) {
                    LOG_RUNTIME("Macro termination symbol not found! Start at %d '%s'", (int) start, text.c_str());
                    // throw Interrupt(ParserMessage(buffer, row, col, "%s", msg), Interrupt::Parser);
                }

                std::string body = text.substr(start + MACROS_START.size() - 1, stop - start - MACROS_START.size() + 1);
                std::string name = ParseMacroName(body);
                if (name.empty()) {
                    LOG_RUNTIME("Fail parse name macro! '%s'", body.c_str());
                    // throw Interrupt(ParserMessage(buffer, row, col, "%s", msg), Interrupt::Parser);
                }
////\__LINE__
////\__FILE__
////\__FUNC__
////\__CLASS__
////\__MACRO__
//
//#\line 20
                auto found = store.find(name);
                if (found != store.end()) {
                    LOG_RUNTIME("Macro name %s are duplicated!", name.c_str());
                    // throw Interrupt(ParserMessage(buffer, row, col, "%s", msg), Interrupt::Parser);
                }
                store[name] = body;

                if (fill) {
                    // Заменить определение макроса пробелами, кроме переводов строк, чтобы не съезжала позиция парсинга
                    std::string fill(stop - start + MACROS_END.size(), ' ');

                    ASSERT(fill.size() > body.size());
                    for (size_t i = 0; i < body.size(); i++) {
                        if (body[i] == '\n') {
                            fill[i + 1] = '\n';
                        }
                    }
                    text.replace(start, fill.size(), fill);
                }
                return true;
            }
            return false;
        }

        static std::string ExpandMacro(const std::string &macro, const std::string &text) {

            std::string name = ParseMacroName(macro);
            MacrosArgs args = ParseMacroArgs(macro);

            std::string body_base;
            std::string result(text);

            if (name.empty() || result.empty()) {
                return result;
            }

            if (name[name.size() - 1] != '(') {
                body_base = macro.substr(name.size());
                if (!body_base.empty() && std::isspace(static_cast<unsigned char> (body_base[0]))) {
                    body_base = body_base.erase(0, 1);
                }
            } else {
                bool done = false;
                size_t pos = name.size();
                while (pos < macro.size()) {
                    if (macro[pos] == ')') {
                        // Аругменты закончились
                        body_base = macro.substr(pos + 1);
                        done = true;
                        break;
                    }
                    pos++;
                }
                if (!done) {
                    // Нет закрывающей скобки
                    LOG_RUNTIME("Closing bracket not found! '%s'", macro.c_str());
                }
            }

            size_t pos_start = 0;
            size_t pos_end = 0;
            while ((pos_start + name.size() - 1) < result.size()) {
                pos_start = result.find(name, pos_start);
                if (pos_start == std::string::npos) {
                    break;
                }

                pos_end = pos_start + name.size();

                if (name[name.size() - 1] != '(') {

                    result.replace(pos_start, pos_end - pos_start, body_base);

                } else {

                    size_t bracet_count = 0;
                    while (pos_end < result.size()) {
                        if (result[pos_end] == '(') {
                            bracet_count++;
                        } else if (result[pos_end] == ')') {
                            if (bracet_count == 0) {
                                pos_end++;
                                break;
                            } else {
                                bracet_count--;
                            }
                        }
                        pos_end++;
                    }
                    if (pos_end > result.size()) {
                        // Нет закрывающей скобки
                        LOG_RUNTIME("Closing bracket not found! '%s'", result.c_str());
                    }


                    MacrosArgs args_define = ParseMacroArgs(result.substr(pos_start, pos_end));
                    if (args_define.empty()) {

                        result.replace(pos_start, pos_end - pos_start, body_base);

                    } else {

                        std::string body(body_base);

                        for (size_t i = 0; i < args.size() && i < args_define.size(); i++) {
                            // Заменить аргумент по имени
                            if (args[i].compare("...") != 0) {
                                std::string arg_name = "\\\\\\$" + args[i];
                                body = std::regex_replace(body, std::regex(arg_name), args_define[i]);
                            }
                        }

                        std::string summary;
                        for (size_t i = 0; i < args_define.size(); i++) {
                            // Заменить аргумент по номеру
                            std::string arg_num = "\\\\\\$" + std::to_string(i + 1);
                            body = std::regex_replace(body, std::regex(arg_num), args_define[i]);

                            if (!summary.empty()) {
                                summary += ",";
                            }
                            summary += args_define[i];
                        }
                        body = std::regex_replace(body, std::regex("\\\\\\$\\*"), summary);

                        result.replace(pos_start, pos_end - pos_start, body);
                    }
                }
            }
            return result;
        }

        static std::string ParseAllMacros(const std::string_view input, MacrosStore *store) {
            std::string result(input);
            if (store) {
                bool done;
                do {
                    done = ExtractMacros(result, *store);
                } while (done);

                for (auto &elem : *store) {
                    result = ExpandMacro(elem.second, result);
                }
            }
            return result;
        }


    private:
        TermPtr &m_ast;

    };

} // namespace example

#endif // NEWLANG_PARSER_H_
