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
    bool CheckCharModuleName(const char *fileName);

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
     */

    class NsStack : SCOPE(private) StringArray {

        struct NsItem {
            TermPtr ns_name;
            std::map<std::string, TermPtr> vars;
        };
        std::vector< NsItem > m_stack;
        static const TermPtr NonameBlock;

        TermPtr m_root;
        StringArray m_last_name;

        StringArray m_blocks;
        TermPtr m_class;
        TermPtr m_function;

    public:
        TermPtr m_op;

        void SetFunc(TermPtr func) {
            m_function = func;
            if (func) {
                m_blocks.push_back(func->m_text);
            } else {
                m_blocks.pop_back();
            }
        }

        void SetClass(TermPtr cls) {
            m_class = cls;
        }

        inline TermPtr GetClass() {
            return m_class;
        }

        inline TermPtr GetFunc() {
            return m_function;
        }

        NsStack(TermPtr root) : m_root(root), m_last_name({""}), m_class(nullptr), m_function(nullptr), m_op(nullptr) {
        }

        TermPtr GetRoot() {
            return m_root;
        }

        static std::string EnumerateString(const StringArray &names) {
            std::string fails;
            for (auto &elem : names) {
                if (!fails.empty()) {
                    fails += ", ";
                }
                fails += elem;
            }
            return fails;
        }

        std::string GetLastNames() {
            return EnumerateString(m_last_name);
        };

        //// error: no member named 'isCreate' in 'newlang::Term'; did you mean 'IsCreate'?

        std::string GetOffer() {
            ASSERT(m_last_name.size());
            return m_last_name[0];
        }

        std::string GetOfferBlock() {
            if (m_blocks.empty()) {
                return "";
            }
            std::string result = " Possible block identifiers: '";
            result += EnumerateString(m_blocks);
            result += "'";
            return result;
        }

        std::string Get__CLASS__();
        std::string Get__FUNCTION__();
        std::string Get__FUNCDNAME__();
        std::string Get__FUNCSIG__();
        std::string Get__BLOCK__();

        static inline bool isRoot(const std::string & name) {
            return name.find("::") == 0;
        }

        // ::, ::name, name, ns::name

        bool NamespacePush(const std::string & name) {
            if (name.empty()) {
                return false;
            }
            if (name.size() > 2 && name.rfind("::") == name.size() - 2) {
                push_back(name.substr(0, name.size() - 2));
            } else {
                push_back(name);
            }
            m_blocks.push_back(back());
            return true;
        }

        void NamespacePop() {
            ASSERT(!empty());
            pop_back();
            m_blocks.pop_back();
        }

        std::string GetNamespace(const std::string name = "") {
            std::string result(name);
            for (size_t i = size(); i > 0; i--) {
                if (result.find("::") == 0) {
                    break;
                }
                if (!result.empty() && at(i - 1).compare("::") != 0) {
                    result.insert(0, "::");
                }
                result.insert(0, at(i - 1));
            }
            return result;
        }

        bool NameMacroExpand(TermPtr term);
        bool LookupBlock(TermPtr &term);

        bool LookupName(TermPtr term, RunTime * rt);

        /*
         * 
         * 
         */
        void PushScope(TermPtr ns) {
            m_stack.push_back(NsItem({ns ? ns : NonameBlock,
                {}}));
        }

        void PopScope() {
            ASSERT(!m_stack.empty());
            m_stack.pop_back();
        }

        bool AddVar(TermPtr var) {
            ASSERT(!m_stack.empty());
            if (m_stack.back().vars.find(var->m_text.GetLocalName()) != m_stack.back().vars.end()) {
                NL_MESSAGE(LOG_LEVEL_INFO, var, "Var '%s' exist!", var->m_text.GetLocalName().c_str());
                return false;
            }
            m_stack.back().vars.insert({var->m_text.GetLocalName(), var});
            return true;
        }

        TermPtr LookupVar(TermPtr var) {
            auto iter = m_stack.rbegin();
            while (iter != m_stack.rend()) {
                if (iter->vars.find(var->m_text.GetLocalName()) != iter->vars.end()) {
                    return iter->vars.at(var->m_text.GetLocalName());
                }
                iter++;
            }
            return nullptr;
        }

        TermPtr LookupNamespace(TermPtr ns) {
            auto iter = m_stack.rbegin();
            while (iter != m_stack.rend()) {
                ASSERT(iter->ns_name);
                if (iter->ns_name->m_text.compare(ns->m_text) == 0) {
                    return iter->ns_name;
                }
                iter++;
            }
            return nullptr;
        }

        std::string GetFullNamespace(const std::string name = "") {
            std::string result(name);
            auto iter = m_stack.rbegin();
            while (iter != m_stack.rend()) {
                if (result.find("::") == 0) {
                    break;
                }
                if (iter->ns_name) {
                    if (!result.empty() && iter->ns_name->m_text.compare("::") != 0) {
                        result.insert(0, "::");
                    }
                    result.insert(0, iter->ns_name->m_text);
                }
                iter++;
            }
            return result;
        }

    };

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

        TermPtr NameRegister(bool new_only, const char *name, TermPtr proto, WeakItem obj = c10::monostate()); // = at::monostate);
        //        bool NameRegister(const char * glob_name, TermPtr proto, WeakItem obj); // = at::monostate);
        GlobItem * NameFind(const char *name);
        TermPtr GlobFindProto(const char *name);

        ObjPtr NameGet(const char *name, bool is_raise = true);

        //        bool MakeFullNames(TermPtr ast);
        std::string Dump();

        void GlobalNameBuildinRegister();
        bool RegisterBuildin(BuildinPtr module);
        bool RegisterModule(ModulePtr module);

        RunTime();

        bool RegisterNativeObj(TermPtr term) {
            ASSERT(term);
            ASSERT(term->getTermID() == TermID::NATIVE);

            return RegisterSystemObj(CreateNative(nullptr, term));
        }

        bool RegisterSystemBuildin(const char *text);
        bool RegisterSystemObj(ObjPtr obj);
        std::vector<ObjPtr> m_sys_obj;

        static ObjPtr CreateNative(Context *ctx, const char *proto, const char *module = nullptr, bool lazzy = false, const char *mangle_name = nullptr);
        static ObjPtr CreateNative(Context *ctx, TermPtr proto, const char *module = nullptr, bool lazzy = false, const char *mangle_name = nullptr);
        static ObjPtr CreateNative(TermPtr proto, void *addr, Context *ctx);

        virtual ~RunTime() {

            LLVMDisposeBuilder(m_llvm_builder);
            m_llvm_builder = nullptr;
            //LLVMShutdown();
        }

        static RuntimePtr Init(int argc = 0, const char** argv = nullptr, bool ignore_error = true);
        static RuntimePtr Init(std::vector<const char *> args, bool ignore_error = true);

        inline static void * GetDirectAddressFromLibrary(void *handle, const char * name) {
            if (isNativeName(name)) {

                return LLVMSearchForAddressOfSymbol(&name[1]);
            }
            return LLVMSearchForAddressOfSymbol(name);
        }


        ModulePtr CheckLoadModule(TermPtr &term);
        bool LoadModuleFromFile(const char *name_str, bool init);

        ObjPtr OpLoadModule(TermPtr term);
        ObjPtr ObjFromModule(ModulePtr module, TermPtr term);

        bool UnLoadModule(const char *name_str, bool deinit);
        ObjPtr ExecModule(const char *module, const char *output, bool cached, Context * ctx);

        static void * GetNativeAddr(const char * name, const char *module = nullptr);

        static std::string GetLastErrorMessage();

        /**
         * Создает иерархию объектов с проверка типов и других семантических правил путём обхода AST
         * 
         * @param ast - AST для анализа
         * @param rt - Объект для хранения глобальных переменных
         * @return Семантический корректный модуль с иерархией глобальных и локальных объектов
         */
        static ModulePtr CreateNameHierarchy(Term ast, RuntimePtr rt);

        /*
         * При раскрытии области видимости объектов и проверки корректности их имен,
         * нужно одновремно регистрировать глобальные и локальные переменные, 
         * так как nameloockup выполняется последовательно, и если создавать переменные позже, 
         * то обращение может произойти к неправильным переменным 
         * (либо будет неправильное перекрытие, либо ошибка, т.к. переменная еще не создана).
         * 
         * А если регистрировать не только глобальные, но и локальные объекты, 
         * тогда в функцию анализатора нужно передавать и объект - владалец локальных переменных.
         * Точнее он уже есть, это либо глоальныей корень для переменных и функций модуля, 
         * либо глобальные объекты - функции с собственными локальными переменными.
         * m_variables
         */
        bool AstAnalyze(TermPtr &term, bool is_main = true);
        void AstRecursiveAnalyzer(TermPtr &term, NsStack &stack);
        bool AstCreateOp(TermPtr &term, NsStack &stack);
        bool AstCreateVar(TermPtr &var, TermPtr &value, NsStack &stack);
        bool AstAssignVar(TermPtr &var, TermPtr &value, NsStack &stack);
        /**
         * Функция проверки наличия ошибок при анализе AST.
         * Прерывает работу анализатора при превышении лимита или при force=true
         * @param force - Если есть ошибки- завершить работу
         */
        void AstCheckError(bool force = false);

        bool CheckName(TermPtr &term);
        bool CheckOp(TermPtr &term);
        bool CalcType(TermPtr &term);
        bool CheckType(TermPtr &left, TermPtr &right);

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

        static ObjPtr EvalStatic(const TermPtr term, bool pure = false);





        LLVMBuilderRef m_llvm_builder;

        ModulePtr m_buildin_obj;
        ModulePtr m_main_module;

        TermPtr m_main;
        std::map<std::string, ModulePtr> m_modules;
        StringArray m_module_loader;

        std::map<std::string, ObjPtr> m_types;

        std::string m_work_dir;
        std::string m_exec_dir;
        std::vector<std::string> m_search_dir;
        bool m_assert_enable;
        bool m_load_dsl;
        bool m_load_runtime;
        int m_error_limit;
        int m_error_count;
        int m_typedef_limit;
        ObjPtr m_args;
        MacroPtr m_macro;
        DiagPtr m_diag;

        ParserPtr GetParser();

    protected:

        static const int default_argc = 3;
        static const char * default_argv[default_argc];

        /*
         * -nlc-search=sss;sss;sss;sss (char [3][const char*]){"", "-nlc-no-runtime", "-nlc-no-dsl"}
         */
        bool ParseArgs(int argc = default_argc, const char** argv = default_argv) {

            std::vector<std::string> load;

            for (int i = 0; i < argc; i++) {

                m_args->push_back(Obj::CreateString(argv[i]));

                const char *arg_test;
                if (strstr(argv[i], "-nlc-") == argv[i] || strstr(argv[i], "--nlc-") == argv[i]) {
                    if (strstr(argv[i], "--nlc-") == argv[i]) {
                        arg_test = &argv[i][1];
                    } else {
                        arg_test = argv[i];
                    }

                    if (strstr(arg_test, "-nlc-search=") == arg_test) {
                        std::string list(arg_test);
                        list = list.substr(strlen("-nlc-search="));
                        m_search_dir = Macro::SplitString(list.c_str(), ";");
                    } else if (strstr(arg_test, "-nlc-search=") == arg_test) {
                    } else if (strcmp(arg_test, "-nlc-no-runtime") == 0) {
                        m_load_runtime = false;
                    } else if (strcmp(arg_test, "-nlc-no-dsl") == 0) {
                        m_load_dsl = false;
                    } else if (strcmp(arg_test, "-nlc-no-assert") == 0) {
                        m_assert_enable = false;
                    } else if (strstr(arg_test, "-nlc-error-limit=") == arg_test) {
                        std::string value(argv[i]);
                        m_error_limit = parseInteger(value.substr(strlen("-nlc-error-limit=")).c_str());
                    } else if (strstr(arg_test, "-nlc-load=") == arg_test) {
                        load.push_back(std::string(arg_test).substr(strlen("-nlc-load=")));
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
    inline ObjType typeFromString(const TermPtr &term, RuntimePtr rt, bool *has_error) {
        if (term) {
            return newlang::typeFromString(term->m_text, rt, has_error);
        }
        return newlang::typeFromString(std::string(":Any"), rt, has_error);
    }
}


#endif //INCLUDED_RUNTIME_CPP_
