#pragma once
#ifndef INCLUDED_NEWLANG_BUILTIN_
#define INCLUDED_NEWLANG_BUILTIN_

#include "pch.h"

#include <types.h>
#include <object.h>
#include <context.h>

#define FUNC_DIRECT(NAME, FUNC) \
    newlang::ObjPtr FUNC(newlang::Context *ctx, newlang::Obj &in);\
    inline newlang::ObjPtr NAME(newlang::Context *ctx, newlang::ObjPtr in){\
        ASSERT(in);\
        return FUNC(ctx, *in);\
    }\
    template <typename... T> \
    typename std::enable_if<newlang::is_all<newlang::Obj::PairType, T ...>::value, newlang::ObjPtr>::type \
    NAME(newlang::Context *ctx, T ... args) { \
        newlang::Obj list = newlang::Obj(newlang::Obj::ArgNull(), args...); \
        return FUNC(ctx, list); \
    }

#define FUNC_TRANSPARENT(NAME, FUNC) \
    newlang::ObjPtr FUNC(const newlang::Context *ctx, const newlang::Obj &in);\
    inline newlang::ObjPtr NAME(const newlang::Context *ctx, const newlang::ObjPtr in){\
        ASSERT(in);\
        return FUNC(ctx, *in);\
    }\
    template <typename... T> \
    typename std::enable_if<newlang::is_all<newlang::Obj::PairType, T ...>::value, newlang::ObjPtr>::type \
    newlang_##NAME(newlang::Context *ctx, T ... args) { \
        newlang::Obj list = newlang::Obj(newlang::Obj::ArgNull(), args...); \
        return FUNC(ctx, list); \
    }


namespace newlang {

FUNC_TRANSPARENT(newlang_min, min);
FUNC_TRANSPARENT(newlang_max, max);
FUNC_TRANSPARENT(newlang_maks, max);

FUNC_TRANSPARENT(newlang_clone, clone);
FUNC_TRANSPARENT(newlang_const_, const_);
FUNC_TRANSPARENT(newlang_mutable_, mutable_);

FUNC_DIRECT(newlang_import, import);
FUNC_DIRECT(newlang_print, print_);

FUNC_DIRECT(newlang_eval, eval);
FUNC_DIRECT(newlang_exec, exec);

FUNC_TRANSPARENT(newlang_help, help);
/*
 * 
 * 
 */

class BuiltInTorchDirect {
public:

    BuiltInTorchDirect();

    bool CheckDirect(CompileInfo &ci, TermPtr &term, std::string &output);

    virtual ~BuiltInTorchDirect() {
    }

private:


    BuiltInTorchDirect(const BuiltInTorchDirect&) = delete;
    const BuiltInTorchDirect& operator=(const BuiltInTorchDirect&) = delete;

    std::set<std::string> m_tensor_ops;
    std::set<std::string> m_tensor_noarg;
    std::set<std::string> m_tensor_funcs;
    std::set<std::string> m_tensor_scalar;
};

}

#endif //INCLUDED_NEWLANG_BUILTIN_
