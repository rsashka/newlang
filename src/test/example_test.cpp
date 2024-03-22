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

    Logger::LogLevelType save = Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    ObjPtr result = rt->RunFile("../examples/speed_test.src");
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    Logger::Instance()->SetLogLevel(save);



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

TEST(Example, Rational) {

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

    ObjPtr str;
    ASSERT_NO_THROW(str = rt->Run("str := 'ABCDEF\\n';"));
    ASSERT_TRUE(str);
    ASSERT_STREQ("ABCDEF\n", str->GetValueAsString().c_str());

    ObjPtr test_printf;
    ASSERT_NO_THROW(test_printf = rt->Run("test_printf(format:FmtChar, ...):Int32 := %printf...")) << rt->Dump() << "\n" << rt->m_main_ast->m_int_vars.Dump();
    ASSERT_TRUE(test_printf);
    ASSERT_STREQ("test_printf::(format:FmtChar, ...):Int32{ }", test_printf->toString().c_str());

    ObjPtr iter;
    ASSERT_NO_THROW(iter = rt->Run("iterator := (1, 5, 9999,)?"));
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

    ObjPtr test_prn = rt->Run("test_printf('%s\\n', :StrChar(test_frac))");
    ASSERT_TRUE(test_prn);
    ASSERT_STREQ("8", test_prn->GetValueAsString().c_str());

    ObjPtr test_arg;
    ASSERT_NO_THROW(test_arg = rt->Run("test_arg(arg:Rational) := {$arg*$arg}"));
    ASSERT_TRUE(test_arg);
    ASSERT_TRUE(test_arg->is_function_type());
    ASSERT_FALSE(test_arg->is_none_type());
    ASSERT_STREQ("test_arg::(arg:Rational){ }", test_arg->toString().c_str());



    ObjPtr frac_test;
    ASSERT_ANY_THROW(frac_test = rt->Run("test_arg()"));
    ASSERT_ANY_THROW(frac_test = rt->Run("test_arg('')"));

    ASSERT_NO_THROW(frac_test = rt->Run("test_arg(1)"))
            << rt->m_main_ast->m_int_vars.Dump() << "\n"
            << rt->m_main_ast->m_int_vars.find("test_arg$")->second->m_int_vars.Dump();

    ASSERT_TRUE(frac_test);
    ASSERT_EQ(ObjType::Rational, frac_test->getType()) << newlang::toString(frac_test->getType());
    ASSERT_STREQ("1\\1", frac_test->GetValueAsString().c_str());

    ASSERT_NO_THROW(frac_test = rt->Run("test_arg(2\\1)"));
    ASSERT_TRUE(frac_test);
    ASSERT_STREQ("4\\1", frac_test->GetValueAsString().c_str());

    ObjPtr iter_test;
    ASSERT_NO_THROW(iter_test = rt->Run("iter_test := :Int64(5+1)..1..-1?"));
    ASSERT_TRUE(iter_test);
    ASSERT_STREQ(":Iterator", iter_test->toString().c_str());

    ObjPtr fact_range_test;
    ASSERT_NO_THROW(fact_range_test = rt->Run(""
            "n:=1\\1;"
            "iter_cnt := @iter(6..1..-1);"
            "[ @curr(iter_cnt) ] <-> {"
            "   print('1: n=%s, iter=%s\\n', :StrChar(n), :StrChar(@curr(iter_cnt)));"
            "   n *= @next(iter_cnt);"
            "   print('2: n=%s\\n', :StrChar(n));"
            "};"
            "n;"
            ));
    ASSERT_TRUE(fact_range_test);
    ASSERT_STREQ("720\\1", fact_range_test->toString().c_str());

    ObjPtr result;

    ASSERT_NO_THROW(result = rt->RunFile("../examples/fact_40.src"));
    ASSERT_TRUE(result);
    ASSERT_STREQ("815915283247897734345611269596115894272000000000\\1", result->GetValueAsString().c_str());
    
    ASSERT_NO_THROW(result = rt->RunFile("../examples/fact_40_dsl.src"));
    ASSERT_TRUE(result);
    ASSERT_STREQ("815915283247897734345611269596115894272000000000\\1", result->GetValueAsString().c_str());
    

    Logger::LogLevelType save = Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    ASSERT_NO_THROW(result = rt->RunFile("../examples/rational.src"));
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    Logger::Instance()->SetLogLevel(save);

    ASSERT_TRUE(result);
    ASSERT_STREQ("402387260077093773543702433923003985719374864210714632543799910429938512398629020592044208486969404800479988610197196058631666872994808558901323829669944590997424504087073759918823627727188732519779505950995276120874975462497043601418278094646496291056393887437886487337119181045825783647849977012476632889835955735432513185323958463075557409114262417474349347553428646576611667797396668820291207379143853719588249808126867838374559731746136085379534524221586593201928090878297308431392844403281231558611036976801357304216168747609675871348312025478589320767169132448426236131412508780208000261683151027341827977704784635868170164365024153691398281264810213092761244896359928705114964975419909342221566832572080821333186116811553615836546984046708975602900950537616475847728421889679646244945160765353408198901385442487984959953319101723355556602139450399736280750137837615307127761926849034352625200015888535147331611702103968175921510907788019393178114194545257223865541461062892187960223838971476088506276862967146674697562911234082439208160153780889893964518263243671616762179168909779911903754031274622289988005195444414282012187361745992642956581746628302955570299024324153181617210465832036786906117260158783520751516284225540265170483304226143974286933061690897968482590125458327168226458066526769958652682272807075781391858178889652208164348344825993266043367660176999612831860788386150279465955131156552036093988180612138558600301435694527224206344631797460594682573103790084024432438465657245014402821885252470935190620929023136493273497565513958720559654228749774011413346962715422845862377387538230483865688976461927383814900140767310446640259899490222221765904339901886018566526485061799702356193897017860040811889729918311021171229845901641921068884387121855646124960798722908519296819372388642614839657382291123125024186649353143970137428531926649875337218940694281434118520158014123344828015051399694290153483077644569099073152433278288269864602789864321139083506217095002597389863554277196742822248757586765752344220207573630569498825087968928162753848863396909959826280956121450994871701244516461260379029309120889086942028510640182154399457156805941872748998094254742173582401063677404595741785160829230135358081840096996372524230560855903700624271243416909004153690105933983835777939410970027753472000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000\\1", result->GetValueAsString().c_str());

    int sec = (int) std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
    int ms = (int) std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() % 1000000;
    LOG_INFO("Test factorial 1000! complete at %d.%d sec", sec, ms);
    

//    save = Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
//    begin = std::chrono::steady_clock::now();
//    result = rt->RunFile("\\\\('../examples/rational.src')");
//    end = std::chrono::steady_clock::now();
//    Logger::Instance()->SetLogLevel(save);
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


    Logger::LogLevelType save = Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    ObjPtr result = rt->RunFile("../examples/tensor.src");
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    Logger::Instance()->SetLogLevel(save);


    ASSERT_TRUE(result);
    ASSERT_TRUE(result->is_integer()) << result->toString();
    ASSERT_TRUE(result->GetValueAsInteger() == 20) << result->toString();


    //    save = Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
    //    begin = std::chrono::steady_clock::now();
    //    result = rt->RunFile("\\\\('../examples/tensor.src')");
    //    end = std::chrono::steady_clock::now();
    //    Logger::Instance()->SetLogLevel(save);
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


    Logger::LogLevelType save = Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    ObjPtr result = rt->RunFile("../examples/hello.src");
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    Logger::Instance()->SetLogLevel(save);


    ASSERT_TRUE(result);
    ASSERT_TRUE(result->is_integral()) << result->toString();
    ASSERT_STREQ("14", result->GetValueAsString().c_str());


    //    save = Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
    //    begin = std::chrono::steady_clock::now();
    //    ASSERT_NO_THROW(result = rt->Run("\\\\('../examples/hello.src')"));
    //    end = std::chrono::steady_clock::now();
    //    Logger::Instance()->SetLogLevel(save);
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