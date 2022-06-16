#ifdef UNITTEST

#include "core/pch.h"
#include "core/term.h"

#include <core/warning_push.h>
#include <gtest/gtest.h>
#include <core/warning_pop.h>

#include <core/newlang.h>
#include <core/builtin.h>


using namespace newlang;

TEST(NewLang, Example1) {

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

TEST(NewLang, MangleName) {

    EXPECT_STREQ("newlang_min", MangleName("мин").c_str()) << MangleName("мин");
    EXPECT_STREQ("newlang_maks", MangleName("макс").c_str()) << MangleName("макс");
}

// TEST(NewLang, Simple) {
//     TermPtr context;
//     Parser parser(context);
//
//     ASSERT_TRUE(parser.Parse(
//             "test_and(arg1, arg2) &&= $arg1 == $arg2, $arg1;\n"
//             "test_or(arg1, arg2) ||= $arg1 == 555, $arg1;\n"
//             "test_xor(arg1, arg2) ^^= $arg1 == $arg2, $arg1;\n"
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
//    EXPECT_FALSE(test_and->Call(Object::Arg(123, "arg1"), Object::Arg(555,
//    "arg2"))->GetValueAsBoolean());
//    EXPECT_TRUE(test_and->Call(Object::Arg(123, "arg1"), Object::Arg(123,
//    "arg2"))->GetValueAsBoolean());
//    EXPECT_TRUE(test_and->Call(Object::Arg(555, "arg1"), Object::Arg(555,
//    "arg2"))->GetValueAsBoolean()); EXPECT_FALSE(test_and->Call(Object::Arg(0,
//    "arg1"), Object::Arg(0, "arg2"))->GetValueAsBoolean());
//
//    EXPECT_TRUE(test_or->Call(Object::Arg(123, "arg1"), Object::Arg(555,
//    "arg2"))->GetValueAsBoolean()); EXPECT_TRUE(test_or->Call(Object::Arg(555,
//    "arg1"), Object::Arg(555, "arg2"))->GetValueAsBoolean());
//    EXPECT_TRUE(test_or->Call(Object::Arg(123, "arg1"), Object::Arg(123,
//    "arg2"))->GetValueAsBoolean()); EXPECT_TRUE(test_or->Call(Object::Arg(555,
//    "arg1"), Object::Arg(0, "arg2"))->GetValueAsBoolean());
//    EXPECT_FALSE(test_or->Call(Object::Arg(0, "arg1"), Object::Arg(0,
//    "arg2"))->GetValueAsBoolean());
//
//    EXPECT_TRUE(test_xor->Call(Object::Arg(123, "arg1"), Object::Arg(555,
//    "arg2"))->GetValueAsBoolean());
//    EXPECT_FALSE(test_xor->Call(Object::Arg(555, "arg1"), Object::Arg(555,
//    "arg2"))->GetValueAsBoolean());
//    EXPECT_FALSE(test_xor->Call(Object::Arg(123, "arg1"), Object::Arg(123,
//    "arg2"))->GetValueAsBoolean()); EXPECT_TRUE(test_xor->Call(Object::Arg(0,
//    "arg1"), Object::Arg(0, "arg2"))->GetValueAsBoolean());
//}

// TEST(NewLang, FunctionFollow) {
//     TermPtr func;
//     Parser parser(func);
//     parser.Parse("func(arg1, arg2 = 5) :- { [$arg1 < $2] -> %{ return $arg1;
//     %} -> %{ return *$1 * *$2; %} };"); std::string to_str =
//     func->toString(); TermPtr func2; Parser parser2(func2);
//     parser2.Parse(to_str);
//     ASSERT_STREQ(to_str.c_str(), func2->toString().c_str());
//
//     ASSERT_EQ(TermID::TRANSPARENT, func->getTermID()) <<
//     newlang::toString(func->getTermID()); ASSERT_TRUE(func->Left());
//     ASSERT_TRUE(func->Right());
//
//     std::ostringstream sstr;
//     ASSERT_TRUE(NewLang::MakeCppFile(func, sstr)) << sstr.str();
//
//     std::filesystem::create_directories("temp");
//     ASSERT_TRUE(std::filesystem::is_directory("temp"));
//     std::ofstream file("temp/follow.temp.cpp");
//     file << sstr.str();
//     file.close();
//
//     std::string out;
//     int exit_code;
//     ASSERT_TRUE(NewLang::GccMakeModule("temp/follow.temp.cpp",
//     "temp/follow.temp.nlm", nullptr, &out, &exit_code)) << exit_code << " "
//     << out;
//
//     Module module;
//     ASSERT_TRUE(module.Load("temp/follow.temp.nlm"));
//
//     FunctionType * call = reinterpret_cast<FunctionType *>
//     (dlsym(module.m_handle, MangleName("func").c_str())); ASSERT_TRUE(func)
//     << MangleName("func");
//
//
//     Object args1(Object::Arg(), Object::Arg(5, "arg1"), Object::Arg(5,
//     "arg2")); ObjPtr result = (*call)(nullptr, args1); ASSERT_TRUE(*result ==
//     Object::CreateValue(25));
//
//     Object args2(Object::Arg(), Object::Arg(3, "arg1"), Object::Arg(10,
//     "arg2")); result = (*call)(nullptr, args2); ASSERT_TRUE(*result ==
//     Object::CreateValue(30));
//
//     Object args3(Object::Arg(), Object::Arg(50, "arg1"), Object::Arg(10,
//     "arg2")); result = (*call)(nullptr, args3); ASSERT_TRUE(*result ==
//     Object::CreateValue(50));
//
//
//     Context ctx(RunTime::Init());
//     ASSERT_TRUE(ctx.m_runtime->GetProcAddress("func",
//     "temp/follow.temp.nlm")); ObjPtr follow = ctx.m_runtime->GetFunc("func");
//     ASSERT_TRUE(follow);
//
//     ASSERT_ANY_THROW(
//             (*follow)() // Первый аргумент обязательный
//             );
//
//     ASSERT_ANY_THROW(
//             (*follow)(Object::Arg(5), Object::Arg(5), Object::Arg(5)) //
//             Функция принимает один или два аргумента
//             );
//
//     result = (*follow)(Object::Arg(5));
//     ASSERT_TRUE(*result == Object::CreateValue(25));
//
//     result = (*follow)(Object::Arg(3, "arg1"));
//     ASSERT_TRUE(*result == Object::CreateValue(15));
//
//     result = (*follow)(Object::Arg(6, "arg1"), Object::Arg(5, "arg2"));
//     ASSERT_TRUE(*result == Object::CreateValue(30));
//
//     result = (*follow)(Object::Arg(3, "arg1"), Object::Arg(7, "arg2"));
//     ASSERT_TRUE(*result == Object::CreateValue(27));
//
//     result = (*follow)(Object::Arg(50, "arg1"), Object::Arg(10, "arg2"));
//     ASSERT_TRUE(*result == Object::CreateValue(50));
// }
//
// TEST(NewLang, FunctionRepeat) {
//     TermPtr func;
//     Parser parser(func);
//     parser.Parse("func(arg1, count) :- {[$count==0]->{## \"Ошибка\";};
//     $arg1[].start:=$count(); $arg1[].stop:=$count(); [$count] <->
//     {$arg1[].step:=$count(); $count--; $arg1.stop=$count;}; $$ $1;};");
//
//     ASSERT_EQ(TermID::TRANSPARENT, func->getTermID()) <<
//     newlang::toString(func->getTermID()); ASSERT_TRUE(func->Left());
//     ASSERT_TRUE(func->Right());
//
//     std::ostringstream sstr;
//     ASSERT_TRUE(NewLang::MakeCppFile(func, sstr)) << sstr.str();
//
//     std::filesystem::create_directories("temp");
//     ASSERT_TRUE(std::filesystem::is_directory("temp"));
//     std::ofstream file("temp/repeat.temp.cpp");
//     file << sstr.str();
//     file.close();
//
//     std::string out;
//     int exit_code;
//     ASSERT_TRUE(NewLang::GccMakeModule("temp/repeat.temp.cpp",
//     "temp/repeat.temp.nlm", nullptr, &out, &exit_code)) << exit_code << " "
//     << out;
//
//
//     Context ctx(RunTime::Init());
//     ASSERT_TRUE(ctx.m_runtime->LoadModule("temp/repeat.temp.nlm"));
//     ASSERT_TRUE(ctx.m_runtime->GetProcAddress("func"));
//     ObjPtr repeat = ctx.GetFunc("func");
//     ASSERT_TRUE(repeat);
//
//     ASSERT_THROW(
//             (*repeat)(), // Первый аргумент обязательный
//             std::invalid_argument
//             );
//
//     ASSERT_THROW(
//             (*repeat)(Object::Arg(5)), // Функция принимает два аргумента
//             std::invalid_argument
//             );
//
//     ASSERT_THROW(
//             (*repeat)(Object::Arg(5), Object::Arg(5), Object::Arg(5)), //
//             Функция принимает два аргумента std::invalid_argument
//             );
//
//     ASSERT_THROW(
//             (*repeat)(Object::Arg(5), Object::Arg(0)), // Второй аргумент не
//             ноль! newlang_exception
//             );
//
//     ObjPtr result = repeat->Call(Object::Arg(Object::CreateDict(), "arg1"),
//     Object::Arg(1, "count")); ASSERT_TRUE(result); ASSERT_STREQ("(start=1,
//     stop=0, step=1,)", result->toString().c_str());
//
//     result = repeat->Call(Object::Arg(Object::CreateDict()), Object::Arg(2));
//     ASSERT_TRUE(result);
//     ASSERT_STREQ("(start=2, stop=0, step=2, step=1,)",
//     result->toString().c_str());
//
//     result = repeat->Call(Object::Arg(Object::CreateDict()), Object::Arg(3));
//     ASSERT_TRUE(result);
//     ASSERT_STREQ("(start=3, stop=0, step=3, step=2, step=1,)",
//     result->toString().c_str());
// }

#define ASSERT_STRSTART(base, val) ASSERT_TRUE(str_cmp_strart(base, val))<<"  \""<<base<<"\"\n  \""<<val<<"\""
#define EXPECT_STRSTART(base, val) EXPECT_TRUE(str_cmp_strart(base, val))<<"  \""<<base<<"\"\n  \""<<val<<"\""

bool str_cmp_strart(const char *base_str, const char *cmp_str) {
    std::string base(base_str);
    std::string cmp(cmp_str);
    for (size_t i = 0; i < base.size() && i < cmp.size(); i++) {
        if(base[i] != cmp[i]) {
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
//        //        {"brother={}", "brother(test1, test2) &&= $test1!=$test2,
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
//    Args min_args(Object::CreateValue(200), Object::CreateValue(100),
//    Object::CreateValue(300)); // min(200,100,300) ASSERT_STREQ("100",
//    Context::CallByName(&ctx, "min", min_args)->GetValueAsString().c_str());
//
//    ObjPtr min_ret = newlang_min(&ctx, Args::Arg(Object::CreateValue(200)),
//    Args::Arg(Object::CreateValue(100)), Args::Arg(Object::CreateValue(300)));
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
//    ASSERT_TRUE(ctx.CreateGlobalTerm(Object::CreateValue(111), "name1"));
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
//    Object::Ptr field = Object::CreateNone();
//    field->ItemAdd(Object::CreateValue("FIELD1"), "field1");
//    field->ItemAdd(Object::CreateValue("FIELD2"), "field2");
//    ASSERT_THROW(
//            Object::GetField(field, "not found"),
//            std::runtime_error);
//
//    ASSERT_STREQ("FIELD1", Object::GetField(field,
//    "field1")->GetValueAsString().c_str());
//
//    Object::Ptr obj = Object::CreateDict(Args::Arg("f", field),
//    Args::Arg("f2", field));
//
//    ASSERT_STREQ("FIELD1", Object::GetField(Object::GetField(obj, "f"),
//    "field1")->GetValueAsString().c_str()); ASSERT_STREQ("FIELD2",
//    Object::GetField(Object::GetField(obj, "f2"),
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
//    ASSERT_STREQ("brother={}", opts->Eval(&ctx, "brother(test1, test2) &&=
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

TEST(NewLang, DISABLED_Function) {

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

TEST(NewLang, String) {

    ObjPtr str_byte = Obj::CreateString("byte");
    ASSERT_STREQ("byte", str_byte->GetValueAsString().c_str());
    ASSERT_EQ(4, str_byte->size());
    ASSERT_EQ(4, str_byte->m_str.size());
    ASSERT_STREQ("b", (*str_byte)[0]->GetValueAsString().c_str());
    ASSERT_STREQ("y", (*str_byte)[1]->GetValueAsString().c_str());
    ASSERT_STREQ("t", (*str_byte)[2]->GetValueAsString().c_str());
    ASSERT_STREQ("e", (*str_byte)[3]->GetValueAsString().c_str());

    str_byte->op_set_index(0, "B");
    str_byte->op_set_index(1, "Y");
    ASSERT_STREQ("BYte", str_byte->GetValueAsString().c_str());
    str_byte->op_set_index(2, "T");
    str_byte->op_set_index(3, "E");
    ASSERT_STREQ("BYTE", str_byte->GetValueAsString().c_str());

    ObjPtr str_char = Obj::CreateString(L"строка");
    ASSERT_EQ(6, str_char->size());
    ASSERT_EQ(6, str_char->m_wstr.size());

    ASSERT_STREQ("с", (*str_char)[0]->GetValueAsString().c_str());
    ASSERT_STREQ("т", (*str_char)[1]->GetValueAsString().c_str());
    ASSERT_STREQ("р", (*str_char)[2]->GetValueAsString().c_str());
    ASSERT_STREQ("о", (*str_char)[3]->GetValueAsString().c_str());
    ASSERT_STREQ("к", (*str_char)[4]->GetValueAsString().c_str());
    ASSERT_STREQ("а", (*str_char)[5]->GetValueAsString().c_str());

    str_char->op_set_index(0, "С");
    str_char->op_set_index(1, "Т");
    ASSERT_STREQ("СТрока", str_char->GetValueAsString().c_str());
    str_char->op_set_index(2, "Р");
    str_char->op_set_index(3, "О");
    ASSERT_STREQ("СТРОка", str_char->GetValueAsString().c_str());


    ObjPtr format = Obj::CreateString("$1 $2 ${name}");
    ObjPtr str1 = (*format)(nullptr);
    ASSERT_STREQ("$1 $2 ${name}", str1->GetValueAsString().c_str());

    ObjPtr str2 = (*format)(nullptr, Obj::Arg(100));
    ASSERT_STREQ("100 $2 ${name}", str2->GetValueAsString().c_str());

    ObjPtr str3 = (*format)(nullptr, Obj::Arg(-1), Obj::Arg("222"));
    ASSERT_STREQ("-1 222 ${name}", str3->GetValueAsString().c_str());

    ObjPtr str4 = (*format)(nullptr, Obj::Arg("value", "name"));
    ASSERT_STREQ("value $2 value", str4->GetValueAsString().c_str());

    format = Obj::CreateString("$nameno ${имя1} $name $имя");
    ObjPtr str5 = (*format)(nullptr, Obj::Arg("value", "name"), Obj::Arg("УТФ8-УТФ8", "имя"), Obj::Arg("УТФ8", "имя1"));
    ASSERT_STREQ("valueno УТФ8 value УТФ8-УТФ8", str5->GetValueAsString().c_str());
}

TEST(NewLang, Tensor) {

    // Байтовые строки
    ObjPtr str = Obj::CreateString("test");
    ObjPtr t_str = Obj::CreateTensor(str->toTensor());
    ASSERT_EQ(t_str->m_var_type_current, ObjType::Char) << toString(t_str->m_var_type_current);
    ASSERT_EQ(4, t_str->size());
    EXPECT_STREQ(t_str->toType(ObjType::StrWide)->GetValueAsString().c_str(), "test");

    ASSERT_EQ(t_str->index_get({0})->GetValueAsInteger(), 't');
    ASSERT_EQ(t_str->index_get({1})->GetValueAsInteger(), 'e');
    ASSERT_EQ(t_str->index_get({2})->GetValueAsInteger(), 's');
    ASSERT_EQ(t_str->index_get({3})->GetValueAsInteger(), 't');

    t_str->index_set_({1}, Obj::CreateString("E"));
    t_str->index_set_({2}, Obj::CreateString("S"));

    EXPECT_STREQ(t_str->toType(ObjType::StrWide)->GetValueAsString().c_str(), "tESt");

    // Символьные сторки
    ObjPtr wstr = Obj::CreateString(L"ТЕСТ");
    ObjPtr t_wstr = Obj::CreateTensor(wstr->toTensor());
    ASSERT_EQ(t_wstr->m_var_type_current, ObjType::Int);
    ASSERT_EQ(4, t_wstr->size());
    EXPECT_STREQ(t_wstr->toType(ObjType::StrWide)->GetValueAsString().c_str(), "ТЕСТ");

    ASSERT_EQ(t_wstr->index_get({0})->GetValueAsInteger(), L'Т');
    ASSERT_EQ(t_wstr->index_get({1})->GetValueAsInteger(), L'Е');
    ASSERT_EQ(t_wstr->index_get({2})->GetValueAsInteger(), L'С');
    ASSERT_EQ(t_wstr->index_get({3})->GetValueAsInteger(), L'Т');

    t_wstr->index_set_({1}, Obj::CreateString(L"е"));
    t_wstr->index_set_({2}, Obj::CreateString(L"с"));

    EXPECT_STREQ(t_wstr->toType(ObjType::StrWide)->GetValueAsString().c_str(), "ТесТ");




//    ObjPtr dict = Context::CreateRVal(nullptr, "(1, [10,20,30,],)");
//
//    ASSERT_TRUE(dict);
//    ASSERT_EQ(2, dict->size());
//    ASSERT_EQ(0, (*dict)[0]->size());
//    ASSERT_EQ(3, (*dict)[1]->size());
//
//    ASSERT_EQ(1, (*dict)[0]->GetValueAsInteger());
//
//    ASSERT_EQ(10, (*(*dict)[1])[0]->GetValueAsInteger());
//    ASSERT_EQ(20, (*(*dict)[1])[1]->GetValueAsInteger());
//    ASSERT_EQ(30, (*(*dict)[1])[2]->GetValueAsInteger());
//
//    ObjPtr t_dict = Object::CreateTensor(dict, ObjType::Int);
//
//    ASSERT_EQ(4, t_dict->size());
//    ASSERT_EQ(t_dict->index_get({0})->GetValueAsInteger(), 1);
//    ASSERT_EQ(t_dict->index_get({1})->GetValueAsInteger(), 10);
//    ASSERT_EQ(t_dict->index_get({2})->GetValueAsInteger(), 20);
//    ASSERT_EQ(t_dict->index_get({3})->GetValueAsInteger(), 30);
//
//
//    ObjPtr diag = Context::CreateRVal(nullptr, "[[ [ [1,], [2,22,], [3, 33, 333.0,] ] ]]");
//
//    ObjPtr t_diag = Object::CreateTensor(diag, ObjType::Float);
//
//    ASSERT_EQ(6, t_diag->size());
//    ASSERT_EQ(t_diag->index_get({0})->GetValueAsNumber(), 1);
//    ASSERT_EQ(t_diag->index_get({1})->GetValueAsNumber(), 2);
//    ASSERT_EQ(t_diag->index_get({2})->GetValueAsNumber(), 22);
//    ASSERT_EQ(t_diag->index_get({3})->GetValueAsNumber(), 3);
//    ASSERT_EQ(t_diag->index_get({4})->GetValueAsNumber(), 33);
//    ASSERT_EQ(t_diag->index_get({5})->GetValueAsNumber(), 333);

}

TEST(NewLang, DISABLED_FuncsTypes) {

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
    ASSERT_THROW(NewLang::MakeCppFile(func, sstr, nullptr, &ctx), parser_exception) << sstr.str();

    // Компилится без ошибок
    parser.Parse(FUNC_ARG "\nfunc_arg(Char(100), 100);");
    sstr.str("");
    ASSERT_NO_THROW(NewLang::MakeCppFile(func, sstr, nullptr, &ctx)) << sstr.str();

    // Не соответствие типа первого аргумента
    parser.Parse(FUNC_ARG "\nfunc_arg(1000, 100);");
    sstr.str("");
    ASSERT_THROW(NewLang::MakeCppFile(func, sstr, nullptr, &ctx), parser_exception) << sstr.str();

    // Не соответствие типа функции
    parser.Parse(FUNC_ARG FUNC_RES "\n$res:Char := func_res(100, 1000);");
    sstr.str("");
    ASSERT_THROW(NewLang::MakeCppFile(func, sstr, nullptr, &ctx), parser_exception) << sstr.str();

    // Не соответствие типа функции в операторе
    parser.Parse(FUNC_ARG FUNC_RES "\n$res:Char := func_arg(100, 100); $res += func_res(100, 100);");
    sstr.str("");
    ASSERT_THROW(NewLang::MakeCppFile(func, sstr, nullptr, &ctx), parser_exception) << sstr.str();

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

TEST(NewLang, FuncsNative) {

    /*
     * Создание типизированных объектов из терминов
     * проверка типов при вызове нативных функций
     * вызов нативных функций
     * проверка типов строки формата
     */

//    std::vector <std::pair < std::string, std::string>> list;
//
//#define DEFINE_TYPES(name, cast) list.push_back(std::make_pair<std::string,std::string>(#name, #cast));
//    NL_BUILTIN_CAST_TYPE(DEFINE_TYPES)
//#undef DEFINE_TYPES
//
//    ASSERT_TRUE(list.size());
//
//    ObjType type_cast;
//    std::string type_str;
//
//    // По идее эта проверка не требуется, т.к. при компиляции идет проверка на наличие эелемнта в перечислении типов
//    for (size_t i = 0; i < list.size(); i++) {
//        ASSERT_TRUE(!list[i].first.empty());
//        ASSERT_TRUE(!list[i].second.empty());
//        for (size_t j = i + 1; j < list.size(); j++) {
//            ASSERT_STRNE(list[i].first.c_str(), list[j].first.c_str());
//        }
//
//        ASSERT_NO_THROW(type_cast = typeFromString(list[i].second));
//        type_str = toString(type_cast);
//        ASSERT_STREQ(list[i].second.c_str(), type_str.c_str());
//    }

//    // Доступ к локальным переменным
//    const bool local_bool = true;
//    ObjPtr loc_bool = Object::CreateLocal(local_bool);
//    ASSERT_TRUE(loc_bool);
//    ASSERT_TRUE(loc_bool->m_is_const);
//    ASSERT_FALSE(loc_bool->m_is_reference);
//    ASSERT_EQ(ObjType::LocalBool, loc_bool->m_var_type);
//    ASSERT_TRUE(loc_bool->GetValueAsBoolean());
//
//    uint8_t local_byte = 10;
//    ObjPtr loc_byte = Object::CreateLocal(local_byte);
//    ASSERT_TRUE(loc_byte);
//    ASSERT_FALSE(loc_byte->m_is_const);
//    ASSERT_FALSE(loc_byte->m_is_reference);
//    ASSERT_EQ(ObjType::LocalByte, loc_byte->m_var_type);
//    ASSERT_EQ(10, loc_byte->GetValueAsInteger());
//    local_byte = 111;
//    ASSERT_EQ(111, loc_byte->GetValueAsInteger());
//    loc_byte->SetValue_(Object::CreateValue(99));
//    ASSERT_EQ(99, local_byte);
//
//
//    double local_double = 99.99;
//    ObjPtr loc_double = Object::CreateLocal(local_double);
//    ASSERT_TRUE(loc_double);
//    ASSERT_FALSE(loc_double->m_is_const);
//    ASSERT_FALSE(loc_double->m_is_reference);
//    ASSERT_EQ(ObjType::LocalDouble, loc_double->m_var_type);
//    ASSERT_EQ(99, loc_double->GetValueAsInteger());
//    ASSERT_EQ(99.99, loc_double->GetValueAsNumber());
//
//    local_double = 33;
//    ASSERT_EQ(33, loc_double->GetValueAsNumber());
//    loc_double->SetValue_(Object::CreateValue(10.33));
//    ASSERT_EQ(10.33, local_double);
//
//    std::string local_str = "String";
//    ObjPtr loc_string = Object::CreateLocal(local_str);
//
//    ASSERT_TRUE(loc_string);
//    ASSERT_FALSE(loc_string->m_is_const);
//    ASSERT_FALSE(loc_string->m_is_reference);
//    ASSERT_EQ(ObjType::LocalStdString, loc_string->m_var_type);
//    ASSERT_STREQ("String", loc_string->GetValueAsString().c_str());
//
//    local_str = "Строка";
//    ASSERT_STREQ("Строка", loc_string->GetValueAsString().c_str());
//    loc_string->SetValue_(Object::CreateString(L"Широкая строка"));
//    ASSERT_STREQ("Широкая строка", local_str.c_str());
//
//
//    // Парсинг строки формата
//
//    ASSERT_EQ(1, parse_printf_format("%d", 0, nullptr));
//    int types[10];
//    ASSERT_EQ(3, parse_printf_format("%lu  %.2f   %100s", 10, types));
//    ASSERT_EQ(types[0] & ~PA_FLAG_MASK, PA_INT);
//    ASSERT_EQ(types[1] & ~PA_FLAG_MASK, PA_DOUBLE);
//    ASSERT_EQ(types[2] & ~PA_FLAG_MASK, PA_STRING);
//    ASSERT_EQ(types[0] & PA_FLAG_MASK, PA_FLAG_LONG);
//    ASSERT_EQ(types[1] & PA_FLAG_MASK, 0);
//    ASSERT_EQ(types[2] & PA_FLAG_MASK, 0);
//
//    RuntimePtr rt = RunTime::Init();
//    Context ctx(rt);
//
//    // Нативные функции
//
//    ASSERT_TRUE(rt->GetProcAddress("puts", nullptr));
//    ASSERT_TRUE(rt->GetProcAddress("atof", nullptr));
//    ASSERT_TRUE(rt->GetProcAddress("printf", nullptr));
//
//    ObjPtr puts_ = ctx.CreateNative("puts(str:StrChar):Int");
//    ObjPtr atof_ = ctx.CreateNative("atof(str:StrChar):Double");
//    ObjPtr printf_ = ctx.CreateNative("printf(format:Format, ...):Int");
//
//    ASSERT_TRUE(puts_);
//    ASSERT_TRUE(atof_);
//    ASSERT_TRUE(printf_);
//
//    ASSERT_TRUE(puts_->Call("Call Native Func!!!")->GetValueAsInteger() > 0);
//    ASSERT_EQ(100.99, atof("100.99"));
//    ASSERT_EQ(100.99, atof_->Call("100.99")->GetValueAsNumber());
//    ASSERT_EQ(-123456.789, atof("-123456.789"));
//    ASSERT_EQ(-123456.789, atof_->Call("-123456.789")->GetValueAsNumber());
//
//    Object args(Object::Arg("%s %f %d %s\n"), Object::Arg("%s %f %d %s"), Object::Arg(99.99), Object::Arg(100), Object::Arg("СТРОКА"));
//    ASSERT_EQ(39, printf_->Call(args)->GetValueAsInteger());
//
//    ASSERT_TRUE(ParsePrintfFormat(args, 0));
//    ASSERT_FALSE(ParsePrintfFormat(args, 1));
//    ASSERT_FALSE(ParsePrintfFormat(args, 2));
//    ASSERT_FALSE(ParsePrintfFormat(args, 3));
//    ASSERT_TRUE(ParsePrintfFormat(args, 4)); // В строке формата "СТРОКА" нет данных для вывода поэтому нет и ошибки формата

}

#endif // UNITTEST