#include "pch.h"

#ifdef BUILD_UNITTEST

#include "parser.h"

#include <signal.h>

#include <warning_pop.h>
#include <warning_push.h>
#include <gtest/gtest.h>

#include <builtin.h>
#include <runtime.h>

using namespace newlang;

TEST(Alg, Follow) {

    RuntimePtr rt = RunTime::Init();
    ASSERT_TRUE(rt);

    ObjPtr result = nullptr;

    ASSERT_NO_THROW(result = rt->Run("[1]-->100"));
    ASSERT_TRUE(result);
    ASSERT_EQ(100, result->GetValueAsInteger());

    ASSERT_NO_THROW(result = rt->Run("[-1]-->99"));
    ASSERT_TRUE(result);
    ASSERT_EQ(99, result->GetValueAsInteger());

    ASSERT_NO_THROW(result = rt->Run("[0]-->111"));
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->is_none_type());

    ASSERT_NO_THROW(result = rt->Run("[1]-->111"));
    ASSERT_TRUE(result);
    ASSERT_EQ(111, result->GetValueAsInteger());



    ASSERT_NO_THROW(result = rt->Run("[0]-->100,[1]-->300"));
    ASSERT_TRUE(result);
    ASSERT_EQ(300, result->GetValueAsInteger());

    ASSERT_NO_THROW(result = rt->Run("[0]-->100,[0]-->300,[99]-->99,[...]-->1000"));
    ASSERT_TRUE(result);
    ASSERT_EQ(99, result->GetValueAsInteger());

    ASSERT_NO_THROW(result = rt->Run("[0]-->111,[...]-->1000"));
    ASSERT_TRUE(result);
    ASSERT_EQ(1000, result->GetValueAsInteger());

    ASSERT_NO_THROW(result = rt->Run("[0]-->{100},[1]-->{10;20;30}"));
    ASSERT_TRUE(result);
    ASSERT_EQ(30, result->GetValueAsInteger());

    ASSERT_NO_THROW(result = rt->Run("[0]-->{100},[0]-->{300},[99]-->{99},[...]-->{1000}"));
    ASSERT_TRUE(result);
    ASSERT_EQ(99, result->GetValueAsInteger());

    ASSERT_NO_THROW(result = rt->Run("[0]-->111,[...]-->{30;50;1000}"));
    ASSERT_TRUE(result);
    ASSERT_EQ(1000, result->GetValueAsInteger());


    ASSERT_NO_THROW(result = rt->Run("cond := 100; [cond]-->111,[...]-->1000"));
    ASSERT_TRUE(result);
    ASSERT_EQ(111, result->GetValueAsInteger());

    ASSERT_NO_THROW(result = rt->Run("cond2 := 0; [cond2]-->111,[...]-->1000"));
    ASSERT_TRUE(result);
    ASSERT_EQ(1000, result->GetValueAsInteger());
}

TEST(Alg, Repeat) {

    RuntimePtr rt = RunTime::Init();
    ASSERT_TRUE(rt);

    ObjPtr result = nullptr;

    ASSERT_NO_THROW(result = rt->Run("result := 0"));
    ASSERT_TRUE(result);
    ASSERT_EQ(0, result->GetValueAsInteger());

    ObjPtr cond;
    ASSERT_NO_THROW(cond = rt->Run("result < 4"));
    ASSERT_TRUE(cond);
    ASSERT_TRUE(cond->GetValueAsBoolean());

    ASSERT_NO_THROW(result = rt->Run("result += 1"));
    ASSERT_TRUE(result);
    ASSERT_EQ(1, result->GetValueAsInteger());

    ASSERT_NO_THROW(cond = rt->Run("result < 4"));
    ASSERT_TRUE(cond);
    ASSERT_TRUE(cond->GetValueAsBoolean());

    ASSERT_NO_THROW(result = rt->Run("result += 3"));
    ASSERT_TRUE(result);
    ASSERT_EQ(4, result->GetValueAsInteger());

    ASSERT_NO_THROW(cond = rt->Run("result < 4"));
    ASSERT_TRUE(cond);
    ASSERT_FALSE(cond->GetValueAsBoolean());


    ObjPtr counter;
    ASSERT_NO_THROW(counter = rt->Run("counter := 0"));
    ASSERT_EQ(0, counter->GetValueAsInteger());

    ASSERT_EQ(4, result->GetValueAsInteger());
    ASSERT_NO_THROW(result = rt->Run("result -= 1"));
    ASSERT_EQ(3, result->GetValueAsInteger());

    ASSERT_NO_THROW(counter = rt->Run("[result -= 1] <-> {counter += 1}"));
    ASSERT_TRUE(counter);
    ASSERT_EQ(2, counter->GetValueAsInteger());


    ASSERT_NO_THROW(counter = rt->Run("{result += 1; counter += 1} <-> [result < 10]"));
    ASSERT_EQ(10, result->GetValueAsInteger());

    ASSERT_TRUE(counter);
    ASSERT_EQ(12, counter->GetValueAsInteger());

}

//TEST(Alg, Foreach) {
//
//    Context ctx(RunTime::Init());
//
//    ObjPtr test = Obj::CreateDict();
//
//    test->push_back(Obj::Arg(10));
//    test->push_back(Obj::Arg(11));
//    test->push_back(Obj::Arg(12));
//    test->push_back(Obj::Arg(13));
//    ASSERT_EQ(4, test->size());
//    ASSERT_EQ(10, (*test)[0].second->GetValueAsInteger());
//    ASSERT_EQ(13, (*test)[3].second->GetValueAsInteger());
//
//    ASSERT_EQ(test->resize_(4, nullptr), test->size());
//    ASSERT_EQ(4, test->size());
//    ASSERT_EQ(10, (*test)[0].second->GetValueAsInteger());
//    ASSERT_EQ(13, (*test)[3].second->GetValueAsInteger());
//
//    test->resize_(-3, nullptr);
//    ASSERT_EQ(3, test->size());
//    ASSERT_EQ(11, (*test)[0].second->GetValueAsInteger());
//    ASSERT_EQ(13, (*test)[2].second->GetValueAsInteger());
//
//    test->resize_(-1, nullptr);
//    ASSERT_EQ(1, test->size());
//    ASSERT_EQ(13, (*test)[0].second->GetValueAsInteger());
//
//    test->resize_(0, nullptr);
//    ASSERT_EQ(0, test->size());
//
//
//    ObjPtr counter = ctx.ExecStr("counter := 100"));
//    ASSERT_EQ(100, counter->GetValueAsInteger());
//
//    ObjPtr dict = ctx.ExecStr("dict := (1,2,3,)"));
//    ASSERT_TRUE(dict);
//    ASSERT_TRUE(dict->is_dictionary_type());
//    ASSERT_EQ(3, dict->size());
//    ASSERT_EQ(1, (*dict)[0].second->GetValueAsInteger());
//    ASSERT_EQ(3, (*dict)[2].second->GetValueAsInteger());
//
//    ObjPtr temp = ctx.ExecStr("counter, dict := ... dict"));
//    ASSERT_TRUE(temp);
//    ASSERT_TRUE(temp->is_dictionary_type());
//    ASSERT_EQ(2, temp->size());
//
//    ASSERT_EQ(1, counter->GetValueAsInteger());
//
//    ASSERT_TRUE(dict->is_dictionary_type());
//    ASSERT_EQ(2, dict->size());
//    ASSERT_EQ(2, (*dict)[0].second->GetValueAsInteger());
//    ASSERT_EQ(3, (*dict)[1].second->GetValueAsInteger());
//
//    temp = ctx.ExecStr("counter, dict := ... dict"));
//    ASSERT_TRUE(temp);
//    ASSERT_TRUE(temp->is_dictionary_type());
//    ASSERT_EQ(1, temp->size());
//
//    ASSERT_EQ(2, counter->GetValueAsInteger());
//
//    ASSERT_TRUE(dict->is_dictionary_type());
//    ASSERT_EQ(1, dict->size());
//    ASSERT_EQ(3, (*dict)[0].second->GetValueAsInteger());
//
//    temp = ctx.ExecStr("counter, dict := ... dict"));
//    ASSERT_TRUE(temp);
//    ASSERT_TRUE(temp->is_dictionary_type());
//    ASSERT_EQ(0, temp->size());
//
//    ASSERT_TRUE(counter);
//    ASSERT_EQ(3, counter->GetValueAsInteger());
//
//    ASSERT_TRUE(dict->is_dictionary_type());
//    ASSERT_EQ(0, dict->size());
//
//    temp = ctx.ExecStr("counter, dict := ... dict"));
//    ASSERT_TRUE(temp);
//    ASSERT_TRUE(temp->is_dictionary_type());
//    ASSERT_TRUE(dict->is_dictionary_type());
//    ASSERT_EQ(0, dict->size());
//
//    ASSERT_TRUE(counter);
//    ASSERT_TRUE(counter->is_none_type());
//
//
//    dict->push_back(Obj::Arg(10));
//    dict->push_back(Obj::Arg(20));
//    dict->push_back(Obj::Arg(30));
//    dict->push_back(Obj::Arg(40));
//    dict->push_back(Obj::Arg(50));
//    ASSERT_EQ(5, dict->size());
//
//    temp = ctx.ExecStr("dict"));
//    ASSERT_TRUE(temp);
//    ASSERT_TRUE(temp->is_dictionary_type());
//    ASSERT_FALSE(temp->is_scalar());
//    ASSERT_TRUE(temp->GetValueAsBoolean());
//    ASSERT_EQ(5, temp->size());
//
//    temp = ctx.ExecStr(":Bool(dict)"));
//    ASSERT_TRUE(temp);
//    ASSERT_TRUE(temp->is_bool_type());
//    ASSERT_FALSE(temp->is_scalar());
//    ASSERT_TRUE(temp->GetValueAsBoolean());
//    ASSERT_EQ(5, temp->size());
//
//    temp = ctx.ExecStr(":Bool[0](dict)"));
//    ASSERT_TRUE(temp);
//    ASSERT_TRUE(temp->is_bool_type());
//    ASSERT_TRUE(temp->is_scalar());
//    ASSERT_TRUE(temp->GetValueAsBoolean());
//    ASSERT_EQ(0, temp->size());
//
//    //    temp = ctx.ExecStr(":Bool(!dict)"));
//    //    ASSERT_TRUE(temp);
//    //    ASSERT_TRUE(temp->is_bool_type());
//    //    ASSERT_FALSE(temp->GetValueAsBoolean());
//
//    ObjPtr summa = ctx.ExecStr("summa := 0"));
//
//    temp = ctx.ExecStr("counter = 0; [dict] <-> {counter, dict := ... dict; summa += counter}"));
//    ASSERT_TRUE(temp);
//    ASSERT_TRUE(temp->is_integer());
//    ASSERT_EQ(150, temp->GetValueAsInteger());
//
//    ASSERT_TRUE(dict->is_dictionary_type());
//    ASSERT_EQ(0, dict->size());
//
//    ASSERT_EQ(150, summa->GetValueAsInteger());
//    ASSERT_EQ(50, counter->GetValueAsInteger());
//
//    summa = ctx.ExecStr("summa := 0"));
//    dict = ctx.ExecStr("dict := (1,2,3,4,5,)"));
//    temp = ctx.ExecStr("item := 0; [dict] <-> {item, dict := ... dict; summa += item}"));
//    ASSERT_TRUE(temp);
//    ASSERT_TRUE(temp->is_integer());
//    ASSERT_EQ(15, temp->GetValueAsInteger());
//    ASSERT_TRUE(dict->is_dictionary_type());
//    ASSERT_EQ(0, dict->size());
//    ASSERT_EQ(15, summa->GetValueAsInteger());
//
//
//
//
//
//    counter = ctx.ExecStr("counter := 55"));
//    ASSERT_EQ(55, counter->GetValueAsInteger());
//
//    ObjPtr tensor = ctx.ExecStr("tensor := [10,11,12,]"));
//    ASSERT_TRUE(tensor);
//    ASSERT_TRUE(tensor->is_tensor_type());
//    ASSERT_TRUE(tensor->GetValueAsBoolean());
//    ASSERT_EQ(3, tensor->size());
//    ASSERT_EQ(10, (*tensor)[0].second->GetValueAsInteger());
//    ASSERT_EQ(11, (*tensor)[1].second->GetValueAsInteger());
//    ASSERT_EQ(12, (*tensor)[2].second->GetValueAsInteger());
//
//    temp = ctx.ExecStr("counter, tensor  := ... tensor "));
//    ASSERT_TRUE(temp);
//    ASSERT_TRUE(temp->is_tensor_type());
//    ASSERT_EQ(2, temp->size());
//
//    ASSERT_EQ(10, counter->GetValueAsInteger());
//
//    ASSERT_TRUE(tensor->is_tensor_type());
//    ASSERT_TRUE(tensor->GetValueAsBoolean());
//    ASSERT_EQ(2, tensor ->size());
//    ASSERT_EQ(11, (*tensor)[0].second->GetValueAsInteger());
//    ASSERT_EQ(12, (*tensor)[1].second->GetValueAsInteger());
//
//    temp = ctx.ExecStr("counter, tensor := ... tensor "));
//    ASSERT_TRUE(temp);
//    ASSERT_TRUE(temp->is_tensor_type());
//    ASSERT_EQ(1, temp->size());
//
//    ASSERT_EQ(11, counter->GetValueAsInteger());
//
//    ASSERT_TRUE(tensor->is_tensor_type());
//    ASSERT_TRUE(tensor->GetValueAsBoolean());
//    ASSERT_EQ(1, tensor ->size());
//    ASSERT_EQ(12, (*tensor)[0].second->GetValueAsInteger());
//
//    temp = ctx.ExecStr("counter, tensor := ... tensor "));
//    ASSERT_TRUE(temp);
//    ASSERT_TRUE(temp->is_tensor_type());
//    ASSERT_EQ(0, temp->size());
//
//    ASSERT_EQ(12, counter->GetValueAsInteger());
//
//    ASSERT_TRUE(tensor->is_tensor_type());
//    ASSERT_FALSE(tensor->GetValueAsBoolean());
//    ASSERT_EQ(0, tensor ->size());
//
//    temp = ctx.ExecStr("counter, tensor := ... tensor"));
//    ASSERT_TRUE(temp);
//    ASSERT_TRUE(temp->is_tensor_type());
//    ASSERT_TRUE(temp->empty());
//    ASSERT_FALSE(tensor->GetValueAsBoolean());
//    ASSERT_TRUE(tensor->is_tensor_type());
//    ASSERT_TRUE(tensor->empty());
//
//    ASSERT_TRUE(counter);
//    ASSERT_TRUE(counter->empty());
//
//
//
//    ObjPtr summa2 = ctx.ExecStr("summa := 0"));
//    ObjPtr tensor2 = ctx.ExecStr("tensor := [10,20,30,40,50,60,]"));
//    ObjPtr temp2 = ctx.ExecStr("item2 := 0; [tensor] <-> {item2, tensor := ... tensor; summa += item2}"));
//    ASSERT_TRUE(temp2);
//    ASSERT_TRUE(temp2->is_integer());
//    ASSERT_EQ(210, temp2->GetValueAsInteger());
//    ASSERT_EQ(210, summa2->GetValueAsInteger());
//    ASSERT_TRUE(tensor2->is_tensor_type());
//    ASSERT_FALSE(tensor2->GetValueAsBoolean());
//
//    ObjPtr tensor_size = ctx.ExecStr("tensor_size := [10,20,30,40,]"));
//    ASSERT_EQ(4, tensor_size->size());
//    ASSERT_EQ(10, (*tensor_size)[0].second->GetValueAsInteger());
//    ASSERT_EQ(40, (*tensor_size)[3].second->GetValueAsInteger());
//
//    tensor_size->resize_(-10, nullptr);
//    ASSERT_EQ(10, tensor_size->size());
//    ASSERT_EQ(0, (*tensor_size)[0].second->GetValueAsInteger());
//    ASSERT_EQ(0, (*tensor_size)[5].second->GetValueAsInteger());
//    ASSERT_EQ(10, (*tensor_size)[6].second->GetValueAsInteger());
//    ASSERT_EQ(40, (*tensor_size)[9].second->GetValueAsInteger());
//
//}
//
//
//TEST(Alg, BreakContinue) {
//    Context::Reset();
//    Context ctx(RunTime::Init());
//
//    const char * run_raw = ""
//            "count:=5;"
//            "[count<10]<->{+"
//            "  [count>5]-->"
//            "    ++100++;"
//            "  ; "
//            "  count+=1;"
//            "+};"
//            ;
//
//    ObjPtr ASSERT_NO_THROW(result = rt->Run(run_raw, nullptr, Context::CatchType::CATCH_ALL);
//            ASSERT_TRUE(result);
//            ASSERT_TRUE(result->is_integer());
//            ASSERT_EQ(6, count->GetValueAsInteger());
//            ASSERT_EQ(100, result->GetValueAsInteger());
//
//            const char * run_macro = ""
//            "count:=5;"
//            "@while(count<10){+"
//            "  @if(count>5){"
//            "    @return(42);"
//            "  };"
//            "  count+=1;"
//            "+};"
//            "";
//
//
//
//            ASSERT_NO_THROW(result = rt->Run(run_macro, nullptr, Context::CatchType::CATCH_ALL);
//            ASSERT_TRUE(result);
//            ASSERT_TRUE(result->is_integer());
//            ASSERT_EQ(6, count->GetValueAsInteger());
//            ASSERT_EQ(42, result->GetValueAsInteger());
//}

TEST(Alg, Else) {

    RuntimePtr rt = RunTime::Init();
    ASSERT_TRUE(rt);

    ObjPtr result = nullptr;

    ASSERT_NO_THROW(result = rt->Run("42"));
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->is_integer()) << result->toString().c_str() << " (" << toString(result->getType()) << ")";
    ASSERT_EQ(42, result->GetValueAsInteger()) << result->toString().c_str();

    ASSERT_NO_THROW(result = rt->Run("[0]<->{ 43 },[...]-->{44}"));
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->is_integer()) << result->toString().c_str() << " (" << toString(result->getType()) << ")";
    ASSERT_EQ(44, result->GetValueAsInteger()) << result->toString().c_str();

    ASSERT_NO_THROW(result = rt->Run("{- --100-- -}"));
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->is_integer()) << result->toString().c_str() << " (" << toString(result->getType()) << ")";
    ASSERT_EQ(100, result->GetValueAsInteger()) << result->toString().c_str();

    //    ASSERT_NO_THROW(result = rt->Run("{- 100 -},[...]-->{200}"));
    //    ASSERT_TRUE(result);
    //    ASSERT_TRUE(result->is_integer()) << result->toString().c_str() << " (" << toString(result->getType()) << ")";
    //    ASSERT_EQ(200, result->GetValueAsInteger()) << result->toString().c_str();
    //
    //    ASSERT_NO_THROW(result = rt->Run("{+ 100 +},[...]-->{201}"));
    //    ASSERT_TRUE(result);
    //    ASSERT_TRUE(result->is_integer()) << result->toString().c_str() << " (" << toString(result->getType()) << ")";
    //    ASSERT_EQ(201, result->GetValueAsInteger()) << result->toString().c_str();
    //
    //    ASSERT_NO_THROW(result = rt->Run("{* 100 *},[...]-->{202}"));
    //    ASSERT_TRUE(result);
    //    ASSERT_TRUE(result->is_integer()) << result->toString().c_str() << " (" << toString(result->getType()) << ")";
    //    ASSERT_EQ(202, result->GetValueAsInteger()) << result->toString().c_str();
    //
    //    ASSERT_NO_THROW(result = rt->Run("{- 100 -},[...]-->{- 204 -}"));
    //    ASSERT_TRUE(result);
    //    ASSERT_TRUE(result->is_integer()) << result->toString().c_str() << " (" << toString(result->getType()) << ")";
    //    ASSERT_EQ(204, result->GetValueAsInteger()) << result->toString().c_str();
    //
    //    ASSERT_NO_THROW(result = rt->Run("{- 100 -},[...]-->{+ 205 +}"));
    //    ASSERT_TRUE(result);
    //    ASSERT_TRUE(result->is_integer()) << result->toString().c_str() << " (" << toString(result->getType()) << ")";
    //    ASSERT_EQ(205, result->GetValueAsInteger()) << result->toString().c_str();
    //
    //    ASSERT_NO_THROW(result = rt->Run("{* 100 *},[...]-->{* 206 *}"));
    //    ASSERT_TRUE(result);
    //    ASSERT_TRUE(result->is_integer()) << result->toString().c_str() << " (" << toString(result->getType()) << ")";
    //    ASSERT_EQ(206, result->GetValueAsInteger()) << result->toString().c_str();
    //
    //    ASSERT_NO_THROW(result = rt->Run("{* 100 *}:Test, [...]-->{* 207 *}"));
    //    ASSERT_TRUE(result);
    //    ASSERT_TRUE(result->is_integer()) << result->toString().c_str() << " (" << toString(result->getType()) << ")";
    //    ASSERT_EQ(207, result->GetValueAsInteger()) << result->toString().c_str();
    //
    //    ASSERT_NO_THROW(result = rt->Run("[1] --> 1, [1] --> 2, [...] -->3"));
    //    ASSERT_TRUE(result);
    //    ASSERT_TRUE(result->is_integral()) << result->toString().c_str() << " (" << toString(result->getType()) << ")";
    //    ASSERT_EQ(1, result->GetValueAsInteger()) << result->toString().c_str();
    //
    //    ASSERT_NO_THROW(result = rt->Run("[0] --> 1, [1] --> 2, [...] -->3"));
    //    ASSERT_TRUE(result);
    //    ASSERT_TRUE(result->is_integral()) << result->toString().c_str() << " (" << toString(result->getType()) << ")";
    //    ASSERT_EQ(2, result->GetValueAsInteger()) << result->toString().c_str();
    //
    //    ASSERT_NO_THROW(result = rt->Run("[0] --> 1, [0] --> 2, [...] -->3"));
    //    ASSERT_TRUE(result);
    //    ASSERT_TRUE(result->is_integral()) << result->toString().c_str() << " (" << toString(result->getType()) << ")";
    //    ASSERT_EQ(3, result->GetValueAsInteger()) << result->toString().c_str();
}

TEST(Alg, Return) {

    RuntimePtr rt = RunTime::Init();
    ASSERT_TRUE(rt);

    ObjPtr result = nullptr;
    ObjPtr value = nullptr;

    try {
        ASSERT_ANY_THROW(result = rt->Run("--"));
    } catch (IntMinus &except) {
        ASSERT_TRUE(except.m_obj);
        ASSERT_EQ(ObjType::RetMinus, except.m_obj->getType()) << newlang::toString(except.m_obj->getType());
        ASSERT_TRUE(except.m_obj->m_return_obj);
        ASSERT_TRUE(except.m_obj->m_return_obj->is_none_type());
    }
    ASSERT_FALSE(result);

    try {
        ASSERT_ANY_THROW(result = rt->Run(":: --"));
    } catch (IntAny &except) {
        ASSERT_TRUE(except.m_obj);
        ASSERT_EQ(ObjType::RetMinus, except.m_obj->getType()) << newlang::toString(result->getType());
        ASSERT_STREQ("::", except.m_obj->m_value.c_str());
        ASSERT_TRUE(except.m_obj->m_return_obj);
        ASSERT_TRUE(except.m_obj->m_return_obj->is_none_type());
    }
    result = nullptr;

    try {
        ASSERT_ANY_THROW(result = rt->Run("--100--"));
    } catch (IntMinus &except) {
        ASSERT_TRUE(except.m_obj);
        ASSERT_EQ(ObjType::RetMinus, except.m_obj->getType()) << newlang::toString(except.m_obj->getType());
        ASSERT_TRUE(except.m_obj->m_return_obj);
        ASSERT_STREQ("100", except.m_obj->m_return_obj->toString().c_str());
    }
    ASSERT_FALSE(result);

    try {
        ASSERT_ANY_THROW(result = rt->Run(":: --100--"));
    } catch (IntAny &except) {
        ASSERT_TRUE(except.m_obj);
        ASSERT_EQ(ObjType::RetPlus, except.m_obj->getType()) << newlang::toString(except.m_obj->getType());
        ASSERT_STREQ("::", except.m_obj->m_value.c_str());
        ASSERT_TRUE(except.m_obj->m_return_obj);
        ASSERT_STREQ("100", except.m_obj->m_return_obj->toString().c_str());
    }
    result = nullptr;



    try {
        ASSERT_ANY_THROW(result = rt->Run("++"));
    } catch (IntPlus &except) {
        ASSERT_TRUE(except.m_obj);
        ASSERT_EQ(ObjType::RetPlus, except.m_obj->getType()) << newlang::toString(except.m_obj->getType());
        ASSERT_STREQ("::", except.m_obj->m_value.c_str());
        ASSERT_TRUE(except.m_obj->m_return_obj);
        ASSERT_TRUE(except.m_obj->m_return_obj->is_none_type());
    }
    ASSERT_FALSE(result);

    try {
        ASSERT_ANY_THROW(result = rt->Run(":: ++"));
    } catch (IntAny &except) {
        ASSERT_TRUE(except.m_obj);
        ASSERT_STREQ("::", except.m_obj->m_value.c_str());
        ASSERT_EQ(ObjType::RetPlus, except.m_obj->getType()) << newlang::toString(except.m_obj->getType());
        ASSERT_TRUE(except.m_obj->m_return_obj);
        ASSERT_TRUE(except.m_obj->m_return_obj->is_none_type());
    }
    result = nullptr;

    try {
        ASSERT_ANY_THROW(result = rt->Run("++100++"));
    } catch (IntPlus &except) {
        ASSERT_TRUE(except.m_obj);
        ASSERT_EQ(ObjType::RetPlus, except.m_obj->getType()) << newlang::toString(except.m_obj->getType());
        ASSERT_TRUE(except.m_obj->m_return_obj);
        ASSERT_STREQ("100", except.m_obj->m_return_obj->toString().c_str());
    }
    ASSERT_FALSE(result);

    try {
        ASSERT_ANY_THROW(result = rt->Run(":: ++100++"));
    } catch (IntAny &except) {
        ASSERT_TRUE(except.m_obj);
        ASSERT_EQ(ObjType::RetPlus, except.m_obj->getType()) << newlang::toString(except.m_obj->getType());
        ASSERT_TRUE(except.m_obj->m_return_obj);
        ASSERT_STREQ("100", except.m_obj->m_return_obj->toString().c_str());
    }
    result = nullptr;





    try {
        ASSERT_ANY_THROW(result = rt->Run("{ -- }"));
    } catch (IntMinus &except) {
        ASSERT_TRUE(except.m_obj);
        ASSERT_EQ(ObjType::RetMinus, except.m_obj->getType()) << newlang::toString(except.m_obj->getType());
        ASSERT_TRUE(except.m_obj->m_return_obj);
        ASSERT_TRUE(except.m_obj->m_return_obj->is_none_type());
    }
    ASSERT_FALSE(result);

    try {
        ASSERT_NO_THROW(result = rt->Run("ns{ ns ++ }"));
        ASSERT_TRUE(result);
        ASSERT_TRUE(result->is_none_type());
    } catch (IntAny &except) {
        ASSERT_TRUE(0);
    }
    result = nullptr;

    try {
        ASSERT_NO_THROW(result = rt->Run("ns{ { ns ++ } }"));
        ASSERT_TRUE(result);
        ASSERT_TRUE(result->is_none_type());
    } catch (IntAny &except) {
        ASSERT_TRUE(0);
    }
    result = nullptr;


    try {
        ASSERT_ANY_THROW(result = rt->Run("{ ++100++; 200 }"));
    } catch (IntPlus &except) {
        ASSERT_TRUE(except.m_obj);
        ASSERT_EQ(ObjType::RetMinus, except.m_obj->getType()) << newlang::toString(except.m_obj->getType());
        ASSERT_TRUE(except.m_obj->m_return_obj);
        ASSERT_STREQ("100", except.m_obj->m_return_obj->toString().c_str());
    }
    ASSERT_FALSE(result);

    try {
        ASSERT_NO_THROW(result = rt->Run("ns { ns ++100++; 200}"));
        ASSERT_TRUE(result);
        ASSERT_STREQ("100", result->toString().c_str());
    } catch (IntAny &except) {
        ASSERT_TRUE(0);
    }
    result = nullptr;

    try {
        ASSERT_NO_THROW(result = rt->Run("br:= 5; ns { br+=1; [br>10]--> ns:: ++ br ++; ns:: --br--; 200 }"));
        ASSERT_TRUE(result);
        ASSERT_STREQ("11", result->toString().c_str());
    } catch (IntAny &except) {
        ASSERT_TRUE(0);
    }
    result = nullptr;





    //    try {
    //        ASSERT_NO_THROW(result = rt->Run("{ ++100++ }"));
    //    } catch (Return &except) {
    //        ASSERT_TRUE(except.m_obj);
    //        ASSERT_EQ(ObjType::RetPlus, except.m_obj->getType());
    //        ASSERT_TRUE(except.m_obj->m_return_obj);
    //        ASSERT_STREQ("100", except.m_obj->m_return_obj->toString().c_str());
    //    }
    //    ASSERT_FALSE(result);
    //
    //    try {
    //        ASSERT_NO_THROW(result = rt->Run("{- ++100++ -}"));
    //    } catch (Return &except) {
    //        ASSERT_TRUE(except.m_obj);
    //        ASSERT_EQ(ObjType::RetPlus, except.m_obj->getType());
    //        ASSERT_TRUE(except.m_obj->m_return_obj);
    //        ASSERT_STREQ("100", except.m_obj->m_return_obj->toString().c_str());
    //    }
    //    ASSERT_FALSE(result);
    //
    //    try {
    //        ASSERT_NO_THROW(result = rt->Run("--'Тест'--"));
    //    } catch (Return &except) {
    //        ASSERT_TRUE(except.m_obj);
    //        ASSERT_EQ(ObjType::RetMinus, except.m_obj->getType());
    //        ASSERT_TRUE(except.m_obj->m_return_obj);
    //        ASSERT_STREQ("'Тест'", except.m_obj->m_return_obj->toString().c_str());
    //    }
    //    ASSERT_FALSE(result);
    //
    //    try {
    //        ASSERT_NO_THROW(result = rt->Run("--:Int32--"));
    //    } catch (Return &except) {
    //        ASSERT_TRUE(except.m_obj);
    //        ASSERT_EQ(ObjType::RetMinus, except.m_obj->getType());
    //        ASSERT_TRUE(except.m_obj->m_return_obj);
    //        ASSERT_EQ(ObjType::Type, except.m_obj->m_return_obj->getType()) << toString(except.m_obj->m_return_obj->getType());
    //        ASSERT_EQ(ObjType::Int32, except.m_obj->m_return_obj->m_var_type_fixed) << toString(except.m_obj->m_return_obj->m_var_type_fixed);
    //    }
    //    ASSERT_FALSE(result);
    //
    //    try {
    //        ASSERT_NO_THROW(result = rt->Run("{ --:Int32-- }"));
    //    } catch (Return &except) {
    //        ASSERT_TRUE(except.m_obj);
    //        ASSERT_EQ(ObjType::RetMinus, except.m_obj->getType());
    //        ASSERT_TRUE(except.m_obj->m_return_obj);
    //        ASSERT_EQ(ObjType::Type, except.m_obj->m_return_obj->getType()) << toString(except.m_obj->m_return_obj->getType());
    //        ASSERT_EQ(ObjType::Int32, except.m_obj->m_return_obj->m_var_type_fixed) << toString(except.m_obj->m_return_obj->m_var_type_fixed);
    //    }
    //    ASSERT_FALSE(result);
    //
    //    try {
    //        ASSERT_NO_THROW(result = rt->Run("{+ --:Int32-- +}"));
    //    } catch (Return &except) {
    //        ASSERT_TRUE(except.m_obj);
    //        ASSERT_EQ(ObjType::RetMinus, except.m_obj->getType());
    //        ASSERT_TRUE(except.m_obj->m_return_obj);
    //        ASSERT_EQ(ObjType::Type, except.m_obj->m_return_obj->getType()) << toString(except.m_obj->m_return_obj->getType());
    //        ASSERT_EQ(ObjType::Int32, except.m_obj->m_return_obj->m_var_type_fixed) << toString(except.m_obj->m_return_obj->m_var_type_fixed);
    //    }
    //    ASSERT_FALSE(result);
    //
    //    try {
    //        ASSERT_NO_THROW(result = rt->Run("--:Error()--"));
    //    } catch (Return &except) {
    //        ASSERT_TRUE(except.m_obj);
    //        ASSERT_EQ(ObjType::RetMinus, except.m_obj->getType());
    //        ASSERT_TRUE(except.m_obj->m_return_obj);
    //        ASSERT_EQ(ObjType::Error, except.m_obj->m_return_obj->getType());
    //        ASSERT_STREQ(":Error()", except.m_obj->m_return_obj->toString().c_str()) << except.m_obj->m_return_obj->toString();
    //    }
    //    ASSERT_FALSE(result);
    //
    //
    //    try {
    //        ASSERT_NO_THROW(result = rt->Run("--:Error('ТЕКСТ')--"));
    //    } catch (Return &except) {
    //        ASSERT_TRUE(except.m_obj);
    //        ASSERT_EQ(ObjType::RetMinus, except.m_obj->getType());
    //        ASSERT_TRUE(except.m_obj->m_return_obj);
    //        ASSERT_STREQ(":RetMinus(:Error('ТЕКСТ'))", except.m_obj->toString().c_str());
    //        ASSERT_STREQ(":Error('ТЕКСТ')", except.m_obj->m_return_obj->toString().c_str());
    //    }
    //    ASSERT_FALSE(result);
    //
    //
    //    try {
    //        ASSERT_NO_THROW(result = rt->Run("[1]--> ++100++"));
    //    } catch (Return &except) {
    //        ASSERT_TRUE(except.m_obj);
    //        ASSERT_EQ(ObjType::RetPlus, except.m_obj->getType()) << toString(except.m_obj->getType());
    //        ASSERT_TRUE(except.m_obj->m_return_obj);
    //        ASSERT_STREQ("100", except.m_obj->m_return_obj->toString().c_str());
    //    }
    //    ASSERT_FALSE(result);
    //
    //    try {
    //        ASSERT_NO_THROW(result = rt->Run("[1]-->{ ++100++ }"));
    //    } catch (Return &except) {
    //        ASSERT_TRUE(except.m_obj);
    //        ASSERT_EQ(ObjType::RetPlus, except.m_obj->getType());
    //        ASSERT_TRUE(except.m_obj->m_return_obj);
    //        ASSERT_STREQ("100", except.m_obj->m_return_obj->toString().c_str());
    //    }
    //    ASSERT_FALSE(result);
    //
    //    try {
    //        ASSERT_NO_THROW(result = rt->Run("[1]-->{- ++100++ -}"));
    //    } catch (Return &except) {
    //        ASSERT_TRUE(except.m_obj);
    //        ASSERT_EQ(ObjType::RetPlus, except.m_obj->getType());
    //        ASSERT_TRUE(except.m_obj->m_return_obj);
    //        ASSERT_STREQ("100", except.m_obj->m_return_obj->toString().c_str());
    //    }
    //    ASSERT_FALSE(result);
    //
    //
    //    result.reset();
    //    ASSERT_NO_THROW(ASSERT_NO_THROW(result = rt->Run("{count := 1; count += 1; count += 1; count += 1; count += 1;};")););
    //    ASSERT_TRUE(result);
    //    ASSERT_TRUE(result->is_integer()) << toString(result->getType());
    //    ASSERT_EQ(5, result->GetValueAsInteger());
    //
    //    result.reset();
    //    ASSERT_NO_THROW(ASSERT_NO_THROW(result = rt->Run("{count := 1; count += 1; count += 1; count += 1; count += 1;}; 99")););
    //    ASSERT_TRUE(result);
    //    ASSERT_TRUE(result->is_integer()) << result->toString().c_str();
    //    ASSERT_EQ(99, result->GetValueAsInteger());
    //
    //    result.reset();
    //    ASSERT_ANY_THROW(ASSERT_NO_THROW(result = rt->Run("{count := 1; count += 1; count += 1; --55--; count += 1; count += 1;}; 5555")););
    //    ASSERT_FALSE(result);
    //
    //    result.reset();
    //    ASSERT_NO_THROW(ASSERT_NO_THROW(result = rt->Run("{- count := 1; count += 1; count += 1; --77--; count += 1; count += 1; -}", nullptr, Context::CatchType::CATCH_ALL););
    //            ASSERT_TRUE(result);
    //            ASSERT_TRUE(result->is_integer()) << result->toString().c_str();
    //            ASSERT_EQ(77, result->GetValueAsInteger());
    //
    //            result.reset();
    //            ASSERT_NO_THROW(ASSERT_NO_THROW(result = rt->Run("{-count := 1; count += 1; count += 1; --77--; count += 1; count += 1;-}; 7777")););
    //            ASSERT_TRUE(result);
    //            ASSERT_TRUE(result->is_integer()) << result->toString().c_str();
    //            ASSERT_EQ(7777, result->GetValueAsInteger());
    //
    //            result.reset();
    //            ASSERT_ANY_THROW(ASSERT_NO_THROW(result = rt->Run("{ {count := 1; count += 1; count += 1; --:Error(88)--; count += 1; count += 1};  8888; }")););
    //            ASSERT_FALSE(result);
    //
    //            result.reset();
    //            ASSERT_NO_THROW(ASSERT_NO_THROW(result = rt->Run("{ {+count := 1; count += 1; count += 1; ++:Error(99)++; count += 1; count += 1+}; 888}")););
    //            ASSERT_TRUE(result);
    //            ASSERT_TRUE(result->is_integer()) << result->toString().c_str();
    //            ASSERT_EQ(888, result->GetValueAsInteger());
    //
    //            result.reset();
    //            ASSERT_NO_THROW(ASSERT_NO_THROW(result = rt->Run("{ {+count := 1; count += 1; count += 1; ++:Error(99)++; count += 1; count += 1+}; }")););
    //            ASSERT_TRUE(result);
    //            ASSERT_EQ(1, result->size()) << result->toString().c_str();
    //            ASSERT_EQ(99, (*result)[0].second->GetValueAsInteger()) << result->toString().c_str();
    //
    //            result.reset();
    //            ASSERT_NO_THROW(ASSERT_NO_THROW(result = rt->Run("{ {-count := 1; count += 1; count += 1; --:Error(33)--; count += 1; count += 1-};  }")););
    //            ASSERT_TRUE(result);
    //            ASSERT_EQ(1, result->size()) << result->toString().c_str();
    //            ASSERT_EQ(33, (*result)[0].second->GetValueAsInteger()) << result->toString().c_str();
    //
    //            result.reset();
    //            ASSERT_NO_THROW(ASSERT_NO_THROW(result = rt->Run("{ {*count := 1; count += 1; count += 1; --:Error(44)--; count += 1; count += 1*};  9999; }")););
    //            ASSERT_TRUE(result);
    //            ASSERT_EQ(9999, result->GetValueAsInteger()) << result->toString().c_str();
    //
    //            result.reset();
    //            // Не должен перехватытвать другой класс объекта
    //            ASSERT_ANY_THROW(ASSERT_NO_THROW(result = rt->Run("{ {*count := 1; count += 1; count += 1; --:Error(55)--; count += 1; count += 1*}:ErrorRunTime;  }")););
    //            ASSERT_FALSE(result);
    //
    //
    //            ObjPtr test_err = ctx.m_runtime->GetTypeFromString(":Error"));
    //    ASSERT_TRUE(test_err->op_class_test(":Error", &ctx));
    //    ASSERT_FALSE(test_err->op_class_test(":ErrorRunTime", &ctx));
    //
    //    ObjPtr test_rt = ctx.m_runtime->GetTypeFromString(":ErrorRunTime"));
    //    ASSERT_TRUE(test_rt->op_class_test(":ErrorRunTime", &ctx));
    //    ASSERT_TRUE(test_rt->op_class_test(":Error", &ctx));
    //
    //    //    result.reset();
    //    //    // Не должен перехватытвать другой класс объекта, но его перехватывает внешний блок
    //    //    ASSERT_NO_THROW(ASSERT_NO_THROW(result = rt->Run("{* {*count := 1; count += 1; count += 1; --:Error(66)--; count += 1; count += 1*}:ErrorRunTime;  777; *}")););
    //    //    ASSERT_TRUE(result);
    //    //    ASSERT_TRUE(result->is_error());
    //    //    ASSERT_STREQ(":RetMinus(:Error(66))", result->toString().c_str());
    //
    //
    //    result.reset();
    //    // Должен перехватытвать прерывание и вернуть его как обычный объект без исключения из внешнего блока
    //    ASSERT_NO_THROW(ASSERT_NO_THROW(result = rt->Run("{ {*count := 1; count += 1; count += 1; --:ErrorRunTime(77)--; count += 1; count += 1*}:ErrorRunTime }")););
    //    ASSERT_TRUE(result);
    //    ASSERT_EQ(1, result->size());
    //    ASSERT_EQ(77, (*result)[0].second->GetValueAsInteger());
    //
    //    result.reset();
    //    ASSERT_NO_THROW(ASSERT_NO_THROW(result = rt->Run("{ {*count := 1; count += 1; count += 1; --:ErrorRunTime(88)--; count += 1; count += 1*}:ErrorRunTime;  9999; }")););
    //    ASSERT_TRUE(result);
    //    ASSERT_EQ(9999, result->GetValueAsInteger());
    //
    //    result.reset();
    //    ASSERT_NO_THROW(ASSERT_NO_THROW(result = rt->Run("{{*count := 1; count += 1; count += 1; --:ErrorRunTime(77)--; count += 1; count += 1*}:Error }")););
    //    ASSERT_TRUE(result);
    //    ASSERT_EQ(1, result->size());
    //    ASSERT_EQ(77, (*result)[0].second->GetValueAsInteger());
    //
    //    result.reset();
    //    ASSERT_NO_THROW(ASSERT_NO_THROW(result = rt->Run("{{*count := 1; count += 1; count += 1; --:ErrorRunTime(88)--; count += 1; count += 1*}:Error;  9999; }")););
    //    ASSERT_TRUE(result);
    //    ASSERT_EQ(9999, result->GetValueAsInteger());
}

#endif // UNITTEST