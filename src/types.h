#ifndef INCLUDED_NEWLANG_TYPES_H_
#define INCLUDED_NEWLANG_TYPES_H_

#include "pch.h"

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

    

typedef at::indexing::TensorIndex Index;
typedef at::IntArrayRef Dimension;

class Term;
class Obj;
class Context;
class Compiler;
class RunTime;
class Diag;

typedef std::shared_ptr<Term> TermPtr;
typedef std::vector<TermPtr> BlockType;
typedef std::shared_ptr<Obj> ObjPtr;
typedef std::shared_ptr<const Obj> ObjPtrConst;
typedef std::shared_ptr<RunTime> RuntimePtr;
typedef std::shared_ptr<Diag> DiagPtr;

typedef ObjPtr FunctionType(Context *ctx, Obj &in);
typedef ObjPtr TransparentType(const Context *ctx, Obj &in);

class Return : public std::exception {
  public:
     
      static const char * RetPlus;
      static const char * RetMinus;
      static const char * IntParser;
      static const char * IntError;
    
      static const char * Break;
      static const char * Continue;
//      static const char * Return;
      static const char * Error;
      static const char * Parser;
      static const char * RunTime;
      static const char * Signal;
      static const char * Abort;

    Return(const ObjPtr obj);
    Return(const std::string message, const std::string error_name=Error);

    virtual const char *what() const noexcept override;
    
    static ObjPtr CreateErrorMessage(const std::string message, const std::string error_name);

    const ObjPtr m_obj;
    char m_buffer_message[1000];
    
};

void NewLangSignalHandler(int signal);


struct Docs;
typedef std::shared_ptr<Docs> DocPtr;

struct Docs {
    
    static std::multimap<std::string, DocPtr> m_docs;
    
    std::string index;
    std::string text;

    Docs(std::string body) {
        size_t pos = body.find("\n");
        size_t pos2 = body.find("\n\n");
        if (pos != std::string::npos && pos == pos2) {
            // Индексируемый заголовок должен быть первой строкой и отделаться пустой строкой от остального текста
            index = body.substr(0, pos);
            text = body.substr(pos + 2);
        } else {
            text = body;
        }
    }

    static DocPtr Build(std::string body) {
        return std::make_shared<Docs>(body);
    }
    
    static DocPtr Append(std::string body, const std::string func=""){
        DocPtr result = Build(body);
        m_docs.insert(std::pair<std::string, DocPtr>(func,result));
        return result;
    }
};


#ifdef __GNUC__
std::string ParserMessage(std::string &buffer, int row, int col, const char *format, ...)
    __attribute__((format(printf, 4, 5)));
#else

#endif

void ParserException(const char *msg, std::string &buffer, int row, int col);

#define NL_PARSER(term, format, ...)                                                                                   \
    do {                                                                                                               \
        std::string empty;                                                                                             \
        std::string message =                                                                                          \
            newlang::ParserMessage(term->m_source ? *term->m_source : empty, term->m_line, term->m_col, format, ##__VA_ARGS__); \
        LOG_EXCEPT_LEVEL(Return, LOG_LEVEL_INFO, "", "%s", message.c_str());                                 \
    } while (0)

#define NL_CHECK(cond, format, ...)                                                                                    \
    do {                                                                                                               \
        if (!(cond)) {                                                                                                 \
            LOG_EXCEPT_LEVEL(Return, LOG_LEVEL_INFO, "", format, ##__VA_ARGS__);                             \
        }                                                                                                              \
    } while (0)

#define NL_TYPECHECK(term, from, to)                                                                                   \
    do {                                                                                                               \
        if (!canCast(from, to)) {                                                                                      \
            std::string message = "Incompatible data type '";                                                          \
            message += newlang::toString(typeFromString(from));                                                        \
            message += "' and '";                                                                                      \
            message += newlang::toString(typeFromString(to));                                                          \
            message += "' (" __FILE__ ":" TO_STR(__LINE__) ")";                                                        \
            LOG_EXCEPT_LEVEL(                                                                                          \
                Return, LOG_LEVEL_INFO, "", "%s",                                                            \
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
    _(String, 55)           \
    \
    _(Iterator, 62)         \
    _(IteratorEnd, 63)      \
    \
    _(Pointer, 64)          \
    _(NativeFunc, 65)       \
    _(Function, 100)        \
    _(PureFunc, 101)        \
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
    _(Virtual, 119)            \
    _(Eval, 118)            \
    _(Other, 120)           \
    _(Plain, 121)           \
    _(Object, 122)          \
    _(Any, 123)             \
    \
    _(Type, 200)            \
    _(Struct, 201)          \
    _(Union, 202)           \
    _(Enum, 203)            \
    \
    _(RetPlus, 210)         \
    _(RetMinus, 211)        \
    _(IntRepeat, 212)        \
    _(IntParser, 213)       \
    _(IntError, 214)        \
    \
    _(Context, 227)          \
    _(Module, 228)          \
    _(Undefined, 229)          \
    _(Return, 230)          \
    _(Break, 231)           \
    _(Continue, 232)        \
    _(Error, 240)           \
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
        _NumOptions
    };

    constexpr uint16_t NumObjTypes = static_cast<uint16_t> (ObjType::_NumOptions);


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

    inline const char *toCXXRef(std::string &ref) {
        if (ref.compare("&") == 0) {
            return "*";
        } else if (ref.compare("&&") == 0) {
            return "&";
        } else if (ref.compare("&&&") == 0) {
            return "&&";
        }
        LOG_RUNTIME("Unknown reference type '%s'!", ref.c_str());
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

    inline bool isObjectType(ObjType t) {
        return t == ObjType::Dictionary || t == ObjType::Interface || t == ObjType::Class;
    }

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

    inline bool isLocalType(ObjType t) {
        return false;
    }

    inline torch::Dtype toTorchType(ObjType t) {
        switch (t) {
            case ObjType::Bool:
                return at::ScalarType::Bool;
            case ObjType::Int8:
            case ObjType::Byte:
            case ObjType::Char:
                return at::ScalarType::Char;
            case ObjType::Int16:
            case ObjType::Word:
                return at::ScalarType::Short;
            case ObjType::Int32:
            case ObjType::DWord:
                return at::ScalarType::Int;
            case ObjType::Int64:
            case ObjType::DWord64:
            case ObjType::Integer:
                return at::ScalarType::Long;
            case ObjType::Float32:
            case ObjType::Single:
            case ObjType::Tensor:
                return at::ScalarType::Float;
            case ObjType::Float64:
            case ObjType::Double:
            case ObjType::Number:
                return at::ScalarType::Double;
            case ObjType::Complex16:
                return at::ScalarType::ComplexHalf;
            case ObjType::Complex32:
                return at::ScalarType::ComplexFloat;
            case ObjType::Complex64:
            case ObjType::Complex:
                return at::ScalarType::ComplexDouble;
        }
        LOG_RUNTIME("Can`t convert type '%s' to torch scalar type!", toString(t));
    }

    inline ObjType fromTorchType(torch::Dtype t) {
        switch (t) {
            case at::ScalarType::Bool:
                return ObjType::Bool;
            case at::ScalarType::Byte:
            case at::ScalarType::Char:
            case at::ScalarType::QInt8:
            case at::ScalarType::QUInt8:
                return ObjType::Int8;
            case at::ScalarType::Short:
                return ObjType::Int16;
            case at::ScalarType::Int:
            case at::ScalarType::QInt32:
                return ObjType::Int32;
            case at::ScalarType::Long:
                return ObjType::Int64;
            case at::ScalarType::BFloat16:
            case at::ScalarType::Half:
                return ObjType::Float16;
            case at::ScalarType::Float:
                return ObjType::Float32;
            case at::ScalarType::Double:
                return ObjType::Float64;
            case at::ScalarType::ComplexHalf:
                return ObjType::Complex16;
            case at::ScalarType::ComplexFloat:
                return ObjType::Complex32;
            case at::ScalarType::ComplexDouble:
                return ObjType::Complex64;
        }
        LOG_RUNTIME("Can`t convert type '%s' to ObjType!", at::toString(t));
    }

    inline ObjType typeFromLimit(int64_t value, ObjType type_default = ObjType::Int64) {
        if (value == 1 || value == 0) {
            return ObjType::Bool;
        } else if (value < std::numeric_limits<int32_t>::min() || value > std::numeric_limits<int32_t>::max()) {
            ASSERT(value > std::numeric_limits<int64_t>::min());
            ASSERT(value < std::numeric_limits<int64_t>::max());
            return ObjType::Int64;
        } else if (value < std::numeric_limits<int16_t>::min() || value > std::numeric_limits<int16_t>::max()) {
            return ObjType::Int32;
        } else if (value < std::numeric_limits<int8_t>::min() ||
                value > std::numeric_limits<int8_t>::max()) { //-127 < ... > 128
            return ObjType::Int16;
        } else {
            return ObjType::Int8;
        }
        return type_default;
    }

    inline ObjType typeFromLimit(double value, ObjType type_default = ObjType::Float64) {
        if (std::equal_to<double>()(value, 0)) {
            return type_default;
        }
        return ObjType::Float64;
    }

    inline ObjType typeFromLimit(std::complex<double> value, ObjType type_default = ObjType::Complex64) {
        LOG_RUNTIME("Not implemented!");
    }

    ObjType typeFromString(const std::string type, Context *ctx = nullptr, bool *has_error = nullptr);

    inline LLVMTypeRef toLLVMType(ObjType t, bool none_if_error = false) {
        switch (t) {
            case ObjType::None:
                return LLVMVoidType();
            case ObjType::Bool:
                return LLVMInt1Type();
            case ObjType::Int8:
            case ObjType::Char:
            case ObjType::Byte:
                return LLVMInt8Type();
            case ObjType::Int16:
            case ObjType::Word:
                return LLVMInt16Type();
            case ObjType::Int32:
            case ObjType::DWord:
                return LLVMInt32Type();
            case ObjType::Int64:
            case ObjType::DWord64:
            case ObjType::Integer:
                return LLVMInt64Type();
            case ObjType::Float32:
            case ObjType::Single:
            case ObjType::Tensor:
                return LLVMFloatType();
            case ObjType::Float64:
            case ObjType::Double:
            case ObjType::Number:
                return LLVMDoubleType();

            case ObjType::Pointer:
            case ObjType::StrChar:
            case ObjType::FmtChar:
                return LLVMPointerType(LLVMInt8Type(), 0);

#ifdef _MSC_VER                
            case ObjType::StrWide:
            case ObjType::FmtWide:
                return LLVMPointerType(LLVMInt16Type(), 0);
#else
            case ObjType::StrWide:
            case ObjType::FmtWide:
                return LLVMPointerType(LLVMInt32Type(), 0);
#endif

        }
        LOG_RUNTIME("Can`t convert type '%s' to LLVM type!", toString(t));
    }

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

    std::string TensorToString(const torch::Tensor &tensor);

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
        return canCast(typeFromString(from), typeFromString(to));
    }

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

    inline std::complex<double> parseComplex(const char *str) {
        LOG_RUNTIME("Not implemented!");
    }

    /*
     *
     *
     *
     */
    inline bool isModule(const std::string &name) {
        return !name.empty() && name[0] == '\\';
    }

    inline bool isLocal(const std::string &name) {
        return !name.empty() && name[0] == '$';
    }

    inline bool isType(const std::string &name) {
        return name.size() > 1 && name[0] == ':' && name[1] != ':';
    }

    inline bool isFullName(const std::string &name) {
        return name.size() > 1 && name[0] == ':' && name[1] == ':';
    }

    inline bool isMacro(const std::string &name) {
        return !name.empty() && name[0] == '@';
    }

    inline bool isLocalAny(const char *name) {
        return name && (name[0] == '$' || name[0] == '@' || name[0] == ':' || name[0] == '%' || name[0] == '\\');
    }

    inline bool isMutableName(const std::string name) {
        // Метод, который изменяет объект, должен заканчиваеться на ОДИН подчерк
        return name.size() > 1 && name[name.size() - 1] == '_' && name[name.size() - 2] != '_';
    }

    inline bool isSystemName(const std::string name) {
        if (name.empty()) {
            return false;
        }
        return name.size() >= 4 && name.find("__") == 0 && name.rfind("__") == name.size() - 2;
    }

    inline bool isPrivateName(const std::string name) {
        if (name.empty()) {
            return false;
        }
        return name.size() >= 3 && name.find("__") == 0;
    }

    inline bool isHidenName(const std::string name) {
        return !isPrivateName(name) && name.find("_") == 0;
    }

    inline bool isVariableName(const std::string name) {
        LOG_DEBUG("%s", name.c_str());
        if (isModule(name)) {
            return name.find("::") != name.npos;
        }
        return !isType(name);
    }

    inline bool isConst(const std::string name) {
        return !name.empty() && name[name.size() - 1] == '^';
    }

    inline std::string DimToString(const Dimension dim) {
        std::stringstream ss;
        ss << dim;
        return ss.str();
    }

    inline std::string IndexToString(const std::vector<Index> &index) {
        std::stringstream ss;

        ss << "[";
        for (int i = 0; i < index.size(); i++) {
            ss << index[i];
        }
        ss << "]";

        return ss.str();
    }

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

#endif // INCLUDED_NEWLANG_TYPES_H_
