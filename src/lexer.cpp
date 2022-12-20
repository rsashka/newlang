
#include "lexer.h"

namespace newlang {

    Scanner::Scanner(std::istream* in, std::ostream* out, std::shared_ptr<std::string> source)
    : NewLangFlexLexer(in, out), source_base(source), source_string(source), m_macro_count(0) {
//        yy_flex_debug = true;
    }

    Scanner::~Scanner() {
    }

}

#ifdef yylex
#undef yylex
#endif

int NewLangFlexLexer::yylex() {
    std::cerr << "in NewLangFlexLexer::yylex() !" << std::endl;
    return 0;
}

/* When the scanner receives an end-of-file indication from YY_INPUT, it then
 * checks the yywrap() function. If yywrap() returns false (zero), then it is
 * assumed that the function has gone ahead and set up `yyin' to point to
 * another input file, and scanning continues. If it returns true (non-zero),
 * then the scanner terminates, returning 0 to its caller. */

int NewLangFlexLexer::yywrap() {
    return 1;
}
