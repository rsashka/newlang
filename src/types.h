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
class NewLang;
class RunTime;
struct CompileInfo;

typedef std::shared_ptr<Term> TermPtr;
typedef std::shared_ptr<Obj> ObjPtr;
typedef std::shared_ptr<const Obj> ObjPtrConst;
typedef std::shared_ptr<RunTime> RuntimePtr;

typedef ObjPtr FunctionType(Context *ctx, Obj &in);
typedef ObjPtr TransparentType(const Context *ctx, Obj &in);

class Interrupt : public std::exception {
  public:
     
      inline static const char * Return = ":Return";
      inline static const char * Error = ":Error";
      inline static const char * Parser = ":ErrorParser";
      inline static const char * RunTime = ":ErrorRunTime";
      inline static const char * Signal = ":ErrorSignal";
      inline static const char * Abort = ":ErrorAbort";

    Interrupt(const ObjPtr obj);
    Interrupt(const std::string message, const std::string error_name=Error);

    virtual const char *what() const noexcept override;
    
    static ObjPtr CreateErrorMessage(const std::string message, const std::string error_name);

    const ObjPtr m_obj;
    char m_buffer_message[1000];
    
};

void NewLangSignalHandler(int signal);

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
        LOG_EXCEPT_LEVEL(Interrupt, LOG_LEVEL_INFO, "", "%s", message.c_str());                                 \
    } while (0)

#define NL_CHECK(cond, format, ...)                                                                                    \
    do {                                                                                                               \
        if (!(cond)) {                                                                                                 \
            LOG_EXCEPT_LEVEL(Interrupt, LOG_LEVEL_INFO, "", format, ##__VA_ARGS__);                             \
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
                Interrupt, LOG_LEVEL_INFO, "", "%s",                                                            \
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


#define NL_TYPES(_)         \
    _(None, 0)              \
    \
    _(Bool, 1)              \
    _(Char, 2)              \
    _(Short, 3)             \
    _(Int, 4)               \
    _(Long, 5)              \
    _(Integer, 15)           \
    \
    _(Float, 16)            \
    _(Double, 17)           \
    _(Number, 24)           \
    \
    _(ComplexFloat, 25)     \
    _(ComplexDouble, 26)    \
    _(Complex, 31)          \
                            \
    _(Tensor, 32)           \
                            \
    _(Fraction, 33)         \
    \
    _(Arithmetic, 47)       \
    \
    _(StrChar, 48)          \
    _(StrWide, 49)          \
    _(Format, 50)           \
    _(ViewChar, 51)         \
    _(ViewWide, 52)         \
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
    _(Class, 106)           \
    _(Ellipsis, 107)        \
    _(BLOCK, 108)           \
    _(BLOCK_TRY, 109)       \
    _(EVAL_FUNCTION, 110)   \
    _(SimplePureFunc, 111)  \
    _(SimplePureAND, 112)   \
    _(SimplePureOR, 113)    \
    _(SimplePureXOR, 114)   \
    \
    _(Eval, 118)            \
    _(Other, 120)           \
    _(Plain, 121)           \
    _(Struct, 201)          \
    _(Union, 202)           \
    _(Enum, 203)            \
    _(Object, 122)          \
    _(Any, 123)             \
    _(Type, 200)            \
    _(Return, 230)          \
    _(Break, 231)           \
    _(Continue, 232)        \
    _(Error, 240)           \
    _(ErrorParser, 241)     \
    _(ErrorRunTime, 242)    \
    _(ErrorSignal, 243)

    // BigNum - Длинные целые числа произвольного размера  100:Big
    // Currency - Fraction со знаменателем `10000 -1`000.00   `-1000  100:Curr
    // Fraction - произвольная дробь с длинными числами    100\1    100:Frac
    // Frac_tion \1 -> Curr_ency `1.0000 -> Big_Num 100`100`000.  100'100'000.

    //Форматирующий символ дроби (fraction slash, U+2044) позволяет создавать произвольные дроби следующим образом:
    // последовательность цифр числителя + форматирующий символ дроби + последовательность цифр знаменателя
    // — при выводе на экран или на печать это должно преобразовываться в правильно сформированную дробь.
    // Например, 22⁄371 должна показываться как 22/371 или как 22 371 {\displaystyle {\frac {22}{371}}} {\displaystyle
    // {\frac {22}{371}}} (может использоваться как «косая», так и «вертикальная» форма представления дроби)[1].
    //
    //Для правильного отображения смешанных дробей (наподобие 3 6 7 {\displaystyle 3{\frac {6}{7}}} {\displaystyle 3{\frac
    //{6}{7}}})
    // целую часть нужно отделять от числителя дробной части подходящим пробелом (например, пробелом нулевой ширины U+200B).
    //
    //Кроме того, существует символ ⅟ (fraction numerator one, U+215F), позволяющий формировать дроби с числителем,
    //равным 1.
    //  "/-5 " - квадратный корень из 5,  "3/-5" - корень третьей степени ????
    // "1/_5" - Одна пятая ??
    // https://github.com/python/cpython/blob/main/Lib/fractions.py
    //
    //_RATIONAL_FORMAT = re.compile(r"""
    //    \A\s*                                 # optional whitespace at the start,
    //    (?P<sign>[-+]?)                       # an optional sign, then
    //    (?=\d|\.\d)                           # lookahead for digit or .digit
    //    (?P<num>\d*|\d+(_\d+)*)               # numerator (possibly empty)
    //    (?:                                   # followed by
    //       (?:/(?P<denom>\d+(_\d+)*))?        # an optional denominator
    //    |                                     # or
    //       (?:\.(?P<decimal>d*|\d+(_\d+)*))?  # an optional fractional part
    //       (?:E(?P<exp>[-+]?\d+(_\d+)*))?     # and optional exponent
    //    )
    //    \s*\Z                                 # and optional whitespace to finish
    //""", re.VERBOSE | re.IGNORECASE)

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
     * - Обычные (UTF8), тип данных Char
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
     * - Определяется общий тип (Bool, Integer, Number, Complex).
     * - Если тип литерала явно не указан, то выбирается минимальный байтовый размер для общего типа.
     * - В выражениях тензор-скаляр, тензором должен быть самый левый элемент выражения.
     * - Итоговым типом поседовательности выражений является тип первого вычисленного элемента.
     * - В операторах присвоения вычисление типа происходит дважды, сперва для правой части выражения, а потом для оператора
     * присовения.
     * - Тип меньшего размера может автоматически приводится к типу большему размеру или к более сложному типу
     *    bool -> char -> short -> int -> long -> float -> double -> complexfloat -> complexdouble
     *    var := 1.0 + 2; // float
     *    var := 1 + 1000; // Short т.к. первый тип изменяемый
     *    var := 1:Char + 1000; // Ошибка в выражении т.к. первый тип не изменяемый  Short -x-> Char
     *    var:Char := 1 + 1000; // Ошибка в присвоении т.к. тип не изменяемый  Short -x-> Char
     *    var := 1000 + 2; // Short
     *    var := 1000 + 2.0; // Ошибка float -> short, но может быть var := 1000.0 + 2;
     *    var := 1:Bool + 2; // Ошибка byte -> bool, но может быть var := 2 + 1:Bool;
     *
     * - Итоговым типом поседовательности выражений является тип первого вычисленого элемента.
     *      var := 1.0 + 2; // float  var := 1000 + 2; // Short  var := 1000 + 2.0;
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
        return t == ObjType::Dictionary || t == ObjType::Class;
    }

    inline bool isBaseType(ObjType t) {
        return t == ObjType::Bool || t == ObjType::Char || t == ObjType::Short || t == ObjType::Int
                || t == ObjType::Long || t == ObjType::Float || t == ObjType::Double;
    }

    inline bool isFunction(ObjType t) {
        return t == ObjType::PureFunc || t == ObjType::Function || t == ObjType::NativeFunc ||
                t == ObjType::EVAL_FUNCTION || t == ObjType::PureFunc || t == ObjType::SimplePureAND ||
                t == ObjType::SimplePureOR || t == ObjType::SimplePureXOR;
    }

    inline bool isEval(ObjType t) {
        return t == ObjType::BLOCK || t == ObjType::BLOCK_TRY;
    }

    inline bool isSimpleType(ObjType t) {
        return static_cast<uint8_t> (t) && static_cast<uint8_t> (t) <= static_cast<uint8_t> (ObjType::Tensor);
    }

    inline bool isIntegralType(ObjType t, bool includeBool) {
        return static_cast<uint8_t> (t) >= static_cast<uint8_t> (ObjType::Char) &&
                static_cast<uint8_t> (t) <= static_cast<uint8_t> (ObjType::Integer) ||
                (includeBool && t == ObjType::Bool);
    }

    inline bool isFloatingType(ObjType t) {
        return t == ObjType::Float || t == ObjType::Double || t == ObjType::Number;
    }

    inline bool isComplexType(ObjType t) {
        return t == ObjType::ComplexFloat || t == ObjType::ComplexDouble || t == ObjType::Complex;
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

    inline bool isString(ObjType t) {
        return t == ObjType::StrChar || t == ObjType::StrWide || t == ObjType::ViewWide || t == ObjType::ViewWide
                || t == ObjType::String || t == ObjType::Format;
    }

    inline bool isPlainDataType(ObjType t) {
        return isTensor(t) || isString(t) || t == ObjType::Struct || t == ObjType::Enum || t == ObjType::Union;
    }

    inline bool isDictionary(ObjType t) {
        return t == ObjType::Dictionary || t == ObjType::Class;
    }

    inline bool isClass(ObjType t) {
        return t == ObjType::Class;
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

    inline bool isLocalType(ObjType t) {
        return false;
    }

    inline torch::Dtype toTorchType(ObjType t) {
        switch (t) {
            case ObjType::Bool:
                return at::ScalarType::Bool;
            case ObjType::Char:
                return at::ScalarType::Char;
            case ObjType::Short:
                return at::ScalarType::Short;
            case ObjType::Int:
                return at::ScalarType::Int;
            case ObjType::Long:
            case ObjType::Integer:
                return at::ScalarType::Long;
            case ObjType::Float:
            case ObjType::Tensor:
                return at::ScalarType::Float;
            case ObjType::Double:
            case ObjType::Number:
                return at::ScalarType::Double;
            case ObjType::ComplexFloat:
                return at::ScalarType::ComplexFloat;
            case ObjType::ComplexDouble:
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
                return ObjType::Char;
            case at::ScalarType::Short:
                return ObjType::Short;
            case at::ScalarType::Int:
            case at::ScalarType::QInt32:
                return ObjType::Int;
            case at::ScalarType::Long:
                return ObjType::Long;
            case at::ScalarType::Float:
            case at::ScalarType::BFloat16:
                return ObjType::Float;
            case at::ScalarType::Double:
                return ObjType::Double;
            case at::ScalarType::ComplexFloat:
                return ObjType::ComplexFloat;
            case at::ScalarType::ComplexDouble:
                return ObjType::ComplexDouble;
        }
        LOG_RUNTIME("Can`t convert type '%s' to ObjType!", at::toString(t));
    }

    inline ObjType typeFromLimit(int64_t value, ObjType type_default = ObjType::Long) {
        if (value == 1 || value == 0) {
            return ObjType::Bool;
        } else if (value < std::numeric_limits<int32_t>::min() || value > std::numeric_limits<int32_t>::max()) {
            ASSERT(value > std::numeric_limits<int64_t>::min());
            ASSERT(value < std::numeric_limits<int64_t>::max());
            return ObjType::Long;
        } else if (value < std::numeric_limits<int16_t>::min() || value > std::numeric_limits<int16_t>::max()) {
            return ObjType::Int;
        } else if (value < std::numeric_limits<int8_t>::min() ||
                value > std::numeric_limits<int8_t>::max()) { //-127 < ... > 128
            return ObjType::Short;
        } else {
            return ObjType::Char;
        }
        return type_default;
    }

    inline ObjType typeFromLimit(double value, ObjType type_default = ObjType::Float) {
        if (std::equal_to<double>()(value, 0)) {
            return type_default;
        }
        return ObjType::Double;
    }

    inline ObjType typeFromLimit(std::complex<double> value, ObjType type_default = ObjType::ComplexFloat) {
        LOG_RUNTIME("Not implemented!");
    }

    ObjType typeFromString(const std::string type, Context *ctx = nullptr, bool *has_error = nullptr);

    /*
     * Можно ли привести один тип к другому типу с учетом размера данных
     * Используется в интепретаторе и при выполнении для выдачи предупреждений
     */
    inline bool canCastLimit(const ObjType from, const ObjType to) {
        if (from == to || from == ObjType::None || to == ObjType::None) {
            // Преобразовывать ненужно или указан тип по по умолчанию
            return true;
        } else if (isSimpleType(from) && isSimpleType(to)) {
            // Для простых типов приведение типов почти как в Torch, только с учетом размера данных
            // Запрещено: Big size -> Small Size, т.е.  Byte_tensor *= Int_tensor.
            // Разрешено: Bool -> Byte, Char -> Short -> Int -> Long -> Float, Double -> ComplexFloat, ComplexDouble
            //   т.е. Int_tensor += Bool_tensor или ComplexFloat_tensor *= Short_tensor.

            //        return at::canCast(toTorchType(from), toTorchType(to))
            //                && (from <= to || from == ObjType::Bool || from <= ObjType::Char || (isFloatingType(from) &&
            //                isFloatingType(to)));
            return (from <= to || (isFloatingType(from) && isFloatingType(to)));

        } else if (isString(from) && isString(to)) {// && isObjectType(to)) {
            // Строковые типы конвертируются только В объект, т.к. нативные типы не изменяются
            return true;
        } else if (isDictionary(from) && isDictionary(to)) {
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

    inline void setTensorValue(torch::Tensor &self, torch::Tensor &set) {
        if (!at::canCast(set.scalar_type(), self.scalar_type())) {
            LOG_RUNTIME("Can`t cast '%s' to '%s'!", at::toString(set.scalar_type()), at::toString(self.scalar_type()));
        }
        if (self.dim() == 0 && set.dim() != 0) {
            LOG_RUNTIME("Fail assign tensor to scalar!");
        }
        if (!self.sizes().equals(set.sizes()) && set.dim() != 0) {
            LOG_RUNTIME("Different sizes of tensors!");
        }

        if (set.dim() != 0) {
            self = set.clone();
            return;

            ASSERT(set.dim() == 0);
        }

        bool *ptr_boll = nullptr;
        signed char *ptr_char = nullptr;
        int16_t *ptr_short = nullptr;
        int32_t *ptr_int = nullptr;
        int64_t *ptr_long = nullptr;
        float *ptr_float = nullptr;
        double *ptr_double = nullptr;

        if (self.dim() == 0 && set.dim() == 0) {
            // scalar := scalar
            switch (fromTorchType(self.scalar_type())) {
                case ObjType::Bool:
                    ptr_boll = self.data_ptr<bool>();
                    ASSERT(ptr_boll);
                    *ptr_boll = set.item().toBool();
                    return;
                case ObjType::Char:
                    ptr_char = self.data_ptr<signed char>();
                    ASSERT(ptr_char);
                    *ptr_char = set.item().toChar();
                    return;
                case ObjType::Short:
                    ptr_short = self.data_ptr<int16_t>();
                    ASSERT(ptr_short);
                    *ptr_short = set.item().toShort();
                    return;
                case ObjType::Int:
                    ptr_int = self.data_ptr<int32_t>();
                    ASSERT(ptr_int);
                    *ptr_int = set.item().toInt();
                    return;
                case ObjType::Long:
                    ptr_long = self.data_ptr<int64_t>();
                    ASSERT(ptr_long);
                    *ptr_long = static_cast<int64_t> (set.item().toLong());
                    return;
                case ObjType::Float:
                    ptr_float = self.data_ptr<float>();
                    ASSERT(ptr_float);
                    *ptr_float = set.item().toFloat();
                    return;
                case ObjType::Double:
                    ptr_double = self.data_ptr<double>();
                    ASSERT(ptr_double);
                    *ptr_double = set.item().toDouble();
                    return;
            }
        } else if (self.dim() != 0 && set.dim() == 0) {
            // tensor := scalar

            if (self.dim() == 1) {
                ObjType type = fromTorchType(self.scalar_type());
                if (ObjType::Bool == type) {
                    auto acc_bool = self.accessor<bool, 1 > ();
                    for (int i = 0; i < acc_bool.size(0); i++) {
                        acc_bool[i] = set.item().toBool();
                    }
                    return;
                } else if (ObjType::Char == type) {
                    auto acc_char = self.accessor<signed char, 1 > ();
                    for (int i = 0; i < acc_char.size(0); i++) {
                        acc_char[i] = set.item().toChar();
                    }
                    return;
                } else if (ObjType::Short == type) {
                    auto acc_short = self.accessor<int16_t, 1>();
                    for (int i = 0; i < acc_short.size(0); i++) {
                        acc_short[i] = set.item().toShort();
                    }
                    return;
                } else if (ObjType::Int == type) {
                    auto acc_int = self.accessor<int32_t, 1>();
                    for (int i = 0; i < acc_int.size(0); i++) {
                        acc_int[i] = set.item().toInt();
                    }
                    return;
                } else if (ObjType::Long == type) {
                    auto acc_long = self.accessor<int64_t, 1>();
                    for (int i = 0; i < acc_long.size(0); i++) {
                        acc_long[i] = static_cast<int64_t> (set.item().toLong());
                    }
                    return;
                } else if (ObjType::Float == type) {
                    auto acc_float = self.accessor<float, 1>();
                    for (int i = 0; i < acc_float.size(0); i++) {
                        acc_float[i] = set.item().toFloat();
                    }
                    return;
                } else if (ObjType::Double == type) {
                    auto acc_double = self.accessor<double, 1>();
                    for (int i = 0; i < acc_double.size(0); i++) {
                        acc_double[i] = set.item().toDouble();
                    }
                    return;
                }
            } else if (self.dim() == 2) {
                ObjType type = fromTorchType(self.scalar_type());
                if (ObjType::Bool == type) {
                    auto acc_bool = self.accessor<bool, 2 > ();
                    for (int i = 0; i < acc_bool.size(0); i++) {
                        for (int j = 0; j < acc_bool.size(1); j++) {
                            acc_bool[i][j] = set.item().toBool();
                        }
                    }
                    return;
                } else if (ObjType::Char == type) {
                    auto acc_char = self.accessor<signed char, 2 > ();
                    for (int i = 0; i < acc_char.size(0); i++) {
                        for (int j = 0; j < acc_char.size(1); j++) {
                            acc_char[i][j] = set.item().toChar();
                        }
                    }
                    return;
                } else if (ObjType::Short == type) {
                    auto acc_short = self.accessor<int16_t, 2>();
                    for (int i = 0; i < acc_short.size(0); i++) {
                        for (int j = 0; j < acc_short.size(1); j++) {
                            acc_short[i][j] = set.item().toShort();
                        }
                    }
                    return;
                } else if (ObjType::Int == type) {
                    auto acc_int = self.accessor<int32_t, 2>();
                    for (int i = 0; i < acc_int.size(0); i++) {
                        for (int j = 0; j < acc_int.size(1); j++) {
                            acc_int[i][j] = set.item().toInt();
                        }
                    }
                    return;
                } else if (ObjType::Long == type) {
                    auto acc_long = self.accessor<int64_t, 2>();
                    for (int i = 0; i < acc_long.size(0); i++) {
                        for (int j = 0; j < acc_long.size(1); j++) {
                            acc_long[i][j] = static_cast<int64_t> (set.item().toLong());
                        }
                    }
                    return;
                } else if (ObjType::Float == type) {
                    auto acc_float = self.accessor<float, 2>();
                    for (int i = 0; i < acc_float.size(0); i++) {
                        for (int j = 0; j < acc_float.size(1); j++) {
                            acc_float[i][j] = set.item().toFloat();
                        }
                    }
                    return;
                } else if (ObjType::Double == type) {
                    auto acc_double = self.accessor<double, 2>();
                    for (int i = 0; i < acc_double.size(0); i++) {
                        for (int j = 0; j < acc_double.size(1); j++) {
                            acc_double[i][j] = set.item().toDouble();
                        }
                    }
                    return;
                }
            }
            LOG_RUNTIME("Set data tensor for dims %d not implemented!", (int) self.dim());
        }
        LOG_RUNTIME("Fail set data tensor type '%s'!", at::toString(self.scalar_type()));
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
    inline bool isGlobal(const std::string_view name) {
        return !name.empty() && name[0] == '@';
    }

    inline bool isLocal(const std::string_view name) {
        return !name.empty() && name[0] == '$';
    }

    inline bool isType(const std::string_view name) {
        return !name.empty() && name[0] == ':';
    }

    inline bool isMacro(const std::string_view name) {
        return !name.empty() && name[0] == '\\';
    }

    inline bool isLocalAny(const char *name) {
        return name && !(name[0] == '$' || name[0] == '@' || name[0] == ':' || name[0] == '%' || name[0] == '\\');
    }

    inline bool isMutableName(const std::string name) {
        // Метод, который изменяет объект, должен заканчиваеться на ОДИН подчерк
        return name.size() > 1 && name[name.size() - 1] == '_' && name[name.size() - 2] != '_';
    }

    inline bool isInternalName(const std::string name) {
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
                return isIntegralType(type, true) || isFloatingType(type) || isComplexType(type) || type == ObjType::Fraction;
            case ObjType::String: // Строка любого типа
                return isString(type);
            case ObjType::Object: // Любой объект (Class или Dictionary)
                return type == ObjType::Dictionary || type == ObjType::Class;
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


    /*
     * Range -> Dict
     * String -> Tensor
     * Tensor -> String
     * Dict -> Tensor
     * Tensor -> Dict
     */
    void ConvertRangeToDict(Obj *from, Obj &to);

    void ConvertStringToTensor(const std::string &from, torch::Tensor &to, ObjType type = ObjType::None);
    void ConvertStringToTensor(const std::wstring &from, torch::Tensor &to, ObjType type = ObjType::None);

    void ConvertTensorToString(const torch::Tensor &from, std::string &to, std::vector<Index> *index = nullptr);
    void ConvertTensorToString(const torch::Tensor &from, std::wstring &to, std::vector<Index> *index = nullptr);

    void ConvertDictToTensor(Obj &from, torch::Tensor &to, ObjType type = ObjType::Tensor);
    void ConvertTensorToDict(const torch::Tensor &from, Obj &to, std::vector<Index> *index = nullptr);
    void ConvertValueToTensor(Obj *from, torch::Tensor &to, ObjType type = ObjType::None);

} // namespace newlang

#endif // INCLUDED_NEWLANG_TYPES_H_
