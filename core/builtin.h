#pragma once
#ifndef INCLUDED_NEWLANG_BUILTIN_
#define INCLUDED_NEWLANG_BUILTIN_

#include "pch.h"

#include <core/types.h>
#include <core/object.h>
#include <core/context.h>

#define FUNC_DIRECT(NAME, FUNC) \
    newlang::ObjPtr FUNC(newlang::Context *ctx, newlang::Object &in);\
    inline newlang::ObjPtr NAME(newlang::Context *ctx, newlang::ObjPtr in){\
        ASSERT(in);\
        return FUNC(ctx, *in);\
    }\
    template <typename... T> \
    typename std::enable_if<newlang::is_all<newlang::Object::PairType, T ...>::value, newlang::ObjPtr>::type \
    NAME(newlang::Context *ctx, T ... args) { \
        newlang::Object list = newlang::Object(newlang::Object::ArgNull(), args...); \
        return FUNC(ctx, list); \
    }

#define FUNC_TRANSPARENT(NAME, FUNC) \
    newlang::ObjPtr FUNC(const newlang::Context *ctx, const newlang::Object &in);\
    inline newlang::ObjPtr NAME(const newlang::Context *ctx, const newlang::ObjPtr in){\
        ASSERT(in);\
        return FUNC(ctx, *in);\
    }\
    template <typename... T> \
    typename std::enable_if<newlang::is_all<newlang::Object::PairType, T ...>::value, newlang::ObjPtr>::type \
    newlang_##NAME(newlang::Context *ctx, T ... args) { \
        newlang::Object list = newlang::Object(newlang::Object::ArgNull(), args...); \
        return FUNC(ctx, list); \
    }

    
#define FUNC_CONVERT(NAME) \
    newlang::ObjPtr NAME(const newlang::Context *ctx, const newlang::Object &in);\
    newlang::ObjPtr NAME##_(newlang::Context *ctx, newlang::Object &in);\
    inline newlang::ObjPtr newlang_##NAME(const newlang::Context *ctx, const newlang::ObjPtr in){\
        ASSERT(in);\
        return NAME(ctx, *in);\
    }\
    inline newlang::ObjPtr newlang_##NAME##_(newlang::Context *ctx, newlang::ObjPtr in){\
        ASSERT(in);\
        return NAME##_(ctx, *in);\
    }\
    template <typename... T> \
    typename std::enable_if<newlang::is_all<newlang::Object::PairType, T ...>::value, newlang::ObjPtr>::type \
    newlang_##NAME(const newlang::Context *ctx, T ... args) { \
        const newlang::Object list = newlang::Object(newlang::Object::ArgNull(), args...); \
        return NAME(ctx, list); \
    }\
    template <typename... T> \
    typename std::enable_if<newlang::is_all<newlang::Object::PairType, T ...>::value, newlang::ObjPtr>::type \
    newlang_##NAME##_(newlang::Context *ctx, T ... args) { \
        newlang::Object list = newlang::Object(newlang::Object::ArgNull(), args...); \
        return NAME##_(ctx, list); \
    }

namespace newlang {

FUNC_TRANSPARENT(newlang_min, min);
FUNC_TRANSPARENT(newlang_max, max);
FUNC_TRANSPARENT(newlang_maks, max);

FUNC_DIRECT(newlang_import, import);
FUNC_DIRECT(newlang_print, print_);

FUNC_DIRECT(newlang_eval, eval);
FUNC_DIRECT(newlang_exec, exec);

//#define DEFINE_ENUM(name, cast) FUNC_CONVERT(name);
//
//    NL_BUILTIN_CAST_TYPE(DEFINE_ENUM)
//   
//#undef DEFINE_ENUM
            
            
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
