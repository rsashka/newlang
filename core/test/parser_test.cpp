#ifdef UNITTEST

#include "core/pch.h"

#include <core/warning_push.h>
#include <gtest/gtest.h>
#include <core/warning_pop.h>

#include <core/parser.h>
#include <core/term.h>
#include "core/newlang.h"
#include "core/nlc.h"

using namespace newlang;

class ParserTest : public ::testing::Test {
protected:

    TermPtr Parse(std::string str) {
        ast = Parser::ParseString(str);
        return ast;
    }

    int Count(TermID token_id) {
        int result = 0;
        for (size_t c = 0; c < ast->size(); c++) {
            if((*ast)[c]->m_id == token_id) {
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

TEST_F(ParserTest, LiteralFraction) {
    ASSERT_TRUE(Parse("1\\1;"));
    ASSERT_EQ(TermID::FRACTION, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("1\\1", ast->toString().c_str());

    ASSERT_TRUE(Parse("100\\100;"));
    ASSERT_EQ(TermID::FRACTION, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("100\\100", ast->toString().c_str());

    ASSERT_TRUE(Parse("123456789123456789123456789\\123456789123456789123456789;"));
    ASSERT_EQ(TermID::FRACTION, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("123456789123456789123456789\\123456789123456789123456789", ast->toString().c_str());
}

TEST_F(ParserTest, DISABLED_LiteralCurrency) {
    ASSERT_TRUE(Parse("`1;"));
    ASSERT_EQ(TermID::CURRENCY, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("`1", ast->toString().c_str());

    ASSERT_TRUE(Parse("1`23;"));
    ASSERT_EQ(TermID::CURRENCY, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("`123", ast->toString().c_str());

    ASSERT_TRUE(Parse("1`2`3456.123;"));
    ASSERT_EQ(TermID::CURRENCY, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("123`456.123", ast->toString().c_str());
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
    ASSERT_EQ(TermID::CALL, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
}

TEST_F(ParserTest, TermName) {
    ASSERT_TRUE(Parse("term.filed();"));
    ASSERT_EQ(TermID::TERM, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_TRUE(ast->m_name.empty());
}

TEST_F(ParserTest, Tensor1) {
    ASSERT_TRUE(Parse("[,]:Char"));
    ASSERT_EQ(TermID::TENSOR, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("[,]:Char", ast->toString().c_str());

    ASSERT_TRUE(Parse("term[1];"));
    ASSERT_EQ(TermID::TERM, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_TRUE(ast->Right());

    ASSERT_EQ(TermID::INDEX, ast->Right()->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_EQ(1, ast->Right()->size());
    ASSERT_STREQ("1", (*ast->Right())[0]->getText().c_str());

    ASSERT_TRUE(Parse("term[1..2];"));
    ASSERT_EQ(TermID::TERM, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term[1..2]", ast->toString().c_str());

    //    ASSERT_TRUE(ast->Right());
    //    ASSERT_EQ(TermID::INDEX, ast->Right()->getTermID()) << EnumStr(ast->getTermID());
    //    ASSERT_EQ(2, ast->Right()->getItemCount());
    //    ASSERT_STREQ("1", (*ast->Right())[0]->getText().c_str());
    //    ASSERT_STREQ("2", (*ast->Right())[1]->getText().c_str());
}

TEST_F(ParserTest, Tensor2) {
    ASSERT_TRUE(Parse("term[1, 2];"));
    ASSERT_EQ(TermID::TERM, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_TRUE(ast->Right());

    ASSERT_EQ(TermID::INDEX, ast->Right()->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_EQ(2, ast->Right()->size());
    ASSERT_STREQ("1", (*ast->Right())[0]->getText().c_str());
    ASSERT_STREQ("2", (*ast->Right())[1]->getText().c_str());

    ASSERT_TRUE(Parse("term[1, 1..2, 3];"));

    ASSERT_EQ(TermID::INDEX, ast->Right()->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_EQ(3, ast->Right()->size());
    ASSERT_STREQ("1", (*ast->Right())[0]->getText().c_str());
    ASSERT_STREQ("1..2", (*ast->Right())[1]->toString().c_str());
    ASSERT_STREQ("3", (*ast->Right())[2]->getText().c_str());
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

    ASSERT_TRUE(Parse(":Int[3]( \"str\" ) "));
    ASSERT_STREQ(":Int[3](\"str\")", ast->toString().c_str());

    ASSERT_TRUE(Parse(":Int[2,2](1,2,3,4);"));
    ASSERT_STREQ(":Int[2,2](1, 2, 3, 4)", ast->toString().c_str());

    ASSERT_TRUE(Parse(":Int[2,2]( 0,   ...    )"));
    ASSERT_STREQ(":Int[2,2](0, ...)", ast->toString().c_str());

    ASSERT_TRUE(Parse(":Int( ... ...  dict )"));
    ASSERT_STREQ(":Int(... ...dict)", ast->toString().c_str());
    
    ASSERT_TRUE(Parse(":Int( ... dict )"));
    ASSERT_STREQ(":Int(...dict)", ast->toString().c_str());

    ASSERT_TRUE(Parse(":Int[2,2](   ...   rand()  ...   )"));
    ASSERT_STREQ(":Int[2,2](...rand()...)", ast->toString().c_str());
    
    ASSERT_TRUE(Parse(":type[10]( 1,     2,  ...    rand()   ... )"));
    ASSERT_STREQ(":type[10](1, 2, ...rand()...)", ast->toString().c_str());

    ASSERT_TRUE(Parse(":range( 0..100  )"));
    ASSERT_STREQ(":range(0..100)", ast->toString().c_str());

    ASSERT_TRUE(Parse(":range(  0 .. 100 .. 0.1 )"));
    ASSERT_STREQ(":range(0..100..0.1)", ast->toString().c_str());
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
    ASSERT_STREQ("Bool", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("1;"));
    ASSERT_STREQ("1", ast->toString().c_str());
    ASSERT_STREQ("Bool", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("2;"));
    ASSERT_STREQ("2", ast->toString().c_str());
    ASSERT_STREQ("Char", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("2_2;"));
    ASSERT_STREQ("2_2", ast->toString().c_str());
    ASSERT_STREQ("Char", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("-1;"));
    ASSERT_STREQ("-1", ast->toString().c_str());
    ASSERT_STREQ("Char", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("256;"));
    ASSERT_STREQ("256", ast->toString().c_str());
    ASSERT_STREQ("Short", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("10_000;"));
    ASSERT_STREQ("10_000", ast->toString().c_str());
    ASSERT_STREQ("Short", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("100_000;"));
    ASSERT_STREQ("100_000", ast->toString().c_str());
    ASSERT_STREQ("Int", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("0.0;"));
    ASSERT_STREQ("0.0", ast->toString().c_str());
    ASSERT_STREQ("Float", ast->m_type_name.c_str());



    ASSERT_TRUE(Parse("0:Bool;"));
    ASSERT_STREQ("0:Bool", ast->toString().c_str());
    ASSERT_STREQ(":Bool", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("0:Int;"));
    ASSERT_STREQ("0:Int", ast->toString().c_str());
    ASSERT_STREQ(":Int", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("0:Long;"));
    ASSERT_STREQ("0:Long", ast->toString().c_str());
    ASSERT_STREQ(":Long", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("0:Float;"));
    ASSERT_STREQ("0:Float", ast->toString().c_str());
    ASSERT_STREQ(":Float", ast->m_type_name.c_str());

    //    ASSERT_TRUE(Parse("0  :  Half"));
    //    ASSERT_STREQ("0:Half", ast->toString().c_str());
    //    ASSERT_STREQ(":Half", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("0:Double;"));
    ASSERT_STREQ("0:Double", ast->toString().c_str());
    ASSERT_STREQ(":Double", ast->m_type_name.c_str());


    ASSERT_TRUE(Parse("1:Bool;"));
    ASSERT_STREQ("1:Bool", ast->toString().c_str());
    ASSERT_STREQ(":Bool", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("1:Char;"));
    ASSERT_STREQ("1:Char", ast->toString().c_str());
    ASSERT_STREQ(":Char", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("1:Char;"));
    ASSERT_STREQ("1:Char", ast->toString().c_str());
    ASSERT_STREQ(":Char", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("1:Double;"));
    ASSERT_STREQ("1:Double", ast->toString().c_str());
    ASSERT_STREQ(":Double", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("2:Short;"));
    ASSERT_STREQ("2:Short", ast->toString().c_str());
    ASSERT_STREQ(":Short", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("2_2:Int;"));
    ASSERT_STREQ("2_2:Int", ast->toString().c_str());
    ASSERT_STREQ(":Int", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("-1:Char;"));
    ASSERT_STREQ("-1:Char", ast->toString().c_str());
    ASSERT_STREQ(":Char", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("-1 :Long;"));
    ASSERT_STREQ("-1:Long", ast->toString().c_str());
    ASSERT_STREQ(":Long", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("256 :Short;"));
    ASSERT_STREQ("256:Short", ast->toString().c_str());
    ASSERT_STREQ(":Short", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("10_000    :Long;"));
    ASSERT_STREQ("10_000:Long", ast->toString().c_str());
    ASSERT_STREQ(":Long", ast->m_type_name.c_str());

    //    ASSERT_THROW(Parse("10__000"), parser_exception);

    ASSERT_TRUE(Parse("100_000:  Int;"));
    ASSERT_STREQ("100_000:Int", ast->toString().c_str());
    ASSERT_STREQ(":Int", ast->m_type_name.c_str());

    ASSERT_TRUE(Parse("1.0  :  Float;"));
    ASSERT_STREQ("1.0:Float", ast->toString().c_str());
    ASSERT_STREQ(":Float", ast->m_type_name.c_str());


    ASSERT_TRUE(Parse("-0.0   :Double;"));
    ASSERT_STREQ("-0.0:Double", ast->toString().c_str());
    ASSERT_STREQ(":Double", ast->m_type_name.c_str());

    ASSERT_THROW(Parse("2:Bool;"), parser_exception);
    ASSERT_THROW(Parse("-1:Bool;"), parser_exception);
    //        ASSERT_THROW(Parse("-1:Char"), parser_exception);
    ASSERT_THROW(Parse("300:Char;"), parser_exception);
    ASSERT_THROW(Parse("100000:Short;"), parser_exception);
    ASSERT_THROW(Parse("0.0:Bool;"), parser_exception);
    ASSERT_THROW(Parse("0.0:Char;"), parser_exception);
    ASSERT_THROW(Parse("0.0:Int;"), parser_exception);
    ASSERT_THROW(Parse("0.0:Long;"), parser_exception);
}

TEST_F(ParserTest, TensorType) {
    ASSERT_TRUE(Parse("term:Char[1,2] := [ [1,2,],[3,4,],];"));
    ASSERT_STREQ("term:Char[1,2] := [[1, 2,], [3, 4,],];", ast->toString().c_str());

    ASSERT_TRUE(Parse("term[..., 3] := 0;"));
    ASSERT_STREQ("term[..., 3] := 0;", ast->toString().c_str());

    ASSERT_TRUE(Parse("term []= [2, 3,]:Int;"));
    ASSERT_STREQ("term []= [2, 3,]:Int;", ast->toString().c_str());

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

    ASSERT_TRUE(Parse("(1, arg=2, '',):Class(value)"));
    ASSERT_STREQ("(1, arg=2, '',):Class(value)", ast->toString().c_str());
}

TEST_F(ParserTest, TermNoArg) {
    ASSERT_TRUE(Parse("term();"));
    ASSERT_EQ(TermID::CALL, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());

    ASSERT_TRUE(Parse("term();"));
    ASSERT_EQ(TermID::CALL, ast->getTermID()) << newlang::toString(ast->getTermID());
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

TEST_F(ParserTest, FuncNoArg) {
    ASSERT_TRUE(Parse("@term();"));
    ASSERT_EQ(TermID::CALL, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("@term", ast->m_text.c_str());
}

TEST_F(ParserTest, TermNoArgSpace) {
    ASSERT_TRUE(Parse("term(  );"));
    ASSERT_EQ(TermID::CALL, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_EQ(0, ast->size());
}

TEST_F(ParserTest, TermArgTerm) {
    ASSERT_TRUE(Parse("term(arg);"));
    ASSERT_EQ(TermID::CALL, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_EQ(1, ast->size());
    ASSERT_STREQ("arg", (*ast)[0]->m_text.c_str());
    ASSERT_FALSE((*ast)[0]->m_is_ref);


    ASSERT_TRUE(Parse("term(name=value);"));
    ASSERT_EQ(TermID::CALL, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());

    ASSERT_EQ(1, ast->size());
    ASSERT_STREQ("name", (*ast)[0]->getName().c_str());
    ASSERT_STREQ("value", (*ast)[0]->getText().c_str());
    ASSERT_FALSE((*ast)[0]->m_is_ref);
}

TEST_F(ParserTest, TermArgTermRef) {
    ASSERT_TRUE(Parse("term(&arg);"));
    ASSERT_EQ(TermID::CALL, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_EQ(1, ast->size());
    ASSERT_STREQ("arg", (*ast)[0]->m_text.c_str());
    ASSERT_TRUE((*ast)[0]->m_is_ref);


    ASSERT_TRUE(Parse("term(name=&value);"));
    ASSERT_EQ(TermID::CALL, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());

    ASSERT_EQ(1, ast->size());
    ASSERT_STREQ("name", (*ast)[0]->getName().c_str());
    ASSERT_STREQ("value", (*ast)[0]->getText().c_str());
    ASSERT_TRUE((*ast)[0]->m_is_ref);
}

TEST_F(ParserTest, TermArgTermSpace) {
    ASSERT_TRUE(Parse("   \n  \t term(  \n  arg  \n  )    ;   \n  "));
    ASSERT_EQ(TermID::CALL, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_EQ(1, ast->size());
    ASSERT_STREQ("arg", (*ast)[0]->m_text.c_str());
}

TEST_F(ParserTest, TermArgs1) {
    ASSERT_TRUE(Parse("term(arg1);"));
    ASSERT_EQ(TermID::CALL, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_EQ(1, ast->size());
    ASSERT_STREQ("arg1", (*ast)[0]->m_text.c_str());
}

TEST_F(ParserTest, TermArgs2) {
    ASSERT_TRUE(Parse("term(arg1,arg2);"));
    ASSERT_EQ(TermID::CALL, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_EQ(2, ast->size());
    ASSERT_STREQ("arg1", (*ast)[0]->getText().c_str());
    ASSERT_STREQ("arg2", (*ast)[1]->getText().c_str());
    ASSERT_FALSE((*ast)[0]->m_is_ref);
    ASSERT_FALSE((*ast)[1]->m_is_ref);
}

TEST_F(ParserTest, TermArgsRef) {
    ASSERT_TRUE(Parse("term(&arg1,&arg2);"));
    ASSERT_EQ(TermID::CALL, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_EQ(2, ast->size());
    ASSERT_STREQ("arg1", (*ast)[0]->getText().c_str());
    ASSERT_STREQ("arg2", (*ast)[1]->getText().c_str());
    ASSERT_TRUE((*ast)[0]->m_is_ref);
    ASSERT_TRUE((*ast)[1]->m_is_ref);
}

TEST_F(ParserTest, TermArgMixed) {
    ASSERT_TRUE(Parse("term(\narg1,\n arg2\n = \narg3 \n);"));
    ASSERT_EQ(TermID::CALL, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());

    ASSERT_EQ(2, ast->size());
    ASSERT_STREQ("arg1", (*ast)[0]->getText().c_str());
    ASSERT_STREQ("arg2", (*ast)[1]->getName().c_str());
    ASSERT_STREQ("arg3", (*ast)[1]->getText().c_str());
}

TEST_F(ParserTest, ArgsType) {
    ASSERT_TRUE(Parse("term(bool:Bool=term(100), int:Int=100, long:Long=@term()):Double:={long;};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term(bool:Bool=term(100), int:Int=100, long:Long=@term()):Double := {long;};", ast->toString().c_str());
}

TEST_F(ParserTest, TermCall) {
    ASSERT_TRUE(Parse("var2 := min(200, var, 400);"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ(":=", ast->m_text.c_str());


    TermPtr right = ast->Right();
    ASSERT_TRUE(right);
    ASSERT_EQ(3, right->size());
    ASSERT_STREQ("200", (*right)[0]->getText().c_str());
    ASSERT_FALSE((*right)[0]->m_left);
    ASSERT_FALSE((*right)[0]->m_right);
    ASSERT_STREQ("var", (*right)[1]->getText().c_str());
    ASSERT_FALSE((*right)[1]->m_left);
    ASSERT_FALSE((*right)[1]->m_right);
    ASSERT_STREQ("400", (*right)[2]->getText().c_str());
    ASSERT_FALSE((*right)[2]->m_left);
    ASSERT_FALSE((*right)[2]->m_right);
}

TEST_F(ParserTest, TermCollection) {
    ASSERT_TRUE(Parse("term([2,], [arg1,arg2,]);"));
    ASSERT_EQ(TermID::CALL, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_EQ(2, ast->size());
    ASSERT_EQ(TermID::TENSOR, (*ast)[0]->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_EQ(TermID::TENSOR, (*ast)[1]->getTermID()) << newlang::toString(ast->getTermID());
}

TEST_F(ParserTest, TermCollection2) {
    ASSERT_TRUE(Parse("term((,), name=[arg1,arg2,]);"));
    ASSERT_EQ(TermID::CALL, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_EQ(2, ast->size());
    ASSERT_EQ(TermID::DICT, (*ast)[0]->getTermID()) << newlang::toString((*ast)[0]->getTermID());
    ASSERT_EQ(TermID::TENSOR, (*ast)[1]->getTermID()) << newlang::toString((*ast)[1]->getTermID());
}

TEST_F(ParserTest, ArgMixedFail) {
    //    EXPECT_THROW(
    //            Parse("term(arg2=arg3, arg1);"), std::runtime_error
    //            );
    EXPECT_THROW(
            Parse("term(arg1,arg2=arg3,,);"), std::runtime_error
            );
    EXPECT_THROW(
            Parse("term(,);"), std::runtime_error
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
    ASSERT_STREQ("arg", (*arg)[0]->getText().c_str());

#pragma GCC warning "ITERATOR"
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
            Parse("term2(arg=value)!;?"), std::runtime_error
            );


    ASSERT_TRUE(Parse("term2(arg=value?);"));
    ASSERT_EQ(1, ast->size());
    ASSERT_STREQ("term2", ast->getText().c_str());
    ASSERT_EQ(1, ast->size());
    ASSERT_STREQ("arg", ast->name(0).c_str());
    ASSERT_STREQ("?", (*ast)[0]->getText().c_str());
    ASSERT_TRUE((*ast)[0]->Left());
    ASSERT_STREQ("value", (*ast)[0]->Left()->getText().c_str());


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
}

TEST_F(ParserTest, DISABLED_MathPioritet) {
    ASSERT_TRUE(Parse("test := 123+456*789;"));
    ASSERT_STREQ(":=", ast->m_text.c_str());
    ASSERT_TRUE(ast->Left());
    ASSERT_TRUE(ast->Right());
    ASSERT_STREQ("test", ast->Left()->m_text.c_str());
    TermPtr op = ast->Right();
    ASSERT_TRUE(op);
    ASSERT_STREQ("*", op->m_text.c_str());
    ASSERT_TRUE(op->Right());
    ASSERT_STREQ("789", op->Right()->m_text.c_str());
    ASSERT_TRUE(op->Left());
    ASSERT_STREQ("+", op->Left()->m_text.c_str());
    TermPtr op2 = op->Right();
    ASSERT_TRUE(op2->Left());
    ASSERT_TRUE(op2->Right());
    ASSERT_STREQ("123", op2->Left()->m_text.c_str());
    ASSERT_STREQ("456", op2->Right()->m_text.c_str());
}

TEST_F(ParserTest, CodeSimple) {
    ASSERT_TRUE(Parse("%{code+code%};"));
    ASSERT_STREQ("code+code", ast->m_text.c_str());
}

TEST_F(ParserTest, CodeSimple2) {
    ASSERT_TRUE(Parse("%{code+code}%;"));
    ASSERT_STREQ("code+code", ast->m_text.c_str());
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
    ASSERT_TRUE(Parse(":class  :=    :Class  ;"));
    ASSERT_STREQ(":class := :Class;", ast->toString().c_str());
}

TEST_F(ParserTest, DISABLED_AssignClass2) {
    ASSERT_TRUE(Parse(":class  :=  class1(), class2(),   test(arg) ;"));
    ASSERT_STREQ(":class := class1(), class2(), test(arg);", ast->toString().c_str());
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

    ASSERT_EQ(TermID::TERM, ast->Left()->getTermID());
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
    ASSERT_STREQ("0", (*ast->Left()->Right())[0]->m_text.c_str());


}

TEST_F(ParserTest, DISABLED_ArrayAssign2) {
    ASSERT_TRUE(Parse("term[1][1..3] =  123;"));
    ASSERT_EQ(TermID::ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_TRUE(ast->Left());
    ASSERT_TRUE(ast->Right());

    ASSERT_EQ(TermID::TERM, ast->Left()->getTermID());
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
    ASSERT_EQ(TermID::CALL, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("term", ast->m_text.c_str());
    ASSERT_EQ(1, ast->size());
    ASSERT_STREQ("[1,]", (*ast)[0]->toString().c_str());
}

TEST_F(ParserTest, LogicEq) {
    ASSERT_TRUE(Parse("var := 1==2;"));
    ASSERT_STREQ("var := 1==2;", ast->toString().c_str());
}

TEST_F(ParserTest, LogicNe) {
    ASSERT_TRUE(Parse("var := 1!=2;"));
    ASSERT_STREQ("var := 1!=2;", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionSimple) {
    ASSERT_TRUE(Parse("func() := {%{%}};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("func() := {%{%}};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionSimpleTwo) {
    ASSERT_TRUE(Parse("func() := {%{ %}%{ %}};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("func() := {%{ %}%{ %}};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionSimple2) {
    ASSERT_TRUE(Parse("func(arg)  :=  {$0:=0;};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("func(arg) := {$0 := 0;};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionSimple3) {
    ASSERT_TRUE(Parse("func(arg)  :=  {%{  %} %{ %} %{  %}; $99:=0;};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("func(arg) := {%{  %}%{ %}%{  %} $99 := 0;};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionSimple4) {
    ASSERT_TRUE(Parse("func(arg) := {$33:=0;};"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("func(arg) := {$33 := 0;};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionSimple5) {
    ASSERT_TRUE(Parse("print(str=\"\") :={ %{ printf(\"%s\", static_cast<char *>($str)); %} };"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("print(str=\"\") := {%{ printf(\"%s\", static_cast<char *>($str)); %}};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionTrans0) {
    ASSERT_TRUE(Parse("func(arg1, arg2=123) &&= $arg1 == $arg2, $11 == $arg1;"));
    ASSERT_STREQ("func(arg1, arg2=123) &&= $arg1==$arg2, $11==$arg1", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionTrans1) {
    ASSERT_TRUE(Parse("human::eq(test=human) &&= $0==$test;"));
    ASSERT_STREQ("human::eq(test=human) &&= $0==$test;", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionTrans2) {
    ASSERT_TRUE(Parse("func(arg1, arg2 = 5) :- %{ return $arg1; %};"));
    ASSERT_STREQ("func(arg1, arg2=5) :- %{ return $arg1; %}", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionTrans3) {
    ASSERT_TRUE(Parse("func(arg1, arg2 = 5) :- { %{ return $arg1; %} };"));
    ASSERT_STREQ("func(arg1, arg2=5) :- {%{ return $arg1; %}};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionTrans4) {
    ASSERT_TRUE(Parse("func(arg1, arg2 = 5) :- { [$arg1 < $arg2] -> {%{ return $arg1; %}}; };"));
    ASSERT_STREQ("func(arg1, arg2=5) :- {[$arg1<$arg2]->{%{ return $arg1; %}};};", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionTrans5) {
    ASSERT_TRUE(Parse("func(arg1, arg2 = 5) :- { [$arg1 < $arg2] -> {%{ return $arg1; %}}, [_] -> {%{ return $arg2; %}}; };"));
    ASSERT_STREQ("func(arg1, arg2=5) :- {[$arg1<$arg2]->{%{ return $arg1; %}},\n [_]->{%{ return $arg2; %}};};", ast->toString().c_str());
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
    ASSERT_EQ(TermID::CALL, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("русс(10, 20)", ast->toString().c_str());
}

TEST_F(ParserTest, FunctionRussian4) {
    ASSERT_TRUE(Parse("мин(10,20);"));
    ASSERT_EQ(TermID::CALL, ast->getTermID()) << newlang::toString(ast->getTermID());
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
    ASSERT_THROW(Parse("мин(... ...) := {$0:=0;};"), parser_exception);
//    ASSERT_THROW(Parse("мин(..., arg) := {$0:=0;};"), parser_exception);
    ASSERT_THROW(Parse("мин(arg ...) := {$0:=0;};"), parser_exception);
    ASSERT_THROW(Parse("мин(arg=1 ..., arg) := {$0:=0;};"), parser_exception);
    ASSERT_THROW(Parse("мин(arg=1, arg ...) := {$0:=0;};"), parser_exception);
    ASSERT_THROW(Parse("мин(arg=1 ...) := {$0:=0;};"), parser_exception);
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
//TEST_F(ParserTest, Fraction) {
//    ASSERT_TRUE(Parse("1\\1"));
//    ASSERT_EQ(TermID::FRACTION, ast->getTermID()) << newlang::toString(ast->getTermID());
//    ASSERT_STREQ("1\\1;", ast->toString().c_str());
//}
//
//TEST_F(ParserTest, Fraction2) {
//    ASSERT_TRUE(Parse("1\\-20"));
//    ASSERT_EQ(TermID::FRACTION, ast->getTermID()) << newlang::toString(ast->getTermID());
//    ASSERT_STREQ("1\\-20;", ast->toString().c_str());
//}
//
//TEST_F(ParserTest, Fraction3) {
//    ASSERT_TRUE(Parse("-3\\+11"));
//    ASSERT_EQ(TermID::FRACTION, ast->getTermID()) << newlang::toString(ast->getTermID());
//    ASSERT_STREQ("-3\\+11;", ast->toString().c_str());
//}
//
//TEST_F(ParserTest, ArrayAdd9) {
//    ASSERT_TRUE(Parse("$name  :=  term2"));
//    ASSERT_EQ(TermID::CREATE, ast->getTermID()) << newlang::toString(ast->getTermID());
//    ASSERT_STREQ("$name := term2;", ast->toString().c_str());
//}

TEST_F(ParserTest, DISABLED_Ellipsis2) {
    ASSERT_TRUE(Parse("@name  :=  term2(... arg);"));
    ASSERT_EQ(TermID::CREATE, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("@name := term2(... arg);", ast->toString().c_str());
}

TEST_F(ParserTest, Func1) {
    ASSERT_TRUE(Parse("func_arg(arg1 :Char, arg2) :Char := { $arg1+$arg2; };"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("func_arg(arg1:Char, arg2):Char := {$arg1+$arg2;};", ast->toString().c_str());
}

TEST_F(ParserTest, Func2) {
    ASSERT_TRUE(Parse("func_arg(arg1:&Char, &arg2) :&Char := { $arg1+$arg2; };"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("func_arg(arg1:&Char, &arg2):&Char := {$arg1+$arg2;};", ast->toString().c_str());
}

TEST_F(ParserTest, Func3) {
    ASSERT_TRUE(Parse("$res:Char ::= func_arg(100, 100);"));
    ASSERT_EQ(TermID::CREATE, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("$res:Char ::= func_arg(100, 100);", ast->toString().c_str());
}

TEST_F(ParserTest, Func4) {
    ASSERT_TRUE(Parse("res() := func_arg(100, 100); res() := func_arg(100, 100); res() := func_arg(100, 100);"));
}

TEST_F(ParserTest, Comment) {
    ASSERT_TRUE(Parse("#!line1\n"));
    //ASSERT_EQ(TermID::COMMENT, ast->getTermID()) << EnumStr(ast->getTermID());
}

TEST_F(ParserTest, Comment2) {
    ASSERT_TRUE(Parse("#!line2\n#line1\n \n\n/* \n \n */ \n"));
    //    ASSERT_EQ(TermID::BLOCK, ast->getTermID()) << EnumStr(ast->getTermID());
    //    ASSERT_EQ(3, ast->m_block.size());
    //    ASSERT_EQ(TermID::COMMENT, ast->m_block[0]->getTermID()) << EnumStr(ast->getTermID());
    //    ASSERT_EQ(TermID::COMMENT, ast->m_block[1]->getTermID()) << EnumStr(ast->getTermID());
    //    ASSERT_EQ(TermID::COMMENT, ast->m_block[2]->getTermID()) << EnumStr(ast->getTermID());
}

TEST_F(ParserTest, Comment3) {
    const char *str = "print(str=\"\") := { %{  %} };\n"
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
            "print1(str=\"\") := { %{  %} };\n"
            "print2(str=\"\") := { %{  %} };\n"
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
            "print1(str=\"\") := { %{  %} };\n"
            "print2(str=\"\") := { %{  %} };\n\n"
            "print3(str=\"\") := { %{  %} };\n\n\n"
            "# @print(\"Привет, мир!\\n\");\n";
    ASSERT_TRUE(Parse(str));
    ASSERT_EQ(TermID::BLOCK, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_EQ(4, ast->m_block.size());
    ASSERT_EQ(TermID::CALL, ast->m_block[0]->getTermID()) << newlang::toString(ast->getTermID());

    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->m_block[1]->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->m_block[2]->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->m_block[3]->getTermID()) << newlang::toString(ast->getTermID());
}

TEST_F(ParserTest, DISABLED_Comment6) {
    const char *str = "term();\n"
            "print1(str=\"\") ::= term();\n"
            "print2(str=\"\") ::= term();\n\n"
            "print3(str=\"\") ::= term();\n\n\n"
            "# @print(\"Привет, мир!\\n\");\n";
    "# @print(\"Привет, мир!\\n\");\n";
    ASSERT_TRUE(Parse(str));
    ASSERT_EQ(TermID::BLOCK, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_EQ(4, ast->m_block.size());
    ASSERT_EQ(TermID::CALL, ast->m_block[0]->getTermID()) << newlang::toString(ast->getTermID());

    ASSERT_EQ(TermID::CREATE, ast->m_block[1]->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_EQ(TermID::CREATE, ast->m_block[2]->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_EQ(TermID::CREATE, ast->m_block[3]->getTermID()) << newlang::toString(ast->getTermID());
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
    ASSERT_TRUE(Parse("const  ::=   \"CONST\";"));
    ASSERT_EQ(TermID::CREATE, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("const ::= \"CONST\";", ast->toString().c_str());
}

TEST_F(ParserTest, Const3) {
    ASSERT_TRUE(Parse("const  :=   123;"));
    ASSERT_EQ(TermID::CREATE_OR_ASSIGN, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("const := 123;", ast->toString().c_str());
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
    ASSERT_NO_THROW(Parse("(){val;};"));
    ASSERT_NO_THROW(Parse("(){val;val;};"));

    ASSERT_NO_THROW(Parse("( ){val;;}; (){{fff;}};"));
    ASSERT_NO_THROW(Parse("( ){val;;;};  (){{fff;}};;"));
    ASSERT_NO_THROW(Parse("(){val;;;;val;(){(){fff;};};};;;;"));
    ASSERT_NO_THROW(Parse("(){{val;;;;val;;;(123, 333){(){{fff;}}};(  ){fff;} }};;;;"));

    ASSERT_NO_THROW(Parse("(){val();};"));
    ASSERT_NO_THROW(Parse("(){val();val();};"));
    ASSERT_NO_THROW(Parse("(){val();val();;};"));

    ASSERT_NO_THROW(Parse("(){val();};"));
    ASSERT_NO_THROW(Parse("(){val();;;};;;"));
    ASSERT_NO_THROW(Parse("(){val();;;val();;;};;;"));
    ASSERT_NO_THROW(Parse("(){val();;;val();;;};;;"));
}

TEST_F(ParserTest, Repeat) {
    ASSERT_TRUE(Parse("[val] -->> val;"));
    ASSERT_TRUE(Parse("val <<-- [val];;"));
    ASSERT_TRUE(Parse("[val] -->> {val;};"));
    ASSERT_TRUE(Parse("[val] -->> {val;};;"));
    ASSERT_TRUE(Parse("[val] -->> {val;};"));
    ASSERT_TRUE(Parse("[val] -->> {val;};;"));

    ASSERT_TRUE(Parse("val <<- [val()];"));
    ASSERT_TRUE(Parse("val <<- [val()];;"));
    ASSERT_TRUE(Parse("[val()] -->> {val();};"));
    ASSERT_TRUE(Parse("[val()] -->> {val();};;"));
    ASSERT_TRUE(Parse("[val()] -->> {val();};"));
    ASSERT_TRUE(Parse("[val()] -->> {val();};;"));

    ASSERT_TRUE(Parse("[val()] -->> {val()};val();"));
    ASSERT_TRUE(Parse("val <<-- [val()];val();"));
    ASSERT_TRUE(Parse("[val()] -->> {val();val();};"));
    ASSERT_TRUE(Parse("[val()] -->> {val();val();};;"));
    ASSERT_TRUE(Parse("[val()] -->> {val();val();};"));
    ASSERT_TRUE(Parse("[val()] -->> {val();val();};;"));
}

TEST_F(ParserTest, Operators) {
    ASSERT_TRUE(Parse("val + val;"));
    ASSERT_TRUE(Parse("val - val;"));
    ASSERT_TRUE(Parse("(){val * val;};"));
    ASSERT_TRUE(Parse("(){val / val;};"));
    ASSERT_TRUE(Parse("(){val ** val;};"));
    ASSERT_TRUE(Parse("(){val++val;;};;"));

    ASSERT_TRUE(Parse("val + val();"));
    ASSERT_TRUE(Parse("val - val();;"));
    ASSERT_TRUE(Parse("val * val();;"));
    ASSERT_TRUE(Parse("(){val / val();};"));
    ASSERT_TRUE(Parse("(){val ** val();};"));
    ASSERT_TRUE(Parse("(){val ++ val();};"));

    ASSERT_TRUE(Parse("val + val();"));
    ASSERT_TRUE(Parse("val - val();; val - val();"));
    ASSERT_TRUE(Parse("val * val();; val - val();;"));
    ASSERT_TRUE(Parse("(){val / val(); val - val()};;"));
    ASSERT_TRUE(Parse("(){val ** val(); val - val();};"));
    ASSERT_TRUE(Parse("(){val ++ val(); val - val();};;"));

    ASSERT_TRUE(Parse("(val + val());"));
    ASSERT_TRUE(Parse("(val - val()) + (val - val());"));
    ASSERT_TRUE(Parse("(val * val()) - (val - val());"));
    ASSERT_TRUE(Parse("(){val / val() + (val - val())};"));
    ASSERT_TRUE(Parse("(){val ** val() / (val - val());};"));
    ASSERT_TRUE(Parse("(){val ++ val() / (val - val());};"));
}

TEST_F(ParserTest, DISABLED_Repeat0) {
    ASSERT_TRUE(Parse("[human || $ttttt && 123 != test[0].field ] -->> if_1;"));
    ASSERT_STREQ("[human || $ttttt && 123 != test[0].field]-->>{if_1;};", ast->toString().c_str());
}

TEST_F(ParserTest, Repeat1) {
    ASSERT_TRUE(Parse("[test == human] -->> if_1;"));
    ASSERT_STREQ("[test==human]-->>if_1;", ast->toString().c_str());
}

TEST_F(ParserTest, Repeat2) {
    ASSERT_TRUE(Parse("[test != human] -->> {if_1;};"));
    ASSERT_STREQ("[test!=human]-->>{if_1;};", ast->toString().c_str());

    ASSERT_TRUE(Parse("{if_1;} <<-- [test != human];"));
    ASSERT_STREQ("{if_1;}<<--[test!=human];", ast->toString().c_str());
}

TEST_F(ParserTest, Repeat3) {
    ASSERT_TRUE(Parse("[test != human] -->> {if_1;if_2;then3;};"));
    ASSERT_STREQ("[test!=human]-->>{if_1; if_2; then3;};", ast->toString().c_str());

    ASSERT_TRUE(Parse("{if_1;if_2;then3;} <<-- [test != human];"));
    ASSERT_STREQ("{if_1; if_2; then3;}<<--[test!=human];", ast->toString().c_str());
}

TEST_F(ParserTest, Repeat4) {
    ASSERT_TRUE(Parse("[test()] -->> {if_1;if_2;then3;};"));
    ASSERT_STREQ("[test()]-->>{if_1; if_2; then3;};", ast->toString().c_str());

    ASSERT_TRUE(Parse("[test()] -->> {if_1;if_2;then3;};"));
    ASSERT_STREQ("[test()]-->>{if_1; if_2; then3;};", ast->toString().c_str());

    ASSERT_TRUE(Parse("{if_1;if_2;then3;} <<--  [test()];"));
    ASSERT_STREQ("{if_1; if_2; then3;}<<--[test()];", ast->toString().c_str());

    ASSERT_TRUE(Parse("{if_1;if_2;then3;} <<--  [test()];"));
    ASSERT_STREQ("{if_1; if_2; then3;}<<--[test()];", ast->toString().c_str());
}

TEST_F(ParserTest, Repeat5) {
    ASSERT_TRUE(Parse(" [ test! ]-->> {if_1;if_2;then3;};"));
    ASSERT_STREQ("[test!]-->>{if_1; if_2; then3;};", ast->toString().c_str());

    ASSERT_TRUE(Parse(" [test!] -->> {if_1;if_2;then3;};"));
    ASSERT_STREQ("[test!]-->>{if_1; if_2; then3;};", ast->toString().c_str());

    ASSERT_TRUE(Parse("{if_1;if_2;then3;}<<--[test!];"));
    ASSERT_STREQ("{if_1; if_2; then3;}<<--[test!];", ast->toString().c_str());

    ASSERT_TRUE(Parse("{if_1;if_2;then3;}<<--[test!];"));
    ASSERT_STREQ("{if_1; if_2; then3;}<<--[test!];", ast->toString().c_str());
}

TEST_F(ParserTest, Repeat6) {
    ASSERT_TRUE(Parse("[test!] -->> {if_1;if_2;then3;};"));
    ASSERT_STREQ("[test!]-->>{if_1; if_2; then3;};", ast->toString().c_str());
}

TEST_F(ParserTest, Repeat7) {
    ASSERT_TRUE(Parse("[test[0].@field != $test()!!] -->> if_1;"));
    ASSERT_STREQ("[test[0].@field!=$test()!!]-->>if_1;", ast->toString().c_str());
}

TEST_F(ParserTest, Range) {
    ASSERT_TRUE(Parse("0.1..0.9..0.1;"));
    ASSERT_STREQ("0.1..0.9..0.1", ast->toString().c_str());
}

TEST_F(ParserTest, Range1) {
    ASSERT_TRUE(Parse("[i!] -->> call();"));
    ASSERT_STREQ("[i!]-->>call();", ast->toString().c_str());

    ASSERT_TRUE(Parse("call() <<-- [i!];"));
    ASSERT_STREQ("call()<<--[i!];", ast->toString().c_str());
}

TEST_F(ParserTest, Range2) {
    ASSERT_TRUE(Parse("[i()] -->> @error(\"Error\");"));
    ASSERT_STREQ("[i()]-->>@error(\"Error\");", ast->toString().c_str());
}

TEST_F(ParserTest, DISABLED_Follow0) {
    //@todo Не получается сделать парсер с простым if, т.к. требуется вторая закрывающая точка с запятой
    ASSERT_TRUE(Parse("[test == human || ttttt && 123 != test.field ] -> if_1;"));
    ASSERT_STREQ("[test==human]->{if_1;};", ast->toString().c_str());
}

TEST_F(ParserTest, Follow0) {
    ASSERT_TRUE(Parse("[test] -> follow;"));
    ASSERT_STREQ("[test]->follow;", ast->toString().c_str());

    ASSERT_TRUE(Parse("[test] -> follow;"));
    ASSERT_STREQ("[test]->follow;", ast->toString().c_str());

    ASSERT_TRUE(Parse("[test] -> {follow;};"));
    ASSERT_STREQ("[test]->{follow;}", ast->toString().c_str());

    ASSERT_TRUE(Parse("[test] -> {follow};"));
    ASSERT_STREQ("[test]->{follow;}", ast->toString().c_str());

    ASSERT_TRUE(Parse("[test] -> {follow;};"));
    ASSERT_STREQ("[test]->{follow;}", ast->toString().c_str());

    ASSERT_TRUE(Parse("[test] -> {follow;};"));
    ASSERT_STREQ("[test]->{follow;}", ast->toString().c_str());
}

TEST_F(ParserTest, Follow1) {
    //@todo Не получается сделать парсер с простым if, т.к. требуется вторая закрывающая точка с запятой
    ASSERT_TRUE(Parse("[test == human] -> if_1;"));
    ASSERT_STREQ("[test==human]->if_1;", ast->toString().c_str());
}

TEST_F(ParserTest, Follow2) {
    ASSERT_TRUE(Parse("[test != human] -> {if_1;};"));
    ASSERT_STREQ("[test!=human]->{if_1;}", ast->toString().c_str());
}

TEST_F(ParserTest, Follow3) {
    ASSERT_TRUE(Parse("[test != human] -> {if_1;if_2;then3;};"));
    ASSERT_STREQ("[test!=human]->{if_1; if_2; then3;}", ast->toString().c_str());
}

TEST_F(ParserTest, Follow4) {
    ASSERT_TRUE(Parse("[test != human] -> {if_1;if_2;then3;}, [_] -> {else;};"));
    //    ASSERT_STREQ("(test!=human)->{if_1; if_2; then3;},\n (_) ->{else;};", ast->toString().c_str());
}

TEST_F(ParserTest, Follow5) {
    ASSERT_TRUE(Parse("[@test1] -> {then1;}, [$test2] -> {then2;then2;}, [@test3+$test3] -> {then3;};"));
    //    ASSERT_STREQ("(@test1)->{then1;}\n ($test2)->{then2; then2;}\n(@test3+$test3)->{then3;};", ast->toString().c_str());
}

TEST_F(ParserTest, Follow6) {
    ASSERT_TRUE(Parse("[@test1] -> {then1;}, [$test2] -> {then2;}, [@test3+$test3] -> {then3;}, [_] -> {else;else();};"));
    //    ASSERT_STREQ("(@test1)->{then1;},\n ($test2)->{then2;},\n (@test3+$test3)->{then3;},\n _ ->{else; else();};", ast->toString().c_str());
}

TEST_F(ParserTest, DISABLED_Follow7) {
    ASSERT_TRUE(Parse("[test.field[0] > iter!!] -> if_1;"));
    //    ASSERT_STREQ("(test.field[0].field2>iter!!)->{if_1;};", ast->toString().c_str());
}

TEST_F(ParserTest, Exit) {
    ASSERT_TRUE(Parse("--;"));
    ASSERT_TRUE(Parse("--;;"));
    ASSERT_TRUE(Parse("--_;"));
    ASSERT_TRUE(Parse("--_;;"));
    ASSERT_TRUE(Parse("--'';"));
    ASSERT_TRUE(Parse("--'';;"));
    ASSERT_TRUE(Parse("--eval;"));
    ASSERT_TRUE(Parse("--eval;;"));
    ASSERT_TRUE(Parse("--eval();"));
    ASSERT_TRUE(Parse("--eval();;"));
    ASSERT_TRUE(Parse("--0;"));
    ASSERT_TRUE(Parse("--0;;"));
    ASSERT_TRUE(Parse("--0.1;"));
    ASSERT_TRUE(Parse("--0.1;;"));
    ASSERT_TRUE(Parse("--[0,];"));
    ASSERT_TRUE(Parse("--(0,);;"));
    ASSERT_TRUE(Parse("--:Class(var);"));
    ASSERT_TRUE(Parse("--:Class[0](1);"));
    ASSERT_TRUE(Parse("--call();"));
    ASSERT_TRUE(Parse("--call();;"));
    ASSERT_TRUE(Parse("--call(arg);"));
    ASSERT_TRUE(Parse("--call(arg);;"));
    ASSERT_TRUE(Parse("--:class;"));
    ASSERT_TRUE(Parse("--:class;;"));
    ASSERT_TRUE(Parse("--:class();"));
    ASSERT_TRUE(Parse("--:class();;"));
    ASSERT_TRUE(Parse("--:class(arg);"));
    ASSERT_TRUE(Parse("--:class(arg);;"));
}


//TEST_F(ParserTest, Lambda1) {
//    ASSERT_TRUE(Parse("lambda := (){};"));
//    ASSERT_STREQ("lambda := (){};", ast->toString().c_str());
//}
//
//TEST_F(ParserTest, Lambda2) {
//    ASSERT_TRUE(Parse("lambda := (arg)^^{test1};"));
//    ASSERT_STREQ("lambda := (arg)^^{test1};", ast->toString().c_str());
//}
//
//TEST_F(ParserTest, Lambda3) {
//    ASSERT_TRUE(Parse("lambda := (arg, arg2=empty  ) || {    test1, test  ==  test2};"));
//    ASSERT_STREQ("lambda := (arg, arg2=empty)||{test1, test==test2};", ast->toString().c_str());
//}
//
//TEST_F(ParserTest, Lambda4) {
//    ASSERT_TRUE(Parse("lambda := (arg, arg2=(){}  ) && {    test1(), test  !=  test2};"));
//    ASSERT_STREQ("lambda := (arg, arg2=(){})&&{test1(), test!=test2};", ast->toString().c_str());
//}
//
//TEST_F(ParserTest, Lambda5) {
//    ASSERT_TRUE(Parse("iter := (obj)&&{obj~~human, obj!~~(sex=_,parent=_,)};"));
//    ASSERT_STREQ("iter := (obj)&&{obj~~human, obj!~~(sex=_, parent=_,)};", ast->toString().c_str());
//}

//TEST_F(ParserTest, ConstName) {
//    ASSERT_TRUE(Parse("#const ;"));
//    ASSERT_STREQ("#const", ast->toString().c_str());
//}
//
//TEST_F(ParserTest, ConstName2) {
//    ASSERT_TRUE(Parse("#const() ;"));
//    ASSERT_STREQ("#const()", ast->toString().c_str());
//}
//
//TEST_F(ParserTest, HashReturn0) {
//    ASSERT_TRUE(Parse("##"));
//    ASSERT_STREQ("##", ast->toString().c_str());
//}
//
//TEST_F(ParserTest, HashReturn1) {
//    ASSERT_TRUE(Parse("## name"));
//    ASSERT_STREQ("## name", ast->toString().c_str());
//}
//
//TEST_F(ParserTest, HashReturn2) {
//    ASSERT_TRUE(Parse("## term();"));
//    ASSERT_STREQ("## term()", ast->toString().c_str());
//}
//
//TEST_F(ParserTest, HashReturn3) {
//    ASSERT_TRUE(Parse("## name(\"str\");"));
//    ASSERT_STREQ("## name(\"str\")", ast->toString().c_str());
//}

//TEST_F(ParserTest, HashExcept0) {
//    ASSERT_TRUE(Parse("###"));
//    ASSERT_STREQ("###", ast->toString().c_str());
//}
//
//TEST_F(ParserTest, HashExcept1) {
//    ASSERT_TRUE(Parse("### name"));
//    ASSERT_STREQ("### name", ast->toString().c_str());
//}
//
//TEST_F(ParserTest, HashExcept2) {
//    ASSERT_TRUE(Parse("### term();"));
//    ASSERT_STREQ("### term()", ast->toString().c_str());
//}
//

TEST_F(ParserTest, HelloWorld) {
    ASSERT_TRUE(Parse("hello(str=\"\") := { printf(format:Format, ...):Int := @import('printf'); printf('%s', $1); $str;};"));
    //    ASSERT_STREQ("!!!!!!!!!!!!!!", ast->toString().c_str());
}

TEST_F(ParserTest, DISABLED_Convert) {
    std::vector<const char *> list = {
        "brother(human!, human!)?;",
        "func(arg1, arg2 = 5) :- { ($arg1 < $2) -> {%{ return $arg1; %}}, [_] -> {%{ return *$1 * *$2; %}}; };",
        "func_sum(arg1, arg2) :- {$arg1 + $arg2;};",
    };
    TermPtr expr;
    Parser parser(expr);
    for (size_t i = 0; i < list.size(); i++) {
        ASSERT_NO_THROW(
                parser.Parse(list[i]);
                ) << "FROM: " << list[i];
        std::string to_str = expr->toString() + ";";
        ASSERT_NO_THROW(
                parser.Parse(to_str);
                ) << "CONVERT: " << to_str;
    }
}

#endif // UNITTEST