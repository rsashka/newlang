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

    static ObjPtr eval_END(Context *ctx, const TermPtr & term, Object *args);
    static ObjPtr func_NOT_SUPPORT(Context *ctx, const TermPtr & term, Object *args);

    enum class CreateMode {
        CREATE_ONLY,
        CREATE_OR_ASSIGN,
        ASSIGN_ONLY,
    };
    static ObjPtr CREATE_OR_ASSIGN(Context *ctx, const TermPtr & term, Object *args, CreateMode mode);

#define DEFINE_CASE(name) \
    static ObjPtr eval_ ## name(Context *ctx, const TermPtr &term, Object *args);

    NL_TERMS(DEFINE_CASE);

#undef DEFINE_CASE

#define PROTO_OP(_, func) \
    static ObjPtr op_ ## func(Context *ctx, const TermPtr &term, Object *args);

    NL_OPS(PROTO_OP);

#undef PROTO_OP


    typedef ObjPtr(*EvalFunction)(Context *ctx, const TermPtr & term, Object *args);

    static std::map<std::string, Context::EvalFunction> m_ops;
    static std::map<std::string, Context::EvalFunction> m_builtin_calls;

    static void Reset() {
        m_types.clear();
        m_funcs.clear();
        m_ops.clear();
        m_builtin_calls.clear();
    }

    inline ObjPtr Eval(const std::string_view str) {
        Object args;
        return Eval(this, Parser::ParseString(str), &args);
    }

    inline ObjPtr ExecFile(const std::string &filename) {
        std::string source = ReadFile(filename.c_str());
        if (source.empty()) {
            LOG_RUNTIME("Empty source or file '%s' not found!", filename.c_str());
        }
        return ExecStr(source);
    }

    inline ObjPtr ExecStr(const std::string &source) {
        TermPtr exec = Parser::ParseString(source);
        if (exec->m_id == TermID::BLOCK) {
            exec->m_id = TermID::CALL_BLOCK;
        } else if (exec->m_id == TermID::BLOCK_TRY) {
            exec->m_id = TermID::CALL_TRY;
        }
        return Eval(this, exec);
    }

    inline ObjPtr Eval(const std::string_view str, Object *args) {
        return Eval(this, Parser::ParseString(str), args);
    }

    inline static ObjPtr Eval(Context *ctx, TermPtr term) {
        Object args;
        return Eval(ctx, term, &args);
    }
    static ObjPtr Eval(Context *ctx, TermPtr term, Object *args);

    static ObjPtr ExpandAssign(Context *ctx, TermPtr lvar, TermPtr rval, Object *args, CreateMode mode);
    static ObjPtr ExpandCreate(Context *ctx, TermPtr lvar, TermPtr rval, Object *args);

    Context(RuntimePtr global);


    static std::map<std::string, ObjPtr> m_types;
    typedef std::variant<ObjPtr, std::vector<ObjPtr>> FuncItem;
    static std::map<std::string, FuncItem> m_funcs; // Системный и встроенные функции 

    inline static ObjPtr CreateLVal(Context *ctx, TermPtr type) {
        Object args;
        return CreateLVal(ctx, type, &args);
    }

    inline static ObjPtr CreateRVal(Context *ctx, TermPtr term) {
        Object args;
        return CreateRVal(ctx, term, &args);
    }

    inline static ObjPtr CreateRVal(Context *ctx, const char *source) {
        Object args;
        return CreateRVal(ctx, source, &args);
    }

    static ObjPtr CreateLVal(Context *ctx, TermPtr type, Object *args);
    static ObjPtr CreateRVal(Context *ctx, TermPtr term, Object *args);
    static ObjPtr CreateRVal(Context *ctx, const char *source, Object *args);

    static std::vector<Index> MakeIndex(Context *ctx, TermPtr term, Object *local_vars);

    void ItemTensorEval_(torch::Tensor &tensor, c10::IntArrayRef shape, std::vector<Index> &ind, const int64_t pos, ObjPtr & obj, ObjPtr &args);
    void ItemTensorEval(torch::Tensor &tensor, ObjPtr obj, ObjPtr args);

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
        return GetObject(name);
    }

    void RegisterInContext(ObjPtr &args) {
        RegisterInContext(*args);
    }

    void RegisterInContext(Object &args) {
        for (int i = static_cast<int> (args.size()) - 1; args.size() && i >= 0; i--) {
            push_front(args.at(i).second, args.at(i).first);
        }
    }

    void UnRegisterInContext(Object &args) {
        for (int i = static_cast<int> (args.size()) - 1; args.size() && i >= 0; i--) {
            pop_front();
        }
    }


    static ObjPtr CallBlock(Context *ctx, const TermPtr &block, Object *local_vars);
    static ObjPtr CallBlockTry(Context *ctx, const TermPtr &block, Object *local_vars);

    static ObjPtr EvalBlockAND(Context *ctx, const TermPtr &block, Object *local_vars);
    static ObjPtr EvalBlockOR(Context *ctx, const TermPtr &block, Object *local_vars);
    static ObjPtr EvalBlockXOR(Context *ctx, const TermPtr &block, Object *local_vars);

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

        TermPtr proto = Parser::ParseString(func_dump);
        ObjPtr obj =
                Object::CreateFunc(this, proto->Left(), type,
                proto->Left()->getName().empty() ? proto->Left()->getText() : proto->Left()->getName());
        obj->m_func_ptr = func;

        return obj;
    }

    ObjPtr Comprehensions(Context *ctx, TermPtr term, Object *local_vars);
    ObjPtr Comprehensions(Context *ctx, Object *type, Object *args);

    ObjType BaseTypeFromString(const std::string & type, bool *has_error = nullptr) {
        ObjPtr obj_type = GetTypeFromString(type);

        if (obj_type == nullptr) {
            if (has_error) {
                *has_error = true;
                return ObjType::None;
            }
            LOG_RUNTIME("Type name '%s' not found!", type.c_str());
        }
        return obj_type->m_var_type_fixed;
    }

    ObjPtr GetTypeFromString(const std::string & type) {
        if (type.empty()) {
            return Object::CreateNone();
        }
        auto result = m_types.find(isType(type) ? type.substr(1) : type);
        if (result == m_types.end()) {
            return nullptr;
            //            LOG_RUNTIME("Type name '%s' not found!", type.c_str());
        }
        return result->second;
    }

    ObjPtr CreateTypeName(TermPtr type, ObjPtr base) {
        ASSERT(base);
        ASSERT(type);
        ASSERT(type->size() == 0);

        return CreateTypeName(type->m_text, base);
    }

    ObjPtr CreateTypeName(std::string type_name, std::string base_name) {
        auto base = m_types.find((isType(base_name) ? base_name.substr(1) : base_name));
        if (base == m_types.end()) {
            LOG_RUNTIME("Base type name '%s' not found!", base_name.c_str());
        }
        return CreateTypeName(type_name, base->second);
    }

    ObjPtr CreateTypeName(std::string type_name, ObjPtr base) {
        if (isType(type_name)) {
            type_name = type_name.substr(1);
        }
        if (m_types.find(type_name) != m_types.end()) {
            LOG_RUNTIME("Type name '%s' already exists!", type_name.c_str());
        }
        ObjPtr result = base->Clone();
        result->m_var_type_name = type_name;
        m_types[type_name] = result;
        return result;
    }

    static ObjPtr CreateClass(std::string name, Object &args) {
        ObjPtr result = Object::CreateClass(name);
        for (int i = 0; i < args.size(); i++) {
            if (args.name(i).empty()) {
                LOG_RUNTIME("Field pos %d has no name!", i);
            }
            result->push_back(args[i], args.name(i));
        }
        return result;
    }

    static ObjPtr CreateSimpleType(ObjType type) {

        ObjPtr result = Object::CreateFunc("Type(...)", &CreateSimpleTypeFunc, ObjType::TRANSPARENT);
        result->m_var_type_current = ObjType::Type;
        result->m_var_type_fixed = type;
        return result;
    }

    /*
     * ТИПЫ ДАННЫХ (без аргументов)
     * 
     * :type_int := :Int; # Синоним типа Int во время компиляции (тип не может быть изменен)
     * :type_int := :Int(); # Копия типа Int во время выполнения (тип может быть изменен после Mutable)
     * var_type := :Int; # Тип в переменной, которую можно передавать как аргумент в функции
     * 
     * 
     * ЗНАЧЕНИЯ УКАЗАННЫХ ТИПОВ  (при наличии аргументов)
     * 
     * scalar_int := :Int(0); # Преобразование типа во время выполнения с автоматической размерностью (скаляр)
     * scalar_int := :Int[0](0); # Преобразование типа во время выполнения с указанием размерности (скаляр)
     * scalar_int := :Int[0]([0,]); # Преобразование типа во время выполнения с указанием размерности (скаляр)
     * 
     * tensor_int := :Int([0,]); # Преобразование типа во время выполнения с автоматической размерностью (тензор)
     * tensor_int := :Int[1](0); # Преобразование типа во время выполнения с указанием размерности (тензор)
     * tensor_int := :Int[...](0); # Преобразование типа во время выполнения с произвольной размернотью (тензор)
     */
    static ObjPtr CreateSimpleTypeFunc(Context *ctx, Object & args) {

        if (args.empty() || !args[0]) {
            LOG_RUNTIME("Self simple type not defined!");
        }
        ASSERT(args[0]->getType() == ObjType::Type);

        ObjPtr result = args[0]->Clone();
        if (args.size() == 1) {
            // Копия существующего типа с возможностью редактирования
            result->m_is_const = false;
            return result;
        }

        // Переданы значения для приведения типов
        result->m_var_type_current = result->m_var_type_fixed;
        // Но само значение пока не установлено
        result->m_var_is_init = false;

        std::vector<int64_t> dims;

        // в m_value пока нет значения и is_tensor() использовать нельзя
        if (isSimpleType(result->getType())) {
            if (result->m_type && result->m_type->size() == 1
                    && (*result->m_type)[0]->toIndex().is_boolean() // 0 и 1 - логические значения
                    && (*result->m_type)[0]->GetValueAsInteger() == 0) {

                if (result->m_value.dim() > 1 || result->m_value.size(0) > 1) {
                    LOG_RUNTIME("Fail convert non single value tenzor to scalar!");
                } else if (result->m_value.dim() == 1 && result->m_value.size(0) == 1) {
                    dims.push_back(0);

                    // Тип скаляра
                    result->m_value = torch::scalar_tensor(0, toTorchType(result->getType()));
                }

            } else {
                if (args.at(0).second->m_type) {

                    // Размерность тензора указана
                    for (size_t i = 0; i < args.at(0).second->m_type->size(); i++) {
                        Index ind = (*args.at(0).second->m_type)[i]->toIndex();
                        if (ind.is_integer()) {
                            dims.push_back(ind.integer());
                        } else {
                            LOG_RUNTIME("Non fixed dimension not implemented!");
                        }
                    }

                }

                result->m_value = torch::empty(dims, toTorchType(result->getType()));
            }
        }

        if (args.size() == 2 && dims.empty()) {
            ObjPtr convert = args[1]->toType(result->m_var_type_current);
            convert->m_var_type_fixed = result->m_var_type_current;
            convert.swap(result);
        } else {
            for (int i = 1; i < args.size(); i++) {
                result->op_concat_(args[i], ConcatMode::Append);
            }
            if (!dims.empty()) {
                result->m_value = result->m_value.reshape(dims);
            }
        }

        return result;
    }

    /*
     * :Enum(One=0, Two=_, "Three", Ten=10);
     */

    static ObjPtr CreateEnum(Context *ctx, Object & args) {
        ObjPtr result = Object::CreateDict(); //Type(ObjType::Dictionary, nullptr, ObjType::Enum);
        result->m_var_type_fixed = ObjType::Dictionary;
        result->m_class_name = ":Enum";

        int64_t val_int = 0;
        ObjPtr enum_value;
        std::string enum_name;

        for (int i = 1; i < args.size(); i++) {
            if (args.name(i).empty()) {
                if (args[i] && args[i]->is_string_type()) {
                    enum_name = args[i]->GetValueAsString();
                } else {
                    LOG_RUNTIME("Field pos %d has no name!", i);
                }
            } else {
                enum_name = args.name(i);

                if (args[i] && (args[i]->is_integer() || args[i]->is_bool_type())) {
                    val_int = args[i]->GetValueAsInteger();
                } else if (!args[i] || !args[i]->is_none_type()) {
                    LOG_RUNTIME("Field value '%s' %d must integer type!", args.name(i).c_str(), i);
                }
            }

            if (!result->select(enum_name).complete()) {
                LOG_RUNTIME("Field value '%s' at index %d already exists!", enum_name.c_str(), i);
            }


            enum_value = Object::CreateValue(val_int, ObjType::None); // , type
            enum_value->m_var_type_fixed = enum_value->m_var_type_current;
            enum_value->m_is_const = true;
            result->push_back(enum_value, enum_name);
            val_int += 1;
        }

        result->m_is_const = true;

        //@todo Реализовать указание типов для элементов перечисления !!!

        // Check BuildInType
        //    bool has_error = false;
        //    ObjType type = ObjType::None; //typeFromString(term->m_class_name, nullptr, &has_error);
        //    //        if (has_error) {
        //    //            NL_PARSER(term, "Typename '%s' not base type!", term->m_class_name.c_str());
        //    //        }
        //    //        if (!isIntegralType(type, true)) {
        //    //            NL_PARSER(term, "Enum must be intergal type, not '%s'!", term->m_class_name.c_str());
        //    //        }
        //    int64_t val_int = 0;
        //    ObjPtr temp;
        //    for (size_t i = 0; i < args.size(); i++) {
        //        if (args.name(i).empty()) {
        //            NL_PARSER(term, "The enum element must have a name!");
        //        }
        //
        //        if (args[i]) {
        //            temp = args[i]->Clone();
        //            val_int = temp->GetValueAsInteger();
        //            type = temp->getType();
        //            NL_TYPECHECK(term, newlang::toString(typeFromLimit(val_int)),
        //                    term->m_class_name); // Соответстствует ли тип значению?
        //        } else {
        //            temp = Object::CreateValue(val_int, type);
        //        }
        //        temp->m_var_type_fixed = type;
        //        result->push_back(temp, args.name(i).c_str());
        //        val_int += 1;
        //    }
        return result;


    }

    //    static ObjPtr Create Struct(Object & args) {
    //        CheckClass(args);
    //        for (int i = 0; i < args.size(); i++) {
    //            ObjPtr item = args[i];
    //
    //            if (!item) {
    //                LOG_RUNTIME("An empty field pos %d in a structure is not allowed!", i);
    //            }
    //
    //            if (!isPlainDataType(item->m_var_type_fixed)) {
    //                LOG_RUNTIME("Field data type '%s'(%d) is not plain!", args.name(i).c_str(), i);
    //            }
    //        }
    //    }

    SCOPE(protected) :
    size_t GetCount();
    ObjPtr GetIndex(size_t index);

private:

    Context(const Context&) = delete;
    const Context& operator=(const Context&) = delete;

};

}
#endif //INCLUDED_NEWLANG_CONTEXT_
