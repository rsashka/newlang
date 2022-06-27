#include "pch.h"

#ifdef UNITTEST

#include <core/warning_push.h>
#include <gtest/gtest.h>
#include <core/warning_pop.h>

#include <core/nlc.h>
#include "core/builtin.h"

using namespace newlang;

TEST(NLC, Options) {

    NLC nlc0;
    ASSERT_TRUE(nlc0.m_path.empty());
    ASSERT_EQ(NLC::Mode::ModeError, nlc0.m_mode);

    NLC nlc1("");
    ASSERT_TRUE(nlc1.m_path.empty());
    ASSERT_EQ(NLC::Mode::ModeError, nlc1.m_mode);

    NLC nlc2("path");
    ASSERT_STREQ("path", nlc2.m_path.c_str());
    ASSERT_EQ(NLC::Mode::ModeInter, nlc2.m_mode) << nlc2.m_output;

    NLC nlc3("   path   --help    ");
    ASSERT_STREQ("path", nlc3.m_path.c_str());
    ASSERT_EQ(NLC::Mode::ModeHelp, nlc3.m_mode);
    ASSERT_FALSE(nlc3.m_output.empty()) << nlc3.m_output;

    NLC nlc4("path   -v");
    ASSERT_STREQ("path", nlc4.m_path.c_str());
    ASSERT_EQ(NLC::Mode::ModeVersion, nlc4.m_mode);

    //    NLC nlc5("path  --output=output_file -cinput_file");
    //    ASSERT_STREQ("path", nlc5.m_path.c_str());
    //    ASSERT_EQ(NLC::COMPILE, nlc5.m_mode) << nlc5.m_output;
    //    ASSERT_STREQ("output_file", nlc5.m_ofile.c_str());
    //    ASSERT_STREQ("input_file", nlc5.m_ifile.c_str());

    //    NLC nlc6("path  --badarg --output=output_file");
    //    ASSERT_EQ(NLC::ERROR, nlc6.m_mode);

    //    NLC nlc7("path func(bad!,,args)");
    //    ASSERT_EQ(NLC::ERROR, nlc7.m_mode) << nlc7.m_output;

    //    NLC nlc8("path  -lmodule.nlm --exec=file_name");
    //    ASSERT_STREQ("path", nlc8.m_path.c_str());
    //    ASSERT_EQ(NLC::EXEC, nlc8.m_mode);
    //    ASSERT_STREQ("file_name", nlc8.m_ifile.c_str());
    //    ASSERT_EQ(1, nlc8.m_modules.size());
    //    ASSERT_STREQ("module.nlm", nlc8.m_modules[0].c_str());

    NLC nlc9("path  --load=module2.nlm  call(arg1,100)");
    ASSERT_STREQ("path", nlc9.m_path.c_str());
    ASSERT_EQ(NLC::Mode::ModeEval, nlc9.m_mode);
    ASSERT_EQ(1, nlc9.m_modules.size());
    ASSERT_STREQ("module2.nlm", nlc9.m_modules[0].c_str());
    ASSERT_STREQ("call(arg1,100)", nlc9.m_eval.c_str());

    NLC nlc10("path  --load=module2.nlm,module3.nlm 100+200");
    ASSERT_STREQ("path", nlc10.m_path.c_str());
    ASSERT_EQ(NLC::Mode::ModeEval, nlc10.m_mode);
    ASSERT_EQ(2, nlc10.m_modules.size());
    ASSERT_STREQ("module2.nlm", nlc10.m_modules[0].c_str());
    ASSERT_STREQ("module3.nlm", nlc10.m_modules[1].c_str());
    ASSERT_STREQ("100+200", nlc10.m_eval.c_str());

    NLC nlc11("path  --load=module2.nlm,,   --load-only=,tt1,tt2,tt3  \"100+200\"");
    ASSERT_STREQ("path", nlc11.m_path.c_str());
    ASSERT_EQ(NLC::Mode::ModeEval, nlc11.m_mode) << nlc11.m_output;
    ASSERT_EQ(1, nlc11.m_modules.size());
    ASSERT_STREQ("module2.nlm", nlc11.m_modules[0].c_str());
    ASSERT_STREQ("\"100+200\"", nlc11.m_eval.c_str());

    ASSERT_EQ(3, nlc11.m_load_only.size());
    ASSERT_STREQ("tt1", nlc11.m_load_only[0].c_str());
    ASSERT_STREQ("tt2", nlc11.m_load_only[1].c_str());
    ASSERT_STREQ("tt3", nlc11.m_load_only[2].c_str());

    //    NLC nlc_empty("path");
    //    ASSERT_STREQ("path", nlc_empty.m_path.c_str());
    //    ASSERT_EQ(NLC::HELP, nlc_empty.m_mode);
    //    ASSERT_FALSE(nlc_empty.m_output.empty()) << nlc_empty.m_output;
}

TEST(NLC, FileFunc) {

    ASSERT_STREQ("", GetFileExt("").c_str());
    ASSERT_STREQ(".", GetFileExt(".").c_str());
    ASSERT_STREQ(".", GetFileExt(".").c_str());
    ASSERT_STREQ(".", GetFileExt("name.").c_str());
    ASSERT_STREQ(".ext", GetFileExt("name.ext").c_str());
    ASSERT_STREQ(".ext", GetFileExt(".ext").c_str());

    ASSERT_STREQ("", AddDefaultFileExt("", ".nlm").c_str());
    ASSERT_STREQ(".", AddDefaultFileExt(".", ".nlm").c_str());
    ASSERT_STREQ("name.nlm", AddDefaultFileExt("name", ".nlm").c_str());
    ASSERT_STREQ("name.nlm", AddDefaultFileExt("name.nlm", ".nlm").c_str());
    ASSERT_STREQ("name.test", AddDefaultFileExt("name.test", ".nlm").c_str());
    ASSERT_STREQ("name.", AddDefaultFileExt("name.", ".nlm").c_str());

    ASSERT_STREQ("", ReplaceFileExt("", ".nlp", ".nlm").c_str());
    ASSERT_STREQ(".", ReplaceFileExt(".", ".nlp", ".nlm").c_str());
    ASSERT_STREQ("name.nlm", ReplaceFileExt("name", ".nlp", ".nlm").c_str());
    ASSERT_STREQ("name.nlm", ReplaceFileExt("name.nlp", ".nlp", ".nlm").c_str());
    ASSERT_STREQ("name.test.nlm", ReplaceFileExt("name.test", ".nlp", ".nlm").c_str());
    ASSERT_STREQ("name.", ReplaceFileExt("name.", ".nlp", ".nlm").c_str());
}

TEST(NLC, Eval) {
    NLC nlc6("path  --badarg --output=output_file");
    ASSERT_EQ(NLC::Mode::ModeEval, nlc6.m_mode);
    ASSERT_EQ(1, nlc6.Run());

    NLC nlc7("path func(bad!,,args)");
    ASSERT_EQ(NLC::Mode::ModeEval, nlc7.m_mode) << nlc7.m_output;
    ASSERT_EQ(1, nlc7.Run());
}

/*
 * #!./dist/Debug/GNU-Linux/nlc --exec
 * print(str="") $= { %{ printf("%s", static_cast<char *>($str)); return $str; %} };
 * @print("Привет, мир!\n");
 */
TEST(NLC, EvalHelloWorld) {

    std::string cmd;
    cmd += "#!./dist/Debug/GNU-Linux/nlc --eval\n";
    cmd += "hello(str='') := { printf := @import('printf(format:Format, ...):Int'); printf('%s', $str); $str;};\n";
    cmd += "hello('Привет, мир!\\n');";

    std::filesystem::create_directories("temp");
    ASSERT_TRUE(std::filesystem::is_directory("temp"));


    std::ofstream out("temp/hello.temp.sh");
    out << cmd;
    out.close();

    NLC run;
    ObjPtr result = run.m_ctx.ExecStr(cmd, nullptr, true);
    ASSERT_TRUE(result);
    ASSERT_STREQ("Привет, мир!\n", result->GetValueAsString().c_str());
}

///*
// * #!./dist/Debug/GNU-Linux/nlc --exec
// * print(str="") $= { %{ printf("%s", static_cast<char *>($str)); return $str;
// %} };
// * @print("Привет, мир!\n");
// */
// TEST(NLC, ExecHelloWorld) {
//
//
//    std::filesystem::create_directories("temp");
//    ASSERT_TRUE(std::filesystem::is_directory("temp"));
//
//    std::ofstream out("temp/hello.temp.sh");
//    out << "#!./dist/Debug/GNU-Linux/nlc --exec\n";
//    out << "hello(str=\"\") := { %{ printf(\"%s\", static_cast<char *>(*$1));
//    return $str; %} };\n"; out << "hello(\"Привет, мир!\\n\");"; out.close();
//
//    NLC run;
//    ObjPtr result = RunTime::Init()->ExecModule("temp/hello.temp.sh",
//    "temp/hello.temp.sh.nlm", false, &run.m_ctx); ASSERT_TRUE(result);
//    ASSERT_STREQ("Привет, мир!\n", result->GetValueAsString().c_str());
//}
//
///*
// * #!./dist/Debug/GNU-Linux/nlc --exec
// * human @= @term(sex=, parent=[,]); // human $= @term(sex=, parent=[,]);
// $[].human := human; @[] := human;
// * Tom @= @human(sex=male);
// * Tom1 @= @human(sex=male):Tom;
// * Tom2 @= @human(sex=male):Tom;
// * Tom3 @= @human(sex=male):Tom;
// * Janna @= @human(sex=female);
// * Jake @= @human(sex=male, [Tom, Janna,]);
// * Tim @= @human(sex=male, parent=[Tom,]);
// *
// * human::brother(test) :&&= $0!=$test, $0.sex==male, @intersec($0.parent,
// $test.parent);
// *
// * Tim.brother(Jake);
// * Tim.brother(human!)?;
// * brother(human!)?;
// *
// *
// * male := "male";
// * male2 := male; // Ссылка пока не изменяется. При изменении - копия
// * male3 := &male; // Ссылка
// * male4 := male(); //??????????????????????????
// *
// * Tim := human(sex=male); // Ссылка пока не изменяется. При изменении - копия
// * Tom := human(sex=&male); // Ссылка
// *
// * male := "new"; // male="new"?    male2="male"?    male3 := "new";
// Tim.sex="male"?   Tom.sex="male"?
// * male2 := "var"; // male2="var"?   male="male"?    Tim.sex="male"?
// Tom.sex="male"?
// * male3 := "ref"; // male3="ref"?   male="ref"?     Tim.sex="male"?
// Tom.sex="ref"?
// *
// * :sex := (male, female,);
// * :human := (sex:sex=_, parent:human=_):(); // human $= @term(sex=,
// parent=[,]); $[].human := human; @[] := human;
// * Tom := human(sex=male);
// * Janna := human(sex=sex.female);
// * Jake := human(sex=male, [Tom, Janna,]);
// * Tim := human(sex=male, parent=(Tom,));
// *
// * :human::brother(test) :&&= $0!=$test, $0.sex==male, @intersec($0.parent,
// $test.parent);
// *
// * Tim.brother(Jake);
// * Tim.brother(human!)?;
// * brother(human!)?;
//
// *
// */
//
////ObjPtr test_brother(Context *ctx, Object &in) {
////    if(in.size() != 3) {
////        return Obj::No();
////    }
////    if(!in[1]->op_class_test("human") || !in[2]->op_class_test("human")) {
////        return Obj::No();
////    }
////    if(in[1]->op_eq(in[2], true)) {
////        return Obj::No();
////    }
////    ObjPtr sex = Obj::CreateDict(Obj::Arg("male", "пол"));
////    if(!in[1]->op_duck_test(sex, true)) {
////        return Obj::No();
////    }
////    if(!in[2]->op_duck_test(sex, true)) {
////        return Obj::No();
////    }
////    return *(*in[1])["parent"] * (*in[2])["parent"];
////}
//
// ObjPtr test_brother(Context *ctx, Object &in) {
//    if(in.size() != 3) {
//        return Obj::No();
//    }
//    if(static_cast<bool> ((*in[1] != *in[2])->GetValueAsBoolean())) {
//        ;
//    } else {
//        return Obj::No();
//    }
//
//    if(static_cast<bool> ((*(*in[1])["пол"] ==
//    Obj::CreateString("male"))->GetValueAsBoolean())) {
//        ;
//    } else {
//        return newlang::Obj::No();
//    }
//
//    if(static_cast<bool> ((*(*in[1])["parent"]*
//    *(*in[2])["parent"])->GetValueAsBoolean())) {
//        ;
//    } else {
//        return newlang::Obj::No();
//    }
//    return newlang::Obj::Yes();
//}
//
////TEST(NLC, Brother) {
////    Context ctx(NewLang::Init());
////
////    ObjPtr test = Obj::CreateType(ObjType::Class, "temp");
////    ASSERT_STREQ("temp=()", test->toString().c_str());
////
////    ObjPtr test2 = (*test)();
////    ASSERT_STREQ("temp=temp()", test2->toString().c_str());
////
////    ASSERT_STREQ(test2->m_var_name.c_str(), test->m_var_name.c_str());
////    ObjPtr test3 = (*test)(Obj::Arg(100, "100"), Obj::Arg(L"string",
///"str")); /    ASSERT_STREQ("temp=temp(100=100, str=\"string\")",
/// test3->toString().c_str());
////
////    ASSERT_STREQ(test3->m_var_name.c_str(), test->m_var_name.c_str());
////    test3->m_var_name = "test3";
////    ASSERT_EQ(2, test3->size());
////    ASSERT_STREQ("100", test3->name(0).c_str());
////    ASSERT_TRUE((*test3)[0]);
////    ASSERT_EQ(100, (*test3)[0]->GetValueAsInteger());
////    ASSERT_TRUE((*test3)["str"]);
////    ASSERT_STREQ("string", (*test3)["str"]->GetValueAsString().c_str());
////    ASSERT_STREQ("test3=temp(100=100, str=\"string\")",
/// test3->toString().c_str());
////
////    ObjPtr test4 = (*test3)(Obj::Arg(999, "100"), Obj::Arg("string2",
///"str2")); /    ASSERT_STREQ(test4->m_var_name.c_str(),
/// test3->m_var_name.c_str()); /    test4->m_var_name = "test4"; / ASSERT_EQ(3,
/// test4->size()); /    ASSERT_TRUE((*test4)[0]); /    ASSERT_STREQ("100",
/// test4->name(0).c_str()); /    ASSERT_TRUE((*test4)[0]); /    ASSERT_EQ(999,
///(*test4)[0]->GetValueAsInteger()); /    ASSERT_TRUE((*test4)["str"]); /
/// ASSERT_STREQ("string", (*test4)["str"]->GetValueAsString().c_str()); /
/// ASSERT_TRUE((*test4)["str2"]); /    ASSERT_STREQ("string2",
///(*test4)["str2"]->GetValueAsString().c_str()); /
/// ASSERT_STREQ("test4=test3(100=999, str=\"string\", str2='string2')",
/// test4->toString().c_str());
////
////    ObjPtr newlang_m = Obj::CreateString(L"male");
////    ASSERT_EQ(1, ctx.size());
////    ctx.CreateSessionTerm(newlang_m, "м");
////    ASSERT_EQ(2, ctx.size());
////    newlang_m->MakeConst();
////    ObjPtr newlang_zh = Obj::CreateString(L"female");
////
////    //    ASSERT_EQ(1, newlang_zh.use_count());
////    ctx.CreateSessionTerm(newlang_zh, "ж");
////    ASSERT_EQ(3, ctx.size());
////    //    ASSERT_EQ(2, newlang_zh.use_count());
////
////    newlang_zh->MakeConst();
////
////    utils::Logger::LogLevelType save =
/// utils::Logger::Instance()->GetLogLevel(); /
/// utils::Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
////
////    ObjPtr human = Obj::CreateDict(Obj::Arg(Obj::CreateNone(),
///"пол"), Obj::Arg(Obj::CreateDict(), "parent")); /    human->m_var_type
///= ObjType::Class; /    human->m_var_name = "human";
////
////    ASSERT_TRUE(human->op_class_test("human"));
////
////    ASSERT_STREQ("human=(пол=, parent=[,])", human->toString().c_str());
////    ASSERT_EQ(0, ctx.m_info.global->size());
////    ctx.CreateGlobalTerm(human, "human");
////    ASSERT_EQ(1, ctx.m_info.global->size());
////    ASSERT_STREQ("human=(пол=, parent=[,])", human->toString().c_str());
////    ObjPtr Tom = (*human)(Obj::Arg(newlang_m, "пол"));
////    ASSERT_TRUE(Tom->op_class_test("human"));
////
////    //    ASSERT_EQ(1, Tom.use_count());
////    ctx.CreateGlobalTerm(Tom, "Tom");
////    ASSERT_EQ(2, ctx.m_info.global->size());
////    //    ASSERT_EQ(3, Tom.use_count()); // USER TERM + GLOBAL TERM
////
////    ASSERT_STREQ("Tom=human(пол=\"male\", parent=[,])",
/// Tom->toString().c_str()); /    ASSERT_STREQ("Tom=human(пол=\"male\",
/// parent=[,])", Tom->toString().c_str()); /    ObjPtr Janna =
///(*human)(Obj::Arg(newlang_zh, "пол")); /
/// ASSERT_STREQ("human=human(пол=\"female\", parent=[,])",
/// Janna->toString().c_str()); /    ASSERT_STREQ("Tom=human(пол=\"male\",
/// parent=[,])", Tom->toString().c_str()); /
/// ASSERT_STREQ("human=human(пол=\"female\", parent=[,])",
/// Janna->toString().c_str());
////
////    ctx.CreateGlobalTerm(Janna, "Janna");
////    ASSERT_TRUE(Janna->op_class_test("human"));
////
////    ASSERT_EQ(3, ctx.m_info.global->size());
////    ASSERT_STREQ("Tom=human(пол=\"male\", parent=[,])",
/// Tom->toString().c_str()); /    ASSERT_STREQ("Janna=human(пол=\"female\",
/// parent=[,])", Janna->toString().c_str());
////
////    //    ASSERT_EQ(3, Tom.use_count());
////
////    ObjPtr array = Obj::CreateDict(Obj::Arg(Tom));
////    ASSERT_STREQ("[Tom=human(пол=\"male\", parent=[,]),]",
/// array->toString().c_str()); /    Tom->RefInc(); /    ASSERT_STREQ("[&Tom,]",
/// array->toString().c_str()); /    ObjPtr array2 =
/// Obj::CreateDict(Obj::Arg(Tom), Obj::Arg(Janna)); / Tom->RefInc();
////    Janna->RefInc();
////    ASSERT_STREQ("[&Tom, &Janna,]", array2->toString().c_str());
////
////    ObjPtr Jake = (*human)(Obj::Arg(newlang_m, "пол"),
/// Obj::Arg(Obj::CreateDict(Obj::Arg(Tom)), "parent")); /
/// ctx.CreateGlobalTerm(Jake, "Jake"); /    ASSERT_EQ(4,
/// ctx.m_info.global->size()); /    ASSERT_STREQ("Jake=human(пол=\"male\",
/// parent=[&Tom,])", Jake->toString().c_str()); /    ObjPtr Tim =
///(*human)(Obj::Arg(newlang_m, "пол"),
/// Obj::Arg(Obj::CreateDict(Obj::Arg(Tom), Obj::Arg(Janna)),
///"parent")); /    ctx.CreateGlobalTerm(Tim, "Tim"); /    ASSERT_EQ(5,
/// ctx.m_info.global->size()); /    ASSERT_STREQ("Tim=human(пол=\"male\",
/// parent=[&Tom, &Janna,])", Tim->toString().c_str());
////
////    utils::Logger::Instance()->SetLogLevel(save);
////
////    ASSERT_TRUE((*Jake)["пол"]->op_eq(newlang_m, true));
////    ASSERT_TRUE((*Tim)["пол"]->op_eq(newlang_m, true));
////    ASSERT_FALSE((*Janna)["пол"]->op_eq(newlang_m, true));
////    ASSERT_TRUE((*Janna)["пол"]->op_eq(newlang_zh, true));
////
////    ASSERT_TRUE(Janna->op_eq(Janna, true));
////    ASSERT_FALSE(Janna->op_eq(Tim, true));
////    ASSERT_TRUE(Tim->op_eq(Tim, true));
////    ASSERT_TRUE(Janna->op_eq(Janna, false));
////    ASSERT_FALSE(Janna->op_eq(Tim, false));
////    ASSERT_TRUE(Tim->op_eq(Tim, false));
////
////    ASSERT_TRUE((*Jake)["parent"]->op_bit_and((*Jake)["parent"],
/// false)->GetValueAsBoolean()); /
/// ASSERT_TRUE((*Jake)["parent"]->op_bit_and((*Tim)["parent"],
/// false)->GetValueAsBoolean()); /
/// ASSERT_FALSE((*Jake)["parent"]->op_bit_and((*Tom)["parent"],
/// false)->GetValueAsBoolean());
////
////    ASSERT_TRUE((*Jake)["parent"]->op_bit_and((*Jake)["parent"],
/// true)->GetValueAsBoolean()); /
/// ASSERT_TRUE((*Jake)["parent"]->op_bit_and((*Tim)["parent"],
/// true)->GetValueAsBoolean()); /
/// ASSERT_FALSE((*Jake)["parent"]->op_bit_and((*Tom)["parent"],
/// true)->GetValueAsBoolean());
////
////    auto cursor_all = ctx.m_info.global->select();
////    ASSERT_FALSE(cursor_all.complete());
////    ASSERT_EQ(5, cursor_all.reset());
////
////    Object args("human");
////    ASSERT_TRUE(NewLang::CompareFunc_(Obj::pair(human, "human"), args,
/// nullptr)); /    ASSERT_TRUE(NewLang::CompareFunc_(Obj::pair(human), args,
/// nullptr)); /    ASSERT_TRUE(NewLang::CompareFunc_(Obj::pair(Tim), args,
/// nullptr)); /    ASSERT_TRUE(NewLang::CompareFunc_(Obj::pair(Tom), args,
/// nullptr)); /    ASSERT_FALSE(NewLang::CompareFunc_(Obj::pair(test2),
/// args, nullptr)); / ASSERT_FALSE(NewLang::CompareFunc_(Obj::pair(test3),
/// args, nullptr));
////
////    Object args_empty;
////    ASSERT_FALSE(NewLang::CompareFunc_(Obj::pair(human, "human"),
/// args_empty, nullptr));
////
////    auto cursor = ctx.m_info.global->select("human");
////    ASSERT_FALSE(cursor.complete());
////    ASSERT_STREQ("human=(пол=, parent=[,])", (*cursor)->toString().c_str());
////
////    ASSERT_EQ(-1, cursor.reset());
////    ASSERT_STREQ("human=(пол=, parent=[,])", (*cursor)->toString().c_str());
////    cursor++;
////    ASSERT_FALSE(cursor.complete());
////    ASSERT_STREQ("&Tom=human(пол=\"male\", parent=[,])",
///(*cursor)->toString().c_str()); /    cursor++; /
/// ASSERT_FALSE(cursor.complete()); /
/// ASSERT_STREQ("&Janna=human(пол=\"female\", parent=[,])",
/// (*cursor)->toString().c_str()); /    cursor++; /
/// ASSERT_FALSE(cursor.complete()); /    ASSERT_STREQ("Jake=human(пол=\"male\",
/// parent=[&Tom,])", (*cursor)->toString().c_str()); /    cursor++; /
/// ASSERT_FALSE(cursor.complete()); /    ASSERT_STREQ("Tim=human(пол=\"male\",
/// parent=[&Tom, &Janna,])", (*cursor)->toString().c_str()); /    cursor++; /
/// ASSERT_TRUE(cursor.complete());
////
////
////    //    for (auto &elem : Tim) {
////    //        std::cout << elem.second->toString() << "\n";
////    //    }
////    auto citer = ctx.begin();
////    for (auto &elem : ctx) {
////        //        std::cout << elem.second->toString() << "\n";
////        ASSERT_STREQ(citer->second->toString().c_str(),
/// elem.second->toString().c_str()); /        citer++; /    } /
/// ASSERT_TRUE(citer == ctx.end());
////
////    //    int index = 0;
////    //    std::cout << "Global:\n";
////    //    for (auto &elem : *ctx.m_info.global) {
////    //        std::cout << elem.second->toString() << " - " << index++ <<
///"\n"; /    //    }
////
////    //    for (auto &elem : *ctx.m_info.global) {
////    //        std::cout << elem.second->toString() << "\n";
////    //    }
////
////
////
////
////    TermPtr ast;
////    Parser parser(ast);
////    parser.Parse("brother(arg1, arg2)");
////    ObjPtr filter_brother = Obj::CreateFunc(nullptr, ast,
/// ObjType::PUREFUNC); /    filter_brother->m_function = (void
///*)&test_brother; /    Object filt_args(Obj::Arg(filter_brother));
////
////
////    ASSERT_TRUE(Tim->op_class_test("human"));
////    ASSERT_TRUE(Jake->op_class_test("human"));
////
////    Object test_args_yes = *Obj::CreateDict(Obj::Arg(Tim),
/// Obj::Arg(Jake));
////
////    ASSERT_EQ(2, test_args_yes.size());
////    ObjPtr test_args_default = filter_brother->ConvertToArgs(test_args_yes,
/// true); /    ASSERT_EQ(2, test_args_yes.size());
////
////    ASSERT_NO_THROW(test_args_yes.CheckArgsValid());
////
////    test_args_yes.push_front(nullptr); // Нулевой аргумент
////    ObjPtr test_res = test_brother(nullptr, test_args_yes);
////    ASSERT_STREQ("@true", test_res->toString().c_str());
////    //    ASSERT_STREQ("[&Tom,]", test_res->toString().c_str());
////    ASSERT_TRUE(test_res->GetValueAsBoolean());
////
////
////    ObjPtr brothers = Obj::CreateDict();
////
////    auto terms = ctx.m_info.global->select();
////    auto humans = ctx.m_info.global->select("human");
////    int count = 0;
////    while(!terms.complete()) {
////        while(!humans.complete()) {
////
////            Object args_iter = *Obj::CreateDict(Obj::Arg(*terms),
/// Obj::Arg(*humans)); /            ObjPtr args_func =
/// filter_brother->ConvertToArgs(args_iter, true); /
/// ASSERT_NO_THROW(args_func->CheckArgsValid()); /
/// args_func->push_front(nullptr); // Нулевой аргумент
////
////
////            ObjPtr result = test_brother(nullptr, *args_func);
////            if(result->GetValueAsBoolean()) {
////                count++;
////                ASSERT_STREQ("@true", result->toString().c_str());
////                //                ASSERT_STREQ("[&Tom,]",
/// result->toString().c_str()); /                (*terms)->RefInc(); /
///(*humans)->RefInc(); /
/// brothers->push_back(Obj::CreateDict(Obj::Arg(*terms),
/// Obj::Arg(*humans))); /            } /            humans++; /        } /
/// humans.reset(); /        terms++; /    }
////
////    ASSERT_EQ(2, count);
////    ASSERT_EQ(2, brothers->size());
////
////    ASSERT_STREQ("[ [&Jake, &Tim,], [&Tim, &Jake,],]",
/// brothers->toString().c_str());
////
/// //
//    std::filesystem::create_directories("temp");
//    ASSERT_TRUE(std::filesystem::is_directory("temp"));
//
////    std::ofstream out("temp/brother.temp.sh");
////    out << "#!./dist/Debug/GNU-Linux/nlc --exec\n";
////    out << "\n";
////    //    out << "var := 100;\n";
////    //    out << "var2 := min(200, var, 400);\n";
////    out << "\n";
////    out << "#м := \"муж.\";\n";
////    out << "#ж := \"жен.\";\n";
////    out << "\n";
////    out << "human := (пол=, parent=[,]):human;\n";
////    out << "Tom := human(пол=м);\n";
////    out << "Janna := human(пол=ж);\n";
////    out << "Jake := @human(#м, [&Tom, &Janna,]);\n";
////    out << "Tim:=@human(пол=м, parent=[&Tom,]);\n";
////    out << "\n";
////    out << "brother(test1, test2) :&&= $test1!=$test2, $test1.пол==#м,
///$test1.parent * $test2.parent;\n"; /    out << "\n"; /    //    out <<
///"Tim.brother(Jake);\n"; /    //    out << "Tim.brother(human!)?;\n"; /    out
///<< "brother(human!, human!)?;\n";
////
////    //    out << "brother(h1, h2) :&&= $h1 != $h2, $h1.sex==male,
///@intersec($h1.parent, $h2.parent);\n"; /    //    out << "brother(h1=human!,
/// h2=human!)? -> @print("${h1.name} brother ${h2.name}\n");\n"; /    //    out
///<< "[h1=human!, h2=human!]? -> /    //               brother(h1, h2) -> / //
///@print("${h1.name} brother ${h2.name}\n");\n"; /    //    var? - массив func(
/// var! )? [test=var!!, func( var! )]? /    //    func()? /    // Итератор для
///функции вызывается пока не будет получен последний элемент (нужное количество
///элементов) /    // Итератор для объекта получает его свойства (первые, если
///задано количество) /    // @debug() /    // @print()
////
////    // @print("${h1.name} brother ${h2.name}\n");\n";
////    out.close();
////
////
////    RuntimePtr opts = NewLang::Init();
////    Context ctx2(opts);
/// //
//    std::filesystem::create_directories("temp");
//    ASSERT_TRUE(std::filesystem::is_directory("temp"));
//
////    //
////    //    ASSERT_TRUE(opts->CompileModule("temp/brother.temp.sh",
///"temp/brother.temp.sh.nlm")); /    //
/// ASSERT_TRUE(opts->LoadModule("temp/brother.temp.sh.nlm")); /    // /    //
/// EXPECT_STREQ("default arg", opts->Eval(&ctx,
///"native()")->GetValueAsString().c_str()); /    //    EXPECT_STREQ("default
/// arg", opts->Eval(&ctx, "@native()")->GetValueAsString().c_str()); /    //
/// EXPECT_STREQ("default arg", opts->Eval(&ctx,
///"@native()")->GetValueAsString().c_str()); /    //    EXPECT_STREQ("",
/// opts->Eval(&ctx, "@native(\"\")")->GetValueAsString().c_str()); /    //
/// EXPECT_STREQ("Hello, world!\n", opts->Eval(&ctx, "@native(\"Hello,
/// world!\\n\")")->GetValueAsString().c_str());
////
////    NLC run;
////    ObjPtr result = RunTime::Instance()->ExecModule("temp/brother.temp.sh",
///"temp/brother.sh.temp.nlm", false, &ctx2); /    ASSERT_TRUE(result); /
/// EXPECT_STREQ("[ [&Jake, &Tim,], [&Tim, &Jake,],]",
/// result->GetValueAsString().c_str()); /    //
/// std::remove("temp/brother.sh.temp.nlm");
////}
//
// TEST(NLC, Iterator) {
//
//    enum sex_ {
//        male,
//        female
//    };
//
//    struct human;
//    typedef std::vector<human *> parent_;
//
//    struct human {
//        const char *name;
//        sex_ sex;
//        parent_ parent;
//    };
//
//
//    human Tom{"Tom", male,
//        {}};
//    human Janna{"Janna", female,
//        {}};
//    human Jake{"Jake", male,
//        {&Tom, &Janna}};
//    human Tim{"Tim", male,
//        {&Tom}};
//
//    std::vector<human *> humans{&Tom, &Janna, &Jake, &Tim};
//
//
//    auto brothers = [](human * a, human * b) {
//
//        auto intersec = [](parent_ &a, parent_ & b) {
//            for (auto elem_a : a) {
//                for (auto elem_b : b) {
//                    if(elem_a && elem_b && elem_a == elem_b) {
//                        return true;
//                    }
//                }
//            }
//            return false;
//        };
//
//        return a && b && a != b && a->sex == male && b->sex == male &&
//        (intersec(a->parent, b->parent));
//    };
//
//    for (auto a : humans) {
//        for (auto b : humans) {
//            if(brothers(a, b)) {
//
//                std::cout << a->name << " and " << b->name << "\n";
//            }
//        }
//    }
//}
//
// TEST(NLC, SaveGlobal) {
//
//    //    RuntimePtr opts = NewLang::Init();
//    //    Context ctx(opts);
//    //
//    //    ASSERT_EQ(3, ctx.Current()->global->m_global_terms.size());
//    //    ASSERT_EQ(1, ctx.Current()->session.size());
//    //    EXPECT_STREQ("LOCAL", opts->Eval(&ctx, "local $=
//    \"LOCAL\"")->GetValueAsString().c_str());
//    //    ASSERT_EQ(3, ctx.Current()->global->m_global_terms.size());
//    //    ASSERT_EQ(2, ctx.Current()->session.size());
//    //    EXPECT_STREQ("Session", opts->Eval(&ctx, "session $=
//    \"Session\"")->GetValueAsString().c_str());
//    //    ASSERT_EQ(3, ctx.Current()->global->m_global_terms.size());
//    //    ASSERT_EQ(3, ctx.Current()->session.size());
//    //    EXPECT_STREQ("1", opts->Eval(&ctx, "global1 @=
//    1")->GetValueAsString().c_str());
//    //    ASSERT_EQ(4, ctx.Current()->global->m_global_terms.size());
//    //    ASSERT_EQ(3, ctx.Current()->session.size());
//    //    EXPECT_STREQ("2", opts->Eval(&ctx, "global2 @=
//    2")->GetValueAsString().c_str());
//    //    ASSERT_EQ(5, ctx.Current()->global->m_global_terms.size());
//    //    ASSERT_EQ(3, ctx.Current()->session.size());
//    //    EXPECT_STREQ("G", opts->Eval(&ctx, "global3 @=
//    \"G\"")->GetValueAsString().c_str());
//    //    ASSERT_EQ(6, ctx.Current()->global->m_global_terms.size());
//    //    ASSERT_EQ(3, ctx.Current()->session.size());
//    //
//    //    const char *funcs = "hello(str=\"\") := { %{ printf(\"%s\",
//    static_cast<char *>(*$1)); return $str; %} };"
//    //            "test_and(arg1, arg2) :&&= $arg1 == $arg2, $arg1;"
//    //            "test_or(arg1, arg2) :||= $arg1 == 555, $arg1;"
//    //            "test_xor(arg1, arg2) :^^= $arg1 == $arg2, $arg1;"
//    //            "native(str=\"default arg\") $= {%{  printf(\"%s\",
//    static_cast<char *>(*$str)); return $str; %}};"
//    //            "func_sum(arg1, arg2) :- {$arg1 + $arg2;};\n"
//    //            "func_call(arg1, arg2) :- {func_sum($arg1, $arg2);};";
//    //
//    //    ASSERT_EQ(0, ctx.Current()->global->m_global_funcs.size());
//    //
//    //    ASSERT_TRUE(opts->Eval(&ctx, funcs, true));
//    //
//    //    ASSERT_EQ(7, ctx.Current()->global->m_global_funcs.size());
//    //
//    //
// //
//    std::filesystem::create_directories("temp");
//    ASSERT_TRUE(std::filesystem::is_directory("temp"));
//
//    //    ASSERT_TRUE(opts->SaveContext("temp/ctx.temp.ctx"));
//
//    //    std::ofstream out("hello.sh");
//    //    out << "#!./dist/Debug/GNU-Linux/nlc --exec\n";
//    //    out << "print(str=\"\") := { %{ $$=$str; printf(\"%s\",
//    static_cast<char *>($str)); %} };\n";
//    //    out << cp1251_to_utf8("@print(\"Привет, мир!\\n\");\n");
//    //    out.close();
//    //
//    //    NLC run;
//    //    ASSERT_STREQ(cp1251_to_utf8("Привет, мир!\n").c_str(),
//    NewLang::ExecModule("hello.sh", "temp/hello.sh.temp.nlm",
//    run.m_ctx).c_str())
//    << cp1251_to_utf8("Привет, мир!\n");
//}
//
// TEST(NLC, UnloadModules) {
//    RuntimePtr opts = NewLang::Init();
//    Context ctx(opts);
//
// //
//    std::filesystem::create_directories("temp");
//    ASSERT_TRUE(std::filesystem::is_directory("temp"));
//
//    std::ofstream out("temp/native.temp.nlp");
//    out << "#!/bin/nlc\n";
//    out << "native(str=\"first\") := { %{ printf(\"%s\", static_cast<char
//    *>(*$str)); return $str; %} };\n"; out << "@native(\"Привет,
//    мир!\\n\");\n"; out.close();
//
//    ASSERT_TRUE(opts->CompileModule("temp/native.temp.nlp", "temp/native.temp.nlm"));
//    ASSERT_TRUE(RunTime::Instance()->LoadModule("temp/native.temp.nlm"));
//
////    EXPECT_STREQ("first", opts->Eval(&ctx,
///"native()")->GetValueAsString().c_str()); /    EXPECT_STREQ("first",
///opts->Eval(&ctx, "@native()")->GetValueAsString().c_str()); /
///EXPECT_STREQ("first", opts->Eval(&ctx,
///"native()")->GetValueAsString().c_str()); /    EXPECT_STREQ("",
///opts->Eval(&ctx, "@native(\"\")")->GetValueAsString().c_str()); /
///EXPECT_STREQ("Hello, world!\n", opts->Eval(&ctx, "@native(\"Hello,
///world!\\n\")")->GetValueAsString().c_str());
//
//
//
//    std::ofstream out2("temp/native.temp.nlp");
//    out2 << "#!/bin/nlc\n";
//    out2 << "native(str=\"second\") := { %{ printf(\"%s\", static_cast<char
//    *>(*$str)); return $str; %} };\n"; out2 << "//@native(\"Привет,
//    мир!\\n\");\n"; out2.close();
//
//    NLC compile2("/bin/nlc --compile=temp/native.temp.nlp");
//    ASSERT_STREQ("native.temp.nlp", compile2.m_ifile.c_str());
//    ASSERT_EQ(0, compile2.Run());
//
//
//    // Модуль уже другой (с другим значением по умолчанию), но при dlopen в
//    памяти остается загруженым еще первый модуль NLC exec("/bin/nlc
//    --load=temp/native.temp.nlm native()"); ASSERT_EQ(1, exec.m_modules.size());
//    ASSERT_STREQ("native()", exec.m_eval.c_str());
//    ASSERT_STREQ("temp/native.temp.nlm", exec.m_modules[0].c_str());
//    ASSERT_EQ(0, exec.Run());
//    ASSERT_STREQ("first", exec.m_output.c_str()); // Значение по умолчанию
//    функции из ПЕРВОГО МОДУЛЯ ! exec.m_ctx.Free(); // Нужно выгружать контекст
//    для осовобождения модуля (загруженного SO)
//
//
//    // Компилировать в GCC нужно с ключем --no-gnu-unique !!!!!!!!!!!!!!!!!!
//
//    // Выгрузили старый моудль
//    ASSERT_EQ(1, RunTime::Instance()->m_modules.size());
//    Module::FuncMap &temp =
//    RunTime::Instance()->m_modules["temp/native.temp.nlm"]->Funcs();
//    ASSERT_FALSE(temp.empty());
//    ASSERT_TRUE(RunTime::Instance()->UnLoadModule("temp/native.temp.nlm"));
//    ASSERT_TRUE(RunTime::Instance()->m_modules.empty());
//
//
////    RuntimePtr opts2 = NewLang::Init();
////    Context ctx2(opts2);
////    ASSERT_TRUE(opts2->LoadModule("temp/native.temp.nlm"));
////
////    EXPECT_STREQ("second", opts2->Eval(&ctx2,
///"native()")->GetValueAsString().c_str()); /    EXPECT_STREQ("second",
///opts2->Eval(&ctx2, "@native()")->GetValueAsString().c_str()); /
///EXPECT_STREQ("second", opts2->Eval(&ctx2,
///"native()")->GetValueAsString().c_str()); /    EXPECT_STREQ("",
///opts2->Eval(&ctx2, "@native(\"\")")->GetValueAsString().c_str());
////
////
////    NLC exec1("/bin/nlc --load=temp/native.temp.nlm native(\"print_test\")");
////    ASSERT_EQ(1, exec1.m_modules.size());
////    ASSERT_STREQ("temp/native.temp.nlm", exec1.m_modules[0].c_str());
////    ASSERT_EQ(0, exec1.Run());
////    ASSERT_STREQ("print_test", exec1.m_output.c_str());
//}

#endif // UNITTEST