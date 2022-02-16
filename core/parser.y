
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

%token			CREATE		"::="
%token			CREATE_OR_ASSIGN ":="
%token			TENSOR_BEGIN
%token			TENSOR_END
%token			POWER		"**"

%token			APPEND		"[]"
%token			FOLLOW		"->"
%token			REPEAT		"<->"

%token			ARGUMENT		
%token			CONCAT

%token			CLASS		"@@"
%token			EXIT		"--"
%token			EXCEPTION       "##"

//%token			ARGCOUNT        "$#"
%token			ARGSSTR		"$*"

%token			RANGE           ".."
%token			ELLIPSIS        "..."
%token			NAMESPACE       "::"
%token			ASSIGN_MATH     "Arithmetic assign value"

%token			END         0	"end of file"

%token                  SOURCE
%token			ITERATOR

%token			TRANSPARENT
%token			SIMPLE_AND
%token			SIMPLE_OR
%token			SIMPLE_XOR
%token			LAMBDA
%token			OPERATOR


/* Есть предупреждения, связанные с выполняемым кодом
 * shift/reduce conflict on token '{'
 * shift/reduce conflict on token '%{'
 *  */
//%expect 2
/* Можно сделать проверку синтаксиса для чистых функций на уровне парсера, но придется делать два варианта основных операций 
 * (кода, выражений, логики и т.д.) только с разрешенными объектами */

%% /*** Grammar Rules ***/


dim: INTEGER
        {
            $$ = $1;
        }
    | TERM  '='  INTEGER
        {
            $$ = $3;
            $$->SetName($1->getText());
        }

dims: dim
        {
            $$ = $1;
        }
    | dims  ','  dim
        {
            $$ = $1;
            $$->AppendSeq($3);
//            $$->Last()->Append($3);
        }

    
type:  ':'  TERM
        {
            $$ = $2;
        }
    | ':'  TERM  '['  dims   ']'
        {
            $$ = $2; 
            $$->SetArgs($4);
        }

/*
type: type_name
        {
            $$ = $1;
        }
    | type_name  '['  ']'
        {
            $$ = $1;
            $$->SetTermID(TermID::DICT);
        }
    | type_name  '['  ELLIPSIS  ']'
        {
            $$ = $1;
            $$->SetArgs($3);
            $$->SetTermID(TermID::DICT);
        }
    | type_name  '['  dim  ']'
        {
            $$ = $1;
            $$->SetArgs($3);
            $$->SetTermID(TermID::DICT);
        }
*/

digits: INTEGER
            {
                $$ = $1;
                $$->SetType(nullptr);
            }
        | INTEGER  type
            {
                $$ = $1;
                $$->SetType($2);
            }
         | NUMBER
            {
                $$ = $1;
                $$->SetType(nullptr);
            }
         | NUMBER  type
            {
                $$ = $1;
                $$->SetType($2);
            }
         | COMPLEX
            {
                $$ = $1;
                $$->SetType(nullptr);
            }
         | COMPLEX  type
            {
                $$ = $1;
                $$->SetType($2);
            }
         | FRACTION
            {
                $$ = $1;
                $$->SetType(nullptr);
            }
         | FRACTION  type
            {
                $$ = $1;
                $$->SetType($2);
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

strindex:  strtype  '['  ind_args  ']'
        {
            $$ = $1; 
            $2->SetTermID(TermID::INDEX);
            $2->SetArgs($ind_args);
            $$->Last()->Append($2);
        }
        
string: strtype
        {
            $$ = $1;
        }
    | strtype  '('  define_vals  ')'
        {
            $$ = $1;
            $$->SetArgs($3);
        }
    | strindex
        {
            $$ = $1;
        }
/*    | STRVAR
        {
            $$ = $1;
        }
    | STRVAR  '('  define_vals  ')'
        {
            $$ = $1;
            $$->SetArgs($3);
        }*/


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

ns_term:  TERM
            {
                  $$ = $1;
            }
        |  namespace  TERM
            {
                $$ = $2;  
                $$->m_namespace.swap($1->m_namespace);
            }
        |  ns_term  type
            {
                $$ = $1;
                $$->SetType($2);
            }
/*
count:  ARGCOUNT
            {
                $$ = $1;
            }
       | ARGCOUNT  '(' arg  ')'
            {
                $$=$1;
                $$->push_back($arg, "");
            }
*/
tostr:  ARGSSTR
            {
                $$ = $1;
            }
       | ARGSSTR  '(' arg  ')'
            {
                $$=$1;
                $$->push_back($arg, "");
            }

term:   ns_term
            {
                $$ = $1;
            }
        | ARGUMENT
            {
                $$ = $1;
            }
        | EXIT
            {
                $$ = $1;
            }
        | tostr
            {
                $$ = $1;
            }
/*        | count
            {
                $$ = $1;
            }*/
        |  term  '['  ind_args  ']'
            {   
                $$ = $1; 
                $2->SetTermID(TermID::INDEX);
                $2->SetArgs($ind_args);
                $$->Last()->Append($2);
            }
        |  term  '['  ind_args  ']'  type
            {   
                $$ = $1; 
                $2->SetTermID(TermID::INDEX);
                $2->SetArgs($ind_args);
                $$->Last()->Append($2);
                $$->SetType($5);
            }
        |  term  '.'  TERM
            {
                $$ = $1; 
                $3->SetTermID(TermID::FIELD);
                $$->Last()->Append($3);
            }
      



ind_arg:  arithmetic
            {
                $$ = $1;
            }
        | range
            {
                $$ = $1;
            }
        | ELLIPSIS
            {
                $$ = $1;
            }


ind_args:  ind_arg
            {
                $$ = $1;
            }
        | ind_args  ','  ind_arg
            {
                $$ = $1;
                $$->AppendSeq($3);
//                $$->Last()->Append($3);
            }
            
            
/* eval возвращает результат выполнения */
eval:  term  
            {
                $$ = $1;  
            }
        | func_term  call
            {
                $$ = $1;  
                $$->SetTermID(TermID::CALL);
                $$->SetArgs($2);
            }
        | term  '.'  TERM  call
            {
                $$ = $1; 
                $3->SetTermID(TermID::FIELD);
                $3->SetArgs($4);
                $$->Last()->Append($3);
            }
        | iterator
            {
                $$ = $1;  
            }
        | TERM   '!'
            {
                $$ = $2;
                $$->SetTermID(TermID::ITERATOR);
                $$->Last()->Append($1, Term::LEFT); 
            }

        
iter_call:  '?'
            {
                $$=$1;
            }
        | '?'  call
            {
                $$=$1;
                $$->SetArgs($2);
            }
        | ITERATOR /* !! ?? */
            {
                $$=$1;
            }
        | ITERATOR  call
            {
                $$=$1;
                $$->SetArgs($2);
            }
          

iterator: eval  iter_call
            {
                $$ = $2;
                $$->SetTermID(TermID::ITERATOR);
                $$->Last()->Append($1, Term::LEFT); 
            }


call:  '('  ')'
            {   
                $$ = $1;
                $$->clear_();
            }
        | '('  args  ')'
            {
                $$ = $2;
            }
        | '('  named_args  ')'
            {
                $$ = $2;
            }
        | '('  args  ','  named_args  ')'
            {
                $$ = $2;
                $$->AppendSeq($4);
//                $$->Last()->Append($4);
            }

call_def:  call
            {   
                $$ = $call;
            }
        | '('  ELLIPSIS  ')'
            {   
                $$ = $2;
            }
        | '('  args  ','  ELLIPSIS  ')'
            {
                $$ = $2;
                $$->AppendSeq($4);
//                $$->Last()->Append($4);
            }
        | '('  named_args  ','  ELLIPSIS  ')'
            {
                $$ = $2;
                $$->AppendSeq($4);
//                $$->Last()->Append($4);
            }
        | '('  args  ','  named_args  ','  ELLIPSIS  ')'
            {
                $$ = $2;
                $$->AppendSeq($4);
                $$->AppendSeq($6);
//                $$->Last()->Append($4);
//                $$->Last()->Append($6);
            }
     

       
args: arg
        {
            $$ = $1;
        }
    | args  ','  arg
        {
            $$ = $1;
            $$->AppendSeq($3);
//            $$->Last()->Append($3); // Link args form right side
        }

    
arg: rval
        {
            $$ = $1;
        }
/*    | arithmetic
        {
            $$ = $1;
        }
    |  logical
        {
            $$ = $1;
        }*/
    |  collection_all
        {
            $$ = $1;
        }
    | '&'  TERM
        {
            $$ = $2;
            $$->MakeRef();
        }
    |  TERM  type
        {
            $$ = $1;
            $$->SetType($2);
        }
    |  '&'  TERM  type
        {
            $$ = $2;
            $$->SetType($3);
            $$->MakeRef();
        }
    |  ELLIPSIS  ELLIPSIS  term
        {
            // Раскрыть элементы словаря в последовательность ИМЕНОВАННЫХ параметров
            $$ = $2;
            $$->Append($1, Term::LEFT); 
            $$->Append($3, Term::RIGHT); 
        }
    |  ELLIPSIS  term  
        {
            // Раскрыть элементы словаря в последовательность не именованных параметров
            $$ = $1; 
            $$->Append($2);
        }
    |  lambda
        {
            $$ = $1;
        }
/*    | TERM   '!'
        {
            $$ = $2;
            $$->SetTermID(TermID::ITERATOR);
            $$->Last()->Append($1, Term::LEFT); 
        }
*/
        
   
lambda: '('  ')' '{'  '}'
        {
            $$ = $1;
            $$->m_text.clear();
            $$->SetTermID(TermID::LAMBDA);
        }
    | '('  ')' LAMBDA '{'  '}'
        {
            $$ = $3;
        }
    | '(' define_vals ')' LAMBDA '{'  '}'
        {
            $$ = $4;
            $$->SetArgs($2);
        }
    | '(' ')' LAMBDA '{'  simple_seq  '}'
        {
            $$ = $3;
            $$->Append($simple_seq, Term::RIGHT);
        }
    | '(' define_vals ')' LAMBDA '{'  simple_seq  '}'
        {
            $$ = $4;
            $$->SetArgs($2);
            $$->Append($simple_seq, Term::RIGHT);
        }
    | '('  ')'  type  '{'  '}'
        {
            $$ = $1;
            $$->m_text.clear();
            $$->SetTermID(TermID::LAMBDA);
            $$->SetType($3);
        }
    

named_args: named_arg
                {
                    $$ = $1;
                }
            | named_args  ','  named_arg
                {
                    $$ = $1;
                    $$->AppendSeq($3);
//                    $$->Last()->Append($3);
                }

named_arg:  TERM  '='
                {
                    $$ = $1;
                    $1->m_text.swap($1->m_name);
//                    $$->SetName($1->getText());
                }
            | TERM  '='  arg
                {
                    $$ = $3;
                    $$->SetName($1->getText());
                }
            |  TERM  type  '='  arg
                {
                    $$ = $4;
                    $$->SetName($1->getText());
                    $$->SetType($2);
                }
           
           
array: '['  args  ','  ']'
            {
                $$ = $1;
                $$->m_text.clear();
                $$->SetTermID(TermID::TENSOR);
                $$->SetArgs($2);
            }
        | '['  args  ','  ']'  ':'  TERM
            {
                $$ = $1;
                $$->m_text.clear();
                $$->SetTermID(TermID::TENSOR);
                $$->SetArgs($2);
                $$->SetType($6);
            }
        
define_val: arg
            {
                $$ = $1;
            }
        | named_arg
            {
                $$ = $1;
            }

define_vals: define_val
                {
                    $$ = $1;
                }
            |  define_vals  ','  define_val
                {
                    $$ = $1;
                    $$->AppendSeq($3);
//                    $$->Last()->Append($3);
                }

dictionary: '('  ','  ')'
            {
                $$ = $1;
                $$->m_text.clear();
                $$->SetTermID(TermID::DICT);
            }
        | '('  define_vals  ','  ')'
            {
                $$ = $1;
                $$->m_text.clear();
                $$->SetTermID(TermID::DICT);
                $$->SetArgs($2);
            }

        
class_no_type: '('  ')'
            {
                $$ = $1;
                $$->m_text.clear();
                $$->SetTermID(TermID::CLASS);
            }
        | '('  define_vals  ')'
            {
                $$ = $1;
                $$->m_text.clear();
                $$->SetTermID(TermID::CLASS);
                $$->SetArgs($2);
            }


class:  class_no_type
            {
                $$ = $1;
            }
        | class_no_type ':'  term
            {
                $$ = $1;
                $$->m_class_name.swap($3->m_text);
            }

collection: array 
            {
                $$ = $1;
            }
        | dictionary
            {
                $$ = $1;
            }
        
collection_all: collection        
            {
                $$ = $1;
            }
        | class
            {
                $$ = $1;
            }
        

lval_item:  term
            {
                $$ = $1;
            }
        |  TERM  type
            {
                $$ = $1;
                $$->SetType($2);
            }
        |  ELLIPSIS
            {
                $$ = $1;
            }
        

lval:  lval_item
            {
                $$ = $1;
            }
        | lval  ','  lval_item
            {
                $$ = $1;
                $$->AppendSeq($3);
//                $$->Last()->Append($3);
            }

/*
append:  APPEND  CREATE
            {
                // Append a new item to the array (:=)
                $$ = $2;
                $$->SetTermID(TermID::APPEND);
            }
        |  APPEND  '.'  TERM  CREATE 
            {
                // Append a new named item to the dictionary (:=)
                $$ = $4;
                $$->SetTermID(TermID::APPEND);
                $$->m_name.assign($3->getText());
            }
*/       
            
assign:  '='  /* ASSIGN */
            {
                $$ = $1;
                $$->SetTermID(TermID::ASSIGN);
            }
        | CREATE_OR_ASSIGN /* := */
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

assign_val: assign 
            {
                $$=$1;
            }

assign_local:  lval  assign   /* REMOVE  value */
            {
                $$ = $2;  
                $$->Append($1, Term::LEFT); 
            }
        |  lval  assign_val  rval
            {
                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }
        |  type  assign_val  type
            {
                $1->SetType($1);
                $1->m_text.insert(0, ":");
                $1->m_type_name.insert(0, ":");

                $3->SetType($3);
                $3->m_text.insert(0, ":");
                $3->m_type_name.insert(0, ":");

                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }
        |  type  assign_val  dictionary
            {
                $1->SetType($1);
                $1->m_text.insert(0, ":");
                $1->m_type_name.insert(0, ":");

                $3->SetType($3);

                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }

        |  type  assign_val  class
            {
                $1->SetType($1);
                $1->m_text.insert(0, ":");
                $1->m_type_name.insert(0, ":");

                $3->SetType($3);

                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }
        |  type  assign_val  func_term  call
            {
                $1->SetType($1);
                $1->m_text.insert(0, ":");
                $1->m_type_name.insert(0, ":");

                $3->SetTermID(TermID::CALL);
                $3->SetArgs($4);

                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }
        |  type  assign_val  ':' func_term  call
            {
                $1->SetType($1);
                $1->m_text.insert(0, ":");
                $1->m_type_name.insert(0, ":");

                $4->SetType($4);
                $4->m_text.insert(0, ":");
                $4->m_type_name.insert(0, ":");
                $4->SetTermID(TermID::CALL);
                $4->SetArgs($5);

                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $$->Append($4, Term::RIGHT); 
            }
/*        | lval  assign_val  arithmetic
            {
                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }
        | lval  assign_val  logical
            {
                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }*/
        | lval  assign_val  collection_all
            {
                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }
/*        | lval  assign_val  class
            {
                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }*/
/*        | lval  assign_val  class_name
            {
                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            } */
/*        | lval  assign_val  tensor
            {
                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            } */
        | lval  assign_val  lambda
            {
                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }
        | lval  CREATE  iter_call
            {
                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }
/*        | lval  SESSION  iter_call
            {
                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }*/
            | function
                {
                    $$ = $1;  
                }
            | simple_func
                {
                    $$ = $1;  
                }
        
assign_global: assign_local
                {
                    $$ = $1;
                }
        
       
func_term: ns_term
            {
                $$ = $1;
            }
        |  symbols
            {
                $$ = $1;
            }        
        |  namespace  symbols
            {
                $$ = $1;
                $$->m_text = $2->m_text;
            }        

        
func_name:  func_term  call_def
            {
                $$ = $1;  
                $$->SetArgs($2);
                $$->SetTermID(TermID::CALL);
            }
        |  func_term   call_def  type
            {
                $$ = $1;  
                $$->SetArgs($2);
                $$->SetTermID(TermID::CALL);
                $$->SetType($3);
            }

        
        
function: func_name  assign  exec_src
            {
                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT);
                $$->SetTermID(TermID::FUNCTION);
            }
        | func_name  TRANSPARENT  exec_src
            {
                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT);
            }
        | func_name  assign  '{'  '}'
            {
                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $3->clear_();
                $$->Append($3, Term::RIGHT);
            }
        | func_name  TRANSPARENT  '{'  '}'
            {
                $$ = $2;  
                $$->Append($1, Term::LEFT); 
                $3->clear_();
                $$->Append($3, Term::RIGHT);
            }
/*        |  func_name  func_def   call_def   '{'  source  '}'
            {
                $$ = $2;  
                $$->SetTermID(TermID::FUNCTION);
                $$->Append($1, Term::LEFT); 
                
                $source->SetArgs($call);
                $$->Append($source, Term::RIGHT);
            }
*/


simple_item:  logical
                {
                    $$ = $1;
                }
            | assign_local
                {
                    $$ = $1;
                }
            | arithmetic
                {
                    $$ = $1;
                }

simple_seq:  simple_item
                {
                    $$ = $1;
                }
            | simple_seq ',' simple_item
                {
                    $$ = $1;
                    // Несколько команд подряд
                    $$->BlockCodeAppend($3); 
//                    $$->push_back($3);
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
            
simple_func: func_name  simple  simple_seq
        {
            $$ = $2;  
            $$->Append($1, Term::LEFT); 
            $$->Append($3, Term::RIGHT);
        }
            
            
    

arithmetic: arithmetic '+' addition 
                { 
                    $$ = $2;
                    $$->SetTermID(TermID::OPERATOR);
                    $$->Append($1, Term::LEFT);                    
                    $$->Append($3, Term::RIGHT); 
                }
            | arithmetic '-' addition 
                { 
                    $$ = $2;
                    $$->SetTermID(TermID::OPERATOR);
                    $$->Append($1, Term::LEFT);                    
                    $$->Append($3, Term::RIGHT); 
                }
            |  digits   digits
                {
                    //@todo location
                    $$ = Term::Create(TermID::OPERATOR, $2->m_text.c_str(), 1, nullptr);
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
            |  digits  digits  op_factor  factor 
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
                }
            |  sfactor
                { 
                    $$ = $1; 
                }    
            

sfactor: factor              
            { 
                $$ = $1; 
            }
/*        | '(' factor ')'
            { 
                $$ = $2; 
            }*/
/*        | '-' factor
            {
                $$ = $1;
                $$->SetTermID(TermID::OPERATOR);
                $$->Append($2, Term::RIGHT); 
            }
        | '+' factor
            {
                $$ = $2;
            }
*/        

factor:   digits
            {
                $$ = $1; 
            }
        |  eval
            {
                $$ = $1; 
            }
/*        | '('  arithmetic  ')'
            {
                $$ = $2; 
            }
*/        

str_op: string
        {
            $$ = $1;
        }
/*    | string CONCAT string
        {
            $$ = $2;
            $$->SetTermID(TermID::CONCAT);
            $$->Append($1, Term::LEFT); 
            $$->Append($3, Term::RIGHT); 
        } */
    | rval CONCAT rval
        {
            $$ = $2;
            $$->SetTermID(TermID::CONCAT);
            $$->Append($1, Term::LEFT); 
            $$->Append($3, Term::RIGHT); 
        }
    | string  POWER  INTEGER
        {
            $$ = $2;
            $$->SetTermID(TermID::OPERATOR);
            $$->Append($1, Term::LEFT); 
            $$->Append($3, Term::RIGHT); 
        }
    
rval:  str_op
        {
            $$ = $1;
        }
    |  arithmetic
        {
            $$ = $1;
        }
    |  logical
        {
            $$ = $1;
        }
    |  collection_all
        {
            $$ = $1;
        }
    | TENSOR_BEGIN   rval  TENSOR_END
        {
            $$ = $1;
            $$->Append($2, Term::LEFT); 
        }
    | TENSOR_BEGIN   rval  TENSOR_END  type
        {
            $$ = $1;
            $$->SetType($4);
            $$->Append($2, Term::LEFT); 
        }
    | TENSOR_BEGIN   rval  ELLIPSIS TENSOR_END
        {
            $$ = $1;
            $$->Append($2, Term::LEFT); 
            $$->Append($3, Term::RIGHT); 
        }
    | TENSOR_BEGIN   rval  ELLIPSIS TENSOR_END  type
        {
            $$ = $1;
            $$->SetType($type);
            $$->Append($2, Term::LEFT); 
            $$->Append($3, Term::RIGHT); 
        }
    | TENSOR_BEGIN   range  TENSOR_END
        {
            $$ = $1;
            $$->Append($2, Term::LEFT); 
        }
    | TENSOR_BEGIN   range  TENSOR_END  type
        {
            $$ = $1;
            $$->SetType($type);
            $$->Append($2, Term::LEFT); 
        }

    
    
operator: OPERATOR
            {
                $$ = $1;
            }
        | POWER
            {
                $$ = $1;
            }
        
logical: rval operator rval
        {
            $$ = $2;
            $$->Append($1, Term::LEFT); 
            $$->Append($3, Term::RIGHT); 
        }
    | rval  LAMBDA  rval /* && || ^^ */
        {
            $$ = $2;
            $$->SetTermID(TermID::OPERATOR);
            $$->Append($1, Term::LEFT); 
            $$->Append($3, Term::RIGHT); 
        }
    | rval  '~'  rval
        {
            $$ = $2;
            $$->SetTermID(TermID::OPERATOR);
            $$->Append($1, Term::LEFT); 
            $$->Append($3, Term::RIGHT); 
        }
/*    | rval  operator  collection_all
        {
            $$ = $2;
            $$->SetTermID(TermID::OPERATOR);
            $$->Append($1, Term::LEFT); 
            $$->Append($3, Term::RIGHT); 
        }
    | rval  '~'  collection_all
        {
            $$ = $2;
            $$->SetTermID(TermID::OPERATOR);
            $$->Append($1, Term::LEFT); 
            $$->Append($3, Term::RIGHT); 
        }
*/

source: SOURCE
            {
                $$=$1;
            }
        |  source  SOURCE
            {
                $$=$1;
                $$->Last()->Append($2); 
            }

end:  ';'  | END


expression: source
            {
                $$=$1;
            }
        | source  ';'
            {
                $$=$1;
            }
        | follow
            {
                $$ = $1; 
            }
        | follow  ';'
            {
                $$ = $1; 
            }
        |  eval   end
            {
                $$=$1;
            }
        |  arg   end
            {
                $$=$1;
            }
        | range  end
            {
                $$=$1;
            }
        | repeat  end
            {
                $$ = $1; 
            }
        |  assign_global  end
            {
                $$=$1;
            }
        | EXIT  end
            {
                $$=$1;
            }
/*        | count  end
            {
                $$=$1;
            }*/

/*       
exit: EXIT   
        {
            $$=$1;
        }
    | RETURN  arg
        {
            $$=$1;
            $$->Append($2, Term::RIGHT); 
        }
*/
        
follow: if_then
            {
                $$ = $1; 
                $$->AppendFollow($1);
            }
        | if_then  FOLLOW  exec_src
            {
                // final else
                $$ = $1; 
                $2->Append($3, Term::RIGHT); 
                $$->AppendFollow($1);
                $$->AppendFollow($2);
            }
        | if_then  follow
            {
                $$ = $1; 
                $$->m_follow.swap($2->m_follow);
                $$->InsertFollow($1);
            }

cond: logical
        {
            $$ = $1;
        }
    | arithmetic
        {
            $$ = $1;
        }
            
        
if_then:  cond  FOLLOW  exec_src
            {
                $$=$2;
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }
        |  '[' cond ']'  FOLLOW  exec_src
            {
                $$=$4;
                $$->Append($2, Term::LEFT); 
                $$->Append($5, Term::RIGHT); 
            }

range_val:  eval 
        {
            $$ = $1;  
        }
    |  digits
        {
            $$ = $1;  
        }
    |  '-'  digits
        {
            $$ = $2;
            $$->m_text.insert(0,"-");
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
        
rep_item:  eval
            {
                $$=$1;
            }
        | eval  '!'
            {
                $$=$2;
                $$->SetTermID(TermID::ITERATOR);
                $$->Last()->Append($1, Term::LEFT); 
            }
        | TERM  '='  eval
            {
                $$ = $2;
                $$->SetTermID(TermID::ASSIGN);
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }
        | TERM  '='  eval  '!'
            {
                $$ = $2;
                $$->SetTermID(TermID::ASSIGN);
                $$->Append($1, Term::LEFT); 
                
                $4->SetTermID(TermID::ITERATOR);
                $4->Last()->Append($3, Term::LEFT); 
                $$->Append($4, Term::RIGHT); 
            }
        | range
            {
                $$=$1;
            }
        |  TERM  '='  range
            {
                $$ = $2;  
                $$->SetTermID(TermID::ASSIGN);
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }
        
repeat_counter:  rep_item
            {
                $$=$1;
                $$->AppendFollow($1);
                
            }
        | repeat_counter  ';'  rep_item
            {
                $$=$1;
                $$->AppendFollow($3);
            }
            
repeat:  cond  REPEAT  exec_src
            {
                $$=$2;
                $$->Append($1, Term::LEFT); 
                $$->Append($3, Term::RIGHT); 
            }
        | '[' cond ']'  REPEAT  exec_src
            {
                $$=$4;
                $$->Append($2, Term::LEFT); 
                $$->Append($5, Term::RIGHT); 
            }
        | '[' cond ']'  '?'  REPEAT  exec_src
            {
                $$=$5;
                $4->AppendFollow($2);
                $4->SetTermID(TermID::ITERATOR);
                $$->Append($4, Term::LEFT); 
                $$->Append($6, Term::RIGHT); 
            }
        | '[' repeat_counter ']'  '?'  REPEAT  exec_src
            {
                $$=$5;
                $4->m_follow.swap($2->m_follow);
                $4->SetTermID(TermID::ITERATOR);
                $$->Append($4, Term::LEFT); 
                $$->Append($6, Term::RIGHT); 
            }

exec_items:  expression
            {
                $$=$1;
            }
        | exec_items  expression
            {
                $$=$1;
                // Несколько команд подряд
                $$->BlockCodeAppend($2); 
            }

        
exec_src:  expression
            {
                $$ = $1; 
            }
        | '{'  exec_items  '}'
            {
                $$ = $2; 
            }

        
execution:  exec_items
            {
                $$ = $1; 
            }
        
ast:  %empty | ';'
        |  execution
            {
               driver.AstAddTerm($1);
            }
        |  rval  END
            {
               driver.AstAddTerm($1);
            }

start	:   ast

%% /*** Additional Code ***/
