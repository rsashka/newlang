#include "pch.h"

#ifdef UNITTEST

#include <warning_push.h>
#include <gtest/gtest.h>
#include <warning_pop.h>

#include <newlang.h>
#include <chrono>
#include <stdio.h> 

using namespace newlang;

char convert(char c) {
    if(c == 'A') return 'C';
    if(c == 'C') return 'G';
    if(c == 'G') return 'T';
    if(c == 'T') return 'A';
    return ' ';
}

TEST(Speed, CPP) {

    LOG_INFO("Start speed test C++");

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    std::string opt = "ACGT";
    std::string s = "";
    std::string s_last = "";
    int len_str = 13;
    bool change_next;

    for (int i = 0; i < len_str; i++) {
        s += opt[0];
    }

    for (int i = 0; i < len_str; i++) {
        s_last += opt.back();
    }

    int pos = 0;
    int counter = 1;
    while(s != s_last) {
        counter++;
        // You can uncomment the next line to see all k-mers.
        //std::cout << s << std::endl;
        change_next = true;
        for (int i = 0; i < len_str; i++) {
            if(change_next) {
                if(s[i] == opt.back()) {
                    s[i] = convert(s[i]);
                    change_next = true;
                } else {
                    s[i] = convert(s[i]);
                    i = len_str; // break;
                }
            }
        }
    }

    // You can uncomment the next line to see all k-mers.
    // cout << s << endl;
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    int sec = std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
    int ms = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() % 1000000;
    LOG_INFO("Number of generated k-mers: %d  at %d.%d sec", counter, sec, ms);

}

TEST(Speed, NewLang) {

    Context::Reset();
    Context ctx(RunTime::Init());

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    setvbuf(stdin, nullptr, _IONBF, 0);
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);

    //    std::string src;
    //    src = "@printf := :Pointer('printf(format:FmtChar, ...):Int'); @counter := 1; printf('%d',counter)";
    //    ObjPtr test = ctx.ExecStr(src.c_str(), nullptr, true);
    //    ASSERT_TRUE(test);
    //    ASSERT_STREQ("0", test->GetValueAsString().c_str());


    LLVMAddSymbol("convert", (void *) &convert);
    //    src = "@test_convert := :Pointer('convert(sym:Char):Char'); test_convert('A')";
    //    ObjPtr test = ctx.ExecStr(src.c_str(), nullptr, true);
    //    ASSERT_TRUE(test);
    //    ASSERT_STREQ("67", test->GetValueAsString().c_str());

    utils::Logger::LogLevelType save = utils::Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
    ObjPtr result = ctx.ExecFile("../examples/speed_test.nlp");
    utils::Logger::Instance()->SetLogLevel(save);

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();


    ASSERT_TRUE(result);
    ASSERT_TRUE(result->is_string_type()) << result->toString();
    ASSERT_STREQ("OK", result->GetValueAsString().c_str());


    int sec = std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
    int ms = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() % 1000000;
    LOG_INFO("Test complete at %d.%d sec", sec, ms);

    /*
     * 
     * Start speed test C++
     * Number of generated k-mers: 67108864  at 2.502049 sec
     * 
     * Start
     * Number of generated k-mers: 67108864
     * real	0m33,794s     
     * 
     * Start speed test NewLang
     * Number of generated k-mers: 67108864
     * Test complete at  ????????????????????
     * 
     * Своя функция @convert 
     * 5 символов - 1.3 сек (с вывоводм строк) 0.491895 sec - без вывода строк
     * 6 символов - 14.789793 sec (с вывоводм строк) 1.898168 sec - без вывода строк
     * 7 символов без вывода строк - 7.671152 sec
     * 8 символов без вывода строк - 30.412468 sec
     * 
     * Импорт С++ функции convert 
     * 5 символов - 1.23 сек (с вывоводм строк) 0.53 sec - без вывода строк
     * 6 символов - 13.538338 sec (с вывоводм строк) 2.30900 sec - без вывода строк
     * 7 символов без вывода строк - 8.43029 sec
     * 8 символов без вывода строк - 32.154832 sec
     * 
     */
}

#endif // UNITTEST