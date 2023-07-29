#ifndef NEWLANG_NAMED_H_
#define NEWLANG_NAMED_H_

#include "pch.h"

#include <variable.h>
#include <diag.h>
#include <object.h>

#include <warning_push.h>
#include "parser.yy.h"
#include <warning_pop.h>

namespace newlang {

    typedef std::shared_ptr<std::string> SourceType;
    typedef std::vector<std::string> PostLexerType;

    inline std::string MakeName(std::string name) {
        if (!name.empty() && (name[0] == '\\' || name[0] == '$' || name[0] == '@' || name[0] == '%')) {
            return name.find("\\\\") == 0 ? name.substr(2) : name.substr(1);
        }
        return name;
    }

    inline std::string ExtractModuleName(const char *str) {
        std::string name(str);
        if (isModule(name)) {
            size_t pos = name.find("::");
            if (pos != std::string::npos) {

                return name.substr(0, pos);
            }
            return name;
        }
        return std::string();
    }

    inline std::string ExtractName(std::string name) {
        size_t pos = name.rfind("::");
        if (pos != std::string::npos) {
            name = name.substr(pos + 2);
        }
        if (isModule(name)) {

            return std::string();
        }
        return name;
    }

    /**
     * Единица хранения информации об именованнм объекте
     * 
     */
    struct NamedItem {
        /** 
         * Определение прототипа, включая макросы и предварительные определения с помощью @__PRAGMA_PROTOTYPE__
         * Должне быть определн всегда, так как именно по этому объекту производится сранение типов у переменных и проверка параметров у функций.
         * Создается для любого именнованного объекта системы (макрос, модуля, тип, функция или переменная) во время парсинга
         */
        BlockType proto;
        /**
         * Термин с определением (инициализацией) объекта в исходном коде (включает в себя место определения в файле в @ref Term::m_lexer_loc)
         * Создается при инициалиазции (загрузке) модуля или определнии (создании) класса, функции или перемной.
         * Не создется для маросов и прототипов (@__PRAGMA_PROTOTYPE__)
         * Заполняется во время парсинга или анализа???
         * 
         * Реально созданный объект хранится в поле @ref Term::m_obj
         * Он создается при выполнении программы (инициалиазции (загрузке) модуля, определении (создании) класса, функции или перемной).
         * @ref Term::m_obj - слабая ссылка (std::weak_ptr<Obj>), чтобы память объекта не держалась в таблице символов
         */
        BlockType term;
        //        /**
        //         * Реально созданный объект.
        //         * Создается при выполнении программы (инициалиазции (загрузке) модуля, определении (создании) класса, функции или перемной).
        //         * Заполняется для модулей и типов - во время парсинга/анализа
        //         * для функций и перемных - во время компиляции и выполнения
        //         */
        //        FuncItem obj;
    };

    class Named : SCOPE(protected) std::map<std::string, NamedItem>, public std::enable_shared_from_this<Named> {
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
         * @ old @ new @; @ old(...) @ new(\$*) @;
         * @replace($old, $new)@\ @ \$old @ \$new @; @ \$old(...)@ \$new(\$*)@  @\;
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


        Named();

        virtual ~Named() {
        }

        typedef at::variant<ObjPtr, std::vector < ObjPtr> > FuncItem;

        std::map<std::string, ObjPtr> m_types;
        std::map<std::string, FuncItem> m_funcs;
        //        std::map<std::string, EvalFunction> m_builtin_calls;

        static const std::string deny_chars_from_macro;
        std::vector<std::string> m_ns_stack;

        static std::string toMacroHash(const TermPtr &term);

        inline static std::string toMacroHashName(const std::string str) {
            if (isLocal(str)) {
                return "$"; // Template
            } else if (isMacro(str)) {
                return str.substr(1); // without macro prefix
            }
            return str;
        }

        std::string GetMacroMaping(const std::string str, const char *separator = ", ");

        void Push(const TermPtr term);
        void Pop(const TermPtr term);

        //        parser::token_type ExpandPredefMacro(TermPtr &term, Parser * parser);
        //
        //        static int m_counter;
        //        std::map<std::string, std::string> m_predef_macro;
        //        bool RegisterPredefMacro(const char * name, const char * desc);




        /**
         * Проверяет оператор на предмет наличия операции создания или удаления макроса.
         * @param term - оператор для проверки
         * @return Истина, если оператор содержит оператор создания или удаления макроса
         */
        //        static bool CheckOpMacros(const TermPtr & term);
        //
        //        static TermPtr CreateMacroFromOp(const TermPtr & term);

        //        static bool CheckAndConvertMacros(TermPtr &term);
        /**
         * Выполнить (применить) макрос
         * @param term - Термин макрос
         */
        TermPtr EvalOpMacros(const TermPtr &term);

        bool RemoveMacro(const TermPtr & term);

        size_t GetCount() {
            size_t count = 0;
            auto iter = begin();
            while (iter != end()) {
                count += iter->second.proto.size();
                iter++;
            }
            return count;
        }

        //        void Clear(size_t level);

        TermPtr GetMacroById(const BlockType block);
        TermPtr GetMacro(std::vector<std::string> list);
        //        bool isExist(const TermPtr & term);

        /**
         * Идентифицирует входной буфер с макросом. 
         * Буфер должен заканчиваться на ';' или концом файла END, т.е. макрос грантированно присуствиет в буфере полностью.
         * @param buffer Входной буфер лексем
         * @param term Макрос для сопоставления
         * @return Соотвествие входного буфера макросу. 
         * Для увеличения скорости работы функция не проверяет аргументы макроса (только формальное наличие или отсутствие скобок).
         * Контроль аргументов макроса выполняется в функции \ref ExtractArgs
         */
        //        static bool IdentityAlias(BlockType &buffer, const TermPtr & term);
        static bool IdentityMacro(const BlockType &buffer, const TermPtr & term);
        static bool CompareMacroName(const std::string & term_name, const std::string & macro_name);

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
         * @return Кол-во символов входного буфера, которые нужно будет заменить телом макросом
         */
        static size_t ExtractArgs(BlockType &buffer, const TermPtr &term, MacroArgsType & args);
        static void InsertArg_(MacroArgsType & args, std::string name, BlockType &buffer, size_t pos = static_cast<size_t> (-1));
        static BlockType SymbolSeparateArg_(const BlockType &buffer, size_t pos, std::vector<std::string> name, std::string & error);

        /* 
         * Раскрывает макрос в последовательность терминов лексера с заменой аргументов
         */
        static BlockType ExpandMacros(const TermPtr &macro, MacroArgsType & args);
        /* 
         * Раскрывает макрос в текстовую строку с заменой аргументов
         */
        static std::string ExpandString(const TermPtr &macro, MacroArgsType & args);


        std::string Dump();
        static std::string Dump(const MacroArgsType & var);
        static std::string Dump(const BlockType & arr);

        static std::vector<std::string> SplitString(const char * str, const char *delim) {

            std::vector<std::string> result;
            std::string s(str);

            size_t pos;
            s.erase(0, s.find_first_not_of(delim));
            while (!s.empty()) {
                pos = s.find(delim);
                if (pos == std::string::npos) {
                    result.push_back(s);
                    break;
                } else {
                    result.push_back(s.substr(0, pos));
                    s.erase(0, pos);
                }
                s.erase(0, s.find_first_not_of(delim));
            }
            return result;
        }



        bool TestName(std::string_view name);

        std::string CreateFullName(std::string_view name);
        //        std::string CreateFullName(TermPtr term) {
        //            ASSERT(term);
        //
        //            return CreateFullName(term->m_text);
        //        }

        /**
         * Зарегистрировать новый прототип.
         * Используется для макросов и прототипов в @__PRAGMA_PROTOTYPE__
         * Заполняется только поле @ref NamedItem::proto, а остальные остаются не инициалмизированными.
         * Требуется только для парсера при раскрытии макросов и при проверке используемых имен
         * @param term - новый прототип для регистрации
         * @param gen_exception - возвращать false или генерировать исключение 
         * @return true - если новый прототип зарегистрирован, иначе false (если gen_exception=false) или создать исключение
         */
        bool RegisterProto(TermPtr term, bool gen_exception = true);
        /**
         * Зарегистрировать новый термин и его прототип.
         * Используется при определении объектов или загрузке модулей
         * Заполняется поле @ref NamedItem::term и @ref NamedItem::proto - если оно не заполнено.
         * Требуется для парсера при проверке используемых имен
         * @param term - новый термин для регистрации
         * @param gen_exception - возвращать false или генерировать исключение 
         * @return true - если новый термин зарегистрирован, иначе false (если gen_exception=false) или создать исключение
         */
        bool RegisterTerm(TermPtr term);
        /**
         * Зарегистрировать объект.
         * Заполняется поле @ref NamedItem::obj. Поля @ref NamedItem::term и @ref NamedItem::proto должны присутствовать.
         * @param term - термин
         * @param obj - Объект
         * @param gen_exception - возвращать false или генерировать исключение 
         * @return true - если объект зарегистрирован, иначе false (если gen_exception=false) или создать исключение
         */
        bool RegisterObj(TermPtr term, ObjPtr obj);

        void ApplyDiags(DiagPtr diag) {
        }

        DiagPtr m_diag;

        bool NamespasePush(const std::string &name) {
            if (name.empty()) {
                return false;
            }
            m_ns_stack.push_back(name);
            return true;
        }

        void NamespasePop() {
            ASSERT(!m_ns_stack.empty());
            m_ns_stack.pop_back();
        }

        std::string NamespaseFull(std::string name = "") {
            if (name.find("::") != 0) {
                for (size_t i = m_ns_stack.size(); i > 0; i--) {
                    if (!name.empty()) {
                        name.insert(0, "::");
                    }
                    if (m_ns_stack[i - 1].compare("::") == 0) {
                        if (name.empty()) {
                            name = "::";
                        }
                        break;
                    }
                    name.insert(0, m_ns_stack[i - 1]);
                    if (m_ns_stack[i - 1].find("::") == 0) {
                        break;
                    }
                }
            }
            //            if (!isFullName(name)) {
            //                name.insert(0, "::");
            //            }
            return name;
        }


        TermPtr FindTerm(std::string_view name);

        inline TermPtr GetTerm(std::string_view name) {
            TermPtr term = FindTerm(name);
            if (!term) {

                LOG_RUNTIME("Term '%s' not found!", name.begin());
            }
            return term;
        }

        ObjPtr FindObj(std::string_view name);

        inline ObjPtr GetObj(std::string_view name) {
            ObjPtr obj = FindObj(name);
            if (!obj) {

                LOG_RUNTIME("Object '%s' not found!", name.begin());
            }
            return obj;
        }


        /**
         * Функция для организации встроенных типов в иерархию наследования.
         * Другие функции: @ref CreateBaseType - создает базовые типы данных (для расширения классов требуется контекст)
         * и @ref BaseTypeConstructor - функция обратного вызова при создании нового объекта базового типа данных
         * @param type - Базовый тип данных @ref ObjType
         * @param parents - Список сторок с именами родительских типов
         * @return - Успешность регистрации базовго типа в иерархии
         */
        bool RegisterTypeHierarchy(ObjType type, std::vector<std::string> parents);

        ObjType BaseTypeFromString(const std::string & type, bool *has_error = nullptr);

        ObjPtr GetTypeFromString(const std::string & type, bool *has_error = nullptr);

        static bool pred_compare(const std::string &find, const std::string &str) {
            size_t pos = 0;
            while (pos < find.size() && pos < str.size()) {
                if (find[pos] != str[pos]) {
                    return false;
                }
                pos++;
            }
            return find.empty() || (pos && find.size() == pos);
        }

        std::vector<std::wstring> SelectPredict(std::wstring wstart, size_t overage_count = 0) {
            return SelectPredict(utf8_encode(wstart), overage_count);
        }
        std::vector<std::wstring> SelectPredict(std::string start, size_t overage_count = 0);


        //        SCOPE(private) :
        //        MacroTokenType m_store;
    };

} // namespace example

#endif // NEWLANG_NAMED_H_
