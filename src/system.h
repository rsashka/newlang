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

            Buildin(ObjType type, const char * name = nullptr) {
                if (!name) {
                    name = newlang::toString(type);
                    ASSERT(name);
                }

//                m_var_type_current = type;
//                m_var_type_fixed = type;
//                m_class_name = name;

                TermPtr term = Parser::ParseTerm(name, nullptr, false);

                if (!term) {
                    LOG_RUNTIME("Fail name '%s'!", name);
                }
//                * const_cast<TermPtr *> (&m_prototype) = term;

                m_file = "__buildin__";
                m_source = "__buildin__";
                m_md5 = "";
                m_timestamp = __TIMESTAMP__;
                m_version = VERSION;

            }

            bool CreateMethod(const char * proto, FunctionType & func, ObjType type = ObjType::Function);
            bool CreateMethodNative(const char * proto, void * addr);
            bool AddMethod(const char * name, ObjPtr obj);


            bool CreateProperty(const char * proto, ObjPtr obj);

        };

        class System : public Buildin {
        public:

            System() : Buildin(ObjType::System) {
//                m_var_type_current = ObjType::Module;
//                m_var_type_fixed = ObjType::Module;
//                m_class_name = "System";


                Obj arg;
                VERIFY(CreateProperty("name^:String", getname(nullptr, arg)));
                VERIFY(CreateMethod("getname():String", getname));

                VERIFY(CreateMethod("getlogin():String", getlogin));
                VERIFY(CreateMethod("getenv(name:String):String", getenv));
                VERIFY(CreateMethodNative("getpid():Int32", (void *) &::getpid));

                VERIFY(CreateMethod("getuname():Dictionary", getuname));
                VERIFY(CreateMethod("getsysinfo():Dictionary", getsysinfo));


                VERIFY(CreateMethod("getmillisec():Int64", getmillisec));
                VERIFY(CreateMethod("gettimeofday():Dictionary", gettimeofday));
                VERIFY(CreateMethod("getenviron():Dictionary", getenviron));
                VERIFY(CreateMethod("system(cmd:String):String", system));


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

    } // namespace runtime

} // namespace newlang

#endif // NEWLANG_SYSTEM_H_
