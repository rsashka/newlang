#pragma once
#ifndef INCLUDED_NEWLANG_CONTEXT_
#define INCLUDED_NEWLANG_CONTEXT_

#include "pch.h"

#include <core/term.h>
#include <core/object.h>

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

inline std::string MakeLocalName(std::string name) {
    return MangleName(MakeName(name).c_str());
}

/*
 * Класс контекст предназначен для хранения контекста среды выполнения при вызове функций.
 * С его помощью передаются переменные среды окружения, параметры и аргументы приложения, входные и выходные параметры функций, 
 * текущие локальные и глобальные переменныеи, создание и доступ к итераторам и т.д.
 * 
 * 
 * 
 */
class ContextCursor;

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

class Context : public Variable< std::weak_ptr<Object> > {
public:

    static ObjPtr eval_END(Context *ctx, const TermPtr & term, Object &args);
    static ObjPtr eval_UNKNOWN(Context *ctx, const TermPtr & term, Object &args);
    static ObjPtr func_NOT_SUPPORT(Context *ctx, const TermPtr & term, Object &args);

    enum class CreateMode {
        CREATE_ONLY,
        CREATE_OR_ASSIGN,
        ASSIGN_ONLY,
    };
    static ObjPtr CREATE_OR_ASSIGN(Context *ctx, const TermPtr & term, Object &args, CreateMode mode);

#define DEFINE_CASE(name) \
    static ObjPtr eval_ ## name(Context *ctx, const TermPtr &term, Object &args);

    NL_TERMS(DEFINE_CASE);

#undef DEFINE_CASE

#define PROTO_OP(_, func) \
    static ObjPtr op_ ## func(Context *ctx, const TermPtr &term, Object &args);

    NL_OPS(PROTO_OP);

#undef PROTO_OP


    typedef ObjPtr(*EvalFunction)(Context *ctx, const TermPtr & term, Object &args);

    static std::map<std::string, Context::EvalFunction> m_ops;
    static std::map<std::string, Context::EvalFunction> m_builtin_calls;

    static void Reset() {
        m_types.clear();
    }

    inline ObjPtr Eval(const std::string_view str) {
        Object args;
        return Eval(this, Parser::ParseString(str), args);
    }

    inline ObjPtr Exec(const std::string &filename) {
        std::string source = ReadFile(filename.c_str());
        if (source.empty()) {
            LOG_RUNTIME("Empty source or file '%s' not found!", filename.c_str());
        }
        return Eval(source);
    }

    inline ObjPtr Eval(const std::string_view str, Object & args) {
        return Eval(this, Parser::ParseString(str), args);
    }

    inline static ObjPtr Eval(Context *ctx, TermPtr term) {
        Object args;
        return Eval(ctx, term, args);
    }
    static ObjPtr Eval(Context *ctx, TermPtr term, Object &args);

    static ObjPtr ExpandAssign(Context *ctx, TermPtr lvar, TermPtr rval, Object &args, CreateMode mode);
    static ObjPtr ExpandCreate(Context *ctx, TermPtr lvar, TermPtr rval, Object &args);

    Context(RuntimePtr global);


    static std::map<std::string, ObjPtr> m_types;
    typedef std::variant<ObjPtr, std::vector<ObjPtr>> FuncItem;
    static std::map<std::string, FuncItem> m_funcs; // Системный и встроенные функции 

    inline static ObjPtr CreateLVal(Context *ctx, TermPtr type) {
        Object args;
        return CreateLVal(ctx, type, args);
    }

    inline static ObjPtr CreateRVal(Context *ctx, TermPtr term) {
        Object args;
        return CreateRVal(ctx, term, args);
    }

    inline static ObjPtr CreateRVal(Context *ctx, const char *source) {
        Object args;
        return CreateRVal(ctx, source, args);
    }

    static ObjPtr CreateLVal(Context *ctx, TermPtr type, Object &args);
    static ObjPtr CreateRVal(Context *ctx, TermPtr term, Object &args);
    static ObjPtr CreateRVal(Context *ctx, const char *source, Object &args);


    void ReadBuiltInProto(ProtoType & proto);

    ObjPtr CreateBuiltin(const char * prototype, void * func, ObjType type);
    ObjPtr RegisterObject(ObjPtr var);

    ObjPtr RemoveObject(const char * name) {
        std::string str(name);
        if (str.size() && (str[0] == '$' || str[0] == '@')) {
            str = str.substr(1);
        }
        auto found = select(name);
        if (!found.complete()) {
            erase(found);
            return Object::Yes();
        }
        return Object::No();
    }

    ObjPtr GetObject(const char *name) {
        std::string str(name);
        if (str.size() && (str[0] == '$' || str[0] == '@')) {
            str = str.substr(1);
        }
        auto found = select(str);
        if (!found.complete()) {
            return (*found).lock();
        }
        auto func = m_funcs.find(str);
        if (func != m_funcs.end()) {
            if (std::holds_alternative<ObjPtr>(func->second)) {
                return std::get<ObjPtr>(func->second);
            }
            ASSERT(std::holds_alternative<std::vector < ObjPtr >> (func->second));
            return std::get<std::vector < ObjPtr >> (func->second)[0];
        }
        return nullptr;
    }

    RuntimePtr m_runtime; // Глобальный контекс, если к нему есть доступ
    Variable<ObjPtr> m_global_terms;

    virtual ~Context() {
    }

#warning CONST !!!!!!!!!!!!!!!!!!!
    const ObjPtr GetConst(const char *name);

    ObjPtr GetTerm(const char *name, bool is_ref);
    ObjPtr FindTerm(const char *name);
    ObjPtr FindSessionTerm(const char *name, bool current_only = false);


    ObjPtr CreateSessionTerm(ObjPtr obj, const char *name);


    ObjPtr FindGlobalTerm(TermPtr term);

    ObjPtr FindGlobalTerm(const char *name) {
        auto found = m_global_terms.select(MakeName(name));
        if (!found.complete()) {

            return *found;
        }
        return nullptr;
    }

    void RegisterInContext(ObjPtr &args) {
        RegisterInContext(*args);
    }

    void RegisterInContext(Object &args) {
        for (int i = static_cast<int> (args.size()) - 1; args.size() && i >= 0; i--) {
            push_front(args.at(i).second, args.at(i).first);
        }
    }

    ObjPtr CallByName(const char * name) {
        Object args;
        return CallByName(name, args, false);
    }

    template <typename... T>
    typename std::enable_if<is_all<Object::PairType, T ...>::value, ObjPtr>::type
    CallByName(const char * name, T ... list) {
        Object args(list...);
        return CallByName(name, args, false);
    }

    ObjPtr CallByName(const TermPtr & term_args, Object &local_vars);
    ObjPtr CallByName(const char * name, Object &args, bool is_ref = false);

    static ObjPtr EvalBlock(Context *ctx, const TermPtr &block, Object &local_vars);


    ObjPtr CreateNative(const char *proto, const char *module = nullptr, bool lazzy = false, const char *mangle_name = nullptr, ffi_abi abi = FFI_DEFAULT_ABI);
    ObjPtr CreateNative(TermPtr proto, const char *module = nullptr, bool lazzy = false, const char *mangle_name = nullptr, ffi_abi abi = FFI_DEFAULT_ABI);
    ObjPtr CreateNative(Object args);

    static bool pred_compare(const std::string_view find, const std::string_view str) {
        size_t pos = 0;
        while (pos < find.size() && pos < str.size()) {
            if (find[pos] != str[pos]) {
                return false;
            }
            pos++;
        }
        return find.empty() || (pos && find.size() == pos);
    }

    std::vector<std::wstring> SelectPredict(std::wstring_view wstart, size_t overage_count = 0) {
        return SelectPredict(utf8_encode(wstart), overage_count);
    }

    std::vector<std::wstring> SelectPredict(std::string_view start, size_t overage_count = 0) {

        std::vector<std::wstring> result;

        bool find_local = false;
        bool find_global = false;
        bool find_types = false;

        std::string prefix;

        if (isGlobal(start)) {
            prefix = start[0];
            start.remove_prefix(1);
            find_global = true;
        } else if (isLocal(start)) {
            prefix = start[0];
            start.remove_prefix(1);
            find_local = true;
        } else if (isType(start)) {
            prefix = start[0];
            start.remove_prefix(1);
            find_types = true;
        } else {
            find_local = true;
            find_global = true;
            find_types = true;
        }


        if (find_local) {
            for (int i = 0; i < size(); i++) {
                if (pred_compare(start, at(i).first)) {
                    ObjPtr object = at(i).second.lock();
                    if (object && object->is_function()) {
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
            for (int i = 0; i < m_global_terms.size(); i++) {
                if (pred_compare(start, m_global_terms.at(i).first)) {
                    if (m_global_terms.at(i).second->is_function()) {
                        result.push_back(utf8_decode(prefix + m_global_terms.at(i).first) + L"(");
                    } else {
                        result.push_back(utf8_decode(prefix + m_global_terms.at(i).first));
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
                    //                    if (std::holds_alternative<ObjPtr>(elem)) {
                    //                        ObjPtr &obj = std::get<ObjPtr>(elem);
                    //                    } else {
                    //                        ASSERT(std::holds_alternative<std::vector < ObjPtr >> (elem));
                    //                        std::vector<ObjPtr> &objs = std::get<std::vector < ObjPtr >> (elem);
                    //                        for (auto &elem : objs) {
                    //                            result.push_back(utf8_decode(prefix + elem.first) + L"(");
                    //                            if (result.size() > overage_count + 1) {
                    //                                break;
                    //                            }
                    //                        }
                    //
                    //                    }
                }
            }
        }

        if (find_types) {
            for (auto &elem : m_types) {
                if (pred_compare(start, elem.first)) {
                    result.push_back(utf8_decode(prefix + elem.first));
                    if (result.size() > overage_count + 1) {
                        break;
                    }
                }
            }
        }
        return result;

    }

    ObjType BaseTypeFromString(const std::string & type, bool *has_error = nullptr) {
        if (type.empty()) {
            return ObjType::None;
        }
        auto result = m_types.find(isType(type) ? type.substr(1) : type);
        if (result == m_types.end()) {
            if (has_error) {
                *has_error = true;
                return ObjType::None;
            }
            LOG_RUNTIME("Type name '%s' not found!", type.c_str());
        }
        return result->second->m_var_type_fixed;
    }

    ObjPtr CreateTypeName(TermPtr type, ObjPtr base) {
        ASSERT(base);
        ASSERT(type);
        ASSERT(type->getTermID() == TermID::TERM);
        ASSERT(type->size() == 0);

        std::string type_name = (isType(type->m_text) ? type->m_text.substr(1) : type->m_text);
        if (m_types.find(type_name) != m_types.end()) {
            LOG_RUNTIME("Type name '%s' already exists!", type_name.c_str());
        }
        ObjPtr result = base->Clone();
        result->m_var_type_name = type_name;
        m_types[type_name] = result;
        return result;
    }

    SCOPE(protected) :
    size_t GetCount();
    ObjPtr GetIndex(size_t index);

private:

    Context(const Context&) = delete;
    const Context& operator=(const Context&) = delete;

};

}
#endif //INCLUDED_NEWLANG_CONTEXT_
