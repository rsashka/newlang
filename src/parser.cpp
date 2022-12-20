#include "pch.h"

#include "parser.h"
#include "lexer.h"

#include <term.h>

using namespace newlang;

Parser::Parser() {
    m_ast = Term::Create(parser::token_type::END, TermID::END, "");
    m_ast->m_parser = this;
    m_macro = nullptr;
}

bool Parser::parse_stream(std::istream& in, const std::string sname) {
    streamname = sname;

    Init(m_macro);

    Scanner scanner(&in);
    this->lexer = &scanner;

    parser parser(*this);
    parser.set_debug_level(trace_parsing);
    return (parser.parse() == 0);
}

template <typename TP>
std::time_t to_time_t(TP tp) {
    using namespace std::chrono;
    auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
            + system_clock::now());
    return system_clock::to_time_t(sctp);
}

bool Parser::parse_file(const std::string filename) {
    std::ifstream in(filename);

    m_file_name = filename;

    llvm::sys::fs::file_status fs;
    std::error_code ec = llvm::sys::fs::status(filename, fs);
    if(!ec) {
        time_t temp = llvm::sys::toTimeT(fs.getLastModificationTime());
        struct tm * timeinfo;
        timeinfo = localtime(&temp);
        m_file_time = asctime(timeinfo);
    }

    auto md5 = llvm::sys::fs::md5_contents(filename);
    if(md5) {
        llvm::SmallString<32> hash;
        llvm::MD5::stringifyResult(*md5, hash);
        m_md5 = hash.c_str();
    }

    if(!in.good()) return false;
    return parse_stream(in, filename.c_str());
}

bool Parser::parse_string(const std::string input, const std::string sname) {
    std::istringstream iss(input.c_str());
    return parse_stream(iss, sname.c_str());
}

TermPtr Parser::Parse(const std::string input, MacroBuffer *macro) {

    Init(macro);

    //    std::string parse_string = ParseAllMacros(input, store);

    m_ast = Term::Create(parser::token_type::END, TermID::END, "");
    //    m_ast->SetSource(std::make_shared<std::string>(input));
    m_ast->m_parser = this;
    m_stream.str(input);
    Scanner scanner(&m_stream, &std::cout, std::make_shared<std::string>(input));
    lexer = &scanner;

    parser parser(*this);
    if(parser.parse() != 0) {
        return m_ast;
    }

    // Исходники требуются для вывода информации ошибках во время анализа типов
    m_ast->SetSource(m_ast->m_source);
    lexer = nullptr;
    return m_ast;
}

TermPtr Parser::ParseString(const std::string str, MacroBuffer *macro) {
    Parser p;
    return p.Parse(str, macro);
}

void Parser::error(const class location& l, const std::string& m) {
    std::cerr << l << ": " << m << std::endl;
}

void Parser::error(const std::string &m) {
    std::cerr << m << std::endl;
}

void newlang::parser::error(const parser::location_type& l, const std::string& msg) {
    //    std::string buffer(driver.m_stream.str());
    std::string buffer(*(driver.lexer->source_string));
    std::cout << driver.lexer->source_string->c_str() << "\n\n";
    std::cout.flush();
    ParserException(msg.c_str(), buffer, l.begin.line, l.begin.column);
}

TermPtr Parser::GetAst() {
    return m_ast;
}

void Parser::AstAddTerm(TermPtr &term) {
    ASSERT(m_ast);
    term->m_parser = this;
    term->m_source = lexer->source_string;
    if(m_ast->m_id == TermID::END) {
        m_ast = term;
        m_ast->ConvertSequenceToBlock(TermID::BLOCK, false);
    } else if(!m_ast->IsBlock()) {
        m_ast->ConvertSequenceToBlock(TermID::BLOCK, true);
    } else {
        ASSERT(m_ast->IsBlock());
        m_ast->m_block.push_back(term);
    }
}

void Parser::MacroLevelBegin(TermPtr &term) {
    m_macro_level++;
}

void Parser::MacroLevelEnd(TermPtr &term) {
    ASSERT(m_macro_level);
    m_macro_level--;
    if(m_macro) {
        m_macro->Clear(m_macro_level);
    }
}

void Parser::MacroTerm(TermPtr &term) {
    if(!m_macro) {
        Warning(term, "MacroBufferEmpty", "Buffer for macros not initialized!");
        return;
    }

    if(term->GetTokenID() == TermID::MACRO_DEL) {
        if(term->m_text.compare("_") == 0) {
            m_macro->Clear(0);
        } else if(!m_macro->Remove(term)) {
            Warning(term, "MacroNotFound", "Macros not found!");
        }
        return;
    } else if(m_macro->find(term->GetMacroId())) {
        NL_PARSER(term, "Macros '%s' duplicate!", term->toString().c_str());
        //        Warning(term, "MacroDuplicate", "Macros duplicate!");
        return;
    }

    LOG_DEBUG("!! Append macros %s %s", term->toString().c_str(), m_macro->Dump().c_str());

    m_macro->Append(term, m_macro_level);
    ASSERT(lexer->m_macro_count == 1);
    lexer->m_macro_count = 0;

    LOG_DEBUG("Macro Dump: %s", m_macro->Dump().c_str());
}

void Parser::Warning(TermPtr &term, const char *id, const char *message) {
    std::string empty;
    std::string str = newlang::ParserMessage(term->m_source ? *term->m_source : empty, term->m_line, term->m_col, "%s", message);

    utils::Logger::LogLevelType save = utils::Logger::Instance()->GetLogLevel();
    utils::Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
    LOG_INFO("%s", str.c_str());
    utils::Logger::Instance()->SetLogLevel(save);
}

void MacroBuffer::Append(const TermPtr &term, size_t level) {
    MacroToken item;
    item.level = level;
    item.macro = term;

    if(find(term->GetMacroId())) {
        LOG_RUNTIME("Macros dublicate %s!", term->toString().c_str());
    }

    //    std::string str;
    //    for (auto &elem : term->GetMacroId()) {
    //        if(!str.empty()) {
    //            str += " ";
    //        }
    //        str += elem;
    //    }
    //    LOG_DEBUG("Append '%s'    %s", str.c_str(), Dump().c_str());

    iterator iter = map::find(term->m_text);
    if(iter == end()) {
        std::vector<MacroToken> vect{item};
        insert(std::make_pair(term->m_text, vect));
    } else {
        iter->second.push_back(item);
    }
    //    std::cout << "   " << size() << "\n";
}

bool MacroBuffer::Remove(const TermPtr &term) {

    std::vector<std::string> list = term->GetMacroId();
    ASSERT(!list.empty());

    iterator found = map::find(list[0]);

    if(found != end()) {
        for (auto iter = found->second.begin(); iter != found->second.end(); ++iter) {

            std::vector<std::string> names = iter->macro->GetMacroId();

            //        for (auto &elem : names) {
            //            LOG_DEBUG("%s ", elem.c_str());
            //        }


            if(names.size() != list.size()) {
                continue;
            }

            for (int pos = 0; pos < list.size(); pos++) {
                if(!CompareTermName(list[pos].c_str(), names[pos].c_str())) {
                    goto skip_remove;
                }
            }

            found->second.erase(iter);

            if(found->second.empty()) {
                erase(list[0].c_str());
            }

            return true;

skip_remove:
            ;
        }
    }
    return false;
}

std::string MacroBuffer::Dump() {
    std::string result;
    auto iter = begin();
    while(iter != end()) {
        if(!result.empty()) {
            result += ", ";
        }

        int pos = 0;
        for (int pos = 0; pos < iter->second.size(); pos++) {

            std::string str;
            for (auto &elem : iter->second[pos].macro->GetMacroId()) {
                if(!str.empty()) {
                    str += " ";
                }
                str += elem;
            }
            result += iter->first + "->'" + str + "'";
        }


        iter++;
    }
    return result;
}

std::string MacroBuffer::Dump(MacroArgsType &var) {
    std::string result;
    auto iter = var.begin();
    while(iter != var.end()) {
        if(!result.empty()) {
            result += ", ";
        }

        int pos = 0;
        std::string str;
        for (int pos = 0; pos < iter->second.size(); pos++) {
            if(!str.empty()) {
                str += " ";
            }
            str += iter->second[pos]->m_text;
        }

        result += iter->first + "->'" + str + "'";
        iter++;
    }
    return result;
}

bool MacroBuffer::CompareTermName(const std::string & term_name, const std::string & macro_name) {
    if(isLocal(macro_name)) {
        // Шаблон соответствует любому термину входного буфера
        return true;
    }
    if(isMacro(term_name)) {
        // Если термин в буфере - имя макроса
        if(isMacro(macro_name)) {
            return macro_name.compare(term_name) == 0;
        }
        // Префикс макроса не учавтствует в сравнении
        return macro_name.compare(&term_name.c_str()[1]) == 0;

    } else if(isLocalAny(term_name.c_str())) {
        // Любой другой термин не подходит
        return false;
    }
    // Без префиксов оба термина
    return term_name.compare(macro_name) == 0;
}

MacroBuffer::CompareResult MacroBuffer::CompareMacro(LexerTokenType &buffer, const TermPtr &macro) {

    ASSERT(macro);
    ASSERT(macro->m_follow.size());

    if(buffer.size() < macro->m_follow.size()) {
        return CompareResult::NEXT_NAME;
    }

    int buff_offset = 0;
    int macro_offset = 0;
    while(macro_offset < macro->m_follow.size() && buff_offset < buffer.size()) {

        //        LOG_DEBUG("TermID: %s, '%s'  '%s'", toString(buffer[buff_offset]->getTermID()),
        //                buffer[buff_offset]->m_text.c_str(), macro->m_follow[macro_offset]->m_text.c_str());
        // Текст термина сравнивается только для опредленных терминов
        if(!IsMacroTermID(buffer[buff_offset]->getTermID()) ||
                !CompareTermName(buffer[buff_offset]->m_text, macro->m_follow[macro_offset]->m_text)) {
            return CompareResult::NOT_EQ;
        }

        if(macro->m_follow[macro_offset]->isCall()) {
            // Пропускаем скобки и все что в них находится
            buff_offset++;
            if(buff_offset >= buffer.size()) {
                // Нет открывающей скобки, но запросим лексемы до закрывающей скобки включительно
                return CompareResult::NEXT_BRAKET;
            }

            if(buffer[buff_offset]->getTermID() != TermID::SYMBOL || buffer[buff_offset]->m_text.compare("(") != 0) {
                return CompareResult::NOT_EQ;
            }

            size_t braket_level = 1;
            buff_offset++;
            while(braket_level && buff_offset < buffer.size()) {
                if(buffer[buff_offset]->getTermID() == TermID::SYMBOL && buffer[buff_offset]->m_text.compare("(") == 0) {
                    braket_level++;
                } else if(buffer[buff_offset]->getTermID() == TermID::SYMBOL && buffer[buff_offset]->m_text.compare(")") == 0) {
                    braket_level--;
                }
                buff_offset++;
            }
            if(braket_level && buff_offset >= buffer.size()) {
                // Нет закрывающий скобки
                return CompareResult::NEXT_BRAKET;
            }
        }

        macro_offset++;

        if(macro_offset == macro->m_follow.size()) {
            LOG_DEBUG("Macro '%s' done for %d lexem!", macro->toString().c_str(), buff_offset);
            return CompareResult::DONE;
        }

        buff_offset++;
    }
    // Нужен следующий термин для сопоставления
    return CompareResult::NEXT_NAME;
}

/*
 * $var macros -> $var
 * macros($var) -> $var $*
 * macros($var) second -> $var $*
 * first macros($var) second -> $var $*
 * first macros($var, $var2) $second -> $var $var2 $* $second
 * $first macros($var) $second -> $first $var $* $second
 */
void MacroBuffer::InsertArg_(MacroArgsType & args, std::string name, LexerTokenType &buffer, size_t pos) {
    if(pos >= buffer.size()) {
        LOG_RUNTIME("Empty data input buffer!");
    }
    if(args.find(name) != args.end()) {
        LOG_RUNTIME("Duplicate arg %s!", name.c_str());
    }
    BlockType vect;
    vect.push_back(buffer[pos]);
    args.insert(std::make_pair(name, vect));
}

void MacroBuffer::InsertArg_(MacroArgsType & args, std::string name, BlockType &data) {
    if(args.find(name) != args.end()) {
        LOG_RUNTIME("Duplicate arg %s!", name.c_str());
    }
    args.insert(std::make_pair(name, data));
}

BlockType MacroBuffer::SymbolSeparateArg_(LexerTokenType &buffer, size_t &pos, std::vector<std::string> name, std::string &error) {
    error.clear();
    BlockType result;
    while(pos < buffer.size()) {
        if(buffer[pos]->GetTokenID() == TermID::SYMBOL) {
            for (auto &elem : name) {
                if(buffer[pos]->m_text.compare(elem) == 0) {
                    return result;
                }
            }

        }
        result.push_back(buffer[pos]);
        pos++;
    }

    for (auto &elem : name) {
        if(!error.empty()) {
            error += " or ";
        }
        error += "'";
        error += elem;
        error += "'";
    }

    error.insert(0, "Expected symbol ");
    error += "!";
    return result;
}

size_t MacroBuffer::ExtractArgs(LexerTokenType &buffer, const TermPtr &term, MacroArgsType &args) {

    ASSERT(term);

    if(!((term->getTermID() == TermID::MACRO || term->getTermID() == TermID::MACRO_BODY ||
            term->getTermID() == TermID::MACRO_DEF || term->getTermID() == TermID::MACRO_STR || term->getTermID() == TermID::MACRO_DEL))) {

        LOG_RUNTIME("Term is not a macro! '%s'", term->toString().c_str());

    }

    args.clear();

    size_t pos = 0; //Позиция во входном буфере
    BlockType all_args;
    bool all_args_done = false;
    for (int i = 0; i < term->m_follow.size(); i++) {
        TermPtr macro = term->m_follow[i];

        if(isLocal(macro->m_text)) {
            InsertArg_(args, macro->m_text, buffer, pos);
        }

        if(macro->isCall()) {
            if(pos + 1 >= buffer.size()) {
                LOG_RUNTIME("No data input buffer!");
            }
            pos++;
            if(buffer[pos]->GetTokenID() != TermID::SYMBOL || buffer[pos]->m_text.compare("(") != 0) {
                LOG_RUNTIME("Expected open bracket!");
            }
            pos++;

            if(all_args_done) {
                LOG_RUNTIME("Support single term call only!");
            }
            all_args_done = true;
            for (int arg_pos = pos; arg_pos < buffer.size(); arg_pos++) {
                if(buffer[arg_pos]->m_text == ")") {
                    break;
                }
                all_args.push_back(buffer[arg_pos]);
            }

            size_t num_arg = 1;
            std::string arg_name;
            BlockType next;
            while(1) {
                next = SymbolSeparateArg_(buffer, pos,{")", ","}, arg_name);

                if(!arg_name.empty()) {
                    LOG_RUNTIME("%s", arg_name.c_str());
                }

                if(!next.empty()) {
                    arg_name = "\\$";
                    arg_name += std::to_string(num_arg);
                    InsertArg_(args, arg_name, next);

                    if(num_arg - 1 < macro->size()) {
                        arg_name = macro->at(num_arg - 1).second->m_text;
                        if(arg_name.find("$") == 0) {
                            arg_name.insert(0, "\\");
                        } else {
                            arg_name.insert(0, "\\$");
                        }
                        InsertArg_(args, arg_name, next);
                    }
                    num_arg++;

                } else if(buffer[pos]->m_text == ",") {
                    pos++;
                } else if(buffer[pos]->m_text == ")") {
                    arg_name = "\\$*";
                    InsertArg_(args, arg_name, all_args);
                    break;
                } else {
                    LOG_RUNTIME(" %s ,,,,,, ?????????", buffer[pos]->m_text.c_str());
                }
            }
        }

        if((i + 1) == term->m_follow.size() && pos < buffer.size()) {
            ASSERT(pos <= buffer.size());
            return pos + 1;
        }
        pos++;
    }

    LOG_RUNTIME("Input buffer empty for extract args macros %s!", term->toString().c_str());
}

BlockType MacroBuffer::ExpandMacros(const TermPtr &macro, MacroArgsType &args) {
    ASSERT(macro);
    if(macro->m_id != TermID::MACRO_DEF) {
        LOG_RUNTIME("Fail convert term type %s as macros!", toString(macro->m_id));
    }
    ASSERT(macro->Right());

    BlockType result;
    for (int i = 0; i < macro->Right()->m_follow.size(); i++) {
        TermPtr check = macro->Right()->m_follow[i];

        if(check->m_text.find("\\$") == 0) {

            auto iter = args.find(check->m_text);
            if(iter == args.end()) {
                LOG_RUNTIME("Name %s not found!", check->m_text.c_str());
            }
            result.insert(result.end(), iter->second.begin(), iter->second.end());

        } else {
            result.push_back(check);
        }
    }
    return result;
}

std::string MacroBuffer::ExpandString(const TermPtr &macro, MacroArgsType & args) {
    ASSERT(macro);
    if(macro->m_id != TermID::MACRO_STR) {
        LOG_RUNTIME("Fail convert term type %s as macros string!", toString(macro->m_id));
    }
    ASSERT(macro->Right());

    LOG_DEBUG("\"%s\"", macro->Right()->m_text.c_str());

    std::string body(macro->Right()->m_text);

    for (auto &elem : args) {

        std::string name;
        if(elem.first.find("\\$*") == 0) {
            name = "\\\\\\$\\*";
        } else if(elem.first.find("\\$") == 0) {
            name = "\\\\";
            name += elem.first;
        } else {
            ASSERT(elem.first.find("$") == 0);
            name = "\\\\\\";
            name += elem.first;
        }

        std::string text;
        for (auto &lex : elem.second) {
            text += lex->toString();
            text += " ";
        }

        LOG_DEBUG("Regex: '%s' -> '%s' in \"%s\"", name.c_str(), text.c_str(), body.c_str());
        body = std::regex_replace(body, std::regex(name), text);
    }

    return body;
}

TermPtr MacroBuffer::find(std::vector<std::string> list) {
    if(list.empty()) {
        return nullptr;
    }
    iterator found = map::find(list[0]);

    if(found != end()) {
        for (auto iter = found->second.begin(); iter != found->second.end(); ++iter) {

            std::vector<std::string> names = iter->macro->GetMacroId();

            //        for (auto &elem : names) {
            //            LOG_DEBUG("%s ", elem.c_str());
            //        }


            if(names.size() != list.size()) {
                continue;
            }

            for (int pos = 0; pos < list.size(); pos++) {
                if(!CompareTermName(list[pos].c_str(), names[pos].c_str())) {
                    goto skip_step;
                }
            }
            return iter->macro;
skip_step:
            ;
        }
    }
    return nullptr;
}

parser::token_type Parser::NewToken(TermPtr * yylval, parser::location_type * yylloc) {

    parser::token_type result;

    ASSERT(yylval);
    ASSERT(yylloc);


    MacroBuffer::CompareResult compare_res = MacroBuffer::CompareResult::NEXT_NAME;

    size_t max_count = 20;

    size_t counter = 0;

    if(!m_is_lexer_complete) {

        m_is_runing = true;

        TermPtr term;
        while(1) {

            parser::location_type loc;

            term = Term::Create(parser::token_type::END, TermID::END, "", 0);
            parser::token_type type = lexer->lex(&term, &loc);
            term->m_lexer_loc = loc;

            ASSERT(type == term->m_lexer_type);

            if(lexer->m_macro_count == 3) {

                ASSERT(type == parser::token_type::MACRO_DEF);
                lexer->m_macro_count = 1;
                term = lexer->m_macro_body;
                type = parser::token_type::MACRO_BODY;

            } else if(lexer->m_macro_count == 2) {

                if(type == parser::token_type::MACRO_DEF) {
                    lexer->m_macro_body = term;
                    lexer->m_macro_body->SetTermID(TermID::MACRO_BODY);
                    lexer->m_macro_body->m_lexer_type = parser::token_type::MACRO_BODY;
                } else {
                    ASSERT(lexer->m_macro_body);
                    lexer->m_macro_body->m_follow.push_back(term);
                }
                continue;

            } else if(type == parser::token_type::END) {

                if(lexer->m_data.empty()) {
                    m_is_lexer_complete = true;
                } else {
                    lexer->yypop_buffer_state();
                    *lexer->m_loc = lexer->m_data[lexer->m_data.size() - 1].loc;

                    delete lexer->m_data[lexer->m_data.size() - 1].iss;
                    lexer->m_data.pop_back();

                    if(lexer->m_data.empty()) {
                        lexer->source_string = lexer->source_base;
                    } else {
                        lexer->source_string = lexer->m_data[lexer->m_data.size() - 1].data;
                    }
                    continue;
                }
            }

            m_prep_buff.push_back(term);

            if(type != parser::token_type::END && compare_res == MacroBuffer::CompareResult::NEXT_BRAKET) {
                if(term->getTermID() == TermID::SYMBOL && term->m_text.compare(")") == 0) {
                    compare_res = MacroBuffer::CompareResult::NEXT_NAME;
                } else {
                    continue;
                }
            }

            if(m_macro && lexer->m_macro_count == 0 &&
                    IsMacroTermID(m_prep_buff.front()->GetTokenID()) &&
                    compare_res == MacroBuffer::CompareResult::NEXT_NAME) {

                // Раскрывать макросы если нет других макров и ищется имя для идентификации


                bool expand_complete = false;

                LOG_DEBUG("Hash: %s", m_prep_buff[0]->m_text.c_str());

                TermPtr macro_done = nullptr;

                // Найти макрос, который может соответствовать текущему буферу (по первому термину буфера)
                MacroBuffer::iterator found = m_macro->map::find(m_prep_buff[0]->m_text);
                for (auto iter = found->second.begin(); found != m_macro->end() && iter != found->second.end(); ++iter) {

                    compare_res = MacroBuffer::CompareMacro(m_prep_buff, iter->macro);

                    if(compare_res == MacroBuffer::CompareResult::NEXT_NAME || compare_res == MacroBuffer::CompareResult::NEXT_BRAKET) {
                        macro_done.reset();
                        break;
                    } else if(compare_res == MacroBuffer::CompareResult::DONE) {
                        macro_done = iter->macro;
                    }
                }

                if(macro_done) {

                    compare_res = MacroBuffer::CompareResult::NEXT_NAME; // Раскрывать макросы в теле раскрытого макроса

                    ASSERT(macro_done);
                    ASSERT(m_prep_buff.size() >= macro_done->m_follow.size());
                    ASSERT(macro_done->Right());

                    MacroBuffer::MacroArgsType macro_args;
                    size_t size_remove = MacroBuffer::ExtractArgs(m_prep_buff, macro_done, macro_args);

                    ASSERT(size_remove);
                    ASSERT(size_remove <= m_prep_buff.size());

                    m_prep_buff.erase(m_prep_buff.begin(), m_prep_buff.begin() + size_remove);

                    if(macro_done->GetTokenID() == TermID::MACRO_DEF) {

                        ASSERT(macro_done->Right());

                        BlockType macro_block = MacroBuffer::ExpandMacros(macro_done, macro_args);
                        for (int i = macro_block.size(); i > 0; i--) {
                            m_prep_buff.insert(m_prep_buff.begin(), macro_block[i - 1]);
                        }

                    } else {

                        LOG_DEBUG("%s", toString(macro_done->GetTokenID()));
                        ASSERT(macro_done->GetTokenID() == TermID::MACRO_STR);
                        ASSERT(m_prep_buff.size() == 0);

                        if(lexer->m_data.size() > 100) {
                            LOG_RUNTIME("Macro expansion '%s' stack overflow?", macro_done->toString().c_str());
                        }

                        std::string macro_str = MacroBuffer::ExpandString(macro_done, macro_args);
                        lexer->source_string = std::make_shared<std::string>(MacroBuffer::ExpandString(macro_done, macro_args));
                        lexer->m_data.push_back({lexer->source_string, new std::istringstream(*lexer->source_string), *lexer->m_loc});
                        lexer->m_loc->initialize();
                        lexer->yypush_buffer_state(lexer->yy_create_buffer(lexer->m_data[lexer->m_data.size() - 1].iss, lexer->source_string->size()));

                    }
                }

            } else {
                break;
            }

            if(m_is_lexer_complete || compare_res == MacroBuffer::CompareResult::NOT_EQ) {
                break;
            }
        }

        //LOG_DEBUG("LexerToken count %d", (int) m_prep_buff.size());

    }


    if(!m_prep_buff.empty()) {

        //        LOG_DEBUG("%d  %s", (int)m_prep_buff.at(0)->m_lexer_type, m_prep_buff.at(0)->m_text.c_str());

        *yylval = m_prep_buff.at(0);
        *yylloc = m_prep_buff.at(0)->m_lexer_loc;
        result = m_prep_buff.at(0)->m_lexer_type;

        //        LOG_DEBUG("Token (%d): %s", result, (*yylval)->m_text.c_str());

        m_prep_buff.pop_front();
        return result;
    }

    *yylval = nullptr;

    return parser::token_type::END;
}

std::string newlang::ParserMessage(std::string &buffer, int row, int col, const char *format, ...) {

    char va_buffer[1024];

    va_list args;
    va_start(args, format);
    vsnprintf(va_buffer, sizeof (va_buffer), format, args);
    va_end(args);

    std::string message(va_buffer);

    if(row) { // Если переданы координаты ошибки
        message += " at line ";
        message += std::to_string(row);
        message += " col ";
        message += std::to_string(col);
        message += "\n";
    }

    // Ищем нужную строку
    size_t pos = 0;
    if(buffer.find("\n") != std::string::npos) {
        int count = 1;
        while(count < row) {
            pos = buffer.find("\n", pos + 1);
            count++;
        }
    }
    // Убираем лишние переводы строк
    std::string tmp = buffer.substr((pos ? pos + 1 : pos), buffer.find("\n", pos + 1));
    tmp = tmp.substr(0, tmp.find("\n", col));

    if(row) { // Если переданы координаты ошибки, показываем место

        // Лексер обрабатывает строки в байтах, а вывод в UTF8
        // поэтому позиция ошибки лексера може не совпадать для многобайтных символов
        std::wstring_convert < std::codecvt_utf8<wchar_t>, wchar_t> converter;
        std::wstring wstr = converter.from_bytes(tmp.substr(0, col));

        message += tmp + "\n";
        std::string placeholder(col - 1 - (tmp.substr(0, col).size() - wstr.size()), ' ');
        placeholder += "^\n";
        message += placeholder;
    } else {

        message += tmp;
    }

    return message;
}

void newlang::ParserException(const char *msg, std::string &buffer, int row, int col) {
    throw Return(ParserMessage(buffer, row, col, "%s", msg), Return::Parser);
}
