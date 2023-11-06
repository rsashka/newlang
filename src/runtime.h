#pragma once
#ifndef INCLUDED_RUNTIME_CPP_
#define INCLUDED_RUNTIME_CPP_

#include "pch.h"

#include <compiler.h>

#include "term.h"
#include "builtin.h"


#include <types.h>
#include <object.h>
#include <context.h>
#include <parser.h>




namespace newlang {

    template <typename T>
    T repeat(T str, const std::size_t n) {
        if (n == 0) {
            str.clear();
            str.shrink_to_fit();
            return str;
        } else if (n == 1 || str.empty()) {
            return str;
        }
        const auto period = str.size();
        if (period == 1) {
            str.append(n - 1, str.front());
            return str;
        }
        str.reserve(period * n);
        std::size_t m{2};
        for (; m < n; m *= 2) str += str;
        str.append(str.c_str(), (n - (m / 2)) * period);
        return str;
    }

    std::string GetFileExt(const char * str);
    std::string AddDefaultFileExt(const char * str, const char *ext_default);
    std::string ReplaceFileExt(const char * str, const char *ext_old, const char *ext_new);
    std::string ReadFile(const char *fileName);

    inline std::string GetDoc(std::string name) {
        return "Help system not implemented!!!!!";
    }

    inline std::string MakeConstructorName(std::string name) {
        ASSERT(isTypeName(name));
        std::string result(name.substr(1));
        result += ":";
        result += name.substr(name.rfind(":"));
        return result;
    }

    inline std::string MakeLocalName(std::string name) {
        return MangleName(MakeName(name).c_str());
    }

    /*
     * 
     * 
     * 
     */

    class RunTime : public GlobNameList, public std::enable_shared_from_this<RunTime> {
    private:

        /**
         * Обновить текущий список глобальных имен новыми из распарсенного текста программы.
         * Выполнить проверку глобальных имен на повтор с разными аргументами или на определение в разных частях программы
         * Выполняется импорт для нативных переменных и функций
         * Проверить обращение к отсуствующим глобальным именам
         * 
         * @param list
         * @return 
         */
        static bool AppendAndCheckGlobalName(TermPtr &term, void * obj);
        static bool CheckNativeName(TermPtr &term, void * obj);

        void RunAstProcessor(TermPtr &ast) {
            Term::TraversingNodes(ast,{
                &CheckNativeName,
                &AppendAndCheckGlobalName
            }, this);
        }

    public:


        void GlobalNameRegisterTypes();
        bool RegisterModule(ModulePtr module);

        RunTime();


        //        GlobNamePtr FindObject(const char *name);
        //
        //        GlobNamePtr GetObject(const char *name) {
        //            GlobNamePtr ret = FindObject(name);
        //            if (!ret) {
        //                LOG_RUNTIME("Object '%s' not found!", name);
        //            }
        //            return ret;
        //        }

        bool RegisterNativeObj(TermPtr term) {
            ASSERT(term);
            ASSERT(term->getTermID() == TermID::NATIVE);
            return RegisterSystemObj(CreateNative(term));
        }

        bool RegisterSystemObj(ObjPtr obj);
        std::vector<ObjPtr> m_sys_obj;

        static ObjPtr CreateNative(const char *proto, const char *module = nullptr, bool lazzy = false, const char *mangle_name = nullptr);
        static ObjPtr CreateNative(TermPtr proto, const char *module = nullptr, bool lazzy = false, const char *mangle_name = nullptr);
        static ObjPtr CreateNative(TermPtr proto, void *addr);

        virtual ~RunTime() {
            LLVMDisposeBuilder(m_llvm_builder);
            m_llvm_builder = nullptr;
            //LLVMShutdown();
        }

        static RuntimePtr Init(int argc = 0, const char** argv = nullptr, bool ignore_error = true);

        inline static void * GetDirectAddressFromLibrary(void *handle, const char * name) {
            if (isNativeName(name)) {
                return LLVMSearchForAddressOfSymbol(&name[1]);
            }
            return LLVMSearchForAddressOfSymbol(name);
        }


        ModulePtr LoadModule(const char *name_str, bool init);
        bool UnLoadModule(const char *name_str, bool deinit);
        ObjPtr ExecModule(const char *module, const char *output, bool cached, Context * ctx);

        static void * GetNativeAddr(const char * name, const char *module = nullptr);

        static std::string GetLastErrorMessage();




        /**
         * Функция для организации встроенных типов в иерархию наследования.
         * Другие функции: @ref CreateBaseType - создает базовые типы данных (для расширения классов требуется контекст)
         * и @ref BaseTypeConstructor - функция обратного вызова при создании нового объекта базового типа данных
         * @param type - Базовый тип данных @ref ObjType
         * @param parents - Список сторок с именами родительских типов
         * @return - Успешность регистрации базовго типа в иерархии
         */
        bool RegisterTypeHierarchy(ObjType type, std::vector<std::string> parents);

        ObjType BaseTypeFromString(const std::string & type, bool *has_error = nullptr);

        ObjPtr GetTypeFromString(const std::string & type, bool *has_error = nullptr);

        static bool pred_compare(const std::string &find, const std::string &str) {
            size_t pos = 0;
            while (pos < find.size() && pos < str.size()) {
                if (find[pos] != str[pos]) {
                    return false;
                }
                pos++;
            }
            return find.empty() || (pos && find.size() == pos);
        }

        std::vector<std::wstring> SelectPredict(std::wstring wstart, size_t overage_count = 0) {
            return SelectPredict(utf8_encode(wstart), overage_count);
        }
        std::vector<std::wstring> SelectPredict(std::string start, size_t overage_count = 0);





        LLVMBuilderRef m_llvm_builder;

        ModulePtr m_buildin_obj;
        ModulePtr m_main_module;
        std::map<std::string, ModulePtr> m_modules;

        std::map<std::string, ObjPtr> m_types;

        bool CheckOrLoadModule(std::string name);


        std::string m_work_dir;
        std::string m_exec_dir;
        std::vector<std::string> m_search_dir;
        ObjPtr m_args;
        MacroPtr m_macro;
        DiagPtr m_diag;
        Parser m_parser;

    protected:

        static const int default_argc = 3;
        static const char * default_argv[default_argc];

        /*
         * -nlc-search=sss;sss;sss;sss (char [3][const char*]){"", "-nlc-no-runtime", "-nlc-no-dsl"}
         */
        bool ParseArgs(int argc = default_argc, const char** argv = default_argv) {

            bool load_dsl = true;
            bool load_runtime = true;
            std::vector<std::string> load;

            for (int i = 0; i < argc; i++) {

                m_args->push_back(Obj::CreateString(argv[i]));

                if (strstr(argv[i], "-nlc-") == argv[i]) {
                    if (strstr(argv[i], "-nlc-search=") == argv[i]) {
                        std::string list(argv[i]);
                        list = list.substr(strlen("-nlc-search="));
                        m_search_dir = Macro::SplitString(list.c_str(), ";");
                    } else if (strstr(argv[i], "-nlc-search=") == argv[i]) {
                    } else if (strstr(argv[i], "-nlc-no-runtime") == argv[i]) {
                        load_runtime = false;
                    } else if (strstr(argv[i], "-nlc-no-dsl") == argv[i]) {
                        load_dsl = false;
                    } else if (strstr(argv[i], "-nlc-load=") == argv[i]) {
                        load.push_back(std::string(argv[i]).substr(strlen("-nlc-load=")));
                    } else {
                        LOG_RUNTIME("System arg '%s' not found!", argv[i]);
                    }
                }
            }


            llvm::SmallString<1024> path;
            auto error = llvm::sys::fs::current_path(path);
            if (error) {
                LOG_RUNTIME("%s", error.message().c_str());
            }
            m_work_dir = path.c_str();
            // LOG_DEBUG("work_dir: %s", m_work_dir.c_str());

            path = llvm::sys::fs::getMainExecutable(nullptr, nullptr);
            // LOG_DEBUG("%s", path.c_str());

            llvm::sys::path::remove_filename(path);
            m_exec_dir = path.c_str();
            // LOG_DEBUG("exec_dir: %s", m_exec_dir.c_str());

            m_search_dir.push_back(m_work_dir);
            m_search_dir.push_back(m_exec_dir);

            if (load_dsl) {

            }
            if (load_runtime) {

            }
            for (auto &elem : load) {

            }
            return true;
        }

        //SCOPE(private) :
    public:
        RunTime(const RunTime&) = delete;
        const RunTime& operator=(const RunTime&) = delete;

    };

}


#endif //INCLUDED_RUNTIME_CPP_
