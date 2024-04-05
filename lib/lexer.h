#ifndef NEWLANG_LEXER_H_INCLUDED_
#define NEWLANG_LEXER_H_INCLUDED_




//#define YY_FATAL_ERROR(msg) newlang::Scanner::FatalError(msg, lineno(), yyleng, *source_string)
// Flex expects the signature of yylex to be defined in the macro YY_DECL, and
// the C++ parser expects it to be declared. We can factor both as follows.

#define  YY_USER_INIT    m_loc = yylloc

#define YY_DECL                             \
    newlang::parser::token_type                \
    newlang::Scanner::lex(                     \
        TermPtr * yylval,                 \
        newlang::parser::location_type* yylloc \
    )

#ifndef __FLEX_LEXER_H
#define yyFlexLexer NewLangFlexLexer
#include "FlexLexer.h"
#undef yyFlexLexer
#endif

#include "term.h"
#include "parser.h"

namespace newlang {

    /** Scanner is a derived class to add some extra function to the scanner
     * class. Flex itself creates a class named yyFlexLexer, which is renamed using
     * macros to ExampleFlexLexer. However we change the context of the generated
     * yylex() function to be contained within the Scanner class. This is required
     * because the yylex() defined in ExampleFlexLexer has no parameters. */
    class Scanner : public NewLangFlexLexer {
    public:
        /** Create a new scanner object. The streams arg_yyin and arg_yyout default
         * to cin and cout, but that assignment is only made when initializing in
         * yylex(). */
        Scanner(std::istream* arg_yyin = &std::cin,
                std::ostream* arg_yyout = &std::cout,
                SourceType source = nullptr);

        /** Required for virtual functions */
        virtual ~Scanner();

        SourceType source_base;
        SourceType source_string;
        parser::location_type* m_loc;
        std::string buffer;

        //        struct BufferDataType {
        //            SourceType data;
        //            std::istringstream *iss;
        //            parser::location_type loc;
        //        };
        //        std::vector<BufferDataType> m_data;

        SourceType m_macro_data;
        std::istringstream *m_macro_iss;
        parser::location_type m_macro_loc;

        size_t m_macro_count;
        size_t m_macro_del;
        TermPtr m_macro_body;

        static BlockType ParseLexem(const std::string str);

        void ApplyDiags(DiagPtr diag = nullptr);

        bool m_ignore_space;
        bool m_ignore_indent;
        bool m_ignore_comment;
        bool m_ignore_crlf;
        int m_bracket_depth;

        /** This is the main lexing function. It is generated by flex according to
         * the macro declaration YY_DECL above. The generated bison parser then
         * calls this virtual function to fetch new tokens. */
        virtual parser::token_type lex(
                TermPtr * yylval,
                parser::location_type* yylloc
                );

        /** Enable debug output (via arg_yyout) if compiled into the scanner. */
        void set_debug(bool b);

        void LexerError(const char* msg) override {
            LOG_RUNTIME("%s", newlang::ParserMessage(buffer, m_loc->begin.line, m_loc->begin.column, "%s", msg).c_str());
            //            LOG_RUNTIME("%s near \'%s\' at line %d", msg, position, line);
        }
    };

}

#endif // NEWLANG_LEXER_H_INCLUDED_