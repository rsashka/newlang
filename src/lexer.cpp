
#include "lexer.h"

namespace newlang {

    Scanner::Scanner(std::istream* in, std::ostream* out, std::shared_ptr<std::string> source)
    : NewLangFlexLexer(in, out),
    source_base(source), source_string(source),
    m_macro_iss(nullptr), m_macro_count(0), m_macro_del(0),
    m_ignore_space(true), m_ignore_indent(true), m_ignore_comment(true), m_ignore_crlf(true) {
        //        yy_flex_debug = true;
    }

    Scanner::~Scanner() {
    }

    BlockType Scanner::ParseLexem(const std::string str) {

        BlockType result;
        std::istringstream strstr(str);

        Scanner lexer(&strstr);

        TermPtr tok;
        parser::location_type loc;
        while(lexer.lex(&tok, &loc) != parser::token::END) {
            result.push_back(tok);
        }
        return result;
    }

    void Scanner::ApplyDiags(DiagPtr diag) {
        if(diag) {
            //@todo Need implement ApplyDiags 
#ifdef UNITTEST            
            //            LOG_WARNING("ApplyDiags not implemenetd!");
#endif
        } else {
            m_ignore_space = true;
            m_ignore_indent = true;
            m_ignore_comment = true;
            m_ignore_crlf = true;
        }
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
