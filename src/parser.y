
%{ /*** C/C++ Declarations ***/

#include "pch.h"

#include <term.h>
#include <context.h>

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
 * Проблема перегоузки функций разными типами аргументов специфична только для компилируемых языков со статической типизацией
 * Для языков с динамической типизацией, перегразука функций не требуется, т.к. типы аргументов могут быть произвольными
 * Но перегрузка фунций в языке со статической типизацие может еще использоваться и для замены одной реализациина функции 
 * на другую, если типы аргументов различаются, что эквивалентно замене (добавлению) новой функции для другого типа аргументов.
 * 
 * Как сделать замену одной реализации фнуции на другую для языка с динамиеческой типизацией без перегрузки функций?
 * 1. Сохранять указатель на предыдущую функцию в новой реализации, тогда  нужнны локальные статические переменные и/или деструкторы.
 * 2. Управлять именами функций средствами языка (не нужны локальные статические переменные и деструкторы, 
 * но нужна лексическая контструкция чтобы указать предыдущую реализацию, а весь список функций можно вытащить итератором)
 * 
 * Связанный вопрос - пересечения имен у переменный и функций и их уникальность в одной области видимости.
 * Таготею к подходу в Эсперанто, где по струкутре слова можно понять часть речи и нет двойных смыслов
 * 
 * Для глобальных объектов - имена уникальны, но есть возможность добавлять несколько варинатов реализации одного и того же термина 
 * (новый термин заменяет старый, но в новом термине остается возможност вызывать предыдущий вариант реализации).
 * 
 * Для локальных объектов - имена <перекрываются>, т.е. объекты не "заменяются", а "перекрываются".
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
 *   var1, var2, var3:Int32 := _;  # Переменная объекта
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
 *   value_at_object_no_error := {* # Код инициализации во время создания экземпляра класса без ошибок
 *      eval at create object
 *   *};  
 * 
 *   func_at_class() := eval at create type;  # Тело функции формируется во время создания типа класса (может возвращаться другой функцией)
 *   func() := {  # Код функции для вызова с генерацией исключений
 *      eval at create object
 *   };  
 *   func_no_error() := {* # Код функции для вызова без генерации исключений
 *      eval at create object
 *   *}; 
 *  
 * };
 * 
 */


%token                  INTEGER		"Integer"
%token                  NUMBER		"Float"
%token                  COMPLEX		"Complex"
%token                  RATIONAL	"Rational"
%token           	STRCHAR		"StrChar"
%token           	STRWIDE		"StrWide"
%token           	TEMPLATE	"Template"
%token           	EVAL            "Eval"

%token			TERM            "Term"
%token			SYMBOL          "Symbol"

%token			UNKNOWN		"Token ONLY UNKNOWN"
%token			OPERATOR_DIV
%token			OPERATOR_AND
%token			OPERATOR_PTR
%token			OPERATOR_ANGLE_EQ

%token			NEWLANG		"@@"
%token			PARENT		"$$"
%token			ARGS		"$*"
%token			MACRO		"Macro"
%token			MACRO_BODY      "Macro body"

%token			CREATE		"::="
%token			CREATE_OR_ASSIGN ":="
%token			APPEND		"[]="

%token			INT_PLUS
%token			INT_MINUS
%token			INT_REPEAT
%token			TRY_PLUS_BEGIN
%token			TRY_PLUS_END
%token			TRY_MINUS_BEGIN
%token			TRY_MINUS_END
%token			TRY_ALL_BEGIN
%token			TRY_ALL_END


%token			FOLLOW
%token			MATCHING
%token			REPEAT

%token			ARGUMENT		


%token			RANGE           ".."
%token			ELLIPSIS        "..."
%token			NAMESPACE       "::"
%token			ASSIGN_MATH     "Arithmetic assign value"

%token			END         0	"end of file"
%token			COMMENT

%token                  SOURCE
%token			ITERATOR
%token			ITERATOR_QQ
%token			ELSE

%token			PUREFUNC
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

fragment:  TERM
            {
                $$ = $1;
                $$->TestConst();
            }

namespace:  fragment
            {
                $$ = $1;
            }
        | namespace  NAMESPACE  fragment
            {
                $$ = $3;
                $$->m_namespace = $1->m_namespace;
                $$->m_namespace += $1->m_text;
                $$->m_namespace += "::";
            }
            
name:  namespace
            {
                $$ = $1;
            }
        | NAMESPACE  namespace
            {
                $$ = $2;
                $$->m_namespace.insert(0, "::");
            }

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
            $$->AppendCommaTerm($3);
        }

type_class:  ':'  name
            {
                $$ = $2;
                $$->m_text.insert(0, ":");
            }

ptr: '&' 
        {
            $$ = Term::Create(TermID::SYMBOL, "&", 1, & @$);
        }
    | OPERATOR_AND
        {
            $$ = $1;
        }
    | OPERATOR_PTR
        {
            $$ = $1;
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
        | ':'  ptr  TERM
            {
                // Для функций, возвращаюющих ссылки
                $$ = $3;
                $$->m_text.insert(0, ":");
                $$->MakeRef($ptr);
            }
        | ':'   ptr   TERM   '['  type_dims   ']'
            {
                // Для функций, возвращаюющих ссылки
                $$ = $3;
                $$->m_text.insert(0, ":");
                $$->SetDims($type_dims);
                $$->MakeRef($ptr);
            }


type:  type_name
            {
                $$ = $1;
                $$->SetTermID(TermID::TYPE);
                $$->TestConst();
            }
        | type_name   call
            {
                $$ = $1;
                $$->SetArgs($2);
                $$->SetTermID(TermID::TYPE_CALL);
                $$->TestConst();
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
         | RATIONAL
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

        
        
range_val:  rval_range
        {
            $$ = $1;  
        }
    | '('  arithmetic  ')'
        {
            $$ = $2;
        }
            
        
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
   
        
/* Допустимые имена переменных и функций. Деструктор с ~ выявляеся на этапе анализа, а не парсинга, т.к. это обычный термин
name:  TERM
            {
                  $$ = $1;
            }
        |  namespace  TERM
            {
                $$ = $2;  
                $$->m_namespace.swap($1->m_namespace);
            }
*/
        
arg_name: name 
        {
            $$ = $1;
            $$->TestConst();
        }
    | strtype 
        {
            $$ = $1;
        }
        
/* Допустимые <имена> объеков */
assign_name:  name
                {
                    $$ = $1;
                    $$->TestConst();
                }
            |  symbols
                {
                    $$ = $1;  
                    $$->TestConst();
                }
            |  namespace  symbols
                {
                    $$ = $2;  
                    $$->m_namespace.swap($1->m_namespace);
                    $$->TestConst();
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
        |  '&'  assign_name
            {
                $$ = $2;
                $$->MakeRef($1);
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
        |  lval  '.'  TERM  type
            {
                $$ = $1; 
                $3->SetTermID(TermID::FIELD);
                $3->SetType($type);
                $$->Last()->Append($3);
            }
        |  lval  '.'  TERM  call  type
            {
                $$ = $1; 
                $3->SetTermID(TermID::FIELD);
                $3->SetArgs($call);
                $3->SetType($type);
                $$->Last()->Append($3);
            }
        |  type
            {   
                $$ = $1; 
            }
        |  type  type
            {   
                $$ = $1; 
                $$->SetType($2);
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

        
rval_range: rval_name
            {
                $$ = $1;
            }
        | digits
            {
                $$ = $1;
            }
        |  string
            {
                $$ = $1;
            }
            
eval:  EVAL 
        {
            $$ = $1;
        }
    |  EVAL  call
        {
            $$ = $1;
            $$->SetArgs($call);
        }
    
rval_var:  rval_range
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
        |  eval 
            {   
                $$ = $1;
            }
        
        
        
rval:   rval_var
            {
                $$ = $1;
            }
        |  assign_arg
            {
                $$ = $1;
            }


iter:  '?'
            {
                $$=$1;
                $$->SetTermID(TermID::ITERATOR);
            }
        | '!'
            {
                $$=$1;
                $$->SetTermID(TermID::ITERATOR);
            }
        | ITERATOR  /* !! ?? */
            {
                $$=$1;
            }

iter_call:  iter  '('  args   ')'
            {
                $$ = $1;
                $$->SetArgs($args);
            }

        
iter_all:  ITERATOR_QQ  /* !?  ?! */
            {
                $$=$1;
                $$->SetTermID(TermID::ITERATOR);
            }
        | iter
            {
                $$=$1;
            }
        | iter_call
            {
                $$=$1;
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
arg: arg_name  '='
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
    | arg_name  '='  logical
        { // Именованный аргумент
            $$ = $3;
            $$->SetName($1->getText());
        }
    | name  type  '='  logical
        { // Именованный аргумент
            $$ = $4;
            $$->SetType($type);
            $$->SetName($1->getText());
        }
    | logical
        {
            // сюда попадают и именованные аргументы как операция присвоения значения
            $$ = $1;
        }
    |  ELLIPSIS
        {
            // Раскрыть элементы словаря в последовательность не именованных параметров
            $$ = $1; 
        }
    |  ELLIPSIS  logical
        {
            // Раскрыть элементы словаря в последовательность не именованных параметров
            $$ = $1; 
            $$->Append($2, Term::RIGHT);
        }
    |  ELLIPSIS  ELLIPSIS  logical
       {            
            // Раскрыть элементы словаря в последовательность ИМЕНОВАННЫХ параметров
            $$ = $2;
            $$->Append($1, Term::LEFT); 
            $$->Append($3, Term::RIGHT); 
        }
    |  ELLIPSIS  logical  ELLIPSIS
       {            
            // Заполнить данные значением
            $$ = $1;
            $$->SetTermID(TermID::FILLING);
            $$->Append($2, Term::RIGHT); 
        }

args: arg
            {
                $$ = $1;
            }
        | args  ','  arg
            {
                $$ = $1;
                $$->AppendCommaTerm($3);
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
        | dictionary   type_class
            {
                $$ = $1;
                $$->m_text = $type_class->m_text;
                $$->m_class_name = $type_class->m_text;
            }
            
           
class_name: type_name
            {
                $$ = $1;
            }
        | type_name   call
            {
                $$ = $1;
                $$->SetArgs($2);
            }

class_base: class_name
            {
                $$ = $1;
            }
        | class_base  ','  class_name
            {
                $$ = $1;
                $$->Last()->Append($3, Term::RIGHT); 
            }


class_props: assign_arg 
            {
                $$ = $1;
            }
        | class_props   ';'   assign_arg
            {
                $$ = $1;
                $$->AppendSequenceTerm($3);
            }
            
            
class_def: class_base '{'  '}'
            {
                $$ = $2;
                $$->m_base = $1;
                $$->SetTermID(TermID::CLASS);
            }
        | class_base '{' class_props  ';'  '}'
            {
                $$ = $class_props;
                $$->m_base = $1;
                $$->ConvertSequenceToBlock(TermID::CLASS);
            }
        
        
collection: array 
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
        | PUREFUNC /* ::- :- */
            {
                $$ = $1;
            }


assign_expr:  body_all
                {
                    $$ = $1;  
                }
/*            | condition
                {
                    $$ = $1;  
                } */
            | ELLIPSIS  rval
                {
                    $$ = $1;  
                    $$->Append($rval, Term::RIGHT); 
                }
            | class_def
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

assign_items: assign_item
                {
                    $$ = $1;
                }
            |  assign_items  ','  assign_item
                {
                    $$ = $1;
                    $$->AppendCommaTerm($3);
                }
/*
lval = rval;
lval, lval, lval = rval;
func() = rval;
*/
assign_arg:  lval  assign_op  assign_expr
            {
                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }
        | lval  '='  assign_expr
            {
                $$ = $2;  
                $$->SetTermID(TermID::ASSIGN);
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }

assign_seq:  assign_items  assign_op  assign_expr
            {
                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }
        | assign_items  '='  assign_expr
            {
                $$ = $2;  
                $$->SetTermID(TermID::ASSIGN);
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }
        
        
ns: NAMESPACE
            {
                $$ = $1;
            }
        | name
            {
                $$ = $1;
            }
            
        
block:  '{'  '}'
            {
                $$ = $1; 
                $$->SetTermID(TermID::BLOCK);
            }
        | '{'  sequence  '}'
            {
                $$ = $2; 
                $$->ConvertSequenceToBlock(TermID::BLOCK);
            }
        | '{'  sequence  separator  '}'
            {
                $$ = $2; 
                $$->ConvertSequenceToBlock(TermID::BLOCK);
            }

block_ns:   ns  '{'  '}'
            {
                $$ = $2; 
                $$->m_namespace = $ns->GetFullName();
                $$->SetTermID(TermID::BLOCK);
            }
        |  ns  '{'  sequence  '}'
            {
                $$ = $2; 
                $$->ConvertSequenceToBlock(TermID::BLOCK);
                $$->m_namespace = $ns->GetFullName();
            }
        | ns  '{'  sequence  separator  '}'
            {
                $$ = $2; 
                $$->ConvertSequenceToBlock(TermID::BLOCK);
                $$->m_namespace = $ns->GetFullName();
            }        
        
        
body:  condition
            {
                $$ = $1;
            }
        |  block
            {
                $$ = $1;
            }
        |  block_ns
            {
                $$ = $1;
            }

body_all: body
            {
                $$ = $1;
            }
        |  try_catch
            {
                $$ = $1;
            }
        |  exit
            {
                $$ = $1;
            }

        
body_else: ',' else  FOLLOW  body_all
            {
                $$ = $4; 
            }


try_all: TRY_ALL_BEGIN  sequence  TRY_ALL_END
            {
                $$ = $2; 
                $$->ConvertSequenceToBlock(TermID::BLOCK_TRY);
            }
        | TRY_ALL_BEGIN  sequence  separator  TRY_ALL_END
            {
                $$ = $2; 
                $$->ConvertSequenceToBlock(TermID::BLOCK_TRY);
            }

try_plus: TRY_PLUS_BEGIN  sequence  TRY_PLUS_END
            {
                $$ = $2; 
                $$->ConvertSequenceToBlock(TermID::BLOCK_PLUS);
            }
        | TRY_PLUS_BEGIN  sequence  separator  TRY_PLUS_END
            {
                $$ = $2; 
                $$->ConvertSequenceToBlock(TermID::BLOCK_PLUS);
            }
        
try_minus: TRY_MINUS_BEGIN  sequence  TRY_MINUS_END
            {
                $$ = $2; 
                $$->ConvertSequenceToBlock(TermID::BLOCK_MINUS);
            }
        | TRY_MINUS_BEGIN  sequence  separator  TRY_MINUS_END
            {
                $$ = $2; 
                $$->ConvertSequenceToBlock(TermID::BLOCK_MINUS);
            }
       
try_catch:  try_plus 
            {
                $$ = $1;
            }
        | try_minus
            {
                $$ = $1;
            }
        | try_all
            {
                $$ = $1;
            }
        | try_all  type_class
            {
                $$ = $1;
                $$->m_type_name = $type_class->m_text;
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
        
/*
 * <arithmetic> -> <arithmetic> + <addition> | <arithmetic> - <addition> | <addition>
 * <addition> -> <addition> * <factor> | <addition> / <factor> | <factor>
 * <factor> -> vars | ( <expr> )
 */

operator: OPERATOR
            {
                $$ = $1;
            }
        | '~'
            {
                $$ = $1;
                $$->SetTermID(TermID::OPERATOR);
            }
        |  OPERATOR_AND
            {
                $$ = $1;
                $$->SetTermID(TermID::OPERATOR);
            }
        |  OPERATOR_ANGLE_EQ
            {
                $$ = $1;
                $$->SetTermID(TermID::OPERATOR);
            }


arithmetic:  arithmetic '+' addition
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
            |  addition   digits
                {
                    if($digits->m_text[0] != '-') {
                        NL_PARSER($digits, "Missing operator!");
                    }
                    //@todo location
                    $$ = Term::Create(TermID::OPERATOR, $2->m_text.c_str(), 1, & @$);
                    $$->Append($1, Term::LEFT); 
                    $2->m_text = $2->m_text.substr(1);
                    $$->Append($2, Term::RIGHT); 
                }
            | addition
                { 
                    $$ = $1; 
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
        
addition:  addition  op_factor  factor
                { 
                    if($1->getTermID() == TermID::INTEGER && $op_factor->m_text.compare("/")==0 && $3->m_text.compare("1")==0) {
                        NL_PARSER($op_factor, "Do not use division by one (e.g. 123/1), "
                                "as this operation does not make sense, but it is easy to "
                                "confuse it with the notation of a rational literal (123\\1).");
                    }
    
                    $$ = $op_factor;
                    $$->SetTermID(TermID::OPERATOR);
                    $$->Append($1, Term::LEFT); 
                    $$->Append($3, Term::RIGHT); 
                }
        |  factor
                { 
                    $$ = $1; 
                }    

factor:   rval_var
            {
                $$ = $1; 
            }
        | '-'  factor
            { 
                $$ = Term::Create(TermID::OPERATOR, "-", 1,  & @$);
                $$->Append($2, Term::RIGHT); 
            }
        | '('  arithmetic  ')'
            {
                $$ = $2; 
            }

       
   
source: SOURCE
            {
                $$=$1;
            }
/*        |  source  SOURCE
            {
                $$=$1;
                $$->Last()->Append($2); 
            } */
        
condition: source
            {
                $$=$1;
            }
        | logical
            {
                $$ = $1;
            }

        
logical:  arithmetic
            {
                    $$ = $1;
            }
        |  logical  operator  arithmetic
            {
                $$ = $2;
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }
        |  arithmetic  iter_all
            {
                $$ = $2;
                $$->Last()->Append($1, Term::LEFT); 
            }
        |  logical  operator  arithmetic   iter_all
            {
                $$ = $2;
                $$->Append($1, Term::LEFT); 
                $iter_all->Last()->Append($arithmetic, Term::LEFT); 
                $$->Append($iter_all, Term::RIGHT); 
            }
        
        
else:   ELSE
            {
                 $$ = Term::Create(TermID::NONE, "_", 1, & @$);
            }
        |  '['  '_'  ']' 
            {
                 $$ = Term::Create(TermID::NONE, "_", 1, & @$);
            }
            

match_cond: '['   condition   ']' 
            {
                $$ = $condition;
            }
        |  else
            {
                $$ = $else;
            }

if_then:  match_cond  FOLLOW  body_all
            {
                $$=$2;
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }

follow: if_then
            {
                $$ = $1; 
                $$->AppendFollow($1);
            }
        | follow  ','  if_then
            {
                $$ = $1; 
                $$->AppendFollow($3);
            }
        
   
repeat: body_all  REPEAT  match_cond
            {
                $$=$2;
                $$->SetTermID(TermID::DOWHILE);
                $$->Append($body_all, Term::LEFT); 
                $$->Append($match_cond, Term::RIGHT); 
            }
        | match_cond  REPEAT  body_all
            {
                $$=$2;
                $$->SetTermID(TermID::WHILE);
                $$->Append($match_cond, Term::LEFT); 
                $$->Append($body_all, Term::RIGHT); 
            }
        | match_cond  REPEAT  body_all  body_else
            {
                $$=$2;
                $$->SetTermID(TermID::WHILE);
                $$->Append($match_cond, Term::LEFT); 
                $$->Append($body_all, Term::RIGHT); 
                $$->AppendFollow($body_else); 
            }

matches:  rval_name
            {
                $$=$1;
            }
        |  matches  ','  rval_var
            {
                $$ = $1;
                $$->AppendFollow($3);
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
                $$->AppendSequenceTerm($match_item);
            }
        
match_body:  '{'  match_items  '}'
            {
                $$ = $2;
                $$->ConvertSequenceToBlock(TermID::BLOCK);
            }



match:  match_cond   MATCHING  match_body
            {
                $$=$2;
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT);
            }
        |  body_all  MATCHING  match_body
            {
                $$=$2;
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT);
            }

interrupt: INT_PLUS 
            {
                $$ = $1;
            }
        | INT_MINUS
            {
                $$ = $1;
            }
        
        
exit:  interrupt
        {
            $$ = $1;
        }
    |  interrupt   rval   interrupt
        {
            $$ = $1;
            $$->Append($2, Term::RIGHT); 
        }
    |  INT_REPEAT   rval   INT_REPEAT
        {
            $$ = $1;
            $$->Append($2, Term::RIGHT); 
        }

/*  expression - одна операция или результат <ОДНОГО выражения без завершающей точки с запятой !!!!!> */
seq_item: assign_seq
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
        |  MACRO
            {
                $$ = $1;  // Их быть не должно, т.к. макросы должны раскрываться до парсинга синтаксиса
            }
        |  MACRO_BODY
            {
                $$ = $1;  // Их быть не должно, т.к. макросы должны раскрываться до парсинга синтаксиса
            }
        | body_all
            {
                $$ = $1;
            }
        | try_catch  body_else
            {
                $$ = $1; 
                $$->AppendFollow($body_else); 
            }
        
sequence:  seq_item
            {
                $$ = $1;  
            }
        | sequence  separator  seq_item
            {
                $$ = $1;  
                // Несколько команд подряд
                $$->AppendSequenceTerm($seq_item);
            }


separator: ';' | separator  ';'        
        

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
/*        | comment     Комменатарии не добавляются в AST, т.к. в парсере они не нужны, а
                        их потенциальное использование - документирование кода, решается 
                        в Python стиле (первый текстовый литерал в коде)
*/

start	:   ast

%% /*** Additional Code ***/
