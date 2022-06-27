#include "pch.h"

#ifdef UNITTEST

#include <core/warning_push.h>
#include <gtest/gtest.h>
#include <core/warning_pop.h>

#include <core/term.h>
#include <core/lexer.h>
#include <core/parser.h>
#include <core/nlc.h>


using namespace newlang;

class Lexer : public ::testing::Test {
protected:

    std::vector<TermPtr> tokens;

    void SetUp() {
    }

    void TearDown() {
    }

    int64_t Parse(const char *str) {
        std::istringstream strstr(str);

        Scanner lexer(&strstr);
        tokens.clear();
        TermPtr tok;
        parser::location_type loc;
        while(lexer.lex(&tok, &loc) != parser::token::END) {
            tokens.push_back(tok);
        }
        return tokens.size();
    }

    int Count(TermID token_id) {
        int result = 0;
        for (size_t i = 0; i < tokens.size(); i++) {
            if(tokens[i]->GetTokenID() == token_id) {
                result++;
            }
        }
        return result;
    }

    std::string Dump() {
        std::string result;
        for (int i = 0; i < tokens.size(); i++) {
            result += tokens[i]->m_text;
            result += " ";
        }
        return result;
    }

};

TEST_F(Lexer, Word) {
    ASSERT_EQ(1, Parse("alpha  "));
    EXPECT_EQ(1, Count(TermID::TERM));
    EXPECT_STREQ("alpha", tokens[0]->getText().c_str());


    ASSERT_EQ(2, Parse("буквы    ещёЁ_99"));
    EXPECT_EQ(2, Count(TermID::TERM));
    EXPECT_STREQ("буквы", tokens[0]->getText().c_str());
    EXPECT_STREQ("ещёЁ_99", tokens[1]->getText().c_str());

    ASSERT_EQ(3, Parse("one two \t three"));
    EXPECT_EQ(3, Count(TermID::TERM));

    EXPECT_STREQ("one", tokens[0]->getText().c_str()) << tokens[0]->getText().c_str();
    EXPECT_STREQ("two", tokens[1]->getText().c_str()) << tokens[1]->getText().c_str();
    EXPECT_STREQ("three", tokens[2]->getText().c_str()) << tokens[2]->getText().c_str();
}

//TEST_F(Lexer, StrvarEmpty) {
//    ASSERT_EQ(0, Parse(""));
//    ASSERT_EQ(1, Parse("``"));
//    EXPECT_EQ(1, Count(TermID::STRVAR));
//    EXPECT_STREQ("", tokens[0]->getText().c_str()) << tokens[0]->getText();
//}
//
//TEST_F(Lexer, StrvarSimple) {
//    ASSERT_EQ(1, Parse("` `"));
//    EXPECT_EQ(1, Count(TermID::STRVAR));
//    EXPECT_STREQ(" ", tokens[0]->getText().c_str()) << tokens[0]->getText();
//}
//
//TEST_F(Lexer, FullStrvar) {
//    ASSERT_EQ(1, Parse("` ${123} \t \xFF\r\\\"`"));
//    EXPECT_EQ(1, Count(TermID::STRVAR));
//    EXPECT_STREQ(" ${123} \t \xFF\r\"", tokens[0]->getText().c_str()) << tokens[0]->getText();
//}

TEST_F(Lexer, Template) {
    ASSERT_EQ(1, Parse("\"\"\"\"\"\""));
    EXPECT_EQ(1, Count(TermID::TEMPLATE));
    EXPECT_STREQ("", tokens[0]->getText().c_str()) << tokens[0]->getText();
}

TEST_F(Lexer, DISABLED_Template2) {
    ASSERT_EQ(1, Parse("\"\"\" ${123} \n \"\"\""));
    EXPECT_EQ(1, Count(TermID::TEMPLATE));
    EXPECT_STREQ(" ${123} \n ", tokens[0]->getText().c_str()) << tokens[0]->getText();
}

TEST_F(Lexer, DISABLED_Template3) {
    ASSERT_EQ(1, Parse("''' ${123} \n\t '''"));
    EXPECT_EQ(1, Count(TermID::TEMPLATE));
    EXPECT_STREQ(" ${123} \n\t ", tokens[0]->getText().c_str()) << tokens[0]->getText();
}

TEST_F(Lexer, StringEmpty) {
    ASSERT_EQ(0, Parse(""));
    ASSERT_EQ(1, Parse("''"));
    EXPECT_EQ(1, Count(TermID::STRCHAR));
    EXPECT_STREQ("", tokens[0]->getText().c_str()) << tokens[0]->getText();
}

TEST_F(Lexer, StringEmpty2) {
    ASSERT_EQ(0, Parse(""));
    ASSERT_EQ(1, Parse("\"\""));
    EXPECT_EQ(1, Count(TermID::STRWIDE));
    EXPECT_STREQ("", tokens[0]->getText().c_str()) << tokens[0]->getText();
}

TEST_F(Lexer, StringSimple) {
    ASSERT_EQ(1, Parse("' '"));
    EXPECT_EQ(1, Count(TermID::STRCHAR));
    EXPECT_STREQ(" ", tokens[0]->getText().c_str()) << tokens[0]->getText();
}

TEST_F(Lexer, StringSimple2) {
    ASSERT_EQ(1, Parse("\" \""));
    EXPECT_EQ(1, Count(TermID::STRWIDE));
    EXPECT_STREQ(" ", tokens[0]->getText().c_str()) << tokens[0]->getText();
}

TEST_F(Lexer, FullString) {
    ASSERT_EQ(1, Parse("'  \t \xFF\r\\''"));
    EXPECT_EQ(1, Count(TermID::STRCHAR));
    EXPECT_STREQ("  \t \xFF\r'", tokens[0]->getText().c_str()) << tokens[0]->getText();
}

TEST_F(Lexer, FullString2) {
    ASSERT_EQ(1, Parse("\"  \t \xFF\r\\\"\""));
    EXPECT_EQ(1, Count(TermID::STRWIDE));
    EXPECT_STREQ("  \t \xFF\r\"", tokens[0]->getText().c_str()) << tokens[0]->getText();
}

TEST_F(Lexer, Integer) {
    ASSERT_EQ(1, Parse("123456"));
    EXPECT_EQ(1, Count(TermID::INTEGER)) << newlang::toString(tokens[0]->GetTokenID());

    EXPECT_STREQ("123456", tokens[0]->getText().c_str());

    ASSERT_EQ(3, Parse("123456++123"));
    EXPECT_EQ(1, Count(TermID::OPERATOR));
    EXPECT_EQ(2, Count(TermID::INTEGER));

    EXPECT_STREQ("123456", tokens[0]->getText().c_str()) << tokens[0]->getText();
    EXPECT_STREQ("++", tokens[1]->getText().c_str()) << tokens[1]->getText();
    EXPECT_STREQ("123", tokens[2]->getText().c_str()) << tokens[2]->getText();
}

TEST_F(Lexer, Number) {
    ASSERT_EQ(1, Parse("1.e10"));
    EXPECT_EQ(1, Count(TermID::NUMBER));
    EXPECT_STREQ("1.e10", tokens[0]->getText().c_str());
}

//TEST_F(Lexer, Complex0) {
//    if(1 != Parse("-1j-0.2")) {
//        for (auto elem : tokens) {
//            std::cout << newlang::toString(elem->m_id) << " " << elem->m_text << "\n";
//        }
//    }
//    ASSERT_EQ(1, tokens.size());
//    EXPECT_EQ(1, Count(TermID::COMPLEX)) << newlang::toString(tokens[0]->m_id);
//    EXPECT_STREQ("-1j-0.2", tokens[0]->getText().c_str());
//}
//
//TEST_F(Lexer, Complex1) {
//    if(1 != Parse("1.333+0.e10j")) {
//        for (auto elem : tokens) {
//            std::cout << newlang::toString(elem->m_id) << " " << elem->m_text << "\n";
//        }
//    }
//    ASSERT_EQ(1, tokens.size());
//    EXPECT_EQ(1, Count(TermID::COMPLEX));
//    EXPECT_STREQ("1.333+0.e10j", tokens[0]->getText().c_str());
//}

TEST_F(Lexer, Term) {

    if(1 != Parse("$alpha  ")) {
        for (auto elem : tokens) {
            std::cout << newlang::toString(elem->m_id) << " " << elem->m_text << "\n";
        }

    }
    ASSERT_EQ(1, tokens.size());
    EXPECT_EQ(1, Count(TermID::TERM));
    EXPECT_STREQ("$alpha", tokens[0]->getText().c_str());


    ASSERT_EQ(2, Parse("буквы    ещёЁ_99"));
    EXPECT_EQ(2, Count(TermID::TERM));
    EXPECT_STREQ("буквы", tokens[0]->getText().c_str());
    EXPECT_STREQ("ещёЁ_99", tokens[1]->getText().c_str());

    ASSERT_EQ(3, Parse("one @two \t $three"));
    EXPECT_EQ(3, Count(TermID::TERM));

    EXPECT_STREQ("one", tokens[0]->getText().c_str()) << tokens[0]->getText().c_str();
    EXPECT_STREQ("@two", tokens[1]->getText().c_str()) << tokens[1]->getText().c_str();
    EXPECT_STREQ("$three", tokens[2]->getText().c_str()) << tokens[2]->getText().c_str();
}

TEST_F(Lexer, AssignEq) {
    ASSERT_EQ(3, Parse("token=ssssssss"));
    EXPECT_EQ(2, Count(TermID::TERM));
    EXPECT_EQ(1, Count(TermID::SYMBOL));

    EXPECT_STREQ("token", tokens[0]->getText().c_str()) << tokens[0]->getText();
    EXPECT_STREQ("ssssssss", tokens[2]->getText().c_str()) << tokens[2]->getText();

    ASSERT_EQ(3, Parse("token:=\"ssssssss\""));
    EXPECT_EQ(1, Count(TermID::TERM));
    EXPECT_EQ(1, Count(TermID::CREATE_OR_ASSIGN));
    EXPECT_EQ(1, Count(TermID::STRWIDE));

    EXPECT_STREQ("token", tokens[0]->getText().c_str()) << tokens[0]->getText();
    EXPECT_STREQ("ssssssss", tokens[2]->getText().c_str()) << tokens[2]->getText();

    ASSERT_EQ(3, Parse("    token   \t  ::=   'ssssssss'       "));
    EXPECT_EQ(1, Count(TermID::TERM));
    EXPECT_EQ(1, Count(TermID::CREATE));
    EXPECT_EQ(1, Count(TermID::STRCHAR));

    EXPECT_STREQ("token", tokens[0]->getText().c_str()) << tokens[0]->getText();
    EXPECT_STREQ("ssssssss", tokens[2]->getText().c_str()) << tokens[2]->getText();
}

TEST_F(Lexer, CodeInner) {
    ASSERT_EQ(3, Parse("%{if(){%}   %{}else{%}   %{} %}"));
    EXPECT_EQ(3, Count(TermID::SOURCE));
    EXPECT_STREQ("if(){", tokens[0]->getText().c_str()) << tokens[0]->getText().c_str();
    EXPECT_STREQ("}else{", tokens[1]->getText().c_str()) << tokens[1]->getText().c_str();
    EXPECT_STREQ("} ", tokens[2]->getText().c_str()) << tokens[2]->getText().c_str();

    ASSERT_EQ(5, Parse("{ %{if(){%}   %{}else{%}   %{} %} }"));
    EXPECT_EQ(2, Count(TermID::SYMBOL));
    EXPECT_EQ(3, Count(TermID::SOURCE));
    EXPECT_STREQ("{", tokens[0]->getText().c_str()) << tokens[0]->getText().c_str();
    EXPECT_STREQ("if(){", tokens[1]->getText().c_str()) << tokens[1]->getText().c_str();
    EXPECT_STREQ("}else{", tokens[2]->getText().c_str()) << tokens[2]->getText().c_str();
    EXPECT_STREQ("} ", tokens[3]->getText().c_str()) << tokens[3]->getText().c_str();
    EXPECT_STREQ("}", tokens[4]->getText().c_str()) << tokens[4]->getText().c_str();
}

TEST_F(Lexer, Code) {
    ASSERT_EQ(2, Parse("{            }"));
    EXPECT_EQ(2, Count(TermID::SYMBOL));
    EXPECT_STREQ("{", tokens[0]->getText().c_str()) << tokens[0]->getText().c_str();
    EXPECT_STREQ("}", tokens[1]->getText().c_str()) << tokens[1]->getText().c_str();

    ASSERT_EQ(4, Parse("{ { } }"));
    EXPECT_EQ(4, Count(TermID::SYMBOL));
}

TEST_F(Lexer, CodeSource) {
    ASSERT_EQ(1, Parse("%{%}"));
    EXPECT_EQ(1, Count(TermID::SOURCE));
    EXPECT_STREQ("", tokens[0]->getText().c_str()) << tokens[0]->getText().c_str();

    ASSERT_EQ(1, Parse("%{ % %}"));
    ASSERT_EQ(1, Count(TermID::SOURCE));
    ASSERT_STREQ(" % ", tokens[0]->getText().c_str()) << tokens[0]->getText().c_str();

    ASSERT_EQ(1, Parse("%{ % }%"));
    ASSERT_EQ(1, Count(TermID::SOURCE));
    ASSERT_STREQ(" % ", tokens[0]->getText().c_str()) << tokens[0]->getText().c_str();
}

TEST_F(Lexer, Assign) {
    ASSERT_EQ(5, Parse(":= :- :&&= :||= :^^="));
    EXPECT_EQ(1, Count(TermID::CREATE_OR_ASSIGN));
    EXPECT_EQ(1, Count(TermID::PUREFUNC));
    EXPECT_EQ(1, Count(TermID::SIMPLE_AND));
    EXPECT_EQ(1, Count(TermID::SIMPLE_OR));
    EXPECT_EQ(1, Count(TermID::SIMPLE_XOR));
}

TEST_F(Lexer, Function) {
    ASSERT_EQ(1, Parse("@"));
    EXPECT_EQ(1, Count(TermID::TERM)) << toString(tokens[0]->getTermID());
    EXPECT_STREQ("@", tokens[0]->getText().c_str()) << tokens[0]->getText().c_str();

    ASSERT_EQ(1, Parse("$"));
    EXPECT_EQ(1, Count(TermID::TERM)) << toString(tokens[0]->getTermID());
    EXPECT_STREQ("$", tokens[0]->getText().c_str()) << tokens[0]->getText().c_str();

    ASSERT_EQ(1, Parse("@name"));
    EXPECT_EQ(1, Count(TermID::TERM));
    EXPECT_STREQ("@name", tokens[0]->getText().c_str()) << tokens[0]->getText().c_str();

    ASSERT_EQ(1, Parse("@функция_alpha_ёЁ"));
    EXPECT_EQ(1, Count(TermID::TERM));
    EXPECT_STREQ("@функция_alpha_ёЁ", tokens[0]->getText().c_str()) << tokens[0]->getText().c_str();
}

TEST_F(Lexer, Sentence) {
    ASSERT_EQ(2, Parse("token."));
    EXPECT_EQ(1, Count(TermID::TERM));
    ASSERT_EQ(2, Parse("token;"));
    EXPECT_EQ(1, Count(TermID::TERM));
}

TEST_F(Lexer, Comment) {
    ASSERT_EQ(0, Parse("# lskdafj ldsjf ldkjfa l;sdj fl;k"));
    //    EXPECT_EQ(1, Count(TermID::COMMENT));
    //    EXPECT_STREQ(" lskdafj ldsjf ldkjfa l;sdj fl;k", tokens[0]->getText().c_str()) << tokens[0]->getText().c_str();

    ASSERT_EQ(0, Parse("#! lskdafj ldsjf ldkjfa l;sdj fl;k\n          "));
    //    EXPECT_EQ(1, Count(TermID::COMMENT));
    //    EXPECT_STREQ("! lskdafj ldsjf ldkjfa l;sdj fl;k", tokens[0]->getText().c_str()) << tokens[0]->getText().c_str();

    ASSERT_EQ(0, Parse("/***** lskdafj\n\n\n\n ldsjf ldkjfa l;sdj fl;k*****/    "));
    //    EXPECT_EQ(1, Count(TermID::COMMENT));
}

TEST_F(Lexer, Comment2) {
    ASSERT_EQ(2, Parse("#!22\n#1\nterm;\n"));
    //    EXPECT_EQ(2, Count(TermID::COMMENT));
    EXPECT_EQ(1, Count(TermID::SYMBOL));
    EXPECT_EQ(1, Count(TermID::TERM));

    ASSERT_EQ(1, Parse("\n\n/* \n\n*/\n\n  term"));
    //    EXPECT_EQ(1, Count(TermID::COMMENT));
    EXPECT_EQ(1, Count(TermID::TERM));
    //    EXPECT_STREQ(" \n\n", tokens[0]->getText().c_str()) << tokens[0]->getText().c_str();
    //    EXPECT_EQ(5, tokens[0]->m_line);
    //    EXPECT_EQ(1, tokens[0]->m_col);
    EXPECT_STREQ("term", tokens[0]->getText().c_str()) << tokens[1]->getText().c_str();
    EXPECT_EQ(7, tokens[0]->m_line);
    EXPECT_EQ(3, tokens[0]->m_col);
}

TEST_F(Lexer, Paren) {
    ASSERT_EQ(3, Parse("@name()"));
    EXPECT_EQ(1, Count(TermID::TERM));
    EXPECT_EQ(2, Count(TermID::SYMBOL));


    ASSERT_EQ(3, Parse("@функция_alpha_ёЁ ()"));
    EXPECT_EQ(1, Count(TermID::TERM));
    EXPECT_EQ(2, Count(TermID::SYMBOL));

}

TEST_F(Lexer, Arg) {
    ASSERT_EQ(7, Parse("term(name=value);"));
    EXPECT_EQ(3, Count(TermID::TERM));
    EXPECT_EQ(4, Count(TermID::SYMBOL));
}

TEST_F(Lexer, Args) {
    ASSERT_EQ(9, Parse("$0 $1 $22 $333 $4sss $sss1 -- $*   ")) << Dump();
    EXPECT_EQ(5, Count(TermID::ARGUMENT));
    EXPECT_EQ(1, Count(TermID::ARGS));
    EXPECT_EQ(1, Count(TermID::EXIT));
    EXPECT_EQ(2, Count(TermID::TERM));
}

TEST_F(Lexer, UTF8) {
    ASSERT_EQ(7, Parse("термин(имя=значение);"));
    EXPECT_EQ(3, Count(TermID::TERM));
    EXPECT_EQ(4, Count(TermID::SYMBOL));
}

TEST_F(Lexer, ELLIPSIS) {
    ASSERT_EQ(2, Parse("... ..."));
    EXPECT_EQ(2, Count(TermID::ELLIPSIS));
}

TEST_F(Lexer, MACRO) {
    ASSERT_EQ(1, Parse("\\\\123 ... 456\\\\\\"));
    EXPECT_EQ(1, Count(TermID::MACRO_BODY));

    ASSERT_EQ(2, Parse("\\macro       \\\\123 ... 456\\\\\\"));
    EXPECT_EQ(1, Count(TermID::MACRO));
    EXPECT_EQ(1, Count(TermID::MACRO_BODY));
    EXPECT_STREQ("\\macro", tokens[0]->m_text.c_str());
    EXPECT_STREQ("123 ... 456", tokens[1]->m_text.c_str());
    EXPECT_EQ(1, tokens[0]->m_line);
    EXPECT_EQ(1, tokens[0]->m_col);
    EXPECT_EQ(1, tokens[1]->m_line);
    EXPECT_EQ(14, tokens[1]->m_col);

    ASSERT_EQ(5, Parse(" \\macro (name)      \\\\123 \n \n ... 456\\\\\\ # Комментарий"));
    EXPECT_EQ(1, Count(TermID::MACRO));
    EXPECT_EQ(1, Count(TermID::MACRO_BODY));
    EXPECT_STREQ("\\macro", tokens[0]->m_text.c_str());
    EXPECT_STREQ("123 \n \n ... 456", tokens[4]->m_text.c_str());
    EXPECT_EQ(1, tokens[0]->m_line);
    EXPECT_EQ(2, tokens[0]->m_col);
    EXPECT_EQ(3, tokens[4]->m_line);
    EXPECT_EQ(1, tokens[4]->m_col);
}

#endif // UNITTEST