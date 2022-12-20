#include "pch.h"

#ifdef UNITTEST

#include <warning_push.h>
#include <gtest/gtest.h>
#include <warning_pop.h>

#include <parser.h>
#include <term.h>
#include "version.h"
#include "newlang.h"
#include "nlc.h"

using namespace newlang;

class ParserTest : public ::testing::Test {
protected:

    TermPtr Parse(std::string str, MacroBuffer *buffer = nullptr) {
        ast = Parser::ParseString(str, buffer);
        return ast;
    }

    int Count(TermID token_id) {
        int result = 0;
        for (int c = 0; c < ast->size(); c++) {
            if((*ast)[c].second->m_id == token_id) {
                result++;
            }
        }
        return result;
    }

    TermPtr ast;
};

TEST_F(ParserTest, LiteralInteger) {
    ASSERT_TRUE(Parse("100;"));
    ASSERT_EQ(TermID::INTEGER, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("100", ast->m_text.c_str());
}

TEST_F(ParserTest, LiteralNumber) {
    ASSERT_TRUE(Parse("100.222;"));
    ASSERT_EQ(TermID::NUMBER, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("100.222", ast->m_text.c_str());

    ASSERT_TRUE(Parse("1.2E-20;"));
    ASSERT_EQ(TermID::NUMBER, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("1.2E-20", ast->m_text.c_str());

    ASSERT_TRUE(Parse("1.2E+20;"));
    ASSERT_EQ(TermID::NUMBER, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("1.2E+20", ast->m_text.c_str());

    ASSERT_TRUE(Parse("0.e-10;"));
    ASSERT_EQ(TermID::NUMBER, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("0.e-10", ast->m_text.c_str());

    ASSERT_TRUE(Parse("0.e+10;"));
    ASSERT_EQ(TermID::NUMBER, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("0.e+10", ast->m_text.c_str());
}

TEST_F(ParserTest, LiteralString) {
    ASSERT_TRUE(Parse("\"\";"));
    ASSERT_EQ(TermID::STRWIDE, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("\"\"", ast->toString().c_str());
}

TEST_F(ParserTest, LiteralString0) {
    ASSERT_TRUE(Parse("\"\"(123);"));
    ASSERT_TRUE(Parse("\"\"(123);"));
    ASSERT_EQ(TermID::STRWIDE, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("\"\"(123)", ast->toString().c_str());
}

TEST_F(ParserTest, LiteralString1) {
    ASSERT_TRUE(Parse("\"\";"));
    ASSERT_EQ(TermID::STRWIDE, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("\"\"", ast->toString().c_str());
}

TEST_F(ParserTest, LiteralString3) {
    ASSERT_TRUE(Parse("'';"));
    ASSERT_EQ(TermID::STRCHAR, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("''", ast->toString().c_str());
}

TEST_F(ParserTest, LiteralString4) {
    ASSERT_TRUE(Parse("'strbyte'(term(), 123);"));
    ASSERT_EQ(TermID::STRCHAR, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("'strbyte'(term(), 123)", ast->toString().c_str());
}

TEST_F(ParserTest, LiteralEval1) {
    ASSERT_TRUE(Parse("``"));
    ASSERT_EQ(TermID::EVAL, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("``", ast->toString().c_str());
}

TEST_F(ParserTest, LiteralEval2) {
    ASSERT_TRUE(Parse("`strbyte(term(), 123);`"));
    ASSERT_EQ(TermID::EVAL, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("`strbyte(term(), 123);`", ast->toString().c_str());
}

TEST_F(ParserTest, LiteralRational) {
    ASSERT_TRUE(Parse("1\\1;"));
    ASSERT_EQ(TermID::RATIONAL, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("1\\1", ast->toString().c_str());

    /* Защита от случайной операции деления на единицу вместо указания дроби */
    ASSERT_ANY_THROW(Parse("1/1"));
    ASSERT_ANY_THROW(Parse("rrr := 1/1"));
    ASSERT_NO_THROW(Parse("rrr := 1\\1"));
    ASSERT_ANY_THROW(Parse("rrr := 11111111111111111/1"));
    ASSERT_NO_THROW(Parse("rrr := 11111111111111111\\1"));

    ASSERT_TRUE(Parse("100\\100;"));
    ASSERT_EQ(TermID::RATIONAL, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("100\\100", ast->toString().c_str());

    ASSERT_TRUE(Parse("123456789123456789123456789\\123456789123456789123456789;"));
    ASSERT_EQ(TermID::RATIONAL, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("123456789123456789123456789\\123456789123456789123456789", ast->toString().c_str());
}

TEST_F(ParserTest, DISABLED_LiteralComplex) {
    ASSERT_TRUE(Parse("1+0j;"));
    ASSERT_EQ(TermID::COMPLEX, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("1+0j", ast->toString().c_str());

    ASSERT_TRUE(Parse("1+0.1j;"));
    ASSERT_EQ(TermID::COMPLEX, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("1+0.1j", ast->toString().c_str());
}

TEST_F(ParserTest, TermSimple) {
    ASSERT_TRUE(Parse("term();"));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_TRUE(ast->isCall());
    ASSERT_STREQ("term", ast->m_text.c_str());
}

TEST_F(ParserTest, TermName) {
    ASSERT_TRUE(Parse("term.filed();"));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_TRUE(ast->m_name.empty());
}

TEST_F(ParserTest, Tensor1) {
    ASSERT_TRUE(Parse("[,]:Int8"));
    ASSERT_EQ(TermID::TENSOR, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("[,]:Int8", ast->toString().c_str());

    ASSERT_TRUE(Parse("term[1];"));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_TRUE(ast->Right());

    ASSERT_EQ(TermID::INDEX, ast->Right()->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_EQ(1, ast->Right()->size());
    ASSERT_STREQ("1", (*ast->Right())[0].second->getText().c_str());

    ASSERT_TRUE(Parse("term[1..2];"));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term[1..2]", ast->toString().c_str());

    //    ASSERT_TRUE(ast->Right());
    //    ASSERT_EQ(TermID::INDEX, ast->Right()->getTermID()) << EnumStr(ast->getTermID());
    //    ASSERT_EQ(2, ast->Right()->getItemCount());
    //    ASSERT_STREQ("1", (*ast->Right())[0]->getText().c_str());
    //    ASSERT_STREQ("2", (*ast->Right())[1]->getText().c_str());
}

TEST_F(ParserTest, Tensor2) {
    ASSERT_TRUE(Parse("term[1, 2];"));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_TRUE(ast->Right());

    ASSERT_EQ(TermID::INDEX, ast->Right()->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_EQ(2, ast->Right()->size());
    ASSERT_STREQ("1", (*ast->Right())[0].second->getText().c_str());
    ASSERT_STREQ("2", (*ast->Right())[1].second->getText().c_str());

    ASSERT_TRUE(Parse("term[1, 1..2, 3];"));

    ASSERT_EQ(TermID::INDEX, ast->Right()->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_EQ(3, ast->Right()->size());
    ASSERT_STREQ("1", (*ast->Right())[0].second->getText().c_str());
    ASSERT_STREQ("1..2", (*ast->Right())[1].second->toString().c_str());
    ASSERT_STREQ("3", (*ast->Right())[2].second->getText().c_str());
}

TEST_F(ParserTest, Tensor3) {
    ASSERT_TRUE(Parse("term := [1,2,];"));

    ASSERT_TRUE(Parse("term[1, 3] := 0;"));

    ASSERT_TRUE(Parse("term[1, 3] := [ [1,2,3,],];"));

    //    ASSERT_EQ(TermID::TERM, ast->getTermID()) << EnumStr(ast->getTermID());
    //    ASSERT_STREQ("term", ast->m_text.c_str());
    //    ASSERT_TRUE(ast->Right());
    //
    //    ASSERT_EQ(TermID::INDEX, ast->Right()->getTermID()) << EnumStr(ast->getTermID());
    //    ASSERT_EQ(2, ast->Right()->size());
    //    ASSERT_STREQ("1", (*ast->Right())[0]->getText().c_str());
    //    ASSERT_STREQ("2", (*ast->Right())[1]->getText().c_str());
    //
    //    ASSERT_TRUE(Parse("term[1, 1..2, 3];"));
    //
    //    ASSERT_EQ(TermID::INDEX, ast->Right()->getTermID()) << EnumStr(ast->getTermID());
    //    ASSERT_EQ(3, ast->Right()->size());
    //    ASSERT_STREQ("1", (*ast->Right())[0]->getText().c_str());
    //    ASSERT_STREQ("1..2", (*ast->Right())[1]->toString().c_str());
    //    ASSERT_STREQ("3", (*ast->Right())[2]->getText().c_str());
}

TEST_F(ParserTest, Tensor4) {
    ASSERT_TRUE(Parse(":Type( 1 );"));
    ASSERT_STREQ(":Type(1)", ast->toString().c_str());

    ASSERT_TRUE(Parse(":Type(  [1,2,] )"));
    ASSERT_STREQ(":Type([1, 2,])", ast->toString().c_str());

    ASSERT_TRUE(Parse(":Type( 1, 2 )"));
    ASSERT_STREQ(":Type(1, 2)", ast->toString().c_str());

    ASSERT_TRUE(Parse(":Type(   name=1  ,   name2=2 )"));
    ASSERT_STREQ(":Type(name=1, name2=2)", ast->toString().c_str());

    ASSERT_TRUE(Parse(":Type( \"str\" )"));
    ASSERT_STREQ(":Type(\"str\")", ast->toString().c_str());

    ASSERT_TRUE(Parse(":Int32[3]( \"str\" ) "));
    ASSERT_STREQ(":Int32[3](\"str\")", ast->toString().c_str());

    ASSERT_TRUE(Parse(":Int32[2,2](1,2,3,4);"));
    ASSERT_STREQ(":Int32[2,2](1, 2, 3, 4)", ast->toString().c_str());

    ASSERT_TRUE(Parse(":Int32[2,2]( 0,   ...    )"));
    ASSERT_STREQ(":Int32[2,2](0, ...)", ast->toString().c_str());

    ASSERT_TRUE(Parse(":Int32( ... ...  dict )"));
    ASSERT_STREQ(":Int32(... ...dict)", ast->toString().c_str());

    ASSERT_TRUE(Parse(":Int32( ... dict )"));
    ASSERT_STREQ(":Int32(...dict)", ast->toString().c_str());

    ASSERT_TRUE(Parse(":Int32[2,2](   ...   rand()  ...   )"));
    ASSERT_STREQ(":Int32[2,2](...rand()...)", ast->toString().c_str());

    ASSERT_TRUE(Parse(":type[10]( 1,     2,  ...    rand()   ... )"));
    ASSERT_STREQ(":type[10](1, 2, ...rand()...)", ast->toString().c_str());

    ASSERT_TRUE(Parse(":range( 0..100  )"));
    ASSERT_STREQ(":range(0..100)", ast->toString().c_str());

    ASSERT_TRUE(Parse("range(  0 .. 100 .. 0.1 )"));
    ASSERT_STREQ("range(0..100..0.1)", ast->toString().c_str());
}

/*
 * - Установка типов у литералов
 * - Проверка соответствия типов литералов и их значений
 * 
 * - Встроенные функции преобразования простых типов данных
 * - Передача аргументов функций по ссылкам
 * - Проверка типов аргументов при вызове функций
 * - Проверка типов возвращаемых значений у функций
 * - Проверка типов у операторов присвоения
 */
TEST_F(ParserTest, ScalarType) {
    /*
     * - Установка типов у литералов
     * - Проверка соответствия типов литералов и их значений
     */

    ASSERT_TRUE(Parse("0;"));
    ASSERT_STREQ("0", ast->toString().c_str());
    ASSERT_STREQ(":Bool", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("1;"));
    ASSERT_STREQ("1", ast->toString().c_str());
    ASSERT_STREQ(":Bool", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("2;"));
    ASSERT_STREQ("2", ast->toString().c_str());
    ASSERT_STREQ(":Int8", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("2_2;"));
    ASSERT_STREQ("2_2", ast->toString().c_str());
    ASSERT_STREQ(":Int8", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("-1;"));
    ASSERT_STREQ("-1", ast->toString().c_str());
    ASSERT_STREQ(":Int8", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("256;"));
    ASSERT_STREQ("256", ast->toString().c_str());
    ASSERT_STREQ(":Int16", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("10_000;"));
    ASSERT_STREQ("10_000", ast->toString().c_str());
    ASSERT_STREQ(":Int16", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("100_000;"));
    ASSERT_STREQ("100_000", ast->toString().c_str());
    ASSERT_STREQ(":Int32", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("0.0;"));
    ASSERT_STREQ("0.0", ast->toString().c_str());
    ASSERT_STREQ(":Float64", ast->m_type_name.c_str());



    ASSERT_TRUE(Parse("0:Bool;"));
    ASSERT_STREQ("0:Bool", ast->toString().c_str());
    ASSERT_STREQ(":Bool", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("0:Int32;"));
    ASSERT_STREQ("0:Int32", ast->toString().c_str());
    ASSERT_STREQ(":Int32", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("0:Int64;"));
    ASSERT_STREQ("0:Int64", ast->toString().c_str());
    ASSERT_STREQ(":Int64", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("0:Float32;"));
    ASSERT_STREQ("0:Float32", ast->toString().c_str());
    ASSERT_STREQ(":Float32", ast->m_type_name.c_str());

    //    ASSERT_TRUE(Parse("0  :  Half"));
    //    ASSERT_STREQ("0:Half", ast->toString().c_str());
    //    ASSERT_STREQ(":Half", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("0:Float64;"));
    ASSERT_STREQ("0:Float64", ast->toString().c_str());
    ASSERT_STREQ(":Float64", ast->m_type_name.c_str());


    ASSERT_TRUE(Parse("1:Bool;"));
    ASSERT_STREQ("1:Bool", ast->toString().c_str());
    ASSERT_STREQ(":Bool", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("1:Int8;"));
    ASSERT_STREQ("1:Int8", ast->toString().c_str());
    ASSERT_STREQ(":Int8", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("1:Int8;"));
    ASSERT_STREQ("1:Int8", ast->toString().c_str());
    ASSERT_STREQ(":Int8", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("1:Float64;"));
    ASSERT_STREQ("1:Float64", ast->toString().c_str());
    ASSERT_STREQ(":Float64", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("2:Int16;"));
    ASSERT_STREQ("2:Int16", ast->toString().c_str());
    ASSERT_STREQ(":Int16", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("2_2:Int32;"));
    ASSERT_STREQ("2_2:Int32", ast->toString().c_str());
    ASSERT_STREQ(":Int32", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("-1:Int8;"));
    ASSERT_STREQ("-1:Int8", ast->toString().c_str());
    ASSERT_STREQ(":Int8", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("-1 :Int64;"));
    ASSERT_STREQ("-1:Int64", ast->toString().c_str());
    ASSERT_STREQ(":Int64", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("256 :Int16;"));
    ASSERT_STREQ("256:Int16", ast->toString().c_str());
    ASSERT_STREQ(":Int16", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("10_000    :Int64;"));
    ASSERT_STREQ("10_000:Int64", ast->toString().c_str());
    ASSERT_STREQ(":Int64", ast->m_type_name.c_str());

    //    ASSERT_THROW(Parse("10__000"), parser_exception);

    ASSERT_TRUE(Parse("100_000:  Int32;"));
    ASSERT_STREQ("100_000:Int32", ast->toString().c_str());
    ASSERT_STREQ(":Int32", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("1.0  :  Float32;"));
    ASSERT_STREQ("1.0:Float32", ast->toString().c_str());
    ASSERT_STREQ(":Float32", ast->m_type_name.c_str());


    ASSERT_TRUE(Parse("-0.0   :Float64;"));
    ASSERT_STREQ("-0.0:Float64", ast->toString().c_str());
    ASSERT_STREQ(":Float64", ast->m_type_name.c_str());

    ASSERT_THROW(Parse("2:Bool;"), Return);
    ASSERT_THROW(Parse("-1:Bool;"), Return);
    //        ASSERT_THROW(Parse("-1:Int8"), parser_exception);
    ASSERT_THROW(Parse("300:Int8;"), Return);
    ASSERT_THROW(Parse("100000:Int16;"), Return);
    ASSERT_THROW(Parse("0.0:Bool;"), Return);
    ASSERT_THROW(Parse("0.0:Int8;"), Return);
    ASSERT_THROW(Parse("0.0:Int32;"), Return);
    ASSERT_THROW(Parse("0.0:Int64;"), Return);
}

TEST_F(ParserTest, TensorType) {
    ASSERT_TRUE(Parse("term:Int8[1,2] := [ [1,2,],[3,4,],];"));
    ASSERT_STREQ("term:Int8[1,2] := [[1, 2,], [3, 4,],];", ast->toString().c_str());

    ASSERT_TRUE(Parse("term[..., 3] := 0;"));
    ASSERT_STREQ("term[..., 3] := 0;", ast->toString().c_str());

    ASSERT_TRUE(Parse("term []= [2, 3,]:Int32;"));
    ASSERT_STREQ("term []= [2, 3,]:Int32;", ast->toString().c_str());

    //    ASSERT_TRUE(Parse("term[1, 3] :$type []= [[1,2,3,],];"));
    //    ASSERT_STREQ("term[1, 3]:$type []= [[1, 2, 3,],];", ast->toString().c_str());

    //    ASSERT_EQ(TermID::TERM, ast->getTermID()) << EnumStr(ast->getTermID());
    //    ASSERT_STREQ("term", ast->m_text.c_str());
    //    ASSERT_TRUE(ast->Right());
    //
    //    ASSERT_EQ(TermID::INDEX, ast->Right()->getTermID()) << EnumStr(ast->getTermID());
    //    ASSERT_EQ(2, ast->Right()->size());
    //    ASSERT_STREQ("1", (*ast->Right())[0]->getText().c_str());
    //    ASSERT_STREQ("2", (*ast->Right())[1]->getText().c_str());
    //
    //    ASSERT_TRUE(Parse("term[1, 1..2, 3];"));
    //
    //    ASSERT_EQ(TermID::INDEX, ast->Right()->getTermID()) << EnumStr(ast->getTermID());
    //    ASSERT_EQ(3, ast->Right()->size());
    //    ASSERT_STREQ("1", (*ast->Right())[0]->getText().c_str());
    //    ASSERT_STREQ("1..2", (*ast->Right())[1]->toString().c_str());
    //    ASSERT_STREQ("3", (*ast->Right())[2]->getText().c_str());
}

TEST_F(ParserTest, DictType) {
    ASSERT_TRUE(Parse("(1,2,)"));
    ASSERT_STREQ("(1, 2,)", ast->toString().c_str());

    ASSERT_TRUE(Parse("(1, arg=2,)"));
    ASSERT_STREQ("(1, arg=2,)", ast->toString().c_str());

    ASSERT_TRUE(Parse("(1, arg=2, '',)"));
    ASSERT_STREQ("(1, arg=2, '',)", ast->toString().c_str());

    ASSERT_TRUE(Parse("(arg1=22, arg2=, arg3=,):Enum"));
    ASSERT_STREQ("(arg1=22, arg2=, arg3=,):Enum", ast->toString().c_str());

    ASSERT_TRUE(Parse("(1, arg=2, '',):Class"));
    ASSERT_STREQ("(1, arg=2, '',):Class", ast->toString().c_str());
}

TEST_F(ParserTest, TermNoArg) {
    ASSERT_TRUE(Parse("term();"));
    ASSERT_TRUE(ast->isCall());
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());

    ASSERT_TRUE(Parse("term();"));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
}

TEST_F(ParserTest, TermFullName2) {
    ASSERT_TRUE(Parse("term::name::name2;"));
    ASSERT_STREQ("term::name::name2", ast->toString().c_str());
}

TEST_F(ParserTest, TermFullName3) {
    ASSERT_TRUE(Parse("term::name::name2();"));
    ASSERT_STREQ("term::name::name2()", ast->toString().c_str());
}

TEST_F(ParserTest, TermFullName4) {
    ASSERT_TRUE(Parse("term::name::name2();"));
    ASSERT_STREQ("term::name::name2()", ast->toString().c_str());
}

TEST_F(ParserTest, TermFullName5) {
    ASSERT_TRUE(Parse("::term::name::name2;"));
    ASSERT_STREQ("::term::name::name2", ast->toString().c_str());
}

TEST_F(ParserTest, TermFullName6) {
    ASSERT_TRUE(Parse("::term::name::name2();"));
    ASSERT_STREQ("::term::name::name2()", ast->toString().c_str());
}

TEST_F(ParserTest, TermFullName7) {
    ASSERT_TRUE(Parse("::term::name::name2();"));
    ASSERT_STREQ("::term::name::name2()", ast->toString().c_str());
}

TEST_F(ParserTest, TermFullName8) {
    ASSERT_TRUE(Parse("::name2(arg);"));
    ASSERT_STREQ("::name2(arg)", ast->toString().c_str());
}

TEST_F(ParserTest, TermFullName9) {
    ASSERT_TRUE(Parse("name::name2(arg);"));
    ASSERT_STREQ("name::name2(arg)", ast->toString().c_str());
}

TEST_F(ParserTest, FuncNoArg) {
    ASSERT_TRUE(Parse("@term();"));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("@term", ast->m_text.c_str());
}

TEST_F(ParserTest, TermNoArgSpace) {
    ASSERT_TRUE(Parse("term(  );"));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_EQ(0, ast->size());
}

TEST_F(ParserTest, TermArgTerm) {
    ASSERT_TRUE(Parse("term(arg);"));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_EQ(1, ast->size());
    ASSERT_STREQ("arg", (*ast)[0].second->m_text.c_str());
    ASSERT_FALSE((*ast)[0].second->m_ref);


    ASSERT_TRUE(Parse("term(name=value);"));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());

    ASSERT_EQ(1, ast->size());
    ASSERT_STREQ("name", (*ast)[0].second->getName().c_str());
    ASSERT_STREQ("value", (*ast)[0].second->getText().c_str());
    ASSERT_FALSE((*ast)[0].second->m_ref);
}

TEST_F(ParserTest, TermArgTermRef) {
    ASSERT_TRUE(Parse("term(&arg);"));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_EQ(1, ast->size());
    ASSERT_STREQ("arg", (*ast)[0].second->m_text.c_str());
    ASSERT_TRUE((*ast)[0].second->m_ref);


    ASSERT_TRUE(Parse("term(name=&value);"));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());

    ASSERT_EQ(1, ast->size());
    ASSERT_STREQ("name", (*ast)[0].second->getName().c_str());
    ASSERT_STREQ("value", (*ast)[0].second->getText().c_str());
    ASSERT_TRUE((*ast)[0].second->m_ref);
}

TEST_F(ParserTest, TermArgTermSpace) {
    ASSERT_TRUE(Parse("   \n  \t term(  \n  arg  \n  )    ;   \n  "));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_EQ(1, ast->size());
    ASSERT_STREQ("arg", (*ast)[0].second->m_text.c_str());
}

TEST_F(ParserTest, TermArgs1) {
    ASSERT_TRUE(Parse("term(arg1);"));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_EQ(1, ast->size());
    ASSERT_STREQ("arg1", (*ast)[0].second->m_text.c_str());
}

TEST_F(ParserTest, TermArgs2) {
    ASSERT_TRUE(Parse("term(arg1,arg2);"));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_EQ(2, ast->size());
    ASSERT_STREQ("arg1", (*ast)[0].second->getText().c_str());
    ASSERT_STREQ("arg2", (*ast)[1].second->getText().c_str());
    ASSERT_FALSE((*ast)[0].second->m_ref);
    ASSERT_FALSE((*ast)[1].second->m_ref);
}

TEST_F(ParserTest, TermArgsRef) {
    ASSERT_TRUE(Parse("term(&arg1,&arg2);"));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_EQ(2, ast->size());
    ASSERT_STREQ("arg1", (*ast)[0].second->getText().c_str());
    ASSERT_STREQ("arg2", (*ast)[1].second->getText().c_str());
    ASSERT_TRUE((*ast)[0].second->m_ref);
    ASSERT_TRUE((*ast)[1].second->m_ref);
}

TEST_F(ParserTest, TermArgMixed) {
    ASSERT_TRUE(Parse("term(\narg1,\n arg2\n = \narg3 \n);"));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());

    ASSERT_EQ(2, ast->size());
    ASSERT_STREQ("arg1", (*ast)[0].second->getText().c_str());
    ASSERT_STREQ("arg2", (*ast)[1].second->getName().c_str());
    ASSERT_STREQ("arg3", (*ast)[1].second->getText().c_str());
}

TEST_F(ParserTest, ArgsType) {
    ASSERT_TRUE(Parse("term(bool:Bool=term(100), int:Int32=100, long:Int64=@term()):Float64:={long;};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term(bool:Bool=term(100), int:Int32=100, long:Int64=@term()):Float64 := {long;};", ast->toString().c_str());
}

TEST_F(ParserTest, TermCall) {
    ASSERT_TRUE(Parse("var2 := min(200, var, 400);"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ(":=", ast->m_text.c_str());


    TermPtr right = ast->Right();
    ASSERT_TRUE(right);
    ASSERT_EQ(3, right->size());
    ASSERT_STREQ("200", (*right)[0].second->getText().c_str());
    ASSERT_FALSE((*right)[0].second->m_left);
    ASSERT_FALSE((*right)[0].second->m_right);
    ASSERT_STREQ("var", (*right)[1].second->getText().c_str());
    ASSERT_FALSE((*right)[1].second->m_left);
    ASSERT_FALSE((*right)[1].second->m_right);
    ASSERT_STREQ("400", (*right)[2].second->getText().c_str());
    ASSERT_FALSE((*right)[2].second->m_left);
    ASSERT_FALSE((*right)[2].second->m_right);
}

TEST_F(ParserTest, TermCollection) {
    ASSERT_TRUE(Parse("term([2,], [arg1,arg2,]);"));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_EQ(2, ast->size());
    ASSERT_EQ(TermID::TENSOR, (*ast)[0].second->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_EQ(TermID::TENSOR, (*ast)[1].second->getTermID()) << newlang::toString(ast->getTermID());
}

TEST_F(ParserTest, TermCollection2) {
    ASSERT_TRUE(Parse("term((,), name=[arg1,arg2,]);"));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_EQ(2, ast->size());
    ASSERT_EQ(TermID::DICT, (*ast)[0].second->getTermID()) << newlang::toString((*ast)[0].second->getTermID());
    ASSERT_EQ(TermID::TENSOR, (*ast)[1].second->getTermID()) << newlang::toString((*ast)[1].second->getTermID());
}

TEST_F(ParserTest, OpsCall) {
    ASSERT_TRUE(Parse("call(1+1)"));
    ASSERT_TRUE(Parse("call(1-1)"));
    ASSERT_TRUE(Parse("call(1\\1-1)"));
    ASSERT_TRUE(Parse("call(term+1)"));
    ASSERT_TRUE(Parse("call(term-1)"));
    ASSERT_TRUE(Parse("call(term+term)"));
    ASSERT_TRUE(Parse("call(term-term)"));
    ASSERT_TRUE(Parse("call(term-term,term*2)"));
}

TEST_F(ParserTest, ArgMixedFail) {
    //    EXPECT_THROW(
    //            Parse("term(arg2=arg3, arg1);"), std::runtime_error
    //            );
    EXPECT_THROW(
            Parse("term(arg1,arg2=arg3,,);"), Return
            );
    EXPECT_THROW(
            Parse("term(,);"), Return
            );

}

TEST_F(ParserTest, Iterator) {
    ASSERT_TRUE(Parse("term!!;"));
    ASSERT_STREQ("!!", ast->m_text.c_str());

    TermPtr arg = ast->Left();
    ASSERT_TRUE(arg);
    ASSERT_STREQ("term", arg->getText().c_str());

    ASSERT_TRUE(Parse("term(arg)!!;"));
    ASSERT_STREQ("!!", ast->getText().c_str());

    arg = ast->Left();
    ASSERT_TRUE(arg);
    ASSERT_STREQ("term", arg->getText().c_str());
    ASSERT_EQ(1, arg->size());
    ASSERT_STREQ("arg", (*arg)[0].second->getText().c_str());

    // #pragma GCC warning "ITERATOR"
    //    ASSERT_TRUE(Parse("term(arg=value)??(100)"));
    //    ASSERT_STREQ("??", ast->getText().c_str());
    //    ASSERT_EQ(1, ast->getItemCount());
    //    ASSERT_STREQ("100", (*ast)[0]->getText().c_str());
    //
    //    arg = ast->Left();
    //    ASSERT_TRUE(arg);
    //    ASSERT_STREQ("term", arg->getText().c_str());
    //    ASSERT_EQ(1, arg->getItemCount());
    //    ASSERT_STREQ("arg", (*arg)[0]->getName().c_str());
    //    ASSERT_STREQ("value", (*arg)[0]->getText().c_str());
    //
    //
    //    ASSERT_TRUE(Parse("term2(arg=value)  ?? (iter_arg=100)"));
    //    ASSERT_EQ(1, ast->getItemCount());
    //    ASSERT_STREQ("??", ast->getText().c_str());
    //    ASSERT_EQ(1, ast->getItemCount());
    //    ASSERT_STREQ("100", (*ast)[0]->getText().c_str());
    //    ASSERT_STREQ("iter_arg", (*ast)[0]->getName().c_str());
    //
    //    arg = ast->Left();
    //    ASSERT_TRUE(arg);
    //    ASSERT_STREQ("term2", arg->getText().c_str());
    //    ASSERT_EQ(1, arg->getItemCount());
    //    ASSERT_STREQ("arg", (*arg)[0]->getName().c_str());
    //    ASSERT_STREQ("value", (*arg)[0]->getText().c_str());



    //    ASSERT_FALSE(Parse("term2(arg=value)?(iter_arg=100)?"));
    //    ASSERT_FALSE(Parse("term2(arg=value)??"));
    EXPECT_THROW(
            Parse("term2(arg=value)!;?"), Return
            );


    ASSERT_TRUE(Parse("term2(arg=value?);"));
    ASSERT_EQ(1, ast->size());
    ASSERT_STREQ("term2", ast->getText().c_str());
    ASSERT_EQ(1, ast->size());
    ASSERT_STREQ("arg", ast->name(0).c_str());
    ASSERT_STREQ("?", (*ast)[0].second->getText().c_str());
    ASSERT_TRUE((*ast)[0].second->Left());
    ASSERT_STREQ("value", (*ast)[0].second->Left()->getText().c_str());


    // @todo Проблемы с итератором у именованного аргумента  !!!!!!!!!!!!!!!!!!!!

    //    ASSERT_TRUE(Parse("term2(arg=value(10)?(iter_val=555))"));
    //    ASSERT_EQ(1, ast->getItemCount());
    //    ASSERT_STREQ("term2", (*ast)[0]->getText().c_str());
    //    ASSERT_EQ(1, (*ast)[0]->getItemCount());
    //    ASSERT_STREQ("arg", (*ast)[0]->name(0).c_str());
    //    ASSERT_STREQ("?", (*(*ast)[0])[0]->getText().c_str());
    //    ASSERT_EQ(1, (*(*ast)[0])[0]->getItemCount());
    //
    //    ASSERT_TRUE((*(*ast)[0])[0]->Left());
    //    ASSERT_STREQ("value", (*(*ast)[0])[0]->Left()->getText().c_str());
    //    ASSERT_EQ(1, (*(*ast)[0])[0]->Left()->getItemCount());

}

TEST_F(ParserTest, MathPlus) {
    ASSERT_TRUE(Parse("test         :=       123+456;"));
    ASSERT_STREQ(":=", ast->m_text.c_str());
    ASSERT_TRUE(ast->Left());
    ASSERT_TRUE(ast->Right());
    ASSERT_STREQ("test", ast->Left()->m_text.c_str());
    TermPtr op = ast->Right();
    ASSERT_TRUE(op);
    ASSERT_STREQ("+", op->m_text.c_str());
    ASSERT_TRUE(op->Right());
    ASSERT_TRUE(op->Left());
    ASSERT_STREQ("123", op->Left()->m_text.c_str());
    ASSERT_STREQ("456", op->Right()->m_text.c_str());
}

TEST_F(ParserTest, MathMinus) {
    ASSERT_TRUE(Parse("test := 123-456;"));
    ASSERT_STREQ(":=", ast->m_text.c_str());
    ASSERT_TRUE(ast->Left());
    ASSERT_TRUE(ast->Right());
    ASSERT_STREQ("test", ast->Left()->m_text.c_str());
    TermPtr op = ast->Right();
    ASSERT_TRUE(op);
    ASSERT_STREQ("-", op->m_text.c_str());
    ASSERT_TRUE(op->Right());
    ASSERT_TRUE(op->Left());
    ASSERT_STREQ("123", op->Left()->m_text.c_str());
    ASSERT_STREQ("456", op->Right()->m_text.c_str());
}

TEST_F(ParserTest, MathMul) {
    ASSERT_TRUE(Parse("test := 123*456;"));
    ASSERT_STREQ(":=", ast->m_text.c_str());
    ASSERT_TRUE(ast->Left());
    ASSERT_TRUE(ast->Right());
    ASSERT_STREQ("test", ast->Left()->m_text.c_str());
    TermPtr op = ast->Right();
    ASSERT_TRUE(op);
    ASSERT_STREQ("*", op->m_text.c_str());
    ASSERT_TRUE(op->Right());
    ASSERT_TRUE(op->Left());
    ASSERT_STREQ("123", op->Left()->m_text.c_str());
    ASSERT_STREQ("456", op->Right()->m_text.c_str());
}

TEST_F(ParserTest, MathDiv) {
    ASSERT_TRUE(Parse("test := 123/456;"));
    ASSERT_STREQ(":=", ast->m_text.c_str());
    ASSERT_TRUE(ast->Left());
    ASSERT_TRUE(ast->Right());
    ASSERT_STREQ("test", ast->Left()->m_text.c_str());
    TermPtr op = ast->Right();
    ASSERT_TRUE(op);
    ASSERT_STREQ("/", op->m_text.c_str());
    ASSERT_TRUE(op->Right());
    ASSERT_TRUE(op->Left());
    ASSERT_STREQ("123", op->Left()->m_text.c_str());
    ASSERT_STREQ("456", op->Right()->m_text.c_str());
}

TEST_F(ParserTest, MathNeg) {
    ASSERT_TRUE(Parse("test := -456;"));
    ASSERT_STREQ(":=", ast->m_text.c_str());
    ASSERT_TRUE(ast->Left());
    ASSERT_TRUE(ast->Right());
    ASSERT_STREQ("test", ast->Left()->m_text.c_str());
    TermPtr op = ast->Right();
    ASSERT_TRUE(op);
    ASSERT_STREQ("-456", op->m_text.c_str());


    ASSERT_TRUE(Parse("1000-456"));
    ASSERT_STREQ("-", ast->m_text.c_str());
    ASSERT_TRUE(ast->Left());
    ASSERT_TRUE(ast->Right());
    ASSERT_STREQ("1000", ast->Left()->m_text.c_str());
    op = ast->Right();
    ASSERT_TRUE(op);
    ASSERT_STREQ("456", op->m_text.c_str());

    ASSERT_TRUE(Parse("-(456)"));
    ASSERT_STREQ("-", ast->m_text.c_str());
    ASSERT_FALSE(ast->Left());
    ASSERT_TRUE(ast->Right());
    op = ast->Right();
    ASSERT_TRUE(op);
    ASSERT_STREQ("456", op->m_text.c_str());

    ASSERT_ANY_THROW(Parse("1000 456"));
}

TEST_F(ParserTest, MathPrioritet) {
    ASSERT_TRUE(Parse("1+2*3"));
    ASSERT_STREQ("+", ast->m_text.c_str());
    ASSERT_TRUE(ast->Left());
    ASSERT_TRUE(ast->Right());
    ASSERT_STREQ("1", ast->Left()->m_text.c_str());
    TermPtr op = ast->Right();
    ASSERT_TRUE(op);
    ASSERT_STREQ("*", op->m_text.c_str());
    ASSERT_TRUE(op->Right());
    ASSERT_TRUE(op->Left());
    ASSERT_STREQ("2", op->Left()->m_text.c_str());
    ASSERT_STREQ("3", op->Right()->m_text.c_str());


    ASSERT_TRUE(Parse("1*2+3"));
    ASSERT_STREQ("+", ast->m_text.c_str());
    ASSERT_TRUE(ast->Left());
    ASSERT_TRUE(ast->Right());
    ASSERT_STREQ("3", ast->Right()->m_text.c_str());
    op = ast->Left();
    ASSERT_TRUE(op);
    ASSERT_STREQ("*", op->m_text.c_str());
    ASSERT_TRUE(op->Right());
    ASSERT_TRUE(op->Left());
    ASSERT_STREQ("1", op->Left()->m_text.c_str());
    ASSERT_STREQ("2", op->Right()->m_text.c_str());


    ASSERT_TRUE(Parse("(1*2)+3"));
    ASSERT_STREQ("+", ast->m_text.c_str());
    ASSERT_TRUE(ast->Left());
    ASSERT_TRUE(ast->Right());
    ASSERT_STREQ("3", ast->Right()->m_text.c_str());
    op = ast->Left();
    ASSERT_TRUE(op);
    ASSERT_STREQ("*", op->m_text.c_str());
    ASSERT_TRUE(op->Right());
    ASSERT_TRUE(op->Left());
    ASSERT_STREQ("1", op->Left()->m_text.c_str());
    ASSERT_STREQ("2", op->Right()->m_text.c_str());


    ASSERT_TRUE(Parse("1*(2+3)"));
    ASSERT_STREQ("*", ast->m_text.c_str());
    ASSERT_TRUE(ast->Left());
    ASSERT_TRUE(ast->Right());
    ASSERT_STREQ("1", ast->Left()->m_text.c_str());
    op = ast->Right();
    ASSERT_TRUE(op);
    ASSERT_STREQ("+", op->m_text.c_str());
    ASSERT_TRUE(op->Right());
    ASSERT_TRUE(op->Left());
    ASSERT_STREQ("2", op->Left()->m_text.c_str());
    ASSERT_STREQ("3", op->Right()->m_text.c_str());
}

TEST_F(ParserTest, CodeSimple) {
    ASSERT_TRUE(Parse("{%code+code%};"));
    ASSERT_STREQ("code+code", ast->m_text.c_str());
}

TEST_F(ParserTest, CodeSimple2) {
    ASSERT_TRUE(Parse("{% code+code %};"));
    ASSERT_STREQ(" code+code ", ast->m_text.c_str());
}

TEST_F(ParserTest, AssignSimple) {
    ASSERT_TRUE(Parse("term := term2;"));
    ASSERT_STREQ("term := term2;", ast->toString().c_str());
}

TEST_F(ParserTest, AssignSimple2) {
    ASSERT_TRUE(Parse("\t term   :=   term2()  ;  \n"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_TRUE(ast->Left());
    ASSERT_TRUE(ast->Right());
    ASSERT_STREQ("term", ast->Left()->m_text.c_str());
    ASSERT_EQ(0, ast->Left()->size());
    ASSERT_STREQ("term2", ast->Right()->m_text.c_str());
    ASSERT_EQ(0, ast->Right()->size());
    ASSERT_STREQ("term := term2();", ast->toString().c_str());
}

TEST_F(ParserTest, AssignFullName) {
    ASSERT_TRUE(Parse("term::name() := {term2;};"));
    ASSERT_STREQ("term::name() := {term2;};", ast->toString().c_str());
}

TEST_F(ParserTest, AssignClass0) {
    ASSERT_TRUE(Parse("term := :Class();"));
    ASSERT_STREQ("term := :Class();", ast->toString().c_str());
}

TEST_F(ParserTest, AssignClass1) {
    ASSERT_TRUE(Parse(":class  :=    :Class() {}  ;"));
    ASSERT_STREQ(":class := :Class(){};", ast->toString().c_str());
}

TEST_F(ParserTest, AssignClass2) {
    ASSERT_TRUE(Parse(":class  :=  ::ns::func(arg1, arg2=\"\") {};"));
    ASSERT_STREQ(":class := ::ns::func(arg1, arg2=\"\"){};", ast->toString().c_str());
}

TEST_F(ParserTest, Namespace) {
    ASSERT_TRUE(Parse("name{ func() := {}  };"));
    ASSERT_TRUE(Parse("name::space{ func() := {}  };"));
    ASSERT_TRUE(Parse("::name::space{ func() := {}  };"));
    ASSERT_TRUE(Parse("::{ func() := {}  };"));
}

TEST_F(ParserTest, DISABLED_Namespace2) {
    ASSERT_TRUE(Parse("name, name2 { func() := {}  };"));
    ASSERT_TRUE(Parse("name::space, ns::name2{ func() := {}  };"));
    ASSERT_TRUE(Parse("::name::space, ::name2 { func() := {}  };"));
    ASSERT_TRUE(Parse("::, ::name2  { func() := {}  };"));
}

TEST_F(ParserTest, AssignFullName2) {
    ASSERT_TRUE(Parse("term::name::name2() := term2;"));
    ASSERT_STREQ("term::name::name2() := term2;", ast->toString().c_str());
}

TEST_F(ParserTest, AssignFullName3) {
    ASSERT_TRUE(Parse("::term::name::name3() := term2;"));
    ASSERT_STREQ("::term::name::name3() := term2;", ast->toString().c_str());
}

TEST_F(ParserTest, FiledAssign) {
    ASSERT_TRUE(Parse("$1.val :=  123;"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_TRUE(ast->Left());
    ASSERT_TRUE(ast->Right());

    ASSERT_EQ(TermID::ARGUMENT, ast->Left()->getTermID());
    ASSERT_STREQ("$1", ast->Left()->m_text.c_str());
    ASSERT_TRUE(ast->Left()->m_name.empty());

    ASSERT_TRUE(ast->Left()->Right());
    ASSERT_STREQ("val", ast->Left()->Right()->m_text.c_str());

    ASSERT_EQ(TermID::INTEGER, ast->Right()->getTermID());
    ASSERT_STREQ("123", ast->Right()->m_text.c_str());
}

TEST_F(ParserTest, FiledAssign2) {
    ASSERT_TRUE(Parse("term.field1.field2 :=  123;"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_TRUE(ast->Left());
    ASSERT_TRUE(ast->Right());

    ASSERT_EQ(TermID::NAME, ast->Left()->getTermID());
    ASSERT_STREQ("term", ast->Left()->m_text.c_str());
    ASSERT_TRUE(ast->Left()->m_name.empty());

    ASSERT_TRUE(ast->Left()->Right());
    ASSERT_STREQ("field1", ast->Left()->Right()->m_text.c_str());
    ASSERT_TRUE(ast->Left()->Right()->Right());
    ASSERT_STREQ("field2", ast->Left()->Right()->Right()->m_text.c_str());
    ASSERT_FALSE(ast->Left()->Right()->Right()->Right());

    ASSERT_EQ(TermID::INTEGER, ast->Right()->getTermID());
    ASSERT_STREQ("123", ast->Right()->m_text.c_str());

    ASSERT_STREQ("term.field1.field2 := 123;", ast->toString().c_str());
}

TEST_F(ParserTest, ArrayAssign) {
    ASSERT_TRUE(Parse("$0[0] =  123;"));
    ASSERT_EQ(TermID::ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_TRUE(ast->Left());
    ASSERT_TRUE(ast->Right());

    ASSERT_EQ(TermID::ARGUMENT, ast->Left()->getTermID());
    ASSERT_STREQ("$0", ast->Left()->m_text.c_str());
    ASSERT_TRUE(ast->Left()->m_name.empty());

    ASSERT_TRUE(ast->Left()->Right());
    ASSERT_EQ(TermID::INDEX, ast->Left()->Right()->getTermID());
    ASSERT_STREQ("[", ast->Left()->Right()->m_text.c_str());
    ASSERT_EQ(1, ast->Left()->Right()->size());
    ASSERT_STREQ("0", (*ast->Left()->Right())[0].second->m_text.c_str());


}

TEST_F(ParserTest, DISABLED_ArrayAssign2) {
    ASSERT_TRUE(Parse("term[1][1..3] =  123;"));
    ASSERT_EQ(TermID::ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_TRUE(ast->Left());
    ASSERT_TRUE(ast->Right());

    ASSERT_EQ(TermID::NAME, ast->Left()->getTermID());
    ASSERT_STREQ("term", ast->Left()->m_text.c_str());
    ASSERT_TRUE(ast->Left()->m_name.empty());

    ASSERT_TRUE(ast->Left()->Right());
    ASSERT_STREQ("[", ast->Left()->Right()->m_text.c_str());
    ASSERT_TRUE(ast->Left()->Right()->Right());
    ASSERT_STREQ("[", ast->Left()->Right()->Right()->m_text.c_str());
    ASSERT_FALSE(ast->Left()->Right()->Right()->Right());

    ASSERT_EQ(TermID::INTEGER, ast->Right()->getTermID());
    ASSERT_STREQ("123", ast->Right()->m_text.c_str());

    ASSERT_STREQ("term[1][1, 2, 3]=123;", ast->toString().c_str());
}

TEST_F(ParserTest, DISABLED_FieldArray) {
    ASSERT_TRUE(Parse("term.val[1].field :=  value[-1..@count()..5].field;"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term.val[1].field := value[-1..@count()..5].field;", ast->toString().c_str());
}

TEST_F(ParserTest, AssignSimple3) {
    ASSERT_TRUE(Parse("\t term   :=   term2(   )  ;  \n"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_TRUE(ast->Left());
    ASSERT_TRUE(ast->Right());
    ASSERT_STREQ("term := term2();", ast->toString().c_str());
}

TEST_F(ParserTest, AssignSimpleArg) {
    ASSERT_TRUE(Parse("\t term    ::=    term2( arg  ) ;   \n"));
    ASSERT_EQ(TermID::CREATE, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_TRUE(ast->Left());
    ASSERT_TRUE(ast->Right());
    ASSERT_STREQ("term ::= term2(arg);", ast->toString().c_str());
}

TEST_F(ParserTest, AssignSimpleNamedArg) {
    ASSERT_TRUE(Parse("\t term  :=  $term2( arg = arg2  )\n;\n\n"));
    ASSERT_STREQ("term := $term2(arg=arg2);", ast->toString().c_str());
}

TEST_F(ParserTest, AssignNamedArgs) {
    ASSERT_TRUE(Parse("\t term   :=   @term2( arg, arg1 = arg2  )  ;  \n"));
    ASSERT_STREQ("term := @term2(arg, arg1=arg2);", ast->toString().c_str());
}

TEST_F(ParserTest, AssignNamedArgs2) {
    ASSERT_TRUE(Parse("\t term   :=   $term2( arg, arg1 = arg2(arg3))  ;  \n"));
    ASSERT_STREQ("term := $term2(arg, arg1=arg2(arg3));", ast->toString().c_str());
}

TEST_F(ParserTest, AssignNamedArgs3) {
    ASSERT_TRUE(Parse("\t $term   :=   term2( @arg, arg1 = 123  )  ;  \n"));
    ASSERT_STREQ("$term := term2(@arg, arg1=123);", ast->toString().c_str());
}

TEST_F(ParserTest, AssignNamedArgs4) {
    ASSERT_TRUE(Parse("\t %term   :=   term2( arg, arg1 = @arg2($arg3))  ;  \n"));
    ASSERT_STREQ("%term := term2(arg, arg1=@arg2($arg3));", ast->toString().c_str());
}

TEST_F(ParserTest, AssignString) {
    ASSERT_TRUE(Parse("term := \"строка\";"));
    ASSERT_STREQ("term := \"строка\";", ast->toString().c_str());
}

TEST_F(ParserTest, AssignString2) {
    ASSERT_TRUE(Parse("$2  :=  \"строка\" ; \n"));
    ASSERT_STREQ("$2 := \"строка\";", ast->toString().c_str());
}

TEST_F(ParserTest, AssignStringControlChar) {
    ASSERT_TRUE(Parse("$2 :=  \"стр\\\"\t\r\xffока\\s\" ; \n"));
    ASSERT_STREQ("$2 := \"стр\"\t\r\xffока\x20\";", ast->toString().c_str());
}

TEST_F(ParserTest, AssignStringMultiline) {
    ASSERT_TRUE(Parse("term  :=  'стр\\\n\t  ока\\\n   \\s' ; \n"));
    ASSERT_STREQ("term := 'стр\n\t  ока\n   \x20';", ast->toString().c_str());
}

TEST_F(ParserTest, AssignDictEmpty) {
    ASSERT_TRUE(Parse("term := (   ,    );"));
    ASSERT_STREQ("term := (,);", ast->toString().c_str());
}

TEST_F(ParserTest, AssignDict) {
    ASSERT_TRUE(Parse("term := (name,)"));
    ASSERT_STREQ("term := (name,);", ast->toString().c_str());

    ASSERT_TRUE(Parse("term := (  123 , )"));
    ASSERT_STREQ("term := (123,);", ast->toString().c_str());

    ASSERT_TRUE(Parse("term := (  name  = 123 ,  )"));
    ASSERT_STREQ("term := (name=123,);", ast->toString().c_str());
}

TEST_F(ParserTest, AssignArray) {
    ASSERT_TRUE(Parse("term := [  123  , ]"));
    ASSERT_STREQ("term := [123,];", ast->toString().c_str());
}

TEST_F(ParserTest, ArgsArray1) {
    ASSERT_TRUE(Parse("term([1,]);"));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_EQ(1, ast->size());
    ASSERT_STREQ("[1,]", (*ast)[0].second->toString().c_str());
}

TEST_F(ParserTest, LogicEq) {
    ASSERT_TRUE(Parse("var := 1==2;"));
    ASSERT_STREQ("var := 1 == 2;", ast->toString().c_str());
}

TEST_F(ParserTest, LogicNe) {
    ASSERT_TRUE(Parse("var := 1!=2;"));
    ASSERT_STREQ("var := 1 != 2;", ast->toString().c_str());
}

TEST_F(ParserTest, InstanceName) {
    ASSERT_TRUE(Parse("var ~ Class"));
    ASSERT_TRUE(Parse("var ~ :Class"));
    ASSERT_TRUE(Parse("var ~ 'name'"));
    ASSERT_TRUE(Parse("var ~ $var"));
    ASSERT_TRUE(Parse("1  ~  $var"));
    ASSERT_TRUE(Parse("'строка'  ~  'тип'"));
    ASSERT_TRUE(Parse("1..20 ~ var_name"));

    ASSERT_TRUE(Parse("var ~~ Class"));
    ASSERT_TRUE(Parse("var ~~ :Class"));
    ASSERT_TRUE(Parse("var ~~ 'name'"));
    ASSERT_TRUE(Parse("var ~~ $var"));
    ASSERT_TRUE(Parse("1  ~~  $var"));
    ASSERT_TRUE(Parse("'строка'  ~~  'тип'"));
    ASSERT_TRUE(Parse("1..20 ~~ var_name"));
}

TEST_F(ParserTest, FunctionSimple) {
    ASSERT_TRUE(Parse("func() := {{%%}};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("func() := {{%%};};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionSimpleTwo) {
    ASSERT_TRUE(Parse("func() := {{% %};{% %}};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("func() := {{% %}; {% %};};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionSimple2) {
    ASSERT_TRUE(Parse("func(arg)  :=  {$0:=0;};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("func(arg) := {$0 := 0;};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionSimple3) {
    ASSERT_TRUE(Parse("func(arg)  :=  {{%  %};{% %};{%  %}; $99:=0;};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("func(arg) := {{%  %}; {% %}; {%  %}; $99 := 0;};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionSimple4) {
    ASSERT_TRUE(Parse("func(arg) := {$33:=0;};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("func(arg) := {$33 := 0;};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionSimple5) {
    ASSERT_TRUE(Parse("print(str=\"\") :={% printf(\"%s\", static_cast<char *>($str)); %};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("print(str=\"\") := {% printf(\"%s\", static_cast<char *>($str)); %};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionTrans2) {
    ASSERT_TRUE(Parse("func(arg1, arg2 = 5) :- {% return $arg1; %};"));
    ASSERT_STREQ("func(arg1, arg2=5) :- {% return $arg1; %};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionTrans3) {
    ASSERT_TRUE(Parse("func(arg1, arg2 = 5) ::- {% return $arg1; %};"));
    ASSERT_STREQ("func(arg1, arg2=5) ::- {% return $arg1; %};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionTrans4) {
    ASSERT_TRUE(Parse("func(arg1, arg2 = 5) :- { [$arg1 < $arg2] --> {% return $arg1; %}; };"));
    ASSERT_STREQ("func(arg1, arg2=5) :- {[$arg1 < $arg2]-->{% return $arg1; %};};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionTrans5) {
    ASSERT_TRUE(Parse("func(arg1, arg2 = 5) :- { [$arg1<$arg2] --> {% return $arg1; %}, [_] --> {% return $arg2; %}; };"));
    ASSERT_STREQ("func(arg1, arg2=5) :- {[$arg1 < $arg2]-->{% return $arg1; %},\n [_]-->{% return $arg2; %};;};", ast->toString().c_str());
    //                  func(arg1, arg2=5) :- {[$arg1 < $arg2]-->{% return $arg1; %},\n [_]-->{% return $arg2; %};;};
}

TEST_F(ParserTest, FunctionRussian1) {
    ASSERT_TRUE(Parse("мин(arg) := {$00:=0;};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("мин(arg) := {$00 := 0;};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionRussian2) {
    ASSERT_TRUE(Parse("мин(арг) := {$1:=0;};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("мин(арг) := {$1 := 0;};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionRussian3) {
    ASSERT_TRUE(Parse("русс(10,20);"));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("русс(10, 20)", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionRussian4) {
    ASSERT_TRUE(Parse("мин(10,20);"));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("мин(10, 20)", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionArgs) {
    ASSERT_TRUE(Parse("мин(...) := {$1:=0;};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("мин(...) := {$1 := 0;};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionArgs2) {
    ASSERT_TRUE(Parse("мин(arg, ...) := {$1:=0;};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("мин(arg, ...) := {$1 := 0;};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionArgs3) {
    ASSERT_TRUE(Parse("мин(arg1, arg2, ...) := {$0:=0;};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("мин(arg1, arg2, ...) := {$0 := 0;};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionKwArgs1) {
    ASSERT_TRUE(Parse("мин(...) := {$0:=0;func();var;};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("мин(...) := {$0 := 0; func(); var;};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionKwArgs2) {
    ASSERT_TRUE(Parse("мин(arg=123 ,  ...) := {$0:=0;};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("мин(arg=123, ...) := {$0 := 0;};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionKwArgs3) {
    ASSERT_TRUE(Parse("мин(arg1=1, arg2=2 ,...) := {$0:=0;};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("мин(arg1=1, arg2=2, ...) := {$0 := 0;};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionArgsAll) {
    ASSERT_TRUE(Parse("мин(arg1=1, arg2=2 , ...) := {$0:=0;};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("мин(arg1=1, arg2=2, ...) := {$0 := 0;};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionArgsAll2) {
    ASSERT_TRUE(Parse("мин(arg, arg1=1, arg2=2, ...) := {$0:=0;};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("мин(arg, arg1=1, arg2=2, ...) := {$0 := 0;};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionEmpty) {
    ASSERT_TRUE(Parse("мин(arg, arg1=1, arg2=2, ...) := {};"));
    ASSERT_STREQ("мин(arg, arg1=1, arg2=2, ...) := {};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionEmpty2) {
    ASSERT_TRUE(Parse("мин(...) :- {};"));
    ASSERT_STREQ("мин(...) :- {};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionArgsFail) {
    ASSERT_ANY_THROW(Parse("мин(... ...) := {$0:=0;};"));
    ASSERT_ANY_THROW(Parse("мин(arg ...) := {$0:=0;};"));
    ASSERT_ANY_THROW(Parse("мин(arg=1 ..., arg) := {$0:=0;};"));
    ASSERT_ANY_THROW(Parse("мин(arg=1, arg ...) := {$0:=0;};"));
    ASSERT_ANY_THROW(Parse("мин(arg=1 ...) := {$0:=0;};"));
}

TEST_F(ParserTest, ArrayAdd7) {
    ASSERT_TRUE(Parse("name()  :=  term2;")); // $[].name:=term2;
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("name() := term2;", ast->toString().c_str());
}

TEST_F(ParserTest, DISABLED_Ellipsis1) {
    ASSERT_TRUE(Parse("name  :=  term2(arg1  ,  ...    ...    dict);")); //
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("name := term2(arg1, ... ... dict);", ast->toString().c_str());
}

//TEST_F(ParserTest, Complex1) {
//    ASSERT_TRUE(Parse("10+20j"));
//    ASSERT_EQ(TermID::COMPLEX, ast->getTermID()) << newlang::toString(ast->getTermID());
//    ASSERT_STREQ("10+20j;", ast->toString().c_str());
//}
//
//TEST_F(ParserTest, Complex2) {
//    ASSERT_TRUE(Parse("0j"));
//    ASSERT_EQ(TermID::COMPLEX, ast->getTermID()) << newlang::toString(ast->getTermID());
//    ASSERT_STREQ("0j;", ast->toString().c_str());
//}
//
//TEST_F(ParserTest, Complex3) {
//    ASSERT_TRUE(Parse("0.1-0.20j"));
//    ASSERT_EQ(TermID::COMPLEX, ast->getTermID()) << newlang::toString(ast->getTermID());
//    ASSERT_STREQ("0.1-0.20j;", ast->toString().c_str());
//}
//
//TEST_F(ParserTest, Rational) {
//    ASSERT_TRUE(Parse("1\\1"));
//    ASSERT_EQ(TermID::RATIONAL, ast->getTermID()) << newlang::toString(ast->getTermID());
//    ASSERT_STREQ("1\\1;", ast->toString().c_str());
//}
//
//TEST_F(ParserTest, Rational2) {
//    ASSERT_TRUE(Parse("1\\-20"));
//    ASSERT_EQ(TermID::RATIONAL, ast->getTermID()) << newlang::toString(ast->getTermID());
//    ASSERT_STREQ("1\\-20;", ast->toString().c_str());
//}
//
//TEST_F(ParserTest, Rational3) {
//    ASSERT_TRUE(Parse("-3\\+11"));
//    ASSERT_EQ(TermID::RATIONAL, ast->getTermID()) << newlang::toString(ast->getTermID());
//    ASSERT_STREQ("-3\\+11;", ast->toString().c_str());
//}
//
//TEST_F(ParserTest, ArrayAdd9) {
//    ASSERT_TRUE(Parse("$name  :=  term2"));
//    ASSERT_EQ(TermID::CREATE, ast->getTermID()) << newlang::toString(ast->getTermID());
//    ASSERT_STREQ("$name := term2;", ast->toString().c_str());
//}

TEST_F(ParserTest, Ellipsis2) {
    ASSERT_TRUE(Parse("@name  :=  term2(   ...   arg);"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("@name := term2(...arg);", ast->toString().c_str());
}

TEST_F(ParserTest, Func1) {
    ASSERT_TRUE(Parse("func_arg(arg1 :Int8, arg2) :Int8 := { $arg1+$arg2; };"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("func_arg(arg1:Int8, arg2):Int8 := {$arg1 + $arg2;};", ast->toString().c_str());
}

TEST_F(ParserTest, Func2) {
    ASSERT_TRUE(Parse("func_arg(arg1:&Int8, &arg2) :&Int8 := { $arg1+$arg2; };"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("func_arg(arg1:&Int8, &arg2):&Int8 := {$arg1 + $arg2;};", ast->toString().c_str());
}

TEST_F(ParserTest, Func3) {
    ASSERT_TRUE(Parse("$res:Int8 ::= func_arg(100, 100);"));
    ASSERT_EQ(TermID::CREATE, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("$res:Int8 ::= func_arg(100, 100);", ast->toString().c_str());
}

TEST_F(ParserTest, Func4) {
    ASSERT_TRUE(Parse("res() := func_arg(100, 100); res() := func_arg(100, 100); res() := func_arg(100, 100);"));
}

TEST_F(ParserTest, Comment) {
    ASSERT_TRUE(Parse("#!line1"));
    ASSERT_TRUE(Parse("#!line1\n"));
    ASSERT_TRUE(Parse("#!line1\n#!line2"));
    ASSERT_TRUE(Parse("#!line1\n#!line2\n\n#!line4"));
    //ASSERT_EQ(TermID::COMMENT, ast->getTermID()) << EnumStr(ast->getTermID());
}

TEST_F(ParserTest, Comment2) {
    ASSERT_TRUE(Parse("#!line1\n#line2\n \n\n/* \n \n */ \n"));
    //    ASSERT_EQ(TermID::BLOCK, ast->getTermID()) << EnumStr(ast->getTermID());
    //    ASSERT_EQ(3, ast->m_block.size());
    //    ASSERT_EQ(TermID::COMMENT, ast->m_block[0]->getTermID()) << EnumStr(ast->getTermID());
    //    ASSERT_EQ(TermID::COMMENT, ast->m_block[1]->getTermID()) << EnumStr(ast->getTermID());
    //    ASSERT_EQ(TermID::COMMENT, ast->m_block[2]->getTermID()) << EnumStr(ast->getTermID());
}

TEST_F(ParserTest, Comment3) {
    const char *str = "print(str=\"\") := { {%  %} };\n"
            "#!/bin/nlc;\n"
            "\n"
            "\n"
            "# @print(\"Привет, мир!\\n\");\n";
    ASSERT_TRUE(Parse(str));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    //    ASSERT_EQ(3, ast->m_block.size());
    //    ASSERT_EQ(TermID::FUNCTION, ast->m_block[0]->getTermID())<< EnumStr(ast->getTermID());
    //    ASSERT_EQ(TermID::COMMENT, ast->m_block[1]->getTermID())<< EnumStr(ast->getTermID());
    //    ASSERT_EQ(TermID::COMMENT, ast->m_block[2]->getTermID())<< EnumStr(ast->getTermID());
}

TEST_F(ParserTest, Comment4) {
    const char *str = "#!/bin/nlc;\n"
            "print1(str=\"\") := {%  %};\n"
            "print2(str=\"\") := {%  %};\n"
            "# @print(\"Привет, мир!\\n\");\n";
    ASSERT_TRUE(Parse(str));
    ASSERT_EQ(TermID::BLOCK, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_EQ(2, ast->m_block.size());
    //    ASSERT_EQ(TermID::COMMENT, ast->m_block[0]->getTermID()) << EnumStr(ast->getTermID());
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->m_block[0]->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->m_block[1]->getTermID()) << newlang::toString(ast->getTermID());
    //    ASSERT_EQ(TermID::COMMENT, ast->m_block[2]->getTermID())<< EnumStr(ast->getTermID());
}

TEST_F(ParserTest, Comment5) {
    const char *str = "term();\n"
            "print1(str=\"\") := {%  %};\n"
            "print2(str=\"\") := { {%  %} };\n\n"
            "print3(str=\"\") := {%  %};\n\n\n"
            "# @print(\"Привет, мир!\\n\");\n";
    ASSERT_TRUE(Parse(str));
    ASSERT_EQ(TermID::BLOCK, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_EQ(4, ast->m_block.size());
    ASSERT_EQ(TermID::NAME, ast->m_block[0]->getTermID()) << newlang::toString(ast->getTermID());

    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->m_block[1]->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->m_block[2]->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->m_block[3]->getTermID()) << newlang::toString(ast->getTermID());
}

TEST_F(ParserTest, Comment6) {
    ASSERT_TRUE(Parse("# \\\\macro \\\\\\"));
    ASSERT_FALSE(ast->size());
    ASSERT_TRUE(Parse("# \\\\macro \\\\\\\n"));
    ASSERT_FALSE(ast->size());
    ASSERT_TRUE(Parse("/* \\\\macro \\\\\\ */"));
    ASSERT_FALSE(ast->size());
    ASSERT_TRUE(Parse("/* \\\\macro \\\\\\\n*/"));
    ASSERT_FALSE(ast->size());
    ASSERT_TRUE(Parse("/* \\\\macro \\\\\\\n\n*/"));
    ASSERT_FALSE(ast->size());
    ASSERT_TRUE(Parse("/*/* \\\\macro \\\\\\\n\n*/*/"));
    ASSERT_FALSE(ast->size());
}

TEST_F(ParserTest, CommentIncluded) {
    //    const char *str = "/* !!!!!!! \n"
    //            "@print(\"Привет, мир!\\n\");\n*/";
    //    "# @print(\"Привет, мир!\\n\");\n";
    //    ASSERT_TRUE(Parse(str));

    const char *str2 = "/* /* /* /* term();\n"
            "print1(str=\"\") ::= term();\n"
            "print2(str=\"\") ::= term();\n\n */ "
            "print3( */ str=\"\") ::= term();\n\n\n"
            "ddd  */  "
            "# @print(\"Привет, мир!\\n\");\n";
    ASSERT_TRUE(Parse(str2));
}

TEST_F(ParserTest, Types) {
    EXPECT_TRUE(Parse(":type1 := :type;"));
    EXPECT_TRUE(Parse(":type2 := :type;"));
    EXPECT_TRUE(Parse(":type3 := type();"));
    EXPECT_TRUE(Parse(":type4 := type();;"));

    //    EXPECT_TRUE(Parse(":type5 ::= ()"));
    //    EXPECT_TRUE(Parse(":type6 ::= ();"));
    EXPECT_TRUE(Parse(":type7 ::= :Type;"));
    EXPECT_TRUE(Parse(":type8 ::= :Type();;"));

    EXPECT_TRUE(Parse(":type9 := (1234,);"));
    EXPECT_TRUE(Parse(":type10 := (1234, name=1234,);"));
    EXPECT_TRUE(Parse(":type11 := class1(1234);"));
    EXPECT_TRUE(Parse(":type12 := :class1(1234, name=1234);;"));

    //    EXPECT_TRUE(Parse(":type13 := class1(), class2()"));
    //    EXPECT_TRUE(Parse(":type14 := class1(), class2(), (name=value);"));
    //    EXPECT_TRUE(Parse(":type15 := :class1, :class1(arg, arg=222)"));
    //    EXPECT_TRUE(Parse(":type16 := class1(args), (extra,), (extra=222,);"));

    EXPECT_TRUE(Parse(":type17 := class(1234);"));
    //    EXPECT_TRUE(Parse(":type18 := class(name=1234), class2();"));
    //    EXPECT_TRUE(Parse(":type19 := class(name=1234), class2()"));
    EXPECT_TRUE(Parse(":type20 := (1234, name=1234,);"));
}

TEST_F(ParserTest, Const2) {
    ASSERT_TRUE(Parse("const^  ::=   \"CONST\";"));
    ASSERT_EQ(TermID::CREATE, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("const^ ::= \"CONST\";", ast->toString().c_str());
}

TEST_F(ParserTest, Const3) {
    ASSERT_TRUE(Parse("const^  :=   123;"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("const^ := 123;", ast->toString().c_str());
}

TEST_F(ParserTest, Sequence) {
    ASSERT_NO_THROW(Parse(";"));
    ASSERT_NO_THROW(Parse(";;"));
    ASSERT_NO_THROW(Parse(";;;"));
    ASSERT_NO_THROW(Parse("val;"));
    ASSERT_NO_THROW(Parse("val;val;"));

    ASSERT_NO_THROW(Parse("val;;"));
    ASSERT_NO_THROW(Parse("val;;;"));
    ASSERT_NO_THROW(Parse("val;;;;val;;;;"));

    ASSERT_NO_THROW(Parse("val();"));
    ASSERT_NO_THROW(Parse("val();val();"));

    ASSERT_NO_THROW(Parse("val();"));
    ASSERT_NO_THROW(Parse("val();;;"));
    ASSERT_NO_THROW(Parse("val();;;val();;;"));

    //    ASSERT_NO_THROW(Parse("(){};"));
    ASSERT_NO_THROW(Parse("_()::={val;};"));
    ASSERT_NO_THROW(Parse("_()::={val;val;};"));

    ASSERT_NO_THROW(Parse("_( ):-{val;;}; _()::={*fff;*};"));
    ASSERT_NO_THROW(Parse("_( ):-{val;;;};  _()::={*fff;*};;"));
    ASSERT_NO_THROW(Parse("_()::={val;;;;val;_()::={_()::={fff;};};};;;;"));
    ASSERT_NO_THROW(Parse("_()::={*val;;;;val;;;_(123, 333)::-{_()::={*fff;*};};_(  ):-{fff;};*};;;;"));

    ASSERT_NO_THROW(Parse("_()::={val();};"));
    ASSERT_NO_THROW(Parse("_()::={val();val();};"));
    ASSERT_NO_THROW(Parse("_()::={val();val();;};"));

    ASSERT_NO_THROW(Parse("_()::={val();};"));
    ASSERT_NO_THROW(Parse("_()::={val();;;};;;"));
    ASSERT_NO_THROW(Parse("_()::= {val();;;val();;;};;;"));
    ASSERT_NO_THROW(Parse("_()::= {val();;;_()::={val();};;;;};;;"));
}

TEST_F(ParserTest, BlockTry) {
    ASSERT_TRUE(Parse("_()::={*1; 2; 3;*}; 4;"));
    ASSERT_EQ(2, ast->m_block.size());
    ASSERT_EQ(TermID::BLOCK, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_EQ(TermID::CREATE, ast->m_block[0]->getTermID()) << newlang::toString(ast->m_block[0]->getTermID());
    ASSERT_EQ(TermID::INTEGER, ast->m_block[1]->getTermID()) << newlang::toString(ast->m_block[1]->getTermID());

    ASSERT_TRUE(Parse("_():={- 1; 2; 3; --4--; 5; 6;-}; 100;"));
    ASSERT_EQ(2, ast->m_block.size());
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->m_block[0]->getTermID()) << newlang::toString(ast->m_block[0]->getTermID());
    ASSERT_EQ(TermID::INTEGER, ast->m_block[1]->getTermID()) << newlang::toString(ast->m_block[1]->getTermID());

    ASSERT_TRUE(Parse("_()::-{+ 1; 2; 3; ++4++; 5; 6;+}; 100;"));
    ASSERT_EQ(2, ast->m_block.size());
    ASSERT_EQ(TermID::BLOCK, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_EQ(TermID::PUREFUNC, ast->m_block[0]->getTermID()) << newlang::toString(ast->m_block[0]->getTermID());

    ASSERT_TRUE(Parse("_():- {1; 2; 3; ++4++; 5; 6;};"));
    ASSERT_EQ(0, ast->m_block.size());
    ASSERT_EQ(TermID::PUREFUNC, ast->getTermID()) << newlang::toString(ast->getTermID());
}

TEST_F(ParserTest, Repeat) {
    ASSERT_TRUE(Parse("[val] <-> val;"));
    ASSERT_TRUE(Parse("val <-> [val];;"));
    ASSERT_TRUE(Parse("[val] <-> {val;};"));
    ASSERT_TRUE(Parse("[val] <-> {val;};;"));
    ASSERT_TRUE(Parse("[val] <-> {val;};"));
    ASSERT_TRUE(Parse("[val] <-> {val;};;"));

    ASSERT_TRUE(Parse("val <-> [val()];"));
    ASSERT_TRUE(Parse("val <-> [val()];;"));
    ASSERT_TRUE(Parse("[val()] <-> {val();};"));
    ASSERT_TRUE(Parse("[val()] <-> {val();};;"));
    ASSERT_TRUE(Parse("[val()] <-> {val();};"));
    ASSERT_TRUE(Parse("[val()] <-> {val();};;"));

    ASSERT_TRUE(Parse("[val()] <-> {val()};val();"));
    ASSERT_TRUE(Parse("val <-> [val()];val();"));
    ASSERT_TRUE(Parse("[val()] <-> {val();val();};"));
    ASSERT_TRUE(Parse("[val()] <-> {val();val();};;"));
    ASSERT_TRUE(Parse("[val()] <-> {val();val();};"));
    ASSERT_TRUE(Parse("[val()] <-> {val();val();};;"));
}

TEST_F(ParserTest, Else) {
    ASSERT_TRUE(Parse("[val] <-> val, [_]-->else;"));
    ASSERT_TRUE(Parse("[val] <->{val}, [_]-->{else}"));

    ASSERT_TRUE(Parse("{- expr -}, [_]-->else"));
    ASSERT_TRUE(Parse("{+ expr +}, [_]-->else"));
    ASSERT_TRUE(Parse("{* expr *}, [_]-->else"));

    ASSERT_TRUE(Parse("{- expr -}, [_]-->{else}"));
    ASSERT_TRUE(Parse("{+ expr +}, [_]-->{else}"));
    ASSERT_TRUE(Parse("{* expr *}, [_]-->{else}"));

    ASSERT_TRUE(Parse("{- expr -}, [_]-->{- else -}"));
    ASSERT_TRUE(Parse("{+ expr +}, [_]-->{+ else +}"));
    ASSERT_TRUE(Parse("{* expr *}, [_]-->{* else *}"));
}

/*
 * \while(1) {
 * }
 * \while(1) {+
 *      ++; # break
 * +}
 * \while(1){-
 *      --; # break
 * -}
 * \while(1) {*
 *      ++; # break
 *      --; # break
 * *}
 * 
 * \(){
 * }
 * \(){+
 * +}
 * \(){-
 * -}
 * \(){*
 *      ++; # break
 *      --; # break
 * *}
 * 
 */
TEST_F(ParserTest, OperInt) {
    ASSERT_TRUE(Parse("_()::={val * val}"));
    ASSERT_TRUE(Parse("_()::={+val * val+}"));
    ASSERT_TRUE(Parse("_():None::={- val * val -}"));
    ASSERT_TRUE(Parse("_():None ::= {*val * val*}"));
    ASSERT_TRUE(Parse("_()::= {val * val}"));
    ASSERT_TRUE(Parse("_()::= {+val * val+}"));
    ASSERT_TRUE(Parse("_()::= {-val * val-}"));
    ASSERT_TRUE(Parse("_()::= {*val * val*}"));
}

TEST_F(ParserTest, Operators) {
    ASSERT_TRUE(Parse("val + val;"));
    ASSERT_TRUE(Parse("val - val;"));
    ASSERT_TRUE(Parse("_()::={val * val;};"));
    ASSERT_TRUE(Parse("_()::={val / val;};"));
    ASSERT_TRUE(Parse("_()::={val ** val;};"));
    ASSERT_TRUE(Parse("_()::= {val**val;;};;"));

    ASSERT_TRUE(Parse("val + val();"));
    ASSERT_TRUE(Parse("val - val();;"));
    ASSERT_TRUE(Parse("val * val();;"));
    ASSERT_TRUE(Parse("_()::={val / val();};"));
    ASSERT_TRUE(Parse("_():None::={val ** val();};"));
    ASSERT_TRUE(Parse("_() :None ::=  {val ** val();};"));

    ASSERT_TRUE(Parse("val + val();"));
    ASSERT_TRUE(Parse("val - val();; val - val();"));
    ASSERT_TRUE(Parse("val * val();; val - val();;"));
    ASSERT_TRUE(Parse("_()::={val / val(); val - val()};;"));
    ASSERT_TRUE(Parse("_()::={val ** val(); val - val();};"));
    ASSERT_TRUE(Parse("_()::= {val ** val(); val - val();};;"));

    ASSERT_TRUE(Parse("(val + val());"));
    ASSERT_TRUE(Parse("(val - val()) + (val - val());"));
    ASSERT_TRUE(Parse("(val * val()) - (val - val());"));
    ASSERT_TRUE(Parse("_()::={val / val() + (val - val())};"));
    ASSERT_TRUE(Parse("_()::={val ** val() / (val - val());};"));
    ASSERT_TRUE(Parse("_()::= {* val ** val() / (val - val()); *} :None;"));
}

TEST_F(ParserTest, Repeat0) {
    ASSERT_TRUE(Parse("[human || $ttttt  &&  123    !=    test[0].field ] <-> if_1;"));
    ASSERT_STREQ("[human || $ttttt && 123 != test[0].field]<->if_1;", ast->toString().c_str());
}

TEST_F(ParserTest, Repeat1) {
    ASSERT_TRUE(Parse("[test == human] <-> if_1;"));
    ASSERT_STREQ("[test == human]<->if_1;", ast->toString().c_str());
}

TEST_F(ParserTest, Repeat2) {
    ASSERT_TRUE(Parse("[test != human] <-> {if_1;};"));
    ASSERT_STREQ("[test != human]<->{if_1;};", ast->toString().c_str());

    ASSERT_TRUE(Parse("{if_1;} <-> [test!=human];"));
    ASSERT_STREQ("{if_1;}<->[test != human];", ast->toString().c_str());
}

TEST_F(ParserTest, Repeat3) {
    ASSERT_TRUE(Parse("[test != human] <-> {if_1;if_2;then3;};"));
    ASSERT_STREQ("[test != human]<->{if_1; if_2; then3;};", ast->toString().c_str());

    ASSERT_TRUE(Parse("{if_1;if_2;then3;} <-> [test != human];"));
    ASSERT_STREQ("{if_1; if_2; then3;}<->[test != human];", ast->toString().c_str());
}

TEST_F(ParserTest, Repeat4) {
    ASSERT_TRUE(Parse("[test()] <-> {if_1;if_2;then3;};"));
    ASSERT_STREQ("[test()]<->{if_1; if_2; then3;};", ast->toString().c_str());

    ASSERT_TRUE(Parse("[test()] <-> {if_1;if_2;then3;};"));
    ASSERT_STREQ("[test()]<->{if_1; if_2; then3;};", ast->toString().c_str());

    ASSERT_TRUE(Parse("{if_1;if_2;then3;} <->  [test()];"));
    ASSERT_STREQ("{if_1; if_2; then3;}<->[test()];", ast->toString().c_str());

    ASSERT_TRUE(Parse("{if_1;if_2;then3;} <->  [test()];"));
    ASSERT_STREQ("{if_1; if_2; then3;}<->[test()];", ast->toString().c_str());
}

TEST_F(ParserTest, Repeat5) {
    ASSERT_TRUE(Parse(" [ test! ]<-> {if_1;if_2;then3;};"));
    ASSERT_STREQ("[test!]<->{if_1; if_2; then3;};", ast->toString().c_str());

    ASSERT_TRUE(Parse(" [test!] <-> {if_1;if_2;then3;};"));
    ASSERT_STREQ("[test!]<->{if_1; if_2; then3;};", ast->toString().c_str());

    ASSERT_TRUE(Parse("{if_1;if_2;then3;}<->[test!];"));
    ASSERT_STREQ("{if_1; if_2; then3;}<->[test!];", ast->toString().c_str());

    ASSERT_TRUE(Parse("{if_1;if_2;then3;}<->[test!];"));
    ASSERT_STREQ("{if_1; if_2; then3;}<->[test!];", ast->toString().c_str());
}

TEST_F(ParserTest, Repeat6) {
    ASSERT_TRUE(Parse("[test!] <-> {if_1;if_2;then3;};"));
    ASSERT_STREQ("[test!]<->{if_1; if_2; then3;};", ast->toString().c_str());
}

TEST_F(ParserTest, Repeat7) {
    ASSERT_TRUE(Parse("[test[0].field != $test!] <-> if_1;"));
    ASSERT_STREQ("[test[0].field != $test!]<->if_1;", ast->toString().c_str());
}

TEST_F(ParserTest, Range) {
    ASSERT_TRUE(Parse("0.1..0.9..0.1;"));
    ASSERT_STREQ("0.1..0.9..0.1", ast->toString().c_str());
}

TEST_F(ParserTest, Range1) {
    ASSERT_TRUE(Parse("[i!] <-> call();"));
    ASSERT_STREQ("[i!]<->call();", ast->toString().c_str());

    ASSERT_TRUE(Parse("call() <-> [i!];"));
    ASSERT_STREQ("call()<->[i!];", ast->toString().c_str());
}

TEST_F(ParserTest, Range2) {
    ASSERT_TRUE(Parse("[i()] <-> @error(\"Error\");"));
    ASSERT_STREQ("[i()]<->@error(\"Error\");", ast->toString().c_str());
}

TEST_F(ParserTest, RangeCall) {
    ASSERT_TRUE(Parse("0.1..(1+1)"));
    ASSERT_TRUE(Parse("0.1..(1*2)"));
    ASSERT_TRUE(Parse("0.1..term()"));
    ASSERT_TRUE(Parse("0.1..term()..(1*2+2-term)"));
    ASSERT_TRUE(Parse("$term..(term()+$term)..(-1*2+2-@term())"));
}

TEST_F(ParserTest, Follow) {
    //@todo Не получается сделать парсер с простым if, т.к. требуется вторая закрывающая точка с запятой
    ASSERT_TRUE(Parse("[test     ==    human    ||    ttttt&&123!=test.field ] --> if_1;"));
    ASSERT_STREQ("[test == human || ttttt && 123 != test.field]-->if_1;", ast->toString().c_str());
}

TEST_F(ParserTest, Follow0) {
    ASSERT_TRUE(Parse("[test] --> follow;"));
    ASSERT_STREQ("[test]-->follow;", ast->toString().c_str());

    ASSERT_TRUE(Parse("[test] --> follow;"));
    ASSERT_STREQ("[test]-->follow;", ast->toString().c_str());

    ASSERT_TRUE(Parse("[test] --> {follow;};"));
    ASSERT_STREQ("[test]-->{follow;};", ast->toString().c_str());

    ASSERT_TRUE(Parse("[test] --> {follow};"));
    ASSERT_STREQ("[test]-->{follow;};", ast->toString().c_str());

    ASSERT_TRUE(Parse("[test] --> {follow;};"));
    ASSERT_STREQ("[test]-->{follow;};", ast->toString().c_str());

    ASSERT_TRUE(Parse("[test] --> {follow;};"));
    ASSERT_STREQ("[test]-->{follow;};", ast->toString().c_str());
}

TEST_F(ParserTest, Follow1) {
    //@todo Не получается сделать парсер с простым if, т.к. требуется вторая закрывающая точка с запятой
    ASSERT_TRUE(Parse("[test == human] --> if_1;"));
    ASSERT_STREQ("[test == human]-->if_1;", ast->toString().c_str());
}

TEST_F(ParserTest, Follow2) {
    ASSERT_TRUE(Parse("[test != human] --> {if_1;};"));
    ASSERT_STREQ("[test != human]-->{if_1;};", ast->toString().c_str());
}

TEST_F(ParserTest, Follow3) {
    ASSERT_TRUE(Parse("[test!=human] --> {if_1;if_2;then3;};"));
    ASSERT_STREQ("[test != human]-->{if_1; if_2; then3;};", ast->toString().c_str());
}

TEST_F(ParserTest, Follow4) {
    ASSERT_TRUE(Parse("[test != human] --> {if_1;if_2;then3;}, [_] --> {else;};"));
    //    ASSERT_STREQ("(test!=human)->{if_1; if_2; then3;},\n (_) ->{else;};", ast->toString().c_str());
}

TEST_F(ParserTest, Follow5) {
    ASSERT_TRUE(Parse("[@test1('')] --> {then1;}, [$test2] --> {then2;then2;}, [@test3+$test3] --> {then3;};"));
    //    ASSERT_STREQ("(@test1)->{then1;}\n ($test2)->{then2; then2;}\n(@test3+$test3)->{then3;};", ast->toString().c_str());
}

TEST_F(ParserTest, Follow6) {
    ASSERT_TRUE(Parse("[@test1()] --> {then1;}, [$test2] --> {then2;}, [@test3+$test3] --> {then3;}, [_] --> {else;else();};"));
    //    ASSERT_STREQ("(@test1)->{then1;},\n ($test2)->{then2;},\n (@test3+$test3)->{then3;},\n _ ->{else; else();};", ast->toString().c_str());
}

TEST_F(ParserTest, DISABLED_Follow7) {
    ASSERT_TRUE(Parse("[test.field[0] > iter!!] --> if_1;"));
    //    ASSERT_STREQ("(test.field[0].field2>iter!!)->{if_1;};", ast->toString().c_str());
}

TEST_F(ParserTest, Return) {
    ASSERT_TRUE(Parse("--;"));
    ASSERT_TRUE(Parse("--;;"));
    ASSERT_TRUE(Parse("--_--;"));
    ASSERT_TRUE(Parse("--_--;;"));
    ASSERT_TRUE(Parse("--  eval --;"));
    ASSERT_TRUE(Parse("--eval--;;"));
    ASSERT_TRUE(Parse("--eval()--;"));
    ASSERT_TRUE(Parse("--  eval() --;;"));
    ASSERT_TRUE(Parse("--0--;"));
    ASSERT_TRUE(Parse("--0--;;"));
    ASSERT_TRUE(Parse("++  0.1 ++;"));
    ASSERT_TRUE(Parse("--  0.1    --;;"));
    ASSERT_TRUE(Parse("--[0,]--;"));
    ASSERT_TRUE(Parse("++(0,)++;;"));
    ASSERT_TRUE(Parse("--(0,1,2,3,):Class--;;"));
    ASSERT_TRUE(Parse("--(0,2,3,) :Class --;;"));
    ASSERT_TRUE(Parse("--(0,)--;;"));
    ASSERT_TRUE(Parse("--:Class(var)--;"));
    ASSERT_TRUE(Parse("--:Class[0](1)--;"));
    ASSERT_TRUE(Parse("--call()--;"));
    ASSERT_TRUE(Parse("--call()--;;"));
    ASSERT_TRUE(Parse("--call(arg)--;"));
    ASSERT_TRUE(Parse("--call(arg)--;;"));
    ASSERT_TRUE(Parse("--:class--;"));
    ASSERT_TRUE(Parse("--:class--;;"));
    ASSERT_TRUE(Parse("--:class()--;"));
    ASSERT_TRUE(Parse("--:class()--;;"));
    ASSERT_TRUE(Parse("--:class(arg)--;"));
    ASSERT_TRUE(Parse("++:class(arg)++;;"));

    ASSERT_TRUE(Parse("++;"));
    ASSERT_TRUE(Parse("++;;"));
    ASSERT_TRUE(Parse("++_++;"));
    ASSERT_TRUE(Parse("++_++;;"));
    ASSERT_TRUE(Parse("++  eval ++;"));
    ASSERT_TRUE(Parse("++eval++;;"));
    ASSERT_TRUE(Parse("++eval()++;"));
    ASSERT_TRUE(Parse("++  eval() ++;;"));
    ASSERT_TRUE(Parse("++0++;"));
    ASSERT_TRUE(Parse("++0++;;"));
    ASSERT_TRUE(Parse("++  0.1 ++;"));
    ASSERT_TRUE(Parse("++  0.1    ++;;"));
    ASSERT_TRUE(Parse("++[0,]++;"));
    ASSERT_TRUE(Parse("++(0,)++;;"));
    ASSERT_TRUE(Parse("++(0,1,2,3,):Class++;;"));
    ASSERT_TRUE(Parse("++(0,2,3,) :Class ++;;"));
    ASSERT_TRUE(Parse("++(0,)++;;"));
    ASSERT_TRUE(Parse("++:Class(var)++;"));
    ASSERT_TRUE(Parse("++:Class[0](1)++;"));
    ASSERT_TRUE(Parse("++call()++;"));
    ASSERT_TRUE(Parse("++call()++;;"));
    ASSERT_TRUE(Parse("++call(arg)++;"));
    ASSERT_TRUE(Parse("++call(arg)++;;"));
    ASSERT_TRUE(Parse("++:class++;"));
    ASSERT_TRUE(Parse("++:class++;;"));
    ASSERT_TRUE(Parse("++:class()++;"));
    ASSERT_TRUE(Parse("++:class()++;;"));
    ASSERT_TRUE(Parse("++:class(arg)++;"));
    ASSERT_TRUE(Parse("++:class(arg)++;;"));

}

/*
 * \\ alias \\ alias::name \\
 * \\ alias(arg, ...) \\ alias::name(\$arg, \$*) \\
 * alias
 * alias(...)
 * 
 * \\ if(cond)   \\ [ \$cond ] --> \\
 * \\ elif(cond) \\ ,[ \$cond ] --> \\
 * \\ else       \\ ,[_] --> \\
 * 
 * if(cond) {
 *      ...
 * } elif(cond) {
 *      ...
 * } else {
 *      ...
 * };
 * 
 * \\while(cond)\\ [\$cond] <-> \\
 * while(cond) {
 *      ...
 * };
 * 
 * \\dowhile(cond) \\ <->[\$cond]\\
 * {
 *      ...
 * } dowhile(cond);
 * 
 * \\return\\  --\\
 * \\return(...)\\  --\$*--\\
 * return;
 * return(...);
 * 
 * \\match(val, op)\\\  [\$val] \$op>\\\   Требуется передача операторов как аргументов? Нет, не требуется, так как до парсера не дойдет
 * \\match(val) op \\\ [\$val] \$op>\\\
 * \\case(...)\\ [\$*]--> \\
 * \\default\\ [_]-->\\
 * 
 * match(val, ~) {
 *      case(val) {
 *          ...
 *      };
 *      case(val2, val3) {
 *          ...
 *      };
 *      default {
 *          ...
 *      };
 * }; 
 * 
 */
TEST_F(ParserTest, MacroBuffer) {
    TermPtr term = Term::Create(parser::token_type::NAME, TermID::NAME, "name");
    term->AppendFollow(term);
    ASSERT_STREQ("name", term->m_text.c_str());

    TermPtr term2 = Term::Create(parser::token_type::NAME, TermID::NAME, "name2");
    term->AppendFollow(term2);
    ASSERT_STREQ("name", term->m_text.c_str());


    MacroBuffer macro;
    ASSERT_EQ(0, macro.size());

    ASSERT_TRUE(Parse("\\\\alias\\\\alias_name\\\\", &macro));

    ASSERT_EQ(1, macro.GetCount());
    ASSERT_TRUE(macro.find({"alias"}));

    ASSERT_ANY_THROW(Parse("\\\\ alias \\\\ alias_name2 \\\\", &macro));
    ASSERT_EQ(1, macro.GetCount()) << macro.Dump();
    ASSERT_TRUE(macro.find({"alias"}));

    ASSERT_ANY_THROW(Parse("\\\\alias+alias\\\\alias_name;aaaaaaaaaa\\\\", &macro));
    ASSERT_EQ(1, macro.GetCount());

    ASSERT_TRUE(Parse("{ \\\\alias_alias\\\\alias_alias2\\\\ }", &macro));
    ASSERT_FALSE(macro.find({"alias_alias"}));

    ASSERT_TRUE(Parse("{* \\\\alias_alias0\\\\alias_alias2\\\\ *}", &macro));
    ASSERT_FALSE(macro.find({"alias_alias0"}));

    ASSERT_TRUE(Parse("{- \\\\alias_alias1\\\\alias_alias2\\\\ -}", &macro));
    ASSERT_FALSE(macro.find({"alias_alias1"}));

    ASSERT_TRUE(Parse("{+ \\\\alias_alias2\\\\alias_alias2\\\\ +}", &macro));
    ASSERT_FALSE(macro.find({"alias_alias2"}));

    ASSERT_TRUE(Parse("{{ \\\\alias_alias3\\\\alias_alias2\\\\ }}", &macro));
    ASSERT_FALSE(macro.find({"alias_alias3"}));

    ASSERT_TRUE(Parse("\\\\alias\\\\\\\\\\", &macro));
    ASSERT_EQ(0, macro.GetCount());


    ASSERT_TRUE(Parse("\\\\if(args) \\\\ [\\$args] --> \\\\", &macro));

    ASSERT_EQ(1, macro.size());
    ASSERT_TRUE(macro.find({"if", "("})) << macro.Dump();
    ASSERT_TRUE(macro.find({"if", "("}));
    ASSERT_TRUE(macro.find({"if", "("})->Right());
    ASSERT_EQ(4, macro.find({"if", "("})->Right()->m_follow.size());

    ASSERT_TRUE(Parse("\\\\if2(...) \\\\\\ [ ... ] --> \\\\\\", &macro));

    ASSERT_EQ(2, macro.size());
    ASSERT_TRUE(macro.find({"if2", "("}));
    ASSERT_TRUE(macro.find({"if2", "("}));
    ASSERT_TRUE(macro.find({"if2", "("})->Right());
    ASSERT_STREQ(" [ ... ] --> ", macro.find({"if2", "("})->Right()->m_text.c_str());

}

TEST_F(ParserTest, MacroAlias) {
    MacroBuffer macro;
    ASSERT_EQ(0, macro.size());

    ASSERT_TRUE(Parse("\\\\alias\\\\replace\\\\", &macro));
    ASSERT_TRUE(Parse("\\\\alias2\\\\alias\\\\", &macro));
    ASSERT_TRUE(Parse("\\\\fail\\\\fail\\\\", &macro));

    ASSERT_EQ(3, macro.size());
    ASSERT_TRUE(macro.find({"alias"}));
    TermPtr macro_alias = macro.find({"alias"});
    ASSERT_STREQ("replace", macro_alias->Right()->m_follow[0]->m_text.c_str());

    ASSERT_TRUE(macro.find({"alias2"}));
    TermPtr macro_alias2 = macro.find({"alias2"});
    ASSERT_STREQ("alias", macro_alias2->Right()->m_follow[0]->m_text.c_str());

    ASSERT_TRUE(macro.find({"fail"}));
    TermPtr macro_fail = macro.find({"fail"});
    ASSERT_STREQ("fail", macro_fail->Right()->m_follow[0]->m_text.c_str());



    TermPtr term = Term::Create(parser::token_type::NAME, TermID::NAME, "alias");

    ASSERT_TRUE(macro.map::find(term->m_text) != macro.end());

    std::vector<MacroToken> vals = macro.map::find(term->m_text)->second;
    ASSERT_EQ(1, vals.size());

    LexerTokenType buff;
    ASSERT_EQ(MacroBuffer::CompareResult::NEXT_NAME, MacroBuffer::CompareMacro(buff, macro_alias));
    ASSERT_EQ(MacroBuffer::CompareResult::NEXT_NAME, MacroBuffer::CompareMacro(buff, macro_alias2));
    ASSERT_EQ(MacroBuffer::CompareResult::NEXT_NAME, MacroBuffer::CompareMacro(buff, macro_fail));


    buff.push_back(term);

    ASSERT_EQ(MacroBuffer::CompareResult::DONE, MacroBuffer::CompareMacro(buff, macro_alias));
    ASSERT_EQ(MacroBuffer::CompareResult::NOT_EQ, MacroBuffer::CompareMacro(buff, macro_alias2));
    ASSERT_EQ(MacroBuffer::CompareResult::NOT_EQ, MacroBuffer::CompareMacro(buff, macro_fail));


    term = Term::Create(parser::token_type::NAME, TermID::NAME, "alias");
    buff.push_back(term);


    ASSERT_EQ(MacroBuffer::CompareResult::DONE, MacroBuffer::CompareMacro(buff, macro_alias));
    ASSERT_EQ(MacroBuffer::CompareResult::NOT_EQ, MacroBuffer::CompareMacro(buff, macro_alias2));
    ASSERT_EQ(MacroBuffer::CompareResult::NOT_EQ, MacroBuffer::CompareMacro(buff, macro_fail));


    MacroBuffer::MacroArgsType macro_args;

    ASSERT_EQ(1, MacroBuffer::ExtractArgs(buff, macro_alias, macro_args));
    ASSERT_EQ(0, macro_args.size());

    //    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias2, macro_args));
    //    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_fail, macro_args));

    ASSERT_ANY_THROW(MacroBuffer::ExpandString(macro_alias, macro_args));

    BlockType block;
    block = MacroBuffer::ExpandMacros(macro_alias, macro_args);
    ASSERT_EQ(1, block.size());
    ASSERT_TRUE(block[0]);
    ASSERT_STREQ("replace", block[0]->m_text.c_str());



    ASSERT_EQ(3, macro.size());
    for (auto &elem : macro) {
        LOG_DEBUG("Hash: %s", elem.first.c_str());
    }

    ASSERT_TRUE(Parse("alias", &macro));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("replace", ast->toString().c_str());

    ASSERT_TRUE(Parse("alias2", &macro));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("replace", ast->toString().c_str());

    //    ASSERT_ANY_THROW(Parse("fail", &macro));
}

TEST_F(ParserTest, MacroMacro) {
    MacroBuffer macro;
    ASSERT_EQ(0, macro.size());

    ASSERT_TRUE(Parse("\\\\alias replace\\\\replace\\\\", &macro));
    ASSERT_EQ(1, macro.GetCount()) << macro.Dump();
    ASSERT_TRUE(Parse("\\\\alias second\\\\second\\\\", &macro));
    ASSERT_EQ(2, macro.GetCount()) << macro.Dump();
    ASSERT_TRUE(Parse("\\\\text\\\\\\text;\n text\\\\\\", &macro));
    ASSERT_EQ(3, macro.GetCount()) << macro.Dump();
    ASSERT_TRUE(Parse("\\\\dsl\\\\\\ \\\\m1\\\\mm\\\\;\n  \\\\m2\\\\mm\\\\;\n\\\\\\", &macro));

    ASSERT_EQ(4, macro.GetCount()) << macro.Dump();
    ASSERT_TRUE(macro.find({"alias", "replace"}));
    TermPtr macro_replace = macro.find({"alias", "replace"});
    ASSERT_TRUE(macro.find({"alias", "second"}));
    TermPtr macro_second = macro.find({"alias", "second"});
    ASSERT_TRUE(macro.find({"text"}));
    TermPtr macro_text = macro.find({"text"});
    ASSERT_TRUE(macro.find({"dsl"}));
    TermPtr macro_dsl = macro.find({"dsl"});


    TermPtr term = Term::Create(parser::token_type::NAME, TermID::NAME, "alias");

    ASSERT_TRUE(macro.find({"alias", "replace"}));
    ASSERT_TRUE(macro.find({"alias", "second"}));


    LexerTokenType buff;
    ASSERT_EQ(MacroBuffer::CompareResult::NEXT_NAME, MacroBuffer::CompareMacro(buff, macro_replace));
    ASSERT_EQ(MacroBuffer::CompareResult::NEXT_NAME, MacroBuffer::CompareMacro(buff, macro_second));
    ASSERT_EQ(MacroBuffer::CompareResult::NEXT_NAME, MacroBuffer::CompareMacro(buff, macro_text));
    ASSERT_EQ(MacroBuffer::CompareResult::NEXT_NAME, MacroBuffer::CompareMacro(buff, macro_dsl));


    buff.push_back(term);

    ASSERT_EQ(MacroBuffer::CompareResult::NEXT_NAME, MacroBuffer::CompareMacro(buff, macro_replace));
    ASSERT_EQ(MacroBuffer::CompareResult::NEXT_NAME, MacroBuffer::CompareMacro(buff, macro_second));
    ASSERT_EQ(MacroBuffer::CompareResult::NOT_EQ, MacroBuffer::CompareMacro(buff, macro_text));
    ASSERT_EQ(MacroBuffer::CompareResult::NOT_EQ, MacroBuffer::CompareMacro(buff, macro_dsl));

    buff.push_back(term);

    ASSERT_EQ(MacroBuffer::CompareResult::NOT_EQ, MacroBuffer::CompareMacro(buff, macro_replace));
    ASSERT_EQ(MacroBuffer::CompareResult::NOT_EQ, MacroBuffer::CompareMacro(buff, macro_second));
    ASSERT_EQ(MacroBuffer::CompareResult::NOT_EQ, MacroBuffer::CompareMacro(buff, macro_text));
    ASSERT_EQ(MacroBuffer::CompareResult::NOT_EQ, MacroBuffer::CompareMacro(buff, macro_dsl));

    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "second"));

    ASSERT_EQ(MacroBuffer::CompareResult::NOT_EQ, MacroBuffer::CompareMacro(buff, macro_replace));
    ASSERT_EQ(MacroBuffer::CompareResult::NOT_EQ, MacroBuffer::CompareMacro(buff, macro_second));
    ASSERT_EQ(MacroBuffer::CompareResult::NOT_EQ, MacroBuffer::CompareMacro(buff, macro_text));
    ASSERT_EQ(MacroBuffer::CompareResult::NOT_EQ, MacroBuffer::CompareMacro(buff, macro_dsl));

    buff.pop_front();

    ASSERT_EQ(MacroBuffer::CompareResult::NOT_EQ, MacroBuffer::CompareMacro(buff, macro_replace));
    ASSERT_EQ(MacroBuffer::CompareResult::DONE, MacroBuffer::CompareMacro(buff, macro_second));
    ASSERT_EQ(MacroBuffer::CompareResult::NOT_EQ, MacroBuffer::CompareMacro(buff, macro_text));
    ASSERT_EQ(MacroBuffer::CompareResult::NOT_EQ, MacroBuffer::CompareMacro(buff, macro_dsl));



    ASSERT_TRUE(Parse("alias", &macro));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("alias", ast->toString().c_str());

    ASSERT_TRUE(Parse("alias replace", &macro));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("replace", ast->m_text.c_str());

    ASSERT_TRUE(Parse("alias second", &macro));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("second", ast->m_text.c_str());



    ASSERT_EQ(4, macro.GetCount());
    ASSERT_FALSE(macro.find({"m1"})) << macro.Dump();
    ASSERT_FALSE(macro.find({"m2"})) << macro.Dump();

    ASSERT_TRUE(Parse("dsl", &macro));

    ASSERT_EQ(6, macro.GetCount());
    ASSERT_TRUE(macro.find({"m1"})) << macro.Dump();
    ASSERT_TRUE(macro.find({"m2"})) << macro.Dump();
}

TEST_F(ParserTest, MacroArgs) {


    MacroBuffer macro;
    std::vector<MacroToken> vect;
    MacroBuffer::iterator iter;
    TermPtr macro_alias1;
    //    
    //    ASSERT_TRUE(Parse("\\\\alias\\\\replace1\\\\;\\\\alias2\\\\replace2\\\\", &macro));
    //    ASSERT_EQ(2, macro.GetCount());
    //
    //    iter = macro.map::find("alias");
    //    ASSERT_TRUE(iter != macro.end());
    //
    // vect = iter->second;
    //
    //    ASSERT_EQ(1, vect.size()) << macro.Dump();
    //
    //    macro_alias1 = vect[0].macro;
    //    ASSERT_TRUE(macro_alias1);
    //    ASSERT_STREQ("alias", macro_alias1->m_text.c_str());
    //    ASSERT_FALSE(macro_alias1->isCall()) << macro_alias1->toString().c_str();
    //    ASSERT_TRUE(macro_alias1->GetTokenID() == TermID::MACRO_DEF) << macro_alias1->toString().c_str();
    //    ASSERT_TRUE(macro_alias1->Right());
    //    ASSERT_EQ(1, macro_alias1->Right()->m_follow.size());
    //    ASSERT_STREQ("replace1", macro_alias1->Right()->m_follow[0]->m_text.c_str());


    macro.clear();
    ASSERT_EQ(0, macro.GetCount());

    ASSERT_TRUE(Parse("\\\\alias\\\\replace1\\\\", &macro));
    ASSERT_TRUE(Parse("\\\\alias($arg)\\\\ replace2(\\$arg) \\\\", &macro));
    ASSERT_ANY_THROW(Parse("\\\\alias(...)\\\\replace3\\\\", &macro));
//    ASSERT_TRUE(Parse("\\\\alias($arg) second\\\\replace3(\\$*)\\\\;\n \\\\macro($arg)\\\\\\ 3*\\$arg \\\\\\", &macro));
    ASSERT_TRUE(Parse("\\\\alias($arg) second\\\\replace3(\\$*)\\\\;", &macro));
    ASSERT_TRUE(Parse("\\\\macro($arg)\\\\\\ 3*\\$arg \\\\\\", &macro));

    ASSERT_EQ(4, macro.GetCount());

    iter = macro.map::find("alias");
    ASSERT_TRUE(iter != macro.end());

    vect = iter->second;

    ASSERT_EQ(3, vect.size());

    macro_alias1 = vect[0].macro;
    ASSERT_TRUE(macro_alias1);
    ASSERT_STREQ("alias", macro_alias1->m_text.c_str());
    ASSERT_FALSE(macro_alias1->isCall());
    ASSERT_TRUE(macro_alias1->GetTokenID() == TermID::MACRO_DEF) << macro_alias1->toString().c_str();
    ASSERT_TRUE(macro_alias1->Right());
    ASSERT_EQ(1, macro_alias1->Right()->m_follow.size());
    ASSERT_STREQ("replace1", macro_alias1->Right()->m_follow[0]->m_text.c_str());


    TermPtr macro_alias2 = vect[1].macro;
    ASSERT_TRUE(macro_alias2);
    ASSERT_STREQ("alias", macro_alias2->m_text.c_str());
    ASSERT_TRUE(macro_alias2->GetTokenID() == TermID::MACRO_DEF) << macro_alias2->toString().c_str();
    ASSERT_TRUE(macro_alias2->isCall());
    ASSERT_TRUE(macro_alias2->Right());
    ASSERT_EQ(4, macro_alias2->Right()->m_follow.size());
    ASSERT_STREQ("replace2", macro_alias2->Right()->m_follow[0]->m_text.c_str());
    ASSERT_STREQ("(", macro_alias2->Right()->m_follow[1]->m_text.c_str());
    ASSERT_STREQ("\\$arg", macro_alias2->Right()->m_follow[2]->m_text.c_str());
    ASSERT_STREQ(")", macro_alias2->Right()->m_follow[3]->m_text.c_str());

    TermPtr macro_alias3 = vect[2].macro;
    ASSERT_TRUE(macro_alias3);
    ASSERT_STREQ("alias", macro_alias3->m_text.c_str());
    ASSERT_TRUE(macro_alias3->GetTokenID() == TermID::MACRO_DEF) << macro_alias3->toString().c_str();
    ASSERT_TRUE(macro_alias3->isCall());
    ASSERT_EQ(1, macro_alias3->size());
    ASSERT_TRUE(macro_alias3->Right());
    ASSERT_EQ(4, macro_alias3->Right()->m_follow.size());
    ASSERT_STREQ("replace3", macro_alias3->Right()->m_follow[0]->m_text.c_str());
    ASSERT_STREQ("(", macro_alias3->Right()->m_follow[1]->m_text.c_str());
    ASSERT_STREQ("\\$*", macro_alias3->Right()->m_follow[2]->m_text.c_str());
    ASSERT_STREQ(")", macro_alias3->Right()->m_follow[3]->m_text.c_str());

    ASSERT_EQ(macro_alias1.get(), macro.find({"alias"}).get());
    ASSERT_EQ(macro_alias2.get(), macro.find({"alias", "("}).get());
    ASSERT_EQ(macro_alias3.get(), macro.find({"alias", "(", "second"}).get());


    iter = macro.map::find("macro");
    ASSERT_TRUE(iter != macro.end());

    vect = iter->second;
    ASSERT_EQ(1, vect.size());
    TermPtr macro_macro1 = vect[0].macro;
    ASSERT_TRUE(macro_macro1);
    ASSERT_STREQ("macro", macro_macro1->m_text.c_str());
    ASSERT_EQ(macro_macro1.get(), macro.find({"macro", "("}).get());
    ASSERT_TRUE(macro_macro1->GetTokenID() == TermID::MACRO_STR) << macro_macro1->toString().c_str();


    LexerTokenType buff;
    MacroBuffer::MacroArgsType macro_args;

    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias1, macro_args));
    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias2, macro_args));
    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias3, macro_args));
    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_macro1, macro_args));

    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "alias"));

    ASSERT_EQ(1, MacroBuffer::ExtractArgs(buff, macro_alias1, macro_args));
    ASSERT_EQ(0, macro_args.size()) << MacroBuffer::Dump(macro_args);

    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias2, macro_args));
    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias3, macro_args));
    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_macro1, macro_args));


    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, "("));

    ASSERT_EQ(1, MacroBuffer::ExtractArgs(buff, macro_alias1, macro_args));
    ASSERT_EQ(0, macro_args.size()) << MacroBuffer::Dump(macro_args);

    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias2, macro_args));
    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias3, macro_args));
    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_macro1, macro_args));

    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "value"));


    ASSERT_EQ(1, MacroBuffer::ExtractArgs(buff, macro_alias1, macro_args));
    ASSERT_EQ(0, macro_args.size()) << MacroBuffer::Dump(macro_args);

    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias2, macro_args));
    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias3, macro_args));
    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_macro1, macro_args));


    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, ","));


    ASSERT_EQ(1, MacroBuffer::ExtractArgs(buff, macro_alias1, macro_args));
    ASSERT_EQ(0, macro_args.size()) << MacroBuffer::Dump(macro_args);

    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias2, macro_args));
    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias3, macro_args));
    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_macro1, macro_args));


    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "value2"));

    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "value3"));


    ASSERT_EQ(1, MacroBuffer::ExtractArgs(buff, macro_alias1, macro_args));
    ASSERT_EQ(0, macro_args.size()) << MacroBuffer::Dump(macro_args);

    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias2, macro_args));
    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias3, macro_args));
    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_macro1, macro_args));



    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, ")"));


    ASSERT_EQ(1, MacroBuffer::ExtractArgs(buff, macro_alias1, macro_args));
    ASSERT_EQ(0, macro_args.size()) << MacroBuffer::Dump(macro_args);

    ASSERT_EQ(7, MacroBuffer::ExtractArgs(buff, macro_alias2, macro_args));
    ASSERT_EQ(4, macro_args.size()) << MacroBuffer::Dump(macro_args);

    ASSERT_EQ(7, MacroBuffer::ExtractArgs(buff, macro_macro1, macro_args));
    ASSERT_EQ(4, macro_args.size()) << MacroBuffer::Dump(macro_args);

    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias3, macro_args));


    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "last_term"));

    ASSERT_EQ(1, MacroBuffer::ExtractArgs(buff, macro_alias1, macro_args));
    ASSERT_EQ(0, macro_args.size()) << MacroBuffer::Dump(macro_args);

    BlockType res = MacroBuffer::ExpandMacros(macro_alias1, macro_args);
    ASSERT_EQ(1, res.size());
    ASSERT_STREQ("replace1", res[0]->m_text.c_str());


    ASSERT_EQ(7, MacroBuffer::ExtractArgs(buff, macro_alias2, macro_args));
    ASSERT_EQ(4, macro_args.size()) << MacroBuffer::Dump(macro_args);

    res = MacroBuffer::ExpandMacros(macro_alias2, macro_args);
    ASSERT_EQ(4, res.size());
    ASSERT_STREQ("replace2", res[0]->m_text.c_str());
    ASSERT_STREQ("(", res[1]->m_text.c_str());
    ASSERT_STREQ("value", res[2]->m_text.c_str()) << MacroBuffer::Dump(macro_args);
    ASSERT_STREQ(")", res[3]->m_text.c_str());

    // Нет анализаи на соотеветстви макроса, только извлечение значений шаблона
    ASSERT_EQ(8, MacroBuffer::ExtractArgs(buff, macro_alias3, macro_args));
    ASSERT_EQ(4, macro_args.size()) << MacroBuffer::Dump(macro_args);

    res = MacroBuffer::ExpandMacros(macro_alias3, macro_args);
    ASSERT_EQ(7, res.size());
    ASSERT_STREQ("replace3", res[0]->m_text.c_str());
    ASSERT_STREQ("(", res[1]->m_text.c_str());
    ASSERT_STREQ("value", res[2]->m_text.c_str()) << MacroBuffer::Dump(macro_args);
    ASSERT_STREQ(",", res[3]->m_text.c_str()) << MacroBuffer::Dump(macro_args);
    ASSERT_STREQ("value2", res[4]->m_text.c_str()) << MacroBuffer::Dump(macro_args);
    ASSERT_STREQ("value3", res[5]->m_text.c_str()) << MacroBuffer::Dump(macro_args);
    ASSERT_STREQ(")", res[6]->m_text.c_str());


    buff.clear();
    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "macro"));
    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, "("));
    buff.push_back(Term::Create(parser::token_type::NUMBER, TermID::NUMBER, "5"));
    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, ")"));

    TermPtr macro_macro = macro.find({"macro", "("});
    ASSERT_TRUE(macro_macro);
    ASSERT_EQ(macro_macro.get(), macro.find({"macro", "("}).get());

    ASSERT_EQ(4, MacroBuffer::ExtractArgs(buff, macro_macro, macro_args));
    ASSERT_EQ(3, macro_args.size()) << MacroBuffer::Dump(macro_args);

    std::string str = MacroBuffer::ExpandString(macro_macro, macro_args);
    ASSERT_STREQ(" 3*5  ", str.c_str());

    //    body = "\\macro(11, ...)";
    //    args = Parser::ParseMacroArgs(body);
    //    ASSERT_EQ(2, args.size());
    //    ASSERT_STREQ("11", args[0].c_str());
    //    ASSERT_STREQ("...", args[1].c_str());
    //
    //    body = "\\return(...)    --\\$*--";
    //    args = Parser::ParseMacroArgs(body);
    //    ASSERT_EQ(1, args.size());
    //    ASSERT_STREQ("...", args[0].c_str());
    //
    //    ASSERT_ANY_THROW(
    //            body = "\\macro(,)";
    //            args = Parser::ParseMacroArgs(body);
    //            );
    //    ASSERT_ANY_THROW(
    //            body = "\\macro( , )";
    //            args = Parser::ParseMacroArgs(body);
    //            );
    //    ASSERT_ANY_THROW(
    //            body = "\\macro(,,)";
    //            args = Parser::ParseMacroArgs(body);
    //            );
    //
    //    body = "\\macro)";
    //    args = Parser::ParseMacroArgs(body);
    //    ASSERT_EQ(0, args.size());
    //
    //    body = "\\macro\n";
    //    args = Parser::ParseMacroArgs(body);
    //    ASSERT_EQ(0, args.size());
    //
    //    body = "\\macro)";
    //    args = Parser::ParseMacroArgs(body);
    //    ASSERT_EQ(0, args.size());
    //
    //    body = "\\\\macro()";
    //    args = Parser::ParseMacroArgs(body);
    //    ASSERT_EQ(0, args.size());
    //
    //    body = "macro";
    //    args = Parser::ParseMacroArgs(body);
    //    ASSERT_EQ(0, args.size());
    //
    //    body = "";
    //    args = Parser::ParseMacroArgs(body);
    //    ASSERT_EQ(0, args.size());
}

//TEST_F(ParserTest, MacroExpand) {
//
//    std::string macro = "\\macro 12345";
//    std::string body = "\\macro";
//    std::string result = Parser::ExpandMacro(macro, body);
//    ASSERT_STREQ("12345", result.c_str());
//
//    body = "\\macro \\macro";
//    result = Parser::ExpandMacro(macro, body);
//    ASSERT_STREQ("12345 12345", result.c_str());
//
//    body = "\\macro \\macro \\macro";
//    result = Parser::ExpandMacro(macro, body);
//    ASSERT_STREQ("12345 12345 12345", result.c_str());
//
//    macro = "\\macro() 12345";
//    body = "\\macro \\macro \\macro";
//    result = Parser::ExpandMacro(macro, body);
//    ASSERT_STREQ("\\macro \\macro \\macro", result.c_str());
//
//    macro = "\\macro()12345";
//    body = "\\macro() \\macro() \\macro";
//    result = Parser::ExpandMacro(macro, body);
//    ASSERT_STREQ("12345 12345 \\macro", result.c_str());
//
//    macro = "\\macro()12345";
//    body = "\\macro(88) \\macro(99) \\macro";
//    result = Parser::ExpandMacro(macro, body);
//    ASSERT_STREQ("12345 12345 \\macro", result.c_str());
//
//
//    macro = "\\macro(arg)\\$arg";
//    body = "\\macro(88)";
//    result = Parser::ExpandMacro(macro, body);
//    ASSERT_STREQ("88", result.c_str());
//
//    macro = "\\macro(arg)no arg \\$arg";
//    body = "\\macro(99)";
//    result = Parser::ExpandMacro(macro, body);
//    ASSERT_STREQ("no arg 99", result.c_str());
//
//    macro = "\\macro(arg)  no arg \\$arg no arg";
//    body = "\\macro(88) \\macro(99)";
//    result = Parser::ExpandMacro(macro, body);
//    ASSERT_STREQ("  no arg 88 no arg   no arg 99 no arg", result.c_str());
//
//    macro = "\\macro(arg1,arg2)  \\$arg1 arg \\$arg2 \\$arg2";
//    body = "\\macro(88,99)";
//    result = Parser::ExpandMacro(macro, body);
//    ASSERT_STREQ("  88 arg 99 99", result.c_str());
//
//    macro = "\\macro(arg1,arg2)  \\$arg1 \\$arg2 \\$arg2";
//    body = "\\macro(1,2) \\macro(3,44)";
//    result = Parser::ExpandMacro(macro, body);
//    ASSERT_STREQ("  1 2 2   3 44 44", result.c_str());
//
//    macro = "\\macro(arg1,arg2)  \\$1 \\$2 \\$1";
//    body = "\\macro(1,2) \\macro(3,44)";
//    result = Parser::ExpandMacro(macro, body);
//    ASSERT_STREQ("  1 2 1   3 44 3", result.c_str());
//
//    macro = "\\macro(arg1,arg2)\\$*";
//    body = "\\macro(1,2)";
//    result = Parser::ExpandMacro(macro, body);
//    ASSERT_STREQ("1,2", result.c_str());
//
//    macro = "\\macro(arg1,arg2)\\$* \\$1 \\$arg2\\$*";
//    body = "\\macro(1,2)";
//    result = Parser::ExpandMacro(macro, body);
//    ASSERT_STREQ("1,2 1 21,2", result.c_str());
//
//    macro = "\\macro(arg1,arg2)\\$* \\$1 \\$arg2\\$*";
//    body = "\\macro(1,2)\\macro(1,2)";
//    result = Parser::ExpandMacro(macro, body);
//    ASSERT_STREQ("1,2 1 21,21,2 1 21,2", result.c_str());
//
//    macro = "\\\\return    --\\\\\\";
//    body = "\\return(100);";
//    result = Parser::ExpandMacro(macro, body);
//    ASSERT_STREQ("\\return(100);", result.c_str());
//
//    macro = "\\return(...)--\\$*--";
//    body = "\\return(100);";
//    result = Parser::ExpandMacro(macro, body);
//    ASSERT_STREQ("--100--;", result.c_str());
//
//}

//TEST_F(ParserTest, MacroDSL) {
//
//    Parser::MacrosStore macros;
//    std::string dsl = ""
//            "\\if(cond)\\\\      [$cond]-->\\\\"
//            "\\elseif(cond)\\\\ ,[$cond]-->\\\\"
//            "\\else\\\\         ,[_]-->\\\\"
//            ""
//            "\\while(cond)\\\\  [$cond]<->\\\\"
//            "\\dowhile(cond)\\\\<->[$cond]\\\\"
//            "\\return\\         --\\"
//            "\\return(...)\\    --$...--\\"
//            "\\dowhile(cond)\\\\\\"
//            "\\\\\\"
//            "";
//
//    while(Parser::ExtractMacros(dsl, macros))
//        ;
//    ASSERT_EQ(7, macros.size());
//
//
//}

TEST_F(ParserTest, Docs) {
    ASSERT_TRUE(Parse("/** doc */ { }"));
    ASSERT_TRUE(Parse("/// \n{ }"));
    ASSERT_TRUE(Parse("{ ///< doc\n }"));

    ASSERT_TRUE(Parse("/** doc\n\n */\n value := { };"));
    ASSERT_TRUE(Parse("/// doc1 \n/// doc2\n value := { };"));
    ASSERT_TRUE(Parse("value := 100; ///< doc"));
    ASSERT_TRUE(Parse("value := 100; ///< doc\n"));
}

TEST_F(ParserTest, HelloWorld) {
    ASSERT_TRUE(Parse("hello(str=\"\") := { printf(format:FmtChar, ...):Int32 := Pointer('printf'); printf('%s', $1); $str;};"));
    //    ASSERT_STREQ("!!!!!!!!!!!!!!", ast->toString().c_str());
}

TEST_F(ParserTest, SysEnv) {
    ASSERT_TRUE(Parse("__NLC_VER__"));
    ASSERT_STREQ(std::to_string(VERSION).c_str(), ast->toString().c_str());

    ASSERT_TRUE(Parse("__LINE__"));
    ASSERT_STREQ(std::to_string(1).c_str(), ast->toString().c_str());
    ASSERT_TRUE(Parse("\n__LINE__"));
    ASSERT_STREQ(std::to_string(2).c_str(), ast->toString().c_str());
    ASSERT_TRUE(Parse("\n\n__LINE__"));
    ASSERT_STREQ(std::to_string(3).c_str(), ast->toString().c_str());

    ASSERT_TRUE(Parse("__COUNTER__"));
    ASSERT_STREQ("0", ast->toString().c_str());
    ASSERT_TRUE(Parse("__COUNTER__"));
    ASSERT_STREQ("1", ast->toString().c_str());
    ASSERT_TRUE(Parse("__COUNTER__"));
    ASSERT_STREQ("2", ast->toString().c_str());

    ASSERT_TRUE(Parse("__FILE__"));
    ASSERT_STREQ("\"File name undefined!!!\"", ast->toString().c_str());

    ASSERT_TRUE(Parse("__DATE__"));
    ASSERT_EQ(27, ast->toString().size()) << ast->toString();

    ASSERT_TRUE(Parse("__TIMESTAMP__"));
    ASSERT_EQ(26, ast->toString().size()) << ast->toString();

    ASSERT_TRUE(Parse("__MD5__"));
    ASSERT_TRUE(ast->toString().size() > 30) << ast->toString();

    ASSERT_TRUE(Parse("__SOURCE_GIT__"));
    ASSERT_STREQ("\"" VERSION_GIT_SOURCE "\"", ast->toString().c_str());

    ASSERT_TRUE(Parse("__DATE_BUILD__"));
    ASSERT_STREQ("\"" VERSION_DATE_BUILD_STR "\"", ast->toString().c_str());

    ASSERT_TRUE(Parse("__SOURCE_BUILD__"));
    ASSERT_STREQ("\"" VERSION_SOURCE_FULL_ID "\"", ast->toString().c_str());
}

TEST_F(ParserTest, Class) {
    EXPECT_TRUE(Parse(":Name := :Class(){};"));
    EXPECT_TRUE(Parse(":Name := :_(){ filed1 = 1; };"));
    EXPECT_TRUE(Parse(":Name := ns::Class(){ filed1 := 1; filed1 ::= 2; };"));
    EXPECT_TRUE(Parse(":Name := ::(){ func = {};};"));
    EXPECT_TRUE(Parse(":Name := :Class(){ func1 := {}; func2 ::= {};};"));
    EXPECT_TRUE(Parse("Name := Class(){ func() = {};};"));
    EXPECT_TRUE(Parse("::Name() := ::Func(){ func1() := {}; func2(arg) ::= {};};"));
    EXPECT_TRUE(Parse(":Name := ::Class(){ func() = {};};"));
    EXPECT_TRUE(Parse(":Name := :Class(args) { func1() := {}; func2(arg) ::= {};};"));
}

TEST_F(ParserTest, Module) {
    ASSERT_TRUE(Parse("@module(func)"));
    ASSERT_TRUE(Parse("@dir.module(func)"));
    ASSERT_TRUE(Parse("@dir.dir.module(func)"));

    //    ASSERT_TRUE(Parse("@module (*)"));
    //    ASSERT_TRUE(Parse("@dir.module (*)"));
    //    ASSERT_TRUE(Parse("@dir.dir.module (*)"));
    //
    //    ASSERT_TRUE(Parse("@module (func, func2)"));
    //    ASSERT_TRUE(Parse("@dir.module (func, *)"));
    //    ASSERT_TRUE(Parse("@dir.dir.module (func, _)"));
    //
    //    ASSERT_TRUE(Parse("@module (func, ::func2)"));
    //    ASSERT_TRUE(Parse("@dir.module (ns::func, *)"));
    //    ASSERT_TRUE(Parse("@dir.dir.module (::ns::func, _)"));

    ASSERT_TRUE(Parse("@module (name=func, name=func2, name=::func3)"));
    ASSERT_TRUE(Parse("@dir.module (name=ns::func, name='')"));
    ASSERT_TRUE(Parse("@dir.dir.module (name=::ns::func, name=_)"));

    ASSERT_TRUE(Parse("@module::var"));
    ASSERT_TRUE(Parse("@module::ns::var"));
    ASSERT_TRUE(Parse("@module::ns::func()"));

    ASSERT_TRUE(Parse("@dir.module::var"));
    ASSERT_TRUE(Parse("@dir.dir.module::ns::var"));
    ASSERT_TRUE(Parse("@dir.dir.dir.module::ns::func()"));

    //    ASSERT_TRUE(Parse("@module (name=func, name=::name::*)"));
    //    ASSERT_TRUE(Parse("@dir.module (name=ns::name::*, name=*)"));
}

TEST_F(ParserTest, DISABLED_Convert) {
    std::vector<const char *> list = {
        "brother(human!, human!)?;",
        "func(arg1, arg2 = 5) :- { ($arg1 < $2) -> {% return $arg1; %}, [_] --> {% return *$1 * *$2; %}; };",
        "func_sum(arg1, arg2) :- {$arg1 + $arg2;};",
    };
    Parser parser;
    for (size_t i = 0; i < list.size(); i++) {
        ASSERT_NO_THROW(
                parser.Parse(list[i], nullptr);
                ) << "FROM: " << list[i];
        std::string to_str = parser.GetAst()->toString() + ";";
        ASSERT_NO_THROW(
                parser.Parse(to_str, nullptr);
                ) << "CONVERT: " << to_str;
    }
}

#endif // UNITTEST