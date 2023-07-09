#include "pch.h"
#include <term.h>

using namespace newlang;

size_t newlang::IndexArg(TermPtr term) {
    ASSERT(term);
    ASSERT(term->m_id == TermID::ARGUMENT);
    ASSERT(!term->m_text.empty());
    char * pEnd = nullptr;
    size_t index = std::strtoul(term->getText().data() + 1, &pEnd, 10);
    if(term->m_text.data() + term->getText().size() != pEnd) {
        LOG_RUNTIME("Float error '%s' or out of range!", term->getText().c_str());
    }
    return index;
}

BlockType Term::GetMacroId() {
    if(!(m_id == TermID::MACRO || m_id == TermID::MACRO_SEQ || m_id == TermID::MACRO_DEL)) {
        LOG_RUNTIME("Term '%s' as %s not a macro!", toString().c_str(), newlang::toString(m_id));
    }
    if(m_macro_id.empty()) {

        if(!m_follow.size()) {
            ASSERT(m_follow.size());
        }

        TermPtr term;
        size_t done;
        size_t pos = 0;

        while(pos < m_follow.size()) {

            if(MacroBuffer::deny_chars_from_macro.find(m_follow[pos]->m_text[0]) != std::string::npos) {
                NL_PARSER(m_follow[pos], "Symbol '%c' in lexem sequence not allowed!", m_follow[pos]->m_text[0]);
            }

            done = MacroBuffer::ParseTerm(term, m_follow, pos);
            if(done) {
                m_macro_id.push_back(term);
                pos += done;
            } else {
                LOG_RUNTIME("Fail convert %s", m_follow[pos]->toString().c_str());
            }

        }
    }

    //    std::vector<std::string> result;
    //    for (auto &elem : m_macro_id) {
    //        result.push_back(MacroBuffer::toHashTermName(elem->m_text));
    //    }
    return m_macro_id;
}
