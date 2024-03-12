#ifndef NEWLANG_SYSTEM_H_
#define NEWLANG_SYSTEM_H_

#include "pch.h"

#include <module.h>
#include <builtin.h>

namespace newlang {

    namespace runtime {

        //    os.name - имя операционной системы. Доступные варианты: ‘posix’, ’nt’, ‘mac’, ‘os2’, ‘ce’, ‘java’.
        //    os.environ - словарь переменных окружения. Изменяемый (можно добавлять и удалять переменные окружения).
        //    os.getlogin()
        //    os.getpid() - текущий id процесса.
        //    os.uname() - информация об ОС. возвращает объект с атрибутами: 
        //        sysname - имя операционной системы, 
        //            nodename - имя машины в сети (определяется реализацией), 
        //            release - релиз, 
        //            version - версия, 
        //            machine - идентификатор машины.
        //    os.system(command) - исполняет системную команду, возвращает код её завершения (в случае успеха 0).
        //
        //    
        //    os.chdir(path) - смена текущей директории.
        //    os.chmod(path, mode, *, dir_fd=None, follow_symlinks=True) - смена прав доступа к объекту (mode - восьмеричное число).
        //    os.chown(path, uid, gid, *, dir_fd=None, follow_symlinks=True) - меняет id владельца и группы (Unix).
        //    os.getcwd() - текущая рабочая директория.
        //    os.link(src, dst, *, src_dir_fd=None, dst_dir_fd=None, follow_symlinks=True) - создаёт жёсткую ссылку.
        //    os.listdir(path=".") - список файлов и директорий в папке.
        //    os.mkdir(path, mode=0o777, *, dir_fd=None) - создаёт директорию. OSError, если директория существует.
        //    os.makedirs(path, mode=0o777, exist_ok=False) - создаёт директорию, создавая при этом промежуточные директории.
        //    os.remove(path, *, dir_fd=None) - удаляет путь к файлу.
        //    os.rename(src, dst, *, src_dir_fd=None, dst_dir_fd=None) - переименовывает файл или директорию из src в dst.
        //    os.renames(old, new) - переименовывает old в new, создавая промежуточные директории.
        //    os.replace(src, dst, *, src_dir_fd=None, dst_dir_fd=None) - переименовывает из src в dst с принудительной заменой.
        //    os.rmdir(path, *, dir_fd=None) - удаляет пустую директорию.
        //    os.removedirs(path) - удаляет директорию, затем пытается удалить родительские директории, и удаляет их рекурсивно, пока они пусты.
        //    os.symlink(source, link_name, target_is_directory=False, *, dir_fd=None) - создаёт символическую ссылку на объект.
        //    os.sync() - записывает все данные на диск (Unix).
        //    os.truncate(path, length) - обрезает файл до длины length.
        //    os.utime(path, times=None, *, ns=None, dir_fd=None, follow_symlinks=True) - модификация времени последнего доступа и изменения файла. Либо times - кортеж (время доступа в секундах, время изменения в секундах), либо ns - кортеж (время доступа в наносекундах, время изменения в наносекундах).
        //    os.walk(top, topdown=True, onerror=None, followlinks=False) - генерация имён файлов в дереве каталогов, сверху вниз (если topdown равен True), либо снизу вверх (если False). Для каждого каталога функция walk возвращает кортеж (путь к каталогу, список каталогов, список файлов).

        class Buildin : public Module {
        public:

            bool RegisterMacros(MacroPtr macro) {
                return true;
            }

        protected:

            Buildin(RuntimePtr rt, ObjType type, const char * name = nullptr) : Module(rt) {
                if (!name) {
                    name = newlang::toString(type);
                    ASSERT(name);
                }

                TermPtr term = Parser::ParseTerm(name, nullptr, false);

                if (!term) {
                    LOG_RUNTIME("Fail name '%s'!", name);
                }

                m_file = name;
//                m_source = "__buildin__";
                m_md5 = "";
                m_timestamp = __TIMESTAMP__;
                m_version = VERSION;
            }


            bool CreateMethod(const char * proto, FunctionType & func, ObjType type = ObjType::Function);
            bool CreateMethodNative(const char * proto, void * addr);
            bool AddMethod(const char * name, ObjPtr obj);


            bool CreateProperty(const char * proto, ObjPtr obj);

            bool CreateMacro(const char * text) {

                //                try {
                //                    MacroPtr macro;
                //                                    TermPtr m = Parser::ParseTerm(text, nulltr, false);
                //                } catch (...) {
                //                    return false;
                //                }
                //                m_macro_list.push_back(text);
                if (m_runtime) {
                    TermPtr m = Parser::ParseTerm(text, m_runtime->m_macro, false);
                }
                return true;
            }

            //            std::vector<std::string> m_macro_list;


        };

        class Base : public Buildin {
        public:

            Base(RuntimePtr rt) : Buildin(rt, ObjType::Base, "::Base") {

                bool dsl_enable = true;
                if (rt) {
                    dsl_enable = rt->m_load_dsl;
                }

                if (dsl_enable) {

                    VERIFY(CreateMacro("@@ true @@ ::= 1"));
                    VERIFY(CreateMacro("@@ yes @@ ::= 1"));
                    VERIFY(CreateMacro("@@ false @@ ::= 0"));
                    VERIFY(CreateMacro("@@ no @@ ::= 0"));

                    VERIFY(CreateMacro("@@ if( ... ) @@ ::= @@ [ @$... ] --> @@"));
                    VERIFY(CreateMacro("@@ elif( ... ) @@ ::= @@ ,[ @$... ] --> @@"));
                    VERIFY(CreateMacro("@@ else @@ ::= @@ ,[...] --> @@"));

                    VERIFY(CreateMacro("@@ while( ... ) @@ ::= @@ [ @$... ] <-> @@"));
                    VERIFY(CreateMacro("@@ dowhile( ... ) @@ ::= @@ <-> [ @$... ] @@"));
                    VERIFY(CreateMacro("@@ loop @@ ::= @@ [ 1 ] <-> @@"));


                    VERIFY(CreateMacro("@@ break $label @@ ::= @@ @$label :: ++ @@"));
                    VERIFY(CreateMacro("@@ continue $label @@ ::= @@ @$label :: -- @@"));
                    VERIFY(CreateMacro("@@ return( result ) @@ ::= @@ @__FUNC_BLOCK__ ++ @$result ++ @@"));
                    VERIFY(CreateMacro("@@ throw( result ) @@ ::= @@ -- @$result -- @@"));

                    VERIFY(CreateMacro("@@ match( ... ) @@ ::= @@ [ @$... ] @__PRAGMA_EXPECTED__( @\\ =>, @\\ ==>, @\\ ===>, @\\ ~>, @\\ ~~>, @\\ ~~~> ) @@"));
                    VERIFY(CreateMacro("@@ case( ... ) @@ ::= @@ [ @$... ] --> @@"));
                    VERIFY(CreateMacro("@@ default @@ ::= @@ [...] --> @@"));


                    VERIFY(CreateMacro("@@ this @@ ::= @@ $0 @@ ##< This object (self)"));
                    VERIFY(CreateMacro("@@ self @@ ::= @@ $0 @@ ##< This object (self)"));
                    VERIFY(CreateMacro("@@ super @@ ::= @@ $$ @@ ##< Super (parent) class or function"));
                    VERIFY(CreateMacro("@@ latter @@ ::= @@ $^ @@  ##< Result of the last operation"));


                    VERIFY(CreateMacro("@@ try @@ ::= @@ [ {*  @__PRAGMA_EXPECTED__( @\\ { ) @@"));
                    VERIFY(CreateMacro("@@ catch(...) @@ ::= @@ *} ] : < @$... > ~> @@"));
                    VERIFY(CreateMacro("@@ forward @@ ::= @@ +- $^ -+ @@  ##< Forward latter result or exception"));


                    VERIFY(CreateMacro("@@ iter( obj, ... ) @@ ::= @@ @$obj ? (@$...) @@"));
                    VERIFY(CreateMacro("@@ next( obj, ... ) @@ ::= @@ @$obj ! (@$...) @@"));
                    VERIFY(CreateMacro("@@ curr( obj ) @@ ::= @@ @$obj !? @@"));
                    VERIFY(CreateMacro("@@ first( obj ) @@ ::= @@ @$obj !! @@"));
                    VERIFY(CreateMacro("@@ all( obj ) @@ ::= @@ @$obj ?? @@"));

                    VERIFY(CreateMacro("@@ and @@ ::= @@ && @@"));
                    VERIFY(CreateMacro("@@ or @@ ::= @@ || @@"));
                    VERIFY(CreateMacro("@@ xor @@ ::= @@ ^^ @@"));
                    VERIFY(CreateMacro("@@ not(value) @@ ::= @@ (:Bool($value)==0) @@"));

//                    VERIFY(CreateMacro("@@ root() @@ ::= @@ @# @\\\\ @@"));
//                    VERIFY(CreateMacro("@@ module() @@ ::= @@ @# $\\\\ @@"));
                    
//                    VERIFY(CreateMacro("@@ namespace() @@ ::= @@ @# @:: @@"));
                    VERIFY(CreateMacro("@@ module() @@ ::= @@ @$$ @@"));
                    VERIFY(CreateMacro("@@ static @@ ::= @@ @:: @@"));
                    VERIFY(CreateMacro("@@ package @@ ::= @@  @$$ = @# @$name @@"));
                    VERIFY(CreateMacro("@@ declare( obj ) @@ ::= @@ @$obj ::= ... @@  ##< Forward declaration of the object"));

                    VERIFY(CreateMacro("@@ typedef($cnt) @@ ::= @@ @__PRAGMA_TYPE_DEFINE__(@$cnt) @@ ##< Disable warning when defining a type inside a namespace"));

                    VERIFY(CreateMacro("@@ coroutine @@ ::= @@ __ANNOTATION_SET__(coroutine) @@"));
                    VERIFY(CreateMacro("@@ co_yield  $val  @@ ::= @@ __ANNOTATION_CHECK__(coroutine) @__FUNC_BLOCK__ :: -- @$val -- @@"));
                    VERIFY(CreateMacro("@@ co_await        @@ ::= @@ __ANNOTATION_CHECK__(coroutine) @__FUNC_BLOCK__ :: +- @@"));
                    VERIFY(CreateMacro("@@ co_return $val  @@ ::= @@ __ANNOTATION_CHECK__(coroutine) @__FUNC_BLOCK__ :: ++ @$val ++ @@"));

                    VERIFY(CreateMacro("@@ exit(code) @@ ::= @@ :: ++ @$code ++ @@"));
                    VERIFY(CreateMacro("@@ abort() @@ ::= @@ :: -- @@"));
                    

                    
                }


                bool assert_enable = true;
                if (rt) {
                    assert_enable = rt->m_assert_enable;
                }
                // @assert(value, ...)
                // @static_assert(value, ...)
                // @verify(value, ...)
                //
                // @__PRAGMA_ASSERT__( is_runtime, is_always, value, val_string, ... )
                //
                // @@ static_assert(value, ...) @@ ::= @@ @__PRAGMA_ASSERT__(0, 0, value, @# value, @$... ) @@;
                // @@ assert(value, ...) @@ ::= @@ @__PRAGMA_ASSERT__(1, 0, value, @# value, @$... ) @@;
                // @@ verify(value, ...) @@ ::= @@ @__PRAGMA_ASSERT__(1, 1, value, @# value, @$... ) @@;

                // @__PRAGMA_ASSERT__ replase to __pragma_assert__ in @ref Parser::PragmaEval
                // @__PRAGMA_STATIC_ASSERT__
                VERIFY(CreateMethod("__assert_abort__(...):None", __assert_abort__));

                VERIFY(CreateMacro("@@ static_assert(...) @@ ::= @@ @__PRAGMA_STATIC_ASSERT__(@$... ) @@"));

                if (assert_enable) {
                    VERIFY(CreateMacro("@@ assert(value, ...) @@ ::= @@ [:Bool(@$value)==0]-->{ ::Base::__assert_abort__(@# @$value, @$... ) } @@"));
                    VERIFY(CreateMacro("@@ verify(value, ...) @@ ::= @@ [:Bool(@$value)==0]-->{ ::Base::__assert_abort__(@# @$value, @$... ) } @@"));
                } else {
                    VERIFY(CreateMacro("@@ assert(value, ...) @@ ::= @@ (_) @@"));
                    VERIFY(CreateMacro("@@ verify(value, ...) @@ ::= @@ (@$value) @@"));
                }


            }

            virtual ~Base() {
            };

            CALSS_METHOD_DEF(__assert_abort__);


        };

        class System : public Buildin {
        public:

            System(RuntimePtr rt) : Buildin(rt, ObjType::Sys, "::Sys") {
                //                m_var_type_current = ObjType::Module;
                //                m_var_type_fixed = ObjType::Module;
                //                m_class_name = "System";


                Obj arg;
                VERIFY(CreateProperty("name^:StrChar", getname(nullptr, arg)));
                VERIFY(CreateMethod("getname():StrChar", getname));

                VERIFY(CreateMethod("getlogin():StrChar", getlogin));
                VERIFY(CreateMethod("getenv(name:StrChar):String", getenv));
                VERIFY(CreateMethodNative("getpid():Int32", (void *) &::getpid));

                VERIFY(CreateMethod("getuname():Dictionary", getuname));
                VERIFY(CreateMethod("getsysinfo():Dictionary", getsysinfo));


                VERIFY(CreateMethod("getmillisec():Int64", getmillisec));
                VERIFY(CreateMethod("gettimeofday():Dictionary", gettimeofday));
                VERIFY(CreateMethod("getenviron():Dictionary", getenviron));
                VERIFY(CreateMethod("system(cmd:StrChar):StrChar", system));


                VERIFY(CreateMethodNative("chdir(dir:StrChar):Int32", (void *) &::chdir));
                VERIFY(CreateMethodNative("getcwd():StrChar", (void *) &::getcwd));

                //                m_var_is_init = true;
            }

            virtual ~System() {
            };

            CALSS_METHOD_DEF(getname);
            CALSS_METHOD_DEF(getlogin);
            CALSS_METHOD_DEF(getenv);

            CALSS_METHOD_DEF(getmillisec);
            CALSS_METHOD_DEF(gettimeofday);

            CALSS_METHOD_DEF(getuname);
            CALSS_METHOD_DEF(getsysinfo);
            CALSS_METHOD_DEF(getenviron);


            CALSS_METHOD_DEF(getcwd);
            CALSS_METHOD_DEF(system);


        };


        /*
         * 
         * 
         * rand():Int32 := %rand;
         * srand(seed:Int32):None;
         * sleep(usec:DWord64):None := %usleep;
         * usleep(usec:DWord64):None := %usleep;
         * 

         * 
         * abs(x) - Возвращает абсолютную величину (модуль числа).
         * max(iter, [args ...] * [, key]) - Максимальный элемент последовательности.
         * min(iter, [args ...] * [, key]) - Минимальный элемент последовательности.
         * pow(x, y[, r]) - ( x ** y ) % r.
         * round(X [, N]) - Округление до N знаков после запятой.
         * sorted(iterable[, key][, reverse]) - Отсортированный список.
         * sum(iter, start=0) - Сумма членов последовательности.
         * divmod(a, b) - Возвращает частное и остаток от деления a на b.
         * 
         * all(последовательность) - Возвращает True, если все элементы истинные (или, если последовательность пуста).
         * any(последовательность) - Возвращает True, если хотя бы один элемент - истина. Для пустой последовательности возвращает False.
         * 
         * ascii(object) - Как repr(), возвращает строку, содержащую представление объекта, но заменяет не-ASCII символы на экранированные последовательности.
         * oct(х) - Преобразование целого числа в восьмеричную строку.
         * ord(с) - Код символа.
         * 
         * callable(x) - Возвращает True для объекта, поддерживающего вызов (как функции).
         * classmethod(x) - Представляет указанную функцию методом класса.
         * compile(source, filename, mode, flags=0, dont_inherit=False) - Компиляция в программный код, который впоследствии может выполниться функцией eval или exec. Строка не должна содержать символов возврата каретки или нулевые байты.
         * 
         * enumerate(iterable, start=0) - Возвращает итератор, при каждом проходе предоставляющем кортеж из номера и соответствующего члена последовательности.
         * 
         * eval(expression, globals=None, locals=None) - Выполняет строку программного кода.
         * exec(object[, globals[, locals]]) - Выполняет программный код на Python.
         * 
         * filter(function, iterable) - Возвращает итератор из тех элементов, для которых function возвращает истину.
         * 
         * globals() - Словарь глобальных имен.
         * locals() - Словарь локальных имен.
         * 
         * dir([object]) - Список имен объекта, а если объект не указан, список имен в текущей локальной области видимости.
         * vars([object]) - Словарь из атрибутов объекта. По умолчанию - словарь локальных имен.
         * 
         * iter(x) - Возвращает объект итератора.
         * next(x) - Возвращает следующий элемент итератора.
         * reversed(object) - Итератор из развернутого объекта.
         * repr(obj) - Представление объекта.
         * 
         * len(x) - Возвращает число элементов в указанном объекте.
         * map(function, iterator) - Итератор, получившийся после применения к каждому элементу последовательности функции function.
         * 
         * property(fget=None, fset=None, fdel=None, doc=None)
         * getattr(object, name ,[default]) - извлекает атрибут объекта или default.
         * hasattr(object, name) - Имеет ли объект атрибут с именем 'name'.
         * setattr(объект, имя, значение) - Устанавливает атрибут объекта.
         * delattr(object, name) - Удаляет атрибут с именем 'name'.
         * isinstance(object, ClassInfo) - Истина, если объект является экземпляром ClassInfo или его подклассом. Если объект не является объектом данного типа, функция всегда возвращает ложь.
         * issubclass(класс, ClassInfo) - Истина, если класс является подклассом ClassInfo. Класс считается подклассом себя.
         * staticmethod(function) - Статический метод для функции.
         * super([тип [, объект или тип]]) - Доступ к родительскому классу.
         * 
         * 
         * help([object]) - Вызов встроенной справочной системы.
         * 
         * hash(x) - Возвращает хеш указанного объекта.
         * id(object) - Возвращает "адрес" объекта. Это целое число, которое гарантированно будет уникальным и постоянным для данного объекта в течение срока его существования.
         * 
         * open(file, mode='r', buffering=None, encoding=None, errors=None, newline=None, closefd=True) - Открывает файл и возвращает соответствующий поток.
         * 
         * 
         * hex(х) - Преобразование целого числа в шестнадцатеричную строку.
         * format(value[,format_spec]) - Форматирование (обычно форматирование строки).
         * print([object, ...], *, sep=" ", end='\n', file=sys.stdout) - Печать.
         * input([prompt]) - Возвращает введенную пользователем строку. Prompt - подсказка пользователю.
         * 
         * type(object) - Возвращает тип объекта.
         * type(name, bases, dict) - Возвращает новый экземпляр класса name.
         * zip(*iters) - Итератор, возвращающий кортежи, состоящие из соответствующих элементов аргументов-последовательностей.
         * 
         * Функция __import__() в Python, находит и импортирует модуль
         * Функция __import__() импортирует имя модуля name, потенциально используя данные значений переменных глобальной globals и локальной locals областей видимости, чтобы определить, как интерпретировать имя name в контексте пакета.
         * 
         * 
         * len()!!!
         * size()
         */

    } // namespace runtime

} // namespace newlang

#endif // NEWLANG_SYSTEM_H_
