#include "pch.h"

#ifdef UNITTEST

#include "rational.h"

#include <warning_push.h>
#include <gtest/gtest.h>
#include <warning_pop.h>

#include <term.h>
#include <context.h>
#include <object.h>
#include <newlang.h>
#include <builtin.h>


using namespace newlang;

TEST(ObjTest, Empty) {
    Obj var;
    ASSERT_TRUE(var.empty());
    ASSERT_EQ(ObjType::None, var.getType());
}

TEST(ObjTest, Value) {
    Obj var;

    var.SetValue_(0);
    ASSERT_EQ(ObjType::Bool, var.getType());

    var.SetValue_(1);
    ASSERT_EQ(ObjType::Bool, var.getType());

    var.SetValue_(1.0);
    ASSERT_EQ(ObjType::Float64, var.getType());

    var.SetValue_(true);
    ASSERT_EQ(ObjType::Bool, var.getType());

    var.SetValue_(2);
    ASSERT_EQ(ObjType::Int8, var.getType());

    var.SetValue_(-100);
    ASSERT_EQ(ObjType::Int8, var.getType());

    var.SetValue_(1000);
    ASSERT_EQ(ObjType::Int16, var.getType());

    var.SetValue_(100000);
    ASSERT_EQ(ObjType::Int32, var.getType());

    var.SetValue_(10000000000);
    ASSERT_EQ(ObjType::Int64, var.getType());

    var.SetValue_(2.0);
    ASSERT_EQ(ObjType::Float64, var.getType());

    var.SetValue_(false);
    ASSERT_EQ(ObjType::Bool, var.getType());
}

TEST(ObjTest, String) {
    Obj var;

    var.SetValue_(std::string("test"));
    ASSERT_EQ(ObjType::StrChar, var.getType()) << toString(var.getType());

    Obj str1;
    str1.SetValue_(L"Test str");
    ASSERT_EQ(ObjType::StrWide, str1.getType()) << toString(str1.getType());

    Obj str2;
    str2.SetValue_(std::string("test2"));
    ASSERT_EQ(ObjType::StrChar, str2.getType()) << toString(str2.getType());

    ObjPtr str_byte = Obj::CreateString("byte");
    ASSERT_STREQ("byte", str_byte->GetValueAsString().c_str());
    ASSERT_EQ(4, str_byte->size());
    ASSERT_EQ(4, str_byte->m_value.size());
    ASSERT_STREQ("b", (*str_byte)[0].second->GetValueAsString().c_str());
    ASSERT_STREQ("y", (*str_byte)[1].second->GetValueAsString().c_str());
    ASSERT_STREQ("t", (*str_byte)[2].second->GetValueAsString().c_str());
    ASSERT_STREQ("e", (*str_byte)[3].second->GetValueAsString().c_str());

    str_byte->op_set_index(0, "B");
    str_byte->op_set_index(1, "Y");
    ASSERT_STREQ("BYte", str_byte->GetValueAsString().c_str());
    str_byte->op_set_index(2, "T");
    str_byte->op_set_index(3, "E");
    ASSERT_STREQ("BYTE", str_byte->GetValueAsString().c_str());

    ObjPtr str_char = Obj::CreateString(L"строка");
    ASSERT_EQ(6, str_char->size());
    ASSERT_EQ(6, str_char->m_string.size());

    ASSERT_STREQ("с", (*str_char)[0].second->GetValueAsString().c_str());
    ASSERT_STREQ("т", (*str_char)[1].second->GetValueAsString().c_str());
    ASSERT_STREQ("р", (*str_char)[2].second->GetValueAsString().c_str());
    ASSERT_STREQ("о", (*str_char)[3].second->GetValueAsString().c_str());
    ASSERT_STREQ("к", (*str_char)[4].second->GetValueAsString().c_str());
    ASSERT_STREQ("а", (*str_char)[5].second->GetValueAsString().c_str());

    str_char->op_set_index(0, "С");
    str_char->op_set_index(1, "Т");
    ASSERT_STREQ("СТрока", str_char->GetValueAsString().c_str());
    str_char->op_set_index(2, "Р");
    str_char->op_set_index(3, "О");
    ASSERT_STREQ("СТРОка", str_char->GetValueAsString().c_str());


    ObjPtr format = Obj::CreateString("$1 $2 ${name}");
    ObjPtr str11 = (*format)(nullptr);
    ASSERT_STREQ("$1 $2 ${name}", str11->GetValueAsString().c_str());

    ObjPtr str22 = (*format)(nullptr, Obj::Arg(100));
    ASSERT_STREQ("100 $2 ${name}", str22->GetValueAsString().c_str());

    ObjPtr str3 = (*format)(nullptr, Obj::Arg(-1), Obj::Arg("222"));
    ASSERT_STREQ("-1 222 ${name}", str3->GetValueAsString().c_str());

    ObjPtr str4 = (*format)(nullptr, Obj::Arg("value", "name"));
    ASSERT_STREQ("value $2 value", str4->GetValueAsString().c_str());

    format = Obj::CreateString("$nameno ${имя1} $name $имя");
    ObjPtr str5 = (*format)(nullptr, Obj::Arg("value", "name"), Obj::Arg("УТФ8-УТФ8", "имя"), Obj::Arg("УТФ8", "имя1"));
    ASSERT_STREQ("valueno УТФ8 value УТФ8-УТФ8", str5->GetValueAsString().c_str());

}

TEST(ObjTest, PrintFormat) {

    ObjPtr format_none = Obj::CreateDict(Obj::Arg(Obj::CreateString("")));
    ASSERT_TRUE(ParsePrintfFormat(format_none.get(), 0));
    format_none->push_back(Obj::CreateString(""));
    ASSERT_FALSE(ParsePrintfFormat(format_none.get(), 0));

    ObjPtr format_string = Obj::CreateDict(Obj::Arg(Obj::CreateString("%s")));
    ASSERT_FALSE(ParsePrintfFormat(format_string.get(), 0));
    format_string->push_back(Obj::CreateString(""));
    ASSERT_TRUE(ParsePrintfFormat(format_string.get(), 0));
    format_string->push_back(Obj::CreateString(""));
    ASSERT_FALSE(ParsePrintfFormat(format_string.get(), 0));

    ObjPtr format_int = Obj::CreateDict(Obj::Arg(Obj::CreateString("%d")));
    ASSERT_FALSE(ParsePrintfFormat(format_int.get(), 0));
    format_int->push_back(Obj::CreateValue(100));
    ASSERT_TRUE(ParsePrintfFormat(format_int.get(), 0));
    format_int->push_back(Obj::CreateString(""));
    ASSERT_FALSE(ParsePrintfFormat(format_int.get(), 0));

    format_int = Obj::CreateDict(Obj::Arg(Obj::CreateString("%d %d")));
    ASSERT_FALSE(ParsePrintfFormat(format_int.get(), 0));
    format_int->push_back(Obj::CreateValue(100));
    ASSERT_FALSE(ParsePrintfFormat(format_int.get(), 0));
    format_int->push_back(Obj::CreateValue(100));
    ASSERT_TRUE(ParsePrintfFormat(format_int.get(), 0));
    format_int->push_back(Obj::CreateString(""));
    ASSERT_FALSE(ParsePrintfFormat(format_int.get(), 0));

    ObjPtr format_number = Obj::CreateDict(Obj::Arg(Obj::CreateString("%f")));
    ASSERT_FALSE(ParsePrintfFormat(format_number.get(), 0));
    format_number->push_back(Obj::CreateValue(0.003));
    ASSERT_TRUE(ParsePrintfFormat(format_number.get(), 0));
    format_number->push_back(Obj::CreateString(""));
    ASSERT_FALSE(ParsePrintfFormat(format_number.get(), 0));

    ObjPtr format_full = Obj::CreateDict(Obj::Arg(Obj::CreateString("%s %d %f %s")));
    ASSERT_FALSE(ParsePrintfFormat(format_full.get(), 0));
    format_full->push_back(Obj::CreateString(""));
    ASSERT_FALSE(ParsePrintfFormat(format_full.get(), 0));
    format_full->push_back(Obj::CreateValue(100));
    ASSERT_FALSE(ParsePrintfFormat(format_full.get(), 0));
    format_full->push_back(Obj::CreateValue(0.003));
    ASSERT_FALSE(ParsePrintfFormat(format_full.get(), 0));
    format_full->push_back(Obj::CreateString(""));
    ASSERT_TRUE(ParsePrintfFormat(format_full.get(), 0));

    format_full->push_back(Obj::CreateString(""));
    ASSERT_FALSE(ParsePrintfFormat(format_full.get(), 0));
}

TEST(ObjTest, Dict) {

    Context ctx(RunTime::Init());

    Obj var(ObjType::Dictionary);
    ASSERT_TRUE(var.empty());
    EXPECT_ANY_THROW(var[0]);

    ASSERT_EQ(0, var.size());
    ObjPtr var2 = ctx.ExecStr("(,)");

    var.push_back(Obj::CreateString("Test1"));
    ASSERT_EQ(1, var.size());
    var.push_back(Obj::CreateString("Test3"));
    ASSERT_EQ(2, var.size());
    var.insert(var.at_index_const(1), Obj::Arg(2, "2"));
    var.insert(var.at_index_const(0), Obj::Arg(0, "0"));
    ASSERT_EQ(4, var.size());

    ASSERT_TRUE(var[0].second->op_accurate(Obj::CreateValue(0, ObjType::None)));
    ASSERT_TRUE(var[1].second->op_accurate(Obj::CreateString("Test1")));
    ASSERT_TRUE(var[2].second->op_accurate(Obj::CreateValue(2, ObjType::None)));
    ASSERT_TRUE(var[3].second->op_accurate(Obj::CreateString(L"Test3")));

    var2 = ctx.ExecStr("(0, \"Test1\", 2, 'Test3',)");

    ASSERT_TRUE((*var2)[0].second->op_accurate(Obj::CreateValue(0, ObjType::None)));
    ASSERT_TRUE((*var2)[1].second->op_accurate(Obj::CreateString("Test1")));
    ASSERT_TRUE((*var2)[2].second->op_accurate(Obj::CreateValue(2, ObjType::None)));
    ASSERT_TRUE((*var2)[3].second->op_accurate(Obj::CreateString(L"Test3")));

    ObjPtr var3 = ctx.ExecStr("(0, \"Test1\", 2, 'Test3',)");

    ASSERT_TRUE((*var3)[0].second->op_accurate(Obj::CreateValue(0, ObjType::None)));
    ASSERT_TRUE((*var3)[1].second->op_accurate(Obj::CreateString("Test1")));
    ASSERT_TRUE((*var3)[2].second->op_accurate(Obj::CreateValue(2, ObjType::None)));
    ASSERT_TRUE((*var3)[3].second->op_accurate(Obj::CreateString(L"Test3")));
}

TEST(ObjTest, AsMap) {

    ObjPtr map = Obj::CreateType(ObjType::Dictionary);
    ASSERT_TRUE(map->empty());
    EXPECT_ANY_THROW((*map)["test1"]);
    ASSERT_EQ(0, map->size());

    ObjPtr temp = Obj::CreateString("Test");
    map->push_back(temp, "test1");
    map->push_back(Obj::CreateValue(100, ObjType::None), "test2");
    ASSERT_EQ(2, map->size());
    ASSERT_STREQ((*map)["test1"].second->toString().c_str(), temp->toString().c_str()) << temp->toString().c_str();

    ASSERT_TRUE((*map)["test2"].second);
    ObjPtr temp100 = Obj::CreateValue(100, ObjType::None);
    ASSERT_TRUE(map->exist(temp100, true));

    ObjPtr test2 = (*map)["test2"].second;
    ASSERT_TRUE(test2);
    ASSERT_TRUE(test2);
    ASSERT_STREQ("100", test2->toString().c_str());
}

TEST(ObjTest, Eq) {

    ObjPtr var_int = Obj::CreateValue(100, ObjType::None);
    ObjPtr var_num = Obj::CreateValue(100.0, ObjType::None);
    ObjPtr var_str = Obj::CreateString(L"STRING");
    ObjPtr var_bool = Obj::CreateBool(true);
    ObjPtr var_empty = Obj::CreateNone();

    ASSERT_EQ(var_int->m_var_type_current, ObjType::Int8) << (int) var_int->getType();
    ASSERT_EQ(var_num->m_var_type_current, ObjType::Float64) << (int) var_num->getType();
    ASSERT_EQ(var_str->m_var_type_current, ObjType::StrWide) << (int) var_str->getType();
    ASSERT_EQ(var_bool->m_var_type_current, ObjType::Bool) << (int) var_bool->getType();
    ASSERT_EQ(var_empty->m_var_type_current, ObjType::None) << (int) var_empty->getType();
    ASSERT_TRUE(var_empty->empty()) << (int) var_empty->getType();
    ASSERT_FALSE(var_int->empty()) << (int) var_int->getType();


    ASSERT_TRUE(var_int->op_accurate(Obj::CreateValue(100, ObjType::None)));
    ASSERT_FALSE(var_int->op_accurate(Obj::CreateValue(111, ObjType::None)));

    ASSERT_TRUE(var_num->op_equal(Obj::CreateValue(100.0, ObjType::None)));
    ASSERT_FALSE(var_num->op_equal(Obj::CreateValue(100.0001, ObjType::None)));
    ASSERT_TRUE(var_num->op_accurate(Obj::CreateValue(100.0, ObjType::None)));
    ASSERT_FALSE(var_num->op_accurate(Obj::CreateValue(100.0001, ObjType::None)));

    ASSERT_TRUE(var_int->op_equal(Obj::CreateValue(100.0, ObjType::None)));
    ASSERT_TRUE(var_int->op_accurate(Obj::CreateValue(100.0, ObjType::None)));
    ASSERT_FALSE(var_int->op_equal(Obj::CreateValue(100.1, ObjType::None)));
    ASSERT_FALSE(var_int->op_accurate(Obj::CreateValue(100.1, ObjType::None)));


    ObjPtr var_int2 = Obj::CreateValue(101, ObjType::None);
    ObjPtr var_num2 = Obj::CreateValue(100.1, ObjType::None);


    ASSERT_TRUE(var_int->op_accurate(var_int));
    ASSERT_FALSE(var_int->op_accurate(var_int2));

    ASSERT_TRUE(var_num->op_accurate(var_num));
    ASSERT_FALSE(var_num->op_accurate(var_num2));

    ASSERT_TRUE(var_int->op_equal(var_num));

    ASSERT_FALSE(var_int->op_equal(var_num2));
    ASSERT_FALSE(var_num2->op_equal(var_int));

    ObjPtr var_bool2 = Obj::CreateBool(false);

    ASSERT_TRUE(var_bool->op_accurate(var_bool));
    ASSERT_FALSE(var_bool->op_accurate(var_bool2));

    ObjPtr var_str2 = Obj::CreateString("STRING2");

    ASSERT_TRUE(var_str->op_accurate(var_str));
    ASSERT_TRUE(var_str->op_accurate(Obj::CreateString("STRING")));
    ASSERT_FALSE(var_str->op_accurate(var_str2));


    ObjPtr var_empty2 = Obj::CreateNone();

    ASSERT_TRUE(var_empty->op_accurate(var_empty));
    ASSERT_TRUE(var_empty->op_accurate(var_empty2));
}

TEST(ObjTest, Ops) {

    ObjPtr var_zero = Obj::CreateValue(0, ObjType::None);
    ASSERT_TRUE(var_zero->is_bool_type());
    ASSERT_TRUE(var_zero->is_arithmetic_type());
    ASSERT_EQ(ObjType::Bool, var_zero->m_var_type_current);

    ObjPtr var_bool = Obj::CreateBool(true);
    ASSERT_TRUE(var_bool->is_bool_type());
    ASSERT_TRUE(var_bool->is_arithmetic_type());
    ASSERT_EQ(ObjType::Bool, var_bool->m_var_type_current);

    ObjPtr var_char = Obj::CreateValue(100);
    ASSERT_TRUE(var_char->is_arithmetic_type());
    ASSERT_EQ(ObjType::Int8, var_char->m_var_type_current);

    ObjPtr var_int = Obj::CreateValue(100000);
    ASSERT_TRUE(var_int->is_arithmetic_type());
    ASSERT_EQ(ObjType::Int32, var_int->m_var_type_current) << newlang::toString(var_char->m_var_type_current);

    ObjPtr var_float = Obj::CreateValue(1.0);
    ASSERT_TRUE(var_float->is_arithmetic_type());
    ASSERT_EQ(ObjType::Float64, var_float->m_var_type_current) << newlang::toString(var_char->m_var_type_current);

    ObjPtr var_tensor = Obj::CreateRange(0, 10)->toType(ObjType::Int32);
    ASSERT_TRUE(var_tensor->is_arithmetic_type());
    ASSERT_EQ(ObjType::Int32, var_tensor->m_var_type_current) << newlang::toString(var_char->m_var_type_current);

    var_tensor->operator*=(var_bool);
    ASSERT_TRUE(var_tensor->is_arithmetic_type());
    EXPECT_EQ(ObjType::Int32, var_tensor->m_var_type_current) << newlang::toString(var_char->m_var_type_current);

    var_tensor->operator*=(var_int);
    ASSERT_TRUE(var_tensor->is_arithmetic_type());
    EXPECT_EQ(ObjType::Int32, var_tensor->m_var_type_current) << newlang::toString(var_char->m_var_type_current);

    var_tensor->operator*=(var_float);
    ASSERT_TRUE(var_tensor->is_arithmetic_type());
    EXPECT_EQ(ObjType::Float64, var_tensor->m_var_type_current) << newlang::toString(var_char->m_var_type_current);
}

TEST(ObjTest, Exist) {

    Obj var_array(ObjType::Dictionary);

    var_array.push_back(Obj::CreateString("item1"));
    var_array.push_back(Obj::CreateString("item2"));

    ObjPtr item = Obj::CreateString("item1");
    ASSERT_TRUE(var_array.exist(item, true));
    item->SetValue_("item2");
    ASSERT_TRUE(var_array.exist(item, true));
    item->SetValue_("item");
    ASSERT_FALSE(var_array.exist(item, true));


    Obj var_map(ObjType::Dictionary);

    var_map.push_back(Obj::CreateString("MAP_VALUE1"), "map1");
    var_map.push_back(Obj::CreateString("MAP_VALUE2"), "map2");

    ASSERT_TRUE(var_map[std::string("map1")].second);
    ASSERT_TRUE(var_map["map2"].second);
    ASSERT_EQ(var_map.find("map"), var_map.end());

}

TEST(ObjTest, Intersec) {

    Obj var_array(ObjType::Dictionary);
    Obj var_array2(ObjType::Dictionary);

    var_array.push_back(Obj::CreateString("item1"));
    var_array.push_back(Obj::CreateString("item2"));

    ObjPtr result = var_array.op_bit_and(var_array2, true);
    ASSERT_TRUE(result->empty());

    var_array2.push_back(Obj::CreateString("item3"));

    result = var_array.op_bit_and(var_array2, true);
    ASSERT_TRUE(result->empty());

    var_array2.push_back(Obj::CreateString("item1"));
    result = var_array.op_bit_and(var_array2, true);
    ASSERT_FALSE(result->empty());
    ASSERT_EQ(1, result->size());

    var_array2.push_back(Obj::CreateString("item2"));
    result = var_array.op_bit_and(var_array2, true);
    ASSERT_FALSE(result->empty());
    ASSERT_EQ(2, result->size());
    //    
    //    result = Op:intersec(var_array, var_array2);
    //    ASSERT_TRUE(Var::isEmpty(result));
    //
    //    
    //    ASSERT_TRUE(var_array.Exist(std::string("item1")));
    //
    //    ASSERT_TRUE(var_array.Exist(std::string("item1")));
    //    ASSERT_TRUE(var_array.Exist(std::string("item2")));
    //    ASSERT_FALSE(var_array.Exist(std::string("item")));
    //
    //
    //    Var var_map(Var::Type::MAP);
    //
    //    var_map.Set("map1", "MAP1");
    //    var_map.Set("map2", "MAP2");
    //
    //    ASSERT_TRUE(var_map.Exist(std::string("map1")));
    //    ASSERT_TRUE(var_map.Exist(std::string("map2")));
    //    ASSERT_FALSE(var_map.Exist(std::string("map")));




    //    Var var_object(Var::Type::OBJECT);
    //    Var var_list(Var::Type::LIST);
    //    
    //    var_list.Set("map1", "MAP1");
    //    var_map.Set("map2", "MAP2");
    //    
    //    ASSERT_TRUE(var_map.Exist("map1"));
    //    ASSERT_TRUE(var_map.Exist("map2"));
    //    ASSERT_FALSE(var_map.Exist("map"));
}

TEST(ObjTest, Print) {

    Context ctx(RunTime::Init());

    ObjPtr var_int = Obj::CreateValue(100, ObjType::None);
    ObjPtr var_num = Obj::CreateValue(100.123, ObjType::None);
    ObjPtr var_str = Obj::CreateString(L"STRCHAR");
    ObjPtr var_strbyte = Obj::CreateString("STRBYTE");
    ObjPtr var_bool = Obj::CreateBool(true);
    ObjPtr var_empty = Obj::CreateNone();

    ASSERT_STREQ("100", var_int->toString().c_str()) << var_int;
    ASSERT_STREQ("100.123", var_num->toString().c_str()) << var_num;
    ASSERT_STREQ("\"STRCHAR\"", var_str->toString().c_str()) << var_str;
    ASSERT_STREQ("'STRBYTE'", var_strbyte->toString().c_str()) << var_strbyte;
    ASSERT_STREQ("1", var_bool->toString().c_str()) << var_bool;
    ASSERT_STREQ("_", var_empty->toString().c_str()) << var_empty;

    ObjPtr var_dict = Obj::CreateType(ObjType::Dictionary);

    ASSERT_STREQ("(,)", var_dict->toString().c_str()) << var_dict;

    var_dict->m_var_name = "dict";
    ASSERT_STREQ("dict=(,)", var_dict->toString().c_str()) << var_dict;

    var_dict->push_back(Obj::CreateString(L"item1"));
    ASSERT_STREQ("dict=(\"item1\",)", var_dict->toString().c_str()) << var_dict;
    var_dict->push_back(var_int);
    var_dict->push_back((*var_bool)(nullptr));
    ASSERT_STREQ("dict=(\"item1\", 100, 1,)", var_dict->toString().c_str()) << var_dict;


    ObjPtr var_array = Obj::CreateDict(); //CreateTensor({1});

    ASSERT_STREQ("(,)", var_array->toString().c_str()) << var_array;

    var_array->m_var_name = "array";
    ASSERT_STREQ("array=(,)", var_array->toString().c_str()) << var_array;

    var_array->push_back(Obj::CreateString("item1"));
    ASSERT_STREQ("array=('item1',)", var_array->toString().c_str()) << var_array;
    var_array->push_back((*var_int.get())(nullptr));
    var_array->push_back((*var_bool.get())(nullptr));
    ASSERT_STREQ("array=('item1', 100, 1,)", var_array->toString().c_str()) << var_array;

}

TEST(ObjTest, CreateFromInteger) {

    Context ctx(RunTime::Init());

    ObjPtr var = Context::CreateRVal(&ctx, Parser::ParseString("123", nullptr));
    ASSERT_TRUE(var);
    ASSERT_EQ(ObjType::Int8, var->getType()) << toString(var->getType());
    ASSERT_EQ(123, var->GetValueAsInteger());

    ObjPtr var2 = ctx.ExecStr("123");
    ASSERT_TRUE(var2);
    ASSERT_EQ(ObjType::Int8, var2->getType()) << toString(var2->getType());
    ASSERT_EQ(123, var2->GetValueAsInteger());

    var = Context::CreateRVal(&ctx, Parser::ParseString("-123", nullptr));
    ASSERT_TRUE(var);
    ASSERT_EQ(ObjType::Int8, var->getType()) << toString(var->getType());
    ASSERT_EQ(-123, var->GetValueAsInteger());

    var2 = ctx.ExecStr("-123");
    ASSERT_TRUE(var2);
    ASSERT_EQ(ObjType::Int8, var2->getType()) << toString(var2->getType());
    ASSERT_EQ(-123, var2->GetValueAsInteger());

    ASSERT_ANY_THROW(Context::CreateRVal(&ctx, Term::Create(parser::token_type::INTEGER, TermID::INTEGER, "")));
    ASSERT_ANY_THROW(Context::CreateRVal(&ctx, Term::Create(parser::token_type::INTEGER, TermID::INTEGER, "lkdfjsha")));
    ASSERT_ANY_THROW(Context::CreateRVal(&ctx, Term::Create(parser::token_type::INTEGER, TermID::INTEGER, "123lkdfjsha")));
}

TEST(ObjTest, CreateFromNumber) {

    Context ctx(RunTime::Init());

    ObjPtr var = Context::CreateRVal(&ctx, Parser::ParseString("123.123", nullptr));
    ASSERT_TRUE(var);
    ASSERT_EQ(ObjType::Float64, var->getType());
    ASSERT_DOUBLE_EQ(123.123, var->GetValueAsNumber());

    ObjPtr var2 = ctx.ExecStr("123.123");
    ASSERT_TRUE(var2);
    ASSERT_EQ(ObjType::Float64, var2->getType());
    ASSERT_DOUBLE_EQ(123.123, var2->GetValueAsNumber());

    var = Context::CreateRVal(&ctx, Parser::ParseString("-123.123", nullptr));
    ASSERT_TRUE(var);
    ASSERT_EQ(ObjType::Float64, var->getType());
    ASSERT_DOUBLE_EQ(-123.123, var->GetValueAsNumber());

    var2 = ctx.ExecStr("-123.123");
    ASSERT_TRUE(var2);
    ASSERT_EQ(ObjType::Float64, var2->getType());
    ASSERT_DOUBLE_EQ(-123.123, var2->GetValueAsNumber());

    ASSERT_ANY_THROW(Context::CreateRVal(&ctx, Term::Create(parser::token_type::NUMBER, TermID::NUMBER, "")));
    ASSERT_ANY_THROW(Context::CreateRVal(&ctx, Term::Create(parser::token_type::NUMBER, TermID::NUMBER, "lkdfjsha")));
    ASSERT_ANY_THROW(Context::CreateRVal(&ctx, Term::Create(parser::token_type::NUMBER, TermID::NUMBER, "123lkdfjsha")));
}

TEST(ObjTest, CreateFromString) {

    Context ctx(RunTime::Init());

    ObjPtr var = Context::CreateRVal(&ctx, Parser::ParseString("\"123.123\"", nullptr));
    ASSERT_TRUE(var);
    ASSERT_EQ(ObjType::StrWide, var->getType());
    ASSERT_STREQ("123.123", var->GetValueAsString().c_str());

    ObjPtr var2 = ctx.ExecStr("\"123.123\"");
    ASSERT_TRUE(var2);
    ASSERT_EQ(ObjType::StrWide, var2->getType());
    ASSERT_STREQ("123.123", var2->GetValueAsString().c_str());

    var = Context::CreateRVal(&ctx, Parser::ParseString("'строка'", nullptr));
    ASSERT_TRUE(var);
    ASSERT_EQ(ObjType::StrChar, var->getType());
    ASSERT_STREQ("строка", var->GetValueAsString().c_str());

    var2 = ctx.ExecStr("'строка'");
    ASSERT_TRUE(var2);
    ASSERT_EQ(ObjType::StrChar, var2->getType());
    ASSERT_STREQ("строка", var2->GetValueAsString().c_str());
}

TEST(ObjTest, CreateFromRational) {

    Context ctx(RunTime::Init());

    ObjPtr var = Context::CreateRVal(&ctx, Parser::ParseString("123\\1", nullptr));
    ASSERT_TRUE(var);
    ASSERT_EQ(ObjType::Rational, var->getType()) << toString(var->getType());
    ASSERT_EQ(123, var->GetValueAsInteger());
    ASSERT_DOUBLE_EQ(123.0, var->GetValueAsNumber());

    ObjPtr var2 = ctx.ExecStr("123\\1");
    ASSERT_TRUE(var2);
    ASSERT_EQ(ObjType::Rational, var2->getType()) << toString(var2->getType());
    ASSERT_EQ(123, var2->GetValueAsInteger());
    ASSERT_DOUBLE_EQ(123, var2->GetValueAsNumber());

    var = Context::CreateRVal(&ctx, Parser::ParseString("-123\\1", nullptr));
    ASSERT_TRUE(var);
    ASSERT_EQ(ObjType::Rational, var->getType()) << toString(var->getType());
    ASSERT_EQ(-123, var->GetValueAsInteger());
    ASSERT_DOUBLE_EQ(-123.0, var->GetValueAsNumber());

    var2 = ctx.ExecStr("-123\\1");
    ASSERT_TRUE(var2);
    ASSERT_EQ(ObjType::Rational, var2->getType()) << toString(var2->getType());
    ASSERT_EQ(-123, var2->GetValueAsInteger());
    ASSERT_DOUBLE_EQ(-123, var2->GetValueAsNumber());

    ASSERT_ANY_THROW(Context::CreateRVal(&ctx, Term::Create(parser::token_type::RATIONAL, TermID::RATIONAL, "1@0")));
    ASSERT_ANY_THROW(Context::CreateRVal(&ctx, Term::Create(parser::token_type::RATIONAL, TermID::RATIONAL, "1@")));
    ASSERT_ANY_THROW(Context::CreateRVal(&ctx, Term::Create(parser::token_type::RATIONAL, TermID::RATIONAL, "")));
    ASSERT_ANY_THROW(Context::CreateRVal(&ctx, Term::Create(parser::token_type::RATIONAL, TermID::RATIONAL, "asdsdff")));
    ASSERT_ANY_THROW(Context::CreateRVal(&ctx, Term::Create(parser::token_type::RATIONAL, TermID::RATIONAL, "asdsdff@dddddd")));
    ASSERT_ANY_THROW(Context::CreateRVal(&ctx, Term::Create(parser::token_type::RATIONAL, TermID::RATIONAL, "123asdsdff@dddddd")));
    ASSERT_ANY_THROW(Context::CreateRVal(&ctx, Term::Create(parser::token_type::RATIONAL, TermID::RATIONAL, "123@111dddddd")));
    ASSERT_ANY_THROW(Context::CreateRVal(&ctx, Term::Create(parser::token_type::RATIONAL, TermID::RATIONAL, "123wwwww@111")));
}

TEST(Args, All) {
    Context ctx(RunTime::Init());
    Parser p1;

    ASSERT_TRUE(p1.Parse("test()"));
    Obj local;
    Obj proto1(&ctx, p1.GetAst(), false, &local); // Функция с принимаемыми аргументами (нет аргументов)
    ASSERT_EQ(0, proto1.size());
    //    ASSERT_FALSE(proto1.m_is_ellipsis);


    ObjPtr arg_999 = Obj::CreateDict(Obj::Arg(Obj::CreateValue(999, ObjType::None)));
    EXPECT_EQ(ObjType::Int16, (*arg_999)[0].second->getType()) << torch::toString(toTorchType((*arg_999)[0].second->getType()));

    ObjPtr arg_empty_named = Obj::CreateDict(Obj::Arg());
    ASSERT_EQ(ObjType::None, (*arg_empty_named)[0].second->getType());

    ObjPtr arg_123_named = Obj::CreateDict(Obj::Arg(123, "named"));
    EXPECT_EQ(ObjType::Int8, (*arg_123_named)[0].second->getType()) << torch::toString(toTorchType((*arg_123_named)[0].second->getType()));

    ObjPtr arg_999_123_named = Obj::CreateDict();
    ASSERT_EQ(0, arg_999_123_named->size());
    *arg_999_123_named += arg_empty_named;
    ASSERT_EQ(1, arg_999_123_named->size());
    *arg_999_123_named += arg_123_named;
    ASSERT_EQ(2, arg_999_123_named->size());

    ASSERT_ANY_THROW(proto1.ConvertToArgs(arg_999.get(), true, nullptr)); // Прототип не принимает позиционных аргументов

    ASSERT_ANY_THROW(proto1.ConvertToArgs(arg_empty_named.get(), true, nullptr)); // Прототип не принимает именованных аргументов


    TermPtr proto_term = Parser::ParseString("test(arg1)", nullptr, nullptr);
    ASSERT_EQ(1, proto_term->size()) << proto_term->toString();

    Parser p2;
    ASSERT_TRUE(p2.Parse("test(arg1)"));
    ASSERT_EQ(1, p2.GetAst()->size()) << p2.GetAst()->toString();

    const Obj proto2(&ctx, p2.GetAst(), false, &local);
    ASSERT_EQ(1, proto2.size()) << proto2.toString();
    //    ASSERT_FALSE(proto2.m_is_ellipsis);
    ASSERT_STREQ("arg1", proto2.name(0).c_str());
    ASSERT_EQ(nullptr, proto2.at(0).second);

    ObjPtr o_arg_999 = proto2.ConvertToArgs(arg_999.get(), true, nullptr);
    ASSERT_TRUE((*o_arg_999)[0].second);
    ASSERT_STREQ("999", (*o_arg_999)[0].second->toString().c_str());

    //    proto2[0].reset(); // Иначе типы гурментов буду отличаться
    ObjPtr o_arg_empty_named = proto2.ConvertToArgs(arg_empty_named.get(), false, nullptr);
    ASSERT_TRUE((*o_arg_empty_named)[0].second);
    ASSERT_STREQ("_", (*o_arg_empty_named)[0].second->toString().c_str());

    ASSERT_ANY_THROW(proto2.ConvertToArgs(arg_123_named.get(), true, nullptr)); // Имя аругмента отличается


    // Нормальный вызов
    Parser p3;
    ASSERT_TRUE(p3.Parse("test(empty=_, ...) := {}"));
    const Obj proto3(&ctx, p3.GetAst()->Left(), false, &local);
    ASSERT_EQ(2, proto3.size());
    //    ASSERT_TRUE(proto3.m_is_ellipsis);
    ASSERT_STREQ("empty", proto3.name(0).c_str());
    ASSERT_TRUE(proto3.at(0).second);
    ASSERT_EQ(ObjType::None, proto3.at(0).second->getType());
    ASSERT_STREQ("...", proto3.name(1).c_str());
    ASSERT_FALSE(proto3.at(1).second);

    ObjPtr proto3_arg = proto3.ConvertToArgs(arg_999.get(), true, nullptr);
    ASSERT_EQ(1, proto3_arg->size());
    ASSERT_TRUE((*proto3_arg)[0].second);
    ASSERT_STREQ("999", (*proto3_arg)[0].second->toString().c_str());
    ASSERT_STREQ("empty", proto3_arg->name(0).c_str());

    // Дополнительный аргумент
    ObjPtr arg_extra = Obj::CreateDict(Obj::Arg(Obj::CreateValue(999, ObjType::None)), Obj::Arg(123, "named"));

    ASSERT_EQ(2, arg_extra->size());
    EXPECT_EQ(ObjType::Int16, (*arg_extra)[0].second->getType()) << torch::toString(toTorchType((*arg_extra)[0].second->getType()));
    EXPECT_EQ(ObjType::Int8, (*arg_extra)[1].second->getType()) << torch::toString(toTorchType((*arg_extra)[1].second->getType()));


    ObjPtr proto3_extra = proto3.ConvertToArgs(arg_extra.get(), true, nullptr);
    ASSERT_EQ(2, proto3_extra->size());
    ASSERT_STREQ("999", (*proto3_extra)[0].second->toString().c_str());
    ASSERT_STREQ("empty", proto3_extra->name(0).c_str());
    ASSERT_STREQ("123", (*proto3_extra)[1].second->toString().c_str());
    ASSERT_STREQ("named", proto3_extra->name(1).c_str());


    // Аргумент по умолчанию
    Parser p4;
    ASSERT_TRUE(p4.Parse("test(num=123) := { }"));
    Obj proto123(&ctx, p4.GetAst()->Left(), false, &local);
    ASSERT_EQ(1, proto123.size());
    //    ASSERT_FALSE(proto123.m_is_ellipsis);
    ASSERT_STREQ("num", proto123.name(0).c_str());
    ASSERT_TRUE(proto123[0].second);
    ASSERT_STREQ("123", proto123[0].second->toString().c_str());


    // Изменен порядок
    Parser p5;
    ASSERT_TRUE(p5.Parse("test(arg1, str=\"string\") := {}"));
    Obj proto_str(&ctx, p5.GetAst()->Left(), false, &local);
    ASSERT_EQ(2, proto_str.size());
    //    ASSERT_FALSE(proto_str.m_is_ellipsis);
    ASSERT_STREQ("arg1", proto_str.at(0).first.c_str());
    ASSERT_EQ(nullptr, proto_str[0].second);

    ObjPtr arg_str = Obj::CreateDict(Obj::Arg(L"СТРОКА", "str"), Obj::Arg(555, "arg1"));

    ObjPtr proto_str_arg = proto_str.ConvertToArgs(arg_str.get(), true, nullptr);
    ASSERT_STREQ("arg1", proto_str_arg->at(0).first.c_str());
    ASSERT_TRUE(proto_str_arg->at(0).second);
    ASSERT_STREQ("555", (*proto_str_arg)[0].second->toString().c_str());
    ASSERT_STREQ("str", proto_str_arg->at(1).first.c_str());
    ASSERT_TRUE((*proto_str_arg)[1].second);
    ASSERT_STREQ("\"СТРОКА\"", (*proto_str_arg)[1].second->toString().c_str());


    Parser p6;
    ASSERT_TRUE(p6.Parse("test(arg1, ...) := {}"));
    Obj proto_any(&ctx, p6.GetAst()->Left(), false, &local);
    //    ASSERT_TRUE(proto_any.m_is_ellipsis);
    ASSERT_EQ(2, proto_any.size());
    ASSERT_STREQ("arg1", proto_any.at(0).first.c_str());
    ASSERT_EQ(nullptr, proto_any.at(0).second);

    ObjPtr any = proto_any.ConvertToArgs(arg_str.get(), true, nullptr);
    ASSERT_EQ(2, any->size());
    ASSERT_STREQ("arg1", any->at(0).first.c_str());
    ASSERT_TRUE(any->at(0).second);
    ASSERT_STREQ("555", (*any)[0].second->toString().c_str());
    ASSERT_STREQ("str", any->at(1).first.c_str());
    ASSERT_TRUE(any->at(1).second);
    ASSERT_STREQ("\"СТРОКА\"", (*any)[1].second->toString().c_str());


    Parser p7;
    ASSERT_TRUE(p7.Parse("func(arg) := {}"));
    Obj proto_func(&ctx, p7.GetAst()->Left(), false, &local);
    //    ASSERT_TRUE(proto_any.m_is_ellipsis);
    ASSERT_EQ(1, proto_func.size());
    ASSERT_STREQ("arg", proto_func.at(0).first.c_str());
    ASSERT_EQ(nullptr, proto_func.at(0).second);

    ObjPtr arg_str2 = Obj::CreateDict(Obj::Arg("STRING"));

    proto_func.ConvertToArgs_(arg_str2.get(), true, nullptr);
    ASSERT_EQ(1, proto_func.size());
    ASSERT_STREQ("arg", proto_func.at(0).first.c_str());
    ASSERT_TRUE(proto_func.at(0).second);
    ASSERT_STREQ("'STRING'", proto_func[0].second->toString().c_str());
    ASSERT_EQ(ObjType::StrChar, proto_func[0].second->getType());

    //
    Parser p8;
    ASSERT_TRUE(p8.Parse("min(arg, ...) := {}"));
    Obj min_proto(&ctx, p8.GetAst()->Left(), false, &local);
    ObjPtr min_args = Obj::CreateDict(Obj::Arg(200), Obj::Arg(100), Obj::Arg(300));
    ObjPtr min_arg = min_proto.ConvertToArgs(min_args.get(), true, nullptr);

    ASSERT_EQ(3, min_arg->size());
    ASSERT_STREQ("200", (*min_arg)[0].second->toString().c_str());
    ASSERT_STREQ("100", (*min_arg)[1].second->toString().c_str());
    ASSERT_STREQ("300", (*min_arg)[2].second->toString().c_str());

    Parser p9;
    ASSERT_TRUE(p9.Parse("min(200, 100, 300)"));
    Obj args_term(&ctx, p9.GetAst(), true, &local);
    ASSERT_STREQ("200", args_term[0].second->toString().c_str());
    ASSERT_STREQ("100", args_term[1].second->toString().c_str());
    ASSERT_STREQ("300", args_term[2].second->toString().c_str());
}

TEST(Types, FromLimit) {

    std::multimap<int64_t, ObjType> IntTypes = {
        {0, ObjType::Bool},
        {1, ObjType::Bool},
        {2, ObjType::Int8},
        {127, ObjType::Int8},
        //        {255, ObjType::Int8},
        {256, ObjType::Int16},
        {-1, ObjType::Int8},
        {-200, ObjType::Int16},
        {66000, ObjType::Int32},
        {-33000, ObjType::Int32},
        {5000000000, ObjType::Int64},
    };

    for (auto elem : IntTypes) {
        ASSERT_EQ(elem.second, typeFromLimit(elem.first)) << elem.first << " " << toString(elem.second);
    }

    ASSERT_EQ(ObjType::Float64, typeFromLimit(1.0));
    ASSERT_EQ(ObjType::Float64, typeFromLimit(0.0));

}

TEST(ObjTest, Tensor) {

    RuntimePtr opts = RunTime::Init();
    Context ctx(opts);

    TermPtr term = Parser::ParseString("var:Int32[2,3]", nullptr);

    ObjPtr t1 = Context::CreateLVal(&ctx, term);
    ASSERT_EQ(ObjType::Int32, t1->m_var_type_current);
    ASSERT_EQ(ObjType::Int32, t1->m_var_type_fixed);
    ASSERT_FALSE(t1->m_var_is_init);

    ASSERT_EQ(2, t1->m_tensor.dim());
    ASSERT_EQ(2, t1->m_tensor.size(0));
    ASSERT_EQ(3, t1->m_tensor.size(1));

    std::string from_str = "русские буквы для ПРОВЕРКИ КОНВЕРТАЦИИ символов";
    std::wstring to_str = utf8_decode(from_str);
    std::string conv_str = utf8_encode(to_str);

    ASSERT_STREQ(from_str.c_str(), conv_str.c_str());

    // Байтовые строки
    ObjPtr str = Obj::CreateString("test");

    ASSERT_STREQ("test", str->m_value.c_str());

    torch::Tensor tstr_t;

    ConvertStringToTensor(str->m_value, tstr_t, ObjType::Int8);

    ASSERT_TRUE(tstr_t.defined());
    ASSERT_EQ(tstr_t.index({0}).item<int>(), 't');
    ASSERT_EQ(tstr_t.index({1}).item<int>(), 'e');
    ASSERT_EQ(tstr_t.index({2}).item<int>(), 's');
    ASSERT_EQ(tstr_t.index({3}).item<int>(), 't');


    torch::Tensor tensot_temp = str->toType(ObjType::Tensor)->m_tensor;
    ASSERT_TRUE(tensot_temp.defined());
    ASSERT_EQ(tensot_temp.index({0}).item<int>(), 't');
    ASSERT_EQ(tensot_temp.index({1}).item<int>(), 'e');
    ASSERT_EQ(tensot_temp.index({2}).item<int>(), 's');
    ASSERT_EQ(tensot_temp.index({3}).item<int>(), 't');

    ObjPtr t_str = Obj::CreateTensor(tensot_temp);
    ASSERT_EQ(t_str->m_var_type_current, ObjType::Int8) << toString(t_str->m_var_type_current);
    ASSERT_EQ(4, t_str->size());
    ASSERT_TRUE(t_str->m_tensor.defined());

    ASSERT_EQ(t_str->m_tensor.index({0}).item<int>(), 't');
    ASSERT_EQ(t_str->m_tensor.index({1}).item<int>(), 'e');
    ASSERT_EQ(t_str->m_tensor.index({2}).item<int>(), 's');
    ASSERT_EQ(t_str->m_tensor.index({3}).item<int>(), 't');

    ASSERT_EQ(t_str->index_get({0})->GetValueAsInteger(), 't');
    ASSERT_EQ(t_str->index_get({1})->GetValueAsInteger(), 'e');
    ASSERT_EQ(t_str->index_get({2})->GetValueAsInteger(), 's');
    ASSERT_EQ(t_str->index_get({3})->GetValueAsInteger(), 't');

    ASSERT_STREQ(t_str->toType(ObjType::StrWide)->GetValueAsString().c_str(), "test") << t_str->toType(ObjType::StrWide)->GetValueAsString();

    t_str->index_set_({1}, Obj::CreateString("E"));
    t_str->index_set_({2}, Obj::CreateString("S"));

    EXPECT_STREQ(t_str->toType(ObjType::StrWide)->GetValueAsString().c_str(), "tESt") << t_str->toType(ObjType::StrWide)->GetValueAsString();

    // Символьные сторки
    ObjPtr wstr = Obj::CreateString(L"ТЕСТ");
    ObjPtr t_wstr = Obj::CreateTensor(wstr->toType(ObjType::Tensor)->m_tensor);
    if(sizeof (wchar_t) == 2) {
        ASSERT_EQ(t_wstr->m_var_type_current, ObjType::Int16);
    } else {
        ASSERT_TRUE(sizeof (wchar_t) == 4);
        ASSERT_EQ(t_wstr->m_var_type_current, ObjType::Int32);
    }
    ASSERT_EQ(4, t_wstr->size());

    ASSERT_EQ(t_wstr->index_get({0})->GetValueAsInteger(), L'Т');
    ASSERT_EQ(t_wstr->index_get({1})->GetValueAsInteger(), L'Е');
    ASSERT_EQ(t_wstr->index_get({2})->GetValueAsInteger(), L'С');
    ASSERT_EQ(t_wstr->index_get({3})->GetValueAsInteger(), L'Т');

    std::wstring test_wide = t_wstr->toType(ObjType::StrWide)->GetValueAsStringWide();
    EXPECT_STREQ(utf8_encode(test_wide).c_str(), "ТЕСТ");

    std::string test_str = t_wstr->toType(ObjType::StrWide)->GetValueAsString();
    EXPECT_STREQ(test_str.c_str(), "ТЕСТ") << test_str;

    t_wstr->index_set_({1}, Obj::CreateString(L"е"));
    t_wstr->index_set_({2}, Obj::CreateString(L"с"));

    test_str = t_wstr->toType(ObjType::StrWide)->GetValueAsString();
    EXPECT_STREQ(test_str.c_str(), "ТесТ") << test_str;

}

TEST(ObjTest, Iterator) {

    ObjPtr dict = Obj::CreateDict();

    dict->push_back(Obj::Arg(1, "1"));
    dict->push_back(Obj::Arg(2, "22"));
    dict->push_back(Obj::Arg(3, "333"));
    dict->push_back(Obj::Arg(4));
    dict->push_back(Obj::Arg(5, "555"));

    ASSERT_EQ(5, dict->size());

    auto all = std::regex("(.|\\n)*");
    ASSERT_TRUE(std::regex_match("1", all));
    ASSERT_TRUE(std::regex_match("22", all));
    ASSERT_TRUE(std::regex_match("333", all));
    ASSERT_TRUE(std::regex_match("", all));
    ASSERT_TRUE(std::regex_match("\n", all));
    ASSERT_TRUE(std::regex_match("\n\n\\n", all));


    Iterator <Obj> iter(dict);

    ASSERT_TRUE(iter == iter.begin());
    ASSERT_TRUE(iter != iter.end());

    ObjPtr copy = Obj::CreateDict();
    for (auto &elem : iter) {
        copy->push_back(elem.second, elem.first);
    }

    ASSERT_TRUE(iter == iter.begin());
    ASSERT_TRUE(iter != iter.end());

    ASSERT_EQ(dict->size(), copy->size());


    ASSERT_TRUE(iter == iter.begin());

    ObjPtr one = iter.read_and_next(0);
    ASSERT_TRUE(one);
    ASSERT_EQ(1, one->GetValueAsInteger());

    ASSERT_EQ(2, (*iter).second->GetValueAsInteger());
    one = iter.read_and_next(0);
    ASSERT_TRUE(one);
    ASSERT_EQ(2, one->GetValueAsInteger());

    ASSERT_EQ(3, (*iter).second->GetValueAsInteger());
    one = iter.read_and_next(0);
    ASSERT_TRUE(one);
    ASSERT_EQ(3, one->GetValueAsInteger());

    ASSERT_EQ(4, (*iter).second->GetValueAsInteger());
    one = iter.read_and_next(0);
    ASSERT_TRUE(one);
    ASSERT_EQ(4, one->GetValueAsInteger());

    ASSERT_EQ(5, (*iter).second->GetValueAsInteger());
    one = iter.read_and_next(0);
    ASSERT_TRUE(one);
    ASSERT_EQ(5, one->GetValueAsInteger());

    one = iter.read_and_next(0);
    ASSERT_TRUE(one);
    ASSERT_EQ(ObjType::IteratorEnd, one->getType()) << one << " " << toString(one->getType());

    one = iter.read_and_next(0);
    ASSERT_TRUE(one);
    ASSERT_EQ(ObjType::IteratorEnd, one->getType()) << one << " " << toString(one->getType());




    ASSERT_TRUE(iter == iter.end());
    iter.reset();
    ASSERT_TRUE(iter == iter.begin());
    ASSERT_TRUE(iter != iter.end());

    ObjPtr dict1 = iter.read_and_next(-3);
    ASSERT_TRUE(dict1);
    ASSERT_EQ(3, dict1->size());
    ASSERT_EQ(1, dict1->at(0).second->GetValueAsInteger());
    ASSERT_EQ(2, dict1->at(1).second->GetValueAsInteger());
    ASSERT_EQ(3, dict1->at(2).second->GetValueAsInteger());

    ObjPtr dict2 = iter.read_and_next(-3);
    ASSERT_TRUE(dict2);
    ASSERT_EQ(3, dict2->size());
    ASSERT_EQ(4, dict2->at(0).second->GetValueAsInteger());
    ASSERT_EQ(5, dict2->at(1).second->GetValueAsInteger());
    ASSERT_EQ(ObjType::IteratorEnd, dict2->at(2).second->getType());

    ObjPtr dict3 = iter.read_and_next(-3);
    ASSERT_TRUE(dict3);
    ASSERT_EQ(3, dict1->size());
    ASSERT_EQ(ObjType::IteratorEnd, dict3->at(0).second->getType());
    ASSERT_EQ(ObjType::IteratorEnd, dict3->at(1).second->getType());
    ASSERT_EQ(ObjType::IteratorEnd, dict3->at(2).second->getType());



    ASSERT_TRUE(iter == iter.end());
    iter.reset();
    ASSERT_TRUE(iter == iter.begin());
    ASSERT_TRUE(iter != iter.end());

    dict1 = iter.read_and_next(3);
    ASSERT_TRUE(dict1);
    ASSERT_EQ(3, dict1->size());
    ASSERT_EQ(1, dict1->at(0).second->GetValueAsInteger());
    ASSERT_EQ(2, dict1->at(1).second->GetValueAsInteger());
    ASSERT_EQ(3, dict1->at(2).second->GetValueAsInteger());

    dict2 = iter.read_and_next(3);
    ASSERT_TRUE(dict2);
    ASSERT_EQ(2, dict2->size());
    ASSERT_EQ(4, dict2->at(0).second->GetValueAsInteger());
    ASSERT_EQ(5, dict2->at(1).second->GetValueAsInteger());

    dict3 = iter.read_and_next(3);
    ASSERT_TRUE(dict3);
    ASSERT_EQ(0, dict3->size());




    Iterator <Obj> flt(dict, "");
    ObjPtr flt_res = flt.read_and_next(100);
    ASSERT_TRUE(flt_res);
    ASSERT_EQ(1, flt_res->size());
    ASSERT_EQ(4, flt_res->at(0).second->GetValueAsInteger());


    Iterator <Obj> flt1(dict, ".");
    ObjPtr flt1_res = flt1.read_and_next(100);
    ASSERT_TRUE(flt1_res);
    ASSERT_EQ(1, flt1_res->size());
    ASSERT_EQ(1, flt1_res->at(0).second->GetValueAsInteger());


    Iterator <Obj> flt2(dict, "..");
    ObjPtr flt2_res = flt2.read_and_next(100);
    ASSERT_TRUE(flt2_res);
    ASSERT_EQ(1, flt2_res->size());
    ASSERT_EQ(2, flt2_res->at(0).second->GetValueAsInteger());

    Iterator <Obj> flt3(dict, "...");
    ObjPtr flt3_res = flt3.read_and_next(100);
    ASSERT_TRUE(flt3_res);
    ASSERT_EQ(2, flt3_res->size());
    ASSERT_EQ(3, flt3_res->at(0).second->GetValueAsInteger());
    ASSERT_EQ(5, flt3_res->at(1).second->GetValueAsInteger());



    //    ObjPtr iter1 = dict->MakeIterator();

}

TEST(ObjTest, System) {

    ASSERT_STREQ("name", ExtractName("name").c_str());
    ASSERT_STREQ("name", ExtractName("::name").c_str());
    ASSERT_STREQ("name", ExtractName("ns::name").c_str());
    ASSERT_STREQ("", ExtractName("\\file").c_str());
    ASSERT_STREQ("", ExtractName("\\\\dir.file").c_str());
    ASSERT_STREQ("var", ExtractName("\\dir.file::var").c_str());
    ASSERT_STREQ("var.field", ExtractName("\\\\dir.file::var.field").c_str());


    ASSERT_STREQ("\\file", ExtractModuleName("\\file").c_str());
    ASSERT_STREQ("\\\\dir.file", ExtractModuleName("\\\\dir.file").c_str());
    ASSERT_STREQ("\\dir.file", ExtractModuleName("\\dir.file::var").c_str());
    ASSERT_STREQ("\\\\dir.file", ExtractModuleName("\\\\dir.file::var.field").c_str());


    ObjPtr none = Obj::CreateNone();

    ASSERT_TRUE(none->at("__error_field_name__").second);
    ASSERT_STREQ("Internal field '__error_field_name__' not exist!", none->at("__error_field_name__").second->GetValueAsString().c_str());

    ASSERT_TRUE(none->at("__type__").second);
    ASSERT_STREQ(":None", none->at("__type__").second->GetValueAsString().c_str());

    ASSERT_TRUE(none->at("__type_fixed__").second);
    ASSERT_STREQ(":None", none->at("__type_fixed__").second->GetValueAsString().c_str());

    ASSERT_TRUE(none->at("__name__").second);
    ASSERT_STREQ("", none->at("__name__").second->GetValueAsString().c_str());

    ASSERT_TRUE(none->at("__full_name__").second);
    ASSERT_STREQ("", none->at("__full_name__").second->GetValueAsString().c_str());

    ASSERT_TRUE(none->at("__module__").second);
    ASSERT_STREQ("", none->at("__module__").second->GetValueAsString().c_str());

    ASSERT_TRUE(none->at("__class__").second);
    ASSERT_STREQ("", none->at("__class__").second->GetValueAsString().c_str());

    ASSERT_TRUE(none->at("__base__").second);
    ASSERT_STREQ("(,)", none->at("__base__").second->GetValueAsString().c_str());

    ASSERT_TRUE(none->at("__doc__").second);
    ASSERT_STREQ("Help system not implemented!!!!!", none->at("__doc__").second->GetValueAsString().c_str());

    ASSERT_TRUE(none->at("__str__").second);
    ASSERT_STREQ("_", none->at("__str__").second->GetValueAsString().c_str());

    none->m_var_is_init = false;
    ASSERT_TRUE(none->at("__error_field_name__").second);
    ASSERT_STREQ(":Undefined", none->at("__error_field_name__").second->GetValueAsString().c_str());


    ObjPtr str = Obj::CreateString("str");
    str->m_var_name = "name";

    ASSERT_TRUE(str->at("__type__").second);
    ASSERT_STREQ(":StrChar", str->at("__type__").second->GetValueAsString().c_str());

    ASSERT_TRUE(str->at("__type_fixed__").second);
    ASSERT_STREQ(":String", str->at("__type_fixed__").second->GetValueAsString().c_str());

    ASSERT_TRUE(str->at("__name__").second);
    ASSERT_STREQ("name", str->at("__name__").second->GetValueAsString().c_str());

    ASSERT_TRUE(str->at("__full_name__").second);
    ASSERT_STREQ("name", str->at("__full_name__").second->GetValueAsString().c_str());

    ASSERT_TRUE(str->at("__module__").second);
    ASSERT_STREQ("", str->at("__module__").second->GetValueAsString().c_str());

    ASSERT_TRUE(str->at("__class__").second);
    ASSERT_STREQ("", str->at("__class__").second->GetValueAsString().c_str());

    ASSERT_TRUE(str->at("__base__").second);
    ASSERT_STREQ("(,)", str->at("__base__").second->GetValueAsString().c_str());

    ASSERT_TRUE(str->at("__doc__").second);
    ASSERT_STREQ("Help system not implemented!!!!!", str->at("__doc__").second->GetValueAsString().c_str());

    ASSERT_TRUE(str->at("__str__").second);
    ASSERT_STREQ("name='str'", str->at("__str__").second->GetValueAsString().c_str());


    ObjPtr cls = Obj::CreateClass("class");
    cls->m_var_name = "::ns::name";
    cls->push_back(Obj::CreateRange(0, 10, 3), "filed");

    ASSERT_TRUE(cls->at("__type__").second);
    ASSERT_STREQ(":Class", cls->at("__type__").second->GetValueAsString().c_str());

    ASSERT_TRUE(cls->at("__type_fixed__").second);
    ASSERT_STREQ(":None", cls->at("__type_fixed__").second->GetValueAsString().c_str());

    ASSERT_TRUE(cls->at("__name__").second);
    ASSERT_STREQ("name", cls->at("__name__").second->GetValueAsString().c_str());

    ASSERT_TRUE(cls->at("__full_name__").second);
    ASSERT_STREQ("::ns::name", cls->at("__full_name__").second->GetValueAsString().c_str());

    ASSERT_TRUE(cls->at("__module__").second);
    ASSERT_STREQ("", cls->at("__module__").second->GetValueAsString().c_str());

    ASSERT_TRUE(cls->at("__class__").second);
    ASSERT_STREQ("class", cls->at("__class__").second->GetValueAsString().c_str());

    ASSERT_TRUE(cls->at("__base__").second);
    ASSERT_STREQ("(,)", cls->at("__base__").second->GetValueAsString().c_str());

    ASSERT_TRUE(cls->at("__doc__").second);
    ASSERT_STREQ("Help system not implemented!!!!!", cls->at("__doc__").second->GetValueAsString().c_str());

    ASSERT_TRUE(cls->at("__str__").second);
    ASSERT_STREQ("::ns::name=class(filed=0..10..3)", cls->at("__str__").second->GetValueAsString().c_str());
}

#endif // UNITTEST