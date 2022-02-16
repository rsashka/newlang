#ifdef UNITTEST

#include <core/pch.h>

#include <signal.h>

#include <core/warning_push.h>
#include <gtest/gtest.h>
#include <core/warning_pop.h>

#include <core/newlang.h>
#include <core/builtin.h>


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

    ASSERT_TRUE(ctx.Eval("var1 = "));
    ASSERT_TRUE(ctx.select("var1").complete());

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
    var_export->SetValue_(Object::CreateValue(59875));
    ASSERT_EQ(59875, var_long);

    list = ctx.Eval("$");
    ASSERT_STREQ("$=('var_str', 'var_num', 'var_export',)", list->toString().c_str());

    ObjPtr func_export = ctx.Eval("func_export := @import(\"func_export(arg1:Long, arg2:Char=100):Long\")");
    ASSERT_TRUE(func_export);
    ASSERT_TRUE(func_export->is_function()) << func_export;
    ASSERT_EQ(func_export->getType(), ObjType::NativeFunc);
    ASSERT_STREQ("func_export=func_export(arg1:Long, arg2:Char=100):Long{}", func_export->toString().c_str());

    ObjPtr result = func_export->Call(nullptr, Object::Arg(200), Object::Arg(10));
    ASSERT_TRUE(result);
    ASSERT_EQ(210, result->GetValueAsInteger());

    result = func_export->Call(nullptr, Object::Arg(10), Object::Arg(10));
    ASSERT_TRUE(result);
    ASSERT_EQ(20, result->GetValueAsInteger());

    result = func_export->Call(nullptr, Object::Arg(10));
    ASSERT_TRUE(result);
    ASSERT_EQ(110, result->GetValueAsInteger());

    // Переполнение второго аргумента
    ASSERT_THROW(func_export->Call(nullptr, Object::Arg(1000), Object::Arg(1000)), std::exception);

    list = ctx.Eval("$");
    ASSERT_STREQ("$=('var_str', 'var_num', 'var_export', 'func_export',)", list->toString().c_str());

    var_num.reset();
    func_export.reset();

    list = ctx.Eval("$");
    ASSERT_STREQ("$=('var_str', 'var_export',)", list->toString().c_str());


    // Функиця возвражщает словарь с именами объектов в текущем контексте
    ObjPtr func_eval = ctx.Eval("func_eval(arg1, arg2) := {$;}");
    ASSERT_TRUE(func_eval);
    ASSERT_TRUE(func_eval->is_function()) << func_eval;
    ASSERT_EQ(func_eval->getType(), ObjType::EVAL_FUNCTION);
    ASSERT_STREQ("func_eval=func_eval(arg1, arg2):={$;}", func_eval->toString().c_str());

    ObjPtr result_eval = func_eval->Call(&ctx, Object::Arg(200), Object::Arg(10));
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
    ASSERT_EQ(ObjType::Dict, dict1->m_var_type_current) << toString(dict1->m_var_type_current);
    ASSERT_EQ(ObjType::None, dict1->m_var_type_fixed) << toString(dict1->m_var_type_fixed);
    ASSERT_EQ(4, dict1->size());
    ASSERT_STREQ("(10, 2, 3, 4,)", dict1->toString().c_str());

    ObjPtr dict2 = ctx.Eval("( (10, 2,  3,   4, (1,2,),   ), (10, 2,  3,   4,   ))");
    ASSERT_TRUE(dict2);
    ASSERT_EQ(ObjType::Dict, dict2->m_var_type_current) << toString(dict2->m_var_type_current);
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

    ObjPtr tensor = ctx.Eval("[[ 1 ]]");
    ASSERT_TRUE(tensor);
    ASSERT_STREQ("[1,]:Bool", tensor->GetValueAsString().c_str());

    tensor = ctx.Eval("[[ [1,2,] ]]");
    ASSERT_TRUE(tensor);
    ASSERT_STREQ("[1, 2,]:Char", tensor->GetValueAsString().c_str());

    tensor = ctx.Eval("[[ 1 ]]:Int");
    ASSERT_TRUE(tensor);
    ASSERT_STREQ("[1,]:Int", tensor->GetValueAsString().c_str());

    ObjPtr tt = ctx.Eval("[[(1,2,3)]]");
    ASSERT_TRUE(tt);
    ASSERT_STREQ("[1, 2, 3,]:Char", tt->GetValueAsString().c_str());

    tt = ctx.Eval("[[(1,2,3)]]:Int");
    ASSERT_TRUE(tt);
    ASSERT_STREQ("[1, 2, 3,]:Int", tt->GetValueAsString().c_str());

    tt = ctx.Eval("[[(1,2,3,4,5,6)]]:Int[2,3]");
    ASSERT_TRUE(tt);
    ASSERT_STREQ("[\n  [1, 2, 3,], [4, 5, 6,],\n]:Int", tt->GetValueAsString().c_str());

    ObjPtr str = ctx.Eval("[['first second']]");
    ASSERT_TRUE(str);
    ASSERT_STREQ("[102, 105, 114, 115, 116, 32, 115, 101, 99, 111, 110, 100,]:Char", str->GetValueAsString().c_str());

    tt = ctx.Eval("[[(first='first', space=32, second='second')]]");
    ASSERT_TRUE(tt);
    ASSERT_STREQ("[102, 105, 114, 115, 116, 32, 115, 101, 99, 111, 110, 100,]:Char", tt->GetValueAsString().c_str());

    ASSERT_TRUE(str->op_equal(tt));

    tt = ctx.Eval("[[\"Тензор Int  \"]]:Int[6,2]");
    ASSERT_TRUE(tt);
    ASSERT_STREQ("[\n  [1058, 1077,], [1085, 1079,], [1086, 1088,], [32, 73,], [110, 116,], [32, 32,],\n]:Int", tt->GetValueAsString().c_str());


    tt = ctx.Eval("[[99]]");
    ASSERT_TRUE(tt);
    ASSERT_STREQ("[99,]:Char", tt->GetValueAsString().c_str());

    tt = ctx.Eval("[[ 0 ... ]]: Double[10,2]");
    ASSERT_TRUE(tt);
    ASSERT_STREQ("[\n  [0, 0,], [0, 0,], [0, 0,], [0, 0,], [0, 0,], [0, 0,], [0, 0,], [0, 0,], [0, 0,], [0, 0,],\n]:Double", tt->GetValueAsString().c_str());

    tt = ctx.Eval("[[ rand() ... ]]: Int[3,2]");
    ASSERT_TRUE(tt);
    ASSERT_TRUE(50 < tt->GetValueAsString().size()) << tt->GetValueAsString();

    tt = ctx.Eval("[[ 0..10 ]]: Int[5,2]");
    ASSERT_TRUE(tt);
    ASSERT_STREQ("[\n  [0, 1,], [2, 3,], [4, 5,], [6, 7,], [8, 9,],\n]:Int", tt->GetValueAsString().c_str());

    tt = ctx.Eval("[[ 0..10 ]]: Double[5,2]");
    ASSERT_TRUE(tt);
    ASSERT_STREQ("[\n  [0, 1,], [2, 3,], [4, 5,], [6, 7,], [8, 9,],\n]:Double", tt->GetValueAsString().c_str());

    tt = ctx.Eval("0..1..0.1");
    ASSERT_TRUE(tt);
    ASSERT_STREQ("0..1..0.1", tt->toString().c_str());
    
    tt = ctx.Eval("[[ 0..0.99..0.1 ]]");
    ASSERT_TRUE(tt);
    ASSERT_STREQ("[0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9,]:Double", tt->GetValueAsString().c_str());
}

TEST(Eval, FuncSimple) {

    Context ctx(RunTime::Init());

    ObjPtr test_and = ctx.Eval("test_and(arg1, arg2) &&= $arg1 == $arg2, $arg1");
    ASSERT_TRUE(test_and);

    //    EXPECT_FALSE((*test_and)(ctx, Object::Arg(123, "arg1"), Object::Arg(555, "arg2"))->GetValueAsBoolean());
    //    EXPECT_TRUE((*test_and)(ctx, Object::Arg(123, "arg1"), Object::Arg(123, "arg2"))->GetValueAsBoolean());
    //    EXPECT_TRUE((*test_and)(ctx, Object::Arg(555, "arg1"), Object::Arg(555, "arg2"))->GetValueAsBoolean());
    //    EXPECT_FALSE((*test_and)(ctx, Object::Arg(0, "arg1"), Object::Arg(0, "arg2"))->GetValueAsBoolean());

    ObjPtr test_or = ctx.Eval("test_or(arg1, arg2) ||= $arg1 == 555, $arg1");
    ASSERT_TRUE(test_or);


    //    EXPECT_TRUE(test_or->Call(ctx, Object::Arg(123, "arg1"), Object::Arg(555, "arg2"))->GetValueAsBoolean());
    //    EXPECT_TRUE(test_or->Call(ctx, Object::Arg(555, "arg1"), Object::Arg(555, "arg2"))->GetValueAsBoolean());
    //    EXPECT_TRUE(test_or->Call(ctx, Object::Arg(123, "arg1"), Object::Arg(123, "arg2"))->GetValueAsBoolean());
    //    EXPECT_TRUE(test_or->Call(ctx, Object::Arg(555, "arg1"), Object::Arg(0, "arg2"))->GetValueAsBoolean());
    //    EXPECT_FALSE(test_or->Call(ctx, Object::Arg(0, "arg1"), Object::Arg(0, "arg2"))->GetValueAsBoolean());

    ObjPtr test_xor = ctx.Eval("test_xor(arg1, arg2) ^^= $arg1 == $arg2, $arg1");
    ASSERT_TRUE(test_xor);

    //    EXPECT_TRUE(test_xor->Call(ctx, Object::Arg(123, "arg1"), Object::Arg(555, "arg2"))->GetValueAsBoolean());
    //    EXPECT_FALSE(test_xor->Call(ctx, Object::Arg(555, "arg1"), Object::Arg(555, "arg2"))->GetValueAsBoolean());
    //    EXPECT_FALSE(test_xor->Call(ctx, Object::Arg(123, "arg1"), Object::Arg(123, "arg2"))->GetValueAsBoolean());
    //    EXPECT_TRUE(test_xor->Call(ctx, Object::Arg(0, "arg1"), Object::Arg(0, "arg2"))->GetValueAsBoolean());
}

TEST(Eval, Types) {

    Context::Reset();
    Context ctx(RunTime::Init());

    ASSERT_EQ(20, ctx.m_types.size());

    std::vector<std::wstring> types = ctx.SelectPredict(":");
    ASSERT_EQ(2, types.size());

    ASSERT_STREQ(":Bool", utf8_encode(types[0]).c_str());
    ASSERT_STREQ(":Char", utf8_encode(types[1]).c_str());

    types = ctx.SelectPredict(":", 5);
    ASSERT_EQ(7, types.size());

    ObjPtr file = ctx.Eval(":File ::= :Pointer;");
    ASSERT_TRUE(file);

    types = ctx.SelectPredict(":File");
    ASSERT_EQ(1, types.size());


    //    ObjPtr f_stdout = ctx.CreateNative("stdout:File");
    //    ASSERT_TRUE(f_stdout);
    //    ASSERT_TRUE(f_stdout->m_func_ptr); // 0x555555cf39c0 <stdout@@GLIBC_2.2.5>
    //    fputs("TEST STDOUT", stdout); // 0x7fff6ec7e760 <_IO_2_1_stdout_>
    //    fputs("TEST STDOUT", (FILE *)f_stdout->m_func_ptr);

    //    //stdout:File ::= import("stdout:File");
    //    ObjPtr f2_stdout = ctx.Eval("stdout:File ::= import('stdout:File')");
    //    ASSERT_TRUE(f2_stdout);
    //    ASSERT_TRUE(f2_stdout->m_func_ptr);
    //    ASSERT_EQ(f_stdout->m_func_ptr, f2_stdout->m_func_ptr);
    //    //    ASSERT_EQ(f_stdout->m_func_ptr, (void *)stdout);


    ObjPtr fopen = ctx.CreateNative("fopen(filename:StrChar, modes:StrChar):File");
    ASSERT_TRUE(fopen);
    ASSERT_TRUE(fopen->m_func_ptr);

    ObjPtr fopen2 = ctx.Eval("fopen2 ::= import('fopen(filename:StrChar, modes:StrChar):File')");
    ASSERT_TRUE(fopen2);
    ASSERT_TRUE(fopen2->m_func_ptr);
    ASSERT_EQ(fopen->m_func_ptr, fopen2->m_func_ptr);

    ObjPtr fopen3 = ctx.Eval("fopen3(filename:String, modes:String):File ::= import('fopen(filename:StrChar, modes:StrChar):File')");
    ASSERT_TRUE(fopen3);
    ASSERT_TRUE(fopen3->m_func_ptr);
    ASSERT_EQ(fopen->m_func_ptr, fopen3->m_func_ptr);

    ObjPtr fclose = ctx.Eval("fclose(stream:File):Int ::= import(\"fclose(stream:File):Int\")");
    ASSERT_TRUE(fclose);
    ASSERT_TRUE(fclose->m_func_ptr);

    ObjPtr fremove = ctx.Eval("fremove(filename:String):Int ::= import(\"remove(filename:StrChar):Int\")");
    ASSERT_TRUE(fremove);
    ASSERT_TRUE(fremove->m_func_ptr);

    ObjPtr frename = ctx.Eval("rename(old:String, new:String):Int ::= import('rename(old:StrChar, new:StrChar):Int')");
    ASSERT_TRUE(frename);
    ASSERT_TRUE(frename->m_func_ptr);


    ObjPtr fprintf = ctx.Eval("fprintf(stream:File, format:Format, ...):Int ::= import('fprintf(stream:File, format:Format, ...):Int')");
    ASSERT_TRUE(fremove);
    ObjPtr fputc = ctx.Eval("fputc(c:Int, stream:File):Int ::= import('fputc(c:Int, stream:File):Int')");
    ASSERT_TRUE(fremove);
    ObjPtr fputs = ctx.Eval("fputs(s:String, stream:File):Int ::= import('fputs(s:StrChar, stream:File):Int')");
    ASSERT_TRUE(fputs);


    ObjPtr F = fopen->Call(&ctx, Object::Arg("ffile.temp"), Object::Arg("w+"));
    ASSERT_TRUE(F);
    ASSERT_TRUE(fputs->Call(&ctx, Object::Arg("test fopen()\ntest fputs()\n"), Object::Arg(F)));

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
    ASSERT_TRUE(fprintf->Call(&ctx, Object::Arg(F), Object::Arg("%s"), Object::Arg(oss.str())));

    ASSERT_TRUE(fclose->Call(&ctx, Object::Arg(F)));



    ObjPtr F2 = ctx.Eval("F2 ::= fopen2('ffile_eval.temp','w+')");
    ASSERT_TRUE(F2);
    ObjPtr F_res = ctx.Eval("fputs('test from eval !!!!!!!!!!!!!!!!!!!!\\n', F2)");
    ASSERT_TRUE(F_res);

    ObjPtr SEEK1 = ctx.Eval("@SEEK1 ::= (SET:Char=10, CUR=, END=20):Int");
    ASSERT_TRUE(SEEK1);

    ASSERT_EQ(3, SEEK1->size());
    ASSERT_EQ(10, (*SEEK1)[0]->GetValueAsInteger());
    ASSERT_EQ(ObjType::Char, (*SEEK1)[0]->getType());
    ASSERT_EQ(ObjType::Int, (*SEEK1)[0]->m_var_type_fixed);
    ASSERT_EQ(11, (*SEEK1)[1]->GetValueAsInteger());
    ASSERT_EQ(ObjType::Char, (*SEEK1)[1]->getType());
    ASSERT_EQ(ObjType::Int, (*SEEK1)[1]->m_var_type_fixed);
    ASSERT_EQ(20, (*SEEK1)[2]->GetValueAsInteger());
    ASSERT_EQ(ObjType::Char, (*SEEK1)[2]->getType());
    ASSERT_EQ(ObjType::Int, (*SEEK1)[2]->m_var_type_fixed);
    ASSERT_EQ(10, (*SEEK1)["SET"]->GetValueAsInteger());
    ASSERT_EQ(11, (*SEEK1)["CUR"]->GetValueAsInteger());
    ASSERT_EQ(20, (*SEEK1)["END"]->GetValueAsInteger());

    ObjPtr SEEK2 = ctx.Eval("@SEEK2 ::= (SET=, CUR=, END=300):Int");
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

    ObjPtr SEEK = ctx.Eval("@SEEK ::= (SET=0, CUR=1, END=2):Int");
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

    ObjPtr seek = ctx.Eval("fseek(stream:File, offset:Long, whence:Int):Int ::= import('fseek(stream:File, offset:Long, whence:Int):Int')");
    ASSERT_TRUE(seek);

    F_res = ctx.Eval("fseek(F2, 10, @SEEK.SET)");
    ASSERT_TRUE(F_res);
    ASSERT_EQ(0, F_res->GetValueAsInteger());

    F_res = ctx.Eval("fclose(F2)");
    ASSERT_TRUE(F_res);

    //extern size_t fread (void *__restrict __ptr, size_t __size,
    //		     size_t __n, FILE *__restrict __stream) __wur;
    //
    //extern size_t fwrite (const void *__restrict __ptr, size_t __size,
    //		      size_t __n, FILE *__restrict __s);

}

TEST(Eval, Fileio) {

    Context::Reset();
    Context ctx(RunTime::Init());

    ASSERT_NO_THROW(
            ctx.Exec("nlp/fileio.nlp")
            );

    ObjPtr file = ctx.Eval("file ::= fopen('ffile_eval2.temp','w+')");
    ASSERT_TRUE(file);
    ObjPtr file_res = ctx.Eval("fputs('test 222 from eval !!!!!!!!!!!!!!!!!!!!\\n', file)");
    ASSERT_TRUE(file_res);
    file_res = ctx.Eval("fclose(file)");
    ASSERT_TRUE(file_res);

    ASSERT_ANY_THROW(
            // Double free
            file_res = ctx.Eval("fclose(file)");
            );

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

class OpEvalTest : public ::testing::Test {
protected:

    Context m_ctx;
    ObjPtr m_result;
    std::string m_string;

    OpEvalTest() : m_ctx(RunTime::Init()) {
    }

    const char * Test(const char *eval, Object &vars) {
        m_result = m_ctx.Eval(eval, vars);
        if(m_result) {
            m_string = m_result->GetValueAsString();
            return m_string.c_str();
        }
        std::cout << "Fail parsing: '" << eval << "'\n";
        ADD_FAILURE();

        return nullptr;
    }

    const char * Test(const char *eval) {
        Object vars;

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

    Object vars(Object::Arg(var1, "var1"));

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
    vars.push_back(Object::Arg(var2, "var2"));

    ASSERT_NO_THROW(Test("$var2", vars));
    ASSERT_STREQ("20", Test("$var2", vars));


    ASSERT_STREQ("120", Test("var1+=var2"));
    ASSERT_STREQ("$=('var1', 'var2',)", Test("$"));
    ASSERT_STREQ("120", Test("var1"));

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