#pragma once
#ifndef INCLUDED_OBJECT_H_
#define INCLUDED_OBJECT_H_

#include <regex>

#include "types.h"
#include "variable.h"
#include "rational.h"


namespace c10 {
    template <typename T>
    class ArrayRef;
}

namespace at {
    namespace indexing {
        struct TensorIndex;
    }
}

namespace at {
    class Tensor;
    class ArrayRef;
    using IntArrayRef = c10::ArrayRef<int64_t>;
} // namespace at

namespace torch {
    using at::Tensor;
    namespace serialize {
        class OutputArchive;
        class InputArchive;
    } // namespace serialize
} // namespace torch


typedef at::indexing::TensorIndex Index;
typedef at::IntArrayRef Dimension;



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
    //    bool ParsePrintfFormat(Obj *args, int start = 1);

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

    std::string utf8_encode(const std::wstring wstr);

    // Convert an UTF8 string to a wide Unicode String

    std::wstring utf8_decode(const std::string str);

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

        const IterPairType &data() {
            if (m_found == m_iter_obj->end()) {
                return m_Iterator_end;
            }
            return *m_found;
        }

        const IterPairType &operator*() {
            return data();
        }

        const IterPairType &data() const {
            if (m_found == m_iter_obj->end()) {
                return m_Iterator_end;
            }
            return *m_found;
        }

        const IterPairType &operator*() const {
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

        const iterator operator++(int) const {
            return iterator::operator++();
        }

        bool operator==(const iterator &other) const {
            return m_found == other.m_found;
        }

        bool operator!=(const iterator &other) const {
            return m_found != other.m_found;
        }

        void reset() {
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
    class Obj : public Variable<Obj>, public std::enable_shared_from_this<Obj> {
    public:

        //    constexpr static const char * BUILDIN_TYPE = "__var_type__";
        //    constexpr static const char * BUILDIN_NAME = "__var_name__";
        //    constexpr static const char * BUILDIN_BASE = "__class_base__";
        //    constexpr static const char * BUILDIN_CLASS = "__class_name__";
        //    constexpr static const char * BUILDIN_NAMESPACE = "__namespace__";

        typedef Variable::PairType PairType;

        Obj(ObjType type = ObjType::None, const char *var_name = nullptr, TermPtr func_proto = nullptr, ObjType fixed = ObjType::None, bool init = false, Sync *sync = nullptr);
        //        Obj(Context *ctx, const TermPtr term, bool as_value, Obj *local_vars);


        [[nodiscard]]
        static PairType ArgNull(const std::string name = "") {
            return pair(Obj::CreateNil(), name);
        }

        [[nodiscard]]
        static PairType Arg() {
            return pair(CreateNone());
        }

        [[nodiscard]]
        static PairType Arg(ObjPtr value, const std::string name = "") {
            return pair(value, name);
        }

        template<typename T>
        typename std::enable_if<std::is_pointer<T>::value || std::is_same<std::string, T>::value, PairType>::type
        static Arg(T value, const std::string name = "") {
            return pair(CreateString(value), name);
        }

        template<typename T>
        typename std::enable_if<!std::is_same<PairType, T>::value && !std::is_pointer<T>::value && !std::is_same<std::string, T>::value, PairType>::type
        static Arg(T value, const std::string name = "") {
            return pair(CreateValue(value, ObjType::None), name);
        }

        ObjPtr shared() {
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

        ObjPtr IteratorNext(ObjPtr count) {
            return IteratorNext(count->GetValueAsInteger());
        }



#define TEST_CONST_() if (m_is_const) {LOG_RUNTIME("Can`t edit const value '%s'!", toString().c_str());}
#define TEST_INIT_() if (!m_var_is_init) {LOG_RUNTIME("Object not initialized '%s'!", toString().c_str());}

        [[nodiscard]]
        ObjType getType() {
            return m_var_type_current;
        }

        [[nodiscard]]
        ObjType getTypeAsLimit() {
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
        std::string & getName() {
            return m_var_name;
        }

        [[nodiscard]]
        const std::string & getName() const {
            return m_var_name;
        }

        void SetClassName(std::string &name) {
            m_class_name = name;
        }

        [[nodiscard]]
        bool is_const() const {
            return m_is_const;
        }

        [[nodiscard]]
        bool is_init() const {
            return m_var_is_init;
        }

        [[nodiscard]]
        bool is_none_type() const {
            return m_var_type_current == ObjType::None;
        }

        [[nodiscard]]
        bool is_bool_type() const {
            return isBooleanType(m_var_type_current);
        }

        [[nodiscard]]
        bool is_arithmetic_type() const {
            return isArithmeticType(m_var_type_current);
        }

        [[nodiscard]]
        bool is_string_type() const {
            return isString(m_var_type_current);
        }

        [[nodiscard]]
        bool is_string_char_type() const {
            return isStringChar(m_var_type_current);
        }

        [[nodiscard]]
        bool is_string_wide_type() const {
            return isStringWide(m_var_type_current);
        }

        [[nodiscard]]
        bool is_dictionary_type() const {
            return isDictionary(m_var_type_current);
        }

        //Plain data — это неизменяемые структуры без ссылок на другие объекты.
        [[nodiscard]]
        bool is_plain_type() const {
            return isPlainDataType(m_var_type_current);
        }

        [[nodiscard]]
        bool is_other_type() const {
            return !(is_none_type() || is_bool_type() || is_arithmetic_type() || is_string_type() || is_dictionary_type());
        }

        [[nodiscard]]
        bool is_class_type() const {
            return isClass(m_var_type_current);
        }

        [[nodiscard]]
        bool is_simple_type() const {
            return isSimpleType(m_var_type_current);
        }

        [[nodiscard]]
        bool is_scalar() const;

        [[nodiscard]]
        bool is_function_type() const {
            return isFunction(m_var_type_current);
        }

        [[nodiscard]]
        bool is_native() const {
            return isNative(m_var_type_current);
        }

        [[nodiscard]]
        bool is_tensor_type() const {
            return isTensor(m_var_type_current);
        }

        [[nodiscard]]
        bool is_integer() const {
            return isIntegralType(m_var_type_current, false);
        }

        [[nodiscard]]
        bool is_integral() const {
            return isIntegralType(m_var_type_current, true);
        }

        [[nodiscard]]
        bool is_complex() const {
            return isComplexType(m_var_type_current);
        }

        [[nodiscard]]
        bool is_floating() const {
            return isFloatingType(m_var_type_current);
        }

        [[nodiscard]]
        bool is_indexing() const {
            return isIndexingType(m_var_type_current, m_var_type_fixed) || is_error() || is_return();
        }

        [[nodiscard]]
        bool is_ellipsis() const {
            return isEllipsis(m_var_type_current);
        }

        [[nodiscard]]
        bool is_range() const {
            return isRange(m_var_type_current);
        }

        [[nodiscard]]
        bool is_rational() const {
            return m_var_type_current == ObjType::Rational;
        }

        [[nodiscard]]
        bool is_type_name() const {
            return isTypeName(m_var_type_current);
        }

        [[nodiscard]]
        bool is_error() const {
            return m_var_type_current == ObjType::Error || m_var_type_fixed == ObjType::Error || m_var_type_current == ObjType::ErrorParser || m_var_type_current == ObjType::ErrorRunTime || m_var_type_current == ObjType::ErrorSignal;
        }

        [[nodiscard]]
        bool is_return() const {
            return m_var_type_current == ObjType::RetPlus || m_var_type_current == ObjType::RetMinus || m_var_type_current == ObjType::RetRepeat;
        }

        [[nodiscard]]
        bool is_block() const {
            return m_var_type_current == ObjType::BLOCK || m_var_type_current == ObjType::BLOCK_PLUS || m_var_type_current == ObjType::BLOCK_MINUS || m_var_type_current == ObjType::BLOCK_TRY;
        }


        [[nodiscard]]
        bool is_defined_type() {
            return m_var_type_fixed != ObjType::None;
        }

        [[nodiscard]]
        bool is_any_size() {
            return m_var_type_current == ObjType::None || m_var_type_current == ObjType::Ellipsis;
        }

        static ObjPtr Take(Obj &args) {
            if (isInterrupt(args.m_var_type_current)) {
                return args.m_return_obj;
            } else if (args.size() && args[0].second->is_return()) {
                ASSERT(args.size() == 1);
                return args[0].second->m_return_obj;
            } else {
                return args.shared();
            }
//            LOG_RUNTIME("Take object for type %s not implemented!", newlang::toString(args[0].second->getType()));
        }

        void SetTermProp(Term &term);

        virtual int64_t size() const {
            return size(0);
        }
        virtual int64_t size(int64_t ind) const;
        int64_t resize_(int64_t size, ObjPtr fill, const std::string = "");

        virtual bool empty() const;

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

        ObjPtr operator()() {
            Obj args(ObjType::Dictionary);
            return op_call(args);
        }

        ObjPtr op_call(ObjPtr args) {
            ASSERT(args);
            return op_call(*args);
        }

        ObjPtr op_call(Obj &args);

        template <typename... T>
        typename std::enable_if<is_all<Obj::PairType, T ...>::value, ObjPtr>::type
        operator()(T ... args) {
            auto list = {args...};
            ObjPtr arg = Obj::CreateDict();
            for (auto &elem : list) {
                arg->push_back(elem);
            }
            return op_call(*arg);
        }

        template <typename... T>
        typename std::enable_if<!is_all<Obj::PairType, T ...>::value, ObjPtr>::type
        operator()(T ... args) {
            auto list = {args...};
            ObjPtr arg = Obj::CreateDict();
            for (auto &elem : list) {
                arg->push_back(Obj::CreateValue(elem));
            }
            return op_call(*arg);
        }


        //         ObjPtr Call(Context *ctx) {
        //            Obj args(ObjType::Dictionary);
        //            return Call(ctx, &args);
        //        }
        //
        //        template <typename... T>
        //        typename std::enable_if<is_all<Obj::PairType, T ...>::value, ObjPtr>::type
        //         Call(Context *ctx, T ... args) {
        //            auto list = {args...};
        //            Obj arg(ObjType::Dictionary);
        //            for (auto &elem : list) {
        //                arg.Variable<Obj>::push_back(elem);
        //            }
        //            return Call(ctx, &arg);
        //        }
        //
        //        ObjPtr Call(Context *ctx, Obj *args, bool direct = false, ObjPtr self = nullptr);

        /*
         * 
         * Интерфейс Variable
         * 
         */

        template <typename I>
        typename std::enable_if < std::is_integral<I>::value && !std::is_pointer<I>::value, const PairType &>::type
        operator[](const I index) {
            if (is_indexing() || m_var_type_current == ObjType::Range) {
                return at(index);
            }
            LOG_RUNTIME("Operator at for object type %s not implemented!", newlang::toString(m_var_type_current));
        }

        template <typename N>
        typename std::enable_if < std::is_same<N, std::string>::value || std::is_pointer<N>::value, const PairType &>::type
        operator[](const N name) {
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

        void erase(const size_t from, const size_t to) override;

        void erase(const int64_t index) override {
            if (!is_indexing()) {
                LOG_RUNTIME("Operator erase(index) for object type %s not implemented!", newlang::toString(m_var_type_current));
            }
            if (is_tensor_type()) {
                LOG_RUNTIME("Operator erase(index) for object type %s not implemented!", newlang::toString(m_var_type_current));
            } else {
                Variable<Obj>::erase(index);
            }
        }

        void clear_() override {
            Variable::clear_();
            clear_(true);
        }

        void clear_(bool clear_iterator_name);

        /*
         *  Cast operators
         * 
         */
        explicit inline operator bool() const {
            return GetValueAsBoolean();
        }

#define PLACE_RANGE_CHECK_VALUE(itype, utype)\
        explicit inline operator itype() const { \
            int64_t result = GetValueAsInteger(); \
            if (result > std::numeric_limits<itype>::max() || result < std::numeric_limits<itype>::lowest()) { \
                LOG_RUNTIME("Value '%s' is out of range of the casting type %s!", GetValueAsString().c_str(), #itype); \
            } \
            return result; \
        } \
        explicit inline operator utype() const { \
            int64_t result = GetValueAsInteger(); \
            if (result > std::numeric_limits<utype>::max() || result < 0) { \
                LOG_RUNTIME("Value '%s' is out of range of the casting type %s!", GetValueAsString().c_str(), #utype); \
            } \
            return result; \
        }

        PLACE_RANGE_CHECK_VALUE(int8_t, uint8_t);
        PLACE_RANGE_CHECK_VALUE(int16_t, uint16_t);
        PLACE_RANGE_CHECK_VALUE(int32_t, uint32_t);

        explicit inline operator int64_t() const {
            return GetValueAsInteger();
        }

        explicit inline operator uint64_t() const {
            int64_t result = GetValueAsInteger();
            if (result < 0) {
                LOG_RUNTIME("Value '%s' is out of range of the casting type %s!", GetValueAsString().c_str(), "uint64_t");
            }
            return result;
        }


#undef PLACE_RANGE_CHECK_VALUE

        explicit operator float() const;
        explicit operator double() const;

        explicit inline operator std::string() const {
            return GetValueAsString();
        }

        explicit inline operator std::wstring() const {
            return GetValueAsStringWide();
        }

        //унарный плюс ничего не делает.

        ObjPtr operator+() {
            if (is_arithmetic_type()) {
                return shared();
            }
            LOG_RUNTIME("Unary plus for object '%s' not supported!", toString().c_str());
        }

        ObjPtr operator-();

        ObjPtr & operator+(ObjPtr & obj) {

            if (is_tensor_type()) {
                return obj;
            }
            LOG_RUNTIME("Object '%s' not numeric!", obj->toString().c_str());
        }

        ObjPtr &operator-(ObjPtr & obj);


        //префиксная версия возвращает значение после инкремента

        ObjPtr operator++();

        //постфиксная версия возвращает значение до инкремента

        ObjPtr operator++(int) {
            ObjPtr old = Clone();
            Obj::operator++();
            return old;
        }

        //префиксная версия возвращает значение после декремента

        ObjPtr operator--();

        //постфиксная версия возвращает значение до декремента

        ObjPtr operator--(int) {
            ObjPtr old = Clone();
            Obj::operator--();
            return old;
        }

        ObjPtr operator*(ObjPtr obj) {
            ASSERT(obj);
            return operator*(*obj);
        }

        ObjPtr operator*(Obj value) {
            ObjPtr result = Clone();
            *result *= value;
            return result;
        }

        ObjPtr operator/(ObjPtr obj) {
            ASSERT(obj);
            return operator/(*obj);
        }

        ObjPtr operator/(Obj value) {
            ObjPtr result = Clone();
            *result /= value;
            return result;
        }

        ObjPtr op_div_ceil(ObjPtr obj) {
            ASSERT(obj);
            return op_div_ceil(*obj);
        }

        ObjPtr op_div_ceil(Obj value) {
            ObjPtr result = Clone();
            result->op_div_ceil_(value);
            return result;
        }

        ObjPtr op_concat(ObjPtr obj, ConcatMode mode = ConcatMode::Error) {
            ASSERT(obj);
            return op_concat(*obj, mode);
        }

        ObjPtr op_concat(Obj &value, ConcatMode mode = ConcatMode::Error) {
            ObjPtr result = Clone();
            result->op_concat_(value, mode);
            return result;
        }

        ObjPtr op_concat_(Obj &obj, ConcatMode mode = ConcatMode::Error) {
            ConcatData(this, obj, mode);
            return shared();
        }

        ObjPtr op_concat_(ObjPtr obj, ConcatMode mode = ConcatMode::Error) {
            return op_concat_(*obj, mode);
        }

        ObjPtr operator%(ObjPtr obj) {
            ASSERT(obj);
            return operator%(*obj);
        }

        ObjPtr operator%(Obj value) {
            ObjPtr result = Clone();
            *result %= value;
            return result;
        }

        ObjPtr operator+(ObjPtr obj) {
            ASSERT(obj);
            return operator+(*obj);
        }

        ObjPtr operator+(Obj value) {
            ObjPtr result = Clone();
            *result += value;
            return result;
        }

        ObjPtr operator-(ObjPtr obj) {
            ASSERT(obj);
            return operator-(*obj);
        }

        ObjPtr operator-(Obj value) {
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

        bool operator<(ObjPtr obj) {
            ASSERT(obj);
            return operator<(*obj);
        }

        bool operator<(Obj obj) {
            return op_compare(obj) < 0;
        }

        bool operator<=(ObjPtr obj) {
            ASSERT(obj);
            return operator<=(*obj);
        }

        bool operator<=(Obj obj) {
            return op_compare(obj) <= 0;
        }

        bool operator>(ObjPtr obj) {
            ASSERT(obj);
            return operator>(*obj);
        }

        bool operator>(Obj obj) {
            return op_compare(obj) > 0;
        }

        bool operator>=(ObjPtr obj) {
            ASSERT(obj);
            return operator>=(*obj);
        }

        bool operator>=(Obj obj) {
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

        bool op_duck_test(ObjPtr obj, bool strong) {
            ASSERT(obj);
            return op_duck_test(obj.get(), strong);
        }
        bool op_duck_test(Obj *value, bool strong);
        static bool op_duck_test_prop(Obj *base, Obj *value, bool strong);

        bool op_equal(ObjPtr value) {
            ASSERT(value);
            if (value) {
                return op_equal(*value);
            }
            return false;
        }

        bool op_equal(Obj & value);

        bool op_accurate(ObjPtr obj) {
            ASSERT(obj);
            return op_accurate(*obj);
        }
        bool op_accurate(Obj & value);

        ObjPtr op_bit_and(ObjPtr obj, bool strong) {
            ASSERT(obj);
            return op_bit_and(*obj, strong);
        }

        ObjPtr op_bit_and(Obj &obj, bool strong) {
            ObjPtr result = Clone();
            result->op_bit_and_set(obj, strong);
            return result;
        }

        ObjPtr op_pow(ObjPtr obj) const {
            ASSERT(obj);
            return op_pow(*obj);
        }

        ObjPtr op_pow(Obj &obj) const {
            ObjPtr result = Clone();
            result->op_pow_(obj);
            return result;
        }

        ObjPtr op_pow_(ObjPtr obj) {
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
            clear_();
            obj.CloneDataTo(*this);
            obj.ClonePropTo(*this);
            return shared();
        }

        bool operator=(Obj & obj) {
            clear_();
            obj.CloneDataTo(*this);
            obj.ClonePropTo(*this);
            return true;
        }

        /*    
         * ?: (выбор из двух операндов)
         * = (присваивание)
         * *=, /=, %=, +=, -=, &=, ^=, |=, <<=, >>=, >>>= (операции с присваиванием)
         * , (отбрасывание первого и возврат второго операнда) 
         */


        ObjPtr operator*=(ObjPtr obj) {
            if (!obj) {
                ASSERT(obj);
            }
            return operator*=(*obj);
        }

        ObjPtr operator*=(Obj obj);

        ObjPtr operator/=(ObjPtr obj) {
            ASSERT(obj);
            return operator/=(*obj);
        }

        ObjPtr operator/=(Obj obj);

        ObjPtr op_div_ceil_(ObjPtr obj) {
            ASSERT(obj);
            return op_div_ceil_(*obj);
        }

        ObjPtr op_div_ceil_(Obj &obj);

        ObjPtr operator%=(ObjPtr obj) {
            ASSERT(obj);
            return operator%=(*obj);
        }

        ObjPtr operator%=(Obj obj);

        ObjPtr operator+=(ObjPtr obj) {
            ASSERT(obj);
            return operator+=(*obj);
        }
        ObjPtr operator+=(Obj obj);

        ObjPtr operator-=(ObjPtr obj) {
            ASSERT(obj);
            return operator-=(*obj);
        }
        ObjPtr operator-=(Obj obj);


        ObjPtr op_bit_and_set(Obj &obj, bool strong);

        ObjPtr operator^=(ObjPtr obj) {
            ASSERT(obj);
            return operator^=(*obj);
        }

        ObjPtr operator^=(Obj) {
            LOG_RUNTIME("Operator '^=' not implementd!");
        }

        ObjPtr operator|=(ObjPtr obj) {
            ASSERT(obj);
            return operator|=(*obj);
        }

        ObjPtr operator|=(Obj) {
            LOG_RUNTIME("Operator '|=' not implementd!");
        }

        ObjPtr op_lshift_set(ObjPtr obj) {
            ASSERT(obj);
            return op_lshift_set(*obj);
        }

        ObjPtr op_lshift_set(Obj) {
            LOG_RUNTIME("Operator '<<=' not implementd!");
        }

        ObjPtr op_rshift_set(ObjPtr obj) {
            ASSERT(obj);
            return op_rshift_set(*obj);
        }

        ObjPtr op_rshift_set(Obj) {
            LOG_RUNTIME("Operator '>>=' not implementd!");
        }

        ObjPtr op_rrshift_set(ObjPtr obj) {
            ASSERT(obj);
            return op_rrshift_set(*obj);
        }

        ObjPtr op_rrshift_set(Obj) {
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

        std::wstring GetValueAsStringWide() const {
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

        int64_t GetValueAsInteger() const;
        double GetValueAsNumber() const;

        bool GetValueAsBoolean() const;

        static ObjPtr CreateType(ObjType type, ObjType fixed = ObjType::None, bool is_init = false, Sync *sync = nullptr) {
            return std::make_shared<Obj>(type, nullptr, nullptr, fixed, is_init, sync);
        }

        static ObjPtr CreateReference(ObjPtr obj, RefType ref_type) {

            ASSERT(obj);

            if (ref_type == RefType::None) {
                LOG_RUNTIME("Invalid reference type!");
            }
            if (!obj->m_sync) {
                LOG_RUNTIME("The object does not allow creating reference!");
            }

            switch (obj->m_sync->GetRefType()) {
                case RefType::LiteSingle:
                case RefType::LiteSingleConst:
                    if (!(ref_type == RefType::LiteSingle || ref_type == RefType::LiteSingleConst)) {
                        LOG_RUNTIME("Reference type error!");
                    }
                    break;
                case RefType::LiteThread:
                case RefType::LiteThreadConst:
                    if (!(ref_type == RefType::LiteThread || ref_type == RefType::LiteThreadConst)) {
                        LOG_RUNTIME("Reference type error!");
                    }
                    break;
                case RefType::SyncMono:
                case RefType::SyncMonoConst:
                    if (!(ref_type == RefType::SyncMono || ref_type == RefType::SyncMonoConst)) {
                        LOG_RUNTIME("Reference type error!");
                    }
                    break;
                case RefType::SyncMulti:
                case RefType::SyncMultiConst:
                    if (!(ref_type == RefType::SyncMulti || ref_type == RefType::SyncMultiConst)) {
                        LOG_RUNTIME("Reference type error!");
                    }
                    break;
                default:
                    LOG_RUNTIME("Invalid reference type!");
            }

            if (isEditableRef(ref_type) && isConstRef(obj->m_sync->GetRefType())) {
                LOG_RUNTIME("Read-only reference are allowed!");
            }

            ObjPtr result = Obj::CreateType(ObjType::Reference, obj->m_var_type_fixed, true, obj->m_sync);
            result->m_reference = obj;
            return result;
        }

        inline Taken Take(bool edit_mode = true, const std::chrono::milliseconds & timeout_duration = Sync::SyncTimeoutDeedlock) {
            if (m_var_type_current == ObjType::Reference) {
                return Taken(m_reference, m_sync, edit_mode, timeout_duration);
            } else {
                return Taken(shared(), m_sync, edit_mode, timeout_duration);
            }
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
        static ObjPtr BaseTypeConstructor(Context *ctx, Obj & in);
        static ObjPtr ConstructorSimpleType_(Context *ctx, Obj & args);
        static ObjPtr ConstructorDictionary_(Context *ctx, Obj & args);
        static ObjPtr ConstructorNative_(Context *ctx, Obj & args);
        static ObjPtr ConstructorStub_(Context *ctx, Obj & args);
        static ObjPtr ConstructorClass_(Context *ctx, Obj & args);
        static ObjPtr ConstructorStruct_(Context *ctx, Obj & args);
        static ObjPtr ConstructorEnum_(Context *ctx, Obj & args);

        //        static ObjPtr ConstructorError_(Context *ctx, Obj & args);
        //        static ObjPtr ConstructorReturn_(Context *ctx, Obj & args);
        //        static ObjPtr ConstructorThread_(Context *ctx, Obj & args);
        //        static ObjPtr ConstructorSystem_(Context *ctx, Obj & args);
        //        static ObjPtr ConstructorInterraption_(Context *ctx, Obj & args, ObjType type);

        static ObjPtr CreateBaseType(ObjType type);

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

        static ObjPtr CreateNone(Sync *sync = nullptr) {
            return CreateType(ObjType::None, ObjType::None, true, sync);
        }

        static ObjPtr CreateEmpty(Sync *sync = nullptr) {
            return CreateType(ObjType::None, ObjType::None, false, sync);
        }

        static ObjPtr CreateNil(Sync *sync = nullptr) {
            ObjPtr obj = CreateType(ObjType::Pointer, ObjType::Pointer, false, sync);
            obj->m_var = (void *) nullptr;
            return obj;
        }

        static ObjPtr CreateBool(bool value, Sync *sync = nullptr) {
            ObjPtr result = Obj::CreateType(ObjType::Bool, ObjType::None, true, sync);
            result->SetValue_(value);

            return result;
        }

        template <typename T1, typename T2, typename T3>
        static ObjPtr CreateRange(T1 start, T2 stop, T3 step, Sync *sync = nullptr) {
            ObjPtr obj = CreateType(ObjType::Dictionary, ObjType::Range, true, sync);
            obj->push_back(CreateValue(start, ObjType::None), "start");
            obj->push_back(CreateValue(stop, ObjType::None), "stop");
            obj->push_back(CreateValue(step, ObjType::None), "step");
            obj->m_var_type_current = ObjType::Range;
            return obj;
        }

        template <typename T1, typename T2>
        static ObjPtr CreateRange(T1 start, T2 stop, Sync *sync = nullptr) {
            ObjPtr obj = CreateType(ObjType::Dictionary, ObjType::Range, true, sync);
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

        template <typename T>
        typename std::enable_if < std::is_same<T, std::string>::value || std::is_same<T, const char *>::value, ObjPtr>::type
        static CreateValue(T value, Sync *sync = nullptr) {
            return Obj::CreateString(value, sync);
        }

        template <typename T>
        typename std::enable_if < std::is_same<T, std::wstring>::value || std::is_same<T, const wchar_t *>::value, ObjPtr>::type
        static CreateValue(T value, Sync *sync = nullptr) {
            return Obj::CreateString(value, sync);
        }

        template <typename T>
        typename std::enable_if<std::is_integral<T>::value, ObjPtr>::type
        static CreateValue(T value, ObjType fix_type = ObjType::None, Sync *sync = nullptr) {
            ObjPtr result = CreateType(typeFromLimit((int64_t) value), fix_type, true, sync);
            if (fix_type != ObjType::None) {
                NL_CHECK(canCast(result->m_var_type_current, fix_type),
                        "Fail cast type from '%s' to '%s'!", newlang::toString(result->m_var_type_current), newlang::toString(fix_type));
            }
            result->m_var = static_cast<int64_t> (value);
            return result;
        }

        template <typename T>
        typename std::enable_if<std::is_floating_point<T>::value, ObjPtr>::type
        static CreateValue(T value, ObjType fix_type = ObjType::None, Sync *sync = nullptr) {
            ObjPtr result = CreateType(typeFromLimit((double) value), fix_type, true, sync);
            if (fix_type != ObjType::None) {
                NL_CHECK(canCast(result->m_var_type_current, fix_type),
                        "Fail cast type from '%s' to '%s'!", newlang::toString(result->m_var_type_current), newlang::toString(fix_type));
            }
            result->m_var = static_cast<double> (value);
            return result;
        }

        static ObjPtr CreateString(const std::string str, Sync *sync = nullptr) {
            ObjPtr result = CreateType(ObjType::StrChar, ObjType::String, true, sync);
            result->m_value = str;
            return result;
        }

        static ObjPtr CreateString(const std::wstring str, Sync *sync = nullptr) {
            ObjPtr result = CreateType(ObjType::StrWide, ObjType::String, true, sync);
            result->m_string = str;
            return result;
        }

        static ObjPtr Yes(Sync *sync = nullptr) {
            ObjPtr result = CreateType(ObjType::Bool, ObjType::Bool, true, sync);
            result->m_var = static_cast<int64_t> (1);
            return result->MakeConst();
        }

        static ObjPtr No(Sync *sync = nullptr) {
            ObjPtr result = CreateType(ObjType::Bool, ObjType::Bool, true, sync);
            result->m_var = static_cast<int64_t> (0);
            return result->MakeConst();
        }

        //        static ObjPtr None(Sync *sync = nullptr) {
        //            return Obj::CreateType(ObjType::None, ObjType::None, true, sync);
        //        }

        static ObjPtr CreateDict(Sync *sync = nullptr) {
            return Obj::CreateType(ObjType::Dictionary, ObjType::Dictionary, true, sync);
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

        static ObjPtr CreateDict(const std::vector<ObjPtr> & arr, Sync *sync = nullptr) {
            ObjPtr result = Obj::CreateType(ObjType::Dictionary, ObjType::None, true, sync);
            for (size_t i = 0; i < arr.size(); i++) {
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
        static ObjPtr CreateClass(std::string name) {
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

        void CloneTo(Obj & clone) {
            if (&clone == this) {
                // Не клонировать сам в себя

                return;
            }
            CloneDataTo(clone);
            ClonePropTo(clone);
        }

        void CloneTo(ObjPtr & clone) {
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

        ObjPtr toType(ObjType type) const {
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

        ObjPtr index_set(const std::vector<Index> & index, const ObjPtr value) const {
            ObjPtr result = Clone();
            result->index_set_(index, value);

            return result;
        }
        ObjPtr index_set_(const std::vector<Index> & index, const ObjPtr value);

        ObjPtr op_set_index(ObjPtr index, ObjPtr value) {

            return op_set_index(index->GetValueAsInteger(), value->GetValueAsString());
        }
        ObjPtr op_set_index(int64_t index, std::string value);

        template < typename T>
        typename std::enable_if < std::is_same<T, bool>::value, void>::type
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
        typename std::enable_if < std::is_same<wchar_t *, T>::value, void>::type
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

        void testConvertType(ObjType type) {
            if (m_var_type_fixed == ObjType::None || canCast(type, m_var_type_fixed)) {

                return;
            }
            LOG_RUNTIME("Cannot changed type from '%s' to '%s'!", newlang::toString(type), newlang::toString(m_var_type_fixed));
        }

        void testResultIntegralType(ObjType type, bool upscalint);

        void SetValue_(ObjPtr value);

        static std::string format(std::string format, Obj * args);


        bool CallAll(const char *func_name, ObjPtr &arg_in, ObjPtr &result, ObjPtr object = nullptr, size_t limit = 0); // ?
        bool CallOnce(ObjPtr &arg_in, ObjPtr &result, ObjPtr object = nullptr); // !

        const TermPtr Proto() {

            return m_prototype;
        }

        SCOPE(protected) :

    public:

        void remove(const Variable::PairType & value) {
            Variable::remove(value);
        }

        ObjType m_var_type_current; ///< Текущий тип значения объекта
        ObjType m_var_type_fixed; ///< Максимальный размер для арифметических типов, который задается разработчиком
        bool m_var_is_init; ///< Содержит ли объект корректное значение ???


        //        std::string m_namespace;
        std::string m_var_name; ///< Имя переменной, в которой хранится объект
        std::string m_class_name; ///< Имя класса объекта (у базовых типов отсуствует)
        std::vector<ObjPtr> m_class_parents; ///< Родительские классы (типы)
        std::string m_module_name;
        const TermPtr m_prototype; ///< Описание прототипа функции (или данных)
    public:
        Context *m_ctx;

        ObjPtr m_dimensions; ///< Размерности для ObjType::Type

        mutable PairType m_str_pair; //< Для доступа к отдельным символам строк

        std::string m_func_mangle_name;

        // Применение variant необходимо для полей хранения данных, чтобы контролировать их инициализацию
        //        std::variant<std::monostate, void *, torch::Tensor, std::string, std::wstring, std::string_view, std::wstring_view,
        //        std::shared_ptr<Rational>, std::shared_ptr< Iterator<Obj> >, TermPtr> m_var;

        //        std::variant<std::monostate, void *, torch::Tensor, std::string, std::wstring, std::string_view, std::wstring_view> m_var;

        struct NativeData {
            void * ptr;
            int64_t size;
        };

        std::variant < std::monostate, int64_t, double, void *, // None, скаляры и ссылки на функции (нужно различать чистые, обычные и нативные???)
        bool *, int8_t *, int16_t *, int32_t *, int64_t *, float *, double *, NativeData, // Ссылки на нативные скаляры и данные
        std::string, TermPtr, Iterator < Obj>> m_var; //Rational, ModulePtr, 

        //        union {
        //            int64_t m_integer;
        //            double m_number;
        //            void *m_pointer; ///< Содержит указатель на нативную функцию или область памяти с данными или скаляр
        //        };

        std::string m_value; ///< Содержит байтовую строку или байтовый массив с данными для представления в нативном виде (Struct, Unuion, Enum)
        std::wstring m_string; ///< Содержит строку широких символов
        std::shared_ptr<torch::Tensor> m_tensor; ///< Содержит только размерные тензоры (скляры хранятся в поле m_pointer и не создают m_tensor->defined())
        Rational m_rational; ///< Содержит дробь из длинных чисел
        std::shared_ptr<Iterator < Obj>> m_iterator; ///< Итератор для данных
        mutable ObjPtr m_iter_range_value;
        TermPtr m_sequence; ///< Последовательно распарсенных команд для выполнения
        ObjPtr m_return_obj;

        ObjWeak m_reference;
        Sync *m_sync;

        bool m_check_args; //< Проверять аргументы на корректность (для всех видов функций) @ref MakeArgs

        /* Для будущей переделки системы типов и базового класса: 
         * Должен быть интерфейс с поддерживаемыми операциями для стандартных типов данных
         * и набор реализаций для скаляров, строк, тензоров, нативных функций, дробей, внутренних функций и т.д.
         */

        //    SCOPE(protected) :
        bool m_is_const; //< Признак константы (по умолчанию изменения разрешено)
        bool m_is_reference; //< Признак ссылки на объект
    };

    class IntAny : public Obj {
    public:
        IntAny(const ObjPtr value, ObjType type);

        virtual ~IntAny() {
        }
    };

    class IntPlus : public IntAny {
    public:

        IntPlus(const ObjPtr value) : IntAny(value, ObjType::RetPlus) {
        }
    };

    class IntMinus : public IntAny {
    public:

        IntMinus(const ObjPtr value) : IntAny(value, ObjType::RetMinus) {
        }
    };


} // namespace newlang

std::ostream & operator<<(std::ostream &out, newlang::Obj & var);
std::ostream & operator<<(std::ostream &out, newlang::ObjPtr var);


#endif // INCLUDED_OBJECT_H_
