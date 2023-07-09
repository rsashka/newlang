#include "pch.h"

#ifdef UNITTEST

#include <warning_push.h>
#include <gtest/gtest.h>
#include <warning_pop.h>

#include <parser.h>
#include <macro.h>
#include <term.h>
#include "version.h"
#include "newlang.h"
#include "nlc.h"
#include "lexer.h"

using namespace newlang;

class MacroTest : public ::testing::Test {
protected:



    std::vector<std::string> m_postlex;

    utils::Logger::FuncCallback *m_log_callback_save;
    void *m_log_callback_arg_save;
    std::string m_output;

    static void LoggerCallback(void *param, utils::Logger::LogLevelType level, const char * str, bool flush) {
        MacroTest *p = static_cast<MacroTest *> (param);
        fprintf(stdout, "%s", str);
        if(flush) {
            fflush(stdout);
        }
        if(p) {
            p->m_output += str;
        }
    }

    void SetUp() {
        utils::Logger::Instance()->SaveCallback(m_log_callback_save, m_log_callback_arg_save);
        utils::Logger::Instance()->Clear();
        utils::Logger::Instance()->SetCallback(&LoggerCallback, this);
    }

    void TearDown() {
        utils::Logger::Instance()->SetCallback(m_log_callback_save, m_log_callback_arg_save);
    }

    TermPtr Parse(std::string str, MacroBuffer *buffer = nullptr, DiagPtr diag = nullptr) {
        m_postlex.clear();
        ast = Parser::ParseString(str, buffer, &m_postlex, diag);
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

    std::string LexOut() {
        std::string result;
        for (int i = 0; i < m_postlex.size(); i++) {
            if(!result.empty()) {
                result += " ";
            }
            result += m_postlex[i];
        }
        trim(result);
        return result;
    }

    TermPtr ast;
};




/*
 * Макросы хрянтся как фрагменты AST в виде объектов типа Term,
 * так как создаются после обработки парсером по правилам стандратной грамматики.
 * 
 * Но анализ входных данных на предмет раскрытия макросов проиходит из потока (последовательности) лексем,
 * т.е. **ДО** обработки парсером, так как при раскрытии макросов может меняться грамматика выражений.
 * 
 * Функция \ref IdentityMacro сравнивает входной буфер (последовательность лексем)
 * на предмет возможного соответствия одному конкретному макросу.
 * Сравниваются только ключевые словам без учета аргументов в скобках, только наличие или отсуствие скобок,
 * так с аргументами и без аргументов, это два разных макроса.
 * Проверка аргументов макроса выполняется в функции \ref ExtractArgs
 * 
 * Проблема:
 * К функции можно обратится с целью её вызова (указав после имени скобки), 
 * так и по имени без скобок (например для получения ссылки на объект).
 * Так и к переменной можно обратиться только по имени, 
 * так и указав скобки после имени как к функции (при копировании объекта).
 * 
 * Сделать один макрос под оба сценария (со скобками и без скобок) не получится, так как непонятно, 
 * что делать с аргументами при раскрытии тела макроса, когда они не указаны.
 * 
 * Но может быть следует переименовывать только одно имя без учета скобок,
 * например, оставив такую возможность только для одиночных макросов.
 * 
 * 
 * @macro() := with_bracket;
 * @macro := without_bracket;
 *   или
 * @macro :- ignore_bracket;
 * 
 * macro() -> ignore_bracket();
 * macro -> ignore_bracket;
 * 
 * 
 * \\name -> name или name (...), hash: name
 * но \\name(...) -> name(...), а name - ошибка !!!!  hash: name
 * 
 * Или все же следует различать макросы со скобками и без скобок как два разных объекта???
 * \\name := name2;  и \\name(...) := name2(...); будут разными объектами
 * 
 * Или добавить макросы-алиасы без аргументов только для переименования отдельных терминов?
 * @alias :- name2; но как их отличать от обычных макросов в операции удаления?
 * Или вообще не нужно отличать и оставить только один варинат (либо макрос, либо алиас)
 * Тода вопрос со скобками закрывается сам собой, алиасы всегда без скобок,
 * макросы со скбками или без скобок, но должно быть полное соответствие.
 * 
 * @@ name name2 @@ -> name name2, но name name2(...) - ошибка!!! (hash: name,name2)
 * @@ name name2(...) @@ -> name name2(...), но name name2 - ошибка !!!! (hash: name,name2)
 * @@ name name2[...](...) @@ -> name name2[...](...), но name name2 - ошибка !!!!  ( hash: name,name2 )
 * @@ name $tmpl[...](...) name3@@ -> name $tmpl[...](...) name3, но name $tmpl name3 - ошибка !!!!  ( hash: name,$,name3 )
 * 
 * Но!
 * @@ name name2 @@ ::-  -> name name2 - ок, но name name2(...) - ок
 * @@ name $tmpl name3 @@ ::- -> name $tmpl[...](...) name3 - ок, name $tmpl(...) name3[...] - ок
 * 
 * 
 * 
 * Для последовательности лексем требуется полное соответствие с учетом скобок ???
 * и может ли быть несколько скобок одного типа (несколько круглых, или несколько квадратных????
 * <Может быть несколько скобок одного типа, например, при указании типа у аргументов или типа возвращаемого значения.>
 * 
 * @@ name name2 @@ -> name name2, но name name2(...) - ошибка!!! (hash: name,name2)
 * @@ name name2(...) @@ -> name name2(...), но name name2 - ошибка !!!! (hash: name,name2)
 * @@ name name2[...](...) @@ -> name name2[...](...), но name name2 - ошибка !!!!  ( hash: name,name2 )
 * @@ name $tmpl[...](...) name3 @@ -> name $tmpl[...](...) name3, но name $tmpl name3 - ошибка !!!!  ( hash: name,$,name3 )
 * 
 * Проблема скобок возникает из-за сценария замены одного термина на другой, 
 * который есть в препроцессоре С/С++, но отсутствует при реализации с использованием шаблонов.
 * 
 * 
 * 
 * 
 * ```python

try:
    a = float(input("Введите число: ")
    print (100 / a)
except ValueError:
    print ("Это не число!")
except ZeroDivisionError:
    print ("На ноль делить нельзя!")
except:
    print ("Неожиданная ошибка.")
else:
    print ("Код выполнился без ошибок")
finally:
    print ("Я выполняюсь в любом случае!")
    
```
```

[1] =>{
    
    [1] --> {
    
    },[2] --> {*
    
 *},[_] --> {
        other
    }
    on_exit();
};


{*   # try:
    a = float(input("Введите число: ");
    print (100 / a);
 *} ~> {
 
    [:ValueError] --> print ("Это не число!"),  # except ValueError:
    [:ZeroDivisionError] --> print ("На ноль делить нельзя!"), # except ZeroDivisionError
    [:IntMinus] --> print ("Неожиданная ошибка."), # except:
    [_] --> print ("Код выполнился без ошибок");  # else:
    
    print ("Я выполняюсь в любом случае!");    # finally:
}
 * try := @ __TERM_EXPECTED__('{*') @
 * 
 * catch := @ *} ~> @ 
 * 
 *      
 */

/*
 @##
 @#

@func(...) := call(?);

@$name

@$name(...)
@$name[...]
@$name<...>

@$name(*)
@$name[*]
@$name<*>

@$name(#)
@$name[#]
@$name<#>

 *  */
TEST_F(MacroTest, Escape) {

    ASSERT_TRUE(Parse("@\\ {+"));
    ASSERT_EQ(TermID::ESCAPE, ast->getTermID());
    EXPECT_STREQ("{+", ast->m_text.c_str());
}

TEST_F(MacroTest, ParseLexem) {

    BlockType arr = Scanner::ParseLexem("1 2 3 4 5");

    ASSERT_EQ(5, arr.size());
    ASSERT_STREQ("1 2 3 4 5", MacroBuffer::Dump(arr).c_str());

    arr = Scanner::ParseLexem("macro    @test(1,2,3,...):type; next \n; # sssssss\n @only lexem((((;;     ;");
    ASSERT_STREQ("macro @test ( 1 , 2 , 3 , ... ) : type ; next ; @only lexem ( ( ( ( ; ; ;", MacroBuffer::Dump(arr).c_str());
}

TEST_F(MacroTest, SkipBrackets) {


    MacroBuffer macro;
    BlockType buffer;

    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 0));
    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 1));

    buffer.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "name"));

    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 0));
    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 1));

    buffer.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, "("));
    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 0));
    ASSERT_ANY_THROW(MacroBuffer::SkipBrackets(buffer, 1));

    buffer.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, ")"));
    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 0));
    ASSERT_EQ(2, MacroBuffer::SkipBrackets(buffer, 1));
    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 2));


    buffer.insert(buffer.begin(), Term::Create(parser::token_type::NAME, TermID::NAME, "first")); // first name ( )

    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 0));
    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 1));

    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 0));
    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 1));
    ASSERT_EQ(2, MacroBuffer::SkipBrackets(buffer, 2));
    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 3));


    buffer.insert(buffer.end() - 1, Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, "..."));
    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 0));
    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 1));
    ASSERT_EQ(3, MacroBuffer::SkipBrackets(buffer, 2));
    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 3));
    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 4));
    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 5));

    buffer.insert(buffer.end() - 1, Term::Create(parser::token_type::NAME, TermID::NAME, "name"));
    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 0));
    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 1));
    ASSERT_EQ(4, MacroBuffer::SkipBrackets(buffer, 2));
    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 3));
    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 4));
    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 5));
    ASSERT_EQ(0, MacroBuffer::SkipBrackets(buffer, 6));

}

TEST_F(MacroTest, PredefMacro) {
    MacroBuffer macro;

    ASSERT_TRUE(macro.m_predef_macro.empty());
    ASSERT_EQ(0, macro.m_predef_macro.size());

    TermPtr term = Term::Create(parser::token_type::NAME, TermID::NAME, "__NLC_VER__");
    ASSERT_EQ(parser::token_type::INTEGER, macro.ExpandPredefMacro(term, nullptr));

    ASSERT_FALSE(macro.m_predef_macro.empty());
    ASSERT_TRUE(macro.m_predef_macro.size() > 5);


    ASSERT_EQ(0, macro.m_counter);
    for (auto &elem : macro.m_predef_macro) {
        ASSERT_FALSE(elem.first.empty());
        ASSERT_FALSE(elem.second.empty());

        term->m_text = elem.first;
        term->m_id = TermID::NAME;
        term->m_lexer_type = parser::token_type::NAME;

        ASSERT_NE(parser::token_type::NAME, macro.ExpandPredefMacro(term, nullptr));
    }
    ASSERT_EQ(1, macro.m_counter);


    ASSERT_TRUE(Parse("__NLC_VER__", &macro));
    ASSERT_STREQ(std::to_string(VERSION).c_str(), ast->toString().c_str());
    ASSERT_EQ(TermID::INTEGER, ast->getTermID());

    macro.m_counter = 0;
    ASSERT_TRUE(Parse("__COUNTER__", &macro));
    ASSERT_STREQ("0", ast->toString().c_str());
    ASSERT_EQ(1, macro.m_counter);
    ASSERT_EQ(TermID::INTEGER, ast->getTermID());

    ASSERT_TRUE(Parse("__COUNTER__", &macro));
    ASSERT_STREQ("1", ast->toString().c_str());
    ASSERT_EQ(TermID::INTEGER, ast->getTermID());

    ASSERT_TRUE(Parse("@__COUNTER__", &macro));
    ASSERT_STREQ("2", ast->toString().c_str());
    ASSERT_EQ(TermID::INTEGER, ast->getTermID());

    ASSERT_TRUE(Parse("@__COUNTER__", &macro));
    ASSERT_STREQ("3", ast->toString().c_str());
    ASSERT_EQ(TermID::INTEGER, ast->getTermID());


    ASSERT_TRUE(Parse("__TIME__", &macro));
    ASSERT_EQ(10, ast->toString().size()) << ast->toString();
    ASSERT_EQ(TermID::STRWIDE, ast->getTermID());

    ASSERT_TRUE(Parse("__DATE__", &macro));
    ASSERT_EQ(13, ast->toString().size()) << ast->toString();
    ASSERT_EQ(TermID::STRWIDE, ast->getTermID());

    ASSERT_TRUE(Parse("__TIMESTAMP__", &macro));
    ASSERT_EQ(26, ast->toString().size()) << ast->toString();
    ASSERT_EQ(TermID::STRWIDE, ast->getTermID());

    ASSERT_TRUE(Parse("__TIMESTAMP_ISO__", &macro));
    ASSERT_EQ(22, ast->toString().size()) << ast->toString();
    ASSERT_EQ(TermID::STRWIDE, ast->getTermID());


    ASSERT_TRUE(Parse("__LINE__", &macro));
    ASSERT_STREQ(std::to_string(1).c_str(), ast->toString().c_str());
    ASSERT_EQ(TermID::INTEGER, ast->getTermID());

    ASSERT_TRUE(Parse("\n@__LINE__", &macro));
    ASSERT_STREQ(std::to_string(2).c_str(), ast->toString().c_str());
    ASSERT_EQ(TermID::INTEGER, ast->getTermID());

    ASSERT_TRUE(Parse("\n\n__FILE_LINE__", &macro));
    ASSERT_STREQ(std::to_string(3).c_str(), ast->toString().c_str());
    ASSERT_EQ(TermID::INTEGER, ast->getTermID());


    ASSERT_TRUE(Parse("__FILE__", &macro));
    ASSERT_STREQ("\"File name undefined!!!\"", ast->toString().c_str());
    ASSERT_EQ(TermID::STRWIDE, ast->getTermID());

    ASSERT_TRUE(Parse("__FILE_NAME__", &macro));
    ASSERT_STREQ("\"File name undefined!!!\"", ast->toString().c_str());
    ASSERT_EQ(TermID::STRWIDE, ast->getTermID());


    ASSERT_TRUE(Parse("__FILE_TIMESTAMP__", &macro));
    ASSERT_EQ(26, ast->toString().size()) << ast->toString();
    ASSERT_EQ(TermID::STRWIDE, ast->getTermID());

    ASSERT_TRUE(Parse("__FILE_MD5__", &macro));
    ASSERT_TRUE(ast->toString().size() > 30) << ast->toString();
    ASSERT_EQ(TermID::STRWIDE, ast->getTermID());


    ASSERT_TRUE(Parse("__NLC_SOURCE_GIT__", &macro));
    ASSERT_STREQ("\"" VERSION_GIT_SOURCE "\"", ast->toString().c_str());
    ASSERT_EQ(TermID::STRWIDE, ast->getTermID());

    ASSERT_TRUE(Parse("__NLC_DATE_BUILD__", &macro));
    ASSERT_STREQ("\"" VERSION_DATE_BUILD_STR "\"", ast->toString().c_str());
    ASSERT_EQ(TermID::STRWIDE, ast->getTermID());

    ASSERT_TRUE(Parse("__NLC_SOURCE_BUILD__", &macro));
    ASSERT_STREQ("\"" VERSION_SOURCE_FULL_ID "\"", ast->toString().c_str());
    ASSERT_EQ(TermID::STRWIDE, ast->getTermID());



    std::filesystem::create_directories("temp");
    ASSERT_TRUE(std::filesystem::is_directory("temp"));

    std::ofstream out("temp/file.md5.test");
    out << "__LINE__; __FILE__; __FILE_TIMESTAMP__; __FILE_MD5__;";
    out.close();

    Parser p(&macro);

    ASSERT_TRUE(ast = p.ParseFile("temp/file.md5.test"));


    ASSERT_EQ(4, ast->m_block.size()) << ast->toString().c_str();

    ASSERT_STREQ("1", ast->m_block[0]->toString().c_str());
    ASSERT_EQ(TermID::INTEGER, ast->m_block[0]->getTermID());

    ASSERT_TRUE(ast->m_block[1]->toString().find("temp/file.md5.test\"") != std::string::npos);
    ASSERT_EQ(TermID::STRWIDE, ast->m_block[1]->getTermID());

    ASSERT_EQ(26, ast->m_block[2]->toString().size()) << ast->m_block[2]->toString();
    ASSERT_EQ(TermID::STRWIDE, ast->m_block[2]->getTermID());

    ASSERT_STREQ("\"6c5d2069ede975a4e09f4f432a8ad648\"", ast->m_block[3]->toString().c_str());
    ASSERT_EQ(TermID::STRWIDE, ast->m_block[3]->getTermID());

    std::remove("temp/file.md5.test");



    term->m_text = "__NOT_FOUND__";
    term->m_id = TermID::NAME;
    term->m_lexer_type = parser::token_type::NAME;

    ASSERT_NO_THROW(macro.ExpandPredefMacro(term, nullptr));

    ASSERT_TRUE(macro.RegisterPredefMacro("__NOT_FOUND__", ""));
    ASSERT_ANY_THROW(macro.ExpandPredefMacro(term, nullptr));
    ASSERT_FALSE(macro.RegisterPredefMacro("__NOT_FOUND__", ""));

}

TEST_F(MacroTest, ParseTerm) {

    TermPtr term;
    BlockType buff;
    size_t size;


    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "alias")); // alias 

    ASSERT_NO_THROW(size = MacroBuffer::ParseTerm(term, buff, 0));
    ASSERT_EQ(1, size);
    ASSERT_TRUE(term);
    ASSERT_FALSE(term->isCall());
    ASSERT_STREQ("alias", term->toString().c_str());


    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "alias")); // alias alias 

    ASSERT_NO_THROW(size = MacroBuffer::ParseTerm(term, buff, 0));
    ASSERT_EQ(1, size);
    ASSERT_TRUE(term);
    ASSERT_FALSE(term->isCall());
    ASSERT_STREQ("alias", term->toString().c_str());

    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "second")); // alias alias second 

    ASSERT_NO_THROW(size = MacroBuffer::ParseTerm(term, buff, 0));
    ASSERT_EQ(1, size);
    ASSERT_TRUE(term);
    ASSERT_FALSE(term->isCall());
    ASSERT_STREQ("alias", term->toString().c_str());

    buff.erase(buff.begin(), buff.begin() + 2);
    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, "(")); // second (

    ASSERT_ANY_THROW(MacroBuffer::ParseTerm(term, buff, 0));

    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, ")")); // second ( )

    ASSERT_NO_THROW(size = MacroBuffer::ParseTerm(term, buff, 0));
    ASSERT_EQ(3, size);
    ASSERT_TRUE(term);
    ASSERT_TRUE(term->isCall());
    ASSERT_STREQ("second()", term->toString().c_str());

    buff.erase(buff.end()); // second ( 

    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "name")); // second ( name
    ASSERT_ANY_THROW(MacroBuffer::ParseTerm(term, buff, 0));

    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, ")")); // second ( name )

    ASSERT_NO_THROW(size = MacroBuffer::ParseTerm(term, buff, 0));
    ASSERT_EQ(4, size);
    ASSERT_TRUE(term);
    ASSERT_TRUE(term->isCall());
    ASSERT_STREQ("second(name)", term->toString().c_str());


    buff.erase(buff.end()); // second ( name
    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, "=")); // second ( name =
    ASSERT_ANY_THROW(MacroBuffer::ParseTerm(term, buff, 0));

    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "value")); // second ( name = value
    ASSERT_ANY_THROW(MacroBuffer::ParseTerm(term, buff, 0));

    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, ")")); // second ( name = value )

    ASSERT_NO_THROW(size = MacroBuffer::ParseTerm(term, buff, 0));
    ASSERT_EQ(6, size);
    ASSERT_TRUE(term);
    ASSERT_TRUE(term->isCall());
    ASSERT_EQ(1, term->size());
    ASSERT_STREQ("name", term->name(0).c_str());
    ASSERT_STREQ("second(name=value)", term->toString().c_str());


    buff = Scanner::ParseLexem("second2 ( 1 , ( 123 , ) );\n\n\n\n;");

    ASSERT_NO_THROW(size = MacroBuffer::ParseTerm(term, buff, 0));
    ASSERT_EQ(9, size);
    ASSERT_TRUE(term);
    ASSERT_TRUE(term->isCall());
    ASSERT_EQ(2, term->size());
    ASSERT_STREQ("1", term->at(0).second->toString().c_str());
    ASSERT_STREQ("(123,)", term->at(1).second->toString().c_str());
    ASSERT_STREQ("second2(1, (123,))", term->toString().c_str());

}

TEST_F(MacroTest, Pragma) {

    m_output.clear();
    ASSERT_ANY_THROW(Parse("@__PRAGMA_NOT_FOUND__()"));
    ASSERT_STREQ("", LexOut().c_str());
    ASSERT_TRUE(m_output.find("Uknown pragma '@__PRAGMA_NOT_FOUND__()") != std::string::npos) << m_output;

    m_output.clear();
    ASSERT_ANY_THROW(Parse("@__PRAGMA_DIAG__("));
    ASSERT_STREQ("", LexOut().c_str());
    ASSERT_TRUE(m_output.find("Closed bracket ')' not found!") != std::string::npos) << m_output;

    m_output.clear();
    ASSERT_ANY_THROW(Parse("@__PRAGMA_DIAG__)"));
    ASSERT_STREQ("", LexOut().c_str());
    ASSERT_TRUE(m_output.find("Expected") != std::string::npos) << m_output;

    m_output.clear();
    ASSERT_ANY_THROW(Parse("@__PRAGMA_DIAG__(,)"));
    ASSERT_STREQ("", LexOut().c_str());
    ASSERT_TRUE(m_output.find("unexpected ','") != std::string::npos) << m_output;

    m_output.clear();
    ASSERT_ANY_THROW(Parse("@__PRAGMA_DIAG__(push,)"));
    ASSERT_STREQ("", LexOut().c_str());
    ASSERT_TRUE(m_output.find("syntax error, unexpected ')'") != std::string::npos) << m_output;


    DiagPtr diag = Diag::Init();
    ASSERT_EQ(1, diag->m_diag_stack.size());
    ASSERT_TRUE(Parse("@__PRAGMA_DIAG__(push)", nullptr, diag));
    ASSERT_EQ(2, diag->m_diag_stack.size());
    ASSERT_STREQ("", LexOut().c_str());

    ASSERT_TRUE(Parse("@__PRAGMA_DIAG__(   pop  )", nullptr, diag));
    ASSERT_EQ(1, diag->m_diag_stack.size());
    ASSERT_STREQ("", LexOut().c_str());


    m_output.clear();
    ASSERT_TRUE(Parse("@__PRAGMA_MESSAGE__(test)", nullptr, diag));
    ASSERT_STREQ("", LexOut().c_str());
    ASSERT_TRUE(m_output.find("note: test") != std::string::npos) << m_output;

    m_output.clear();
    ASSERT_TRUE(Parse("@__PRAGMA_MESSAGE__(\"test\")", nullptr, diag));
    ASSERT_STREQ("", LexOut().c_str());
    ASSERT_TRUE(m_output.find("note: test") != std::string::npos) << m_output;

    m_output.clear();
    ASSERT_TRUE(Parse("@__PRAGMA_MESSAGE__(\"test test2\")", nullptr, diag));
    ASSERT_STREQ("", LexOut().c_str());
    ASSERT_TRUE(m_output.find("note: test test2") != std::string::npos) << m_output;

    m_output.clear();
    ASSERT_TRUE(Parse("@__PRAGMA_MESSAGE__(test,\"test2\")", nullptr, diag));
    ASSERT_STREQ("", LexOut().c_str());
    ASSERT_TRUE(m_output.find("note: testtest2") != std::string::npos) << m_output;

    m_output.clear();
    ASSERT_ANY_THROW(Parse("@__PRAGMA_EXPECTED__( @\\ {, @\\ {*, @\\ {+)", nullptr, diag));
    ASSERT_STREQ("", LexOut().c_str());
    ASSERT_TRUE(m_output.find("Term '{', '{*', '{+' expected!") != std::string::npos) << m_output;

    m_output.clear();
    ASSERT_ANY_THROW(Parse("@__PRAGMA_EXPECTED__( @\\ {, @\\ {*, @\\ {+) {- -}", nullptr, diag));
    ASSERT_STREQ("", LexOut().c_str());
    ASSERT_TRUE(m_output.find("Term '{', '{*', '{+' expected!") != std::string::npos) << m_output;

    m_output.clear();
    ASSERT_NO_THROW(Parse("@__PRAGMA_EXPECTED__( @\\ {, @\\ {*, @\\ {+ ){}", nullptr, diag));
    ASSERT_STREQ("{ }", LexOut().c_str());
    ASSERT_TRUE(m_output.find("Term '{', '{*', '{+' expected!") == std::string::npos) << m_output;

    m_output.clear();
    ASSERT_NO_THROW(Parse("@__PRAGMA_EXPECTED__( @\\ {, @\\ {*, @\\ {+){+ +}", nullptr, diag));
    ASSERT_STREQ("{+ +}", LexOut().c_str());
    ASSERT_TRUE(m_output.find("Term '{', '{*', '{+' expected!") == std::string::npos) << m_output;


    m_output.clear();
    ASSERT_ANY_THROW(Parse("@__PRAGMA_UNEXPECTED__( @\\{, @\\{*, @\\{+)  {", nullptr, diag));
    ASSERT_STREQ("", LexOut().c_str());
    ASSERT_TRUE(m_output.find("Term '{' unexpected!") != std::string::npos) << m_output;

    m_output.clear();
    ASSERT_NO_THROW(Parse("@__PRAGMA_UNEXPECTED__( @\\{, @\\{*, @\\{+){- -}", nullptr, diag));
    ASSERT_STREQ("{- -}", LexOut().c_str());
    ASSERT_TRUE(m_output.find("expected") == std::string::npos) << m_output;

    m_output.clear();
    ASSERT_NO_THROW(Parse("@__PRAGMA_UNEXPECTED__( @\\{*, @\\{+){}", nullptr, diag));
    ASSERT_STREQ("{ }", LexOut().c_str());
    ASSERT_TRUE(m_output.find("expected") == std::string::npos) << m_output;

    m_output.clear();
    ASSERT_ANY_THROW(Parse("@__PRAGMA_UNEXPECTED__( name ) name", nullptr, diag));
    ASSERT_STREQ("", LexOut().c_str());
    ASSERT_TRUE(m_output.find("Term 'name' unexpected!") != std::string::npos) << m_output;

}

TEST_F(MacroTest, Annotate) {

    m_output.clear();
    ASSERT_ANY_THROW(Parse("@__ANNOTATION_SET__"));
    ASSERT_STREQ("", LexOut().c_str());
    ASSERT_TRUE(m_output.find("Annotation args in '@__ANNOTATION_SET__' not recognized!") != std::string::npos) << m_output;


    m_output.clear();
    ASSERT_ANY_THROW(Parse("@__ANNOTATION_SET__()"));
    ASSERT_STREQ("", LexOut().c_str());
    ASSERT_TRUE(m_output.find("Annotation args in '@__ANNOTATION_SET__()' not recognized!") != std::string::npos) << m_output;

    m_output.clear();
    ASSERT_NO_THROW(Parse("@__ANNOTATION_SET__(name)"));
    ASSERT_STREQ("", LexOut().c_str());

    m_output.clear();
    ASSERT_NO_THROW(Parse("@__ANNOTATION_SET__(name, \"value\")"));
    ASSERT_STREQ("", LexOut().c_str());

    m_output.clear();
    ASSERT_NO_THROW(Parse("@__ANNOTATION_SET__(name, 1)"));
    ASSERT_STREQ("", LexOut().c_str());



    m_output.clear();
    ASSERT_ANY_THROW(Parse("@__ANNOTATION_IIF__"));
    ASSERT_STREQ("", LexOut().c_str());
    ASSERT_TRUE(m_output.find("Annotation IIF must have three arguments!") != std::string::npos) << m_output;

    m_output.clear();
    ASSERT_ANY_THROW(Parse("@__ANNOTATION_IIF__()"));
    ASSERT_STREQ("", LexOut().c_str());
    ASSERT_TRUE(m_output.find("Annotation IIF must have three arguments!") != std::string::npos) << m_output;

    m_output.clear();
    ASSERT_ANY_THROW(Parse("@__ANNOTATION_IIF__(name)"));
    ASSERT_STREQ("", LexOut().c_str());
    ASSERT_TRUE(m_output.find("Annotation IIF must have three arguments!") != std::string::npos) << m_output;

    m_output.clear();
    ASSERT_ANY_THROW(Parse("@__ANNOTATION_IIF__(name, 1)"));
    ASSERT_STREQ("", LexOut().c_str());
    ASSERT_TRUE(m_output.find("Annotation IIF must have three arguments!") != std::string::npos) << m_output;


    m_output.clear();
    ASSERT_NO_THROW(Parse("@__ANNOTATION_SET__(name, 1)    @__ANNOTATION_IIF__(name, 1, 2)"));
    ASSERT_STREQ("1", LexOut().c_str());

    //    m_output.clear();
    //    ASSERT_NO_THROW(Parse("@__ANNOTATION_SET__(name, 1);\n   @__ANNOTATION_IIF__(other, 1, 2);\n"));
    //    ASSERT_STREQ("2", LexOut().c_str());

    m_output.clear();
    ASSERT_NO_THROW(Parse("@__ANNOTATION_SET__(name, 0)   @__ANNOTATION_IIF__(name, 1, 2)"));
    ASSERT_STREQ("2", LexOut().c_str());
}

TEST_F(MacroTest, Buffer) {

    ASSERT_STREQ("name", MacroBuffer::toHashTermName("name").c_str());
    ASSERT_STREQ("$", MacroBuffer::toHashTermName("$name").c_str());
    ASSERT_STREQ("name", MacroBuffer::toHashTermName("@name").c_str());

    TermPtr term;
    BlockType buffer;

    ASSERT_FALSE(MacroBuffer::IdentityMacro(buffer, term));

#define CREATE_TERM(type, text)  Term::Create(parser::token_type:: type, TermID:: type, text)

    term = CREATE_TERM(MACRO, "macro");
    term->m_follow.push_back(CREATE_TERM(MACRO, "macro"));

    buffer.push_back(CREATE_TERM(MACRO, "macro"));
    ASSERT_TRUE(MacroBuffer::IdentityMacro(buffer, term));

    // Входной буфер больше
    buffer.push_back(CREATE_TERM(MACRO, "macro2"));
    ASSERT_TRUE(MacroBuffer::IdentityMacro(buffer, term));

    // Разные имена терминов
    term->m_text = "macro2";
    term->m_follow[0]->m_text = "macro2";
    term->m_macro_id[0]->m_text = "macro2";
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buffer, term));

    ASSERT_EQ(2, buffer.size());
    buffer.erase(buffer.begin(), buffer.begin() + 1);
    ASSERT_EQ(1, buffer.size());
    ASSERT_TRUE(MacroBuffer::IdentityMacro(buffer, term));





    TermPtr hash = CREATE_TERM(MACRO_SEQ, "name1");
    hash->m_follow.push_back(CREATE_TERM(NAME, "name2"));

    ASSERT_STREQ("name2", MacroBuffer::toHash(hash).c_str());

    term = CREATE_TERM(NAME, "name");
    term->AppendFollow(term);
    ASSERT_STREQ("name", term->m_text.c_str());

    TermPtr term2 = CREATE_TERM(NAME, "name2");
    term->AppendFollow(term2);
    ASSERT_STREQ("name", term->m_text.c_str());


    MacroBuffer macro;
    ASSERT_EQ(0, macro.size());

    ASSERT_TRUE(term = Parse("@@alias@@ := alias_name", &macro));
    ASSERT_STREQ("@@ alias @@ := alias_name", LexOut().c_str());
    ASSERT_TRUE(term);
    ASSERT_STREQ("alias", term->toString().c_str());
    ASSERT_EQ(1, term->m_follow.size());
    ASSERT_TRUE(term->m_follow[0]);
    ASSERT_STREQ("alias", term->m_follow[0]->toString().c_str());
    ASSERT_FALSE(term->m_follow[0]->isCall());

    BlockType id = macro.begin()->second[0]->GetMacroId();
    ASSERT_EQ(1, id.size()) << macro.begin()->second[0]->toString().c_str();
    ASSERT_STREQ("alias", id[0]->m_text.c_str());


    ASSERT_EQ(1, macro.GetCount()) << macro.Dump();
    ASSERT_TRUE(macro.GetMacro({"alias"})) << macro.Dump();

    // FAIL REDEFINE
    ASSERT_ANY_THROW(Parse("@@alias@@ ::= alias_name2;", &macro)) << macro.Dump();
    ASSERT_EQ(1, macro.GetCount()) << macro.Dump();
    //    ASSERT_TRUE(macro.GetMacro({"alias"})) << macro.Dump();
    //    ASSERT_STREQ("@@ alias @@ ::= alias_name2 ;", LexOut().c_str());

    //    ASSERT_ANY_THROW(Parse("@alias+alias := alias_name;", &macro)) << macro.Dump();
    //    ASSERT_EQ(1, macro.GetCount());

    ASSERT_TRUE(term = Parse("@@alias2@@ := alias_name", &macro));
    ASSERT_EQ(2, macro.GetCount());
    ASSERT_STREQ("@@ alias2 @@ := alias_name", LexOut().c_str());

    ASSERT_EQ(1, term->m_follow.size());
    ASSERT_TRUE(term->m_follow[0]);
    ASSERT_STREQ("alias2", term->m_follow[0]->toString().c_str());
    ASSERT_FALSE(term->m_follow[0]->isCall());


    ASSERT_TRUE(Parse("@@@@alias@@@@", &macro));
    ASSERT_EQ(1, macro.GetCount()) << macro.Dump();
    ASSERT_STREQ("@@@@ alias @@@@", LexOut().c_str());

    ASSERT_TRUE(Parse("@@@@_@@@@;", &macro));
    ASSERT_EQ(0, macro.GetCount()) << macro.Dump();
    ASSERT_STREQ("@@@@ _ @@@@ ;", LexOut().c_str());


    ASSERT_TRUE(term = Parse("@@if(args)@@ := @@ [@$args] --> @@", &macro));
    ASSERT_EQ(1, macro.size()) << macro.Dump();
    ASSERT_STREQ("@@ if ( args ) @@ := @@ [ @$args ] --> @@", LexOut().c_str());

    ASSERT_EQ(4, term->m_follow.size());
    ASSERT_EQ(1, term->GetMacroId().size());
    ASSERT_STREQ("if(args)", term->GetMacroId()[0]->toString().c_str());
    //    ASSERT_TRUE(term->m_follow[0]->isCall());
    //    ASSERT_EQ(1, term->m_follow[0]->size());
    //    ASSERT_EQ("args", term->m_follow[0]->at(0).second->toString().c_str());


    id = macro.begin()->second[0]->GetMacroId();
    ASSERT_EQ(1, id.size()) << macro.begin()->second[0]->toString().c_str();
    ASSERT_STREQ("if", id[0]->m_text.c_str());


    ASSERT_TRUE(macro.GetMacro({"if"})) << macro.Dump();
    ASSERT_TRUE(macro.GetMacro({"if"})->Right());
    ASSERT_EQ(4, macro.GetMacro({"if"})->Right()->m_follow.size()) << macro.GetMacro({"if"})->Right()->toString().c_str();

    ASSERT_TRUE(term = Parse("@@if2(...)@@ := @@ [ __LINE__ ] --> @@", &macro));
    ASSERT_STREQ("@@ if2 ( ... ) @@ := @@ [ 1 ] --> @@", LexOut().c_str());

    ASSERT_EQ(4, term->m_follow.size());
    ASSERT_EQ(1, term->m_macro_id.size());
    ASSERT_TRUE(term->m_macro_id[0]);
    ASSERT_STREQ("if2(...)", term->m_macro_id[0]->toString().c_str());


    ASSERT_EQ(2, macro.size());
    ASSERT_TRUE(macro.GetMacro({"if2"}));
    ASSERT_TRUE(macro.GetMacro({"if2"})->Right());
    ASSERT_STREQ("[ 1 ] -->", macro.GetMacro({"if2"})->Right()->toString().c_str());




    ASSERT_TRUE(term = Parse("@@ func $name(arg= __LINE__ , ...) @@ := @@@ [ __LINE__ ] --> @@@", &macro));
    ASSERT_STREQ("@@ func $name ( arg = 1 , ... ) @@ :=  [ __LINE__ ] -->", LexOut().c_str());

    ASSERT_EQ(2, term->m_macro_id.size());
    ASSERT_TRUE(term->m_macro_id[0]);
    ASSERT_STREQ("func", term->m_macro_id[0]->toString().c_str());
    ASSERT_TRUE(term->m_macro_id[1]);
    ASSERT_STREQ("$name(arg=1, ...)", term->m_macro_id[1]->toString().c_str());


    ASSERT_EQ(3, macro.size());
    ASSERT_TRUE(macro.GetMacro(std::vector<std::string>({"func", "$"})));
    ASSERT_TRUE(macro.GetMacro(std::vector<std::string>({"func", "$"}))->Right());
    ASSERT_STREQ("@@@ [ __LINE__ ] --> @@@", macro.GetMacro(std::vector<std::string>({"func", "$"}))->Right()->toString().c_str());

#undef CREATE_TERM

}

TEST_F(MacroTest, MacroMacro) {
    MacroBuffer macro;
    ASSERT_EQ(0, macro.size());

    ASSERT_TRUE(Parse("@@alias replace@@ := @@replace@@", &macro));
    ASSERT_EQ(1, macro.GetCount()) << macro.Dump();
    ASSERT_TRUE(Parse("@@alias second@@ := @@second@@", &macro));
    ASSERT_EQ(2, macro.GetCount()) << macro.Dump();
    ASSERT_TRUE(Parse("@@text@@ := @@@text;\n text@@@", &macro));
    ASSERT_EQ(3, macro.GetCount()) << macro.Dump();
    ASSERT_TRUE(Parse("@@dsl@@ := @@@ @@m1@@ := @@mm@@;\n  @@m2@@ := @@mm@@;\n@@@", &macro));

    ASSERT_EQ(4, macro.GetCount()) << macro.Dump();
    ASSERT_TRUE(macro.GetMacro({"alias", "replace"}));
    TermPtr macro_replace = macro.GetMacro({"alias", "replace"});
    ASSERT_TRUE(macro.GetMacro({"alias", "second"}));
    TermPtr macro_second = macro.GetMacro({"alias", "second"});
    ASSERT_TRUE(macro.GetMacro({"text"}));
    TermPtr macro_text = macro.GetMacro({"text"});
    ASSERT_TRUE(macro.GetMacro({"dsl"}));
    TermPtr macro_dsl = macro.GetMacro({"dsl"});


    TermPtr term = Term::Create(parser::token_type::NAME, TermID::NAME, "alias");

    ASSERT_TRUE(macro.GetMacro({"alias", "replace"}));
    ASSERT_TRUE(macro.GetMacro({"alias", "second"}));


    BlockType buff;
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_replace)); // alias replace
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_second)); // alias second
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl


    buff.push_back(term); // alias 

    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_replace)); // alias replace
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_second)); // alias second
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl

    buff.push_back(term); // alias alias 

    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_replace)); // alias replace
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_second)); // alias second
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl

    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "second")); // alias alias second 

    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_replace)); // alias replace
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_second)); // alias second
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl

    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, "(")); // alias alias second (

    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_replace)); // alias replace
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_second)); // alias second
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl

    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, ")")); // alias alias second ( )

    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_replace)); // alias replace
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_second)); // alias second
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl

    buff.erase(buff.begin()); // alias second ( )

    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_replace)); // alias replace
    ASSERT_TRUE(MacroBuffer::IdentityMacro(buff, macro_second)); // alias second
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl



    //    ASSERT_TRUE(Parse("alias", &macro));
    //    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    //    ASSERT_STREQ("alias", ast->toString().c_str());

    ASSERT_TRUE(Parse("alias replace", &macro));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("replace", ast->m_text.c_str());

    ASSERT_TRUE(Parse("alias second", &macro));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("second", ast->m_text.c_str());



    ASSERT_EQ(4, macro.GetCount());
    ASSERT_FALSE(macro.GetMacro({"m1"})) << macro.Dump();
    ASSERT_FALSE(macro.GetMacro({"m2"})) << macro.Dump();

    //@todo Bug: https://github.com/rsashka/newlang/issues/22
    //    ASSERT_TRUE(Parse("dsl", &macro));
    //
    //    ASSERT_EQ(6, macro.GetCount());
    //    ASSERT_TRUE(macro.GetMacro({"m1"})) << macro.Dump();
    //    ASSERT_TRUE(macro.GetMacro({"m2"})) << macro.Dump();
}

TEST_F(MacroTest, Simple) {
    MacroBuffer macro;
    ASSERT_EQ(0, macro.size());

    ASSERT_NO_THROW(Parse("@@alias@@ ::= @@replace@@", &macro));
    ASSERT_EQ(1, macro.GetCount()) << macro.Dump();

    ASSERT_NO_THROW(Parse("@@alias()@@ := @@replace@@", &macro));
    ASSERT_EQ(1, macro.GetCount()) << macro.Dump();

    ASSERT_ANY_THROW(Parse("@@alias(...)@@ ::= @@error@@", &macro));
    ASSERT_EQ(1, macro.GetCount()) << macro.Dump();

    ASSERT_NO_THROW(Parse("@@second(...)@@ := @@second2(@$#, @$...)@@", &macro));
    ASSERT_EQ(2, macro.GetCount()) << macro.Dump();

    ASSERT_NO_THROW(Parse("@@second@@ := @@second2(@$#, @$...)@@", &macro));
    ASSERT_EQ(2, macro.GetCount()) << macro.Dump();

    ASSERT_NO_THROW(Parse("@@text(...)@@ := @@text1(@$#, @$*)@@", &macro));
    ASSERT_EQ(3, macro.GetCount()) << macro.Dump();
    ASSERT_NO_THROW(Parse("@@dsl@@ := @@@\n @@m1@@ := @@mm@@;\n @@m2@@ := @@mm@@;\n@@@", &macro));

    ASSERT_EQ(4, macro.GetCount()) << macro.Dump();
    ASSERT_TRUE(macro.GetMacro({"alias"}));
    TermPtr macro_alias = macro.GetMacro({"alias"});
    ASSERT_TRUE(macro.GetMacro({"second"}));
    TermPtr macro_second = macro.GetMacro({"second"});
    ASSERT_TRUE(macro.GetMacro({"text"}));
    TermPtr macro_text = macro.GetMacro({"text"});
    ASSERT_TRUE(macro.GetMacro({"dsl"}));
    TermPtr macro_dsl = macro.GetMacro({"dsl"});


    TermPtr term = Term::Create(parser::token_type::NAME, TermID::NAME, "alias");

    ASSERT_TRUE(macro.GetMacro({"alias"}));
    ASSERT_TRUE(macro.GetMacro({"second"}));


    BlockType buff; // 
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_alias)); // alias
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_second)); // second(...)
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text(...)
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl


    buff.push_back(term); // alias 

    ASSERT_TRUE(MacroBuffer::IdentityMacro(buff, macro_alias)); // alias
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_second)); // second(...)
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text(...)
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl

    buff.push_back(term); // alias alias 

    ASSERT_TRUE(MacroBuffer::IdentityMacro(buff, macro_alias)); // alias
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_second)); // second(...)
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text(...)
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl

    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "second")); // alias alias second 

    ASSERT_TRUE(MacroBuffer::IdentityMacro(buff, macro_alias)); // alias
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_second)); // second(...)
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text(...)
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl

    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, "(")); // alias alias second (

    ASSERT_TRUE(MacroBuffer::IdentityMacro(buff, macro_alias)); // alias
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_second)); // second(...)
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text(...)
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl

    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "arg")); // alias alias second ( arg

    ASSERT_TRUE(MacroBuffer::IdentityMacro(buff, macro_alias)); // alias
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_second)); // second(...)
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text(...)
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl()

    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, ")")); // alias alias second ( arg )

    ASSERT_TRUE(MacroBuffer::IdentityMacro(buff, macro_alias)); // alias
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_second)); // second(...)
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text(...)
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl

    buff.erase(buff.begin()); // alias second ( arg )

    ASSERT_TRUE(MacroBuffer::IdentityMacro(buff, macro_alias)); // alias
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_second)); // second(...)
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text(...)
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl

    buff.erase(buff.begin()); // second ( arg )

    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_alias)); // alias
    ASSERT_TRUE(MacroBuffer::IdentityMacro(buff, macro_second)); // second(...)
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text(...)
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl


    ASSERT_NO_THROW(Parse("@alias", &macro));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("replace", ast->toString().c_str());

    ASSERT_NO_THROW(Parse("alias", &macro));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("replace", ast->m_text.c_str());

    ASSERT_NO_THROW(Parse("second()", &macro));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("second2", ast->m_text.c_str());

    ASSERT_NO_THROW(Parse("@second()", &macro));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("second2", ast->m_text.c_str());


    ASSERT_NO_THROW(Parse("second(123)", &macro));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_EQ(2, ast->size()) << LexOut();
    ASSERT_STREQ("second2(1, 123)", ast->toString().c_str());

    ASSERT_NO_THROW(Parse("@second(123, 456)", &macro));
    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
    ASSERT_STREQ("second2(2, 123, 456)", ast->toString().c_str());

    ASSERT_ANY_THROW(Parse("second", &macro));
    ASSERT_ANY_THROW(Parse("@second", &macro));

//    ASSERT_ANY_THROW(Parse("text", &macro));
//    ASSERT_NO_THROW(Parse("text()", &macro));
//    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
//    ASSERT_STREQ("text1(0, (,))", ast->toString().c_str());
//    
//    ASSERT_NO_THROW(Parse("text(123)", &macro));
//    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
//    ASSERT_STREQ("text1(1, (123,) )", ast->toString().c_str());
//
//    ASSERT_NO_THROW(Parse("text(123, 456)", &macro));
//    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
//    ASSERT_STREQ("text1(2, (123, 456,))", ast->toString().c_str());

    ASSERT_EQ(4, macro.GetCount());
    ASSERT_FALSE(macro.GetMacro({"m1"})) << macro.Dump();
    ASSERT_FALSE(macro.GetMacro({"m2"})) << macro.Dump();

    //@todo Bug: https://github.com/rsashka/newlang/issues/22

//    ASSERT_NO_THROW(
//            ASSERT_TRUE(Parse("dsl", &macro));
//            );
//
//
//    ASSERT_EQ(6, macro.GetCount());
//    ASSERT_TRUE(macro.GetMacro({"m1"})) << macro.Dump();
//    ASSERT_TRUE(macro.GetMacro({"m2"})) << macro.Dump();
}


//TEST_F(MacroTest, Multiple) {
//    MacroBuffer macro;
//    ASSERT_EQ(0, macro.size());
//
//    ASSERT_NO_THROW(Parse("@@alias@@ := @@replace@@", &macro));
//    ASSERT_EQ(1, macro.GetCount()) << macro.Dump();
//    ASSERT_NO_THROW(Parse("@@alias second(...)@@ := @@second(@$#, @$*)@@", &macro));
//    ASSERT_EQ(2, macro.GetCount()) << macro.Dump();
//    ASSERT_NO_THROW(Parse("@@text(...)@@ := @@@text1(@$#, @$*);\n text1@@@", &macro));
//    ASSERT_EQ(3, macro.GetCount()) << macro.Dump();
//    ASSERT_NO_THROW(Parse("@@dsl()@@ := @@@ @@m1@@ := @@mm@@;\n  @@m2@@ := @@mm@@;\n@@@", &macro));
//
//    ASSERT_EQ(4, macro.GetCount()) << macro.Dump();
//    ASSERT_TRUE(macro.GetMacro({"alias"}));
//    TermPtr macro_alias = macro.GetMacro({"alias"});
//    ASSERT_TRUE(macro.GetMacro({"alias", "second"}));
//    TermPtr macro_second = macro.GetMacro({"alias", "second"});
//    ASSERT_TRUE(macro.GetMacro({"text"}));
//    TermPtr macro_text = macro.GetMacro({"text"});
//    ASSERT_TRUE(macro.GetMacro({"dsl"}));
//    TermPtr macro_dsl = macro.GetMacro({"dsl"});
//
//
//    TermPtr term = Term::Create(parser::token_type::NAME, TermID::NAME, "alias");
//
//    ASSERT_TRUE(macro.GetMacro({"alias"}));
//    ASSERT_TRUE(macro.GetMacro({"alias", "second"}));
//
//
//    BlockType buff; // 
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_alias)); // alias
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_second)); // alias second(...)
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text(...)
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl()
//
//
//    buff.push_back(term); // alias 
//
//    ASSERT_TRUE(MacroBuffer::IdentityMacro(buff, macro_alias)); // alias
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_second)); // alias second(...)
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text(...)
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl()
//
//    buff.push_back(term); // alias alias 
//
//    ASSERT_TRUE(MacroBuffer::IdentityMacro(buff, macro_alias)); // alias
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_second)); // alias second(...)
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text(...)
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl()
//
//    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "second")); // alias alias second 
//
//    ASSERT_TRUE(MacroBuffer::IdentityMacro(buff, macro_alias)); // alias
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_second)); // alias second(...)
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text(...)
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl()
//
//    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, "(")); // alias alias second (
//
//    ASSERT_TRUE(MacroBuffer::IdentityMacro(buff, macro_alias)); // alias replace(...)
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_second)); // alias second(...)
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text(...)
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl()
//
//    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "arg")); // alias alias second ( arg
//
//    ASSERT_TRUE(MacroBuffer::IdentityMacro(buff, macro_alias)); // alias replace(...)
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_second)); // alias second(...)
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text(...)
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl()
//
//    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, ")")); // alias alias second ( arg )
//
//    ASSERT_TRUE(MacroBuffer::IdentityMacro(buff, macro_alias)); // alias replace(...)
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_second)); // alias second(...)
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text(...)
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl()
//
//    buff.erase(buff.begin()); // alias second ( arg )
//
//    ASSERT_TRUE(MacroBuffer::IdentityMacro(buff, macro_alias)); // alias replace
//    ASSERT_TRUE(MacroBuffer::IdentityMacro(buff, macro_second)); // alias second(...)
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_text)); // text(...)
//    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_dsl)); // dsl()
//
//
//    ASSERT_TRUE(Parse("@alias", &macro));
//    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
//    ASSERT_STREQ("replace", ast->toString().c_str());
//
//    ASSERT_TRUE(Parse("alias", &macro));
//    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
//    ASSERT_STREQ("replace", ast->m_text.c_str());
//
//    ASSERT_TRUE(Parse("alias second", &macro));
//    ASSERT_EQ(TermID::NAME, ast->getTermID()) << newlang::toString(ast->getTermID());
//    ASSERT_STREQ("second", ast->m_text.c_str());
//
//
//
//    ASSERT_EQ(4, macro.GetCount());
//    ASSERT_FALSE(macro.GetMacro({"m1"})) << macro.Dump();
//    ASSERT_FALSE(macro.GetMacro({"m2"})) << macro.Dump();
//
//    //@todo Bug: https://github.com/rsashka/newlang/issues/22
//    //    ASSERT_TRUE(Parse("dsl", &macro));
//    //
//    //    ASSERT_EQ(6, macro.GetCount());
//    //    ASSERT_TRUE(macro.GetMacro({"m1"})) << macro.Dump();
//    //    ASSERT_TRUE(macro.GetMacro({"m2"})) << macro.Dump();
//}

TEST_F(MacroTest, MacroAlias) {
    MacroBuffer macro;
    ASSERT_EQ(0, macro.size());

    ASSERT_ANY_THROW(Parse("@@@@ macro @@  @@@@"));
    ASSERT_ANY_THROW(Parse("@@@@ @@  macro  @@@@"));
    ASSERT_ANY_THROW(Parse("@@  macro @@@@  @@"));
    ASSERT_ANY_THROW(Parse("@@  @@@@  macro  @@"));
    ASSERT_ANY_THROW(Parse("@@  @macro  @@"));
    ASSERT_ANY_THROW(Parse("@@  @$macro  @@"));

    ASSERT_TRUE(Parse("@@@@ alias  @@@@", &macro));
    ASSERT_EQ(0, macro.size());
    ASSERT_TRUE(Parse("@@@@ alias $alias2 @@@@", &macro));
    ASSERT_EQ(0, macro.size());

    ASSERT_TRUE(Parse("@@alias@@ ::= @@replace@@", &macro));
    ASSERT_TRUE(Parse("@@alias2@@ := @@alias@@", &macro));
    ASSERT_TRUE(Parse("@@fail@@ := @@fail@@", &macro));

    ASSERT_EQ(3, macro.size()) << macro.Dump();
    ASSERT_TRUE(macro.GetMacro({"alias"}));
    TermPtr macro_alias = macro.GetMacro({"alias"});
    ASSERT_TRUE(macro_alias);
    ASSERT_EQ(TermID::MACRO_SEQ, macro_alias->getTermID());
    ASSERT_TRUE(macro_alias->Right());
    ASSERT_TRUE(macro_alias->Right()->m_follow.size()) << macro_alias->Right()->toString();
    ASSERT_STREQ("replace", macro_alias->Right()->m_follow[0]->m_text.c_str());

    ASSERT_TRUE(macro.GetMacro({"alias2"})) << macro.Dump();
    TermPtr macro_alias2 = macro.GetMacro({"alias2"});
    ASSERT_TRUE(macro_alias2);
    ASSERT_EQ(TermID::MACRO_SEQ, macro_alias2->getTermID());
    ASSERT_STREQ("alias", macro_alias2->Right()->m_follow[0]->m_text.c_str());

    ASSERT_TRUE(macro.GetMacro({"fail"})) << macro.Dump();
    TermPtr macro_fail = macro.GetMacro({"fail"});
    ASSERT_TRUE(macro_fail);
    ASSERT_EQ(TermID::MACRO_SEQ, macro_fail->getTermID());
    ASSERT_STREQ("fail", macro_fail->Right()->m_follow[0]->m_text.c_str());



    TermPtr term = Term::Create(parser::token_type::NAME, TermID::NAME, "alias");

    ASSERT_TRUE(macro.map::find(term->m_text) != macro.end());

    BlockType vals = macro.map::find(term->m_text)->second;
    ASSERT_EQ(1, vals.size());

    BlockType buff;
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_alias));
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_alias2));
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_fail));


    buff.push_back(term);

    ASSERT_TRUE(MacroBuffer::IdentityMacro(buff, macro_alias));
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_alias2));
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_fail));


    term = Term::Create(parser::token_type::NAME, TermID::NAME, "alias");
    buff.push_back(term);


    ASSERT_TRUE(MacroBuffer::IdentityMacro(buff, macro_alias));
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_alias2));
    ASSERT_FALSE(MacroBuffer::IdentityMacro(buff, macro_fail));


    MacroBuffer::MacroArgsType macro_args;

    ASSERT_EQ(1, MacroBuffer::ExtractArgs(buff, macro_alias, macro_args));
    ASSERT_EQ(3, macro_args.size()) << MacroBuffer::Dump(macro_args);

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

TEST_F(MacroTest, MacroArgs) {

    MacroBuffer macro;
    BlockType buffer;

    BlockType vect;
    MacroBuffer::iterator iter;
    TermPtr macro_alias1;
    //    
    //    ASSERT_TRUE(Parse("@@alias@@replace1@@;@@alias2@@replace2@@", &macro));
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
    //    ASSERT_TRUE(macro_alias1->getTermID() == TermID::MACRO_DEF) << macro_alias1->toString().c_str();
    //    ASSERT_TRUE(macro_alias1->Right());
    //    ASSERT_EQ(1, macro_alias1->Right()->m_follow.size());
    //    ASSERT_STREQ("replace1", macro_alias1->Right()->m_follow[0]->m_text.c_str());


    macro.clear();
    ASSERT_EQ(0, macro.GetCount());

    ASSERT_NO_THROW(Parse("@@alias@@ := @@ replace1 @@", &macro)) << macro.Dump();
    ASSERT_EQ(1, macro.GetCount()) << macro.Dump();

    ASSERT_ANY_THROW(Parse("@@alias@@ ::= @@replace2@@", &macro)) << macro.Dump();
    ASSERT_EQ(1, macro.GetCount()) << macro.Dump();
    ASSERT_NO_THROW(Parse("@@alias@@ = @@replace3@@", &macro)) << macro.Dump();
    ASSERT_EQ(1, macro.GetCount()) << macro.Dump();

    ASSERT_ANY_THROW(Parse("@@alias(arg)@@ ::= @@ replace2(@$arg) @@", &macro)) << macro.Dump();


    ASSERT_NO_THROW(Parse("@@@@ alias @@@@", &macro)) << macro.Dump();
    ASSERT_EQ(0, macro.GetCount()) << macro.Dump();

    ASSERT_NO_THROW(Parse("@@alias(arg, ... )@@ := @@ replace2(@$arg) @@", &macro)) << macro.Dump();
    ASSERT_EQ(1, macro.GetCount()) << macro.Dump();
    ASSERT_ANY_THROW(Parse("@@alias(arg, ... )@@ ::= @@ replace3(@$arg) @@", &macro)) << macro.Dump();
    ASSERT_EQ(1, macro.GetCount()) << macro.Dump();
    ASSERT_NO_THROW(Parse("@@alias(arg, ... )@@ = @@ replace4(@$arg) @@", &macro)) << macro.Dump();
    ASSERT_EQ(1, macro.GetCount()) << macro.Dump();

    ASSERT_NO_THROW(Parse("@@alias3(...)@@ := @@replace3(@$#, @$...)@@", &macro)) << macro.Dump();
    ASSERT_EQ(2, macro.GetCount()) << macro.Dump();

    ASSERT_ANY_THROW(Parse("@@alias3(...)@@ ::= @@ error double macros@@", &macro)) << macro.Dump();
    ASSERT_EQ(2, macro.GetCount()) << macro.Dump();

    //    ASSERT_NO_THROW(Parse("@@alias(arg) second@@ ::= @@replace3(@$*)@@;", &macro)) << macro.Dump();
    //    TermPtr test = macro.GetMacro({"alias", "second"});
    //    ASSERT_TRUE(test) << macro.Dump();
    //    ASSERT_EQ(TermID::MACRO_SEQ, test->getTermID()) << test->toString().c_str();
    //    ASSERT_EQ(2, macro.GetCount()) << macro.Dump();
    //
    //    std::vector<std::string> id = MacroBuffer::GetMacroId(test);
    //    ASSERT_EQ(2, id.size()) << test->toString().c_str();
    //    ASSERT_STREQ("alias", id[0].c_str());
    //    ASSERT_STREQ("second", id[1].c_str());


    ASSERT_NO_THROW(Parse("@@macro(arg, ... )@@ ::= @@@ 3*@$arg @@@", &macro)) << macro.Dump();
    ASSERT_EQ(3, macro.GetCount()) << macro.Dump();

    iter = macro.map::find("alias");
    ASSERT_TRUE(iter != macro.end());

    vect = iter->second;

    ASSERT_EQ(1, vect.size());

    macro_alias1 = vect[0];
    ASSERT_TRUE(macro_alias1);
    ASSERT_STREQ("alias", macro_alias1->m_text.c_str());
    ASSERT_FALSE(macro_alias1->isCall());
    ASSERT_TRUE(macro_alias1->getTermID() == TermID::MACRO_SEQ) << macro_alias1->toString().c_str();
    ASSERT_TRUE(macro_alias1->Right());
    ASSERT_EQ(4, macro_alias1->Right()->m_follow.size());
    ASSERT_EQ(1, macro_alias1->Right()->GetMacroId().size());
    ASSERT_EQ(4, macro_alias1->Right()->m_follow.size());
    ASSERT_STREQ("replace4", macro_alias1->Right()->m_follow[0]->m_text.c_str());


    //    TermPtr macro_alias2 = vect[1];
    //    ASSERT_TRUE(macro_alias2);
    //    ASSERT_STREQ("alias", macro_alias2->m_text.c_str());
    //    ASSERT_TRUE(macro_alias2->getTermID() == TermID::MACRO_SEQ) << macro_alias2->toString().c_str();
    //    ASSERT_EQ(4, macro_alias2->m_follow.size());
    //    ASSERT_TRUE(macro_alias2->Right());
    //    ASSERT_EQ(4, macro_alias2->Right()->m_follow.size()) << macro_alias2->Right()->m_follow[0]->m_text.c_str();
    //    ASSERT_STREQ("replace2", macro_alias2->Right()->m_follow[0]->m_text.c_str());
    //    ASSERT_STREQ("(", macro_alias2->Right()->m_follow[1]->m_text.c_str());
    //    ASSERT_STREQ("@$arg", macro_alias2->Right()->m_follow[2]->m_text.c_str());
    //    ASSERT_STREQ(")", macro_alias2->Right()->m_follow[3]->m_text.c_str());

    //    TermPtr macro_alias3 = vect[2];
    //    ASSERT_TRUE(macro_alias3);
    //    ASSERT_STREQ("alias", macro_alias3->m_text.c_str());
    //    ASSERT_TRUE(macro_alias3->getTermID() == TermID::MACRO_SEQ) << macro_alias3->toString().c_str();
    //    ASSERT_EQ(5, macro_alias3->m_follow.size());
    //    ASSERT_STREQ("(", macro_alias3->m_follow[1]->m_text.c_str());
    //    ASSERT_TRUE(macro_alias3->Right());
    //    ASSERT_EQ(4, macro_alias3->Right()->m_follow.size());
    //    ASSERT_STREQ("replace3", macro_alias3->Right()->m_follow[0]->m_text.c_str());
    //    ASSERT_STREQ("(", macro_alias3->Right()->m_follow[1]->m_text.c_str());
    //    ASSERT_STREQ("@$*", macro_alias3->Right()->m_follow[2]->m_text.c_str());
    //    ASSERT_STREQ(")", macro_alias3->Right()->m_follow[3]->m_text.c_str());

    //    ASSERT_EQ(macro_alias1.get(), macro.GetMacro({"alias"}).get()) << macro.Dump();
    //    //    ASSERT_EQ(macro_alias2.get(), macro.GetMacro({"alias", "second"}).get()) << macro.Dump();
    //    //    ASSERT_EQ(macro_alias3.get(), macro.GetMacro({"alias", "(", "$", ")", "second"}).get()) << macro.Dump();
    //
    //
    iter = macro.map::find("macro"); // Это поиск для map - возвращает итератор
    ASSERT_TRUE(iter != macro.end());

    vect = iter->second;
    ASSERT_EQ(1, vect.size());
    TermPtr macro_macro1 = vect[0];
    ASSERT_TRUE(macro_macro1);
    ASSERT_STREQ("macro", macro_macro1->m_text.c_str());
    ASSERT_EQ(macro_macro1.get(), macro.GetMacro({"macro"}).get()); // Поиск по MacroID и возврат TermPtr
    ASSERT_TRUE(macro_macro1->Right());
    ASSERT_TRUE(macro_macro1->Right()->getTermID() == TermID::MACRO_STR) << macro_macro1->toString().c_str();
    //

    BlockType buff;
    MacroBuffer::MacroArgsType macro_args;

    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias1, macro_args));
    //    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias2, macro_args));
    //    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias3, macro_args));
    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_macro1, macro_args));

    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "alias"));

    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias1, macro_args)) << macro_alias1->toString().c_str();

    //    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias2, macro_args));
    //    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias3, macro_args));
    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_macro1, macro_args)) << macro_macro1->toString().c_str();


    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, "("));

    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias1, macro_args));

    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, ")"));

    size_t count;
    ASSERT_NO_THROW(count = MacroBuffer::ExtractArgs(buff, macro_alias1, macro_args));
    ASSERT_EQ(3, count);

    buff.erase(buff.end());

    //    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias2, macro_args));
    //    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias3, macro_args));
    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_macro1, macro_args));

    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "value"));

    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias1, macro_args));

    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, ")"));

    ASSERT_EQ(4, buff.size());
    ASSERT_NO_THROW(count = MacroBuffer::ExtractArgs(buff, macro_alias1, macro_args)) << MacroBuffer::Dump(buff);
    ASSERT_EQ(4, count);
    buff.erase(buff.end());

    //    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias2, macro_args));
    //    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias3, macro_args));
    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_macro1, macro_args));


    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, ","));


    //    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias2, macro_args));
    //    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias3, macro_args));
    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_macro1, macro_args));


    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "value2"));

    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "value3"));

    ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_macro1, macro_args));


    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, ")"));


    ASSERT_NO_THROW(
            ASSERT_EQ(7, MacroBuffer::ExtractArgs(buff, macro_alias1, macro_args));
            );
    ASSERT_EQ(6, macro_args.size()) << MacroBuffer::Dump(macro_args);

    //        ASSERT_EQ(7, MacroBuffer::ExtractArgs(buff, macro_alias2, macro_args));
    //        ASSERT_EQ(4, macro_args.size()) << MacroBuffer::Dump(macro_args);

    ASSERT_EQ(7, buff.size());
    ASSERT_NO_THROW(
            ASSERT_EQ(7, MacroBuffer::ExtractArgs(buff, macro_macro1, macro_args));
            );
    ASSERT_EQ(6, macro_args.size()) << MacroBuffer::Dump(macro_args);

    //        ASSERT_ANY_THROW(MacroBuffer::ExtractArgs(buff, macro_alias3, macro_args));



    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, ";"));


    ASSERT_NO_THROW(
            ASSERT_EQ(7, MacroBuffer::ExtractArgs(buff, macro_alias1, macro_args));
            );

    ASSERT_EQ(6, macro_args.size()) << MacroBuffer::Dump(macro_args);

    ASSERT_EQ(8, buff.size()) << MacroBuffer::Dump(buff);
    ASSERT_NO_THROW(
            ASSERT_EQ(7, MacroBuffer::ExtractArgs(buff, macro_macro1, macro_args));
            );
    ASSERT_EQ(6, macro_args.size()) << MacroBuffer::Dump(macro_args);


    //    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "last_term"));
    //
    //    ASSERT_EQ(1, MacroBuffer::ExtractArgs(buff, macro_alias1, macro_args));
    //    ASSERT_EQ(0, macro_args.size()) << MacroBuffer::Dump(macro_args);
    //
    //    BlockType res = MacroBuffer::ExpandMacros(macro_alias1, macro_args);
    //    ASSERT_EQ(1, res.size());
    //    ASSERT_STREQ("replace1", res[0]->m_text.c_str());


    //        ASSERT_EQ(7, MacroBuffer::ExtractArgs(buff, macro_alias2, macro_args));
    //        ASSERT_EQ(4, macro_args.size()) << MacroBuffer::Dump(macro_args);
    //    
    //        res = MacroBuffer::ExpandMacros(macro_alias2, macro_args);
    //        ASSERT_EQ(4, res.size());
    //        ASSERT_STREQ("replace2", res[0]->m_text.c_str());
    //        ASSERT_STREQ("(", res[1]->m_text.c_str());
    //        ASSERT_STREQ("value", res[2]->m_text.c_str()) << MacroBuffer::Dump(macro_args);
    //        ASSERT_STREQ(")", res[3]->m_text.c_str());
    //    
    //        // Нет анализаи на соотеветстви макроса, только извлечение значений шаблона
    //        ASSERT_EQ(8, MacroBuffer::ExtractArgs(buff, macro_alias3, macro_args)) << MacroBuffer::Dump(macro_args);
    //        ASSERT_EQ(4, macro_args.size()) << MacroBuffer::Dump(macro_args);

    //        res = MacroBuffer::ExpandMacros(macro_alias3, macro_args);
    //        ASSERT_EQ(7, res.size());
    //        ASSERT_STREQ("replace3", res[0]->m_text.c_str());
    //        ASSERT_STREQ("(", res[1]->m_text.c_str());
    //        ASSERT_STREQ("value", res[2]->m_text.c_str()) << MacroBuffer::Dump(macro_args);
    //        ASSERT_STREQ(",", res[3]->m_text.c_str()) << MacroBuffer::Dump(macro_args);
    //        ASSERT_STREQ("value2", res[4]->m_text.c_str()) << MacroBuffer::Dump(macro_args);
    //        ASSERT_STREQ("value3", res[5]->m_text.c_str()) << MacroBuffer::Dump(macro_args);
    //        ASSERT_STREQ(")", res[6]->m_text.c_str());


    buff.clear();
    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "macro"));
    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, "("));
    buff.push_back(Term::Create(parser::token_type::NUMBER, TermID::NUMBER, "5"));
    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, ")"));
    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, ";"));

    TermPtr macro_macro = macro.GetMacro({"macro"});
    ASSERT_TRUE(macro_macro);

    ASSERT_NO_THROW(
            ASSERT_EQ(4, MacroBuffer::ExtractArgs(buff, macro_macro, macro_args)) << MacroBuffer::Dump(macro_args);
            );
    ASSERT_EQ(5, macro_args.size()) << MacroBuffer::Dump(macro_args);

    std::string str = MacroBuffer::ExpandString(macro_macro, macro_args);
    ASSERT_STREQ(" 3*5  ", str.c_str());


    buff.clear();
    buff.push_back(Term::Create(parser::token_type::NAME, TermID::NAME, "alias3"));
    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, "("));
    buff.push_back(Term::Create(parser::token_type::NUMBER, TermID::NUMBER, "5"));
    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, ")"));
    buff.push_back(Term::Create(parser::token_type::SYMBOL, TermID::SYMBOL, ";"));

    TermPtr macro_alias3 = macro.GetMacro({"alias3"});
    ASSERT_TRUE(macro_alias3);

    ASSERT_STREQ("alias3", macro_alias3->m_text.c_str());
    ASSERT_FALSE(macro_alias3->isCall());
    ASSERT_TRUE(macro_alias3->getTermID() == TermID::MACRO_SEQ) << macro_alias3->toString().c_str();
    ASSERT_TRUE(macro_alias3->Right());
    ASSERT_EQ(6, macro_alias3->Right()->m_follow.size());
    ASSERT_EQ(1, macro_alias3->Right()->GetMacroId().size());
    ASSERT_EQ(6, macro_alias3->Right()->m_follow.size());
    ASSERT_STREQ("replace3", macro_alias3->Right()->m_follow[0]->m_text.c_str());


    ASSERT_NO_THROW(
            ASSERT_EQ(4, MacroBuffer::ExtractArgs(buff, macro_alias3, macro_args)) << MacroBuffer::Dump(macro_args);
            );
    ASSERT_EQ(4, macro_args.size()) << MacroBuffer::Dump(macro_args);

    auto iter_arg = macro_args.begin();
    ASSERT_TRUE(iter_arg != macro_args.end()) << MacroBuffer::Dump(macro_args);

    ASSERT_STREQ("@$#", iter_arg->first.c_str());
    ASSERT_EQ(1, iter_arg->second.size());
    ASSERT_STREQ("1", iter_arg->second.at(0)->m_text.c_str()) << MacroBuffer::Dump(macro_args);

    iter_arg++;
    ASSERT_TRUE(iter_arg != macro_args.end());

    ASSERT_STREQ("@$*", iter_arg->first.c_str());
    ASSERT_STREQ("( 5 , )", MacroBuffer::Dump(iter_arg->second).c_str());

    iter_arg++;
    ASSERT_TRUE(iter_arg != macro_args.end());

    ASSERT_STREQ("@$...", iter_arg->first.c_str());
    ASSERT_EQ(1, iter_arg->second.size());

    iter_arg++;
    ASSERT_TRUE(iter_arg != macro_args.end());

    ASSERT_STREQ("@$1", iter_arg->first.c_str());
    ASSERT_EQ(1, iter_arg->second.size());

    iter_arg++;
    ASSERT_TRUE(iter_arg == macro_args.end());

    //    ASSERT_EQ(1, macro_args[1].size());
    //    ASSERT_STREQ("@$...", (macro_args.begin() + 1)->first.c_str());

    //    ASSERT_EQ(1, macro_args[0].size());
    //    ASSERT_STREQ("@$1", macro_args[0][0]->name(0).c_str());
    //    
    //    ASSERT_EQ(1, macro_args[1].size());
    //    ASSERT_STREQ("@$1", macro_args[1][0]->name(0).c_str());
    //    ASSERT_EQ(1, macro_args[1].size());
    //    
    //    ASSERT_EQ(1, macro_args[2].size());
    //    ASSERT_STREQ("@$#", macro_args[2][0]->name(0).c_str());
    //    ASSERT_EQ(1, macro_args[2].size());
    //    
    //    ASSERT_EQ(1, macro_args[3].size());
    //    ASSERT_STREQ("@$*", macro_args[3][0]->name(0).c_str());
    //    ASSERT_EQ(1, macro_args[3].size());

    // alias3(5) -> replace3(@$#, @$*) т.е replace3(1,5)
    BlockType blk = MacroBuffer::ExpandMacros(macro_alias3, macro_args);
    ASSERT_EQ(6, blk.size()) << MacroBuffer::Dump(blk).c_str();
    ASSERT_STREQ("replace3", blk[0]->m_text.c_str()) << macro_alias3->Right()->toString();
    ASSERT_STREQ("(", blk[1]->m_text.c_str()) << macro_alias3->Right()->toString();
    ASSERT_STREQ("1", blk[2]->m_text.c_str()) << macro_alias3->Right()->toString();
    ASSERT_STREQ(",", blk[3]->m_text.c_str()) << macro_alias3->Right()->toString();
    ASSERT_STREQ("5", blk[4]->m_text.c_str()) << macro_alias3->Right()->toString();
    ASSERT_STREQ(")", blk[5]->m_text.c_str()) << macro_alias3->Right()->toString();

    //    body = "@macro(11, ...)";
    //    args = Parser::ParseMacroArgs(body);
    //    ASSERT_EQ(2, args.size());
    //    ASSERT_STREQ("11", args[0].c_str());
    //    ASSERT_STREQ("...", args[1].c_str());
    //
    //    body = "@return(...)    --@$*--";
    //    args = Parser::ParseMacroArgs(body);
    //    ASSERT_EQ(1, args.size());
    //    ASSERT_STREQ("...", args[0].c_str());
    //
    //    ASSERT_ANY_THROW(
    //            body = "@macro(,)";
    //            args = Parser::ParseMacroArgs(body);
    //            );
    //    ASSERT_ANY_THROW(
    //            body = "@macro( , )";
    //            args = Parser::ParseMacroArgs(body);
    //            );
    //    ASSERT_ANY_THROW(
    //            body = "@macro(,,)";
    //            args = Parser::ParseMacroArgs(body);
    //            );
    //
    //    body = "@macro)";
    //    args = Parser::ParseMacroArgs(body);
    //    ASSERT_EQ(0, args.size());
    //
    //    body = "@macro\n";
    //    args = Parser::ParseMacroArgs(body);
    //    ASSERT_EQ(0, args.size());
    //
    //    body = "@macro)";
    //    args = Parser::ParseMacroArgs(body);
    //    ASSERT_EQ(0, args.size());
    //
    //    body = "@@macro()";
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

//TEST_F(MacroTest, MacroExpand) {
//
//    std::string macro = "@macro 12345";
//    std::string body = "@macro";
//    std::string result = MacroBuffer::ExpandMacro(macro, body);
//    ASSERT_STREQ("12345", result.c_str());
//
//    body = "@macro @macro";
//    result = MacroBuffer::ExpandMacro(macro, body);
//    ASSERT_STREQ("12345 12345", result.c_str());
//
//    body = "@macro @macro @macro";
//    result = MacroBuffer::ExpandMacro(macro, body);
//    ASSERT_STREQ("12345 12345 12345", result.c_str());
//
//    macro = "@macro() 12345";
//    body = "@macro @macro @macro";
//    result = MacroBuffer::ExpandMacro(macro, body);
//    ASSERT_STREQ("@macro @macro @macro", result.c_str());
//
//    macro = "@macro()12345";
//    body = "@macro() @macro() @macro";
//    result = MacroBuffer::ExpandMacro(macro, body);
//    ASSERT_STREQ("12345 12345 @macro", result.c_str());
//
//    macro = "@macro()12345";
//    body = "@macro(88) @macro(99) @macro";
//    result = MacroBuffer::ExpandMacro(macro, body);
//    ASSERT_STREQ("12345 12345 @macro", result.c_str());
//
//
//    macro = "@macro(arg)@$arg";
//    body = "@macro(88)";
//    result = MacroBuffer::ExpandMacro(macro, body);
//    ASSERT_STREQ("88", result.c_str());
//
//    macro = "@macro(arg)no arg @$arg";
//    body = "@macro(99)";
//    result = MacroBuffer::ExpandMacro(macro, body);
//    ASSERT_STREQ("no arg 99", result.c_str());
//
//    macro = "@macro(arg)  no arg @$arg no arg";
//    body = "@macro(88) @macro(99)";
//    result = MacroBuffer::ExpandMacro(macro, body);
//    ASSERT_STREQ("  no arg 88 no arg   no arg 99 no arg", result.c_str());
//
//    macro = "@macro(arg1,arg2)  @$arg1 arg @$arg2 @$arg2";
//    body = "@macro(88,99)";
//    result = MacroBuffer::ExpandMacro(macro, body);
//    ASSERT_STREQ("  88 arg 99 99", result.c_str());
//
//    macro = "@macro(arg1,arg2)  @$arg1 @$arg2 @$arg2";
//    body = "@macro(1,2) @macro(3,44)";
//    result = MacroBuffer::ExpandMacro(macro, body);
//    ASSERT_STREQ("  1 2 2   3 44 44", result.c_str());
//
//    macro = "@macro(arg1,arg2)  @$1 @$2 @$1";
//    body = "@macro(1,2) @macro(3,44)";
//    result = MacroBuffer::ExpandMacro(macro, body);
//    ASSERT_STREQ("  1 2 1   3 44 3", result.c_str());
//
//    macro = "@macro(arg1,arg2)@$*";
//    body = "@macro(1,2)";
//    result = MacroBuffer::ExpandMacro(macro, body);
//    ASSERT_STREQ("1,2", result.c_str());
//
//    macro = "@macro(arg1,arg2)@$* @$1 @$arg2@$*";
//    body = "@macro(1,2)";
//    result = MacroBuffer::ExpandMacro(macro, body);
//    ASSERT_STREQ("1,2 1 21,2", result.c_str());
//
//    macro = "@macro(arg1,arg2)@$* @$1 @$arg2@$*";
//    body = "@macro(1,2)@macro(1,2)";
//    result = MacroBuffer::ExpandMacro(macro, body);
//    ASSERT_STREQ("1,2 1 21,21,2 1 21,2", result.c_str());
//
//    macro = "@@return    --@@@";
//    body = "@return(100);";
//    result = MacroBuffer::ExpandMacro(macro, body);
//    ASSERT_STREQ("@return(100);", result.c_str());
//
//    macro = "@return(...)--@$*--";
//    body = "@return(100);";
//    result = MacroBuffer::ExpandMacro(macro, body);
//    ASSERT_STREQ("--100--;", result.c_str());
//
//}

//TEST_F(MacroTest, MacroDSL) {
//
//    Parser::MacrosStore macros;
//    std::string dsl = ""
//            "@if(cond)@@      [$cond]-->@@"
//            "@elseif(cond)@@ ,[$cond]-->@@"
//            "@else@@         ,[_]-->@@"
//            ""
//            "@while(cond)@@  [$cond]<->@@"
//            "@dowhile(cond)@@<->[$cond]@@"
//            "@return@         --@"
//            "@return(...)@    --$...--@"
//            "@dowhile(cond)@@@"
//            "@@@"
//            "";
//
//    while(Parser::ExtractMacros(dsl, macros))
//        ;
//    ASSERT_EQ(7, macros.size());
//
//
//}

TEST_F(MacroTest, MacroTest) {

    MacroBuffer macro;
    BlockType buffer;

    ASSERT_EQ(0, macro.GetCount());

    ASSERT_NO_THROW(Parse("@@alias@@ := @@ replace @@", &macro)) << macro.Dump();
    ASSERT_EQ(1, macro.GetCount()) << macro.Dump();
    ASSERT_EQ(1, macro["alias"].size()) << macro.Dump();
    ASSERT_TRUE(macro["alias"][0]) << macro.Dump();
    ASSERT_STREQ("alias", macro["alias"][0]->m_text.c_str()) << macro["alias"][0]->toString();
    ASSERT_EQ(1, macro["alias"][0]->GetMacroId().size());
    ASSERT_TRUE(macro.GetMacro({"alias"}));
    ASSERT_EQ(TermID::MACRO_SEQ, macro.GetMacro({"alias"})->getTermID());

    ASSERT_NO_THROW(Parse("alias", &macro)) << macro.Dump();
    ASSERT_STREQ("replace", LexOut().c_str());

    ASSERT_NO_THROW(Parse("alias()", &macro)) << macro.Dump();
    ASSERT_STREQ("replace ( )", LexOut().c_str());

    ASSERT_NO_THROW(Parse("alias(...)", &macro)) << macro.Dump();
    ASSERT_STREQ("replace ( ... )", LexOut().c_str());

    ASSERT_NO_THROW(Parse("alias(1,2,3)", &macro)) << macro.Dump();
    ASSERT_STREQ("replace ( 1 , 2 , 3 )", LexOut().c_str());

    ASSERT_NO_THROW(Parse("@alias", &macro)) << macro.Dump();
    ASSERT_STREQ("replace", LexOut().c_str());

    ASSERT_NO_THROW(Parse("@alias(); @alias", &macro)) << macro.Dump() << " LEX: \"" << LexOut().c_str() << "\"";
    ASSERT_STREQ("replace ( ) ; replace", LexOut().c_str());

    ASSERT_NO_THROW(Parse("@alias(...)", &macro)) << macro.Dump();
    ASSERT_STREQ("replace ( ... )", LexOut().c_str());

    ASSERT_NO_THROW(Parse("@alias(1,2,3); none", &macro)) << macro.Dump();
    ASSERT_STREQ("replace ( 1 , 2 , 3 ) ; none", LexOut().c_str());


    ASSERT_NO_THROW(Parse("@@ macro1 @@ ::= @@ replace1 @@", &macro)) << macro.Dump();
    ASSERT_EQ(2, macro.GetCount()) << macro.Dump();
    ASSERT_EQ(TermID::MACRO_SEQ, macro.GetMacro({"macro1"})->getTermID()) << toString(macro.GetMacro({"macro1"})->getTermID());

    ASSERT_NO_THROW(Parse("macro1", &macro)) << macro.Dump();
    ASSERT_STREQ("replace1", LexOut().c_str());

    ASSERT_NO_THROW(Parse("@macro1()", &macro)) << macro.Dump();
    ASSERT_STREQ("replace1 ( )", LexOut().c_str());

    ASSERT_NO_THROW(Parse("@macro1(...)", &macro)) << macro.Dump();
    ASSERT_STREQ("replace1 ( ... )", LexOut().c_str());

    ASSERT_NO_THROW(Parse("@macro1(1,2,3)", &macro)) << macro.Dump();
    ASSERT_STREQ("replace1 ( 1 , 2 , 3 )", LexOut().c_str());

    ASSERT_NO_THROW(Parse("@macro1", &macro)) << macro.Dump();
    ASSERT_STREQ("replace1", LexOut().c_str());

    // Макрос macro1 определн без скобок, а тут скобки есть
    ASSERT_ANY_THROW(Parse("@macro1() @alias", &macro)) << macro.Dump();
    ASSERT_ANY_THROW(Parse("none @macro1(...)", &macro)) << macro.Dump();




    ASSERT_NO_THROW(Parse("@@ macro2(...) @@ ::= @@ replace2( @$... ) @@", &macro)) << macro.Dump();
    ASSERT_EQ(3, macro.GetCount()) << macro.Dump();
    ASSERT_TRUE(macro.GetMacro({"macro2"}));
    ASSERT_EQ(TermID::MACRO_SEQ, macro.GetMacro({"macro2"})->getTermID());

    ASSERT_ANY_THROW(Parse("macro2", &macro)) << macro.Dump();

    ASSERT_NO_THROW(Parse("macro2()", &macro)) << macro.Dump();
    ASSERT_STREQ("replace2 ( )", LexOut().c_str());

    ASSERT_NO_THROW(Parse("macro2()", &macro)) << macro.Dump();
    ASSERT_STREQ("replace2 ( )", LexOut().c_str());

    ASSERT_NO_THROW(Parse("macro2( 1 )", &macro)) << macro.Dump();
    ASSERT_STREQ("replace2 ( 1 )", LexOut().c_str());

    ASSERT_NO_THROW(Parse("macro2(1,2,3)", &macro)) << macro.Dump();
    ASSERT_STREQ("replace2 ( 1 , 2 , 3 )", LexOut().c_str());

    ASSERT_ANY_THROW(Parse("@macro2", &macro)) << macro.Dump();

    ASSERT_NO_THROW(Parse("@macro2(); @alias(123)", &macro)) << macro.Dump();
    ASSERT_STREQ("replace2 ( ) ; replace ( 123 )", LexOut().c_str());

    ASSERT_NO_THROW(Parse("none;@macro2(...)", &macro)) << macro.Dump();
    ASSERT_STREQ("none ; replace2 ( ... )", LexOut().c_str());

    ASSERT_NO_THROW(Parse("@macro2(1,2,3);\n", &macro)) << macro.Dump();
    ASSERT_STREQ("replace2 ( 1 , 2 , 3 ) ;", LexOut().c_str());
    //    ASSERT_NO_THROW(Parse("@macro2(1,2,3);\nnone", &macro)) << macro.Dump();
    //    ASSERT_STREQ("replace2 ( 1 , 2 , 3 ) ; none", LexOut().c_str());

}

#endif // UNITTEST