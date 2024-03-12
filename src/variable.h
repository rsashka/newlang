#ifndef INCLUDED_COLLECTION_
#define INCLUDED_COLLECTION_

#include "pch.h"

#include <types.h>

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


    template <typename T> class Iter;
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

    template <typename T, typename PTR = std::shared_ptr<T>>
    class Variable : public std::list<std::pair<std::string, PTR>>
    {
        public:
        typedef PTR Type;
        typedef std::pair<std::string, Type> PairType;
        typedef std::list<PairType> ListType;

        friend class Context;

        template <typename I>
                typename std::enable_if < std::is_integral<I>::value && !std::is_pointer<I>::value, const PairType &>::type
                inline operator[](I index) {
            return at(index);
        }

        template <typename N>
                typename std::enable_if < std::is_same<N, std::string>::value || std::is_pointer<N>::value, const PairType &>::type
                inline operator[](N name) {
            return at(name);
        }

        inline PairType & push_back(const PairType & p) {
            ListType::push_back(p);
            return *at_index(-1);
        }

        inline PairType & push_back(const Type value, const std::string &name = "") {
            return push_back(pair(value, name));
        }

        //        inline PairType top() const {
        //            if (ListType::empty()) {
        //                LOG_RUNTIME("Empty Index '%ld' not exists!", index);
        //            }
        //            return *ListType::back();
        //        }

        static inline PairType pair(const Type value, const std::string name = "") {
            return std::pair<std::string, Type>(name, value);
        }

        virtual PairType & at(const int64_t index) {
            return *at_index(index);
        }

        virtual const PairType & at(const int64_t index) const {
            return *at_index_const(index);
        }

        typename ListType::iterator find(const std::string_view name) {
            auto iter = ListType::begin();
            while (iter != ListType::end()) {
                if (iter->first.compare(name.begin()) == 0) {
                    return iter;
                }
                iter++;
            }
            return ListType::end();
        }

        //        typename ListType::const_iterator find(const std::string_view name) const {
        //            return find(name);
        //        }

        virtual PairType & at(const std::string name) {
            auto iter = find(name);
            if (iter != ListType::end()) {
                return *iter;
            }
            LOG_RUNTIME("Property '%s' not found!", name.c_str());
        }

        virtual const std::string & name(const int64_t index) const {
            return at_index_const(index)->first;
        }

        virtual void clear_() {
            ListType::clear();
        }

        virtual int64_t resize(int64_t new_size, const Type fill, const std::string &name = "") {
            if (new_size >= 0) {
                // Размер положительный, просто изменить число элементов добавив или удалив последние
                ListType::resize(new_size, std::pair<std::string, Type>(name, fill));
            } else {
                // Если размер отрицательный - добавить или удалить вначале
                new_size = -new_size;
                if (static_cast<int64_t> (ListType::size()) > new_size) {

                    ListType::erase(ListType::begin(), at_index(ListType::size() - new_size));

                } else if (static_cast<int64_t> (ListType::size()) < new_size) {
                    ListType::insert(ListType::begin(), (ListType::size() - new_size), std::pair<std::string, Type>(name, fill));
                } else {
                    ListType::clear();
                }
            }
            return ListType::size();
        }

        typename ListType::iterator at_index(const int64_t index) {
            if (index < 0) {
                if (-index <= static_cast<int64_t> (ListType::size())) {
                    int64_t pos = index + 1;
                    typename ListType::iterator iter = ListType::end();
                    while (iter != ListType::begin()) {
                        iter--;
                        if (pos == 0) {
                            return iter;
                        }
                        pos++;
                    }
                }
            } else {
                int64_t pos = 0;
                typename ListType::iterator iter = ListType::begin();
                while (iter != ListType::end()) {
                    if (pos == index) {
                        return iter;
                    }
                    pos++;
                    iter++;
                }
            }
            LOG_RUNTIME("Index '%ld' not exists!", index);
        }

        typename ListType::const_iterator at_index_const(const int64_t index) const {
            if (index < 0) {
                if (-index < static_cast<int64_t> (ListType::size())) {
                    int64_t pos = index + 1;
                    typename ListType::const_iterator iter = ListType::end();
                    while (iter != ListType::begin()) {
                        iter--;
                        if (pos == 0) {
                            return iter;
                        }
                        pos++;
                    }
                }
            } else {
                int64_t pos = 0;
                typename ListType::const_iterator iter = ListType::begin();
                while (iter != ListType::end()) {
                    if (pos == index) {
                        return iter;
                    }
                    pos++;
                    iter++;
                }
            }
            LOG_RUNTIME("Index '%ld' not exists!", index);
        }

        virtual void erase(const int64_t index) {
            ListType::erase(at_index_const(index));
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
    };

} // namespace newlang


#endif // INCLUDED_COLLECTION_
