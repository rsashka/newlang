#pragma once
#ifndef INCLUDED_NEWLANG_CONTEXT_
#define INCLUDED_NEWLANG_CONTEXT_

#include "pch.h"

#include <ffi.h>

#include <term.h>
#include <object.h>

namespace newlang {


    std::string GetFileExt(const char * str);
    std::string AddDefaultFileExt(const char * str, const char *ext_default);
    std::string ReplaceFileExt(const char * str, const char *ext_old, const char *ext_new);
    std::string ReadFile(const char *fileName);

    bool Tranliterate(const wchar_t c, std::wstring &str);
    std::string MangleName(const char * name);

    std::string MangaledFuncCPP(const char *name, const char *space = nullptr);
    std::string MangaledFunc(const std::string name);

    inline std::string MakeName(std::string name) {
        if (!name.empty() && (name[0] == '$' || name[0] == '@' || name[0] == '%')) {
            return name.substr(1);
        }
        return name;
    }

    inline std::string MakeConstructorName(std::string name) {
        ASSERT(isType(name));
        std::string result(name.substr(1));
        result += ":";
        result += name.substr(name.rfind(":"));
        return result;
    }

    inline std::string MakeLocalName(std::string name) {
        return MangleName(MakeName(name).c_str());
    }

    inline std::string ExtractModuleName(const char *str) {
        std::string name(str);
        if (isModule(name)) {
            size_t pos = name.find("::");
            if (pos != std::string::npos) {
                return name.substr(0, pos);
            }
            return name;
        }
        return std::string();
    }

    inline std::string ExtractName(std::string name) {
        size_t pos = name.rfind("::");
        if (pos != std::string::npos) {
            name = name.substr(pos + 2);
        }
        if (isModule(name)) {
            return std::string();
        }
        return name;
    }

    inline std::string GetDoc(std::string name) {
        return "Help system not implemented!!!!!";
    }


    /*
     * Класс контекст предназначен для хранения контекста среды выполнения при вызове функций.
     * С его помощью передаются переменные среды окружения, параметры и аргументы приложения, входные и выходные параметры функций, 
     * текущие локальные и глобальные переменныеи, создание и доступ к итераторам и т.д.
     * 
     * 
     * 
     */

    typedef std::map<std::string, const TermPtr> ProtoType;


#define NL_OPS(_) \
        _("==", EQUAL) \
        _("===", ACCURATE) \
        _("!=", NE)\
        _("<", LT)\
        _(">", GT)\
        _("<=", LE)\
        _(">=", GE)\
        \
        _("&&", AND)\
        _("||", OR)\
        _("^^", XOR)\
        \
        _("&", BIT_AND)\
        _("|", BIT_OR)\
        _("^", BIT_XOR)\
        _("&=", BIT_AND_)\
        _("|=", BIT_OR_)\
        _("^=", BIT_XOR_)\
        \
        _("+", PLUS)\
        _("-", MINUS)\
        _("*", MUL)\
        _("/", DIV)\
        _("//", DIV_CEIL)\
        _("%", REM)\
        _("**", POW)\
        \
        _("+=", PLUS_)\
        _("-=", MINUS_)\
        _("*=", MUL_)\
        _("/=", DIV_)\
        _("//=", DIV_CEIL_)\
        _("%=", REM_)\
        _("**=", POW_)\
        \
        _("++", CONCAT)\
/*        _("--", DEC_)\*/\
        \
        _("~", TYPE_EQ)\
        _("~~", TYPE_EQ2)\
        _("~~~", TYPE_EQ3)\
        _("!~", TYPE_NE)\
        _("!~~", TYPE_NE2)\
        _("!~~~", TYPE_NE3)\
        \
        _(">>", RSHIFT)\
        _("<<", LSHIFT)\
        _(">>=", RSHIFT_)\
        _("<<=", LSHIFT_)\
        _(">>>", RRSHIFT)\
        _(">>>=", RRSHIFT_)\
        _("<=>", SPACESHIP)

#define NL_BUILTIN(_) \
        _("export", NOT_SUPPORT)\
        _("local", NOT_SUPPORT)

    class Module : public Obj {
    public:
        std::string m_file;
        std::string m_source;
        std::string m_md5;
        std::string m_timestamp;
        std::string m_version;
        bool m_is_main;

    public:

        Module() : Obj(ObjType::Module) {
            m_var_is_init = true;

        }

        bool Load(Context & ctx, const char * path, bool is_main) {
            m_is_main = is_main;
            m_file = path;
            m_var_name = ExtractModuleName(path);
            auto file = llvm::sys::fs::openNativeFileForRead(path);
            if (!file) {
                LOG_ERROR("Error open module '%s' from file %s!", m_var_name.c_str(), path);
                return false;
            }


            char buffer[llvm::sys::fs::DefaultReadChunkSize];
            llvm::MutableArrayRef<char> Buf(buffer, llvm::sys::fs::DefaultReadChunkSize);

            llvm::Expected<size_t> readed(0);
            do {
                readed = llvm::sys::fs::readNativeFile(*file, Buf);
                if (!readed) {
                    LOG_ERROR("Error read module '%s' from file %s!", m_var_name.c_str(), path);
                    return false;
                }
                m_source.append(Buf.data(), *readed);
            } while (*readed);

            llvm::sys::fs::file_status fs;
            std::error_code ec = llvm::sys::fs::status(*file, fs);
            if (ec) {
                m_timestamp = "??? ??? ?? ??:??:?? ????";
            } else {
                //                auto tp = fs.getLastModificationTime();
                time_t temp = llvm::sys::toTimeT(fs.getLastModificationTime());
                struct tm * timeinfo;
                timeinfo = localtime(&temp);
                m_timestamp = asctime(timeinfo);
            }

            llvm::ErrorOr<llvm::MD5::MD5Result> md5 = llvm::sys::fs::md5_contents((int)*file);
            if (!md5) {
                m_md5 = "????????????????????????????????";
            } else {
                llvm::SmallString<32> hash;
                llvm::MD5::stringifyResult((*md5), hash);
                m_md5 = hash.c_str();
            }
            llvm::sys::fs::closeFile(*file);

            m_var_is_init = true;
            return true;
        }

        virtual ~Module() {

        }
    };

    class Context : public Variable<Obj, std::weak_ptr<Obj> >, public std::enable_shared_from_this<Context> {
    public:

        const char * SYS__DESTRUCTOR__ = "_____";
        //        const char * SYS__ATTACHE__ = "__attache__";
        //        const char * SYS__DETACHE__ = "__detache__";


        static ObjPtr eval_END(Context *ctx, const TermPtr & term, Obj * args, bool eval_block = false);
        static ObjPtr func_NOT_SUPPORT(Context *ctx, const TermPtr & term, Obj * args, bool eval_block = false);

        enum class CatchType {
            CATCH_AUTO = 0,
            CATCH_NONE, // {   }
            CATCH_PLUS, // {+   +}
            CATCH_MINUS, // {-   -}
            CATCH_ALL, // {*   *}
        };

        enum class CreateMode {
            CREATE_ONLY,
            CREATE_AUTO,
            ASSIGN_ONLY,
        };
        static ObjPtr CREATE_OR_ASSIGN(Context *ctx, const TermPtr & term, Obj *args, CreateMode mode);

#define DEFINE_CASE(name) \
    static ObjPtr eval_ ## name(Context *ctx, const TermPtr &term, Obj *args, bool eval_block = false);

        NL_TERMS(DEFINE_CASE);

#undef DEFINE_CASE

#define PROTO_OP(_, func) \
    static ObjPtr op_ ## func(Context *ctx, const TermPtr &term, Obj *args, bool eval_block = false);

        NL_OPS(PROTO_OP);

#undef PROTO_OP


        typedef ObjPtr(*EvalFunction)(Context *ctx, const TermPtr & term, Obj * args, bool eval_block);

        static std::map<std::string, Context::EvalFunction> m_ops;
        static std::map<std::string, Context::EvalFunction> m_builtin_calls;
//        static Parser::MacrosStore m_macros; ///< Хотя макросы и могут обработываться в рантайме, но доступны они только для парсера
        static MacroBuffer m_macros;

        LLVMBuilderRef m_llvm_builder;

        std::map<std::string, std::shared_ptr<Module>> m_modules;

        static std::vector<std::string> SplitString(const char * str, const char *delim) {

            std::vector<std::string> result;
            std::string s(str);

            size_t pos;
            s.erase(0, s.find_first_not_of(delim));
            while (!s.empty()) {
                pos = s.find(delim);
                if (pos == std::string::npos) {
                    result.push_back(s);
                    break;
                } else {
                    result.push_back(s.substr(0, pos));
                    s.erase(0, pos);
                }
                s.erase(0, s.find_first_not_of(delim));
            }
            return result;
        }

        bool CheckOrLoadModule(std::string name);

        static void Reset() {
            m_types.clear();
            m_funcs.clear();
            m_macros.clear();
            m_ops.clear();
            m_builtin_calls.clear();
            Docs::m_docs.clear();
        }

        void clear_() override {
            Variable::clear_();

            m_modules.clear();

            ASSERT(m_terms);
            m_terms->clear_();
            m_terms->m_var_is_init = true;
            m_terms->m_var_type_current = ObjType::Module;

            m_ns_stack.clear();
        }

        inline ObjPtr ExecFile(const std::string &filename, Obj *args = nullptr, CatchType int_catch = CatchType::CATCH_ALL) {
            std::string source = ReadFile(filename.c_str());
            if (source.empty()) {
                LOG_RUNTIME("Empty source or file '%s' not found!", filename.c_str());
            }
            return ExecStr(source, args, int_catch);
        }

        inline ObjPtr ExecStr(const std::string str, Obj *args = nullptr, CatchType int_catch = CatchType::CATCH_AUTO) {
            TermPtr exec = Parser::ParseString(str, &m_macros);
            ObjPtr temp;
            if (args == nullptr) {
                temp = Obj::CreateNone();
                args = temp.get();
            }
            return Eval(this, exec, args, true, int_catch);
        }

        static ObjPtr Eval(Context *ctx, TermPtr term, Obj *args, bool eval_block, CatchType int_catch = CatchType::CATCH_AUTO);

        static ObjPtr ExpandAssign(Context *ctx, TermPtr lvar, TermPtr rval, Obj *args, CreateMode mode);
        static ObjPtr ExpandCreate(Context *ctx, TermPtr lvar, TermPtr rval, Obj * args);

        Context(RuntimePtr global);


        static std::map<std::string, ObjPtr> m_types;
        typedef at::variant<ObjPtr, std::vector < ObjPtr> > FuncItem;
        static std::map<std::string, FuncItem> m_funcs; // Системный и встроенные функции 

        ObjPtr CreateClass(std::string class_name, TermPtr type, Obj *args);

        inline static ObjPtr CreateLVal(Context *ctx, TermPtr type) {
            Obj args;
            return CreateLVal(ctx, type, &args);
        }

        inline static ObjPtr CreateRVal(Context *ctx, TermPtr term, bool eval_block = true, CatchType int_catch = CatchType::CATCH_ALL) {
            Obj args;
            return CreateRVal(ctx, term, &args, eval_block, int_catch);
        }

        inline static ObjPtr CreateRVal(Context *ctx, const char *source, bool eval_block = true, CatchType int_catch = CatchType::CATCH_ALL) {
            Obj args;
            return CreateRVal(ctx, source, &args, eval_block, int_catch);
        }

        static ObjPtr CreateLVal(Context *ctx, TermPtr type, Obj * args);
        static ObjPtr CreateRVal(Context *ctx, TermPtr term, Obj *args, bool eval_block = true, CatchType int_catch = CatchType::CATCH_ALL);
        static ObjPtr CreateRVal(Context *ctx, const char *source, Obj *args, bool eval_block = true, CatchType int_catch = CatchType::CATCH_ALL);
        void CreateArgs_(ObjPtr &args, TermPtr &term, Obj * local_vars);

        static std::vector<Index> MakeIndex(Context *ctx, TermPtr term, Obj * local_vars);

        void ItemTensorEval_(torch::Tensor &tensor, c10::IntArrayRef shape, std::vector<Index> &ind, const int64_t pos, ObjPtr & obj, ObjPtr & args);
        void ItemTensorEval(torch::Tensor &tensor, ObjPtr obj, ObjPtr args);

        void ReadBuiltInProto(ProtoType & proto);

        bool CreateBuiltin(const char * prototype, void * func, ObjType type);
        ObjPtr RegisterObject(ObjPtr var);

        ObjPtr RemoveObject(const char * name) {
            std::string str(name);
            if (str.size() && (str[0] == '$' || str[0] == '@')) {
                str = str.substr(1);
            }
            auto found = find(name);
            if (found != end()) {
                ListType::erase(found);
                return Obj::Yes();
            }
            return Obj::No();
        }

        ObjPtr GetObject(const std::string name) {
            std::string str(name);
            if (str.size() && (str[0] == '$')) {
                str = str.substr(1);
            }
            auto found = find(str);
            if (found != end()) {
                return found->second.lock();
            }
            auto func = m_funcs.find(str);
            if (func != m_funcs.end()) {
                if (at::holds_alternative<ObjPtr>(func->second)) {
                    return at::get<ObjPtr>(func->second);
                }
                ASSERT(at::holds_alternative<std::vector < ObjPtr >> (func->second));
                return at::get<std::vector < ObjPtr >> (func->second)[0];
            }
            return nullptr;
        }

        RuntimePtr m_runtime; // Глобальный контекс, если к нему есть доступ

        std::shared_ptr<Module> m_main_module;
        Module * m_terms;
        std::vector<std::string> m_ns_stack;

        bool NamespasePush(const std::string &name) {
            if (name.empty()) {
                return false;
            }
            m_ns_stack.push_back(name);
            return true;
        }

        void NamespasePop() {
            ASSERT(!m_ns_stack.empty());
            m_ns_stack.pop_back();
        }

        std::string NamespaseFull(std::string name = "") {
            if (name.find("::") != 0) {
                for (size_t i = m_ns_stack.size(); i > 0; i--) {
                    if (!name.empty()) {
                        name.insert(0, "::");
                    }
                    if (m_ns_stack[i - 1].compare("::") == 0) {
                        if (name.empty()) {
                            name = "::";
                        }
                        break;
                    }
                    name.insert(0, m_ns_stack[i - 1]);
                    if (m_ns_stack[i - 1].find("::") == 0) {
                        break;
                    }
                }
            }
            //            if (!isFullName(name)) {
            //                name.insert(0, "::");
            //            }
            return name;
        }

        std::string Dump(const char *separator = "") {
            std::string result;
            for (int i = 0; i < size(); i++) {
                ObjPtr obj = at(i).second.lock();
                if (obj) {
                    result += obj->toString();
                } else {
                    result += at(i).first;
                    result += "=nullptr";
                }
                if (separator) {
                    result += separator;
                }
            }
            return result;
        }

        virtual ~Context() {
            LLVMDisposeBuilder(m_llvm_builder);
            m_llvm_builder = nullptr;
        }

        ObjPtr GetTerm(const std::string name, bool is_ref);
        ObjPtr FindTerm(const std::string name);
        ObjPtr FindSessionTerm(const char *name, bool current_only = false);
        void CleanUp();


        ObjPtr CreateSessionTerm(ObjPtr obj, const char *name);


        ObjPtr FindGlobalTerm(TermPtr term);

        ObjPtr FindGlobalTerm(const std::string name) {
            auto found = m_terms->find(MakeName(name));
            if (found != m_terms->end()) {
                return found->second;
            }
            return GetObject(name);
        }

        void RegisterInContext(ObjPtr & args) {
            RegisterInContext(*args);
        }

        void RegisterInContext(Obj & args) {
            for (int i = static_cast<int> (args.size()) - 1; args.size() && i >= 0; i--) {
                push_front(pair(args.at(i).second, args.at(i).first));
            }
        }

        void UnRegisterInContext(Obj & args) {
            for (int i = static_cast<int> (args.size()) - 1; args.size() && i >= 0; i--) {
                pop_front();
            }
        }

        static ObjPtr CallBlock(Context *ctx, const TermPtr &block, Obj *local_vars, bool eval_block, CatchType catch_type, bool *has_interrupt);

        static ObjPtr EvalBlockAND(Context *ctx, const TermPtr &block, Obj * local_vars);
        static ObjPtr EvalBlockOR(Context *ctx, const TermPtr &block, Obj * local_vars);
        static ObjPtr EvalBlockXOR(Context *ctx, const TermPtr &block, Obj * local_vars);

        ObjPtr CreateNative(const char *proto, const char *module = nullptr, bool lazzy = false, const char *mangle_name = nullptr);
        ObjPtr CreateNative(TermPtr proto, const char *module = nullptr, bool lazzy = false, const char *mangle_name = nullptr);
        ObjPtr CreateNative(Obj args);


        std::string ffi_file;

        typedef ffi_status ffi_prep_cif_type(ffi_cif *cif, ffi_abi abi, unsigned int nargs, ffi_type *rtype, ffi_type **atypes);
        typedef ffi_status ffi_prep_cif_var_type(ffi_cif *cif, ffi_abi abi, unsigned int nfixedargs, unsigned int ntotalargs, ffi_type *rtype, ffi_type **atypes);
        typedef void ffi_call_type(ffi_cif *cif, void (*fn)(void), void *rvalue, void **avalue);

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
        //        m_func_abi

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

        std::vector<std::wstring> SelectPredict(std::string start, size_t overage_count = 0) {

            std::vector<std::wstring> result;

            bool find_local = false;
            bool find_global = false;
            bool find_types = false;
            bool find_macro = false;

            std::string prefix;

            if (isModule(start)) {
                prefix = start[0];
                start = start.substr(1);
                find_global = true;
            } else if (isLocal(start)) {
                prefix = start[0];
                start = start.substr(1);
                find_local = true;
            } else if (isMacro(start)) {
                find_macro = true;
            } else if (isType(start)) {
                find_types = true;
            } else {
                find_local = true;
                find_global = true;
                find_types = true;
                find_macro = true;
            }


            if (find_macro) {
                for (auto &elem : m_macros) {
                    if (pred_compare(start, elem.first)) {
                        result.push_back(utf8_decode(prefix + elem.first));
                        if (result.size() > overage_count + 1) {
                            break;
                        }
                    }
                }
            }

            if (find_local) {
                for (int i = 0; i < size(); i++) {
                    if (pred_compare(start, at(i).first)) {
                        ObjPtr object = at(i).second.lock();
                        if (object && object->is_function_type()) {
                            result.push_back(utf8_decode(prefix + at(i).first) + L"(");
                        } else if (object) {
                            result.push_back(utf8_decode(prefix + at(i).first));
                        }
                        if (result.size() > overage_count + 1) {
                            break;
                        }
                    }
                }
            }

            if (find_global) {
                for (int i = 0; i < m_terms->size(); i++) {
                    if (pred_compare(start, m_terms->at(i).first)) {
                        if (m_terms->at(i).second->is_function_type()) {
                            result.push_back(utf8_decode(prefix + m_terms->at(i).first) + L"(");
                        } else {
                            result.push_back(utf8_decode(prefix + m_terms->at(i).first));
                        }
                        if (result.size() > overage_count + 1) {
                            break;
                        }
                    }
                }

                for (auto &elem : m_funcs) {

                    if (pred_compare(start, elem.first)) {
                        result.push_back(utf8_decode(prefix + elem.first) + L"(");
                        if (result.size() > overage_count + 1) {
                            break;
                        }
                    }
                }
            }

            if (find_types) {
                for (auto &elem : m_types) {
                    if (pred_compare(start, elem.first)) {
                        result.push_back(utf8_decode(elem.first));
                        if (result.size() > overage_count + 1) {
                            break;
                        }
                    }
                }
            }
            return result;

        }

        inline ObjPtr ConvertType(const ObjType type, const Dimension *dims, ObjPtr obj, const ObjPtr obj2 = nullptr) {
            ObjPtr result = obj->Clone();
            ConvertType_(type, dims, result, obj2);
            return result;
        }
        void ConvertType_(const ObjType type, const Dimension *dims, ObjPtr obj, const ObjPtr obj2 = nullptr);

        ObjPtr CreateConvertTypeFunc(const char *prototype, void *func, ObjType type) {
            ASSERT(prototype);
            ASSERT(func);

            std::string func_dump(prototype);
            func_dump += " := { };";

            TermPtr proto = Parser::ParseString(func_dump, &m_macros);
            ObjPtr obj =
                    Obj::CreateFunc(this, proto->Left(), type,
                    proto->Left()->getName().empty() ? proto->Left()->getText() : proto->Left()->getName());
            obj->m_var = func;

            return obj;
        }

        /**
         * Функция для организации встроенных типов в иерархию наследования.
         * Другие функции: CreateBaseType - создает базовые типы данных (для расширения классов требуется контекст)
         * и BaseTypeConstructor - функция обратного вызова при создании нового объекта базового типа данных
         * @param type - Базовый тип данных \ref ObjType
         * @param parents - Список сторок с именами родительских типов
         * @return - Успешность регистрации базовго типа в иерархии
         */
        bool RegisterTypeHierarchy(ObjType type, std::vector<std::string> parents) {
            //            std::array < std::string, sizeof...(parents) > list = {parents...};

            std::string type_name(toString(type));
            auto base = m_types.find(type_name);
            if (base != m_types.end()) {
                return false;
            }

            ObjPtr result = Obj::CreateBaseType(type);
            ASSERT(result->m_var_type_fixed == type);
            ASSERT(result->m_var_type_current == ObjType::Type);
            ASSERT(!type_name.empty() && result->m_class_name.compare(type_name) == 0);
            ASSERT(result->m_class_parents.empty());

            for (auto &parent : parents) {
                auto iter = m_types.find(parent);
                if (iter == m_types.end()) {
                    LOG_DEBUG("Parent type '%s' not found!", parent.c_str());
                    return false;
                }
                for (auto &elem : result->m_class_parents) {
                    ASSERT(elem);
                    if (!elem->m_class_name.empty() && elem->m_class_name.compare(parent) == 0) {
                        LOG_DEBUG("The type '%s' already exists in the parents of '%s'!", parent.c_str(), type_name.c_str());
                        return false;
                    }
                }
                ASSERT(iter->first.compare(parent) == 0);
                result->m_class_parents.push_back(iter->second);
            }
            m_types[type_name] = result;
            return true;
        }

        ObjType BaseTypeFromString(const std::string & type, bool *has_error = nullptr) {
            ObjPtr obj_type = GetTypeFromString(type, has_error);

            if (obj_type == nullptr) {
                if (has_error) {
                    *has_error = true;
                    return ObjType::None;
                }
                LOG_RUNTIME("Type name '%s' not found!", type.c_str());
            }
            return obj_type->m_var_type_fixed;
        }

        ObjPtr GetTypeFromString(const std::string & type, bool *has_error = nullptr) {
            if (type.empty()) {
                if (has_error) {
                    *has_error = true;
                    return Obj::CreateNone();
                }
                LOG_RUNTIME("Type name '%s' not found!", type.c_str());
            }

            auto result_types = m_types.find(type);
            if (result_types != m_types.end()) {
                return result_types->second;
            }

            auto result_terms = m_terms->find(type);
            if (result_terms != m_terms->end()) {
                return result_terms->second;
            }

            auto result_find = find(type);
            if (result_find != end()) {
                return result_find->second.lock();
            }

            if (has_error) {
                *has_error = true;
                return nullptr;
            }
            LOG_RUNTIME("Type name '%s' not found!", type.c_str());
        }

        enum class MatchMode {
            MatchEqual,
            MatchStrict,
            TYPE_NAME,
            TYPE_EQUAL,
            TYPE_STRICT,
        };
        static bool MatchCompare(Obj &match, ObjPtr &value, MatchMode mode, Context * ctx);
        static bool MatchEstimate(Obj &match, const TermPtr &match_item, MatchMode mode, Context *ctx, Obj * args);

        SCOPE(protected) :
        size_t GetCount();
        ObjPtr GetIndex(size_t index);

    private:

        Context(const Context&) = delete;
        const Context& operator=(const Context&) = delete;

    };

}
#endif //INCLUDED_NEWLANG_CONTEXT_
