#include "core/parser.h"
#ifdef UNITTEST

#include <core/pch.h>

#include <signal.h>

#include <core/warning_pop.h>
#include <core/warning_push.h>
#include <gtest/gtest.h>

#include <core/builtin.h>
#include <core/newlang.h>

using namespace newlang;

extern "C" int64_t var_long;
int64_t var_long = 987654321;

extern "C" int64_t func_export(int64_t arg_long, uint8_t arg_byte) {
    return arg_long + arg_byte;
}

TEST(Eval, Assign) {

    Context ctx(RunTime::Init());

    ObjPtr list = ctx.Eval("$");
    ASSERT_STREQ("$=(,)", list->toString().c_str());

    ObjPtr var1 = ctx.Eval("var1 ::= 123");
    ASSERT_TRUE(var1);
    ASSERT_TRUE(var1->is_arithmetic_type());
    ASSERT_TRUE(var1->is_integer());
    ASSERT_EQ(var1->m_var_type_current, ObjType::Char);
    ASSERT_EQ(var1->m_var_type_fixed, ObjType::None);
    ASSERT_STREQ("var1=123", var1->toString().c_str());
    ASSERT_FALSE(ctx.select("var1").complete());

    list = ctx.Eval("$");
    ASSERT_STREQ("$=('var1',)", list->toString().c_str());

    ASSERT_THROW(ctx.Eval("var1 ::= 123"), std::exception);

    ASSERT_TRUE(ctx.Eval("var1 = 100:Char"));
    ASSERT_EQ(var1->m_var_type_current, ObjType::Char);
    ASSERT_EQ(var1->m_var_type_fixed, ObjType::None);
    ASSERT_STREQ("var1=100", var1->toString().c_str());

    ASSERT_TRUE(ctx.Eval("var1 = 999"));
    ASSERT_STREQ("var1=999", var1->toString().c_str());
    ASSERT_FALSE(ctx.select("var1").complete());

    ASSERT_TRUE(ctx.Eval("var1 = _"));
    ASSERT_EQ(var1->getType(), ObjType::None);
    ASSERT_STREQ("var1=_", var1->toString().c_str());
    ASSERT_FALSE(ctx.select("var1").complete());

    list = ctx.Eval("$");
    ASSERT_STREQ("$=('var1',)", list->toString().c_str());

    //    ASSERT_TRUE(ctx.Eval("var1 = "));
    //    ASSERT_TRUE(ctx.select("var1").complete());
    ctx.clear_();

    list = ctx.Eval("$");
    ASSERT_STREQ("$=(,)", list->toString().c_str());

    ObjPtr var_str = ctx.Eval("var_str := 'Строка'");
    ASSERT_TRUE(var_str);
    ASSERT_TRUE(var_str->is_string_type());
    ASSERT_EQ(var_str->m_var_type_current, ObjType::StrChar);
    //    ASSERT_EQ(var_str->m_var_type_fixed, ObjType::String);
    ASSERT_STREQ("var_str='Строка'", var_str->toString().c_str());
    ASSERT_FALSE(ctx.select("var_str").complete());

    list = ctx.Eval("$");
    ASSERT_STREQ("$=('var_str',)", list->toString().c_str());

    ObjPtr var_num = ctx.Eval("var_num := 123.456: Number");
    ASSERT_TRUE(var_num);
    ASSERT_TRUE(var_num->is_arithmetic_type());
    ASSERT_TRUE(var_num->is_tensor());
    //    ASSERT_EQ(var_num->m_var_type_current, ObjType::Double);
    //    ASSERT_EQ(var_num->m_var_type_fixed, ObjType::Number);
    ASSERT_STREQ("var_num=123.456", var_num->toString().c_str());

    list = ctx.Eval("$");
    ASSERT_STREQ("$=('var_str', 'var_num',)", list->toString().c_str());

    var_long = 987654321;
    ObjPtr var_export = ctx.Eval("var_export := @import(\"var_long:Long\")");
    ASSERT_TRUE(var_export);
    ASSERT_TRUE(var_export->is_tensor()) << var_export;
    ASSERT_EQ(var_export->getType(), ObjType::Long);
    ASSERT_STREQ("var_export=987654321", var_export->toString().c_str());
    var_long = 123132132;
    ASSERT_STREQ("var_export=123132132", var_export->toString().c_str());
    var_export->SetValue_(Obj::CreateValue(59875, ObjType::None));
    ASSERT_EQ(59875, var_long);

    list = ctx.Eval("$");
    ASSERT_STREQ("$=('var_str', 'var_num', 'var_export',)", list->toString().c_str());

    ObjPtr func_export = ctx.Eval("func_export := @import(\"func_export(arg1:Long, arg2:Char=100):Long\")");
    ASSERT_TRUE(func_export);
    ASSERT_TRUE(func_export->is_function()) << func_export;
    ASSERT_EQ(func_export->getType(), ObjType::NativeFunc);
    ASSERT_STREQ("func_export=func_export(arg1:Long, arg2:Char=100):Long{}", func_export->toString().c_str());

    ObjPtr result = func_export->Call(nullptr, Obj::Arg(200), Obj::Arg(10));
    ASSERT_TRUE(result);
    ASSERT_EQ(210, result->GetValueAsInteger());

    result = func_export->Call(nullptr, Obj::Arg(10), Obj::Arg(10));
    ASSERT_TRUE(result);
    ASSERT_EQ(20, result->GetValueAsInteger());

    result = func_export->Call(nullptr, Obj::Arg(10));
    ASSERT_TRUE(result);
    ASSERT_EQ(110, result->GetValueAsInteger());

    // Переполнение второго аргумента
    ASSERT_THROW(func_export->Call(nullptr, Obj::Arg(1000), Obj::Arg(1000)), std::exception);

    list = ctx.Eval("$");
    ASSERT_STREQ("$=('var_str', 'var_num', 'var_export', 'func_export',)", list->toString().c_str());

    var_num.reset();
    func_export.reset();

    list = ctx.Eval("$");
    ASSERT_STREQ("$=('var_str', 'var_export',)", list->toString().c_str());

    // Функция возвращает словарь с именами объектов в текущем контексте
    ObjPtr func_eval = ctx.Eval("func_eval(arg1, arg2) := {$;}");
    ASSERT_TRUE(func_eval);
    ASSERT_TRUE(func_eval->is_function()) << func_eval;
    ASSERT_EQ(func_eval->getType(), ObjType::EVAL_FUNCTION) << toString(func_eval->getType());
    ASSERT_STREQ("func_eval=func_eval(arg1, arg2):={$;}", func_eval->toString().c_str());

    ObjPtr result_eval = func_eval->Call(&ctx, Obj::Arg(200), Obj::Arg(10));
    ASSERT_TRUE(result_eval);
    ASSERT_STREQ("$=('$0', 'arg1', 'arg2', 'var_str', 'var_export', 'func_eval',)", result_eval->toString().c_str());

    list = ctx.Eval("$");
    ASSERT_STREQ("$=('var_str', 'var_export', 'func_eval',)", list->toString().c_str());

    ObjPtr tensor3 = ctx.Eval("t:Int[2,3] := 123");
    ASSERT_TRUE(tensor3);
    ASSERT_EQ(ObjType::Int, tensor3->m_var_type_current) << toString(tensor3->m_var_type_current);
    ASSERT_EQ(ObjType::Int, tensor3->m_var_type_fixed) << toString(tensor3->m_var_type_fixed);
    ASSERT_EQ(2, tensor3->m_value.dim());
    ASSERT_EQ(2, tensor3->m_value.size(0));
    ASSERT_EQ(3, tensor3->m_value.size(1));

    for (int x = 0; x < tensor3->size(0); x++) {
        for (int y = 0; y < tensor3->size(1); y++) {
            ASSERT_STREQ("123", tensor3->index_get({x, y})->GetValueAsString().c_str());
        }
    }

    ObjPtr dict1 = ctx.Eval("(10, 2,  3,   4,   )");
    ASSERT_TRUE(dict1);
    ASSERT_EQ(ObjType::Dictionary, dict1->m_var_type_current) << toString(dict1->m_var_type_current);
    ASSERT_EQ(ObjType::None, dict1->m_var_type_fixed) << toString(dict1->m_var_type_fixed);
    ASSERT_EQ(4, dict1->size());
    ASSERT_STREQ("(10, 2, 3, 4,)", dict1->toString().c_str());

    ObjPtr dict2 = ctx.Eval("( (10, 2,  3,   4, (1,2,),   ), (10, 2,  3,   4,   ),)");
    ASSERT_TRUE(dict2);
    ASSERT_EQ(ObjType::Dictionary, dict2->m_var_type_current) << toString(dict2->m_var_type_current);
    ASSERT_EQ(ObjType::None, dict2->m_var_type_fixed) << toString(dict2->m_var_type_fixed);
    ASSERT_EQ(2, dict2->size());
    ASSERT_STREQ("((10, 2, 3, 4, (1, 2,),), (10, 2, 3, 4,),)", dict2->toString().c_str());

    ObjPtr tensor = ctx.Eval("[1,1,0,0,]");
    ASSERT_TRUE(tensor);
    ASSERT_EQ(ObjType::Bool, tensor->m_var_type_current) << toString(tensor->m_var_type_current);
    ASSERT_EQ(ObjType::None, tensor->m_var_type_fixed) << toString(tensor->m_var_type_fixed);
    ASSERT_EQ(1, tensor->m_value.dim());
    ASSERT_EQ(4, tensor->m_value.size(0));
    ASSERT_EQ(1, tensor->index_get({0})->GetValueAsInteger());
    ASSERT_EQ(1, tensor->index_get({1})->GetValueAsInteger());
    ASSERT_EQ(0, tensor->index_get({2})->GetValueAsInteger());
    ASSERT_EQ(0, tensor->index_get({3})->GetValueAsInteger());

    ASSERT_STREQ("[1, 1, 0, 0,]:Bool", tensor->GetValueAsString().c_str());

    ObjPtr tensor2 = ctx.Eval("[222,333,3333,]");
    ASSERT_TRUE(tensor2);
    ASSERT_STREQ("[222, 333, 3333,]:Short", tensor2->GetValueAsString().c_str());

    ObjPtr tensorf = ctx.Eval("[1.2, 0.22, 0.69,]");
    ASSERT_TRUE(tensorf);
    ASSERT_STREQ("[1.2, 0.22, 0.69,]:Double", tensorf->GetValueAsString().c_str());

    ObjPtr tensor_all = ctx.Eval("[ [1, 1, 0, 0,], [10, 10, 0.1, 0.2,], ]");
    ASSERT_TRUE(tensor_all);
    ASSERT_EQ(ObjType::Double, tensor_all->m_var_type_current) << toString(tensor_all->m_var_type_current);
    ASSERT_EQ(ObjType::None, tensor_all->m_var_type_fixed) << toString(tensor_all->m_var_type_fixed);
    ASSERT_EQ(2, tensor_all->m_value.dim());
    ASSERT_EQ(2, tensor_all->m_value.size(0));
    ASSERT_EQ(4, tensor_all->m_value.size(1));

    ASSERT_STREQ("1", tensor_all->index_get({0, 0})->GetValueAsString().c_str());
    ASSERT_STREQ("1", tensor_all->index_get({0, 1})->GetValueAsString().c_str());
    ASSERT_STREQ("0", tensor_all->index_get({0, 2})->GetValueAsString().c_str());
    ASSERT_STREQ("0", tensor_all->index_get({0, 3})->GetValueAsString().c_str());

    ASSERT_STREQ("10", tensor_all->index_get({1, 0})->GetValueAsString().c_str());
    ASSERT_STREQ("10", tensor_all->index_get({1, 1})->GetValueAsString().c_str());
    ASSERT_STREQ("0.1", tensor_all->index_get({1, 2})->GetValueAsString().c_str());
    ASSERT_STREQ("0.2", tensor_all->index_get({1, 3})->GetValueAsString().c_str());

    ASSERT_STREQ("[\n  [1, 1, 0, 0,], [10, 10, 0.1, 0.2,],\n]:Double", tensor_all->GetValueAsString().c_str());
}

TEST(Eval, Tensor) {

    Context ctx(RunTime::Init());

    ObjPtr tensor = ctx.Eval(":Tensor(1)");
    ASSERT_TRUE(tensor);
    ASSERT_EQ(ObjType::Tensor, tensor->m_var_type_fixed) << toString(tensor->m_var_type_fixed);
    ASSERT_EQ(ObjType::Bool, tensor->getType()) << toString(tensor->m_var_type_current);
    ASSERT_EQ(0, tensor->size());

    ASSERT_STREQ("1", tensor->GetValueAsString().c_str()) << tensor->GetValueAsString().c_str();

    tensor = ctx.Eval("(1,2,3,)");
    ASSERT_TRUE(tensor);
    ASSERT_STREQ("(1, 2, 3,)", tensor->GetValueAsString().c_str()) << tensor->GetValueAsString().c_str();

    tensor = ctx.Eval(":Dictionary( [1,2,3,] )");
    ASSERT_TRUE(tensor);
    ASSERT_STREQ("(1, 2, 3,)", tensor->GetValueAsString().c_str()) << tensor->GetValueAsString().c_str();

    tensor = ctx.Eval(":Tensor([1,2,3,])");
    ASSERT_TRUE(tensor);
    ASSERT_STREQ("[1, 2, 3,]:Char", tensor->GetValueAsString().c_str()) << tensor->GetValueAsString().c_str();

    tensor = ctx.Eval(":Int([1,])");
    ASSERT_TRUE(tensor);
    ASSERT_STREQ("[1,]:Int", tensor->GetValueAsString().c_str()) << tensor->GetValueAsString().c_str();

    ObjPtr tt = ctx.Eval(":Tensor[3]( (1,2,3,) )");
    ASSERT_TRUE(tt);
    ASSERT_STREQ("[1, 2, 3,]:Char", tt->GetValueAsString().c_str()) << tensor->GetValueAsString().c_str();

    tt = ctx.Eval(":Int((1,2,3,))");
    ASSERT_TRUE(tt);
    ASSERT_STREQ("[1, 2, 3,]:Int", tt->GetValueAsString().c_str()) << tensor->GetValueAsString().c_str();

    tt = ctx.Eval(":Int[2,3]((1,2,3,4,5,6,))");
    ASSERT_TRUE(tt);

    EXPECT_EQ(2, tt->m_value.dim());
    EXPECT_EQ(2, tt->m_value.size(0));
    EXPECT_EQ(3, tt->m_value.size(1));

    ASSERT_STREQ("[\n  [1, 2, 3,], [4, 5, 6,],\n]:Int", tt->GetValueAsString().c_str());

    ObjPtr str = ctx.Eval(":Dictionary('first second')");
    ASSERT_TRUE(str);
    ASSERT_STREQ("(102, 105, 114, 115, 116, 32, 115, 101, 99, 111, 110, 100,)", str->GetValueAsString().c_str());

    str = ctx.Eval(":Tensor('first second')");
    ASSERT_TRUE(str);
    ASSERT_STREQ("[102, 105, 114, 115, 116, 32, 115, 101, 99, 111, 110, 100,]:Char", str->GetValueAsString().c_str());

    tt = ctx.Eval(":Tensor((first='first', space=32, second='second',))");
    ASSERT_TRUE(tt);
    ASSERT_STREQ("[102, 105, 114, 115, 116, 32, 115, 101, 99, 111, 110, 100,]:Char", tt->GetValueAsString().c_str());

    ASSERT_TRUE(str->op_equal(tt));

    tt = ctx.Eval(":Int[6,2](\"Тензор Int  \")");
    ASSERT_TRUE(tt);
    ASSERT_STREQ("[\n  [1058, 1077,], [1085, 1079,], [1086, 1088,], [32, 73,], "
            "[110, 116,], [32, 32,],\n]:Int",
            tt->GetValueAsString().c_str());

    tt = ctx.Eval(":Tensor(99)");
    ASSERT_TRUE(tt);
    ASSERT_STREQ("99", tt->GetValueAsString().c_str());

    tt = ctx.Eval(":Double[10,2](0, ... )");
    ASSERT_TRUE(tt);
    ASSERT_STREQ("[\n  [0, 0,], [0, 0,], [0, 0,], [0, 0,], [0, 0,], [0, 0,], [0, "
            "0,], [0, 0,], [0, 0,], [0, 0,],\n]:Double",
            tt->GetValueAsString().c_str());

    ObjPtr rand = ctx.Eval("rand := @import('rand():Int')");

    // Может быть раскрытие словаря, который возвращает вызов функции
    // и может быть многократный вызов одной и той функции
    // :Int[3,2]( ... rand() ... )
    tt = ctx.Eval(":Int[3,2]( ... rand() ... )");
    ASSERT_TRUE(tt);
    std::string rand_str = tt->GetValueAsString();
    ASSERT_TRUE(50 < tt->GetValueAsString().size()) << rand_str;

    tt = ctx.Eval(":Int[5,2]( 0..10 )");
    ASSERT_TRUE(tt);
    ASSERT_STREQ("[\n  [0, 1,], [2, 3,], [4, 5,], [6, 7,], [8, 9,],\n]:Int", tt->GetValueAsString().c_str());

    tt = ctx.Eval(":Double[5,2]( 0..10 )");
    ASSERT_TRUE(tt);
    ASSERT_STREQ("[\n  [0, 1,], [2, 3,], [4, 5,], [6, 7,], [8, 9,],\n]:Double", tt->GetValueAsString().c_str());

    tt = ctx.Eval("0..1..0.1");
    ASSERT_TRUE(tt);
    ASSERT_STREQ("0..1..0.1", tt->toString().c_str());

    tt = ctx.Eval(":Dictionary( 0..0.99..0.1 )");
    ASSERT_TRUE(tt);
    ASSERT_STREQ("(0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9,)", tt->GetValueAsString().c_str());

    tt = ctx.Eval(":Tensor( 0..0.99..0.1 )");
    ASSERT_TRUE(tt);
    ASSERT_STREQ("[0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9,]:Double", tt->GetValueAsString().c_str());
}

TEST(Eval, FuncSimple) {

    Context ctx(RunTime::Init());

    ObjPtr test_and = ctx.Eval("test_and(arg1, arg2) &&= $arg1 == $arg2, $arg1");
    ASSERT_TRUE(test_and);

    //    EXPECT_FALSE((*test_and)(ctx, Object::Arg(123, "arg1"),
    //    Object::Arg(555, "arg2"))->GetValueAsBoolean());
    //    EXPECT_TRUE((*test_and)(ctx, Object::Arg(123, "arg1"),
    //    Object::Arg(123, "arg2"))->GetValueAsBoolean());
    //    EXPECT_TRUE((*test_and)(ctx, Object::Arg(555, "arg1"),
    //    Object::Arg(555, "arg2"))->GetValueAsBoolean());
    //    EXPECT_FALSE((*test_and)(ctx, Object::Arg(0, "arg1"), Object::Arg(0,
    //    "arg2"))->GetValueAsBoolean());

    ObjPtr test_or = ctx.Eval("test_or(arg1, arg2) ||= $arg1 == 555, $arg1");
    ASSERT_TRUE(test_or);

    //    EXPECT_TRUE(test_or->Call(ctx, Object::Arg(123, "arg1"),
    //    Object::Arg(555, "arg2"))->GetValueAsBoolean());
    //    EXPECT_TRUE(test_or->Call(ctx, Object::Arg(555, "arg1"),
    //    Object::Arg(555, "arg2"))->GetValueAsBoolean());
    //    EXPECT_TRUE(test_or->Call(ctx, Object::Arg(123, "arg1"),
    //    Object::Arg(123, "arg2"))->GetValueAsBoolean());
    //    EXPECT_TRUE(test_or->Call(ctx, Object::Arg(555, "arg1"),
    //    Object::Arg(0, "arg2"))->GetValueAsBoolean());
    //    EXPECT_FALSE(test_or->Call(ctx, Object::Arg(0, "arg1"), Object::Arg(0,
    //    "arg2"))->GetValueAsBoolean());

    ObjPtr test_xor = ctx.Eval("test_xor(arg1, arg2) ^^= $arg1 == $arg2, $arg1");
    ASSERT_TRUE(test_xor);

    //    EXPECT_TRUE(test_xor->Call(ctx, Object::Arg(123, "arg1"),
    //    Object::Arg(555, "arg2"))->GetValueAsBoolean());
    //    EXPECT_FALSE(test_xor->Call(ctx, Object::Arg(555, "arg1"),
    //    Object::Arg(555, "arg2"))->GetValueAsBoolean());
    //    EXPECT_FALSE(test_xor->Call(ctx, Object::Arg(123, "arg1"),
    //    Object::Arg(123, "arg2"))->GetValueAsBoolean());
    //    EXPECT_TRUE(test_xor->Call(ctx, Object::Arg(0, "arg1"), Object::Arg(0,
    //    "arg2"))->GetValueAsBoolean());
}

TEST(Eval, Types) {

    Context::Reset();
    Context ctx(RunTime::Init());

    ASSERT_EQ(42, ctx.m_types.size());

    std::vector<std::wstring> types = ctx.SelectPredict(":");
    ASSERT_EQ(2, types.size());

    //    ASSERT_STREQ(":Bool", utf8_encode(types[0]).c_str());
    //    ASSERT_STREQ(":Char", utf8_encode(types[1]).c_str());

    types = ctx.SelectPredict(":", 5);
    ASSERT_EQ(7, types.size());

    ObjPtr file = ctx.Eval(":File ::= :Pointer;");
    ASSERT_TRUE(file);

    types = ctx.SelectPredict(":File");
    ASSERT_EQ(1, types.size());

    //    ObjPtr f_stdout = ctx.CreateNative("stdout:File");
    //    ASSERT_TRUE(f_stdout);
    //    ASSERT_TRUE(f_stdout->m_func_ptr); // 0x555555cf39c0
    //    <stdout@@GLIBC_2.2.5> fputs("TEST STDOUT", stdout); // 0x7fff6ec7e760
    //    <_IO_2_1_stdout_> fputs("TEST STDOUT", (FILE *)f_stdout->m_func_ptr);

    //    //stdout:File ::= import("stdout:File");
    //    ObjPtr f2_stdout = ctx.Eval("stdout:File ::= import('stdout:File')");
    //    ASSERT_TRUE(f2_stdout);
    //    ASSERT_TRUE(f2_stdout->m_func_ptr);
    //    ASSERT_EQ(f_stdout->m_func_ptr, f2_stdout->m_func_ptr);
    //    //    ASSERT_EQ(f_stdout->m_func_ptr, (void *)stdout);

    ObjPtr fopen = ctx.CreateNative("fopen(filename:StrChar, modes:StrChar):File");
    ASSERT_TRUE(fopen);
    ASSERT_TRUE(fopen->m_func_ptr);

    ObjPtr fopen2 = ctx.Eval("@fopen2 ::= import('fopen(filename:StrChar, modes:StrChar):File')");
    ASSERT_TRUE(fopen2);
    ASSERT_TRUE(fopen2->m_func_ptr);
    ASSERT_EQ(fopen->m_func_ptr, fopen2->m_func_ptr);
    ASSERT_TRUE(ctx.FindTerm("fopen2"));
    auto iter = ctx.m_global_terms.select("fopen2");
    ASSERT_TRUE(!iter.complete());

    ObjPtr fopen3 = ctx.Eval("@fopen3(filename:String, modes:String):File ::= "
            "import('fopen(filename:StrChar, modes:StrChar):File')");
    ASSERT_TRUE(fopen3);
    ASSERT_TRUE(fopen3->m_func_ptr);
    ASSERT_EQ(fopen->m_func_ptr, fopen3->m_func_ptr);

    ObjPtr fclose = ctx.Eval("@fclose(stream:File):Int ::= import(\"fclose(stream:File):Int\")");
    ASSERT_TRUE(fclose);
    ASSERT_TRUE(fclose->m_func_ptr);

    ObjPtr fremove = ctx.Eval("@fremove(filename:String):Int ::= "
            "import(\"remove(filename:StrChar):Int\")");
    ASSERT_TRUE(fremove);
    ASSERT_TRUE(fremove->m_func_ptr);

    ObjPtr frename = ctx.Eval("@rename(old:String, new:String):Int ::= "
            "import('rename(old:StrChar, new:StrChar):Int')");
    ASSERT_TRUE(frename);
    ASSERT_TRUE(frename->m_func_ptr);

    ObjPtr fprintf = ctx.Eval("@fprintf(stream:File, format:Format, ...):Int ::= "
            "import('fprintf(stream:File, format:Format, ...):Int')");
    ASSERT_TRUE(fremove);
    ObjPtr fputc = ctx.Eval("@fputc(c:Int, stream:File):Int ::= "
            "import('fputc(c:Int, stream:File):Int')");
    ASSERT_TRUE(fremove);
    ObjPtr fputs = ctx.Eval("@fputs(s:String, stream:File):Int ::= "
            "import('fputs(s:StrChar, stream:File):Int')");
    ASSERT_TRUE(fputs);

    std::filesystem::create_directories("temp");
    ASSERT_TRUE(std::filesystem::is_directory("temp"));
    
    ObjPtr F = fopen->Call(&ctx, Obj::Arg("temp/ffile.temp"), Obj::Arg("w+"));
    ASSERT_TRUE(F);
    ASSERT_TRUE(F->GetValueAsInteger());
    ASSERT_TRUE(fputs->Call(&ctx, Obj::Arg("test fopen()\ntest fputs()\n"), Obj::Arg(F)));

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    ASSERT_TRUE(fprintf->Call(&ctx, Obj::Arg(F), Obj::Arg("%s"), Obj::Arg(oss.str())));

    ASSERT_TRUE(fclose->Call(&ctx, Obj::Arg(F)));

    ObjPtr F2 = ctx.Eval("F2 ::= fopen2('temp/ffile_eval.temp','w+')");
    ASSERT_TRUE(F2);
    ObjPtr F_res = ctx.Eval("fputs('test from eval !!!!!!!!!!!!!!!!!!!!\\n', F2)");
    ASSERT_TRUE(F_res);

    /*
     * Enum (перечисление) использование символьного названия поля вместо "магического" числа
     * 
     * :EnumStruct = :Enum(One:Int=1, Two=..., Three=10) - все поля имеют имена и автоматическую нумерацию + могут иметь тип
     * :EnumStruct.One
     * var = :EnumStruct(thread, gpu);
     * 
     * :TypeStruct = :Struct(One:Int=1, Two:Char=0, Three=10) - все типы поле определены
     * :Class(One:Int=1, Two=..., Three=10)
     * :Class3 = :Class1(One:Int=1, Two=..., Three=10), Class2(One:Int=1, Two=..., Three=10);
     * 
     * (One:Int=1, Two=_, Three=10,):Enum(thread, gpu) использовать тип значения в имени поля вместо общего типа ----- Enum(One, Two=2, Three=3):Int  ------
     * [[ One, Two=2, Three=3 ]]:Enum(thread, gpu)
     * 
     * :Seek::SET или @Seek::SET - статическое зачение у глобального типа
     * Seek.SET или $Seek.SET - статическое зачение у глобального типа
     */

    ObjPtr SEEK1 = ctx.Eval("@SEEK1 ::= :Enum(SET:Int=10, \"CUR\", END=20)");
    ASSERT_TRUE(SEEK1);

    ASSERT_EQ(3, SEEK1->size());
    ASSERT_TRUE((*SEEK1)[0]);
    ASSERT_EQ(ObjType::Char, (*SEEK1)[0]->getType()) << newlang::toString((*SEEK1)[0]->getType());
    ASSERT_EQ(ObjType::Char, (*SEEK1)[0]->m_var_type_fixed) << newlang::toString((*SEEK1)[0]->m_var_type_fixed);
    ASSERT_EQ(10, (*SEEK1)[0]->GetValueAsInteger());

    ASSERT_TRUE((*SEEK1)[1]);
    ASSERT_EQ(ObjType::Char, (*SEEK1)[1]->getType()) << newlang::toString((*SEEK1)[1]->getType());
    ASSERT_EQ(ObjType::Char, (*SEEK1)[1]->m_var_type_fixed) << newlang::toString((*SEEK1)[1]->m_var_type_fixed);
    ASSERT_EQ(11, (*SEEK1)[1]->GetValueAsInteger());

    ASSERT_TRUE((*SEEK1)[2]);
    ASSERT_EQ(ObjType::Char, (*SEEK1)[2]->getType()) << newlang::toString((*SEEK1)[2]->getType());
    ASSERT_EQ(ObjType::Char, (*SEEK1)[2]->m_var_type_fixed) << newlang::toString((*SEEK1)[2]->m_var_type_fixed);
    ASSERT_EQ(20, (*SEEK1)[2]->GetValueAsInteger());

    ASSERT_EQ(10, (*SEEK1)["SET"]->GetValueAsInteger());
    ASSERT_EQ(11, (*SEEK1)["CUR"]->GetValueAsInteger());
    ASSERT_EQ(20, (*SEEK1)["END"]->GetValueAsInteger());

    ObjPtr SEEK2 = ctx.Eval("@SEEK2 ::= :Enum(SET=, CUR=, END=300)");
    ASSERT_TRUE(SEEK2);
    ASSERT_EQ(3, SEEK2->size());
    ASSERT_EQ(0, (*SEEK2)[0]->GetValueAsInteger());
    ASSERT_EQ(ObjType::Bool, (*SEEK2)[0]->getType());
    ASSERT_EQ(1, (*SEEK2)[1]->GetValueAsInteger());
    ASSERT_EQ(ObjType::Bool, (*SEEK2)[1]->getType());
    ASSERT_EQ(300, (*SEEK2)[2]->GetValueAsInteger());
    ASSERT_EQ(ObjType::Short, (*SEEK2)[2]->getType());
    ASSERT_EQ(0, (*SEEK2)["SET"]->GetValueAsInteger());
    ASSERT_EQ(1, (*SEEK2)["CUR"]->GetValueAsInteger());
    ASSERT_EQ(300, (*SEEK2)["END"]->GetValueAsInteger());

    ObjPtr SEEK = ctx.Eval("@SEEK ::= :Enum(SET=0, CUR=1, END=2)");
    ASSERT_TRUE(SEEK);

    ASSERT_EQ(3, SEEK->size());
    ASSERT_EQ(0, (*SEEK)[0]->GetValueAsInteger());
    ASSERT_EQ(1, (*SEEK)[1]->GetValueAsInteger());
    ASSERT_EQ(2, (*SEEK)[2]->GetValueAsInteger());
    ASSERT_EQ(0, (*SEEK)["SET"]->GetValueAsInteger());
    ASSERT_EQ(1, (*SEEK)["CUR"]->GetValueAsInteger());
    ASSERT_EQ(2, (*SEEK)["END"]->GetValueAsInteger());

    F_res = ctx.Eval("@SEEK.SET");
    ASSERT_TRUE(F_res);
    ASSERT_EQ(0, F_res->GetValueAsInteger());
    F_res = ctx.Eval("@SEEK.CUR");
    ASSERT_TRUE(F_res);
    ASSERT_EQ(1, F_res->GetValueAsInteger());
    F_res = ctx.Eval("@SEEK.END");
    ASSERT_TRUE(F_res);
    ASSERT_EQ(2, F_res->GetValueAsInteger());

    ObjPtr seek = ctx.Eval("fseek(stream:File, offset:Long, whence:Int):Int ::= "
            "import('fseek(stream:File, offset:Long, whence:Int):Int')");
    ASSERT_TRUE(seek);

    F_res = ctx.Eval("fseek(F2, 10, @SEEK.SET)");
    ASSERT_TRUE(F_res);
    ASSERT_EQ(0, F_res->GetValueAsInteger());

    F_res = ctx.Eval("fclose(F2)");
    ASSERT_TRUE(F_res);

    // extern size_t fread (void *__restrict __ptr, size_t __size,
    //		     size_t __n, FILE *__restrict __stream) __wur;
    //
    // extern size_t fwrite (const void *__restrict __ptr, size_t __size,
    //		      size_t __n, FILE *__restrict __s);
}

TEST(Eval, Fileio) {

    Context::Reset();
    Context ctx(RunTime::Init());

    ASSERT_NO_THROW(ctx.ExecFile("nlp/fileio.nlp"));

    ASSERT_TRUE(ctx.FindTerm("fopen"));
    ASSERT_TRUE(ctx.FindTerm("fputs"));
    ASSERT_TRUE(ctx.FindTerm("fclose"));

    std::filesystem::create_directories("temp");
    ASSERT_TRUE(std::filesystem::is_directory("temp"));
    
    ObjPtr file = ctx.Eval("file ::= fopen('temp/ffile_eval2.temp','w+')");
    ASSERT_TRUE(file);
    ObjPtr file_res = ctx.Eval("fputs('test 222 from eval !!!!!!!!!!!!!!!!!!!!\\n', file)");
    ASSERT_TRUE(file_res);
    file_res = ctx.Eval("fclose(file)");
    ASSERT_TRUE(file_res);

    //@todo try and catch segfault
    //    ASSERT_ANY_THROW(
    //            // Double free
    //            file_res = ctx.Eval("fclose(file)"););
    //            
    //    Context::Reset();
}

TEST(Eval, Funcs) {

    Context::Reset();
    Context ctx(RunTime::Init());

    ObjPtr printf = ctx.Eval("printf := @import('printf(format:Format, ...):Int');");
    ASSERT_TRUE(printf);
    ASSERT_STREQ("printf=printf(format:Format, ...):Int{}", printf->toString().c_str());

    ObjPtr hello = ctx.Eval("hello(str='') := {printf('%s', $str); $str;}");
    ASSERT_TRUE(hello);
    ASSERT_STREQ("hello=hello(str=''):={printf('%s', $str); $str;}", hello->toString().c_str());

    ObjPtr result = ctx.Eval("hello('Привет, мир!\\n');");
    ASSERT_TRUE(result);
    ASSERT_STREQ("Привет, мир!\n", result->GetValueAsString().c_str());
}

/*
 * scalar_int := 100:Int; # Тип скаляра во время компиляции
 * scalar_int := 100:Int(__device__="GPU"); # Тип скаляра во время компиляции
 * scalar_int := 100:Int(); ?????? # Тип скаляра во время компиляции
 * :type_int := :Int; # Синоним типа Int во время компиляции (тип не может быть изменен)
 * :type_int := :Int(); # Копия типа Int во время выполнения (тип может быть изменен после Mutable)
 * 
 * scalar_int := :Int(0); # Преобразование типа во время выполнения с автоматической размерностью (скаляр)
 * scalar_int := :Int[0](0); # Преобразование типа во время выполнения с указанием размерности (скаляр)
 * scalar_int := :Int[0]([0,]); # Преобразование типа во время выполнения с указанием размерности (скаляр)
 * 
 * tensor_int := :Int([0,]); # Преобразование типа во время выполнения с автоматической размерностью (тензор)
 * tensor_int := :Int[1](0); # Преобразование типа во время выполнения с указанием размерности (тензор)
 * tensor_int := :Int[...](0); # Преобразование типа во время выполнения с произвольной размернотью (тензор)
 * 
 * 
 * :синоним := :Int; # Неизменяемый ?????
 * :копия := :Int(); # Изменяемый ?????
 * 
 * Mutable(:синоним); # Ошибка
 * Mutable(:копия); # Норма
 * 
 * :Int(1); # Не меняет размерность, только тип !!!!
 * :Int[2, ...](100, 200); # Одномерный тензор Int произвольного размера
 * :Int([,], 100, 200); # Объединить аргументы, если их несколько и преобразовать их тип к Int
 * :Int[0](10000000); # Преобразовать тип к скаляру
 * :Int[2](100, 200); # Преобразовать аргументы в тензор указанной размерности и заданного типа
 * :Int[2,1](100, 200); # Преобразовать аргументы в тензор указанной размерности и заданного типа
 * :Int[1,2](100, 200); # Преобразовать аргументы в тензор указанной размерности и заданного типа
 * 
 */
TEST(Eval, Convert) {

    /*
     * - Встроеные функции преобразования простых типов данных
     * - Передача аргументов функци по ссылкам
     */

    RuntimePtr opts = RunTime::Init();
    Context ctx(opts);

    ObjPtr type_int = ctx.Eval(":Int");
    ASSERT_TRUE(type_int);
    ASSERT_EQ(ObjType::Type, type_int->getType()) << toString(type_int->m_var_type_current);
    ASSERT_EQ(ObjType::Int, type_int->m_var_type_fixed) << toString(type_int->m_var_type_fixed);
    ASSERT_EQ(0, type_int->size());


    ObjPtr type_dim = ctx.Eval(":Int[0]");
    ASSERT_TRUE(type_dim);
    ASSERT_EQ(ObjType::Type, type_dim->getType()) << toString(type_dim->m_var_type_current);
    ASSERT_EQ(ObjType::Int, type_dim->m_var_type_fixed) << toString(type_dim->m_var_type_fixed);

    ASSERT_TRUE(type_dim->m_type);
    ASSERT_EQ(1, type_dim->m_type->size());
    ASSERT_EQ(0, (*type_dim->m_type)[0]->GetValueAsInteger());

    ObjPtr type_ell = ctx.Eval(":Int[10, ...]");
    ASSERT_TRUE(type_ell);
    ASSERT_EQ(ObjType::Type, type_ell->getType());
    ASSERT_EQ(ObjType::Int, type_ell->m_var_type_fixed);

    ASSERT_TRUE(type_ell->m_type);
    ASSERT_EQ(2, type_ell->m_type->size());
    ASSERT_EQ(10, (*type_ell->m_type)[0]->GetValueAsInteger());
    ASSERT_EQ(ObjType::Ellipsis, (*type_ell->m_type)[1]->getType());



    ObjPtr obj_0 = ctx.Eval("0");
    ASSERT_TRUE(obj_0);
    ASSERT_EQ(at::ScalarType::Bool, obj_0->m_value.scalar_type());
    ASSERT_EQ(ObjType::Bool, obj_0->getType());
    ASSERT_EQ(ObjType::None, obj_0->m_var_type_fixed);
    ASSERT_STREQ("0", obj_0->GetValueAsString().c_str());

    ObjPtr obj_1 = ctx.Eval("1");
    ASSERT_TRUE(obj_1);
    ASSERT_EQ(at::ScalarType::Bool, obj_1->m_value.scalar_type());
    ASSERT_EQ(ObjType::Bool, obj_1->getType());
    ASSERT_EQ(ObjType::None, obj_1->m_var_type_fixed);
    ASSERT_STREQ("1", obj_1->GetValueAsString().c_str());

    ObjPtr obj_2 = ctx.Eval("2");
    ASSERT_TRUE(obj_2);
    ASSERT_EQ(at::ScalarType::Char, obj_2->m_value.scalar_type());
    ASSERT_EQ(ObjType::Char, obj_2->getType());
    ASSERT_EQ(ObjType::None, obj_2->m_var_type_fixed);
    ASSERT_STREQ("2", obj_2->GetValueAsString().c_str());

    ObjPtr obj_int = ctx.Eval(":Int(0)");
    ASSERT_TRUE(obj_int);
    ASSERT_TRUE(obj_int->is_tensor());
    ASSERT_TRUE(obj_int->is_scalar());
    ASSERT_EQ(at::ScalarType::Int, obj_int->m_value.scalar_type());
    ASSERT_EQ(ObjType::Int, obj_int->getType()) << toString(obj_int->m_var_type_current);
    ASSERT_EQ(ObjType::Int, obj_int->m_var_type_fixed) << toString(obj_int->m_var_type_fixed);

    ASSERT_TRUE(obj_int->m_var_is_init);
    ASSERT_STREQ("0", obj_int->GetValueAsString().c_str());
    ASSERT_EQ(0, obj_int->GetValueAsInteger());


    ObjPtr scalar = ctx.Eval(":Int[0](0)");
    ASSERT_TRUE(scalar);
    ASSERT_TRUE(scalar->is_tensor());
    ASSERT_TRUE(scalar->is_scalar());
    ASSERT_EQ(at::ScalarType::Int, scalar->m_value.scalar_type());
    ASSERT_EQ(ObjType::Int, scalar->getType()) << toString(scalar->m_var_type_current);
    ASSERT_EQ(ObjType::Int, scalar->m_var_type_fixed) << toString(scalar->m_var_type_fixed);

    ASSERT_TRUE(scalar->m_var_is_init);
    ASSERT_STREQ("0", scalar->GetValueAsString().c_str());
    ASSERT_EQ(0, scalar->GetValueAsInteger());


    ObjPtr ten = ctx.Eval("[0,]");
    ASSERT_TRUE(ten);
    ASSERT_TRUE(ten->is_tensor());
    ASSERT_FALSE(ten->is_scalar());

    ASSERT_TRUE(ten->m_var_is_init);
    ASSERT_STREQ("[0,]:Bool", ten->GetValueAsString().c_str());
    ASSERT_EQ(0, ten->GetValueAsInteger());



    ObjPtr obj_ten = ctx.Eval(":Int([0,])");
    ASSERT_TRUE(obj_ten);
    ASSERT_TRUE(obj_ten->is_tensor());
    ASSERT_FALSE(obj_ten->is_scalar());
    ASSERT_EQ(at::ScalarType::Int, obj_ten->m_value.scalar_type());
    ASSERT_EQ(ObjType::Int, obj_ten->getType()) << toString(obj_ten->m_var_type_current);
    ASSERT_EQ(ObjType::Int, obj_ten->m_var_type_fixed) << toString(obj_ten->m_var_type_fixed);

    ASSERT_TRUE(obj_ten->m_var_is_init);
    ASSERT_STREQ("[0,]:Int", obj_ten->GetValueAsString().c_str());
    ASSERT_EQ(0, obj_ten->GetValueAsInteger());


    ObjPtr obj_auto = ctx.Eval(":Int(0, 1, 2, 3)");
    ASSERT_TRUE(obj_auto);
    ASSERT_EQ(at::ScalarType::Int, obj_auto->m_value.scalar_type());
    ASSERT_EQ(ObjType::Int, obj_auto->getType()) << toString(obj_auto->m_var_type_current);
    ASSERT_EQ(ObjType::Int, obj_auto->m_var_type_fixed) << toString(obj_auto->m_var_type_fixed);

    ASSERT_TRUE(obj_auto->m_var_is_init);
    ASSERT_STREQ("[0, 1, 2, 3,]:Int", obj_auto->GetValueAsString().c_str());


    ObjPtr obj_float = ctx.Eval(":Float[2,2](3, 4, 1, 2)");
    ASSERT_TRUE(obj_float);
    ASSERT_EQ(ObjType::Float, obj_float->getType()) << toString(obj_float->m_var_type_current);
    ASSERT_EQ(ObjType::Float, obj_float->m_var_type_fixed) << toString(obj_float->m_var_type_fixed);

    ASSERT_TRUE(obj_float->m_var_is_init);
    ASSERT_STREQ("[\n  [3, 4,], [1, 2,],\n]:Float", obj_float->GetValueAsString().c_str());


    //    EXPECT_TRUE(dlsym(nullptr, "_ZN7newlang4CharEPKNS_7ContextERKNS_6ObjectE"));
    //    EXPECT_TRUE(dlsym(nullptr, "_ZN7newlang6Short_EPNS_7ContextERNS_6ObjectE"));


    //    ASSERT_TRUE(opts->GetProcAddress(MangaledFunc("Bool").c_str()));
    //    ASSERT_TRUE(opts->GetProcAddress(MangaledFunc("Char").c_str()));
    //    ASSERT_TRUE(opts->GetProcAddress(MangaledFunc("Short").c_str()));
    //    ASSERT_TRUE(opts->GetProcAddress(MangaledFunc("Int").c_str()));
    //    ASSERT_TRUE(opts->GetProcAddress(MangaledFunc("Long").c_str()));
    //    //    ASSERT_TRUE(opts->GetProcAddress(MangaledFunc("Half").c_str()));
    //    ASSERT_TRUE(opts->GetProcAddress(MangaledFunc("Float").c_str()));
    //    ASSERT_TRUE(opts->GetProcAddress(MangaledFunc("Double").c_str()));
    //    ASSERT_TRUE(opts->GetProcAddress(MangaledFunc("ComplexFloat").c_str()));
    //    ASSERT_TRUE(opts->GetProcAddress(MangaledFunc("ComplexDouble").c_str()));
    //    //    ASSERT_TRUE(opts->GetProcAddress(MangaledFunc("BFloat16").c_str()));
    //    ASSERT_TRUE(opts->GetProcAddress(MangaledFunc("StrChar").c_str()));
    //    ASSERT_TRUE(opts->GetProcAddress(MangaledFunc("StrWide").c_str()));
    //    ASSERT_TRUE(opts->GetProcAddress(MangaledFunc("Dictionary").c_str()));
    //    ASSERT_TRUE(opts->GetProcAddress(MangaledFunc("Class").c_str()));
    //    ASSERT_TRUE(opts->GetProcAddress(MangaledFunc("None").c_str()));
    //    ASSERT_TRUE(opts->GetProcAddress(MangaledFunc("Integer").c_str()));
    //    ASSERT_TRUE(opts->GetProcAddress(MangaledFunc("Number").c_str()));
    //    ASSERT_TRUE(opts->GetProcAddress(MangaledFunc("Complex").c_str()));
    //    ASSERT_TRUE(opts->GetProcAddress(MangaledFunc("String").c_str()));
    //
    //
    //    ObjPtr byte = Object::CreateValue(1, ObjType::Bool);
    //    ASSERT_EQ(ObjType::Bool, byte->m_var_type_current) << toString(byte->m_var_type_current);
    //
    //    ASSERT_TRUE(ctx.CallByName("Char", Object::Arg(byte)));
    //    ASSERT_EQ(ObjType::Char, ctx.CallByName("Char", Object::Arg(byte))->m_var_type_current) << toString(byte->m_var_type_current);
    //    ASSERT_EQ(ObjType::Bool, byte->m_var_type_current) << toString(byte->m_var_type_current);
    //
    //    ASSERT_EQ(ObjType::Char, newlang_Char(nullptr, Object::Arg(byte))->m_var_type_current) << toString(byte->m_var_type_current);
    //    //    ASSERT_STREQ("1", newlang_Char(nullptr, Object::Arg(byte))->toString().c_str());
    //
    //    ASSERT_EQ(ObjType::Short, newlang_Short(nullptr, Object::Arg(byte))->m_var_type_current) << toString(byte->m_var_type_current);
    //    ASSERT_EQ(ObjType::Int, newlang_Int(nullptr, Object::Arg(byte))->m_var_type_current) << toString(byte->m_var_type_current);
    //    ASSERT_EQ(ObjType::Long, newlang_Long(nullptr, Object::Arg(byte))->m_var_type_current) << toString(byte->m_var_type_current);
    //    //    ASSERT_EQ(ObjType::Half, newlang_Half(nullptr, Object::Arg(byte))->m_var_type) << toString(byte->m_var_type);
    //    ASSERT_EQ(ObjType::Float, newlang_Float(nullptr, Object::Arg(byte))->m_var_type_current) << toString(byte->m_var_type_current);
    //    ASSERT_EQ(ObjType::Double, newlang_Double(nullptr, Object::Arg(byte))->m_var_type_current) << toString(byte->m_var_type_current);
    //
    //    ASSERT_EQ(ObjType::Long, newlang_Integer(nullptr, Object::Arg(byte))->m_var_type_current) << toString(byte->m_var_type_current);
    //    ASSERT_EQ(ObjType::Float, newlang_Number(nullptr, Object::Arg(byte))->m_var_type_current) << toString(byte->m_var_type_current);
    //
    //    //    ASSERT_EQ(ObjType::Byte, newlang_Byte_(nullptr, Object::Arg(byte))->m_var_type) << toString(byte->m_var_type);
    //    //    ASSERT_EQ(ObjType::Byte, byte->m_var_type) << toString(byte->m_var_type);
    //
    //
    //    //    ASSERT_EQ(ObjType::Byte, ctx.CallByName("Byte_", Object::Arg(byte))->m_var_type) << toString(byte->m_var_type);
    //    //    ASSERT_EQ(ObjType::Byte, byte->m_var_type) << toString(byte->m_var_type);
    //
    //    // @todo Для работы функций mutable функций требуется передача по ссылкам !!!!!!!!!!!!1
    //
    //    //    ASSERT_EQ(ObjType::Char, ctx.CallByName("Char_", Object::Arg(byte))->m_var_type) << toString(byte->m_var_type);
    //    //    ASSERT_EQ(ObjType::Char, byte->m_var_type) << toString(byte->m_var_type);
    //    //
    //    //    ASSERT_EQ(ObjType::Double, ctx.CallByName("Double_", Object::Arg(byte))->m_var_type) << toString(byte->m_var_type);
    //    //    ASSERT_EQ(ObjType::Double, byte->m_var_type) << toString(byte->m_var_type);
    //
    //    //    Object::CreateFromType();
    //
    //    ObjPtr value = Object::CreateNone();
    //    ObjPtr range1 = Object::CreateRange(0, 0.99, 0.1);
    //
    //    ASSERT_TRUE(range1);
    //    ASSERT_EQ(range1->getType(), ObjType::Range);
    //
    //    ASSERT_NO_THROW(ConvertRangeToDict(range1.get(), *value.get()));
    //    ASSERT_TRUE(value);
    //
    //    ASSERT_EQ(value->getType(), ObjType::Dictionary);
    //    ASSERT_EQ(10, value->size());
    //
    //    ASSERT_DOUBLE_EQ(0, (*value)[0]->GetValueAsNumber());
    //    ASSERT_DOUBLE_EQ(0.1, (*value)[1]->GetValueAsNumber());
    //    ASSERT_DOUBLE_EQ(0.2, (*value)[2]->GetValueAsNumber());
    //    ASSERT_DOUBLE_EQ(0.3, (*value)[3]->GetValueAsNumber());
    //    ASSERT_DOUBLE_EQ(0.4, (*value)[4]->GetValueAsNumber());
    //    ASSERT_DOUBLE_EQ(0.5, (*value)[5]->GetValueAsNumber());
    //    ASSERT_DOUBLE_EQ(0.6, (*value)[6]->GetValueAsNumber());
    //    ASSERT_DOUBLE_EQ(0.7, (*value)[7]->GetValueAsNumber());
    //    ASSERT_DOUBLE_EQ(0.8, (*value)[8]->GetValueAsNumber());
    //    ASSERT_DOUBLE_EQ(0.9, (*value)[9]->GetValueAsNumber());
    //
    //
    //    ObjPtr range2 = Object::CreateRange(0, -5);
    //
    //    ASSERT_TRUE(range2);
    //    ASSERT_EQ(range2->getType(), ObjType::Range);
    //    ASSERT_EQ(0, (*range2)["start"]->GetValueAsInteger());
    //    ASSERT_EQ(-5, (*range2)["stop"]->GetValueAsInteger());
    //    ASSERT_EQ(-1, (*range2)["step"]->GetValueAsInteger());
    //
    //    ASSERT_NO_THROW(ConvertRangeToDict(range2.get(), *value.get()));
    //    ASSERT_TRUE(value);
    //
    //    ASSERT_EQ(value->getType(), ObjType::Dictionary);
    //    ASSERT_EQ(15, value->size());
    //
    //    ASSERT_EQ(0, (*value)[10]->GetValueAsInteger());
    //    ASSERT_EQ(-1, (*value)[11]->GetValueAsInteger());
    //    ASSERT_EQ(-2, (*value)[12]->GetValueAsInteger());
    //    ASSERT_EQ(-3, (*value)[13]->GetValueAsInteger());
    //    ASSERT_EQ(-4, (*value)[14]->GetValueAsInteger());
    //
    //    torch::Tensor tensor1 = torch::empty({0});
    //    ASSERT_NO_THROW(ConvertStringToTensor("test", tensor1));
    //
    //    ASSERT_EQ(1, tensor1.dim());
    //    ASSERT_EQ(4, tensor1.size(0));
    //    ASSERT_EQ('t', tensor1.index({0}).item<int>());
    //    ASSERT_EQ('e', tensor1.index({1}).item<int>());
    //    ASSERT_EQ('s', tensor1.index({2}).item<int>());
    //    ASSERT_EQ('t', tensor1.index({3}).item<int>());
    //
    //    torch::Tensor tensor2 = torch::empty({0});
    //    ASSERT_NO_THROW(ConvertStringToTensor(L"TESTЁ", tensor2));
    //    ASSERT_EQ(1, tensor2.dim());
    //    ASSERT_EQ(5, tensor2.size(0));
    //    ASSERT_EQ(L'T', tensor2.index({0}).item<int>());
    //    ASSERT_EQ(L'E', tensor2.index({1}).item<int>());
    //    ASSERT_EQ(L'S', tensor2.index({2}).item<int>());
    //    ASSERT_EQ(L'T', tensor2.index({3}).item<int>());
    //    ASSERT_EQ(L'Ё', tensor2.index({4}).item<int>());
    //
    //    torch::Tensor tensor_utf = torch::empty({0});
    //    ASSERT_NO_THROW(ConvertStringToTensor(L"Русё", tensor_utf));
    //    ASSERT_EQ(1, tensor_utf.dim());
    //    ASSERT_EQ(4, tensor_utf.size(0));
    //    ASSERT_EQ(L'Р', tensor_utf.index({0}).item<int>());
    //    ASSERT_EQ(L'у', tensor_utf.index({1}).item<int>());
    //    ASSERT_EQ(L'с', tensor_utf.index({2}).item<int>());
    //    ASSERT_EQ(L'ё', tensor_utf.index({3}).item<int>());
    //
    //
    //    std::string str1;
    //    ASSERT_NO_THROW(ConvertTensorToString(torch::range(1, 4, 1), str1));
    //    ASSERT_STREQ("\x1\x2\x3\x4", str1.c_str());
    //
    //    std::wstring str2;
    //    ASSERT_NO_THROW(ConvertTensorToString(torch::range(0x1001, 0x4008, 0x1002), str2));
    //    ASSERT_EQ(4, str2.size());
    //    ASSERT_EQ(L'\x10\x01', str2[0]);
    //    ASSERT_EQ(L'\x20\x03', str2[1]);
    //    ASSERT_EQ(L'\x30\x05', str2[2]);
    //    ASSERT_EQ(L'\x40\x07', str2[3]);
    //
    //    //    ASSERT_EQ(0x1001, str2[0]);
    //    //    ASSERT_EQ(0x2003, str2[1]);
    //    //    ASSERT_EQ(0x3005, str2[2]);
    //    //    ASSERT_EQ(0x4007, str2[3]);
    //
    //    std::string str3;
    //    int array[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    //    torch::Tensor tensor4 = torch::from_blob(array,{2, 4}, at::kInt);
    //
    //    ASSERT_EQ(2, tensor4.dim());
    //    ASSERT_EQ(2, tensor4.size(0));
    //    ASSERT_EQ(4, tensor4.size(1));
    //    ASSERT_NO_THROW(ConvertTensorToString(tensor4, str1));
    //    ASSERT_STREQ("\x1\x2\x3\x4\x5\x6\x7\x8", str1.c_str());
    //
    //
    //    ObjPtr dict1 = Object::CreateNone();
    //    ASSERT_NO_THROW(ConvertTensorToDict(tensor1, *dict1.get()));
    //    ASSERT_EQ(4, dict1->size());
    //    ASSERT_EQ('t', (*dict1)[0]->GetValueAsInteger());
    //    ASSERT_EQ('e', (*dict1)[1]->GetValueAsInteger());
    //    ASSERT_EQ('s', (*dict1)[2]->GetValueAsInteger());
    //    ASSERT_EQ('t', (*dict1)[3]->GetValueAsInteger());
    //
    //    ASSERT_NO_THROW(ConvertTensorToDict(tensor4, *dict1.get()));
    //    ASSERT_EQ(12, dict1->size());
    //    ASSERT_EQ('t', (*dict1)[0]->GetValueAsInteger());
    //    ASSERT_EQ('e', (*dict1)[1]->GetValueAsInteger());
    //    ASSERT_EQ('s', (*dict1)[2]->GetValueAsInteger());
    //    ASSERT_EQ('t', (*dict1)[3]->GetValueAsInteger());
    //    ASSERT_EQ(1, (*dict1)[4]->GetValueAsInteger());
    //    ASSERT_EQ(2, (*dict1)[5]->GetValueAsInteger());
    //    ASSERT_EQ(3, (*dict1)[6]->GetValueAsInteger());
    //    ASSERT_EQ(4, (*dict1)[7]->GetValueAsInteger());
    //    ASSERT_EQ(5, (*dict1)[8]->GetValueAsInteger());
    //    ASSERT_EQ(6, (*dict1)[9]->GetValueAsInteger());
    //    ASSERT_EQ(7, (*dict1)[10]->GetValueAsInteger());
    //    ASSERT_EQ(8, (*dict1)[11]->GetValueAsInteger());
    //
    //    ASSERT_NO_THROW(ConvertTensorToDict(tensor2, *dict1.get()));
    //    ASSERT_EQ(17, dict1->size());
    //
    //    ASSERT_EQ(L'T', (*dict1)[12]->GetValueAsInteger());
    //    ASSERT_EQ(L'E', (*dict1)[13]->GetValueAsInteger());
    //    ASSERT_EQ(L'S', (*dict1)[14]->GetValueAsInteger());
    //    ASSERT_EQ(L'T', (*dict1)[15]->GetValueAsInteger());
    //    ASSERT_EQ(L'Ё', (*dict1)[16]->GetValueAsInteger());
    //
    //    torch::Tensor result = torch::empty({0});
    //    ASSERT_NO_THROW(ConvertDictToTensor(*dict1.get(), result));
    //    ASSERT_EQ(1, result.dim());
    //    ASSERT_EQ(17, result.size(0));
    //
    //
    //    ASSERT_EQ('t', result.index({0}).item<int>()) << TensorToString(result) << "\n";
    //    ASSERT_EQ('e', result.index({1}).item<int>());
    //    ASSERT_EQ('s', result[2].item<int>());
    //    ASSERT_EQ('t', result[3].item<int>());
    //    ASSERT_EQ(1, result[4].item<int>());
    //    ASSERT_EQ(2, result[5].item<int>());
    //    ASSERT_EQ(3, result[6].item<int>());
    //    ASSERT_EQ(4, result[7].item<int>());
    //    ASSERT_EQ(5, result[8].item<int>());
    //    ASSERT_EQ(6, result[9].item<int>());
    //    ASSERT_EQ(7, result[10].item<int>());
    //    ASSERT_EQ(8, result[11].item<int>());
    //
    //    ASSERT_EQ(L'T', result[12].item<int>());
    //    ASSERT_EQ(L'E', result[13].item<int>());
    //    ASSERT_EQ(L'S', result[14].item<int>());
    //    ASSERT_EQ(L'T', result[15].item<int>());
    //    ASSERT_EQ(L'Ё', result[16].item<int>());

}

class OpEvalTest : public ::testing::Test {
protected:
    Context m_ctx;
    ObjPtr m_result;
    std::string m_string;

    OpEvalTest() : m_ctx(RunTime::Init()) {
    }

    const char *Test(std::string eval, Obj &vars) {
        eval += ";";
        m_result = m_ctx.Eval(eval, &vars);
        if(m_result) {
            m_string = m_result->GetValueAsString();
            return m_string.c_str();
        }
        std::cout << "Fail parsing: '" << eval << "'\n";
        ADD_FAILURE();

        return nullptr;
    }

    const char *Test(const char *eval) {
        Obj vars;

        return Test(eval, vars);
    }
};

TEST_F(OpEvalTest, Ops) {
    ASSERT_STREQ("10", Test("10"));
    ASSERT_STREQ("32", Test("10+22"));
    ASSERT_STREQ("5.1", Test("1.1+4"));
    ASSERT_STREQ("5.5", Test("1+4.5"));

    ASSERT_STREQ("-12", Test("10 - 22"));
    ASSERT_STREQ("-2.9", Test("1.1 - 4"));
    ASSERT_STREQ("-3.5", Test("1 - 4.5"));

    ASSERT_STREQ("66", Test("2 * 33"));
    ASSERT_STREQ("-5.5", Test("1.1 * -5"));
    ASSERT_STREQ("180", Test("10 * 18"));

    ASSERT_STREQ("5", Test("10/2"));
    ASSERT_STREQ("5.05", Test("10.1 / 2"));
    ASSERT_STREQ("0.1", Test("1 / 10"));
    ASSERT_STREQ("0.1", Test("1.0 / 10"));

    ASSERT_STREQ("5", Test("10//2"));
    ASSERT_STREQ("5", Test("10.0 // 2"));
    ASSERT_STREQ("0", Test("1 // 10"));
    ASSERT_STREQ("-3", Test("-25 // 10"));
    ASSERT_STREQ("-3", Test("-30 // 10"));
    ASSERT_STREQ("-4", Test("-31 // 10"));

    //    ASSERT_STREQ("100", Test("2*20+10*5"));

    //    ASSERT_STREQ("1", Test("1 ** 1"));
    //    ASSERT_STREQ("4", Test("2 ** 2"));
    //    ASSERT_STREQ("-8.0", Test("-2.0 ** 3"));

    ASSERT_STREQ("", Test("\"\""));
    ASSERT_STREQ(" ", Test("\" \""));
    ASSERT_STREQ("строка", Test("\"\"++\"строка\" "));
    ASSERT_STREQ("строка 222", Test("\"строка \" ++ \"222\" "));
    ASSERT_STREQ("строка строка строка ", Test("\"строка \" ** 3 "));

    ASSERT_STREQ("100", Test("var1:=100"));
    ObjPtr var1 = m_result;
    ASSERT_TRUE(var1);
    ASSERT_STREQ("$=('var1',)", Test("$"));
    ASSERT_STREQ("100", Test("var1"));

    Obj vars(Obj::Arg(var1, "var1"));

    ASSERT_THROW(Test("$var1"), std::out_of_range);
    ASSERT_NO_THROW(Test("$var1", vars));
    ASSERT_STREQ("100", Test("$var1", vars));

    ASSERT_STREQ("20", Test("var2:=9+11"));
    ObjPtr var2 = m_result;
    ASSERT_TRUE(var2);
    ASSERT_STREQ("$=('var1', 'var2',)", Test("$"));
    ASSERT_STREQ("20", Test("var2"));

    ASSERT_THROW(Test("$var2"), std::out_of_range);
    ASSERT_THROW(Test("$var2", vars), std::out_of_range);
    vars.push_back(Obj::Arg(var2, "var2"));

    ASSERT_NO_THROW(Test("$var2", vars));
    ASSERT_STREQ("20", Test("$var2", vars));

    ASSERT_STREQ("100", Test("var1"));
    ASSERT_STREQ("120", Test("var1+=var2"));
    ASSERT_STREQ("$=('var1', 'var2',)", Test("$"));

    ASSERT_THROW(Test("$var1"), std::out_of_range);
    ASSERT_NO_THROW(Test("$var1", vars));
    ASSERT_STREQ("120", Test("$var1", vars));

    vars.clear_();
    m_result.reset();
    var1.reset();
    ASSERT_STREQ("$=('var2',)", Test("$"));
    var2.reset();
    ASSERT_STREQ("$=(,)", Test("$"));
}

#endif // UNITTEST