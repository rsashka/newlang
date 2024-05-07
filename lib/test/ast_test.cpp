#ifdef BUILD_UNITTEST

#include "warning_push.h"
#include <gtest/gtest.h>
#include "warning_pop.h"


#include "builtin.h"
#include "runtime.h"
#include "analysis.h"
#include "jit.h"

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
TEST(Ast, Mangling) {
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

    ASSERT_STREQ("", ExtractModuleName("").c_str());
    ASSERT_STREQ("", ExtractModuleName("_$$_").c_str());
    ASSERT_STREQ("\\\\file", ExtractModuleName("_$file$_").c_str());
    ASSERT_STREQ("\\\\file\\dir", ExtractModuleName("_$file_dir$_").c_str());
    ASSERT_STREQ("\\\\file\\dir", ExtractModuleName("_$file_dir$_var$").c_str());
    ASSERT_STREQ("\\\\file\\dir", ExtractModuleName("_$file_dir$_var$$").c_str());
    ASSERT_STREQ("\\\\file\\dir", ExtractModuleName("_$file_dir$_var$$$").c_str());
}

TEST(Ast, ScopeStack) {

    StorageTerm storage;
    ScopeStack ns_stack(storage);
    ASSERT_EQ(ns_stack.m_static, storage);

    ASSERT_STREQ("::name1::", ns_stack.CreateVarName("::name1").c_str());
    ASSERT_STREQ("name1$", ns_stack.CreateVarName("name1").c_str());
    ASSERT_STREQ("name1$", ns_stack.CreateVarName("$name1").c_str());
    ASSERT_STREQ("name1:::", ns_stack.CreateVarName(":name1").c_str());

    ASSERT_STREQ("Storage: \n", ns_stack.Dump().c_str());
    ns_stack.PushScope(nullptr);

    ASSERT_STREQ("::name1::", ns_stack.CreateVarName("::name1").c_str());
    ASSERT_STREQ("1::name1$", ns_stack.CreateVarName("name1").c_str());
    ASSERT_STREQ("1::name1$", ns_stack.CreateVarName("$name1").c_str());
    ASSERT_STREQ("name1:::", ns_stack.CreateVarName(":name1").c_str());

    ns_stack.back().vars.insert({"name1", Term::Create(TermID::NAME, "term_name1", parser::token_type::NAME)});
    ns_stack.back().vars.insert({"name2", Term::Create(TermID::NAME, "term_name2", parser::token_type::NAME)});

    ns_stack.PushScope(Term::Create(TermID::NAME, "ns", parser::token_type::NAME));
    ns_stack.back().vars.insert({"name3", Term::Create(TermID::NAME, "term_name3", parser::token_type::NAME)});

    ASSERT_STREQ("::name1::", ns_stack.CreateVarName("::name1").c_str());
    ASSERT_STREQ("1::ns::name1$", ns_stack.CreateVarName("name1").c_str());
    ASSERT_STREQ("1::ns::name1$", ns_stack.CreateVarName("$name1").c_str());
    ASSERT_STREQ("ns::name1:::", ns_stack.CreateVarName(":name1").c_str());


    storage.insert({"name4", Term::Create(TermID::NAME, "term_name4", parser::token_type::NAME)});
    storage.insert({"name5", Term::Create(TermID::NAME, "term_name5", parser::token_type::NAME)});

    ASSERT_STREQ("Storage: name4, name5\nStack [1::]: name1, name2\nStack [ns::]: name3\n", ns_stack.Dump().c_str());
    storage.clear();
    ns_stack.clear();

    //  ... 
    ASSERT_EQ(0, ns_stack.size());
    ASSERT_STREQ("", ns_stack.GetNamespace().c_str());

    ASSERT_FALSE(ns_stack.LookupName("name1"));
    ASSERT_FALSE(ns_stack.LookupName("$name1"));
    ASSERT_FALSE(ns_stack.LookupName("::name1"));

    TermPtr name1 = Term::Create(TermID::NAME, "name1", parser::token_type::NAME);
    ASSERT_EQ(0, storage.size()) << ns_stack.Dump();
    ASSERT_TRUE(storage.find("$name1") == storage.end()) << ns_stack.Dump();

    name1->m_int_name = ns_stack.CreateVarName(name1->m_text);
    ASSERT_TRUE(ns_stack.AddName(name1)) << ns_stack.Dump();

    ASSERT_EQ(0, ns_stack.size());
    ASSERT_EQ(1, storage.size()) << ns_stack.Dump();
    ASSERT_TRUE(storage.find("name1$") != storage.end()) << ns_stack.Dump();

    ASSERT_STREQ("name1$", name1->m_int_name.c_str());
    TermPtr temp1;
    ASSERT_TRUE(temp1 = ns_stack.LookupName("name1")) << ns_stack.Dump();
    ASSERT_EQ(temp1.get(), name1.get());
    ASSERT_TRUE(temp1 = ns_stack.LookupName("$name1")) << ns_stack.Dump();
    ASSERT_EQ(temp1.get(), name1.get());
    ASSERT_FALSE(ns_stack.LookupName("::name1")) << ns_stack.Dump();

    //  name { ... }
    ns_stack.PushScope(Term::Create(TermID::NAMESPACE, "name", parser::token_type::NAMESPACE));
    ASSERT_EQ(1, ns_stack.size());
    ASSERT_STREQ("name::", ns_stack.GetNamespace().c_str());


    ASSERT_STREQ("::name1::", ns_stack.CreateVarName("::name1").c_str());
    ASSERT_STREQ("name::name1$", ns_stack.CreateVarName("name1").c_str());
    ASSERT_STREQ("name::name1$", ns_stack.CreateVarName("$name1").c_str());
    ASSERT_STREQ("name::name1:::", ns_stack.CreateVarName(":name1").c_str());


    TermPtr name2 = Term::Create(TermID::NAME, "name2", parser::token_type::NAME);
    ASSERT_EQ(1, ns_stack.size()) << ns_stack.Dump();
    ASSERT_EQ(1, storage.size()) << ns_stack.Dump();
    ASSERT_TRUE(storage.find("$name2") == storage.end()) << ns_stack.Dump();
    ASSERT_TRUE(ns_stack[0].vars.find("$name2") == ns_stack[0].vars.end()) << ns_stack.Dump();

    name2->m_int_name = ns_stack.CreateVarName(name2->m_text);
    ASSERT_TRUE(ns_stack.AddName(name2)) << ns_stack.Dump();

    ASSERT_STREQ("name2", name2->m_text.c_str());
    TermPtr temp2;
    ASSERT_TRUE(temp2 = ns_stack.LookupName("name2")) << ns_stack.Dump();
    ASSERT_EQ(temp2.get(), name2.get());
    ASSERT_TRUE(temp2 = ns_stack.LookupName("$name2")) << ns_stack.Dump();
    ASSERT_EQ(temp2.get(), name2.get());
    ASSERT_FALSE(ns_stack.LookupName("::name2")) << ns_stack.Dump();

    TermPtr temp;
    ASSERT_TRUE(temp = ns_stack.LookupName("name1")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name1.get());
    ASSERT_TRUE(temp = ns_stack.LookupName("$name1")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name1.get());


    //  name {  {...}  }
    ns_stack.PushScope(nullptr);
    ASSERT_EQ(2, ns_stack.size());

    ASSERT_STREQ("::name1::", ns_stack.CreateVarName("::name1").c_str());
    ASSERT_STREQ("name::2::name1$", ns_stack.CreateVarName("name1").c_str());
    ASSERT_STREQ("name::2::name1$", ns_stack.CreateVarName("$name1").c_str());
    ASSERT_STREQ("name::name1:::", ns_stack.CreateVarName(":name1").c_str());


    TermPtr name3 = Term::Create(TermID::NAME, "$name3", parser::token_type::NAME);
    ASSERT_EQ(2, ns_stack.size()) << ns_stack.Dump();
    ASSERT_EQ(2, storage.size()) << ns_stack.Dump();
    ASSERT_TRUE(ns_stack[1].vars.find("$name3") == ns_stack[1].vars.end()) << ns_stack.Dump();

    name3->m_int_name = ns_stack.CreateVarName(name3->m_text);
    ASSERT_TRUE(ns_stack.AddName(name3)) << ns_stack.Dump();

    ASSERT_STREQ("$name3", name3->m_text.c_str());
    TermPtr temp3;
    ASSERT_TRUE(temp3 = ns_stack.LookupName("name3")) << ns_stack.Dump();
    ASSERT_EQ(temp3.get(), name3.get());
    ASSERT_TRUE(temp3 = ns_stack.LookupName("$name3")) << ns_stack.Dump();
    ASSERT_EQ(temp3.get(), name3.get());
    ASSERT_FALSE(ns_stack.LookupName("::name3")) << ns_stack.Dump();

    ASSERT_TRUE(temp = ns_stack.LookupName("name1")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name1.get());
    ASSERT_TRUE(temp = ns_stack.LookupName("$name1")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name1.get());

    ASSERT_TRUE(temp = ns_stack.LookupName("name2")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name2.get());
    ASSERT_TRUE(temp = ns_stack.LookupName("$name2")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name2.get());


    //  name {  {  name2 {...}  }  }
    ns_stack.PushScope(Term::Create(TermID::NAMESPACE, "name2", parser::token_type::NAMESPACE));
    ASSERT_EQ(3, ns_stack.size());
    ASSERT_STREQ("name::2::name2::", ns_stack.GetNamespace(false).c_str());
    ASSERT_STREQ("name::name2::", ns_stack.GetNamespace(true).c_str());

    ASSERT_STREQ("::name1::", ns_stack.CreateVarName("::name1").c_str());
    ASSERT_STREQ("name::2::name2::name1$", ns_stack.CreateVarName("name1").c_str());
    ASSERT_STREQ("name::2::name2::name1$", ns_stack.CreateVarName("$name1").c_str());
    ASSERT_STREQ("name::name2::name1:::", ns_stack.CreateVarName(":name1").c_str());

    TermPtr name4 = Term::Create(TermID::NAME, "name4", parser::token_type::NAME);
    ASSERT_EQ(3, ns_stack.size()) << ns_stack.Dump();
    ASSERT_EQ(3, storage.size()) << ns_stack.Dump();
    ASSERT_TRUE(ns_stack[2].vars.find("$name4") == ns_stack[2].vars.end()) << ns_stack.Dump();

    name4->m_int_name = ns_stack.CreateVarName(name4->m_text);
    ASSERT_TRUE(ns_stack.AddName(name4)) << ns_stack.Dump();

    ASSERT_TRUE(temp = ns_stack.LookupName("name4")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name4.get()) << ns_stack.Dump();
    ASSERT_TRUE(temp = ns_stack.LookupName("$name4")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name4.get()) << ns_stack.Dump();

    ASSERT_TRUE(temp = ns_stack.LookupName("name1")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name1.get()) << ns_stack.Dump();
    ASSERT_TRUE(temp = ns_stack.LookupName("$name1")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name1.get()) << ns_stack.Dump();

    ASSERT_TRUE(temp = ns_stack.LookupName("name2")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name2.get()) << temp->toString() << " " << name2->toString() << " " << ns_stack.Dump();
    ASSERT_TRUE(temp = ns_stack.LookupName("$name2")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name2.get()) << ns_stack.Dump();

    ASSERT_TRUE(temp = ns_stack.LookupName("name3")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name3.get()) << ns_stack.Dump();
    ASSERT_TRUE(temp = ns_stack.LookupName("$name3")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name3.get()) << ns_stack.Dump();



    //  name {  {  name2 {  name3::name4 {...}  }  }  }
    ns_stack.PushScope(Term::Create(TermID::NAMESPACE, "name3::name4", parser::token_type::NAMESPACE));
    ASSERT_EQ(4, ns_stack.size());
    ASSERT_STREQ("name::2::name2::name3::name4::", ns_stack.GetNamespace().c_str());

    ASSERT_STREQ("::name1::", ns_stack.CreateVarName("::name1").c_str());
    ASSERT_STREQ("name::2::name2::name3::name4::name1$", ns_stack.CreateVarName("name1").c_str());
    ASSERT_STREQ("name::2::name2::name3::name4::name1$", ns_stack.CreateVarName("$name1").c_str());
    ASSERT_STREQ("name::name2::name3::name4::name1:::", ns_stack.CreateVarName(":name1").c_str());

    //  name {  {  name2 {  name3::name4 {   ::{...}  }  }  }  }
    ns_stack.PushScope(Term::Create(TermID::NAMESPACE, "::", parser::token_type::NAMESPACE));
    ASSERT_EQ(5, ns_stack.size());
    ASSERT_STREQ("::", ns_stack.GetNamespace(false).c_str());
    ASSERT_STREQ("::", ns_stack.GetNamespace(true).c_str());
    //    ASSERT_STREQ("::name", ns_stack.GetFullName("name").c_str());

    ASSERT_STREQ("::name1::", ns_stack.CreateVarName("::name1").c_str());
    ASSERT_STREQ("::name1$", ns_stack.CreateVarName("name1").c_str());
    ASSERT_STREQ("::name1$", ns_stack.CreateVarName("$name1").c_str());
    ASSERT_STREQ("::name1:::", ns_stack.CreateVarName(":name1").c_str());

    //  name {  {  name2 {  name3::name4 {   ::{  {...}  }  }  }  }  }
    ns_stack.PushScope(nullptr);
    ASSERT_EQ(6, ns_stack.size());

    ASSERT_STREQ("::3::", ns_stack.GetNamespace(false).c_str());
    ASSERT_STREQ("::", ns_stack.GetNamespace(true).c_str());

    ASSERT_STREQ("::name1::", ns_stack.CreateVarName("::name1").c_str());
    ASSERT_STREQ("::3::name1$", ns_stack.CreateVarName("name1").c_str());
    ASSERT_STREQ("::3::name1$", ns_stack.CreateVarName("$name1").c_str());
    ASSERT_STREQ("::name1:::", ns_stack.CreateVarName(":name1").c_str());


    TermPtr name_g = Term::Create(TermID::NAME, "@::name_g", parser::token_type::NAME);
    ASSERT_EQ(6, ns_stack.size()) << ns_stack.Dump();
    ASSERT_EQ(4, storage.size()) << ns_stack.Dump();
    ASSERT_TRUE(storage.find("::name_g") == storage.end()) << ns_stack.Dump();
    ASSERT_TRUE(ns_stack[5].vars.find("$name4") == ns_stack[5].vars.end()) << ns_stack.Dump();

    name_g->m_int_name = ns_stack.CreateVarName(name_g->m_text);
    ASSERT_TRUE(ns_stack.AddName(name_g)) << ns_stack.Dump();

    ASSERT_EQ(5, storage.size()) << ns_stack.Dump();
    ASSERT_TRUE(storage.find("::3::name_g::") != storage.end()) << ns_stack.Dump();
    ASSERT_TRUE(ns_stack[5].vars.find("::3::name_g::") != ns_stack[5].vars.end()) << ns_stack.Dump();

    ASSERT_STREQ("::3::name_g::", name_g->m_int_name.c_str());

    ASSERT_TRUE(temp = ns_stack.LookupName("name_g")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name_g.get());
    ASSERT_FALSE(temp = ns_stack.LookupName("$name_g")) << ns_stack.Dump();
    ASSERT_FALSE(ns_stack.LookupName("::name_g")) << ns_stack.Dump();
    ASSERT_TRUE(temp = ns_stack.LookupName("@::name_g")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name_g.get());

    ASSERT_TRUE(temp = ns_stack.LookupName("name1")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name1.get());
    ASSERT_TRUE(temp = ns_stack.LookupName("$name1")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name1.get());

    ASSERT_TRUE(temp = ns_stack.LookupName("name2")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name2.get());
    ASSERT_TRUE(temp = ns_stack.LookupName("$name2")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name2.get());

    ASSERT_TRUE(temp = ns_stack.LookupName("name3")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name3.get());
    ASSERT_TRUE(temp = ns_stack.LookupName("$name3")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name3.get());

    ASSERT_TRUE(temp = ns_stack.LookupName("name_g")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name_g.get());
    ASSERT_FALSE(temp = ns_stack.LookupName("$name_g")) << ns_stack.Dump();
    ASSERT_FALSE(temp = ns_stack.LookupName("::name_g")) << ns_stack.Dump();
    ASSERT_TRUE(temp = ns_stack.LookupName("@::name_g")) << ns_stack.Dump();
    ASSERT_EQ(temp.get(), name_g.get());





    //  name {  {  name2 {  name3::name4 {   ::{  {  ::name5::name6 {...}  }  }  }  }  }  }
    ns_stack.PushScope(Term::Create(TermID::NAMESPACE, "::name5::name6", parser::token_type::NAMESPACE));
    ASSERT_EQ(7, ns_stack.size());
    ASSERT_STREQ("::name5::name6::", ns_stack.GetNamespace().c_str());

    //  name {  {  name2 {  name3::name4 {   ::{  {  ::name5::name6 {  ::{ ... }  }  }  }  }  }  }  }
    ns_stack.PushScope(Term::Create(TermID::NAMESPACE, "::", parser::token_type::NAMESPACE));
    ASSERT_STREQ("::", ns_stack.GetNamespace().c_str());

    //  name {  {  name2 {  name3::name4 {   ::{  {  ::name5::name6 { ::{ name7 {...}  }  }  }  }  }  }  }  }
    ns_stack.PushScope(Term::Create(TermID::NAMESPACE, "name7", parser::token_type::NAMESPACE));
    ASSERT_EQ(9, ns_stack.size());
    ASSERT_STREQ("::name7::", ns_stack.GetNamespace().c_str());

    //  name {  {  name2 {  name3::name4 {   ::{  {  ::name5::name6 { ...  }  }  }  }  }  }  }
    ns_stack.PopScope();
    ns_stack.PopScope();
    ASSERT_EQ(7, ns_stack.size());
    ASSERT_STREQ("::name5::name6::", ns_stack.GetNamespace().c_str());

    //  name {  {  name2 {  name3::name4 {   ::{  {  ...  }  }  }  }  }  }
    ns_stack.PopScope();
    ASSERT_EQ(6, ns_stack.size());
    ASSERT_STREQ("::", ns_stack.GetNamespace(true).c_str()) << ns_stack.Dump();
    ASSERT_STREQ("::3::", ns_stack.GetNamespace(false).c_str()) << ns_stack.Dump();

    //  name {  {  name2 {  name3::name4 {   ::{  ...  }  }  }  }  }
    ns_stack.PopScope();
    ASSERT_EQ(5, ns_stack.size());
    ASSERT_STREQ("::", ns_stack.GetNamespace(true).c_str()) << ns_stack.Dump();
    ASSERT_STREQ("::", ns_stack.GetNamespace(false).c_str()) << ns_stack.Dump();

    //  name {  {  name2 {  name3::name4 {  ...  }  }  }  }
    ns_stack.PopScope();
    ASSERT_EQ(4, ns_stack.size());
    ASSERT_STREQ("name::name2::name3::name4::", ns_stack.GetNamespace(true).c_str());
    ASSERT_STREQ("name::2::name2::name3::name4::", ns_stack.GetNamespace(false).c_str());

    //  name {  {  name2 {  ...  }  }  }
    ns_stack.PopScope();
    ASSERT_EQ(3, ns_stack.size());
    ASSERT_STREQ("name::name2::", ns_stack.GetNamespace(true).c_str());
    ASSERT_STREQ("name::2::name2::", ns_stack.GetNamespace(false).c_str());

    //  name {  {  ...  }  }
    ns_stack.PopScope();
    ASSERT_EQ(2, ns_stack.size());
    ASSERT_STREQ("name::", ns_stack.GetNamespace(true).c_str());
    ASSERT_STREQ("name::2::", ns_stack.GetNamespace(false).c_str());


    //  name {  ...  }
    ns_stack.PopScope();
    ASSERT_EQ(1, ns_stack.size());
    ASSERT_STREQ("name::", ns_stack.GetNamespace(true).c_str());
    ASSERT_STREQ("name::", ns_stack.GetNamespace(false).c_str());

    //  - 
    ns_stack.PopScope();
    ASSERT_EQ(0, ns_stack.size());
    ASSERT_STREQ("", ns_stack.GetNamespace(true).c_str());
    ASSERT_STREQ("", ns_stack.GetNamespace(false).c_str());

    //  ns::name {  ...  }
    ns_stack.PushScope(Term::Create(TermID::NAMESPACE, "ns::name", parser::token_type::NAMESPACE));
    ASSERT_EQ(1, ns_stack.size());
    ASSERT_STREQ("ns::name::", ns_stack.GetNamespace().c_str());


    //  ns::name {  ::{ ... } }
    ns_stack.PushScope(Term::Create(TermID::NAMESPACE, "::", parser::token_type::NAMESPACE));
    ASSERT_EQ(2, ns_stack.size());
    ASSERT_STREQ("::", ns_stack.GetNamespace().c_str());

    //  ns::name {  ... }
    ns_stack.PopScope();
    //  ... 
    ns_stack.PopScope();
    ASSERT_EQ(0, ns_stack.size());

}

TEST(Ast, AstAnalyze) {

    JIT * jit = JIT::ReCreate();

    AstAnalysis analysis(*jit, jit->m_diag.get());

    TermPtr ast = jit->GetParser()->Parse("var1 ::= '1';");

    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();
    ASSERT_EQ(1, ast->m_int_vars.size());
    ASSERT_TRUE(ast->m_int_vars.find("var1$") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ("var1", ast->m_int_vars.find("var1$")->second->m_text.c_str());
    ASSERT_STREQ("var1$", ast->m_int_vars.find("var1$")->second->m_int_name.c_str());

    ast = jit->GetParser()->Parse("$var2 ::= '1';$var3 ::= '1';");

    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_EQ(2, ast->m_int_vars.size());
    ASSERT_TRUE(ast->m_int_vars.find("1::var2$") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_TRUE(ast->m_int_vars.find("1::var3$") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ("$var2", ast->m_int_vars.find("1::var2$")->second->m_text.c_str());
    ASSERT_STREQ("$var3", ast->m_int_vars.find("1::var3$")->second->m_text.c_str());
    ASSERT_STREQ("1::var2$", ast->m_int_vars.find("1::var2$")->second->m_int_name.c_str());
    ASSERT_STREQ("1::var3$", ast->m_int_vars.find("1::var3$")->second->m_int_name.c_str());

    ast = jit->GetParser()->Parse("$var ::= '1'; $var := '2';");

    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_EQ(1, ast->m_int_vars.size());
    ASSERT_TRUE(ast->m_int_vars.find("1::var$") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ("$var", ast->m_int_vars.find("1::var$")->second->m_text.c_str());
    ASSERT_STREQ("1::var$", ast->m_int_vars.find("1::var$")->second->m_int_name.c_str());

    ast = jit->GetParser()->Parse("$var := '1'; $var = '2';");

    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_EQ(1, ast->m_int_vars.size());
    ASSERT_TRUE(ast->m_int_vars.find("1::var$") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ("$var", ast->m_int_vars.find("1::var$")->second->m_text.c_str());
    ASSERT_STREQ("1::var$", ast->m_int_vars.find("1::var$")->second->m_int_name.c_str());


    ast = jit->GetParser()->Parse("$var = '1'; $var := '2';");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast));

    ast = jit->GetParser()->Parse("$var ::= '1'; $var ::= '2';");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast));

    ast = jit->GetParser()->Parse("$var := 'строка'; $var = 2;");
    ASSERT_TRUE(ast);
    ASSERT_EQ(2, ast->m_block.size());
    ASSERT_STREQ(":=", ast->m_block[0]->m_text.c_str());
    ASSERT_TRUE(ast->m_block[0]->m_left);
    ASSERT_STREQ("$var", ast->m_block[0]->m_left->m_text.c_str());
    ASSERT_FALSE(ast->m_block[0]->m_left->m_type);
    ASSERT_TRUE(ast->m_block[0]->m_right);
    ASSERT_STREQ("строка", ast->m_block[0]->m_right->m_text.c_str());
    ASSERT_TRUE(ast->m_block[0]->m_right->m_type);
    ASSERT_STREQ(":StrChar", ast->m_block[0]->m_right->m_type->asTypeString().c_str());

    ASSERT_STREQ("=", ast->m_block[1]->m_text.c_str());
    ASSERT_TRUE(ast->m_block[1]->m_left);
    ASSERT_STREQ("$var", ast->m_block[1]->m_left->m_text.c_str());
    ASSERT_FALSE(ast->m_block[1]->m_left->m_type);
    ASSERT_TRUE(ast->m_block[1]->m_right);
    ASSERT_STREQ("2", ast->m_block[1]->m_right->m_text.c_str());
    ASSERT_TRUE(ast->m_block[1]->m_right->m_type);
    ASSERT_STREQ(":Int8", ast->m_block[1]->m_right->m_type->asTypeString().c_str());

    ASSERT_FALSE(analysis.Analyze(ast, ast));

    ast = jit->GetParser()->Parse("$var := 2; $var = '';");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast));

    //    ast = jit->GetParser()->Parse("$var := 2; $var = _;");
    //    ASSERT_TRUE(ast);
    //    ASSERT_TRUE(analysis.Analyze(ast));
    //    ASSERT_STREQ(":None", ast->m_int_vars["$var"].proto->GetType()->asTypeString().c_str());

    ast = jit->GetParser()->Parse("$var := 2; $var = 2222;");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast->m_int_vars.find("1::var$") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ(":Int16", ast->m_int_vars.find("1::var$")->second->GetType()->asTypeString().c_str());

    ast = jit->GetParser()->Parse("$var:Int8 := 2; $var = 2222;");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast));


    ast = jit->GetParser()->Parse("$var := 2; $var = 22222222;");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast->m_int_vars.find("1::var$") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ(":Int32", ast->m_int_vars.find("1::var$")->second->GetType()->asTypeString().c_str());

    ast = jit->GetParser()->Parse("$var := 2; $var = 2222222222222;");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast->m_int_vars.find("1::var$") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ(":Int64", ast->m_int_vars.find("1::var$")->second->GetType()->asTypeString().c_str());

    ast = jit->GetParser()->Parse("$var := 2; $var = 2.0;");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast->m_int_vars.find("1::var$") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ(":Float32", ast->m_int_vars.find("1::var$")->second->GetType()->asTypeString().c_str());

}

TEST(Ast, AstArgs) {

    JIT * jit = JIT::ReCreate();

    AstAnalysis analysis(*jit, jit->m_diag.get());

    TermPtr ast = jit->GetParser()->Parse("dict := (1,2,); dict[1];");

    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();
    ASSERT_EQ(1, ast->m_int_vars.size());
    ASSERT_TRUE(ast->m_int_vars.find("1::dict$") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ("dict", ast->m_int_vars.find("1::dict$")->second->m_text.c_str());
    ASSERT_STREQ("1::dict$", ast->m_int_vars.find("1::dict$")->second->m_int_name.c_str());

    ast = jit->GetParser()->Parse("dict := (1,2,); (dict[1], dict[2],);");

    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();
    ASSERT_EQ(1, ast->m_int_vars.size());
    ASSERT_TRUE(ast->m_int_vars.find("1::dict$") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ("dict", ast->m_int_vars.find("1::dict$")->second->m_text.c_str());
    ASSERT_STREQ("1::dict$", ast->m_int_vars.find("1::dict$")->second->m_int_name.c_str());

    
    ast = jit->GetParser()->Parse("recursive(arg) ::= { recursive(arg) };");

    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();
    ASSERT_EQ(1, ast->m_int_vars.size());
    ASSERT_TRUE(ast->m_int_vars.find("recursive$") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ("recursive::", ast->m_int_vars.find("recursive$")->second->m_text.c_str());
    ASSERT_STREQ("recursive$", ast->m_int_vars.find("recursive$")->second->m_int_name.c_str());
    std::cout << ast->toString() << "\n";

}

TEST(Ast, Reference) {

    JIT * jit = JIT::ReCreate();

    AstAnalysis analysis(*jit, jit->m_diag.get());

    TermPtr ast = jit->GetParser()->Parse("noref ::= 1;");

    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();
    ASSERT_EQ(1, ast->m_int_vars.size());
    ASSERT_TRUE(ast->m_int_vars.find("noref$") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();


    // Нельзя создавать ссылки
    ast = jit->GetParser()->Parse("noref ::= 1; ref := &noref;");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("noref ::= 1; ref := &&noref;");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("noref ::= 1; ref := &*noref;");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();


    // Нельзя копировать владельцев на одном уровне
    ast = jit->GetParser()->Parse(" { val ::= 1; $local := val; } ");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();


    // Можно копировать в локальные на уровень ниже
    ast = jit->GetParser()->Parse("::val ::= 1; { $local := val; }");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("$val ::= 1; { $local := val; }");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    // Нельзя копировать владельцев в статические переменные
    ast = jit->GetParser()->Parse("val ::= 1; { ::local := val; }");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    // Можно скопировать значение
    ast = jit->GetParser()->Parse("val ::= 1; ::val2 := *val;");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();


    // Можно создавать только правильные ссылки
    ast = jit->GetParser()->Parse("&ref ::= 1; test := &ref;");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&^ref ::= 1; test := &^ref;");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&^ref ::= 1; test := &ref;");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&?ref ::= 1; test := &?ref;");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();
    ast = jit->GetParser()->Parse("&&ref ::= 1; test := &&ref;");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();
    ast = jit->GetParser()->Parse("&*ref ::= 1; test := &*ref;");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();


    // Контроль типа
    ast = jit->GetParser()->Parse("&ref ::= 1; test := &ref; test := 1");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&ref ::= 1; test := &ref; *test := 1");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&^ref ::= 1; test := &^ref; test := 1");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    // Const ref
    ast = jit->GetParser()->Parse("&^ref ::= 1; test := &^ref; *test := 1");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();


    ast = jit->GetParser()->Parse("&?ref ::= 1; test := &?ref; test := 1");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    //with requered!
    ast = jit->GetParser()->Parse("&?ref ::= 1; test := &?ref; *test := 1");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&&ref ::= 1; test := &&ref; test := 1");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    // Cost ref
    ast = jit->GetParser()->Parse("&^ref ::= 1; test := &^ref; *test := 1");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&*ref ::= 1; test := &*ref; test := 1");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&*ref ::= 1; test := &*ref; *test := 1");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();



    // Контроль типа 2
    ast = jit->GetParser()->Parse("&ref ::= 1; test := &ref; test := ref");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&ref ::= 1; test := &ref; *test := ref");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&^ref ::= 1; test := &^ref; test := ref");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&^ref ::= 1; test := &^ref; *test := ref");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();


    ast = jit->GetParser()->Parse("&?ref ::= 1; test := &?ref; test := ref");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&?ref ::= 1; test := &?ref; *test := ref");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&&ref ::= 1; test := &&ref; test := ref");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&&ref ::= 1; test := &&ref; *test := ref");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();


    ast = jit->GetParser()->Parse("&*ref ::= 1; test := &*ref; test := ref");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&*ref ::= 1; test := &*ref; *test := ref");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();


    // Контроль типа 3
    ast = jit->GetParser()->Parse("&val ::= 1; ref := &val; ref := *val");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&val ::= 1; ref := &val; *ref := *val");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&^val ::= 1; ref := &^val; ref := *val");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    // Read only
    ast = jit->GetParser()->Parse("&^val ::= 1; ref := &^val; *ref := *val");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();


    ast = jit->GetParser()->Parse("&?val ::= 1; ref := &?val; ref := *val");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();


    ast = jit->GetParser()->Parse("&?val ::= 1; ref := &?val; *ref := *val");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&&val ::= 1; ref := &&val; ref := *val");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&&val ::= 1; ref := &&val; *ref := *val");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();


    ast = jit->GetParser()->Parse("&*val ::= 1; ref := &*val; ref := *val");
    ASSERT_TRUE(ast);
    ASSERT_FALSE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&*val ::= 1; ref := &*val; *ref := *val");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();


    ast = jit->GetParser()->Parse("& lite ::= 2; &^ lite_ro ::= 22; ");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();
    ASSERT_EQ(2, ast->m_int_vars.size()) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&? thread ::= 3; &?^ thread_ro ::= 33; ");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();
    ASSERT_EQ(2, ast->m_int_vars.size()) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&& mono ::= 4; &&^ mono_ro ::= 44; &?(mono) thread := 123; &?^(mono_ro, __timeout__=1000) thread2 := 123;");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();
    ASSERT_EQ(4, ast->m_int_vars.size()) << ast->m_int_vars.Dump();

    ast = jit->GetParser()->Parse("&* multi ::= 5; &*^ multi_ro ::= 55; &?(multi) other := 123; &?^(multi_ro, __timeout__=1000) other2 := 123;");
    ASSERT_TRUE(ast);
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << ast->m_int_vars.Dump();
    ASSERT_EQ(4, ast->m_int_vars.size()) << ast->m_int_vars.Dump();
}

/*
 * 
 */

TEST(Ast, Namespace) {

    JIT * jit = JIT::ReCreate();
    AstAnalysis analysis(*jit, jit->m_diag.get());

    TermPtr ast = jit->GetParser()->Parse("$var := 1;");
    ASSERT_TRUE(ast);
    ASSERT_EQ(0, ast->m_int_vars.size());
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_EQ(1, ast->m_int_vars.size());
    ASSERT_TRUE(ast->m_int_vars.find("var$") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ("$var", ast->m_int_vars.find("var$")->second->m_text.c_str());
    ASSERT_STREQ("var$", ast->m_int_vars.find("var$")->second->m_int_name.c_str());

    ast = jit->GetParser()->Parse("var := 1;");
    ASSERT_TRUE(ast);
    ASSERT_EQ(0, ast->m_int_vars.size());
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast->m_int_vars.find("var$") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ("var", ast->m_int_vars.find("var$")->second->m_text.c_str());
    ASSERT_STREQ("var$", ast->m_int_vars.find("var$")->second->m_int_name.c_str());


    ast = jit->GetParser()->Parse("::ns::var := 1;");
    ASSERT_TRUE(ast);
    ASSERT_EQ(0, ast->m_int_vars.size());
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast->m_int_vars.find("::ns::var::") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ("::ns::var", ast->m_int_vars.find("::ns::var::")->second->m_text.c_str());
    ASSERT_STREQ("::ns::var::", ast->m_int_vars.find("::ns::var::")->second->m_int_name.c_str());

    ast = jit->GetParser()->Parse("ns::var := 1;");
    ASSERT_TRUE(ast);
    ASSERT_EQ(0, ast->m_int_vars.size());
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast->m_int_vars.find("ns::var::") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ("ns::var", ast->m_int_vars.find("ns::var::")->second->m_text.c_str());
    ASSERT_STREQ("ns::var::", ast->m_int_vars.find("ns::var::")->second->m_int_name.c_str());


    ast = jit->GetParser()->Parse("name { var := 1; }");
    ASSERT_TRUE(ast);
    ASSERT_EQ(0, ast->m_int_vars.size());
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast->m_int_vars.find("1::name::var$") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ("var", ast->m_int_vars.find("1::name::var$")->second->m_text.c_str());
    ASSERT_STREQ("1::name::var$", ast->m_int_vars.find("1::name::var$")->second->m_int_name.c_str());


    ASSERT_EQ(1, ast->m_block.size());
    ASSERT_STREQ(":=", ast->m_block[0]->m_text.c_str());
    ASSERT_TRUE(ast->m_block[0]->m_left);
    ASSERT_STREQ("1::name::var$", ast->m_block[0]->m_left->m_int_name.c_str());
    ASSERT_TRUE(ast->m_block[0]->m_right);
    ASSERT_STREQ("1", ast->m_block[0]->m_right->m_text.c_str());

    ast = jit->GetParser()->Parse("name:: { var := 1; }");
    ASSERT_TRUE(ast);
    ASSERT_EQ(0, ast->m_int_vars.size());
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast->m_int_vars.find("1::name::var$") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ("var", ast->m_int_vars.find("1::name::var$")->second->m_text.c_str());
    ASSERT_STREQ("1::name::var$", ast->m_int_vars.find("1::name::var$")->second->m_int_name.c_str());

    ast = jit->GetParser()->Parse("ns::name:: { var := 1; }");
    ASSERT_TRUE(ast);
    ASSERT_EQ(0, ast->m_int_vars.size());
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast->m_int_vars.find("1::ns::name::var$") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ("var", ast->m_int_vars.find("1::ns::name::var$")->second->m_text.c_str());
    ASSERT_STREQ("1::ns::name::var$", ast->m_int_vars.find("1::ns::name::var$")->second->m_int_name.c_str());

    ast = jit->GetParser()->Parse("name:: { ::var := 1; }");
    ASSERT_TRUE(ast);
    ASSERT_EQ(0, ast->m_int_vars.size());
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast->m_int_vars.find("::var::") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ("::var", ast->m_int_vars.find("::var::")->second->m_text.c_str());
    ASSERT_STREQ("::var::", ast->m_int_vars.find("::var::")->second->m_int_name.c_str());

    ast = jit->GetParser()->Parse("::name:: { ::ns2::var := 1; }");
    ASSERT_TRUE(ast);
    ASSERT_EQ(0, ast->m_int_vars.size());
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast->m_int_vars.find("::ns2::var::") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ("::ns2::var", ast->m_int_vars.find("::ns2::var::")->second->m_text.c_str());
    ASSERT_STREQ("::ns2::var::", ast->m_int_vars.find("::ns2::var::")->second->m_int_name.c_str());

    ast = jit->GetParser()->Parse("name { $var := 1; }");
    ASSERT_TRUE(ast);
    ASSERT_EQ(0, ast->m_int_vars.size());
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast->m_int_vars.find("1::name::var$") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ("$var", ast->m_int_vars.find("1::name::var$")->second->m_text.c_str());
    ASSERT_STREQ("1::name::var$", ast->m_int_vars.find("1::name::var$")->second->m_int_name.c_str());

    ast = jit->GetParser()->Parse("name { { $var := 1; } }");
    ASSERT_TRUE(ast);
    ASSERT_EQ(0, ast->m_int_vars.size());
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast->m_int_vars.find("1::name::2::var$") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ("$var", ast->m_int_vars.find("1::name::2::var$")->second->m_text.c_str());
    ASSERT_STREQ("1::name::2::var$", ast->m_int_vars.find("1::name::2::var$")->second->m_int_name.c_str());

    ast = jit->GetParser()->Parse("ns{ name { { $var := 1; } } }");
    ASSERT_TRUE(ast);
    ASSERT_EQ(0, ast->m_int_vars.size());
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast->m_int_vars.find("1::ns::name::2::var$") != ast->m_int_vars.end()) << ast->m_int_vars.Dump();
    ASSERT_STREQ("$var", ast->m_int_vars.find("1::ns::name::2::var$")->second->m_text.c_str());
    ASSERT_STREQ("1::ns::name::2::var$", ast->m_int_vars.find("1::ns::name::2::var$")->second->m_int_name.c_str());

}

TEST(Ast, Interruption) {

    JIT * jit = JIT::ReCreate();

    AstAnalysis analysis(*jit, jit->m_diag.get());

    TermPtr ast = jit->GetParser()->Parse(":: --;");
    ASSERT_TRUE(ast);
    ASSERT_STREQ("::", ast->m_namespace->m_text.c_str());
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast->m_namespace);
    ASSERT_STREQ("::", ast->m_namespace->m_text.c_str());

    ASSERT_NO_THROW(ast = jit->GetParser()->Parse("name {  name -- };"));
    ASSERT_TRUE(ast);
    ASSERT_EQ(1, ast->m_block.size());
    ASSERT_STREQ("--", ast->m_block[0]->m_text.c_str());
    ASSERT_TRUE(ast->m_block[0]->isBlock());
    ASSERT_TRUE(ast->m_block[0]->m_block[0]->isInterrupt());
    ASSERT_TRUE(ast->m_block[0]->m_block[0]->m_namespace);
    ASSERT_STREQ("name", ast->m_block[0]->m_block[0]->m_namespace->m_text.c_str());
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_STREQ("name", ast->m_block[0]->m_block[0]->m_namespace->m_text.c_str());

    ASSERT_NO_THROW(ast = jit->GetParser()->Parse("ns::name {  ns::name -+ };"));
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast);

    ASSERT_NO_THROW(ast = jit->GetParser()->Parse("ns { name {  ns::name +- } };"));
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast);

    ASSERT_NO_THROW(ast = jit->GetParser()->Parse("ns { { name {  { ns::name ++ } } } };"));
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast);

    ASSERT_NO_THROW(ast = jit->GetParser()->Parse("{ ns { name {  ns::name +- } } };"));
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast);

    ASSERT_NO_THROW(ast = jit->GetParser()->Parse("ns { { name {  local:: { ns::name ++ } } } };"));
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast);

    ASSERT_NO_THROW(ast = jit->GetParser()->Parse("ns { { name {  local:: { local:: ++ } } } };"));
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast);

    ASSERT_NO_THROW(ast = jit->GetParser()->Parse("ns { { name {  local:: { ns ++ } } } };"));
    ASSERT_TRUE(analysis.Analyze(ast, ast));
    ASSERT_TRUE(ast);


    ASSERT_NO_THROW(ast = jit->GetParser()->Parse("ns { { name {  local:: { bad_name ++ } } } };"));
    ASSERT_FALSE(analysis.Analyze(ast, ast));

    //    ast = jit->GetParser()->Parse("name {  bad_name -- };");
    //    ASSERT_TRUE(ast);
    //    ASSERT_FALSE(analysis.Analyze(ast));
    //
    //    ast = jit->GetParser()->Parse("name {  bad_name:: -- };");
    //    ASSERT_TRUE(ast);
    //    ASSERT_FALSE(analysis.Analyze(ast));
    //
    //    ast = jit->GetParser()->Parse("name1 { name2 { name3 {  ::bad_name -- }}};");
    //    ASSERT_TRUE(ast);
    //    ASSERT_FALSE(analysis.Analyze(ast));
    //
    //    ast = jit->GetParser()->Parse("name { name2::name3 {  ::bad_name::name -- }};");
    //    ASSERT_TRUE(ast);
    //    ASSERT_FALSE(analysis.Analyze(ast));
    //
    //    ast = jit->GetParser()->Parse("name {  name::bad -- };");
    //    ASSERT_TRUE(ast);
    //    ASSERT_FALSE(analysis.Analyze(ast));
    //
    //    //    ast = jit->GetParser()->Parse("@$$ --;");
    //    //    ASSERT_TRUE(ast);
    //    //    ASSERT_STREQ("@$$", ast->m_namespace->m_text.c_str());
    //    //    ASSERT_TRUE(analysis.Analyze(ast));
    //    //    ASSERT_STREQ("", ast->m_namespace->m_text.c_str());
    //
    //    //    ast = jit->GetParser()->Parse("name {  @$$ -- };");
    //    //    ASSERT_TRUE(ast);
    //    //    ASSERT_EQ(1, ast->m_block.size());
    //    //    ASSERT_STREQ("--", ast->m_block[0]->m_text.c_str());
    //    //    ASSERT_TRUE(ast->m_block[0]->m_namespace);
    //    //    ASSERT_STREQ("@$$", ast->m_block[0]->m_namespace->m_text.c_str());
    //    //    ASSERT_TRUE(analysis.Analyze(ast));
    //    //    ASSERT_STREQ("name", ast->m_block[0]->m_namespace->m_text.c_str());
    //
    //    ast = jit->GetParser()->Parse("@::var := 1");
    //    ASSERT_TRUE(ast);
    //    ASSERT_EQ(0, ast->m_variables.size());
    //    ASSERT_EQ(3, rt->size() - buildin_count);
    //    ASSERT_TRUE(analysis.Analyze(ast));
    //    ASSERT_EQ(3, rt->size() - buildin_count);
    //    ASSERT_EQ(1, ast->m_variables.size());
    //    ASSERT_STREQ("$$var", ast->m_variables.begin()->second.proto->m_text.c_str());
    //
    //    ast = jit->GetParser()->Parse("@::var ::= 1");
    //    ASSERT_TRUE(ast);
    //    ASSERT_EQ(0, ast->m_variables.size());
    //    ASSERT_EQ(3, rt->size() - buildin_count);
    //    ASSERT_TRUE(analysis.Analyze(ast));
    //    ASSERT_EQ(3, rt->size() - buildin_count);
    //    ASSERT_EQ(1, ast->m_variables.size());
    //    ASSERT_STREQ("$$var", ast->m_variables.begin()->second.proto->m_text.c_str());
    //
    //    ast = jit->GetParser()->Parse("ns { name { @::var := 1 } };");
    //    ASSERT_TRUE(ast);
    //    ASSERT_EQ(0, ast->m_variables.size());
    //    ASSERT_EQ(3, rt->size() - buildin_count);
    //    ASSERT_TRUE(analysis.Analyze(ast));
    //    ASSERT_EQ(3, rt->size() - buildin_count);
    //    ASSERT_EQ(1, ast->m_variables.size());
    //    ASSERT_STREQ("ns::name::var", ast->m_variables.begin()->second.proto->m_text.c_str());
    //
    //
    //    //    ast = jit->GetParser()->Parse("ns { name { { var7 := @:: } } };");
    //    //    ASSERT_TRUE(ast);
    //    //    ASSERT_EQ(0, ast->m_variables.size());
    //    //    ASSERT_EQ(3, rt->size() - buildin_count);
    //    //    ASSERT_TRUE(analysis.Analyze(ast));
    //    //    ASSERT_EQ(3, rt->size() - buildin_count);
    //    //    ASSERT_EQ(1, ast->m_variables.size()) << ast->m_variables.Dump();
    //    //    ASSERT_STREQ("ns::name$var7", ast->m_variables.begin()->second.proto->m_text.c_str());
    //
    //    //    // В каком модуле ::$var_glob ???
    //    //    ast = jit->GetParser()->Parse(":: { var_glob := 1 };");
    //    //    ASSERT_TRUE(ast);
    //    //    ASSERT_EQ(0, ast->m_variables.size());
    //    //    ASSERT_EQ(3, rt->size() - buildin_count);
    //    //    ASSERT_TRUE(analysis.Analyze(ast));
    //    //    ASSERT_EQ(0, ast->m_variables.size()) << ast->m_variables.begin()->second.proto->m_text.c_str();
    //    //    ASSERT_EQ(4, rt->size() - buildin_count);
    //    //    ASSERT_STREQ("::$var_glob", ast->m_variables.begin()->second.proto->m_text.c_str());
    //
    //
    //    /*
    //     * Функции
    //     */
    //
    //    buildin_count = rt->size();
    //    ast = jit->GetParser()->Parse("func() ::= {};");
    //    ASSERT_TRUE(ast);
    //    ASSERT_EQ(0, rt->size() - buildin_count);
    //    ASSERT_EQ(0, ast->m_variables.size());
    //    ASSERT_TRUE(analysis.Analyze(ast));
    //    ASSERT_EQ(0, rt->size() - buildin_count);
    //    ASSERT_EQ(1, ast->m_variables.size());
    //    ASSERT_STREQ("$$func", ast->m_variables.begin()->second.proto->m_text.c_str());
    //
    //    ast = jit->GetParser()->Parse("@::func() ::= {};");
    //    ASSERT_TRUE(ast);
    //
    //    ASSERT_TRUE(ast->m_left);
    //    ASSERT_TRUE(ast->m_left->isCall()) << ast->toString();
    //    ASSERT_STREQ("@::func", ast->m_left->m_text.c_str());
    //
    //    ASSERT_EQ(0, rt->size() - buildin_count);
    //    ASSERT_EQ(0, ast->m_variables.size());
    //    ASSERT_TRUE(analysis.Analyze(ast));
    //    ASSERT_EQ(0, rt->size() - buildin_count);
    //    ASSERT_EQ(1, ast->m_variables.size()) << ast->m_variables.Dump();
    //    ASSERT_TRUE(ast->m_variables.begin()->second.proto);
    //    ASSERT_STREQ("$$func", ast->m_variables.begin()->second.proto->m_text.c_str());
    //
    //
    //    ast = jit->GetParser()->Parse("name { func() ::= {}  };");
    //    ASSERT_TRUE(ast);
    //
    //    ASSERT_EQ(1, ast->m_block.size());
    //    ASSERT_TRUE(ast->m_block[0]->m_left);
    //    ASSERT_TRUE(ast->m_block[0]->m_left->isCall()) << ast->m_block[0]->toString();
    //    ASSERT_STREQ("func", ast->m_block[0]->m_left->m_text.c_str());
    //
    //    ASSERT_EQ(0, rt->size() - buildin_count);
    //    ASSERT_EQ(0, ast->m_variables.size());
    //    ASSERT_TRUE(analysis.Analyze(ast));
    //    ASSERT_EQ(0, rt->size() - buildin_count);
    //    ASSERT_EQ(1, ast->m_variables.size()) << ast->m_variables.Dump();
    //    ASSERT_TRUE(ast->m_variables.begin()->second.proto);
    //    ASSERT_STREQ("name$$func", ast->m_variables.begin()->second.proto->m_text.c_str());
    //    ASSERT_STREQ("name$$func", ast->m_block[0]->m_left->m_text.c_str());
    //
    //    ast = jit->GetParser()->Parse("ns { name { func() ::= {}  } };");
    //    ASSERT_TRUE(ast);
    //
    //    ASSERT_EQ(1, ast->m_block.size());
    //    ASSERT_TRUE(ast->m_block[0]->m_left);
    //    ASSERT_TRUE(ast->m_block[0]->m_left->isCall()) << ast->m_block[0]->toString();
    //    ASSERT_STREQ("func", ast->m_block[0]->m_left->m_text.c_str());
    //
    //    ASSERT_EQ(0, rt->size() - buildin_count);
    //    ASSERT_EQ(0, ast->m_variables.size());
    //    ASSERT_TRUE(analysis.Analyze(ast));
    //    ASSERT_EQ(0, rt->size() - buildin_count);
    //    ASSERT_EQ(1, ast->m_variables.size()) << ast->m_variables.Dump();
    //    ASSERT_TRUE(ast->m_variables.begin()->second.proto);
    //    ASSERT_STREQ("ns::name$$func", ast->m_variables.begin()->second.proto->m_text.c_str());
    //    ASSERT_STREQ("ns::name$$func", ast->m_block[0]->m_left->m_text.c_str());
    //
    //    //    ast = jit->GetParser()->Parse("var := 1;");
    //    //    ASSERT_TRUE(ast);
    //    //    ASSERT_EQ(0, ast->m_variables.size());
    //    //    ASSERT_EQ(0, rt->size() - buildin_count);
    //    //    ASSERT_TRUE(analysis.Analyze(ast));
    //    //    ASSERT_EQ(0, rt->size() - buildin_count);
    //    //    ASSERT_EQ(1, ast->m_variables.size());
    //    //    ASSERT_STREQ("$var", ast->m_variables.begin()->second.proto->m_text.c_str());
    //    //
    //    //
    //    //    ast = jit->GetParser()->Parse("ns::var := 1;");
    //    //    ASSERT_TRUE(ast);
    //    //    ASSERT_EQ(0, ast->m_variables.size());
    //    //    ASSERT_EQ(0, rt->size() - buildin_count);
    //    //    ASSERT_TRUE(analysis.Analyze(ast));
    //    //    ASSERT_EQ(0, rt->size() - buildin_count);
    //    //    ASSERT_EQ(1, ast->m_variables.size());
    //    //    ASSERT_STREQ("ns::var", ast->m_variables.begin()->second.proto->m_text.c_str());
    //    //
    //    //
    //    //    ast = jit->GetParser()->Parse("::ns::var := 1;");
    //    //    ASSERT_TRUE(ast);
    //    //    ASSERT_EQ(0, ast->m_variables.size());
    //    //    ASSERT_EQ(0, rt->size() - buildin_count);
    //    //    ASSERT_TRUE(analysis.Analyze(ast));
    //    //    ASSERT_EQ(1, rt->size() - buildin_count);
    //    //    ASSERT_EQ(0, ast->m_variables.size());
    //    //    ASSERT_STREQ("::ns::var", (*rt)["::ns::var"].proto->m_text.c_str());
    //    //
    //    //
    //    //    ast = jit->GetParser()->Parse("name { var := 1 };");
    //    //    ASSERT_TRUE(ast);
    //    //    ASSERT_EQ(0, ast->m_variables.size());
    //    //    ASSERT_EQ(1, rt->size() - buildin_count);
    //    //    ASSERT_TRUE(analysis.Analyze(ast));
    //    //    ASSERT_EQ(1, rt->size() - buildin_count);
    //    //    ASSERT_EQ(1, ast->m_variables.size());
    //    //
    //    //    ASSERT_EQ(1, ast->m_block.size());
    //    //    ASSERT_STREQ(":=", ast->m_block[0]->m_text.c_str());
    //    //    ASSERT_TRUE(ast->m_block[0]->m_left);
    //    //    ASSERT_STREQ("name$var", ast->m_block[0]->m_left->m_text.c_str());
    //    //    ASSERT_TRUE(ast->m_block[0]->m_right);
    //    //    ASSERT_STREQ("1", ast->m_block[0]->m_right->m_text.c_str());
    //    //
    //    //    ASSERT_STREQ("name$var", ast->m_variables.begin()->second.proto->m_text.c_str());
    //    //
    //    //    ast = jit->GetParser()->Parse("name:: { var := 1 };");
    //    //    ASSERT_TRUE(ast);
    //    //    ASSERT_EQ(0, ast->m_variables.size());
    //    //    ASSERT_EQ(1, rt->size() - buildin_count);
    //    //    ASSERT_TRUE(analysis.Analyze(ast));
    //    //    ASSERT_EQ(1, rt->size() - buildin_count);
    //    //    ASSERT_EQ(1, ast->m_variables.size());
    //    //    ASSERT_STREQ("name$var", ast->m_variables.begin()->second.proto->m_text.c_str());
    //    //
    //    //
    //    //    ast = jit->GetParser()->Parse("ns::name:: { var := 1 };");
    //    //    ASSERT_TRUE(ast);
    //    //    ASSERT_EQ(0, ast->m_variables.size());
    //    //    ASSERT_EQ(1, rt->size() - buildin_count);
    //    //    ASSERT_TRUE(analysis.Analyze(ast));
    //    //    ASSERT_EQ(1, rt->size() - buildin_count);
    //    //    ASSERT_EQ(1, ast->m_variables.size());
    //    //    ASSERT_STREQ("ns::name$var", ast->m_variables.begin()->second.proto->m_text.c_str());
    //    //
    //    //    ast = jit->GetParser()->Parse("name:: { ::var := 1 };");
    //    //    ASSERT_TRUE(ast);
    //    //    ASSERT_EQ(0, ast->m_variables.size());
    //    //    ASSERT_EQ(1, rt->size() - buildin_count);
    //    //    ASSERT_TRUE(analysis.Analyze(ast));
    //    //    ASSERT_EQ(2, rt->size() - buildin_count);
    //    //    ASSERT_EQ(0, ast->m_variables.size());
    //    //    ASSERT_STREQ("::var", (*rt)["::var"].proto->m_text.c_str());
    //    //
    //    //    ast = jit->GetParser()->Parse("::name:: { ::ns2::var := 1 };");
    //    //    ASSERT_TRUE(ast);
    //    //    ASSERT_EQ(0, ast->m_variables.size());
    //    //    ASSERT_EQ(2, rt->size() - buildin_count);
    //    //    ASSERT_TRUE(analysis.Analyze(ast));
    //    //    ASSERT_EQ(3, rt->size() - buildin_count);
    //    //    ASSERT_EQ(0, ast->m_variables.size());
    //    //    ASSERT_STREQ("::ns2::var", (*rt)["::ns2::var"].proto->m_text.c_str());
    //    //
    //    //
    //    //    ast = jit->GetParser()->Parse("name { $var := 1 };");
    //    //    ASSERT_TRUE(ast);
    //    //    ASSERT_EQ(0, ast->m_variables.size());
    //    //    ASSERT_EQ(3, rt->size() - buildin_count);
    //    //    ASSERT_TRUE(analysis.Analyze(ast));
    //    //    ASSERT_EQ(3, rt->size() - buildin_count);
    //    //    ASSERT_EQ(1, ast->m_variables.size());
    //    //    ASSERT_STREQ("name$var", ast->m_variables.begin()->second.proto->m_text.c_str());
    //    //
    //    //    ast = jit->GetParser()->Parse("name { { $var := 1 } };");
    //    //    ASSERT_TRUE(ast);
    //    //    ASSERT_EQ(0, ast->m_variables.size());
    //    //    ASSERT_EQ(3, rt->size() - buildin_count);
    //    //    ASSERT_TRUE(analysis.Analyze(ast));
    //    //    ASSERT_EQ(3, rt->size() - buildin_count);
    //    //    ASSERT_EQ(1, ast->m_variables.size());
    //    //    ASSERT_STREQ("name$var", ast->m_variables.begin()->second.proto->m_text.c_str());
}

TEST(Ast, Dims) {
    // :Bool[...]() - OK
    // :Bool[0]() - Error !!!!!!!!!!!!!!!!!
    // :Bool() - Ok
    // :Bool[..., ...]() - Error !!!!!!!!!!!!!!!!!

    JIT * jit = JIT::ReCreate();
    AstAnalysis analysis(*jit, jit->m_diag.get());

    TermPtr ast;
    ast = jit->GetParser()->Parse(":Bool(1)");
    ASSERT_TRUE(analysis.Analyze(ast, ast)) << Dump(*jit);

    ASSERT_ANY_THROW(ast = jit->GetParser()->Parse(":Bool[](1)"));

    AstAnalysis analysis2(*jit, jit->m_diag.get());
    ast = jit->GetParser()->Parse(":Bool[...](1)");
    ASSERT_TRUE(analysis2.Analyze(ast, ast));

    AstAnalysis analysis3(*jit, jit->m_diag.get());
    ast = jit->GetParser()->Parse(":Bool[ ..., ...](1)");
    ASSERT_FALSE(analysis3.Analyze(ast, ast));

    AstAnalysis analysis4(*jit, jit->m_diag.get());
    ast = jit->GetParser()->Parse(":Bool[0](1)");
    ASSERT_TRUE(analysis4.Analyze(ast, ast));

    AstAnalysis analysis5(*jit, jit->m_diag.get());
    ast = jit->GetParser()->Parse(":Bool[_](1)");
    ASSERT_TRUE(analysis5.Analyze(ast, ast));

    AstAnalysis analysis6(*jit, jit->m_diag.get());
    ast = jit->GetParser()->Parse(":Bool[0..1..2](1)");
    ASSERT_TRUE(analysis6.Analyze(ast, ast));

    AstAnalysis analysis7(*jit, jit->m_diag.get());
    ast = jit->GetParser()->Parse("0.._..2");
    ASSERT_TRUE(analysis7.Analyze(ast, ast));
}


//TEST(Ast, ClassFunc) {
//
//    Context::Reset();
//    Context ctx(RunTime::Init());
//
//    size_t buildin_count = ctx.m_runtime->size();
//
//    ASSERT_EQ(0, ctx.m_runtime->m_macro->m_ns_stack.size());
//    ASSERT_EQ(0, ctx.size());
//    
//    ASSERT_EQ(0, ctx.size());
//
//    ObjPtr class1 = ctx.ExecStr(":class1 := :Class() { func1() := {1}; }");
//    ASSERT_TRUE(class1);
//    ASSERT_EQ(ObjType::Type, class1->m_var_type_current) << toString(class1->m_var_type_current);
//    ASSERT_EQ(ObjType::Class, class1->m_var_type_fixed) << toString(class1->m_var_type_fixed);
//    ASSERT_STREQ(":class1", class1->getName().c_str());
//
//    //    ASSERT_TRUE(ctx.size() > 0) << ctx.Dump("; ").c_str();
//    //    ASSERT_STREQ(":class1", ctx.at(0).first.c_str()) << ctx.Dump("; ").c_str();
//    //    ASSERT_TRUE(ctx.find(":class1") != ctx.end()) << ctx.Dump("; ").c_str();
//    //    ASSERT_STREQ("class1::func1", ctx.at(1).first.c_str()) << ctx.Dump("; ").c_str();
//    ASSERT_EQ(2, ctx.m_runtime->size()-buildin_count);
//    ASSERT_STREQ("class1::class1", ctx.m_terms->at(0).first.c_str());
//    ASSERT_STREQ("class1::func1", ctx.m_terms->at(1).first.c_str());
//    ObjPtr func1 = ctx.ExecStr("class1::func1");
//    ASSERT_EQ(1, func1->Call(&ctx)->GetValueAsInteger());
//
//    ASSERT_STREQ("class::class", MakeConstructorName(":class").c_str());
//    ASSERT_STREQ("ns::class::class", MakeConstructorName(":ns::class").c_str());
//    ASSERT_STREQ("ns::class::class::class", MakeConstructorName(":ns::class::class").c_str());
//    
//    EXPECT_ANY_THROW(ctx.ExecStr(":class_dup_func1 := :class1 { func1() ::= {11}; }"));
//    EXPECT_ANY_THROW(ctx.ExecStr(":class_func_not_found := :class1 { func_not_found() = {0}; }"));
//
//    ObjPtr class2 = ctx.ExecStr(":class2 := :class1() { func2() := {2}; }");
//    ASSERT_TRUE(class2);
//    ASSERT_EQ(ObjType::Type, class2->m_var_type_current) << toString(class2->m_var_type_current);
//    ASSERT_EQ(ObjType::Class, class2->m_var_type_fixed) << toString(class2->m_var_type_fixed);
//    ASSERT_STREQ(":class2", class2->getName().c_str());
//
//    ASSERT_TRUE(ctx.size() > 2) << ctx.Dump("; ").c_str();
//    //    ASSERT_STREQ(":class2", ctx.at(2).first.c_str()) << ctx.Dump("; ").c_str();
//
//    std::string dump;
//    for (int i = 0; i < ctx.m_terms->size(); i++) {
//        dump += ctx.m_terms->at(i).first;
//        dump += "; ";
//    }
//    ASSERT_EQ(5, ctx.m_terms->size()) << dump;
//    ASSERT_STREQ("class2::class2", ctx.m_terms->at(2).first.c_str());
//    ASSERT_STREQ("class2::func1", ctx.m_terms->at(3).first.c_str());
//    ASSERT_STREQ("class2::func2", ctx.m_terms->at(4).first.c_str());
//    ObjPtr func2_1 = ctx.ExecStr("class2::func1");
//    ASSERT_EQ(1, func2_1->Call(&ctx)->GetValueAsInteger());
//    ObjPtr func2_2 = ctx.ExecStr("class2::func2");
//    ASSERT_EQ(2, func2_2->Call(&ctx)->GetValueAsInteger());
//
//
//
//    ObjPtr func_name = ctx.ExecStr("class2 { func_name() := {'func_name'}; }");
//    ASSERT_TRUE(func_name);
//    ASSERT_EQ(ObjType::EVAL_FUNCTION, func_name->m_var_type_current) << toString(func_name->m_var_type_current);
//    ASSERT_STREQ("class2::func_name", func_name->getName().c_str());
//
//    dump = "";
//    for (int i = 0; i < ctx.m_terms->size(); i++) {
//        dump += ctx.m_terms->at(i).first;
//        dump += "; ";
//    }
//    ASSERT_EQ(6, ctx.m_terms->size()) << dump;
//    ASSERT_STREQ("class2::func_name", ctx.m_terms->at(5).first.c_str());
//    ASSERT_STREQ("func_name", func_name->Call(&ctx)->GetValueAsString().c_str());
//
//
//    ObjPtr class3 = ctx.ExecStr(":class3 := :class1, :class2 { func3() := {33}; func4() := {4}; }");
//    ASSERT_TRUE(class3);
//    ASSERT_EQ(ObjType::Type, class3->m_var_type_current) << toString(class3->m_var_type_current);
//    ASSERT_EQ(ObjType::Class, class3->m_var_type_fixed) << toString(class3->m_var_type_fixed);
//    ASSERT_STREQ(":class3", class3->getName().c_str());
//
//    ASSERT_TRUE(ctx.size() > 4) << ctx.Dump("; ").c_str();
//
//    dump = "";
//    for (int i = 0; i < ctx.m_terms->size(); i++) {
//        dump += ctx.m_terms->at(i).first;
//        dump += "; ";
//    }
//    ASSERT_EQ(12, ctx.m_terms->size()) << dump;
//    ASSERT_STREQ("class3::class3", ctx.m_terms->at(6).first.c_str());
//    ASSERT_STREQ("class3::func1", ctx.m_terms->at(7).first.c_str());
//    ASSERT_STREQ("class3::func3", ctx.m_terms->at(8).first.c_str());
//    ASSERT_STREQ("class3::func4", ctx.m_terms->at(9).first.c_str());
//    ASSERT_STREQ("class3::func2", ctx.m_terms->at(10).first.c_str());
//    ASSERT_STREQ("class3::func_name", ctx.m_terms->at(11).first.c_str());
//    ObjPtr func3_1 = ctx.ExecStr("class3::func1");
//    ASSERT_EQ(1, func3_1->Call(&ctx)->GetValueAsInteger());
//    ObjPtr func3_2 = ctx.ExecStr("class3::func2");
//    ASSERT_EQ(2, func3_2->Call(&ctx)->GetValueAsInteger());
//    ObjPtr func3_3 = ctx.ExecStr("class3::func3");
//    ASSERT_EQ(33, func3_3->Call(&ctx)->GetValueAsInteger());
//    ObjPtr func3_4 = ctx.ExecStr("class3::func4");
//    ASSERT_EQ(4, func3_4->Call(&ctx)->GetValueAsInteger());
//    ObjPtr result = ctx.ExecStr("class3::func_name()");
//    ASSERT_TRUE(result);
//    ASSERT_STREQ("func_name", result->GetValueAsString().c_str());
//
//
//
//
//    ObjPtr var1 = ctx.ExecStr("var1 := :class1()");
//    ASSERT_TRUE(var1);
//    ObjPtr var1_1 = ctx.ExecStr("var1.func1()");
//    ASSERT_TRUE(var1_1);
//    ASSERT_EQ(1, var1_1->GetValueAsInteger());
//
//
//    ObjPtr var2 = ctx.ExecStr("var2 := :class2()");
//    ASSERT_TRUE(var2);
//    ObjPtr var2_1 = ctx.ExecStr("var2.func1()");
//    ASSERT_TRUE(var2_1);
//    ASSERT_EQ(1, var2_1->GetValueAsInteger());
//    ObjPtr var2_2 = ctx.ExecStr("var2.func2()");
//    ASSERT_TRUE(var2_2);
//    ASSERT_EQ(2, var2_2->GetValueAsInteger());
//
//
//    ObjPtr var3 = ctx.ExecStr("var3 := :class3()");
//    ASSERT_TRUE(var3);
//    ObjPtr var3_1 = ctx.ExecStr("var3.func1()");
//    ASSERT_TRUE(var3_1);
//    ASSERT_EQ(1, var3_1->GetValueAsInteger());
//    ObjPtr var3_2 = ctx.ExecStr("var3.func2()");
//    ASSERT_TRUE(var3_2);
//    ASSERT_EQ(2, var3_2->GetValueAsInteger());
//    ObjPtr var3_3 = ctx.ExecStr("var3.func3()");
//    ASSERT_TRUE(var3_3);
//    ASSERT_EQ(33, var3_3->GetValueAsInteger());
//    ObjPtr var3_4 = ctx.ExecStr("var3.func4()");
//    ASSERT_TRUE(var3_4);
//    ASSERT_EQ(4, var3_4->GetValueAsInteger());
//
//}
//
//TEST(Ast, ClassProp) {
//
//    Context::Reset();
//    Context ctx(RunTime::Init());
//
//    ASSERT_EQ(0, ctx.m_runtime->m_macro->m_ns_stack.size());
//    ASSERT_EQ(0, ctx.size());
//    ASSERT_EQ(0, ctx.m_terms->size());
//
//    ASSERT_EQ(0, ctx.size());
//
//    ObjPtr class1 = ctx.ExecStr(":class1 := :Class() { prop1 := 1; }");
//    ASSERT_TRUE(class1);
//    ASSERT_TRUE(class1->is_init());
//    ASSERT_EQ(ObjType::Type, class1->m_var_type_current) << toString(class1->m_var_type_current);
//    ASSERT_EQ(ObjType::Class, class1->m_var_type_fixed) << toString(class1->m_var_type_fixed);
//    ASSERT_STREQ(":class1", class1->getName().c_str());
//    ASSERT_EQ(1, class1->size());
//    ASSERT_TRUE(class1->at("prop1").second);
//    ASSERT_STREQ("1", class1->at("prop1").second->toString().c_str());
//
//    ObjPtr class2 = ctx.ExecStr(":class2 := :class1 { prop2 := 2; }");
//    ASSERT_TRUE(class2);
//    ASSERT_EQ(ObjType::Type, class2->m_var_type_current) << toString(class2->m_var_type_current);
//    ASSERT_EQ(ObjType::Class, class2->m_var_type_fixed) << toString(class2->m_var_type_fixed);
//    ASSERT_STREQ(":class2", class2->getName().c_str());
//    ASSERT_EQ(2, class2->size());
//    ASSERT_STREQ("1", class2->at("prop1").second->toString().c_str());
//    ASSERT_STREQ("2", class2->at("prop2").second->toString().c_str());
//
//
//    ObjPtr class3 = ctx.ExecStr(":class3 := :class2 { prop1 = 10; }");
//    ASSERT_TRUE(class3);
//    ASSERT_EQ(ObjType::Type, class3->m_var_type_current) << toString(class2->m_var_type_current);
//    ASSERT_EQ(ObjType::Class, class3->m_var_type_fixed) << toString(class2->m_var_type_fixed);
//    ASSERT_STREQ(":class3", class3->getName().c_str());
//    ASSERT_EQ(2, class3->size());
//    ASSERT_STREQ("10", class3->at("prop1").second->toString().c_str());
//    ASSERT_STREQ("2", class3->at("prop2").second->toString().c_str());
//
//
//    EXPECT_ANY_THROW(ctx.ExecStr(":class_dup_prop2 := :class2 { prop2 ::= 1; }"));
//    EXPECT_ANY_THROW(ctx.ExecStr(":class_prop_not_found := :class2 { prop_not_found = 1; }"));
//
//
//    ObjPtr var1 = ctx.ExecStr("var1 := :class1()");
//    ASSERT_TRUE(var1);
//    ObjPtr var1_1;
//
//    ASSERT_NO_THROW(var1_1 = ctx.ExecStr("var1.prop1")) << ctx.Dump() << "  (" << var1->toString() << ")";
//
//    ASSERT_TRUE(var1_1);
//    ASSERT_EQ(1, var1_1->GetValueAsInteger());
//
//
//    ObjPtr var2 = ctx.ExecStr("var2 := :class2()");
//    ASSERT_TRUE(var2);
//    ObjPtr var2_1 = ctx.ExecStr("var2.prop1");
//    ASSERT_TRUE(var2_1);
//    ASSERT_EQ(1, var2_1->GetValueAsInteger());
//    ObjPtr var2_2 = ctx.ExecStr("var2.prop2");
//    ASSERT_TRUE(var2_2);
//    ASSERT_EQ(2, var2_2->GetValueAsInteger());
//
//    ObjPtr var3 = ctx.ExecStr("var3 := :class3()");
//    ASSERT_TRUE(var3);
//    ObjPtr var3_1 = ctx.ExecStr("var3.prop1");
//    ASSERT_TRUE(var3_1);
//    ASSERT_EQ(10, var3_1->GetValueAsInteger());
//    ObjPtr var3_2 = ctx.ExecStr("var3.prop2");
//    ASSERT_TRUE(var3_2);
//    ASSERT_EQ(2, var3_2->GetValueAsInteger());
//
//
//    ObjPtr var4 = ctx.ExecStr("var4 := :class3(prop2=100, prop1=99)");
//    ASSERT_TRUE(var4);
//    ObjPtr var4_1 = ctx.ExecStr("var4.prop1");
//    ASSERT_TRUE(var4_1);
//    ASSERT_EQ(99, var4_1->GetValueAsInteger());
//    ObjPtr var4_2 = ctx.ExecStr("var4.prop2");
//    ASSERT_TRUE(var4_2);
//    ASSERT_EQ(100, var4_2->GetValueAsInteger());
//
//
//    EXPECT_ANY_THROW(ctx.ExecStr("var_not_found := :class3(prop_not_found=100)"));
//    EXPECT_ANY_THROW(ctx.ExecStr("var_bad_type := :class3(prop1='BAD TYPE')"));
//}




//TEST(Ast, ClassConstruct) {
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

TEST(Ast, CheckStrPrintf) {

    TermPtr args = Term::CreateDict();

    ASSERT_TRUE(AstAnalysis::CheckStrPrintf("", args, 0));
    args->push_back(Term::CreateName(""));
    ASSERT_FALSE(AstAnalysis::CheckStrPrintf("", args, 0));
    ASSERT_TRUE(AstAnalysis::CheckStrPrintf("", args, 1));

    ASSERT_TRUE(AstAnalysis::CheckStrPrintf("%s", args, 0));
    ASSERT_FALSE(AstAnalysis::CheckStrPrintf("%s", args, 1));
    args->push_back(Term::CreateName(""));
    ASSERT_TRUE(AstAnalysis::CheckStrPrintf("%s", args, 1));
    ASSERT_FALSE(AstAnalysis::CheckStrPrintf("%s", args, 2));

    ASSERT_TRUE(AstAnalysis::CheckStrPrintf("%s%d", args, 0));
    ASSERT_FALSE(AstAnalysis::CheckStrPrintf("%s%d", args, 1));
    args->push_back(Term::CreateName("1"));
    args->back().second->m_type = getDefaultType(ObjType::Int8);
    ASSERT_TRUE(AstAnalysis::CheckStrPrintf("%s%d", args, 1));
    ASSERT_FALSE(AstAnalysis::CheckStrPrintf("%s%d", args, 2));

    ASSERT_TRUE(AstAnalysis::CheckStrPrintf("%s%d%d", args, 0));
    ASSERT_FALSE(AstAnalysis::CheckStrPrintf("%s%d%d", args, 1));
    args->push_back(Term::CreateName("1"));
    args->back().second->m_type = getDefaultType(ObjType::Int64);
    ASSERT_FALSE(AstAnalysis::CheckStrPrintf("%s%d%d", args, 1));
    ASSERT_TRUE(AstAnalysis::CheckStrPrintf("%s%d%ld", args, 1));
    args->back().second->m_type = getDefaultType(ObjType::Int32);
    ASSERT_TRUE(AstAnalysis::CheckStrPrintf("%s%d%ld", args, 1));
    ASSERT_FALSE(AstAnalysis::CheckStrPrintf("%s%d%ld", args, 2));

    ASSERT_TRUE(AstAnalysis::CheckStrPrintf("%s%d%ld%f", args, 0));
    ASSERT_FALSE(AstAnalysis::CheckStrPrintf("%s%d%ld%f", args, 1));
    args->push_back(Term::CreateName("0"));
    ASSERT_TRUE(AstAnalysis::CheckStrPrintf("%s%d%ld%f", args, 1));
    ASSERT_FALSE(AstAnalysis::CheckStrPrintf("%s%d%ld%f", args, 2));

    ASSERT_TRUE(AstAnalysis::CheckStrPrintf("%s%d%ld%f%s", args, 0));
    ASSERT_FALSE(AstAnalysis::CheckStrPrintf("%s%d%ld%f%s", args, 1));
    args->push_back(Term::CreateName(""));
    ASSERT_TRUE(AstAnalysis::CheckStrPrintf("%s%d%ld%f%s", args, 1));
    ASSERT_FALSE(AstAnalysis::CheckStrPrintf("%s%d%ld%f%s", args, 2));


    JIT * jit = JIT::ReCreate();
    ASSERT_TRUE(jit);

    ASSERT_ANY_THROW(jit->MakeAst("print()"));
    ASSERT_ANY_THROW(jit->MakeAst("print(123)"));
    ASSERT_ANY_THROW(jit->MakeAst("print('%s - %s', 1, 'test')"));
    ASSERT_ANY_THROW(jit->MakeAst("print('%d - %d', 1, 'test')"));

    ASSERT_NO_THROW(jit->MakeAst("print(format='')"));
    ASSERT_NO_THROW(jit->MakeAst("print(format='%d', 1)"));

    ASSERT_ANY_THROW(jit->MakeAst("print(format='', 1)"));
    ASSERT_ANY_THROW(jit->MakeAst("print(format='%d', named=1)"));
    ASSERT_ANY_THROW(jit->MakeAst("print(format='%d', __sys__=1)"));

    ObjPtr print;
    ASSERT_NO_THROW(print = jit->Run("print(format='\\n')"));
    ASSERT_NO_THROW(print = jit->Run("print('%d - %s\\n', 1, 'test')"));
}

TEST(Ast, CheckStrFormat) {

    TermPtr args = Term::CreateDict();

//    ASSERT_ANY_THROW(AstAnalysis::ConvertToVFormat_("", nullptr));
    ASSERT_ANY_THROW(AstAnalysis::ConvertToVFormat_("{", *args));

    ASSERT_STREQ("", AstAnalysis::ConvertToVFormat_("", *args).c_str());
    ASSERT_STREQ("{{", AstAnalysis::ConvertToVFormat_("{{", *args).c_str());
    ASSERT_STREQ("format", AstAnalysis::ConvertToVFormat_("format", *args).c_str());
    ASSERT_STREQ("{}", AstAnalysis::ConvertToVFormat_("{}", *args).c_str());
    ASSERT_STREQ("{1}", AstAnalysis::ConvertToVFormat_("{1}", *args).c_str());
    ASSERT_STREQ("{1:}", AstAnalysis::ConvertToVFormat_("{1:}", *args).c_str());
    ASSERT_STREQ("{:0}", AstAnalysis::ConvertToVFormat_("{:0}", *args).c_str());
    ASSERT_STREQ("{:0}{:0}", AstAnalysis::ConvertToVFormat_("{:0}{:0}", *args).c_str());
    ASSERT_STREQ("{1:1}{1:1}", AstAnalysis::ConvertToVFormat_("{1:1}{1:1}", *args).c_str());

    ASSERT_ANY_THROW(AstAnalysis::ConvertToVFormat_("{{{1:1}{1:1", *args));
    ASSERT_STREQ("{{{1:1}{1:1}", AstAnalysis::ConvertToVFormat_("{{{1:1}{1:1}", *args).c_str());
    ASSERT_STREQ("{{{1:1}{1:1}}}", AstAnalysis::ConvertToVFormat_("{{{1:1}{1:1}}}", *args).c_str());

    args->push_back(Term::CreateNone(), "name");
    ASSERT_STREQ("{0}", AstAnalysis::ConvertToVFormat_("{name}", *args).c_str());
    ASSERT_STREQ("{0}{0}", AstAnalysis::ConvertToVFormat_("{name}{name}", *args).c_str());
    ASSERT_STREQ("{0:0}{0:123}{0:0.0}", AstAnalysis::ConvertToVFormat_("{name:0}{0:123}{name:0.0}", *args).c_str());
    ASSERT_STREQ("{0}{0}", AstAnalysis::ConvertToVFormat_("{0}{name}", *args).c_str());

    ASSERT_STREQ("{{{0}}}", AstAnalysis::ConvertToVFormat_("{{{0}}}", *args).c_str());
    ASSERT_STREQ("{{{0}}}", AstAnalysis::ConvertToVFormat_("{{{name}}}", *args).c_str());

    ASSERT_STREQ("{{0}{0}}", AstAnalysis::ConvertToVFormat_("{{0}{name}}", *args).c_str());
    ASSERT_STREQ("{{{0}{0}}", AstAnalysis::ConvertToVFormat_("{{{0}{name}}", *args).c_str());


    args->push_back(Term::CreateNone(), "name2");
    ASSERT_STREQ("{0}{1}", AstAnalysis::ConvertToVFormat_("{name}{name2}", *args).c_str());
    ASSERT_STREQ("{1}", AstAnalysis::ConvertToVFormat_("{name2}", *args).c_str());
    ASSERT_STREQ("{1:0}{1:.0}{0:0.0}", AstAnalysis::ConvertToVFormat_("{name2:0}{1:.0}{name:0.0}", *args).c_str());
    ASSERT_STREQ("{1}{0}", AstAnalysis::ConvertToVFormat_("{name2}{name}", *args).c_str());

    ASSERT_STREQ("{{name2}{0}}", AstAnalysis::ConvertToVFormat_("{{name2}{name}}", *args).c_str());
    ASSERT_STREQ("{{{1}{0}}", AstAnalysis::ConvertToVFormat_("{{{name2}{name}}", *args).c_str());
    ASSERT_STREQ("{{{1}}}", AstAnalysis::ConvertToVFormat_("{{{name2}}}", *args).c_str());

    ASSERT_STREQ("{{{0}}}", AstAnalysis::ConvertToVFormat_("{{{name}}}", *args).c_str());

    args->clear();
    ASSERT_TRUE(!args->size());

    ASSERT_STREQ("", AstAnalysis::MakeFormat("", args, nullptr). c_str());
    ASSERT_ANY_THROW(AstAnalysis::MakeFormat("{", args, nullptr));
    ASSERT_STREQ("{", AstAnalysis::MakeFormat("{{", args, nullptr). c_str());

    ASSERT_NO_THROW(ASSERT_STREQ("{}", AstAnalysis::MakeFormat("{{}}", args, nullptr). c_str()));
    ASSERT_ANY_THROW(ASSERT_STREQ("{}", AstAnalysis::MakeFormat("{{{}}}", args, nullptr). c_str()));

    args->push_back(Term::CreateNone(), "none");
    ASSERT_NO_THROW(ASSERT_STREQ("{}", AstAnalysis::MakeFormat("{{{}}}", args, nullptr). c_str()));
    ASSERT_NO_THROW(ASSERT_STREQ("{}", AstAnalysis::MakeFormat("{{{none}}}", args, nullptr). c_str()));


    ASSERT_NO_THROW(ASSERT_STREQ("", AstAnalysis::MakeFormat("{none}", args, nullptr). c_str()));

    args->push_back(Term::CreateName("string", TermID::STRCHAR), "str");
    ASSERT_NO_THROW(ASSERT_STREQ("'string'", AstAnalysis::MakeFormat("{str}", args, nullptr). c_str()));

    args->push_back(Term::CreateName("123", TermID::STRCHAR), "int");
    ASSERT_NO_THROW(ASSERT_STREQ("'123'", AstAnalysis::MakeFormat("{int}", args, nullptr). c_str()));

    ASSERT_NO_THROW(ASSERT_STREQ("", AstAnalysis::MakeFormat("{}", args, nullptr). c_str()));
    ASSERT_NO_THROW(ASSERT_STREQ("'string'", AstAnalysis::MakeFormat("{}{}", args, nullptr). c_str()));
    ASSERT_NO_THROW(ASSERT_STREQ("'string''123'", AstAnalysis::MakeFormat("{}{}{}", args, nullptr). c_str()));

    ASSERT_NO_THROW(ASSERT_STREQ("'string'", AstAnalysis::MakeFormat("{1}", args, nullptr). c_str()));
    ASSERT_NO_THROW(ASSERT_STREQ("'string'", AstAnalysis::MakeFormat("{0}{1}{0}", args, nullptr). c_str()));
    ASSERT_NO_THROW(ASSERT_STREQ("'string''123'", AstAnalysis::MakeFormat("{1}{2}{none}", args, nullptr). c_str()));

    ASSERT_NO_THROW(ASSERT_STREQ("'string''123'", AstAnalysis::MakeFormat("{none}{str}{int}{none}", args, nullptr). c_str()));
}

TEST(Ast, MakeInclude) {

    std::string str;
    str = AstAnalysis::MakeInclude(Term::CreateName("name"));
    ASSERT_STREQ("", str.c_str());

    str = AstAnalysis::MakeInclude(Parser::ParseString("::val := 1;"));
    ASSERT_STREQ("::val := ...;\n", str.c_str());

    str = AstAnalysis::MakeInclude(Parser::ParseString("::val := 1; val2 := 1; @::val3 := 1"));
    ASSERT_STREQ("::val := ...;\n@::val3 := ...;\n", str.c_str());

    str = AstAnalysis::MakeInclude(Parser::ParseString("::val, val2, @::val3 := 1"));
    ASSERT_STREQ("::val := ...;\n@::val3 := ...;\n", str.c_str());

    str = AstAnalysis::MakeInclude(Parser::ParseString("@@ macro @@ := 1; @@ macro2 @@ := @@ 2 @@; @@@@ macro @@@@;"));
    ASSERT_STREQ("@@ macro @@ := 1;\n@@ macro2 @@ := @@ 2 @@;\n@@@@ macro @@@@;\n", str.c_str());

}
#endif // UNITTEST