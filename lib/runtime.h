#pragma once
#ifndef INCLUDED_RUNTIME_CPP_
#define INCLUDED_RUNTIME_CPP_

//#include "pch.h"

////??????????????????????????

#include <sys/time.h>
#include <ffi.h>

#include "nlc-rt.h"

#include "term.h"
//#include "module.h"
//#include "system.h"


using namespace newlang;

ObjPtr CreateTensor(torch::Tensor tensor);
void ConvertStringToTensor(const std::string &from, torch::Tensor &to, ObjType type);

namespace newlang {

#ifdef __GNUC__
    extern "C" int nlc_prinft_sub_(char const *format, ...) __attribute__ ((format(printf, 1, 2)));
#else
    EXTERN_C int nlc_prinft_sub_(char const *format, ...);
#endif    

    std::string GetFileExt(const char * str);
    std::string AddDefaultFileExt(const char * str, const char *ext_default);
    std::string ReplaceFileExt(const char * str, const char *ext_old, const char *ext_new);
    std::string ReadFile(const char *fileName);

    std::string GetDoc(std::string name);

    std::string MakeConstructorName(std::string name);

    std::string MakeLocalName(std::string name);

    /*
     * Последовательность работы к кодом:
     * 1. Лексер (последовательность токенов) - использовать только для отладки макросов
     * 2. Парсер - AST раскрытие макросов и чтение файлов модулей (или их загрузка с созданим иерархии имен объектов). 
     * 
     * 3. Выполнение интерпретатора - пошаговое выполнение AST (реальное создание объектов и выполнение операторов)
     * 
     * 4. Генерация кода - обход AST для генерации промежуточного код на языке LLVM-IR
     * 5. Оптимиация кода для NewLang (согласно типам данных и взаимодействия в нативными функциями)
     * 6. Оптимиация кода для LLVM-IR
     * 7. Сохрнение промежуточного кода в файл модуля.
     * 8. Компиляция промежуточного кода в машинный код (или выполнение в режиме интерпретатора)
     * 
     * 
     * В Runtime хранятся глобальные объекты, память под которые выделяется во время компиляции программы
     * В Context хрантся локальны объекты, память под которые выделяется только во ремя выполнения
     * Объект Context создается для основонго потока, а так же по одному объекту для каждого потока программы.
     * 
     * 
     https://ps-group.github.io/compilers/llvm_ir_translator
     * В интерпретаторе был использован паттерн “Интерпретатор”, 
     * т.е. каждый узел AST имел метод, который принимал контекст выполнения и выполнял рекурсивное 
     * вычисление узла с учётом текущего контекста выполнения:
     * 
     * // Вычисление выражения возвращает результат в виде CValue
     * class IExpressionAst
     * {
     * public:
     *     virtual ~IExpressionAst() = default;
     *     virtual CValue Evaluate(CInterpreterContext& context) const = 0;
     * };
     * 
     * // Выполнение инструкции результата не возвращает
     * class IStatementAst
     * {
     * public:
     *     virtual ~IStatementAst() = default;
     *     virtual void Execute(CInterpreterContext& context) const = 0;
     * };
     * 
     * // Вычисление функции требует список аргументов и возвращает результат в виде CValue
     * class IFunctionAst
     * {
     * public:
     *     virtual ~IFunctionAst() = default;
     *     virtual CValue Call(CInterpreterContext& context, std::vector<CValue> const& arguments) const = 0;
     *     virtual unsigned GetNameId() const = 0;
     * };
     * 
     * Такой паттерн подходит для интерпретатора, где входные данные поступают вместе с исходным кодом программы 
     * и могут быть сразу использованы для вычисления. 
     * 
     * Процессы внутри компилятора выглядят сложнее:
     * -    сначала должна произойти проверка типов и других семантических правил путём обхода AST
     * -    затем путём ещё одного обхода AST нужно сгенерировать промежуточный код на языке LLVM-IR
     * -    уже после генерации промежуточного кода он может быть превращён в машинный 
     * (это тема следующих статей) и использован для обработки входных данных
     * 
     * Для множественной обработки AST лучше подходит паттерн “Посетитель”, который позволяет абстрагироваться от узлов AST. 
     * Для реализации паттерна достаточно создать у каждого узла дерева метод, например void Accept(IVisitor&), 
     * который будет вызывать правильную перегрузку метода IVisitor::Visit, соответствующую типу текущего узла, 
     * и рекурсивно вызывать Accept для дочерних узлов:
     * 
     * Генерация кода выражений
     * 
     * Для генерации кода выражений мы реализуем интерфейс IExpressionVisitor
     * class CExpressionCodeGenerator : protected IExpressionVisitor
     * {
     * public:
     *     CExpressionCodeGenerator(llvm::IRBuilder<>& builder, CFrontendContext& context);
     * 
     *     // Can throw std::exception.
     *     llvm::Value* Codegen(IExpressionAST& ast);
     * protected:
     *     void Visit(CBinaryExpressionAST& expr) override;
     *     void Visit(CUnaryExpressionAST& expr) override;
     *     void Visit(CLiteralAST& expr) override;
     *     void Visit(CCallAST& expr) override;
     *     void Visit(CVariableRefAST& expr) override;
     * private:
     *     // Стек используется для временного хранения
     *     // по мере рекурсивного обхода дерева выражения.
     *     std::vector<llvm::Value*> m_values;
     *     CFrontendContext& m_context;
     *     llvm::IRBuilder<>& m_builder;
     * };
     * 
     * 
     */

    struct FileModule {
        std::string name;
        std::string include;
        std::string source;
        std::string bytecode;
    };

    struct GlobItem {
        TermPtr proto;
        WeakItem obj;
    };

    class RunTime : public std::map<std::string, GlobItem>, public std::enable_shared_from_this<RunTime> {
    private:

        //        void RunAstProcessor(TermPtr &ast) {
        //            Term::TraversingNodes(ast,{
        //                &CheckNativeName,
        //                &AppendAndCheckGlobalName
        //            }, this);
        //        }

    public:

        TermPtr NameRegister(bool new_only, const char *name, TermPtr proto, WeakItem obj = std::monostate()); // = std::monostate);
        //        bool NameRegister(const char * glob_name, TermPtr proto, WeakItem obj); // = std::monostate);
        GlobItem * NameFind(const char *name);
        TermPtr GlobFindProto(const std::string_view name);

        ObjPtr NameGet(const char *name, bool is_raise = true);

        //        bool MakeFullNames(TermPtr ast);


        void GlobalNameBuildinRegister();
        bool RegisterSystemFunc(const char *source);
        bool RegisterBuildin(BuildinPtr module);
        bool RegisterModule(ModulePtr module);

        void Clear();

        bool RegisterNativeObj(TermPtr term) {
            ASSERT(term);
            ASSERT(term->getTermID() == TermID::NATIVE);

            return RegisterSystemObj(CreateNative(term));
        }

        bool RegisterSystemBuildin(const char *text);
        bool RegisterSystemObj(ObjPtr obj);
        std::vector<ObjPtr> m_sys_obj;

        ObjPtr CreateNative(const char *proto, const char *module = nullptr, bool lazzy = false, const char *mangle_name = nullptr);
        ObjPtr CreateNative(TermPtr proto, const char *module = nullptr, bool lazzy = false, const char *mangle_name = nullptr);
        ObjPtr CreateNative(TermPtr proto, void *addr);
        ObjPtr CreateFunction(TermPtr proto, TermPtr block);
        ObjPtr CreateFunction(TermPtr proto, void *addr);


        static std::string Escape(const std::string_view str);
        /*
         * class A {
        public:
          static std::shared_ptr<A> create();
        private:
          A() = default;
          friend struct make_shared_enabler;
        };

        struct make_shared_enabler : A {
          template<typename... Args>
          make_shared_enabler(Args&&... args)
            : A(std::forward<Args>(args)...) {
          }
        };

        static std::shared_ptr<A> A::create() {
          return std::make_shared<make_shared_enabler>();
        }

         */
        RunTime();

        virtual ~RunTime() {

            //            LLVMDisposeBuilder(m_llvm_builder);
            //            m_llvm_builder = nullptr;
            //LLVMShutdown();
        }

        static bool LLVMInitialize() {
            if (LLVMInitializeNativeTarget() || LLVMInitializeNativeAsmParser() || LLVMInitializeNativeAsmPrinter()) {
                LOG_RUNTIME("Fail LLVM initialize!");
            }
            return true;
        }





        std::string ffi_file;

        typedef ffi_status ffi_prep_cif_type(ffi_cif *cif, ffi_abi abi, unsigned int nargs, ffi_type *rtype, ffi_type **atypes);
        typedef ffi_status ffi_prep_cif_var_type(ffi_cif *cif, ffi_abi abi, unsigned int nfixedargs, unsigned int ntotalargs, ffi_type *rtype, ffi_type **atypes);
        typedef void ffi_call_type(ffi_cif *cif, void (*fn)(void), void *rvalue, void **avalue);

        static ffi_type * m_ffi_type_void;
        static ffi_type * m_ffi_type_uint8;
        static ffi_type * m_ffi_type_sint8;
        static ffi_type * m_ffi_type_uint16;
        static ffi_type * m_ffi_type_sint16;
        static ffi_type * m_ffi_type_uint32;
        static ffi_type * m_ffi_type_sint32;
        static ffi_type * m_ffi_type_uint64;
        static ffi_type * m_ffi_type_sint64;
        static ffi_type * m_ffi_type_float;
        static ffi_type * m_ffi_type_double;
        static ffi_type * m_ffi_type_pointer;

        static ffi_prep_cif_type *m_ffi_prep_cif;
        static ffi_prep_cif_var_type * m_ffi_prep_cif_var;
        static ffi_call_type * m_ffi_call;

        static ObjType m_wide_char_type;
        static ffi_type * m_wide_char_type_ffi;
        static ObjType m_integer_type;

        static RuntimePtr Init(StringArray args);
        static RuntimePtr Init(int argc = 0, const char** argv = nullptr, const char** penv = nullptr);
        //        static RuntimePtr Init(std::vector<const char *> args, bool ignore_error = true);

        static std::string NativeNameMangling(const Term *term, RunTime *rt);
        static std::string NativeNameMangling(std::string_view name);

        static void * GetDirectAddressFromLibrary(void *handle, std::string_view name) {
            if (isNativeName(name)) {
                return LLVMSearchForAddressOfSymbol(NativeNameMangling(name).c_str());
            }
            return LLVMSearchForAddressOfSymbol(name.begin());
        }


        ModulePtr CheckLoadModule(TermPtr &term);
        bool LoadModuleFromFile(const char *name_str, bool init);

        ObjPtr OpLoadModule(TermPtr term);
        ObjPtr ObjFromModule(ModulePtr module, TermPtr term);

        bool UnLoadModule(const char *name_str, bool deinit);
        ObjPtr ExecModule(const char *module, const char *output, bool cached, Context * ctx);

        //        bool CreateModule(const TermPtr &ast, const std::string_view source, const std::string_view module_name, const std::string_view filename, llvm::Module *bc = nullptr);

        bool ModuleCreate(FileModule &data, const std::string_view source);
        static bool ModuleCreate(FileModule &data, const std::string_view module_name, const TermPtr &include, const std::string_view source, llvm::Module *bc = nullptr);
        static bool ModuleSave(const FileModule &data, const std::string_view filename, const std::string_view module_name="");
        static bool ModuleRead(FileModule &data, const std::string_view filename, const std::string_view modulename = "");


        static void * GetNativeAddr(const char * name, void *module = nullptr);

        static std::string GetLastErrorMessage();

        /**
         * Создает иерархию объектов с проверка типов и других семантических правил путём обхода AST
         * 
         * @param ast - AST для анализа
         * @param rt - Объект для хранения глобальных переменных
         * @return Семантический корректный модуль с иерархией глобальных и локальных объектов
         */
        static ModulePtr CreateNameHierarchy(Term ast, RuntimePtr rt);

        /**
         * Функция для организации встроенных типов в иерархию наследования.
         * Другие функции: @ref CreateBaseType - создает базовые типы данных (для расширения классов требуется контекст)
         * и @ref BaseTypeConstructor - функция обратного вызова при создании нового объекта базового типа данных
         * @param type - Базовый тип данных @ref ObjType
         * @param parents - Список сторок с именами родительских типов
         * @return - Успешность регистрации базовго типа в иерархии
         */
        bool RegisterBuildinType(ObjType type, std::vector<std::string> parents);

        static ObjType BaseTypeFromString(RunTime * rt, const std::string_view text, bool *has_error = nullptr);
        ObjPtr GetTypeFromString(const std::string_view type, bool *has_error = nullptr);

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

        //        ObjPtr Eval(const TermPtr &term, Runner *runner = nullptr);

        void CreateArgs_(ObjPtr &args, const TermPtr &term, Context *runner = nullptr);
        ObjPtr CreateDict(const TermPtr &term, Context *runner = nullptr);
        ObjPtr CreateTensor(const TermPtr &term, Context *runner = nullptr);




        //        LLVMBuilderRef m_llvm_builder;

        //        ModulePtr m_buildin_obj;
        ModulePtr m_main_module;

        TermPtr m_main;
        std::map<std::string, ModulePtr> m_modules;
        StringArray m_module_loader;

        std::map<std::string, ObjPtr> m_buildin_obj;

        std::string m_work_dir;
        std::string m_exec_dir;
        std::vector<std::string> m_search_dir;
        bool m_assert_enable;
        bool m_load_dsl;
        bool m_embed_source;
        bool m_import_module;
        bool m_import_native;
        bool m_eval_enable;
        bool m_load_runtime;
        int m_typedef_limit;
        //        ObjPtr m_cmd_args;
        //        ObjPtr m_main_args;
        MacroPtr m_macro;
    public:
        DiagPtr m_diag;

        TermPtr m_main_ast;
        RunnerPtr m_main_runner;

        // Выполняет одну строку в контексте главного модуля программы
        // При первом вызове в m_main_ast создается AST как BLOCK
        // При повторнвых вызовах в m_main_ast->m_block добавляется новая строка
        ObjPtr Run(const std::string_view str, Obj* args = nullptr);

        // Выполняет скопилированное AST как главный модуль программы
        // При повторном вызове m_main_ast заменяется на новый AST
        ObjPtr Run(TermPtr ast, Obj* args = nullptr);
        ObjPtr RunFile(std::string file, Obj* args = nullptr);
        bool ExpandFileName(std::string &filename);

        int RunMain();
        bool CompileCppSource(const std::string_view source, std::string &out, std::vector<std::string> opts = {});


        TermPtr MakeAst(const std::string_view src, bool skip_analize = false);

        ParserPtr GetParser();

        static StringArray MakeMainArgs(int argc, const char** argv, const char** penv) {
            StringArray result;
            for (int i = 0; i < argc; i++) {
                result.push_back(std::string(argv[i]));
            }
            return result;
        }

        static ObjPtr MakeObjArgs(int argc, const char** argv) {
            ObjPtr result = Obj::CreateDict();
            for (int i = 0; i < argc; i++) {
                result->push_back(Obj::CreateString(argv[i]));
            }
            return result;
        }

    protected:

        bool ParseArgs(StringArray args) {

            std::vector<std::string> load;

            for (int i = 0; i < args.size(); i++) {

                //                m_cmd_args->push_back(Obj::CreateString(args[i]));

                if (args[i].find("--nlc-search=") == 0) {
                    std::string list;
                    list = args[i].substr(strlen("--nlc-search="));
                    m_search_dir = Macro::SplitString(list.c_str(), ";");
                } else if (args[i].compare("--nlc-search=") == 0) {
                } else if (args[i].compare("--nlc-no-runtime") == 0) {
                    m_load_runtime = false;
                } else if (args[i].compare("--nlc-no-dsl") == 0) {
                    m_load_dsl = false;
                    //                } else if (args[i].compare("--nlc-main-arg=") == 0 || strcmp(arg_test, "--nlc-main-args=") == 0) {
                    //
                    //                    std::string temp(arg_test);
                    //                    std::string call("(");
                    //
                    //                    call += temp.substr(temp.find("=") + 1);
                    //                    call += ",)";
                    //                    m_main_args = EvalStatic(MakeAst(call, true), false);

                } else if (args[i].compare("--nlc-embed-source") == 0) {
                    LOG_RUNTIME("Flag '--nlc-embed-source' not implemented!");
                    m_embed_source = true;
                } else if (args[i].compare("--nlc-no-embed-source") == 0) {
                    m_embed_source = false;
                } else if (args[i].compare("--nlc-no-import-module") == 0) {
                    m_import_module = false;
                } else if (args[i].compare("--nlc-no-import-native") == 0) {
                    m_import_native = false;
                } else if (args[i].compare("--nlc-no-eval-enable") == 0) {
                    m_eval_enable = false;
                } else if (args[i].compare("--nlc-no-assert") == 0) {
                    m_assert_enable = false;
                } else if (args[i].find("--nlc-error-limit=") == 0) {
                    std::string value(args[i]);
                    m_diag->m_error_limit = parseInteger(value.substr(strlen("--nlc-error-limit=")).c_str());
                    //                } else if (args[i].compare("--nlc-load=") == arg_test) {
                    //                    load.push_back(std::string(arg_test).substr(strlen("--nlc-load=")));
                } else {
                    LOG_RUNTIME("System argument '%s' not recognized!", args[i].c_str());
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

            for (auto &elem : load) {

            }

            return true;
        }

        //SCOPE(private) :
    public:
        RunTime(const RunTime&) = delete;
        const RunTime& operator=(const RunTime&) = delete;

    };

    /*
     * 
     * 
     *      
     */
    ObjType typeFromString(TermPtr &term, RunTime *rt, bool *has_error);
}


#endif //INCLUDED_RUNTIME_CPP_
