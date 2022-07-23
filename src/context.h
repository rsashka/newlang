#pragma once
#ifndef INCLUDED_NEWLANG_CONTEXT_
#define INCLUDED_NEWLANG_CONTEXT_

#include "pch.h"

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

    class Context : public Variable<Obj, std::weak_ptr<Obj> > {
    public:

        friend class Obj;

        static ObjPtr eval_END(Context *ctx, const TermPtr & term, Obj * args);
        static ObjPtr func_NOT_SUPPORT(Context *ctx, const TermPtr & term, Obj * args);

        enum class CreateMode {
            CREATE_ONLY,
            CREATE_OR_ASSIGN,
            ASSIGN_ONLY,
        };
        static ObjPtr CREATE_OR_ASSIGN(Context *ctx, const TermPtr & term, Obj *args, CreateMode mode);

#define DEFINE_CASE(name) \
    static ObjPtr eval_ ## name(Context *ctx, const TermPtr &term, Obj *args);

        NL_TERMS(DEFINE_CASE);

#undef DEFINE_CASE

#define PROTO_OP(_, func) \
    static ObjPtr op_ ## func(Context *ctx, const TermPtr &term, Obj *args);

        NL_OPS(PROTO_OP);

#undef PROTO_OP


        typedef ObjPtr(*EvalFunction)(Context *ctx, const TermPtr & term, Obj * args);

        static std::map<std::string, Context::EvalFunction> m_ops;
        static std::map<std::string, Context::EvalFunction> m_builtin_calls;
        static Parser::MacrosStore m_macros; ///< Хотя макросы и могут обработываться в рантайме, но доступны они только для парсера


        LLVMBuilderRef m_llvm_builder;

        static void Reset() {
            m_types.clear();
            m_funcs.clear();
            m_macros.clear();
            m_ops.clear();
            m_builtin_calls.clear();
            Docs::m_docs.clear();
        }

        inline ObjPtr ExecFile(const std::string &filename, Obj *args = nullptr, bool int_catch = true) {
            std::string source = ReadFile(filename.c_str());
            if (source.empty()) {
                LOG_RUNTIME("Empty source or file '%s' not found!", filename.c_str());
            }
            return ExecStr(source, args, int_catch);
        }

        inline ObjPtr ExecStr(const std::string str, Obj *args = nullptr, bool int_catch = false) {
            TermPtr exec = Parser::ParseString(str, &m_macros);
            if (exec->m_id == TermID::BLOCK) {
                exec->m_id = TermID::CALL_BLOCK;
            } else if (exec->m_id == TermID::BLOCK_TRY) {
                exec->m_id = TermID::CALL_TRY;
            }
            ObjPtr temp;
            if (args == nullptr) {
                temp = Obj::CreateNone();
                args = temp.get();
            }
            return Eval(this, exec, args, int_catch);
        }

        static ObjPtr Eval(Context *ctx, TermPtr term, Obj *args, bool int_catch = false);

        static ObjPtr ExpandAssign(Context *ctx, TermPtr lvar, TermPtr rval, Obj *args, CreateMode mode);
        static ObjPtr ExpandCreate(Context *ctx, TermPtr lvar, TermPtr rval, Obj * args);

        Context(RuntimePtr global);


        static std::map<std::string, ObjPtr> m_types;
        typedef at::variant<ObjPtr, std::vector < ObjPtr> > FuncItem;
        static std::map<std::string, FuncItem> m_funcs; // Системный и встроенные функции 

        inline static ObjPtr CreateLVal(Context *ctx, TermPtr type) {
            Obj args;
            return CreateLVal(ctx, type, &args);
        }

        inline static ObjPtr CreateRVal(Context *ctx, TermPtr term, bool int_catch = true) {
            Obj args;
            return CreateRVal(ctx, term, &args, int_catch);
        }

        inline static ObjPtr CreateRVal(Context *ctx, const char *source, bool int_catch = true) {
            Obj args;
            return CreateRVal(ctx, source, &args, int_catch);
        }

        static ObjPtr CreateLVal(Context *ctx, TermPtr type, Obj * args);
        static ObjPtr CreateRVal(Context *ctx, TermPtr term, Obj *args, bool int_catch = true);
        static ObjPtr CreateRVal(Context *ctx, const char *source, Obj *args, bool int_catch = true);
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
            if (str.size() && (str[0] == '$' || str[0] == '@')) {
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
        Variable<Obj> m_global_terms;

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
            auto found = m_global_terms.find(MakeName(name));
            if (found != m_global_terms.end()) {
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


        static ObjPtr CallBlock(Context *ctx, const TermPtr &block, Obj *local_vars, bool int_catch);

        static ObjPtr EvalBlockAND(Context *ctx, const TermPtr &block, Obj * local_vars);
        static ObjPtr EvalBlockOR(Context *ctx, const TermPtr &block, Obj * local_vars);
        static ObjPtr EvalBlockXOR(Context *ctx, const TermPtr &block, Obj * local_vars);

        ObjPtr CreateNative(const char *proto, const char *module = nullptr, bool lazzy = false, const char *mangle_name = nullptr);
        ObjPtr CreateNative(TermPtr proto, const char *module = nullptr, bool lazzy = false, const char *mangle_name = nullptr);
        ObjPtr CreateNative(Obj args);

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

            if (isGlobal(start)) {
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
            func_dump += " := {};";

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
            
            auto result = m_types.find(type);
            if (result == m_types.end()) {
                if (has_error) {
                    *has_error = true;
                    return nullptr;
                }
                LOG_RUNTIME("Type name '%s' not found!", type.c_str());
            }
            return result->second;
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
