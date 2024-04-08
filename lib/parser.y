
%{ /*** C/C++ Declarations ***/

//#include "pch.h"

#include "term.h"
#include "context.h"    

#include "parser.h"
#include "lexer.h"

/* this "connects" the bison parser in the driver to the flex scanner class
 * object. it defines the yylex() function call to pull the next token from the
 * current lexer object of the driver context. */
#undef yylex
#define yylex driver.GetNextToken
    //driver.lexer->lex

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
    @$.begin.filename = @$.end.filename = &driver.m_filename;
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
 * Как сделать замену одной реализации фунции на другую для языка с динамиеческой типизацией без перегрузки функций?
 * 1. Сохранять указатель на предыдущую функцию в новой реализации, тогда  нужны локальные статические переменные и/или деструкторы.
 * 2. Управлять именами функций средствами языка (не нужны локальные статические переменные и деструкторы, 
 * но нужна лексическая контструкция чтобы обращаться к предыдущей реализации (что эквавалентно досутпу к родительсокму классу),
 * а весь список функций можно вытащить итератором)
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
 *   :class.var_class (static) := "";
 *   :class.var_class (static) := 1; # публичное поле объекта
 *   :class._var_class (static) := 1; # защиенное поле объекта
 *   :class.__var_class (static) := 1; # приватное поле объекта
 *   :class.__var_class__ (static) := 1; # Системное 
 * 
 *   var := 1; # публичное поле объекта
 *   _var := 1; # защиенное поле объекта
 *   __var := 1; # приватное поле объекта
 *   __var__ := 1; # системное поле
 * 
 */


%token                  INTEGER		"Integer"
%token                  NUMBER		"Float"
%token                  COMPLEX		"Complex"
%token                  RATIONAL	"Rational"
%token           	STRCHAR		"StrChar"
%token           	STRWIDE		"StrWide"
%token           	STRFMTCHAR	"StrFmtChar"
%token           	STRFMTWIDE	"StrFmtWide"
%token           	TEMPLATE	"Template"
%token           	EVAL            "Eval"

%token			NAME
%token			LOCAL
%token			MODULE
%token			NATIVE
%token			SYMBOL

%token			UNKNOWN		"Token ONLY UNKNOWN"
%token			OPERATOR_DIV
%token			OPERATOR_AND
%token			OPERATOR_PTR
%token			OPERATOR_ANGLE_EQ
%token			OPERATOR_DUCK

%token			SPACE
%token			COMMENT
%token			INDENT
%token			CRLF
%token			ESCAPE

 %token			NEWLANG		"\\\\"
%token			PARENT		"$$"
%token			ARGS		"$*"

%token			MACRO           "Macro"
%token			MACRO_SEQ
%token			MACRO_STR       "Macro str"
%token			MACRO_DEL       "Macro del"
%token			MACRO_CONCAT    "Macro concatenation"
%token			MACRO_TOSTR     "Macro to string"
%token			MACRO_ARGUMENT  "Macro argument"
%token			MACRO_ARGCOUNT  "Macro args count"
%token			MACRO_ARGPOS    "Macro argument pos"
%token			MACRO_ARGNAME   "Macro argument name"


%token			MACRO_NAMESPACE

%token			CREATE_ONCE              "::="
%token			CREATE_OVERLAP             ":="
%token			APPEND                  "[]="
%token			SWAP

/*
%token			RIGHT_ASSIGN            "=>"
%token			RIGHT_CREATE_OR_ASSIGN  ":=>"
%token			RIGHT_CREATE            "::=>"
*/

%token			INT_PLUS
%token			INT_MINUS
%token			INT_REPEAT
%token			TRY_PLUS_BEGIN
%token			TRY_PLUS_END
%token			TRY_MINUS_BEGIN
%token			TRY_MINUS_END
%token			TRY_ALL_BEGIN
%token			TRY_ALL_END
%token			SYM_BEGIN
%token			SYM_END


%token			FOLLOW
%token			MATCHING
%token			REPEAT
%token			WITH
%token			TAKE

%token			ARGUMENT


%token			RANGE           ".."
%token			ELLIPSIS        "..."
%token			NAMESPACE       "::"
%token			ASSIGN_MATH     "Arithmetic assign value"

%token			END         0	"end of file"

%token                  EMBED
%token			ITERATOR
%token			ITERATOR_QQ
%token			COROUTINE
%token			FUNCTION

%token			PURE_ONCE
%token			PURE_OVERLAP
%token			OP_LOGICAL
%token			OP_MATH
%token			OP_COMPARE
%token			OP_BITWISE
%token			DOC_BEFORE
%token			DOC_AFTER


%% /*** Grammar Rules ***/


/* Разделитель */
separator: ';' | separator  ';'

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



ns_part:  NAME
            {
                $$ = $1;
            }
        | MACRO_NAMESPACE   NAME
            {
                $$ = $2;
                $$->m_text.insert(0, $1->m_text);
            }
        | ns_part  NAMESPACE  NAME   
            {
                $$ = $3;
                $$->m_text.insert(0, $2->m_text);
                $$->m_text.insert(0, $1->m_text);
                $$->SetTermID(TermID::STATIC);
                // У переменных m_namespace заполняется в AstExpandNamespace
            }
        
ns_start:  NAMESPACE
            {
                $$ = $1;
            }
/*        | MACRO_NAMESPACE
            {
                $$ = $1;
                $$->SetTermID(TermID::NAMESPACE);
            }
*/

name:   ns_part
            {
                $$ = $1;
                $$->TestConst();
            }
        | ns_start
            {
                $$ = $1;
                $$->SetTermID(TermID::STATIC);
                // У переменных m_namespace заполняется в AstExpandNamespace
            }
        | ns_start  ns_part
            {
                $$ = $2;
                $$->m_text.insert(0, $1->m_text);
                $$->SetTermID(TermID::STATIC);
                // У переменных m_namespace заполняется в AstExpandNamespace
            }
        | ns_part  NAMESPACE  type_class
            {
                $$ = $3;
                $$->m_text.insert(0, $2->m_text);
                $$->m_text.insert(0, $1->m_text);
                $$->SetTermID(TermID::STATIC);
                // У переменных m_namespace заполняется в AstExpandNamespace
            }
        | ns_start  ns_part  NAMESPACE  type_class
            {
                $$ = $4;
                $$->m_text.insert(0, $3->m_text);
                $$->m_text.insert(0, $2->m_text);
                $$->m_text.insert(0, $1->m_text);
                $$->SetTermID(TermID::STATIC);
                // У переменных m_namespace заполняется в AstExpandNamespace
            } 
        |  LOCAL
            {
                $$ = $1;
            }
        | '$'
            {
                $$ = $1;
                $$->SetTermID(TermID::LOCAL);
                $$->TestConst();
            }
        |  MODULE
            {
                $1->TestConst();
                $$ = driver.CheckLoadModule($1);
            }
        |  '\\'
            {
                $1->TestConst();
                $1->SetTermID(TermID::MODULE);
                $$ = driver.CheckLoadModule($1);
            }
        |  MODULE  ns_start   ns_part
            {
                $3->m_namespace = $2;
                $3->SetTermID(TermID::STATIC);
                $3->TestConst();
                $1->Last()->Append($3);
                $$ = driver.CheckLoadModule($1);

            }
        |  native
            {
                $$ = $1;
                $$->TestConst();
            }
        |  PARENT  /* $$ - rval */
            {
                $$ = $1;
                $$->SetTermID(TermID::NAME);
            }
        |  NEWLANG  /* \\ - rval */
            {
                $$ = $1;
//                $$->SetTermID(TermID::NAME);
            }
        | MACRO
            {
                $$ = $1;
            }
        | '@'
            {
                $$ = $1;
                $$->SetTermID(TermID::MACRO);
            }
        | MACRO_ARGUMENT
            {
                $$ = $1;
            }
        | MACRO_ARGPOS
            {
                $$ = $1;
            }
        | MACRO_ARGNAME
            {
                $$ = $1;
            }
        
        

/* Фиксированная размерность тензоров для использования в типах данных */
type_dim: rval_var
        {
            $$ = $1;
        }
    | NAME  '='  rval_var
        { // torch поддерживает именованные диапазоны, но пока незнаю, нужны ли они?
            $$ = $3;
            $$->SetName($1->getText());
        }
    |  ELLIPSIS
        {
            // Произвольное количество элементов
            $$ = $1; 
        }

type_dims: type_dim
        {
            $$ = $1;
        }
    | type_dims  ','  type_dim
        {
            $$ = $1;
            $$->AppendList($3);
        }

type_class:  ':'  name
            {
                $$ = $2;
                $$->m_text.insert(0, ":");
                $$->SetTermID(TermID::TYPE);
            }
        | ':'  '~'  name
            {
                $$ = $3;
                $$->m_text.insert(0, ":");
                $$->SetTermID(TermID::TYPE);
            }
        | ':'  OPERATOR_DUCK  name
            {
                $$ = $3;
                $$->m_text.insert(0, ":");
                $$->SetTermID(TermID::TYPE);
            }

ptr: '&' 
        {
            $$ = $1;
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
                $$->m_dims = $2;
                $$->m_dims->SetArgs($type_dims);
            }
        | ':'  ptr  NAME
            {
                // Для функций, возвращаюющих ссылки
                $$ = $3;
                $$->m_text.insert(0, ":");
                $$->MakeRef($ptr);
            }
        | ':'  ptr  NAME   '['  type_dims   ']'
            {
                // Для функций, возвращаюющих ссылки
                $$ = $3;
                $$->m_text.insert(0, ":");
                $$->m_dims = $type_dims;
                $$->m_dims->SetArgs($type_dims);
                $$->MakeRef($ptr);
            }


type_call: type_name   call
            {
                $$ = $1;
                $$->SetArgs($2);
                $$->SetTermID(TermID::TYPE);
                $$->TestConst();
            }
        
type_item:  type_name
            {
                $$ = $1;
                $$->SetTermID(TermID::TYPE);
                $$->TestConst();
            }
        | type_call
            {
                $$ = $1;
            }
/*        | ':'  eval
            {
                // Если тип еще не определён и/или его ненужно проверять во время компиляции, то имя типа можно взять в кавычки.
                $$ = $2;
                $$->SetTermID(TermID::TYPENAME);
                $$->m_text.insert(0, ":");
            } */

type_items:  type_item
            {
                $$ = $1;
            }
        | type_items   ','   type_item
            {
                $$ = $1;
                $$->AppendList($3);
            }

        
type_list:  ':'  '<'  type_items  '>'
            {
                $$ = $type_items;
            }
        |  ':'  '<'  ELLIPSIS  '>'
            {
                $$ = $3;
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
        | MACRO_ARGCOUNT
            {
                $$ = $1;
            }
        
digits:  digits_literal
            {
                $$ = $1;
            }
        | digits_literal  type_item
            {
                $$ = $1;
                $$->SetType($type_item);
            }

        
        
range_val:  rval_range
        {
            $$ = $1;  
        }
/*    | '('  arithmetic  ')'
        {
            $$ = $2;
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
        
        
        
name_to_concat:  MACRO_ARGUMENT  
        {
            $$ = $1;
        }
    |  MACRO_ARGNAME
        {
            $$ = $1;
        }
    |  NAME
        {
            $$ = $1;
        }

strwide: STRWIDE
            {
                $$ = $1;
                $$->SetType(nullptr);
            }
        | strwide  STRWIDE
            {
                $$ = $1;
                $$->m_text.append($2->m_text);
            }

strchar: STRCHAR
            {
                $$ = $1;
                $$->SetType(nullptr);
            }
        | strchar  STRCHAR
            {
                $$ = $1;
                $$->m_text.append($2->m_text);
            }

strtype: strwide
            {
                $$ = $1;
            }
        | strchar
            {
                $$ = $1;
            }
        |  MACRO_TOSTR   name_to_concat
            {            
                $$ = $1;
                $$->Append($2, Term::RIGHT); 
            }
        |  name_to_concat  MACRO_CONCAT  name_to_concat
           {            
                $$ = $2;
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
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
   

doc_before: DOC_BEFORE 
            {
                $$ = $1;
            }    
        | doc_before  DOC_BEFORE 
            {
                $$ = $1;
                $$->AppendList($2);
            }    
    
doc_after: DOC_AFTER
            {
                $$ = $1;
            }    
        | doc_after  DOC_AFTER
            {
                $$ = $1;
                $$->AppendList($2);
            }    

        
arg_name: name 
        {
            $$ = $1;
            $$->TestConst();
        }
    | strtype 
        {
            $$ = $1;
        }
    | '.'  NAME
        {
            $$ = $2; 
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
            |  ns_part  symbols
                {
                    $$ = $2;
                    $$->m_text.insert(0, $1->m_text);
                    $$->SetTermID(TermID::STATIC);
                    $$->TestConst();
                    // У переменных m_namespace заполняется в AstExpandNamespace
                }
            |  ns_start   ns_part   symbols
                {
                    $$ = $3;
                    $$->m_text.insert(0, $2->m_text);
                    $$->m_text.insert(0, $1->m_text);
                    $$->SetTermID(TermID::STATIC);
                    $$->TestConst();
                    // У переменных m_namespace заполняется в AstExpandNamespace
                }
           | ARGUMENT  /* $123 */
                {
                    $$ = $1;
                }
            
field:  '.'  NAME
            {
                $$ = $1; 
                $NAME->SetTermID(TermID::FIELD);
                $$->Last()->Append($NAME);
            }
        |  '.'  NAME  call
            {
                $$ = $1; 
                $NAME->SetTermID(TermID::FIELD);
                $NAME->SetArgs($call);
                $$->Last()->Append($NAME);
            }
        |  '.'  NAME  type_item
            {
                $$ = $1; 
                $NAME->SetTermID(TermID::FIELD);
                $NAME->SetType($type_item);
                $$->Last()->Append($NAME);
            }
        |  '.'  NAME  call  type_item
            {
                $$ = $1; 
                $NAME->SetTermID(TermID::FIELD);
                $NAME->SetArgs($call);
                $NAME->SetType($type_item);
                $$->Last()->Append($NAME);
            }
        |  '.'  NAME  call  type_list
            {
                $$ = $1; 
                $NAME->SetTermID(TermID::FIELD);
                $NAME->SetArgs($call);
                $$->Last()->Append($NAME);
                $$->SetType($type_list);
            }

        
native:  '%'  ns_part
            {
                $$ = $2;
                $$->m_text.insert(0, $1->m_text);
                $$->SetTermID(TermID::NATIVE);
            }

        | '%'  ns_start  ns_part 
            {
                $$ = $3;
                $$->m_text.insert(0, $2->m_text);
                $$->m_text.insert(0, $1->m_text);
                $$->SetTermID(TermID::NATIVE);
            }
        | '%'  '.'  NAME
            {
                $$ = $NAME; 
                $$->m_text.insert(0, $2->m_text);
                $$->m_text.insert(0, $1->m_text);
                $$->SetTermID(TermID::NATIVE);
            }
        
lval_obj: assign_name
            {
                $$ = $1;
            }
        |  assign_name  '['  args  ']'
            {   
                $$ = $1; 
                $2->SetTermID(TermID::INDEX);
                $2->SetArgs($args);
                $$->Last()->Append($2);
            }
        | field 
            {
                $$ = $1; 
            }
        |  lval_obj  field
            {
                $$ = $1; 
                $$->Last()->Append($field);
            }        
        
        
take:   TAKE  /*  *^  */
        {
            $$ = $1;
            $$->SetTermID(TermID::TAKE);
        }
    | '*' 
        {
            $$ = $1;
            $$->SetTermID(TermID::TAKE);
        }  
    
/* Допустимые lvalue объекты */
lval:  lval_obj
            {
                $$ = $1; 
            }
        |  take  rval_name
            {
                $$ = $1;
                $$->SetArgs($2);
            } 
        |  take  call
            {
                $$ = $1;
                $$->SetArgs($call);
            } 
        |  type_item
            {   
                $$ = $type_item; 
            }
        |  type_item  type_item
            {   
                $$ = $1; 
                $$->SetType($2);
            }
        |  name  type_item
            {   
                $$ = $1; 
                $$->SetType($type_item);
            }
        |  name  call
            {   
                $$ = $name; 
                $$->SetArgs($call);
            }
        |  name  call  type_item
            {   
                $$ = $name; 
                $$->SetArgs($call);
                $$->SetType($type_item);
            }
        |  name  call  type_list
            {   
                $$ = $name; 
                $$->SetArgs($call);
                $$->SetType($type_list);
            }

rval_name: lval
            {
                $$ = $1; 
            }
/*        | with_op  '('  lval  ')'
            {
                $$ = $1; 
                $$->Last()->Append($lval);
            } */
        | ARGS /* $* и @* - rval */
            {
                $$ = $1;
            }

        
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
        |  assign_lval
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
        | iter  '('  ')'
            {
                $$ = $1;
                $$->SetArgs(nullptr);
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
/*    | arg_name  type_item  '='
        { // Именованный аргумент
            $$ = $3;
            $$->SetType($type_item);
            $$->m_name.swap($1->m_text);
            $$->SetTermID(TermID::EMPTY);
        } */
    | arg_name  '='  logical
        { // Именованный аргумент
            $$ = $3;
            $$->SetName($1->getText());
        }
    | ptr   arg_name  '='  logical
        { // Именованный аргумент
            $$ = $logical;
            $$->SetName($arg_name->getText());
            $$->MakeRef($ptr);
        }
    | name  type_item  '='  logical
        { // Именованный аргумент
            $$ = $4;
            $$->SetType($type_item);
            $$->SetName($1->getText());
        }
    | name  type_list  '='  logical
        { // Именованный аргумент
            $$ = $4;
            $$->SetName($1->getText());
            $$->SetType($type_list);
        }
    | ptr  name  type_item  '='  logical
        { // Именованный аргумент
            $$ = $logical;
            $$->SetType($type_item);
            $$->SetName($name->getText());
            $$->MakeRef($ptr);
        }
    | ptr  name  type_list  '='  logical
        { // Именованный аргумент
            $$ = $logical;
            $$->SetName($name->getText());
            $$->SetType($type_list);
            $$->MakeRef($ptr);
        }
    | arg_name  '='  ptr  logical
        { // Именованный аргумент
            $$ = $4;
            $$->SetName($1->getText());
            $$->MakeRef($ptr);
        }
    | name  type_item  '='  ptr  logical
        { // Именованный аргумент
            $$ = $5;
            $$->SetType($type_item);
            $$->SetName($1->getText());
            $$->MakeRef($ptr);
        }
    | name  type_list  '='  ptr  logical
        { // Именованный аргумент
            $$ = $5;
            $$->SetName($1->getText());
            $$->SetType($type_list);
            $$->MakeRef($ptr);
        }
/*    | arg_name  '='  take  logical
        { // Именованный аргумент
            $$ = $take;
            $logical->SetName($1->getText());
            $$->SetArgs($logical);
        }
    | name  type_item  '='  take  logical
        { // Именованный аргумент
            $$ = $take;
            $logical->SetType($type_item);
            $logical->SetName($1->getText());
            $$->SetArgs($logical);
        }
    | name  type_list  '='  take  logical
        { // Именованный аргумент
            $$ = $take;
            Term::ListToVector($type_list, $logical->m_type_allowed);
            $logical->SetName($1->getText());
            $$->SetArgs($logical);
        } */ 
    | logical
        {
            // сюда попадают и именованные аргументы как операция присвоения значения
            $$ = $1;
        }
    | ptr  logical
        {
            $$ = $2;  
            $$->MakeRef($ptr);
        }   
/*    | take  logical
        {
            $$ = $1;  
            $$->SetArgs($2);
        } */  
    |  ELLIPSIS
        {
            // Раскрыть элементы словаря в последовательность не именованных параметров
            $$ = $1; 
        }
    |  ELLIPSIS type_list
        {
            $$ = $1; 
            $$->SetType($type_list);
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
    |  ESCAPE /* for pragma terms */
       {            
            $$ = $1;
        }
    /*    |  operator
       {            
            $$ = $1;
       }
    |  op_factor
       {            
            $$ = $1;
       } */

args: arg
            {
                $$ = $1;
            }
        | args  ','  arg
            {
                $$ = $1;
                $$->AppendList($3);
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
        | '['  args  ','  ']'  type_item
            {
                $$ = $1;
                $$->m_text.clear();
                $$->SetTermID(TermID::TENSOR);
                $$->SetArgs($args);
                $$->SetType($type_item);
            }
        | '['  ','  ']'  type_item
            {
                // Не инициализированый тензор должен быть с конкретным типом 
                $$ = $1;
                $$->m_text.clear();
                $$->SetTermID(TermID::TENSOR);
                $$->SetType($type_item);
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
                $$->m_class = $type_class->m_text;
            }

collection: array 
            {
                $$ = $1;
            }
        | class
            {
                $$ = $1;
            }
        
class_props: assign_lval
            {
                $$ = $1;
            }
        | class_props   separator   assign_lval
            {
                $$ = $1;
                $$->AppendSequenceTerm($3);
            }

class_item:  type_call
            {
                $$ = $1;
            }
        | name  call
            {
                $$ = $1;
                $$->SetArgs($call);
            }

class_base: class_item
            {
                $$ = $1;
            }
        | class_base   ','   class_item
            {
                $$ = $1;
                $$->AppendList($3);
            }


class_def:  class_base  '{'  '}'
            {
                $$ = $2;
                Term::ListToVector($class_base, $$->m_base);
                $$->SetTermID(TermID::CLASS);
            }
        | class_base '{' class_props  separator  '}'
            {
                $$ = $class_props;
                Term::ListToVector($class_base, $$->m_base);
                $$->ConvertSequenceToBlock(TermID::CLASS);
            }
        | class_base '{' doc_after  '}'
            {
                $$ = $2;
                $$->SetTermID(TermID::CLASS);
                Term::ListToVector($class_base, $$->m_base);
                Term::ListToVector($doc_after, $$->m_docs);
            }
        | class_base '{' doc_after  class_props  separator  '}'
            {
                $$ = $class_props;
                $$->ConvertSequenceToBlock(TermID::CLASS);
                Term::ListToVector($class_base, $$->m_base);
                Term::ListToVector($doc_after, $$->m_docs);
            }
        
        
        
assign_op: CREATE_OVERLAP /* := */
            {
                $$ = $1;
            }
        | CREATE_ONCE /* ::= */
            {
                $$ = $1;
            }
        | APPEND /* []= */
            {
                $$ = $1;
            }
        | PURE_ONCE /* :- */
            {
                $$ = $1;
            }
        | PURE_OVERLAP /* ::- */
            {
                $$ = $1;
            }
        

coro: '(' ')'
        {
            $$ = $1;
        }
    | '%'  '(' args ')'
        {
            $$ = $1;
            $$->SetArgs($args);
        }
        
assign_expr:  body_all
                {
                    $$ = $1;  
                }
            |  ptr  body_all
                {
                    $$ = $2;  
                    $$->MakeRef($ptr);
                }
            | ELLIPSIS  rval
                {
                    $$ = $1;  
                    $$->Append($rval, Term::RIGHT); 
                }
            | class_def
                {
                    $$ = $1;  
                }
            |  MACRO_SEQ
                {
                    $$ = $1;  
                }
            |  MACRO_STR
                {
                    $$ = $1;
                }
            |  native  ELLIPSIS
                {
                    $$ = $1;
                    $$->Last()->Append($2);
                }
            |  coro  block_all
                {
                // Короутина?
                    $$ = $1;  
                    $$->Append($block_all, Term::RIGHT); 
                    $$->SetTermID(TermID::COROUTINE);
                }
            |  coro  block_all  type_name
                {
                // Короутина?
                    $$ = $1;  
                    $$->Append($block_all, Term::RIGHT); 
                    $$->SetTermID(TermID::COROUTINE);
                    $$->SetType($type_name);
                    $block_all->SetType($type_name);
                }

            
assign_item:  lval
                {
                    $$ = $1;
                }
            | ptr   lval
                {
                    $$ = $2;
                    $$->MakeRef($ptr);
                }
            | ptr  call  logical
                {
                    $ptr->SetArgs($call);
                    $$ = $3;  
                    $$->MakeRef($ptr);
                }   
            |  MACRO_SEQ
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
                    $$->AppendList($3);
                }

/*
 * Для применения в определениях классов и в качестве rval
 */            
assign_lval:  lval  assign_op  assign_expr
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

                if($$->isMacro()){
                    $$ = ProcessMacro(driver, $$);
                }
            }
        | assign_items  '='  assign_expr
            {
                $$ = $2;  
                $$->SetTermID(TermID::ASSIGN);
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 

                if($$->isMacro()){
                    $$ = ProcessMacro(driver, $$);
                }
            }
        | MACRO_DEL
            {
                $$ = ProcessMacro(driver, $$);
            }

        
block:  '{'  '}'
            {
                $$ = $1; 
                $$->SetTermID(TermID::BLOCK);
            }
        | '{'  sequence  '}'
            {
                $$ = $sequence; 
                $$->ConvertSequenceToBlock(TermID::BLOCK);
            }
        | '{'  sequence  separator  '}'
            {
                $$ = $sequence; 
                $$->ConvertSequenceToBlock(TermID::BLOCK);
            }
        |  '{'  doc_after  '}'
            {
                $$ = $1; 
                $$->SetTermID(TermID::BLOCK);
                Term::ListToVector($doc_after, $$->m_docs);
            }
        | '{'  doc_after  sequence  '}'
            {
                $$ = $sequence; 
                $$->ConvertSequenceToBlock(TermID::BLOCK);
                Term::ListToVector($doc_after, $$->m_docs);
            }
        | '{'  doc_after  sequence  separator  '}'
            {
                $$ = $sequence; 
                $$->ConvertSequenceToBlock(TermID::BLOCK);
                Term::ListToVector($doc_after, $$->m_docs);
            }

block_any: block
            {
                $$ = $1;
            }
        |  try_any
            {
                $$ = $1;
            }
        |  WITH  try_any
            {
                $$ = $2;
                $$->Append($1, Term::LEFT); 
            }

block_all: block_any
            {
                $$ = $1;
            }
        | ns_part  block_any
            {
                $$ = $2;
                $$->m_namespace = $1;
            }
        |  ns_part  NAMESPACE  block_any
            {
                $$ = $3;
                $$->m_namespace = $2;
                $$->m_namespace->m_text.insert(0, $1->m_text);
            }
        |  ns_start  ns_part  NAMESPACE  block_any
            {
                $$ = $4;
                $$->m_namespace = $3;
                $$->m_namespace->m_text.insert(0, $2->m_text);
                $$->m_namespace->m_text.insert(0, $1->m_text);
            }
        |  ns_start  ns_part  block_any
            {
                $$ = $3;
                $$->m_namespace = $2;
                $$->m_namespace->m_text.insert(0, $1->m_text);
            } 
        |  ns_start  block_any
            {
                $$ = $2;
                $$->m_namespace = $1;
            } 
        

block_type: block_all
            {
                $$ = $1;
            }
        | block_all  type_name
            {
                $$ = $1;
                $$->SetType($type_name);
            }
        | block_all  type_list
            {
                $$ = $1;
                $$->SetType($type_list);
            }
        
body:  condition
            {
                $$ = $1;
            }
        |  block_type
            {
                $$ = $1;
            }
        |  doc_before  block_type
            {
                $$ = $block_type;
                Term::ListToVector($doc_before, $$->m_docs);
            } 
        |  exit
            {
                $$ = $1;
            }
        

body_all: body
            {
                $$ = $1;
            }
        
body_else: ',' else  FOLLOW  body_all
            {
                $$ = $3; 
                $$->Append($else, Term::LEFT); 
                $$->Append($body_all, Term::RIGHT); 
            }


try_all: TRY_ALL_BEGIN  TRY_ALL_END
            {
                $$ = $1; 
                $$->ConvertSequenceToBlock(TermID::BLOCK_TRY);
            }
        | TRY_ALL_BEGIN  sequence  TRY_ALL_END
            {
                $$ = $2; 
                $$->ConvertSequenceToBlock(TermID::BLOCK_TRY);
            }
        | TRY_ALL_BEGIN  sequence  separator  TRY_ALL_END
            {
                $$ = $2; 
                $$->ConvertSequenceToBlock(TermID::BLOCK_TRY);
            }

try_plus: TRY_PLUS_BEGIN  TRY_PLUS_END
            {
                $$ = $1; 
                $$->ConvertSequenceToBlock(TermID::BLOCK_PLUS);
            }
        | TRY_PLUS_BEGIN  sequence  TRY_PLUS_END
            {
                $$ = $2; 
                $$->ConvertSequenceToBlock(TermID::BLOCK_PLUS);
            }
        | TRY_PLUS_BEGIN  sequence  separator  TRY_PLUS_END
            {
                $$ = $2; 
                $$->ConvertSequenceToBlock(TermID::BLOCK_PLUS);
            }
        
try_minus: TRY_MINUS_BEGIN  TRY_MINUS_END
            {
                $$ = $1; 
                $$->ConvertSequenceToBlock(TermID::BLOCK_MINUS);
            }
        | TRY_MINUS_BEGIN  sequence  TRY_MINUS_END
            {
                $$ = $2; 
                $$->ConvertSequenceToBlock(TermID::BLOCK_MINUS);
            }
        | TRY_MINUS_BEGIN  sequence  separator  TRY_MINUS_END
            {
                $$ = $2; 
                $$->ConvertSequenceToBlock(TermID::BLOCK_MINUS);
            }

try_any:  try_plus 
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

operator: '~'
            {
                $$ = $1;
                $$->SetTermID(TermID::OP_COMPARE);
            }
        | '>'
            {
                $$ = $1;
                $$->SetTermID(TermID::OP_COMPARE);
            }
        | '<'
            {
                $$ = $1;
                $$->SetTermID(TermID::OP_COMPARE);
            }
        |  OPERATOR_AND
            {
                $$ = $1;
                $$->SetTermID(TermID::OP_LOGICAL);
            }
        |  OPERATOR_ANGLE_EQ
            {
                $$ = $1;
                $$->SetTermID(TermID::OP_COMPARE);
            }
        |  OPERATOR_DUCK
            {
                $$ = $1;
                $$->SetTermID(TermID::OP_COMPARE);
            }
        |  OP_MATH
            {
                $$ = $1;
            }
        |  OP_LOGICAL
            {
                $$ = $1;
            }
        |  OP_BITWISE
            {
                $$ = $1;
            }
        |  OP_COMPARE
            {
                $$ = $1;
            }
        


arithmetic:  arithmetic '+' addition
                { 
                    $$ = $2;
                    $$->SetTermID(TermID::OP_MATH);
                    $$->Append($1, Term::LEFT);                    
                    $$->Append($3, Term::RIGHT); 
                }
            | arithmetic '-'  addition
                { 
                    $$ = $2;
                    $$->SetTermID(TermID::OP_MATH);
                    $$->Append($1, Term::LEFT);                    
                    $$->Append($3, Term::RIGHT); 
                }
            |  addition   digits
                {
                    if($digits->m_text[0] != '-') {
                        NL_PARSER($digits, "Missing operator between '%s' and '%s'", $addition->m_text.c_str(), $digits->m_text.c_str());
                    }
                    //@todo location
                    $$ = Term::Create(token::OP_MATH, TermID::OP_MATH, $2->m_text.c_str(), 1, & @$);
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
                    $$->SetTermID(TermID::OP_MATH);
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
                $$ = Term::Create(token::OP_MATH, TermID::OP_MATH, "-", 1,  & @$);
                $$->Append($2, Term::RIGHT); 
            }
        | '('  logical  ')'
            {
                $$ = $2; 
            }


symbolyc: SYM_BEGIN  arithmetic  SYM_END   PURE_OVERLAP   SYM_BEGIN  sequence  SYM_END
            {
                $$ = $PURE_OVERLAP;
                $$->SetTermID(TermID::SYM_RULE);
                $$->Append($arithmetic, Term::LEFT); 
                $$->Append($sequence, Term::RIGHT); 
            }
        | SYM_BEGIN  arithmetic  SYM_END   PURE_OVERLAP   SYM_BEGIN  sequence  separator  SYM_END
            {
                $$ = $PURE_OVERLAP;
                $$->SetTermID(TermID::SYM_RULE);
                $$->Append($arithmetic, Term::LEFT); 
                $$->Append($sequence, Term::RIGHT); 
            }
        | SYM_BEGIN  arithmetic  separator SYM_END   PURE_OVERLAP   SYM_BEGIN  sequence  SYM_END
            {
                $$ = $PURE_OVERLAP;
                $$->SetTermID(TermID::SYM_RULE);
                $$->Append($arithmetic, Term::LEFT); 
                $$->Append($sequence, Term::RIGHT); 
            }
        | SYM_BEGIN  arithmetic  separator SYM_END   PURE_OVERLAP   SYM_BEGIN  sequence  separator  SYM_END
            {
                $$ = $PURE_OVERLAP;
                $$->SetTermID(TermID::SYM_RULE);
                $$->Append($arithmetic, Term::LEFT); 
                $$->Append($sequence, Term::RIGHT); 
            }
   

embed: EMBED
            {
                $$ = $1;
            }
        | embed  EMBED
            {
                $$ = $1;
                $$->m_text.append($2->m_text);
            }
        
condition: embed
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
        
        
else:   '['  ELLIPSIS  ']' 
            {
                $$ = $2;
            }
            

match_cond: '['   condition   ']' 
            {
                $$ = $condition;
            }

if_then:  match_cond  FOLLOW  body_all
            {
                $$=$2;
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }

if_list: if_then
            {
                $$ = $1; 
                $$->AppendFollow($1);
            }
        | if_list  ','  if_then
            {
                $$ = $1; 
                $$->AppendFollow($3);
            }
        
follow: if_list
            {
                $$ = $1; 
            }
        | if_list  body_else
            {
                $$ = $1; 
                $$->AppendFollow($2);
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

matches:  rval_range
            {
                $$=$1;
            }
        |  matches  ','  rval_range
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
/*        | else  FOLLOW  body
            {
                $$=$2;
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            } */

match_items:  match_item  ';'
            {
                $$ = $1;
            }
        | match_items  match_item  ';'
            {
                $$ = $1;
                $$->AppendSequenceTerm($match_item);
            }

match_items_else:  match_items
            {
                $$=$1;
            } 
        |  match_items  else  FOLLOW  body
            {
                $$=$1;

                $3->Append($else, Term::LEFT); 
                $3->Append($body, Term::RIGHT); 

                $$->AppendSequenceTerm($3);
            } 
      
match_body: '{'  match_items_else  '}'
            {
                $$ = $2;
                $$->ConvertSequenceToBlock(TermID::BLOCK);
            }
        | '{'  match_items_else  separator '}'
            {
                $$ = $2;
                $$->ConvertSequenceToBlock(TermID::BLOCK);
            }
        | TRY_ALL_BEGIN  match_items_else  TRY_ALL_END
            {
                $$ = $2;
                $$->ConvertSequenceToBlock(TermID::BLOCK_TRY);
            }
        | TRY_ALL_BEGIN  match_items_else  separator TRY_ALL_END
            {
                $$ = $2;
                $$->ConvertSequenceToBlock(TermID::BLOCK_TRY);
            }
        | TRY_PLUS_BEGIN  match_items_else  TRY_PLUS_END
            {
                $$ = $2;
                $$->ConvertSequenceToBlock(TermID::BLOCK_PLUS);
            }
        | TRY_PLUS_BEGIN  match_items_else  separator TRY_PLUS_END
            {
                $$ = $2;
                $$->ConvertSequenceToBlock(TermID::BLOCK_PLUS);
            }
        | TRY_MINUS_BEGIN  match_items_else  TRY_MINUS_END
            {
                $$ = $2;
                $$->ConvertSequenceToBlock(TermID::BLOCK_MINUS);
            }
        | TRY_MINUS_BEGIN  match_items_else  separator TRY_MINUS_END
            {
                $$ = $2;
                $$->ConvertSequenceToBlock(TermID::BLOCK_MINUS);
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
        | INT_REPEAT
            {
                $$ = $1;
            }
        

exit_part:  interrupt
        {
            $$ = $1;
        }
    |  interrupt   rval_var   interrupt
        {
            $$ = $1;
            $$->Append($2, Term::RIGHT); 
        }


exit_prefix: ns_part
        {
            $$ = $1;
        }

    |  MACRO_NAMESPACE
        {
            $$ = $1;
        }
    |  ns_start
        {
            $$ = $1;
        }
    |  ns_start   ns_part
        {
            $$ = $2;
            $$->m_text.insert(0, $1->m_text);
        }
    | ns_part  NAMESPACE
        {
            $$ = $2;
            $$->m_text.insert(0, $1->m_text);
        }
    |  ns_start   ns_part  NAMESPACE
        {
            $$ = $3;
            $$->m_text.insert(0, $2->m_text);
            $$->m_text.insert(0, $1->m_text);
        }
            
exit:   exit_part
        {
            $$ = $1;
        }
    | exit_prefix  exit_part 
        {
            $$ = $2;
            $$->m_namespace = $1;
        }


with_op:  WITH
        {
            $$ = $1;
        }

with_arg: name  '='  rval_name
        { // Именованный аргумент
            $$ = $3;
            $$->SetName($1->getText());
        }

with_args: with_arg
        {
            $$ = $1;
        }
    |  with_args  ','  with_arg
        {
            $$ = $1;
            $$->Append($2, Term::RIGHT); 
        }
        
    
with: with_op  '('  rval_name  ')'   body
        {
                $$ = $1;
                $$->SetArgs($3);
                $$->Append($5, Term::RIGHT); 
        }
    | with_op  '('  with_args  ')'  body
        {
                $$ = $1; 
                $$->SetArgs($3);
                $$->Append($5, Term::RIGHT); 
        }
    |  with_op  '('  rval_name  ')'  body  body_else
        {
                $$ = $1; 
                $$->SetArgs($3);
                $$->Append($5, Term::RIGHT); 
                $$->AppendFollow($body_else); 
        }
    |  with_op  '('  with_args  ')'  body  body_else
        {
                $$ = $1; 
                $$->SetArgs($3);
                $$->Append($5, Term::RIGHT); 
                $$->AppendFollow($body_else); 
        }

using_list: exit_prefix
            {
                $$ = $1;
            }
        | using_list  ','  exit_prefix
            {
                $$ = $1;
                $$->AppendList($3);
            }
    
ns_using:  ELLIPSIS  '='  using_list
        {
            $$ = $2;
            $$->Append($1, Term::LEFT); 
            $$->Append($3, Term::RIGHT); 
        }
    
/*  expression - одна операция или результат <ОДНОГО выражения без завершающей точки с запятой !!!!!> */
seq_item: assign_seq
            {
                $$ = $1;
            }
        | doc_before assign_seq
            {
                $$ = $assign_seq;
                Term::ListToVector($doc_before, $$->m_docs);
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
        | body_all
            {
                $$ = driver.LoadIfModule($1);
            }
        |  with
            {            
                $$ = $1;
            }
        |  ESCAPE /* for pragma terms */
            {            
                $$ = $1;
            }
        |  symbolyc
            {            
                $$ = $1;
            }
        | ns_using
            {            
                $$ = $1;
            }
        
sequence:  seq_item
            {
                $$ = $1;
            }
        | seq_item  doc_after
            {
                $$ = $1;  
                Term::ListToVector($doc_after, $$->m_docs);
            }
        | sequence  separator  seq_item
            {
                $$ = $1;  
                // Несколько команд подряд
                $$->AppendSequenceTerm($seq_item);
            }
        | sequence  separator  doc_after  seq_item
            {
                $$ = $1;  
                Term::ListToVector($doc_after, $$->m_docs);
                // Несколько команд подряд
                $$->AppendSequenceTerm($seq_item);
            }


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
        | sequence separator  doc_after
            {
                Term::ListToVector($doc_after, $1->m_docs);
                driver.AstAddTerm($1);
            }
        | separator  sequence
            {
                driver.AstAddTerm($2);
            }
        | separator  sequence separator
            {
                driver.AstAddTerm($2);
            }
        | separator  sequence separator  doc_after
            {
                Term::ListToVector($doc_after, $1->m_docs);
                driver.AstAddTerm($2);
            }
/*        | comment     Комменатарии не добавляются в AST, т.к. в парсере они не нужны, а
                        их потенциальное использование - документирование кода, решается 
                        элементами DOC_BEFORE и DOC_AFTER
*/

start	:   ast

%% /*** Additional Code ***/
