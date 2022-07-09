#pragma once
#ifndef INCLUDED_NEWLANG_CPP_
#define INCLUDED_NEWLANG_CPP_

#include "pch.h"
#include "term.h"
#include "builtin.h"

#include <types.h>
#include <object.h>
#include <context.h>
#include <parser.h>
//#include <contrib/cpp-jit-llvm/cpp_jit.h>

namespace newlang {

#define NEWLANG_INDENT_OP   "  "
#define NEWLANG_PREFIX      "newlang"
#define NEWLANG_NS          "newlang"

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

    class Context;
    struct CompileInfo;

    class Module {
    public:

        typedef std::map<std::string, ObjPtr> FuncMap;
        typedef
#ifdef _MSC_VER
        HMODULE
#else
        void *
#endif
        HandleType;

        Module(const char * name = nullptr) {
            m_handle = nullptr;
            if (!Load(name)) {
                LOG_RUNTIME("Can`t load %s", name);
            }
        }

        bool Load(const char *name) {
            m_name = name;
#ifdef _MSC_VER
            std::wstring win_name = utf8_decode(name);
            m_handle = LoadLibrary(win_name.c_str());
#else
            m_handle = dlopen(name, RTLD_LAZY | RTLD_LOCAL); // Для GCC нужно собирать с --no-gnu-unique для замены модулей в рантайме
#endif
            return m_handle;
        }

        virtual ~Module() {
            //Если динамическая библиотека экпортировала функцию, названную _fini, то эта функция вызывается перед выгрузкой библиотеки.  
            if (m_handle) {
#ifdef _MSC_VER
                FreeLibrary(m_handle);
#else
                dlclose(m_handle);
#endif
            }
            m_handle = nullptr;
        }

        ObjPtr Main(Context *ctx, Obj &args) {
            if (m_main) {
                return (*m_main)(ctx, args);
            }
            LOG_DEBUG("Main function in module '%s' not found!", m_name.c_str());
            return Obj::CreateNone();
        }

        inline FuncMap& Funcs() {
            return m_funcs;
        }

        const char **m_source;
        FunctionType *m_main;

        inline HandleType GetHandle() {
            return m_handle;
        }

        SCOPE(private) :
        HandleType m_handle;
        std::string m_name;
        FuncMap m_funcs;
    };


    //typedef std::map<std::string, const TermPtr> ProtoType;

    class RunTime {
    public:

        static RuntimePtr Init(int argc = 0, const char** argv = nullptr, bool ignore_error = true) {
            RuntimePtr rt = std::make_shared<RunTime>();

            std::string ffi_file;

#ifdef _MSC_VER

            std::wstring sys_file;
            std::string sys_init;

//#define CYGWIN
#ifdef CYGWIN
            sys_file = L"cygwin1.dll";
            sys_init = "cygwin_dll_init";
            ffi_file = "cygffi-6.dll";
#else
            sys_file = L"msys-2.0.dll";
            sys_init = "msys_dll_init";
            ffi_file = "libffi-7.dll";
#endif

            rt->m_msys = LoadLibrary(sys_file.c_str());
            if (!rt->m_msys) {
                LOG_RUNTIME("Fail LoadLibrary %s: %s", sys_file.c_str(), RunTime::GetLastErrorMessage().c_str());
            }

            typedef void init_type();

            init_type *init = (init_type *) GetProcAddress((HMODULE) rt->m_msys, sys_init.c_str());
            if (rt->m_msys && !init) {
                FreeLibrary((HMODULE) rt->m_msys);
                LOG_RUNTIME("Func %s not found! %s", sys_init.c_str(), RunTime::GetLastErrorMessage().c_str());
                (*init)();
            }
            rt->m_ffi_handle = LoadLibrary(utf8_decode(ffi_file).c_str());
#else
            ffi_file = "libffi.so";
            rt->m_ffi_handle = dlopen(ffi_file.c_str(), RTLD_NOW);
#endif

            if (!rt->m_ffi_handle) {
                LOG_RUNTIME("Fail load %s!", ffi_file.c_str());
            }

            rt->m_ffi_type_void = static_cast<ffi_type *> (GetDirectAddressFromLibrary(rt->m_ffi_handle, "ffi_type_void"));
            rt->m_ffi_type_uint8 = static_cast<ffi_type *> (GetDirectAddressFromLibrary(rt->m_ffi_handle, "ffi_type_uint8"));
            rt->m_ffi_type_sint8 = static_cast<ffi_type *> (GetDirectAddressFromLibrary(rt->m_ffi_handle, "ffi_type_sint8"));
            rt->m_ffi_type_uint16 = static_cast<ffi_type *> (GetDirectAddressFromLibrary(rt->m_ffi_handle, "ffi_type_uint16"));
            rt->m_ffi_type_sint16 = static_cast<ffi_type *> (GetDirectAddressFromLibrary(rt->m_ffi_handle, "ffi_type_sint16"));
            rt->m_ffi_type_uint32 = static_cast<ffi_type *> (GetDirectAddressFromLibrary(rt->m_ffi_handle, "ffi_type_uint32"));
            rt->m_ffi_type_sint32 = static_cast<ffi_type *> (GetDirectAddressFromLibrary(rt->m_ffi_handle, "ffi_type_sint32"));
            rt->m_ffi_type_uint64 = static_cast<ffi_type *> (GetDirectAddressFromLibrary(rt->m_ffi_handle, "ffi_type_uint64"));
            rt->m_ffi_type_sint64 = static_cast<ffi_type *> (GetDirectAddressFromLibrary(rt->m_ffi_handle, "ffi_type_sint64"));
            rt->m_ffi_type_float = static_cast<ffi_type *> (GetDirectAddressFromLibrary(rt->m_ffi_handle, "ffi_type_float"));
            rt->m_ffi_type_double = static_cast<ffi_type *> (GetDirectAddressFromLibrary(rt->m_ffi_handle, "ffi_type_double"));
            rt->m_ffi_type_pointer = static_cast<ffi_type *> (GetDirectAddressFromLibrary(rt->m_ffi_handle, "ffi_type_pointer"));

            rt->m_ffi_prep_cif = reinterpret_cast<ffi_prep_cif_type *> (GetDirectAddressFromLibrary(rt->m_ffi_handle, "ffi_prep_cif"));
            rt->m_ffi_prep_cif_var = reinterpret_cast<ffi_prep_cif_var_type *> (GetDirectAddressFromLibrary(rt->m_ffi_handle, "ffi_prep_cif_var"));
            rt->m_ffi_call = reinterpret_cast<ffi_call_type *> (GetDirectAddressFromLibrary(rt->m_ffi_handle, "ffi_call"));

            if (!(rt->m_ffi_type_uint8 && rt->m_ffi_type_sint8 && rt->m_ffi_type_uint16 && rt->m_ffi_type_sint16 &&
                    rt->m_ffi_type_uint32 && rt->m_ffi_type_sint32 && rt->m_ffi_type_uint64 && rt->m_ffi_type_sint64 &&
                    rt->m_ffi_type_float && rt->m_ffi_type_double && rt->m_ffi_type_pointer && rt->m_ffi_type_void &&
                    rt->m_ffi_prep_cif && rt->m_ffi_prep_cif_var && rt->m_ffi_call)) {
                LOG_RUNTIME("Fail init data from %s!", ffi_file.c_str());
            }

            return std::move(rt);
        }

        inline static void * GetDirectAddressFromLibrary(void *handle, const char * name) {
#ifdef _MSC_VER
            return static_cast<void *> (::GetProcAddress((HMODULE) handle, name));
#else
            return dlsym(handle, name);
#endif
        }

        //    void ReadBuiltInProto(ProtoType &proto);

        bool LoadModule(const char *name, bool init = true, Context *ctx = nullptr, const char *module_name = nullptr);
        bool UnLoadModule(Context *ctx, const char *name = nullptr, bool deinit = true);
        ObjPtr ExecModule(const char *module, const char *output, bool cached, Context * ctx);

        void * GetNativeAddr(const char * name, const char *module = nullptr);

        virtual ~RunTime() {

#ifdef _MSC_VER
            if (m_ffi_handle) {
                FreeLibrary((HMODULE) m_ffi_handle);
                m_ffi_handle = nullptr;
            }
            if (m_msys) {
                FreeLibrary((HMODULE) m_msys);
                m_msys = nullptr;
            }
#else
            if (m_ffi_handle) {
                dlclose(m_ffi_handle);
                m_ffi_handle = nullptr;
            }
#endif
        }

        RunTime() {
        }

        static std::string GetLastErrorMessage();

        typedef ffi_status ffi_prep_cif_type(ffi_cif *cif, ffi_abi abi, unsigned int nargs, ffi_type *rtype, ffi_type **atypes);
        typedef ffi_status ffi_prep_cif_var_type(ffi_cif *cif, ffi_abi abi, unsigned int nfixedargs, unsigned int ntotalargs, ffi_type *rtype, ffi_type **atypes);
        typedef void ffi_call_type(ffi_cif *cif, void (*fn)(void), void *rvalue, void **avalue);

    protected:

        //SCOPE(private) :
    public:
        RunTime(const RunTime&) = delete;
        const RunTime& operator=(const RunTime&) = delete;

#ifdef _MSC_VER
        void * m_msys;
#endif

        std::map<std::string, Module *> m_modules;

        void * m_ffi_handle;
        ffi_type * m_ffi_type_void;
        ffi_type * m_ffi_type_uint8;
        ffi_type * m_ffi_type_sint8;
        ffi_type * m_ffi_type_uint16;
        ffi_type * m_ffi_type_sint16;
        ffi_type * m_ffi_type_uint32;
        ffi_type * m_ffi_type_sint32;
        ffi_type * m_ffi_type_uint64;
        ffi_type * m_ffi_type_sint64;
        ffi_type * m_ffi_type_float;
        ffi_type * m_ffi_type_double;
        ffi_type * m_ffi_type_pointer;

        ffi_prep_cif_type *m_ffi_prep_cif;
        ffi_prep_cif_var_type * m_ffi_prep_cif_var;
        ffi_call_type * m_ffi_call;

    };

    struct CompileInfo {
        BuiltInTorchDirect * m_builtin_direct;

        CompileInfo(RuntimePtr rt) {
            indent = 0;
            if (rt) {
                //            rt->ReadBuiltInProto(builtin);
            }

            m_builtin_direct = new BuiltInTorchDirect();
        }

        virtual ~CompileInfo() {

            delete m_builtin_direct;
        }

        ProtoType arguments; //< Аргументы функции
        ProtoType functions; //< Локальные функции
        ProtoType variables; //< Локальные переменные
        ProtoType consts; //< Локальные константы
        ProtoType builtin;
        std::set<std::string> iterators; //< Текущие итреаторы

        std::string last_type; //< Тип последней операции

        TermPtr isFunction(TermPtr term);
        TermPtr isArgument(TermPtr term);
        TermPtr isVariable(TermPtr term);
        TermPtr findObject(std::string name);

        bool isLocalAccess(TermPtr term);

        size_t indent;

        std::string GetIndent(int64_t offset = 0) {

            return repeat(std::string(NEWLANG_INDENT_OP), indent + offset);
        }

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

    class NewLang : public Variable<Obj> {
    public:

        ObjPtr Run(const char *source, void *context);

        ObjPtr RunFile(const char *filename) {

            return Obj::CreateNone();
        }

        void Free();


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

        NewLang(RuntimePtr rt);

        virtual ~NewLang() {

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

        static std::string WriteSimpleBodyAND_(CompileInfo &ci, TermPtr &op, FunctionStep step = FunctionStep::OPERATION) {
            auto indent = ci.NewIndent();
            switch (step) {
                case FunctionStep::PREPARE:
                    return "";
                case FunctionStep::COMPLETE:
                    return ci.GetIndent() + "return " NEWLANG_NS "::Obj::Yes();\n";

                case FunctionStep::OPERATION:
                    return WriteFunctionCheckOp_(ci, op, "", "return " NEWLANG_NS "::Obj::No()");
            }
            LOG_RUNTIME("Unknown function step %d", (int) step);
        }

        static std::string WriteSimpleBodyOR_(CompileInfo &ci, TermPtr &op, FunctionStep step = FunctionStep::OPERATION) {
            auto indent = ci.NewIndent();
            switch (step) {
                case FunctionStep::PREPARE:
                    return "";
                case FunctionStep::COMPLETE:
                    return ci.GetIndent() + "return " NEWLANG_NS "::Obj::No();\n";

                case FunctionStep::OPERATION:
                    return WriteFunctionCheckOp_(ci, op, "return " NEWLANG_NS "::Obj::Yes()", "");
            }
            LOG_RUNTIME("Unknown function step %d", (int) step);
        }

        static std::string WriteSimpleBodyXOR_(CompileInfo &ci, TermPtr &op, FunctionStep step = FunctionStep::OPERATION) {
            auto indent = ci.NewIndent();
            switch (step) {
                case FunctionStep::PREPARE:
                    return ci.GetIndent() + "size_t xor_counter = 0;\n";
                case FunctionStep::COMPLETE:
                    // Результат равен 0, если нет операндов, равных 1, либо их чётное количество.
                    return ci.GetIndent() + "return (xor_counter & 1) ? " NEWLANG_NS "::Obj::Yes() : " NEWLANG_NS "::Obj::No();\n";

                case FunctionStep::OPERATION:
                    return WriteFunctionCheckOp_(ci, op, "xor_counter++", "");
            }
            LOG_RUNTIME("Unknown function step %d", (int) step);
        }

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


#endif //INCLUDED_NEWLANG_CPP_
