#include "pch.h"

#include "parser.h"
#include "lexer.h"
#include "builtin.h"

#include <term.h>

using namespace newlang;

Diag::Diag() {

    /*
        `llvm-config-16 --cxxflags` -std=c++17 -fexceptions -fcxx-exceptions 
     * -Wall 
     * -Wextra 
     * -Werror 
     * 
     * -Wfloat-equal 
     * -Wundef 
     * -Wcast-align 
     * -Wwrite-strings 
     * -Wmissing-declarations 
     * -Wredundant-decls 
     * -Wshadow 
     * -Woverloaded-virtual 
     * -Wno-trigraphs 
     * -Wno-invalid-source-encoding 
     * -Wno-error=unused-variable 
     * -Wno-error=unused-parameter 
     * -Wno-error=switch 
     * 
     * -fsanitize=undefined-trap 
     * -fsanitize-undefined-trap-on-error             
     * 
     * -gdwarf-4                            
     * 
     * -Wno-undefined-var-template 
     * -Wno-switch  
     * -fvisibility=default  
     * -ggdb -O0
 
     */

    m_diag_stack.push_back({});

    VERIFY(Register("-clang-Wall", State::flag));
    VERIFY(Register("-clang-Wextra", State::flag));
    VERIFY(Register("-clang-Werror", State::flag));

    VERIFY(Register("-clang-fvisibility=default", State::flag)); // -fvisibility=default

    VERIFY(Register("-clang-Wtrigraphs", State::ignored)); // -Wno-trigraphs
    VERIFY(Register("-clang-Winvalid-source-encoding", State::ignored)); // -Wno-invalid-source-encoding

    VERIFY(Register("-clang-Wfloat-equal", State::warning));
    VERIFY(Register("-clang-Wundef", State::warning));
    VERIFY(Register("-clang-Wcast-align", State::warning));
    VERIFY(Register("-clang-Wwrite-strings", State::warning));
    VERIFY(Register("-clang-Wmissing-declarations", State::warning));
    VERIFY(Register("-clang-Wredundant-decls", State::warning));
    VERIFY(Register("-clang-Wshadow", State::warning));
    VERIFY(Register("-clang-Woverloaded-virtual", State::warning));


    VERIFY(Register("-clang-fsanitize=undefined", State::flag));
    VERIFY(Register("-clang-fsanitize-trap=undefined", State::flag));


    VERIFY(Register("-clang-Wunused-variable", State::warning)); // -Wno-error=unused-variable
    VERIFY(Register("-clang-Wunused-parameter", State::warning)); // -Wno-error=unused-parameter
    //    VERIFY(Register("-clang-Wswitch", State::warning)); // -Wno-error=switch

    VERIFY(Register("-clang-Wundefined-var-template", State::ignored)); // -Wno-undefined-var-template
    VERIFY(Register("-clang-Wswitch", State::ignored)); // -Wno-switch

    //    VERIFY(Register("-clang-gdwarf-4", State::flag));
    //    VERIFY(Register("-clang-ggdb", State::flag));
    //    VERIFY(Register("-clang-O0", State::flag));

    VERIFY(Register(DIAG_MACRO_NOT_FOUND, State::error, "Macro not found!"));
    VERIFY(Register(DIAG_MACRO_STORAGE_NOT_EXIST, State::error, "Strorage for macro not exists!"));
    VERIFY(Register(DIAG_FILL_REMAINDER, State::error, "The data is filled in with the remainder!"));



    VERIFY(Register(DIAG_ERROR_LIMIT, State::flag));
    VERIFY(Register(DIAG_EXTRA_TOKENS, State::option));

    m_error_limit = 10;
    m_error_count = 0;
    m_fill_remainder = true;
}

bool Diag::Register(const char *name, State state, const char * desc) {
    ASSERT(name);
    ASSERT(state != State::none);
    ASSERT(!m_diag_stack.empty());
    if (Test(name) == State::none) {
        m_diag_stack[0].push_back({name, state, desc});
        return true;
    }
    return false;
}

std::string Diag::ChangeState(const std::string name, State from_state, State to_state) {

    std::string result;
    if (from_state == State::flag) {
        return name;
    } else if (from_state == State::option) {
        if (to_state == State::ignored) {
            result = "-Wno-";
            result += RemoveDiagPrefix(name);
        } else {
            result = name;
        }
    } else { // for diags only
        result = RemoveDiagPrefix(name);
        if (to_state == State::ignored) {
            result = result.insert(0, "-Wno-");
        } else if (to_state == State::warning) {
            result = result.insert(0, "-Wno-error=");
        } else if (to_state == State::error) {
            result = result.insert(0, "-Werror=");
        } else {
            result = result.insert(0, "-W");
        }
    }
    return result;
}

void Diag::Push(const TermPtr term) {
    ASSERT(!m_diag_stack.empty());
    m_diag_stack.push_back(m_diag_stack[m_diag_stack.size() - 1]);
}

void Diag::Pop(const TermPtr term) {
    if (m_diag_stack.empty()) {
        if (term) {
            NL_PARSER(term, "Empty stack diags at '%s'!", term->toString().c_str());
        } else {
            LOG_RUNTIME("Empty stack diags!");
        }
    }
    m_diag_stack.pop_back();
}

bool Diag::Apply(const char *name, State state, const TermPtr term) {
    if (term) {
        NL_PARSER(term, "Empty stack diags at '%s'!", term->toString().c_str());
    } else {
        LOG_RUNTIME("Empty stack diags!");
    }
    return false;
}

Diag::State Diag::Test(const char *name) {
    ASSERT(m_diag_stack.size());
    size_t top = m_diag_stack.size() - 1;
    for (int pos = 0; pos < m_diag_stack[top].size(); pos++) {
        if (m_diag_stack[top][pos].Name.compare(name) == 0) {
            return m_diag_stack[top][pos].State;
        }
    }
    //    LOG_RUNTIME("Diag name '%s' not found!", name);
    return State::none;
}

bool Diag::TestIgnore(const char *name) {
    State st = Test(name);
    return st == State::none || st == State::ignored;
}

bool Diag::Emit(const char *name, const TermPtr term) {
    switch (Test(name)) {
        case Diag::State::error:
            if (term) {
                NL_PARSER(term, "Emit %s near at '%s'!", name, term->toString().c_str());
            } else {
                LOG_RUNTIME("Emit %s!", name);
            }
        case Diag::State::warning:
            if (term) {
                NL_MESSAGE(LOG_LEVEL_WARNING, term, "Emit %s near at '%s'!", name, term->toString().c_str());
            } else {
                LOG_WARNING("Emit %s!", name);
            }
        case Diag::State::ignored:
        case Diag::State::none:
            return false;
    }
    return true;
}


//void Diag::Diag(const TermPtr &term, DiagName id) {
//
//    const char *message = DiagNameString(id);
//    if(!message) {
//        LOG_RUNTIME("DiagName '%d' not found!", static_cast<int> (id));
//    }
//    ASSERT(!m_diags.empty());
//
//    DiagMapType::iterator iter = m_diags[m_diags.size() - 1].find();
//
//    const char *message = DiagNameString(id);
//    if(!message) {
//        LOG_RUNTIME("DiagName '%d' not found!", static_cast<int> (id));
//    }
//
//    std::string empty;
//    std::string str = newlang::ParserMessage(term->m_source ? *term->m_source : empty, term->m_line, term->m_col, "%s", message);
//
//    Logger::LogLevelType save = Logger::Instance()->GetLogLevel();
//    Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
//    if() {
//    }
//    LOG_INFO("%s", str.c_str());
//    Logger::Instance()->SetLogLevel(save);
//}

