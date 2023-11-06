#include "pch.h"

#ifdef UNITTEST

#include <warning_push.h>
#include <gtest/gtest.h>
#include <warning_pop.h>

#include <parser.h>
#include <term.h>
#include "version.h"
#include "runtime.h"
#include "nlc.h"

using namespace newlang;

class ParserAnalysis : public ::testing::Test {
protected:



    Parser *m_parser;
    std::vector<std::string> m_postlex;

    utils::Logger::FuncCallback *m_log_callback_save;
    void *m_log_callback_arg_save;
    std::string m_output;

    static void LoggerCallback(void *param, utils::Logger::LogLevelType level, const char * str, bool flush) {
        ParserAnalysis *p = static_cast<ParserAnalysis *> (param);
        fprintf(stdout, "%s", str);
        if (flush) {
            fflush(stdout);
        }
        if (p) {
            p->m_output += str;
        }
    }

    void SetUp() {
        m_parser = nullptr;
        utils::Logger::Instance()->SaveCallback(m_log_callback_save, m_log_callback_arg_save);
        utils::Logger::Instance()->Clear();
        utils::Logger::Instance()->SetCallback(&LoggerCallback, this);
    }

    void TearDown() {
        delete m_parser;
        m_parser = nullptr;
        utils::Logger::Instance()->SetCallback(m_log_callback_save, m_log_callback_arg_save);
    }

    TermPtr Parse(std::string str, MacroPtr buffer = nullptr, DiagPtr diag = nullptr) {
        if (m_parser) {
            delete m_parser;
        }

        m_parser = new newlang::Parser(buffer, &m_postlex, diag);

        m_postlex.clear();
        ast = m_parser->Parse(str);
        return ast;
    }

    int Count(TermID token_id) {
        int result = 0;
        for (int c = 0; c < ast->size(); c++) {
            if ((*ast)[c].second->m_id == token_id) {
                result++;
            }
        }
        return result;
    }

    std::string LexOut() {
        std::string result;
        for (int i = 0; i < m_postlex.size(); i++) {
            if (!result.empty()) {
                result += " ";
            }
            result += m_postlex[i];
        }
        trim(result);
        return result;
    }

    TermPtr ast;
};

/*
 * 
 *         std::vector<std::string> m_ns_stack;
        std::map<std::string, TermPtr> m_prototypes;

 */

/*
 * // ffi_abi  
 * // int printf(char * format, ...)
 * @native printf(format:FmtChar, ...): Int32;
 * @native printf(format:FmtChar, ...): Int32, FFI_DEFAULT_ABI;
 * @__PRAGMA_NATIVE__( printf(format:FmtChar, ...): Int32, FFI_DEFAULT_ABI)
 * 
 * @proto 
 * 
 */

TEST_F(ParserAnalysis, Native) {

    ASSERT_ANY_THROW(Parse("@__PRAGMA_NATIVE__(printf(format:FmtChar, ...))"));
    ASSERT_ANY_THROW(Parse("@__PRAGMA_NATIVE__(printf(format:FmtChar, ...):Int32, FFI_DEFAULT_ABI)"));
    ASSERT_ANY_THROW(Parse("@__PRAGMA_NATIVE__(printf(format, ...):Int32)"));
    
    ASSERT_NO_THROW(Parse("@__PRAGMA_NATIVE__(printf(format:FmtChar, ...):Int32)"));

    ASSERT_TRUE(m_parser);
    ASSERT_EQ(1, m_parser->m_native.size());
    std::cout << m_parser->m_native.begin()->second->toString() << "\n";

    TermPtr obj = m_parser->m_native.begin()->second;
    ASSERT_STREQ("printf", obj->m_text.c_str());
    ASSERT_TRUE(obj->isCall());
    ASSERT_STREQ(":Int32", obj->m_type_name.c_str());
    ASSERT_EQ(2, obj->size());
    ASSERT_STREQ("format", obj->at(0).second->m_text.c_str());
    ASSERT_STREQ("...", obj->at(1).second->m_text.c_str());


    ASSERT_ANY_THROW(Parse("@__PRAGMA_NATIVE__(printf(format:FmtChar='', ...):Int32)"));


    ASSERT_NO_THROW(Parse("@__PRAGMA_NATIVE__(variable:Double)"));

    ASSERT_TRUE(m_parser);
    ASSERT_EQ(1, m_parser->m_native.size());
    std::cout << m_parser->m_native.begin()->second->toString() << "\n";

    obj = m_parser->m_native.begin()->second;
    ASSERT_STREQ("variable", obj->m_text.c_str());
    ASSERT_FALSE(obj->isCall());
    ASSERT_EQ(0, obj->size());
    ASSERT_STREQ(":Double", obj->m_type_name.c_str());


    ASSERT_ANY_THROW(Parse("@__PRAGMA_NATIVE__(variable)"));

}


TEST_F(ParserAnalysis, Declare) {

    ASSERT_ANY_THROW(Parse("@__PRAGMA_DECLARE__(printf(format:FmtChar, ...):Int32, FFI_DEFAULT_ABI)"));

    ASSERT_NO_THROW(Parse("@__PRAGMA_DECLARE__(printf(format:FmtChar, ...))"));
    ASSERT_NO_THROW(Parse("@__PRAGMA_DECLARE__(printf(format, ...):Int32)"));
    
    ASSERT_NO_THROW(Parse("@__PRAGMA_DECLARE__(printf(format:FmtChar, ...):Int32)  @__PRAGMA_DECLARE__(printf2(format:FmtChar, ...):Int32)"));

    ASSERT_TRUE(m_parser);
    ASSERT_EQ(2, m_parser->m_declare.size());
    std::cout << m_parser->m_declare.begin()->second->toString() << "\n";

    TermPtr obj = m_parser->m_declare.begin()->second;
    ASSERT_STREQ("printf", obj->m_text.c_str());
    ASSERT_TRUE(obj->isCall());
    ASSERT_STREQ(":Int32", obj->m_type_name.c_str());
    ASSERT_EQ(2, obj->size());
    ASSERT_STREQ("format", obj->at(0).second->m_text.c_str());
    ASSERT_STREQ("...", obj->at(1).second->m_text.c_str());


    ASSERT_ANY_THROW(Parse("@__PRAGMA_DECLARE__(printf(format:FmtChar='', ...):Int32)"));


    ASSERT_NO_THROW(Parse("@__PRAGMA_DECLARE__(variable:Double)"));

    ASSERT_TRUE(m_parser);
    ASSERT_EQ(1, m_parser->m_declare.size());
    std::cout << m_parser->m_declare.begin()->second->toString() << "\n";

    obj = m_parser->m_declare.begin()->second;
    ASSERT_STREQ("variable", obj->m_text.c_str());
    ASSERT_FALSE(obj->isCall());
    ASSERT_EQ(0, obj->size());
    ASSERT_STREQ(":Double", obj->m_type_name.c_str());


    ASSERT_NO_THROW(Parse("@__PRAGMA_DECLARE__(variable)"));

}


TEST_F(ParserAnalysis, CheckArg) {

    TermPtr term = Term::Create(parser::token_type::NAME, TermID::NAME, "%printf");
    ASSERT_TRUE(term);


    ASSERT_TRUE(LLVMLoadLibraryPermanently(nullptr) == 0);

    TermPtr proto = Term::Create(parser::token_type::NAME, TermID::NAME, "%printf");
    ASSERT_ANY_THROW(Term::CheckArgsProto(term, proto));

    proto->Append(Term::Create(parser::token_type::ELLIPSIS, TermID::ELLIPSIS, "..."), Term::RIGHT);
    ASSERT_TRUE(Term::CheckArgsProto(term, proto));

    ASSERT_ANY_THROW(Term::CheckArgsProto(term, Term::Create(parser::token_type::NAME, TermID::NAME, "%printf")));
    ASSERT_ANY_THROW(Term::CheckArgsProto(term, Term::Create(parser::token_type::NAME, TermID::NAME, "%printfsssssssssssssssssss")));
    ASSERT_ANY_THROW(Term::CheckArgsProto(term, Term::Create(parser::token_type::NAME, TermID::NAME, "%printfsssssssssssssssssss...")));
    ASSERT_ANY_THROW(Term::CheckArgsProto(term, Term::Create(parser::token_type::NAME, TermID::NAME, "%printfsssssssssssssssssss...")));

}


#endif // UNITTEST