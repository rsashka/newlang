#include "pch.h"
#include <term.h>

using namespace newlang;

size_t newlang::IndexArg(TermPtr term) {
    ASSERT(term);
    ASSERT(term->m_id == TermID::ARGUMENT);
    ASSERT(!term->m_text.empty());
    char * pEnd = nullptr;
    size_t index = std::strtoul(term->getText().data() + 1, &pEnd, 10);
    if (term->m_text.data() + term->getText().size() != pEnd) {
        LOG_RUNTIME("Float error '%s' or out of range!", term->getText().c_str());
    }
    return index;
}

BlockType Term::MakeMacroId(const BlockType &seq) {

    BlockType result;
    size_t pos = 0;
    TermPtr term;
    size_t done;

    while (pos < seq.size()) {

        if (Named::deny_chars_from_macro.find(seq[pos]->m_text[0]) != std::string::npos) {
            NL_PARSER(seq[pos], "Symbol '%c' in lexem sequence not allowed!", seq[pos]->m_text[0]);
        }

        done = Parser::ParseTerm(term, seq, pos);
        if (done) {
            result.push_back(term);
            pos += done;
        } else {
            LOG_RUNTIME("Fail convert %s", seq[pos]->toString().c_str());
        }
    }
    //    std::vector<std::string> result;
    //    for (auto &elem : m_macro_id) {
    //        result.push_back(MacroBuffer::toHashTermName(elem->m_text));
    //    }
    return result;
}

BlockType Term::GetMacroId() {
    if (!isMacro()) {
        LOG_RUNTIME("Term '%s' as %s not a macro!", toString().c_str(), newlang::toString(m_id));
    }

    if (m_macro_id.empty()) {

        if (m_id == TermID::MACRO_DEL) {
            ASSERT(m_macro_seq.size());

            m_macro_id = MakeMacroId(m_macro_seq);

        } else {
            ASSERT(IsCreate());
            ASSERT(m_left);
            ASSERT(m_left->m_id == TermID::MACRO_SEQ);
            ASSERT(m_left->m_macro_seq.size());

            m_macro_id = MakeMacroId(m_left->m_macro_seq);
            m_left->m_macro_id = m_macro_id;
        }
        
        ASSERT(m_macro_id.size());
    }
    return m_macro_id;
}
