#ifdef UNITTEST

#include "core/pch.h"
#include "core/term.h"

#include <core/warning_push.h>
#include <gtest/gtest.h>
#include <core/warning_pop.h>

#include <core/newlang.h>
#include <core/builtin.h>


using namespace newlang;

TEST(Compiler, EvalExample) {

    TermPtr p;
    Parser parser(p);
    parser.Parse("newlang(cpp);\n#Simple example\n%{printf(\"Hello world!\");%}");
    ASSERT_EQ(2, p->BlockCode().size());

    TermPtr op = p->BlockCode()[0];
    ASSERT_TRUE(op);

    ASSERT_EQ(TermID::CALL, op->GetTokenID());
    ASSERT_STREQ("newlang", op->getText().c_str());
    ASSERT_EQ(1, op->size());
    ASSERT_STREQ("cpp", (*op)[0]->getText().c_str());

    op = p->BlockCode()[1];
    ASSERT_EQ(TermID::SOURCE, op->GetTokenID());
    ASSERT_STREQ("printf(\"Hello world!\");", op->getText().c_str());
    ASSERT_EQ(0, op->size());
}

TEST(Compiler, MangleName) {

    EXPECT_STREQ("newlang_min", MangleName("мин").c_str()) << MangleName("мин");
    EXPECT_STREQ("newlang_maks", MangleName("макс").c_str()) << MangleName("макс");
}

// TEST(NewLang, Simple) {
//     TermPtr context;
//     Parser parser(context);
//
//     ASSERT_TRUE(parser.Parse(
//             "test_and(arg1, arg2) :&&= $arg1 == $arg2, $arg1;\n"
//             "test_or(arg1, arg2) :||= $arg1 == 555, $arg1;\n"
//             "test_xor(arg1, arg2) :^^= $arg1 == $arg2, $arg1;\n"
//             "test_filed(arg1, arg2):_ := {$arg1.all();
//             $arg1.mul_($arg2);_;}\n"
//             ));
//
//     ASSERT_TRUE(context->m_block.size() >= 3);
//     for (size_t i = 0; i < 3; i++) {
//         ASSERT_TRUE(context->m_block[i]);
//
//         ASSERT_EQ(TermID::SIMPLE, context->m_block[i]->GetTokenID()) << i;
//         ASSERT_TRUE(context->m_block[i]->Left());
//         ASSERT_TRUE(context->m_block[i]->Right());
//         ASSERT_EQ(2, context->m_block[i]->Left()->size());
//
//         TermPtr block = context->m_block[i]->Right();
//         ASSERT_TRUE(block);
//         ASSERT_EQ(TermID::BLOCK, block->getTermID());
//         ASSERT_EQ(2, block->BlockCode().size());
//
//         ASSERT_EQ(TermID::OPERATOR, block->BlockCode()[0]->getTermID());
//
//         TermPtr result = block->BlockCode()[1];
//         ASSERT_EQ(TermID::TERM, result->getTermID());
//     }
//
//     std::ostringstream sstr;
//
//     ASSERT_TRUE(NewLang::MakeCppFile(context, sstr)) << sstr.str();
//
//    std::filesystem::create_directories("temp");
//    ASSERT_TRUE(std::filesystem::is_directory("temp"));
// 
//    std::ofstream file("temp/fsimple_test.temp.cpp");
//    file << sstr.str();
//    file.close();
//
//    std::string out;
//    int exit_code;
//    ASSERT_TRUE(NewLang::GccMakeModule("temp/fsimple_test.temp.cpp",
//    "temp/fsimple_test.temp.nlm", nullptr, &out, &exit_code)) << exit_code <<
//    " "
//    << out;
//
//    RuntimePtr opts = RunTime::Init();
//    Context ctx(opts);
//
//    Module module;
//    ASSERT_TRUE(opts->LoadModule("temp/fsimple_test.temp.nlm"));
//
//    ObjPtr test_and = opts->GetFunc("test_and");
//    ASSERT_TRUE(test_and);
//    ObjPtr test_or = opts->GetFunc("test_or");
//    ASSERT_TRUE(test_or);
//    ObjPtr test_xor = opts->GetFunc("test_xor");
//    ASSERT_TRUE(test_xor);
//
//
//    EXPECT_FALSE(test_and->Call(Obj::Arg(123, "arg1"), Obj::Arg(555,
//    "arg2"))->GetValueAsBoolean());
//    EXPECT_TRUE(test_and->Call(Obj::Arg(123, "arg1"), Obj::Arg(123,
//    "arg2"))->GetValueAsBoolean());
//    EXPECT_TRUE(test_and->Call(Obj::Arg(555, "arg1"), Obj::Arg(555,
//    "arg2"))->GetValueAsBoolean()); EXPECT_FALSE(test_and->Call(Obj::Arg(0,
//    "arg1"), Obj::Arg(0, "arg2"))->GetValueAsBoolean());
//
//    EXPECT_TRUE(test_or->Call(Obj::Arg(123, "arg1"), Obj::Arg(555,
//    "arg2"))->GetValueAsBoolean()); EXPECT_TRUE(test_or->Call(Obj::Arg(555,
//    "arg1"), Obj::Arg(555, "arg2"))->GetValueAsBoolean());
//    EXPECT_TRUE(test_or->Call(Obj::Arg(123, "arg1"), Obj::Arg(123,
//    "arg2"))->GetValueAsBoolean()); EXPECT_TRUE(test_or->Call(Obj::Arg(555,
//    "arg1"), Obj::Arg(0, "arg2"))->GetValueAsBoolean());
//    EXPECT_FALSE(test_or->Call(Obj::Arg(0, "arg1"), Obj::Arg(0,
//    "arg2"))->GetValueAsBoolean());
//
//    EXPECT_TRUE(test_xor->Call(Obj::Arg(123, "arg1"), Obj::Arg(555,
//    "arg2"))->GetValueAsBoolean());
//    EXPECT_FALSE(test_xor->Call(Obj::Arg(555, "arg1"), Obj::Arg(555,
//    "arg2"))->GetValueAsBoolean());
//    EXPECT_FALSE(test_xor->Call(Obj::Arg(123, "arg1"), Obj::Arg(123,
//    "arg2"))->GetValueAsBoolean()); EXPECT_TRUE(test_xor->Call(Obj::Arg(0,
//    "arg1"), Obj::Arg(0, "arg2"))->GetValueAsBoolean());
//}

#define ASSERT_STRSTART(base, val) ASSERT_TRUE(str_cmp_strart(base, val))<<"  \""<<base<<"\"\n  \""<<val<<"\""
#define EXPECT_STRSTART(base, val) EXPECT_TRUE(str_cmp_strart(base, val))<<"  \""<<base<<"\"\n  \""<<val<<"\""

bool str_cmp_strart(const char *base_str, const char *cmp_str) {
    std::string base(base_str);
    std::string cmp(cmp_str);
    for (size_t i = 0; i < base.size() && i < cmp.size(); i++) {
        if (base[i] != cmp[i]) {
            return false;
        }
    }
    return base.size() <= cmp.size();
}

//bool simple_int_filter(Context *ctx, Object &in, ObjPtr & out) {
//    if(!ctx || !ctx->Current()->self) {
//        return false;
//    }
//    for (int i = 0; i < in.size(); i++) {
//        if(ctx->Current()->self->op_equal(in[i])) {
//            return false;
//        }
//    }
//    return true;
//}

///*
// * Основонй тест соответствия между компиляцией и интерпретацией текста
// программы
// * Реализован как набор выражений, возвращающих результат.
// * В начале все выражения выпоняются интерпретатором, тестируются на
// корректность и собираются в массив.
// * Далее выражения записываюстя в исходный фай в виде модуля, который
// компилируется, загружается и выполняется.
// * Результатом его выполнения так же является массив.
// * В итоге два маасива сравниваются между собой по элементно.
// */
// TEST(NewLang, CompileEvalConsistency) {
//
//    struct TestItem {
//        const char *result;
//        const char *eval;
//    };
//
//    // Выполняются построчно
//    std::vector<TestItem> eval_items = {
//        //        {"(val=456,)", "field_test $= (val=456,)"},
//        //        {"(val=456,)", "field_test()"},
//        //        {"123", "field_test.name := 123"},
//        //        {"(val=456, name=123,)", "field_test()"},
//        {"123.456", "float #= 123.456"},
//        {"муж.", "м #= \"муж.\""},
//        {"жен.", "ж #= \"жен.\""},
//        {"human=@term(пол=, parent=[,])", "human   @=  term(пол=,    parent  =
//        [,]   )"},
//        {"Tom2=human(пол=\"муж.\", parent=[,])", "Tom2 $= @human(пол=м)"},
//        {"Janna2=human(пол=\"жен.\", parent=[,])", "Janna2 $= @human(пол=ж)"},
//        {"Jake2=human(пол=\"муж.\", parent=[&Tom2, &Janna2,])", "Jake2 $=
//        @human(м, [  &Tom2, &Janna2, ] )"},
//        {"Tim2=human(пол=\"муж.\", parent=[&Tom2,])", "Tim2$=@human(пол=м,
//        parent=[&Tom2,])"},
//        //        {"brother={}", "brother(test1, test2) :&&= $test1!=$test2,
//        $test1.sex==м, intersec($test1.parent, $test2.parent)"},
//    };
//    // Возвращаются в виде элементов массива
//    std::vector<TestItem> array_items = {
//        {"123.456", "float"},
//        {"муж.", "м"},
//        {"жен.", "ж"},
//        {"100", "100"},
//        {"300", "100+200"},
//        {"100", "min(200,100,300)"},
//    };
//
//    RuntimePtr opts = NewLang::Init();
//    Context ctx(opts);
//    Parser parser;
//
//
//    // Литерал
//    ASSERT_STREQ("100", opts->Eval(&ctx, "100")->GetValueAsString().c_str());
//    // Оператор (символ +)
//    ASSERT_STREQ("300", opts->Eval(&ctx,
//    "100+200")->GetValueAsString().c_str());
//    // Вызов встроенной функции
//    Args min_args(Obj::CreateValue(200), Obj::CreateValue(100),
//    Obj::CreateValue(300)); // min(200,100,300) ASSERT_STREQ("100",
//    Context::CallByName(&ctx, "min", min_args)->GetValueAsString().c_str());
//
//    ObjPtr min_ret = newlang_min(&ctx, Args::Arg(Obj::CreateValue(200)),
//    Args::Arg(Obj::CreateValue(100)), Args::Arg(Obj::CreateValue(300)));
//    ASSERT_TRUE(min_ret);
//    ASSERT_STREQ("100", min_ret->GetValueAsString().c_str());
//
//
//    ASSERT_THROW(
//            opts->Eval(&ctx, "name2"),
//            std::runtime_error);
//    ASSERT_THROW(
//            opts->Eval(&ctx, "name3"),
//            std::runtime_error);
//
//    ASSERT_EQ(0, opts->m_user_terms.size());
//    ASSERT_TRUE(ctx.CreateGlobalTerm(Obj::CreateValue(111), "name1"));
//    ASSERT_EQ(1, opts->m_user_terms.size());
//    ASSERT_TRUE(opts->Eval(&ctx, "name2 $= 222; name3 $=
//    333")->GetValueAsString().c_str());
//
//    ASSERT_EQ(0, ctx.Current()->session.find("name2")->second->RefCount());
//    ASSERT_STREQ("222", opts->Eval(&ctx,
//    "name2")->GetValueAsString().c_str()); ASSERT_EQ(0,
//    ctx.Current()->session.find("name2")->second->RefCount());
//
//    ASSERT_STREQ("333", opts->Eval(&ctx,
//    "name3")->GetValueAsString().c_str());
//
//
//    Obj::Ptr field = Obj::CreateNone();
//    field->ItemAdd(Obj::CreateValue("FIELD1"), "field1");
//    field->ItemAdd(Obj::CreateValue("FIELD2"), "field2");
//    ASSERT_THROW(
//            Obj::GetField(field, "not found"),
//            std::runtime_error);
//
//    ASSERT_STREQ("FIELD1", Obj::GetField(field,
//    "field1")->GetValueAsString().c_str());
//
//    Obj::Ptr obj = Obj::CreateDict(Args::Arg("f", field),
//    Args::Arg("f2", field));
//
//    ASSERT_STREQ("FIELD1", Obj::GetField(Obj::GetField(obj, "f"),
//    "field1")->GetValueAsString().c_str()); ASSERT_STREQ("FIELD2",
//    Obj::GetField(Obj::GetField(obj, "f2"),
//    "field2")->GetValueAsString().c_str());
//
//    ASSERT_STREQ("(val=456,)", opts->Eval(&ctx, "field $=
//    (val=456,)")->GetValueAsString().c_str()); ASSERT_STREQ("(val=456,)",
//    opts->Eval(&ctx, "field")->GetValueAsString().c_str());
//    ASSERT_STREQ("123", opts->Eval(&ctx, "field.name :=
//    123")->GetValueAsString().c_str()); ASSERT_STREQ("(val=456, name=123,)",
//    opts->Eval(&ctx, "field")->GetValueAsString().c_str());
//
//
//
//
//    ASSERT_EQ(1, opts->m_user_terms.size());
//    ASSERT_STREQ("test=@term(пол=, parent=[,])", opts->Eval(&ctx, "test   @=
//    term(пол=,    parent  =   [,]   )")->GetValueAsString().c_str());
//    ASSERT_EQ(2, opts->m_user_terms.size());
//    ASSERT_STREQ("name1", opts->m_user_terms[0]->getName().c_str());
//
//    ASSERT_EQ(4, ctx.Current()->session.size());
//    ASSERT_EQ(0, ctx.Current()->session.find("name2")->second->RefCount());
//    ASSERT_STREQ("222",
//    ctx.Current()->session.find("name2")->second->toString().c_str());
//    ASSERT_STREQ("Tom=test(пол=222, parent=[,])", opts->Eval(&ctx, "Tom $=
//    @test(пол=name2)")->GetValueAsString().c_str()); ASSERT_EQ(0,
//    ctx.Current()->session.find("name2")->second->RefCount());
//
//    ASSERT_STREQ("[&Tom,]", opts->Eval(&ctx,
//    "ref$=[&Tom,]")->GetValueAsString().c_str()); ASSERT_STREQ("Tim=test(пол=,
//    parent=[&Tom,])", opts->Eval(&ctx, "Tim $=
//    @test(parent=[&Tom,])")->GetValueAsString().c_str());
//    ASSERT_STREQ("brother={}", opts->Eval(&ctx, "brother(test1, test2) :&&=
//    $test1!=$test2, $test1.sex==м, intersec($test1.parent,
//    $test2.parent)")->GetValueAsString().c_str());
//
//
//    std::string str = "#!./dist/Debug/GNU-Linux/nlc --exec\n"
//            "// Test source code for verify consistency between compilation
//            and evaluation modes\n"
//            "\n";
//
//    for (auto item : eval_items) {
//        ASSERT_STREQ(item.result, opts->Eval(&ctx,
//        item.eval)->GetValueAsString().c_str()); str += item.eval; str +=
//        ";\n";
//    }
//
//    str += "\nconsistency $= (,);\n";
//    for (auto item : array_items) {
//        ASSERT_STREQ(item.result, opts->Eval(&ctx,
//        item.eval)->GetValueAsString().c_str()); str += "consistency[] := ";
//        str += item.eval;
//        str += ";\n";
//    }
//
//    str += "\n$$ := consistency;\n";
//
//    std::filesystem::create_directories("temp");
//    ASSERT_TRUE(std::filesystem::is_directory("temp"));
//
//    std::ofstream out("temp/consistency.temp.sh");
//    out << str;
//    out.close();
//
//
//    Context ctx_module(NewLang::Init());
//    ObjPtr result_module = NewLang::ExecModule("temp/consistency.temp.sh",
//    "temp/consistency.temp.nlm", false, &ctx_module);
//    ASSERT_TRUE(result_module);
//
//
//    RuntimePtr copy = NewLang::Init();
//    Context ctx_copy(copy);
//    ObjPtr result_eval = copy->Eval(&ctx_copy, str.c_str());
//
//    ASSERT_TRUE(result_module->getItemCount());
//    ASSERT_EQ(array_items.size(), result_module->getItemCount());
//
//    ASSERT_TRUE(result_eval->getItemCount());
//    ASSERT_EQ(array_items.size(), result_eval->getItemCount());
//
//    RuntimePtr copy2 = NewLang::Init();
//    Context ctx_copy2(copy2);
//    ObjPtr res;
//    for (int i = 0; i < eval_items.size(); i++) {
//        res = copy2->Eval(&ctx_copy2, eval_items[i].eval);
//        ASSERT_TRUE(res);
//        ASSERT_STREQ(eval_items[i].result, res->GetValueAsString().c_str()) <<
//        eval_items[i].result << " != " << res->GetValueAsString();
//    }
//
//    for (int i = 0; i < array_items.size(); i++) {
//
//        res = copy2->Eval(&ctx_copy2, array_items[i].eval);
//        ASSERT_TRUE(res);
//        ASSERT_STREQ(array_items[i].result, res->GetValueAsString().c_str())
//        << array_items[i].result << " != " << res->GetValueAsString();
//
//        ObjPtr temp = result_module->getItem(i);
//        ASSERT_TRUE(temp->op_eq(res, true)) << i << "  " << res->toString();
//        temp = result_eval->getItem(i);
//        ASSERT_TRUE(temp->op_eq(res, true)) << i << "  " << res->toString();
//    }
//
//
//    //std::remove("temp/brother.sh.temp.nlm");
//}

// TEST(NewLang, JitLLVM) {
//
//     RuntimePtr opts = NewLang::Init();
//     Context ctx(opts);
//          const char *src = "\
//#include <stdio.h>\n\
//#include <string.h>\n\
//extern \"C\" int jit_print(const char *str) {\n\
//    printf(\"%s\\n\", str);\n\
//    return strlen(str);\n\
//}\n";
//
//     llvm::ExecutionEngine * e = opts->JITCompileCPP(src);
//
//
//     typedef int (*JIT_PRINT)(const char *);
//
//     e = opts->JITCompileCPP(src);
//     ASSERT_TRUE(e);
//     JIT_PRINT jit_print = reinterpret_cast<JIT_PRINT>
//     (opts->m_jit->GetFunction("jit_print")); ASSERT_TRUE(jit_print);
//     ASSERT_EQ(0, jit_print(""));
//     ASSERT_EQ(9, jit_print("jit_print"));
//
//
//
//    std::filesystem::create_directories("temp");
//    ASSERT_TRUE(std::filesystem::is_directory("temp"));
//
//     //    std::ofstream out("temp/native.temp.nlp");
//     //    out << "native(str=\"default arg\") $= { %{ $$=$str; printf(\"%s\",
//     static_cast<char *>($str)); %} };";
//     //    out.close();
//     //
//     //    ASSERT_TRUE(opts->CompileFunctions("native(str=\"default arg\") $=
//     { %{ $$=$str; printf(\"%s\", static_cast<char *>($str)); %} };"));
//     //    ASSERT_TRUE(opts->CompileModule("temp/native.temp.nlp",
//     "temp/native.temp.nlm"));
//     //    ASSERT_TRUE(opts->LoadModule("temp/native.temp.nlm"));
//     //
//     //    EXPECT_STREQ("default arg", opts->Eval(&ctx,
//     "native()")->GetValueAsString().c_str());
//     //    EXPECT_STREQ("default arg", opts->Eval(&ctx,
//     "@native()")->GetValueAsString().c_str());
//     //    EXPECT_STREQ("default arg", opts->Eval(&ctx,
//     "@native()")->GetValueAsString().c_str());
//     //    EXPECT_STREQ("", opts->Eval(&ctx,
//     "@native(\"\")")->GetValueAsString().c_str());
//     //    EXPECT_STREQ("Hello, world!\n", opts->Eval(&ctx, "@native(\"Hello,
//     world!\\n\")")->GetValueAsString().c_str());
// }

TEST(Compiler, DISABLED_Function) {

    const char *func_text =
            "func_sum(arg1, arg2) :- {$arg1 + $arg2;};\n"
            "func_call(arg1, arg2) :- {func_sum($arg1, $arg2);};";

    TermPtr funcs;
    Parser parser(funcs);

    parser.Parse(func_text);
    ASSERT_TRUE(funcs);
    ASSERT_EQ(TermID::BLOCK, funcs->GetTokenID());
    ASSERT_EQ(2, funcs->BlockCode().size());

    TermPtr func1 = funcs->BlockCode()[0];

    ASSERT_EQ(TermID::TRANSPARENT, func1->GetTokenID());
    ASSERT_TRUE(func1->Left());
    ASSERT_EQ(2, func1->Left()->size());

    ASSERT_TRUE(func1->Right());
    TermPtr body1 = func1->Right();

    //    ASSERT_EQ(TermID::LOCAL, body1->getTermID());
    //    ASSERT_TRUE(body1->Left());
    //    ASSERT_TRUE(body1->Right());
    //    ASSERT_STREQ(":=", body1->getText().c_str());
    //    ASSERT_EQ(TermID::RESULT, body1->Left()->getTermID());
    //    ASSERT_STREQ("$$", body1->Left()->getText().c_str());
    //    ASSERT_EQ(TermID::OPERATOR, body1->Right()->getTermID());

    TermPtr op = body1->Right();
    //    ASSERT_TRUE(op);
    //    ASSERT_TRUE(op->Left());
    //    ASSERT_TRUE(op->Right());
    //    ASSERT_EQ(TermID::TERM, op->Left()->getTermID());
    //    ASSERT_STREQ("+", op->getText().c_str());
    //    ASSERT_STREQ("$arg1", op->Left()->getText().c_str());
    //    ASSERT_STREQ("$arg2", op->Right()->getText().c_str());


    TermPtr func2 = funcs->BlockCode()[1];

    ASSERT_EQ(TermID::TRANSPARENT, func2->GetTokenID());
    ASSERT_TRUE(func2->Left());
    ASSERT_EQ(2, func2->Left()->size());

    //    ASSERT_TRUE(func2->Right());
    //    TermPtr body2 = func2->Right();
    //    ASSERT_TRUE(body2);
    //
    //    ASSERT_TRUE(body2->Left());
    //    ASSERT_TRUE(body2->Right());
    //    ASSERT_EQ(TermID::LOCAL, body2->getTermID());
    //    ASSERT_STREQ(":=", body2->getText().c_str());
    //    ASSERT_EQ(TermID::RESULT, body2->Left()->getTermID());
    //    ASSERT_STREQ("$$", body2->Left()->getText().c_str());
    //
    //    op = body2->Right();
    //    ASSERT_TRUE(op);
    //    ASSERT_FALSE(op->Left());
    //    ASSERT_FALSE(op->Right());
    //    ASSERT_EQ(TermID::CALL, op->getTermID());
    //    ASSERT_STREQ("func_sum", op->getText().c_str());
    //    ASSERT_EQ(2, op->size());
    //    ASSERT_STREQ("$arg1", (*op)[0]->getText().c_str());
    //    ASSERT_STREQ("$arg2", (*op)[1]->getText().c_str());


    std::ostringstream sstr;
    ASSERT_TRUE(NewLang::MakeCppFile(funcs, sstr)); // << sstr.str();


    std::filesystem::create_directories("temp");
    ASSERT_TRUE(std::filesystem::is_directory("temp"));

    std::ofstream file("temp/function_test.temp.cpp");
    file << sstr.str();
    file.close();
}

TEST(Compiler, DISABLED_FuncsTypes) {

    /*
     * - Проверка типов аргументов при вызове функций
     * - Проверка типов возвращаемых значений у функций
     */

    RuntimePtr opts = RunTime::Init();
    Context ctx(opts);

#define FUNC_ARG "func_arg(arg1: Char, arg2): Char := { $arg1+$arg2; };"
#define FUNC_RES "func_res(arg1: Char, arg2: Int): Integer := { $arg2+=$arg1; };"

    TermPtr func;
    Parser parser(func);

    std::ostringstream sstr;

    // Не соответствие типа функции в операторе
    parser.Parse(FUNC_ARG FUNC_RES "\n$res:Char := func_arg(100, 100); $res += func_res(100, 100);");
    sstr.str("");
    ASSERT_THROW(NewLang::MakeCppFile(func, sstr, nullptr, &ctx), Interrupt) << sstr.str();

    // Компилится без ошибок
    parser.Parse(FUNC_ARG "\nfunc_arg(Char(100), 100);");
    sstr.str("");
    ASSERT_NO_THROW(NewLang::MakeCppFile(func, sstr, nullptr, &ctx)) << sstr.str();

    // Не соответствие типа первого аргумента
    parser.Parse(FUNC_ARG "\nfunc_arg(1000, 100);");
    sstr.str("");
    ASSERT_THROW(NewLang::MakeCppFile(func, sstr, nullptr, &ctx), Interrupt) << sstr.str();

    // Не соответствие типа функции
    parser.Parse(FUNC_ARG FUNC_RES "\n$res:Char := func_res(100, 1000);");
    sstr.str("");
    ASSERT_THROW(NewLang::MakeCppFile(func, sstr, nullptr, &ctx), Interrupt) << sstr.str();

    // Не соответствие типа функции в операторе
    parser.Parse(FUNC_ARG FUNC_RES "\n$res:Char := func_arg(100, 100); $res += func_res(100, 100);");
    sstr.str("");
    ASSERT_THROW(NewLang::MakeCppFile(func, sstr, nullptr, &ctx), Interrupt) << sstr.str();

    // Нет типа у $res как в предыдщем случае
    parser.Parse(FUNC_ARG FUNC_RES "\n$res := func_arg(100, 100); $res += func_res(100, 100);");
    sstr.str("");
    ASSERT_NO_THROW(NewLang::MakeCppFile(func, sstr, nullptr, &ctx)) << sstr.str();

    // Тип есть, но делается каст возвращаемого типа у функции
    parser.Parse(FUNC_ARG FUNC_RES "\n$res: Char := func_arg(100, 100); $res += Char(func_res(100, 100));");
    sstr.str("");
    ASSERT_NO_THROW(NewLang::MakeCppFile(func, sstr, nullptr, &ctx)) << sstr.str();


    std::filesystem::create_directories("temp");
    ASSERT_TRUE(std::filesystem::is_directory("temp"));


    std::ofstream file("temp/call_types.temp.cpp");
    file << sstr.str();
    file.close();

    std::string out;
    int exit_code;
    ASSERT_TRUE(NewLang::GccMakeModule("temp/call_types.temp.cpp",
            "temp/call_types.temp.nlm", nullptr,
            &out, &exit_code))
            << exit_code << " " << out;

    ASSERT_TRUE(ctx.m_runtime->LoadModule("call_types.temp.nlm", false, &ctx));

    // Переполнение байтовой переменной $res во время выполнения последнего оператора "+="
    Obj args;
    ASSERT_TRUE(ctx.m_runtime->m_modules["temp/call_types.temp.nlm"]);
    ASSERT_NO_THROW(
            ctx.m_runtime->m_modules["temp/call_types.temp.nlm"]->Main(&ctx, args));
    //@todo Контроль переполнения при операциях для типизированных переменных
    //????????????????
    //@todo Такой же как и для остальных операций
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // ASSERT_THROW(opts->m_modules["temp/call_types.temp.nlm"]->Main(&ctx,
    // args), newlang_exception);

#undef FUNC_ARG
#undef FUNC_RES

}
#endif // UNITTEST