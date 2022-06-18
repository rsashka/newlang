#ifndef INCLUDED_COLLECTION_
#define INCLUDED_COLLECTION_

#include "pch.h"

#include <core/types.h>

namespace newlang {

    template<typename T, typename... Rest>
    struct is_all : std::false_type {
    };

    template<typename T, typename First>
    struct is_all<T, First> : std::is_same<T, First> {
    };

    template<typename T, typename First, typename... Rest>
    struct is_all<T, First, Rest...>
    : std::integral_constant<bool, std::is_same<T, First>::value && is_all<T, Rest...>::value> {
    };

    template<typename T, typename... Rest>
    struct is_any : std::false_type {
    };

    template<typename T, typename First>
    struct is_any<T, First> : std::is_same<T, First> {
    };

    template<typename T, typename First, typename... Rest>
    struct is_any<T, First, Rest...>
    : std::integral_constant<bool, std::is_same<T, First>::value || is_any<T, Rest...>::value> {
    };

    template<class T>
    typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
    almost_equal(T x, T y, int ulp) {
        // the machine epsilon has to be scaled to the magnitude of the values used
        // and multiplied by the desired precision in ULPs (units in the last place)
        return std::fabs(x - y) <= std::numeric_limits<T>::epsilon() * std::fabs(x + y) * ulp
                // unless the result is subnormal
                || std::fabs(x - y) < std::numeric_limits<T>::min();
    }

    /*
     * 
     * 
     */

    /*
     * Шаблон для объектов типа словарь (именованные и не именованный свойства с доступом по индексу и/или имени).
     * Имена свойств у объекта должны быть уникальными (одинаковыми быть не могут).
     * Имена объектов могут быть одинаковыми, т.к. имя хранится внутри объекта.
     * Глобальные объекты имет идентификатром объекта и имя локальной/сессионной переменной, которая хранит на него ссылку.
     * Им объекта и имя переменной, содержащей объект - это разные имена.
     * 
     */

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

    template <typename T>
    class Variable {
    public:
        typedef T Type;
        typedef std::pair<std::string, Type> PairType;
        //    typedef std::pair<std::string, const Type> PairTypeConst;
        typedef std::deque<PairType> DataType;

        class iterator;
        typedef iterator const_iterator;
        //typedef typename _Rep_type::const_iterator            iterator;
        //typedef typename _Rep_type::const_iterator            const_iterator;    

        typedef bool CompareFuncType(const PairType &pair, Variable<T> &args, const void *extra);

        template <typename I>
        typename std::enable_if<std::is_integral<I>::value && !std::is_pointer<I>::value, const Type &>::type
        inline operator[](I index) {
            return at(index).second;
        }

        template <typename N>
        typename std::enable_if<std::is_same<N, std::string>::value || std::is_pointer<N>::value, const Type &>::type
        inline operator[](N name) {
            return at(name).second;
        }

        inline Type & push_back(Type value, const std::string &name = "") {
            return push_back(pair(value, name));
        }

        inline Type & push_back(PairType data) {
            m_data.push_back(data);
            return at(m_data.size() - 1).second;
        }

        inline Type & push_front(Type value, const std::string &name = "") {
            return push_front(pair(value, name));
        }

        inline Type & push_front(PairType pair) {
            m_data.push_front(pair);
            return at(0).second;
        }

        inline void pop_front() {
            m_data.pop_front();
        }

        inline Type top() const {
            if (m_data.empty()) {
                return nullptr;
            }
            return m_data[m_data.size() - 1].second;
        }

        static inline PairType pair(Type value, const std::string name = "") {
            return std::pair<std::string, Type>(name, value);
        }

        inline Type & insert(int64_t index, Type value, const std::string &name = "") {
            if (index < 0 || index >= m_data.size()) {
                LOG_CALLSTACK(std::out_of_range, "Index '%ld' not exists!", index);
            }
            return m_data.insert(m_data.begin() + index, std::pair<std::string, Type>(name, value))->second;
        }

        virtual PairType & at(const int64_t index) {
            if (index >= 0) {
                if (index < m_data.size()) {
                    return m_data.at(index);
                }
            } else {
                if (-index < m_data.size()) {
                    return m_data.at(m_data.size() + index);
                }
            }
            LOG_CALLSTACK(std::out_of_range, "Index '%ld' not exists!", index);
        }

        virtual const PairType & at(const int64_t index) const {
            if (index >= 0) {
                if (index < m_data.size()) {
                    return m_data.at(index);
                }
            } else {
                if (-index < m_data.size()) {
                    return m_data.at(m_data.size() + index);
                }
            }
            LOG_CALLSTACK(std::out_of_range, "Index '%ld' not exists!", index);
        }

        template <typename N>
        typename std::enable_if<std::is_same<char, std::remove_cv<N>>::value, Type &>::type
        inline at(N * name) {
            return at(std::string(name));
        }

        virtual PairType & at(const std::string name) {
            iterator found = select(name);
            if (found != m_data.end()) {
                return found.data();
            }
            LOG_CALLSTACK(std::out_of_range, "Property '%s' not found!", name.c_str());
        }

        //    template <typename N>
        //    typename std::enable_if<std::is_same<char, std::remove_cv<N>>::value, Type &>::type
        //    inline at(N * name) const {
        //        return at(std::string(name));
        //    }
        //
        //    virtual PairType & at(const std::string name) const {
        //        iterator found = select(name);
        //        if (found != m_data.end()) {
        //            return found.data();
        //        }
        //        LOG_CALLSTACK(std::out_of_range, "Property '%s' not found!", name.c_str());
        //    }

        //    virtual const PairType & at(const std::string name) {
        //        iterator found = select(name);
        //        if (found != m_data.end()) {
        //            return found.data();
        //        }
        //        LOG_EXCEPT(std::out_of_range, "Property '%s' not found!", name.c_str());
        //    }

        virtual const std::string & name(const size_t index) const {
            if (index < m_data.size()) {
                return m_data.at(index).first;
            }
            LOG_CALLSTACK(std::out_of_range, "Index '%lu' not exists!", static_cast<unsigned long> (index));
        }

        virtual bool empty() const {
            return m_data.empty();
        }

        virtual void clear_() {
            m_data.clear();
        }

        virtual int64_t size() const {
            return m_data.size();
        }

        virtual int64_t resize(int64_t size, const Type fill, const std::string &name = "") {
            if (size >= 0) {
                // Размер положительный, просто изменить число элементов добавив или удалив последние
                m_data.resize(size, std::pair<std::string, Type>(name, fill));
            } else {
                // Если размер отрицательный - добавить или удалить вначале
                size = -size;
                if (m_data.size() > size) {
                    m_data.erase(m_data.begin(), m_data.begin() + (m_data.size() - size));
                } else if (m_data.size() < size) {
                    m_data.insert(m_data.begin(), (m_data.size() - size), std::pair<std::string, Type>(name, fill));
                } else {
                    m_data.clear();
                }
            }
            return m_data.size();
        }

        /*
         * 
         */
        class iterator {
            friend class Variable<T>;
        public:

            Type &operator*() {
                if (m_found != m_data.end()) {
                    return m_found->second;
                }
                LOG_CALLSTACK(std::out_of_range, "Property '%s' not found  or iterator completed!", m_key.c_str());
            }

            inline const Type &operator*() const {
                return operator*();
            }

            PairType &data() {
                if (m_found != m_data.end()) {
                    return *m_found;
                }
                LOG_CALLSTACK(std::out_of_range, "Property '%s' not found  or iterator completed!", m_key.c_str());
            }

            inline const PairType &data() const {
                return data();
            }

            const iterator &operator++() {
                if (m_found == m_data.end()) {
                    LOG_CALLSTACK(std::out_of_range, "Property '%s' not found  or iterator completed!", m_key.c_str());
                }
                m_found++;
                search_loop();
                return *this;
            }

            inline const const_iterator &operator++() const {
                return operator++();
            }

            iterator operator++(int) {
                if (m_found == m_data.end()) {
                    LOG_CALLSTACK(std::out_of_range, "Property '%s' not found  or iterator completed!", m_key.c_str());
                }
                iterator copy(*this);
                m_found++;
                search_loop();
                return copy;
            }

            //        inline const_iterator operator++(int) const {
            //            return iterator::operator++(int);
            //        }

            inline bool operator==(const typename DataType::iterator &other) const {
                return m_found == other;
            }

            inline bool operator==(const typename DataType::const_iterator &other) const {
                return m_found == other;
            }

            inline bool operator!=(const typename DataType::iterator &other) const {
                return m_found != other;
            }

            inline bool operator!=(const typename DataType::const_iterator &other) const {
                return m_found != other;
            }

            //        inline bool complete() {
            //            return m_found == m_data.end();
            //        }

            inline bool complete() const {
                return m_found == m_data.end();
            }

            inline size_t reset() {
                m_found = m_data.begin();
                search_loop();
                return m_find_key ? -1 : m_data.size();
            }


        protected:

            iterator(DataType &data) : m_data(data), m_find_key(false), m_func(nullptr), m_found(data.begin()) {
                search_loop();
            }

            iterator(DataType &data, const std::string & find_key) : m_data(data), m_find_key(true), m_key(find_key), m_func(nullptr), m_found(data.begin()) {
                search_loop();
            }

            //        iterator(DataType &data, const std::string & find_key) : m_data(data), m_find_key(true), m_key(find_key), m_func(nullptr), m_found(data.begin()) {
            //            search_loop();
            //        }

            iterator(DataType &data, CompareFuncType *func, Variable<T> &arg, void * extra = nullptr) :
            m_data(data), m_find_key(true), m_found(data.begin()), m_func(func), m_func_args(arg), m_func_extra(extra) {
                //            m_func_args.in
                search_loop();
            }

            void search_loop() {
                if (!m_find_key) {
                    return;
                }
                while (m_found != m_data.end()) {
                    if (m_func) {
                        if ((*m_func)(*m_found, m_func_args, m_func_extra)) {
                            return;
                        }
                    } else {
                        if (m_found->first.compare(m_key) == 0) {
                            return;
                        }
                    }
                    m_found++;
                }
            }

        private:
            DataType & m_data;
            const bool m_find_key;
            const std::string m_key;
            typename DataType::iterator m_found;

            const CompareFuncType *m_func;
            Variable<T> m_func_args;
            const void *m_func_extra;
        };

        /* Базовый класс остается открытым, чтобы можно было использовать STL для обработки типовых итераторов в пермеенных.
         * Но для использования в NewLang предназначены вирутальные методы iter, которые для простых объектов соответствуют
         * типовым STL итераторам, а в производных класса (Context и т.д.) могу быть переопределены
         * 
         *      
         */

        virtual typename DataType::iterator begin() {
            return m_data.begin();
        }

        inline typename DataType::const_iterator begin() const {
            return m_data.begin();
        }

        virtual typename DataType::iterator end() {
            return m_data.end();
        }

        inline typename DataType::const_iterator end() const {
            return m_data.end();
        }

        virtual typename DataType::iterator erase(size_t index) {
            if (index < m_data.size()) {
                return m_data.erase(m_data.begin() + index);
            }
            LOG_CALLSTACK(std::out_of_range, "Index '%lu' not exists!", static_cast<unsigned long> (index));
        }

        typename DataType::iterator erase(iterator iter) {
            if (!iter.complete()) {
                return m_data.erase(iter.m_found);
            }
            LOG_CALLSTACK(std::out_of_range, "Try erase end of iter!");
        }

        virtual iterator select() {
            return iterator(m_data);
        }

        //    inline const_iterator select() const {
        //        return select();
        //    }

        virtual iterator select(const std::string & key) {
            return iterator(m_data, key);
        }

        //    virtual iterator select(const std::string & key) const {
        //        return iterator(m_data, key);
        //    }

        inline iterator select(CompareFuncType *func, void * extra = nullptr) {
            Variable<T> args;
            return select(func, args, extra);
        }

        virtual iterator select(CompareFuncType *func, Variable<T> args, void * extra = nullptr) {
            return iterator(m_data, func, args, extra);
        }

        /*
         * 
         * 
         */

        inline static PairType Arg(Type obj, const std::string name = "") {
            return std::pair<std::string, Type>(name, obj);
        }

        virtual ~Variable() {
        }

        /* 
         * Конструкторы для создания списка параметров (при подготовке аргументов перед вызовом функции)
         */
        Variable() {
        }

        Variable(PairType arg) {
            push_front(arg.second, arg.first);
        }

        template <class... A> inline Variable(PairType arg, A... rest) : Variable(rest...) {
            push_front(arg.second, arg.first);
        }

        SCOPE(protected) :
        DataType m_data;


        /**
         * Обновить аргументы по умолчанию реальными значениями по следующим правилам:
         * 1. Изначально объект содержит аргументы по умолчанию.
         * 2. Имена именованныех аргументов должны быть уникальны.
         * 3. Имена передаваемые именованных аргументов должны присутствовать в аргументах по умолчанию.
         * 4. Не именованные аргументы должны передаваться перед именованными.
         * 5. Лимит количества аргументов определнных типов при создании объекта (описан в протитипе функции). 
         */

    };

} // namespace newlang


#endif // INCLUDED_COLLECTION_
