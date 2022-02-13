#include "pch.h"

#include <core/types.h>
#include <core/term.h>
#include <core/context.h>
#include <core/newlang.h>
#include <filesystem>


using namespace newlang;

/*
 Идентификатором объектов является его имя, причем у сессионных и глобальных объектов оно присваивается автоматически даже для переменных литералов.
Имена локальных объектов перекрывают сессионные и глобальные, а сессионный объкт перекрывает глобальный.
Дубли локальных объектов не допускаются, т.к. имена локальных переменных должны быть уникальные на уровне языка реализации.
Дубли сессионных объектов для текущей сессии не допускаются, но можно перекрывать сессионные объекты более высокого уровня.
Дубли глобальных объектов допускаются без ограничений, а при создании дубля выводится предупреждение.

При наличии переменных с одним именем, но разным временем жизни, приоритет доступпа будет следующим.
обращение по имени объекта - доступ только к локальному объекту (разрешение имени происходит во время компиляции) или будет ошибка если объект отсутствует.
обращение как к сессионному обекту (префикс $) - если есть локальный объект, вернется локальный, иначе вернется сессионный или будет ошибка если объект отсутствует.
обращение как к глобальному обекту (префикс @) - если есть локальный, вернется локальный, иначе вернется сессионный, иначе вернется глобальный или будет ошибка если объект отсутствует.

Можно всегда указывать глобальный идентификатор объекта, то доступ будет в зависимости от наличия локальных или сессионных переменных/объектов.

$.obj прямое обращения к сессионному объекту при наличии локального
@.obj прямое обращения к глобальному объекту при наличии локального или сессионного

 */

std::map<std::string, Context::EvalFunction> Context::m_ops;
std::map<std::string, Context::EvalFunction> Context::m_builtin_calls;
std::map<std::string, ObjPtr> Context::m_types;
std::map<std::string, Context::FuncItem> Context::m_funcs;

Context::Context(RuntimePtr global) {
    m_runtime = global;

    if(Context::m_funcs.empty()) {
        Context::m_funcs["min"] = CreateBuiltin("min(arg, ...)", (void *) &min, ObjType::TRANSPARENT);
        Context::m_funcs["мин"] = Context::m_funcs["min"];

        Context::m_funcs["max"] = CreateBuiltin("max(arg, ...)", (void *) &max, ObjType::TRANSPARENT);
        Context::m_funcs["макс"] = Context::m_funcs["max"];

        Context::m_funcs["import"] = CreateBuiltin("import(arg, module='', lazzy=0)", (void *) &import, ObjType::FUNCTION);

        Context::m_funcs["eval"] = CreateBuiltin("eval(string:String)", (void *) &eval, ObjType::FUNCTION);
        Context::m_funcs["exec"] = CreateBuiltin("exec(filename:String)", (void *) &exec, ObjType::FUNCTION);

        //        Context::m_builtin_funcs["loadonce"] = CreateBuiltin("loadonce(name:String, init:Bool=@true, filename:String=_):Bool", (void *) &loadonce, ObjType::FUNCTION);
        //        Context::m_builtin_funcs["load"] = CreateBuiltin("load(name:String, init:Bool=@true, filename:String=_):Bool", (void *) &load, ObjType::FUNCTION);
        //        Context::m_builtin_funcs["unload"] = CreateBuiltin("unload(name:String):Bool", (void *) &unload, ObjType::FUNCTION);
        //        Context::m_builtin_funcs["isloaded"] = CreateBuiltin("isloaded(name:String):Bool", (void *) &isloaded, ObjType::FUNCTION);

#define REGISTER_TYPES(name, cast) \
    ASSERT(Context::m_funcs.find(#name) == Context::m_funcs.end()); \
    Context::m_funcs[#name] = CreateBuiltin(#name "(var, shape=_): " #cast, (void *)& newlang:: name, ObjType::TRANSPARENT); \
    Context::m_funcs[#name "_"] = CreateBuiltin(#name "_(&var, shape=_): " #cast, (void *)& newlang:: name##_, ObjType::FUNCTION);

        NL_BUILTIN_CAST_TYPE(REGISTER_TYPES)

#undef REGISTER_TYPES
    }

    if(Context::m_types.empty()) {
#define REGISTER_TYPES(name, cast) \
    ASSERT(Context::m_types.find(#name) == Context::m_types.end()); \
    Context::m_types[#name] = Object::CreateType(ObjType:: cast, nullptr, ObjType:: cast);

        NL_BUILTIN_CAST_TYPE(REGISTER_TYPES)

#undef REGISTER_TYPES

    }

    if(Context::m_builtin_calls.empty()) {
#define REGISTER_FUNC(name, func) \
    ASSERT(Context::m_builtin_calls.find(name) == Context::m_builtin_calls.end()); \
    Context::m_builtin_calls[name] = &Context::func_##func;

        NL_BUILTIN(REGISTER_FUNC);

#undef REGISTER_FUNC
    }

    if(Context::m_ops.empty()) {
#define REGISTER_OP(op, func) \
    ASSERT(Context::m_ops.find(op) == Context::m_ops.end()); \
    Context::m_ops[op] = &Context::op_##func;

        NL_OPS(REGISTER_OP);

#undef REGISTER_OP 
    }
}


//    std::string func_dump(prototype);
//    func_dump += " := {}";
//
//    TermPtr proto;
//    Parser parser(proto);
//    parser.Parse(func_dump);
//
//    ObjPtr obj = Object::CreateFunc(ctx, proto->Left(), type, proto->Left()->getName().empty() ? proto->Left()->getText() : proto->Left()->getName());
//    obj->m_function = func;
//
//    return RegisterFunc(obj, true);

ObjPtr Context::CreateBuiltin(const char * prototype, void * func, ObjType type) {
    ASSERT(prototype);
    ASSERT(func);

    std::string func_dump(prototype);
    func_dump += " := {}";

    TermPtr proto = Parser::ParseString(func_dump);
    ObjPtr obj = Object::CreateFunc(this, proto->Left(), type, proto->Left()->getName().empty() ? proto->Left()->getText() : proto->Left()->getName());
    obj->m_func_ptr = func;

    return obj;
}

inline ObjType newlang::typeFromString(const std::string type, Context *ctx, bool *has_error) {

    if(ctx) {
        return ctx->BaseTypeFromString(type, has_error);
    }

#define DEFINE_CASE(name, cast) else if(type.compare(#name)==0) { return ObjType:: cast; }

    if(type.compare("") == 0) {
        return ObjType::None;
    } else if(type.compare("_") == 0) {
        return ObjType::None;
    }
    NL_BUILTIN_CAST_TYPE(DEFINE_CASE)

#undef DEFINE_CASE

    if(has_error) {
        *has_error = true;
        return ObjType::None;
    }
    LOG_RUNTIME("Undefined type name '%s'!", type.c_str());
}

ObjPtr Context::RegisterObject(ObjPtr var) {
    if(!var || var->getName().empty()) {
        LOG_RUNTIME("Empty object name %s", var ? var->toString().c_str() : "");
    }
    ASSERT(var->m_namespace.empty());
    //    std::string full_name = var->m_namespace;
    //    if(!full_name.empty()) {
    //        full_name += "::";
    //    }
    //    full_name += var->getName();

    if(isGlobal(var->getName())) {
        var->getName() = var->getName().substr(1);
        m_global_terms.push_back(var, var->getName());
    }
    if(isLocal(var->getName())) {
        var->getName() = var->getName().substr(1);
    }
    push_back(var, var->getName());

    //    if(!var->is_function()) {
    //    full_name += var->getName();
    //        LOG_RUNTIME("Object '%s' not function!", var->toString().c_str());
    //    }
    //    push_back(var, full_name);
    return var;
}

void newlang::NewLangSignalHandler(int signal) {
    throw abort_exception("Signal SIGABRT received");
}

//#include "StdCapture.h"

ObjPtr Context::Eval(Context *ctx, TermPtr term, Object &args) {


    //    StdCapture Capture;
    //
    //    Capture.BeginCapture();

    auto previous_handler = signal(SIGABRT, &NewLangSignalHandler);
    try {

        switch(term->m_id) {
            case TermID::END:
                return eval_END(ctx, term, args);

#define DEFINE_CASE(name)   \
        case TermID::name:\
          return eval_ ## name(ctx, term, args); 

                NL_TERMS(DEFINE_CASE)

#undef DEFINE_CASE

            default:
                return eval_UNKNOWN(ctx, term, args);
        }

    } catch (abort_exception &err) {
        signal(SIGABRT, previous_handler);
        //        Capture.EndCapture();
        NL_EVAL(term, "Exception eval '%s' (%s)", term->toString().c_str(), err.what());
    }
}

ObjPtr Context::eval_END(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term && term->m_text.compare("") == 0);
    LOG_RUNTIME("eval_END: %s", term->toString().c_str());
    return nullptr;
}

ObjPtr Context::eval_UNKNOWN(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term && term->m_text.compare("") == 0);
    LOG_RUNTIME("eval_UNKNOWN: %s", term->toString().c_str());
    return nullptr;
}

/*
 * 
 * 
 */


ObjPtr Context::eval_BLOCK(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term && term->getTermID() == TermID::BLOCK);
    return EvalBlock(ctx, term, args);
}

inline ObjPtr Context::eval_INTEGER(Context *ctx, const TermPtr &term, Object &args) {
    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_NUMBER(Context *ctx, const TermPtr &term, Object &args) {
    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_COMPLEX(Context *ctx, const TermPtr &term, Object &args) {
    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_CURRENCY(Context *ctx, const TermPtr &term, Object &args) {
    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_FRACTION(Context *ctx, const TermPtr &term, Object &args) {
    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_STRWIDE(Context *ctx, const TermPtr &term, Object &args) {
    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_STRCHAR(Context *ctx, const TermPtr &term, Object &args) {
    return CreateRVal(ctx, term, args);
}

//inline ObjPtr Context::eval_STRVAR(Context *ctx, const TermPtr &term, Object &args) {
//    return CreateRVal(ctx, term, args);
//}

inline ObjPtr Context::eval_TENSOR(Context *ctx, const TermPtr &term, Object &args) {
    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_NONE(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term && term->m_id == TermID::NONE);
    return Object::CreateNone();
}

ObjPtr Context::eval_TERM(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term && term->m_id == TermID::TERM);
    return CreateRVal(ctx, term, args);
}

ObjPtr Context::eval_CALL(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term && term->m_id == TermID::CALL);
    ObjPtr result = ctx->CallByName(term, args);
    if(result) {
        return result;
    }
    //    auto found = Context::m_builtin_calls.find(term->m_text);
    //    if(found == Context::m_builtin_calls.e*nd()) {
    NL_PARSER(term, "Function or term '%s' not found!", term->m_text.c_str());
    //    }
    //    //    return this->*found->second(term);
    //    return nullptr;
}

ObjPtr Context::func_NOT_SUPPORT(Context *ctx, const TermPtr &term, Object &args) {
    NL_PARSER(term, "Function or operator '%s' is not supported in interpreter mode!", term->m_text.c_str());
    return nullptr;
}

ObjPtr Context::eval_TEMPLATE(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::eval_COMMENT(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::eval_TYPENAME(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::eval_SYMBOL(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::eval_NAMESPACE(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

/*
 * 
 * 
 */


ObjPtr Context::ExpandAssign(Context *ctx, TermPtr lvar, TermPtr rval, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::ExpandCreate(Context *ctx, TermPtr lvar, TermPtr rval, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::eval_ASSIGN(Context *ctx, const TermPtr &term, Object &local_vars) {
    // Присвоить значение можно как одному термину, так и сразу нескольким при раскрытии словаря:
    // var1, var2, _ = ... func(); // Первый и второй элементы словаря записывается в var1 и var2, а остальные элементы возвращаемого словаря игнорируются (если они есть)
    // var1, var2 = ... func(); // Если функция вернула словрь с двумя элементами, то их значения записываются в var1 и var2. 
    //   Если в словаре было больше двух элементов, то первый записывается в var1, а оставшиеся в var2. !!!!!!!!!!!!!
    // var1, ..., var2 = ... func(); // Первый элемент словаря записывается в var1, а последний в var2.

    ASSERT(term && term->getTermID() == TermID::ASSIGN);
    ASSERT(term->Left());

    auto found = ctx->select(term->Left()->m_text);
    if(!term->Right()) {
        if(!found.complete()) {
            ctx->erase(found);
            return Object::Yes();
        }
        return Object::No();
    }

    if(term->Left()->Right() || term->Right()->getTermID() == TermID::ELLIPSIS) {
        return ExpandAssign(ctx, term->Left(), term->Right(), local_vars);
    }
    ObjPtr rval = Eval(ctx, term->Right(), local_vars);
    if(!rval) {
        NL_PARSER(term->Right(), "Object is missing or expression is not evaluated!");
    }

    if(found.complete()) {
        NL_PARSER(term->Left(), "Object '%s' not found!", term->Left()->m_text.c_str());
    }
    ObjPtr lval = found.data().second.lock();
    lval->SetValue_(rval);
    return lval;
}

ObjPtr Context::eval_CREATE(Context *ctx, const TermPtr &term, Object &local_vars) {
    // Создать можно один или сразу несколько терминов за раз
    // var1, var2, var3 := value;
    // var1, var2, var3 := ... value; // Раскрыть словарь?

    ASSERT(ctx);
    ASSERT(term && term->getTermID() == TermID::CREATE);
    ASSERT(term->Left() && term->Right());

    if(term->Left()->Right() || term->Right()->getTermID() == TermID::ELLIPSIS) {
        return ExpandCreate(ctx, term->Left(), term->Right(), local_vars);
    }

    ObjPtr rval = Eval(ctx, term->Right(), local_vars);
    if(!rval) {
        NL_PARSER(term->Right(), "Object is missing or expression is not evaluated!");
    }

    if(isType(term->Left()->m_text)) {
        return ctx->CreateTypeName(term->Left(), rval);
    }

    ObjPtr lval = CreateLVal(ctx, term->Left(), local_vars);
    if(!lval) {
        NL_PARSER(term->Left(), "Fail create lvalue object!");
    }

    lval->SetValue_(rval);

    return ctx->RegisterObject(lval);
}

ObjPtr Context::eval_APPEND(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::eval_FUNCTION(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term && (term->getTermID() == TermID::FUNCTION || term->getTermID() == TermID::TRANSPARENT || term->getTermID() == TermID::SIMPLE));
    ASSERT(term->Left());
    ASSERT(ctx);

    auto found = ctx->select(term->Left()->m_text);
    if(!term->Right()) {
        if(!found.complete()) {
            ctx->erase(found);
            return Object::Yes();
        }
        return Object::No();
    }


    ObjPtr lval = CreateLVal(ctx, term->Left(), args);
    if(!lval) {
        NL_PARSER(term->Left(), "Fail create lvalue object!");
    }

    if(term->Right()->getTermID() == TermID::CALL) {
        lval->SetValue_(CreateRVal(ctx, term->Right()));
    } else {
        if(term->getTermID() == TermID::FUNCTION) {
            lval->m_var_type_current = ObjType::EVAL_FUNCTION;
        } else if(term->getTermID() == TermID::TRANSPARENT) {
            lval->m_var_type_current = ObjType::EVAL_TRANSP;
        } else {
            LOG_RUNTIME("Create function '%s' not implemented!", term->toString().c_str());
        }
        lval->m_var_type_fixed = lval->m_var_type_current;
        lval->m_var_is_init = true;
        lval->m_func_source = term->Right();
    }

    return ctx->RegisterObject(lval);
}

ObjPtr Context::eval_SIMPLE(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::eval_LAMBDA(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::eval_TRANSPARENT(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

/*
 * 
 * 
 */
ObjPtr Context::eval_ITERATOR(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::eval_FOLLOW(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::eval_REPEAT(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::eval_RANGE(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::eval_ELLIPSIS(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::eval_ARGUMENT(Context *ctx, const TermPtr &term, Object &args) {
    return CreateRVal(ctx, term, args);
}

ObjPtr Context::eval_ARGSSTR(Context *ctx, const TermPtr &term, Object &args) {
    return CreateRVal(ctx, term, args);
}

ObjPtr Context::eval_ARGCOUNT(Context *ctx, const TermPtr &term, Object &args) {
    return CreateRVal(ctx, term, args);
}

ObjPtr Context::eval_EXIT(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::eval_EXCEPTION(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::eval_INDEX(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::eval_FIELD(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::eval_CLASS(Context *ctx, const TermPtr &term, Object &args) {
    return CreateRVal(ctx, term, args);
}

ObjPtr Context::eval_DICT(Context *ctx, const TermPtr &term, Object &args) {
    return CreateRVal(ctx, term, args);
}

ObjPtr Context::eval_CONCAT(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    return Eval(ctx, term->Left(), args)->op_concat(Eval(ctx, term->Right(), args));
}

ObjPtr Context::eval_SOURCE(Context *ctx, const TermPtr &term, Object &args) {
    NL_PARSER(term, "Inclusion on the implementation language is not supported in interpreter mode!");
    return nullptr;
}

/*
 * 
 * 
 */

ObjPtr Context::eval_OPERATOR(Context *ctx, const TermPtr &term, Object &args) {
    if(Context::m_ops.find(term->m_text) == Context::m_ops.end()) {
        LOG_RUNTIME("Eval op '%s' not exist!", term->m_text.c_str());
    }
    return (*Context::m_ops[term->m_text])(ctx, term, args);
}

/*
 * 
 * 
 */


ObjPtr Context::op_EQUAL(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    return Eval(ctx, term->Left(), args)->op_equal(Eval(ctx, term->Right(), args)) ? Object::Yes() : Object::No();
}

ObjPtr Context::op_ACCURATE(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    return Eval(ctx, term->Left(), args)->op_accurate(Eval(ctx, term->Right(), args)) ? Object::Yes() : Object::No();
}

ObjPtr Context::op_NE(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    return Eval(ctx, term->Left(), args)->operator!=(Eval(ctx, term->Right(), args)) ? Object::Yes() : Object::No();
}

ObjPtr Context::op_LT(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    return Eval(ctx, term->Left(), args)->operator<(Eval(ctx, term->Right(), args)) ? Object::Yes() : Object::No();
}

ObjPtr Context::op_GT(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    return Eval(ctx, term->Left(), args)->operator>(Eval(ctx, term->Right(), args)) ? Object::Yes() : Object::No();
}

ObjPtr Context::op_LE(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    return Eval(ctx, term->Left(), args)->operator<=(Eval(ctx, term->Right(), args)) ? Object::Yes() : Object::No();
}

ObjPtr Context::op_GE(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    return Eval(ctx, term->Left(), args)->operator>=(Eval(ctx, term->Right(), args)) ? Object::Yes() : Object::No();
}

/*
 * 
 * 
 */

ObjPtr Context::op_AND(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    return Eval(ctx, term->Left(), args)->op_bit_and(Eval(ctx, term->Right(), args), false);
}

ObjPtr Context::op_OR(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::op_XOR(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::op_BIT_AND(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::op_BIT_OR(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::op_BIT_XOR(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::op_BIT_AND_(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::op_BIT_OR_(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::op_BIT_XOR_(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

/*
 * 
 * 
 */
ObjPtr Context::op_PLUS(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Right());
    if(term->Left()) {
        return Eval(ctx, term->Left(), args)->operator+(Eval(ctx, term->Right(), args));
    }
    return Eval(ctx, term->Left(), args)->operator+();
}

ObjPtr Context::op_MINUS(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Right());
    if(term->Left()) {
        return Eval(ctx, term->Left(), args)->operator-(Eval(ctx, term->Right(), args));
    }
    return Eval(ctx, term->Left(), args)->operator-();
}

ObjPtr Context::op_DIV(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    return Eval(ctx, term->Left(), args)->operator/(Eval(ctx, term->Right(), args));
}

ObjPtr Context::op_DIV_CEIL(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    return Eval(ctx, term->Left(), args)->op_div_ceil(Eval(ctx, term->Right(), args));
}

ObjPtr Context::op_MUL(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    return Eval(ctx, term->Left(), args)->operator*(Eval(ctx, term->Right(), args));
}

ObjPtr Context::op_REM(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    return Eval(ctx, term->Left(), args)->operator%(Eval(ctx, term->Right(), args));
}

ObjPtr Context::op_POW(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    return Eval(ctx, term->Left(), args)->op_pow(Eval(ctx, term->Right(), args));
}

/*
 * 
 * 
 */
ObjPtr Context::op_PLUS_(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    return Eval(ctx, term->Left(), args)->operator+=(Eval(ctx, term->Right(), args));
}

ObjPtr Context::op_MINUS_(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    return Eval(ctx, term->Left(), args)->operator-=(Eval(ctx, term->Right(), args));
}

ObjPtr Context::op_DIV_(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    return Eval(ctx, term->Left(), args)->operator/=(Eval(ctx, term->Right(), args));
}

ObjPtr Context::op_DIV_CEIL_(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    return Eval(ctx, term->Left(), args)->op_div_ceil_(Eval(ctx, term->Right(), args));
}

ObjPtr Context::op_MUL_(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    return Eval(ctx, term->Left(), args)->operator*=(Eval(ctx, term->Right(), args));
}

ObjPtr Context::op_REM_(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    return Eval(ctx, term->Left(), args)->operator%=(Eval(ctx, term->Right(), args));
}

ObjPtr Context::op_POW_(Context *ctx, const TermPtr &term, Object &args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    return Eval(ctx, term->Left(), args)->op_pow_(Eval(ctx, term->Right(), args));
}

/*
 * 
 * 

ObjPtr Context::op_INC_(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::op_DEC_(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}
 */

ObjPtr Context::op_CONCAT(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::op_TYPE_EQ(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::op_TYPE_EQ2(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::op_TYPE_EQ3(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::op_TYPE_NE(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::op_TYPE_NE2(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::op_TYPE_NE3(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

/*
 * 
 */
ObjPtr Context::op_RSHIFT(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::op_LSHIFT(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::op_RSHIFT_(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::op_LSHIFT_(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::op_RRSHIFT(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::op_RRSHIFT_(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

ObjPtr Context::op_SPACESHIP(Context *ctx, const TermPtr &term, Object &args) {
    LOG_RUNTIME("Not implemented!");
    return nullptr;
}

/*
 * 
 * 
 */

ObjPtr Context::EvalBlock(Context *ctx, const TermPtr &block, Object &local_vars) {
    ObjPtr result = Object::CreateNone();
    if(block->GetTokenID() == TermID::BLOCK) {
        for (size_t i = 0; i < block->m_block.size(); i++) {
            result = Eval(ctx, block->m_block[i], local_vars);
        }
    } else {
        result = Eval(ctx, block, local_vars);
    }
    return result;
}

ObjPtr Context::CallByName(const TermPtr &term, Object &local_vars) {
    Object args(this, term, true, local_vars);
    return CallByName(term->m_text.c_str(), args, term->isRef());
}

ObjPtr Context::CallByName(const char * name, Object & args, bool is_ref) {

    ObjPtr sess = FindTerm(name);
    if(sess && !sess->is_function()) {
        // Если есть объект
        ObjPtr obj;
        if(is_ref) {
            obj = sess;
        } else {
            obj = sess->Clone();
            obj->m_class_base = sess;
        }
        if(args.size()) {
            if(obj->is_string_type()) {
                obj->SetValue_(obj->format(obj->GetValueAsString(), args));
            } else {
                obj = obj->Call(this, args);
            }
        }
        return obj;
    }

    ObjPtr func;
    if(sess) {
        func = sess;
    } else {
        // Функция с указанным именем
        func = GetObject(name);
    }
    if(func && func->is_function()) {

        Object arg_def(this, func->m_func_proto, false, args);

        ObjPtr arg_in = arg_def.ConvertToArgs(args, true, this);

        //        ObjPtr arg_in = func->Clone();
        //        arg_in = arg_in->ConvertToArgs(args, true);
        arg_in->push_front(Object::CreateNone());


        RegisterInContext(arg_in);

        if(func->m_var_type_current == ObjType::FUNCTION) {
            ASSERT(func->m_func_ptr);
            return (*reinterpret_cast<FunctionType *> (func->m_func_ptr))(this, *arg_in); // Непосредственно вызов функции
        } else if(func->m_var_type_current == ObjType::TRANSPARENT) {
            ASSERT(func->m_func_ptr);
            return (*reinterpret_cast<TransparentType *> (func->m_func_ptr))(this, *arg_in); // Непосредственно вызов функции
        } else if(func->m_var_type_current == ObjType::NativeFunc) {
            return func->CallNative(this, *arg_in);
        } else if(func->m_var_type_current == ObjType::EVAL_FUNCTION || func->m_var_type_current == ObjType::EVAL_TRANSP) {
            return Context::EvalBlock(this, func->m_func_source, *arg_in);
        }
        LOG_RUNTIME("Call by name not implemted '%s'!", func->toString().c_str());

    }
    LOG_RUNTIME("Function or term '%s' not found!%s", name, isLocalAny(name) ? " Try session($) or global(@) scope." : (isLocal(name) ? " Try global(@) scope." : ""));
}

ObjPtr Context::CreateNative(const char *proto, const char *module, bool lazzy, const char *mangle_name, ffi_abi abi) {
    TermPtr term;
    try {
        // Термин или термин + тип парсятся без ошибок
        term = Parser::ParseString(proto);
    } catch (std::exception &e) {
        try {
            std::string func(proto);
            func += ":={}";
            term = Parser::ParseString(func)->Left();
        } catch (std::exception &e) {
            LOG_RUNTIME("Fail parsing prototype '%s'!", e.what());
        }
    }
    return CreateNative(term, module, lazzy, mangle_name, abi);
}

ObjPtr Context::CreateNative(TermPtr proto, const char *module, bool lazzy, const char *mangle_name, ffi_abi abi) {

    NL_CHECK(proto, "Fail prototype native function!");
    NL_CHECK((module == nullptr || (module && *module == '\0')) || m_runtime, "You cannot load a module '%s' without access to the runtime context!", module);

    ObjPtr result;
    ObjType type;
    if(proto->GetTokenID() == TermID::TERM) {
        if(proto->m_type_name.empty()) {
            LOG_RUNTIME("Cannot create native variable without specifying the type!");
        }

        type = typeFromString(proto->m_type_name, this);
        switch(type) {
            case ObjType::Bool:
                //            case ObjType::Byte:
            case ObjType::Char:
            case ObjType::Short:
            case ObjType::Int:
            case ObjType::Long:
            case ObjType::Float:
            case ObjType::Double:
            case ObjType::Pointer:
                break;
            default:
                LOG_RUNTIME("Creating a variable with type '%s' is not supported!", proto->m_type_name.c_str());
        }

    } else if(proto->GetTokenID() == TermID::CALL) {
        type = ObjType::NativeFunc;
    }

    result = Object::CreateType(type);
    result->m_var_type_fixed = type; // Тип определен и не может измениться в дальнейшем

    *const_cast<TermPtr *> (&result->m_func_proto) = proto;
    result->m_func_abi = abi;

    if(mangle_name) {
        result->m_func_mangle_name = mangle_name;
    }
    if(module) {
        result->m_module_name = module;
    }
    if(lazzy) {
        result->m_func_ptr = nullptr;
    } else {
        result->m_func_ptr = m_runtime->GetProcAddress(result->m_func_mangle_name.empty() ? proto->m_text.c_str() : result->m_func_mangle_name.c_str(), module);
        if(result->is_function() || type == ObjType::Pointer) {
            NL_CHECK(result->m_func_ptr, "Error getting address '%s' from '%s'!", proto->toString().c_str(), module);
        } else if(result->m_func_ptr && result->is_tensor()) {
            result->m_value = torch::from_blob(result->m_func_ptr, {
            }, toTorchType(type));
        } else {
            LOG_RUNTIME("Fail CreateNative for object %s", result->toString().c_str());
        }
    }
    return result;
}

void * RunTime::GetProcAddress(const char *name, const char *module) {
    if(module && module[0]) {
        if(m_modules.find(module) == m_modules.end()) {
            LoadModule(module, false, nullptr);
        }
        if(m_modules.find(module) == m_modules.end()) {
            LOG_WARNING("Fail load module '%s'!", module);
            return nullptr;
        }
        return dlsym(m_modules[module]->GetHandle(), name);
    }
    return dlsym(nullptr, name);
}

ObjPtr Context::FindSessionTerm(const char *name, bool current_only) {
    auto found = select(MakeName(name));
    while(!found.complete()) {
        ObjPtr obj = found.data().second.lock();
        if(obj) {
            return obj;
        }
        erase(found);
        found++;
    }
    return nullptr;
}

/*
 * обращение по имени - доступ только к локальному объекту (разрешение имени во время компиляции).
 * обращение как к сессионному обекту - если есть локальный, будет локальный (разрешение имени во время компиляции), 
 *      иначе вернется сессионный или будет ошибка если объект отсутствует.
 * обращение как к глобальному обекту - если есть локальный, будет локальный (разрешение имени во время компиляции), 
 *      иначе вернется сессионный, иначе вернется глобальный или будет ошибка если объект отсутствует.
 * Можно всегда общащсять как к глобальному объекту, а доступ будет в зависимости от наличия локальных или сессионных переменных/объектов.
 */
ObjPtr Context::FindTerm(const char *name) {
    ObjPtr result = FindSessionTerm(name);
    if(result || isLocalAny(name) || isLocal(name)) {

        return result;
    }
    return FindGlobalTerm(name);
}

ObjPtr Context::GetTerm(const char *name, bool is_ref) {
    return FindTerm(name);
}

const ObjPtr Context::GetConst(const char *name) {
    return GetTerm(name, true);
}

std::string newlang::GetFileExt(const char * str) {
    std::string filename(str);
    std::string::size_type idx = filename.rfind('.');
    if(idx != std::string::npos) {

        return filename.substr(idx);
    }
    return std::string("");
}

std::string newlang::AddDefaultFileExt(const char * str, const char *ext_default) {
    std::string filename(str);
    std::string file_ext = GetFileExt(str);
    if(file_ext.empty() && !filename.empty() && filename.compare(".") != 0) {

        filename.append(ext_default);
    }
    return filename;
}

std::string newlang::ReplaceFileExt(const char * str, const char *ext_old, const char *ext_new) {
    std::string filename(str);
    std::string file_ext = GetFileExt(str);
    if(file_ext.compare(ext_old) == 0) {
        filename = filename.substr(0, filename.length() - file_ext.length());
    }
    file_ext = GetFileExt(filename.c_str());
    if(file_ext.compare(".") != 0 && file_ext.compare(ext_new) != 0 && !filename.empty() && filename.compare(".") != 0) {

        filename.append(ext_new);
    }
    return filename;
}

std::string newlang::ReadFile(const char *fileName) {
    std::ifstream f(fileName);
    std::stringstream ss;
    ss << f.rdbuf();
    f.close();
    return ss.str();
}

ObjPtr Context::CreateLVal(Context *ctx, TermPtr term, Object &args) {

    ASSERT(ctx);
    ASSERT(term);
    ASSERT(!term->m_text.empty());

    if(!ctx->select(term->m_text).complete()) {
        // Объект должне отсутствовать
        NL_PARSER(term, "Object '%s' already exists!", term->m_text.c_str());
    }


    ObjPtr result = Object::CreateNone();
    //    result->m_ctx = ctx;
    result->m_var_name = term->m_text;

    *const_cast<TermPtr *> (&result->m_func_proto) = term;

    TermPtr type = term->GetType();
    if(term->IsFunction() || term->getTermID() == TermID::CALL) {

        result->m_var_type_current = ObjType::FUNCTION;
        result->m_var_type_fixed = result->m_var_type_current;
        result->m_var_is_init = false;

    } else if(type) {
        result->m_var_type_current = typeFromString(type->getText().c_str(), ctx);
        result->m_var_type_fixed = result->m_var_type_current;
        if(result->is_tensor()) {
            std::vector<int64_t> dims;
            if(type->size()) {
                for (size_t i = 0; i < type->size(); i++) {
                    NL_CHECK(type->name(i).empty(), "Dimension named not supported!");
                    ObjPtr temp = CreateRVal(ctx, type->at(i).second);
                    if(!temp) {
                        NL_PARSER(type, "Term not found!");
                    }
                    if(!temp->is_integer()) {
                        NL_PARSER(type, "Term type not integer!");
                    }
                    dims.push_back(temp->GetValueAsInteger());
                }
            }
            result->m_value = torch::empty(dims, toTorchType(result->m_var_type_current));
            result->m_var_is_init = false;
        }
    }

    return result;
}

ObjPtr Context::CreateRVal(Context *ctx, const char *source, Object & local_vars) {
    TermPtr ast;
    Parser parser(ast);
    parser.Parse(source);
    return CreateRVal(ctx, ast, local_vars);
}

ObjPtr Context::CreateRVal(Context *ctx, TermPtr term, Object & local_vars) {

    ASSERT(term);

    ObjPtr result = nullptr;
    ObjPtr temp = nullptr;
    TermPtr field = nullptr;
    std::string full_name;

    result = Object::CreateNone();
    result->m_is_reference = term->m_is_ref;

    char *ptr;
    int64_t val_int;
    double val_dbl;
    ObjType type;
    bool has_error;
    std::vector<int64_t> sizes;
    switch(term->getTermID()) {
        case TermID::INTEGER:
            val_int = parseInteger(term->getText().c_str());
            NL_TYPECHECK(term, newlang::toString(typeFromLimit(val_int)), term->m_type_name); // Соответстствует ли тип значению?
            result->m_var_type_current = typeFromLimit(val_int);
            if(term->GetType()) {
                result->m_var_type_fixed = typeFromString(term->m_type_name, ctx);
                result->m_var_type_current = result->m_var_type_fixed;
            }
            result->m_value = torch::scalar_tensor(val_int, toTorchType(result->m_var_type_current));
            result->m_var_is_init = true;
            return result;
        case TermID::NUMBER:
            val_dbl = parseDouble(term->getText().c_str());
            NL_TYPECHECK(term, newlang::toString(typeFromLimit(val_dbl)), term->m_type_name); // Соответстствует ли тип значению?
            result->m_var_type_current = typeFromLimit(val_dbl);
            if(term->GetType()) {
                result->m_var_type_fixed = typeFromString(term->m_type_name, ctx);
                result->m_var_type_current = result->m_var_type_fixed;
            }
            result->m_value = torch::scalar_tensor(val_dbl, toTorchType(result->m_var_type_current));
            result->m_var_is_init = true;
            return result;
        case TermID::STRWIDE:
            result->m_var_type_current = ObjType::StrWide;
            result->m_wstr = utf8_decode(term->getText());
            result->m_var_is_init = true;
            return result;

        case TermID::STRCHAR:
            result->m_var_type_current = ObjType::StrChar;
            result->m_str = term->getText();
            result->m_var_is_init = true;
            return result;

            /*        case TermID::FIELD:
                        if(module && module->HasFunc(term->GetFullName().c_str())) {
                            // Если поле является функцией и она загружена
                            result = Object::CreateType(Object::Type::FUNCTION, term->GetFullName().c_str());
                            result->m_module = module;
                            result->m_is_const = term->m_is_const;
                            result->m_is_ref = term->m_is_ref;
                            return result;
                        }
                        if(!result) {
                            LOG_RUNTIME("Term '%s' not found!", term->toString().c_str());
                        }
                        return result;
             */
        case TermID::TERM:
            //            if(term->GetFullName().empty()) {
            //                return Object::CreateNone();
            //            }
            if(term->GetType()) {

                result->m_var_type_current = typeFromString(term->GetType()->m_text, ctx);
                result->m_var_type_fixed = result->m_var_type_current;
                result->m_var_is_init = false; // Нельзя считать значение

                // Check BuildInType
                has_error = false;
                typeFromString(term->GetType()->m_text, nullptr, &has_error);
                if(has_error) {
                    result->m_var_type_name = term->GetType()->m_text;
                }

                return result;

            }
            if(term->m_text.compare("_") == 0) {
                result->m_var_type_current = ObjType::None;
                result->m_var_is_init = false;
                return result;
            } else if(term->m_text.compare("$") == 0) {
                result->m_var_type_current = ObjType::Dict;
                result->m_var_name = "$";
                size_t pos = 0;
                while(ctx && pos < ctx->size()) {
                    if(ctx->at(pos).second.lock()) {
                        result->push_back(Object::Arg(ctx->at(pos).first));
                        pos++;
                    } else {
                        ctx->erase(pos);
                    }
                }
                result->m_var_is_init = true;
                return result;
            } else if(term->m_text.compare("@") == 0) {

            } else if(term->m_text.compare("%") == 0) {

            }

            if(isLocal(term->m_text.c_str())) {
                full_name = MakeName(term->m_text);
                return local_vars.at(full_name);
            } else {
                result = ctx->GetTerm(term->GetFullName().c_str(), term->isRef());
            }
            if(!result) {
                // Делать ислкючение или возвращать объект "ошибка" ?????
                LOG_RUNTIME("Term '%s' not found!", term->GetFullName().c_str());
            }
            result->m_var_is_init = true;

            field = term->m_right;
            while(field) {
                ASSERT(field->getTermID() == TermID::FIELD);
                result = result->at(field->getText());
                field = field->m_right;
                ASSERT(!field); // Нужно выполнять, а не просто получать значение поля
            }

            return result;

        case TermID::CALL:
            return ctx->CallByName(term, local_vars);

        case TermID::CLASS:
            if(!term->m_class_name.empty()) {
                // Создается перечисление, если тип у имени класса один из арифметических типов

                // Check BuildInType
                has_error = false;
                type = typeFromString(term->m_class_name, nullptr, &has_error);
                if(has_error) {
                    NL_PARSER(term, "Typename '%s' not base type!", term->m_class_name.c_str());
                }
                if(!isIntegralType(type, true)) {
                    NL_PARSER(term, "Enum must be intergal type, not '%s'!", term->m_class_name.c_str());
                }
                result->m_var_type_current = ObjType::Dict;
                val_int = 0;
                for (size_t i = 0; i < term->size(); i++) {
                    if(term->name(i).empty()) {
                        NL_PARSER(term, "The enum element must have a name!");
                    }

                    if((*term)[i]->m_text.empty()) {
                        temp = Object::CreateValue(val_int, type);
                    } else {
                        temp = CreateRVal(ctx, (*term)[i], local_vars);
                        val_int = temp->GetValueAsInteger();
                        NL_TYPECHECK(term, newlang::toString(typeFromLimit(val_int)), term->m_class_name); // Соответстствует ли тип значению?
                    }
                    temp->m_var_type_fixed = type;
                    result->push_back(temp, term->name(i).c_str());
                    val_int += 1;
                }
                return result;
            }

        case TermID::TENSOR:
        case TermID::DICT:
            result->m_var_type_current = ObjType::Dict;
            for (size_t i = 0; i < term->size(); i++) {
                if(term->name(i).empty()) {
                    result->push_back(CreateRVal(ctx, (*term)[i], local_vars));
                } else {
                    result->push_back(CreateRVal(ctx, (*term)[i], local_vars), term->name(i).c_str());
                }
            }
            if(term->getTermID() == TermID::TENSOR) {
                type = getSummaryTensorType(result, typeFromString(term->m_type_name, ctx));

                //                std::vector<int64_t> sizes = getTensorSizes(result.get());

                result->m_value = ConvertToTensor(result.get(), toTorchType(type));
                //                result->m_value = result->m_value.reshape(sizes);
                result->m_var_type_current = type;

                //                result->

                //                if(!canCastLimit(type, typeFromString(term->m_type_name))) {
                //                    LOG_RUNTIME("Can`t cast type '%s' to '%s'!", type, newlang::toString(typeFromString(term->m_type_name)));
                //                }
                // Обычное конвертирование словаря в тензор выполняется без контроля размерности,
                // а создание тензора из литерала должно выполняться с контролем размеров тензора
                //                if(result->size() == 0) {
                //                LOG_RUNTIME("Can`t create empty tensor!");
                //                }
                //                std::vector<int64_t> dims;
                //                calcTensorDims(result, dims);

                //                dims
                //                sizes = result->
                //
                //
                //
                //                        result->toType_(type);
            }
            result->m_var_is_init = true;
            return result;

        case TermID::ARGUMENT:

            val_int = IndexArg(term);
            if(val_int < local_vars.size()) {
                return local_vars.at(val_int).second;
            }
            LOG_RUNTIME("Argument '%s' not exist!", term->toString().c_str());


            //        case TermID::FUNCTION:
            //        case TermID::TRANSPARENT:
            //            ASSERT(ctx);
            //            
            //            if(term->getTermID() == TermID::FUNCTION) {
            //                result->m_var_type_current = ObjType::EVAL_FUNCTION;
            //            } else {
            //                result->m_var_type_current = ObjType::EVAL_TRANSP;
            //            }
            //            result->m_var_name = term->Left()
            //            result->m_func_source = term->Right();
            //            result->m_var_is_init = true;
            //            ctx->RegisterObject();
            //            return result;

    }
    LOG_RUNTIME("Fail create type %s from '%s'", newlang::toString(term->getTermID()), term->toString().c_str());
    return nullptr;

}
