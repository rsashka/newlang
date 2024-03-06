#pragma once
#ifndef INCLUDED_RUNTIME_CPP_
#define INCLUDED_RUNTIME_CPP_

#include "pch.h"

//#include "stdlib.h"
//#include "stdio.h"
//#include "string.h"
//#include "sys/times.h"
//#include "sys/vtimes.h"
#include <sys/time.h>

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

    //    /*
    //     * Класс для хренения имен переменных в соответствии с их областью видимости.
    //     * Имена во внутренних областях могут перекрывать внешние, но в
    //     * одной области видимости (блоке) имена переменных должны быть уникальны.
    //     */
    //    class VarScope {
    //    public:
    //        
    //        typedef std::map<std::string, TermPtr> VarTermMap;
    //        
    //        struct Block {
    //            TermPtr scope_name; ///< Имя блока кода
    //            VarTermMap vars; ///< Список имен переменных определеных для текущего блока кода
    //        };
    //        static const TermPtr NonameBlock; ///< Имя "_" для безымянного блока кода {...}
    //        std::vector< Block > m_stack; ///< Иерархия (стек) блоков кода
    //        VarTermMap *m_root; ///< Корневой узел для хренения переменных модуля
    //        //        StringArray m_last_name;
    //
    //        StringArray m_blocks;
    //        TermPtr m_class;
    //        TermPtr m_function;
    //
    //    public:
    //        TermPtr m_op;
    //
    //        void SetFunc(TermPtr func) {
    //            m_function = func;
    //            if (func) {
    //                m_blocks.push_back(func->m_text);
    //            } else {
    //                m_blocks.pop_back();
    //            }
    //        }
    //
    //        void SetClass(TermPtr cls) {
    //            m_class = cls;
    //        }
    //
    //        inline TermPtr GetClass() {
    //            return m_class;
    //        }
    //
    //        inline TermPtr GetFunc() {
    //            return m_function;
    //        }
    //
    //        VarScope(VarTermMap *root = nullptr) : m_root(root), /*m_last_name({""}), */m_class(nullptr), m_function(nullptr), m_op(nullptr) {
    //        }
    //
    ////        TermPtr GetRoot() {
    ////            return m_root;
    ////        }
    //
    //        static std::string EnumerateString(const StringArray &names) {
    //            std::string fails;
    //            for (auto &elem : names) {
    //                if (!fails.empty()) {
    //                    fails += ", ";
    //                }
    //                fails += elem;
    //            }
    //            return fails;
    //        }
    //
    ////        std::string GetLastNames() {
    ////            return EnumerateString(m_last_name);
    ////        };
    //
    //        //// error: no member named 'isCreate' in 'newlang::Term'; did you mean 'IsCreate'?
    //
    //        std::string GetOffer() {
    ////            ASSERT(m_last_name.size());
    //            return "";
    //        }
    //
    //        std::string GetOfferBlock() {
    //            if (m_blocks.empty()) {
    //                return "";
    //            }
    //            std::string result = " Possible block identifiers: '";
    //            result += EnumerateString(m_blocks);
    //            result += "'";
    //            return result;
    //        }
    //
    //        std::string Get__CLASS__();
    //        std::string Get__FUNCTION__();
    //        std::string Get__FUNCDNAME__();
    //        std::string Get__FUNCSIG__();
    //        std::string Get__BLOCK__();
    //
    //        static inline bool isRoot(const std::string & name) {
    //            return name.find("::") == 0;
    //        }
    //
    //        // ::, ::name, name, ns::name
    //
    //        //        bool NamespacePush(const std::string & name) {
    //        //            if (name.empty()) {
    //        //                return false;
    //        //            }
    //        //            if (name.size() > 2 && name.rfind("::") == name.size() - 2) {
    //        //                push_back(name.substr(0, name.size() - 2));
    //        //            } else {
    //        //                push_back(name);
    //        //            }
    //        //            m_blocks.push_back(back());
    //        //            return true;
    //        //        }
    //        //
    //        //        void NamespacePop() {
    //        //            ASSERT(!empty());
    //        //            pop_back();
    //        //            m_blocks.pop_back();
    //        //        }
    //        //
    //        //        std::string GetNamespace(const std::string name = "") {
    //        //            std::string result(name);
    //        //            for (size_t i = size(); i > 0; i--) {
    //        //                if (result.find("::") == 0) {
    //        //                    break;
    //        //                }
    //        //                if (!result.empty() && at(i - 1).compare("::") != 0) {
    //        //                    result.insert(0, "::");
    //        //                }
    //        //                result.insert(0, at(i - 1));
    //        //            }
    //        //            return result;
    //        //        }
    //
    //        bool NameMacroExpand(TermPtr term);
    //        bool LookupBlock(TermPtr &term);
    //
    //        bool LookupName(TermPtr term, RunTime * rt);
    //
    //        /*
    //         * 
    //         * 
    //         */
    //        void PushScope(TermPtr ns) {
    //            m_stack.push_back(Block({ns ? ns : NonameBlock,
    //                {}}));
    //        }
    //
    //        void PopScope() {
    //            ASSERT(!m_stack.empty());
    //            m_stack.pop_back();
    //        }
    //
    //        bool AddVar(TermPtr var) {
    //            ASSERT(!m_stack.empty());
    //            if (m_stack.back().vars.find(var->m_text.GetLocalName()) != m_stack.back().vars.end()) {
    //                NL_MESSAGE(LOG_LEVEL_INFO, var, "Var '%s' exist!", var->m_text.GetLocalName().c_str());
    //                return false;
    //            }
    //            m_stack.back().vars.insert({var->m_text.GetLocalName(), var});
    //            return true;
    //        }
    //
    //        TermPtr LookupVar(TermPtr var) {
    //            auto iter = m_stack.rbegin();
    //            while (iter != m_stack.rend()) {
    //                if (iter->vars.find(var->m_text.GetLocalName()) != iter->vars.end()) {
    //                    return iter->vars.at(var->m_text.GetLocalName());
    //                }
    //                iter++;
    //            }
    //            return nullptr;
    //        }
    //
    //        TermPtr LookupNamespace(TermPtr ns) {
    //            auto iter = m_stack.rbegin();
    //            while (iter != m_stack.rend()) {
    //                ASSERT(iter->scope_name);
    //                if (iter->scope_name->m_text.compare(ns->m_text) == 0) {
    //                    return iter->scope_name;
    //                }
    //                iter++;
    //            }
    //            return nullptr;
    //        }
    //
    //        std::string GetFullNamespace(const std::string name = "") {
    //            std::string result(name);
    //            auto iter = m_stack.rbegin();
    //            while (iter != m_stack.rend()) {
    //                if (result.find("::") == 0) {
    //                    break;
    //                }
    //                if (iter->scope_name) {
    //                    if (!result.empty() && iter->scope_name->m_text.compare("::") != 0) {
    //                        result.insert(0, "::");
    //                    }
    //                    result.insert(0, iter->scope_name->m_text);
    //                }
    //                iter++;
    //            }
    //            return result;
    //        }
    //
    //        std::string Dump() {
    //            std::string result;
    //            auto iter = m_stack.begin();
    //            while (iter != m_stack.end()) {
    //                if (!result.empty()) {
    //                    result += "\n";
    //                }
    //                ASSERT(iter->scope_name);
    //                result += iter->scope_name->m_text;
    //
    //                std::string list;
    //                auto iter_list = iter->vars.begin();
    //                while (iter_list != iter->vars.end()) {
    //
    //                    if (!list.empty()) {
    //                        list += ", ";
    //                    }
    //
    //                    list += iter_list->first;
    //                    iter_list++;
    //                }
    //
    //
    //                result += " = <";
    //                result += list;
    //                result += ">";
    //
    //                iter++;
    //            }
    //
    //            return result;
    //
    //        }
    //
    //    };

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
        TermPtr GlobFindProto(const std::string_view name);

        ObjPtr NameGet(const char *name, bool is_raise = true);

        //        bool MakeFullNames(TermPtr ast);
        std::string Dump();

        void GlobalNameBuildinRegister();
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

        RunTime();

        virtual ~RunTime() {

            LLVMDisposeBuilder(m_llvm_builder);
            m_llvm_builder = nullptr;
            //LLVMShutdown();
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



        static RuntimePtr Init(StringArray args);
        static RuntimePtr Init(int argc = 0, const char** argv = nullptr);
        //        static RuntimePtr Init(std::vector<const char *> args, bool ignore_error = true);

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

        /*
         * При раскрытии области видимости объектов и проверки корректности их имен,
         * нужно одновремно регистрировать глобальные и локальные переменные, 
         * так как nameloockup выполняется последовательно и если создавать переменные позже, 
         * то обращение может произойти к неправильным переменным 
         * (либо будет неправильное перекрытие, либо ошибка, т.к. переменная еще не создана).
         * 
         * А если регистрировать не только глобальные, но и локальные объекты, 
         * тогда в функцию анализатора нужно передавать и объект - владалец локальных переменных.
         * Точнее он уже есть, это либо глоальныей корень для переменных и функций модуля, 
         * либо глобальные объекты - функции с собственными локальными переменными.
         * m_variables
         */
        bool AstAnalyze(TermPtr &term, TermPtr &root);
        bool AstRecursiveAnalyzer(TermPtr &term, ScopeStack &stack);
        bool AstCreateOp_(TermPtr &term, ScopeStack &stack);
        bool AstCreateVar(TermPtr &var, TermPtr &value, ScopeStack &stack);
        bool AstAssignVar(TermPtr &var, TermPtr &value, ScopeStack &stack);
        bool AstIterator_(TermPtr &term, ScopeStack &stack);

        TermPtr AstLockupName(TermPtr &term, ScopeStack &stack);
        bool AstCheckNative_(TermPtr &term);

        /**
         * Функция проверки наличия ошибок при анализе AST.
         * Прерывает работу анализатора при превышении лимита или при force=true
         * @param force - Если есть ошибки- завершить работу
         */
        bool AstCheckError(bool result);

        bool CheckName(TermPtr &term);
        bool CheckOp(TermPtr &term);
        bool CalcType(TermPtr &term);
        bool AstUpcastOpType(TermPtr &op);
        bool AstCheckType(TermPtr &left, const TermPtr right);
        bool AstCheckCall_(TermPtr &proto, TermPtr &term);
        bool AstCheckNative_(TermPtr &proto, TermPtr &term);

        /**
         * Функция для организации встроенных типов в иерархию наследования.
         * Другие функции: @ref CreateBaseType - создает базовые типы данных (для расширения классов требуется контекст)
         * и @ref BaseTypeConstructor - функция обратного вызова при создании нового объекта базового типа данных
         * @param type - Базовый тип данных @ref ObjType
         * @param parents - Список сторок с именами родительских типов
         * @return - Успешность регистрации базовго типа в иерархии
         */
        bool RegisterBuildinType(ObjType type, std::vector<std::string> parents);

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

        ObjPtr EvalStatic(const TermPtr term, bool pure);
        ObjPtr Eval(const TermPtr term, RunnerPtr runner = nullptr, bool pure = false);

        void CreateArgs_(ObjPtr &args, TermPtr &term, RunnerPtr runner = nullptr, bool is_pure = false);
        ObjPtr CreateDict(TermPtr term, RunnerPtr runner = nullptr, bool is_pure = false);
        ObjPtr CreateTensor(TermPtr term, RunnerPtr runner = nullptr, bool is_pure = false);




        LLVMBuilderRef m_llvm_builder;

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
        bool m_import_natime;
        bool m_load_runtime;
        int m_error_limit;
        int m_error_count;
        int m_typedef_limit;
        //        ObjPtr m_cmd_args;
        //        ObjPtr m_main_args;
        MacroPtr m_macro;
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

        TermPtr MakeAst(const std::string_view src, bool skip_analize = false);

        ParserPtr GetParser();
        bool AstCheckArgsType_(TermPtr proto, TermPtr value);
        bool AstCheckArgs_(TermPtr proto, TermPtr args);


        //        static const int default_argc = 4;
        //        static const char * default_argv[default_argc];

        static StringArray MakeMainArgs(int argc, const char** argv) {
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

        //        /*
        //         * -nlc-search=sss;sss;sss;sss (char [3][const char*]){"", "-nlc-no-runtime", "-nlc-no-dsl"}
        //         */
        //        bool ParseArgs(int argc, const char** argv) {
        //
        //            std::vector<std::string> load;
        //
        //            for (int i = 0; i < argc; i++) {
        //
        //                m_cmd_args->push_back(Obj::CreateString(argv[i]));
        //
        //                const char *arg_test;
        //                if (strstr(argv[i], "-nlc-") == argv[i] || strstr(argv[i], "--nlc-") == argv[i]) {
        //                    if (strstr(argv[i], "--nlc-") == argv[i]) {
        //                        arg_test = &argv[i][1];
        //                    } else {
        //                        arg_test = argv[i];
        //                    }
        //
        //                    /*
        //                     * nlc
        //                     * dlc
        //                     * 
        //                     * Запуска 
        //                     * --playground-cgi="format='json', port=1000, thread=10, memory=5, timeout=10"
        //                     * 
        //                     * --eval-source --eval-src     - выполнить одну строку
        //                     * --eval-file                  - выполнить файл
        //                     * --make-module                - создать (скомпилировать) модуль
        //                     * --make-program --make-prg    - создать (скомпилировать) приложение
        //                     * --repl (или без аргументов)
        //                     * 
        //                     */
        //                    if (strstr(arg_test, "-nlc-search=") == arg_test) {
        //                        std::string list(arg_test);
        //                        list = list.substr(strlen("-nlc-search="));
        //                        m_search_dir = Macro::SplitString(list.c_str(), ";");
        //                    } else if (strstr(arg_test, "-nlc-search=") == arg_test) {
        //                    } else if (strcmp(arg_test, "-nlc-no-runtime") == 0) {
        //                        m_load_runtime = false;
        //                    } else if (strcmp(arg_test, "-nlc-no-dsl") == 0) {
        //                        m_load_dsl = false;
        //                    } else if (strcmp(arg_test, "-nlc-main-arg=") == 0 || strcmp(arg_test, "-nlc-main-args=") == 0) {
        //
        //                        std::string temp(arg_test);
        //                        std::string call("(");
        //
        //                        call += temp.substr(temp.find("=") + 1);
        //                        call += ",)";
        //                        m_main_args = EvalStatic(MakeAst(call, true), false);
        //
        //                    } else if (strcmp(arg_test, "-nlc-embed-source") == 0) {
        //                        m_embed_source = true;
        //                    } else if (strcmp(arg_test, "-nlc-no-embed-source") == 0) {
        //                        m_embed_source = false;
        //                    } else if (strcmp(arg_test, "-nlc-no-import-module") == 0) {
        //                        m_import_module = false;
        //                    } else if (strcmp(arg_test, "-nlc-no-import-native") == 0) {
        //                        m_import_natime = false;
        //                    } else if (strcmp(arg_test, "-nlc-no-assert") == 0) {
        //                        m_assert_enable = false;
        //                    } else if (strstr(arg_test, "-nlc-error-limit=") == arg_test) {
        //                        std::string value(argv[i]);
        //                        m_error_limit = parseInteger(value.substr(strlen("-nlc-error-limit=")).c_str());
        //                    } else if (strstr(arg_test, "-nlc-load=") == arg_test) {
        //                        load.push_back(std::string(arg_test).substr(strlen("-nlc-load=")));
        //                    } else {
        //                        LOG_RUNTIME("System arg '%s' not found!", argv[i]);
        //                    }
        //                }
        //            }
        //
        //
        //            llvm::SmallString<1024> path;
        //            auto error = llvm::sys::fs::current_path(path);
        //            if (error) {
        //                LOG_RUNTIME("%s", error.message().c_str());
        //            }
        //            m_work_dir = path.c_str();
        //            // LOG_DEBUG("work_dir: %s", m_work_dir.c_str());
        //
        //            path = llvm::sys::fs::getMainExecutable(nullptr, nullptr);
        //            // LOG_DEBUG("%s", path.c_str());
        //
        //            llvm::sys::path::remove_filename(path);
        //            m_exec_dir = path.c_str();
        //            // LOG_DEBUG("exec_dir: %s", m_exec_dir.c_str());
        //
        //            m_search_dir.push_back(m_work_dir);
        //            m_search_dir.push_back(m_exec_dir);
        //
        //            for (auto &elem : load) {
        //
        //            }
        //
        //            return true;
        //        }

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
                    //                } else if (args[i].compare("--nlc-main-arg=") == 0 || strcmp(arg_test, "-nlc-main-args=") == 0) {
                    //
                    //                    std::string temp(arg_test);
                    //                    std::string call("(");
                    //
                    //                    call += temp.substr(temp.find("=") + 1);
                    //                    call += ",)";
                    //                    m_main_args = EvalStatic(MakeAst(call, true), false);

                } else if (args[i].compare("--nlc-embed-source") == 0) {
                    m_embed_source = true;
                } else if (args[i].compare("--nlc-no-embed-source") == 0) {
                    m_embed_source = false;
                } else if (args[i].compare("--nlc-no-import-module") == 0) {
                    m_import_module = false;
                } else if (args[i].compare("--nlc-no-import-native") == 0) {
                    m_import_natime = false;
                } else if (args[i].compare("--nlc-no-assert") == 0) {
                    m_assert_enable = false;
                } else if (args[i].find("--nlc-error-limit=") == 0) {
                    std::string value(args[i]);
                    m_error_limit = parseInteger(value.substr(strlen("--nlc-error-limit=")).c_str());
                    //                } else if (args[i].compare("--nlc-load=") == arg_test) {
                    //                    load.push_back(std::string(arg_test).substr(strlen("-nlc-load=")));
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
    inline ObjType typeFromString(const TermPtr &term, RunTime *rt, bool *has_error) {
        if (term) {
            return newlang::typeFromString(term->m_text, rt, has_error);
        }
        return newlang::typeFromString(std::string(":Any"), rt, has_error);
    }
}


#endif //INCLUDED_RUNTIME_CPP_
