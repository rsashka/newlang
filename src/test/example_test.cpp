#include "pch.h"

#ifdef BUILD_UNITTEST

#include <warning_push.h>
#include <gtest/gtest.h>
#include <warning_pop.h>

#include <runtime.h>
#include <chrono>
#include <stdio.h> 

using namespace newlang;

/*
 * Тест производительности на базе исходников из статьи
 * https://towardsdatascience.com/how-fast-is-c-compared-to-python-978f18f474c7
 */
extern "C" char convert(char c);

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
    while (s != s_last) {
        counter++;
        // You can uncomment the next line to see all k-mers.
        //std::cout << s << std::endl;
        change_next = true;
        for (int i = 0; i < len_str; i++) {
            if (change_next) {
                if (s[i] == opt.back()) {
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
    int sec = (int) std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
    int ms = (int) std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() % 1000000;
    LOG_INFO("Float of generated k-mers: %d  at %d.%d sec", counter, sec, ms);

}

TEST(Example, DISABLED_SpeedNewLang) {

    RuntimePtr rt = RunTime::Init();
    ASSERT_TRUE(rt);

    setvbuf(stdin, nullptr, _IONBF, 0);
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);


    ObjPtr test;

    ObjPtr str = rt->Run("str := 'ABCDEF\\n';", nullptr);
    ASSERT_TRUE(str);
    ASSERT_STREQ("ABCDEF\n", str->GetValueAsString().c_str());

    test = rt->Run("str := 'ABCDEF\\n'; print('%s', str)");
    ASSERT_TRUE(test);
    ASSERT_STREQ("7", test->GetValueAsString().c_str());

    test = rt->Run("str[2] = 32; str", nullptr);
    ASSERT_TRUE(test);
    ASSERT_STREQ("AB DEF\n", test->GetValueAsString().c_str());

    test = rt->Run("str[0]", nullptr);
    ASSERT_TRUE(test);
    ASSERT_STREQ("A", test->GetValueAsString().c_str());

    test = rt->Run("str[1]", nullptr);
    ASSERT_TRUE(test);
    ASSERT_STREQ("B", test->GetValueAsString().c_str());

    test = rt->Run("str[2]", nullptr);
    ASSERT_TRUE(test);
    ASSERT_STREQ(" ", test->GetValueAsString().c_str());

    //    LLVMAddSymbol("convert", (void *) &convert);
    ObjPtr test_convert = rt->Run("test_convert(sym:Int8):Int8 := %convert ...");
    ASSERT_TRUE(test_convert);

//    [c == 'A'] --> 'C',
//    [c == 'C'] --> 'G',
//    [c == 'G'] --> 'T',
//    [c == 'T'] --> 'A',
//    [_] --> ' ';    

    test = rt->Run("test_convert(65)", nullptr);
    ASSERT_TRUE(test);
    ASSERT_STREQ("67", test->GetValueAsString().c_str());
    test = rt->Run("test_convert(67)", nullptr);
    ASSERT_TRUE(test);
    ASSERT_STREQ("71", test->GetValueAsString().c_str());

    test = rt->Run("test_convert(71)", nullptr);
    ASSERT_TRUE(test);
    ASSERT_STREQ("84", test->GetValueAsString().c_str());

    test = rt->Run("test_convert(84)", nullptr);
    ASSERT_TRUE(test);
    ASSERT_STREQ("65", test->GetValueAsString().c_str());

    ASSERT_NO_THROW(test = rt->Run("str"));
    ASSERT_TRUE(test);
    ASSERT_STREQ("AB DEF\n", test->GetValueAsString().c_str());

    ASSERT_NO_THROW(test = rt->Run("str[0]"));
    ASSERT_TRUE(test);
    ASSERT_STREQ("A", test->GetValueAsString().c_str());

    ASSERT_NO_THROW(test = rt->Run(":Int8('A')"));
    ASSERT_TRUE(test);
    ASSERT_STREQ("65", test->GetValueAsString().c_str());

//    ASSERT_NO_THROW(test = rt->Run("test_convert('A')"));
//    ASSERT_TRUE(test);
//    ASSERT_STREQ("C", test->GetValueAsString().c_str());

    ASSERT_NO_THROW(test = rt->Run("test_convert(:Int8(str[0]))"));
    ASSERT_TRUE(test);
    ASSERT_STREQ("67", test->GetValueAsString().c_str());

    ASSERT_NO_THROW(test = rt->Run("str[0] = test_convert(:Int8(str[0])); str"));
    ASSERT_TRUE(test);
    ASSERT_STREQ("CB DEF\n", test->GetValueAsString().c_str());

    utils::Logger::LogLevelType save = utils::Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    ObjPtr result = rt->RunFile("../examples/speed_test.src");
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    utils::Logger::Instance()->SetLogLevel(save);



    ASSERT_TRUE(result);
    ASSERT_TRUE(result->is_string_type()) << result->toString();
    ASSERT_STREQ("OK", result->GetValueAsString().c_str());


    int sec = (int) std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
    int ms = (int) std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() % 1000000;
    LOG_INFO("Test speed complete at %d.%d sec", sec, ms);

    /*
     * 
     * Start speed test C++
     * Float of generated k-mers: 67108864  at 2.502049 sec
     * 
     * Start
     * Float of generated k-mers: 67108864
     * real	0m33,794s     
     * 
     * Start speed test NewLang
     * Float of generated k-mers: 67108864
     * Test complete at  ????????????????????
     * 
     * Своя функция convert 
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
     * Своя функция convert 
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
     * Float of generated k-mers: 1048576
     * Test complete at 320.401650 sec (более 5 минут)
     * 
     * Никакой оптимизации не проводилось. 
     * Программа не преобразуется в промежуточный byte-code, а интерпретируется какждый раз при выполнении.
     * LLVM код вызова нативных функий генерируется каждый раз,  не при создании функции.
     * Возможности для оптимиации производительности чрезвычайно общширны ;-)
     */
}

TEST(Example, DISABLED_Rational) {

    RuntimePtr rt = RunTime::Init();
    ASSERT_TRUE(rt);

    setvbuf(stdin, nullptr, _IONBF, 0);
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);

    ObjPtr test;
    //fact_recur(n:Integer) := {
    //    [n > 1] --> {
    //        -- n * fact_recur(n-1) --;
    //    };
    //    1;
    //};

    ObjPtr str = rt->Run("str := 'ABCDEF\\n';");
    ASSERT_TRUE(str);
    ASSERT_STREQ("ABCDEF\n", str->GetValueAsString().c_str());

    ObjPtr test_printf = rt->Run("test_printf := :Pointer('printf(format:FmtChar, ...):Int32')");
    ASSERT_TRUE(test_printf);
    ASSERT_STREQ("test_printf={ }", test_printf->GetValueAsString().c_str());

    ObjPtr iter = rt->Run("iterator := (1, 5, 9999,)?");
    ASSERT_TRUE(iter);
    ASSERT_TRUE(iter->getType() == ObjType::Iterator);
    ASSERT_TRUE(iter->m_iterator);
    ASSERT_TRUE(*iter->m_iterator.get() == iter->m_iterator->begin());
    ASSERT_TRUE(*iter->m_iterator.get() != iter->m_iterator->end());

    ObjPtr test_frac = rt->Run("test_frac := 999\\123");
    ASSERT_TRUE(test_frac);
    ASSERT_TRUE(test_frac->getType() == ObjType::Rational);
    ASSERT_STREQ("999\\123", test_frac->GetValueAsString().c_str());

    ObjPtr str_frac = rt->Run(":StrChar(test_frac)");
    ASSERT_TRUE(str_frac);
    ASSERT_TRUE(str_frac->getType() == ObjType::StrChar) << newlang::toString(str_frac->getType());
    ASSERT_STREQ("999\\123", str_frac->GetValueAsString().c_str()) << str_frac->GetValueAsString();

    ObjPtr test_prn = rt->Run("test_printf('%s', :StrChar(test_frac))");
    ASSERT_TRUE(test_prn);
    ASSERT_STREQ("7", test_prn->GetValueAsString().c_str());

    ObjPtr test_arg = rt->Run("test_arg(arg:Rational) := {$arg}");
    ASSERT_TRUE(test_arg);
    ASSERT_TRUE(test_arg->is_function_type());
    ASSERT_FALSE(test_arg->is_none_type());
    ASSERT_STREQ("test_arg={ }", test_arg->GetValueAsString().c_str());



    //    ObjPtr frac_test;
    //    ASSERT_ANY_THROW(test_arg->Call(&ctx)); // Нет обязательно аргумента
    //    ASSERT_ANY_THROW(test_arg->Call(&ctx, Obj::Arg("неправильный тип аругумента")));
    //
    //    frac_test = test_arg->Call(&ctx, Obj::Arg(1));
    //    ASSERT_TRUE(frac_test);
    //    ASSERT_EQ(ObjType::Rational, frac_test->getType()) << newlang::toString(frac_test->getType());
    //
    //    frac_test = rt->Run("test_arg(1)", nullptr);
    //    ASSERT_TRUE(frac_test);
    //    ASSERT_STREQ("1\\1", frac_test->GetValueAsString().c_str());
    //
    //
    //
    //    utils::Logger::LogLevelType save = utils::Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
    //    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    //    ObjPtr result = rt->RunFile("../examples/rational.src");
    //    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    //    utils::Logger::Instance()->SetLogLevel(save);
    //
    //
    //
    //    ASSERT_TRUE(result);
    //    ASSERT_TRUE(result->is_string_type()) << result->toString();
    //    ASSERT_STREQ("OK", result->GetValueAsString().c_str());
    //
    //
    //    int sec = (int) std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
    //    int ms = (int) std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() % 1000000;
    //    LOG_INFO("Test rational complete at %d.%d sec", sec, ms);
    //
    //    save = utils::Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
    //    begin = std::chrono::steady_clock::now();
    //    result = rt->RunFile("\\\\('../examples/rational.src')");
    //    end = std::chrono::steady_clock::now();
    //    utils::Logger::Instance()->SetLogLevel(save);
    //
    //
    //    ASSERT_TRUE(result);
    //    ASSERT_TRUE(result->is_string_type()) << result->toString();
    //    ASSERT_STREQ("OK", result->GetValueAsString().c_str());

}

TEST(Example, Tensor) {

    RuntimePtr rt = RunTime::Init();
    ASSERT_TRUE(rt);


    TermPtr ast = rt->GetParser()->Parse("print('%%%s\\n', :StrChar(123))");
    ASSERT_TRUE(ast);

    AstAnalysis analysis(*rt, rt->m_diag.get());
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_EQ(2, ast->size());
    ASSERT_TRUE((*ast)[0].second->m_id == TermID::STRCHAR);
    ASSERT_TRUE((*ast)[1].second->m_id == TermID::TYPE);
    ASSERT_TRUE(!(*ast)[1].second->m_int_name.empty());


    ast = rt->GetParser()->Parse("var := 123; var2 := var * 123");
    ASSERT_TRUE(ast);

    AstAnalysis analysis2(*rt, rt->m_diag.get());
    ASSERT_TRUE(analysis2.Analyze(ast, ast));
    ASSERT_EQ(2, ast->m_block.size()) << ast->m_int_vars.Dump();
    ASSERT_TRUE(!ast->m_block[0]->m_left->m_int_name.empty());
    ASSERT_TRUE(!ast->m_block[1]->m_left->m_int_name.empty());
    ASSERT_TRUE(ast->m_block[1]->m_right->getTermID() == TermID::OP_MATH);
    TermPtr math = ast->m_block[1]->m_right;
    ASSERT_TRUE(math->m_right);
    ASSERT_TRUE(math->m_left);
    ASSERT_TRUE(!math->m_left->m_int_name.empty());


    ast = rt->GetParser()->Parse("rand():Int32 := %rand ...;");
    ASSERT_TRUE(ast);

    AstAnalysis analysis3(*rt, rt->m_diag.get());
    ASSERT_TRUE(analysis3.Analyze(ast, ast));
    ASSERT_TRUE(ast->m_right);
    ASSERT_TRUE(ast->m_left);
    ASSERT_TRUE(!ast->m_left->m_int_name.empty());


    ast = rt->GetParser()->Parse("r():Int32 := %rand ...; :Tensor[10](42, ... r() ...)");
    ASSERT_TRUE(ast);

    AstAnalysis analysis4(*rt, rt->m_diag.get());
    ASSERT_TRUE(analysis4.Analyze(ast, ast));
    ASSERT_EQ(2, ast->m_block.size()) << ast->m_int_vars.Dump();
    ASSERT_TRUE(!ast->m_block[0]->m_left->m_int_name.empty());

    TermPtr t = ast->m_block[0]->m_left;
    ASSERT_TRUE(t);
    ASSERT_TRUE(!t->m_int_name.empty()) << t->toString();

    t = ast->m_block[1];
    ASSERT_TRUE(t);
    ASSERT_EQ(2, t->size());
    ASSERT_TRUE((*t)[1].second);
    ASSERT_TRUE((*t)[1].second->m_right);
    ASSERT_TRUE(!(*t)[1].second->m_right->m_int_name.empty()) << (*t)[1].second->m_right->toString();



    ast = rt->GetParser()->Parse("sss := :Tensor[10](42, ... rnda() ...)");
    ASSERT_TRUE(ast);

    AstAnalysis analysis5(*rt, rt->m_diag.get());
    ASSERT_FALSE(analysis5.Analyze(ast, ast));


    setvbuf(stdin, nullptr, _IONBF, 0);
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);


    utils::Logger::LogLevelType save = utils::Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    ObjPtr result = rt->RunFile("../examples/tensor.src");
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    utils::Logger::Instance()->SetLogLevel(save);


    ASSERT_TRUE(result);
    ASSERT_TRUE(result->is_integer()) << result->toString();
    ASSERT_TRUE(result->GetValueAsInteger() > 300) << result->toString();


    //    save = utils::Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
    //    begin = std::chrono::steady_clock::now();
    //    result = rt->RunFile("\\\\('../examples/tensor.src')");
    //    end = std::chrono::steady_clock::now();
    //    utils::Logger::Instance()->SetLogLevel(save);
    //
    //    ASSERT_TRUE(result);
    //    ASSERT_TRUE(result->is_string_type()) << result->toString();
    //    ASSERT_STREQ("OK", result->GetValueAsString().c_str());

}

TEST(Example, Hello) {

    RuntimePtr rt = RunTime::Init();
    ASSERT_TRUE(rt);

    ObjPtr prn;
    ASSERT_NO_THROW(prn = rt->Run("prn(format:FmtChar, ...):Int32 ::= %printf ..."));
    ASSERT_TRUE(prn);

    ObjPtr res = (*prn)("Привет, мир!\n");
    ASSERT_TRUE(res);
    ASSERT_TRUE(res->is_integer()) << res->toString();
    ASSERT_STREQ("22", res->GetValueAsString().c_str());

    //    ObjPtr func = rt->Run("func(arg) := {$arg}");
    //    ASSERT_TRUE(func);
    //
    //    res = (*func)("TEST");
    //    ASSERT_TRUE(res);
    //    ASSERT_TRUE(res->is_string_char_type()) << res->toString();
    //    ASSERT_STREQ("TEST", res->GetValueAsString().c_str());
    //    hello(str) := { 
    //      printf := :Pointer('printf(format:FmtChar, ...):Int32');  # Импорт стандартной C функции
    //      printf('%s\n', $str);  # Вызов C функции с проверкой типов аргументов по строке формата
    //    };
    //    hello('Привет, мир!'); # Вызвать функцию    


    setvbuf(stdin, nullptr, _IONBF, 0);
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);


    utils::Logger::LogLevelType save = utils::Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    ObjPtr result = rt->RunFile("../examples/hello.src");
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    utils::Logger::Instance()->SetLogLevel(save);


    ASSERT_TRUE(result);
    ASSERT_TRUE(result->is_integral()) << result->toString();
    ASSERT_STREQ("14", result->GetValueAsString().c_str());


    //    save = utils::Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
    //    begin = std::chrono::steady_clock::now();
    //    ASSERT_NO_THROW(result = rt->Run("\\\\('../examples/hello.src')"));
    //    end = std::chrono::steady_clock::now();
    //    utils::Logger::Instance()->SetLogLevel(save);
    //
    //
    //    ASSERT_TRUE(result);
    //    ASSERT_TRUE(result->is_integral()) << result->toString();
    //    ASSERT_STREQ("14", result->GetValueAsString().c_str());
}

TEST(Example, Exec) {
    RuntimePtr rt = RunTime::Init({"--nlc-no-eval-enable"});

    ASSERT_FALSE(rt->m_eval_enable);
    ASSERT_ANY_THROW(rt->Run("`ls`"));
    rt->m_eval_enable = true;

    ObjPtr exec;
    ASSERT_NO_THROW(exec = rt->Run("`ls`"));
    ASSERT_TRUE(exec);
    ASSERT_TRUE(exec->is_string_char_type()) << exec->toString();
    ASSERT_TRUE(exec->GetValueAsString().find("nlc_test\n") != std::string::npos);
}


#endif // UNITTEST