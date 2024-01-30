#include "pch.h"

#include <types.h>
#include <term.h>

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
            
    MAKE_TYPE(":Any")};

#undef MAKE_TYPE

static const TermPtr type_default_none = default_types.find(":None")->second;
static const TermPtr type_default_any = default_types.find(":Any")->second;

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
