#ifndef INCLUDED_TYPES_H_
#define INCLUDED_TYPES_H_

#include <map>
#include <set>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <future>
#include <vector>
#include <deque>
#include <iterator>
#include <iomanip>
#include <variant>
#include <algorithm>
#include <stdexcept>
#include <type_traits>
#include <string_view>
#include <cmath>
#include <random>
#include <typeindex>
#include <locale>
#include <codecvt>
#include <functional>
#include <regex>
#include <filesystem>
#include <utility>
#include <cstdlib>
#include <ctime>
#include <complex>

#include <sstream>
#include <iostream>
#include <fstream>
#include <fcntl.h>

#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/limits.h>

#ifdef _MSC_VER

#include <windows.h>
#include <wchar.h>

#else

#include <wait.h>
#include <dlfcn.h>
#include <sys/param.h>
#include <sys/utsname.h>
#include <unistd.h>

#endif


#include "logger.h"

namespace newlang {

    static constexpr const char* ws = " \t\n\r\f\v";

    inline std::string & rtrim(std::string& s, const char* t = ws) {
        s.erase(s.find_last_not_of(t) + 1);
        return s;
    }

    inline std::string & ltrim(std::string& s, const char* t = ws) {
        s.erase(0, s.find_first_not_of(t));
        return s;
    }

    inline std::string & trim(std::string& s, const char* t = ws) {
        return ltrim(rtrim(s, t), t);
    }

    template <typename T>
    T repeat(T str, const std::size_t n) {
        if (n == 0) {
            str.clear();
            str.shrink_to_fit();
            return str;
        } else if (n == 1 || str.empty()) {
            return str;
        }
        const auto period = str.size();
        if (period == 1) {
            str.append(n - 1, str.front());
            return str;
        }
        str.reserve(period * n);
        std::size_t m{2};
        for (; m < n; m *= 2) str += str;
        str.append(str.c_str(), (n - (m / 2)) * period);
        return str;
    }

    template<class T>
    T BaseFileName(T const & path, T const & delims = "/\\") {
        return path.substr(path.find_last_of(delims) + 1);
    }

    template<class T>
    T RemoveFileExtension(T const & filename) {
        typename T::size_type const p(filename.find_last_of('.'));
        return p > 0 && p != T::npos ? filename.substr(0, p) : filename;
    }

    
typedef std::vector<std::string> StringArray;

class Term;
class Obj;
class Context;
class Module;
class JIT;

class IntAny;
class IntPlus;
class IntMinus;

namespace runtime {
    class  Buildin;
}

class Macro;
class Parser;
class RunTime;
class Diag;

typedef std::shared_ptr<Obj> ObjPtr;
typedef std::shared_ptr<const Obj> ObjPtrConst;
typedef std::weak_ptr<Obj> ObjWeak;
typedef std::weak_ptr<const Obj> ObjWeakConst;

/*
 * RunTime - класс синглетон для процесса. В нем происходит загрузка модулей, взаимодействие 
 * со средой выполнения. В нем находится таблица имен для всего приложения Named (weak_ptr)
 * Module - класс для одного модуля. Один файл - один модуль. Являеся владельцем объектов данного модуля (shared_ptr)
 * Context - класс локальных переменных выполнения одного потока.
 * 
 * val := 123; # Обычная переменная - владелец объекта без возможности создания ссылки на объект
 * т.е. val2 := &val; # Ошибка !!!!
 * & ref := & 123; # Переменная - владелец объекта с возможностью создать ссылку в текущем потоке
 * т.е. ref2 := &ref; # ОК, но только в рамках одного потока !!!!
 * && ref_mt := && 123; # Переменная - владелец объекта с возможностью создать ссылку в одном потоке без синхронизации
 * в разных потоках приложения (с межпотоковой синхронизацией) т.е. ref_mt2 := &&ref_mt; # ОК !!!!

 * 
 */

/**
 * Типы ссылочных объектов
 */
    enum class RefType : uint8_t {
        None = 0, // Без ссылок
        LiteSingle = 1, // Легкая ссылка для одного потока (без синхронизации)  &
        LiteThread = 2, // Легкая многопоточная ссылка (внешняя синхронизация with) &? 
        SyncMono = 3, // Объект синхронизации с монопольным мьютексом &&
        SyncMulti = 4, // Объект синхронизации с рекурсивным мьютексом &*

        LiteSingleConst = 5,
        LiteThreadConst = 6,
        SyncMonoConst = 7,
        SyncMultiConst = 8,
    };

    inline RefType RefTypeFromString(const std::string_view text){
        if(text.empty()){
            return RefType::None;
        } else if(text.compare("&")==0){
            return RefType::LiteSingle;
        } else if(text.compare("&^")==0){
            return RefType::LiteSingleConst;
        } else if(text.compare("&?")==0){
            return RefType::LiteThread;
        } else if(text.compare("&?^")==0){
            return RefType::LiteThreadConst;
        } else if(text.compare("&*")==0){
            return RefType::SyncMulti;
        } else if(text.compare("&*^")==0){
            return RefType::SyncMultiConst;
        } else if(text.compare("&&")==0){
            return RefType::SyncMono;
        } else if(text.compare("&&^")==0){
            return RefType::SyncMonoConst;
        }
        LOG_RUNTIME("Unknown reference type '%s'!", text.begin());
    }
    
    inline bool isLiteRef(RefType type){
        return  type == RefType::LiteSingle || type == RefType::LiteSingleConst || type == RefType::LiteThread || type == RefType::LiteThreadConst;
    }
    inline bool isLiteSyncRef(RefType type){
        return  type == RefType::LiteThread || type == RefType::LiteThreadConst;
    }
    inline bool isHeavyRef(RefType type){
        return  type == RefType::SyncMono || type == RefType::SyncMonoConst || type == RefType::SyncMulti || type == RefType::SyncMultiConst;
    }
    inline bool isConstRef(RefType type){
        return  type == RefType::LiteSingleConst || type == RefType::LiteThreadConst || type == RefType::SyncMonoConst || type == RefType::SyncMultiConst;
    }
    inline bool isEditableRef(RefType type){
        return  type == RefType::LiteSingle || type == RefType::LiteThread || type == RefType::SyncMono || type == RefType::SyncMulti;
    }
    inline bool isValidReference(RefType type, RefType test){
        switch(type){
            case RefType::None:
                return test == RefType::None;
            case RefType::LiteSingle:
                return test == RefType::LiteSingle || test == RefType::LiteSingleConst;
            case RefType::LiteSingleConst:
                return test == RefType::LiteSingleConst;
            case RefType::LiteThread:
                return test == RefType::LiteThread || test == RefType::LiteThreadConst;
            case RefType::LiteThreadConst:
                return test == RefType::LiteThreadConst;
            case RefType::SyncMono:
                return test == RefType::SyncMono || test == RefType::SyncMonoConst;
            case RefType::SyncMonoConst:
                return test == RefType::SyncMonoConst;
            case RefType::SyncMulti:
                return test == RefType::SyncMulti || test == RefType::SyncMultiConst;
            case RefType::SyncMultiConst:
                return test == RefType::SyncMultiConst;
        }
        LOG_RUNTIME("Unknown reference type code! (%d)", static_cast<int>(type));
    }

/**
 * Интерфейс класса захвата/освобождения объекта межпотоковой синхронизации
 * Сильная ссылка только одна - остальные могут быть слабыми или сильными в локальных переменных,
 * но с захватом и последующим освобождением объекта синхронизации.
 */
class Sync  {
protected:
    friend class Reference;
    friend class Taken;
    const RefType m_type;
    const std::thread::id m_thread_id;
    const std::chrono::milliseconds & m_timeout;
    mutable std::variant<std::monostate, std::unique_ptr<std::shared_timed_mutex>, std::unique_ptr<std::recursive_timed_mutex>> m_sync;
public:
    
    static constexpr std::chrono::milliseconds SyncTimeoutDeedlock = std::chrono::milliseconds(5000);
    
    Sync(RefType type, const std::chrono::milliseconds & timeout_duration=SyncTimeoutDeedlock): 
    m_type(type), m_thread_id(std::this_thread::get_id()), m_timeout(timeout_duration), m_sync(std::monostate()) {
        ASSERT(m_type != RefType::None);
        if(type == RefType::LiteSingle || type == RefType::LiteSingleConst || type == RefType::LiteThread || type == RefType::LiteThreadConst){
            ASSERT(std::holds_alternative<std::monostate>(m_sync));
        } else if(type == RefType::SyncMono || type == RefType::SyncMonoConst){
            m_sync = std::unique_ptr<std::shared_timed_mutex>(new std::shared_timed_mutex());
        } else if(type == RefType::SyncMulti || type == RefType::SyncMultiConst){
            m_sync = std::unique_ptr<std::recursive_timed_mutex>(new std::recursive_timed_mutex());
        } else {
            LOG_RUNTIME("Unknown synchronization type! (%d)", static_cast<int>(type));
        }
    }
    
    inline RefType GetRefType() const { 
        return m_type;
    }
    
    inline std::thread::id GetThreadId() const { 
        return m_thread_id;
    }
    
    inline bool SyncLock(bool edit_mode=true, const std::chrono::milliseconds & timeout_duration=Sync::SyncTimeoutDeedlock){
        if(m_type == RefType::LiteSingle || m_type == RefType::LiteSingleConst){
            if(m_thread_id != std::this_thread::get_id()){
                LOG_RUNTIME("Calling a function on another thread!");
            }
        } else if(m_type == RefType::SyncMono || m_type == RefType::SyncMonoConst){
            ASSERT(std::holds_alternative<std::unique_ptr<std::shared_timed_mutex>>(m_sync));
            if(edit_mode){
                return std::get<std::unique_ptr<std::shared_timed_mutex>>(m_sync)->try_lock_for(&timeout_duration == &Sync::SyncTimeoutDeedlock ? m_timeout:timeout_duration);
            } else {
                return std::get<std::unique_ptr<std::shared_timed_mutex>>(m_sync)->try_lock_shared_for(&timeout_duration == &Sync::SyncTimeoutDeedlock ? m_timeout:timeout_duration);
            }
        } else if(m_type == RefType::SyncMulti || m_type == RefType::SyncMultiConst){
            ASSERT(std::holds_alternative<std::unique_ptr<std::recursive_timed_mutex>>(m_sync));
            return std::get<std::unique_ptr<std::recursive_timed_mutex>>(m_sync)->try_lock_for(&timeout_duration == &Sync::SyncTimeoutDeedlock ? m_timeout:timeout_duration);
        }
        return true;
    }
    
    inline bool SyncUnLock(){
        ASSERT(m_type != RefType::None);
        if(m_type == RefType::LiteSingle || m_type == RefType::LiteSingleConst){
            ASSERT(m_thread_id == std::this_thread::get_id());
        } else if(m_type == RefType::SyncMono || m_type == RefType::SyncMonoConst){
            ASSERT(std::holds_alternative<std::unique_ptr<std::shared_timed_mutex>>(m_sync));
            std::get<std::unique_ptr<std::shared_timed_mutex>>(m_sync)->unlock();
        } else if(m_type == RefType::SyncMulti || m_type == RefType::SyncMultiConst){
            ASSERT(std::holds_alternative<std::unique_ptr<std::recursive_timed_mutex>>(m_sync));
            std::get<std::unique_ptr<std::recursive_timed_mutex>>(m_sync)->unlock();;
        }
        return true;
    }
    virtual ~Sync(){
    }
};    


class Taken  {
SCOPE(protected):
    ObjPtr m_obj;
    Sync *m_sync;
    bool m_is_locked;
public:
    
    Taken(ObjWeak obj, Sync *sync, bool edit_mode=false, const std::chrono::milliseconds & timeout_duration=Sync::SyncTimeoutDeedlock):
        m_obj(nullptr), m_sync(sync), m_is_locked(false){
        if(!m_sync || (m_is_locked = m_sync->SyncLock(edit_mode, timeout_duration))){
           m_obj =  obj.lock();
        }
    }
    
    inline operator bool() const {
        return !!m_obj;
    }

    inline Obj & operator *(){
        ASSERT(m_obj);
        return *m_obj;
    }
    
    ~Taken(){
        if(m_sync && m_is_locked){
           m_sync->SyncUnLock();
        }
    }
};

    
typedef std::shared_ptr<Term> TermPtr;
typedef std::shared_ptr<Module> ModulePtr;
typedef std::shared_ptr<runtime::Buildin> BuildinPtr;


//typedef std::shared_ptr<Obj> ObjPtr;
//typedef std::shared_ptr<const Obj> ObjPtrConst;
//
//void SyncUnLock(ObjPtr &obj);
//typedef std::unique_ptr<ObjPtr, decltype(&SyncUnLock)> LocalObj;
//inline LocalPtr MakeLocalPtr(ObjPtr &obj){
//    return std::move(LocalPtr(obj, &SyncUnLock));
//}

typedef std::vector<TermPtr> BlockType;
typedef std::vector<TermPtr> ArrayTermType;
typedef std::vector<ObjPtr> ArrayObjType;

typedef std::shared_ptr<RunTime> RuntimePtr;
typedef std::shared_ptr<Diag> DiagPtr;
typedef std::shared_ptr<Macro> MacroPtr;
typedef std::shared_ptr<Parser> ParserPtr;
typedef std::shared_ptr<Context> RunnerPtr;

typedef ObjPtr FunctionType(Context *ctx, Obj &in);

typedef std::variant<std::monostate, ObjWeak, std::vector < ObjWeak> > WeakItem;

typedef std::weak_ptr<Term> TermWeak;
typedef std::variant<std::monostate, TermWeak, std::vector < TermWeak> > TermWeakItem;

typedef ObjPtr(*EvalFunction)(Context *ctx, const TermPtr & term, Obj * args, bool eval_block);

    typedef std::shared_ptr<std::string> SourceType;
    typedef std::vector<std::string> PostLexerType;


void NewLangSignalHandler(int signal);


//struct Docs;
//typedef std::shared_ptr<Docs> DocPtr;
//
//struct Docs {
//    
//    static std::multimap<std::string, DocPtr> m_docs;
//    
//    std::string index;
//    std::string text;
//
//    Docs(std::string body) {
//        size_t pos = body.find("\n");
//        size_t pos2 = body.find("\n\n");
//        if (pos != std::string::npos && pos == pos2) {
//            // Индексируемый заголовок должен быть первой строкой и отделаться пустой строкой от остального текста
//            index = body.substr(0, pos);
//            text = body.substr(pos + 2);
//        } else {
//            text = body;
//        }
//    }
//
//    static DocPtr Build(std::string body) {
//        return std::make_shared<Docs>(body);
//    }
//    
//    static DocPtr Append(std::string body, const std::string func=""){
//        DocPtr result = Build(body);
//        m_docs.insert(std::pair<std::string, DocPtr>(func,result));
//        return result;
//    }
//};
//

#ifdef __GNUC__
std::string ParserMessage(std::string &buffer, int row, int col, const char *format, ...)
    __attribute__((format(printf, 4, 5)));
#else

#endif

void ParserException(const char *msg, std::string &buffer, int row, int col);

#ifdef BUILD_UNITTEST
#define LOG_LEVEL_ERROR_MESSAGE  LOG_LEVEL_DEBUG
#else
#define LOG_LEVEL_ERROR_MESSAGE  LOG_LEVEL_INFO
#endif

#define NL_PARSER(term, format, ...)                                                                                   \
    do {                                                                                                               \
        std::string empty;                                                                                             \
        std::string message =                                                                                          \
            newlang::ParserMessage(term->m_source ? *term->m_source : empty, term->m_line, term->m_col, format, ##__VA_ARGS__); \
        LOG_THROW(ParserError, LOG_LEVEL_ERROR_MESSAGE, "", "%s", message.c_str());                                 \
    } while (0)

#define NL_MESSAGE(level, term, format, ...)                                                                                   \
    do {                                                                                                               \
        std::string empty;                                                                                             \
        std::string message =                                                                                          \
            newlang::ParserMessage((term && term->m_source) ? *term->m_source : empty, term?term->m_line:1, term?term->m_col:0, format, ##__VA_ARGS__); \
        LOG_MAKE(level, "", "%s", message.c_str());                                 \
    } while (0)

#define NL_CHECK(cond, format, ...)                                                                                    \
    do {                                                                                                               \
        if (!(cond)) {                                                                                                 \
            LOG_THROW(ParserError, LOG_LEVEL_ERROR_MESSAGE, "", format, ##__VA_ARGS__);                             \
        }                                                                                                              \
    } while (0)

#define NL_TYPECHECK(term, from, to)                                                                                   \
    do {                                                                                                               \
        if (!canCast(from, to)) {                                                                                      \
            std::string message = "Incompatible data type '";                                                          \
            message += newlang::toString(from);                                                        \
            message += "' and '";                                                                                      \
            message += newlang::toString(to);                                                          \
            message += "' (" __FILE__ ":" TO_STR(__LINE__) ")";                                                        \
            LOG_THROW(                                                                                          \
                ParserError, LOG_LEVEL_ERROR_MESSAGE, "", "%s",                                                            \
                newlang::ParserMessage(*term->m_source, term->m_line, term->m_col, "%s", message.c_str()).c_str());             \
        }                                                                                                              \
    } while (0)

//// Типы объектов (пустой, скаляр, тензор (массив), строка, словарь, функция, класс, диапазон, ошибка
//// пустой тип - не инициализаированный тип, совместимый с любых другим значением
//// скаляр (тензор нулевой размерности) - целые числа, числа с плавающей запятой, комплектыне числа, булевые значения
//// строка - последовательность символов в байтовом или символьном представлении
//// тензор (массивы произвольной размерности одного типа данных) - элементы хранятся последовательно в упакованном виде
//// функция - выполняемый тип данных (обычная, чистая, нейросеть)
//// словарь - массив произвольных типов с именованными элементами - элементы хранятся в виде отдельных объектов
//// класс - словарь с поддержкой иерархии наследования свойств
//// диапазон - счетчик/срез для указания неизменяемой последовательности чисел для индексов или счетчиков
//// ошибка - информация о состоянии ошибки приложения

/* Синонимы типов требуются для точного указания типа при импорте С++ функций, т.к. mangling name для них отличаются*/
#define NL_TYPES(_)         \
    _(None, 0)              \
    \
    _(Bool, 1)              \
    _(Int8, 2)              \
    _(Char, 3)              /* signed char*/ \
    _(Byte, 4)             /* unsigned char*/ \
    _(Int16, 5)            /*short*/ \
    _(Word, 6)            /*unsigned short*/ \
    _(Int32, 7)            /*int*/ \
    _(DWord, 8)             /*unsigned int*/ \
    _(Int64, 9)            /*long*/ \
    _(DWord64, 10)           /*unsigned long*/ \
    _(Integer, 15)          \
    \
    _(Float16, 16)          \
    _(Float32, 17)          \
    _(Single,  18)          \
    _(Float64, 19)          \
    _(Double, 20)          \
    _(Number, 24)           \
    \
    _(Complex16, 25)     \
    _(Complex32, 26)     \
    _(Complex64, 27)    \
    _(Complex, 31)          \
                            \
    _(Tensor, 32)           \
                            \
    _(Rational, 33)         \
    \
    _(Arithmetic, 47)       \
    \
    _(StrChar, 48)          \
    _(StrWide, 49)          \
    _(FmtChar, 50)           \
    _(FmtWide, 51)           \
    _(ViewChar, 52)         \
    _(ViewWide, 53)         \
    _(StdChar, 54)         \
    _(StdWide, 55)         \
    _(PtrChar, 56)         \
    _(PtrWide, 57)         \
    _(String, 59)           \
    \
    _(Iterator, 62)         \
    _(IteratorEnd, 63)      \
    \
    _(Pointer, 64)          \
    _(NativeFunc, 65)       \
    _(Function, 70)        \
    _(PureFunc, 71)        \
    \
    _(Thread, 80)        \
    _(Base, 84)        \
    _(Sys, 85)        \
    _(TorchTensor, 86)        \
    \
    _(Range, 104)           \
    _(Dictionary, 105)      \
    _(Interface, 106)      \
    _(Class, 107)           \
    _(Ellipsis, 108)        \
    _(EVAL_FUNCTION, 110)   \
    \
    _(BLOCK, 111)           \
    _(BLOCK_TRY, 112)       \
    _(BLOCK_PLUS, 113)       \
    _(BLOCK_MINUS, 114)       \
    _(Macro, 115)       \
    \
    _(Reference, 117)          \
    _(Virtual, 118)            \
    _(Eval, 119)            \
    _(Other, 120)           \
    _(Plain, 121)           \
    _(Object, 122)          \
    _(Any, 123)             \
    \
    _(Type, 200)            \
/*    _(Void, 201)          */\
    _(Struct, 202)          \
    _(Union, 203)           \
    _(Enum, 204)            \
    \
    _(RetPlus, 210)         \
    _(RetMinus, 211)        \
    _(RetRepeat, 212)        \
    _(IntParser, 213)       \
    _(IntError, 214)        \
    \
    _(Context, 227)          \
    _(Module, 228)          \
    _(Undefined, 229)          \
    _(Error, 230)          \
    _(Break, 231)           \
    _(Continue, 232)        \
    _(ErrorParser, 241)     \
    _(ErrorRunTime, 242)    \
    _(ErrorSignal, 243)

    /*
     * Типы данных различаются:
     * Пусто:
     * - Тип данные Any и значение None (toType(None))
     * - Значение None "_" (подчерк) может быть у другних типов. т.е.
     *    у (не инициализированных) переменных, при попытке чтения значения которых возникает ошибка. (clear_())
     * Числа:
     * - общий тип (булевый, целое, с плавающей точкой, комплексное)
     * - размером представления в памяти (1,2,4,8,16 и т.д. байт)
     * - может быть изменяемым / не изменяемым (если тип указан явно)
     * - местом хранения (тензор/ссылка на последовательность байт)
     * Строки:
     * - Обычные (UTF8), тип данных Int8
     * - Широкие (WIDE), тип данных wchar_t
     * - Нативные обычные ViewChar
     * - Нативные широкие ViewWide
     * Функции:
     * - Обычные
     * - Чистые (в том числе простые)
     * - Нативные (ссылка на функцию с определенными параметрами, типом вызова и возвращаемым значением)
     * Словари:
     * - Словарь (одноуровненывый список с доступом по индексу и возможным именем элемента)
     * - Класс (обязательное имя у каждого элемента и хранение информации о родителях)
     * Специальные:
     * - Ошибка
     * - Диапазон
     * - Многоточие
     * - Адрес (указатель)
     *
     * Нативные строки, массивы и функции можно регистрировать в стандартных типах данных, т.к. интерфейс работы с ними
     * реализуется объектом. Все остальные типы данных требуют реализации специфических методом, которые нельзя определить
     * заранее сразу для всех, но можно реализовывать кастомные классы с данными, специфическими для конкретных объектов
     * С++.
     *
     *
     *
     *
     * Автоматическое приведение типов в выражениях происходит по следующим правилам.
     * - Если тип указан явно, то он не может быть изменен
     * - Определяется общий тип (Bool, Integer, Float, Complex).
     * - Если тип литерала явно не указан, то выбирается минимальный байтовый размер для общего типа.
     * - В выражениях тензор-скаляр, тензором должен быть самый левый элемент выражения.
     * - Итоговым типом поседовательности выражений является тип первого вычисленного элемента.
     * - В операторах присвоения вычисление типа происходит дважды, сперва для правой части выражения, а потом для оператора
     * присовения.
     * - Тип меньшего размера может автоматически приводится к типу большему размеру или к более сложному типу
     *    bool -> char -> short -> int -> long -> float -> double -> Complex32 -> Complex64
     *    var := 1.0 + 2; // float
     *    var := 1 + 1000; // Int16 т.к. первый тип изменяемый
     *    var := 1:Int8 + 1000; // Ошибка в выражении т.к. первый тип не изменяемый  Int16 -x-> Int8
     *    var:Int8 := 1 + 1000; // Ошибка в присвоении т.к. тип не изменяемый  Int16 -x-> Int8
     *    var := 1000 + 2; // Int16
     *    var := 1000 + 2.0; // Ошибка float -> short, но может быть var := 1000.0 + 2;
     *    var := 1:Bool + 2; // Ошибка byte -> bool, но может быть var := 2 + 1:Bool;
     *
     * - Итоговым типом поседовательности выражений является тип первого вычисленого элемента.
     *      var := 1.0 + 2; // float  var := 1000 + 2; // Int16  var := 1000 + 2.0;
     * -
     * АПриведение ра
     * Совместимость типов данныъ между собой определяется по следующему принципу.
     * Если данные
     *
     */

    enum class ObjType : uint8_t {
#define DEFINE_ENUM(name, value) name = static_cast<uint8_t>(value),
        NL_TYPES(DEFINE_ENUM)
#undef DEFINE_ENUM
    };

#define MAKE_TYPE_NAME(type_name)  type_name

    inline const char *toString(ObjType type) {
#define DEFINE_CASE(name, _)                                                                                           \
    case ObjType::name:                                                                                                \
        return MAKE_TYPE_NAME(":" #name);

        switch (type) {
                NL_TYPES(DEFINE_CASE)
            default:
                LOG_RUNTIME("UNKNOWN type code %d", static_cast<int> (type));
        }
#undef DEFINE_CASE
#undef MAKE_TYPE_NAME
    }

    /*
     * https://en.cppreference.com/w/cpp/language/types
     */

    inline const char *toCXXType(ObjType type, bool int_bool) {
        switch (type) {
            case ObjType::None:
                return "void";

            case ObjType::Bool:
                return int_bool ? "int" : "bool";
            case ObjType::Int8:
                return "char";
            case ObjType::Char:
                return "signed char";
            case ObjType::Byte:
                return "unsigned char";
            case ObjType::Int16:
                return "signed short";
            case ObjType::Word:
                return "unsigned short";
            case ObjType::Int32:
                return "signed int";
            case ObjType::DWord:
                return "unsigned int";
            case ObjType::Int64:
                return "signed long long int";
            case ObjType::DWord64:
                return "unsigned long long int";

            case ObjType::Float32:
            case ObjType::Single:
                return "float";
            case ObjType::Float64:
            case ObjType::Double:
                return "double";
        }
        LOG_RUNTIME("NewLang type '%s'(%d) can't be represented by C++ type!", toString(type), static_cast<int> (type));
    }

    ObjType GetBaseTypeFromString(const std::string_view type_arg, bool *has_error = nullptr);

    /*
     *    C++     NewLang
     *      *       &*
     *      &       &
     *      &&      &&
     *  const *     &*^
     *  const &     &^
     *  const &&    &&^
     *      **      &?      ????????????????????????
     *  const **    &?^     ????????????????????????
     */
    inline const char *toCXXRef(const std::string_view ref) {
        if (ref.compare("&*") == 0) {
            return "*";
        } else if (ref.compare("&") == 0) {
            return "&";
        } else if (ref.compare("&&") == 0) {
            return "&&";
        } else if (ref.compare("&?") == 0) {
            return "**";

        } else if (ref.compare("&*^") == 0) {
            return "const *";
        } else if (ref.compare("&^") == 0) {
            return "const &";
        } else if (ref.compare("&&^") == 0) {
            return "const &&";
        } else if (ref.compare("&?^") == 0) {
            return "const **";
        }
        LOG_RUNTIME("Unknown reference type '%s'!", ref.begin());
    }
    // Обобщенные типы данных

    inline bool isGenericType(ObjType t) {
        switch (t) {
            case ObjType::Integer: // Любое ЦЕЛОЕ число включая логический тип
            case ObjType::Number: // Любое число с ПЛАВАЮЩЕЙ ТОЧКОЙ
            case ObjType::Complex: // Любое КОМПЛЕКСНОЕ число
            case ObjType::Tensor: // Любое число в виде тензора (включая логический тип)
            case ObjType::Arithmetic: // Все числа, включая длинные, дроби и денежный формат
            case ObjType::String: // Строка любого типа
            case ObjType::Object: // Любой объект (Class или Dictionary)
            case ObjType::Any: // Любой тип кроме None
            case ObjType::Plain: // Любой тип для машинного представления (Flat Raw ?)
            case ObjType::Other: // Специальные типы (многоточие, диапазон)
            case ObjType::Eval: // Код для выполнения ?????
                return true;
            default:
                return false;
        }
    }

    inline bool isNativeType(ObjType t) {
        switch (t) {
            case ObjType::Bool:
            case ObjType::Int8:
            case ObjType::Char:
            case ObjType::Byte:
            case ObjType::Int16:
            case ObjType::Word:
            case ObjType::Int32:
            case ObjType::DWord:
            case ObjType::Int64:
            case ObjType::DWord64:
            case ObjType::Float32:
            case ObjType::Float64:
            case ObjType::Single:
            case ObjType::Double:
            case ObjType::Pointer:

                //            case ObjType::String:
            case ObjType::StrChar:
            case ObjType::FmtChar:
            case ObjType::StrWide:
            case ObjType::FmtWide:
                return true;
            default:
                return false;
        }
    }

    inline bool isObjectType(ObjType t) {
        return t == ObjType::Dictionary || t == ObjType::Interface || t == ObjType::Class;
    }

    bool isDefaultType(const TermPtr & term);
    const TermPtr getDefaultType(const std::string_view text);
    const TermPtr getDefaultType(ObjType type);
    const TermPtr getNoneTerm();
    const TermPtr getEllipsysTerm();
    const TermPtr getRequiredTerm();
    const ObjPtr getNoneObj();
    const ObjPtr getEllipsysObj();

    inline bool isBaseType(ObjType t) {
        return t == ObjType::Bool || t == ObjType::Int8 || t == ObjType::Int16 || t == ObjType::Int32
                || t == ObjType::Int64 || t == ObjType::Float32 || t == ObjType::Float64
                || t == ObjType::Char || t == ObjType::Byte || t == ObjType::Word
                || t == ObjType::DWord || t == ObjType::DWord64
                || t == ObjType::Single || t == ObjType::Double;
    }

    inline bool isFunction(ObjType t) {
        return t == ObjType::PureFunc || t == ObjType::Function || t == ObjType::NativeFunc ||
                t == ObjType::EVAL_FUNCTION || t == ObjType::PureFunc || t == ObjType::Virtual;
    }

    inline bool isNative(ObjType t) {
        return t == ObjType::NativeFunc || t == ObjType::NativeFunc;
    }

    inline bool isEval(ObjType t) {
        return t == ObjType::BLOCK || t == ObjType::BLOCK_TRY;
    }

    inline bool isSimpleType(ObjType t) {
        return static_cast<uint8_t> (t) && static_cast<uint8_t> (t) <= static_cast<uint8_t> (ObjType::Tensor);
    }

    inline bool isIntegralType(ObjType t, bool includeBool) {
        return (static_cast<uint8_t> (t) > static_cast<uint8_t> (ObjType::Bool) &&
                static_cast<uint8_t> (t) <= static_cast<uint8_t> (ObjType::Integer)) ||
                (includeBool && t == ObjType::Bool);
    }

    inline bool isFloatingType(ObjType t) {
        return t == ObjType::Single || t == ObjType::Double || t == ObjType::Float16 || t == ObjType::Float32 || t == ObjType::Float64 || t == ObjType::Number;
    }

    inline bool isComplexType(ObjType t) {
        return t == ObjType::Complex16 || t == ObjType::Complex32 || t == ObjType::Complex64 || t == ObjType::Complex;
    }

    inline bool isTensor(ObjType t) {
        return static_cast<uint8_t> (t) && static_cast<uint8_t> (t) <= static_cast<uint8_t> (ObjType::Tensor);
    }

    inline bool isBooleanType(ObjType t) {
        return t == ObjType::Bool;
    }

    inline bool isArithmeticType(ObjType t) {
        // Арифметический тип данных - все числа, включая логический тип
        return static_cast<uint8_t> (t) >= static_cast<uint8_t> (ObjType::Bool) &&
                static_cast<uint8_t> (t) <= static_cast<uint8_t> (ObjType::Arithmetic);
    }

    inline bool isStringChar(ObjType t) {
        return t == ObjType::StrChar || t == ObjType::FmtChar || t == ObjType::ViewChar;
    }

    inline bool isStringWide(ObjType t) {
        return t == ObjType::StrWide || t == ObjType::ViewWide || t == ObjType::FmtWide;
    }

    inline bool isString(ObjType t) {
        return isStringChar(t) || isStringWide(t) || t == ObjType::String;
    }

    inline bool isInterrupt(ObjType t) {
        return t == ObjType::RetPlus || t == ObjType::RetMinus || t == ObjType::RetRepeat;
    }

    inline bool isPlainDataType(ObjType t) {
        return isTensor(t) || isString(t) || t == ObjType::Struct || t == ObjType::Enum || t == ObjType::Union;
    }

    inline bool isDictionary(ObjType t) {
        return t == ObjType::Dictionary || t == ObjType::Class; // ObjType::Interface - не имеет полей данных, т.е. не словарь
    }

    inline bool isClass(ObjType t) {
        return t == ObjType::Class || t == ObjType::Interface;
    }

    inline bool isEllipsis(ObjType t) {
        return t == ObjType::Ellipsis;
    }

    inline bool isRange(ObjType t) {
        return t == ObjType::Range;
    }

    inline bool isTypeName(ObjType t) {
        return t == ObjType::Type;
    }

    inline bool isModule(ObjType t) {
        return t == ObjType::Module;
    }

    inline bool isIndexingType(ObjType curr, ObjType fix) {
        return isTensor(curr) || isString(curr) || isDictionary(curr) || isFunction(curr) || isModule(curr) || (isTypeName(curr) && isIndexingType(fix, fix));
    }

    inline bool isLocalType(ObjType) {
        return false;
    }

    //#define AT_FORALL_SCALAR_TYPES_WITH_COMPLEX_AND_QINTS(_) \
//  _(uint8_t, Byte) /* 0 */                               \
//  _(int8_t, Char) /* 1 */                                \
//  _(int16_t, Short) /* 2 */                              \
//  _(int, Int) /* 3 */                                    \
//  _(int64_t, Long) /* 4 */                               \
//  _(at::Half, Half) /* 5 */                              \
//  _(float, Float) /* 6 */                                \
//  _(double, Double) /* 7 */                              \
//  _(c10::complex<c10::Half>, ComplexHalf) /* 8 */        \
//  _(c10::complex<float>, ComplexFloat) /* 9 */           \
//  _(c10::complex<double>, ComplexDouble) /* 10 */        \
//  _(bool, Bool) /* 11 */                                 \
//  _(c10::qint8, QInt8) /* 12 */                          \
//  _(c10::quint8, QUInt8) /* 13 */                        \
//  _(c10::qint32, QInt32) /* 14 */                        \
//  _(at::BFloat16, BFloat16) /* 15 */                     \
//  _(c10::quint4x2, QUInt4x2) /* 16 */                    \
//  _(c10::quint2x4, QUInt2x4) /* 17 */                    \
//  _(c10::bits1x8, Bits1x8) /* 18 */                      \
//  _(c10::bits2x4, Bits2x4) /* 19 */                      \
//  _(c10::bits4x2, Bits4x2) /* 20 */                      \
//  _(c10::bits8, Bits8) /* 21 */                          \
//  _(c10::bits16, Bits16) /* 22 */                        \
//  _(c10::Float8_e5m2, Float8_e5m2) /* 23 */              \
//  _(c10::Float8_e4m3fn, Float8_e4m3fn) /* 24 */

    //    
    //  switch (scalarType) {
    //    case at::ScalarType::Byte:
    //      // no "byte" because byte is signed in numpy and we overload
    //      // byte to mean bool often
    //      return std::make_pair("uint8", "");
    //    case at::ScalarType::Char:
    //      // no "char" because it is not consistently signed or unsigned; we want
    //      // to move to int8
    //      return std::make_pair("int8", "");
    //    case at::ScalarType::Double:
    //      return std::make_pair("float64", "double");
    //    case at::ScalarType::Float:
    //      return std::make_pair("float32", "float");
    //    case at::ScalarType::Int:
    //      return std::make_pair("int32", "int");
    //    case at::ScalarType::Long:
    //      return std::make_pair("int64", "long");
    //    case at::ScalarType::Short:
    //      return std::make_pair("int16", "short");
    //    case at::ScalarType::Half:
    //      return std::make_pair("float16", "half");
    //    case at::ScalarType::ComplexHalf:
    //      return std::make_pair("complex32", "chalf");
    //    case at::ScalarType::ComplexFloat:
    //      return std::make_pair("complex64", "cfloat");
    //    case at::ScalarType::ComplexDouble:
    //      return std::make_pair("complex128", "cdouble");
    //    case at::ScalarType::Bool:
    //      return std::make_pair("bool", "");
    //    case at::ScalarType::QInt8:
    //      return std::make_pair("qint8", "");
    //    case at::ScalarType::QUInt8:
    //      return std::make_pair("quint8", "");
    //    case at::ScalarType::QInt32:
    //      return std::make_pair("qint32", "");
    //    case at::ScalarType::BFloat16:
    //      return std::make_pair("bfloat16", "");
    //    case at::ScalarType::QUInt4x2:
    //      return std::make_pair("quint4x2", "");
    //    case at::ScalarType::QUInt2x4:
    //      return std::make_pair("quint2x4", "");
    //    case at::ScalarType::Bits1x8:
    //      return std::make_pair("bits1x8", "");
    //    case at::ScalarType::Bits2x4:
    //      return std::make_pair("bits2x4", "");
    //    case at::ScalarType::Bits4x2:
    //      return std::make_pair("bits4x2", "");
    //    case at::ScalarType::Bits8:
    //      return std::make_pair("bits8", "");
    //    case at::ScalarType::Bits16:
    //      return std::make_pair("bits16", "");
    //    case at::ScalarType::Float8_e5m2:
    //      return std::make_pair("float8_e5m2", "");
    //    case at::ScalarType::Float8_e4m3fn:
    //      return std::make_pair("float8_e4m3fn", "");
    //    case at::ScalarType::Float8_e5m2fnuz:
    //      return std::make_pair("float8_e5m2fnuz", "");
    //    case at::ScalarType::Float8_e4m3fnuz:
    //      return std::make_pair("float8_e4m3fnuz", "");
    //    default:
    //      throw std::runtime_error("Unimplemented scalar type");
    //  }
    //}    

    ObjType typeFromLimit(int64_t value, ObjType type_default = ObjType::Int64);

    ObjType typeFromLimit(double value, ObjType type_default = ObjType::Float64);

    //    ObjType typeFromLimit(std::complex<double> value, ObjType type_default = ObjType::Complex64) {
    //        LOG_RUNTIME("Not implemented!");
    //    }

    ObjType typeFromString(TermPtr &term, RunTime *rt, bool *has_error = nullptr);
    //    ObjType typeFromString(const std::string type, RunTime *rt, bool *has_error = nullptr);

    //    typedef * void LLVMTypeRef;

    /*
     * Можно ли привести один тип к другому типу с учетом размера данных
     * Используется в интепретаторе и при выполнении для выдачи предупреждений
     */
    inline bool canCastLimit(const ObjType from, const ObjType to) {
        if (from == to || from == ObjType::None || to == ObjType::None || to == ObjType::Any) {
            // Преобразовывать ненужно или указан тип по по умолчанию
            return true;
        } else if (isSimpleType(from)) {
            if (isSimpleType(to)) {
                // Для простых типов приведение типов почти как в Torch, только с учетом размера данных
                // Запрещено: Big size -> Small Size, т.е.  Byte_tensor *= Int_tensor.
                // Разрешено: Bool -> Byte, Int8 -> Int16 -> Int32 -> Int64 -> Float32, Float64 -> Complex32, Complex64
                //   т.е. Int_tensor += Bool_tensor или Complex32_tensor *= Short_tensor.

                //        return at::canCast(toTorchType(from), toTorchType(to))
                //                && (from <= to || from == ObjType::Bool || from <= ObjType::Int8 || (isFloatingType(from) &&
                //                isFloatingType(to)));
                return (from <= to || (isFloatingType(from) && isFloatingType(to)));
            } else if (to == ObjType::Rational) {
                return true;
            }
        } else if (isString(from) && isString(to)) {// && isObjectType(to)) {
            // Строковые типы конвертируются только В объект, т.к. нативные типы не изменяются
            return true;
        } else if (isDictionary(from) && isDictionary(to)) {
            return true;
        } else if (isFunction(from) && isFunction(to)) {
            return true;
        }
        return false;
    }

    /*
     * Можно ли привести один тип к другому типу в операциях
     * Используется при выполнении для генерации ошибок выполнения
     */
    inline bool canCast(const ObjType from, const ObjType to) {
        //    if (isSimpleType(from) && isSimpleType(to)) {
        //        // Для простых типов приведение как в Torch
        //
        //        // We disallow complex -> non complex, e.g., float_tensor *= complex is disallowed.
        //        // We disallow float -> integral, e.g., int_tensor *= float is disallowed.
        //
        //        // Treat bool as a distinct "category," to be consistent with type promotion
        //        // rules (e.g. `bool_tensor + 5 -> int64_tensor`). If `5` was in the same
        //        // category as `bool_tensor`, we would not promote. Differing categories
        //        // implies `bool_tensor += 5` is disallowed.
        //        //
        //        // NB: numpy distinguishes "unsigned" as a category to get the desired
        //        // `bool_tensor + 5 -> int64_tensor` behavior. We don't, because:
        //        // * We don't want the performance hit of checking the runtime sign of Scalars.
        //        // * `uint8_tensor + 5 -> int64_tensor` would be undesirable.
        //
        //        return at::canCast(toTorchType(from), toTorchType(to));
        //    }
        return canCastLimit(from, to);
    }

    inline bool canCast(const std::string from, const std::string to) {
        return canCast(GetBaseTypeFromString(from), GetBaseTypeFromString(to));
    }

    bool canCast(const TermPtr &from, const ObjType to);
    bool canCast(const TermPtr &from, const TermPtr &to);

    inline int64_t parseInteger(const char *str) {
        char *ptr;
        std::string temp = str;
        for (size_t k = temp.find('_'); k != temp.npos; k = temp.find('_', k)) {
            temp.erase(k, 1);
        }
        int64_t result = std::strtol(temp.c_str(), &ptr, 10);
        if (ptr == temp.c_str() || (ptr && *ptr != '\0')) {
            LOG_RUNTIME("Fail integer convert %s", str);
        }
        return result;
    }

    inline double parseDouble(const char *str) {
        char *ptr;
        std::string temp = str;
        for (size_t k = temp.find('_'); k != temp.npos; k = temp.find('_', k)) {
            temp.erase(k, 1);
        }
        double result = std::strtod(temp.c_str(), &ptr);
        if (ptr == temp.c_str() || (ptr && *ptr != '\0')) {
            LOG_RUNTIME("Fail double convert %s", str);
        }
        return result;
    }

    inline std::complex<double> parseComplex(const char *) {
        LOG_RUNTIME("Not implemented!");
    }

    /*
     *
     *
     *
     */
    inline bool isReservedName(const std::string_view name) {
        if (name.empty()) {
            return false;
        }
        if (name.size() > 3 || !(name[0] == '$' || name[0] == '@' || name[0] == '%')) {
            return name.compare("_") == 0;
        }
        return name.compare("$") == 0 || name.compare("@") == 0 || name.compare("%") == 0
                || name.compare("$$") == 0 || name.compare("@$") == 0 || name.compare("$^") == 0
                || name.compare("@::") == 0 || name.compare("$*") == 0 || name.compare("$#") == 0;
    }
    
    inline bool isInternalName(const std::string_view name) {
        return !name.empty() && (name.rbegin()[0] == ':' || name.rbegin()[0] == '$' || isReservedName(name));
    }

    inline bool isMangledName(const std::string_view name) {
        return name.size() > 4 && name[0] == '_' && name[1] == '$';
    }

    inline bool isModuleName(const std::string_view name) {
        return !name.empty() && name[0] == '\\';
    }

    inline bool isStaticName(const std::string_view name) {
        return name.find("::") != std::string::npos; // && name.find("&&") != std::string::npos;
    }

    inline bool isFieldName(const std::string_view name) {
        return name.find(".") != std::string::npos;
    }

    inline bool isTrivialName(const std::string_view name) {
        return name.find("$") == std::string::npos && name.find(":") == std::string::npos && name.find("@") == std::string::npos;
    }

    inline bool isLocalName(const std::string_view name) {
        return !isStaticName(name) && name.find("$") != std::string::npos && name[0] != '@';
    }

    inline bool isGlobalScope(const std::string_view name) {
        ASSERT(!isMangledName(name));
        return name.size() > 1 && ((name[0] == ':' && name[1] == ':') || (name[0] == '$' && name[1] == '$'));
    }

    inline bool isModuleScope(const std::string_view name) {
        size_t pos = name.find("::");
        return pos && pos != std::string::npos && name[0] != '@';
    }

    inline bool isTypeName(const std::string_view name) {
        return name.find(":::") != std::string::npos || (name.size() > 1 && name[0] == ':' && name[1] != ':');
    }

    inline bool isFullName(const std::string_view name) {

        return name.size() > 1 && name[0] == ':' && name[1] == ':';
    }

    inline bool isMacroName(const std::string_view name) {

        return !name.empty() && name[0] == '@';
    }

    inline bool isNativeName(const std::string_view name) {

        return !name.empty() && name[0] == '%';
    }

    inline bool isLocalAnyName(const std::string_view name) {

        return !name.empty() && (name[0] == '$' || name[0] == '@' || name[0] == ':' || name[0] == '%' || name[0] == '\\');
    }

    inline bool isMutableName(const std::string_view name) {
        // Метод, который изменяет объект, должен заканчиваеться на ОДИН подчерк

        return name.size() > 1 && name[name.size() - 1] == '_' && name[name.size() - 2] != '_';
    }

    inline bool isSystemName(const std::string_view name) {
        if (name.empty()) {

            return false;
        }
        return name.size() >= 4 && name.find("__") == 0 && name.rfind("__") == name.size() - 2;
    }

    inline bool isPrivateName(const std::string_view name) {
        if (name.empty()) {

            return false;
        }
        return name.size() >= 3 && name.find("__") == 0;
    }

    inline bool isHidenName(const std::string_view name) {

        return !isPrivateName(name) && name.find("_") == 0;
    }

    inline bool isVariableName(const std::string_view name) {
        LOG_DEBUG("%s", name.begin());
        if (isModuleName(name)) {

            return name.find("::") != name.npos;
        }
        return !isTypeName(name);
    }

    inline bool isConstName(const std::string_view name) {

        return !name.empty() && name[name.size() - 1] == '^';
    }

    inline std::string NormalizeName(const std::string_view name) {
        std::string result(name.begin());
        ASSERT(result.size());
        if (isInternalName(name)) {
            return result;
        } else if (isLocalName(name)) {
            result = result.substr(1);
            result += "$";
        } else if (isTrivialName(name)) {
            result += "$";
        } else if (isTypeName(name)) {
            result = result.substr(1);
            result += ":::";
        } else {
            if (!isStaticName(name)) {
                ASSERT(isStaticName(name));
            }
            if (result[0] == '@' && result.find("@::") == 0) {
                result = result.substr(3);
            }
            result += "::";
        }
        return result;
    }

    inline std::string MakeName(std::string name) {
        if (!name.empty() && (name[0] == '\\' || name[0] == '$' || name[0] == '@' || name[0] == '%')) {

            return name.find("\\\\") == 0 ? name.substr(2) : name.substr(1);
        }
        return name;
    }

    inline std::string ExtractModuleName(const std::string_view name) {
        if (isMangledName(name)) {
            std::string result(name.begin(), name.begin() + name.find("$_"));
            result[1] = '_';
            std::replace(result.begin(), result.end(), '_', '\\');
            return result;
        } else {
            if (isModuleName(name)) {
                size_t pos = name.find("::");
                if (pos != std::string::npos) {
                    return std::string(name.begin(), name.begin() + pos);
                }
                return std::string(name.begin(), name.end());
            }
        }
        return std::string();
    }

    inline bool CheckCharModuleName(const std::string_view name) {
        if (name.empty() || name[0] == '_') {
            // The first underscore character in the module name is prohibited!
            return false;
        }
        for (size_t i = 0; i < name.size(); i++) {
            // Module name - backslash, underscore, lowercase English letters or number
            if (!(name[i] == '\\' || name[i] == '_' || islower(name[i]) || isdigit(name[i]))) {
                return false;
            }
        }
        return true;
    }

    inline std::string ExtractName(std::string name) {
        size_t pos = name.rfind("::");
        if (pos != std::string::npos) {
            name = name.substr(pos + 2);
        }
        if (isModuleName(name)) {
            return std::string();
        }
        return name;
    }

    class InternalName : public std::string {
    public:

        InternalName(const std::string str) {

            this->assign(str);
        }

        InternalName(const char * str = nullptr) {

            this->assign(str ? str : "");
        }

        InternalName(const InternalName &name) {

            this->assign(name);
        }

        InternalName& operator=(const InternalName & name) {
            this->assign(name);

            return *this;
        }

        InternalName& operator=(const char * name) {
            this->assign(name);

            return *this;
        }

        /*
         * 
         * 
         * 
         */

        inline bool isInternalName() {
            return newlang::isInternalName(*this);
        }

        inline std::string getMangledName(const std::string_view module) {
            std::string result(*this);
            if (!isInternalName()) {
                LOG_RUNTIME("The name '%s' is not internal!", result.c_str());
            }
            std::replace(result.begin(), result.end(), ':', '$');
            result.insert(0, "$_");
            if (module.size() > 2) {
                result.insert(result.begin(), module.begin(), module.end());
                std::replace(result.begin(), result.begin() + module.size(), '\\', '$');
            }
            result.insert(0, "_$");
            return result;
        }

        static std::string ExtractModuleName(const std::string_view name) {
            return newlang::ExtractModuleName(name);
        }

        inline bool isModule() {

            return newlang::isModuleName(this->c_str());
        }

        inline bool isStatic() {

            return newlang::isStaticName(this->c_str());
        }

        inline bool isLocal() {

            return newlang::isLocalName(this->c_str());
        }

        inline bool isGlobalScope() {

            return newlang::isGlobalScope(*this);
        }

        inline bool isModuleScope() {

            return newlang::isModuleScope(*this);
        }

        inline bool isTypeName() {

            return newlang::isTypeName(this->c_str());
        }

        inline bool isFullName() {

            return newlang::isFullName(this->c_str());
        }

        inline bool isMacroName() {

            return newlang::isMacroName(this->c_str());
        }

        inline bool isNativeName() {

            return newlang::isNativeName(this->c_str());
        }

        inline bool isLocalAnyName() {

            return newlang::isLocalAnyName(this->c_str());
        }

        inline bool isMutableName() {
            // Метод, который изменяет объект, должен заканчиваеться на ОДИН подчерк

            return newlang::isMutableName(this->c_str());
        }

        inline bool isSystemName() {

            return newlang::isSystemName(this->c_str());
        }

        inline bool isPrivateName() {
            return newlang::isPrivateName(this->c_str());
        }

        inline bool isHidenName() {

            return newlang::isHidenName(this->c_str());
        }

        inline bool isVariableName() {

            return newlang::isVariableName(this->c_str());
        }

        inline bool isConstName() {

            return newlang::isConstName(this->c_str());
        }

        inline std::string SetFromLocalName(std::string name) {
            this->assign(name);

            return *this;
        }

        inline std::string SetFromGlobalName(std::string name) {
            this->assign(name);

            return *this;
        }

        inline std::string GetLocalName() {

            return *this;
        }

        //        inline std::string GetGlobalName(std::string module_name) {
        //
        //            return *this;
        //        }

        //        inline std::string MakeName(std::string name) {
        //            if (!name.empty() && (name[0] == '\\' || name[0] == '$' || name[0] == '@' || name[0] == '%')) {
        //                return name.find("\\\\") == 0 ? name.substr(2) : name.substr(1);
        //            }
        //            return name;
        //        }

        inline std::string ExtractModuleName() {

            return newlang::ExtractModuleName(this->c_str());
        }

        inline std::string ExtractName() {

            return newlang::ExtractName(this->c_str());
        }


    };

    /*
     * 
     * 
     * 
     */

    inline bool isContainsType(ObjType generic, ObjType type) {
        if (!isGenericType(generic)) {
            if ((isTensor(generic) || isBooleanType(generic)) && (isTensor(type) || isBooleanType(type))) {
                return static_cast<uint8_t> (type) <= static_cast<uint8_t> (generic);
            }
            return generic == type && type != ObjType::None;
        }
        switch (generic) {
            case ObjType::Tensor: // Любое число включая логический тип
                return isTensor(type);
            case ObjType::Integer: // Любое ЦЕЛОЕ число включая логический тип
                return isIntegralType(type, true);
            case ObjType::Number: // Любое число с ПЛАВАЮЩЕЙ ТОЧКОЙ
                return isFloatingType(type) || isIntegralType(type, true);
            case ObjType::Complex: // Любое КОМПЛЕКСНОЕ число
                return isIntegralType(type, true) || isFloatingType(type) || isComplexType(type);
            case ObjType::Arithmetic: // Любое число
                return isIntegralType(type, true) || isFloatingType(type) || isComplexType(type) || type == ObjType::Rational;
            case ObjType::String: // Строка любого типа
                return isString(type);
            case ObjType::Object: // Любой объект (Class или Dictionary)
                return type == ObjType::Dictionary || type == ObjType::Interface || type == ObjType::Class;
            case ObjType::Plain: // Любой тип для машинного представления
                return isPlainDataType(type);
            case ObjType::Other: // Специальные типы (многоточие, диапазон)
                return type == ObjType::Ellipsis || type == ObjType::Range;
            case ObjType::Function: // Любая функция
                return isFunction(type);
            case ObjType::Eval: // Код для выполнения ?????
                return isEval(type);
            case ObjType::Any: // Любой тип кроме None
                return type != ObjType::None;

            case ObjType::None:
            default:
                return false;
        }
    }

} // namespace newlang

#endif // INCLUDED_TYPES_H_
