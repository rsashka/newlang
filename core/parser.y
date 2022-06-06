
%{ /*** C/C++ Declarations ***/

#include <core/pch.h>

#include <core/term.h>
#include <core/context.h>

#include "parser.h"
#include "lexer.h"

/* this "connects" the bison parser in the driver to the flex scanner class
 * object. it defines the yylex() function call to pull the next token from the
 * current lexer object of the driver context. */
#undef yylex
#define yylex driver.lexer->lex
    
#define YYDEBUG 1
    
%}

/*** yacc/bison Declarations ***/

%require "3.6"

/* add debug output code to generated parser. disable this for release
 * versions. */
%debug 

/* start symbol is named "start" */
%start start

/* write out a header file containing the token defines */
%defines

//%no-lines

/* use newer C++ skeleton file */
%skeleton "lalr1.cc"

/* namespace to enclose parser in */
%define api.prefix {newlang}

/* verbose error messages */
/* %define parse.error verbose */
%define parse.error detailed

/* set the parser's class identifier */

/* keep track of the current position within the input */
%locations
%initial-action
{
    // initialize the initial location object
    @$.begin.filename = @$.end.filename = &driver.streamname;
//    yydebug_ = 1;
};

/* The driver is passed by reference to the parser and to the scanner. This
 * provides a simple but effective pure interface, not relying on global
 * variables. */
%parse-param { class Parser& driver }


/*** BEGIN EXAMPLE - Change the example grammar's tokens below ***/
%define api.value.type {TermPtr}

/*
 * Термины в CamelCase соответствуют типам данных NewLang
 * БНФ лексики NewLang
 * 
 * Integer ::= целые числа | целые числа "_"  целые числа  (Bool, Char, Short, Int, Long  +  BigNum)
 * Number ::= с плавающей точкой  (Float, Double)
 * Complex ::= комплексные числа  (ComplexFloat, ComplexDouble)
 * Fraction ::= Integer "\" Integer    (Представление BigNum / BigNum)
 * Currency ::= "`" Integer  | "`" Integer "."  Integer  (Представление Fraction == BigNum / 10000)
 * 
 * Ariphmetic ::= Integer  | Number | Complex | Currency | Fraction
 * 
 * StrChar ::= "'" символы "'"
 * StrWide ::= "\"" символы "\""
 * RawChar ::= r"'" символы "'"  |  R"'" символы "'"
 * RawWide ::= r"\"" символы "\""  | R"\"" символы "\""
 * RawStr ::= RawChar | RawWide
 * TemplateChar::= "'''" символы "'''"
 * TemplateWide ::= "\"\"\"" символы  "\"\"\""
 * Template ::=  TemplateChar |  TemplateWide
 * 
 * String  ::= StrWide | StrChar | RawStr | Template
 * 
 * Literal := String | Ariphmetic
 * 
 * name ::= идентификатор (буквы, цифры(крмое первой), подчерк)
 * 
 * Term ::= [@$~]? name
 * Type ::=  :name
 *   ::=  :name
 * Call ::= Term( ... )
 * 
 * eval ::= Leteral | Term | Call
 * 
 * 
 * Exit ::= "--"
 * Error ::= "--" eval "--"
 * 
 * rval  ::= eval
 * lval 
 * 
 * ops ::= "+" | "-" ...
 * expression ::= rval  ops  rval
 * 
 * exec ::=  rval
 * block ::=  exec |  {  exec  } |  {  block  exec  }
 * 
 * setup ::=   "::=" | ":=" | "="
 * assign ::=  lval   setup  rval  
 *      |  lval   setup  execution
 * 
 * 
 * 
 * Константа нужна в сигнатуре константы/функции при её определении
 * Ссылка (адрес) нужен и при определнии и при вызове функции
 * func( & ref = default ):={}
 * %func( ref = &value );  func( &value );
 * func( %const = default ):={}
 * func( & %const_ref = default ):={}
 * const := 123; # Переменная
 * %const := 123; # Константа
 * %const := 123; # Константа
 * 
 * :class%const   определение констант снаружи класса
 * :class%const() а внури класса 
 * 
 * Один конструктор для классов (несколько конструкторов ненужно, т.к. есть именованые аргументы и произольноекол-во параметров)
 * 
 * :class (<аргументы конструктора>) := :base1(), :base2() {
 * 
 * 
 * 
 *   конструктор всегда статический (функция класса, т.к. объект отсутствует)
 *   ::class::class(<аргументы должны совпадать ???????  >) ::= <конструктор>;
 *   :class::class() ::= {<конструктор>};
 *   :class::class() ::= {{<конструктор>}};
 * 
 * # Инициализация для функции  demo
 * @::var = _;
 * @print := NewLang(import= «printf(format:Format, ...):Int» );
 * --;
 * 
 * %{   std::string demo(const char *arg) {; $var = arg;  %}
 *      (arg === _ ) --> {
 *                      @print(«Нет аргументов»);
 *                      %{ return std::string(«Нет аргументов»); %}
 *                  },
 *      _ --> {
 *              @print(«Передан %s», $var);
 *              %{ return static_cast<std::string>($var); %}                   
 *      }
 * %{ }  %}  # End demo func
 * 
 *   Для реализации DSL, единая точка входа переименованием аргументов
 *   NewLang(eval=_, import=_, exec=_, load=_, compile=_, file=_,) ::= {{
 *      $* ==> {  # Выполняется точное сравнение
 *          (,) --> help();
 *          (eval='',) --> %{ NewLang::eval(static_cast<const char *>($eval)); %};
 *          (import='',), (import="",) --> %{ NewLang::import(static_cast<const char *>($import)); %};
 *          (exec:String=,) --> %{ NewLang::exec(static_cast<const char *>($exec)); %};
 *          (compile:String=, file:String, ) --> %{ NewLang::compile(static_cast<const char *>($compile), static_cast<const char *>($file)); %};
 *      }
 *   }}
 *   NewLang("eval string");
 *   NewLang(import='class():class');
 *   NewLang(file="eval file name");
 *   NewLang(import="import object");
 *   NewLang(compile="import object");
 *   NewLang(load="module_name", init=1);
 * 
 *   __constructor1() := NewLang(import = 'class():class');
 *   __constructor2(arg) := NewLang(import = 'class(arg:Int):class');
 *   __constructor3(arg) := NewLang(import = 'class(arg:StrChar):class');
 * 
 *   :class::class(arg=_) ::= {
 *      arg:? => { # Выполняется не точное сравнение c приведеним типов
 *          :None -> __constructor1();
 *          :Int -> __constructor2(arg);
 *          :String -> __constructor3(arg);
 *          _ -> -- "Неизвестный тип аргумента" --;
 *      }
 *  }
 * 
 * Проблема перегразки функций разными типами аргументов специфична только для компилируемых языков сос ставтической ьтипизацией
 * Для языков с динамической типизацией перегразука функций не требуется, т.к. типы аргументов могут быть произвольными
 * Но перегрузка фунций в языке со статической типизаице может еще исолпьзоваться и для замены одной реалихациина функции 
 * на другую если типы аргументов различаются, что эквивалентно замене (добавлению) новой функции для другого типа аргументов.
 * 
 * Как сделать замену одной реалихации фнуции на дургуо для языка с динамиечской типизацией без перегрузки функций?
 * 1. Сохранять предыдущую функцию в реализации (нужнны локальные статические переменные и/или деструкторы)
 * 2. Добавлять новую реализацию функции средствами языка (не нужны локальные статические переменные и деструкторы, 
 * но нужно перечисление списка функций чтобы вызвать предыдущую реализацию. Все функции можно вытащить итератором)
 * 
 * Связанный вопрос - пересечения имен у переменный и функций и их уникальности в одной области видимости.
 * Таготею к Эсперанто, по струкутре слова часто можно понять смысл и часть речи и нет двойных смыслов
 * 
 * Для глобальных объектов - имена уникальны, но есть возможност добавлять несколько варинатов реализации одного и того же термина 
 * (новый термин заменяет старый, но в новом есть возможност вызывать предыдущий вариант реализации).
 * 
 * Для локальных объектов - имена <перекрываются>, а объекты не "заменяются", а "перекрываются".
 * Это получается из-за того, что к локальным объектам кроме имени можно обратить по <индексу>, а к глобальным только по имени.
 * Локальный объект добаляется в начало списка, а его имя ищется последовательно (потом можно будет прикрутить хешмап)
 * 
 * super() ??????????????
 * 
 *  func(arg) []= { # Добавить(переопределить) фукнцию/объект
 *      (arg ~ type) -> -- "Переопределенное значение";
 *      -- $$(arg);  # Вызвать родительский объект
 *   }
 * 
 *   деструктор всегда метод объекта
 *   Деструктор импортировать НЕ НУЖНО, т.к. для класса он должне вызываться автоматически (сделать shared_ptr)
 *   но сам по себе деструктор нужен для действий внутри NewLang
 *   ~class() ::= <деструктор>;
 *   ~class() ::= {<деструктор>};
 *   ~class() ::= {{<деструктор>}};
 * 
 * все метод классов, кроме статических - вирутальные (могут быть переопределены в наследниках)
 * Класс с чистым вирутальным методом (None) создать можно, но вызвать метод нельзя (будет ошибка)
 * Интерфейсы ненужны, т.к. есть утиная типизация
 * 
 * Ссылка на статические поля класса
 * Ссылка на статические методы класса
 * Ссылка на поля класса
 * Ссылка на методы класса
 * Конструкторы
 * Дестурктор
 * 
 * Derived2::*Derived2_mfp
 * 
 * 
 * https://docs.microsoft.com/ru-ru/cpp/build/reference/decorated-names?view=msvc-160
 * Формат внутреннего имени C++
 * Внутреннее имя функции C++ содержит следующие сведения.
 * - Имя функции.
 * - Класс, членом которого является функция, если это функция-член. Это может быть класс, в который входит содержащий функцию класс, и т. д.
 * - Пространство имен, которой принадлежит функция, если она входит в пространство имен.
 * - Типы параметров функции.
 * - Соглашение о вызовах.
 * - Тип значения, возвращаемого функцией.
 * Имена функций и классов кодируются во внутреннем имени. Остальная часть внутреннего имени — это код, 
 * который имеет смысл только для компилятора и компоновщика. 
 * 
 * 
 * 
 * 
 *   :class.var_class (static) := "";
 *   :class.var_class (static) := 1; # публичное поле объекта
 *   :class._var_class (static) := 1; # защиенное поле объекта
 *   :class.__var_class (static) := 1; # приватное поле объекта
 *   :class.__var_class__ (static) := 1; # Системное 
 * 
 *   var := 1; # публичное поле объекта
 *   _var := 1; # защиенное поле объекта
 *   __var := 1; # приватное поле объекта
 *   __var__ := 1; # Системное 
 *   var1, var2, var3:Int := _;  # Переменная объекта
 * 
 *   :class.var_class  - к статическому полю
 * 
 *   :class::base2.field  - к статическому полю
 *   class::base2.field  - к обычному полю
 * 
 *   value_at_class := eval at create type;  # Код инициализации во время создания типа класса
 *   value_at_object := {  # Код инициализации во время создания экземпляра класса
 *      eval at create object
 *   };  
 *   value_at_object_no_error := {{ # Код инициализации во время создания экземпляра класса без ошибок
 *      eval at create object
 *   }};  
 * 
 *   func_at_class() := eval at create type;  # Тело функции формируется во время создания типа класса (может возвращаться другой функцией)
 *   func() := {  # Код функции для вызова с генерацией исключений
 *      eval at create object
 *   };  
 *   func_no_error() := {{ # Код функции для вызова без генерации исключений
 *      eval at create object
 *   }}; 
 *  
 * };
 * 
 * 
 * <условный оператор FOLLOW>  ::=  
 * 
 * <условный оператор if> ::= if <булево выражение> then <оператор> [else <оператор>]
 * <булево выражение> ::= "NOT" <булево выражение>
 *     | <булево выражение> <логическая операция> <булево выражение>
 *     | <выражение> <операция сравнения> <выражение>
 * <логическая операция> ::= "OR" | "AND"
 * <выражение> ::= <переменная> | <строка> | <символ>
 * <операция сравнения> ::= "=" | "<" | ">"
 * 
 * 
 * 
 * 
 * https://habr.com/ru/post/141756/
 * 
 * Для начала опишем нужную часть грамматики с помощью расширенной формы Бэкуса-Наура (РБНФ) (wiki).


class_def = CLASS classname [inheritance] COLON suite
classname = ID
inheritance = LBRACE class_args_list RBRACE
class_args_list = [class_arg]
class_arg = dotted_name {COMMA dotted_name}
dotted_name = ID {DOT ID}


Здесь [X] — 0 или 1 вхождение X, {Y} — 0 и более вхождений Y.

Для определения имени класса и его зависимостей вполне достаточно. Теперь для функций.


func_def = DEF funcname LBRACE [func_args_list] RBRACE COLON suite
funcname = ID
func_args_list = [func_arg]
func_arg = (dotted_name | star_arg) {OTHER | COMMA | dotted_name | star_arg | MESSAGE}
star_arg = [STAR] STAR ID


Кстати, предполагается, что код пользователя будет корректным (с точки зрения интерпретатора), иначе правила грамматики надо определять строже.
 * 
 */


%token                  INTEGER		"Integer"
%token                  NUMBER		"Number"
%token                  COMPLEX		"Complex"
%token                  FRACTION	"Fraction"
%token                  CURRENCY	"Currency"
%token           	STRCHAR		"StrChar"
%token           	STRWIDE		"StrWide"
%token           	TEMPLATE	"Template"

%token			TERM            "Term"
%token			SYMBOL          "Symbol"

%token			UNKNOWN		"Token ONLY UNKNOWN"
%token			OPERATOR_DIV

%token			NEWLANG		"@@"
%token			PARENT		"$$"
%token			ARGS		"$*"
%token			EXIT		"--"
%token			MACRO		"Macro"
%token			MACRO_BODY      "Macro body"

%token			CREATE		"::="
%token			CREATE_OR_ASSIGN ":="
%token			APPEND		"[]="

%token			TENSOR_BEGIN
%token			TENSOR_END
%token			TRY_BEGIN
%token			TRY_END
%token			BLOCK_TRY
%token			MIDDLE_CALL_TRY
%token			MIDDLE_CALL_BLOCK


%token			FOLLOW		"->"
%token			MATCHING	"=>"
%token			WHILE		"-->>"
%token			UNTIL		"<<--"

%token			ARGUMENT		


%token			RANGE           ".."
%token			ELLIPSIS        "..."
%token			NAMESPACE       "::"
%token			ASSIGN_MATH     "Arithmetic assign value"

%token			END         0	"end of file"

%token                  SOURCE
%token			ITERATOR        "!!"
%token			ITERATOR_QQ     "??"

%token			TRANSPARENT
%token			SIMPLE_AND
%token			SIMPLE_OR
%token			SIMPLE_XOR
%token			OPERATOR


/* Есть предупреждения, связанные с выполняемым кодом
 * shift/reduce conflict on token '{'
 * shift/reduce conflict on token '%{'
 *  */
//%expect 2
/* Можно сделать проверку синтаксиса для чистых функций на уровне парсера, но придется делать два варианта основных операций 
 * (кода, выражений, логики и т.д.) только с разрешенными объектами */

/*
%left '+' '-'
%left '*' '/'
%nonassoc ABS NEG
*/

/* %glr-parser */
/*
%right '='  CREATE  CREATE_OR_ASSIGN   APPEND
%left  '-' '+'
%left  '*' '/' '%'
%left  NEG     
%right CONCAT  POWER    // Exponentiation
*/

/*%left  '-' '+'
%left  '*' '/' '%' */
/*%left  '-' '+'
%left  NEG */

//%expect 2
/* () - вызов без аргументов может быть rcall и lcall 
 * ; - разделители игнорируются в любом количестве
 */

%% /*** Grammar Rules ***/


/* Фиксированная размерность тензоров для использования в типах данных */
type_dim: INTEGER
        {
            $$ = $1;
        }
    |  ELLIPSIS
        {
            // Произвольное количество элементов
            $$ = $1; 
        }
/*    | TERM  '='  INTEGER
        { // torch поддерживает именованные диапазоны, но пока незнаю, нужны ли они?
            $$ = $3;
            $$->SetName($1->getText());
        } */

type_dims: type_dim
        {
            $$ = $1;
        }
    | type_dims  ','  type_dim
        {
            $$ = $1;
            $$->AppendSeq($3);
        }

type_class:  ':'  TERM
            {
                $$ = $2;
                $$->m_text.insert(0, ":");
            }

type_name:  type_class
            {
                $$ = $1;
            }
        |  type_class   '['  type_dims   ']'
            {
                $$ = $1;
                $$->SetDims($type_dims);
            }
        | ':'  '&'  TERM
            {
                // Для функций, возвращаюющих ссылки
                $$ = $3;
                $$->m_text.insert(0, ":");
                $$->MakeRef();
            }
        | ':'   '&'   TERM   '['  type_dims   ']'
            {
                // Для функций, возвращаюющих ссылки
                $$ = $3;
                $$->m_text.insert(0, ":");
                $$->SetDims($type_dims);
                $$->MakeRef();
            }


type:  type_name
            {
                $$ = $1;
                $$->SetTermID(TermID::TYPE);
            }
        | type_name   call
            {
                $$ = $1;
                $$->SetArgs($2);
                $$->SetTermID(TermID::TYPE_CALL);
            }
        | ':'  strtype 
            {
                // Если тип еще не определён и/или его ненужно проверять во время компиляции, то имя типа можно взять в кавычки.
                $$ = $2;
                $$->SetTermID(TermID::TYPENAME);
                $$->m_text.insert(0, ":");
            }

        
        

digits_literal: INTEGER
            {
                $$ = $1;
                $$->SetType(nullptr);
            }
         | NUMBER
            {
                $$ = $1;
                $$->SetType(nullptr);
            }
         | COMPLEX
            {
                $$ = $1;
                $$->SetType(nullptr);
            }
         | FRACTION
            {
                $$ = $1;
                $$->SetType(nullptr);
            }
        
digits:  digits_literal
            {
                $$ = $1;
            }
        | digits_literal  type
            {
                $$ = $1;
                $$->SetType($2);
            }

        
        
range_val:  rval_name
        {
            $$ = $1;  
        }
    |  digits
        {
            $$ = $1;
        }
/*    |  '-'  digits
        {
            $$ = $2;
            $$->m_text.insert(0,"-");
        } */
            
        
range: range_val  RANGE  range_val
        {
            $$ = $2;
            $$->push_back($1, "start");
            $$->push_back($3, "stop");
        }
    | range_val  RANGE  range_val  RANGE  range_val
        {
            $$ = $2;
            $$->push_back($1, "start");
            $$->push_back($3, "stop");
            $$->push_back($5, "step");
        }
        
        
        
        
strtype: STRWIDE
        {
            $$ = $1;
            $$->SetType(nullptr);
        }
    | STRCHAR
        {
            $$ = $1;
            $$->SetType(nullptr);
        }

string: strtype
        {
            $$ = $1;
        }
    | strtype  call
        {
            $$ = $1;
            $$->SetArgs($2);
        }
   
/* Незнаю, нужны ли теперь символы? Раньше планировалось с их помощью расширять синтаксис языковых конструкций, 
 * но это относится к парсеру и не может изменяться динамически в зависимости от наличия существующий объектов и определений.
 * Если относится к символам как к идентификаторам на других языках, то опять же это лучше делать на уровне лексера и парсера,
 * чтобы еще при обработке исходников вместо создания неопределнных последовательностьей возникала ошибка времени компиляции,
 * а не передача отдельных символов как не распознанных терминалов.
 */
symbols: SYMBOL
            {
                $$ = $1;
            }
        | symbols  SYMBOL
            {
                $$ = $1; 
                $$->AppendText($2->getText());
            }

name_fragment:  TERM  NAMESPACE
            {
                $$ = $1;
                $$->m_namespace.swap($$->m_text);
                $$->m_namespace += "::";
            }
            
namespace:  name_fragment
            {
                $$ = $1;
            }
        |  namespace  name_fragment
            {
                $$ = $1;
                $$->m_namespace += $2->m_namespace;
            }
        | NAMESPACE name_fragment
            {
                $$ = $2;
                $$->m_namespace.insert(0, "::");
            }

        
/* Допустимые имена переменных и функций. Деструктор с ~ выявляеся на этапе анализа, а не парсинга, т.к. это обычный термин*/
name:  TERM
            {
                  $$ = $1;
            }
        |  namespace  TERM
            {
                $$ = $2;  
                $$->m_namespace.swap($1->m_namespace);
            }


        
/* Допустимые <имена> объеков */
assign_name:  name
                {
                    $$ = $1;
                }
            |  symbols
                {
                    $$ = $1;  
                }
            |  namespace  symbols
                {
                    $$ = $2;  
                    $$->m_namespace.swap($1->m_namespace);
                }
            | ARGUMENT  /* $123 */
                {
                    $$ = $1;
                }
            |  PARENT  /* $$ - rval */
                {
                    $$ = $1; 
                }
       
/* Допустимые lvalue объекты */
lval:  assign_name
            {
                $$ = $1;
            }
        | '&' assign_name
            {
                $$ = $2;
                $$->MakeRef();
            }
        /* Не могут быть ссылками*/
        |  assign_name  '['  args  ']'
            {   
                $$ = $1; 
                $2->SetTermID(TermID::INDEX);
                $2->SetArgs($args);
                $$->Last()->Append($2);
            }
        |  lval  '.'  TERM
            {
                $$ = $1; 
                $3->SetTermID(TermID::FIELD);
                $$->Last()->Append($3);
            }
        |  lval  '.'  TERM  call
            {
                $$ = $1; 
                $3->SetTermID(TermID::FIELD);
                $3->SetArgs($call);
                $$->Last()->Append($3);
            }
        |  type
            {   
                $$ = $1; 
            }
        |  name  type
            {   
                $$ = $1; 
                $$->SetType($type);
            }
        |  name  call
            {   
                $$ = $name; 
                $$->SetTermID(TermID::CALL);
                $$->SetArgs($call);
            }
        |  name  call  type
            {   
                $$ = $name; 
                $$->SetTermID(TermID::CALL);
                $$->SetArgs($call);
                $$->SetType($type);
            }

rval_name: lval
            {
                $$ = $1; 
            }
        | NEWLANG  call /* @@ - rval */
            {
                $$ = $1; 
                $$->SetTermID(TermID::CALL);
                $$->SetArgs($2);
            }
        | ARGS /* $* - rval */
            {
                $$ = $1;
            }
/*        | GLOBAL  @*  - rval
            {
                $$ = $1;
            } */

        
       
rval_var:  rval_name
            {
                $$ = $1;
            }
        |  rval_name  iter_all
            {
                $$ = $2;
                $$->SetTermID(TermID::ITERATOR);
                $$->Last()->Append($1, Term::LEFT); 
            }
        |  rval_name  iter_all  call
            {
                $$ = $2;
                $$->SetTermID(TermID::ITERATOR);
                $$->Last()->Append($1, Term::LEFT); 
                $$->SetArgs($call);
            }
        |  string
            {
                $$ = $1;
            }
        | digits
            {
                $$ = $1;
            }
        |  collection
            {
                $$ = $1;
            }
        |  range
            {
                $$ = $1;
            }
        
        
        
rval:   rval_var
            {
                $$ = $1;
            }


iter_all:  '!'
            {
                $$=$1;
            }
        | '?'
            {
                $$=$1;
            }
        | ITERATOR
            {
                $$=$1;
            }
        | ITERATOR_QQ
            {
                $$=$1;
                $$->SetTermID(TermID::ITERATOR);
            }
       

/*
 * Порядок аргументов проверяется не на уровне парсера, а при анализе объекта, поэтому 
 * в парсере именованные и не именованные аргуметы могут идти в любом порядке и в любом месте.
 * 
 * Но различаются аругменты с левой и правой стороны от оператора присвоения!
 * С левой стороны в скобках указывается прототип функции, где у каждого аргумента должно быть имя, может быть указан тип данных 
 * и признак ссылки, а последним оператором может быть многоточие (т.е. произвольное кол-во аргументов).
 * С правой стороны в скобках происходит вызов функции (для функции, которая возвращает ссылку, перед именем "&" не ставится),
 * а перед аргументами может стоять многоточие или два (т.е. операторы раскрытия словаря).
 * 
 * <Но все это анализирутся тоже после парсера на уровне компилятора/интерпретатора!>
 * 
 */
        
    

/* Аргументом может быть что угодно */
arg: name  '='
        {  // Именованный аргумент
            $$ = $2;
            $$->m_name.swap($1->m_text);
            $$->SetTermID(TermID::EMPTY);
        }
    | name  type  '='
        { // Именованный аргумент
            $$ = $3;
            $$->SetType($type);
            $$->m_name.swap($1->m_text);
            $$->SetTermID(TermID::EMPTY);
        }
    | name  '='  rval
        { // Именованный аргумент
            $$ = $rval;
            $$->SetName($1->getText());
        }
    | name  type  '='  rval
        { // Именованный аргумент
            $$ = $rval;
            $$->SetType($type);
            $$->SetName($1->getText());
        }
    | rval
        {
            // сюда попадают и именованные аргументы как операция присвоения значения
            $$ = $1;
        }
    |  ELLIPSIS
        {
            // Раскрыть элементы словаря в последовательность не именованных параметров
            $$ = $1; 
        }
    |  ELLIPSIS  rval
        {
            // Раскрыть элементы словаря в последовательность не именованных параметров
            $$ = $1; 
            $$->Append($rval, Term::RIGHT);
        }
    |  ELLIPSIS  ELLIPSIS  rval
       {            
            // Раскрыть элементы словаря в последовательность ИМЕНОВАННЫХ параметров
            $$ = $2;
            $$->Append($1, Term::LEFT); 
            $$->Append($rval, Term::RIGHT); 
        }

args: arg
            {
                $$ = $1;
            }
        | args  ','  arg
            {
                $$ = $1;
                $$->AppendSeq($3);
            }
        
        
call:  '('  ')'
            {   
                $$ = $1;
                $$->SetTermID(TermID::END);
            }
        | '('  args   ')'
            {
                $$ = $2;
            }
        
        
array: '['  args  ','  ']'
            {
                $$ = $1;
                $$->m_text.clear();
                $$->SetTermID(TermID::TENSOR);
                $$->SetArgs($args);
            }
        | '['  args  ','  ']'  type
            {
                $$ = $1;
                $$->m_text.clear();
                $$->SetTermID(TermID::TENSOR);
                $$->SetArgs($args);
                $$->SetType($type);
            }
        | '['  ','  ']'  type
            {
                // Не инициализированый тензор должен быть с конкретным типом 
                $$ = $1;
                $$->m_text.clear();
                $$->SetTermID(TermID::TENSOR);
                $$->SetType($type);
            }

            
tensor:   TENSOR_BEGIN   TENSOR_END
                {
                    $$ = $1;
                }
            | TENSOR_BEGIN  TENSOR_END  type
                {
                    $$ = $1;
                    $$->SetType($type);
                }
            |  TENSOR_BEGIN   args  TENSOR_END
                {
                    $$ = $1;
                    $$->SetArgs($2);
                }
            | TENSOR_BEGIN  args  TENSOR_END  type
                {
                    $$ = $1;
                    $$->SetArgs($2);
                    $$->SetType($type);
                }
//            | TENSOR_BEGIN  args  TENSOR_END  '['  type_dims   ']'
//                {
//                    $$ = $1;
//                    $$->SetArgs($2);
//                    // Изменение размерности тензора без конвертирования типа данных
//                    $4->m_text.clear();
//                    $4->SetDims($type_dims);
//                    $$->SetType($4);
//                }


dictionary: '('  ','  ')'
            {
                $$ = $1;
                $$->m_text.clear();
                $$->SetTermID(TermID::DICT);
            }
        | '('  args  ','  ')'
            {
                $$ = $1;
                $$->m_text.clear();
                $$->SetTermID(TermID::DICT);
                $$->SetArgs($2);
            }


class:  dictionary
            {
                $$ = $1;
            }
        | dictionary   type
            {
                $$ = $1;
                $$->SetType($2);
            }
            
           
collection: array 
            {
                $$ = $1;
            }
        | tensor
            {
                $$ = $1;
            }
        | class
            {
                $$ = $1;
            }


      
           
assign_op:  /* '='  
            {
                // ASSIGN
                $$ = $1;
                $$->SetTermID(TermID::ASSIGN);
            }
        | */ CREATE_OR_ASSIGN /* := */
            {
                $$ = $1;
            }
        | CREATE /* ::= */
            {
                $$ = $1;
            }
        | APPEND /* []= */
            {
                $$ = $1;
            }
        | TRANSPARENT /* ::- :- */
            {
                $$ = $1;
            }


assign_expr:  block
                {
                    $$ = $1;  
                }
            | condition
                {
                    $$ = $1;  
                }


assign_item:  lval
                {
                    $$ = $1;
                }
            |  ELLIPSIS
                {
                    $$ = $1;
                }

assign_seq: assign_item
                {
                    $$ = $1;
                }
/*            |  assign_seq  ','  assign_item
                {
                    $$ = $1;
                    $$->AppendSeq($3);
                } */
/*
lval = rval;
lval, lval, lval = rval;
func() = rval;
func() = simple, simple, simple;
*/
assign:  assign_seq  assign_op  assign_expr
            {
                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }
        | assign_seq  '='  assign_expr
            {
                $$ = $2;  
                $$->SetTermID(TermID::ASSIGN);
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }

            
body:   rval
            {
                $$ = $1;
            }
        |  block
            {
                $$ = $1;
            }

        
simple_seq:  condition
                {
                    $$ = $1;
                }
            | simple_seq ',' condition
                {
                    $$ = $1;
                    // Несколько команд подряд
                    $$->ConvertToBlock(TermID::BLOCK);
                    $$->BlockCodeAppend($3); 
                }

            
simple: SIMPLE_AND
        {
            $$ = $1;
        }
    | SIMPLE_OR
        {
            $$ = $1;
        }
    | SIMPLE_XOR
        {
            $$ = $1;
        }

/* Допустимые <имена> функций. Они тоже могут возвращать ссылки на объекты, но тип взвращаемого значения стоит после аргументов */    
simple_func:  lval  simple  simple_seq
        {
            $$ = $2;  
            $$->Append($1, Term::LEFT); 
            $$->Append($3, Term::RIGHT);
        }
         

block:  '{'  '}'
            {
                $$ = $1; 
                $$->SetTermID(TermID::BLOCK);
            }
        | '{'  sequence  '}'
            {
                $$ = $2; 
                $$->ConvertToBlock(TermID::BLOCK);
            }
        | '{'  sequence  separator  '}'
            {
                $$ = $2; 
                $$->ConvertToBlock(TermID::BLOCK);
            }
        | TRY_BEGIN  TRY_END
            {
                $$ = $1; 
                $$->ConvertToBlock(TermID::BLOCK_TRY);
            }
        | TRY_BEGIN  sequence  TRY_END
            {
                $$ = $2; 
                $$->ConvertToBlock(TermID::BLOCK_TRY);
            }
        | TRY_BEGIN  sequence  separator  TRY_END
            {
                $$ = $2; 
                $$->ConvertToBlock(TermID::BLOCK_TRY);
            }
        | TRY_BEGIN  sequence  TRY_END  type_class
            {
                $$ = $2; 
                $$->ConvertToBlock(TermID::BLOCK_TRY);
                $$->SetType($type_class);
            }
        | TRY_BEGIN  sequence  separator  TRY_END  type_class
            {
                $$ = $2; 
                $$->ConvertToBlock(TermID::BLOCK_TRY);
                $$->SetType($type_class);
            }

block_call:  '('  MIDDLE_CALL_BLOCK  sequence  '}' 
            {
                $$ = $sequence; 
                $$->ConvertToBlock(TermID::CALL_BLOCK);
            }
        | '('  MIDDLE_CALL_BLOCK  sequence  separator '}' 
            {
                $$ = $sequence; 
                $$->ConvertToBlock(TermID::CALL_BLOCK);
            }
        |  '('  args  MIDDLE_CALL_BLOCK  sequence  '}'
            {
                $$ = $sequence; 
                $$->SetArgs($args);
                $$->ConvertToBlock(TermID::CALL_BLOCK);
            }
        | '('  args  MIDDLE_CALL_BLOCK  sequence  separator  '}'
            {
                $$ = $sequence; 
                $$->SetArgs($args);
                $$->ConvertToBlock(TermID::CALL_BLOCK);
            }
        | '('  MIDDLE_CALL_TRY  sequence  TRY_END
            {
                $$ = $sequence; 
                $$->ConvertToBlock(TermID::CALL_TRY);
            }
        | '('  MIDDLE_CALL_TRY  sequence  separator  TRY_END
            {
                $$ = $sequence; 
                $$->ConvertToBlock(TermID::CALL_TRY);
            }
        | '('  args  MIDDLE_CALL_TRY  sequence  TRY_END
            {
                $$ = $sequence; 
                $$->SetArgs($args);
                $$->ConvertToBlock(TermID::CALL_TRY);
            }
        | '('  args  MIDDLE_CALL_TRY  sequence  separator  TRY_END
            {
                $$ = $sequence; 
                $$->SetArgs($args);
                $$->ConvertToBlock(TermID::CALL_TRY);
            }
        | '('  args  MIDDLE_CALL_TRY  sequence  TRY_END  type_class
            {
                $$ = $sequence; 
                $$->SetArgs($args);
                $$->ConvertToBlock(TermID::CALL_TRY);
                $$->SetType($type_class);
            }
        | '('  args  MIDDLE_CALL_TRY  sequence  separator  TRY_END  type_class
            {
                $$ = $sequence; 
                $$->SetArgs($args);
                $$->ConvertToBlock(TermID::CALL_TRY);
                $$->SetType($type_class);
            }

        

/* 
 * lvalue - объект в памяти, которому может быть присовено значение (может быть ссылкой и/или константой)
 * rvalue - объект, которому <НЕ> может быть присвоено значение (литерал, итератор, вызов функции)
 * Все lvalue могут быть преобразованы в rvalue. 
 * eval - rvalue или операция с rvalue. Возвращает результат выполнения <ОДНОЙ операции !!!!!!!>
 * 
 * Операции присвоения используют lvalue, многоточие или определение функций
 * Алгоритмы используют eval или блок кода (у matching)
 */

/* lvalue объекты с вызовом как у функций 
eval:   lval
            {
                $$ = $1; 
            }
        | lval  '('  ')'
            {
                $$ = $1; 
                $$->SetTermID(TermID::CALL);
            }
        | lval '('  args_all  ')'
            {
                $$ = $1; 
                $$->SetTermID(TermID::CALL);
                $$->SetArgs($args_all);
            }*/

operator: OPERATOR
            {
                $$ = $1;
            }


arithmetic:  addition
                { 
                    $$ = $1; 
                }
            |  arithmetic '+' addition
                { 
                    $$ = $2;
                    $$->SetTermID(TermID::OPERATOR);
                    $$->Append($1, Term::LEFT);                    
                    $$->Append($3, Term::RIGHT); 
                }
            | arithmetic '-'  addition
                { 
                    $$ = $2;
                    $$->SetTermID(TermID::OPERATOR);
                    $$->Append($1, Term::LEFT);                    
                    $$->Append($3, Term::RIGHT); 
                }
            | '-'  addition /*  %prec NEG */
                { 
                    $$ = $2; 
                }
            |  digits   digits
                {
                    //@todo location
                    $$ = Term::Create(TermID::OPERATOR, $2->m_text.c_str(), 1, nullptr);
                    $$->Append($1, Term::LEFT); 
                    $2->m_text = $2->m_text.substr(1);
                    $$->Append($2, Term::RIGHT); 
                }


op_factor: '*'
            {
                $$ = $1;
            }
        |  '/'
            {
                $$ = $1;
            }
        |  OPERATOR_DIV
            {
                $$ = $1;
            }
        |  '%'
            {
                $$ = $1;
            }
        
addition:   addition  op_factor  factor 
                { 
                    $$ = $2;
                    $$->SetTermID(TermID::OPERATOR);
                    $$->Append($1, Term::LEFT); 
                    $$->Append($3, Term::RIGHT); 
                }
/*            | '-'  addition  %prec NEG
                { 
                    $$ = $2; 
                } */
    
/*            |  digits  digits  op_factor  factor 
                {
#warning PRIORITET OP
                    //@todo location
                    TermPtr temp = Term::Create(TermID::OPERATOR, $2->m_text.c_str(), 1, nullptr);
                    temp->Append($1, Term::LEFT); 
                    $2->m_text = $2->m_text.substr(1);
                    temp->Append($2, Term::RIGHT); 

                    $$ = $3;
                    $$->SetTermID(TermID::OPERATOR);
                    $$->Append(temp, Term::LEFT); 
                    $$->Append($4, Term::RIGHT); 
                } */
            |  sfactor
                { 
                    $$ = $1; 
                }    
            

sfactor: factor              
            { 
                $$ = $1; 
            }

factor:   rval_var
            {
                $$ = $1; 
            }
        | '('  arithmetic  ')'
            { 
                $$ = $2; 
            }
/*        | '-'  rval   %prec  NEG
            {
                $$ = $2; 
            } */

       
   
source: SOURCE
            {
                $$=$1;
            }
        |  source  SOURCE
            {
                $$=$1;
                $$->Last()->Append($2); 
            }
        
condition: source
            {
                $$=$1;
            }
        | logical
            {
                $$ = $1;
            }
        | exit
            {
                $$=$1;
            }
/*        | range
            {
                $$ = $1;  
            }
*/

        
logical:  arithmetic
            {
                    $$ = $1;
            }
        |  arithmetic operator arithmetic
            {
                $$ = $2;
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }
        | arithmetic  '~'  arithmetic
            {
                $$ = $2;
                $$->SetTermID(TermID::OPERATOR);
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }

        
        


       
exit:  EXIT
        {
            $$ = $1;
        }

    |  EXIT  rval_var
        {
            $$ = $1;
            $$->Append($2, Term::RIGHT); 
        }
    |  EXIT  rval_var  EXIT
        {
            $$ = $1;
            $$->Append($2, Term::RIGHT); 
            $$->SetTermID(TermID::ERROR);
        }

      

follow:  if_then_body


if_then_body: if_then
            {
                $$ = $1; 
                $$->AppendFollow($1);
            }
        | follow  ','  if_then
            {
                $$ = $1; 
                $$->AppendFollow($3);
/*                $2->Append($3, Term::RIGHT); 
                $$->AppendFollow($1);
                $$->AppendFollow($2); */
            }
        
match_cond: /*rval
            {
                $$ = $1;
            }
        |  */'[' condition ']' 
            {
                $$ = $condition;
            }
        
if_then:  match_cond  FOLLOW  body
            {
                $$=$2;
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }
/*        |  '[' match_cond ']'  FOLLOW  body
            {
                $$=$4;
                $$->Append($2, Term::LEFT); 
                $$->Append($5, Term::RIGHT); 
            } */

        
  
   
/* term -->> {};
 * (op) -->> {};
 * {} <<-- term;
 * {} <<-- (op);
 */        
repeat:  /*rval_name  WHILE  body
            {
                $$=$2;
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }
        | */ match_cond  WHILE  body
            {
                $$=$2;
                $$->Append($match_cond, Term::LEFT); 
                $$->Append($body, Term::RIGHT); 
            }
/*        |  body  UNTIL  rval_name
            {
                $$=$2;
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            } */
        |  body  UNTIL  match_cond
            {
                $$=$2;
                $$->Append($body, Term::LEFT); 
                $$->Append($match_cond, Term::RIGHT); 
            }

matches:  rval_name
            {
                $$=$1;
            }
        |  matches  ','  rval_var
            {
                $$ = $1;
                $$->AppendSeq($3);
            }        
        
match_item: '[' matches  ']' FOLLOW  body
            {
                $$=$4;
                $$->Append($2, Term::LEFT); 
                $$->Append($5, Term::RIGHT); 
            }

match_items:  match_item
            {
                $$ = $1;
            }
        | match_items  ';'  match_item
            {
                $$ = $1; 
                $$->AppendFollow($3);
            }
        
match_body:  '{'  match_items  '}'
            {
                $$ = $2;
            }



match:  '['  match_cond ']'  MATCHING  match_body
            {
                $$=$2;
/*                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); */
            }
        |  block  MATCHING  match_body
            {
                $$=$2;
/*                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); */
            }

/*  expression - одна операция или результат <ОДНОГО выражения без завершающей точки с запятой !!!!!> */
seq_item: condition
            {
                $$=$1;
            } 
/*        | condition  end
            {
                $$=$1;
            } */
        | simple_func
            {
                $$=$1;
            }
        | assign
            {
                $$ = $1;
            }
        | follow
            {
                $$ = $1; 
            }
        | match
            {
                $$ = $1; 
            }
        | repeat
            {
                $$ = $1; 
            }
        |  MACRO  assign_op  MACRO_BODY
            {
                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }
        
sequence:  seq_item
            {
                $$ = $1;  
            }
        | block_call
            {
                $$ = $1;  
            }
        | sequence  separator  block_call 
            {
                $$ = $1;  
                // Несколько команд подряд
                $$->ConvertToBlock(TermID::BLOCK);
                $$->BlockCodeAppend($block_call); 
            }
        | sequence  separator  seq_item
            {
                $$ = $1;  
                // Несколько команд подряд
                $$->ConvertToBlock(TermID::BLOCK);
                $$->BlockCodeAppend($seq_item); 
            }

        
separator: ';' | separator  ';'        
        
//execution:  block_call
//            {
//                $$ = $1;  
//            }
//        | execution  separator  block_call
//            {
//                $$ = $1; 
//                $$->ConvertToBlock(TermID::BLOCK);
//                $$->BlockCodeAppend($3); 
//            }

ast:    END
        | separator
        | sequence
            {
               driver.AstAddTerm($1);
            }
        | sequence separator
            {
               driver.AstAddTerm($1);
            }
//        | execution separator
//            {
//               driver.AstAddTerm($1);
//            }

start	:   ast

%% /*** Additional Code ***/
