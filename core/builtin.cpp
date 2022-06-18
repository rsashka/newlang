#include "pch.h"

#include <core/builtin.h>
#include <core/newlang.h>

using namespace newlang;

#define NEWLANG_FUNCTION(NAME)  newlang::ObjPtr NAME(newlang::Context *ctx, newlang::Obj &in) 
#define NEWLANG_TRANSPARENT(NAME)  newlang::ObjPtr NAME(const newlang::Context *ctx, const newlang::Obj &in) 

namespace newlang {

    NEWLANG_TRANSPARENT(min) {
        if(in.size() < 2) {
            LOG_CALLSTACK(std::invalid_argument, "Empty argument list parameter!");
        }
        ObjPtr out = in.at(1).second;
        for (size_t i = 2; i < in.size(); i++) {
            if(*in.at(i).second < out) {
                out = in.at(i).second;
            }
        }
        return out;
    }

    NEWLANG_TRANSPARENT(max) {
        if(in.size() < 2) {
            LOG_CALLSTACK(std::invalid_argument, "Empty argument list parameter!");
        }
        ObjPtr out = in.at(1).second;
        for (size_t i = 2; i < in.size(); i++) {
            if(*in.at(i).second > out) {
                out = in.at(i).second;
            }
        }
        return out;
    }

    NEWLANG_FUNCTION(clone) {
        if(in.size() != 2) {
            LOG_CALLSTACK(std::invalid_argument, "Bad argument count parameter!");
        }
        return in[1]->Clone(nullptr);
    }

    NEWLANG_FUNCTION(const_) {
        if(in.size() != 2) {
            LOG_CALLSTACK(std::invalid_argument, "Bad argument count parameter!");
        }
        return in[1]->MakeConst();
    }

    NEWLANG_FUNCTION(mutable_) {
        if(in.size() != 2) {
            LOG_CALLSTACK(std::invalid_argument, "Bad argument count parameter!");
        }
        return in[1]->MakeMutable();
    }

    NEWLANG_FUNCTION(import) {
        if(!ctx) {
            LOG_RUNTIME("No access to context!");
        }
        return ctx->CreateNative(in.at(1).second->GetValueAsString().c_str(),
                in["module"]->GetValueAsString().c_str(), in["lazzy"]->GetValueAsBoolean());
    }

    NEWLANG_FUNCTION(eval) {
        if(!ctx) {
            LOG_RUNTIME("No access to context!");
        }
        return ctx->ExecStr(in.at(1).second->GetValueAsString().c_str());
    }

    NEWLANG_FUNCTION(exec) {
        if(!ctx) {
            LOG_RUNTIME("No access to context!");
        }
        return ctx->ExecFile(in.at(1).second->GetValueAsString().c_str());
    }



#undef NEWLANG_FUNCTION
#undef NEWLANG_TRANSPARENT



#define DEFINE_ENUM(name) \
newlang::ObjPtr name(const newlang::Context *ctx, const newlang::Obj &in) {\
    if(in.size() < 1 || !in.at(1).second) {\
        LOG_CALLSTACK(std::invalid_argument, "Bad argument count parameter!");\
    }\
    return nullptr; /*in.at(1).second->Convert(ObjType:: cast);*/\
}
//    \
//newlang::ObjPtr name##_(newlang::Context *ctx, newlang::Obj &in) {\
//    if(in.size() < 1 || !in.at(1).second) {\
//        LOG_CALLSTACK(std::invalid_argument, "Bad argument count parameter!");\
//    }\
//    return nullptr; /*in.at(1).second->Convert_(ObjType:: cast);*/\
//}

    NL_BUILTIN_CAST_TYPE(DEFINE_ENUM)

#undef DEFINE_ENUM


}

//void Context::CreateBuiltin() {
//    //    VERIFY(ctx->RegisterLibFunction(nullptr, "term(...)", &term_, Object::TRANSPARENT));
//
//    VERIFY(RegisterLibFunction(nullptr, "min(arg, ...)", (void *) &min, ObjType::TRANSPARENT));
//    VERIFY(RegisterLibFunction(nullptr, "max(arg, ...)", (void *) &max, ObjType::TRANSPARENT));
//    VERIFY(RegisterLibFunction(nullptr, "мин(arg, ...)", (void *) &min, ObjType::TRANSPARENT));
//    VERIFY(RegisterLibFunction(nullptr, "макс(arg, ...)", (void *) &max, ObjType::TRANSPARENT));
//
//    //    VERIFY(ctx->RegisterLibFunction(nullptr, "range(start, stop, step=1)", &range, Object::Type::TRANSPARENT));
//    //    VERIFY(ctx->RegisterLibFunction(nullptr, "dict(...)", &dictionary, Object::Type::TRANSPARENT));
//    //    VERIFY(ctx->RegisterLibFunction(nullptr, "dictionary(...)", &dictionary, Object::Type::TRANSPARENT));
//
//    VERIFY(RegisterLibFunction(nullptr, "const(arg1)", (void *) &const_, ObjType::FUNCTION));
//    //    VERIFY(ctx->RegisterLibFunction(nullptr, "clone(arg1)", &clone_, Object::Type::TRANSPARENT));
//    VERIFY(RegisterLibFunction(nullptr, "print(...)", (void *) &print_, ObjType::FUNCTION));
//
//#define DEFINE_ENUM(name, cast) \
//    VERIFY(RegisterLibFunction(nullptr, #name "(var, shape=_): " #cast, (void *)& newlang:: name, ObjType::TRANSPARENT)); \
//    VERIFY(RegisterLibFunction(nullptr, #name "_(&var, shape=_): " #cast, (void *)& newlang:: name##_, ObjType::FUNCTION));
//
//    NL_BUILTIN_CAST_TYPE(DEFINE_ENUM)
//
//#undef DEFINE_ENUM
//}

bool BuiltInTorchDirect::CheckDirect(CompileInfo &ci, TermPtr &term, std::string &output) {

    if(term->size() == 0 && m_tensor_noarg.find(term->getText()) != m_tensor_noarg.end()) {
        output += "->asTensor_()." + term->getText() + "(), " + output;

        return true;
    } else if(term->size() == 1) {

        if((*term)[0]->IsScalar() && m_tensor_scalar.find(term->getText()) != m_tensor_scalar.end()) {
            output += "->asTensor_()." + term->getText() + "(" + (*term)[0]->getText() + "), " + output;
            return true;

        } else if((*term)[0]->getTermID() == TermID::TERM) {
            std::string temp;
            NewLang::GetImpl(ci, (*term)[0], temp);
            output += "->asTensor_()." + term->getText() + "(" + temp + "->asTensor_()), " + output;
            return true;
        }
    }
    return false;
}

BuiltInTorchDirect::BuiltInTorchDirect() {

    //  Tensor& operator+=(const Tensor & other);
    //  Tensor& operator+=(Scalar other);
    //  Tensor& operator-=(const Tensor & other);
    //  Tensor& operator-=(Scalar other);
    //  Tensor& operator*=(const Tensor & other);
    //  Tensor& operator*=(Scalar other);
    //  Tensor& operator/=(const Tensor & other);
    //  Tensor& operator/=(Scalar other);
    //  Tensor& operator&=(const Tensor & other);
    //  Tensor& operator|=(const Tensor & other);
    //  Tensor& operator^=(const Tensor & other);
    //  
    //  at::Tensor copysign(const at::Tensor & other) const;
    //  at::Tensor & copysign_(const at::Tensor & other) const;
    //  at::Tensor copysign(const at::Scalar & other) const;
    //  at::Tensor & copysign_(const at::Scalar & other) const;
    //  at::Tensor logical_xor(const at::Tensor & other) const;
    //  at::Tensor & logical_xor_(const at::Tensor & other) const;
    //  at::Tensor logical_and(const at::Tensor & other) const;
    //  at::Tensor & logical_and_(const at::Tensor & other) const;
    //  at::Tensor logical_or(const at::Tensor & other) const;
    //  at::Tensor & logical_or_(const at::Tensor & other) const;
    //  at::Tensor bmm(const at::Tensor & mat2) const;
    //  at::Tensor clamp_max(const at::Scalar & max) const;
    //  at::Tensor clamp_max(const at::Tensor & max) const;
    //  at::Tensor & clamp_max_(const at::Scalar & max) const;
    //  at::Tensor & clamp_max_(const at::Tensor & max) const;
    //  at::Tensor clamp_min(const at::Scalar & min) const;
    //  at::Tensor clamp_min(const at::Tensor & min) const;
    //  at::Tensor & clamp_min_(const at::Scalar & min) const;
    //  at::Tensor & clamp_min_(const at::Tensor & min) const;
    //  at::Tensor div(const at::Tensor & other) const;
    //  at::Tensor & div_(const at::Tensor & other) const;
    //  at::Tensor div(const at::Scalar & other) const;
    //  at::Tensor & div_(const at::Scalar & other) const;
    //  at::Tensor divide(const at::Tensor & other) const;
    //  at::Tensor & divide_(const at::Tensor & other) const;
    //  at::Tensor divide(const at::Scalar & other) const;
    //  at::Tensor & divide_(const at::Scalar & other) const;
    //  at::Tensor true_divide(const at::Tensor & other) const;
    //  at::Tensor & true_divide_(const at::Tensor & other) const;
    //  at::Tensor true_divide(const at::Scalar & other) const;
    //  at::Tensor & true_divide_(const at::Scalar & other) const;
    //  at::Tensor & fill_(const at::Scalar & value) const;
    //  at::Tensor & fill_(const at::Tensor & value) const;
    //  at::Tensor floor_divide(const at::Tensor & other) const;
    //  at::Tensor & floor_divide_(const at::Tensor & other) const;
    //  at::Tensor floor_divide(const at::Scalar & other) const;
    //  at::Tensor & floor_divide_(const at::Scalar & other) const;
    //  at::Tensor gcd(const at::Tensor & other) const;
    //  at::Tensor & gcd_(const at::Tensor & other) const;
    //  at::Tensor lcm(const at::Tensor & other) const;
    //  at::Tensor & lcm_(const at::Tensor & other) const;
    //  at::Tensor ldexp(const at::Tensor & other) const;
    //  at::Tensor & ldexp_(const at::Tensor & other) const;
    //  at::Tensor logaddexp(const at::Tensor & other) const;
    //  at::Tensor logaddexp2(const at::Tensor & other) const;
    //  at::Tensor xlogy(const at::Tensor & other) const;
    //  at::Tensor xlogy(const at::Scalar & other) const;
    //  at::Tensor & xlogy_(const at::Tensor & other) const;
    //  at::Tensor & xlogy_(const at::Scalar & other) const;
    //  at::Tensor matmul(const at::Tensor & other) const;
    //  at::Tensor mm(const at::Tensor & mat2) const;
    //  at::Tensor mul(const at::Tensor & other) const;
    //  at::Tensor & mul_(const at::Tensor & other) const;
    //  at::Tensor mul(const at::Scalar & other) const;
    //  at::Tensor & mul_(const at::Scalar & other) const;
    //  at::Tensor multiply(const at::Tensor & other) const;
    //  at::Tensor & multiply_(const at::Tensor & other) const;
    //  at::Tensor multiply(const at::Scalar & other) const;
    //  at::Tensor & multiply_(const at::Scalar & other) const;
    //  at::Tensor mv(const at::Tensor & vec) const;
    //  at::Tensor reshape_as(const at::Tensor & other) const;
    //  at::Tensor smm(const at::Tensor & mat2) const;
    //  at::Tensor flip(at::IntArrayRef dims) const;
    //  at::Tensor type_as(const at::Tensor & other) const;
    //  at::Tensor heaviside(const at::Tensor & values) const;
    //  at::Tensor & heaviside_(const at::Tensor & values) const;
    //  at::Tensor & _coalesced_(bool coalesced) const;
    //  at::Tensor & eq_(const at::Scalar & other) const;
    //  at::Tensor & eq_(const at::Tensor & other) const;
    //  at::Tensor bitwise_and(const at::Scalar & other) const;
    //  at::Tensor bitwise_and(const at::Tensor & other) const;
    //  at::Tensor & bitwise_and_(const at::Scalar & other) const;
    //  at::Tensor & bitwise_and_(const at::Tensor & other) const;
    //  at::Tensor __and__(const at::Scalar & other) const;
    //  at::Tensor __and__(const at::Tensor & other) const;
    //  at::Tensor & __iand__(const at::Scalar & other) const;
    //  at::Tensor & __iand__(const at::Tensor & other) const;
    //  at::Tensor bitwise_or(const at::Scalar & other) const;
    //  at::Tensor bitwise_or(const at::Tensor & other) const;
    //  at::Tensor & bitwise_or_(const at::Scalar & other) const;
    //  at::Tensor & bitwise_or_(const at::Tensor & other) const;
    //  at::Tensor __or__(const at::Scalar & other) const;
    //  at::Tensor __or__(const at::Tensor & other) const;
    //  at::Tensor & __ior__(const at::Scalar & other) const;
    //  at::Tensor & __ior__(const at::Tensor & other) const;
    //  at::Tensor bitwise_xor(const at::Scalar & other) const;
    //  at::Tensor bitwise_xor(const at::Tensor & other) const;
    //  at::Tensor & bitwise_xor_(const at::Scalar & other) const;
    //  at::Tensor & bitwise_xor_(const at::Tensor & other) const;
    //  at::Tensor __xor__(const at::Scalar & other) const;
    //  at::Tensor __xor__(const at::Tensor & other) const;
    //  at::Tensor & __ixor__(const at::Scalar & other) const;
    //  at::Tensor & __ixor__(const at::Tensor & other) const;
    //  at::Tensor __lshift__(const at::Scalar & other) const;
    //  at::Tensor __lshift__(const at::Tensor & other) const;
    //  at::Tensor & __ilshift__(const at::Scalar & other) const;
    //  at::Tensor & __ilshift__(const at::Tensor & other) const;
    //  at::Tensor bitwise_left_shift(const at::Tensor & other) const;
    //  at::Tensor & bitwise_left_shift_(const at::Tensor & other) const;
    //  at::Tensor bitwise_left_shift(const at::Scalar & other) const;
    //  at::Tensor & bitwise_left_shift_(const at::Scalar & other) const;
    //  at::Tensor __rshift__(const at::Scalar & other) const;
    //  at::Tensor __rshift__(const at::Tensor & other) const;
    //  at::Tensor & __irshift__(const at::Scalar & other) const;
    //  at::Tensor & __irshift__(const at::Tensor & other) const;
    //  at::Tensor bitwise_right_shift(const at::Tensor & other) const;
    //  at::Tensor & bitwise_right_shift_(const at::Tensor & other) const;
    //  at::Tensor bitwise_right_shift(const at::Scalar & other) const;
    //  at::Tensor & bitwise_right_shift_(const at::Scalar & other) const;
    //  at::Tensor & tril_(int64_t diagonal=0) const;
    //  at::Tensor & triu_(int64_t diagonal=0) const;
    //  at::Tensor ne(const at::Scalar & other) const;
    //  at::Tensor ne(const at::Tensor & other) const;
    //  at::Tensor & ne_(const at::Scalar & other) const;
    //  at::Tensor & ne_(const at::Tensor & other) const;
    //  at::Tensor not_equal(const at::Scalar & other) const;
    //  at::Tensor not_equal(const at::Tensor & other) const;
    //  at::Tensor & not_equal_(const at::Scalar & other) const;
    //  at::Tensor & not_equal_(const at::Tensor & other) const;
    //  at::Tensor eq(const at::Scalar & other) const;
    //  at::Tensor eq(const at::Tensor & other) const;
    //  at::Tensor ge(const at::Scalar & other) const;
    //  at::Tensor ge(const at::Tensor & other) const;
    //  at::Tensor & ge_(const at::Scalar & other) const;
    //  at::Tensor & ge_(const at::Tensor & other) const;
    //  at::Tensor greater_equal(const at::Scalar & other) const;
    //  at::Tensor greater_equal(const at::Tensor & other) const;
    //  at::Tensor & greater_equal_(const at::Scalar & other) const;
    //  at::Tensor & greater_equal_(const at::Tensor & other) const;
    //  at::Tensor le(const at::Scalar & other) const;
    //  at::Tensor le(const at::Tensor & other) const;
    //  at::Tensor & le_(const at::Scalar & other) const;
    //  at::Tensor & le_(const at::Tensor & other) const;
    //  at::Tensor less_equal(const at::Scalar & other) const;
    //  at::Tensor less_equal(const at::Tensor & other) const;
    //  at::Tensor & less_equal_(const at::Scalar & other) const;
    //  at::Tensor & less_equal_(const at::Tensor & other) const;
    //  at::Tensor gt(const at::Scalar & other) const;
    //  at::Tensor gt(const at::Tensor & other) const;
    //  at::Tensor & gt_(const at::Scalar & other) const;
    //  at::Tensor & gt_(const at::Tensor & other) const;
    //  at::Tensor greater(const at::Scalar & other) const;
    //  at::Tensor greater(const at::Tensor & other) const;
    //  at::Tensor & greater_(const at::Scalar & other) const;
    //  at::Tensor & greater_(const at::Tensor & other) const;
    //  at::Tensor lt(const at::Scalar & other) const;
    //  at::Tensor lt(const at::Tensor & other) const;
    //  at::Tensor & lt_(const at::Scalar & other) const;
    //  at::Tensor & lt_(const at::Tensor & other) const;
    //  at::Tensor less(const at::Scalar & other) const;
    //  at::Tensor less(const at::Tensor & other) const;
    //  at::Tensor & less_(const at::Scalar & other) const;
    //  at::Tensor & less_(const at::Tensor & other) const;
    //  at::Tensor take(const at::Tensor & index) const;
    //  at::Tensor masked_select(const at::Tensor & mask) const;
    //  at::Tensor swapaxes(int64_t axis0, int64_t axis1) const;
    //  at::Tensor & swapaxes_(int64_t axis0, int64_t axis1) const;
    //  at::Tensor swapdims(int64_t dim0, int64_t dim1) const;
    //  at::Tensor & swapdims_(int64_t dim0, int64_t dim1) const;
    //  at::Tensor polygamma(int64_t n) const;
    //  at::Tensor & polygamma_(int64_t n) const;
    //  at::Tensor & atan2_(const at::Tensor & other) const;
    //  at::Tensor atan2(const at::Tensor & other) const;
    //  at::Tensor fmod(const at::Scalar & other) const;
    //  at::Tensor & fmod_(const at::Scalar & other) const;
    //  at::Tensor fmod(const at::Tensor & other) const;
    //  at::Tensor & fmod_(const at::Tensor & other) const;
    //  at::Tensor hypot(const at::Tensor & other) const;
    //  at::Tensor & hypot_(const at::Tensor & other) const;
    //  at::Tensor igamma(const at::Tensor & other) const;
    //  at::Tensor & igamma_(const at::Tensor & other) const;
    //  at::Tensor igammac(const at::Tensor & other) const;
    //  at::Tensor & igammac_(const at::Tensor & other) const;
    //  at::Tensor nextafter(const at::Tensor & other) const;
    //  at::Tensor & nextafter_(const at::Tensor & other) const;
    //  at::Tensor remainder(const at::Scalar & other) const;
    //  at::Tensor & remainder_(const at::Tensor & other) const;
    //  at::Tensor fmin(const at::Tensor & other) const;
    //  at::Tensor fmax(const at::Tensor & other) const;
    //  at::Tensor maximum(const at::Tensor & other) const;
    //  at::Tensor max(const at::Tensor & other) const;
    //  at::Tensor minimum(const at::Tensor & other) const;
    //  at::Tensor min(const at::Tensor & other) const;
    //  bool equal(const at::Tensor & other) const;
    //  at::Tensor pow(const at::Tensor & exponent) const;
    //  at::Tensor & pow_(const at::Scalar & exponent) const;
    //  at::Tensor float_power(const at::Scalar & exponent) const;
    //  at::Tensor & float_power_(const at::Scalar & exponent) const;

    //  at::Tensor abs() const;
    //  at::Tensor & abs_() const;
    //  at::Tensor absolute() const;
    //  at::Tensor & absolute_() const;
    //  at::Tensor angle() const;
    //  at::Tensor sgn() const;
    //  at::Tensor & sgn_() const;
    //  at::Tensor _conj() const;
    //  at::Tensor __dispatch_conj() const;
    //  at::Tensor _conj_physical() const;
    //  at::Tensor conj_physical() const;
    //  at::Tensor & conj_physical_() const;
    //  at::Tensor resolve_conj() const;
    //  at::Tensor resolve_neg() const;
    //  at::Tensor _neg_view() const;
    //  at::Tensor acos() const;
    //  at::Tensor & acos_() const;
    //  at::Tensor arccos() const;
    //  at::Tensor & arccos_() const;
    //  at::Tensor acosh() const;
    //  at::Tensor & acosh_() const;
    //  at::Tensor arccosh() const;
    //  at::Tensor & arccosh_() const;
    //  at::Tensor asinh() const;
    //  at::Tensor & asinh_() const;
    //  at::Tensor arcsinh() const;
    //  at::Tensor & arcsinh_() const;
    //  at::Tensor atanh() const;
    //  at::Tensor & atanh_() const;
    //  at::Tensor arctanh() const;
    //  at::Tensor & arctanh_() const;
    //  at::Tensor asin() const;
    //  at::Tensor & asin_() const;
    //  at::Tensor arcsin() const;
    //  at::Tensor & arcsin_() const;
    //  at::Tensor atan() const;
    //  at::Tensor & atan_() const;
    //  at::Tensor arctan() const;
    //  at::Tensor & arctan_() const;
    //  at::Tensor bitwise_not() const;
    //  at::Tensor & bitwise_not_() const;
    //  at::Tensor logical_not() const;
    //  at::Tensor & logical_not_() const;
    //  at::Tensor ceil() const;
    //  at::Tensor & ceil_() const;
    //  at::Tensor cos() const;
    //  at::Tensor & cos_() const;
    //  at::Tensor cosh() const;
    //  at::Tensor & cosh_() const;
    //  at::Tensor erf() const;
    //  at::Tensor & erf_() const;
    //  at::Tensor erfc() const;
    //  at::Tensor & erfc_() const;
    //  at::Tensor exp() const;
    //  at::Tensor & exp_() const;
    //  at::Tensor exp2() const;
    //  at::Tensor & exp2_() const;
    //  at::Tensor expm1() const;
    //  at::Tensor & expm1_() const;
    //  at::Tensor floor() const;
    //  at::Tensor & floor_() const;
    //  at::Tensor frac() const;
    //  at::Tensor & frac_() const;
    //  at::Tensor inverse() const;
    //  at::Tensor isnan() const;
    //  at::Tensor isreal() const;
    //  at::Tensor log() const;
    //  at::Tensor & log_() const;
    //  at::Tensor log10() const;
    //  at::Tensor & log10_() const;
    //  at::Tensor log1p() const;
    //  at::Tensor & log1p_() const;
    //  at::Tensor log2() const;
    //  at::Tensor & log2_() const;
    //  at::Tensor logdet() const;
    //  at::Tensor matrix_exp() const;
    //  at::Tensor median() const;
    //  at::Tensor nanmedian() const;
    //  at::Tensor numpy_T() const;
    //  at::Tensor rad2deg() const;
    //  at::Tensor & rad2deg_() const;
    //  at::Tensor deg2rad() const;
    //  at::Tensor & deg2rad_() const;
    //  at::Tensor ravel() const;
    m_tensor_noarg = {"any", "all", "msort", "max", "min", "signbit", "sign_", "sign", "i0_", "i0", "erfinv_", "erfinv",
        "lgamma", "lgamma_", "nonzero", "digamma", "digamma_", "set_", "col_indices", "crow_indices", "values", "_values", "_indices", "indices",
        "coalesce", "zero_", "positive", "fix_", "fix", "trunc_", "trunc", "flipud", "fliplr", "tanh_", "tanh", "tan_", "tan",
        "square_", "square", "sqrt_", "sqrt", "detach_", "detach", "sinh_", "sinh", "sinc_", "sinc", "sin_", "sin", "sigmoid_", "sigmoid",
        "rsqrt_", "rsqrt", "relu_", "relu", "round_", "round", "negative_", "negative", "neg_", "neg", "reciprocal_", "reciprocal"};

    m_tensor_scalar = {"=", "~", "+", "-", "*", "/", "+=", "-=", "*=", "/=",
        "copysign", "copysign_", "clamp_max", "clamp_max_", "clamp_min_", "clamp_min", "div_", "div", "fill_", "fill"};

    m_tensor_funcs = {"=", "~", "+", "-", "*", "/", "+=", "-=", "*=", "/=", "&=", "|=", "^=",
        "copysign", "copysign_", "logical_xor_", "logical_xor", "logical_and_", "logical_and", "logical_or_", "logical_or",
        "clamp_max", "clamp_max_", "clamp_min_", "clamp_min", "div_", "div", "fill_", "fill"};

}


