#include "core/parser.h"
#ifdef UNITTEST

#include <core/pch.h>

#include <signal.h>

#include <core/warning_pop.h>
#include <core/warning_push.h>
#include <gtest/gtest.h>

#include <core/builtin.h>
#include <core/newlang.h>


TEST(Alg, Exception) {

//    Context ctx(RunTime::Init());
//
//    ObjPtr rand = ctx.Eval("rand := @import('rand():Int')");
//    // --
//    // -- "Ошибка" --
//    // -- ;# "Ошибка" --
//
//    ObjPtr tt = ctx.Eval("[[ rand(), ... ]]: Int[3,2]");
//    ASSERT_TRUE(tt);
//    ASSERT_TRUE(50 < tt->GetValueAsString().size()) << tt->GetValueAsString();
//
//    ASSERT_THROW(, parser_exception);
//    ASSERT_NO_THROW(
//
//            );
}


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


#endif // UNITTEST