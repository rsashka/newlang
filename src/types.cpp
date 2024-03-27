#include "pch.h"

#include <types.h>
#include <term.h>
#include <object.h>

using namespace newlang;


#define MAKE_TYPE(type_name) {type_name, Term::Create(parser::token_type::END, TermID::TYPE, type_name)}

static const std::map<const std::string, const TermPtr> default_types{
    MAKE_TYPE(":None"),

    MAKE_TYPE(":Bool"),
    MAKE_TYPE(":Int8"),
    MAKE_TYPE(":Int16"),
    MAKE_TYPE(":Int32"),
    MAKE_TYPE(":Int64"),

    MAKE_TYPE(":Float16"),
    MAKE_TYPE(":Float32"),
    MAKE_TYPE(":Float64"),

    MAKE_TYPE(":Complex16"),
    MAKE_TYPE(":Complex32"),
    MAKE_TYPE(":Complex64"),

    MAKE_TYPE(":Rational"),

    MAKE_TYPE(":StrChar"),
    MAKE_TYPE(":StrWide"),

    MAKE_TYPE(":Range"),
    MAKE_TYPE(":Iterator"),

    MAKE_TYPE(":Dictionary"),

    MAKE_TYPE(":Any")};

#undef MAKE_TYPE

static const TermPtr type_default_none = default_types.find(":None")->second;
static const TermPtr type_default_any = default_types.find(":Any")->second;
//static const TermPtr type_default_dict = default_types.find(":Dict")->second;
static const TermPtr term_none = Term::Create(parser::token_type::NAME, TermID::NAME, "_");
static const TermPtr term_ellipsys = Term::Create(parser::token_type::ELLIPSIS, TermID::ELLIPSIS, "...");
static const TermPtr term_required = Term::Create(parser::token_type::END, TermID::NONE, "_");
static const ObjPtr obj_none = Obj::CreateNone();
static const ObjPtr obj_ellipsys = Obj::CreateType(ObjType::Ellipsis, ObjType::Ellipsis, true);

const TermPtr newlang::getNoneTerm() {
    return term_none;
}

const TermPtr newlang::getRequiredTerm() {
    return term_required;
}

const TermPtr newlang::getEllipsysTerm() {
    return term_ellipsys;
}

const ObjPtr newlang::getNoneObj() {
    return obj_none;
}

const ObjPtr newlang::getEllipsysObj() {
    return obj_ellipsys;
}

bool newlang::isDefaultType(const TermPtr & term) {
    if (term) {
        TermPtr def = getDefaultType(term->m_text);
        return def && (term.get() == def.get());
    }
    return true; // Not defined type as type default
}

const TermPtr newlang::getDefaultType(ObjType type) {
    return getDefaultType(toString(type));
}

const TermPtr newlang::getDefaultType(const std::string_view text) {
    if (!text.empty()) {
        auto iter = default_types.find(text.begin());
        if (iter != default_types.end()) {
            return iter->second;
        }
    } else {
        ASSERT(type_default_none->m_text.compare(":None") == 0);
        ASSERT(type_default_any->m_text.compare(":Any") == 0);

        return type_default_any;
    }
    return nullptr;
}

bool newlang::canCast(const TermPtr &from, const ObjType to) {
    if (!from->m_type) {
        return true; // Empty type cast any type
    }
    ObjType test_type = GetBaseTypeFromString(from->m_type->m_text);
    return canCast(test_type, to);
}

bool newlang::canCast(const TermPtr &from, const TermPtr &to) {
    if (!from || !to || !from->m_type || !to->m_type) {
        return true; // Empty type cast any type
    }
    return canCast(GetBaseTypeFromString(from->m_type->m_text), GetBaseTypeFromString(to->m_type->m_text));
}

ObjType newlang::GetBaseTypeFromString(const std::string_view type_arg, bool *has_error) {

    std::string type(type_arg);

    if (type.find("~") != std::string::npos) {
        type.erase(std::remove(type.begin(), type.end(), '~'), type.end());
    }

    if (type.empty()) {
        return ObjType::None;
    } else if (type.compare("_") == 0) {
        return ObjType::None;
    }

#define DEFINE_CASE(name, _)                    \
    else if (type.compare(":"#name) == 0) {     \
        return ObjType:: name;                  \
    }

    NL_TYPES(DEFINE_CASE)
#undef DEFINE_CASE

    if (has_error) {
        *has_error = true;
        return ObjType::None;
    }
    LOG_RUNTIME("Undefined type name '%s'!", type.c_str());
}
