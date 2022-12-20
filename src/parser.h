#ifndef NEWLANG_PARSER_H_
#define NEWLANG_PARSER_H_

#include "pch.h"

#include <variable.h>

#include <warning_push.h>
#include "parser.yy.h"
#include <warning_pop.h>

namespace newlang {

    typedef std::vector<TermPtr> BlockType;
    typedef std::shared_ptr<std::string> SourceType;

    typedef std::deque<TermPtr> LexerTokenType;

    struct MacroToken {
        size_t level;
        TermPtr macro;
    };

    class MacroBuffer : SCOPE(protected) std::map<std::string, std::vector<MacroToken> > {
    public:

        /*
         * Макрос - в общем случае, это один или несколько терминов с префиксом макроса '\' или без префикса.
         * Определение макроса - синтаксическая конструкция, которая содержит идентификатор макроса и его тело.
         * Тело макроса - это последовательность произвольных лексических единиц или текстовая строка без разбивки на лексемы.
         * Идентификатору макроса - один или несколько терминов подряд (терминами могут быть имя, макрос или шаблон подстановки).
         * Макросы могут быть параметризированными за счет шаблонов подстановки и аргументов.
         * Щаблоны подстановки и аргументы в идентификаторе макроса должны записываться с префиксом $ как имена локальных переменных.
         * Щаблоны подстановки и аргументы в теле мароса должны записываться с префиксом \$.
         * В идентификаторе макроса ургументы в скобках могут быть только у одного термина или шаблона подстановки.
         * 
         * Сопоставление макросов просиходит по точному соотвествию идентификатора имени термина, а возможные аргументы скобках игнорируются.
         * Идентификаторы макросов, состоящие только из шаблонов подстановок не допускаются.
         * Идентификаторы макроса с аргументами и без аргументов считаются разными идентификаторами, 
         * поэтому для переименования всех возможных варинатов использования одного термина нужно определять
         * сразу два макроса со скобкамии и без скобок.
         * \\ old \\ new \\; \\ old(...) \\ new(\$*) \\;
         * \\replace($old, $new)\\\ \\ \$old \\ \$new \\; \\ \$old(...)\\ \$new(\$*)\\  \\\;
         * 
         * Параметры макроса могут быль любыми последовательности терминов между скобками, которые разделяются запятыми.
         * Вложенность скобок анализируется, и в качестве параметров макросов используется только верхний уровень вложенности.
         * 
         * Контроль типов аргументов у макросов пока не реализован, т.к. перегрузка функция по типам аргументов 
         * в языке отсуствует и контроль типов аргументов выполняется интерпретатором (компилятором).
         * 
         * 
         * 
         * 
         
         * 
         * Сложность парсинга макросов с такими начальными условиями заключается в том, что параметризированны макросы
         * могут быть произвольной длинны, и их завершение можно определить по двум критериям:
         * 1. Конец исходных данных (лексер вернул END)
         * 2. Очередной термин не соответствует термину в идентификаторе макроса
         * 
         * Из-за этого алгоритм анализа маросов выполняется следующим образом:
         * - термины считываются из лексера пока не появится имя или макрос.
         * - цикл по всем зарегистрированным макросам
         * - чтение новых терминов из лексера, пока сопсотавление с идентификатором макроса не вернет равно/ложь/END
         * - Если термин '(', то считываются термины до получения термина ')' с учетом их вложенности -> в начало.
         * 4. 
         * 
         * 3. Ошибка в аргументах шаблона термина.
         * 
         * Ошибка в аргументах шаблона термина могут быть следующего вида:
         * - У входного термина есть й термин соотвестетствует шаблону и оба являются вызовами функций, но аргументы , а у 
         * 
         * 
         * Макросы раскрываются после лексера до передачи термина в парсер.
         * 
         * 
         * Распознавание и раскрытие макросов происходит до
         * 
         * Нужно ли различать термин с вызвовм и без вызова???? Скорее всего различать не требуется (все равно, есть скобки или нет),
         * но если они нужны при раскрытии (например именованные аргументы, которые отсуствуют), то при раскрытии будет ошибка.
         * 
         * 
         * \term name\ term_name\ 
         * \term name()\ term_name\ # Ошибка, Два одинаковых имени макроса!
         * 
         * \term($arg1) name($arg2)\ term_name($arg1)\
         * term name("name") # Ошибка,  $arg1 не определен?
         * 
         * 
         * \term(...) name(...)\ term_name($term...)\
         * term name("name") # Ошибка,  $arg1 не определен?
         */




        /*
         * Для быстрого отбора макросов для сравнения нужно учитывать индексацию доступа к элементам,
         * но у multimap может быть несколько элементов, каждый из которых необходимо сравнивать индивидуально.
         * А так как длина имен у макрососв может состоять сразу из нескольких терминов, 
         * то есть следующие варинаты поиска:
         * 1. перебирать все существующие макросы и сравнивать последние термины в буфере 
         * (отступ от конца буфера будет зависеть от количества имен в идентификаторе макроса).
         * Кол-во шагов перебора для поиска будет равно кол-ву определенных макросов, 
         * а сам поиск должен выполняться при добавлении каждого нового термина в буфер!!!!
         * 2. Последовательность терминов в имени мароса сохнаять в обратном порядке (последний, предпоследний и т.д.),
         * а поиск выполнять в multimap определнных буферов для каждого нового термина (при добавлении он становится последним).
         * Поиск будет по индексу multimap, но с последнего термина имени (это нужно учитывать при возможных оптимизациях и вероятных ошибках)
         * 3. Второй вариант лучше черм первый, но все равно может оказаться не очень удачным, так как человеку более привычно 
         * использовать ключевое слове первым, а при подобной индексации оно будет анализироваться последним.
         * Возможно придется реализовывать какой нибудь вариант хеширования, чтобы индексировать поиск терминов. 
         * 
         * Какой бы алгоритм поиска не был выбран, пока это преждевременная оптимизация.
         * Поэтому сейчас делаю полный перебор, а оптимизировать нужно будет потом.
         *          
         */


        /*
         * Добавляет определение нового макроса
         */
        void Append(const TermPtr &term, size_t level);

        bool Remove(const TermPtr & term);

        size_t GetCount() {
            size_t count = 0;
            auto iter = begin();
            while (iter != end()) {
                count += iter->second.size();
                iter++;
            }
            return count;
        }

        void Clear(size_t level) {
            if (level == 0) {
                clear();
            } else {
                auto iter = begin();
                while (iter != end()) {

                    int pos = 0;
                    while (pos < iter->second.size()) {
                        if (iter->second[pos].level >= level) {
                            iter->second.erase(iter->second.begin() + pos);
                        } else {
                            pos++;
                        }
                    }

                    if (iter->second.size() == 0) {
                        iter = erase(iter);
                    } else {
                        iter++;
                    }
                }
            }
        }

        TermPtr find(std::vector<std::string> list);
        bool isExist(const TermPtr & term);

        /*
         * Ищет макрос для соответствия текущему буферу токенов лексера
         * 
         * Поиск с последнего токена
         */
        //        TermPtr CheckExpand(LexerTokenType &buffer);

        enum class CompareResult {
            NOT_EQ = 0,
            NEXT_NAME = 1,
            NEXT_BRAKET = 2,
            DONE = 3,
        };
        /**
         * Сравнивает входящий буфер с макросом
         * @param buffer Входной буфер лексем
         * @param term Макрос для сопоставления
         * @return Кол-во лексем, которые нужно дочитать для сопоставления с образцом, npos, если нет соответствия 
         * или 0 если соответствие полное и макрос нужно раскрывать.
         * 
         */
        static CompareResult CompareMacro(LexerTokenType &buffer, const TermPtr & term);
        static bool CompareTermName(const std::string & term_name, const std::string & macro_name);

        /**
         * Коллекция аргументов макроса
         */
        typedef std::map<std::string, BlockType> MacroArgsType;
        /**
         * Создает коллекцию аргументов и шаблонов из буфера терминов лексера, которые используются в теле макроса 
         * Аргументами макроса могут быть шаблоны сопсоталения (термины) и аргументы в скобках верхнего уровня
         * @param buffer Входной буфер лексем
         * @param term Макрос для сопоставления
         * @param args Коллекция аргументов макроса
         * @return Кол-во символов входного буфера, которые нужно будет заменить макросом
         */
        static size_t ExtractArgs(LexerTokenType &buffer, const TermPtr &term, MacroArgsType & args);
        static void InsertArg_(MacroArgsType & args, std::string name, LexerTokenType &buffer, size_t pos);
        static void InsertArg_(MacroArgsType & args, std::string name, BlockType &data);
        static BlockType SymbolSeparateArg_(LexerTokenType &buffer, size_t &pos, std::vector<std::string> name, std::string &error);
        /* 
         * Раскрывает макрос в последовательность терминов лексера с заменой аргументов
         */
        static BlockType ExpandMacros(const TermPtr &macro, MacroArgsType & args);
        /* 
         * Раскрывает макрос в текстовую строку с заменой аргументов
         */
        static std::string ExpandString(const TermPtr &macro, MacroArgsType & args);


        std::string Dump();
        static std::string Dump(MacroArgsType &var);

        //        SCOPE(private) :
        //        MacroTokenType m_store;
    };

    /** The Driver class brings together all components. It creates an instance of
     * the Parser and Scanner classes and connects them. Then the input stream is
     * fed into the scanner object and the parser gets it's token
     * sequence. Furthermore the driver object is available in the grammar rules as
     * a parameter. Therefore the driver class contains a reference to the
     * structure into which the parsed data is saved. */
    class Parser {
    public:

        Parser();

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
        void error(const std::string &m);

        /** Pointer to the current lexer instance, this is used to connect the
         * parser to the scanner. It is used in the yylex macro. */
        class Scanner* lexer;

        /** Reference to the calculator context filled during parsing of the
         * expressions. */
        //    class CalcContext& calc;

        void AstAddTerm(TermPtr &term);
        TermPtr GetAst();

        LexerTokenType m_prep_buff;

        parser::token_type NewToken(TermPtr * yylval, parser::location_type* yylloc);
        void MacroLevelBegin(TermPtr &term);
        void MacroLevelEnd(TermPtr &term);
        void MacroTerm(TermPtr &term);
        void Warning(TermPtr &term, const char *id, const char *message);

        //        typedef std::vector<std::string> MacrosArgs;
        //        typedef std::map<std::string, std::string, std::greater<std::string>> MacrosStore;

        void Init(MacroBuffer *macro) {
            time_t rawtime;
            struct tm * timeinfo;
            time(&rawtime);
            timeinfo = localtime(&rawtime);
            m_time = asctime(timeinfo);

            m_is_runing = false;
            m_is_lexer_complete = false;
            m_macro_level = 0;

            m_file_name = "";
            m_file_time = "??? ??? ?? ??:??:?? ????";
            m_md5 = "??????????????????????????????";
            m_macro = macro;
        }

        static const std::string MACROS_START;
        static const std::string MACROS_END;


        TermPtr Parse(const std::string str, MacroBuffer *macro);
        static TermPtr ParseString(const std::string str, MacroBuffer *macro);

        //        static inline std::string ParseMacroName(const std::string &body) {
        //            // имя макроса должно быть в самом начале строки без пробелов и начинаться на один слешь
        //            if (body.size() < 3 || body[0] != '\\' || body[1] == '\\') {
        //                return std::string(); // Нет имени макроса
        //            }
        //            for (size_t i = 0; i < body.size(); i++) {
        //                if (std::isspace(static_cast<unsigned char> (body[i]))) {
        //                    return body.substr(0, i);
        //                } else if (body[i] == '(') {
        //                    return body.substr(0, i + 1); // Для макросов с аргументами имя включает открывающую скобку
        //                }
        //            }
        //            return body; // Макрос без тела
        //        }
        //
        //        static inline MacrosArgs ParseMacroArgs(const std::string &body) {
        //            std::string name = ParseMacroName(body);
        //            MacrosArgs result;
        //
        //            if (name.size() < 2 || name[name.size() - 1] != '(') {
        //                return result; // Нет имени макроса или аргументов
        //            }
        //
        //            std::string arg;
        //            for (size_t i = name.size(); i < body.size(); i++) {
        //                if (body[i] == ',' || body[i] == ')') {
        //                    trim(arg);
        //                    if (!arg.empty()) {
        //                        if (arg.find(".") != std::string::npos && arg.compare("...") != 0) {
        //                            LOG_RUNTIME("Macro argument name failure '%s'!", arg.c_str());
        //                        }
        //                        result.push_back(arg); // новый аргумент макроса
        //                        arg.clear();
        //                    } else if (body[i] == ',') {
        //                        LOG_RUNTIME("Macro argument missing!");
        //                    }
        //                } else {
        //                    arg.append(1, body[i]); // имя аргумента
        //                }
        //                if (body[i] == ')') {
        //                    return result; // Аругменты закончились
        //                }
        //            }
        //            // Нет закрывающей скобки
        //            LOG_RUNTIME("Closing bracket not found!");
        //        }
        //
        //        /**
        //         * Искать в строке text начало определения макроса \\, потом завершение определения макроса \\\
//         * Вырезать тело определения макроса из строки и добавить макрос в хранилище store.
        //         * @param text Строка для парсинга
        //         * @param store Хранилище макросов
        //         * @param fill Удалять ли макрос из входной строки
        //         * @return Истина, если была произведена замена, иначе ложь
        //         */
        //        static bool ExtractMacros(std::string &text, MacrosStore &store, bool fill = true) {
        //
        //            ASSERT(MACROS_START.size() == 2);
        //            ASSERT(MACROS_END.size() == 3);
        //
        //            size_t start = text.find(MACROS_START); // Начало макроса
        //            size_t stop = std::string::npos;
        //            if (start != std::string::npos) {
        //                stop = text.find(MACROS_END, start + MACROS_START.size() + 1);
        //
        //                if (stop == std::string::npos) {
        //                    LOG_RUNTIME("Macro termination symbol not found! Start at %d '%s'", (int) start, text.c_str());
        //                    // throw Interrupt(ParserMessage(buffer, row, col, "%s", msg), Interrupt::Parser);
        //                }
        //
        //                std::string body = text.substr(start + MACROS_START.size() - 1, stop - start - MACROS_START.size() + 1);
        //                std::string name = ParseMacroName(body);
        //                if (name.empty()) {
        //                    LOG_RUNTIME("Fail parse name macro! '%s'", body.c_str());
        //                    // throw Interrupt(ParserMessage(buffer, row, col, "%s", msg), Interrupt::Parser);
        //                }
        //
        //                auto found = store.find(name);
        //                if (found != store.end()) {
        //                    LOG_RUNTIME("Macro name %s are duplicated!", name.c_str());
        //                    // throw Interrupt(ParserMessage(buffer, row, col, "%s", msg), Interrupt::Parser);
        //                }
        //                store[name] = body;
        //
        //                if (fill) {
        //                    // Заменить определение макроса пробелами, кроме переводов строк, чтобы не съезжала позиция парсинга
        //                    std::string fill_str(stop - start + MACROS_END.size(), ' ');
        //
        //                    ASSERT(fill_str.size() > body.size());
        //                    for (size_t i = 0; i < body.size(); i++) {
        //                        if (body[i] == '\n') {
        //                            fill_str[i + 1] = '\n';
        //                        }
        //                    }
        //                    text.replace(start, fill_str.size(), fill_str);
        //                }
        //                return true;
        //            }
        //            return false;
        //        }
        //
        //        static std::string ExpandMacro(const std::string &macro, const std::string &text) {
        //
        //            std::string name = ParseMacroName(macro);
        //            MacrosArgs args = ParseMacroArgs(macro);
        //
        //            std::string body_base;
        //            std::string result(text);
        //
        //            if (name.empty() || result.empty()) {
        //                return result;
        //            }
        //
        //            if (name[name.size() - 1] != '(') {
        //                body_base = macro.substr(name.size());
        //                if (!body_base.empty() && std::isspace(static_cast<unsigned char> (body_base[0]))) {
        //                    body_base = body_base.erase(0, 1);
        //                }
        //            } else {
        //                bool done = false;
        //                size_t pos = name.size();
        //                while (pos < macro.size()) {
        //                    if (macro[pos] == ')') {
        //                        // Аругменты закончились
        //                        body_base = macro.substr(pos + 1);
        //                        done = true;
        //                        break;
        //                    }
        //                    pos++;
        //                }
        //                if (!done) {
        //                    // Нет закрывающей скобки
        //                    LOG_RUNTIME("Closing bracket not found! '%s'", macro.c_str());
        //                }
        //            }
        //
        //            size_t pos_start = 0;
        //            size_t pos_end = 0;
        //            while ((pos_start + name.size() - 1) < result.size()) {
        //                pos_start = result.find(name, pos_start);
        //                if (pos_start == std::string::npos) {
        //                    break;
        //                }
        //
        //                pos_end = pos_start + name.size();
        //
        //                if (name[name.size() - 1] != '(') {
        //
        //                    result.replace(pos_start, pos_end - pos_start, body_base);
        //
        //                } else {
        //
        //                    size_t bracet_count = 0;
        //                    while (pos_end < result.size()) {
        //                        if (result[pos_end] == '(') {
        //                            bracet_count++;
        //                        } else if (result[pos_end] == ')') {
        //                            if (bracet_count == 0) {
        //                                pos_end++;
        //                                break;
        //                            } else {
        //                                bracet_count--;
        //                            }
        //                        }
        //                        pos_end++;
        //                    }
        //                    if (pos_end > result.size()) {
        //                        // Нет закрывающей скобки
        //                        LOG_RUNTIME("Closing bracket not found! '%s'", result.c_str());
        //                    }
        //
        //
        //                    MacrosArgs args_define = ParseMacroArgs(result.substr(pos_start, pos_end));
        //                    if (args_define.empty()) {
        //
        //                        result.replace(pos_start, pos_end - pos_start, body_base);
        //
        //                    } else {
        //
        //                        std::string body(body_base);
        //
        //                        for (size_t i = 0; i < args.size() && i < args_define.size(); i++) {
        //                            // Заменить аргумент по имени
        //                            if (args[i].compare("...") != 0) {
        //                                std::string arg_name = "\\\\\\$" + args[i];
        //                                body = std::regex_replace(body, std::regex(arg_name), args_define[i]);
        //                            }
        //                        }
        //
        //                        std::string summary;
        //                        for (size_t i = 0; i < args_define.size(); i++) {
        //                            // Заменить аргумент по номеру
        //                            std::string arg_num = "\\\\\\$" + std::to_string(i + 1);
        //                            body = std::regex_replace(body, std::regex(arg_num), args_define[i]);
        //
        //                            if (!summary.empty()) {
        //                                summary += ",";
        //                            }
        //                            summary += args_define[i];
        //                        }
        //                        body = std::regex_replace(body, std::regex("\\\\\\$\\*"), summary);
        //
        //                        result.replace(pos_start, pos_end - pos_start, body);
        //                    }
        //                }
        //            }
        //            return result;
        //        }
        //
        //        static std::string ParseAllMacros(const std::string input, MacrosStore *store) {
        //            std::string result(input);
        //            if (store) {
        //                bool done;
        //                do {
        //                    done = ExtractMacros(result, *store);
        //                } while (done);
        //
        //                for (auto &elem : *store) {
        //                    result = ExpandMacro(elem.second, result);
        //                }
        //            }
        //            return result;
        //        }


        std::string m_time;
        std::string m_file_name;
        std::string m_file_time;
        std::string m_md5;

    private:
        TermPtr m_ast;
        bool m_is_runing;
        bool m_is_lexer_complete;
        size_t m_macro_level;
        MacroBuffer *m_macro;

    };

} // namespace example

#endif // NEWLANG_PARSER_H_
