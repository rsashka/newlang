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

#include <core/types.h>
#include <core/term.h>

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

#define YY_TOKEN(tok) *yylval = Term::Create(TermID:: tok, yytext, yyleng, yylloc, source_string);  return token:: tok
#define YY_TOKEN_ONLY(tok) *yylval = Term::Create(TermID::UNKNOWN, yytext, yyleng, yylloc, source_string);  return token:: tok

%}

alpha   [A-Za-z]
U1      [\x80-\xbf]
U2      [\xc2-\xdf]
U3      [\xe0-\xef]
U4      [\xf0-\xf4]
ualpha  {alpha}|{U2}{U1}|{U3}{U1}{U1}|{U4}{U1}{U1}{U1}

/* запрет использования U+202A, U+202B, U+202C, U+202D, U+202E, U+2066, U+2067, U+2068, U+2069, U+061C, U+200E и U+200F */

digit   [0-9]
integer {digit}([_]?{digit}+)*
currency -?`{digit}(`?{digit}+)*
name    ({ualpha}|[_])({ualpha}|{digit}|[_])*
macro   \\({ualpha}|[_])?({name})?

/* 
type    :{name}  Не использовать термин тип в лексере, чтобы была возможность записывать типы с пробелами 
между двоеточием и имененем "term: type" или "term : type", что более привычно при чтении, чем "term :type"
Не использовать деструктов ~ в лексере, чтобы в парсере можно было проверять его место расположение (в функции или классе)
Дестрокутор калсса - имя класса, котрое начинается на два подчерка
Class __Class ????
*/
number1 -?{digit}+\.{digit}+([eE][-+]?{digit}+)?
number2 (-?{digit}+\.([eE][-+]?)?)?-?{digit}+
number  ({number1}|{number2})

/* number  [-+]?{integer}[.]{integer}([eE][-+]?[0-9]+)? */

term    [$@%]({ualpha}|[_])?({name})?

%x state_STRWIDE
%x state_STRCHAR
%x state_STRWIDE_RAW
%x state_STRCHAR_RAW
%x state_TEMPLATE1
%x state_TEMPLATE2
%x state_COMMENT
%x state_SOURCE
%x state_MACRO


%% /*** Regular Expressions Part ***/


 /* code to place at the beginning of yylex() */
%{
    // reset location
    yylloc->step();
%}


"/*"         {
                BEGIN(state_COMMENT);
                //buffer.clear();
}

<state_COMMENT>{

\n              {
                    //buffer.append(yytext, yyleng);
                    yylloc->lines(yyleng); 
                    yylloc->step();
                }

[^*^\n]*       //buffer.append(yytext, yyleng);


"*"[^*/]*      //buffer.append(yytext, yyleng);

"*/"            {
                    BEGIN(INITIAL);
//                    *yylval = Term::Create(TermID::COMMENT, buffer.c_str(), buffer.size(), yylloc, source_string);
//                    return token::COMMENT;
                }
} /* End of state_COMMENT state */

"#"[^\n]* {
      //  *yylval = Term::Create(TermID::COMMENT, &yytext[2], yyleng-2, yylloc, source_string);
        //return token::COMMENT;
}


"%{"         {
                BEGIN(state_SOURCE);
                buffer.clear();
}

<state_SOURCE>{
.                   buffer.append(yytext, yyleng);
"%}"            {
                    BEGIN(INITIAL);
                    *yylval = Term::Create(TermID::SOURCE, buffer.c_str(), buffer.size(), yylloc, source_string);
                    return token::SOURCE;
                }
"}%"            {
                    BEGIN(INITIAL);
                    *yylval = Term::Create(TermID::SOURCE, buffer.c_str(), buffer.size(), yylloc, source_string);
                    return token::SOURCE;
                }
} /* End of state_SOURCE state */


"\\\\"         {
                BEGIN(state_MACRO);
                buffer.clear();
}

<state_MACRO>{

\n              {
                    buffer.append(yytext, yyleng);
                    yylloc->lines(yyleng); 
                    yylloc->step();
                }

[^\\^\n]*       buffer.append(yytext, yyleng);
"\\\\"[^\\^\n]* buffer.append(yytext, yyleng);

"\\\\\\"        {
                    BEGIN(INITIAL);
                    *yylval = Term::Create(TermID::MACRO_BODY, buffer.c_str(), buffer.size(), yylloc, source_string);
                    return token::MACRO_BODY;
                }
} /* End of state_MARCO state */


"'''"       {
                BEGIN(state_TEMPLATE1);
                buffer.clear();
}

<state_TEMPLATE1>{
.           buffer.append(yytext, yyleng);
"'''"       {
                BEGIN(INITIAL);
                *yylval = Term::Create(TermID::TEMPLATE, buffer.c_str(), buffer.size(), yylloc, source_string);
                return token::TEMPLATE;
            }
} /* End of state_TEMPLATE1 state */



"\"\"\""    {
                BEGIN(state_TEMPLATE2);
                buffer.clear();
}

<state_TEMPLATE2>{
.                   buffer.append(yytext, yyleng);
"\"\"\""        {
                    BEGIN(INITIAL);
                    *yylval = Term::Create(TermID::TEMPLATE, buffer.c_str(), buffer.size(), yylloc, source_string);
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
            *yylval = Term::Create(TermID::STRWIDE, buffer.c_str(), buffer.size(), yylloc, source_string);
            return token::STRWIDE;
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
[^\\\n\"]+  buffer.append(yytext, yyleng);
            

} /* End of state_STRWIDE state*/


'      { /* Start state state_STRCHAR */
            BEGIN(state_STRCHAR);
            buffer.clear();
}
            
"r'"    {
                BEGIN(state_STRCHAR_RAW);
                buffer.clear();
}

<state_STRCHAR_RAW>{
.           buffer.append(yytext, yyleng);
}

<state_STRCHAR>{

'      { /* saw closing quote - all done */
            BEGIN(INITIAL);
            *yylval = Term::Create(TermID::STRCHAR, buffer.c_str(), buffer.size(), yylloc, source_string);
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


-?{number}[-+]{number}j | {number}j  YY_TOKEN(COMPLEX);
-?{number}[-+]{number}i | {number}i  YY_TOKEN(COMPLEX);
-?{integer}\\-?{integer} YY_TOKEN(FRACTION);
-?{integer}     YY_TOKEN(INTEGER);
{currency}      YY_TOKEN(CURRENCY);
{number}        YY_TOKEN(NUMBER);

"::"        YY_TOKEN(NAMESPACE);
"$*"        YY_TOKEN(ARGS);
"$$"        YY_TOKEN(PARENT);
"@@"        YY_TOKEN(NEWLANG);

"$"[0-9]+   YY_TOKEN(ARGUMENT);
{term}      YY_TOKEN(TERM);
{name}      YY_TOKEN(TERM);
{macro}     YY_TOKEN(MACRO);
".."        YY_TOKEN(RANGE);
"..."       YY_TOKEN(ELLIPSIS);
"<<->>"     YY_TOKEN_ONLY(REPEAT);
"<<-->>"    YY_TOKEN_ONLY(REPEAT);
"->"        YY_TOKEN(FOLLOW);
"-->"       YY_TOKEN(FOLLOW);
"~>"        YY_TOKEN(MATCHING);
"~~>"       YY_TOKEN(MATCHING);
"~~~>"      YY_TOKEN(MATCHING);
"==>"       YY_TOKEN(MATCHING);
"===>"      YY_TOKEN(MATCHING);

"::="       YY_TOKEN(CREATE);
":="        YY_TOKEN(CREATE_OR_ASSIGN);
"[]="       YY_TOKEN(APPEND);

"){{"       YY_TOKEN_ONLY(MIDDLE_CALL_TRY);
"){"        YY_TOKEN_ONLY(MIDDLE_CALL_BLOCK);
"{{"        YY_TOKEN_ONLY(TRY_BEGIN);
"}}"        YY_TOKEN_ONLY(TRY_END);

"::-"       YY_TOKEN(TRANSPARENT);
":-"        YY_TOKEN(TRANSPARENT);
"::&&="     YY_TOKEN(SIMPLE_AND);
":&&="      YY_TOKEN(SIMPLE_AND);
"::||="     YY_TOKEN(SIMPLE_OR);
":||="      YY_TOKEN(SIMPLE_OR);
"::^^="     YY_TOKEN(SIMPLE_XOR);
":^^="      YY_TOKEN(SIMPLE_XOR);

"//"        YY_TOKEN_ONLY(OPERATOR_DIV);
"++="       YY_TOKEN(OPERATOR);
"+="        YY_TOKEN(OPERATOR);
"-="        YY_TOKEN(OPERATOR);
"*="        YY_TOKEN(OPERATOR);
"**="       YY_TOKEN(OPERATOR);
"/="        YY_TOKEN(OPERATOR);
"//="       YY_TOKEN(OPERATOR);
"%="        YY_TOKEN(OPERATOR);
"~="        YY_TOKEN(OPERATOR);

"--"        YY_TOKEN(EXIT);

".>."        YY_TOKEN(OPERATOR);
".<."        YY_TOKEN(OPERATOR);
".>>."       YY_TOKEN(OPERATOR);
".&."        YY_TOKEN(OPERATOR);
".|."        YY_TOKEN(OPERATOR);
".^."        YY_TOKEN(OPERATOR);
".~."        YY_TOKEN(OPERATOR);

"**"        YY_TOKEN(OPERATOR);
"++"        YY_TOKEN(OPERATOR);
"!~~~"      YY_TOKEN(OPERATOR);
"~~~"       YY_TOKEN(OPERATOR);
"!~~"       YY_TOKEN(OPERATOR);
"~~"        YY_TOKEN(OPERATOR);
"!~"        YY_TOKEN(OPERATOR);
"==="       YY_TOKEN(OPERATOR);
"=="        YY_TOKEN(OPERATOR);
"!=="       YY_TOKEN(OPERATOR);
"!="        YY_TOKEN(OPERATOR);
"<="        YY_TOKEN(OPERATOR);
"<"         YY_TOKEN(OPERATOR);
">="        YY_TOKEN(OPERATOR);
">"         YY_TOKEN(OPERATOR);

"??"        YY_TOKEN(ITERATOR_QQ);
"!!"        YY_TOKEN(ITERATOR);


 /* gobble up white-spaces */
[ \t\r]+ {
    yylloc->step();
}

 /* gobble up end-of-lines */
\n {
    yylloc->lines(yyleng); yylloc->step();
}

. {

    /* Нужно для работы парсера */

    *yylval = Term::Create(TermID::SYMBOL, yytext, yyleng, yylloc, source_string);
    return static_cast<token_type>(*yytext);
}

<<EOF>>     return token::END;

%% /*** Additional Code ***/

