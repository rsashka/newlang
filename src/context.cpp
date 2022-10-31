#include "contrib/logger/logger.h"
#include "pch.h"

#include <context.h>
#include <newlang.h>
#include <term.h>
#include <types.h>
#include <filesystem>
#include <stdbool.h>

using namespace newlang;


std::map<std::string, Context::EvalFunction> Context::m_ops;
std::map<std::string, Context::EvalFunction> Context::m_builtin_calls;
std::map<std::string, ObjPtr> Context::m_types;
std::map<std::string, Context::FuncItem> Context::m_funcs;
Parser::MacrosStore Context::m_macros;
std::multimap<std::string, DocPtr> Docs::m_docs;

const char * Return::RetPlus = ":IntPlus";
const char * Return::RetMinus = ":IntMinus";
const char * Return::IntParser = ":IntParser";
const char * Return::IntError = ":IntError";

const char * Return::Break = ":Break";
const char * Return::Continue = ":Continue";
//const char * Return::Return = ":Return";
const char * Return::Error = ":Error";
const char * Return::Parser = ":ErrorParser";
const char * Return::RunTime = ":ErrorRunTime";
const char * Return::Signal = ":ErrorSignal";
const char * Return::Abort = ":ErrorAbort";

Context::Context(RuntimePtr global) : m_llvm_builder(LLVMCreateBuilder()) {
    m_runtime = global;



#ifdef _MSC_VER

    std::wstring sys_file;
    std::string sys_init;

    //#define CYGWIN
#ifdef CYGWIN
    sys_file = L"cygwin1.dll";
    sys_init = "cygwin_dll_init";
    ffi_file = "cygffi-6.dll";
#else
    //sys_file = L"msys-2.0.dll";
    //sys_init = "msys_dll_init";
    ffi_file = "libffi-6.dll";
#endif

    //m_msys = LoadLibrary(sys_file.c_str());
    //if(!m_msys) {
    //    LOG_RUNTIME("Fail LoadLibrary %s: %s", sys_file.c_str(), RunTime::GetLastErrorMessage().c_str());
    //}

    //    typedef void init_type();
    //    init_type *init = (init_type *) GetProcAddress((HMODULE) m_msys, sys_init.c_str());
    //    if(m_msys && !init) {
    //        FreeLibrary((HMODULE) m_msys);
    //        LOG_RUNTIME("Func %s not found! %s", sys_init.c_str(), RunTime::GetLastErrorMessage().c_str());
    //        (*init)();
    //    }

    static void * m_ffi_handle = nullptr;

    if(!m_ffi_handle) {
        m_ffi_handle = LoadLibrary(utf8_decode(ffi_file).c_str());
    }
    if(!m_ffi_handle) {
        LOG_RUNTIME("Fail load %s!", ffi_file.c_str());
    }

    m_ffi_type_void = reinterpret_cast<ffi_type *> (GetProcAddress((HMODULE) m_ffi_handle, "ffi_type_void"));
    m_ffi_type_uint8 = reinterpret_cast<ffi_type *> (GetProcAddress((HMODULE) m_ffi_handle, "ffi_type_uint8"));
    m_ffi_type_sint8 = reinterpret_cast<ffi_type *> (GetProcAddress((HMODULE) m_ffi_handle, "ffi_type_sint8"));
    m_ffi_type_uint16 = reinterpret_cast<ffi_type *> (GetProcAddress((HMODULE) m_ffi_handle, "ffi_type_uint16"));
    m_ffi_type_sint16 = reinterpret_cast<ffi_type *> (GetProcAddress((HMODULE) m_ffi_handle, "ffi_type_sint16"));
    m_ffi_type_uint32 = reinterpret_cast<ffi_type *> (GetProcAddress((HMODULE) m_ffi_handle, "ffi_type_uint32"));
    m_ffi_type_sint32 = reinterpret_cast<ffi_type *> (GetProcAddress((HMODULE) m_ffi_handle, "ffi_type_sint32"));
    m_ffi_type_uint64 = reinterpret_cast<ffi_type *> (GetProcAddress((HMODULE) m_ffi_handle, "ffi_type_uint64"));
    m_ffi_type_sint64 = reinterpret_cast<ffi_type *> (GetProcAddress((HMODULE) m_ffi_handle, "ffi_type_sint64"));
    m_ffi_type_float = reinterpret_cast<ffi_type *> (GetProcAddress((HMODULE) m_ffi_handle, "ffi_type_float"));
    m_ffi_type_double = reinterpret_cast<ffi_type *> (GetProcAddress((HMODULE) m_ffi_handle, "ffi_type_double"));
    m_ffi_type_pointer = reinterpret_cast<ffi_type *> (GetProcAddress((HMODULE) m_ffi_handle, "ffi_type_pointer"));

    m_ffi_prep_cif = reinterpret_cast<ffi_prep_cif_type *> (GetProcAddress((HMODULE) m_ffi_handle, "ffi_prep_cif"));
    m_ffi_prep_cif_var = reinterpret_cast<ffi_prep_cif_var_type *> (GetProcAddress((HMODULE) m_ffi_handle, "ffi_prep_cif_var"));
    m_ffi_call = reinterpret_cast<ffi_call_type *> (GetProcAddress((HMODULE) m_ffi_handle, "ffi_call"));

#else
    //    std::string error;
    if(LLVMLoadLibraryPermanently("libffi") == 0) {
        LOG_RUNTIME("Fail load library libffi!");
    }

    m_ffi_type_void = static_cast<ffi_type *> (LLVMSearchForAddressOfSymbol("ffi_type_void"));
    m_ffi_type_uint8 = static_cast<ffi_type *> (LLVMSearchForAddressOfSymbol("ffi_type_uint8"));
    m_ffi_type_sint8 = static_cast<ffi_type *> (LLVMSearchForAddressOfSymbol("ffi_type_sint8"));
    m_ffi_type_uint16 = static_cast<ffi_type *> (LLVMSearchForAddressOfSymbol("ffi_type_uint16"));
    m_ffi_type_sint16 = static_cast<ffi_type *> (LLVMSearchForAddressOfSymbol("ffi_type_sint16"));
    m_ffi_type_uint32 = static_cast<ffi_type *> (LLVMSearchForAddressOfSymbol("ffi_type_uint32"));
    m_ffi_type_sint32 = static_cast<ffi_type *> (LLVMSearchForAddressOfSymbol("ffi_type_sint32"));
    m_ffi_type_uint64 = static_cast<ffi_type *> (LLVMSearchForAddressOfSymbol("ffi_type_uint64"));
    m_ffi_type_sint64 = static_cast<ffi_type *> (LLVMSearchForAddressOfSymbol("ffi_type_sint64"));
    m_ffi_type_float = static_cast<ffi_type *> (LLVMSearchForAddressOfSymbol("ffi_type_float"));
    m_ffi_type_double = static_cast<ffi_type *> (LLVMSearchForAddressOfSymbol("ffi_type_double"));
    m_ffi_type_pointer = static_cast<ffi_type *> (LLVMSearchForAddressOfSymbol("ffi_type_pointer"));

    m_ffi_prep_cif = reinterpret_cast<ffi_prep_cif_type *> (LLVMSearchForAddressOfSymbol("ffi_prep_cif"));
    m_ffi_prep_cif_var = reinterpret_cast<ffi_prep_cif_var_type *> (LLVMSearchForAddressOfSymbol("ffi_prep_cif_var"));
    m_ffi_call = reinterpret_cast<ffi_call_type *> (LLVMSearchForAddressOfSymbol("ffi_call"));

#endif

    if(!(m_ffi_type_uint8 && m_ffi_type_sint8 && m_ffi_type_uint16 && m_ffi_type_sint16 &&
            m_ffi_type_uint32 && m_ffi_type_sint32 && m_ffi_type_uint64 && m_ffi_type_sint64 &&
            m_ffi_type_float && m_ffi_type_double && m_ffi_type_pointer && m_ffi_type_void &&
            m_ffi_prep_cif && m_ffi_prep_cif_var && m_ffi_call)) {
        LOG_RUNTIME("Fail init data from libffi!");
    }


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
        VERIFY(RegisterTypeHierarchy(ObjType::Rational,{":Arithmetic"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Tensor,{":Arithmetic"}));

        VERIFY(RegisterTypeHierarchy(ObjType::Integer,{":Tensor"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Bool,{":Integer"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Int8,{":Integer"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Char,{":Integer"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Byte,{":Integer"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Int16,{":Integer"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Word,{":Integer"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Int32,{":Integer"}));
        VERIFY(RegisterTypeHierarchy(ObjType::DWord,{":Integer"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Int64,{":Integer"}));
        VERIFY(RegisterTypeHierarchy(ObjType::DWord64,{":Integer"}));

        VERIFY(RegisterTypeHierarchy(ObjType::Number,{":Tensor"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Float16,{":Number"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Float32,{":Number"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Float64,{":Number"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Single,{":Number"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Double,{":Number"}));

        VERIFY(RegisterTypeHierarchy(ObjType::Complex,{":Tensor"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Complex32,{":Complex"}));
        VERIFY(RegisterTypeHierarchy(ObjType::Complex64,{":Complex"}));

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

    obj->m_var = func;
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
    //    ASSERT(var->m_namespace.empty());

    if(isLocal(var->getName())) {
        var->getName() = var->getName().substr(1);
    } else {
        m_terms.push_back(var, var->getName());
    }
    push_back(var, var->getName());

    return var;
}

//#ifndef _MSC_VER
//
//void newlang::NewLangSignalHandler(int signal) {
//    throw Interrupt("Signal SIGABRT received", Interrupt::Abort);
//}
//#endif

//#include "StdCapture.h"

ObjPtr Context::Eval(Context *ctx, TermPtr term, Obj *args, bool eval_block, CatchType int_catch) {

    //    StdCapture Capture;
    //
    //    Capture.BeginCapture();

    //#ifndef _MSC_VER
    //    auto previous_handler = signal(SIGABRT, &NewLangSignalHandler);
    //#endif
    //    try {

    switch(term->m_id) {
        case TermID::END:
            return eval_END(ctx, term, args, eval_block);

#define DEFINE_CASE(name)                                                                                              \
    case TermID::name:                                                                                                 \
        return eval_##name(ctx, term, args, eval_block);

            NL_TERMS(DEFINE_CASE)

#undef DEFINE_CASE

    }
    //    } catch (Interrupt &obj) {
    //
    //#ifndef _MSC_VER
    //        signal(SIGABRT, previous_handler);
    //#endif        
    //
    //        ASSERT(obj.m_obj);
    //
    //        if(int_catch && obj.m_obj->getType() == ObjType::Return) {
    //
    //            ASSERT(obj.m_obj->size() == 1);
    //            return (*obj.m_obj)[0].second; // Возврат данных
    //
    //        } else if(int_catch) {
    //            ASSERT(obj.m_obj);
    //            return obj.m_obj; // Прерывания анализуирются выше по уровню
    //        }
    //
    //        throw; // Пробросить прерывание дальше
    //    }
    //
    //#ifndef _MSC_VER
    //    signal(SIGABRT, previous_handler);
    //#endif

    return Obj::CreateNone();
}

ObjPtr Context::eval_END(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    return Obj::CreateNone();
}

ObjPtr Context::eval_UNKNOWN(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    ASSERT(term);
    LOG_RUNTIME("Fail EVAL: '%s'", term->toString().c_str());

    return nullptr;
}

/*
 *
 *
 */

ObjPtr Context::eval_BLOCK(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    ASSERT(term && term->getTermID() == TermID::BLOCK);
    ObjPtr obj = Obj::CreateType(ObjType::BLOCK);
    obj->m_sequence = term;
    obj->m_var_is_init = true;

    if(eval_block) {
        return CallBlock(ctx, term, args, eval_block, CatchType::CATCH_AUTO, nullptr);
    }

    return obj;
}

ObjPtr Context::eval_LAMBDA(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    LOG_RUNTIME("Fail LAMBDA: '%s'", term->toString().c_str());

    //    ASSERT(term && term->getTermID() == TermID::LAMBDA);
    //    ObjPtr obj = Obj::CreateType(ObjType::LAMBDA);
    //    obj->m_sequence = term;
    //    obj->m_var_is_init = true;
    //
    //    if(term->size() && term->at(0).second->IsString()) {
    //        obj->m_help = Docs::Append(term->at(0).second->m_text);
    //    }
    //
    //    return obj;
}

ObjPtr Context::eval_BLOCK_TRY(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    ASSERT(term && term->getTermID() == TermID::BLOCK_TRY);
    ObjPtr obj = Obj::CreateType(ObjType::BLOCK_TRY);
    obj->m_sequence = term;
    obj->m_var_is_init = true;

    if(eval_block) {
        return CallBlock(ctx, term, args, eval_block, CatchType::CATCH_ALL, nullptr);
    }

    return obj;
}

ObjPtr Context::eval_BLOCK_PLUS(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    ASSERT(term && term->getTermID() == TermID::BLOCK_PLUS);
    ObjPtr obj = Obj::CreateType(ObjType::BLOCK_PLUS);
    obj->m_sequence = term;
    obj->m_var_is_init = true;

    if(eval_block) {
        return CallBlock(ctx, term, args, eval_block, CatchType::CATCH_PLUS, nullptr);
    }

    return obj;
}

ObjPtr Context::eval_BLOCK_MINUS(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    ASSERT(term && term->getTermID() == TermID::BLOCK_MINUS);
    ObjPtr obj = Obj::CreateType(ObjType::BLOCK_MINUS);
    obj->m_sequence = term;
    obj->m_var_is_init = true;

    if(eval_block) {
        return CallBlock(ctx, term, args, eval_block, CatchType::CATCH_MINUS, nullptr);
    }

    return obj;
}


ObjPtr Context::eval_MACRO(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    LOG_ERROR("Macro %s not found!", term->toString().c_str());
    return Obj::CreateNone();
}

ObjPtr Context::eval_MACRO_BODY(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    LOG_RUNTIME("eval_MACRO_BODY: %s", term->toString().c_str());

    return nullptr;
}

ObjPtr Context::eval_PARENT(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    LOG_RUNTIME("eval_PARENT: %s", term->toString().c_str());

    return nullptr;
}

ObjPtr Context::eval_NEWLANG(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    LOG_RUNTIME("eval_NEWLANG: %s", term->toString().c_str());

    return nullptr;
}

ObjPtr Context::eval_TYPE(Context *ctx, const TermPtr &term, Obj *local_vars, bool eval_block) {
    return CreateRVal(ctx, term, local_vars);
}

ObjPtr Context::eval_TYPE_CALL(Context *ctx, const TermPtr &term, Obj *local_vars, bool eval_block) {
    return CreateRVal(ctx, term, local_vars);
}

ObjPtr Context::eval_TYPENAME(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    LOG_RUNTIME("TYPENAME Not implemented!");

    return nullptr;
}

inline ObjPtr Context::eval_INTEGER(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_NUMBER(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_COMPLEX(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_EVAL(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_RATIONAL(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_STRWIDE(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_STRCHAR(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_TENSOR(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {

    return CreateRVal(ctx, term, args);
}

inline ObjPtr Context::eval_NONE(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    ASSERT(term && term->m_id == TermID::NONE);

    return Obj::CreateNone();
}

inline ObjPtr Context::eval_EMPTY(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    LOG_RUNTIME("EMPTY Not implemented!");

    return nullptr;
}

ObjPtr Context::eval_NAME(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    ASSERT(term && term->m_id == TermID::NAME);

    return CreateRVal(ctx, term, args);
}

ObjPtr Context::eval_LOCAL(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    LOG_RUNTIME("LOCAL Not implemented!");
    return nullptr;
}

ObjPtr Context::eval_MODULE(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    LOG_RUNTIME("MODULE Not implemented!");
    return nullptr;
}

ObjPtr Context::eval_NATIVE(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    LOG_RUNTIME("NATIVE Not implemented!");
    return nullptr;
}

ObjPtr Context::eval_CALL(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    ASSERT(term && term->m_id == TermID::CALL);
    return CreateRVal(ctx, term, args);
}

ObjPtr Context::func_NOT_SUPPORT(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    NL_PARSER(term, "Function or operator '%s' is not supported in interpreter mode!", term->m_text.c_str());

    return nullptr;
}

ObjPtr Context::eval_TEMPLATE(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    LOG_RUNTIME("TEMPLATE Not implemented!");
    return nullptr;
}

ObjPtr Context::eval_DOC_BEFORE(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    LOG_RUNTIME("DOC_BEFORE Not implemented!");

    return nullptr;
}

ObjPtr Context::eval_DOC_AFTER(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    LOG_RUNTIME("DOC_AFTER Not implemented!");

    return nullptr;
}

ObjPtr Context::eval_SYMBOL(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
    LOG_RUNTIME("SYMBOL '%s' Not implemented!", term->m_text.c_str());
    return nullptr;
}

ObjPtr Context::eval_NAMESPACE(Context *ctx, const TermPtr &term, Obj *args, bool eval_block) {
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

    bool is_eval_block = true;

    TermPtr next = term->Left();
    while(next && next->getTermID() != TermID::END) {
        if(next->m_is_call || next->GetTokenID() == TermID::CALL) {
            is_eval_block = false;
        }
        list_term.push_back(next);
        next = next->m_list;
    }

    if(!is_eval_block && list_term.size() > 1) {
        //@todo добавить поддержку присвоения сразу нескольким функциям
        NL_PARSER(term, "Multiple function assignment not implemented!");
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
            // LOG_DEBUG("find: %s", ctx->NamespaseFull(elem->GetFullName()).c_str());
            auto found = ctx->find(ctx->NamespaseFull(elem->GetFullName()));
            if(found == ctx->end() && mode == CreateMode::ASSIGN_ONLY) {
                NL_PARSER(elem, "Object '%s' (%s) not found!", elem->m_text.c_str(), ctx->NamespaseFull(elem->GetFullName()).c_str());
            }

            if(found != ctx->end()) {
                result = (*found).second.lock(); // Но она может быть возвращена как локальная
            }

            if(result && mode == CreateMode::CREATE_ONLY) {
                NL_PARSER(elem, "Object '%s' (%s) already exists!", elem->m_text.c_str(), ctx->NamespaseFull(elem->GetFullName()).c_str());
            }

            if(!term->Right()) { // Удаление глобальной переменной
                ctx->ListType::erase(found);
            } else {
                if(!result && (mode == CreateMode::ASSIGN_ONLY)) {
                    NL_PARSER(term->Left(), "Object '%s' (%s) not found!", term->Left()->m_text.c_str(), ctx->NamespaseFull(elem->GetFullName()).c_str());
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
    if(term->Right() && term->Right()->getTermID() != TermID::ELLIPSIS && term->Right()->m_list) {
        NL_PARSER(term->Right()->Right(), "Multiple assignments not implemented!");
    }

    ObjPtr rval;
    if(term->Right()->getTermID() == TermID::ELLIPSIS) {
        ASSERT(term->Right()->Right());
        rval = Eval(ctx, term->Right()->Right(), local_vars, is_eval_block, CatchType::CATCH_AUTO);
    } else if(term->Right()->getTermID() == TermID::CLASS) {
        ASSERT(list_obj.size() == 1);
        // Имя класса появляется только при операции присвоения в левой части оператора
        rval = ctx->CreateClass(term->Left()->GetFullName(), term->Right(), local_vars);
    } else {
        rval = Eval(ctx, term->Right(), local_vars, is_eval_block, CatchType::CATCH_AUTO);
    }
    if(!rval) {
        NL_PARSER(term->Right(), "Object is missing or expression is not evaluated!");
    }

    ASSERT(list_obj.size() == list_term.size());

    if(term->Right()->getTermID() == TermID::ELLIPSIS) {
        if(rval->is_dictionary_type() || rval->is_tensor_type()) {
            if(!rval->empty() && rval->is_scalar()) {
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
                if(list_term[i]->Right()) {
                    ASSERT(list_term[i]->Right()->GetTokenID() == TermID::INDEX);
                    list_obj[i]->index_set_(MakeIndex(ctx, list_term[i]->Right(), local_vars), rval);
                } else {
                    list_obj[i]->SetValue_(rval);
                }
                if(list_obj[i]->m_var_type_current == ObjType::Function && rval->is_block()) {
                    list_obj[i]->m_var_type_current = ObjType::EVAL_FUNCTION;
                }
                result = list_obj[i];
            }
        }
    }
    return result;
}

ObjPtr Context::eval_ASSIGN(Context *ctx, const TermPtr &term, Obj * local_vars, bool eval_block) {

    return CREATE_OR_ASSIGN(ctx, term, local_vars, CreateMode::ASSIGN_ONLY);
}

ObjPtr Context::eval_CREATE(Context *ctx, const TermPtr &term, Obj * local_vars, bool eval_block) {

    return CREATE_OR_ASSIGN(ctx, term, local_vars, CreateMode::CREATE_ONLY);
}

ObjPtr Context::eval_CREATE_OR_ASSIGN(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {

    return CREATE_OR_ASSIGN(ctx, term, args, CreateMode::CREATE_AUTO);
}

ObjPtr Context::eval_APPEND(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    LOG_RUNTIME("APPEND Not implemented!");

    return nullptr;
}

ObjPtr Context::eval_FUNCTION(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term && (term->getTermID() == TermID::FUNCTION || term->getTermID() == TermID::PUREFUNC));
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
        lval->SetValue_(CreateRVal(ctx, term->Right(), eval_block));
    } else {
        if(term->getTermID() == TermID::FUNCTION) {
            lval->m_var_type_current = ObjType::EVAL_FUNCTION;
        } else {

            LOG_RUNTIME("Create function '%s' not implemented!", term->toString().c_str());
        }
        lval->m_var_type_fixed = lval->m_var_type_current;
        lval->m_var_is_init = true;
        lval->m_sequence = term->Right();
    }

    return ctx->RegisterObject(lval);
}

ObjPtr Context::eval_PUREFUNC(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {

    return eval_FUNCTION(ctx, term, args);
}

/*
 *
 *
 */
ObjPtr Context::eval_ITERATOR(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    return CreateRVal(ctx, term, args, eval_block);
}

ObjPtr Context::eval_RANGE(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {

    return CreateRVal(ctx, term, args, eval_block);
}

ObjPtr Context::eval_ELLIPSIS(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    LOG_RUNTIME("ELLIPSIS Not implemented!");

    return nullptr;
}

ObjPtr Context::eval_FILLING(Context* ctx, const TermPtr& term, Obj * args, bool eval_block) {
    LOG_RUNTIME("FILLING Not implemented!");

    return nullptr;
}

ObjPtr Context::eval_ARGUMENT(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {

    return CreateRVal(ctx, term, args, eval_block);
}

ObjPtr Context::eval_ARGS(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {

    return CreateRVal(ctx, term, args, eval_block);
}

ObjPtr Context::eval_WHILE(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {

    ASSERT(term->Left());
    ASSERT(term->Right());

    bool is_interrupt;

    ObjPtr result = Obj::CreateNone();
    ObjPtr cond = CallBlock(ctx, term->Left(), args, eval_block, CatchType::CATCH_AUTO, &is_interrupt);
    if(!cond->GetValueAsBoolean() && !term->m_follow.empty()) {
        ASSERT(term->m_follow.size() == 1);
        result = CreateRVal(ctx, term->m_follow[0], args, CatchType::CATCH_AUTO);
    } else {
        while(cond->GetValueAsBoolean()) {

            LOG_DEBUG("result %s", result->toString().c_str());
            result = CallBlock(ctx, term->Right(), args, eval_block, CatchType::CATCH_AUTO, &is_interrupt);

            if(is_interrupt || result->op_class_test(Return::Break, ctx)) {
                break;
            } else if(is_interrupt || result->op_class_test(Return::Continue, ctx)) {
                continue;
            }

            cond = CallBlock(ctx, term->Left(), args, eval_block, CatchType::CATCH_AUTO, nullptr);
        }
    }
    return result;
}

ObjPtr Context::eval_DOWHILE(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term->Left());
    ASSERT(term->Right());

    ObjPtr result;
    ObjPtr cond;
    bool is_interrupt;
    do {

        result = CreateRVal(ctx, term->Left(), args, CatchType::CATCH_AUTO);
        cond = CallBlock(ctx, term->Right(), args, true, CatchType::CATCH_AUTO, &is_interrupt);

        if(is_interrupt && cond->op_class_test(Return::Break, ctx)) {
            break;
        } else if(is_interrupt && cond->op_class_test(Return::Continue, ctx)) {
            continue;
        }

    } while(cond->GetValueAsBoolean());

    return result;
}

ObjPtr Context::eval_FOLLOW(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {

    /*
     * [cond] --> {expr};
     * [cond] --> {expr}, [_] --> {else};
     * 
     */

    for (int64_t i = 0; i < static_cast<int64_t> (term->m_follow.size()); i++) {

        ASSERT(term->m_follow[i]->Left());
        ObjPtr cond = Eval(ctx, term->m_follow[i]->Left(), args, true, CatchType::CATCH_AUTO);

        if(cond->GetValueAsBoolean() || (i + 1 == term->m_follow.size() && cond->is_none_type())) {

            return CallBlock(ctx, term->m_follow[i]->Right(), args, true, CatchType::CATCH_AUTO, nullptr);
            //            return CreateRVal(ctx, term->m_follow[i]->Right(), args, eval_block, CatchType::CATCH_AUTO);
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

ObjPtr Context::eval_MATCHING(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    /*
     * [match] ==> { # ~> ~~> ~~~> ===>
     *  [cond1] --> {expr};
     *  [cond2, cond3] --> {expr};
     *  [_] --> {expr};
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

ObjPtr Context::eval_INDEX(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    LOG_RUNTIME("INDEX Not implemented!");

    return nullptr;
}

ObjPtr Context::eval_FIELD(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    LOG_RUNTIME("FIELD Not implemented!");

    return nullptr;
}

ObjPtr Context::eval_DICT(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {

    return CreateRVal(ctx, term, args);
}

ObjPtr Context::eval_CLASS(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    LOG_RUNTIME("CLASS Not implemented!");

    return nullptr;
}

ObjPtr Context::eval_SOURCE(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    NL_PARSER(term, "Inclusion on the implementation language is not supported in interpreter mode!");

    return nullptr;
}

/*
 *
 *
 */

ObjPtr Context::eval_OPERATOR(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    if(Context::m_ops.find(term->m_text) == Context::m_ops.end()) {

        LOG_RUNTIME("Eval op '%s' not exist!", term->m_text.c_str());
    }
    return (*Context::m_ops[term->m_text])(ctx, term, args, eval_block);
}

/*
 *
 *
 */

ObjPtr Context::op_EQUAL(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args, eval_block)->op_equal(Eval(ctx, term->Right(), args, eval_block)) ? Obj::Yes() : Obj::No();
}

ObjPtr Context::op_ACCURATE(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args, eval_block)->op_accurate(Eval(ctx, term->Right(), args, eval_block)) ? Obj::Yes() : Obj::No();
}

ObjPtr Context::op_NE(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args, eval_block)->op_equal(Eval(ctx, term->Right(), args, eval_block)) ? Obj::No() : Obj::Yes();
}

ObjPtr Context::op_LT(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args, eval_block)->operator<(Eval(ctx, term->Right(), args, eval_block)) ? Obj::Yes() : Obj::No();
}

ObjPtr Context::op_GT(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args, eval_block)->operator>(Eval(ctx, term->Right(), args, eval_block)) ? Obj::Yes() : Obj::No();
}

ObjPtr Context::op_LE(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args, eval_block)->operator<=(Eval(ctx, term->Right(), args, eval_block)) ? Obj::Yes() : Obj::No();
}

ObjPtr Context::op_GE(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args, eval_block)->operator>=(Eval(ctx, term->Right(), args, eval_block)) ? Obj::Yes() : Obj::No();
}

/*
 *
 *
 */

ObjPtr Context::op_AND(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args, eval_block)->op_bit_and(Eval(ctx, term->Right(), args, eval_block), false);
}

ObjPtr Context::op_OR(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    LOG_RUNTIME("OR Not implemented!");

    return nullptr;
}

ObjPtr Context::op_XOR(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    LOG_RUNTIME("XOR Not implemented!");

    return nullptr;
}

ObjPtr Context::op_BIT_AND(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    LOG_RUNTIME("BIT_AND Not implemented!");

    return nullptr;
}

ObjPtr Context::op_BIT_OR(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    LOG_RUNTIME("BIT_OR Not implemented!");

    return nullptr;
}

ObjPtr Context::op_BIT_XOR(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    LOG_RUNTIME("BIT_XOR Not implemented!");

    return nullptr;
}

ObjPtr Context::op_BIT_AND_(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    LOG_RUNTIME("BIT_AND_ Not implemented!");

    return nullptr;
}

ObjPtr Context::op_BIT_OR_(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    LOG_RUNTIME("BIT_OR_ Not implemented!");

    return nullptr;
}

ObjPtr Context::op_BIT_XOR_(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    LOG_RUNTIME("BIT_XOR_ Not implemented!");

    return nullptr;
}

/*
 *
 *
 */
ObjPtr Context::op_PLUS(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Right());
    if(term->Left()) {

        return Eval(ctx, term->Left(), args, eval_block)->operator+(Eval(ctx, term->Right(), args, eval_block));
    }
    return Eval(ctx, term->Left(), args, eval_block)->operator+();
}

ObjPtr Context::op_MINUS(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Right());
    if(term->Left()) {

        return Eval(ctx, term->Left(), args, eval_block)->operator-(Eval(ctx, term->Right(), args, eval_block));
    }
    return Eval(ctx, term->Left(), args, eval_block)->operator-();
}

ObjPtr Context::op_DIV(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args, eval_block)->operator/(Eval(ctx, term->Right(), args, eval_block));
}

ObjPtr Context::op_DIV_CEIL(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args, eval_block)->op_div_ceil(Eval(ctx, term->Right(), args, eval_block));
}

ObjPtr Context::op_MUL(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args, eval_block)->operator*(Eval(ctx, term->Right(), args, eval_block));
}

ObjPtr Context::op_REM(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args, eval_block)->operator%(Eval(ctx, term->Right(), args, eval_block));
}

ObjPtr Context::op_POW(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args, eval_block)->op_pow(Eval(ctx, term->Right(), args, eval_block));
}

/*
 *
 *
 */
ObjPtr Context::op_PLUS_(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args, eval_block)->operator+=(Eval(ctx, term->Right(), args, eval_block));
}

ObjPtr Context::op_MINUS_(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args, eval_block)->operator-=(Eval(ctx, term->Right(), args, eval_block));
}

ObjPtr Context::op_DIV_(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args, eval_block)->operator/=(Eval(ctx, term->Right(), args, eval_block));
}

ObjPtr Context::op_DIV_CEIL_(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args, eval_block)->op_div_ceil_(Eval(ctx, term->Right(), args, eval_block));
}

ObjPtr Context::op_MUL_(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args, eval_block)->operator*=(Eval(ctx, term->Right(), args, eval_block));
}

ObjPtr Context::op_REM_(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args, eval_block)->operator%=(Eval(ctx, term->Right(), args, eval_block));
}

ObjPtr Context::op_POW_(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args, eval_block)->op_pow_(Eval(ctx, term->Right(), args, eval_block));
}

ObjPtr Context::op_CONCAT(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Eval(ctx, term->Left(), args, eval_block)->op_concat_(Eval(ctx, term->Right(), args, eval_block), ConcatMode::Append);
}

ObjPtr Context::op_TYPE_EQ(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    if(isType(term->Right()->GetFullName())) {
        return Obj::CreateBool(Eval(ctx, term->Left(), args, eval_block)->op_class_test(term->Right()->GetFullName().c_str(), ctx));
    }
    return Obj::CreateBool(Eval(ctx, term->Left(), args, eval_block)->op_class_test(Eval(ctx, term->Right(), args, eval_block), ctx));
}

ObjPtr Context::op_TYPE_EQ2(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Obj::CreateBool(Eval(ctx, term->Left(), args, eval_block)->op_duck_test(Eval(ctx, term->Right(), args, eval_block), false));
}

ObjPtr Context::op_TYPE_EQ3(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Obj::CreateBool(Eval(ctx, term->Left(), args, eval_block)->op_duck_test(Eval(ctx, term->Right(), args, eval_block), true));
}

ObjPtr Context::op_TYPE_NE(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());
    if(isType(term->Right()->GetFullName())) {
        return Obj::CreateBool(!Eval(ctx, term->Left(), args, eval_block)->op_class_test(term->Right()->GetFullName().c_str(), ctx));
    }
    return Obj::CreateBool(!Eval(ctx, term->Left(), args, eval_block)->op_class_test(Eval(ctx, term->Right(), args, eval_block), ctx));
}

ObjPtr Context::op_TYPE_NE2(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Obj::CreateBool(!Eval(ctx, term->Left(), args, eval_block)->op_duck_test(Eval(ctx, term->Right(), args, eval_block), false));
}

ObjPtr Context::op_TYPE_NE3(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    return Obj::CreateBool(!Eval(ctx, term->Left(), args, eval_block)->op_duck_test(Eval(ctx, term->Right(), args, eval_block), true));
}

/*
 *
 */
ObjPtr Context::op_RSHIFT(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    LOG_RUNTIME("RSHIFT Not implemented!");

    return nullptr;
}

ObjPtr Context::op_LSHIFT(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    LOG_RUNTIME("LSHIFT Not implemented!");

    return nullptr;
}

ObjPtr Context::op_RSHIFT_(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    LOG_RUNTIME("RSHIFT_ Not implemented!");

    return nullptr;
}

ObjPtr Context::op_LSHIFT_(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    LOG_RUNTIME("LSHIFT_ Not implemented!");

    return nullptr;
}

ObjPtr Context::op_RRSHIFT(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    LOG_RUNTIME("RRSHIFT Not implemented!");

    return nullptr;
}

ObjPtr Context::op_RRSHIFT_(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    LOG_RUNTIME("RRSHIFT_ Not implemented!");

    return nullptr;
}

ObjPtr Context::op_SPACESHIP(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    LOG_RUNTIME("SPACESHIP Not implemented!");

    return nullptr;
}

/*
 *
 *
 */

ObjPtr eval_int(Context *ctx, const TermPtr &term, Obj * args, bool eval_block, ObjType type) {

    ObjPtr ret = Obj::CreateType(type, type, true);
    if(term->Right()) {
        ret->m_return_obj = Context::CreateRVal(ctx, term->Right(), args, eval_block, Context::CatchType::CATCH_NONE);
        //        ret->push_back(Obj::Arg(Context::CreateRVal(ctx, term->Right(), args, Context::CatchType::CATCH_NONE)));
    } else {
        ret->m_return_obj = Obj::CreateNone();
        //        ret->push_back(Obj::CreateNone());
    }
    ASSERT(ret);
    throw Return(ret);
}

ObjPtr Context::eval_INT_PLUS(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    return eval_int(ctx, term, args, eval_block, ObjType::RetPlus);
}

ObjPtr Context::eval_INT_MINUS(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    return eval_int(ctx, term, args, eval_block, ObjType::RetMinus);
}

ObjPtr Context::eval_INT_REPEAT(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
    LOG_RUNTIME("eval_INT_REPEAT '%s' Not implemented!", term->m_text.c_str());
    return nullptr;
}

ObjPtr Context::CallBlock(Context *ctx, const TermPtr &block, Obj * local_vars, bool eval_block, CatchType type_catch, bool *has_interrupt) {
    if(has_interrupt) {
        *has_interrupt = false;
    }

    bool call_else = !block->m_follow.empty();
    ObjPtr result = Obj::CreateNone();
    TermID auto_type = TermID::NONE;

    if(block->IsBlock()) {
        auto_type = block->GetTokenID();
    }

    try {

        if(!block->m_block.empty()) {

            for (size_t i = 0; i < block->m_block.size(); i++) {
                if(block->m_block[i]->IsBlock()) {
                    //                    LOG_DEBUG("NS %s (%d)", block->m_block[i]->m_class.c_str(), (int)ctx->m_ns_stack.size());
                    bool is_ns = ctx->NamespasePush(block->m_block[i]->m_class);
                    try {
                        result = CallBlock(ctx, block->m_block[i], local_vars, eval_block, CatchType::CATCH_AUTO, has_interrupt);
                    } catch (...) {
                        if(is_ns) {
                            ctx->NamespasePop();
                        }
                        throw;
                    }
                    if(is_ns) {
                        ctx->NamespasePop();
                    }
                } else {
                    result = Eval(ctx, block->m_block[i], local_vars, eval_block, CatchType::CATCH_NONE);
                }
            }

        } else {
            if(block->IsBlock()) {
                //                LOG_DEBUG("NS %s (%d)", block->m_class.c_str(), (int)ctx->m_ns_stack.size());
                bool is_ns = ctx->NamespasePush(block->m_class);
                try {
                    result = CallBlock(ctx, block, local_vars, eval_block, CatchType::CATCH_AUTO, has_interrupt);
                } catch (...) {
                    if(is_ns) {
                        ctx->NamespasePop();
                    }
                    throw;
                }
                if(is_ns) {
                    ctx->NamespasePop();
                }
            } else {
                result = Eval(ctx, block, local_vars, eval_block, CatchType::CATCH_NONE);
            }
        }

    } catch (Return &obj) {

        call_else = false;
        if(has_interrupt) {
            *has_interrupt = true;
        }

        ASSERT(obj.m_obj);
        ASSERT(obj.m_obj->m_return_obj);

        if(type_catch == CatchType::CATCH_NONE || auto_type == TermID::NONE || (type_catch == CatchType::CATCH_AUTO && auto_type == TermID::BLOCK)) {
            throw;
        } else if(auto_type == TermID::BLOCK_PLUS) {
            if((type_catch == CatchType::CATCH_PLUS || type_catch == CatchType::CATCH_AUTO)
                    && obj.m_obj->m_var_type_current == ObjType::RetPlus) {
                return obj.m_obj->m_return_obj;
            }
            throw;
        } else if(auto_type == TermID::BLOCK_MINUS) {
            if((type_catch == CatchType::CATCH_MINUS || type_catch == CatchType::CATCH_AUTO)
                    && obj.m_obj->m_var_type_current == ObjType::RetMinus) {
                return obj.m_obj->m_return_obj;
            }
            throw;
        } else if((type_catch == CatchType::CATCH_ALL ||
                (type_catch == CatchType::CATCH_AUTO && auto_type == TermID::BLOCK_TRY))
                && !block->m_type_allowed.empty()) { // Если есть фильтр для типа
            // Тип данных при возврате не соответствует фильтру, пробросить исключение дальше
            bool is_return = false;
            for (size_t i = 0; i < block->m_type_allowed.size(); i++) {
                if(obj.m_obj->m_return_obj->op_class_test(block->m_type_allowed[i]->getText().c_str(), ctx)) {
                    is_return = true;
                    break;
                }
            }
            if(!is_return) {
                throw;
            }
        }

        result = obj.m_obj->m_return_obj;

    } catch (std::exception &obj) {

        call_else = false;
        if(type_catch == CatchType::CATCH_NONE || (type_catch == CatchType::CATCH_AUTO && auto_type == TermID::BLOCK)) {
            throw;
        }

        if(has_interrupt) {
            *has_interrupt = true;
        }
        result = Obj::CreateType(ObjType::Error, ObjType::Error, true);
        result->m_value = std::string(obj.what());
    }

    if(call_else) {
        //        
        //        if(block->m_follow.size()!=1){
        //            LOG_DEBUG("block->m_follow.size() %d", (int)block->m_follow.size());
        //            for (auto &elem : block->m_follow) {
        //                LOG_DEBUG("%s", elem->toString().c_str());
        //            }
        //        }
        //        
        //        ASSERT(block->m_follow.size() == 1);

        if(block->IsBlock()) {
            result = CallBlock(ctx, block->m_follow[0], local_vars, eval_block, CatchType::CATCH_AUTO, has_interrupt);
        } else {
            result = Eval(ctx, block->m_follow[0], local_vars, eval_block, CatchType::CATCH_NONE);
        }

        //        result = Eval(ctx, block->m_follow[0], local_vars, eval_block, CatchType::CATCH_AUTO);
    }

    return result;
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
    if(proto->GetTokenID() == TermID::NAME) {
        if(proto->m_type_name.empty()) {
            LOG_RUNTIME("Cannot create native variable without specifying the type!");
        }

        type = typeFromString(proto->m_type_name, this);
        switch(type) {
            case ObjType::Bool:
            case ObjType::Int8:
            case ObjType::Char:
            case ObjType::Byte:
            case ObjType::Int16:
            case ObjType::Word:
            case ObjType::Int32:
            case ObjType::DWord:
            case ObjType::Int64:
            case ObjType::DWord64:
            case ObjType::Float32:
            case ObjType::Float64:
            case ObjType::Single:
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
    result->m_var_type_fixed = ObjType::Pointer; // Тип определен и не может измениться в дальнейшем

    *const_cast<TermPtr *> (&result->m_prototype) = proto;
    //    result->m_func_abi = abi;

    if(mangle_name) {
        result->m_func_mangle_name = mangle_name;
    }
    if(module) {
        result->m_module_name = module;
    }
    void * ptr = nullptr;
    if(lazzy) {
        result->m_var = static_cast<void *> (nullptr);
    } else {
        ASSERT(at::holds_alternative<at::monostate>(result->m_var));

        ptr = m_runtime->GetNativeAddr(result->m_func_mangle_name.empty() ? proto->m_text.c_str() : result->m_func_mangle_name.c_str(), module);

        switch(type) {
            case ObjType::Bool:
                result->m_var = static_cast<bool *> (ptr);
                break;
            case ObjType::Int8:
            case ObjType::Char:
            case ObjType::Byte:
                result->m_var = static_cast<int8_t *> (ptr);
                break;
            case ObjType::Int16:
            case ObjType::Word:
                result->m_var = static_cast<int16_t *> (ptr);
                break;
            case ObjType::Int32:
            case ObjType::DWord:
                result->m_var = static_cast<int32_t *> (ptr);
                break;
            case ObjType::Int64:
            case ObjType::DWord64:
                result->m_var = static_cast<int64_t *> (ptr);
                break;
            case ObjType::Float32:
            case ObjType::Single:
                result->m_var = static_cast<float *> (ptr);
                break;
            case ObjType::Float64:
            case ObjType::Double:
                result->m_var = static_cast<double *> (ptr);
                break;

            case ObjType::NativeFunc:
            default:
                result->m_var = ptr;
        }
        //        result->m_var = m_runtime->GetNativeAddr(
        //                result->m_func_mangle_name.empty() ? proto->m_text.c_str() : result->m_func_mangle_name.c_str(), module);

        if(result->is_function_type() || type == ObjType::Pointer) {
            NL_CHECK(at::get<void *>(result->m_var), "Error getting address '%s' from '%s'!", proto->toString().c_str(), module);
        } else if(ptr && result->is_tensor_type()) {
            //            result->m_tensor = torch::from_blob(at::get<void *>(result->m_var),{
            //            }, toTorchType(type));
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

    return GetDirectAddressFromLibrary(nullptr, name);
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
        result = GetObject(name.c_str());
    }

    if(result && (isLocalAny(name.c_str()) || isLocal(name))) {

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

    auto iter = ctx->find(ctx->NamespaseFull(term->GetFullName()));

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
    result->m_var_name = ctx->NamespaseFull(term->GetFullName());

    *const_cast<TermPtr *> (&result->m_prototype) = term;

    TermPtr type = term->GetType();
    if(term->IsFunction() || term->getTermID() == TermID::CALL) {

        result->m_var_type_current = ObjType::Function;
        result->m_var_type_fixed = result->m_var_type_current;
        *const_cast<TermPtr *> (&result->m_prototype) = term;
    } else if(type) {
        result->m_var_type_current = typeFromString(type->getText().c_str(), ctx);
        result->m_var_type_fixed = result->m_var_type_current;
        if(result->is_tensor_type()) {
            std::vector<int64_t> dims;
            if(type->m_dims.size()) {
                for (size_t i = 0; i < type->m_dims.size(); i++) {
                    NL_CHECK(type->m_dims[i]->getName().empty(), "Dimension named not supported!");
                    ObjPtr temp = CreateRVal(ctx, type->m_dims[i], true);
                    if(!temp) {
                        NL_PARSER(type, "Term not found!");
                    }
                    if(!temp->is_integer()) {

                        NL_PARSER(type, "Term type not integer!");
                    }
                    dims.push_back(temp->GetValueAsInteger());
                }
            }
            result->m_tensor = torch::empty(dims, toTorchType(result->m_var_type_current));
        }
    }
    if(!isType(term->GetFullName())) {

        ctx->RegisterObject(result);
    }
    return result;
}

ObjPtr Context::CreateRVal(Context *ctx, const char *source, Obj * local_vars, bool eval_block, CatchType no_catch) {
    TermPtr ast;
    Parser parser(ast);
    parser.Parse(source);

    return CreateRVal(ctx, ast, local_vars, eval_block, no_catch);
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
                case ObjType::Int8:
                case ObjType::Char:
                case ObjType::Byte:
                case ObjType::Int16:
                case ObjType::Word:
                case ObjType::Int32:
                case ObjType::DWord:
                case ObjType::Int64:
                case ObjType::DWord64:
                    value = at::Scalar(obj->Call(this)->GetValueAsInteger()); // args
                    tensor.index_put_(ind, value);
                    break;
                case ObjType::Float32:
                case ObjType::Float64:
                case ObjType::Single:
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
            case ObjType::Int8:
                ptr_char = self.data_ptr<signed char>();
                ASSERT(ptr_char);
                *ptr_char = static_cast<signed char> (obj->Call(this)->GetValueAsInteger());
                return;
            case ObjType::Int16:
                ptr_short = self.data_ptr<int16_t>();
                ASSERT(ptr_short);
                *ptr_short = static_cast<int16_t> (obj->Call(this)->GetValueAsInteger());
                return;
            case ObjType::Int32:
                ptr_int = self.data_ptr<int32_t>();
                ASSERT(ptr_int);
                *ptr_int = static_cast<int32_t> (obj->Call(this)->GetValueAsInteger());
                return;
            case ObjType::Int64:
                ptr_long = self.data_ptr<int64_t>();
                ASSERT(ptr_long);
                *ptr_long = static_cast<int64_t> (obj->Call(this)->GetValueAsInteger());
                return;
            case ObjType::Float32:
                ptr_float = self.data_ptr<float>();
                ASSERT(ptr_float);
                *ptr_float = static_cast<float> (obj->Call(this)->GetValueAsNumber());
                return;
            case ObjType::Float64:
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

std::vector<int64_t> GetTensorShape(Context *ctx, TermPtr type, Obj * local_vars, bool eval_block) {
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
                } else if(temp->m_tensor.dim() == 1) {
                    result.push_back(Index(temp->m_tensor));
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

ObjPtr Context::CreateRVal(Context *ctx, TermPtr term, Obj * local_vars, bool eval_block, CatchType int_catch) {

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
    result->m_is_reference = !!term->m_ref;

    int64_t val_int;
    double val_dbl;
    ObjType type;
    bool has_error;
    std::vector<int64_t> sizes;
    at::Scalar torch_scalar;
    switch(term->getTermID()) {
        case TermID::INTEGER:

            val_int = parseInteger(term->getText().c_str());
            NL_TYPECHECK(term, newlang::toString(typeFromLimit(val_int)), term->m_type_name); // Соответстствует ли тип значению?

            result = Obj::CreateValue(val_int);
            result->m_var_type_current = typeFromLimit(val_int);
            if(term->GetType()) {
                result->m_var_type_fixed = typeFromString(term->m_type_name, ctx);
                result->m_var_type_current = result->m_var_type_fixed;
            }
            return result;

        case TermID::NUMBER:
            val_dbl = parseDouble(term->getText().c_str());
            NL_TYPECHECK(term, newlang::toString(typeFromLimit(val_dbl)), term->m_type_name); // Соответстствует ли тип значению?

            result = Obj::CreateValue(val_dbl);
            result->m_var_type_current = typeFromLimit(val_dbl);
            if(term->GetType()) {
                result->m_var_type_fixed = typeFromString(term->m_type_name, ctx);
                result->m_var_type_current = result->m_var_type_fixed;
            }
            return result;

        case TermID::STRWIDE:
            return Obj::CreateString(utf8_decode(term->getText()));

        case TermID::STRCHAR:
            return Obj::CreateString(term->getText());

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

        case TermID::NAME:
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
                    if(field->m_is_call) {

                        if(result->m_class_name.empty()) {
                            NL_PARSER(term, "Object '%s' not a class! %s", term->GetFullName().c_str(), result->m_class_name.c_str());
                        }

                        full_name = result->m_class_name;
                        if(isType(full_name)) {
                            full_name = full_name.substr(1);
                        }
                        full_name += "::";
                        full_name += field->getText();

                        temp = ctx->GetTerm(full_name, true);
                        if(!temp) {
                            LOG_RUNTIME("Function '%s' not found!", full_name.c_str());
                        }

                        args = Obj::CreateDict();
                        for (size_t i = 0; i < field->size(); i++) {
                            if(field->name(i).empty()) {
                                args->push_back(CreateRVal(ctx, (*field)[i].second, local_vars));
                            } else {
                                args->push_back(CreateRVal(ctx, (*field)[i].second, local_vars), field->name(i).c_str());
                            }
                        }

                        result = temp->Call(ctx, args.get(), true, result);

                    } else {
                        result = result->at(field->getText()).second;
                    }
                    field = field->m_right;
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

            //        case TermID::EVAL:
            //            return ctx->ExecStr(term->m_text.c_str(), local_vars, CatchType::CATCH_NONE, false);


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
            result->m_dimensions = Obj::CreateType(ObjType::Dictionary, ObjType::Dictionary, true);
            for (size_t i = 0; i < term->m_dims.size(); i++) {
                result->m_dimensions->push_back(CreateRVal(ctx, term->m_dims[i], local_vars, eval_block, int_catch));
            }

            args = Obj::CreateDict();
            for (int64_t i = 0; i < static_cast<int64_t> (term->size()); i++) {


                if((*term)[i].second->GetTokenID() == TermID::FILLING) {

                    // Заполнение значений вызовом функции
                    // :Type(1, 2, 3, ... rand() ... );


                    ASSERT(!(*term)[i].second->Left());
                    ASSERT((*term)[i].second->Right());


                    ObjPtr expr = ctx->FindTerm((*term)[i].second->Right()->GetFullName());
                    ASSERT(expr);

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

                    for (int64_t dim_index = args->size(); dim_index < full_size; dim_index++) {
                        args->push_back(expr->Call(ctx));
                    }

                    break;

                } else if((*term)[i].second->GetTokenID() == TermID::ELLIPSIS) {

                    if(!term->name(i).empty()) {
                        LOG_RUNTIME("Named ellipsys not implemented!");
                    }

                    if((*term)[i].second->Right()) {

                        bool named = ((*term)[i].second->Left() && (*term)[i].second->Left()->getTermID() == TermID::ELLIPSIS);
                        ObjPtr exp = CreateRVal(ctx, (*term)[i].second->Right(), eval_block);

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



        case TermID::INT_PLUS:
        case TermID::INT_MINUS:
        case TermID::CALL:

            temp = ctx->GetTerm(term->GetFullName().c_str(), term->isRef());

            if(!temp) {
                NL_PARSER(term, "Term '%s' not found!", term->GetFullName().c_str());
            }

            args = Obj::CreateDict();
            ctx->CreateArgs_(args, term, local_vars);

            if(term->getTermID() == TermID::INT_MINUS || term->getTermID() == TermID::INT_PLUS) {
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

                if(term->m_type_name.empty()) {
                    result->m_var_type_fixed = ObjType::None;
                } else {
                    result->m_var_type_fixed = typeFromString(term->m_type_name, ctx);
                }
                type = getSummaryTensorType(result.get(), result->m_var_type_fixed);

                if(type != ObjType::None) {


                    sizes = TensorShapeFromDict(result.get());
                    result->toType_(type);

                    if(!sizes.empty()) {
                        ASSERT(result->m_tensor.defined());
                        result->m_tensor = result->m_tensor.reshape(sizes);
                    }


                } else {
                    result->m_var_is_init = false;
                }
                //                result->resize(0, nullptr, "");
                result->m_var_type_current = type;
            } else {
                result->m_class_name = term->m_class;
            }
            return result;

        case TermID::ARGUMENT:

            val_int = IndexArg(term);
            if(val_int < local_vars->size()) {
                return local_vars->at(val_int).second;
            }
            LOG_RUNTIME("Argument '%s' not exist!", term->toString().c_str());

        case TermID::BLOCK:
            return CallBlock(ctx, term, local_vars, eval_block, CatchType::CATCH_AUTO, nullptr);

        case TermID::BLOCK_TRY:
        case TermID::BLOCK_PLUS:
        case TermID::BLOCK_MINUS:
            return CallBlock(ctx, term, local_vars, eval_block, int_catch, nullptr);

            //        case TermID::BLOCK_PLUS:
            //            return CallBlock(ctx, term, local_vars, CatchType::CATCH_PLUS);
            //            
            //        case TermID::BLOCK_MINUS:
            //            return CallBlock(ctx, term, local_vars, CatchType::CATCH_MINUS);

        case TermID::ELLIPSIS:
            result->m_var_type_current = ObjType::Ellipsis;
            result->m_var_type_fixed = ObjType::None;
            result->m_var_is_init = true;
            return result;

        case TermID::RANGE:

            result->m_var_type_current = ObjType::Dictionary;
            for (int i = 0; i < term->size(); i++) {
                ASSERT(!term->name(i).empty());
                result->push_back(Eval(ctx, (*term)[i].second, local_vars, eval_block), term->name(i).c_str());
            }

            if(result->size() == 2) {
                result->push_back(Obj::CreateValue(1, ObjType::None), "step");
            }

            result->m_var_type_current = ObjType::Range;
            result->m_var_type_fixed = ObjType::Range;
            result->m_var_is_init = true;

            return result;

        case TermID::RATIONAL:
            return Obj::CreateRational(term->m_text);

        case TermID::ITERATOR:

            ASSERT(term->Left());

            temp = Eval(ctx, term->Left(), local_vars, eval_block, CatchType::CATCH_AUTO);
            if(!temp) {
                LOG_RUNTIME("Term '%s' not found!", term->Left()->GetFullName().c_str());
            }

            args = Obj::CreateDict();
            ctx->CreateArgs_(args, term, local_vars);


            /*
             * Создание итератора
             * ?, ?(), ?("Фильтр"), ?(func), ?(func, args...)
             * 
             * Перебор элементов итератора
             * !, !(), !(0), !(3), !(-3)
             * 
             * dict! и dict!(0) эквивалентны
             * dict! -> 1,  dict! -> 2, dict! -> 3, dict! -> 4, dict! -> 5, dict! -> :IteratorEnd
             * 
             * Различия отрицательного размера возвращаемого словаря для итератора
             * dict!(-1) -> (1,),  ...  dict!(-1) -> (5,),  dict!(-1) -> (:IteratorEnd,),  
             * dict!(1) -> (1,),  ...  dict!(1) -> (5,),  dict!(1) -> (,),  
             * dict!(-3) -> (1, 2, 3,),  dict!(-3) -> (4, 5, :IteratorEnd,)
             * dict!(3) -> (1, 2, 3,), dict!(3) -> (4, 5,)
             * 
             * Операторы ?! и !? эквивалентны и возвращают текущие данные без перемещения указателя итератора.
             * 
             * Оператор ?? создает итератор и сразу его выполняет, возвращая все значения 
             * в виде элементов словаря, т.е. аналог последовательности ?(LINQ); !(:Int64.__max__);
             * 
             * Оператор !! - сбрасывает итератор в начальное состояние и возвращает первый элемент
             */

            if(term->m_text.compare("?") == 0) {
                return temp->IteratorMake(args.get());
            } else if(term->m_text.compare("!") == 0) {
                ASSERT(!args->size() && "Argument processing not implemented");
                return temp->IteratorNext(0);
            } else if(term->m_text.compare("!!") == 0) {
                ASSERT(!args->size() && "Argument processing not implemented");
                temp->IteratorReset();
                return temp->IteratorData();
            } else if(term->m_text.compare("!?") == 0 || term->m_text.compare("?!") == 0) {
                return temp->IteratorData();
            } else if(term->m_text.compare("??") == 0) {

                val_int = std::numeric_limits<int64_t>::max();
                if(args->empty() || (args->size() == 1 && args->at(0).second->is_integer())) {
                    result = temp->IteratorMake(nullptr, false);
                    if(args->size()) {
                        val_int = args->at(0).second->GetValueAsInteger();
                    }
                } else if(args->size() == 1 && args->at(0).second->is_string_type()) {
                    result = temp->IteratorMake(args->at(0).second->GetValueAsString().c_str(), false);
                } else if(args->size() == 2 && args->at(0).second->is_string_type() && args->at(1).second->is_integer()) {
                    result = temp->IteratorMake(args->at(0).second->GetValueAsString().c_str(), false);
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

void Context::CreateArgs_(ObjPtr &args, TermPtr &term, Obj * local_vars) {
    for (int i = 0; i < term->size(); i++) {
        if(term->name(i).empty()) {
            args->push_back(Eval(this, (*term)[i].second, local_vars, true));
        } else {
            args->push_back(Eval(this, (*term)[i].second, local_vars, true), term->name(i).c_str());
        }
    }
}

ObjPtr Context::CreateClass(std::string class_name, TermPtr body, Obj * local_vars) {

    ASSERT(body->GetTokenID() == TermID::CLASS);
    ASSERT(body->m_base.size());

    ObjPtr new_class = Obj::CreateBaseType(ObjType::Class);
    new_class->m_var_name = class_name;
    std::string constructor = MakeConstructorName(class_name);

    // LOG_DEBUG("\nCreate class: '%s', constructor %s", class_name.c_str(), constructor.c_str());


    if(class_name.find(":") == 0) {
        class_name.erase(0, 1);
    }

    // Пройтись по всем базовым классам
    for (int i = 0; i < body->m_base.size(); i++) {

        ObjPtr base = GetTerm(body->m_base[i]->GetFullName().c_str(), false);

        // LOG_DEBUG("Base %s: '%s' %d", body->m_base[i]->GetFullName().c_str(), base->toString().c_str(), (int) base->size());

        bool has_error = false;
        ObjType type = typeFromString(body->m_base[i]->GetFullName(), this, &has_error);
        if(has_error) {
            LOG_RUNTIME("Type name '%s' undefined!", body->m_base[i]->GetFullName().c_str());
        }
        ASSERT(base);
        ASSERT(base->m_var_type_fixed == type);
        ASSERT(base->m_class_name.compare(body->m_base[i]->GetFullName()) == 0);
        ASSERT(!base->m_dimensions);

        // Клонировать все методы (функции) базового класса с новыми именами
        std::set<std::string> methods;
        std::string base_constructor = MakeConstructorName(base->m_class_name);
        // LOG_DEBUG("base_constructor %s", base_constructor.c_str());


        std::string find_substr = base->m_class_name.substr(1);
        find_substr += "::";

        auto iter = m_terms.begin();
        while(iter != m_terms.end()) {
            if(iter->first.find(find_substr) == 0) {
                if(base_constructor.compare(iter->first) != 0) {
                    methods.insert(iter->first);
                }
            }
            iter++;
        }


        std::string replace(base->m_class_name.substr(1));
        replace += "::";

        std::set<Variable<Obj>::PairType *> rollback;


        try {

            ObjPtr obj;

            if(i == 0) {
                // Только один конструктор у класса
                obj = Obj::CreateFunc(constructor, &Obj::ConstructorStub_, ObjType::PureFunc);
                push_back(obj, constructor); // weak_ptr
                rollback.insert(&m_terms.push_back(obj, constructor));
            }

            for (auto &elem : methods) {

                // LOG_DEBUG("Func: %s  Replace: %s", elem.c_str(), replace.c_str());

                auto iter = m_terms.find(elem);
                ASSERT(iter != m_terms.end());

                obj = iter->second; //.lock();
                ASSERT(obj);

                std::string name(elem);
                size_t pos = name.find(replace);

                ASSERT(pos != name.npos);
                ASSERT(pos == 0 || name[pos - 1] == ':');

                name.erase(pos, replace.size() - 2);
                name.insert(pos, class_name);

                if(m_terms.find(name) == m_terms.end()) {
                    // LOG_DEBUG("new name %s", name.c_str());
                    push_back(obj, name); // weak_ptr
                    rollback.insert(&m_terms.push_back(obj, name));
                }
            }

            base->ClonePropTo(*new_class);


            // Выполнить тело конструктора типа для создания новых полей и методов у создаваемого типа класса
            bool is_pop_ns = NamespasePush(class_name);
            try {
                for (int i = 0; i < body->m_block.size(); i++) {
                    if(body->m_block[i]->IsCreate()) {
                        ASSERT(body->m_block[i]->Left());
                        if(body->m_block[i]->Left()->IsFunction() || body->m_block[i]->Left()->getTermID() == TermID::CALL) {
                            Eval(this, body->m_block[i], local_vars, true);
                        } else {

                            std::string name = body->m_block[i]->Left()->getText();
                            bool is_exists = (new_class->find(name) != new_class->end());

                            if(body->m_block[i]->getText().compare("::=") == 0) {
                                if(is_exists) {
                                    LOG_RUNTIME("Dublicate property name '%s' in class '%s'!", name.c_str(), class_name.c_str());
                                }
                                new_class->push_back(nullptr, name);
                            } else if(body->m_block[i]->getText().compare("=") == 0) {
                                if(!is_exists) {
                                    LOG_RUNTIME("Property name '%s' not found on base classes '%s'!", name.c_str(), class_name.c_str());
                                }
                            } else if(!is_exists) {
                                new_class->push_back(nullptr, name);
                            }

                            new_class->at(name).second = CreateRVal(this, body->m_block[i]->Right(), local_vars, true);
                        }
                    } else {
                        LOG_RUNTIME("Only create or assignment operators allowed! %s", body->m_block[i]->toString().c_str());
                    }
                }

                if(is_pop_ns) {
                    NamespasePop();
                }
            } catch (...) {
                if(is_pop_ns) {
                    NamespasePop();
                }
                throw;
            }


        } catch (...) {


            for (auto &elem : rollback) {
                // LOG_DEBUG("Rollback: '%s'", elem->first.c_str());
                // remove(find(elem.first)); // weak_ptr
                m_terms.remove(*elem);
            }

            throw;
        }

    }
    new_class->m_var_is_init = true;
    return new_class;
}

