#pragma once
#ifndef INCLUDED_NEWLANG_CONTEXT_
#define INCLUDED_NEWLANG_CONTEXT_

#include "pch.h"

#include <ffi.h>

#include <term.h>
#include <object.h>
#include <module.h>
#include <runtime.h>

namespace newlang {

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
        \
        _("+=", PLUS_)\
        _("-=", MINUS_)\
        _("*=", MUL_)\
        _("/=", DIV_)\
        _("//=", DIV_CEIL_)\
        _("%=", REM_)\
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


        //        typedef ObjPtr(*EvalFunction)(Context *ctx, const TermPtr & term, Obj * args, bool eval_block);

        static std::map<std::string, EvalFunction> m_ops;
        //        static std::map<std::string, Context::EvalFunction> m_builtin_calls;
        //        static Parser::MacrosStore m_macros; ///< Хотя макросы и могут обработываться в рантайме, но доступны они только для парсера

        static void Reset() {
            //            m_types.clear();
            //            m_funcs.clear();
            m_ops.clear();
            //            m_builtin_calls.clear();
            Docs::m_docs.clear();
        }

        void clear_() override;

        ObjPtr ExecFile(const std::string &filename, Obj *args = nullptr, CatchType int_catch = CatchType::CATCH_ALL);
        ObjPtr ExecStr(const std::string str, Obj *args = nullptr, CatchType int_catch = CatchType::CATCH_AUTO);

        static ObjPtr Eval(Context *ctx, TermPtr term, Obj *args, bool eval_block, CatchType int_catch = CatchType::CATCH_AUTO);

        static ObjPtr ExpandAssign(Context *ctx, TermPtr lvar, TermPtr rval, Obj *args, CreateMode mode);
        static ObjPtr ExpandCreate(Context *ctx, TermPtr lvar, TermPtr rval, Obj * args);

        Context(RuntimePtr global);


        //        static std::map<std::string, ObjPtr> m_types;
        //        typedef at::variant<ObjPtr, std::vector < ObjPtr> > FuncItem;
        //        static std::map<std::string, FuncItem> m_funcs; // Системный и встроенные функции 

        ObjPtr CreateClass(std::string class_name, TermPtr type, Obj *args);

        inline static ObjPtr CreateLVal(Context *ctx, TermPtr type) {
            Obj args;
            return CreateLVal(ctx, type, &args);
        }

        //        static ObjPtr Exec(Context *ctx, const char * cmd, ObjPtr opts);

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

        //        void ReadBuiltInProto(ProtoType & proto);

        //        bool CreateBuiltin(const char * prototype, void * func, ObjType type);
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

        ObjPtr GetObject(const std::string name);

        RuntimePtr m_runtime; // Глобальный контекс, если к нему есть доступ

        //        ModulePtr m_main_module;
        //        Module * m_terms;

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
        }

        ObjPtr GetTerm(const std::string name, bool is_ref);
        ObjPtr FindTerm(const std::string name);
        ObjPtr FindSessionTerm(const char *name, bool current_only = false);
        void CleanUp();


        ObjPtr CreateSessionTerm(ObjPtr obj, const char *name);


        //        ObjPtr FindGlobalTerm(TermPtr term);
        //        ObjPtr FindGlobalTerm(const std::string name);

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

        //        ObjPtr CreateNative(const char *proto, const char *module = nullptr, bool lazzy = false, const char *mangle_name = nullptr);
        //        ObjPtr CreateNative(TermPtr proto, const char *module = nullptr, bool lazzy = false, const char *mangle_name = nullptr);
        //        ObjPtr CreateNative(Obj args);


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

        //        static bool pred_compare(const std::string &find, const std::string &str) {
        //            size_t pos = 0;
        //            while (pos < find.size() && pos < str.size()) {
        //                if (find[pos] != str[pos]) {
        //                    return false;
        //                }
        //                pos++;
        //            }
        //            return find.empty() || (pos && find.size() == pos);
        //        }
        //
        //        std::vector<std::wstring> SelectPredict(std::wstring wstart, size_t overage_count = 0) {
        //            return SelectPredict(utf8_encode(wstart), overage_count);
        //        }
        //
        //        std::vector<std::wstring> SelectPredict(std::string start, size_t overage_count = 0) {
        //
        //            std::vector<std::wstring> result;
        //
        //            bool find_local = false;
        //            bool find_global = false;
        //            bool find_types = false;
        //            bool find_macro = false;
        //
        //            std::string prefix;
        //
        //            if (isModule(start)) {
        //                prefix = start[0];
        //                start = start.substr(1);
        //                find_global = true;
        //            } else if (isLocal(start)) {
        //                prefix = start[0];
        //                start = start.substr(1);
        //                find_local = true;
        //            } else if (isMacro(start)) {
        //                find_macro = true;
        //            } else if (isType(start)) {
        //                find_types = true;
        //            } else {
        //                find_local = true;
        //                find_global = true;
        //                find_types = true;
        //                find_macro = true;
        //            }
        //
        //
        //            if (find_macro) {
        //                for (auto &elem : *m_named) {
        //                    if (pred_compare(start, elem.first)) {
        //                        result.push_back(utf8_decode(prefix + elem.first));
        //                        if (result.size() > overage_count + 1) {
        //                            break;
        //                        }
        //                    }
        //                }
        //            }
        //
        //            if (find_local) {
        //                for (int i = 0; i < size(); i++) {
        //                    if (pred_compare(start, at(i).first)) {
        //                        ObjPtr object = at(i).second.lock();
        //                        if (object && object->is_function_type()) {
        //                            result.push_back(utf8_decode(prefix + at(i).first) + L"(");
        //                        } else if (object) {
        //                            result.push_back(utf8_decode(prefix + at(i).first));
        //                        }
        //                        if (result.size() > overage_count + 1) {
        //                            break;
        //                        }
        //                    }
        //                }
        //            }
        //
        //            if (find_global) {
        //                for (int i = 0; i < m_terms->size(); i++) {
        //                    if (pred_compare(start, m_terms->at(i).first)) {
        //                        if (m_terms->at(i).second->is_function_type()) {
        //                            result.push_back(utf8_decode(prefix + m_terms->at(i).first) + L"(");
        //                        } else {
        //                            result.push_back(utf8_decode(prefix + m_terms->at(i).first));
        //                        }
        //                        if (result.size() > overage_count + 1) {
        //                            break;
        //                        }
        //                    }
        //                }
        //
        //                for (auto &elem : m_named->m_funcs) {
        //
        //                    if (pred_compare(start, elem.first)) {
        //                        result.push_back(utf8_decode(prefix + elem.first) + L"(");
        //                        if (result.size() > overage_count + 1) {
        //                            break;
        //                        }
        //                    }
        //                }
        //            }
        //
        //            if (find_types) {
        //                for (auto &elem : m_named->m_types) {
        //                    if (pred_compare(start, elem.first)) {
        //                        result.push_back(utf8_decode(elem.first));
        //                        if (result.size() > overage_count + 1) {
        //                            break;
        //                        }
        //                    }
        //                }
        //            }
        //            return result;
        //
        //        }

        inline ObjPtr ConvertType(const ObjType type, const Dimension *dims, ObjPtr obj, const ObjPtr obj2 = nullptr) {
            ObjPtr result = obj->Clone();
            ConvertType_(type, dims, result, obj2);
            return result;
        }
        void ConvertType_(const ObjType type, const Dimension *dims, ObjPtr obj, const ObjPtr obj2 = nullptr);

        ObjPtr CreateConvertTypeFunc(const char *prototype, void *func, ObjType type);

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
