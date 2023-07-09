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
 * Предположим, что:
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
 *  
 */


TEST(Module, Env) {

    const char *args[1] = {"-nlc-search=../example;../src"};

    RuntimePtr env = RunTime::Init(1, args);
    Context ctx(env);

    ctx.CheckOrLoadModule("\\file");
    ctx.CheckOrLoadModule("\\dir\\file");
    ctx.CheckOrLoadModule("\\dir\\file::var");
    ctx.CheckOrLoadModule("\\dir\\file::var.ddd");
}

TEST(Module, Load) {

    const char *args[1] = {"-nlc-search=../example;../src"};

    Context::Reset();

    RuntimePtr env = RunTime::Init(1, args);
    Context ctx(env);


    std::filesystem::create_directories("temp");
    ASSERT_TRUE(std::filesystem::is_directory("temp"));

    std::ofstream file("temp/module_test.nlp");
    file << "module_var1 := 1;\n";
    file << "ns { module_var2 := 2;\n";
    file << "   ns2 { module_var3 := 3;};\n";
    file << "};\n";
    file << "ns3::ns4::module_var4 := 4;\n";
    file.close();

    EXPECT_FALSE(ctx.FindTerm("module_var1"));
    EXPECT_FALSE(ctx.FindTerm("ns::module_var2"));
    EXPECT_FALSE(ctx.FindTerm("ns::ns2::module_var3"));
    EXPECT_FALSE(ctx.FindTerm("ns3::ns4::module_var4"));

    EXPECT_ANY_THROW(
            ctx.ExecStr("\\temp\\module_test::module_var1");
            ) << ctx.Dump("; ");

    // Загрузить модуль и импортировать все объекты
    ASSERT_STREQ("\\temp\\module_test", ExtractModuleName("\\temp\\module_test").c_str());

    ObjPtr result;
    result = ctx.ExecStr("\\temp\\module_test()");
    ASSERT_TRUE(result);
    ASSERT_EQ(4, result->GetValueAsInteger()) << ctx.Dump("; ");


    result = ctx.FindTerm("module_var1");
    ASSERT_TRUE(result) << ctx.Dump("; ");
    ASSERT_EQ(1, result->GetValueAsInteger());

    result = ctx.FindTerm("ns::module_var2");
    ASSERT_TRUE(result) << ctx.Dump("; ");
    ASSERT_EQ(2, result->GetValueAsInteger());

    result = ctx.FindTerm("\\temp\\module_test::ns::ns2::module_var3");
    ASSERT_TRUE(result) << ctx.Dump("; ");
    ASSERT_EQ(3, result->GetValueAsInteger());

    result = ctx.FindTerm("\\temp\\module_test::ns3::ns4::module_var4");
    ASSERT_TRUE(result) << ctx.Dump("; ");
    ASSERT_EQ(4, result->GetValueAsInteger());


    ASSERT_TRUE(ctx.m_modules.find("\\temp\\module_test") != ctx.m_modules.end());

    // Выгрузить модуль
    result = ctx.ExecStr("\\temp\\module_test(_)");
    ASSERT_TRUE(result);

    ASSERT_TRUE(ctx.m_modules.find("\\temp\\module_test") == ctx.m_modules.end());


    result = ctx.FindTerm("module_var1");
    ASSERT_FALSE(result) << ctx.Dump("; ");
    result = ctx.FindTerm("ns::module_var2");
    ASSERT_FALSE(result) << ctx.Dump("; ");
    result = ctx.FindTerm("ns::ns2::module_var3");
    ASSERT_FALSE(result) << ctx.Dump("; ");
    result = ctx.FindTerm("ns3::ns4::module_var4");
    ASSERT_FALSE(result) << ctx.Dump("; ");

    EXPECT_ANY_THROW(
            ctx.ExecStr("\\temp\\module_test::module_var1");
            ) << ctx.Dump("; ");

    // Импортировать по маске
    result = ctx.ExecStr("\\temp\\module_test('*')");
    ASSERT_TRUE(result);
    ASSERT_EQ(4, result->GetValueAsInteger()) << ctx.Dump("; ");

    result = ctx.FindTerm("ns::module_var2");
    ASSERT_TRUE(result) << ctx.Dump("; ");
    ASSERT_EQ(2, result->GetValueAsInteger()) << ctx.Dump("; ");

    result = ctx.FindTerm("\\temp\\module_test::ns::ns2::module_var3");
    ASSERT_TRUE(result) << ctx.Dump("; ");
    ASSERT_EQ(3, result->GetValueAsInteger()) << ctx.Dump("; ");

    ASSERT_TRUE(ctx.FindTerm("module_var1")) << ctx.Dump("; ");
    ASSERT_TRUE(ctx.FindTerm("\\temp\\module_test::ns3::ns4::module_var4"));


    ASSERT_TRUE(ctx.m_modules.find("\\temp\\module_test") != ctx.m_modules.end());

    // Выгрузить модуль
    result = ctx.ExecStr("\\temp\\module_test(_)");
    ASSERT_TRUE(result);

    ASSERT_TRUE(ctx.m_modules.find("\\temp\\module_test") == ctx.m_modules.end());


    //    // Импортировать по маске
    //    result = ctx.ExecStr("\\temp\\module_test('ns::*')");
    //    ASSERT_TRUE(result);
    //    ASSERT_EQ(4, result->GetValueAsInteger()) << ctx.Dump("; ");
    //
    //    result = ctx.FindTerm("ns::module_var2");
    //    ASSERT_TRUE(result) << ctx.Dump("; ");
    //    ASSERT_EQ(2, result->GetValueAsInteger()) << ctx.Dump("; ");
    //
    //    result = ctx.FindTerm("\\temp\\module_test::ns::ns2::module_var3");
    //    ASSERT_TRUE(result) << ctx.Dump("; ");
    //    ASSERT_EQ(3, result->GetValueAsInteger()) << ctx.Dump("; ");
    //
    //    ASSERT_FALSE(ctx.FindTerm("module_var1")) << ctx.Dump("; ");
    //    ASSERT_FALSE(ctx.FindTerm("\\temp\\module_test::ns3::ns4::module_var4"));

}

TEST(Module, SysEnv) {

    const char *args[1] = {"-nlc-search=../example;../src"};

    Context::Reset();

    RuntimePtr env = RunTime::Init(1, args);
    Context ctx(env);


    std::filesystem::create_directories("temp");
    ASSERT_TRUE(std::filesystem::is_directory("temp"));

    std::ofstream file("temp/module_test.nlp");
    file << "module_var1 := 1;\n";
    file << "ns { module_var2 := 2;\n";
    file << "   ns2 { module_var3 := 3;};\n";
    file << "};\n";
    file << "ns3::ns4::module_var4 := 4;\n";
    file.close();

    EXPECT_FALSE(ctx.FindTerm("module_var1"));
    EXPECT_FALSE(ctx.FindTerm("ns::module_var2"));
    EXPECT_FALSE(ctx.FindTerm("ns::ns2::module_var3"));
    EXPECT_FALSE(ctx.FindTerm("ns3::ns4::module_var4"));

    EXPECT_ANY_THROW(
            ctx.ExecStr("\\temp\\module_test::module_var1");
            ) << ctx.Dump("; ");

    // Загрузить модуль и импортировать все объекты
    ASSERT_STREQ("\\temp\\module_test", ExtractModuleName("\\temp\\module_test").c_str());

    ObjPtr result;
    ASSERT_NO_THROW(
            result = ctx.ExecStr("\\temp\\module_test()");
            );
    ASSERT_TRUE(result);
    ASSERT_EQ(4, result->GetValueAsInteger()) << ctx.Dump("; ");

    ASSERT_NO_THROW(
            result = ctx.ExecStr("\\temp\\module_test::module_var1");
            );
    ASSERT_TRUE(result);
    ASSERT_STREQ("1", result->GetValueAsString().c_str());

    ASSERT_NO_THROW(
            result = ctx.ExecStr("\\temp\\module_test::__md5__");
            );
    ASSERT_TRUE(result);
    ASSERT_EQ(32, result->GetValueAsString().size()) << result->GetValueAsString();

    ASSERT_NO_THROW(
            result = ctx.ExecStr("\\temp\\module_test::__timestamp__");
            );
    ASSERT_TRUE(result);
    ASSERT_EQ(24, result->GetValueAsString().size()) << result->GetValueAsString();

    ASSERT_NO_THROW(
            result = ctx.ExecStr("\\temp\\module_test::__file__");
            );
    ASSERT_TRUE(result);
    ASSERT_TRUE(result->GetValueAsString().find("temp/module_test.nlp") != std::string::npos) << result->GetValueAsString();


    ASSERT_NO_THROW(
            result = ctx.ExecStr("\\temp\\module_test::__main__");
            );
    ASSERT_TRUE(result);
    ASSERT_STREQ("0", result->GetValueAsString().c_str()) << result->GetValueAsString();
}

#endif // UNITTEST