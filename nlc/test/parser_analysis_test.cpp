#ifdef BUILD_UNITTEST

#include "warning_push.h"
#include <gtest/gtest.h>
#include "warning_pop.h"



#include "parser.h"
#include "term.h"
#include "version.h"
#include "runtime.h"
#include "analysis.h"

using namespace newlang;

class ParserAnalysis : public ::testing::Test {
protected:



    Parser *m_parser;
    std::vector<std::string> m_postlex;

    Logger::FuncCallback *m_log_callback_save;
    void *m_log_callback_arg_save;
    std::string m_output;

    static void LoggerCallback(void *param, Logger::LogLevelType level, const char * str, bool flush) {
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
        Logger::Instance()->SaveCallback(m_log_callback_save, m_log_callback_arg_save);
        Logger::Instance()->SetCallback(&LoggerCallback, this);
    }

    void TearDown() {
        delete m_parser;
        m_parser = nullptr;
        Logger::Instance()->SetCallback(m_log_callback_save, m_log_callback_arg_save);
    }

    TermPtr Parse(std::string str, MacroPtr buffer = nullptr, DiagPtr diag = nullptr, RuntimePtr rt = nullptr) {
        if (m_parser) {
            delete m_parser;
        }

        m_parser = new newlang::Parser(buffer, &m_postlex, diag, true, rt);

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

//TEST_F(ParserAnalysis, Declare) {
//
//    ASSERT_ANY_THROW(Parse("@__PRAGMA_DECLARE__(printf(format:FmtChar, ...):Int32, FFI_DEFAULT_ABI)"));
//
//    ASSERT_NO_THROW(Parse("@__PRAGMA_DECLARE__(printf(format:FmtChar, ...))"));
//    ASSERT_NO_THROW(Parse("@__PRAGMA_DECLARE__(printf(format, ...):Int32)"));
//
//    ASSERT_NO_THROW(Parse("@__PRAGMA_DECLARE__(printf(format:FmtChar, ...):Int32)  @__PRAGMA_DECLARE__(printf2(format:FmtChar, ...):Int32)"));
//
//    ASSERT_TRUE(m_parser);
//    ASSERT_EQ(2, m_parser->m_declare.size());
//    std::cout << m_parser->m_declare.begin()->second->toString() << "\n";
//
//    TermPtr obj = m_parser->m_declare.begin()->second;
//    ASSERT_STREQ("printf", obj->m_text.c_str());
//    ASSERT_TRUE(obj->m_type);
//    ASSERT_TRUE(obj->isCall());
//    ASSERT_STREQ(":Int32", obj->m_type->m_text.c_str());
//    ASSERT_EQ(2, obj->size());
//    ASSERT_STREQ("format", obj->at(0).second->m_text.c_str());
//    ASSERT_STREQ("...", obj->at(1).second->m_text.c_str());
//
//
//    ASSERT_ANY_THROW(Parse("@__PRAGMA_DECLARE__(printf(format:FmtChar='', ...):Int32)"));
//
//
//    ASSERT_NO_THROW(Parse("@__PRAGMA_DECLARE__(variable:Double)"));
//
//    ASSERT_TRUE(m_parser);
//    ASSERT_EQ(1, m_parser->m_declare.size());
//    std::cout << m_parser->m_declare.begin()->second->toString() << "\n";
//
//    obj = m_parser->m_declare.begin()->second;
//    ASSERT_STREQ("variable", obj->m_text.c_str());
//    ASSERT_FALSE(obj->isCall());
//    ASSERT_EQ(0, obj->size());
//    ASSERT_TRUE(obj->m_type);
//    ASSERT_STREQ(":Double", obj->m_type->m_text.c_str());
//
//
//    ASSERT_NO_THROW(Parse("@__PRAGMA_DECLARE__(variable)"));
//
//}


TEST_F(ParserAnalysis, ErrorLimit1) {

    RuntimePtr rt_default = RunTime::Init();
    AstAnalysis analysis(*rt_default, rt_default->m_diag.get());

    ASSERT_EQ(10, rt_default->m_diag->m_error_limit);

    TermPtr term;

    ASSERT_NO_THROW(term = Parse("1\\1 + 1:Int8; 1\\1 + 1:Int8; 1\\1 + 1:Int8", nullptr, nullptr, rt_default));
    ASSERT_TRUE(analysis.Analyze(term, term));

    ASSERT_TRUE(m_output.find("fatal error") == std::string::npos) << m_output;
}

TEST_F(ParserAnalysis, ErrorLimit2) {

    RuntimePtr rt_default = RunTime::Init();
    AstAnalysis analysis(*rt_default, rt_default->m_diag.get());

    ASSERT_EQ(10, rt_default->m_diag->m_error_limit);

    TermPtr term;

    ASSERT_NO_THROW(term = Parse("1:Int8 + 1\\1; 1:Int8 + 1\\1; 1:Int8 + 1\\1", nullptr, nullptr, rt_default));
    ASSERT_FALSE(analysis.Analyze(term, term));

    ASSERT_TRUE(m_output.find("fatal error: 3 generated") != std::string::npos) << m_output;

}

TEST_F(ParserAnalysis, ErrorLimit3) {

    TermPtr term;

    RuntimePtr rt = RunTime::Init({"--nlc-error-limit=1"});
    AstAnalysis analysis(*rt, rt->m_diag.get());

    ASSERT_EQ(1, rt->m_diag->m_error_limit);

    ASSERT_NO_THROW(term = Parse("1\\1 + 1:Int8; 1\\1 + 1:Int8; 1\\1 + 1:Int8", nullptr, nullptr, rt));
    ASSERT_TRUE(analysis.Analyze(term, term));

    ASSERT_TRUE(m_output.find("fatal error") == std::string::npos) << m_output;
}

TEST_F(ParserAnalysis, ErrorLimit4) {

    TermPtr term;

    RuntimePtr rt = RunTime::Init({"--nlc-error-limit=1"});
    AstAnalysis analysis(*rt, rt->m_diag.get());

    ASSERT_EQ(1, rt->m_diag->m_error_limit);

    ASSERT_NO_THROW(term = Parse("1:Int8 + 1\\1; 1:Int8 + 1\\1; 1:Int8 + 1\\1", nullptr, nullptr, rt));
    ASSERT_FALSE(analysis.Analyze(term, term));
    ASSERT_TRUE(m_output.find("fatal error: too many errors emitted 1, stopping now [-nlc-error-limit=]") != std::string::npos) << m_output;
}


#endif // UNITTEST