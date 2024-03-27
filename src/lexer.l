/*** Flex Declarations and Options ***/


/* enable c++ scanner class generation */
%option c++

%option 8bit

/* change the name of the scanner class. results in "ExampleFlexLexer" */
%option prefix="NewLang"

/* the manual says "somewhat more optimized" */
%option batch

/* enable scanner to generate debug output. disable this for release
 * versions. */
%option debug
%option warn

/* no support for include files is planned */
%option yywrap nounput 

%option yylineno

/* enables the use of start condition stacks */
%option stack


%{ /*** C/C++ Declarations ***/

#include "lexer.h"

#include <types.h>
#include <term.h>

using namespace newlang;

/* import the parser's token type into a local typedef */
typedef newlang::parser::token token;
typedef newlang::parser::token_type token_type;

/* By default yylex returns int, we use token_type. Unfortunately yyterminate
 * by default returns 0, which is not of token_type. */
#define yyterminate() return token::END

/* This disables inclusion of unistd.h, which is not available under Visual C++
 * on Win32. The C++ scanner uses STL streams instead. */
#define YY_NO_UNISTD_H

/* The following paragraph suffices to track locations accurately. Each time
 * yylex is invoked, the begin position is moved onto the end position. */
#define YY_USER_ACTION  yylloc->columns(yyleng);

/* #define YY_TOKEN(tok) *yylval = Term::Create(token:: tok, TermID:: tok, yytext, yyleng, yylloc, source_string);  return token:: tok */
#define YY_TOKEN(tok) *yylval = Term::Create(token:: tok, TermID:: tok, yytext, yyleng, yylloc, source_string);  (*yylval)->m_bracket_depth = m_bracket_depth; return token:: tok
#define YY_TOKEN_ONLY(tok) *yylval = Term::Create(token:: tok, TermID::UNKNOWN, yytext, yyleng, yylloc, source_string);  return token:: tok

%}

alpha   [A-Za-z]
U1      [\x80-\xbf]
U2      [\xc2-\xdf]
U3      [\xe0-\xef]
U4      [\xf0-\xf4]
ualpha  {alpha}|{U2}{U1}|{U3}{U1}{U1}|{U4}{U1}{U1}{U1}

/* запрет использования U+202A, U+202B, U+202C, U+202D, U+202E, U+2066, U+2067, U+2068, U+2069, U+061C, U+200E и U+200F */


/*
sign        ::=  "+" | "-"
infinity    ::=  "Infinity" | "inf"
nan         ::=  "nan"
digit       ::=  <a Unicode decimal digit, i.e. characters in Unicode general category Nd>
digitpart   ::=  digit (["_"] digit)*
number      ::=  [digitpart] "." digitpart | digitpart ["."]
exponent    ::=  ("e" | "E") ["+" | "-"] digitpart
floatnumber ::=  number [exponent]
floatvalue  ::=  [sign] (floatnumber | infinity | nan)
*/

digit   [0-9]
/* In numbers, it is allowed to separate digits using a handwriting symbol... */
integer {digit}([_]?{digit}+)*
name    ({ualpha}|[_])({ualpha}|{digit}|[_])*^?
/* macro   @({ualpha}|[_])?({name})? */

number1 -?{digit}+\.{digit}+([eE][-+]?{digit}+)?
number2 (-?{digit}+\.([eE][-+]?)?)?-?{digit}+
number  ({number1}|{number2})

/* number  [-+]?{integer}[.]{integer}([eE][-+]?[0-9]+)? */

/* field   \.({name}|{digit})+ */

/* local   $({ualpha}|[_])?({name}|{digit})? */

/* The first digit character is allowed in module names because they are filenames... */
/* The module name uses only lowercase letters of the Latin alphabet.
   The first character cannot be an underscore, but this check occurs at the level 
   of parsing the syntax tree and checking for the presence of the file. */
module_name  \\({ualpha}|[_]|{digit})+
module  \\?{module_name}+

%x state_STRWIDE
%x state_STRCHAR
%x state_STRWIDE_RAW
%x state_STRCHAR_RAW
%x state_TEMPLATE1
%x state_TEMPLATE2
%x state_DOC_BEFORE
%x state_DOC_AFTER
%x state_COMMENT
%x state_SOURCE
%x state_MACRO_STR
%x state_EVAL


%% /*** Regular Expressions Part ***/


 /* code to place at the beginning of yylex() */
%{
    // reset location
    yylloc->step();
%}

"///<"[^\n]* {
        *yylval = Term::Create(token::DOC_AFTER, TermID::DOC_AFTER, &yytext[5], yyleng-6, yylloc, source_string);
        return token::DOC_AFTER;
}

"///"[^\n]* {
        *yylval = Term::Create(token::DOC_BEFORE, TermID::DOC_BEFORE, &yytext[4], yyleng-5, yylloc, source_string);
        return token::DOC_BEFORE;
}

"@##"           YY_TOKEN(MACRO_CONCAT);
"@#'"           YY_TOKEN(MACRO_TOSTR);
"@#\""          YY_TOKEN(MACRO_TOSTR);
"@#"            YY_TOKEN(MACRO_TOSTR);

"@$"{name}      YY_TOKEN(MACRO_ARGNAME);
"@$"[0-9]+      YY_TOKEN(MACRO_ARGPOS);

"@$..."         YY_TOKEN(MACRO_ARGUMENT);
"@$*"           YY_TOKEN(MACRO_ARGUMENT);
"@$#"           YY_TOKEN(MACRO_ARGCOUNT);

"##<"[^\n]* {
        *yylval = Term::Create(token::DOC_AFTER, TermID::DOC_AFTER, &yytext[4], yyleng-5, yylloc, source_string);
        return token::DOC_AFTER;
}

"##"[^\n]* {
        *yylval = Term::Create(token::DOC_BEFORE, TermID::DOC_BEFORE, &yytext[3], yyleng-4, yylloc, source_string);
        return token::DOC_BEFORE;
}

"/**"           {
                    yy_push_state(state_DOC_BEFORE);
                    buffer.clear();
                    buffer.append(yytext, yyleng);
                }

<state_DOC_BEFORE>{

"/*"            {
                    YY_FATAL_ERROR("Document comments cannot be nested!");
                }

\n              {
                    buffer.append(yytext, yyleng);
                    yylloc->lines(yyleng); 
                    yylloc->step();
                }

"*/"            {
                    yy_pop_state();
                    buffer.append(yytext, yyleng);
                    *yylval = Term::Create(token::DOC_BEFORE, TermID::DOC_BEFORE, buffer.c_str(), buffer.size(), yylloc, source_string);
                    return token::DOC_BEFORE;
                }

.               {
                    buffer.append(yytext, yyleng);
                }

} /* End of state_DOC_BEFORE state */


"/*"            {
                    yy_push_state(state_COMMENT);
                    buffer.clear();
                    buffer.append(yytext, yyleng);
                }

<state_COMMENT>{

"/*"            {
                    yy_push_state(state_COMMENT);
                    buffer.append(yytext, yyleng);
                }

\n              {
                    buffer.append(yytext, yyleng);
                    yylloc->lines(yyleng); 
                    yylloc->step();
                }

"*/"            {
                    yy_pop_state();
                    buffer.append(yytext, yyleng);
                    if(!m_ignore_comment && yy_start_stack_ptr == 0){
                        *yylval = Term::Create(token::COMMENT, TermID::COMMENT, buffer.c_str(), buffer.size(), yylloc, source_string);
                        return token::COMMENT;
                    }
                }

.               {
                    buffer.append(yytext, yyleng);
                }

} /* End of state_COMMENT state */

"#"[^\n]*   {
                if(!m_ignore_comment){
                    *yylval = Term::Create(token::COMMENT, TermID::COMMENT, yytext, yyleng, yylloc, source_string);  
                    return token::COMMENT;
                }
}


"{%"         {
                BEGIN(state_SOURCE);
                buffer.clear();
}

<state_SOURCE>{
.                   buffer.append(yytext, yyleng);
\n              {
                    buffer.append(yytext, yyleng);
                    yylloc->lines(yyleng); 
                    yylloc->step();
                }
"%}"            {
                    BEGIN(INITIAL);
                    *yylval = Term::Create(token::EMBED, TermID::EMBED, buffer.c_str(), buffer.size(), yylloc, source_string);
                    return token::EMBED;
                }
} /* End of state_SOURCE state */


"@@@@"      {
                m_macro_del++; 
                *yylval = Term::Create(token::MACRO_DEL, TermID::MACRO_DEL, yytext, yyleng, yylloc, source_string);
                return token::MACRO_DEL;
            }

"@@@"       {
                BEGIN(state_MACRO_STR);
                buffer.clear();
            }

"@@"        { 
                m_macro_count++; 
                *yylval = Term::Create(token::MACRO_SEQ, TermID::MACRO_SEQ, yytext, yyleng, yylloc, source_string);  
                return token::MACRO_SEQ; 
            }

<state_MACRO_STR>{

\n              {
                    buffer.append(yytext, yyleng);
                    yylloc->lines(yyleng); 
                    yylloc->step();
                }

[^@^\n]*        buffer.append(yytext, yyleng);
"@"[^@^\n]*     buffer.append(yytext, yyleng);
"@@"[^@^\n]*    buffer.append(yytext, yyleng);

"@@@"           {
                    BEGIN(INITIAL);
                    *yylval = Term::Create(token::MACRO_STR, TermID::MACRO_STR, buffer.c_str(), buffer.size(), yylloc, source_string);
                    return token::MACRO_STR;
                }
} /* End of state_MACRO_STR state */


"'''"       {
                BEGIN(state_TEMPLATE1);
                buffer.clear();
}

<state_TEMPLATE1>{
.                   buffer.append(yytext, yyleng);
\n              {
                    buffer.append(yytext, yyleng);
                    yylloc->lines(yyleng); 
                    yylloc->step();
                }
"'''"       {
                BEGIN(INITIAL);
                *yylval = Term::Create(token::TEMPLATE, TermID::TEMPLATE, buffer.c_str(), buffer.size(), yylloc, source_string);
                return token::TEMPLATE;
            }
} /* End of state_TEMPLATE1 state */



"\"\"\""    {
                BEGIN(state_TEMPLATE2);
                buffer.clear();
}

<state_TEMPLATE2>{
.                   buffer.append(yytext, yyleng);
\n              {
                    buffer.append(yytext, yyleng);
                    yylloc->lines(yyleng); 
                    yylloc->step();
                }

"\"\"\""        {
                    BEGIN(INITIAL);
                    *yylval = Term::Create(token::TEMPLATE, TermID::TEMPLATE, buffer.c_str(), buffer.size(), yylloc, source_string);
                    return token::TEMPLATE;
                }
} /* End of state_TEMPLATE2 state */


\"      { /* Start state state_STRWIDE*/
            BEGIN(state_STRWIDE);
            buffer.clear();
}
            
"r\""       {
                BEGIN(state_STRWIDE_RAW);
                buffer.clear();
}

<state_STRWIDE_RAW>{
.           buffer.append(yytext, yyleng);
}

<state_STRWIDE>{

\"      { /* saw closing quote - all done */
            BEGIN(INITIAL);
            *yylval = Term::Create(token::STRWIDE, TermID::STRWIDE, buffer.c_str(), buffer.size(), yylloc, source_string);
            return token::STRWIDE;
        }

\n      {
            /* generate error message */
            YY_FATAL_ERROR("Unterminated string constant");
        }

@[0-7]{1,3}    {
                    /* octal escape sequence */
                    int result;

                    (void) sscanf( yytext + 1, "%o", &result );
                    if ( result > 0xff ) {
                        YY_FATAL_ERROR("Octal constant is out-of-bounds");
                    }
                    char byte = (char)result;
                    buffer.append(&byte, 1);
                }

\\[0-9]+ {
            /* generate error - bad escape sequence; something
            * like '\48' or '\0777777'
            */
            YY_FATAL_ERROR("Bad escape sequence");
        }

\\n         buffer.append("\n");
\\t         buffer.append("\t");
\\r         buffer.append("\r");
\\b         buffer.append("\b");
\\f         buffer.append("\f");
\\s         buffer.append("\x20");

\\(.|\n)    buffer.append(&yytext[1], 1);
[^\\\n\"]+  buffer.append(yytext, yyleng);
            

} /* End of state_STRWIDE state*/


"r'"    {
                BEGIN(state_STRCHAR_RAW);
                buffer.clear();
}

<state_STRCHAR_RAW>{
.           buffer.append(yytext, yyleng);
}


'      { /* Start state state_STRCHAR */
            BEGIN(state_STRCHAR);
            buffer.clear();
}

<state_STRCHAR>{

'      { /* saw closing quote - all done */
            BEGIN(INITIAL);
            *yylval = Term::Create(token::STRCHAR, TermID::STRCHAR, buffer.c_str(), buffer.size(), yylloc, source_string);
            return token::STRCHAR;
        }

\n      {
            /* generate error message */
            YY_FATAL_ERROR("Unterminated string constant");
        }

\\[0-7]{1,3}    {
                    /* octal escape sequence */
                    int result;

                    (void) sscanf( yytext + 1, "%o", &result );
                    if ( result > 0xff ) {
                        YY_FATAL_ERROR("Octal constant is out-of-bounds");
                    }
                    char byte = (char)result;
                    buffer.append(&byte, 1);
                }

\\[0-9]+ {
            /* generate error - bad escape sequence; something
            * like '\48' or '\0777777'
            */
            YY_FATAL_ERROR("Bad escape sequence");
        }

\\n         buffer.append("\n");
\\t         buffer.append("\t");
\\r         buffer.append("\r");
\\b         buffer.append("\b");
\\f         buffer.append("\f");
\\s         buffer.append("\x20");

\\(.|\n)    buffer.append(&yytext[1], 1);
[^\\\n']+   buffer.append(yytext, yyleng);
            

} /* End of state_STRCHAR state*/


"`"     { /* Start state state_EVAL */
            BEGIN(state_EVAL);
            buffer.clear();
}

<state_EVAL>{

"`"     {
            BEGIN(INITIAL);
            *yylval = Term::Create(token::EVAL, TermID::EVAL, buffer.c_str(), buffer.size(), yylloc, source_string);
            return token::EVAL;
        }

\n      {
            /* generate error message */
            YY_FATAL_ERROR("Unterminated eval string");
        }

\\[0-7]{1,3}    {
                    /* octal escape sequence */
                    int result;

                    (void) sscanf( yytext + 1, "%o", &result );
                    if ( result > 0xff ) {
                        YY_FATAL_ERROR("Octal constant is out-of-bounds");
                    }
                    char byte = (char)result;
                    buffer.append(&byte, 1);
                }

\\[0-9]+ {
            /* generate error - bad escape sequence; something
            * like '\48' or '\0777777'
            */
            YY_FATAL_ERROR("Bad escape sequence");
        }

\\n         buffer.append("\n");
\\t         buffer.append("\t");
\\r         buffer.append("\r");
\\b         buffer.append("\b");
\\f         buffer.append("\f");
\\s         buffer.append("\x20");

\\(.|\n)    buffer.append(&yytext[1], 1);
[^\\\n`]+  buffer.append(yytext, yyleng);

.           buffer.append(yytext, yyleng);
            

} /* End of state_EVAL state*/


-?{number}[-+]{number}j | {number}j  YY_TOKEN(COMPLEX);
-?{number}[-+]{number}i | {number}i  YY_TOKEN(COMPLEX);
-?{integer}\\-?{integer} YY_TOKEN(RATIONAL);
-?{integer}     YY_TOKEN(INTEGER);
{number}        YY_TOKEN(NUMBER);


"$"[0-9]+   YY_TOKEN(ARGUMENT);

{name}      YY_TOKEN(NAME);
@{name}     YY_TOKEN(MACRO);
${name}     YY_TOKEN(LOCAL);
{module}    YY_TOKEN(MODULE);
"..."       YY_TOKEN(ELLIPSIS);
".."        YY_TOKEN(RANGE);
"<->"       YY_TOKEN_ONLY(REPEAT);
"-->"       YY_TOKEN(FOLLOW);
"~>"        YY_TOKEN(MATCHING);
"~~>"       YY_TOKEN(MATCHING);
"~~~>"      YY_TOKEN(MATCHING);
"==>"       YY_TOKEN(MATCHING);
"===>"      YY_TOKEN(MATCHING);

"::="       YY_TOKEN(CREATE_ONCE);
":="        YY_TOKEN(CREATE_OVERLAP);
"[]="       YY_TOKEN(APPEND);
"::-"       YY_TOKEN(PURE_OVERLAP);
":-"        YY_TOKEN(PURE_ONCE);
":=:"       YY_TOKEN(SWAP);

"::"        YY_TOKEN(NAMESPACE);
"$*"        YY_TOKEN(ARGS);
"$^"        YY_TOKEN(ARGS);
"$$"        YY_TOKEN(NAME);
"\\\\"      YY_TOKEN(MODULE);

    /*  
    "\\\\$"     YY_TOKEN(NEWLANG);
    "\\$"       YY_TOKEN(NEWLANG);
    */

"@::"       YY_TOKEN_ONLY(MACRO_NAMESPACE);
"@$$"       YY_TOKEN_ONLY(MACRO_NAMESPACE);
    /* "@\\\\"     YY_TOKEN_ONLY(MACRO); */
"$\\\\"     YY_TOKEN_ONLY(MACRO);

"@\\"       YY_TOKEN(ESCAPE);
       /* 
          "@\\)"

          "@\\,"
          "@\\;"
          "@\\:"

          "@\\s"
          "@\\t"
          "@\\n"
          "@\\r"
          "@\\b"
          "@\\f"

          "@\\#"
          "@\\##"
        */
"{@"        m_bracket_depth++; YY_TOKEN_ONLY(SYM_BEGIN);
"@}"        m_bracket_depth--; YY_TOKEN_ONLY(SYM_END);


"++"        YY_TOKEN(INT_PLUS);
"--"        YY_TOKEN(INT_MINUS);
"-+"        YY_TOKEN(INT_REPEAT);
"+-"        YY_TOKEN(INT_REPEAT);

"{*"        m_bracket_depth++; YY_TOKEN_ONLY(TRY_ALL_BEGIN);
"*}"        m_bracket_depth--; YY_TOKEN_ONLY(TRY_ALL_END);
"{+"        m_bracket_depth++; YY_TOKEN_ONLY(TRY_PLUS_BEGIN);
"+}"        m_bracket_depth--; YY_TOKEN_ONLY(TRY_PLUS_END);
"{-"        m_bracket_depth++; YY_TOKEN_ONLY(TRY_MINUS_BEGIN);
"-}"        m_bracket_depth--; YY_TOKEN_ONLY(TRY_MINUS_END);


"//"        YY_TOKEN_ONLY(OPERATOR_DIV);
    /* "++="       YY_TOKEN(OP_MATH); */
"+="        YY_TOKEN(OP_MATH);
"-="        YY_TOKEN(OP_MATH);
"*="        YY_TOKEN(OP_MATH);
        /* "**="       YY_TOKEN(OPERATOR); */
"/="        YY_TOKEN(OP_MATH);
"//="       YY_TOKEN(OP_MATH);
"%="        YY_TOKEN(OP_MATH);
"~="        YY_TOKEN(OP_COMPARE);

"||"        YY_TOKEN(OP_LOGICAL);
"^^"        YY_TOKEN(OP_LOGICAL);
"&&"        YY_TOKEN_ONLY(OPERATOR_AND);

"&^"        YY_TOKEN_ONLY(OPERATOR_PTR);
"&&^"       YY_TOKEN_ONLY(OPERATOR_PTR);
"&*"        YY_TOKEN_ONLY(OPERATOR_PTR);
"&*^"       YY_TOKEN_ONLY(OPERATOR_PTR);
"&?"        YY_TOKEN_ONLY(OPERATOR_PTR);
"&?^"       YY_TOKEN_ONLY(OPERATOR_PTR);

    /*
    "&%"        YY_TOKEN_ONLY(OPERATOR_PTR);
    "&@"        YY_TOKEN_ONLY(OPERATOR_PTR);
    "&~"        YY_TOKEN_ONLY(OPERATOR_PTR);
    "&:"        YY_TOKEN_ONLY(OPERATOR_PTR);
    "&$"        YY_TOKEN_ONLY(OPERATOR_PTR);
    "&#"        YY_TOKEN_ONLY(OPERATOR_PTR);
    */

".>."        YY_TOKEN(OP_BITWISE);
".<."        YY_TOKEN(OP_BITWISE);
".>>."       YY_TOKEN(OP_BITWISE);
".&."        YY_TOKEN(OP_BITWISE);
".|."        YY_TOKEN(OP_BITWISE);
".^."        YY_TOKEN(OP_BITWISE);
".~."        YY_TOKEN(OP_BITWISE);

"*^"        YY_TOKEN(TAKE);
"**"        YY_TOKEN(WITH);
"!~~~"      YY_TOKEN(OP_COMPARE);
"~~~"       YY_TOKEN_ONLY(OPERATOR_DUCK);
"!~~"       YY_TOKEN(OP_COMPARE);
"~~"        YY_TOKEN_ONLY(OPERATOR_DUCK);
"!~"        YY_TOKEN(OP_COMPARE);
"==="       YY_TOKEN(OP_COMPARE);
"=="        YY_TOKEN(OP_COMPARE);
"!=="       YY_TOKEN(OP_COMPARE);
"!="        YY_TOKEN(OP_COMPARE);
"=/="       YY_TOKEN(OP_COMPARE);
"<="        YY_TOKEN(OP_COMPARE);
">="        YY_TOKEN_ONLY(OPERATOR_ANGLE_EQ);

"[+]"        YY_TOKEN(OP_MATH); /* Объединение множеств */
"[-]"        YY_TOKEN(OP_MATH); /* Разность множеств */
"[*]"        YY_TOKEN(OP_MATH); /* Пересечение множеств */
"[/]"        YY_TOKEN(OP_MATH);

    /* Симметри́ческая ра́зность двух множеств — теоретико-множественная операция, результатом которой является новое множество, включающее все элементы исходных множеств, не принадлежащие одновременно обоим исходным множествам. Другими словами, если есть два множества A A и B B, их симметрическая разность есть объединение элементов A A, не входящих в B B, с элементами B B, не входящими в A A. На письме для обозначения симметрической разности множеств A A и B B используется обозначение A △ B {\displaystyle A\bigtriangleup B}, реже используется обозначение A − ˙ B {\displaystyle A\,{\dot {-}}\,B} или A + B A+B[1].
    */
    /*
    Приоритет операций
    Последовательность выполнения операций над множествами, как и обычно, может быть задана скобками. 
    При отсутствии скобок сначала выполняются унарные операции (дополнение), затем — пересечения, затем — объединения, разности и симметрической разности. 
    Операции одного приоритета выполняются слева направо. При этом надо иметь в виду, что в отличие от арифметических сложения и вычитания, 
    для которых, в частности, верно, что ( a + b ) − c = a + ( b − c ) {\displaystyle (a+b)-c=a+(b-c)}, 
    для аналогичных операций над множествами это неверно. 
    Например, если A = { 1 , 3 } {\displaystyle A=\{1,3\}}, B = { 1 , 2 } {\displaystyle B=\{1,2\}}, C = { 2 , 3 } {\displaystyle C=\{2,3\}}, 
    то ( A ∪ B ) ∖ C = { 1 } {\displaystyle (A\cup B)\setminus C=\{1\}}, 
    но, в то же время, A ∪ ( B ∖ C ) = { 1 , 3 } {\displaystyle A\cup (B\setminus C)=\{1,3\}}. 
    */

"!!"        YY_TOKEN(ITERATOR);
"??"        YY_TOKEN(ITERATOR);
"?!"        YY_TOKEN_ONLY(ITERATOR_QQ);
"!?"        YY_TOKEN_ONLY(ITERATOR_QQ);


^[ \t]+ {
    yylloc->step();
    if(!m_ignore_indent){
        *yylval = Term::Create(token::INDENT, TermID::INDENT, yytext, yyleng, yylloc, source_string);
        return token::INDENT;
    }
}

[ \t]+ {
    yylloc->step();
    if(!m_ignore_space){
        *yylval = Term::Create(token::SPACE, TermID::SPACE, yytext, yyleng, yylloc, source_string);
        return token::SPACE;
    }
}

\n {
    yylloc->lines(yyleng); yylloc->step();
    if(!m_ignore_crlf){
        *yylval = Term::Create(token::CRLF, TermID::CRLF, yytext, yyleng, yylloc, source_string);
        return token::CRLF;
    }
}

"{"     {
            m_bracket_depth++;
            *yylval = Term::Create(static_cast<token_type>(*yytext), TermID::SYMBOL, yytext, yyleng, yylloc, source_string);
            return static_cast<token_type>(*yytext);
        }
"}"     {
            m_bracket_depth--;
            *yylval = Term::Create(static_cast<token_type>(*yytext), TermID::SYMBOL, yytext, yyleng, yylloc, source_string);
            return static_cast<token_type>(*yytext);
        }

. {

    /* Нужно для работы парсера */

    *yylval = Term::Create(static_cast<token_type>(*yytext), TermID::SYMBOL, yytext, yyleng, yylloc, source_string);
    return static_cast<token_type>(*yytext);
}

<<EOF>>     {
        *yylval = Term::Create(token::END, TermID::END, yytext, yyleng, yylloc, source_string);
        return token::END;
}

%% /*** Additional Code ***/

