#ifndef NEWLANG_PARSER_H_
#define NEWLANG_PARSER_H_

#include "pch.h"

#include <variable.h>
#include <diag.h>
#include <macro.h>

#include <warning_push.h>
#include "parser.yy.h"
#include <warning_pop.h>

namespace newlang {

    /** The Driver class brings together all components. It creates an instance of
     * the Parser and Scanner classes and connects them. Then the input stream is
     * fed into the scanner object and the parser gets it's token
     * sequence. Furthermore the driver object is available in the grammar rules as
     * a parameter. Therefore the driver class contains a reference to the
     * structure into which the parsed data is saved. */
    class Parser {
    public:

        Parser(MacroBuffer *macro = nullptr, PostLexerType *postlex = nullptr, DiagPtr diag = nullptr, bool pragma_enable=true);

        /// enable debug output in the flex scanner
        bool trace_scanning;

        /// enable debug output in the bison parser
        bool trace_parsing;

        /// stream name (file or input stream) used for error messages.
        std::string streamname;

        std::istringstream m_stream;

        // To demonstrate pure handling of parse errors, instead of
        // simply dumping them on the standard error output, we will pass
        // them to the driver using the following two member functions.

        /** Error handling with associated line number. This can be modified to
         * output the error e.g. to a dialog box. */
        void error(const class location& l, const std::string& m);

        /** General error handling. This can be modified to output the error
         * e.g. to a dialog box. */
        void error(const std::string &m);

        /** Pointer to the current lexer instance, this is used to connect the
         * parser to the scanner. It is used in the yylex macro. */
        class Scanner* lexer;

        /** Reference to the calculator context filled during parsing of the
         * expressions. */

        void AstAddTerm(TermPtr &term);
        TermPtr GetAst();

        BlockType m_macro_analisys_buff; ///< Последовательность лексем для анализа на наличие макросов

        TermPtr m_expected;
        TermPtr m_unexpected;
        TermPtr m_finalize;
        int m_finalize_counter;
        TermPtr m_annotation;
        bool m_no_macro;
        bool m_enable_pragma;

        //        struct IndentBlock {
        //            TermPtr indent;
        //            TermPtr block;
        //        };
        //        std::vector<IndentBlock> m_indent_stack;


        parser::token_type GetNextToken(TermPtr * yylval, parser::location_type* yylloc);
        TermPtr MacroEval(const TermPtr &term);

        // Проверяет термин на наличие команды препроцессора (прагмы)
        bool PragmaCheck(const TermPtr &term);
        // Выполняет команду препроцессора (прагму)
        bool PragmaEval(const TermPtr &term, BlockType &buffer);

        TermPtr Parse(const std::string str);
        static TermPtr ParseString(const std::string str, MacroBuffer *macro, PostLexerType *postlex = nullptr, DiagPtr diag = nullptr);
        TermPtr ParseFile(const std::string str);


        time_t m_timestamp;

        std::string m_file_name;
        std::string m_file_time;
        std::string m_file_md5;

    private:
        TermPtr m_ast;
        bool m_is_runing;
        bool m_is_lexer_complete;
        MacroBuffer *m_macro;
        PostLexerType *m_postlex;
        DiagPtr m_diag;

    };

} // namespace example

#endif // NEWLANG_PARSER_H_
