#include "pch.h"

#ifdef UNITTEST

#include <warning_push.h>
#include <gtest/gtest.h>
#include <warning_pop.h>

#include <newlang.h>
#include <chrono>
#include <stdio.h> 

using namespace newlang;

/*
 * Тест производительности на базе исходников из статьи
 * https://towardsdatascience.com/how-fast-is-c-compared-to-python-978f18f474c7
 */

char convert(char c) {
    if(c == 'A') return 'C';
    if(c == 'C') return 'G';
    if(c == 'G') return 'T';
    if(c == 'T') return 'A';
    return ' ';
}

TEST(Example, SpeedCPP) {

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

TEST(Example, SpeedNewLang) {

    Context::Reset();
    Context ctx(RunTime::Init());

    setvbuf(stdin, nullptr, _IONBF, 0);
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);


    ObjPtr test;

    ObjPtr str = ctx.ExecStr("@str := 'ABCDEF\\n';", nullptr, true);
    ASSERT_TRUE(str);
    ASSERT_STREQ("ABCDEF\n", str->GetValueAsString().c_str());

    test = ctx.ExecStr("@printf := :Pointer('printf(format:FmtChar, ...):Int'); @str := 'ABCDEF\\n'; printf('%s', str)", nullptr, true);
    ASSERT_TRUE(test);
    ASSERT_STREQ("7", test->GetValueAsString().c_str());

    test = ctx.ExecStr("str[1] = 32; str", nullptr, true);
    ASSERT_TRUE(test);
    ASSERT_STREQ("A CDEF\n", test->GetValueAsString().c_str());


    LLVMAddSymbol("convert", (void *) &convert);
    ObjPtr test_convert = ctx.ExecStr("@test_convert := :Pointer('convert(sym:Char):Char')", nullptr, true);
    ASSERT_TRUE(test_convert);

    test = ctx.ExecStr("test_convert('A')", nullptr, true);
    ASSERT_TRUE(test);
    ASSERT_STREQ("67", test->GetValueAsString().c_str());

    test = ctx.ExecStr("str[1] = test_convert('A'); str", nullptr, true);
    ASSERT_TRUE(test);
    ASSERT_STREQ("ACCDEF\n", test->GetValueAsString().c_str());

    utils::Logger::LogLevelType save = utils::Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    ObjPtr result = ctx.ExecFile("../examples/speed_test.nlp");
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    utils::Logger::Instance()->SetLogLevel(save);



    ASSERT_TRUE(result);
    ASSERT_TRUE(result->is_string_type()) << result->toString();
    ASSERT_STREQ("OK", result->GetValueAsString().c_str());


    int sec = std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
    int ms = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() % 1000000;
    LOG_INFO("Test speed complete at %d.%d sec", sec, ms);

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
     * После переделки способа хранения скаляров в нативном виде, а не в тензорах
     * Своя функция @convert 
     * 5 символов - 1.255214 сек (с вывоводм строк) 0.306725 sec - без вывода строк
     * 6 символов - 15.995722 sec (с вывоводм строк) 1.253190 sec - без вывода строк
     * 7 символов без вывода строк - 5.12946 sec
     * 8 символов без вывода строк - 19.653851 sec
     * 
     * Импорт С++ функции convert 
     * 5 символов -  1.195008 sec (с вывоводм строк)  0.351575 sec - без вывода строк
     * 6 символов -  13.666785 sec (с вывоводм строк) 1.428339 sec - без вывода строк
     * 7 символов без вывода строк - 5.628200 sec
     * 8 символов без вывода строк - 22.258760 sec
     * 
     * Start speed test NewLang
     * From AAAAAAAAAA to TTTTTTTTTT
     * Number of generated k-mers: 1048576
     * Test complete at 320.401650 sec (более 5 минут)
     * 
     * Никакой оптимизации не проводилось. 
     * Программа не преобразуется в промежуточный byte-code, а интерпретируется какждый раз при выполнении.
     * LLVM код вызова нативных функий генерируется каждый раз,  не при создании функции.
     * Возможности для оптимиации производительности чрезвычайно общширны ;-)
     */
}

TEST(Example, Fraction) {

    Context::Reset();
    Context ctx(RunTime::Init());

    setvbuf(stdin, nullptr, _IONBF, 0);
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);

    ObjPtr test;
    //@fact_recur(n:Integer) := {
    //    [n > 1] --> {
    //        -- n * fact_recur(n-1) --;
    //    };
    //    1;
    //};

    ObjPtr str = ctx.ExecStr("@str := 'ABCDEF\\n';", nullptr, true);
    ASSERT_TRUE(str);
    ASSERT_STREQ("ABCDEF\n", str->GetValueAsString().c_str());

    ObjPtr test_printf = ctx.ExecStr("@test_printf := :Pointer('printf(format:FmtChar, ...):Int')", nullptr, true);
    ASSERT_TRUE(test_printf);
    ASSERT_STREQ("test_printf={}", test_printf->GetValueAsString().c_str());

    ObjPtr iter = ctx.ExecStr("@iterator := (1, 5, 9999,)?", nullptr, true);
    ASSERT_TRUE(iter);
    ASSERT_TRUE(iter->getType() == ObjType::Iterator);
    ASSERT_TRUE(iter->m_iterator);
    ASSERT_TRUE(*iter->m_iterator.get() == iter->m_iterator->begin());
    ASSERT_TRUE(*iter->m_iterator.get() != iter->m_iterator->end());

    ObjPtr test_frac = ctx.ExecStr("@test_frac := 999\\123", nullptr, true);
    ASSERT_TRUE(test_frac);
    ASSERT_TRUE(test_frac->getType() == ObjType::Fraction);
    ASSERT_STREQ("999\\123", test_frac->GetValueAsString().c_str());

    ObjPtr str_frac = ctx.ExecStr(":StrChar(test_frac)", nullptr, true);
    ASSERT_TRUE(str_frac);
    ASSERT_TRUE(str_frac->getType() == ObjType::StrChar) << newlang::toString(str_frac->getType());
    ASSERT_STREQ("999\\123", str_frac->GetValueAsString().c_str()) << str_frac->GetValueAsString();

    ObjPtr test_prn = ctx.ExecStr("test_printf('%s', :StrChar(test_frac))", nullptr, true);
    ASSERT_TRUE(test_prn);
    ASSERT_STREQ("7", test_prn->GetValueAsString().c_str());

    ObjPtr test_arg = ctx.ExecStr("@test_arg(arg:Fraction) := {$arg}", nullptr, true);
    ASSERT_TRUE(test_arg);
    ASSERT_TRUE(test_arg->is_function());
    ASSERT_FALSE(test_arg->is_none_type());
    ASSERT_STREQ("test_arg={}", test_arg->GetValueAsString().c_str());



    ObjPtr frac_test;
    ASSERT_ANY_THROW(test_arg->Call(&ctx)); // Нет обязательно аргумента
    ASSERT_ANY_THROW(test_arg->Call(&ctx, Obj::Arg("неправильный тип аругумента")));

    frac_test = test_arg->Call(&ctx, Obj::Arg(1));
    ASSERT_TRUE(frac_test);
    ASSERT_EQ(ObjType::Fraction, frac_test->getType()) << newlang::toString(frac_test->getType());

    frac_test = ctx.ExecStr("test_arg(1)", nullptr, true);
    ASSERT_TRUE(frac_test);
    ASSERT_STREQ("1\\1", frac_test->GetValueAsString().c_str());



    utils::Logger::LogLevelType save = utils::Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    ObjPtr result = ctx.ExecFile("../examples/fraction.nlp");
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    utils::Logger::Instance()->SetLogLevel(save);



    ASSERT_TRUE(result);
    ASSERT_TRUE(result->is_string_type()) << result->toString();
    ASSERT_STREQ("OK", result->GetValueAsString().c_str());


    int sec = std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
    int ms = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() % 1000000;
    LOG_INFO("Test fraction complete at %d.%d sec", sec, ms);

    /*
     * 
     */
}

#endif // UNITTEST