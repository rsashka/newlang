#include "contrib/logger/logger.h"
#include "pch.h"

#include <context.h>
#include <newlang.h>
#include <term.h>
#include <types.h>
#include <filesystem>
#include <stdbool.h>

using namespace newlang;

/*
 Идентификатором объектов является его имя, причем у сессионных и глобальных
объектов оно присваивается автоматически даже для переменных литералов. Имена
локальных объектов перекрывают сессионные и глобальные, а сессионный объкт
перекрывает глобальный. Дубли локальных объектов не допускаются, т.к. имена
локальных переменных должны быть уникальные на уровне языка реализации. Дубли
сессионных объектов для текущей сессии не допускаются, но можно перекрывать
сессионные объекты более высокого уровня. Дубли глобальных объектов допускаются
без ограничений, а при создании дубля выводится предупреждение.

При наличии переменных с одним именем, но разным временем жизни, приоритет
доступпа будет следующим. обращение по имени объекта - доступ только к
локальному объекту (разрешение имени происходит во время компиляции) или будет
ошибка если объект отсутствует. обращение как к сессионному обекту (префикс $) -
если есть локальный объект, вернется локальный, иначе вернется сессионный или
будет ошибка если объект отсутствует. обращение как к глобальному обекту
(префикс @) - если есть локальный, вернется локальный, иначе вернется
сессионный, иначе вернется глобальный или будет ошибка если объект отсутствует.

Можно всегда указывать глобальный идентификатор объекта, то доступ будет в
зависимости от наличия локальных или сессионных переменных/объектов.

$.obj прямое обращения к сессионному объекту при наличии локального
@.obj прямое обращения к глобальному объекту при наличии локального или
сессионного

 */

std::map<std::string, Context::EvalFunction> Context::m_ops;
std::map<std::string, Context::EvalFunction> Context::m_builtin_calls;
std::map<std::string, ObjPtr> Context::m_types;
std::map<std::string, Context::FuncItem> Context::m_funcs;
Parser::MacrosStore Context::m_macros;
std::multimap<std::string, DocPtr> Docs::m_docs;

const char * Interrupt::Return = ":Return";
const char * Interrupt::Error = ":Error";
const char * Interrupt::Parser = ":ErrorParser";
const char * Interrupt::RunTime = ":ErrorRunTime";
const char * Interrupt::Signal = ":ErrorSignal";
const char * Interrupt::Abort = ":ErrorAbort";

Context::Context(RuntimePtr global) {
    m_runtime = global;

    if(Context::m_funcs.empty()) {

        VERIFY(CreateBuiltin("min(arg, ...)", (void *) &min, ObjType::PureFunc));
        VERIFY(CreateBuiltin("мин(arg, ...)", (void *) &min, ObjType::PureFunc));
        VERIFY(CreateBuiltin("max(arg, ...)", (void *) &max, ObjType::PureFunc));
        VERIFY(CreateBuiltin("макс(arg, ...)", (void *) &max, ObjType::PureFunc));

        //        VERIFY(CreateBuiltin("import(arg, module='', lazzy=0)", (void *) &import, ObjType::Function));
        //        VERIFY(CreateBuiltin("eval(string:String)", (void *) &eval, ObjType::Function));
        //        VERIFY(CreateBuiltin("exec(filename:String)", (void *) &exec, ObjType::Function));

        VERIFY(CreateBuiltin("help(...)", (void *) &help, ObjType::PureFunc));

    }

    if(Context::m_types.empty()) {

        VERIFY(RegisterTypeHierarchy(ObjType::None,{}));
        VERIFY(RegisterTypeHierarchy(ObjType::Any,{}));

        VERIFY(RegisterTypeHierarchy(ObjType::Arithmetic,{":Any"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Fraction,{":Arithmetic"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Tensor,{":Arithmetic"}));

        VERIFY(RegisterTypeHierarchy(ObjType::Integer,{":Tensor"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Bool,{":Integer"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Char,{":Integer"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Short,{":Integer"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Int,{":Integer"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Long,{":Integer"}));

        VERIFY(RegisterTypeHierarchy(ObjType::Number,{":Tensor"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Float,{":Number"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Double,{":Number"}));

        VERIFY(RegisterTypeHierarchy(ObjType::Complex,{":Tensor"}));
        VERIFY(RegisterTypeHierarchy(ObjType::ComplexFloat,{":Complex"}));
        VERIFY(RegisterTypeHierarchy(ObjType::ComplexDouble,{":Complex"}));

        VERIFY(RegisterTypeHierarchy(ObjType::String,{":Any"}));
        VERIFY(RegisterTypeHierarchy(ObjType::StrChar,{":String"}));
        VERIFY(RegisterTypeHierarchy(ObjType::StrWide,{":String"}));
        VERIFY(RegisterTypeHierarchy(ObjType::FmtChar,{":String"}));

        VERIFY(RegisterTypeHierarchy(ObjType::Dictionary,{":Any"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Class,{":Dictionary"}));

        VERIFY(RegisterTypeHierarchy(ObjType::Pointer,{":Any"})); // Указатели на машиннозависимую реализауию объектов

        VERIFY(RegisterTypeHierarchy(ObjType::Plain,{":Pointer"})); // Могут быть представленые в одном блоке памяти
        VERIFY(RegisterTypeHierarchy(ObjType::Enum,{":Plain", ":Integer"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Union,{":Plain", ":Dictionary"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Struct,{":Plain", ":Class"}));


        VERIFY(RegisterTypeHierarchy(ObjType::Eval,{":Any"})); // Может быть выполнен
        VERIFY(RegisterTypeHierarchy(ObjType::Function,{":Eval"}));
        VERIFY(RegisterTypeHierarchy(ObjType::NativeFunc,{":Function", ":Pointer"}));

        VERIFY(RegisterTypeHierarchy(ObjType::Other,{":Any"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Range,{":Other"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Ellipsis,{":Other"}));

        VERIFY(RegisterTypeHierarchy(ObjType::BLOCK,{":Eval"}));
        VERIFY(RegisterTypeHierarchy(ObjType::BLOCK_TRY,{":Eval"}));

        VERIFY(RegisterTypeHierarchy(ObjType::PureFunc,{":Function"}));
        VERIFY(RegisterTypeHierarchy(ObjType::SimplePureFunc,{":PureFunc"}));
        VERIFY(RegisterTypeHierarchy(ObjType::SimplePureOR,{":SimplePureFunc"}));
        VERIFY(RegisterTypeHierarchy(ObjType::SimplePureXOR,{":SimplePureFunc"}));
        VERIFY(RegisterTypeHierarchy(ObjType::SimplePureAND,{":SimplePureFunc"}));

        VERIFY(RegisterTypeHierarchy(ObjType::Type,{":Any"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Return,{":Any"}));

        VERIFY(RegisterTypeHierarchy(ObjType::Error,{":Return"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Break,{":Return"})); // Синонимы прерывания последовательности выполнения для совместимости
        VERIFY(RegisterTypeHierarchy(ObjType::Continue,{":Return"})); // со стндартными алгоритмическими приемами (синтаксический сахар)


        VERIFY(RegisterTypeHierarchy(ObjType::ErrorParser,{":Error"}));
        VERIFY(RegisterTypeHierarchy(ObjType::ErrorRunTime,{":Error"}));
        VERIFY(RegisterTypeHierarchy(ObjType::ErrorSignal,{":Error"}));

    }

    if(Context::m_builtin_calls.empty()) {
#define REGISTER_FUNC(name, func)                                                                                      \
    ASSERT(Context::m_builtin_calls.find(name) == Context::m_builtin_calls.end());                                     \
    Context::m_builtin_calls[name] = &Context::func_##func;

        NL_BUILTIN(REGISTER_FUNC);

#undef REGISTER_FUNC
    }

    if(Context::m_ops.empty()) {
#define REGISTER_OP(op, func)                                                                                          \
    ASSERT(Context::m_ops.find(op) == Context::m_ops.end());                                                           \
    Context::m_ops[op] = &Context::op_##func;

        NL_OPS(REGISTER_OP);

#undef REGISTER_OP
    }
}

bool Context::CreateBuiltin(const char *prototype, void *func, ObjType type) {
    ASSERT(prototype);
    ASSERT(func);

    std::string func_dump(prototype);
    func_dump += " := {};";

    TermPtr proto = Parser::ParseString(func_dump, &m_macros);
    ASSERT(proto->Left() && !proto->Left()->getText().empty());
    ObjPtr obj = Obj::CreateFunc(this, proto->Left(), type, proto->Left()->getText());

    obj->m_func_ptr = func;
    obj->m_var_is_init = true;

    auto found = m_funcs.find(proto->Left()->getText());
    if(found != m_funcs.end()) {
        LOG_DEBUG("Buildin function %s already exists!", proto->Left()->toString().c_str());
        return false;
    }

    Context::m_funcs[proto->Left()->getText()] = obj;
    return true;
}

inline ObjType newlang::typeFromString(const std::string type, Context *ctx, bool *has_error) {

    if(ctx) {
        return ctx->BaseTypeFromString(type, has_error);
    }

#define DEFINE_CASE(name, _)                    \
    else if (type.compare(":"#name) == 0) {     \
        return ObjType:: name;                  \
    }

    if(type.empty()) {
        return ObjType::None;
    } else if(type.compare("_") == 0) {
        return ObjType::None;
    }
    NL_TYPES(DEFINE_CASE)
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

    if(isGlobal(var->getName())) {
        var->getName() = var->getName().substr(1);
        m_global_terms.push_back(var, var->getName());
    }
    if(isLocal(var->getName())) {
        var->getName() = var->getName().substr(1);
    }
    push_back(var, var->getName());

    return var;
}

#ifndef _MSC_VER

void newlang::NewLangSignalHandler(int signal) {
    throw Interrupt("Signal SIGABRT received", Interrupt::Abort);
}
#endif

//#include "StdCapture.h"

ObjPtr Context::Eval(Context *ctx, TermPtr term, Obj *args, bool int_catch) {

    //    StdCapture Capture;
    //
    //    Capture.BeginCapture();

#ifndef _MSC_VER
    auto previous_handler = signal(SIGABRT, &NewLangSignalHandler);
#endif
    try {

        switch(term->m_id) {
            case TermID::END:
                return eval_END(ctx, term, args);

#define DEFINE_CASE(name)                                                                                              \
    case TermID::name:                                                                                                 \
        return eval_##name(ctx, term, args);

                NL_TERMS(DEFINE_CASE)

#undef DEFINE_CASE

        }
    } catch (Interrupt &obj) {

#ifndef _MSC_VER
        signal(SIGABRT, previous_handler);
#endif        

        ASSERT(obj.m_obj);

        if(int_catch && obj.m_obj->getType() == ObjType::Return) {

            ASSERT(obj.m_obj->size() == 1);
            return (*obj.m_obj)[0].second; // Возврат данных

        } else if(int_catch) {
            ASSERT(obj.m_obj);
            return obj.m_obj; // Прерывания анализуирются выше по уровню
        }

        throw; // Пробросить прерывание дальше
    }

#ifndef _MSC_VER
    signal(SIGABRT, previous_handler);
#endif

    return Obj::CreateNone();
}

ObjPtr Context::eval_END(Context *ctx, const TermPtr &term, Obj *args) {
    return Obj::CreateNone();
}

ObjPtr Context::eval_UNKNOWN(Context *ctx, const TermPtr &term, Obj *args) {
    ASSERT(term);
    LOG_RUNTIME("Fail EVAL: '%s'", term->toString().c_str());

    return nullptr;
}

/*
 *
 *
 */

ObjPtr Context::eval_BLOCK(Context *ctx, const TermPtr &term, Obj *args) {
    ASSERT(term && term->getTermID() == TermID::BLOCK);
    ObjPtr obj = Obj::CreateType(ObjType::BLOCK);
    obj->m_block_source = term;
    obj->m_var_is_init = true;

    if(term->size() && term->at(0).second->IsString()) {
        obj->m_help = Docs::Append(term->at(0).second->m_text);
    }

    return obj;
}

ObjPtr Context::eval_BLOCK_TRY(Context *ctx, const TermPtr &term, Obj *args) {
    ASSERT(term && term->getTermID() == TermID::BLOCK_TRY);
    ObjPtr obj = Obj::CreateType(ObjType::BLOCK_TRY);
    obj->m_block_source = term;
    obj->m_var_is_init = true;

    if(term->size() && term->at(0).second->IsString()) {
        obj->m_help = Docs::Append(term->at(0).second->m_text);
    }

    return obj;
}

ObjPtr Context::eval_CALL_BLOCK(Context *ctx, const TermPtr &term, Obj *args) {
    ASSERT(term && term->getTermID() == TermID::CALL_BLOCK);

    return CallBlock(ctx, term, args, false);
}

ObjPtr Context::eval_CALL_TRY(Context *ctx, const TermPtr &term, Obj *args) {
    ASSERT(term && term->getTermID() == TermID::CALL_TRY);

    return CallBlock(ctx, term, args, true);
}

ObjPtr Context::eval_MACRO(Context *ctx, const TermPtr &term, Obj *args) {
    LOG_ERROR("Macro %s not found!", term->toString().c_str());
    return Obj::CreateNone();
}

ObjPtr Context::eval_MACRO_BODY(Context *ctx, const TermPtr &term, Obj *args) {
    LOG_RUNTIME("eval_MACRO_BODY: %s", term->toString().c_str());

    return nullptr;
}

ObjPtr Context::eval_PARENT(Context *ctx, const TermPtr &term, Obj *args) {
    LOG_RUNTIME("eval_PARENT: %s", term->toString().c_str());

    return nullptr;
}

ObjPtr Context::eval_NEWLANG(Context *ctx, const TermPtr &term, Obj *args) {
    LOG_RUNTIME("eval_NEWLANG: %s", term->toString().c_str());

    return nullptr;
}

ObjPtr Context::eval_TYPE(Context *ctx, const TermPtr &term, Obj *local_vars) {
    return CreateRVal(ctx, term, local_vars);
}

ObjPtr Context::eval_TYPE_CALL(Context *ctx, const TermPtr &term, Obj *local_vars) {
    return CreateRVal(ctx, term, local_vars);
}

ObjPtr Context::eval_TYPENAME(Context *ctx, const TermPtr &term, Obj *args) {
    LOG_RUNTIME("TYPENAME Not implemented!");

    return nullptr;
}

inline ObjPtr Context::eval_INTEGER(Context *ctx, const TermPtr &term, Obj *args) {
    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_NUMBER(Context *ctx, const TermPtr &term, Obj *args) {
    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_COMPLEX(Context *ctx, const TermPtr &term, Obj *args) {
    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_EVAL(Context *ctx, const TermPtr &term, Obj *args) {
    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_FRACTION(Context *ctx, const TermPtr &term, Obj *args) {
    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_STRWIDE(Context *ctx, const TermPtr &term, Obj *args) {
    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_STRCHAR(Context *ctx, const TermPtr &term, Obj *args) {
    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_TENSOR(Context *ctx, const TermPtr &term, Obj *args) {

    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_NONE(Context *ctx, const TermPtr &term, Obj *args) {
    ASSERT(term && term->m_id == TermID::NONE);

    return Obj::CreateNone();
}

inline ObjPtr Context::eval_EMPTY(Context *ctx, const TermPtr &term, Obj *args) {
    LOG_RUNTIME("EMPTY Not implemented!");

    return nullptr;
}

ObjPtr Context::eval_TERM(Context *ctx, const TermPtr &term, Obj *args) {
    ASSERT(term && term->m_id == TermID::TERM);

    return CreateRVal(ctx, term, args);
}

ObjPtr Context::eval_CALL(Context *ctx, const TermPtr &term, Obj *args) {
    ASSERT(term && term->m_id == TermID::CALL);
    return CreateRVal(ctx, term, args);
}

ObjPtr Context::func_NOT_SUPPORT(Context *ctx, const TermPtr &term, Obj *args) {
    NL_PARSER(term, "Function or operator '%s' is not supported in interpreter mode!", term->m_text.c_str());

    return nullptr;
}

ObjPtr Context::eval_TEMPLATE(Context *ctx, const TermPtr &term, Obj *args) {
    LOG_RUNTIME("TEMPLATE Not implemented!");
    return nullptr;
}

ObjPtr Context::eval_COMMENT(Context *ctx, const TermPtr &term, Obj *args) {
    LOG_RUNTIME("COMMENT Not implemented!");

    return nullptr;
}

ObjPtr Context::eval_SYMBOL(Context *ctx, const TermPtr &term, Obj *args) {
    LOG_RUNTIME("SYMBOL '%s' Not implemented!", term->m_text.c_str());
    return nullptr;
}

ObjPtr Context::eval_NAMESPACE(Context *ctx, const TermPtr &term, Obj *args) {
    LOG_RUNTIME("NAMESPACE Not implemented!");
    return nullptr;
}

ObjPtr Context::CREATE_OR_ASSIGN(Context *ctx, const TermPtr &term, Obj *local_vars, CreateMode mode) {
    // Присвоить значение можно как одному термину, так и сразу нескольким при
    // раскрытии словаря: var1, var2, _ = ... func(); 
    // Первый и второй элементы словаря записывается в var1 и var2, а остальные элементы возвращаемого
    // словаря игнорируются (если они есть)
    // var1, var2 = ... func(); Если функция вернула словрь с двумя элементами, 
    // то их значения записываются в var1 и var2. Если в словаре было больше двух элементов, 
    // то первый записывается в var1, а оставшиеся в var2. !!!!!!!!!!!!!
    // var1, ..., var2 = ... func(); 
    // Первый элемент словаря записывается в var1, а последний в var2.

    ASSERT(term && (term->getTermID() == TermID::ASSIGN ||
            term->getTermID() == TermID::CREATE ||
            term->getTermID() == TermID::CREATE_OR_ASSIGN));
    ASSERT(term->Left());


    std::vector<TermPtr> list_term; // Список терминов для создания/присвоения значения
    std::vector<ObjPtr> list_obj; // Список реальных переменных ассоциированных с терминами

    TermPtr next = term->Left();
    while(next && next->getTermID() != TermID::END) {
        list_term.push_back(next);
        next = next->m_comma_seq;
    }

    ObjPtr result;
    bool is_ellipsis = false;
    for (auto & elem : list_term) {

        if(elem->getTermID() == TermID::ELLIPSIS) {

            //@todo добавить поддержку многоточия с левой стороный оператора присвоения
            NL_PARSER(elem, "Ellipsis on the left side in assignment not implemented!");

            if(is_ellipsis) {
                NL_PARSER(elem, "Multiple ellipsis on the left side of the assignment!");
            }
            is_ellipsis = true;
            result = Obj::CreateType(ObjType::Ellipsis);
        } else if(elem->getTermID() == TermID::NONE) {
            result = Obj::CreateNone();
        } else {
            auto found = ctx->find(elem->m_text);
            if(found == ctx->end() && mode == CreateMode::ASSIGN_ONLY) {
                NL_PARSER(elem, "Variable '%s' not found!", elem->m_text.c_str());
            }

            if(found != ctx->end()) {
                result = (*found).second.lock(); // Но она может быть возвращена как локальная
            }

            if(result && mode == CreateMode::CREATE_ONLY) {
                NL_PARSER(elem, "Variable '%s' already exists!", elem->m_text.c_str());
            }

            if(!term->Right()) { // Удаление глобальной переменной
                ctx->ListType::erase(found);
            } else {
                if(!result && (mode == CreateMode::ASSIGN_ONLY)) {
                    NL_PARSER(term->Left(), "Object '%s' not found!", term->Left()->m_text.c_str());
                }
                if(!result) {
                    result = CreateLVal(ctx, elem, local_vars);
                    if(!result) {
                        NL_PARSER(term->Left(), "Fail create lvalue object!");
                    }
                }
            }
        }
        list_obj.push_back(result);
    }

    if(!term->Right()) {
        // Для удаления переменных все сделано
        return result;
    }

    // Что присваиваем (правая часть выражения) - пока единичный объект
    // @todo В будущем можно будет сделать сахар для обмена значениями при одинаковом кол-ве объектов у оператора присваивания
    // a, b = b, a;   a, b, c = c, b, a; и т.д.
    if(term->Right() && term->Right()->getTermID() != TermID::ELLIPSIS && term->Right()->m_comma_seq) {
        NL_PARSER(term->Right()->Right(), "Multiple assignments not implemented!");
    }

    ObjPtr rval;
    if(term->Right()->getTermID() == TermID::ELLIPSIS) {
        ASSERT(term->Right()->Right());
        rval = Eval(ctx, term->Right()->Right(), local_vars, false);
    } else {
        rval = Eval(ctx, term->Right(), local_vars, false);
    }
    if(!rval) {
        NL_PARSER(term->Right(), "Object is missing or expression is not evaluated!");
    }

    ASSERT(list_obj.size() == list_term.size());

    if(term->Right()->getTermID() == TermID::ELLIPSIS) {
        if(rval->is_dictionary_type() || rval->is_tensor()) {
            if(rval->is_scalar()) {
                LOG_RUNTIME("Fail expand scalar!");
            }
            for (int i = 0; i < list_obj.size() - 1; i++) {
                if(list_term[i]->getTermID() != TermID::NONE) {
                    if(i < rval->size()) {
                        list_obj[i]->SetValue_((*rval)[i].second); //->Clone()
                    } else {
                        list_obj[i]->SetValue_(Obj::CreateNone());
                    }
                }
            }
            if(static_cast<int64_t> (list_obj.size()) - 1 < rval->size()) {
                // Удалить первые элементы
                rval->resize_(-(rval->size() - (static_cast<int64_t> (list_obj.size()) - 1)), nullptr);
            } else {
                rval->resize_(0, nullptr);
            }
            list_obj[list_obj.size() - 1]->SetValue_(rval->Clone());

            result = list_obj[list_obj.size() - 1];

        } else {
            LOG_RUNTIME("Fail expand type '%s'!", toString(rval->getType()));
        }
    } else {
        // Присвоеить единственное значение всем элементам с левой стороны оператора присовения

        for (int i = 0; i < list_obj.size(); i++) {
            if(isType(list_term[i]->m_text)) {

                // Новый тип
                if(ctx->m_types.find(list_term[i]->m_text) != ctx->m_types.end()) {
                    LOG_RUNTIME("Type name '%s' already exists!", list_term[i]->m_text.c_str());
                }

                result = rval->Clone();
                result->m_class_name = list_term[i]->m_text;
                result->m_class_parents.push_back(rval);

                ctx->m_types[list_term[i]->m_text] = result;

            } else if(list_term[i]->getTermID() == TermID::NONE) {
                // Skip
            } else {
                list_obj[i]->SetValue_(rval);
                if(list_obj[i]->m_var_type_current == ObjType::Function && (rval->m_var_type_current == ObjType::BLOCK || rval->m_var_type_current == ObjType::BLOCK_TRY)) {
                    list_obj[i]->m_var_type_current = ObjType::EVAL_FUNCTION;
                }
                result = list_obj[i];
            }
        }
    }
    return result;
}

ObjPtr Context::eval_ASSIGN(Context *ctx, const TermPtr &term, Obj * local_vars) {

    return CREATE_OR_ASSIGN(ctx, term, local_vars, CreateMode::ASSIGN_ONLY);
}

ObjPtr Context::eval_CREATE(Context *ctx, const TermPtr &term, Obj * local_vars) {

    return CREATE_OR_ASSIGN(ctx, term, local_vars, CreateMode::CREATE_ONLY);
}

ObjPtr Context::eval_CREATE_OR_ASSIGN(Context *ctx, const TermPtr &term, Obj * args) {

    return CREATE_OR_ASSIGN(ctx, term, args, CreateMode::CREATE_OR_ASSIGN);
}

ObjPtr Context::eval_APPEND(Context *ctx, const TermPtr &term, Obj * args) {
    LOG_RUNTIME("APPEND Not implemented!");

    return nullptr;
}

ObjPtr Context::eval_FUNCTION(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term && (term->getTermID() == TermID::FUNCTION || term->getTermID() == TermID::PUREFUNC ||
            term->getTermID() == TermID::SIMPLE_AND || term->getTermID() == TermID::SIMPLE_OR ||
            term->getTermID() == TermID::SIMPLE_XOR));
    ASSERT(term->Left());
    ASSERT(ctx);

    auto found = ctx->find(term->Left()->m_text);
    if(!term->Right()) {
        if(found != ctx->end()) {
            ctx->ListType::erase(found);
            return Obj::Yes();
        }
        return Obj::No();
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
        } else if(term->getTermID() == TermID::PUREFUNC) {
            lval->m_var_type_current = ObjType::SimplePureFunc;
        } else if(term->getTermID() == TermID::SIMPLE_AND) {
            lval->m_var_type_current = ObjType::SimplePureAND;
        } else if(term->getTermID() == TermID::SIMPLE_OR) {
            lval->m_var_type_current = ObjType::SimplePureOR;
        } else if(term->getTermID() == TermID::SIMPLE_XOR) {
            lval->m_var_type_current = ObjType::SimplePureXOR;
        } else {

            LOG_RUNTIME("Create function '%s' not implemented!", term->toString().c_str());
        }
        lval->m_var_type_fixed = lval->m_var_type_current;
        lval->m_var_is_init = true;
        lval->m_block_source = term->Right();
    }

    return ctx->RegisterObject(lval);
}

ObjPtr Context::eval_SIMPLE_AND(Context *ctx, const TermPtr &term, Obj * args) {

    return eval_FUNCTION(ctx, term, args);
}

ObjPtr Context::eval_SIMPLE_OR(Context *ctx, const TermPtr &term, Obj * args) {

    return eval_FUNCTION(ctx, term, args);
}

ObjPtr Context::eval_SIMPLE_XOR(Context *ctx, const TermPtr &term, Obj * args) {

    return eval_FUNCTION(ctx, term, args);
}

ObjPtr Context::eval_PUREFUNC(Context *ctx, const TermPtr &term, Obj * args) {

    return eval_FUNCTION(ctx, term, args);
}

/*
 *
 *
 */
ObjPtr Context::eval_ITERATOR(Context *ctx, const TermPtr &term, Obj * args) {
    return CreateRVal(ctx, term, args);
}

ObjPtr Context::eval_RANGE(Context *ctx, const TermPtr &term, Obj * args) {

    return CreateRVal(ctx, term, args);
}

ObjPtr Context::eval_ELLIPSIS(Context *ctx, const TermPtr &term, Obj * args) {
    LOG_RUNTIME("ELLIPSIS Not implemented!");

    return nullptr;
}

ObjPtr Context::eval_FILLING(Context* ctx, const TermPtr& term, Obj * args) {
    LOG_RUNTIME("FILLING Not implemented!");

    return nullptr;
}

ObjPtr Context::eval_ARGUMENT(Context *ctx, const TermPtr &term, Obj * args) {

    return CreateRVal(ctx, term, args);
}

ObjPtr Context::eval_ARGS(Context *ctx, const TermPtr &term, Obj * args) {

    return CreateRVal(ctx, term, args);
}

ObjPtr Context::eval_WHILE(Context *ctx, const TermPtr &term, Obj * args) {

    ASSERT(term->Left());
    ASSERT(term->Right());

    ObjPtr result = Obj::CreateNone();
    ObjPtr cond = Eval(ctx, term->Left(), args, false);
    while(cond->GetValueAsBoolean()) {

        try {

            LOG_DEBUG("result %s", result->toString().c_str());

            result = CreateRVal(ctx, term->Right(), args, false);
            cond = Eval(ctx, term->Left(), args, false);

        } catch (Interrupt &obj) {

            ASSERT(obj.m_obj);

            if(obj.m_obj->getType() == ObjType::Break) {
                break;
            } else if(obj.m_obj->getType() == ObjType::Continue) {
                continue;
            } else {
                throw; // Пробросить прерывание дальше
            }
        }
    }
    return result;
}

ObjPtr Context::eval_DOWHILE(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term->Left());
    ASSERT(term->Right());

    ObjPtr result;
    ObjPtr cond;
    do {
        try {

            result = CreateRVal(ctx, term->Left(), args, false);
            cond = Eval(ctx, term->Right(), args, false);

        } catch (Interrupt &obj) {

            ASSERT(obj.m_obj);

            if(obj.m_obj->getType() == ObjType::Break) {
                break;
            } else if(obj.m_obj->getType() == ObjType::Continue) {
                continue;
            } else {
                throw; // Пробросить прерывание дальше
            }
        }

    } while(cond->GetValueAsBoolean());

    return result;
}

ObjPtr Context::eval_FOLLOW(Context *ctx, const TermPtr &term, Obj * args) {

    /*
     * [cond] -> {expr};
     * [cond] -> {expr}, [_] -> {else};
     * 
     */

    for (int64_t i = 0; i < static_cast<int64_t> (term->m_follow.size()); i++) {

        ASSERT(term->m_follow[i]->Left());
        ObjPtr cond = Eval(ctx, term->m_follow[i]->Left(), args, false);

        if(cond->GetValueAsBoolean() || (i + 1 == term->m_follow.size() && cond->is_none_type())) {

            return CreateRVal(ctx, term->m_follow[i]->Right(), args, false);
        }
    }
    return Obj::CreateNone();
}

bool Context::MatchCompare(Obj &match, ObjPtr &value, MatchMode mode, Context *ctx) {
    switch(mode) {
        case MatchMode::MatchEqual:
            return match.op_equal(value);
        case MatchMode::MatchStrict:
            return match.op_accurate(value);
        case MatchMode::TYPE_NAME:
            return match.op_class_test(value, ctx);
        case MatchMode::TYPE_EQUAL:
            return match.op_duck_test(value, false);

        case MatchMode::TYPE_STRICT:
            return match.op_duck_test(value, true);
    }
    LOG_RUNTIME("Unknown pattern matching type %d!", static_cast<int> (mode));
}

bool Context::MatchEstimate(Obj &match, const TermPtr &match_item, MatchMode mode, Context *ctx, Obj * args) {

    ObjPtr cond = CreateRVal(ctx, match_item, args);

    if(cond->is_none_type() || MatchCompare(match, cond, mode, ctx)) {
        return true;
    } else {
        for (int i = 0; i < match_item->m_follow.size(); i++) {

            ASSERT(match_item->m_follow[i]);
            cond = CreateRVal(ctx, match_item->m_follow[i], args);

            if(cond->is_none_type() || MatchCompare(match, cond, mode, ctx)) {

                return true;
            }
        }
    }
    return false;
}

ObjPtr Context::eval_MATCHING(Context *ctx, const TermPtr &term, Obj * args) {
    /*
     * [match] ==> { # ~> ~~> ~~~> ===>
     *  [cond1] -> {expr};
     *  [cond2, cond3] -> {expr};
     *  [_] -> {expr};
     * };
     */

    ASSERT(term->Left());
    ASSERT(term->Right());

    MatchMode mode;
    if(term->m_text.compare("==>") == 0) {
        mode = MatchMode::MatchEqual;
    } else if(term->m_text.compare("===>") == 0) {
        mode = MatchMode::MatchStrict;
    } else if(term->m_text.compare("~>") == 0) {
        mode = MatchMode::TYPE_NAME;
    } else if(term->m_text.compare("~~>") == 0) {
        mode = MatchMode::TYPE_EQUAL;
    } else if(term->m_text.compare("~~~>") == 0) {
        mode = MatchMode::TYPE_STRICT;
    } else {
        NL_PARSER(term, "Unknown pattern matching type!");
    }

    ObjPtr value = CreateRVal(ctx, term->Left(), args);
    TermPtr list = term->Right();


    ASSERT(list->Left());
    ASSERT(list->Right());

    ObjPtr cond = CreateRVal(ctx, list->Left(), args);

    if(MatchEstimate(*cond.get(), list->Left(), mode, ctx, args)) {
        return CreateRVal(ctx, list->Right(), args);
    } else {
        for (int i = 0; i < list->m_block.size(); i++) {

            ASSERT(list->m_block[i]->Left());
            ASSERT(list->m_block[i]->Right());

            if(MatchEstimate(*cond.get(), list->m_block[i]->Left(), mode, ctx, args)) {

                return CreateRVal(ctx, list->m_block[i]->Right(), args);
            }
        }
    }
    return Obj::CreateNone();
}

ObjPtr Context::eval_INDEX(Context *ctx, const TermPtr &term, Obj * args) {
    LOG_RUNTIME("INDEX Not implemented!");

    return nullptr;
}

ObjPtr Context::eval_FIELD(Context *ctx, const TermPtr &term, Obj * args) {
    LOG_RUNTIME("FIELD Not implemented!");

    return nullptr;
}

ObjPtr Context::eval_DICT(Context *ctx, const TermPtr &term, Obj * args) {

    return CreateRVal(ctx, term, args);
}

ObjPtr Context::eval_SOURCE(Context *ctx, const TermPtr &term, Obj * args) {
    NL_PARSER(term, "Inclusion on the implementation language is not supported in interpreter mode!");

    return nullptr;
}

/*
 *
 *
 */

ObjPtr Context::eval_OPERATOR(Context *ctx, const TermPtr &term, Obj * args) {
    if(Context::m_ops.find(term->m_text) == Context::m_ops.end()) {

        LOG_RUNTIME("Eval op '%s' not exist!", term->m_text.c_str());
    }
    return (*Context::m_ops[term->m_text])(ctx, term, args);
}

/*
 *
 *
 */

ObjPtr Context::op_EQUAL(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args)->op_equal(Eval(ctx, term->Right(), args)) ? Obj::Yes() : Obj::No();
}

ObjPtr Context::op_ACCURATE(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args)->op_accurate(Eval(ctx, term->Right(), args)) ? Obj::Yes() : Obj::No();
}

ObjPtr Context::op_NE(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args)->operator!=(Eval(ctx, term->Right(), args)) ? Obj::Yes() : Obj::No();
}

ObjPtr Context::op_LT(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args)->operator<(Eval(ctx, term->Right(), args)) ? Obj::Yes() : Obj::No();
}

ObjPtr Context::op_GT(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args)->operator>(Eval(ctx, term->Right(), args)) ? Obj::Yes() : Obj::No();
}

ObjPtr Context::op_LE(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args)->operator<=(Eval(ctx, term->Right(), args)) ? Obj::Yes() : Obj::No();
}

ObjPtr Context::op_GE(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args)->operator>=(Eval(ctx, term->Right(), args)) ? Obj::Yes() : Obj::No();
}

/*
 *
 *
 */

ObjPtr Context::op_AND(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args)->op_bit_and(Eval(ctx, term->Right(), args), false);
}

ObjPtr Context::op_OR(Context *ctx, const TermPtr &term, Obj * args) {
    LOG_RUNTIME("OR Not implemented!");

    return nullptr;
}

ObjPtr Context::op_XOR(Context *ctx, const TermPtr &term, Obj * args) {
    LOG_RUNTIME("XOR Not implemented!");

    return nullptr;
}

ObjPtr Context::op_BIT_AND(Context *ctx, const TermPtr &term, Obj * args) {
    LOG_RUNTIME("BIT_AND Not implemented!");

    return nullptr;
}

ObjPtr Context::op_BIT_OR(Context *ctx, const TermPtr &term, Obj * args) {
    LOG_RUNTIME("BIT_OR Not implemented!");

    return nullptr;
}

ObjPtr Context::op_BIT_XOR(Context *ctx, const TermPtr &term, Obj * args) {
    LOG_RUNTIME("BIT_XOR Not implemented!");

    return nullptr;
}

ObjPtr Context::op_BIT_AND_(Context *ctx, const TermPtr &term, Obj * args) {
    LOG_RUNTIME("BIT_AND_ Not implemented!");

    return nullptr;
}

ObjPtr Context::op_BIT_OR_(Context *ctx, const TermPtr &term, Obj * args) {
    LOG_RUNTIME("BIT_OR_ Not implemented!");

    return nullptr;
}

ObjPtr Context::op_BIT_XOR_(Context *ctx, const TermPtr &term, Obj * args) {
    LOG_RUNTIME("BIT_XOR_ Not implemented!");

    return nullptr;
}

/*
 *
 *
 */
ObjPtr Context::op_PLUS(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Right());
    if(term->Left()) {

        return Eval(ctx, term->Left(), args)->operator+(Eval(ctx, term->Right(), args));
    }
    return Eval(ctx, term->Left(), args)->operator+();
}

ObjPtr Context::op_MINUS(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Right());
    if(term->Left()) {

        return Eval(ctx, term->Left(), args)->operator-(Eval(ctx, term->Right(), args));
    }
    return Eval(ctx, term->Left(), args)->operator-();
}

ObjPtr Context::op_DIV(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args)->operator/(Eval(ctx, term->Right(), args));
}

ObjPtr Context::op_DIV_CEIL(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args)->op_div_ceil(Eval(ctx, term->Right(), args));
}

ObjPtr Context::op_MUL(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args)->operator*(Eval(ctx, term->Right(), args));
}

ObjPtr Context::op_REM(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args)->operator%(Eval(ctx, term->Right(), args));
}

ObjPtr Context::op_POW(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args)->op_pow(Eval(ctx, term->Right(), args));
}

/*
 *
 *
 */
ObjPtr Context::op_PLUS_(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args)->operator+=(Eval(ctx, term->Right(), args));
}

ObjPtr Context::op_MINUS_(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args)->operator-=(Eval(ctx, term->Right(), args));
}

ObjPtr Context::op_DIV_(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args)->operator/=(Eval(ctx, term->Right(), args));
}

ObjPtr Context::op_DIV_CEIL_(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args)->op_div_ceil_(Eval(ctx, term->Right(), args));
}

ObjPtr Context::op_MUL_(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args)->operator*=(Eval(ctx, term->Right(), args));
}

ObjPtr Context::op_REM_(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args)->operator%=(Eval(ctx, term->Right(), args));
}

ObjPtr Context::op_POW_(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args)->op_pow_(Eval(ctx, term->Right(), args));
}

ObjPtr Context::op_CONCAT(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args)->op_concat_(Eval(ctx, term->Right(), args), ConcatMode::Append);
}

ObjPtr Context::op_TYPE_EQ(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    if(isType(term->Right()->GetFullName())) {
        return Obj::CreateBool(Eval(ctx, term->Left(), args)->op_class_test(term->Right()->GetFullName().c_str(), ctx));
    }
    return Obj::CreateBool(Eval(ctx, term->Left(), args)->op_class_test(Eval(ctx, term->Right(), args), ctx));
}

ObjPtr Context::op_TYPE_EQ2(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Obj::CreateBool(Eval(ctx, term->Left(), args)->op_duck_test(Eval(ctx, term->Right(), args), false));
}

ObjPtr Context::op_TYPE_EQ3(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Obj::CreateBool(Eval(ctx, term->Left(), args)->op_duck_test(Eval(ctx, term->Right(), args), true));
}

ObjPtr Context::op_TYPE_NE(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    if(isType(term->Right()->GetFullName())) {
        return Obj::CreateBool(!Eval(ctx, term->Left(), args)->op_class_test(term->Right()->GetFullName().c_str(), ctx));
    }
    return Obj::CreateBool(!Eval(ctx, term->Left(), args)->op_class_test(Eval(ctx, term->Right(), args), ctx));
}

ObjPtr Context::op_TYPE_NE2(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Obj::CreateBool(!Eval(ctx, term->Left(), args)->op_duck_test(Eval(ctx, term->Right(), args), false));
}

ObjPtr Context::op_TYPE_NE3(Context *ctx, const TermPtr &term, Obj * args) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Obj::CreateBool(!Eval(ctx, term->Left(), args)->op_duck_test(Eval(ctx, term->Right(), args), true));
}

/*
 *
 */
ObjPtr Context::op_RSHIFT(Context *ctx, const TermPtr &term, Obj * args) {
    LOG_RUNTIME("RSHIFT Not implemented!");

    return nullptr;
}

ObjPtr Context::op_LSHIFT(Context *ctx, const TermPtr &term, Obj * args) {
    LOG_RUNTIME("LSHIFT Not implemented!");

    return nullptr;
}

ObjPtr Context::op_RSHIFT_(Context *ctx, const TermPtr &term, Obj * args) {
    LOG_RUNTIME("RSHIFT_ Not implemented!");

    return nullptr;
}

ObjPtr Context::op_LSHIFT_(Context *ctx, const TermPtr &term, Obj * args) {
    LOG_RUNTIME("LSHIFT_ Not implemented!");

    return nullptr;
}

ObjPtr Context::op_RRSHIFT(Context *ctx, const TermPtr &term, Obj * args) {
    LOG_RUNTIME("RRSHIFT Not implemented!");

    return nullptr;
}

ObjPtr Context::op_RRSHIFT_(Context *ctx, const TermPtr &term, Obj * args) {
    LOG_RUNTIME("RRSHIFT_ Not implemented!");

    return nullptr;
}

ObjPtr Context::op_SPACESHIP(Context *ctx, const TermPtr &term, Obj * args) {
    LOG_RUNTIME("SPACESHIP Not implemented!");

    return nullptr;
}

/*
 *
 *
 */

ObjPtr Context::eval_EXIT(Context *ctx, const TermPtr &term, Obj * args) {

    ObjPtr ret = nullptr;
    bool is_return_data = (bool)term->Right();

    if(is_return_data) {
        if(isType(term->Right()->GetFullName())) {
            ret = CreateRVal(ctx, term->Right(), args, false);
        } else {
            ret = ctx->GetTypeFromString(Interrupt::Return);
            ret = ret->Call(ctx, Obj::Arg(CreateRVal(ctx, term->Right(), args)));
        }
    } else {
        ret = ctx->GetTypeFromString(Interrupt::Return);
        ret = ret->Call(ctx, Obj::Arg(Obj::CreateNone()));
    }
    ASSERT(ret);
    throw Interrupt(ret);
}

ObjPtr Context::CallBlock(Context *ctx, const TermPtr &block, Obj * local_vars, bool int_catch) {
    ObjPtr result = Obj::CreateNone();
    std::string type_return(Interrupt::Return);
    try {
        if(!block->m_block.empty()) {
            for (size_t i = 0; i < block->m_block.size(); i++) {
                result = Eval(ctx, block->m_block[i], local_vars, false);
            }
        } else {
            result = Eval(ctx, block, local_vars, false);
        }
    } catch (Interrupt &obj) {

        if(!int_catch) {
            throw;
        }

        ASSERT(obj.m_obj);

        if(!block->m_class_name.empty()) {
            type_return = block->m_class_name;
        }
        if(obj.m_obj->op_class_test(type_return.c_str(), ctx)) {
            if(block->m_class_name.empty()) {
                // Только при возврате значения :Return
                ASSERT(obj.m_obj->size() == 1);
                return (*obj.m_obj)[0].second;
            }
        } else if(!block->m_class_name.empty()) {
            throw; // Объект возврата не соответствует требуемому типу
        }
        result = obj.m_obj;
    }
    return result;
}

ObjPtr Context::EvalBlockAND(Context *ctx, const TermPtr &block, Obj * local_vars) {
    ObjPtr result = nullptr;
    if(block->GetTokenID() == TermID::BLOCK) {
        for (size_t i = 0; i < block->m_block.size(); i++) {
            result = Eval(ctx, block->m_block[i], local_vars, false);
            if(!result || !result->GetValueAsBoolean()) {
                return Obj::No();
            }
        }
    } else {
        result = Eval(ctx, block, local_vars, false);
    }
    if(!result || !result->GetValueAsBoolean()) {

        return Obj::No();
    }
    return Obj::Yes();
}

ObjPtr Context::EvalBlockOR(Context *ctx, const TermPtr &block, Obj * local_vars) {
    ObjPtr result = nullptr;
    if(block->GetTokenID() == TermID::BLOCK) {
        for (size_t i = 0; i < block->m_block.size(); i++) {
            result = Eval(ctx, block->m_block[i], local_vars, false);
            if(result && result->GetValueAsBoolean()) {
                return Obj::Yes();
            }
        }
    } else {
        result = Eval(ctx, block, local_vars, false);
    }
    if(result && result->GetValueAsBoolean()) {

        return Obj::Yes();
    }
    return Obj::No();
}

ObjPtr Context::EvalBlockXOR(Context *ctx, const TermPtr &block, Obj * local_vars) {
    ObjPtr result;
    size_t xor_counter = 0;
    if(block->GetTokenID() == TermID::BLOCK) {
        for (size_t i = 0; i < block->m_block.size(); i++) {
            result = Eval(ctx, block->m_block[i], local_vars, false);
            if(result && result->GetValueAsBoolean()) {
                xor_counter++;
            }
        }
    } else {
        result = Eval(ctx, block, local_vars, false);
        if(result && result->GetValueAsBoolean()) {

            xor_counter++;
        }
    }
    // Результат равен 0, если нет операндов, равных 1, либо их чётное количество.
    return (xor_counter & 1) ? Obj::Yes() : Obj::No();
}

ObjPtr Context::CreateNative(const char *proto, const char *module, bool lazzy, const char *mangle_name) {
    TermPtr term;
    try {
        // Термин или термин + тип парсятся без ошибок
        term = Parser::ParseString(proto, &m_macros);
    } catch (std::exception &) {
        try {
            std::string func(proto);
            func += ":={}";
            term = Parser::ParseString(func, &m_macros)->Left();
        } catch (std::exception &e) {

            LOG_RUNTIME("Fail parsing prototype '%s'!", e.what());
        }
    }
    return CreateNative(term, module, lazzy, mangle_name);
}

ObjPtr Context::CreateNative(TermPtr proto, const char *module, bool lazzy, const char *mangle_name) {

    NL_CHECK(proto, "Fail prototype native function!");
    NL_CHECK((module == nullptr || (module && *module == '\0')) || m_runtime,
            "You cannot load a module '%s' without access to the runtime context!", module);

    ObjPtr result;
    ObjType type = ObjType::None;
    if(proto->GetTokenID() == TermID::TERM) {
        if(proto->m_type_name.empty()) {
            LOG_RUNTIME("Cannot create native variable without specifying the type!");
        }

        type = typeFromString(proto->m_type_name, this);
        switch(type) {
            case ObjType::Bool:
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
    } else {
        LOG_RUNTIME("Native type undefined! '%s'", proto->toString().c_str());
    }

    result = Obj::CreateType(type);
    result->m_var_type_fixed = type; // Тип определен и не может измениться в дальнейшем

    *const_cast<TermPtr *> (&result->m_func_proto) = proto;
//    result->m_func_abi = abi;

    if(mangle_name) {
        result->m_func_mangle_name = mangle_name;
    }
    if(module) {
        result->m_module_name = module;
    }
    if(lazzy) {
        result->m_func_ptr = nullptr;
    } else {
        result->m_func_ptr = m_runtime->GetNativeAddr(
                result->m_func_mangle_name.empty() ? proto->m_text.c_str() : result->m_func_mangle_name.c_str(), module);
        if(result->is_function() || type == ObjType::Pointer) {
            NL_CHECK(result->m_func_ptr, "Error getting address '%s' from '%s'!", proto->toString().c_str(), module);
        } else if(result->m_func_ptr && result->is_tensor()) {
            result->m_value = torch::from_blob(result->m_func_ptr, {
            }, toTorchType(type));
            result->m_var_is_init = true;
        } else {

            LOG_RUNTIME("Fail CreateNative for object %s", proto->toString().c_str());
        }
    }
    return result;
}

std::string RunTime::GetLastErrorMessage() {
#ifndef _MSC_VER
    return std::string(strerror(errno));
#else
    wchar_t buffer[256];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, sizeof (buffer), NULL);
    return utf8_encode(buffer);
#endif        
}

void *RunTime::GetNativeAddr(const char *name, const char *module) {
//    if(module && module[0]) {
//        if(m_modules.find(module) == m_modules.end()) {
//            LoadModule(module, false, nullptr);
//        }
//        if(m_modules.find(module) == m_modules.end()) {
//            LOG_WARNING("Fail load module '%s'!", module);
//
//            return nullptr;
//        }
//
////#ifndef _MSC_VER
//
//        return GetDirectAddressFromLibrary(m_modules[module]->GetHandle(), name);
////#else
//        //return static_cast<void *> (::GetProcAddress(m_modules[module]->GetHandle(), name));
////#endif
//    }

//#ifndef _MSC_VER
    //    ASSERT(m_llvm_engine);

    //    LOG_DEBUG("getAddressToGlobalIfAvailable( %s ) = %ld", "var_long", m_llvm_engine->getAddressToGlobalIfAvailable("var_long"));
    //    LOG_DEBUG("getGlobalValueAddress( %s ) = %ld", "var_long", m_llvm_engine->getGlobalValueAddress("var_long"));
    //    LOG_DEBUG("getPointerToNamedFunction( %s ) = %ld", "var_long", (long)m_llvm_engine->getPointerToNamedFunction("var_long", false));
    //    
    //    LOG_DEBUG("getAddressToGlobalIfAvailable( %s ) = %ld", "_var_long", m_llvm_engine->getAddressToGlobalIfAvailable("_var_long"));
    //    LOG_DEBUG("getGlobalValueAddress( %s ) = %ld", "_var_long", m_llvm_engine->getGlobalValueAddress("_var_long"));
    //    LOG_DEBUG("getPointerToNamedFunction( %s ) = %ld", "_var_long", (long)m_llvm_engine->getPointerToNamedFunction("_var_long", false));
    //    
    //    LOG_DEBUG("getAddressToGlobalIfAvailable( %s ) = %ld", "func_export", m_llvm_engine->getAddressToGlobalIfAvailable("func_export"));
    //    LOG_DEBUG("getGlobalValueAddress( %s ) = %ld", "func_export", m_llvm_engine->getGlobalValueAddress("func_export"));
    //    LOG_DEBUG("getPointerToNamedFunction( %s ) = %ld", "func_export", (long)m_llvm_engine->getPointerToNamedFunction("func_export", false));
    //    
    //    LOG_DEBUG("getAddressToGlobalIfAvailable( %s ) = %ld", "_func_export", m_llvm_engine->getAddressToGlobalIfAvailable("_func_export"));
    //    LOG_DEBUG("getGlobalValueAddress( %s ) = %ld", "_func_export", m_llvm_engine->getGlobalValueAddress("_func_export"));
    //    LOG_DEBUG("getPointerToNamedFunction( %s ) = %ld", "_func_export", (long)m_llvm_engine->getPointerToNamedFunction("_func_export", false));
    //
    //    
    //    LOG_DEBUG("getAddressToGlobalIfAvailable( %s ) = %ld", name, m_llvm_engine->getAddressToGlobalIfAvailable(name));
    //    LOG_DEBUG("getGlobalValueAddress( %s ) = %ld", name, m_llvm_engine->getGlobalValueAddress(name));
    //    LOG_DEBUG("getPointerToNamedFunction( %s ) = %ld", name, (long)m_llvm_engine->getPointerToNamedFunction(name, false));

    //m_llvm_engine->getPointerToNamedFunction(name, false);
    return GetDirectAddressFromLibrary(nullptr, name);
    //    return ::dlsym(::dlopen(nullptr, RTLD_NOW | RTLD_GLOBAL), name);
//#else
//    void *result = static_cast<void *> (::GetProcAddress(GetModuleHandle(nullptr), name));
    //if(result) {
      //  return result;
//    }
  //  return static_cast<void *> (::GetProcAddress((HMODULE) m_msys, name));
//#endif
}

void Context::CleanUp() {
    auto iter = begin();
    while(iter != end()) {
        if(iter->second.expired()) {
            iter = ListType::erase(iter);
        } else {
            iter++;
        }
    }
}

ObjPtr Context::FindSessionTerm(const char *name, bool current_only) {
    CleanUp();
    auto found = find(MakeName(name));
    while(found != end()) {
        ObjPtr obj = found->second.lock();
        if(obj) {

            return obj;
        }
        ListType::erase(found);
        found++;
    }
    return nullptr;
}

/*
 * обращение по имени - доступ только к локальному объекту (разрешение имени во
 * время компиляции). обращение как к сессионному обекту - если есть локальный,
 * будет локальный (разрешение имени во время компиляции), иначе вернется
 * сессионный или будет ошибка если объект отсутствует. обращение как к
 * глобальному обекту - если есть локальный, будет локальный (разрешение имени
 * во время компиляции), иначе вернется сессионный, иначе вернется глобальный
 * или будет ошибка если объект отсутствует. Можно всегда общащсять как к
 * глобальному объекту, а доступ будет в зависимости от наличия локальных или
 * сессионных переменных/объектов.
 */
ObjPtr Context::FindTerm(const std::string name) {
    ObjPtr result = FindSessionTerm(name.c_str());
    if(!result && isType(name)) {
        return GetTypeFromString(name);
    }

    if(!result) {
        return GetObject(name.c_str());
    }

    if(result || isLocalAny(name.c_str()) || isLocal(name)) {

        return result;
    }
    return FindGlobalTerm(name);
}

ObjPtr Context::GetTerm(const std::string name, bool is_ref) {
    if(isType(name)) {

        return GetTypeFromString(name);
    }
    return FindTerm(name);
}

std::string newlang::GetFileExt(const char *str) {
    std::string filename(str);
    std::string::size_type idx = filename.rfind('.');
    if(idx != std::string::npos) {

        return filename.substr(idx);
    }
    return std::string("");
}

std::string newlang::AddDefaultFileExt(const char *str, const char *ext_default) {
    std::string filename(str);
    std::string file_ext = GetFileExt(str);
    if(file_ext.empty() && !filename.empty() && filename.compare(".") != 0) {

        filename.append(ext_default);
    }
    return filename;
}

std::string newlang::ReplaceFileExt(const char *str, const char *ext_old, const char *ext_new) {
    std::string filename(str);
    std::string file_ext = GetFileExt(str);
    if(file_ext.compare(ext_old) == 0) {
        filename = filename.substr(0, filename.length() - file_ext.length());
    }
    file_ext = GetFileExt(filename.c_str());
    if(file_ext.compare(".") != 0 && file_ext.compare(ext_new) != 0 && !filename.empty() &&
            filename.compare(".") != 0) {

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

ObjPtr Context::CreateLVal(Context *ctx, TermPtr term, Obj * args) {

    ASSERT(ctx);
    ASSERT(term);
    ASSERT(!term->m_text.empty());

    ctx->CleanUp();

    auto iter = ctx->find(term->m_text);

    if(iter != ctx->end()) {
        ObjPtr obj = (*iter).second.lock();
        if(obj) {
            return obj;
        }
    }

    //    if (ctx->select(term->m_text).complete()) {
    //        // Объект должен отсутствовать
    //        NL_PARSER(term, "Object '%s' already exists!", term->m_text.c_str());
    //    }

    ObjPtr result = Obj::CreateNone();
    result->m_var_is_init = false;
    result->m_var_name = term->m_text;

    *const_cast<TermPtr *> (&result->m_func_proto) = term;

    TermPtr type = term->GetType();
    if(term->IsFunction() || term->getTermID() == TermID::CALL) {

        result->m_var_type_current = ObjType::Function;
        result->m_var_type_fixed = result->m_var_type_current;
        *const_cast<TermPtr *> (&result->m_func_proto) = term;
    } else if(type) {
        result->m_var_type_current = typeFromString(type->getText().c_str(), ctx);
        result->m_var_type_fixed = result->m_var_type_current;
        if(result->is_tensor()) {
            std::vector<int64_t> dims;
            if(type->m_dims.size()) {
                for (size_t i = 0; i < type->m_dims.size(); i++) {
                    NL_CHECK(type->m_dims[i]->getName().empty(), "Dimension named not supported!");
                    ObjPtr temp = CreateRVal(ctx, type->m_dims[i]);
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
        }
    }
    if(!isType(term->m_text)) {
        ctx->RegisterObject(result);
    }
    return result;
}

ObjPtr Context::CreateRVal(Context *ctx, const char *source, Obj * local_vars, bool no_catch) {
    TermPtr ast;
    Parser parser(ast);
    parser.Parse(source);

    return CreateRVal(ctx, ast, local_vars, no_catch);
}

void Context::ItemTensorEval_(torch::Tensor &tensor, c10::IntArrayRef shape, std::vector<Index> &ind, const int64_t pos,
        ObjPtr &obj, ObjPtr & args) {
    ASSERT(pos < static_cast<int64_t> (ind.size()));
    if(pos + 1 < static_cast<int64_t> (ind.size())) {
        for (ind[pos] = 0; ind[pos].integer() < shape[pos]; ind[pos] = ind[pos].integer() + 1) {
            ItemTensorEval_(tensor, shape, ind, pos + 1, obj, args);
        }
    } else {

        at::Scalar value;
        ObjType type = fromTorchType(tensor.scalar_type());

        for (ind[pos] = 0; ind[pos].integer() < shape[pos]; ind[pos] = ind[pos].integer() + 1) {

            switch(type) {
                case ObjType::Char:
                case ObjType::Short:
                case ObjType::Int:
                case ObjType::Long:
                    value = at::Scalar(obj->Call(this)->GetValueAsInteger()); // args
                    tensor.index_put_(ind, value);
                    break;
                case ObjType::Float:
                case ObjType::Double:
                    value = at::Scalar(obj->Call(this)->GetValueAsNumber()); // args
                    tensor.index_put_(ind, value);

                    break;
                default:
                    ASSERT(!"Not implemented!");
            }
        }
    }
}

void Context::ItemTensorEval(torch::Tensor &self, ObjPtr obj, ObjPtr args) {
    if(self.dim() == 0) {

        signed char *ptr_char = nullptr;
        int16_t *ptr_short = nullptr;
        int32_t *ptr_int = nullptr;
        int64_t *ptr_long = nullptr;
        float *ptr_float = nullptr;
        double *ptr_double = nullptr;

        switch(fromTorchType(self.scalar_type())) {
            case ObjType::Char:
                ptr_char = self.data_ptr<signed char>();
                ASSERT(ptr_char);
                *ptr_char = static_cast<signed char> (obj->Call(this)->GetValueAsInteger());
                return;
            case ObjType::Short:
                ptr_short = self.data_ptr<int16_t>();
                ASSERT(ptr_short);
                *ptr_short = static_cast<int16_t> (obj->Call(this)->GetValueAsInteger());
                return;
            case ObjType::Int:
                ptr_int = self.data_ptr<int32_t>();
                ASSERT(ptr_int);
                *ptr_int = static_cast<int32_t> (obj->Call(this)->GetValueAsInteger());
                return;
            case ObjType::Long:
                ptr_long = self.data_ptr<int64_t>();
                ASSERT(ptr_long);
                *ptr_long = static_cast<int64_t> (obj->Call(this)->GetValueAsInteger());
                return;
            case ObjType::Float:
                ptr_float = self.data_ptr<float>();
                ASSERT(ptr_float);
                *ptr_float = static_cast<float> (obj->Call(this)->GetValueAsNumber());
                return;
            case ObjType::Double:
                ptr_double = self.data_ptr<double>();
                ASSERT(ptr_double);
                *ptr_double = static_cast<double> (obj->Call(this)->GetValueAsNumber());
                return;
        }

        ASSERT(!"Not implemented!");
    } else {

        c10::IntArrayRef shape = self.sizes(); // Кол-во эментов в каждом измерении
        std::vector<Index> ind(shape.size(),
                0); // Счетчик обхода всех эелемнтов тензора
        ItemTensorEval_(self, shape, ind, 0, obj, args);
    }
}

std::vector<int64_t> GetTensorShape(Context *ctx, TermPtr type, Obj * local_vars) {
    std::vector<int64_t> result(type->size());
    for (int i = 0; i < type->size(); i++) {
        ObjPtr temp = ctx->CreateRVal(ctx, type->at(i).second, local_vars);
        if(temp->is_integer() || temp->is_bool_type()) {
            result[i] = temp->GetValueAsInteger();
        } else {
            NL_PARSER(type->at(i).second, "Measurement dimension can be an integer only!");
        }
        if(result[i] <= 0) {

            NL_PARSER(type->at(i).second, "Dimension size can be greater than zero!");
        }
    }
    return result;
}

std::vector<Index> Context::MakeIndex(Context *ctx, TermPtr term, Obj * local_vars) {

    // `at::indexing::TensorIndex` is used for converting C++ tensor indices such
    // as
    // `{None, "...", Ellipsis, 0, true, Slice(1, None, 2), torch::tensor({1,
    // 2})}` into its equivalent `std::vector<TensorIndex>`, so that further
    // tensor indexing operations can be performed using the supplied indices.
    //
    // There is one-to-one correspondence between Python and C++ tensor index
    // types: Python                  | C++
    // -----------------------------------------------------
    // `None`                  | `at::indexing::None`
    // `Ellipsis`              | `at::indexing::Ellipsis`
    // `...`                   | `"..."`
    // `123`                   | `123`
    // `True` / `False`        | `true` / `false`
    // `:`                     | `Slice()` / `Slice(None, None)`
    // `::`                    | `Slice()` / `Slice(None, None, None)`
    // `1:`                    | `Slice(1, None)`
    // `1::`                   | `Slice(1, None, None)`
    // `:3`                    | `Slice(None, 3)`
    // `:3:`                   | `Slice(None, 3, None)`
    // `::2`                   | `Slice(None, None, 2)`
    // `1:3`                   | `Slice(1, 3)`
    // `1::2`                  | `Slice(1, None, 2)`
    // `:3:2`                  | `Slice(None, 3, 2)`
    // `1:3:2`                 | `Slice(1, 3, 2)`
    // `torch.tensor([1, 2])`) | `torch::tensor({1, 2})`

    std::vector<Index> result;

    if(!term->size()) {
        NL_PARSER(term, "Index not found!");
    }
    for (int i = 0; i < term->size(); i++) {
        if(!term->name(i).empty() || (term->at(i).second && term->at(i).second->IsString())) {
            NL_PARSER(term, "Named index not support '%d'!", i);
        }
        if(!term->at(i).second) {
            NL_PARSER(term, "Empty index '%d'!", i);
        }

        if(term->at(i).second->getTermID() == TermID::ELLIPSIS) {
            result.push_back(Index("..."));
        } else {

            ObjPtr temp = ctx->CreateRVal(ctx, term->at(i).second, local_vars);

            if(temp->is_none_type()) {

                result.push_back(Index(at::indexing::None));
            } else if(temp->is_integer() || temp->is_bool_type()) {

                if(temp->is_scalar()) {
                    result.push_back(Index(temp->GetValueAsInteger()));
                } else if(temp->m_value.dim() == 1) {
                    result.push_back(Index(temp->m_value));
                } else {
                    NL_PARSER(term->at(i).second, "Extra dimensions index not support '%d'!", i);
                }
            } else if(temp->is_range()) {

                int64_t start = temp->at("start").second->GetValueAsInteger();
                int64_t stop = temp->at("stop").second->GetValueAsInteger();
                int64_t step = temp->at("step").second->GetValueAsInteger();

                result.push_back(Index(at::indexing::Slice(start, stop, step)));
            } else {

                NL_PARSER(term->at(i).second, "Fail tensor index '%d'!", i);
            }
        }
    }
    return result;
}

ObjPtr Context::CreateRVal(Context *ctx, TermPtr term, Obj * local_vars, bool int_catch) {

    if(!term) {
        ASSERT(term);
    }
    ASSERT(local_vars);

    ObjPtr result = nullptr;
    ObjPtr temp = nullptr;
    ObjPtr args = nullptr;
    ObjPtr value = nullptr;
    TermPtr field = nullptr;
    std::string full_name;

    result = Obj::CreateNone();
    result->m_is_reference = term->m_is_ref;

    int64_t val_int;
    double val_dbl;
    ObjType type;
    bool has_error;
    std::vector<int64_t> sizes;
    at::Scalar torch_scalar;
    switch(term->getTermID()) {
        case TermID::INTEGER:
            val_int = parseInteger(term->getText().c_str());
            NL_TYPECHECK(term, newlang::toString(typeFromLimit(val_int)),
                    term->m_type_name); // Соответстствует ли тип значению?
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
            NL_TYPECHECK(term, newlang::toString(typeFromLimit(val_dbl)),
                    term->m_type_name); // Соответстствует ли тип значению?
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
                            result = Obj::CreateType(Obj::Type::FUNCTION,
               term->GetFullName().c_str()); result->m_module = module;
                            result->m_is_const = term->m_is_const;
                            result->m_is_ref = term->m_is_ref;
                            return result;
                        }
                        if(!result) {
                            LOG_RUNTIME("Term '%s' not found!",
               term->toString().c_str());
                        }
                        return result;
             */

            //@todo Что делать с пустыми значениями? Это None ???
        case TermID::EMPTY:
            result->m_var_type_current = ObjType::None;
            result->m_var_is_init = false;
            return result;

        case TermID::TERM:
            if(term->GetType()) {

                result->m_var_type_current = typeFromString(term->GetType()->m_text, ctx);
                result->m_var_type_fixed = result->m_var_type_current;
                result->m_var_is_init = false; // Нельзя считать значение

                // Check BuildInType
                has_error = false;
                typeFromString(term->GetType()->m_text, nullptr, &has_error);
                if(has_error) {
                    result->m_class_name = term->GetType()->m_text;
                }

                return result;
            }
            if(term->m_text.compare("_") == 0) {
                result->m_var_type_current = ObjType::None;
                return result;
            } else if(term->m_text.compare("$") == 0) {

                result->m_var_type_current = ObjType::Dictionary;
                result->m_var_name = "$";

                ASSERT(ctx);

                auto iter = ctx->begin();
                while(iter != ctx->end()) {
                    if(!iter->second.expired()) {
                        result->push_back(Obj::CreateString(iter->first));
                        iter++;
                    } else {
                        iter = ctx->ListType::erase(iter);
                    }
                }

                result->m_var_is_init = true;
                return result;
            } else if(term->m_text.compare("@") == 0) {
            } else if(term->m_text.compare("%") == 0) {
            }

            if(isLocal(term->m_text.c_str())) {
                full_name = MakeName(term->m_text);
                return local_vars->at(full_name).second;
            } else {
                result = ctx->GetTerm(term->GetFullName().c_str(), term->isRef());

                // Типы данных обрабатываются тут, а не в вызовах функций (TermID::CALL)

                if(term->size()) {
                    Obj args(ctx, term, true, local_vars);
                    result = result->Call(ctx, &args);
                }

            }
            if(!result) {
                // Делать ислкючение или возвращать объект "ошибка" ?????
                LOG_RUNTIME("Term '%s' not found!", term->GetFullName().c_str());
            }

            field = term->m_right;
            if(field && field->getTermID() == TermID::FIELD) {
                while(field) {
                    result = result->at(field->getText()).second;
                    field = field->m_right;
                    ASSERT(!field); // Нужно выполнять, а не просто получать значение поля
                }
            } else if(field && field->getTermID() == TermID::INDEX) {
                while(field) {
                    result = result->index_get(MakeIndex(ctx, field, local_vars));
                    field = field->m_right;
                    ASSERT(!field); // Нужно выполнять, а не просто получать значение поля
                }
            } else if(field) {
                LOG_RUNTIME("Not implemented! %s", field->toString().c_str());
            }

            return result;

        case TermID::EVAL:
            return ctx->ExecStr(term->m_text.c_str(), local_vars, false);


        case TermID::TYPE:
        case TermID::TYPE_CALL:

            result = ctx->GetTerm(term->GetFullName().c_str(), term->isRef());


            has_error = false;
            type = typeFromString(term->GetFullName(), ctx, &has_error);
            if(has_error) {
                LOG_RUNTIME("Type name '%s' undefined!", term->GetFullName().c_str());
            }
            ASSERT(result);
            ASSERT(result->m_var_type_fixed == type);

            // Размерность, если указана
            result->m_dimensions = Obj::CreateType(result->m_var_type_current, result->m_var_type_current, true);
            for (size_t i = 0; i < term->m_dims.size(); i++) {
                result->m_dimensions->push_back(CreateRVal(ctx, term->m_dims[i], local_vars));
            }

            args = Obj::CreateDict();
            for (int64_t i = 0; i < static_cast<int64_t>(term->size()); i++) {


                if((*term)[i].second->GetTokenID() == TermID::FILLING) {

                    // Заполнение значений вызовом функции
                    // :Type(1, 2, 3, ... rand() ... );


                    ASSERT(!(*term)[i].second->Left());
                    ASSERT((*term)[i].second->Right());


                    ObjPtr expr = ctx->FindTerm((*term)[i].second->Right()->GetFullName());

                    if((*term)[i].second->Right()->getTermID() != TermID::CALL) {
                        LOG_RUNTIME("Operator filling supported function call only!");
                    }

                    if(i + 1 != term->size()) {
                        LOG_RUNTIME("Function filling is supported for the last argument only!");
                    }

                    if(!result->m_dimensions || !result->m_dimensions->size()) {
                        LOG_RUNTIME("Object has no dimensions!");
                    }

                    int64_t full_size = 1;
                    for (int dim_index = 0; dim_index < result->m_dimensions->size(); dim_index++) {

                        if(!(*result->m_dimensions)[dim_index].second->is_integer()) {
                            LOG_RUNTIME("Dimension index for function filling support integer value only!");
                        }

                        full_size *= (*result->m_dimensions)[dim_index].second->GetValueAsInteger();
                    }

                    if(full_size <= 0) {
                        LOG_RUNTIME("Items count error for all dimensions!");
                    }


                    if(expr->size()) {
                        LOG_RUNTIME("Argument in function for filling not implemented!");
                    }

                    for (int64_t dim_index = result->size(); dim_index < full_size; dim_index++) {
                        args->push_back(expr->Call(ctx));
                    }

                    break;

                } else if((*term)[i].second->GetTokenID() == TermID::ELLIPSIS) {

                    if(!term->name(i).empty()) {
                        LOG_RUNTIME("Named ellipsys not implemented!");
                    }

                    if((*term)[i].second->Right()) {

                        bool named = ((*term)[i].second->Left() && (*term)[i].second->Left()->getTermID() == TermID::ELLIPSIS);
                        ObjPtr exp = CreateRVal(ctx, (*term)[i].second->Right());

                        if(!exp->is_dictionary_type()) {
                            LOG_RUNTIME("Expansion operator applies to dictionary only!");
                        }


                        for (int index = 0; index < exp->size(); index++) {
                            if(named) {
                                args->push_back((*exp)[index].second, exp->name(index).empty() ? "" : exp->name(index));
                            } else {
                                args->push_back((*exp)[index].second);
                            }
                        }

                        continue;
                    }
                }

                if(term->name(i).empty()) {
                    args->push_back(CreateRVal(ctx, (*term)[i].second, local_vars));
                } else {
                    args->push_back(CreateRVal(ctx, (*term)[i].second, local_vars), term->name(i).c_str());
                }

            }

            result = result->Call(ctx, args.get());
            ASSERT(result);

            return result;



        case TermID::EXIT:
        case TermID::CALL:

            temp = ctx->GetTerm(term->GetFullName().c_str(), term->isRef());

            if(!temp) {
                LOG_RUNTIME("Term '%s' not found!", term->GetFullName().c_str());
            }

            args = Obj::CreateDict();
            ctx->CreateArgs_(args, term, local_vars);

            if(term->getTermID() == TermID::EXIT) {
                return result;
            }

            result = temp->Call(ctx, args.get());
            return result;


        case TermID::TENSOR:
        case TermID::DICT:
            result->m_var_type_current = ObjType::Dictionary;
            ctx->CreateArgs_(result, term, local_vars);

            result->m_var_is_init = true;
            if(term->getTermID() == TermID::TENSOR) {

                result->m_var_type_fixed = typeFromString(term->m_type_name, ctx);
                type = getSummaryTensorType(result, result->m_var_type_fixed);

                if(type != ObjType::None) {
                    result->m_value = ConvertToTensor(result.get(), toTorchType(type));
                } else {
                    result->m_var_is_init = false;
                }
                result->resize(0, nullptr, "");
                result->m_var_type_current = type;
            } else {
                result->m_class_name = term->m_class_name;
            }
            return result;

        case TermID::ARGUMENT:

            val_int = IndexArg(term);
            if(val_int < local_vars->size()) {
                return local_vars->at(val_int).second;
            }
            LOG_RUNTIME("Argument '%s' not exist!", term->toString().c_str());

        case TermID::BLOCK:
            return CallBlock(ctx, term, local_vars, false);

        case TermID::BLOCK_TRY:
            return CallBlock(ctx, term, local_vars, int_catch);

        case TermID::ELLIPSIS:
            result->m_var_type_current = ObjType::Ellipsis;
            result->m_var_type_fixed = ObjType::None;
            result->m_var_is_init = true;
            return result;

        case TermID::RANGE:

            for (int i = 0; i < term->size(); i++) {
                ASSERT(!term->name(i).empty());
                result->push_back(CreateRVal(ctx, (*term)[i].second, local_vars), term->name(i).c_str());
            }

            if(result->size() == 2) {
                result->push_back(Obj::CreateValue(1, ObjType::None), "step");
            }

            result->m_var_type_current = ObjType::Range;
            result->m_var_type_fixed = ObjType::Range;
            result->m_var_is_init = true;

            return result;

        case TermID::FRACTION:
            return Obj::CreateFraction(term->m_text);

        case TermID::ITERATOR:

            ASSERT(term->Left());

            temp = ctx->GetTerm(term->Left()->GetFullName().c_str(), true);
            if(!temp) {
                LOG_RUNTIME("Term '%s' not found!", term->GetFullName().c_str());
            }

            args = Obj::CreateDict();
            ctx->CreateArgs_(args, term, local_vars);

            if(term->m_text.compare("?") == 0) {
                return temp->MakeIterator(args.get());
            } else if(term->m_text.compare("??") == 0) {
                return temp->IteratorReset();
            } else if(term->m_text.compare("!") == 0) {
                return temp->IteratorNext(0);
            } else if(term->m_text.compare("!!") == 0) {
                return temp->IteratorNext(1);
            } else if(term->m_text.compare("!?") == 0 || term->m_text.compare("?!") == 0) {

                val_int = std::numeric_limits<int64_t>::max();
                if(args->empty() || (args->size() == 1 && args->at(0).second->is_integer())) {
                    result = temp->MakeIterator(Iterator<Obj>::FIND_KEY_DEFAULT, false);
                    if(args->size()) {
                        val_int = args->at(0).second->GetValueAsInteger();
                    }
                } else if(args->size() == 1 && args->at(0).second->is_string_type()) {
                    result = temp->MakeIterator(args->at(0).second->GetValueAsString(), false);
                } else if(args->size() == 2 && args->at(0).second->is_string_type() && args->at(1).second->is_integer()) {
                    result = temp->MakeIterator(args->at(0).second->GetValueAsString(), false);
                    val_int = args->at(1).second->GetValueAsInteger();
                } else {
                    LOG_RUNTIME("Iterator`s args '%s' not allowed!", args->toString().c_str());
                }
                return result->IteratorNext(val_int);

            } else {
                LOG_RUNTIME("Iterator '%s' not recognized in '%s'!", term->m_text.c_str(), term->toString().c_str());
            }


    }
    LOG_RUNTIME("Fail create type %s from '%s'", newlang::toString(term->getTermID()), term->toString().c_str());

    return nullptr;
}

void Context::CreateArgs_(ObjPtr &args, TermPtr &term, Obj *local_vars) {
    for (int i = 0; i < term->size(); i++) {
        if(term->name(i).empty()) {
            args->push_back(CreateRVal(this, (*term)[i].second, local_vars));
        } else {
            args->push_back(CreateRVal(this, (*term)[i].second, local_vars), term->name(i).c_str());
        }
    }
}