#include "pch.h"

#ifdef UNITTEST

#include "parser.h"

#include <signal.h>

#include <warning_pop.h>
#include <warning_push.h>
#include <gtest/gtest.h>

#include <builtin.h>
#include <newlang.h>

using namespace newlang;

TEST(Alg, Follow) {

    Context ctx(RunTime::Init());

    ObjPtr result = nullptr;

    result = ctx.ExecStr("[1]-->100");
    ASSERT_TRUE(result);
    ASSERT_EQ(100, result->GetValueAsInteger());

    result = ctx.ExecStr("[-1]-->99");
    ASSERT_TRUE(result);
    ASSERT_EQ(99, result->GetValueAsInteger());

    result = ctx.ExecStr("[0]-->111");
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->is_none_type());

    result = ctx.ExecStr("[1]-->111");
    ASSERT_TRUE(result);
    ASSERT_EQ(111, result->GetValueAsInteger());



    result = ctx.ExecStr("[0]-->100,[1]-->300");
    ASSERT_TRUE(result);
    ASSERT_EQ(300, result->GetValueAsInteger());

    result = ctx.ExecStr("[0]-->100,[0]-->300,[99]-->99,[_]-->1000");
    ASSERT_TRUE(result);
    ASSERT_EQ(99, result->GetValueAsInteger());

    result = ctx.ExecStr("[0]-->111,[_]-->1000");
    ASSERT_TRUE(result);
    ASSERT_EQ(1000, result->GetValueAsInteger());

    result = ctx.ExecStr("[0]-->{100},[1]-->{10;20;30}");
    ASSERT_TRUE(result);
    ASSERT_EQ(30, result->GetValueAsInteger());

    result = ctx.ExecStr("[0]-->{100},[0]-->{300},[99]-->{99},[_]-->{1000}");
    ASSERT_TRUE(result);
    ASSERT_EQ(99, result->GetValueAsInteger());

    result = ctx.ExecStr("[0]-->111,[_]-->{30;50;1000}");
    ASSERT_TRUE(result);
    ASSERT_EQ(1000, result->GetValueAsInteger());


    result = ctx.ExecStr("@cond := 100; [@cond]-->111,[_]-->1000");
    ASSERT_TRUE(result);
    ASSERT_EQ(111, result->GetValueAsInteger());

    result = ctx.ExecStr("@cond2 := 0; [@cond2]-->111,[_]-->1000");
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

    counter = ctx.ExecStr("[result -= 1] <-> {counter += 1}");
    ASSERT_TRUE(counter);
    ASSERT_EQ(2, counter->GetValueAsInteger());


    counter = ctx.ExecStr("{result += 1; counter += 1} <-> [result < 10]");
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
    ASSERT_EQ(10, (*test)[0].second->GetValueAsInteger());
    ASSERT_EQ(13, (*test)[3].second->GetValueAsInteger());

    ASSERT_EQ(test->resize_(4, nullptr), test->size());
    ASSERT_EQ(4, test->size());
    ASSERT_EQ(10, (*test)[0].second->GetValueAsInteger());
    ASSERT_EQ(13, (*test)[3].second->GetValueAsInteger());

    test->resize_(-3, nullptr);
    ASSERT_EQ(3, test->size());
    ASSERT_EQ(11, (*test)[0].second->GetValueAsInteger());
    ASSERT_EQ(13, (*test)[2].second->GetValueAsInteger());

    test->resize_(-1, nullptr);
    ASSERT_EQ(1, test->size());
    ASSERT_EQ(13, (*test)[0].second->GetValueAsInteger());

    test->resize_(0, nullptr);
    ASSERT_EQ(0, test->size());


    ObjPtr counter = ctx.ExecStr("counter := 100");
    ASSERT_EQ(100, counter->GetValueAsInteger());

    ObjPtr dict = ctx.ExecStr("dict := (1,2,3,)");
    ASSERT_TRUE(dict);
    ASSERT_TRUE(dict->is_dictionary_type());
    ASSERT_EQ(3, dict->size());
    ASSERT_EQ(1, (*dict)[0].second->GetValueAsInteger());
    ASSERT_EQ(3, (*dict)[2].second->GetValueAsInteger());

    ObjPtr temp = ctx.ExecStr("counter, dict := ... dict");
    ASSERT_TRUE(temp);
    ASSERT_TRUE(temp->is_dictionary_type());
    ASSERT_EQ(2, temp->size());

    ASSERT_EQ(1, counter->GetValueAsInteger());

    ASSERT_TRUE(dict->is_dictionary_type());
    ASSERT_EQ(2, dict->size());
    ASSERT_EQ(2, (*dict)[0].second->GetValueAsInteger());
    ASSERT_EQ(3, (*dict)[1].second->GetValueAsInteger());

    temp = ctx.ExecStr("counter, dict := ... dict");
    ASSERT_TRUE(temp);
    ASSERT_TRUE(temp->is_dictionary_type());
    ASSERT_EQ(1, temp->size());

    ASSERT_EQ(2, counter->GetValueAsInteger());

    ASSERT_TRUE(dict->is_dictionary_type());
    ASSERT_EQ(1, dict->size());
    ASSERT_EQ(3, (*dict)[0].second->GetValueAsInteger());

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

    temp = ctx.ExecStr("dict");
    ASSERT_TRUE(temp);
    ASSERT_TRUE(temp->is_dictionary_type());
    ASSERT_FALSE(temp->is_scalar());
    ASSERT_TRUE(temp->GetValueAsBoolean());
    ASSERT_EQ(5, temp->size());

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

    temp = ctx.ExecStr("counter = 0; [dict] <-> {counter, dict := ... dict; summa += counter}");
    ASSERT_TRUE(temp);
    ASSERT_TRUE(temp->is_integer());
    ASSERT_EQ(150, temp->GetValueAsInteger());

    ASSERT_TRUE(dict->is_dictionary_type());
    ASSERT_EQ(0, dict->size());

    ASSERT_EQ(150, summa->GetValueAsInteger());
    ASSERT_EQ(50, counter->GetValueAsInteger());

    summa = ctx.ExecStr("summa := 0");
    dict = ctx.ExecStr("dict := (1,2,3,4,5,)");
    temp = ctx.ExecStr("item := 0; [dict] <-> {item, dict := ... dict; summa += item}");
    ASSERT_TRUE(temp);
    ASSERT_TRUE(temp->is_integer());
    ASSERT_EQ(15, temp->GetValueAsInteger());
    ASSERT_TRUE(dict->is_dictionary_type());
    ASSERT_EQ(0, dict->size());
    ASSERT_EQ(15, summa->GetValueAsInteger());





    counter = ctx.ExecStr("counter := 55");
    ASSERT_EQ(55, counter->GetValueAsInteger());

    ObjPtr tensor = ctx.ExecStr("tensor := [10,11,12,]");
    ASSERT_TRUE(tensor);
    ASSERT_TRUE(tensor->is_tensor_type());
    ASSERT_TRUE(tensor->GetValueAsBoolean());
    ASSERT_EQ(3, tensor->size());
    ASSERT_EQ(10, (*tensor)[0].second->GetValueAsInteger());
    ASSERT_EQ(11, (*tensor)[1].second->GetValueAsInteger());
    ASSERT_EQ(12, (*tensor)[2].second->GetValueAsInteger());

    temp = ctx.ExecStr("counter, tensor  := ... tensor ");
    ASSERT_TRUE(temp);
    ASSERT_TRUE(temp->is_tensor_type());
    ASSERT_EQ(2, temp->size());

    ASSERT_EQ(10, counter->GetValueAsInteger());

    ASSERT_TRUE(tensor->is_tensor_type());
    ASSERT_TRUE(tensor->GetValueAsBoolean());
    ASSERT_EQ(2, tensor ->size());
    ASSERT_EQ(11, (*tensor)[0].second->GetValueAsInteger());
    ASSERT_EQ(12, (*tensor)[1].second->GetValueAsInteger());

    temp = ctx.ExecStr("counter, tensor := ... tensor ");
    ASSERT_TRUE(temp);
    ASSERT_TRUE(temp->is_tensor_type());
    ASSERT_EQ(1, temp->size());

    ASSERT_EQ(11, counter->GetValueAsInteger());

    ASSERT_TRUE(tensor->is_tensor_type());
    ASSERT_TRUE(tensor->GetValueAsBoolean());
    ASSERT_EQ(1, tensor ->size());
    ASSERT_EQ(12, (*tensor)[0].second->GetValueAsInteger());

    temp = ctx.ExecStr("counter, tensor := ... tensor ");
    ASSERT_TRUE(temp);
    ASSERT_TRUE(temp->is_tensor_type());
    ASSERT_EQ(0, temp->size());

    ASSERT_EQ(12, counter->GetValueAsInteger());

    ASSERT_TRUE(tensor->is_tensor_type());
    ASSERT_FALSE(tensor->GetValueAsBoolean());
    ASSERT_EQ(0, tensor ->size());

    temp = ctx.ExecStr("counter, tensor := ... tensor");
    ASSERT_TRUE(temp);
    ASSERT_TRUE(temp->is_tensor_type());
    ASSERT_TRUE(temp->empty());
    ASSERT_FALSE(tensor->GetValueAsBoolean());
    ASSERT_TRUE(tensor->is_tensor_type());
    ASSERT_TRUE(tensor->empty());

    ASSERT_TRUE(counter);
    ASSERT_TRUE(counter->empty());



    ObjPtr summa2 = ctx.ExecStr("summa := 0");
    ObjPtr tensor2 = ctx.ExecStr("tensor := [10,20,30,40,50,60,]");
    ObjPtr temp2 = ctx.ExecStr("item2 := 0; [tensor] <-> {item2, tensor := ... tensor; summa += item2}");
    ASSERT_TRUE(temp2);
    ASSERT_TRUE(temp2->is_integer());
    ASSERT_EQ(210, temp2->GetValueAsInteger());
    ASSERT_EQ(210, summa2->GetValueAsInteger());
    ASSERT_TRUE(tensor2->is_tensor_type());
    ASSERT_FALSE(tensor2->GetValueAsBoolean());

    ObjPtr tensor_size = ctx.ExecStr("tensor_size := [10,20,30,40,]");
    ASSERT_EQ(4, tensor_size->size());
    ASSERT_EQ(10, (*tensor_size)[0].second->GetValueAsInteger());
    ASSERT_EQ(40, (*tensor_size)[3].second->GetValueAsInteger());

    tensor_size->resize_(-10, nullptr);
    ASSERT_EQ(10, tensor_size->size());
    ASSERT_EQ(0, (*tensor_size)[0].second->GetValueAsInteger());
    ASSERT_EQ(0, (*tensor_size)[5].second->GetValueAsInteger());
    ASSERT_EQ(10, (*tensor_size)[6].second->GetValueAsInteger());
    ASSERT_EQ(40, (*tensor_size)[9].second->GetValueAsInteger());

}

TEST(Alg, Interrupt) {
    /* Проблема прерывания последовательности выполнения команд, которая была описана вначале, 
     * более не актуальна, так как обработка ошибков и прерываний решается следующим образом:
     * 
     * На уровне синтаксиса вводится два способа прервать поток команд, -- и ++ (управляемые прерывания).
     * Их реализация полностью идентична и они отличаются только семантикой записи и соглашениями по использованию.
     * Кроме этого добавялется блоки кода с разным типов перехвата прерываний.
     * { } - без перехвата прерываний
     * {+  +} - перехватывает прерывание ++ и извлекает возвращаемое значение, т.е. {+  ++100++  +} - вернет 100;
     * {-  -} - перехватывает прерывание -- и извлекает возвращаемое значение
     * {{  }} - перехватывает оба вида управляемых прерываний без извлечения возвращаемчх значений (но можно использовать 
     * типизацию для точного указания перехватываемого типа, т.е. {{ ++100++ }}:IntPlus - перехватит прерывание :IntPlus(100), 
     * но не извлекает данные как в {+  +}).
     * {{{  }}} -  Аналогично предыдущему варинату + перехватывает не управяемые прерывания (системные исключения и ошибки парсинга).
     * 
     * Проблема прерывания последовательности выполнения команд.
     * 
     * Последовательность команд может прервываться из-за возврата значения (любого типа)
     * и из-за возникновения ошибки (которая тоже может содержать значение любого типа ???)
     * Нужно иметь возможность разделять возврат и ошибку, но ни одно из возможных значений не может быть признаком возврата или ошибки, 
     * т.к. может возвращаться нормальным способом.
     * Механизм прерывания выполнения для возврата и обработки ошибок одинаковый, но нужно уметь их различаться.
     * И различать ух нужно как на уровне синтаксиса, так и в коде реализации (без лишних заморочек).
     * Вводить специальные имена классов <НЕЛЬЗЯ> !!!!
     * Конструкция прерывания выполнения одна для возврата и ошибки, поэтому различаться они могут только возвращаемым значением!!!!
     * А раз возвращаемое значение может быть любым, то ошибка и возврат визуальано должны различаться префиксами.
     * Префиксы переменных $ и @ сами по себе могут быть результатом возврата.
     * Можно было бы использовать префикс #, но он уже используется как признак однострочного комментария.
     * : - склоняюсь к указанию типа, но тип тоже может быть использовать как возвращаемое значение!!!!
     * Может быть % - признак константы и обработки времени компиляции? Ведь в этом есть определнная логика, т.к. 
     * логика обработки возвратов и ошибок может закладываться еще во время компиляции.
     * Но в этом случае требуется дополнить синтаксив перехвата возврата для конкретизации захватываемого значения.
     * Сейчас для этого используется указани <типа> возвращаемого значения, а если использовать дополнительный символ,
     * тогда потребуется обоснование его связи с типом возвращаемого значения или переделывать сам возврат (перехват).
     * --; --_--; --%Error--; --%Break1--; --%Error%--; --%Error("Описание ошибки")--; --%Error("Описание ошибки")--; 
     * 
     * Если разделать возвращаемые значения и признак ошибки по признако возвращения <ТИПА>, тогда вернуть тип обычным способм будет уже нельзя, 
     * так как это будет воспринимать как ошибка!!! Хотя в этом случае можно поместить тип в переменную и потом возвращать саму переменную.
     * или еще варинат - вернуть тип в обертке типа возврата, т.е. :Return(:Type), а это будет ужен не ошибкой, т.к. тип указан явно.
     * Тогда в этом случае, возможность не указывать тип :Return будет являться синтаксическим сахаром для упрощения записи.
     * А вот в макросах лучше указать в явном виде т.е. \return(value) \\ --:Return($value)-- \\\
     * 
     * 
     * \Break \\ --:Break-- \\\   \Break(label) \\ --:Break($label)-- \\\
     * 
     * :Break := :Return;
     * {{
     * 
     * }}:Break
     * dict := (10,20,30,40,50,60,70,80,);
     * :Break := :Return;
     * [dict] <-> {{
     *      item, dict := ... dict; 
     *      summa += item;
     *      [summa > 100] --> --:Break--;
     *      summa += item;
     * }}:Break;
     * 
     * :Continue := :Return;
     * dict := (10,20,30,40,50,60,70,80,);
     * [dict] <-> {
     *      {{
     *          item, dict := ... dict; 
     *          summa += item;
     *          [summa > 100] --> --:Continue--;
     *          summa += item;
     *      }}:Continue;
     * }
     * 
     * result := (){{
     *      100 / 0;
     * }};
     * [result] ~> {
     *      [:Return] --> print( "Значение: $1"(result[0]) ); # Как получить возвращенное значение?
     *      [:Error] --> print( "Ошибка: $1"( result[0] ) ); ??????????????????  "Ошибка "++result  
     * }
     * Сделать :Break и :Continue встроенными классами с проверкой на отсуствуие передаваемых аргументов.
     * Для втроенного типа :Result первый аргумент по умолчанию None, а для встроенного класса :Error сделать проверку 
     * на наличие обязательного строкового аргумента на первом месте (или строка по умолчанию "Ошибка") ??
     * Остальные типизированные возвраты могут быть любыми на усмотрение разработчика
     * 
     * 
     * {+      Подобное расширение синтаксиса ненравится !!!!!!!!!!!!!!!!!!!!!!!!!!
     * --+"Ошибка"+--   --+:Fatal("Ошибка")+--
     * +}
     *  
     * А вот такое вполне понятно и логично
     * {-
     * {+      
     *   --"Ошибка"--   ++"Данные"++
     * +} - тут return
     * 
     * -} - тут error
     */

    // :Tensor(1,2,3) - Создание экземпляра данных заданного типа
    // Орператор создания объекта    --("Тест ошибки"):Error--;  ("Тест ошибки"):Error();
    // --
    // --_--
    // --100--
    // --$var--
    // --"Возврат"--
    // --:Return("Возврат")--
    // --:Return--
    // --:Error--
    // --:Break--
    // --:Continue--
    // --:Error--
    // --:Error("Описание ошибки")--
    // --:Error("Описание ошибки", 2, 3, 4)--
    // func() := {--100--}; # Никогда не вернет результат, т.к. не перехватывает прерывания
    // func() := {{--100--}}; # Перехватывает прерывание и возвращает объект, находящийся в Interruption или производном классе
    // func() := {{--100--}}:Integer; # Перехватывает Interruption только с данными Integer и возвращает данные, а не объект прерывания
    // func():Integer := {{--100--}}; # Аналогична предыдущей записи как и func():Integer := {{--100--}}:Integer; 

    // Вызывается прерывание Interruption с числом в виде данных
    // Interruption ловится только двойным блоком

    // Interruption - любое прерывание выполнения кода (реализуется исключением)
    //  Если в прерывании указаны данные, то создается Interruption с этими данными и предназначается для возвращаемого значения
    // -- "Строка" --; -- 1000 --; -- $var --; --_--; # вернуть None, тоже что и --;
    //        VERIFY(CreateTypeName(":Break", ":Interruption")); // -- :Break --; # Вернуть тип
    // -- :Error("Описание ошибки") --; # Вернуть тип с данными

    Context ctx(RunTime::Init());

    ObjPtr result = nullptr;
    ObjPtr value = nullptr;

    try {
        result = ctx.ExecStr("--");
    } catch (Interrupt &except) {
        ASSERT_EQ(ObjType::IntMinus, except.m_obj->getType()) << newlang::toString(except.m_obj->getType());
        ASSERT_EQ(1, except.m_obj->size());
        ASSERT_TRUE((*except.m_obj)[0].second->is_none_type());
    }
    ASSERT_FALSE(result);

    try {
        result = ctx.ExecStr("--100--");
    } catch (Interrupt &except) {
        ASSERT_EQ(ObjType::IntMinus, except.m_obj->getType()) << newlang::toString(except.m_obj->getType());
        ASSERT_EQ(1, except.m_obj->size());
        ASSERT_STREQ("100", (*except.m_obj)[0].second->toString().c_str());
    }
    ASSERT_FALSE(result);


    try {
        result = ctx.ExecStr("++");
    } catch (Interrupt &except) {
        ASSERT_EQ(ObjType::IntPlus, except.m_obj->getType());
        ASSERT_EQ(1, except.m_obj->size());
        ASSERT_TRUE((*except.m_obj)[0].second->is_none_type());
    }
    ASSERT_FALSE(result);

    try {
        result = ctx.ExecStr("++100++");
    } catch (Interrupt &except) {
        ASSERT_EQ(ObjType::IntPlus, except.m_obj->getType());
        ASSERT_EQ(1, except.m_obj->size());
        ASSERT_STREQ("100", (*except.m_obj)[0].second->toString().c_str());
    }
    ASSERT_FALSE(result);


    try {
        result = ctx.ExecStr("--'Тест'--");
    } catch (Interrupt &except) {
        ASSERT_EQ(ObjType::IntMinus, except.m_obj->getType());
        ASSERT_EQ(1, except.m_obj->size());
        ASSERT_STREQ("'Тест'", (*except.m_obj)[0].second->toString().c_str());
    }
    ASSERT_FALSE(result);

    try {
        result = ctx.ExecStr("--:Int32--");
    } catch (Interrupt &except) {
        ASSERT_EQ(ObjType::IntMinus, except.m_obj->getType());
        ASSERT_EQ(1, except.m_obj->size());
        ASSERT_EQ(ObjType::Type, (*except.m_obj)[0].second->getType()) << toString((*except.m_obj)[0].second->getType());
        ASSERT_EQ(ObjType::Int32, (*except.m_obj)[0].second->m_var_type_fixed) << toString((*except.m_obj)[0].second->m_var_type_fixed);
    }
    ASSERT_FALSE(result);

    try {
        result = ctx.ExecStr("--:Error()--");
    } catch (Interrupt &except) {
        ASSERT_EQ(ObjType::Error, except.m_obj->getType());
        ASSERT_EQ(1, except.m_obj->size());
        ASSERT_TRUE(except.m_obj->toString().size() > 20);
    }
    ASSERT_FALSE(result);


    try {
        result = ctx.ExecStr("--:Error('ТЕКСТ')--");
    } catch (Interrupt &except) {
        ASSERT_EQ(ObjType::IntMinus, except.m_obj->getType());
        ASSERT_EQ(1, except.m_obj->size());
        ASSERT_STREQ(":IntMinus(:Error('ТЕКСТ'))", except.m_obj->toString().c_str());
        ASSERT_STREQ(":Error('ТЕКСТ')", (*except.m_obj)[0].second->toString().c_str());
    }
    ASSERT_FALSE(result);


    result.reset();
    ASSERT_NO_THROW(result = ctx.ExecStr("\\(){count := 1; count += 1; count += 1; count += 1; count += 1;};"););
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->is_integer()) << result->toString().c_str();
    ASSERT_EQ(5, result->GetValueAsInteger());

    result.reset();
    ASSERT_NO_THROW(result = ctx.ExecStr("\\(){count := 1; count += 1; count += 1; count += 1; count += 1;}; 99"););
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->is_integer()) << result->toString().c_str();
    ASSERT_EQ(99, result->GetValueAsInteger());

    result.reset();
    ASSERT_ANY_THROW(result = ctx.ExecStr("\\(){count := 1; count += 1; count += 1; --55--; count += 1; count += 1;}; 5555"););
    ASSERT_FALSE(result);

    result.reset();
    ASSERT_NO_THROW(result = ctx.ExecStr("\\(){{count := 1; count += 1; count += 1; --77--; count += 1; count += 1;}}", nullptr, Context::CatchType::CATCH_ANY););
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->is_integer()) << result->toString().c_str();
    ASSERT_EQ(77, result->GetValueAsInteger());

    result.reset();
    ASSERT_NO_THROW(result = ctx.ExecStr("\\(){{count := 1; count += 1; count += 1; --77--; count += 1; count += 1;}}; 7777"););
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->is_integer()) << result->toString().c_str();
    ASSERT_EQ(7777, result->GetValueAsInteger());

    result.reset();
    ASSERT_ANY_THROW(result = ctx.ExecStr("\\(){ \\(){count := 1; count += 1; count += 1; --:Error(88)--; count += 1; count += 1};  8888; }"););
    ASSERT_FALSE(result);

    result.reset();
    ASSERT_NO_THROW(result = ctx.ExecStr("\\(){ \\(){{count := 1; count += 1; count += 1; --:Error(99)--; count += 1; count += 1}}; 888}"););
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->is_integer()) << result->toString().c_str();
    ASSERT_EQ(888, result->GetValueAsInteger());

    result.reset();
    ASSERT_NO_THROW(result = ctx.ExecStr("\\(){ \\(){{count := 1; count += 1; count += 1; --:Error(99)--; count += 1; count += 1}}; }"););
    ASSERT_TRUE(result);
    ASSERT_EQ(1, result->size()) << result->toString().c_str();
    ASSERT_EQ(99, (*result)[0].second->GetValueAsInteger()) << result->toString().c_str();

    result.reset();
    ASSERT_NO_THROW(result = ctx.ExecStr("\\(){ \\(){{count := 1; count += 1; count += 1; --:Error(33)--; count += 1; count += 1}};  }"););
    ASSERT_TRUE(result);
    ASSERT_EQ(1, result->size()) << result->toString().c_str();
    ASSERT_EQ(33, (*result)[0].second->GetValueAsInteger()) << result->toString().c_str();

    result.reset();
    ASSERT_NO_THROW(result = ctx.ExecStr("\\(){ \\(){{count := 1; count += 1; count += 1; --:Error(44)--; count += 1; count += 1}};  9999; }"););
    ASSERT_TRUE(result);
    ASSERT_EQ(9999, result->GetValueAsInteger()) << result->toString().c_str();

    result.reset();
    // Не должен перехватытвать другой класс объекта
    ASSERT_ANY_THROW(result = ctx.ExecStr("\\(){ \\(){{count := 1; count += 1; count += 1; --:Error(55)--; count += 1; count += 1}}:ErrorRunTime;  }"););
    ASSERT_FALSE(result);


    ObjPtr test_err = ctx.GetTypeFromString(":Error");
    ASSERT_TRUE(test_err->op_class_test(":Error", &ctx));
    ASSERT_FALSE(test_err->op_class_test(":ErrorRunTime", &ctx));

    ObjPtr test_rt = ctx.GetTypeFromString(":ErrorRunTime");
    ASSERT_TRUE(test_rt->op_class_test(":ErrorRunTime", &ctx));
    ASSERT_TRUE(test_rt->op_class_test(":Error", &ctx));

    result.reset();
    // Не должен перехватытвать другой класс объекта, но его перехватывает внешний блок
    ASSERT_NO_THROW(result = ctx.ExecStr("\\(){{ \\(){{count := 1; count += 1; count += 1; --:Error(66)--; count += 1; count += 1}}:ErrorRunTime;  777; }}"););
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->is_error());
    ASSERT_STREQ(":Error(66)", result->toString().c_str());


    result.reset();
    // Должен перехватытвать прерывание и вернуть его как обычный объект без исключения из внешнего блока
    ASSERT_NO_THROW(result = ctx.ExecStr("\\(){ \\(){{count := 1; count += 1; count += 1; --:ErrorRunTime(77)--; count += 1; count += 1}}:ErrorRunTime }"););
    ASSERT_TRUE(result);
    ASSERT_EQ(1, result->size());
    ASSERT_EQ(77, (*result)[0].second->GetValueAsInteger());

    result.reset();
    ASSERT_NO_THROW(result = ctx.ExecStr("\\(){ \\(){{count := 1; count += 1; count += 1; --:ErrorRunTime(88)--; count += 1; count += 1}}:ErrorRunTime;  9999; }"););
    ASSERT_TRUE(result);
    ASSERT_EQ(9999, result->GetValueAsInteger());

    result.reset();
    ASSERT_NO_THROW(result = ctx.ExecStr("\\(){ \\(){{count := 1; count += 1; count += 1; --:ErrorRunTime(77)--; count += 1; count += 1}}:Error }"););
    ASSERT_TRUE(result);
    ASSERT_EQ(1, result->size());
    ASSERT_EQ(77, (*result)[0].second->GetValueAsInteger());

    result.reset();
    ASSERT_NO_THROW(result = ctx.ExecStr("\\(){ \\(){{count := 1; count += 1; count += 1; --:ErrorRunTime(88)--; count += 1; count += 1}}:Error;  9999; }"););
    ASSERT_TRUE(result);
    ASSERT_EQ(9999, result->GetValueAsInteger());
}

TEST(Alg, BreakContinue) {

    //    !!!!!!!!!!!!!
}

#endif // UNITTEST