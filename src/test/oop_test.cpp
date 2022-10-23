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
 * $name::local; @name::global;
 * но ns {
 *   $name::local;  # Будет $ns::name::local
 *   @name::global; # Все равно @name::global или тоже @ns::name::global ????
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
 *      @name::func() := {1111}; # глобальная @name::func и тоже самое, что и локальная $func() !!!
 *      name {
 *          var := 2;  # ::name::name::var или $var или @name::name::var
 *          @var := 2;  # ::var
 *          @name::name::var := 2;  # ::name::name::var или @name::name::var
 *          func() := {2}; # ::name::name::func или @name::name::func - 2
 *          name3 {
 *              var := 3;           # ::name::name::name3::var или $name::name::name3::var - 3
 *              func() := {3};      # ::name::name::name3::func или @name::name::name3::func() - 3
 * 
 *              var??;              # name::name::name3::var или $name::name::name3::var - 3
 *              name::var??;        # name::name::var или @name::name::var - 2
 *              name::name::var??;  # name::var или @name::name::var - 2
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
 * @name::name::func - такая запись ненравится, но будет самой правильной и логичной, так как все остальные еще хуже для понимания и разбора
 * @::name::name::func
 * @::name::name@func
 * ::name::name::func@@
 * 
 * <<<<<<<<<<<<<<<<<< НОВАЯ КОНЦЕПЦИЯ >>>>>>>>>>>>>>>>>>
 * 
 * Все объекты физически размещаюится в куче, но время их жизни определяется областью видимости.
 * Объекты с декораторами $ обозначают локальные объекты (аналог "на стеке"), которые удаляются по мере выхода из текущей области видимости.
 * Объекты с декораторами @ являются модулями или его объектами, доступны из других частй программы и сохраняют состояние при выходе из текущей области видимости.
 * 
 * Модуль (имя файла) @file или @dir.file или @dir1.dir2.file
 * Объект в модуле @file::var или @dir.file::ns::func() или @dir.file::ns::obj.field; @file::ns:type() ???
 * Файл модуля загружается однократно при первом обращении и не выгружается до явной команды???
 * @module, @dir.module2 {};
 * \\import(...) \$* \\\
 * \import(@module, @dir.module2){};
 * @module = _; Выгрузить модуль?????
 * 
 * 
 * \\ns ::name::space::long\\\
 * \ns::name;
 * 
 * @dsl{}; # Загрузка модуля с определниями макросов для DSL
 * 
 * @@ - текущий модуль
 * [@@.__main__] --> { # 1 или 0
 * 
 * }
 * @@.__@@__ := {}; # Деструктор модуля
 * __@@__ := {}; # Деструктор модуля
 * 
 * 
 * @file::var
 * @file::name::var
 * @file::file2::file3::name::var
 * @file, @dir::file, @dir1::dir2::file { # include module ???
 * 
 * };
 * ::ns, @file::ns, @dir.file::ns, @dir1.dir2.file::ns1::ns2 { # using namespace + include module
 * 
 * };
 * 
 * 
 * [__name__ == '__main__'] --> {
 * 
 * }
 * \\main [__name__ == '__main__'] -->\\\
 * \main {
 * 
 * }
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
 */
TEST(Oop, Namespace) {

    Context::Reset();
    Context ctx(RunTime::Init());

    ASSERT_EQ(0, ctx.m_ns_stack.size());
    ASSERT_STREQ("name", ctx.NamespaseFull("name").c_str());

    ASSERT_TRUE(ctx.NamespasePush("name"));
    ASSERT_EQ(1, ctx.m_ns_stack.size());
    ASSERT_STREQ("name", ctx.NamespaseFull().c_str());

    ASSERT_FALSE(ctx.NamespasePush(""));
    ASSERT_EQ(1, ctx.m_ns_stack.size());

    ASSERT_TRUE(ctx.NamespasePush("name2"));
    ASSERT_EQ(2, ctx.m_ns_stack.size());
    ASSERT_STREQ("name::name2", ctx.NamespaseFull().c_str());

    ASSERT_TRUE(ctx.NamespasePush("name3::name4"));
    ASSERT_EQ(3, ctx.m_ns_stack.size());
    ASSERT_STREQ("name::name2::name3::name4", ctx.NamespaseFull().c_str());

    ASSERT_TRUE(ctx.NamespasePush("::"));
    ASSERT_EQ(4, ctx.m_ns_stack.size());
    ASSERT_STREQ("::", ctx.NamespaseFull().c_str());

    ASSERT_FALSE(ctx.NamespasePush(""));
    ASSERT_EQ(4, ctx.m_ns_stack.size());

    ASSERT_STREQ("::ns", ctx.NamespaseFull("::ns").c_str());

    ASSERT_TRUE(ctx.NamespasePush("::name5::name6"));
    ASSERT_EQ(5, ctx.m_ns_stack.size());
    ASSERT_STREQ("::name5::name6", ctx.NamespaseFull().c_str());

    ASSERT_STREQ("::ns", ctx.NamespaseFull("::ns").c_str());

    ASSERT_TRUE(ctx.NamespasePush("name7"));
    ASSERT_EQ(6, ctx.m_ns_stack.size());
    ASSERT_STREQ("::name5::name6::name7", ctx.NamespaseFull().c_str());

    ctx.NamespasePop();
    ASSERT_EQ(5, ctx.m_ns_stack.size());
    ASSERT_STREQ("::name5::name6", ctx.NamespaseFull().c_str());

    ctx.NamespasePop();
    ASSERT_EQ(4, ctx.m_ns_stack.size());
    ASSERT_STREQ("::", ctx.NamespaseFull().c_str());

    ctx.NamespasePop();
    ASSERT_EQ(3, ctx.m_ns_stack.size());
    ASSERT_STREQ("name::name2::name3::name4", ctx.NamespaseFull().c_str());
    ASSERT_STREQ("::ns", ctx.NamespaseFull("::ns").c_str());

    ctx.NamespasePop();
    ASSERT_EQ(2, ctx.m_ns_stack.size());
    ASSERT_STREQ("name::name2", ctx.NamespaseFull().c_str());

    ctx.NamespasePop();
    ASSERT_EQ(1, ctx.m_ns_stack.size());
    ASSERT_STREQ("name", ctx.NamespaseFull().c_str());

    ctx.NamespasePop();
    ASSERT_EQ(0, ctx.m_ns_stack.size());
    ASSERT_STREQ("", ctx.NamespaseFull().c_str());


    ASSERT_TRUE(ctx.NamespasePush("ns::name"));
    ASSERT_EQ(1, ctx.m_ns_stack.size());
    ASSERT_STREQ("ns::name::var6", ctx.NamespaseFull("var6").c_str());


    ASSERT_TRUE(ctx.NamespasePush("::"));
    ASSERT_EQ(2, ctx.m_ns_stack.size());
    ASSERT_STREQ("::var6", ctx.NamespaseFull("var6").c_str());

    ctx.NamespasePop();
    ctx.NamespasePop();
    ASSERT_EQ(0, ctx.m_ns_stack.size());



    ASSERT_EQ(0, ctx.size());

    ObjPtr var1 = ctx.ExecStr("var1 := 1;");
    ASSERT_TRUE(var1);
    ASSERT_EQ(1, var1->GetValueAsInteger());
    ASSERT_EQ(1, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("var1", ctx.at(0).first.c_str());

    ObjPtr var2 = ctx.ExecStr("name { var2 := 2; }");
    ASSERT_TRUE(var2);
    ASSERT_EQ(2, var2->GetValueAsInteger());
    ASSERT_EQ(2, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("name::var2", ctx.at(1).first.c_str());

    ObjPtr var3 = ctx.ExecStr("ns3::name { var3 := 3; }");
    ASSERT_TRUE(var3);
    ASSERT_EQ(3, var3->GetValueAsInteger());
    ASSERT_EQ(3, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("ns3::name::var3", ctx.at(2).first.c_str());

    ObjPtr var4 = ctx.ExecStr("ns4::name { ::var4 := 4; }");
    ASSERT_TRUE(var4);
    ASSERT_EQ(4, var4->GetValueAsInteger());
    ASSERT_EQ(4, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("::var4", ctx.at(3).first.c_str());

    ObjPtr var5 = ctx.ExecStr("ns5::name { ns::var5 := 5; }");
    ASSERT_TRUE(var5);
    ASSERT_EQ(5, var5->GetValueAsInteger());
    ASSERT_EQ(5, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("ns5::name::ns::var5", ctx.at(4).first.c_str());

    ObjPtr var6 = ctx.ExecStr("ns6::name { { ::var6 := 6;} }");
    ASSERT_EQ(0, ctx.m_ns_stack.size());

    ASSERT_TRUE(var6);
    ASSERT_EQ(6, var6->GetValueAsInteger());
    ASSERT_EQ(6, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("::var6", ctx.at(5).first.c_str());

    ObjPtr var7 = ctx.ExecStr("ns7::name { :: { var7 := 7;} }");
    ASSERT_EQ(0, ctx.m_ns_stack.size());

    ASSERT_TRUE(var7);
    ASSERT_EQ(7, var7->GetValueAsInteger());
    ASSERT_EQ(7, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("::var7", ctx.at(6).first.c_str());

    ObjPtr var8 = ctx.ExecStr("ns8::name { ::ext { var8 := 8;}}");
    ASSERT_TRUE(var8);
    ASSERT_EQ(8, var8->GetValueAsInteger());
    ASSERT_EQ(8, ctx.size()) << ctx.Dump("; ").c_str();
    ASSERT_STREQ("::ext::var8", ctx.at(7).first.c_str());
}


#endif // UNITTEST