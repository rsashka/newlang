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

TEST(Alg, Follow) {

    Context ctx(RunTime::Init());

    ObjPtr result = nullptr;

    result = ctx.ExecStr("[1]->100");
    ASSERT_TRUE(result);
    ASSERT_EQ(100, result->GetValueAsInteger());

    result = ctx.ExecStr("[-1]->99");
    ASSERT_TRUE(result);
    ASSERT_EQ(99, result->GetValueAsInteger());

    result = ctx.ExecStr("[0]->111");
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->is_none_type());

    result = ctx.ExecStr("[1]->111");
    ASSERT_TRUE(result);
    ASSERT_EQ(111, result->GetValueAsInteger());



    result = ctx.ExecStr("[0]->100,[1]->300");
    ASSERT_TRUE(result);
    ASSERT_EQ(300, result->GetValueAsInteger());

    result = ctx.ExecStr("[0]->100,[0]->300,[99]->99,[_]->1000");
    ASSERT_TRUE(result);
    ASSERT_EQ(99, result->GetValueAsInteger());

    result = ctx.ExecStr("[0]->111,[_]->1000");
    ASSERT_TRUE(result);
    ASSERT_EQ(1000, result->GetValueAsInteger());

    result = ctx.ExecStr("[0]->{100},[1]->{10;20;30}");
    ASSERT_TRUE(result);
    ASSERT_EQ(30, result->GetValueAsInteger());

    result = ctx.ExecStr("[0]->{100},[0]->{300},[99]->{99},[_]->{1000}");
    ASSERT_TRUE(result);
    ASSERT_EQ(99, result->GetValueAsInteger());

    result = ctx.ExecStr("[0]->111,[_]->{30;50;1000}");
    ASSERT_TRUE(result);
    ASSERT_EQ(1000, result->GetValueAsInteger());


    result = ctx.ExecStr("@cond := 100; [@cond]->111,[_]->1000");
    ASSERT_TRUE(result);
    ASSERT_EQ(111, result->GetValueAsInteger());

    result = ctx.ExecStr("@cond2 := 0; [@cond2]->111,[_]->1000");
    ASSERT_TRUE(result);
    ASSERT_EQ(1000, result->GetValueAsInteger());
}

TEST(Alg, Repeat) {

    Context ctx(RunTime::Init());

    ObjPtr result = nullptr;

    result = ctx.ExecStr("result := 0");
    ASSERT_TRUE(result);
    ASSERT_EQ(0, result->GetValueAsInteger());

    ObjPtr cond = ctx.ExecStr("result < 4");
    ASSERT_TRUE(cond);
    ASSERT_TRUE(cond->GetValueAsBoolean());

    result = ctx.ExecStr("result += 1");
    ASSERT_TRUE(result);
    ASSERT_EQ(1, result->GetValueAsInteger());

    cond = ctx.ExecStr("result < 4");
    ASSERT_TRUE(cond);
    ASSERT_TRUE(cond->GetValueAsBoolean());

    result = ctx.ExecStr("result += 3");
    ASSERT_TRUE(result);
    ASSERT_EQ(4, result->GetValueAsInteger());

    cond = ctx.ExecStr("result < 4");
    ASSERT_TRUE(cond);
    ASSERT_FALSE(cond->GetValueAsBoolean());


    ObjPtr counter = ctx.ExecStr("counter := 0");
    ASSERT_EQ(0, counter->GetValueAsInteger());

    ASSERT_EQ(4, result->GetValueAsInteger());
    result = ctx.ExecStr("result -= 1");
    ASSERT_EQ(3, result->GetValueAsInteger());

    counter = ctx.ExecStr("[result -= 1] ->> {counter += 1}");
    ASSERT_TRUE(counter);
    ASSERT_EQ(2, counter->GetValueAsInteger());


    counter = ctx.ExecStr("{result += 1; counter += 1} <<- [result < 10]");
    ASSERT_EQ(10, result->GetValueAsInteger());

    ASSERT_TRUE(counter);
    ASSERT_EQ(12, counter->GetValueAsInteger());

}

TEST(Alg, Foreach) {

    Context ctx(RunTime::Init());

    ObjPtr test = Obj::CreateDict();

    test->push_back(Obj::Arg(10));
    test->push_back(Obj::Arg(11));
    test->push_back(Obj::Arg(12));
    test->push_back(Obj::Arg(13));
    ASSERT_EQ(4, test->size());
    ASSERT_EQ(10, (*test)[0]->GetValueAsInteger());
    ASSERT_EQ(13, (*test)[3]->GetValueAsInteger());

    ASSERT_EQ(test->resize_(4, nullptr), test->size());
    ASSERT_EQ(4, test->size());
    ASSERT_EQ(10, (*test)[0]->GetValueAsInteger());
    ASSERT_EQ(13, (*test)[3]->GetValueAsInteger());

    test->resize_(-3, nullptr);
    ASSERT_EQ(3, test->size());
    ASSERT_EQ(11, (*test)[0]->GetValueAsInteger());
    ASSERT_EQ(13, (*test)[2]->GetValueAsInteger());

    test->resize_(-1, nullptr);
    ASSERT_EQ(1, test->size());
    ASSERT_EQ(13, (*test)[0]->GetValueAsInteger());

    test->resize_(0, nullptr);
    ASSERT_EQ(0, test->size());


    ObjPtr counter = ctx.ExecStr("counter := 100");
    ASSERT_EQ(100, counter->GetValueAsInteger());

    ObjPtr dict = ctx.ExecStr("dict := (1,2,3,)");
    ASSERT_TRUE(dict);
    ASSERT_TRUE(dict->is_dictionary_type());
    ASSERT_EQ(3, dict->size());
    ASSERT_EQ(1, (*dict)[0]->GetValueAsInteger());
    ASSERT_EQ(3, (*dict)[2]->GetValueAsInteger());

    ObjPtr temp = ctx.ExecStr("counter, dict := ... dict");
    ASSERT_TRUE(temp);
    ASSERT_TRUE(temp->is_dictionary_type());
    ASSERT_EQ(2, temp->size());

    ASSERT_EQ(1, counter->GetValueAsInteger());

    ASSERT_TRUE(dict->is_dictionary_type());
    ASSERT_EQ(2, dict->size());
    ASSERT_EQ(2, (*dict)[0]->GetValueAsInteger());
    ASSERT_EQ(3, (*dict)[1]->GetValueAsInteger());

    temp = ctx.ExecStr("counter, dict := ... dict");
    ASSERT_TRUE(temp);
    ASSERT_TRUE(temp->is_dictionary_type());
    ASSERT_EQ(1, temp->size());

    ASSERT_EQ(2, counter->GetValueAsInteger());

    ASSERT_TRUE(dict->is_dictionary_type());
    ASSERT_EQ(1, dict->size());
    ASSERT_EQ(3, (*dict)[0]->GetValueAsInteger());

    temp = ctx.ExecStr("counter, dict := ... dict");
    ASSERT_TRUE(temp);
    ASSERT_TRUE(temp->is_dictionary_type());
    ASSERT_EQ(0, temp->size());

    ASSERT_TRUE(counter);
    ASSERT_EQ(3, counter->GetValueAsInteger());

    ASSERT_TRUE(dict->is_dictionary_type());
    ASSERT_EQ(0, dict->size());

    temp = ctx.ExecStr("counter, dict := ... dict");
    ASSERT_TRUE(temp);
    ASSERT_TRUE(temp->is_dictionary_type());
    ASSERT_TRUE(dict->is_dictionary_type());
    ASSERT_EQ(0, dict->size());

    ASSERT_TRUE(counter);
    ASSERT_TRUE(counter->is_none_type());


    dict->push_back(Obj::Arg(10));
    dict->push_back(Obj::Arg(20));
    dict->push_back(Obj::Arg(30));
    dict->push_back(Obj::Arg(40));
    dict->push_back(Obj::Arg(50));
    ASSERT_EQ(5, dict->size());
    
    temp = ctx.ExecStr(":Bool(dict)");
    ASSERT_TRUE(temp);
    ASSERT_TRUE(temp->is_bool_type());
    ASSERT_FALSE(temp->is_scalar());
    ASSERT_TRUE(temp->GetValueAsBoolean());
    ASSERT_EQ(5, temp->size());
    
    temp = ctx.ExecStr(":Bool[0](dict)");
    ASSERT_TRUE(temp);
    ASSERT_TRUE(temp->is_bool_type());
    ASSERT_TRUE(temp->is_scalar());
    ASSERT_TRUE(temp->GetValueAsBoolean());
    ASSERT_EQ(0, temp->size());

    //    temp = ctx.ExecStr(":Bool(!dict)");
//    ASSERT_TRUE(temp);
//    ASSERT_TRUE(temp->is_bool_type());
//    ASSERT_FALSE(temp->GetValueAsBoolean());

    ObjPtr summa = ctx.ExecStr("summa := 0");

    temp = ctx.ExecStr("counter = 0; [dict] ->> {counter, dict := ... dict; summa += counter}");
    ASSERT_TRUE(temp);
    ASSERT_TRUE(temp->is_integer());
    ASSERT_EQ(150, temp->GetValueAsInteger());
    
    ASSERT_TRUE(dict->is_dictionary_type());
    ASSERT_EQ(0, dict->size());

    ASSERT_EQ(150, summa->GetValueAsInteger());
    ASSERT_EQ(50, counter->GetValueAsInteger());

}


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


//TEST(Alg, Interruption) {
///* Проблема прерывания последовательности выполнения команд.
// * 
// * Последовательность команд может прервываться из-за возврата значения (любого типа)
// * и из-за возникновения ошибки (которая тоже может содержать значение любого типа ???)
// * Нужно иметь возможность разделять возврат и ошибку, но ни одно из возможных значений не может быть признаком возврата или ошибки, 
// * т.к. может возвращаться нормальным способом.
// * Механизм прерывания выполнения для возврата и обработки ошибок одинаковый, но нужно уметь их различаться.
// * И различать ух нужно как на уровне синтаксиса, так и в коде реализации (без лишних заморочек).
// * Вводить специальные имена классов <НЕЛЬЗЯ> !!!!
// * Конструкция прерывания выполнения одна для возврата и ошибки, поэтому различаться они могут только возвращаемым значением!!!!
// * А раз возвращаемое значение может быть любым, то ошибка и возврат визуальано должны различаться префиксами.
// * Префиксы переменных $ и @ сами по себе могут быть результатом возврата.
// * Можно было бы использовать префикс #, но он уже используется как признак однострочного комментария.
// * : - склоняюсь к указанию типа, но тип тоже может быть использовать как возвращаемое значени!!!!
// * Может быть % - признак константы и обработки времени компиляции? Ведь в этом есть определнная логика, т.к. 
// * логика обработки возвратов и ошибок может закладываться еще во время компиляции.
// * Но в этом случае требуется дополнить синтаксив перехвата возврата для конкретизации захватываемого значения.
// * Сейчас для этого используется указани <типа> возвращаемого значения, а если использовать дополнительный символ,
// * тогда потребуется обоснование его связи с типом возвращаемого значения или переделывать сам возврат (перехват).
// * --; --_--; --%Error--; --%Break1--; --%Error%--; --%Error("Описание ошибки")--; --%Error("Описание ошибки")--; 
// * 
// * :Break := :Return;
// * {{
// * 
// * }}:Break
// * 
// * 
// *  
// */
//    
//    // :Tensor(1,2,3) - Создание экземпляра данных заданного типа
//    // Орператор создания объекта    --("Тест ошибки"):Error--;  ("Тест ошибки"):Error();
//    // --
//    // --_--
//    // --100--
//    // --$var--
//    // --"Возврат"--
//    // --:Return("Возврат")--
//    // --:Return--
//    // --:Error--
//    // --:Break--
//    // --:Continue--
//    // --:Error--
//    // --:Error("Описание ошибки")--
//    // --:Error("Описание ошибки", 2, 3, 4)--
//    // func() := {--100--}; # Никогда не вернет результат, т.к. не перехватывает прерывания
//    // func() := {{--100--}}; # Перехватывает прерывание и возвращает объект, находящийся в Interruption или производном классе
//    // func() := {{--100--}}:Integer; # Перехватывает Interruption только с данными Integer и возвращает данные, а не объект прерывания
//    // func():Integer := {{--100--}}; # Аналогична предыдущей записи как и func():Integer := {{--100--}}:Integer; 
//
//    // Вызывается прерывание Interruption с числом в виде данных
//    // Interruption ловится только двойным блоком
//
//    // Interruption - любое прерывание выполнения кода (реализуется исключением)
//    //  Если в прерывании указаны данные, то создается Interruption с этими данными и предназначается для возвращаемого значения
//    // -- "Строка" --; -- 1000 --; -- $var --; --_--; # вернуть None, тоже что и --;
//    //        VERIFY(CreateTypeName(":Break", ":Interruption")); // -- :Break --; # Вернуть тип
//    // -- :Error("Описание ошибки") --; # Вернуть тип с данными
//
//    Context ctx(RunTime::Init());
//
//    ObjPtr result = nullptr;
//    ObjPtr value = nullptr;
//
//    try {
//        result = ctx.ExecStr("--");
//    } catch (Interruption &except) {
//        ASSERT_STREQ(":Return", except.m_obj->m_class_name.c_str());
//        ASSERT_EQ(1, except.m_obj->size());
//    }
//    ASSERT_FALSE(result);
//
//    try {
//        result = ctx.ExecStr("--100--");
//    } catch (Interruption &except) {
//        ASSERT_STREQ(":Return", except.m_obj->m_class_name.c_str());
//        ASSERT_EQ(1, except.m_obj->size());
//        ASSERT_STREQ("100", (*except.m_obj)[0]->toString().c_str());
//    }
//    ASSERT_FALSE(result);
//
//    try {
//        result = ctx.ExecStr("--'Тест'--");
//    } catch (Interruption &except) {
//        ASSERT_STREQ(":Return", except.m_obj->m_class_name.c_str());
//        ASSERT_EQ(1, except.m_obj->size());
//        ASSERT_STREQ("'Тест'", (*except.m_obj)[0]->toString().c_str());
//    }
//    ASSERT_FALSE(result);
//
//    try {
//        result = ctx.ExecStr("--:Int--");
//    } catch (Interruption &except) {
//        ASSERT_STREQ(":Int", except.m_obj->m_class_name.c_str());
//        ASSERT_EQ(1, except.m_obj->size());
//        ASSERT_STREQ(":Type(Type=:Int)", (*except.m_obj)[0]->toString().c_str());
//    }
//    ASSERT_FALSE(result);
//
//    try {
////        result = ctx.Eval("--(,):Error--");
//        result = ctx.ExecStr("--:Error()--");
//    } catch (Interruption &except) {
//        ASSERT_STREQ(":Error", except.m_obj->m_class_name.c_str());
//        ASSERT_EQ(1, except.m_obj->size());
//        ASSERT_STREQ("Error()", (*except.m_obj)[0]->toString().c_str());
//    }
//    ASSERT_FALSE(result);
//
//
//    try {
//        result = ctx.ExecStr("--:Error('ТЕКСТ')--");
//    } catch (Interruption &except) {
//        ASSERT_STREQ(":Error", except.m_obj->m_class_name.c_str());
//        ASSERT_EQ(1, except.m_obj->size());
//        ASSERT_STREQ("Error('ТЕКСТ')", (*except.m_obj)[0]->toString().c_str());
//    }
//    ASSERT_FALSE(result);
//
//}


#endif // UNITTEST