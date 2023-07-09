#include "pch.h"

#ifdef UNITTEST

#include <warning_push.h>
#include <gtest/gtest.h>
#include <warning_pop.h>

#include <term.h>

using namespace newlang;

TEST(Diag, Init) {

}


TEST(Diag, State) {
    EXPECT_STREQ("-Wfoo", Diag::ChangeState("-Wfoo", Diag::State::flag, Diag::State::option).c_str());
    EXPECT_STREQ("-Wfoo", Diag::ChangeState("-Wfoo", Diag::State::flag, Diag::State::ignored).c_str());
    EXPECT_STREQ("-Wfoo", Diag::ChangeState("-Wfoo", Diag::State::flag, Diag::State::error).c_str());
    EXPECT_STREQ("-Wfoo", Diag::ChangeState("-Wfoo", Diag::State::flag, Diag::State::warning).c_str());

    EXPECT_STREQ("-Wfoo", Diag::ChangeState("-Wfoo", Diag::State::option, Diag::State::option).c_str());
    EXPECT_STREQ("-Wno-foo", Diag::ChangeState("-Wfoo", Diag::State::option, Diag::State::ignored).c_str());
    EXPECT_STREQ("-Wfoo", Diag::ChangeState("-Wfoo", Diag::State::option, Diag::State::error).c_str());
    EXPECT_STREQ("-Wfoo", Diag::ChangeState("-Wfoo", Diag::State::option, Diag::State::warning).c_str());

    EXPECT_STREQ("-Wfoo", Diag::ChangeState("-Wfoo", Diag::State::ignored, Diag::State::option).c_str());
    EXPECT_STREQ("-Wno-foo", Diag::ChangeState("-Wfoo", Diag::State::ignored, Diag::State::ignored).c_str());
    EXPECT_STREQ("-Werror=foo", Diag::ChangeState("-Wfoo", Diag::State::ignored, Diag::State::error).c_str());
    EXPECT_STREQ("-Wno-error=foo", Diag::ChangeState("-Wfoo", Diag::State::ignored, Diag::State::warning).c_str());

    EXPECT_STREQ("-Wfoo", Diag::ChangeState("-Wfoo", Diag::State::warning, Diag::State::option).c_str());
    EXPECT_STREQ("-Wno-foo", Diag::ChangeState("-Wfoo", Diag::State::warning, Diag::State::ignored).c_str());
    EXPECT_STREQ("-Werror=foo", Diag::ChangeState("-Wfoo", Diag::State::warning, Diag::State::error).c_str());
    EXPECT_STREQ("-Wno-error=foo", Diag::ChangeState("-Wfoo", Diag::State::warning, Diag::State::warning).c_str());

    EXPECT_STREQ("-Wfoo", Diag::ChangeState("-Wfoo", Diag::State::error, Diag::State::option).c_str());
    EXPECT_STREQ("-Wno-foo", Diag::ChangeState("-Wfoo", Diag::State::error, Diag::State::ignored).c_str());
    EXPECT_STREQ("-Werror=foo", Diag::ChangeState("-Wfoo", Diag::State::error, Diag::State::error).c_str());
    EXPECT_STREQ("-Wno-error=foo", Diag::ChangeState("-Wfoo", Diag::State::error, Diag::State::warning).c_str());
}


#endif // UNITTEST