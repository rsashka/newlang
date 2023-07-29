#include "pch.h"

#ifdef UNITTEST

#include <gtest/gtest.h>

#include <builtin.h>
#include <newlang.h>

using namespace newlang;

/* Проблема совместного использования namespace и декораторов имен.
 * 
 * Дектораторы имен ($ и @) импользуютсядля указания области видимости и времени жизни объектов.
 * $ - локальные объекты, размещаемые "условно" на стеке. Удаляются по мере выхода из текущей области видимости.
 * Локальный объект скрывает другие локальные и глобальные объекты с таким же именем.
 * @ - глобальные объекты, размещаемые "условно" в куче и их время жизни не зависит от текущей области видимости.
 * Глобальный объект заменяет другой глобальный объект с таким же именем.
 * 
 * Namespace позволяют использовать одинаковые имена объектов разделяя для них области видимости,
 * но можно ли совместить и использовтаь namespace и дектораторы одновременно?
 * 
 * $local; @global;
 * $name::local; \\name::global;
 * но ns {
 *   $name::local;  # Будет $ns::name::local
 *   \\name::global; # Все равно \\name::global или тоже \\ns::name::global ????
 * }
 * тогда без указания области видимости (без начального символа "::", т.е. name::var - объект выбирается автоматически,
 * т.е. сперва ищется локальный объект с таким именем, потом глобальный)
 * Какая разница между ::name::var и name::var?
 * Первый в глобальной области видимости, второй в текущей?
 * 
 *  $var := 0; # ::var или $var
 *  @func() := {0}; # ::func или @func - 0
 *  name {
 *      $var := 1; # ::name::var
 *      $func() := {11}; # локальная с именем - ::name::func
 *      @func() := {111}; # глобальная с именем - @func
 *      \\name::func() := {1111}; # глобальная \\name::func и тоже самое, что и локальная $func() !!!
 *      name {
 *          var := 2;  # ::name::name::var или $var или \\name::name::var
 *          @var := 2;  # ::var
 *          \\name::name::var := 2;  # ::name::name::var или \\name::name::var
 *          func() := {2}; # ::name::name::func или \\name::name::func - 2
 *          name3 {
 *              var := 3;           # ::name::name::name3::var или $name::name::name3::var - 3
 *              func() := {3};      # ::name::name::name3::func или \\name::name::name3::func() - 3
 * 
 *              var??;              # name::name::name3::var или $name::name::name3::var - 3
 *              name::var??;        # name::name::var или \\name::name::var - 2
 *              name::name::var??;  # name::var или \\name::name::var - 2
 *              ::name::var??;      # name::var или $name::var - 1
 *              ::var??;            # ::var или $var - 0
 *          }
 *      }
 *  }
 * Предположим, что:   <<<<<<<<<<<<<<<  НЕ АКТУАЛЬНО после изменения ситаксиса мароксов !!!!!!!!!!!!!!!!!!!!!!!!!!  >>>>>>>>>>>
 * :: - глобальный корень. Может быть заменен на @ или $
 * @@ — родительская функция после её переопределния
 * $$ — родительский объект
 * тогда:
 * $$.field - поле родительского класса
 * @@() - вызов переопределенной функции текущего класса или области видимости
 * @func() []= {@@() + 10}; # @func - 10
 * ::name::name@func
 * \\name::name::func - такая запись ненравится, но будет самой правильной и логичной, так как все остальные еще хуже для понимания и разбора
 * @::name::name::func
 * @::name::name@func
 * ::name::name::func@@
 * 
 * <<<<<<<<<<<<<<<<<< НОВАЯ КОНЦЕПЦИЯ >>>>>>>>>>>>>>>>>>
 * 
 * Все объекты физически размещаюится в куче, но время их жизни определяется областью видимости.
 * Объекты с декораторами $ обозначают локальные объекты (аналог "на стеке") и они удаляются по мере выхода из текущей области видимости.
 * Объекты с декораторами @ являются модулями или его объектами, доступны из других частй программы и сохраняют состояние при выходе из текущей области видимости.
 * 
 * Указание модуля (имя файла) @file или @dir.file или @dir1.dir2.file
 * Указание объекта в модуле @file::var или @dir.file::ns::func() или @dir.file::ns::obj.field; @file::ns:type @file::ns$local ???
 * 
 * Файл модуля загружается однократно при первом обращении и не выгружается до явной команды???
 * @module <*>;
 * @dir.module2 <*>; # Импорт всех функций модуля (кроме начинающихся на подчерк)
 * @dir.module2 <ns::name::*>; # Импорт всех функций из указанной области имен (кроме начинающихся на подчерк)
 * @dir.module2 <map=ns::name::*>; # Импорт всех функций из указанной области имен с переименованием (кроме начинающихся на подчерк)
 * @dir.module2 <func1, func2=::module2::ns::name::func2>;  # Импорт только конкретных функций + переименование
 * @dir.module2 <_>; # Выгрузить модуль?????
 * 
 * \\ns ::name::space::long@\
 * \ns::name;
 * 
 * @dsl{}; # Загрузка модуля с определниями макросов для DSL в самом начале любой программы?
 * 
 * @@ - главный модуль
 * @$ - текущий модуль
 * $$ - родительский объект (базовый объект или переопределяемая функция)
 * $0 - текущий объект (this)
 * 
 * [@$.__main__] --> { # 1 или 0
 * 
 * }
 * @$.__@@__ := {}; # Деструктор модуля
 * __@@__ := {}; # Деструктор модуля
 * 
 * \destructor() { // _____ ::= 
 * }
 * 
 * 
 * Дектораторы имен ($ и @) импользуютсядля указания области видимости и времени жизни объектов.
 * $ - локальные объекты, размещаемые "условно" на стеке. Удаляются по мере выхода из текущей области видимости.
 * Локальный объект скрывает другие локальные и глобальные объекты с таким же именем.
 * @ - глобальные объекты, размещаемые "условно" в куче и их время жизни не зависит от текущей области видимости.
 * Глобальный объект заменяет другой глобальный объект с таким же именем.
 * 
 * Namespace позволяют использовать одинаковые имена объектов разделяя для них области видимости,
 * но можно ли совместить и использовтаь namespace и дектораторы одновременно?
 * 
 * 
 * Наследование
 * Часто композиция класса более подходяща, чем наследование. Когда используйте наследование, делайте его открытым (public).
 * Когда дочерний класс наследуется от базового, он включает определения всех данных и операций от базового. "Наследование интерфейса" - это наследование от чистого абстрактного базового класса (в нём не определены состояние или методы). Всё остальное - это "наследование реализации".
 * Наследование реализации уменьшает размер кода благодаря повторному использованию частей базового класса (который становится частью нового класса). Т.к. наследование является декларацией времени компиляции, это позволяет компилятору понимать структуру и находить ошибки. Наследование интерфейса может быть использовано чтобы класс поддерживал требуемый API. И также, компилятор может находить ошибки, если класс не определяет требуемый метод наследуемого API.
 * В случае наследования реализации, код начинает размазываться между базовым и дочерним классом и это может усложнить понимание кода. Также, дочерний класс не может переопределять код невиртуальных функций (не может менять их реализацию).
 * Множественное наследование ещё более проблемное, а также иногда приводит к уменьшению производительности. Часто просадка производительности при переходе от одиночного наследования к множественному может быть больше, чем переход от обычных функций к виртуальным. Также от множественного наследования один шаг до ромбического, а это уже ведёт к неопределённости, путанице и, конечно же, багам.
 * Любое наследование должно быть открытым (public). Если хочется сделать закрытое (private), то лучше добавить новый член с экземпляром базового класса.
 * Не злоупотребляйте наследованием реализации. Композиция классов часто более предпочтительна. Попытайтесь ограничить использование наследования семантикой "Является": Bar можно наследовать от Foo, если можно сказать, что Bar "Является" Foo (т.е. там, где используется Foo, можно также использовать и Bar).
 * Защищёнными (protected) делайте лишь те функции, которые должны быть доступны для дочерних классов. Заметьте, что данные должны быть закрытыми (private).
 * Явно декларируйте переопределение виртуальных функций/деструктора с помошью спецификаторов: либо override, либо (если требуется) final. Не используйте спецификатор virtual при переопределении функций. Объяснение: функция или деструктор, помеченные override или final, но не являющиеся виртуальными просто не скомпилируются (что помогает обнаружить общие ошибки). Также спецификаторы работают как документация; а если спецификаторов нет, то программист будет вынужден проверить всю иерархию, чтобы уточнить виртуальность функции.
 * Множественное наследование <интерфейсов> допустимо, однако множественное наследование реализации не рекомендуется от слова совсем.
 * 
 * Для замены множественного наследования можно исопльзовать оператор утиной типизации (сравнения интерфейсов)
 * 
 * Конструкторы и деструкторы классов
 * 
 * Конструктор у класса может быть только один - и это обычная функция с именем класса.
 * Конструктор должен вернуть созданный экземпляр класса, который ему передается в $0 аргументе.
 * Другие аргументы соотетствуют передаваемым при вызове конструктора.
 * Конструктор по умолчанию принимает произвольное количество аргументов (для переопределения значения у
 * свойств по время вызова конструтокра) и возвращает аргумент $0, т.е. constructor(...) :- {$0($*)};
 * Пользователський конструтор лучше переопреелять в теле класса через оператор присвоения значения, 
 * аналог override, т.е. constructor(...) **=** {...};  (чтобы исключить ошибки в написании имени функции).
 * Конструторы базовых классов автоматически не вызываются, и если это требуется по логике работы, 
 * то вызовы конструторов базовых классов необходимо выполнять вручную, например так: $0 = $$.base_constructor();
 * 
 * Деструктор класса определяется в теле класса анонимной функицей (имя функци подчерк), 
 * т.е. _() :- {} и по умолчанию отсутствует. Деструткоры базовых классов вызываются автоматически?????
 *  

 - Расширен список имен базовых типов данных для более точного указания нативной функций С++ при её импорте.
- Добавлен признак константности объекта (символ карет в конце имени), причем идентификатор константы частью имени не является.
- Оператор добавления элемента в словарь или тензор "[]="
- С помощью операторов добавления "[]=" и обращения к родительскому обекту "$$" реализована возможность перегрузки и отката перегрузки функции
- Документирование свойств и методов ?????

 *  
 */
TEST(Oop, Namespace) {

    Context::Reset();
    Context ctx(RunTime::Init());

    ASSERT_EQ(0, ctx.m_named->m_ns_stack.size());
    ASSERT_STREQ("name", ctx.m_named->NamespaseFull("name").c_str());

    ASSERT_TRUE(ctx.m_named->NamespasePush("name"));
    ASSERT_EQ(1, ctx.m_named->m_ns_stack.size());
    ASSERT_STREQ("name", ctx.m_named->NamespaseFull().c_str());

    ASSERT_FALSE(ctx.m_named->NamespasePush(""));
    ASSERT_EQ(1, ctx.m_named->m_ns_stack.size());

    ASSERT_TRUE(ctx.m_named->NamespasePush("name2"));
    ASSERT_EQ(2, ctx.m_named->m_ns_stack.size());
    ASSERT_STREQ("name::name2", ctx.m_named->NamespaseFull().c_str());

    ASSERT_TRUE(ctx.m_named->NamespasePush("name3::name4"));
    ASSERT_EQ(3, ctx.m_named->m_ns_stack.size());
    ASSERT_STREQ("name::name2::name3::name4", ctx.m_named->NamespaseFull().c_str());

    ASSERT_TRUE(ctx.m_named->NamespasePush("::"));
    ASSERT_EQ(4, ctx.m_named->m_ns_stack.size());
    ASSERT_STREQ("::", ctx.m_named->NamespaseFull().c_str());

    ASSERT_FALSE(ctx.m_named->NamespasePush(""));
    ASSERT_EQ(4, ctx.m_named->m_ns_stack.size());

    ASSERT_STREQ("::ns", ctx.m_named->NamespaseFull("::ns").c_str());

    ASSERT_TRUE(ctx.m_named->NamespasePush("::name5::name6"));
    ASSERT_EQ(5, ctx.m_named->m_ns_stack.size());
    ASSERT_STREQ("::name5::name6", ctx.m_named->NamespaseFull().c_str());

    ASSERT_STREQ("::ns", ctx.m_named->NamespaseFull("::ns").c_str());

    ASSERT_TRUE(ctx.m_named->NamespasePush("name7"));
    ASSERT_EQ(6, ctx.m_named->m_ns_stack.size());
    ASSERT_STREQ("::name5::name6::name7", ctx.m_named->NamespaseFull().c_str());

    ctx.m_named->NamespasePop();
    ASSERT_EQ(5, ctx.m_named->m_ns_stack.size());
    ASSERT_STREQ("::name5::name6", ctx.m_named->NamespaseFull().c_str());

    ctx.m_named->NamespasePop();
    ASSERT_EQ(4, ctx.m_named->m_ns_stack.size());
    ASSERT_STREQ("::", ctx.m_named->NamespaseFull().c_str());

    ctx.m_named->NamespasePop();
    ASSERT_EQ(3, ctx.m_named->m_ns_stack.size());
    ASSERT_STREQ("name::name2::name3::name4", ctx.m_named->NamespaseFull().c_str());
    ASSERT_STREQ("::ns", ctx.m_named->NamespaseFull("::ns").c_str());

    ctx.m_named->NamespasePop();
    ASSERT_EQ(2, ctx.m_named->m_ns_stack.size());
    ASSERT_STREQ("name::name2", ctx.m_named->NamespaseFull().c_str());

    ctx.m_named->NamespasePop();
    ASSERT_EQ(1, ctx.m_named->m_ns_stack.size());
    ASSERT_STREQ("name", ctx.m_named->NamespaseFull().c_str());

    ctx.m_named->NamespasePop();
    ASSERT_EQ(0, ctx.m_named->m_ns_stack.size());
    ASSERT_STREQ("", ctx.m_named->NamespaseFull().c_str());


    ASSERT_TRUE(ctx.m_named->NamespasePush("ns::name"));
    ASSERT_EQ(1, ctx.m_named->m_ns_stack.size());
    ASSERT_STREQ("ns::name::var6", ctx.m_named->NamespaseFull("var6").c_str());


    ASSERT_TRUE(ctx.m_named->NamespasePush("::"));
    ASSERT_EQ(2, ctx.m_named->m_ns_stack.size());
    ASSERT_STREQ("::var6", ctx.m_named->NamespaseFull("var6").c_str());

    ctx.m_named->NamespasePop();
    ctx.m_named->NamespasePop();
    ASSERT_EQ(0, ctx.m_named->m_ns_stack.size());



    ASSERT_EQ(0, ctx.size());
    ASSERT_EQ(0, ctx.m_terms->size());

    ObjPtr var1 = ctx.ExecStr("var1 := 1;");
    ASSERT_TRUE(var1);
    ASSERT_EQ(1, var1->GetValueAsInteger());
    ASSERT_EQ(1, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("var1", ctx.at(0).first.c_str());
    ASSERT_EQ(1, ctx.m_terms->size());

    ObjPtr var2 = ctx.ExecStr("name { var2 := 2; }");
    ASSERT_TRUE(var2);
    ASSERT_EQ(2, var2->GetValueAsInteger());
    ASSERT_EQ(2, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("name::var2", ctx.at(1).first.c_str());
    ASSERT_EQ(2, ctx.m_terms->size());

    ObjPtr var3 = ctx.ExecStr("ns3::name { var3 := 3; }");
    ASSERT_TRUE(var3);
    ASSERT_EQ(3, var3->GetValueAsInteger());
    ASSERT_EQ(3, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("ns3::name::var3", ctx.at(2).first.c_str());
    ASSERT_EQ(3, ctx.m_terms->size());

    ObjPtr var4 = ctx.ExecStr("ns4::name { ::var4 := 4; }");
    ASSERT_TRUE(var4);
    ASSERT_EQ(4, var4->GetValueAsInteger());
    ASSERT_EQ(4, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("::var4", ctx.at(3).first.c_str());
    ASSERT_EQ(4, ctx.m_terms->size());

    ObjPtr var5 = ctx.ExecStr("ns5::name { ns::var5 := 5; }");
    ASSERT_TRUE(var5);
    ASSERT_EQ(5, var5->GetValueAsInteger());
    ASSERT_EQ(5, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("ns5::name::ns::var5", ctx.at(4).first.c_str());
    ASSERT_EQ(5, ctx.m_terms->size());

    ObjPtr var6 = ctx.ExecStr("ns6::name { { ::var6 := 6;} }");
    ASSERT_EQ(0, ctx.m_named->m_ns_stack.size());

    ASSERT_TRUE(var6);
    ASSERT_EQ(6, var6->GetValueAsInteger());
    ASSERT_EQ(6, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("::var6", ctx.at(5).first.c_str());
    ASSERT_EQ(6, ctx.m_terms->size());

    ObjPtr var7 = ctx.ExecStr("ns7::name { :: { var7 := 7;} }");
    ASSERT_EQ(0, ctx.m_named->m_ns_stack.size());

    ASSERT_TRUE(var7);
    ASSERT_EQ(7, var7->GetValueAsInteger());
    ASSERT_EQ(7, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("::var7", ctx.at(6).first.c_str());
    ASSERT_EQ(7, ctx.m_terms->size());

    ObjPtr var8 = ctx.ExecStr("ns8::name { ::ext { var8 := 8;}}");
    ASSERT_TRUE(var8);
    ASSERT_EQ(8, var8->GetValueAsInteger());
    ASSERT_EQ(8, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("::ext::var8", ctx.at(7).first.c_str());
    ASSERT_EQ(8, ctx.m_terms->size());



    Context::Reset();
    ctx.clear_();

    ASSERT_EQ(0, ctx.m_named->m_ns_stack.size());
    ASSERT_EQ(0, ctx.size());
    ASSERT_EQ(0, ctx.m_terms->size());

    ASSERT_EQ(0, ctx.size());

    ObjPtr func1 = ctx.ExecStr("func1() := {1};");
    ASSERT_TRUE(func1);
    ASSERT_EQ(1, func1->Call(&ctx)->GetValueAsInteger());
    ASSERT_EQ(1, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("func1", ctx.at(0).first.c_str());
    ASSERT_EQ(1, ctx.m_terms->size());

    ObjPtr func2 = ctx.ExecStr("name { func2() := {2}; }");
    ASSERT_TRUE(func2);
    ASSERT_EQ(2, func2->Call(&ctx)->GetValueAsInteger());
    ASSERT_EQ(2, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("name::func2", ctx.at(1).first.c_str());
    ASSERT_EQ(2, ctx.m_terms->size());

    ObjPtr func3 = ctx.ExecStr("ns3::name { func3() := {3}; }");
    ASSERT_TRUE(func3);
    ASSERT_EQ(3, func3->Call(&ctx)->GetValueAsInteger());
    ASSERT_EQ(3, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("ns3::name::func3", ctx.at(2).first.c_str());
    ASSERT_EQ(3, ctx.m_terms->size());

    ObjPtr func4 = ctx.ExecStr("ns4::name { ::func4() := {4}; }");
    ASSERT_TRUE(func4);
    ASSERT_EQ(4, func4->Call(&ctx)->GetValueAsInteger());
    ASSERT_EQ(4, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("::func4", ctx.at(3).first.c_str());
    ASSERT_EQ(4, ctx.m_terms->size());

    ObjPtr func5 = ctx.ExecStr("ns5::name { ns::func5() := {5}; }");
    ASSERT_TRUE(func5);
    ASSERT_EQ(5, func5->Call(&ctx)->GetValueAsInteger());
    ASSERT_EQ(5, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("ns5::name::ns::func5", ctx.at(4).first.c_str());
    ASSERT_EQ(5, ctx.m_terms->size());

    ObjPtr func6 = ctx.ExecStr("ns6::name { { ::func6() := {6};} }");
    ASSERT_EQ(0, ctx.m_named->m_ns_stack.size());

    ASSERT_TRUE(func6);
    ASSERT_EQ(6, func6->Call(&ctx)->GetValueAsInteger());
    ASSERT_EQ(6, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("::func6", ctx.at(5).first.c_str());
    ASSERT_EQ(6, ctx.m_terms->size());

    ObjPtr func7 = ctx.ExecStr("ns7::name { :: { func7() := {7};} }");
    ASSERT_EQ(0, ctx.m_named->m_ns_stack.size());

    ASSERT_TRUE(func7);
    ASSERT_EQ(7, func7->Call(&ctx)->GetValueAsInteger());
    ASSERT_EQ(7, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("::func7", ctx.at(6).first.c_str());
    ASSERT_EQ(7, ctx.m_terms->size());

    ObjPtr func8 = ctx.ExecStr("ns8::name { ::ext { func8() := {8};}}");
    ASSERT_TRUE(func8);
    ASSERT_EQ(8, func8->Call(&ctx)->GetValueAsInteger());
    ASSERT_EQ(8, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("::ext::func8", ctx.at(7).first.c_str());
    ASSERT_EQ(8, ctx.m_terms->size());
}

TEST(Oop, ClassFunc) {

    Context::Reset();
    Context ctx(RunTime::Init());

    ASSERT_EQ(0, ctx.m_named->m_ns_stack.size());
    ASSERT_EQ(0, ctx.size());
    ASSERT_EQ(0, ctx.m_terms->size());

    ASSERT_EQ(0, ctx.size());

    ObjPtr class1 = ctx.ExecStr(":class1 := :Class() { func1() := {1}; }");
    ASSERT_TRUE(class1);
    ASSERT_EQ(ObjType::Type, class1->m_var_type_current) << toString(class1->m_var_type_current);
    ASSERT_EQ(ObjType::Class, class1->m_var_type_fixed) << toString(class1->m_var_type_fixed);
    ASSERT_STREQ(":class1", class1->getName().c_str());

    //    ASSERT_TRUE(ctx.size() > 0) << ctx.Dump("; ").c_str();
    //    ASSERT_STREQ(":class1", ctx.at(0).first.c_str()) << ctx.Dump("; ").c_str();
    //    ASSERT_TRUE(ctx.find(":class1") != ctx.end()) << ctx.Dump("; ").c_str();
    //    ASSERT_STREQ("class1::func1", ctx.at(1).first.c_str()) << ctx.Dump("; ").c_str();
    ASSERT_EQ(2, ctx.m_terms->size());
    ASSERT_STREQ("class1::class1", ctx.m_terms->at(0).first.c_str());
    ASSERT_STREQ("class1::func1", ctx.m_terms->at(1).first.c_str());
    ObjPtr func1 = ctx.ExecStr("class1::func1");
    ASSERT_EQ(1, func1->Call(&ctx)->GetValueAsInteger());

    ASSERT_STREQ("class::class", MakeConstructorName(":class").c_str());
    ASSERT_STREQ("ns::class::class", MakeConstructorName(":ns::class").c_str());
    ASSERT_STREQ("ns::class::class::class", MakeConstructorName(":ns::class::class").c_str());
    
    EXPECT_ANY_THROW(ctx.ExecStr(":class_dup_func1 := :class1 { func1() ::= {11}; }"));
    EXPECT_ANY_THROW(ctx.ExecStr(":class_func_not_found := :class1 { func_not_found() = {0}; }"));

    ObjPtr class2 = ctx.ExecStr(":class2 := :class1() { func2() := {2}; }");
    ASSERT_TRUE(class2);
    ASSERT_EQ(ObjType::Type, class2->m_var_type_current) << toString(class2->m_var_type_current);
    ASSERT_EQ(ObjType::Class, class2->m_var_type_fixed) << toString(class2->m_var_type_fixed);
    ASSERT_STREQ(":class2", class2->getName().c_str());

    ASSERT_TRUE(ctx.size() > 2) << ctx.Dump("; ").c_str();
    //    ASSERT_STREQ(":class2", ctx.at(2).first.c_str()) << ctx.Dump("; ").c_str();

    std::string dump;
    for (int i = 0; i < ctx.m_terms->size(); i++) {
        dump += ctx.m_terms->at(i).first;
        dump += "; ";
    }
    ASSERT_EQ(5, ctx.m_terms->size()) << dump;
    ASSERT_STREQ("class2::class2", ctx.m_terms->at(2).first.c_str());
    ASSERT_STREQ("class2::func1", ctx.m_terms->at(3).first.c_str());
    ASSERT_STREQ("class2::func2", ctx.m_terms->at(4).first.c_str());
    ObjPtr func2_1 = ctx.ExecStr("class2::func1");
    ASSERT_EQ(1, func2_1->Call(&ctx)->GetValueAsInteger());
    ObjPtr func2_2 = ctx.ExecStr("class2::func2");
    ASSERT_EQ(2, func2_2->Call(&ctx)->GetValueAsInteger());



    ObjPtr func_name = ctx.ExecStr("class2 { func_name() := {'func_name'}; }");
    ASSERT_TRUE(func_name);
    ASSERT_EQ(ObjType::EVAL_FUNCTION, func_name->m_var_type_current) << toString(func_name->m_var_type_current);
    ASSERT_STREQ("class2::func_name", func_name->getName().c_str());

    dump = "";
    for (int i = 0; i < ctx.m_terms->size(); i++) {
        dump += ctx.m_terms->at(i).first;
        dump += "; ";
    }
    ASSERT_EQ(6, ctx.m_terms->size()) << dump;
    ASSERT_STREQ("class2::func_name", ctx.m_terms->at(5).first.c_str());
    ASSERT_STREQ("func_name", func_name->Call(&ctx)->GetValueAsString().c_str());


    ObjPtr class3 = ctx.ExecStr(":class3 := :class1, :class2 { func3() := {33}; func4() := {4}; }");
    ASSERT_TRUE(class3);
    ASSERT_EQ(ObjType::Type, class3->m_var_type_current) << toString(class3->m_var_type_current);
    ASSERT_EQ(ObjType::Class, class3->m_var_type_fixed) << toString(class3->m_var_type_fixed);
    ASSERT_STREQ(":class3", class3->getName().c_str());

    ASSERT_TRUE(ctx.size() > 4) << ctx.Dump("; ").c_str();

    dump = "";
    for (int i = 0; i < ctx.m_terms->size(); i++) {
        dump += ctx.m_terms->at(i).first;
        dump += "; ";
    }
    ASSERT_EQ(12, ctx.m_terms->size()) << dump;
    ASSERT_STREQ("class3::class3", ctx.m_terms->at(6).first.c_str());
    ASSERT_STREQ("class3::func1", ctx.m_terms->at(7).first.c_str());
    ASSERT_STREQ("class3::func3", ctx.m_terms->at(8).first.c_str());
    ASSERT_STREQ("class3::func4", ctx.m_terms->at(9).first.c_str());
    ASSERT_STREQ("class3::func2", ctx.m_terms->at(10).first.c_str());
    ASSERT_STREQ("class3::func_name", ctx.m_terms->at(11).first.c_str());
    ObjPtr func3_1 = ctx.ExecStr("class3::func1");
    ASSERT_EQ(1, func3_1->Call(&ctx)->GetValueAsInteger());
    ObjPtr func3_2 = ctx.ExecStr("class3::func2");
    ASSERT_EQ(2, func3_2->Call(&ctx)->GetValueAsInteger());
    ObjPtr func3_3 = ctx.ExecStr("class3::func3");
    ASSERT_EQ(33, func3_3->Call(&ctx)->GetValueAsInteger());
    ObjPtr func3_4 = ctx.ExecStr("class3::func4");
    ASSERT_EQ(4, func3_4->Call(&ctx)->GetValueAsInteger());
    ObjPtr result = ctx.ExecStr("class3::func_name()");
    ASSERT_TRUE(result);
    ASSERT_STREQ("func_name", result->GetValueAsString().c_str());




    ObjPtr var1 = ctx.ExecStr("var1 := :class1()");
    ASSERT_TRUE(var1);
    ObjPtr var1_1 = ctx.ExecStr("var1.func1()");
    ASSERT_TRUE(var1_1);
    ASSERT_EQ(1, var1_1->GetValueAsInteger());


    ObjPtr var2 = ctx.ExecStr("var2 := :class2()");
    ASSERT_TRUE(var2);
    ObjPtr var2_1 = ctx.ExecStr("var2.func1()");
    ASSERT_TRUE(var2_1);
    ASSERT_EQ(1, var2_1->GetValueAsInteger());
    ObjPtr var2_2 = ctx.ExecStr("var2.func2()");
    ASSERT_TRUE(var2_2);
    ASSERT_EQ(2, var2_2->GetValueAsInteger());


    ObjPtr var3 = ctx.ExecStr("var3 := :class3()");
    ASSERT_TRUE(var3);
    ObjPtr var3_1 = ctx.ExecStr("var3.func1()");
    ASSERT_TRUE(var3_1);
    ASSERT_EQ(1, var3_1->GetValueAsInteger());
    ObjPtr var3_2 = ctx.ExecStr("var3.func2()");
    ASSERT_TRUE(var3_2);
    ASSERT_EQ(2, var3_2->GetValueAsInteger());
    ObjPtr var3_3 = ctx.ExecStr("var3.func3()");
    ASSERT_TRUE(var3_3);
    ASSERT_EQ(33, var3_3->GetValueAsInteger());
    ObjPtr var3_4 = ctx.ExecStr("var3.func4()");
    ASSERT_TRUE(var3_4);
    ASSERT_EQ(4, var3_4->GetValueAsInteger());

}

TEST(Oop, ClassProp) {

    Context::Reset();
    Context ctx(RunTime::Init());

    ASSERT_EQ(0, ctx.m_named->m_ns_stack.size());
    ASSERT_EQ(0, ctx.size());
    ASSERT_EQ(0, ctx.m_terms->size());

    ASSERT_EQ(0, ctx.size());

    ObjPtr class1 = ctx.ExecStr(":class1 := :Class() { prop1 := 1; }");
    ASSERT_TRUE(class1);
    ASSERT_TRUE(class1->is_init());
    ASSERT_EQ(ObjType::Type, class1->m_var_type_current) << toString(class1->m_var_type_current);
    ASSERT_EQ(ObjType::Class, class1->m_var_type_fixed) << toString(class1->m_var_type_fixed);
    ASSERT_STREQ(":class1", class1->getName().c_str());
    ASSERT_EQ(1, class1->size());
    ASSERT_TRUE(class1->at("prop1").second);
    ASSERT_STREQ("1", class1->at("prop1").second->toString().c_str());

    ObjPtr class2 = ctx.ExecStr(":class2 := :class1 { prop2 := 2; }");
    ASSERT_TRUE(class2);
    ASSERT_EQ(ObjType::Type, class2->m_var_type_current) << toString(class2->m_var_type_current);
    ASSERT_EQ(ObjType::Class, class2->m_var_type_fixed) << toString(class2->m_var_type_fixed);
    ASSERT_STREQ(":class2", class2->getName().c_str());
    ASSERT_EQ(2, class2->size());
    ASSERT_STREQ("1", class2->at("prop1").second->toString().c_str());
    ASSERT_STREQ("2", class2->at("prop2").second->toString().c_str());


    ObjPtr class3 = ctx.ExecStr(":class3 := :class2 { prop1 = 10; }");
    ASSERT_TRUE(class3);
    ASSERT_EQ(ObjType::Type, class3->m_var_type_current) << toString(class2->m_var_type_current);
    ASSERT_EQ(ObjType::Class, class3->m_var_type_fixed) << toString(class2->m_var_type_fixed);
    ASSERT_STREQ(":class3", class3->getName().c_str());
    ASSERT_EQ(2, class3->size());
    ASSERT_STREQ("10", class3->at("prop1").second->toString().c_str());
    ASSERT_STREQ("2", class3->at("prop2").second->toString().c_str());


    EXPECT_ANY_THROW(ctx.ExecStr(":class_dup_prop2 := :class2 { prop2 ::= 1; }"));
    EXPECT_ANY_THROW(ctx.ExecStr(":class_prop_not_found := :class2 { prop_not_found = 1; }"));


    ObjPtr var1 = ctx.ExecStr("var1 := :class1()");
    ASSERT_TRUE(var1);
    ObjPtr var1_1;

    ASSERT_NO_THROW(var1_1 = ctx.ExecStr("var1.prop1")) << ctx.Dump() << "  (" << var1->toString() << ")";

    ASSERT_TRUE(var1_1);
    ASSERT_EQ(1, var1_1->GetValueAsInteger());


    ObjPtr var2 = ctx.ExecStr("var2 := :class2()");
    ASSERT_TRUE(var2);
    ObjPtr var2_1 = ctx.ExecStr("var2.prop1");
    ASSERT_TRUE(var2_1);
    ASSERT_EQ(1, var2_1->GetValueAsInteger());
    ObjPtr var2_2 = ctx.ExecStr("var2.prop2");
    ASSERT_TRUE(var2_2);
    ASSERT_EQ(2, var2_2->GetValueAsInteger());

    ObjPtr var3 = ctx.ExecStr("var3 := :class3()");
    ASSERT_TRUE(var3);
    ObjPtr var3_1 = ctx.ExecStr("var3.prop1");
    ASSERT_TRUE(var3_1);
    ASSERT_EQ(10, var3_1->GetValueAsInteger());
    ObjPtr var3_2 = ctx.ExecStr("var3.prop2");
    ASSERT_TRUE(var3_2);
    ASSERT_EQ(2, var3_2->GetValueAsInteger());


    ObjPtr var4 = ctx.ExecStr("var4 := :class3(prop2=100, prop1=99)");
    ASSERT_TRUE(var4);
    ObjPtr var4_1 = ctx.ExecStr("var4.prop1");
    ASSERT_TRUE(var4_1);
    ASSERT_EQ(99, var4_1->GetValueAsInteger());
    ObjPtr var4_2 = ctx.ExecStr("var4.prop2");
    ASSERT_TRUE(var4_2);
    ASSERT_EQ(100, var4_2->GetValueAsInteger());


    EXPECT_ANY_THROW(ctx.ExecStr("var_not_found := :class3(prop_not_found=100)"));
    EXPECT_ANY_THROW(ctx.ExecStr("var_bad_type := :class3(prop1='BAD TYPE')"));
}

//TEST(Oop, ClassConstruct) {
//
//
//    ASSERT_STREQ("class::class", MakeConstructorName(":class").c_str());
//    ASSERT_STREQ("ns::class::class", MakeConstructorName(":ns::class").c_str());
//    ASSERT_STREQ("ns::class::class::class", MakeConstructorName(":ns::class::class").c_str());
//
//
//    Context::Reset();
//    Context ctx(RunTime::Init());
//
//    ASSERT_EQ(0, ctx.m_ns_stack.size());
//    ASSERT_EQ(0, ctx.size());
//    ASSERT_EQ(0, ctx.m_terms->size());
//
//    ASSERT_EQ(0, ctx.size());
//
//    ObjPtr class1 = ctx.ExecStr(":class1 := :Class() { func1() := {1}; prop1 := 1; class1() = {$0.prop1 = 99; $0}; }");
//    ASSERT_TRUE(class1);
//    ASSERT_EQ(ObjType::Type, class1->m_var_type_current) << toString(class1->m_var_type_current);
//    ASSERT_EQ(ObjType::Class, class1->m_var_type_fixed) << toString(class1->m_var_type_fixed);
//    ASSERT_STREQ(":class1", class1->getName().c_str());
//
//    ASSERT_EQ(3, ctx.m_terms->size());
//    ASSERT_STREQ(":class1::class1", ctx.m_terms->at(0).first.c_str());
//    ASSERT_STREQ("class1::func1", ctx.m_terms->at(1).first.c_str());
//    ObjPtr func1 = ctx.ExecStr("class1::func1");
//    ASSERT_EQ(1, func1->Call(&ctx)->GetValueAsInteger());
//
//    ASSERT_ANY_THROW(ctx.ExecStr("class1::class1()")); // Object not found
//
//    ObjPtr obj1 = ctx.ExecStr("var1 := :class1()");
//    ASSERT_TRUE(obj1);
//    ObjPtr var1_1 = ctx.ExecStr("var1.prop1");
//    ASSERT_TRUE(var1_1);
//    ASSERT_EQ(99, var1_1->GetValueAsInteger());
//
//    //
//    //    ASSERT_ANY_THROW(ctx.ExecStr(":class_dup_func1 := :class1 { func1() ::= {11}; }"));
//    //    ASSERT_ANY_THROW(ctx.ExecStr(":class_func_not_found := :class1 { func_not_found() = {0}; }"));
//    //
//    //    ObjPtr class2 = ctx.ExecStr(":class2 := :class1() { func2() := {2}; }");
//    //    ASSERT_TRUE(class2);
//    //    ASSERT_EQ(ObjType::Type, class2->m_var_type_current) << toString(class2->m_var_type_current);
//    //    ASSERT_EQ(ObjType::Class, class2->m_var_type_fixed) << toString(class2->m_var_type_fixed);
//    //    ASSERT_STREQ(":class2", class2->getName().c_str());
//    //
//    //    ASSERT_TRUE(ctx.size() > 2) << ctx.Dump("; ").c_str();
//    //    //    ASSERT_STREQ(":class2", ctx.at(2).first.c_str()) << ctx.Dump("; ").c_str();
//    //    ASSERT_EQ(3, ctx.m_terms->size());
//    //    ASSERT_STREQ("class2::func1", ctx.m_terms->at(1).first.c_str());
//    //    ASSERT_STREQ("class2::func2", ctx.m_terms->at(2).first.c_str());
//    //    ObjPtr func2_1 = ctx.ExecStr("class2::func1");
//    //    ASSERT_EQ(1, func2_1->Call(&ctx)->GetValueAsInteger());
//    //    ObjPtr func2_2 = ctx.ExecStr("class2::func2");
//    //    ASSERT_EQ(2, func2_2->Call(&ctx)->GetValueAsInteger());
//    //
//    //
//    //
//    //    ObjPtr func_name = ctx.ExecStr("class2 { func_name() := {'func_name'}; }");
//    //    ASSERT_TRUE(func_name);
//    //    ASSERT_EQ(ObjType::EVAL_FUNCTION, func_name->m_var_type_current) << toString(func_name->m_var_type_current);
//    //    ASSERT_STREQ("class2::func_name", func_name->getName().c_str());
//    //
//    //    ASSERT_EQ(4, ctx.m_terms->size());
//    //    ASSERT_STREQ("class2::func_name", ctx.m_terms->at(3).first.c_str());
//    //    ASSERT_STREQ("func_name", func_name->Call(&ctx)->GetValueAsString().c_str());
//    //
//    //
//    //    ObjPtr class3 = ctx.ExecStr(":class3 := :class1, :class2 { func3() := {33}; func4() := {4}; }");
//    //    ASSERT_TRUE(class3);
//    //    ASSERT_EQ(ObjType::Type, class3->m_var_type_current) << toString(class3->m_var_type_current);
//    //    ASSERT_EQ(ObjType::Class, class3->m_var_type_fixed) << toString(class3->m_var_type_fixed);
//    //    ASSERT_STREQ(":class3", class3->getName().c_str());
//    //
//    //    ASSERT_TRUE(ctx.size() > 4) << ctx.Dump("; ").c_str();
//    //    //    ASSERT_STREQ(":class2", ctx.at(2).first.c_str()) << ctx.Dump("; ").c_str();
//    //    ASSERT_EQ(9, ctx.m_terms->size());
//    //    ASSERT_STREQ("class3::func1", ctx.m_terms->at(4).first.c_str());
//    //    ASSERT_STREQ("class3::func2", ctx.m_terms->at(5).first.c_str());
//    //    ASSERT_STREQ("class3::func_name", ctx.m_terms->at(6).first.c_str());
//    //    ASSERT_STREQ("class3::func3", ctx.m_terms->at(7).first.c_str());
//    //    ASSERT_STREQ("class3::func4", ctx.m_terms->at(8).first.c_str());
//    //    ObjPtr func3_1 = ctx.ExecStr("class3::func1");
//    //    ASSERT_EQ(1, func3_1->Call(&ctx)->GetValueAsInteger());
//    //    ObjPtr func3_2 = ctx.ExecStr("class3::func2");
//    //    ASSERT_EQ(2, func3_2->Call(&ctx)->GetValueAsInteger());
//    //    ObjPtr func3_3 = ctx.ExecStr("class3::func3");
//    //    ASSERT_EQ(33, func3_3->Call(&ctx)->GetValueAsInteger());
//    //    ObjPtr func3_4 = ctx.ExecStr("class3::func4");
//    //    ASSERT_EQ(4, func3_4->Call(&ctx)->GetValueAsInteger());
//    //    ObjPtr result = ctx.ExecStr("class3::func_name()");
//    //    ASSERT_TRUE(result);
//    //    ASSERT_STREQ("func_name", result->GetValueAsString().c_str());
//    //
//    //
//    //
//    //
//    //    ObjPtr var1 = ctx.ExecStr("var1 := :class1()");
//    //    ASSERT_TRUE(var1);
//    //    ObjPtr var1_1 = ctx.ExecStr("var1.func1()");
//    //    ASSERT_TRUE(var1_1);
//    //    ASSERT_EQ(1, var1_1->GetValueAsInteger());
//    //
//    //
//    //    ObjPtr var2 = ctx.ExecStr("var2 := :class2()");
//    //    ASSERT_TRUE(var2);
//    //    ObjPtr var2_1 = ctx.ExecStr("var2.func1()");
//    //    ASSERT_TRUE(var2_1);
//    //    ASSERT_EQ(1, var2_1->GetValueAsInteger());
//    //    ObjPtr var2_2 = ctx.ExecStr("var2.func2()");
//    //    ASSERT_TRUE(var2_2);
//    //    ASSERT_EQ(2, var2_2->GetValueAsInteger());
//    //
//    //
//    //    ObjPtr var3 = ctx.ExecStr("var3 := :class3()");
//    //    ASSERT_TRUE(var3);
//    //    ObjPtr var3_1 = ctx.ExecStr("var3.func1()");
//    //    ASSERT_TRUE(var3_1);
//    //    ASSERT_EQ(1, var3_1->GetValueAsInteger());
//    //    ObjPtr var3_2 = ctx.ExecStr("var3.func2()");
//    //    ASSERT_TRUE(var3_2);
//    //    ASSERT_EQ(2, var3_2->GetValueAsInteger());
//    //    ObjPtr var3_3 = ctx.ExecStr("var3.func3()");
//    //    ASSERT_TRUE(var3_3);
//    //    ASSERT_EQ(33, var3_3->GetValueAsInteger());
//    //    ObjPtr var3_4 = ctx.ExecStr("var3.func4()");
//    //    ASSERT_TRUE(var3_4);
//    //    ASSERT_EQ(4, var3_4->GetValueAsInteger());
//    //
//    //}
//    //
//    //TEST(Oop, ClassProp) {
//    //
//    //    Context::Reset();
//    //    Context ctx(RunTime::Init());
//    //
//    //    ASSERT_EQ(0, ctx.m_ns_stack.size());
//    //    ASSERT_EQ(0, ctx.size());
//    //    ASSERT_EQ(0, ctx.m_terms->size());
//    //
//    //    ASSERT_EQ(0, ctx.size());
//    //
//    //    ObjPtr class1 = ctx.ExecStr(":class1 := :Class() { prop1 := 1; }");
//    //    ASSERT_TRUE(class1);
//    //    ASSERT_EQ(ObjType::Type, class1->m_var_type_current) << toString(class1->m_var_type_current);
//    //    ASSERT_EQ(ObjType::Class, class1->m_var_type_fixed) << toString(class1->m_var_type_fixed);
//    //    ASSERT_STREQ(":class1", class1->getName().c_str());
//    //    ASSERT_EQ(1, class1->size());
//    //    ASSERT_STREQ("1", class1->at("prop1").second->toString().c_str());
//    //
//    //    ObjPtr class2 = ctx.ExecStr(":class2 := :class1 { prop2 := 2; }");
//    //    ASSERT_TRUE(class2);
//    //    ASSERT_EQ(ObjType::Type, class2->m_var_type_current) << toString(class2->m_var_type_current);
//    //    ASSERT_EQ(ObjType::Class, class2->m_var_type_fixed) << toString(class2->m_var_type_fixed);
//    //    ASSERT_STREQ(":class2", class2->getName().c_str());
//    //    ASSERT_EQ(2, class2->size());
//    //    ASSERT_STREQ("1", class2->at("prop1").second->toString().c_str());
//    //    ASSERT_STREQ("2", class2->at("prop2").second->toString().c_str());
//    //
//    //
//    //    ObjPtr class3 = ctx.ExecStr(":class3 := :class2 { prop1 = 10; }");
//    //    ASSERT_TRUE(class3);
//    //    ASSERT_EQ(ObjType::Type, class3->m_var_type_current) << toString(class2->m_var_type_current);
//    //    ASSERT_EQ(ObjType::Class, class3->m_var_type_fixed) << toString(class2->m_var_type_fixed);
//    //    ASSERT_STREQ(":class3", class3->getName().c_str());
//    //    ASSERT_EQ(2, class3->size());
//    //    ASSERT_STREQ("10", class3->at("prop1").second->toString().c_str());
//    //    ASSERT_STREQ("2", class3->at("prop2").second->toString().c_str());
//    //
//    //
//    //    EXPECT_ANY_THROW(ctx.ExecStr(":class_dup_prop2 := :class2 { prop2 ::= 1; }"));
//    //    EXPECT_ANY_THROW(ctx.ExecStr(":class_prop_not_found := :class2 { prop_not_found = 1; }"));
//    //
//    //
//    //    ObjPtr var1 = ctx.ExecStr("var1 := :class1()");
//    //    ASSERT_TRUE(var1);
//    //    ObjPtr var1_1;
//    //
//    //    ASSERT_NO_THROW(var1_1 = ctx.ExecStr("var1.prop1")) << ctx.Dump() << "  (" << var1->toString() << ")";
//    //
//    //    ASSERT_TRUE(var1_1);
//    //    ASSERT_EQ(1, var1_1->GetValueAsInteger());
//    //
//    //
//    //    ObjPtr var2 = ctx.ExecStr("var2 := :class2()");
//    //    ASSERT_TRUE(var2);
//    //    ObjPtr var2_1 = ctx.ExecStr("var2.prop1");
//    //    ASSERT_TRUE(var2_1);
//    //    ASSERT_EQ(1, var2_1->GetValueAsInteger());
//    //    ObjPtr var2_2 = ctx.ExecStr("var2.prop2");
//    //    ASSERT_TRUE(var2_2);
//    //    ASSERT_EQ(2, var2_2->GetValueAsInteger());
//    //
//    //    ObjPtr var3 = ctx.ExecStr("var3 := :class3()");
//    //    ASSERT_TRUE(var3);
//    //    ObjPtr var3_1 = ctx.ExecStr("var3.prop1");
//    //    ASSERT_TRUE(var3_1);
//    //    ASSERT_EQ(10, var3_1->GetValueAsInteger());
//    //    ObjPtr var3_2 = ctx.ExecStr("var3.prop2");
//    //    ASSERT_TRUE(var3_2);
//    //    ASSERT_EQ(2, var3_2->GetValueAsInteger());
//    //
//    //
//    //    ObjPtr var4 = ctx.ExecStr("var4 := :class3(prop2=100, prop1=99)");
//    //    ASSERT_TRUE(var4);
//    //    ObjPtr var4_1 = ctx.ExecStr("var4.prop1");
//    //    ASSERT_TRUE(var4_1);
//    //    ASSERT_EQ(99, var4_1->GetValueAsInteger());
//    //    ObjPtr var4_2 = ctx.ExecStr("var4.prop2");
//    //    ASSERT_TRUE(var4_2);
//    //    ASSERT_EQ(100, var4_2->GetValueAsInteger());
//    //
//    //
//    //    EXPECT_ANY_THROW(ctx.ExecStr("var_not_found := :class3(prop_not_found=100)"));
//    //    EXPECT_ANY_THROW(ctx.ExecStr("var_bad_type := :class3(prop1='BAD TYPE')"));
//
//}

#endif // UNITTEST