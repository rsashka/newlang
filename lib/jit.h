#ifndef INCLUDED_JIT_H_
#define INCLUDED_JIT_H_

#include "warning_push.h"

#include <clang/AST/ASTConcept.h>
#include <clang/Sema/SemaConcept.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Frontend/CompilerInstance.h>

#include <clang/AST/Mangle.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>

#include <llvm-c/Core.h>
#include <llvm-c/Support.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Analysis.h>

#include "llvm/Support/Path.h"
#include "llvm/Support/FileSystem.h"
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/TargetSelect.h>

#include <llvm/InitializePasses.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>

#include <llvm/IR/LegacyPassManagers.h>
#include <llvm/Transforms/Scalar.h>



#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/LinkAllIR.h>
#include <llvm/LinkAllPasses.h>

#include "clang/Driver/Driver.h"
#include "clang/Driver/Compilation.h"

#include "llvm-c/Core.h"
#include "llvm-c/Target.h"
#include "llvm-c/TargetMachine.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/CodeGen.h"

#include "llvm/Support/InitLLVM.h"

#include "llvm/Target/TargetMachine.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/CodeGen/CodeGenAction.h"

#include "warning_pop.h"

#include "nlc-rt.h"

#include "runtime.h"
#include "macro.h"
#include "analysis.h"
#include "module.h"



namespace newlang {

#define NEWLANG_INDENT_OP   "  "
#define NEWLANG_PREFIX      "newlang"
#define NEWLANG_NS          "newlang"

    bool Tranliterate(const wchar_t c, std::wstring &str);
    std::string MangleName(const char * name);

    std::string MangaledFuncCPP(const char *name, const char *space = nullptr);
    std::string MangaledFunc(const std::string name);


    LLVMGenericValueRef GetGenericValueRef(Obj &obj, LLVMTypeRef type);
    ObjPtr CreateFromGenericValue(ObjType type, LLVMGenericValueRef ref, LLVMTypeRef llvm_type);
    LLVMTypeRef toLLVMType(ObjType t, bool none_if_error = false);
    ObjPtr JitLastResult(ObjPtr val);

    class JitParser : public Parser {
    public:

        JitParser(MacroPtr macro = nullptr, PostLexerType *postlex = nullptr, DiagPtr diag = nullptr,
                bool pragma_enable = true, RunTime *rt = nullptr)
        : Parser(macro, postlex, diag, pragma_enable, rt) {

        }

        TermPtr CheckLoadModule(const TermPtr & term) override;
        TermPtr LoadIfModule(const TermPtr & term) override;
        TermPtr ParseFile(const std::string_view filename);
    };

    class JIT : public RunTime {
    public:

        //        RuntimePtr m_rt;
        MacroPtr m_macro;
        size_t m_repl_count;
        ObjPtr m_latter;

        std::vector<ModulePtr> m_module;
        std::vector<std::shared_ptr<Context>> m_ctx;

        inline TermPtr & GetAst() {
            return (*m_module.begin())->m_ast;
        }

        inline Context & GetCtx() {
            ASSERT(m_ctx.size());
            return *(*m_ctx.begin());
        }

        clang::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts;
        clang::TextDiagnosticPrinter *textDiagPrinter;
        clang::IntrusiveRefCntPtr<clang::DiagnosticIDs> pDiagIDs;
        clang::DiagnosticsEngine *pDiagnosticsEngine;


        JIT(const StringArray &args = {});

        virtual ~JIT() {

            delete pDiagnosticsEngine;
            pDiagnosticsEngine = nullptr;
            pDiagIDs.reset();

            //            delete textDiagPrinter;
            //            textDiagPrinter = nullptr;
            DiagOpts.reset();
            LLVMShutdown();
        }


        static JIT * m_instance;

        void Clear() {
            if (m_instance) {
                delete m_instance;
            }
            m_instance = nullptr;
            //void LLVMGetVersion(unsigned *Major, unsigned *Minor, unsigned *Patch);

        }

        static JIT * ReCreate(const StringArray &args = {}) {
            Instance()->Clear();
            return Instance(args);
        }

        static JIT * Instance(const StringArray &args = {}) { //RuntimePtr rt
            //            ASSERT(rt);
            if (m_instance == nullptr) { // || (rt && &m_instance->m_rt != rt)) {
                //                ASSERT(rt);
                //                if (m_instance) {
                //                    delete m_instance;
                //                }
                m_instance = new JIT(args);
            }
            ASSERT(m_instance);
            //            m_instance->m_rt = rt;
            return m_instance;
        }

        JIT(const JIT&) = delete;
        const JIT& operator=(const JIT&) = delete;

        bool CreateMacro(const std::string_view text);

        /*
         * Корневая область имен C++ для генерации кода namespace nlc::gen { }, но все функции определяются как extern "C" 
         * Тело модуля - это тело головной функции __init__, которая выполняется при его загрузке (выполнении).
         * Функцию можно определить в любом блоке кода, но это влияет только на ее доступность в NewLang,
         * так как в коде C++ она все равно создается в корневой области имен.
         * 
         * 
         * Если в теле модуля разрешить только определения и инициализацию переменных и функций,
         * тогда отвалятся простые приложения без главной функции (скриптовые приложения)
         * 
         * Если разрешить скриптовые приложения (код и определение функций вперемежку),
         * тогда сложно анализировать и сложнее отлажиать, 
         * так как не полуится сгенерировать итоговый код на C++ один в один.
         * 
         * 
         */
        static bool LLVMInitialize() {

            if (LLVMInitializeNativeTarget() || LLVMInitializeNativeAsmParser() || LLVMInitializeNativeAsmPrinter()) {
                LOG_RUNTIME("Fail LLVM initialize!");
            }
            return true;
        }

        static bool LLVMInitializeJIT() {
            LLVMInitializeAllTargetInfos();
            LLVMInitializeAllTargets();
            LLVMInitializeAllTargetMCs();
            LLVMInitializeAllAsmParsers();
            LLVMInitializeAllAsmPrinters();
            return true;
        }

        TermPtr MakeAst(const std::string_view src, bool skip_analize = false);
        ParserPtr GetParser();


        // Выполняет одну строку в контексте главного модуля программы
        // При первом вызове в m_main_ast создается AST как BLOCK
        // При повторнвых вызовах в m_main_ast->m_block добавляется новая строка
        ObjPtr Run(const std::string_view str, Obj* args = nullptr);

        // Выполняет скопилированное AST как главный модуль программы
        // При повторном вызове m_main_ast заменяется на новый AST
        //        ObjPtr Run(TermPtr ast, Obj* args = nullptr);
        ObjPtr Run(Module *module, Obj* args = nullptr);
        ObjPtr RunFile(std::string file, Obj* args = nullptr);

        bool ModuleCreate(FileModule &data, const std::string_view source);
        static bool ModuleCreate(FileModule &data, const std::string_view module_name, const TermPtr &include, const std::string_view source, llvm::Module *bc = nullptr);

        std::string MakeCodeModule(const TermPtr &term, std::string_view name, bool is_main);
        std::string MakeCodeFunction(const TermPtr &term);
        std::string MakeFuncDeclarations_(const TermPtr &term);
        std::string MakeCodeRepl(const std::string_view source, const std::string_view name);

        void MakeFunctionRecursive_(const TermPtr &term, std::string &output, const std::string_view module);

        std::unique_ptr<llvm::Module> MakeLLVMModule(std::string_view source, const std::vector<std::string> opts, std::string temp_dir = "");
        bool MakeObjFile(const std::string_view filename, llvm::Module &module, const std::vector<std::string> opts);
        bool LinkObjToExec(const std::string_view filename, std::vector<std::string> objs, std::vector<std::string> opts = {});

        static std::string ExtractFunctionDecls(const TermPtr &ast, const std::string_view module);
        static std::string ExtractStaticVars(const TermPtr &ast, const std::string_view module);
        static std::string MakeBodyFunction(const TermPtr &ast);
        static std::string MakeFunctionPrototype(const TermPtr &func, const std::string_view module);
        //        static std::string MakeFunctionArgs(const TermPtr &args);
        static std::string RegExpInlineComment(const std::string_view src);
        static std::string MakeCommentPlace(const TermPtr &term);


        static std::string ReplaceObjectInEmbedSource(const std::string_view embed, std::vector<std::string> &include, const TermPtr args = nullptr);
        bool MakeCppExec(const std::string_view source, const std::string_view filename, std::vector<std::string> opts = {});

        std::string MakeApplicationSource(const TermPtr &ast);

        static std::string MakeMain(const std::vector<std::string> &include);
        static std::string MakeMainEmbed(const std::string_view embed_source, const std::vector<std::string> &include);

        static TermPtr MainArgs();

        //
        //
        //        ObjPtr EvalCreate_(TermPtr & op);
        //        ObjPtr AssignVars_(ArrayTermType &l_vars, const TermPtr &r_term, bool is_pure);
        //
        //        ObjPtr GetIndexValue(TermPtr &term, ObjPtr &obj, Context * runner);
        //        ObjPtr GetFieldValue(TermPtr &term, ObjPtr &value, Context * runner);
        //        ObjPtr SetIndexValue(TermPtr &term, ObjPtr &value, Context * runner);
        //        ObjPtr SetFieldValue(TermPtr &term, ObjPtr &value, Context * runner);

        std::vector<std::string> m_includes;


    };


    struct CompileInfo;

    struct CompileInfo {
        //        BuiltInTorchDirect * m_builtin_direct;

        CompileInfo(RuntimePtr rt) {
            indent = 0;
            if (rt) {
                //            rt->ReadBuiltInProto(builtin);
            }

            //            m_builtin_direct = new BuiltInTorchDirect();
        }

        virtual ~CompileInfo() {

            //            delete m_builtin_direct;
        }

        //        ProtoType arguments; //< Аргументы функции
        //        ProtoType functions; //< Локальные функции
        //        ProtoType variables; //< Локальные переменные
        //        ProtoType consts; //< Локальные константы
        //        ProtoType builtin;
        std::set<std::string> iterators; //< Текущие итреаторы

        std::string last_type; //< Тип последней операции

        TermPtr isFunction(TermPtr term);
        TermPtr isArgument(TermPtr term);
        TermPtr isVariable(TermPtr term);
        TermPtr findObject(std::string name);

        bool isLocalAccess(TermPtr term);

        size_t indent;

        std::string GetIndent(int64_t offset = 0);

        class Indent {
        public:

            Indent(CompileInfo &i) : info(i) {

                info.indent++;
            }

            ~Indent() {

                info.indent--;
            }
            CompileInfo &info;
        };

        Indent NewIndent() {

            return Indent(*this);
        }

        //    BuiltInInfo m_builtin_info;
        //private:
        //    Context * m_ctx;
    };

    class Compiler {
    public:

        //        ObjPtr Run(const char *source, void *context);
        //
        //        ObjPtr RunFile(const char *filename) {
        //
        //            return Obj::CreateNone();
        //        }
        //
        //        void Free();

        /*
         *  
         */
        static bool MakeFunctionCpp(CompileInfo &ci, std::string func_name, TermPtr &func_define, std::ostream &out);
        static bool MakeCppFile(TermPtr list, std::ostream &out, const char *source = nullptr, Context *ctx = nullptr);

        // Преобразуют последовательность команд в эквивалентный код на С++ и возвращают результат выполнения последенй команды (или nullptr если не применимо)
        static std::string MakeFunctionBodyCpp(CompileInfo &ci, TermPtr ast);
        static std::string MakeSequenceOpsCpp(CompileInfo &ci, TermPtr elem, bool top_level);
        // Преобразуют один термин в эквивалентый код на C++
        static std::string GetImpl(CompileInfo &ci, TermPtr term, std::string &output);
        static std::string BeginIterators(CompileInfo &ci, TermPtr args, std::string &output, std::vector<std::string> &iters);

        static void ReplaceSourceVariable(CompileInfo &ci, size_t count, std::string &body);

        static void MakeCppFileFunctions(CompileInfo &ci, TermPtr elem, std::ostream &out);
        static void MakeCppFileConstants(CompileInfo &ci, TermPtr elem, std::ostream &out);
        static void MakeCppFileCallTerm(CompileInfo &ci, TermPtr ast, std::ostream &out, size_t deep);
        static std::string MakeCppFileCallArgs(CompileInfo &ci, TermPtr &args, TermPtr proto);
        static std::string MakeIteratorCallArgs_(CompileInfo &ci, TermPtr args, std::vector<std::string> &iters);
        static std::string MakeCppFileVariable(CompileInfo &ci, TermPtr &args, std::ostream &out);


        static bool Execute(const char *exec, std::string *out = nullptr, int *exit_code = nullptr);
        static bool GccMakeModule(const char * file, const char * module, const char * opts = nullptr, std::string *out = nullptr, int *exit_code = nullptr);

        static std::string EncodeNonAsciiCharacters(const char * text);


        static bool CompileModule(const char *filename, const char *output);
        //    static ObjPtr ExecModule(const char *module, const char *output, bool cached, Context *ctx);

        //    std::string MakeFunctionsSourceForJIT(TermPtr ast, Context *ctx);
        //    llvm::ExecutionEngine * JITCompileCPP(const char* source, const char* file_name = nullptr);

        //    bool LoadModule(const char *name, bool init = true, Context *ctx = nullptr, const char *module_name = nullptr);
        //    bool UnLoadModule(const char *name = nullptr, bool deinit = true);

        //    bool SaveContext(const char *name);
        //    static bool LoadContext(Context *ctx, const char *name);


        //    static ObjPtr Eval(Context *ctx, const char *text, bool make_function = false);
        //    static ObjPtr Eval(Context *ctx, TermPtr calc, bool make_function = false);
        static ObjPtr GetIndexField(Context *ctx, ObjPtr obj, TermPtr term, bool create_field = false);

        static bool ExecFunc(FunctionType * func, Obj *def_arg, Context *ctx, Obj &in, ObjPtr & out);

        static bool StringTemplate(std::string &format, Context * ctx);

        Compiler(RuntimePtr rt);

        virtual ~Compiler() {

        }

        static void WriteIncludeFiles_(std::ostream & out) {

            out << "#include \"pch.h\"\n";
            out << "#include \"newlang.h\"\n";
            out << "#include \"builtin.h\"\n";
            out << "\n\n";
            out << "using namespace " NEWLANG_NS ";\n";
            out << "\n\n";
        }

        static void WriteDeclarationFunctions_(CompileInfo &ci, TermPtr &func, std::ostream & out, std::vector<std::string> &func_list);

        enum FunctionStep {
            PREPARE = 1,
            OPERATION = 2,
            COMPLETE = 3
        };

        typedef std::string WriteFunctionOp(CompileInfo &ci, TermPtr &op, FunctionStep step);

        static std::string WriteFunctionCheckOp_(CompileInfo &ci, TermPtr &op, const char *check_true, const char *check_false);

        //        static std::string WriteSimpleBodyAND_(CompileInfo &ci, TermPtr &op, FunctionStep step = FunctionStep::OPERATION) {
        //            auto indent = ci.NewIndent();
        //            switch (step) {
        //                case FunctionStep::PREPARE:
        //                    return "";
        //                case FunctionStep::COMPLETE:
        //                    return ci.GetIndent() + "return " NEWLANG_NS "::Obj::Yes();\n";
        //
        //                case FunctionStep::OPERATION:
        //                    return WriteFunctionCheckOp_(ci, op, "", "return " NEWLANG_NS "::Obj::No()");
        //            }
        //            LOG_RUNTIME("Unknown function step %d", (int) step);
        //        }
        //
        //        static std::string WriteSimpleBodyOR_(CompileInfo &ci, TermPtr &op, FunctionStep step = FunctionStep::OPERATION) {
        //            auto indent = ci.NewIndent();
        //            switch (step) {
        //                case FunctionStep::PREPARE:
        //                    return "";
        //                case FunctionStep::COMPLETE:
        //                    return ci.GetIndent() + "return " NEWLANG_NS "::Obj::No();\n";
        //
        //                case FunctionStep::OPERATION:
        //                    return WriteFunctionCheckOp_(ci, op, "return " NEWLANG_NS "::Obj::Yes()", "");
        //            }
        //            LOG_RUNTIME("Unknown function step %d", (int) step);
        //        }
        //
        //        static std::string WriteSimpleBodyXOR_(CompileInfo &ci, TermPtr &op, FunctionStep step = FunctionStep::OPERATION) {
        //            auto indent = ci.NewIndent();
        //            switch (step) {
        //                case FunctionStep::PREPARE:
        //                    return ci.GetIndent() + "size_t xor_counter = 0;\n";
        //                case FunctionStep::COMPLETE:
        //                    // Результат равен 0, если нет операндов, равных 1, либо их чётное количество.
        //                    return ci.GetIndent() + "return (xor_counter & 1) ? " NEWLANG_NS "::Obj::Yes() : " NEWLANG_NS "::Obj::No();\n";
        //
        //                case FunctionStep::OPERATION:
        //                    return WriteFunctionCheckOp_(ci, op, "xor_counter++", "");
        //            }
        //            LOG_RUNTIME("Unknown function step %d", (int) step);
        //        }

        static std::string WriteFunctionBodyCLEAN_(CompileInfo &ci, TermPtr &op, FunctionStep step = FunctionStep::OPERATION) {
            auto indent = ci.NewIndent();
            switch (step) {
                case FunctionStep::PREPARE:
                    return "";
                case FunctionStep::COMPLETE:
                    return ci.GetIndent(-1) + "return nullptr; // default return from function\n";
                default: // FunctionStep::OPERATION
                    break;
            }
            std::string str;
            GetImpl(ci, op, str);

            return str + "\n";
        }

        static std::string WriteFunctionBodyOTHER_(CompileInfo &ci, TermPtr &op, FunctionStep step = FunctionStep::OPERATION) {
            auto indent = ci.NewIndent();
            switch (step) {
                case FunctionStep::PREPARE:
                    return WriteArgsForCall_(ci, op);
                case FunctionStep::COMPLETE:
                    return ci.GetIndent() + "return nullptr; // default return from function\n";
                default: // FunctionStep::OPERATION
                    break;
            }
            std::string str;
            GetImpl(ci, op, str);

            return str + ";\n";
        }

        static std::string WriteArgsForCall_(CompileInfo &ci, TermPtr & func);

        static void SelectTerms_(TermPtr &obj, std::vector<TermPtr> &terms);

        static bool WriteFunctionName_(TermPtr &func, std::ostream & out, bool is_transparent = false);

        static void ReplaceFuncArgs(TermPtr &func_define, std::string & str);

        static std::string MakeCommentLine(std::string comment);

        static std::string WriteSimpleBody_(CompileInfo &ci, TermPtr &func);


    public:
        //    cppjit::CppJIT *m_jit;

    protected:

        RuntimePtr m_runtime;

    };

}


#endif //INCLUDED_JIT_H_
