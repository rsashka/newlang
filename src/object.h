#pragma once
#ifndef INCLUDED_NEWLANG_OBJECT_
#define INCLUDED_NEWLANG_OBJECT_

#include <pch.h>

#include <types.h>
#include <variable.h>
#include <rational.h>

namespace newlang {

    /*
     * Аргумент по умолчанию может быть литерал или выражение.
     * Если аргумент по умолчанию — это выражение, то оно вычисляется только один раз для всей программы при загрузке модуля.
     * Аргументы по умолчанию парсятся из токенов (создаются или вычисляются) при загрузке модуля, т.е. при создании
     * аругментов по умолчанию, а сами значения хранятся уже как объекты.
     * 
     * Аргументы в функции всегда имеют номер, который начинается с единицы в порядке определения в прототипе + некоторые могут иметь наименование.
     * Даже обязательные аргументы (которые не имеют значения по умолчанию в прототипе финкции), могут быть переданы по имени, 
     * а сами именованные аргументы могут быть указаны в произвольном порядке. Поэтому в реализации Args для обязательных 
     * аргументов так же хранится имя аргумента, но его значение отсутствует.
     * 
     * Так как позционные аргументы тоже могут передавать как именованные, то контроль и подстчет кол-ва 
     * не именованных аргументов при вызове фунции ничего не определяет. 
     * Следовательно, при вызове функции не именованные аргументы записываются по порядку, а именованные по имени.
     * Контроль передаваемых аргументов производится на наличие значения у каждого аргумента.
     * Если при определении функции после всех аргументов стоит многоточие, то разрешается добавлять 
     * новые аргументы по мимо тех, которые уже определены в прототипе функции.
     */

    //    ObjType DictionarySummaryType(const Obj *obj);
    std::vector<int64_t> TensorShapeFromDict(const Obj *obj);
    //    torch::Tensor ConvertToTensor(Obj *obj, at::ScalarType type = at::ScalarType::Undefined, bool reshape = true);
    //
    //    at::TensorOptions ConvertToTensorOptions(const Obj *obj);
    //    at::DimnameList ConvertToDimnameList(const Obj *obj);
    bool ParsePrintfFormat(Obj *args, int start = 1);

    ObjPtr CheckSystemField(const Obj *obj, const std::string name);

    enum class ConcatMode : uint8_t {
        Error = 0,
        Append = 1,
        Discard = 2,
    };
    /* 
     * Для строк, словарей и классов (преобразование в одно измерение), тензоров (преобразование согласно ConcatMode)
     */
    int64_t ConcatData(Obj *dest, Obj &src, ConcatMode mode = ConcatMode::Error);

    // Convert a wide Unicode string to an UTF8 string

    inline std::string utf8_encode(const std::wstring wstr) {
        std::string utf8line;

        if (wstr.empty()) {
            return utf8line;
        }
        utf8line = std::wstring_convert < std::codecvt_utf8<wchar_t>>().to_bytes(wstr.c_str());
        return utf8line;
    }

    // Convert an UTF8 string to a wide Unicode String

    inline std::wstring utf8_decode(const std::string str) {
        std::wstring wide_line;

        if (str.empty()) {
            return wide_line;
        }
        wide_line = std::wstring_convert < std::codecvt_utf8<wchar_t>>().from_bytes(str.c_str());
        return wide_line;
    }

    /* Для конвертирования словаря в тензор для вывода общего типа данных для всех элементов */
    ObjType getSummaryTensorType(Obj *obj, ObjType start);
    void ConvertStringToTensor(const std::string &from, torch::Tensor &to, ObjType type = ObjType::None);
    void ConvertStringToTensor(const std::wstring &from, torch::Tensor &to, ObjType type = ObjType::None);
    void ConvertTensorToString(const torch::Tensor &from, std::string &to, std::vector<Index> *index = nullptr);
    void ConvertTensorToString(const torch::Tensor &from, std::wstring &to, std::vector<Index> *index = nullptr);
    void ConvertTensorToDict(const torch::Tensor &from, Obj &to, std::vector<Index> *index = nullptr);

    /*
     * Требуется разделять конейнеры с данными и итераторы по данным.
     * В С++ контейнеры предоставялют итераторы непосредственно из самого контейнера.
     * Это удобно для использования в рукописном коде, но сложно контроллировать в генерируемом исходнике, 
     * т.к. требуется использовать и итертор и сам контейнер с данными (для сравнения текущего элемента с end()).
     * 
     * Итератор - отдельный объект, которому для создания требуется контейнер с данными.
     * Итератор реализует два интерфейса перебора данных:
     * 1. - первый интерфейс итератора как в С++ 
     * 2. - второй интерфейс для использования в генерируемом коде  с логикой NewLang
     * 
     * template Iterator - Обертка над классическим С++ итератором с добавлением возможности отбора
     * элементов по имени поля или с помощь функции обратного вызова для пользовательской фильтрации.
     * 
     * Программынй интерфейс итераторов для NewLang следующий:
     * 
     * ObjPtr IteratorMake(const std::string filter) - создать итератор с возможностью фильтрации по имени поля (данные на начало)
     * ObjPtr IteratorMake(Obj * args) - создать итератор с фильтрации с помощью пользовательской функции (данные на начало)
     * ObjPtr IteratorReset() - Сбросить итератор на начало данных (возвращает сам итератор)
     * ObjPtr IteratorData() - прочитать текущий элемент без смещения указателя. Если данных нет возвращается "конец итератора"
     * ObjPtr IteratorNext(int64_t count)- прочитать заданное кол-во элементов и переместить итератор на следующий элемент. 
     * При не нулевом кол-ве, данные возвращаются как элементы словаря. Если указан кол-во элеметов 0 - возвращается текущий элемент.
     * 
     * Реализаиця итераторов NewLang с помощью данного интерфейса:
     *
     * Создание итератора
     * ?, ?("Фильтр"), ?(func), ?(func, args...)    - IteratorMake
     * 
     * Перебор элементов итератора
     * !, !(0), !(3), !(-3)
     * 
     * dict! и dict!(0) <НЕ> эквивалентны, т.к. по разному обработывают конец данных
     * dict! -> 1,  dict! -> 2, dict! -> 3, dict! -> 4, dict! -> 5, dict! -> исключение "конец итератора"
     * dict!(0) -> 1,  dict!(0) -> 2, ... dict!(0) -> 5, dict!(0) -> :IteratorEnd (может :Empty  - пустое значение ?? )
     * dict!(1) -> (1,),  dict!(1) -> (2,), ... dict!(1) -> (5,), dict!(1) -> (,)
     * 
     * Различия отрицательного размера возвращаемого словаря для итератора 
     * (Для отрцетельного размера всегда зозвращается словарь указанного размера)
     * dict!(-1) -> (1,),  ...  dict!(-1) -> (5,),  dict!(-1) -> (:IteratorEnd,),  
     * dict!(1) -> (1,),  ...  dict!(1) -> (5,),  dict!(1) -> (,),  
     * dict!(-3) -> (1, 2, 3,),  dict!(-3) -> (4, 5, :IteratorEnd,)
     * dict!(3) -> (1, 2, 3,), dict!(3) -> (4, 5,)
     * 
     * Операторы ?! и !? эквивалентны и возвращают текущие данные без перемещения указателя итератора (IteratorData) и не принимают аргументов.
     * Остальные итераторы можно вызвать либо без скобок, либо с аргментами в скобрках. Вызов без аргументов зарпрешщен 
     * (чтобы не пересекаться с логикой копирования объектов и не делать для итераторов аругменты по умолчанию)
     * 
     * Оператор ?? создает итератор и сразу его выполняет, возвращая все значения 
     * в виде элементов словаря, т.е. аналог последовательности ?(LINQ); !(:Int64.__max__);
     * 
     * Оператор !! без аргументов - сбрасывает итератор в начальное состояние (IteratorReset),
     * Обператор !! с аргументами выполняется как ! с аругментами, но с начла коллекции.
     * 
     * [ ] - оператор приведения данных в логический тип. Испольуется в алгоритмических конструкциях (проверка условий и циклы)
     * Правила преобразования в логический тип:
     * Словарь или класс - нет элементов - false, есть элементы - true
     * Число - ноль или нулевой тензо - false, иначе true
     * Строка - пустая строка false, иначе true
     * Итератор - конец данных false, иначе true
     * :IteratorEnd (:Empty ?) - всегда false
     * None - true (Это объекст со значением пусто)
     * Empty - false (Не инициализированный объект)
     * 
     * 
     * Логика обработки ссылок
     * term1 :=  term; # Объект term1 - копия term.
     * term2 :=  &term; # Объект term2 - ссылка на term (одни и те же данные, т.е. shared_ptr ссылаются на один и тот же объект)
     * &term3 :=  term; # Создать объект term3 и вернуть ссылку на него (сахар для term3 :=  term; &term3;)
     * 
     * 
     * copy(arg) :=  {}; # Обычная функция принимает любой аргумент как <КОПИЮ> значения
     * copy(term1); # ОК - передается <КОПИЯ> term1
     * copy(term2); # ОК - передается <КОПИЯ> term2
     * copy(&term1); # ОК - передается ссылка на term1 (На самом деле копия ссылки, которая указывает на те же данные)
     * copy(&term2); # ОК- передается ссылка на term2 (На самом деле копия ссылки, которая указывает на те же данные)
     * 
     * ptr(&arg) :=  {}; # Функция, которая принимает только аргумент - <ссылку>
     * ptr(term1); # Ошибка при компиляции - нужно передавать ссылку !!!!!!!!
     * ptr(&term1); # ОК
     * ptr(&term2); # ОК
     * ptr(term2); # Ошибка при компиляции - нужно передавать ссылку, несмотря на то что term2 УЖЕ содержит ссылку !!!!!!!!
     * 
     * 
     * 
     * 
     * <% - Кнстанты и свойста>
     * Значением константы может быть любой литерал (строка или число)
     * 
     * Так как константа определяется во время компиляции, то её значение может быть использовано компилятором 
     * для анализа исходного текста программы после парсинга ATS и на основе их значений влиять на генерируемый 
     * трансплайтером финальный код на языке реализации.
     * 
     * Константа доступна во время выполнения для чтения как значение системного поля __option__ ( для константы %option)
     * ::%option - Глобальная область видимости для константы
     * 
     * С помощью констант реализуется одноименный функционал (переменные только для чтения), путем установки 
     * соответствующего поля (term%const=1; или term%const=0;)
     * 
     * <Для релиза компилатора, т.к. требует анализа исходникой на уровне файла и предназаначена для генерации кода на С++>
     * 
     */

    template <typename T>
    class Iterator : public std::iterator<std::input_iterator_tag, T> {
    public:

        friend class Obj;
        friend class Variable<Obj>;
        friend class Context;
        friend class Variable<Context, std::weak_ptr<Context>>;

        enum class IterCmp : int8_t {
            No = static_cast<int8_t> (ObjType::None), /* skip data */
            Yes = static_cast<int8_t> (ObjType::Iterator), /* return data */
            End = static_cast<int8_t> (ObjType::IteratorEnd), /* iterator complete */
        };

        STATIC_ASSERT(static_cast<bool> (IterCmp::Yes));
        STATIC_ASSERT(!static_cast<bool> (IterCmp::No));

        typedef Iterator<T> iterator;
        typedef Iterator<const T> const_iterator;

        typedef Variable<T> IterObj;
        typedef typename Variable<T>::Type IterObjPtr;
        typedef typename Variable<T>::PairType IterPairType;
        typedef typename Variable<T>::ListType IterListType;


        typedef IterCmp CompareFuncType(const IterPairType &pair, const T *args, void *extra);

        //        static const std::string FIND_KEY_DEFAULT;

        /**
         * Итератор для элементов списка с regex фильтром по имени элемента (по умолчанию для всех элементов списка)
         * @param obj
         * @param find_key
         */
        explicit Iterator(std::shared_ptr<T> obj, const char * find_key = "(.|\\n)*") :
        Iterator(obj, &CompareFuncDefault, reinterpret_cast<T *> (const_cast<char *> (find_key)), static_cast<void *> (this)) {
        }

        /**
         * Итератор для элементов списка с фильтром в виде функции обратного вызова
         * @param obj
         * @param func
         * @param arg
         * @param extra
         */
        Iterator(std::shared_ptr<T> obj, CompareFuncType *func, T *arg, void * extra = nullptr) :
        m_iter_obj(obj), m_match(), m_func(func), m_func_args(arg), m_func_extra(extra), m_found(m_iter_obj->begin()), m_base_filter(nullptr) {
            search_loop();
        }

        Iterator(const Iterator &iter) : m_iter_obj(iter.m_iter_obj), m_match(iter.m_match), m_func(iter.m_func),
        m_func_args(iter.m_func_args), m_func_extra(iter.m_func_extra), m_found(iter.m_found), m_base_filter(iter.m_base_filter) {
        }

        SCOPE(private) :
        std::shared_ptr<T> m_iter_obj;
        std::regex m_match;
        std::string m_filter;
        CompareFuncType *m_func;
        T *m_func_args;
        void *m_func_extra;
        mutable typename Variable<T>::iterator m_found;
        const char * m_base_filter;

        static const IterPairType m_Iterator_end;

        static IterCmp CompareFuncDefault(const IterPairType &pair, const T *filter, void *extra) {
            const char * str_filter = reinterpret_cast<const char *> (filter);
            Iterator * iter = static_cast<Iterator *> (extra);
            if (iter && str_filter) {

                iter->m_base_filter = str_filter;
                iter->m_func_args = nullptr; // Строка для фильтрации передается однократно при создании объекта
                iter->m_filter.assign(str_filter);

                if (!iter->m_filter.empty()) {
                    try {
                        iter->m_match = std::regex(iter->m_filter);
                    } catch (const std::regex_error &err) {
                        LOG_RUNTIME("Regular expression for '%s' error '%s'!", str_filter, err.what());
                    }
                }
            }

            if (iter) {
                if (iter->m_base_filter == nullptr) { // Без фильтра отдаются вообще все поля
                    return IterCmp::Yes;
                } else if (iter->m_filter.empty()) { // Если передана пустая строка, то выдаются только поля без имен
                    return pair.first.empty() ? IterCmp::Yes : IterCmp::No;
                } else {
                    // Иначе если имя поля совпадает с регуляркой
                    return std::regex_match(pair.first, iter->m_match) ? IterCmp::Yes : IterCmp::No;
                }
            }
            return IterCmp::Yes;
        }

    public:

        iterator begin() {
            Iterator<T> copy(*this);
            copy.reset();
            return copy;
        }

        iterator end() {
            Iterator<T> copy(*this);
            copy.m_found = copy.m_iter_obj->end();
            return copy;
        }

        inline const IterPairType &data() {
            if (m_found == m_iter_obj->end()) {
                return m_Iterator_end;
            }
            return *m_found;
        }

        inline const IterPairType &operator*() {
            return data();
        }

        inline const IterPairType &data() const {
            if (m_found == m_iter_obj->end()) {
                return m_Iterator_end;
            }
            return *m_found;
        }

        inline const IterPairType &operator*() const {
            return data();
        }

        ObjPtr read_and_next(int64_t count);

        const iterator &operator++() const {
            if (m_found != m_iter_obj->end()) {
                m_found++;
                search_loop();
            }
            return *this;
        }

        inline const iterator operator++(int) const {
            return iterator::operator++();
        }

        inline bool operator==(const iterator &other) const {
            return m_found == other.m_found;
        }

        inline bool operator!=(const iterator &other) const {
            return m_found != other.m_found;
        }

        inline void reset() {
            m_found = m_iter_obj->begin();
            search_loop();
        }


    protected:

        void search_loop() const {
            while (m_found != m_iter_obj->end()) {
                IterCmp result = IterCmp::Yes;
                if (m_func) {
                    result = (*m_func)(*m_found, m_func_args, m_func_extra);
                    if (result == IterCmp::End) {
                        m_found = m_iter_obj->end();
                    }
                }
                if (result != IterCmp::No) {
                    // IterCmp::Yes || IterCmp::End
                    return;
                }
                m_found++;
            }
        }
    };

    /*
     * 
     * 
     */
    class Obj : protected Variable<Obj>, public std::enable_shared_from_this<Obj> {
    public:

        //    constexpr static const char * BUILDIN_TYPE = "__var_type__";
        //    constexpr static const char * BUILDIN_NAME = "__var_name__";
        //    constexpr static const char * BUILDIN_BASE = "__class_base__";
        //    constexpr static const char * BUILDIN_CLASS = "__class_name__";
        //    constexpr static const char * BUILDIN_NAMESPACE = "__namespace__";

        typedef Variable::PairType PairType;

        Obj(ObjType type = ObjType::None, const char *var_name = nullptr, TermPtr func_proto = nullptr, ObjType fixed = ObjType::None, bool init = false) :
        m_var_type_current(type), m_var_name(var_name ? var_name : ""), m_prototype(func_proto) {
            m_is_const = false;
            m_check_args = false;
            m_dimensions = nullptr;
            m_is_reference = false;
            m_var_type_fixed = fixed;
            m_var_is_init = init;
            m_is_const = false;
            m_var = at::monostate();
            ASSERT(!m_tensor.defined());
        }

        Obj(Context *ctx, const TermPtr term, bool as_value, Obj *local_vars);


        [[nodiscard]]
        static inline PairType ArgNull(const std::string name = "") {
            return pair(nullptr, name);
        }

        [[nodiscard]]
        static inline PairType Arg() {
            return pair(CreateNone());
        }

        [[nodiscard]]
        static inline PairType Arg(ObjPtr value, const std::string name = "") {
            return pair(value, name);
        }

        template<typename T>
        typename std::enable_if<std::is_pointer<T>::value || std::is_same<std::string, T>::value, PairType>::type
        static inline Arg(T value, const std::string name = "") {
            return pair(CreateString(value), name);
        }

        template<typename T>
        typename std::enable_if<!std::is_same<PairType, T>::value && !std::is_pointer<T>::value && !std::is_same<std::string, T>::value, PairType>::type
        static inline Arg(T value, const std::string name = "") {
            return pair(CreateValue(value, ObjType::None), name);
        }

        inline ObjPtr shared() {
            try {
                return shared_from_this();
            } catch (std::bad_weak_ptr &err) {
                LOG_RUNTIME("Exception thrown bad_weak_ptr! %s", err.what());
            }
        }

        ObjPtr MakeConst() {
            m_is_const = true;
            return shared();
        }

        ObjPtr MakeMutable() {
            m_is_const = false;
            return shared();
        }

        virtual ObjPtr IteratorMake(const char * filter = nullptr, bool check_create = true);
        virtual ObjPtr IteratorMake(Obj *args);
        virtual ObjPtr IteratorData();
        virtual ObjPtr IteratorReset();
        virtual ObjPtr IteratorNext(int64_t count);

        inline ObjPtr IteratorNext(ObjPtr count) {
            return IteratorNext(count->GetValueAsInteger());
        }



#define TEST_CONST_() if (m_is_const) {LOG_RUNTIME("Can`t edit const value '%s'!", toString().c_str());}
#define TEST_INIT_() if (!m_var_is_init) {LOG_RUNTIME("Object not initialized '%s'!", toString().c_str());}

        [[nodiscard]]
        inline ObjType getType() {
            return m_var_type_current;
        }

        [[nodiscard]]
        inline ObjType getTypeAsLimit() {
            if (is_arithmetic_type()) {
                if (isIntegralType(m_var_type_current, true) && is_scalar()) {
                    return typeFromLimit(GetValueAsInteger());
                } else if (isFloatingType(m_var_type_current) && is_scalar()) {
                    return typeFromLimit(GetValueAsNumber());
                }
            } else if (is_type_name()) {
                return m_var_type_fixed;
            }
            return m_var_type_current;
        }

        [[nodiscard]]
        inline std::string & getName() {
            return m_var_name;
        }

        [[nodiscard]]
        inline const std::string & getName() const {
            return m_var_name;
        }

        void SetClassName(std::string &name) {
            m_class_name = name;
        }

        [[nodiscard]]
        inline bool is_const() const {
            return m_is_const;
        }

        [[nodiscard]]
        inline bool is_init() const {
            return m_var_is_init;
        }

        [[nodiscard]]
        inline bool is_none_type() const {
            return m_var_type_current == ObjType::None;
        }

        [[nodiscard]]
        inline bool is_bool_type() const {
            return isBooleanType(m_var_type_current);
        }

        [[nodiscard]]
        inline bool is_arithmetic_type() const {
            return isArithmeticType(m_var_type_current);
        }

        [[nodiscard]]
        inline bool is_string_type() const {
            return isString(m_var_type_current);
        }

        [[nodiscard]]
        inline bool is_string_char_type() const {
            return isStringChar(m_var_type_current);
        }

        [[nodiscard]]
        inline bool is_string_wide_type() const {
            return isStringWide(m_var_type_current);
        }

        [[nodiscard]]
        inline bool is_dictionary_type() const {
            return isDictionary(m_var_type_current);
        }

        //Plain data — это неизменяемые структуры без ссылок на другие объекты.
        [[nodiscard]]
        inline bool is_plain_type() const {
            return isPlainDataType(m_var_type_current);
        }

        [[nodiscard]]
        inline bool is_other_type() const {
            return !(is_none_type() || is_bool_type() || is_arithmetic_type() || is_string_type() || is_dictionary_type());
        }

        [[nodiscard]]
        inline bool is_class_type() const {
            return isClass(m_var_type_current);
        }

        [[nodiscard]]
        inline bool is_simple_type() const {
            return isSimpleType(m_var_type_current);
        }

        [[nodiscard]]
        inline bool is_scalar() const {
            return is_tensor_type() && !m_tensor.defined();
        }

        [[nodiscard]]
        inline bool is_function_type() const {
            return isFunction(m_var_type_current);
        }

        [[nodiscard]]
        inline bool is_tensor_type() const {
            return isTensor(m_var_type_current);
        }

        [[nodiscard]]
        inline bool is_integer() const {
            return isIntegralType(m_var_type_current, false);
        }

        [[nodiscard]]
        inline bool is_integral() const {
            return isIntegralType(m_var_type_current, true);
        }

        [[nodiscard]]
        inline bool is_complex() const {
            return isComplexType(m_var_type_current);
        }

        [[nodiscard]]
        inline bool is_floating() const {
            return isFloatingType(m_var_type_current);
        }

        [[nodiscard]]
        inline bool is_indexing() const {
            return isIndexingType(m_var_type_current, m_var_type_fixed) || is_error() || is_return();
        }

        [[nodiscard]]
        inline bool is_ellipsis() const {
            return isEllipsis(m_var_type_current);
        }

        [[nodiscard]]
        inline bool is_range() const {
            return isRange(m_var_type_current);
        }

        [[nodiscard]]
        inline bool is_rational() const {
            return m_var_type_current == ObjType::Rational;
        }

        [[nodiscard]]
        inline bool is_type_name() const {
            return isTypeName(m_var_type_current);
        }

        [[nodiscard]]
        inline bool is_error() const {
            return m_var_type_current == ObjType::Error || m_var_type_fixed == ObjType::Error || m_var_type_current == ObjType::ErrorParser || m_var_type_current == ObjType::ErrorRunTime || m_var_type_current == ObjType::ErrorSignal;
        }

        [[nodiscard]]
        inline bool is_return() const {
            return m_var_type_current == ObjType::Return || m_var_type_fixed == ObjType::Return || m_var_type_current == ObjType::RetPlus || m_var_type_current == ObjType::RetMinus;
        }

        [[nodiscard]]
        inline bool is_block() const {
            return m_var_type_current == ObjType::BLOCK || m_var_type_current == ObjType::BLOCK_PLUS || m_var_type_current == ObjType::BLOCK_MINUS || m_var_type_current == ObjType::BLOCK_TRY;
        }


        [[nodiscard]]
        inline bool is_defined_type() {
            return m_var_type_fixed != ObjType::None;
        }

        inline void SetTermProp(Term &term);

        virtual int64_t size() const {
            return size(0);
        }
        virtual int64_t size(int64_t ind) const;
        int64_t resize_(int64_t size, ObjPtr fill, const std::string = "");

        virtual bool empty() const {
            if (is_none_type()) {
                return true;
            } else if (m_var_type_current == ObjType::StrChar) {
                return !m_var_is_init || m_value.empty();
            } else if (m_var_type_current == ObjType::StrWide) {
                return !m_var_is_init || m_string.empty();
            } else if (is_tensor_type()) {
                return !m_var_is_init || at::_is_zerotensor(m_tensor);
            }
            return Variable<Obj>::empty();
        }

        Variable<Obj>::PairType & at(const std::string name) const {
            Obj * const obj = (Obj * const) this;
            ObjPtr sys = CheckSystemField(this, name.c_str());
            if (!!sys) {
                m_str_pair.first = name;
                m_str_pair.second = sys;
                return m_str_pair;
            }
            return obj->Variable<Obj>::at(name);
        }

        Variable<Obj>::PairType & at(const std::string name) override {
            ObjPtr sys = CheckSystemField(this, name.c_str());
            if (!!sys) {
                m_str_pair.first = name;
                m_str_pair.second = sys;
                return m_str_pair;
            }
            return Variable<Obj>::at(name);
        }

        Variable<Obj>::PairType & at(const int64_t index) override;
        const Variable<Obj>::PairType & at(const int64_t index) const override;

        Variable<Obj>::PairType & at(ObjPtr find) {
            if (!find->is_string_type()) {
                LOG_RUNTIME("Value must be a string type %d", (int) find->getType());
            }
            return Variable<Obj>::at(find->GetValueAsString());
        }

        bool exist(ObjPtr &find, bool strong);
        size_t ItemValueCount(ObjPtr &find, bool strong);

        static bool is_true(const char *text) {
            std::string temp(text);
            std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
            return temp.compare("true") == 0;
        }

        /**
         * Создать копию объекта (клонировать)
         * @return 
         */
        ObjPtr operator()(Context *ctx) {
            Obj args(ObjType::Dictionary);
            return Call(ctx, &args);
        }

        template <typename... T>
        typename std::enable_if<is_all<Obj::PairType, T ...>::value, ObjPtr>::type
        inline operator()(Context *ctx, T ... args) {
            auto list = {args...};
            ObjPtr arg = Obj::CreateDict();
            for (auto &elem : list) {
                arg->push_back(elem);
            }
            return Call(ctx, arg.get());
        }

        inline ObjPtr Call(Context *ctx) {
            Obj args(ObjType::Dictionary);
            return Call(ctx, &args);
        }

        template <typename... T>
        typename std::enable_if<is_all<Obj::PairType, T ...>::value, ObjPtr>::type
        inline Call(Context *ctx, T ... args) {
            auto list = {args...};
            Obj arg(ObjType::Dictionary);
            for (auto &elem : list) {
                arg.Variable<Obj>::push_back(elem);
            }
            return Call(ctx, &arg);
        }

        ObjPtr Call(Context *ctx, Obj *args, bool direct = false, ObjPtr self = nullptr);

        /*
         * 
         * Интерфейс Variable
         * 
         */

        template <typename I>
        typename std::enable_if < std::is_integral<I>::value && !std::is_pointer<I>::value, const PairType &>::type
        inline operator[](const I index) {
            if (is_indexing() || m_var_type_current == ObjType::Range) {
                return at(index);
            }
            LOG_RUNTIME("Operator at for object type %s not implemented!", newlang::toString(m_var_type_current));
        }

        template <typename N>
        typename std::enable_if < std::is_same<N, std::string>::value || std::is_pointer<N>::value, const PairType &>::type
        inline operator[](const N name) {
            if (is_indexing() || m_var_type_current == ObjType::Range) {
                return at(name);
            }
            LOG_RUNTIME("Operator at for object type %s not implemented!", newlang::toString(m_var_type_current));
        }

        Obj::iterator find(const std::string name) {
            if (is_indexing() || m_var_type_current == ObjType::Range) {
                return Variable::find(name);
            }
            LOG_RUNTIME("Operator find for object type %s not implemented!", newlang::toString(m_var_type_current));
        }

        Obj::iterator begin() {
            if (is_indexing() || m_var_type_current == ObjType::Range) {
                return Variable::begin();
            }
            LOG_RUNTIME("Iterator for object type %s not implemented!", newlang::toString(m_var_type_current));
        }

        Obj::iterator end() {
            if (is_indexing() || m_var_type_current == ObjType::Range) {
                return Variable::end();
            }
            LOG_RUNTIME("Iterator for object type %s not implemented!", newlang::toString(m_var_type_current));
        }

        Obj::const_iterator begin() const {
            if (is_indexing() || m_var_type_current == ObjType::Range) {
                return Variable::begin();
            }
            LOG_RUNTIME("Iterator for object type %s not implemented!", newlang::toString(m_var_type_current));
        }

        Obj::const_iterator end() const {
            if (is_indexing() || m_var_type_current == ObjType::Range) {
                return Variable::end();
            }
            LOG_RUNTIME("Iterator for object type %s not implemented!", newlang::toString(m_var_type_current));
        }

        PairType & push_back(const PairType & p) {
            if (is_indexing()) {
                return Variable::push_back(p);
            }
            LOG_RUNTIME("Operator push_back for object type %s not implemented!", newlang::toString(m_var_type_current));
        }

        PairType & push_back(const Type value, const std::string &name = "") {
            if (is_indexing()) {
                return Variable::push_back(value, name);
            }
            LOG_RUNTIME("Operator push_back for object type %s not implemented!", newlang::toString(m_var_type_current));
        }

        Obj::iterator at_index(const int64_t index) {
            if (is_indexing()) {
                return Variable::at_index(index);
            }
            LOG_RUNTIME("Operator at_index for object type %s not implemented!", newlang::toString(m_var_type_current));
        }

        Obj::const_iterator at_index_const(const int64_t index) const {
            if (is_indexing()) {
                return Variable::at_index_const(index);
            }
            LOG_RUNTIME("Operator at_index_const for object type %s not implemented!", newlang::toString(m_var_type_current));
        }

        Obj::const_iterator insert(Obj::const_iterator pos, const PairType &data) {
            if (is_indexing()) {
                return Variable::insert(pos, data);
            }
            LOG_RUNTIME("Operator insert for object type %s not implemented!", newlang::toString(m_var_type_current));
        }

        const std::string & name(const int64_t index) const override {
            return Variable<Obj>::name(index);
        }

        int64_t resize(int64_t new_size, const Type fill, const std::string &name = "") override {
            if (is_indexing()) {
                return Variable<Obj>::resize(new_size, fill, name);
            }
            LOG_RUNTIME("Operator resize for object type %s not implemented!", newlang::toString(m_var_type_current));
        }

        void erase(const int64_t index) override {
            if (is_indexing()) {
                Variable<Obj>::erase(index);
                return;
            }
            LOG_RUNTIME("Operator erase(index) for object type %s not implemented!", newlang::toString(m_var_type_current));
        }

        void clear_() override {
            Variable::clear_();
            clear_(true);
        }

        void clear_(bool clear_iterator_name) {

            m_value.clear();
            m_string.clear();
            m_var_type_current = ObjType::None;

            m_class_parents.clear();
            m_var_is_init = false;
            m_tensor.reset();
            m_rational.set_(0);
            m_var = at::monostate();
            //        m_value.reset(); //????????????????
            //        m_items.clear();
        }


        /*
         * 
         * 
         */

        //унарный плюс ничего не делает.

        ObjPtr operator+() {
            if (is_arithmetic_type()) {
                return shared();
            }
            LOG_RUNTIME("Unary plus for object '%s' not supported!", toString().c_str());
        }

        ObjPtr operator-() {
            if (is_arithmetic_type()) {
                if (is_tensor_type()) {
                    m_tensor = -m_tensor;
                } else if (is_integer()) {
                    SetValue_(-GetValueAsInteger());
                } else if (isFloatingType(m_var_type_current)) {
                    SetValue_(-GetValueAsNumber());
                } else {
                    LOG_RUNTIME("Unary minus for object '%s' not supported!", toString().c_str());
                }
                return shared();
            }
            LOG_RUNTIME("Unary minus for object '%s' not supported!", toString().c_str());
        }

        ObjPtr & operator+(ObjPtr & obj) {

            if (is_tensor_type()) {
                return obj;
            }
            LOG_RUNTIME("Object '%s' not numeric!", obj->toString().c_str());
        }

        ObjPtr &operator-(ObjPtr & obj) {
            if (is_tensor_type()) {
                obj->m_tensor = torch::zeros_like(obj->m_tensor) - obj->m_tensor;
                return obj;
            }
            LOG_RUNTIME("Object '%s' not numeric!", obj->toString().c_str());
        }


        //префиксная версия возвращает значение после инкремента

        ObjPtr operator++() {
            if (is_tensor_type()) {
                m_tensor.add_(torch::ones_like(m_tensor));

                return shared();
            }
            LOG_RUNTIME("Object '%s' not numeric!", toString().c_str());
        }

        //постфиксная версия возвращает значение до инкремента

        ObjPtr operator++(int) {
            ObjPtr old = Clone();
            Obj::operator++();
            return old;
        }

        //префиксная версия возвращает значение после декремента

        ObjPtr operator--() {
            if (is_tensor_type()) {
                m_tensor.sub_(torch::ones_like(m_tensor));
                return shared();
            }
            LOG_RUNTIME("Object '%s' not numeric!", toString().c_str());
        }

        //постфиксная версия возвращает значение до декремента

        ObjPtr operator--(int) {
            ObjPtr old = Clone();
            Obj::operator--();
            return old;
        }

        inline ObjPtr operator*(ObjPtr obj) {
            ASSERT(obj);
            return operator*(*obj);
        }

        inline ObjPtr operator*(Obj value) {
            ObjPtr result = Clone();
            *result *= value;
            return result;
        }

        inline ObjPtr operator/(ObjPtr obj) {
            ASSERT(obj);
            return operator/(*obj);
        }

        inline ObjPtr operator/(Obj value) {
            ObjPtr result = Clone();
            *result /= value;
            return result;
        }

        inline ObjPtr op_div_ceil(ObjPtr obj) {
            ASSERT(obj);
            return op_div_ceil(*obj);
        }

        inline ObjPtr op_div_ceil(Obj value) {
            ObjPtr result = Clone();
            result->op_div_ceil_(value);
            return result;
        }

        inline ObjPtr op_concat(ObjPtr obj, ConcatMode mode = ConcatMode::Error) {
            ASSERT(obj);
            return op_concat(*obj, mode);
        }

        inline ObjPtr op_concat(Obj &value, ConcatMode mode = ConcatMode::Error) {
            ObjPtr result = Clone();
            result->op_concat_(value, mode);
            return result;
        }

        inline ObjPtr op_concat_(Obj &obj, ConcatMode mode = ConcatMode::Error) {
            ConcatData(this, obj, mode);
            return shared();
        }

        inline ObjPtr op_concat_(ObjPtr obj, ConcatMode mode = ConcatMode::Error) {
            return op_concat_(*obj, mode);
        }

        inline ObjPtr operator%(ObjPtr obj) {
            ASSERT(obj);
            return operator%(*obj);
        }

        inline ObjPtr operator%(Obj value) {
            ObjPtr result = Clone();
            *result %= value;
            return result;
        }

        inline ObjPtr operator+(ObjPtr obj) {
            ASSERT(obj);
            return operator+(*obj);
        }

        inline ObjPtr operator+(Obj value) {
            ObjPtr result = Clone();
            *result += value;
            return result;
        }

        inline ObjPtr operator-(ObjPtr obj) {
            ASSERT(obj);
            return operator-(*obj);
        }

        inline ObjPtr operator-(Obj value) {
            ObjPtr result = Clone();
            *result -= value;
            return result;
        }

        //    ObjPtr op_lshift(ObjPtr obj) {
        //        LOG_RUNTIME("Operator '<<' not implementd!");
        //    }
        //
        //    ObjPtr op_rshift(ObjPtr obj) {
        //        LOG_RUNTIME("Operator '>>' not implementd!");
        //    }
        //
        //    ObjPtr op_rrshift(ObjPtr obj) {
        //        LOG_RUNTIME("Operator '>>>' not implementd!");
        //    }

        inline bool operator<(ObjPtr obj) {
            ASSERT(obj);
            return operator<(*obj);
        }

        inline bool operator<(Obj obj) {
            return op_compare(obj) < 0;
        }

        inline bool operator<=(ObjPtr obj) {
            ASSERT(obj);
            return operator<=(*obj);
        }

        inline bool operator<=(Obj obj) {
            return op_compare(obj) <= 0;
        }

        inline bool operator>(ObjPtr obj) {
            ASSERT(obj);
            return operator>(*obj);
        }

        inline bool operator>(Obj obj) {
            return op_compare(obj) > 0;
        }

        inline bool operator>=(ObjPtr obj) {
            ASSERT(obj);
            return operator>=(*obj);
        }

        inline bool operator>=(Obj obj) {
            return op_compare(obj) >= 0;
        }
        int op_compare(Obj & value);

        /*
         *  instanceof (проверка класса объекта) test_obj ~~ obj  объекты совместимы по свойствим и их типам (утиная типизация)
         *  in (проверка существования свойства)  test_obj ~~ (prop=,) объект содержит указанные свойства (т.к. пустой тип совместим с любым типом)
         *  
         * Реализация модели наследования:
         * ~ - проверка по классу объекта, т.е. проверка имени объекта или имени базового типа (объект должен быть наследником образца)
         * Реализация утиной типизации:
         * ~~ - сравнение с образцом на совместимость типов (пустой тип совместим с любым)
         * ~~~ - сравнение с образцом на равенство типов, т.е. типы объекта должны быть идентичны образцу, включая пустой
         * Для двух последних случаев сравнение производится только для одного уровня (без раскрытия сложенных словарей, если они присутствуют в образце)
         * или сравнение должно зависить от того, есть ли в образце вложенные словари (классы) ??????????????
         */

        bool op_class_test(ObjPtr obj, Context *ctx) const;
        bool op_class_test(const char * name, Context *ctx) const;

        inline bool op_duck_test(ObjPtr obj, bool strong) {
            ASSERT(obj);
            return op_duck_test(obj.get(), strong);
        }
        bool op_duck_test(Obj *value, bool strong);
        static bool op_duck_test_prop(Obj *base, Obj *value, bool strong);

        inline bool op_equal(ObjPtr value) {
            ASSERT(value);
            if (value) {
                return op_equal(*value);
            }
            return false;
        }

        bool op_equal(Obj & value);

        inline bool op_accurate(ObjPtr obj) {
            ASSERT(obj);
            return op_accurate(*obj);
        }
        bool op_accurate(Obj & value);

        inline ObjPtr op_bit_and(ObjPtr obj, bool strong) {
            ASSERT(obj);
            return op_bit_and(*obj, strong);
        }

        inline ObjPtr op_bit_and(Obj &obj, bool strong) {
            ObjPtr result = Clone();
            result->op_bit_and_set(obj, strong);
            return result;
        }

        inline ObjPtr op_pow(ObjPtr obj) const {
            ASSERT(obj);
            return op_pow(*obj);
        }

        inline ObjPtr op_pow(Obj &obj) const {
            ObjPtr result = Clone();
            result->op_pow_(obj);
            return result;
        }

        inline ObjPtr op_pow_(ObjPtr obj) {
            ASSERT(obj);
            return op_pow_(*obj);
        }

        ObjPtr op_pow_(Obj &obj);

        //    ObjPtr operator^(ObjPtr obj) {
        //        LOG_RUNTIME("Operator '^' not implementd!");
        //    }
        //
        //    ObjPtr operator|(ObjPtr obj) {
        //        LOG_RUNTIME("Operator '|' not implementd!");
        //    }
        //
        //    ObjPtr operator&&(ObjPtr obj) {
        //        LOG_RUNTIME("Operator '&&' not implementd!");
        //    }
        //
        //    ObjPtr operator||(ObjPtr obj) {
        //        LOG_RUNTIME("Operator '||' not implementd!");
        //    }

        ObjPtr op_assign(ObjPtr obj) {
            if (obj) {
                return op_assign(*obj);
            }
            clear_();
            return shared();
        }

        ObjPtr op_assign(Obj & obj) {
            obj.CloneDataTo(*this);
            obj.ClonePropTo(*this);
            return shared();
        }

        inline bool operator=(Obj & obj) {
            obj.CloneDataTo(*this);
            return true;
        }

        /*    
         * ?: (выбор из двух операндов)
         * = (присваивание)
         * *=, /=, %=, +=, -=, &=, ^=, |=, <<=, >>=, >>>= (операции с присваиванием)
         * , (отбрасывание первого и возврат второго операнда) 
         */


        inline ObjPtr operator*=(ObjPtr obj) {
            if (!obj) {
                ASSERT(obj);
            }
            return operator*=(*obj);
        }

        ObjPtr operator*=(Obj obj);

        inline ObjPtr operator/=(ObjPtr obj) {
            ASSERT(obj);
            return operator/=(*obj);
        }

        ObjPtr operator/=(Obj obj);

        inline ObjPtr op_div_ceil_(ObjPtr obj) {
            ASSERT(obj);
            return op_div_ceil_(*obj);
        }

        ObjPtr op_div_ceil_(Obj &obj);

        inline ObjPtr operator%=(ObjPtr obj) {
            ASSERT(obj);
            return operator%=(*obj);
        }

        ObjPtr operator%=(Obj obj);

        inline ObjPtr operator+=(ObjPtr obj) {
            ASSERT(obj);
            return operator+=(*obj);
        }
        ObjPtr operator+=(Obj obj);

        inline ObjPtr operator-=(ObjPtr obj) {
            ASSERT(obj);
            return operator-=(*obj);
        }
        ObjPtr operator-=(Obj obj);


        ObjPtr op_bit_and_set(Obj &obj, bool strong);

        inline ObjPtr operator^=(ObjPtr obj) {
            ASSERT(obj);
            return operator^=(*obj);
        }

        ObjPtr operator^=(Obj obj) {
            LOG_RUNTIME("Operator '^=' not implementd!");
        }

        inline ObjPtr operator|=(ObjPtr obj) {
            ASSERT(obj);
            return operator|=(*obj);
        }

        ObjPtr operator|=(Obj obj) {
            LOG_RUNTIME("Operator '|=' not implementd!");
        }

        inline ObjPtr op_lshift_set(ObjPtr obj) {
            ASSERT(obj);
            return op_lshift_set(*obj);
        }

        ObjPtr op_lshift_set(Obj obj) {
            LOG_RUNTIME("Operator '<<=' not implementd!");
        }

        inline ObjPtr op_rshift_set(ObjPtr obj) {
            ASSERT(obj);
            return op_rshift_set(*obj);
        }

        ObjPtr op_rshift_set(Obj obj) {
            LOG_RUNTIME("Operator '>>=' not implementd!");
        }

        inline ObjPtr op_rrshift_set(ObjPtr obj) {
            ASSERT(obj);
            return op_rrshift_set(*obj);
        }

        ObjPtr op_rrshift_set(Obj obj) {
            LOG_RUNTIME("Operator '>>>=' not implementd!");
        }

        template <typename T>
        bool operator=(T value) {
            SetValue_(value);
            return true;
        }


        static ObjPtr GetIndex(ObjPtr obj, TermPtr index_arg);

        static ObjPtr GetIndex(ObjPtr obj, size_t index) {
            return (*obj)[index].second;
        }

        void dump_dict_(std::string &str, bool deep = true) const {
            bool first = true;
            for (auto &elem : * this) {
                if (first) {
                    first = false;
                } else {
                    str.append(", ");
                }
                if (elem.first.empty()) {
                    if (elem.second) {
                        str.append(elem.second->toString(false));
                    } else {
                        str.append("_");
                    }
                } else {
                    if (elem.second) {

                        if (deep || !(elem.second->is_tensor_type() || elem.second->getType() == ObjType::Class)) {
                            str.append(elem.first);
                            str.append("=");
                            str.append(elem.second->toString(false));
                        } else if (elem.second->getType() == ObjType::Class && elem.second->m_is_reference) {
                            str.append("&");
                            str.append(elem.second->getName());
                        } else {

                            str.append(elem.second->getName());
                            str.append("=");
                            str.append(elem.second->toString());
                        }
                    } else {
                        str.append("_");
                    }
                }
            }
        }

        std::string toString(bool deep = true) const;

        std::string GetValueAsString() const;

        inline std::wstring GetValueAsStringWide() const {
            return utf8_decode(GetValueAsString());
        }

        std::shared_ptr<Rational> GetValueAsRational() const {
            TEST_INIT_();
            if (m_var_type_current == ObjType::Rational) {
                return m_rational.clone();
            } else if (is_integral()) {
                return std::make_shared<Rational>(GetValueAsInteger());
            } else if (is_floating()) {
                ASSERT("Not implemented!");
            }
            LOG_RUNTIME("Value %s incompatible to Rational or convert not implemented!", toString().c_str());
        }

        int64_t GetValueAsInteger() const {
            TEST_INIT_();

            switch (m_var_type_current) {
                case ObjType::Bool:
                    if (at::holds_alternative<int64_t>(m_var)) {
                        return at::get<int64_t>(m_var);
                    } else if (at::holds_alternative<bool *>(m_var)) {
                        return *at::get<bool *>(m_var);
                    }
                case ObjType::Int8:
                case ObjType::Char:
                case ObjType::Byte:
                    if (at::holds_alternative<int64_t>(m_var)) {
                        return at::get<int64_t>(m_var);
                    } else if (at::holds_alternative<int8_t *>(m_var)) {
                        return *at::get<int8_t *>(m_var);
                    }
                case ObjType::Int16:
                case ObjType::Word:
                    if (at::holds_alternative<int64_t>(m_var)) {
                        return at::get<int64_t>(m_var);
                    } else if (at::holds_alternative<int16_t *>(m_var)) {
                        return *at::get<int16_t *>(m_var);
                    }
                case ObjType::Int32:
                case ObjType::DWord:
                    if (at::holds_alternative<int64_t>(m_var)) {
                        return at::get<int64_t>(m_var);
                    } else if (at::holds_alternative<int32_t *>(m_var)) {
                        return *at::get<int32_t *>(m_var);
                    }
                case ObjType::Int64:
                case ObjType::DWord64:
                    if (at::holds_alternative<int64_t>(m_var)) {
                        return at::get<int64_t>(m_var);
                    } else if (at::holds_alternative<int64_t *>(m_var)) {
                        return *at::get<int64_t *>(m_var);
                    }
                case ObjType::Integer:
                    if (at::holds_alternative<int64_t>(m_var)) {
                        return at::get<int64_t>(m_var);
                    }
                    ASSERT(!is_scalar());
                    LOG_RUNTIME("Can`t convert tensor to scalar!");

                case ObjType::Float16:
                case ObjType::Float32:
                case ObjType::Float64:
                case ObjType::Single:
                case ObjType::Double:
                case ObjType::Number:
                    return static_cast<int64_t> (GetValueAsNumber());

                case ObjType::Rational:
                    return m_rational.GetAsInteger();

                case ObjType::StrWide:
                case ObjType::FmtWide:
                    if (m_string.size() == 1) {
                        return m_string[0];
                    }
                case ObjType::StrChar:
                case ObjType::FmtChar:
                    if (m_value.size() == 1) {
                        return m_value[0];
                    }

                case ObjType::Iterator:
                    ASSERT(m_iterator);
                    return m_iterator->data().second->GetValueAsInteger();

                default:
                    if (m_var_type_current == ObjType::Pointer || m_var_type_fixed == ObjType::Pointer) {
                        ASSERT(at::holds_alternative<void *>(m_var));
                        return reinterpret_cast<int64_t> (at::get<void *>(m_var));
                    }
            }
            LOG_RUNTIME("Data type incompatible %s", toString().c_str());
        }

        inline double GetValueAsNumber() const {
            TEST_INIT_();

            switch (m_var_type_current) {

                case ObjType::Single:
                case ObjType::Float16:
                case ObjType::Float32:
                    if (at::holds_alternative<double>(m_var)) {
                        return at::get<double>(m_var);
                    } else if (at::holds_alternative<float *>(m_var)) {
                        return *at::get<float *>(m_var);
                    }
                case ObjType::Double:
                case ObjType::Float64:
                    if (at::holds_alternative<double>(m_var)) {
                        return at::get<double>(m_var);
                    } else if (at::holds_alternative<double *>(m_var)) {
                        return *at::get<double *>(m_var);
                    }
                case ObjType::Number:
                    if (at::holds_alternative<double>(m_var)) {
                        return at::get<double>(m_var);
                    }
                    if (is_scalar()) {
                        ASSERT(!is_scalar());
                    }
                    LOG_RUNTIME("Can`t convert tensor to scalar!");

                case ObjType::Rational:
                    return m_rational.GetAsNumber();

                case ObjType::Iterator:
                    ASSERT(m_iterator);
                    return m_iterator->data().second->GetValueAsNumber();

                default:
                    if (is_simple_type() || is_string_type()) {
                        return static_cast<double> (GetValueAsInteger());
                    }
            }
            LOG_RUNTIME("Data type incompatible %s", toString().c_str());
        }

        inline bool GetValueAsBoolean() const {
            if (!m_var_is_init || m_var_type_current == ObjType::IteratorEnd) {
                return false;
            }
            if (is_scalar()) {
                return GetValueAsInteger();
                //return m_tensor.toType(at::ScalarType::Bool).item<double>();
            } else if (isSimpleType(m_var_type_current)) {
                // Error: Boolean value of Tensor with more than one value is ambiguous
                return !at::_is_zerotensor(m_tensor);
            } else {
                switch (m_var_type_current) {
                    case ObjType::StrWide:
                        return !m_string.empty();
                    case ObjType::StrChar:
                        return !m_value.empty();
                    case ObjType::None:
                        return false;

                    case ObjType::Rational:
                        return m_rational.GetAsBoolean();

                    case ObjType::Dictionary:
                    case ObjType::Class:
                        if (size()) {
                            return true;
                        }
                        for (auto &elem : m_class_parents) {
                            if (elem->GetValueAsBoolean()) {
                                return true;
                            }
                        }
                        return false;

                    case ObjType::Iterator:
                        ASSERT(m_iterator);
                        ASSERT(m_iterator->m_iter_obj);
                        if (m_iterator->m_iter_obj->getType() == ObjType::Range) {
                            if (m_iterator->m_iter_obj->m_iter_range_value && m_iterator->m_iter_obj->m_iter_range_value->m_var_type_current != ObjType::IteratorEnd) {
                                return true;
                            }
                        } else {
                            if ((*m_iterator) != m_iterator->end()) {
                                return m_iterator->data().second->GetValueAsBoolean();
                            }
                        }
                        return false;

                    default:
                        LOG_RUNTIME("Type cast to bool %s", toString().c_str());
                }
                return true;
            }
        }

        LLVMGenericValueRef GetGenericValueRef(LLVMTypeRef type) {
            if (type == LLVMInt1Type() || type == LLVMInt8Type() || type == LLVMInt16Type() || type == LLVMInt32Type() || type == LLVMInt64Type()) {
                return LLVMCreateGenericValueOfInt(type, GetValueAsInteger(), true);
            } else if (type == LLVMFloatType() || type == LLVMDoubleType()) {
                return LLVMCreateGenericValueOfFloat(type, GetValueAsNumber());
            } else if (type == LLVMPointerType(LLVMInt8Type(), 0)) {
                if (getType() == ObjType::StrChar || getType() == ObjType::FmtChar) {
                    return LLVMCreateGenericValueOfPointer((void *) m_value.c_str());
                } else if (getType() == ObjType::Pointer) {
                    ASSERT(at::holds_alternative<void *>(m_var));
                    return LLVMCreateGenericValueOfPointer(at::get<void *>(m_var));
                }
            } else if (type == LLVMPointerType(LLVMInt32Type(), 0)) {
                if (getType() == ObjType::StrWide || getType() == ObjType::FmtWide) {

                    return LLVMCreateGenericValueOfPointer((void *) m_string.c_str());
                }
            }
            LOG_RUNTIME("Not support LLVM type '%s'", newlang::toString(m_var_type_current));
        }

        static ObjPtr CreateFromGenericValue(ObjType type, LLVMGenericValueRef ref, LLVMTypeRef llvm_type) {
            if (type == ObjType::None) {
                return Obj::CreateNone();
            } else if (isIntegralType(type, true)) {
                int64_t value = LLVMGenericValueToInt(ref, true);
                unsigned width = LLVMGenericValueIntWidth(ref);
                return Obj::CreateValue(value, type);
            } else if (isFloatingType(type)) {
                return Obj::CreateValue((double) LLVMGenericValueToFloat(llvm_type, ref), type);
            } else if (type == ObjType::StrChar || type == ObjType::FmtChar) {
                if (llvm_type == LLVMInt8Type()) {
                    return Obj::CreateString(std::string(1, (char) LLVMGenericValueToInt(ref, false)));
                } else {
                    //@todo Нужна проверка на тип LLVM данных?
                    return Obj::CreateString(std::string((const char *) LLVMGenericValueToPointer(ref)));
                }
            } else if (type == ObjType::StrWide || type == ObjType::FmtWide) {
                if (llvm_type == LLVMInt32Type()) {
                    return Obj::CreateString(std::wstring(1, (wchar_t) LLVMGenericValueToInt(ref, false)));
                } else {
                    //@todo Нужна проверка на тип LLVM данных?
                    return Obj::CreateString(std::wstring((const wchar_t *)LLVMGenericValueToPointer(ref)));
                }
            } else if (type == ObjType::Pointer) {

                ObjPtr result = Obj::CreateType(type, ObjType::None, true);
                result->m_var = LLVMGenericValueToPointer(ref);

                return result;
            }
            LOG_RUNTIME("Create to type '%s' form LLVM type not implemented!", newlang::toString(type));
        }

        static ObjPtr CreateType(ObjType type, ObjType fixed = ObjType::None, bool is_init = false) {

            return std::make_shared<Obj>(type, nullptr, nullptr, fixed, is_init);
        }

        static ObjPtr CreateRational(const std::string val) {

            std::string str;
            std::remove_copy(val.begin(), val.end(), back_inserter(str), '_');

            if (str.empty()) {
                LOG_RUNTIME("Empty string!");
            }

            ObjPtr obj = Obj::CreateType(ObjType::Rational);

            // Ищем разделитель дроби
            size_t pos = str.find("\\");

            // Если символ не найден - то вся строка является числом 
            if (pos == std::string::npos) {
                obj->m_rational.set_(0);
            } else {
                // Числитель - левая часть
                // Знаменатель - правая часть
                obj->m_rational.set_(str.substr(0, pos), str.substr(pos + 1, str.length()));
                // Знаменатель не должен быть равен нулю
                if (obj->m_rational.m_denominator.isZero()) {
                    LOG_RUNTIME("Denominator must be different from zero!");
                }
            }

            obj->m_var_is_init = true;
            if (str.compare(obj->m_rational.GetAsString().c_str()) != 0) {

                LOG_RUNTIME("Rational value '%s' does not match source string  '%s'!", obj->m_rational.GetAsString().c_str(), str.c_str());
            }

            return obj;
        }


        // чистая функция
        static ObjPtr BaseTypeConstructor(const Context *ctx, Obj &in);
        static ObjPtr ConstructorSimpleType_(const Context *ctx, Obj & args);
        static ObjPtr ConstructorDictionary_(const Context *ctx, Obj & args);
        static ObjPtr ConstructorNative_(const Context *ctx, Obj & args);
        static ObjPtr ConstructorStub_(const Context *ctx, Obj & args);
        static ObjPtr ConstructorClass_(const Context *ctx, Obj & args);
        static ObjPtr ConstructorStruct_(const Context *ctx, Obj & args);
        static ObjPtr ConstructorEnum_(const Context *ctx, Obj & args);

        static ObjPtr ConstructorError_(const Context *ctx, Obj & args);
        static ObjPtr ConstructorReturn_(const Context *ctx, Obj & args);
        static ObjPtr ConstructorInterraption_(const Context *ctx, Obj & args, ObjType type);

        static ObjPtr CreateBaseType(ObjType type);

        static ObjPtr CreateNone() {

            return CreateType(ObjType::None, ObjType::None, true);
        }

        template <typename T1, typename T2, typename T3>
        static ObjPtr CreateRange(T1 start, T2 stop, T3 step) {
            ObjPtr obj = CreateType(ObjType::Dictionary, ObjType::Range, true);
            obj->push_back(CreateValue(start, ObjType::None), "start");
            obj->push_back(CreateValue(stop, ObjType::None), "stop");
            obj->push_back(CreateValue(step, ObjType::None), "step");
            obj->m_var_type_current = ObjType::Range;
            return obj;
        }

        template <typename T1, typename T2>
        static ObjPtr CreateRange(T1 start, T2 stop) {
            ObjPtr obj = CreateType(ObjType::Dictionary, ObjType::Range, true);
            obj->push_back(CreateValue(start, ObjType::None), "start");
            obj->push_back(CreateValue(stop, ObjType::None), "stop");
            if (start < stop) {
                obj->push_back(CreateValue(1, ObjType::None), "step");
            } else {
                obj->push_back(CreateValue(-1, ObjType::None), "step");
            }
            obj->m_var_type_current = ObjType::Range;

            return obj;
        }

        static ObjType GetType(torch::Tensor & val) {
            switch (val.dtype().toScalarType()) {
                case at::ScalarType::Bool:
                    return ObjType::Bool;
                case at::ScalarType::Half:
                case at::ScalarType::BFloat16:
                    return ObjType::Float16;
                case at::ScalarType::Float:
                    return ObjType::Float32;
                case at::ScalarType::Double:
                    return ObjType::Float64;
                case at::ScalarType::Byte:
                case at::ScalarType::Char:
                case at::ScalarType::QInt8:
                case at::ScalarType::QUInt8:
                case at::ScalarType::QUInt4x2:
                    return ObjType::Int8;
                case at::ScalarType::Short:
                    return ObjType::Int16;
                case at::ScalarType::Int:
                case at::ScalarType::QInt32:
                    return ObjType::Int32;
                case at::ScalarType::Long:
                    return ObjType::Int64;
                case at::ScalarType::ComplexHalf:
                    return ObjType::Complex16;
                case at::ScalarType::ComplexFloat:
                    return ObjType::Complex32;
                case at::ScalarType::ComplexDouble:
                    return ObjType::Complex64;
            }
            LOG_RUNTIME("Fail tensor type %s", val.toString().c_str());
        }

        static ObjPtr CreateBool(bool value) {
            ObjPtr result = CreateType(ObjType::Bool, ObjType::None, true);
            result->SetValue_(value);

            return result;
        }

        static ObjPtr CreateTensor(torch::Tensor tensor) {
            ObjType check_type = GetType(tensor);
            if (!isTensor(check_type)) {
                LOG_RUNTIME("Unsupport torch type %s (%d)!", at::toString(tensor.dtype().toScalarType()), (int) tensor.dtype().toScalarType());
            }
            ObjPtr result;
            if (tensor.dim() == 0) {
                if (tensor.is_floating_point()) {
                    result = Obj::CreateValue(tensor.item<double>(), check_type);
                } else {
                    ASSERT(!tensor.is_complex());
                    result = Obj::CreateValue(tensor.item<int64_t>(), check_type);
                }
            } else {

                result = CreateType(check_type);
                result->m_tensor = tensor;
                result->m_var_is_init = true;
            }
            return result;
        }

        at::indexing::Slice toSlice() {
            NL_CHECK(is_range(), "Convert to slice supported for range only!");

            ASSERT(size() == 3 && Variable<Obj>::at("start").second && Variable<Obj>::at("stop").second && Variable<Obj>::at("step").second);

            NL_CHECK(Variable<Obj>::at("start").second->is_integer(), "Slice value start support integer type only!");
            NL_CHECK(Variable<Obj>::at("stop").second->is_integer(), "Slice value stop support integer type only!");
            NL_CHECK(Variable<Obj>::at("step").second->is_integer(), "Slice value step support integer type only!");

            return at::indexing::Slice(
                    Variable<Obj>::at("start").second->GetValueAsInteger(),
                    Variable<Obj>::at("stop").second->GetValueAsInteger(),
                    Variable<Obj>::at("step").second->GetValueAsInteger());
        }

        /*
         * From TensorIndexing.h
    // There is one-to-one correspondence between Python and C++ tensor index types:
    // Python                  | C++
    // -----------------------------------------------------
    // `None`                  | `at::indexing::None`
    // `Ellipsis`              | `at::indexing::Ellipsis`
    // `...`                   | `"..."`
    // `123`                   | `123`
    // `True` / `False`        | `true` / `false`
    // `:`                     | `Slice()` / `Slice(None, None)`
    // `::`                    | `Slice()` / `Slice(None, None, None)`
    // `1:`                    | `Slice(1, None)`
    // `1::`                   | `Slice(1, None, None)`
    // `:3`                    | `Slice(None, 3)`
    // `:3:`                   | `Slice(None, 3, None)`
    // `::2`                   | `Slice(None, None, 2)`
    // `1:3`                   | `Slice(1, 3)`
    // `1::2`                  | `Slice(1, None, 2)`
    // `:3:2`                  | `Slice(None, 3, 2)`
    // `1:3:2`                 | `Slice(1, 3, 2)`
    // `torch.tensor([1, 2])`) | `torch::tensor({1, 2})`
         */
        Index toIndex() {
            if (is_none_type()) {
                return Index(c10::nullopt);
            } else if (is_dictionary_type()) {
                std::vector<int64_t> temp = toIntVector(true);
                if (temp.size()) {
                    torch::Tensor tensor = torch::from_blob(temp.data(), temp.size(), torch::Dtype::Long);
                    return Index(tensor.clone());
                } else {
                    return Index(c10::nullopt);
                }

            } else if (is_scalar()) {
                switch (m_var_type_current) {
                    case ObjType::Bool:
                        return Index(GetValueAsBoolean());

                    case ObjType::Int8:
                    case ObjType::Int16:
                    case ObjType::Int32:
                    case ObjType::Int64:
                        return Index(GetValueAsInteger());
                    default:
                        LOG_RUNTIME("Fail convert scalar type '%s' to Index!", newlang::toString(m_var_type_current));
                }
            } else if (is_tensor_type()) {
                return Index(m_tensor);
            } else if (is_ellipsis()) {
                return Index(at::indexing::Ellipsis);
            } else if (is_range()) {

                return Index(toSlice());
            }
            LOG_RUNTIME("Fail convert object '%s' to Index!", toString().c_str());
        }

        inline operator bool () const {

            return GetValueAsBoolean();
        }

        std::vector<int64_t> toIntVector(bool raise = true) const {
            std::vector<int64_t> result;
            for (int i = 0; i < size(); i++) {
                if (raise && !at(i).second->is_integer()) {

                    LOG_RUNTIME("Item does not contain an integer value! '%s'", at(i).second->GetValueAsString().c_str());
                }
                result.push_back(at(i).second->GetValueAsInteger());
            }
            return result;
        }

        template <typename T>
        typename std::enable_if<std::is_integral<T>::value, ObjPtr>::type
        static CreateValue(T value, ObjType fix_type = ObjType::None) {
            ObjPtr result = CreateType(fix_type);
            result->m_var_type_fixed = fix_type;
            result->m_var_type_current = typeFromLimit((int64_t) value);
            if (fix_type != ObjType::None) {
                NL_CHECK(canCast(result->m_var_type_current, fix_type),
                        "Fail cast type from '%s' to '%s'!", newlang::toString(result->m_var_type_current), newlang::toString(fix_type));
            }
            result->m_var = static_cast<int64_t> (value);
            // result->m_tensor = torch::scalar_tensor(value, toTorchType(result->m_var_type_current));
            result->m_var_is_init = true;

            return result;
        }

        template <typename T>
        typename std::enable_if<std::is_floating_point<T>::value, ObjPtr>::type
        static CreateValue(T value, ObjType fix_type = ObjType::None) {
            ObjPtr result = CreateType(fix_type);
            result->m_var_type_fixed = fix_type;
            result->m_var_type_current = typeFromLimit((double) value);
            if (fix_type != ObjType::None) {
                NL_CHECK(canCast(result->m_var_type_current, fix_type),
                        "Fail cast type from '%s' to '%s'!", newlang::toString(result->m_var_type_current), newlang::toString(fix_type));
            }
            result->m_var = static_cast<double> (value);
            //result->m_tensor = torch::scalar_tensor(value, toTorchType(result->m_var_type_current));
            result->m_var_is_init = true;

            return result;
        }

        static ObjPtr CreateString(const std::string str) {
            ObjPtr result = CreateType(ObjType::StrChar, ObjType::String, true);
            result->m_value = str;
            return result;
        }

        static ObjPtr CreateString(const std::wstring str) {
            ObjPtr result = CreateType(ObjType::StrWide, ObjType::String, true);
            result->m_string = str;
            return result;
        }

        inline static ObjPtr Yes() {
            ObjPtr result = std::make_shared<Obj>(ObjType::Bool);
            result->m_var = static_cast<int64_t> (1);
            result->m_var_is_init = true;

            return result->MakeConst();
        }

        inline static ObjPtr No() {
            ObjPtr result = std::make_shared<Obj>(ObjType::Bool);
            result->m_var = static_cast<int64_t> (0);
            result->m_var_is_init = true;

            return result->MakeConst();
        }

        inline static ObjPtr CreateDict() {
            return Obj::CreateType(ObjType::Dictionary);
        }

        template <typename... T>
        typename std::enable_if<is_all<PairType, T ...>::value, ObjPtr>::type
        static CreateDict(T ... args) {
            std::array < PairType, sizeof...(args) > list = {args...};
            ObjPtr result = Obj::CreateType(ObjType::Dictionary);
            for (auto &elem : list) {
                result->push_back(elem);
            }
            result->m_var_is_init = true;

            return result;
        }

        inline static ObjPtr CreateDict(const std::vector<ObjPtr> & arr) {
            ObjPtr result = Obj::CreateType(ObjType::Dictionary, ObjType::None, true);
            for (int i = 0; i < arr.size(); i++) {
                result->push_back(arr[i]);
            }
            return result;
        }

        /*
         * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
         * Различие между классом и словарям в том, что элементы словаря могут добавлятся и удаляться динамически,
         * а у класса состав полей фиуксируется при определении и в последствии они не могут быть добалвены или удалены.
         * >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
         *          
         */
        inline static ObjPtr CreateClass(std::string name) {
            ObjPtr result = Obj::CreateType(ObjType::Class);
            result->m_class_name = name;
            result->m_var_is_init = true;

            return result;
        }

        template <typename... T>
        typename std::enable_if<is_all<PairType, T ...>::value, ObjPtr>::type
        static CreateClass(std::string name, T ... args) {
            std::array < PairType, sizeof...(args) > list = {args...};
            ObjPtr result = Obj::CreateType(ObjType::Class);
            result->m_class_name = name;
            for (auto &elem : list) {
                result->push_back(elem);
            }
            result->m_var_is_init = true;

            return result;
        }


        ObjPtr CallNative(Context *ctx, Obj args);

        ObjPtr Clone(const char *new_name = nullptr) const {
            ObjPtr clone = Obj::CreateNone();
            CloneDataTo(*clone);
            ClonePropTo(*clone);
            if (new_name) {

                clone->m_var_name = new_name;
            }
            return clone;
        }

        inline void CloneTo(Obj & clone) {
            if (&clone == this) {
                // Не клонировать сам в себя

                return;
            }
            CloneDataTo(clone);
            ClonePropTo(clone);
        }

        inline void CloneTo(ObjPtr & clone) {
            if (clone.get() == this) {
                // Не клонировать сам в себя

                return;
            }
            clone.reset();
            clone = Obj::CreateNone();
            CloneDataTo(*clone);
            ClonePropTo(*clone);
        }

        void CloneDataTo(Obj & clone) const;
        void ClonePropTo(Obj & clone) const;

        inline ObjPtr toType(ObjType type) const {
            ObjPtr clone = Clone();
            clone->toType_(type);

            return clone;
        }

        /*
         * Варианты преобраования типов
         * Range -> Dict
         * Tensor -> Dict
         * Dict -> Tensor
         * String -> Tensor
         * Tensor -> String
         */
        void toType_(ObjType type);

        virtual ~Obj() {

            clear_();
        }


        const ObjPtr index_get(const std::vector<Index> & index) const;

        inline ObjPtr index_set(const std::vector<Index> & index, const ObjPtr value) const {
            ObjPtr result = Clone();
            result->index_set_(index, value);

            return result;
        }
        ObjPtr index_set_(const std::vector<Index> & index, const ObjPtr value);

        inline ObjPtr op_set_index(ObjPtr index, ObjPtr value) {

            return op_set_index(index->GetValueAsInteger(), value->GetValueAsString());
        }
        ObjPtr op_set_index(int64_t index, std::string value);

        template < typename T>
        typename std::enable_if<std::is_same<T, bool>::value, void>::type
        SetValue_(bool value) {

            SetValue_(static_cast<int64_t> (value));
        }

        template < typename T>
        typename std::enable_if<std::is_integral<T>::value, void>::type
        SetValue_(T value) {

            TEST_CONST_();
            ASSERT(m_var_type_current != ObjType::Class);
            clear_();
            m_var = static_cast<int64_t> (value);
            m_var_type_current = typeFromLimit(static_cast<int64_t> (value));
            m_var_is_init = true;
        }

        template < typename T>
        typename std::enable_if<std::is_floating_point<T>::value, void>::type
        SetValue_(T value) {

            TEST_CONST_();
            ASSERT(m_var_type_current != ObjType::Class);
            clear_();
            m_var = static_cast<double> (value);
            m_var_type_current = typeFromLimit(value);
            m_var_is_init = true;
        }

        template < typename T>
        typename std::enable_if<std::is_same<char *, T>::value, void>::type
        SetValue_(T text) {

            std::string str(text);
            SetValue_(text);
        }

        template < typename T>
        typename std::enable_if<std::is_same<wchar_t *, T>::value, void>::type
        SetValue_(T text) {

            std::wstring str(text);
            SetValue_(text);
        }

        void SetValue_(std::string text) {
            TEST_CONST_();
            if (m_var_type_current != ObjType::StrChar) {

                testConvertType(ObjType::StrChar);
                m_var_type_current = ObjType::StrChar;
            }
            m_value.swap(text);
            m_var_is_init = true;
        }

        void SetValue_(std::wstring text) {
            TEST_CONST_();
            if (m_var_type_current != ObjType::StrWide) {

                testConvertType(ObjType::StrWide);
                m_var_type_current = ObjType::StrWide;
            }
            m_string.swap(text);
            m_var_is_init = true;
        }

        inline void testConvertType(ObjType type) {
            if (m_var_type_fixed == ObjType::None || canCast(type, m_var_type_fixed)) {

                return;
            }
            LOG_RUNTIME("Cannot changed type from '%s' to '%s'!", newlang::toString(type), newlang::toString(m_var_type_fixed));
        }

        inline void testResultIntegralType(ObjType type, bool upscalint) {
            ObjType new_type = m_var_type_current;

            if (((is_integral() && isIntegralType(type, true)) || (is_floating() && isFloatingType(type)))
                    && static_cast<uint8_t> (type) <= static_cast<uint8_t> (m_var_type_current)) {
                // type already including to current type

            } else {

                if (!canCast(type, m_var_type_current)) {
                    testConvertType(type);
                    new_type = type;
                }
                bool check = false;
                if (upscalint && isIntegralType(m_var_type_current, true) && isIntegralType(type, true)) {
                    if (type < ObjType::Int64) {
                        new_type = static_cast<ObjType> (static_cast<uint8_t> (type) + 1);
                        check = true;
                    }
                    if (check && m_var_type_fixed != ObjType::None && !canCast(new_type, m_var_type_fixed)) {
                        new_type = type; // Тип данных менять нельзя, но сама операция возможна
                        LOG_WARNING("Data type '%s' cannot be changed to '%s', loss of precision is possible!", newlang::toString(type), newlang::toString(m_var_type_fixed));
                    }
                }
            }
            if (new_type != m_var_type_current) {
                if (is_scalar()) {
                    if (isFloatingType(new_type) && isIntegralType(m_var_type_current, true)) {
                        // Для скаляров повышение типа с целочисленного на число с плавающий точкой
                        m_var = static_cast<double> (GetValueAsInteger());
                    } else {
                        // Измерение размерности, а не типа - ничего делать ненужно
                        ASSERT((isFloatingType(new_type) && isFloatingType(m_var_type_current)) ||
                                (isIntegralType(new_type, true) && isIntegralType(m_var_type_current, true)));
                    }
                } else {

                    ASSERT(m_tensor.defined());
                    m_tensor = m_tensor.toType(toTorchType(new_type));
                }
                m_var_type_current = new_type;
            }
        }

        void SetValue_(ObjPtr value) {
            TEST_CONST_();
            if (value->is_none_type()) {
                clear_();
                return;
            } else if ((is_none_type() || is_class_type()) && (value->is_class_type() || (value->is_type_name() && isClass(value->m_var_type_fixed)))) {
                if (is_class_type() && m_class_name.compare(value->m_class_name) != 0) {
                    ASSERT(!value->m_class_name.empty());
                    LOG_RUNTIME("Fail set value class '%s' as class '%s'!", m_class_name.c_str(), value->m_class_name.c_str());
                }

                std::string old_name = m_var_name;
                value->CloneDataTo(*this);
                value->ClonePropTo(*this);
                m_var_name.swap(old_name);
                m_var_is_init = true;
                return;

            } else if ((is_none_type() || is_tensor_type()) && value->is_tensor_type()) {

                if (value->empty()) {
                    m_var = at::monostate();
                    m_tensor.reset();
                    m_var_is_init = false;
                    return;
                }

                if (!canCast(value->m_var_type_current, m_var_type_current)) {
                    testConvertType(value->m_var_type_current);
                }

                if (is_none_type()) {

                    // Присаеваем данные пустому значению
                    ASSERT(at::holds_alternative<at::monostate>(m_var));
                    ASSERT(!m_tensor.defined());

                    if (value->is_scalar()) {
                        m_var = value->m_var;
                        //                        if (value->is_integral()) {
                        //                            m_var = value->GetValueAsInteger(); // Нужно считывать значение, т.к. может быть ссылка
                        //                        } else {
                        //                            ASSERT(value->is_floating());
                        //                            m_var = value->GetValueAsNumber(); // Нужно считывать значение, т.к. может быть ссылка
                        //                        }
                    } else {
                        m_tensor = value->m_tensor.clone();
                    }
                    m_var_type_current = value->m_var_type_current;

                } else {

                    // текущая переменная уже сожержит данные

                    if (is_scalar() && value->is_scalar()) {
                        // Два скаляра
                        switch (m_var_type_current) {
                            case ObjType::Bool:
                                if (at::holds_alternative<int64_t>(m_var)) {
                                    m_var = value->GetValueAsInteger();
                                } else if (at::holds_alternative<bool *>(m_var)) {
                                    ASSERT(at::get<bool *>(m_var));
                                    *at::get<bool *>(m_var) = value->GetValueAsInteger();
                                }
                                break;
                            case ObjType::Int8:
                            case ObjType::Char:
                            case ObjType::Byte:
                                if (at::holds_alternative<int64_t>(m_var)) {
                                    m_var = value->GetValueAsInteger();
                                } else if (at::holds_alternative<int8_t *>(m_var)) {
                                    ASSERT(at::get<int8_t *>(m_var));
                                    *at::get<int8_t *>(m_var) = static_cast<int8_t> (value->GetValueAsInteger());
                                }
                                break;
                            case ObjType::Int16:
                            case ObjType::Word:
                                if (at::holds_alternative<int64_t>(m_var)) {
                                    m_var = value->GetValueAsInteger();
                                } else if (at::holds_alternative<int16_t *>(m_var)) {
                                    ASSERT(at::get<int16_t *>(m_var));
                                    *at::get<int16_t *>(m_var) = static_cast<int16_t> (value->GetValueAsInteger());
                                }
                                break;
                            case ObjType::Int32:
                            case ObjType::DWord:
                                if (at::holds_alternative<int64_t>(m_var)) {
                                    m_var = value->GetValueAsInteger();
                                } else if (at::holds_alternative<int32_t *>(m_var)) {
                                    ASSERT(at::get<int32_t *>(m_var));
                                    *at::get<int32_t *>(m_var) = static_cast<int32_t> (value->GetValueAsInteger());
                                }
                                break;
                            case ObjType::Int64:
                            case ObjType::DWord64:
                                if (at::holds_alternative<int64_t>(m_var)) {
                                    m_var = value->GetValueAsInteger();
                                } else if (at::holds_alternative<int64_t *>(m_var)) {
                                    ASSERT(at::get<int64_t *>(m_var));
                                    *at::get<int64_t *>(m_var) = value->GetValueAsInteger();
                                }
                                break;
                            case ObjType::Float32:
                            case ObjType::Single:
                                if (at::holds_alternative<double>(m_var)) {
                                    m_var = value->GetValueAsNumber();
                                } else if (at::holds_alternative<float *>(m_var)) {
                                    ASSERT(at::get<float *>(m_var));
                                    *at::get<float *>(m_var) = static_cast<float> (value->GetValueAsNumber());
                                }
                                break;
                            case ObjType::Float64:
                            case ObjType::Double:
                                if (at::holds_alternative<double>(m_var)) {
                                    m_var = value->GetValueAsNumber();
                                } else if (at::holds_alternative<double *>(m_var)) {
                                    ASSERT(at::get<double *>(m_var));
                                    *at::get<double *>(m_var) = value->GetValueAsNumber();
                                }
                                break;
                            default:
                                LOG_RUNTIME("Fail set value type '%s'!", newlang::toString(m_var_type_current));
                        }

                    } else if (is_scalar() && !value->is_scalar()) {

                        m_var = at::monostate();
                        ASSERT(!m_tensor.defined());
                        m_tensor = value->m_tensor.clone();

                    } else if (!is_scalar() && value->is_scalar()) {

                        // Установить одно значение для всех элементов тензора
                        if (is_integral()) {
                            m_tensor.set_(torch::scalar_tensor(value->GetValueAsInteger(), m_tensor.scalar_type()));
                        } else {
                            ASSERT(is_floating());
                            m_tensor.set_(torch::scalar_tensor(value->GetValueAsNumber(), m_tensor.scalar_type()));
                        }

                    } else {
                        //  Продублировать значения тензора если они одинакового размера
                        if (m_tensor.sizes().equals(value->m_tensor.sizes())) {
                            m_tensor = value->m_tensor.toType(m_tensor.scalar_type()).clone();
                        } else {
                            LOG_RUNTIME("Different sizes of tensors!");
                        }
                    }
                }
                m_var_is_init = true;
                return;

            } else if ((is_none_type() || is_string_type()) && value->is_string_type()) {

                switch (m_var_type_current) {
                    case ObjType::None: // @todo Какой тип сроки по умолчанию? Пока байтовые
                    case ObjType::StrChar:
                    case ObjType::FmtChar:
                        SetValue_(value->GetValueAsString());
                        return;
                    case ObjType::StrWide:
                    case ObjType::FmtWide:
                        SetValue_(value->GetValueAsStringWide());
                        return;
                }

            } else if ((is_none_type() || is_dictionary_type()) && (value->is_dictionary_type() || value->getType() == ObjType::Iterator)) {

                std::string old_name = m_var_name;
                clear_();
                value->CloneDataTo(*this);
                value->ClonePropTo(*this);
                m_var_name.swap(old_name);
                m_var_is_init = true;
                return;

                //            } else if ((is_none_type() || m_var_type_current == ObjType::Pointer) && value->m_var_type_current == ObjType::Pointer) {
                //                //@todo Check tree type !!!
                //
                //                std::string old_name = m_var_name;
                //                value->CloneDataTo(*this);
                //                value->ClonePropTo(*this);
                //                m_var_name.swap(old_name);
                //                m_var_is_init = true;
                //                return;

            } else if (((is_none_type() || m_var_type_current == ObjType::Function) && value->is_function_type()) ||
                    ((is_none_type() || m_var_type_current == ObjType::Pointer) && value->m_var_type_current == ObjType::Pointer)) {
                //@todo Check function type args !!!

                std::string old_name = m_var_name;
                value->CloneDataTo(*this);
                value->ClonePropTo(*this);
                m_var_name.swap(old_name);
                m_var_is_init = true;
                return;

            } else if ((is_none_type() && value->getType() == ObjType::Rational) || ((m_var_type_current == ObjType::Rational) && value->is_arithmetic_type())) {

                if (is_none_type()) {
                    m_rational = *value->GetValueAsRational();
                    m_var_is_init = true;
                } else {
                    m_rational.set_(*value->GetValueAsRational());
                }
                m_var = at::monostate();
                m_var_type_current = ObjType::Rational;
                return;

            } else if ((is_none_type() || m_var_type_current == ObjType::Function || m_var_type_current == ObjType::EVAL_FUNCTION) && value->is_block()) {
                //@todo Check function type args !!!

                //            std::string old_name = m_var_name;
                //            TermPtr save_proto = m_func_proto;
                //            TermPtr save_block = m_block_source;
                //            ObjType save_type = m_var_type_current;
                //            value->CloneDataTo(*this);
                //            value->ClonePropTo(*this);
                //            m_var_name.swap(old_name);
                //            *const_cast<TermPtr *> (&m_func_proto) = save_proto;
                m_sequence = value->m_sequence;
                m_var_is_init = value->m_var_is_init;
                //            m_var_type_current = save_type;

                return;

            } else if ((is_none_type() || m_var_type_current == ObjType::Pointer || m_var_type_current == ObjType::NativeFunc) && (value->m_var_type_current == ObjType::Pointer || value->m_var_type_fixed == ObjType::Pointer)) {

                m_var_is_init = value->m_var_is_init;
                ASSERT(at::holds_alternative<void *>(value->m_var));
                m_var = at::get<void *>(value->m_var);
                return;

            }

            LOG_RUNTIME("Set value type '%s' as '%s' not implemented!", newlang::toString(m_var_type_current), value->toString().c_str());
        }

        static std::string format(std::string format, Obj * args);


        bool CallAll(const char *func_name, ObjPtr &arg_in, ObjPtr &result, ObjPtr object = nullptr, size_t limit = 0); // ?
        bool CallOnce(ObjPtr &arg_in, ObjPtr &result, ObjPtr object = nullptr); // !


        static ObjPtr CreateFunc(Context *ctx, TermPtr proto, ObjType type, const std::string var_name = "");
        static ObjPtr CreateFunc(std::string proto, FunctionType *func_addr, ObjType type = ObjType::Function);
        static ObjPtr CreateFunc(std::string proto, TransparentType *func_addr, ObjType type = ObjType::PureFunc);

        ObjPtr ConvertToArgs(Obj *args, bool check_valid, Context * ctx) const {
            ObjPtr result = Clone();
            result->ConvertToArgs_(args, check_valid, ctx);

            return result;
        }

        void CheckArgsValid() const;
        bool CheckArgs() const;

        inline const TermPtr Proto() {

            return m_prototype;
        }

        SCOPE(protected) :

        void ConvertToArgs_(Obj *args, bool check_valid, Context *ctx = nullptr); // Обновить параметры для вызова функции или элементы у словаря при создании копии

    public:

        void remove(const Variable::PairType & value) {
            Variable::remove(value);
        }

        ObjType m_var_type_current; ///< Текущий тип значения объекта
        ObjType m_var_type_fixed; ///< Максимальный размер для арифметических типов, который задается разработчиком
        bool m_var_is_init; ///< Содержит ли объект корректное значение ???


        //        std::string m_namespace;
        std::string m_var_name; ///< Имя переменной, в которой хранится объект

        ObjPtr m_dimensions; ///< Размерности для ObjType::Type
        std::string m_class_name; ///< Имя класса объекта (у базовых типов отсуствует)
        std::vector<ObjPtr> m_class_parents; ///< Родительские классы (типы)

        mutable PairType m_str_pair; //< Для доступа к отдельным символам строк

        std::string m_func_mangle_name;
        std::string m_module_name;

        // Применение variant необходимо для полей хранения данных, чтобы контролировать их инициализацию
        //        std::variant<std::monostate, void *, torch::Tensor, std::string, std::wstring, std::string_view, std::wstring_view,
        //        std::shared_ptr<Rational>, std::shared_ptr< Iterator<Obj> >, TermPtr> m_var;

        //        std::variant<std::monostate, void *, torch::Tensor, std::string, std::wstring, std::string_view, std::wstring_view> m_var;

        struct NativeData {
            void * ptr;
            int64_t size;
        };

        at::variant < at::monostate, int64_t, double, void *, // None, скаляры и ссылки на функции (нужно различать чистые, обычные и нативные???)
        bool *, int8_t *, int16_t *, int32_t *, int64_t *, float *, double *, NativeData, // Ссылки на нативные скаляры и данные
        torch::Tensor, std::string, std::wstring, TermPtr, ModulePtr, Iterator < Obj>> m_var; //Rational,

        //        union {
        //            int64_t m_integer;
        //            double m_number;
        //            void *m_pointer; ///< Содержит указатель на нативную функцию или область памяти с данными или скаляр
        //        };

        std::string m_value; ///< Содержит байтовую строку или байтовый массив с данными для представления в нативном виде (Struct, Unuion, Enum)
        std::wstring m_string; ///< Содержит строку широких символов
        torch::Tensor m_tensor; ///< Содержит только размерные тензоры (скляры хранятся в поле m_pointer и не создают m_tensor.defined())
        Rational m_rational; ///< Содержит дробь из длинных чисел
        std::shared_ptr<Iterator < Obj>> m_iterator; ///< Итератор для данных
        mutable ObjPtr m_iter_range_value;
        TermPtr m_sequence; ///< Последовательно распарсенных команд для выполнения
        const TermPtr m_prototype; ///< Описание прототипп функции (или данных)
        ObjPtr m_return_obj;

        bool m_check_args; //< Проверять аргументы на корректность (для всех видов функций) @ref MakeArgs

        /* Для будущей переделки системы типов и базового класса: 
         * Должен быть интерфейс с поддерживаемыми операциями для стандартных типов данных
         * и набор реализаций для скаляров, строк, тензоров, нативных функций, дробей, внутренних функций и т.д.
         */

        //    SCOPE(protected) :
        bool m_is_const; //< Признак константы (по умолчанию изменения разрешено)
        bool m_is_reference; //< Признак ссылки на объект
    };

} // namespace newlang

std::ostream & operator<<(std::ostream &out, newlang::Obj & var);
std::ostream & operator<<(std::ostream &out, newlang::ObjPtr var);


#endif // INCLUDED_NEWLANG_OBJECT_
