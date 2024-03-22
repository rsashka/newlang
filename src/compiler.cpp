#include "pch.h"

#include "types.h"
#include "compiler.h"

using namespace newlang;

//LLVMBuilderRef RunTime::m_llvm_builder = nullptr;
//LLVMModuleRef RunTime::m_llvm_module = nullptr;
//LLVMExecutionEngineRef RunTime::m_llvm_engine = nullptr;

bool newlang::Tranliterate(const wchar_t c, std::wstring &str) {
    switch (c) {
        case L'а': str.append(L"a");
            return true;
        case L'б': str.append(L"b");
            return true;
        case L'в': str.append(L"v");
            return true;
        case L'г': str.append(L"g");
            return true;
        case L'д': str.append(L"d");
            return true;
        case L'е': str.append(L"e");
            return true;
        case L'ё': str.append(L"ye");
            return true;
        case L'ж': str.append(L"zh");
            return true;
        case L'з': str.append(L"z");
            return true;
        case L'и': str.append(L"i");
            return true;
        case L'й': str.append(L"y");
            return true;
        case L'к': str.append(L"k");
            return true;
        case L'л': str.append(L"l");
            return true;
        case L'м': str.append(L"m");
            return true;
        case L'н': str.append(L"n");
            return true;
        case L'о': str.append(L"o");
            return true;
        case L'п': str.append(L"p");
            return true;
        case L'р': str.append(L"r");
            return true;
        case L'с': str.append(L"s");
            return true;
        case L'т': str.append(L"t");
            return true;
        case L'у': str.append(L"u");
            return true;
        case L'ф': str.append(L"f");
            return true;
        case L'х': str.append(L"h");
            return true;
        case L'ц': str.append(L"z");
            return true;
        case L'ч': str.append(L"ch");
            return true;
        case L'ш': str.append(L"sh");
            return true;
        case L'щ': str.append(L"sch");
            return true;
        case L'ы': str.append(L"y");
            return true;
        case L'э': str.append(L"e");
            return true;
        case L'ю': str.append(L"yu");
            return true;
        case L'я': str.append(L"ya");
            return true;
        case L'А': str.append(L"A");
            return true;
        case L'Б': str.append(L"B");
            return true;
        case L'В': str.append(L"V");
            return true;
        case L'Г': str.append(L"G");
            return true;
        case L'Д': str.append(L"D");
            return true;
        case L'Е': str.append(L"E");
            return true;
        case L'Ё': str.append(L"YE");
            return true;
        case L'Ж': str.append(L"ZH");
            return true;
        case L'З': str.append(L"Z");
            return true;
        case L'И': str.append(L"I");
            return true;
        case L'Й': str.append(L"Y");
            return true;
        case L'К': str.append(L"K");
            return true;
        case L'Л': str.append(L"L");
            return true;
        case L'М': str.append(L"M");
            return true;
        case L'Н': str.append(L"N");
            return true;
        case L'О': str.append(L"O");
            return true;
        case L'П': str.append(L"P");
            return true;
        case L'Р': str.append(L"R");
            return true;
        case L'С': str.append(L"S");
            return true;
        case L'Т': str.append(L"T");
            return true;
        case L'У': str.append(L"U");
            return true;
        case L'Ф': str.append(L"F");
            return true;
        case L'Х': str.append(L"H");
            return true;
        case L'Ц': str.append(L"Z");
            return true;
        case L'Ч': str.append(L"CH");
            return true;
        case L'Ш': str.append(L"SH");
            return true;
        case L'Щ': str.append(L"SCH");
            return true;
        case L'Ы': str.append(L"Y");
            return true;
        case L'ъ': str.append(L"_");
        case L'ь': str.append(L"_");
        case L'Ь': str.append(L"_");
        case L'Ъ': str.append(L"_");
            return true;
        case L'Э': str.append(L"E");
            return true;
        case L'Ю': str.append(L"YU");
            return true;
        case L'Я': str.append(L"YA");
            return true;
        default:
            str.append(std::to_wstring(static_cast<unsigned int> (c)));
    }
    return true;
}


//_Z5name1PN7newlang7ContextERNS_4ObjectE
//_Z6name22PN7newlang7ContextERNS_4ObjectE
//_Z7name333PN7newlang7ContextERNS_4ObjectE
// Для типа
//typedef ObjPtr FunctionType(Context *ctx, Object &in);

std::string newlang::MangaledFuncCPP(const char *name, const char *space) {
    std::string result("_Z");
    result += std::to_string(strlen(name));
    result += name;
    result += "PN7newlang7ContextERNS_4ObjectE";
    return result;
}

//Int8    _ZN7newlang4CharEPKNS_7ContextERKNS_6ObjectE
//Int16   _ZN7newlang5ShortEPKNS_7ContextERKNS_6ObjectE
//Char_   _ZN7newlang5Char_EPNS_7ContextERNS_6ObjectE
//Short_  _ZN7newlang6Short_EPNS_7ContextERNS_6ObjectE

std::string newlang::MangaledFunc(const std::string name) {
    std::string result("_ZN7newlang");
    result += std::to_string(name.size());
    result += name;
    result += isMutableName(name) ? "EPNS_7ContextERNS_6ObjectE" : "EPKNS_7ContextERKNS_6ObjectE";
    return result;
}

std::string newlang::MangleName(const char * name) {

#define ALLOW_CHAR L"_ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
    static const std::wstring first_char(ALLOW_CHAR);
    static const std::wstring next_char(L"0123456789" ALLOW_CHAR);
#undef ALLOW_CHAR
    std::wstring_convert < std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::wstring temp = converter.from_bytes(name);

    std::wstring result = converter.from_bytes(NEWLANG_PREFIX "_");
    if (temp.size()) {
        size_t len = temp.size();
        for (size_t i = 0; i < len; i++) {
            if (i == 0 && first_char.find(temp[i]) != std::wstring::npos) {
                result += temp[i]; // Первый символ имени только буква или подчерк
            } else if (i != 0 && next_char.find(temp[i]) != std::wstring::npos) {
                result += temp[i]; // Последующие могут содержать ещё и цифры
            } else {
                Tranliterate(temp[i], result); // Иначе конвертировать в ASCII символы
            }
        }
    }
    return converter.to_bytes(result);
}


//TermPtr CompileInfo::isFunction(TermPtr term) {
//    ASSERT(term);
//    if (term) {
//        std::string local_name = term->GetFullName();
//        if (functions.find(local_name) != functions.end()) {
//            return functions.find(local_name)->second;
//        }
//        if (builtin.find(local_name) != builtin.end()) {
//            return builtin.find(local_name)->second;
//        }
//    }
//    return nullptr;
//}
//
//TermPtr CompileInfo::isArgument(TermPtr term) {
//    ASSERT(term);
//    if (term) {
//        std::string local_name = MakeName(term->GetFullName().c_str());
//        if (arguments.find(local_name) != arguments.end()) {
//            return arguments.find(local_name)->second;
//        }
//    }
//    return nullptr;
//}
//
//TermPtr CompileInfo::isVariable(TermPtr term) {
//    ASSERT(term);
//    if (term) {
//        std::string local_name = MakeName(term->GetFullName().c_str());
//        if (variables.find(local_name) != variables.end()) {
//            return variables.find(local_name)->second;
//        }
//    }
//    return nullptr;
//}
//
//TermPtr CompileInfo::findObject(std::string name) {
//    name = MakeName(name);
//    if (arguments.find(name) != arguments.end()) {
//        return arguments.find(name)->second;
//    }
//    if (variables.find(name) != variables.end()) {
//        return variables.find(name)->second;
//    }
//    if (consts.find(name) != consts.end()) {
//        return consts.find(name)->second;
//    }
//    if (functions.find(name) != functions.end()) {
//        return functions.find(name)->second;
//    }
//    if (builtin.find(name) != builtin.end()) {
//        return builtin.find(name)->second;
//    }
//    return nullptr;
//}
//
//bool CompileInfo::isLocalAccess(TermPtr term) {
//    ASSERT(term);
//    std::string name = MakeName(term->GetFullName());
//    return variables.find(name) != variables.end() || consts.find(name) != consts.end();
//}
//
//void Compiler::WriteDeclarationFunctions_(CompileInfo &ci, TermPtr &func, std::ostream & out, std::vector<std::string> &func_list) {
//    if (func->IsFunction()) {
//        ASSERT(func->Left());
//        ci.functions.insert(std::pair<std::string, TermPtr>(func->Left()->GetFullName().c_str(), func->Left()));
//        WriteFunctionName_(func, out);
//        out << ";\n";
//        out << "extern \"C\" const ObjPtr " << MangleName(func->Left()->GetFullName().c_str()) << "_arguments;\n";
//        out << "inline " NEWLANG_NS "::ObjPtr " << MangleName(func->Left()->GetFullName().c_str()) << "(" NEWLANG_NS "::Context *ctx, " NEWLANG_NS "::ObjPtr args) {\n";
//        out << "ObjPtr temp=" << MangleName(func->Left()->GetFullName().c_str()) << "_arguments->Call" << "(*args);\n";
//        //            out << NEWLANG_INDENT_OP "args->UpdateFrom( *" << MangleName(func->Left()->GetFullName().c_str()) << "_arguments" << ");\n";
//
//        out << NEWLANG_INDENT_OP "return " << MangleName(func->Left()->GetFullName().c_str()) << "(ctx, *temp);\n";
//        out << "}\n";
//        out << "extern \"C\" const char * " << MangleName(func->Left()->GetFullName().c_str()) << "_text;\n";
//        func_list.push_back(func->Left()->GetFullName());
//    } else if (func->getTermID() == TermID::BLOCK) {
//        for (TermPtr &elem : func->BlockCode()) {
//            WriteDeclarationFunctions_(ci, elem, out, func_list);
//        }
//    }
//}
//
//std::string Compiler::WriteFunctionCheckOp_(CompileInfo &ci, TermPtr &op, const char *check_true, const char *check_false) {
//    std::string str;
//
//    std::string result;
//
//    auto indent = ci.NewIndent();
//
//    result = ci.GetIndent() + "if(static_cast<bool>(";
//    TermID id = op->getTermID();
//    if (id == TermID::NAME || op->isCall()) {
//
//        GetImpl(ci, op, result);
//        result += "->GetValueAsBoolean()";
//
//    } else if (id == TermID::OPERATOR || id == TermID::CREATE_ONCE) {
//
//        result += "(*";
//
//        GetImpl(ci, op->Left(), result);
//
//        if (op->getText().compare(":=") == 0) {
//            result += "=";
//        } else {
//            result += op->getText();
//            result += " *";
//        }
//
//        GetImpl(ci, op->Right(), result);
//        result += ")->GetValueAsBoolean()";
//
//    } else {
//        LOG_RUNTIME("Check operation '%s' as type %s not implemented!", op->toString().c_str(), newlang::toString(id));
//    }
//    result += ")) {\n  " + ci.GetIndent();
//    result += check_true;
//    result += ";\n  } else {\n  " + ci.GetIndent();
//    result += check_false;
//    result += ";\n" + ci.GetIndent() + "}\n";
//
//    return result;
//}
//
//void Compiler::SelectTerms_(TermPtr &obj, std::vector<TermPtr> &terms) {
//    if (obj->getTermID() == TermID::NAME) {
//        terms.push_back(obj);
//    }
//    if (obj->Left()) {
//        SelectTerms_(obj->m_left, terms);
//    }
//    if (obj->Right()) {
//
//        SelectTerms_(obj->m_right, terms);
//    }
//}
//
//bool Compiler::WriteFunctionName_(TermPtr &func, std::ostream & out, bool is_transparent) {
//    out << "extern \"C\" " NEWLANG_NS "::ObjPtr " << MangleName(func->Left()->GetFullName().c_str()) << "(" NEWLANG_NS "::Context " << (is_transparent ? "const" : "") << " *ctx, " NEWLANG_NS "::Object &in)";
//
//    return true;
//}
//
//std::string Compiler::MakeCommentLine(std::string comment) {
//    comment = std::regex_replace(comment, std::regex("\n"), "\\n");
//    comment = std::regex_replace(comment, std::regex("\""), "@\"");
//
//    return "// " + comment + "\n";
//}
//
//std::string Compiler::WriteSimpleBody_(CompileInfo &ci, TermPtr &func) {
//    ASSERT(!"Not implemented");
//    //    auto indent = ci.NewIndent();
//    //    WriteFunctionOp *func_op;
//    //    if(func->getTermID() != TermID::SIMPLE) {
//    //        LOG_RUNTIME("Term is not a simple pure function %s(%d) %s", newlang::toString(func->getTermID()), (int) func->getTermID(), func->toString().c_str());
//    //    }
//    //
//    //    if(func->getText().compare(":&&=") == 0) {
//    //        func_op = &WriteSimpleBodyAND_;
//    //    } else if(func->getText().compare(":||=") == 0) {
//    //        func_op = &WriteSimpleBodyOR_;
//    //    } else if(func->getText().compare(":^^=") == 0) {
//    //        func_op = &WriteSimpleBodyXOR_;
//    //    } else {
//    //        LOG_RUNTIME("Unknown function type %s(%d) %s", newlang::toString(func->getTermID()), (int) func->getTermID(), func->toString().c_str());
//    //    }
//    //
//    //    std::string temp;
//    //    std::string result = (*func_op)(ci, func, FunctionStep::PREPARE);
//    //
//    //    if(func->Right()->getTermID() == TermID::BLOCK) {
//    //        for (auto &elem : func->Right()->BlockCode()) {
//    //
//    //            // В комментарий добавляется исходный код, в котором могут быть текстовые строки с управляющими символами
//    //            result += "\n" + ci.GetIndent() + MakeCommentLine(elem->toString());
//    //
//    //            temp = (*func_op)(ci, elem, FunctionStep::OPERATION);
//    //            //                ReplaceFuncArgs(func, temp);
//    //            result += temp;
//    //
//    //        }
//    //    } else {
//    //
//    //        TermPtr term = func->Right();
//    //
//    //        // В комментарий добавляется исходный код, в котором могут быть текстовые строки с управляющими символами
//    //        result += "\n" + ci.GetIndent() + MakeCommentLine(term->toString());
//    //
//    //        temp = (*func_op)(ci, term, FunctionStep::OPERATION);
//    //
//    //        //            ReplaceFuncArgs(func, temp);
//    //        result += temp;
//    //    }
//    //
//    //    result += (*func_op)(ci, func, FunctionStep::COMPLETE);
//    //
//    //    return result;
//    return "";
//}



//bool Compiler::MakeFunctionCpp(CompileInfo &ci, std::string func_name, TermPtr &func_define, std::ostream &out) {
//    LOG_RUNTIME("MakeFunctionCpp Not implemeneted!");
//    //    if(!func_define->IsFunction()) {
//    //        LOG_RUNTIME("No function name");
//    //    }
//    //    if(!func_define->Right()) {
//    //        LOG_RUNTIME("No function body");
//    //    }
//    //
//    //    std::string text = func_define->toString();
//    //    text = std::regex_replace(text, std::regex("\""), "@\"");
//    //    text = std::regex_replace(text, std::regex("\n"), "\\n");
//    //    text = std::regex_replace(text, std::regex("\t"), "@t");
//    //    text = std::regex_replace(text, std::regex("\r"), "@r");
//    //    text = std::regex_replace(text, std::regex("\b"), "@b");
//    //    text = std::regex_replace(text, std::regex("\f"), "@f");
//    //    out << "const char * " << MangleName(func_define->Left()->GetFullName().c_str()) << "_text";
//    //    out << "=\"" << text << "\";\n";
//    //
//    //    out << "const ObjPtr " << MangleName(func_define->Left()->GetFullName().c_str()) << "_arguments";
//    //    out << "=Obj::CreateDict(Obj::ArgNull(\"self\")";
//    //    if(func_define->Left()->size()) {
//    //        for (size_t i = 0; i < func_define->Left()->size(); i++) {
//    //            out << ", ";
//    //            std::string var_name;
//    //            if(func_define->Left()->name(i).empty()) {
//    //                var_name = func_define->Left()->at(i).second->GetFullName();
//    //                out << "Obj::ArgNull(";
//    //
//    //            } else {
//    //                out << "Obj::Arg(";
//    //                var_name = func_define->Left()->name(i);
//    //                std::string impl;
//    //                GetImpl(ci, func_define->Left()->at(i).second, impl);
//    //                out << impl;
//    //                out << ", ";
//    //            }
//    //            out << "\"" << var_name << "\")";
//    //        }
//    //    }
//    //    out << ");";
//    //    out << " // Default function args \n";
//    //
//    //
//    //    WriteFunctionName_(func_define, out);
//    //    out << " {\n";
//    //
//    //
//    //    // Аргументы функции с локальным доступом по имени или индексу
//    //    if(func_define->Left()->size()) {
//    //        for (size_t i = 0; i < func_define->Left()->size(); i++) {
//    //            std::string var_name;
//    //            if(func_define->Left()->name(i).empty()) {
//    //                var_name = func_define->Left()->at(i).second->GetFullName();
//    //            } else {
//    //                var_name = func_define->Left()->name(i);
//    //            }
//    //            ci.arguments.insert(std::pair<std::string, TermPtr>(var_name, func_define->Left()));
//    //        }
//    //    }
//    //
//    //    std::string body;
//    //    if(func_define->getTermID() == TermID::SIMPLE) {
//    //        body = WriteSimpleBody_(ci, func_define);
//    //    } else {
//    //        body = MakeFunctionBodyCpp(ci, func_define->Right());
//    //    }
//    //
//    //    out << body;
//    //
//    //    // Проверка типа возвращаемого значения из функции
//    //    NL_TYPECHECK(func_define->Left(), func_define->Left()->m_type_name, ci.last_type);
//    //
//    //    out << "}\n";
//    //
//    //    ci.arguments.clear();
//    //
//    //    return true;
//
//}
//
//void Compiler::MakeCppFileFunctions(CompileInfo &ci, TermPtr ast, std::ostream &out) {
//    if (ast->getTermID() == TermID::BLOCK) {
//        for (TermPtr &elem : ast->BlockCode()) {
//            MakeCppFileFunctions(ci, elem, out);
//        }
//    } else if (ast->IsFunction()) {
//        //        out << "#line " << ast->GetLine() << "\n";
//        MakeFunctionCpp(ci, ast->Left()->getText(), ast, out);
//    }
//}
//
//void Compiler::MakeCppFileConstants(CompileInfo &ci, TermPtr ast, std::ostream &out) {
//    //    if(ast->getTermID() == TermID::BLOCK) {
//    //        for (TermPtr &elem : ast->BlockCode()) {
//    //            MakeCppFileConstants(ci, elem, out);
//    //        }
//    //    } else if(ast->getTermID() == TermID::CREATE_ONCE && isConst(ast->Left()->m_text.c_str())) {
//    //        ASSERT(ast->Left());
//    //        ASSERT(ast->Right());
//    //
//    //        //        out << "#line " << ast->GetLine() << "\n";
//    //        std::string impl_name = MakeLocalName(ast->Left()->GetFullName().c_str());
//    //        ci.consts.insert(std::pair<std::string, TermPtr>(MakeName(ast->Left()->GetFullName()), ast->Left()));
//    //        NL_TYPECHECK(ast->Left(), ast->Right()->m_type_name, ast->Left()->m_type_name);
//    //
//    //        out << ci.GetIndent() << "static const ObjPtr " << impl_name;
//    //        std::string impl;
//    //        GetImpl(ci, ast->Right(), impl);
//    //        out << "=" << impl << "->MakeConst(); ";
//    //        out << "// " << ast->toString() << "\n";
//    //    }
//}
//
//std::string Compiler::MakeFunctionBodyCpp(CompileInfo &ci, TermPtr ast) {
//    std::string result;
//    if (ast->getTermID() == TermID::BLOCK) {
//        for (size_t i = 0; i < ast->BlockCode().size(); i++) {
//            if (i < ast->BlockCode().size() - 1) {
//                result += MakeSequenceOpsCpp(ci, ast->BlockCode()[i], false);
//            } else {
//                result += MakeSequenceOpsCpp(ci, ast->BlockCode()[i], true);
//            }
//        }
//    } else {
//        result = MakeSequenceOpsCpp(ci, ast, true);
//    }
//    return result;
//}
//
//std::string Compiler::MakeSequenceOpsCpp(CompileInfo &ci, TermPtr ast, bool top_level) {
//
//    //    auto indent = ci.NewIndent();
//    //    std::ostringstream ostr;
//    std::string result;
//    //    std::string temp;
//    //
//    //    std::string last_item;
//    //    TermPtr last_term = ast;
//    //    if(ast->getTermID() == TermID::BLOCK) {
//    //
//    //        for (size_t i = 0; i < ast->BlockCode().size(); i++) {
//    //            TermPtr elem = ast->BlockCode()[i];
//    //            if(top_level && i == ast->BlockCode().size() - 1) {
//    //                last_item = MakeSequenceOpsCpp(ci, elem, false);
//    //                result += last_item;
//    //                last_term = elem;
//    //            } else {
//    //                result += MakeSequenceOpsCpp(ci, elem, false);
//    //            }
//    //        }
//    //    } else if(!(ast->IsFunction() || (ast->getTermID() == TermID::CREATE_ONCE && isConst(ast->Left()->m_text.c_str())))) {
//    //
//    //        //                out << "#line " << ast->GetLine() << "\n";
//    //        result += ci.GetIndent() + MakeCommentLine(ast->toString());
//    //        last_item = GetImpl(ci, ast, temp);
//    //        //        result += ci.GetIndent() + temp;
//    //        //        result += ";\n";
//    //        //        result += ci.GetIndent();
//    //        //        last_item = GetImpl(ci, ast, result);
//    //        //        result += "; " + MakeCommentLine(ast->toString());
//    //    }
//    //
//    //    if(top_level) {
//    //        if(!last_item.empty()) {
//    //            //            NL_TYPECHECK(last_term, ci.last_type, ast->m_type);
//    //            //            if(!canCast(ci.last_type, last_term->m_type)) {
//    //            //                Parser::RaiseException("Incompatible data type '%s' to '%s'", *ast->m_source, last_term->m_line, last_term->m_col);
//    //            //            }
//    //
//    //            //                        NL_CHECK(canCast(ci.last_type, ast->m_type), "Incompatible data type '%s' to '%s'",
//    //            //                    ci.last_type.empty() ? newlang::toString(ObjType::None) : ci.last_type.c_str(),
//    //            //                    ast->m_type.c_str());
//    //            result += ci.GetIndent() + "return " + last_item + ";\n";
//    //        } else {
//    //            //            NL_TYPECHECK(last_term, "", ast->m_type);
//    //            //            NL_CHECK(canCast("", ast->m_type), "Incompatible data type '%s' to '%s'", newlang::toString(ObjType::None), ast->m_type.c_str());
//    //            result += ci.GetIndent() + "return Obj::CreateNone(); // default return\n";
//    //        }
//    //    } else {
//    //        result += ci.GetIndent() + temp;
//    //        result += ";\n";
//    //    }
//    return result;
//}
//
//std::string Compiler::MakeCppFileVariable(CompileInfo &ci, TermPtr &var, std::ostream &out) {
//    ASSERT(var->Left());
//    ASSERT(var->Right());
//    std::string local_name = MakeName(var->Left()->GetFullName());
//    std::string impl_name = MakeLocalName(local_name);
//
//    bool is_new = false;
//    out << ci.GetIndent(-1);
//    if (ci.isArgument(var->Left())) {
//        // Аргумент
//        impl_name = local_name;
//    } else if (!ci.isVariable(var->Left())) {
//        if (var->Left()->Right()) {
//            NL_PARSER(var->Left()->Right(), "First you need to create a variable! %s", var->toString().c_str());
//        }
//        // Создать новую локальную переменную
//        is_new = true;
//        ci.variables.insert(std::pair<std::string, TermPtr>(local_name, var->Left()));
//        out << "ObjPtr " << impl_name;
//    } else {
//        std::string impl;
//        GetImpl(ci, var->Left(), impl);
//        out << impl;
//    }
//
//    std::string temp2;
//    GetImpl(ci, var->Right(), temp2);
//    out << "=" << temp2 << "; ";
//
//    TermPtr term = ci.findObject(var->Right()->GetFullName());
//    if (term) {
//        NL_TYPECHECK(var->Left(), term->m_type_name, var->Left()->m_type_name);
//    }
//
//    if (is_new) {
//        if (var->getTermID() == TermID::CREATE_ONCE) {
//            out << "ctx->CreateGlobalTerm(" << impl_name << ", \"" << local_name << "\"); ";
//        } else {
//            if (var->Left()->getTermID() != TermID::ARGUMENT) {
//                out << "ctx->CreateSessionTerm(" << impl_name << ", \"" << local_name << "\"); ";
//            }
//        }
//    }
//    out << "\n";
//    return impl_name;
//}
//
//std::string Compiler::MakeCppFileCallArgs(CompileInfo &ci, TermPtr &args, TermPtr proto) {
//    std::string result;
//    for (int i = 0; i < args->size(); i++) {
//        if (i) {
//            result += ", ";
//        }
//        std::string impl;
//        GetImpl(ci, args->at(i).second, impl);
//
//        if (proto) {
//            if (i < proto->size()) {
//                TermPtr term = (*proto)[i].second;
//                if (args->at(i).second) {
//                    NL_TYPECHECK(args->at(i).second, args->at(i).second->m_type_name, term->m_type_name);
//                }
//            } else if (!proto->is_variable_args()) {
//                NL_PARSER(args->at(i).second, "Extra argument error");
//            }
//        }
//
//        if (args->at(i).first.empty()) {
//            result += "Obj::Arg(" + impl + ")";
//        } else {
//            result += "Obj::Arg(" + impl + ", \"" + args->at(i).first + "\")";
//        }
//        //        }
//    }
//    return result;
//}
//
//std::string Compiler::EncodeNonAsciiCharacters(const char * in) {
//    std::string text(in);
//    text = std::regex_replace(text, std::regex("\""), "@\"");
//    text = std::regex_replace(text, std::regex("\n"), "@x0A\" \"");
//    text = std::regex_replace(text, std::regex("@\\n"), "@\\n");
//
//    std::string src;
//    for (size_t i = 0; i < text.length(); i++) {
//        if (static_cast<unsigned char> (text[i]) > 127) {
//            src += "@x";
//            src += BinToHex((const uint8_t *) &text[i], 1);
//        } else {
//
//            src += text[i];
//        }
//    }
//    return src;
//}
//
//bool Compiler::MakeCppFile(TermPtr ast, std::ostream &out, const char * source, Context *ctx) {
//
//    CompileInfo ci(RunTime::Init());
//
//    //    RunTime::Instance()->ReadBuiltIn(ci);
//
//
//    WriteIncludeFiles_(out);
//
//    if (source) {
//        std::string src(source);
//        out << "\n//Module source\n";
//        out << "extern \"C\" const char * " NEWLANG_PREFIX "_module_source;\n";
//        out << "const char * " NEWLANG_PREFIX "_module_source=\"" << EncodeNonAsciiCharacters(src.c_str()) << "\";\n\n";
//    }
//
//    out << "//Module function declarations\n";
//    out << "extern \"C\" " NEWLANG_NS "::ObjPtr " NEWLANG_PREFIX "_main_module_func(" NEWLANG_NS "::Context *ctx, " NEWLANG_NS "::Object &in);\n";
//
//    std::vector<std::string> func_list;
//    WriteDeclarationFunctions_(ci, ast, out, func_list);
//
//    out << "\n//List of module funtions\n";
//    out << "extern \"C\" const size_t " NEWLANG_PREFIX "_func_list_count=" << func_list.size() << ";\n";
//    out << "extern \"C\" const char * " NEWLANG_PREFIX "_func_list[" << func_list.size() << "];\n";
//    out << "const char * " NEWLANG_PREFIX "_func_list[" << func_list.size() << "] = {\n";
//    for (auto elem : func_list) {
//        out << NEWLANG_INDENT_OP "\"" << elem.c_str() << "\",\n";
//    }
//    out << "};\n";
//
//
//    out << "\n//Constants form module\n";
//    MakeCppFileConstants(ci, ast, out);
//
//    out << "\n//Main function module\n";
//    out << "extern \"C\" " NEWLANG_NS "::ObjPtr " NEWLANG_PREFIX "_main_module_func(" NEWLANG_NS "::Context *ctx, " NEWLANG_NS "::Object &in) {\n";
//
//    out << MakeFunctionBodyCpp(ci, ast);
//    out << "}\n";
//
//    out << "\n//Module function code\n";
//    MakeCppFileFunctions(ci, ast, out);
//
//    return true;
//}
//
//bool Compiler::Execute(const char *exec, std::string *out, int *exit_code) {
//    //    int status;
//    //    pid_t pid;
//    //
//    //    int mypipe[2];
//    //    pipe(mypipe);
//    //
//    //    pid = fork();
//    //    if(pid < 0) {
//    //        LOG_ERROR("Fork fail");
//    //        return false;
//    //    } else if(!pid) {
//    //        close(mypipe[0]); /* close unused in */
//    //        dup2(mypipe[1], 1); /* stdout to pipe out */
//    //
//    //        execl("/bin/bash", "bash", "-c", exec, NULL);
//    //
//    //        close(mypipe[1]);
//    //        _exit(EXIT_SUCCESS);
//    //    }
//    //
//    //    pid = wait(&status);
//    //
//    //    /* parent process */
//    //    close(mypipe[1]); /* close unused out */
//    //
//    //    char buf[1024] = "";
//    //    ssize_t nbytes;
//    //    /* read from pipe in */
//    //    while((nbytes = read(mypipe[0], buf, sizeof (buf))) > 0) {
//    //        if(out) {
//    //            out->append(buf, nbytes);
//    //        }
//    //    }
//    //    close(mypipe[0]);
//    //
//    //    if(WIFEXITED(status)) {
//    //        if(exit_code) {
//    //
//    //            *exit_code = WEXITSTATUS(status);
//    //        }
//    //        return true;
//    //    }
//    return false;
//}

//llvm::ExecutionEngine * NewLang::JITCompileCPP(const char* source, const char *file_name) {
//
//    std::vector<std::string> opts;
//    opts.push_back("-x");
//    opts.push_back("c++");
//    opts.push_back("-std=c++14");
//
//    opts.push_back("-I/usr/lib/llvm-12/lib/clang/12.0.1/include");
//    opts.push_back("-I/usr/include/x86_64-linux-gnu/c++/10");
//    opts.push_back("-I/usr/include/c++/10");
//    opts.push_back("-I/usr/local/include");
//    opts.push_back("-I/usr/include");
//    opts.push_back("-I..");
//
//    //-I/usr/lib/llvm-12/include -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
//    opts.push_back("-I/usr/lib/llvm-12/include");
//    opts.push_back("-D_GNU_SOURCE");
//    opts.push_back("-D__STDC_CONSTANT_MACROS");
//    opts.push_back("-D__STDC_FORMAT_MACROS");
//    opts.push_back("-D__STDC_LIMIT_MACROS");
//    opts.push_back("-D__GCC_ATOMIC_TEST_AND_SET_TRUEVAL=1");
//
//
//    //`llvm-config-12 --cppflags` -Wno-trigraphs -Wno-invalid-source-encoding -stdlib=libstdc++ -fsanitize=undefined-trap -fsanitize-undefined-trap-on-error -ftrap-function=abort
//    opts.push_back("-fexceptions");
//    opts.push_back("-fcxx-exceptions");
//    opts.push_back("-Wno-trigraphs");
//    opts.push_back("-Wno-invalid-source-encoding");
//    opts.push_back("-stdlib=libstdc++");
//
//    opts.push_back("-fno-rtti");
//    opts.push_back("-ffreestanding");
//    opts.push_back("-fno-use-cxa-atexit");
//
//    opts.push_back("-Wall");
//    opts.push_back("-Werror");
//
//    opts.push_back("-Wno-error=unused-variable");
//    opts.push_back("-Wno-error=switch");
//
//    //    opts.push_back("-g");
//
//    if(file_name) {
//        std::ofstream file(file_name, std::ios::trunc);
//        file << source;
//        file << "\n\n#include <stdio.h>\n\
//#include <string.h>\n\
//int nv_add(int a, int b) {\n\
//    printf(\"call nv_add(%d, %d)\\n\", a, b);\n\
//    return a + b;\n\
//}\n\
//\n\
//int nv_sub(int a, int b) {\n\
//    printf(\"call nv_sub(%d, %d)\\n\", a, b);\n\
//    return a - b;\n\
//}\n\
//";
//        file.close();
//
//        opts.push_back(file_name);
//
//    } else {
//
//        // Компиляция из памяти
//        int codeInPipe[2];
//        pipe2(codeInPipe, O_NONBLOCK);
//        ::write(codeInPipe[1], (void *) source, strlen(source));
//        ::close(codeInPipe[1]); // We need to close the pipe to send an EOF
//        dup2(codeInPipe[0], STDIN_FILENO);
//
//        opts.push_back("-"); // Read code from stdin
//    }
//
//    return m_jit->CompileModule(source, opts);
//}

//bool Compiler::CompileModule(const char* filename, const char* output) {
//
//    std::string in_data = ReadFile(filename);
//    if(in_data.empty()) {
//        LOG_DEBUG("Not found or empty file '%s'", filename);
//        return false;
//    }
//
//    TermPtr term;
//    Parser p(term);
//    p.Parse(in_data.c_str());
//    if(!term) {
//        LOG_DEBUG("Fail parsing file '%s'", filename);
//        return false;
//    }
//
//    std::string temp_name(filename);
//    temp_name.append(".temp");
//
//    std::ostringstream sstr;
//    Context ctx(RunTime::Init());
//    MakeCppFile(term, sstr, in_data.c_str(), &ctx);
//
//    std::string file_name(temp_name);
//    file_name.append(".cpp");
//
//    std::ofstream file(file_name);
//    file << sstr.str();
//    file.close();
//
//    return Compiler::GccMakeModule(file_name.c_str(), output);
//}


//std::string NewLang::MakeFunctionsHeaderSourceForJIT(TermPtr asg);
//    if(asg->IsFunction()) {
//    }
//}
//std::string NewLang::MakeFunctionsSourceForJIT(TermPtr asg);
//    if(asg->IsFunction()) {
//        
//    } else if(asg->getTermID() == TermID::BLOCK) {
//        ObjPtr temp;
//        for (TermPtr &elem : calcasgBlockCode()) {
//
//        }
//    }
//}

//std::string NewLang::MakeFunctionsSourceForJIT(TermPtr ast, Context *ctx) { //, const char *source) {
//
//    CompileInfo ci;
//    std::string result;
//    std::ostringstream out;
//
//    if(ctx && ctx->m_info.global) {
//        for (auto &elem : ctx->m_info.global->m_funcs) {
//            ci.builtin.insert(elem.first);
//        }
//    }
//
//
//    WriteIncludeFiles_(out);
//    result += out.str();
//
//    //    if(source) {
//    //        result += "\n//Module source\n";
//    //        result += "extern \"C\" const char * " NEWLANG_PREFIX "_module_source;\n";
//    //        result += "const char * " NEWLANG_PREFIX "_module_source=\"" << EncodeNonAsciiCharacters(source) << "\";\n\n";
//    //    }
//
//    result += "//Module function declarations\n";
//    result += NEWLANG_NS "::ObjPtr " NEWLANG_PREFIX "_main_module_func(" NEWLANG_NS "::Context *ctx, " NEWLANG_NS "::Object &in);\n";
//
//    std::vector<std::string> func_list;
//    out.str("");
//    WriteDeclarationFunctions_(ast, out, func_list);
//    result += out.str();
//
//    result += "\n//List of module funtions\n";
//    result += "size_t " NEWLANG_PREFIX "_func_list_count=" + std::to_string(func_list.size()) + ";\n";
//    //    result += "extern \"C\" const char * " NEWLANG_PREFIX "_func_list[" + std::to_string(func_list.size()) + "];\n";
//    result += "const char * " NEWLANG_PREFIX "_func_list[" + std::to_string(func_list.size()) + "] = {\n";
//    for (auto elem : func_list) {
//        result += NEWLANG_INDENT_OP "\"" + elem + "\",\n";
//    }
//    result += "};\n";
//
//
//    result += "\n//Constants form module\n";
//    out.str("");
//    MakeCppFileConstants(ci, ast, out);
//    result += out.str();
//
//    //    result += "\n//Main function module\n";
//    //    result += NEWLANG_NS "::ObjPtr"bool " NEWLANG_PREFIX "_main_module_func(" NEWLANG_NS "::Context *ctx, " NEWLANG_NS "::Object &in, " NEWLANG_NS "::ObjPtr &out) {\n";
//    //    std::string main_module;
//    //    if(ast->getTermID() == TermID::BLOCK) {
//    //        for (TermPtr &elem : ast->BlockCode()) {
//    //            main_module += MakeCppFileMainModule(ci, elem);
//    //        }
//    //    } else {
//    //        main_module = MakeCppFileMainModule(ci, ast);
//    //    }
//    //    std::cout << main_module << "\n";
//    //    TermPtr term_empty;
//    //    ReplaceFuncObject(term_empty, main_module);
//    //    result += main_module;
//    //    result += "\n" NEWLANG_INDENT_OP "return true;\n}\n";
//
//    result += "\n//Module function code\n";
//
//    out.str("");
//    MakeCppFileFunctions(ci, ast, out);
//
//    result += out.str();
//    return result;
//}

//bool Compiler::GccMakeModule(const char * in_file, const char * module, const char * opts, std::string *out, int *exit_code) {
//
//    //char temp[MAXPATHLEN];
//    //if(!getcwd(temp, sizeof (temp))) {
//    //    return false;
//    //}
//
//    //std::string dir(temp);
//
//    //std::string out_file(dir);
//
//    //out_file.append("/");
//    //out_file.append(module);
//    ////    out_file.append(".so");
//
//    //if(!opts) {
//    //    struct stat file_src;
//    //    struct stat file_so;
//    //    if(lstat(in_file, &file_src) == 0 && lstat(out_file.c_str(), &file_so) == 0) {
//    //        if(file_src.st_mtime < file_so.st_mtime && file_so.st_size) {
//    //            LOG_DEBUG("Skip build %s", out_file.c_str());
//    //            return true;
//    //        }
//    //    }
//    //}
//
//    //std::remove(out_file.c_str());
//
//
//    ////g++ -std=c++17  -c -g -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG -DUNITTEST -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -MMD -MP -MF "test_gen.cpp.o.d" -o test_out.o test_gen.cpp
//
//    //std::string exec("cd ");
//    //exec.append(dir);
//    //exec.append(" && ");
//
//    //exec.append("g++ -std=c++14 -Werror=return-type -c -g -fPIC -DDEBUG -DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG ");
//    //if(opts) {
//    //    exec.append(opts);
//    //}
//    //exec.append(" -I.. -I../contrib/googletest/googletest -I../contrib/googletest/googletest/include -I../contrib/torch/include/torch/csrc/api/include -I../contrib/torch/include -MMD -MP -MF \"");
//    //exec.append(in_file);
//    //exec.append(".o.d\" -o ");
//    //exec.append(in_file);
//    //exec.append(".o ");
//    //exec.append(in_file);
//
//    //exec.append(" && ");
//
//    ////g++ -o test_gen.so test_gen.o -shared -fPIC
//    //exec.append("g++ -o ");
//    //exec.append(out_file);
//    //exec.append(" ");
//    //exec.append(in_file);
//    //exec.append(".o -shared -fPIC");
//
//    //LOG_DEBUG("%s", exec.c_str());
//
//    //return Execute(exec.c_str(), out, exit_code);
//    return false;
//}
//
//Compiler::Compiler(RuntimePtr rt) : m_runtime(rt) {
//    if (!m_runtime) {
//        m_runtime = RunTime::Init();
//    }
//
//    //    m_jit = cppjit::CppJIT::Instance();
//}

//ObjPtr NewLang::Eval(Context *ctx, const char *text, bool make_function) {
//    TermPtr calc;
//    Parser parser(calc);
//    try {
//        parser.Parse(text);
//    } catch (parser_exception &ex) {
//        return Obj::CreateError(ex.what());
//    }
//
//    char filename[] = "lazy.temp.XXXXXX";
//    int file = mkstemp(filename);
//    if(filename && file != -1) {
//        write(file, text, strlen(text));
//        close(file);
//    } else {
//        LOG_RUNTIME("Fail create temp file '%s'!", filename);
//    }
//
//    return ExecModule(filename, filename, true, ctx);
//}

//ObjPtr NewLang::GetIndexField(Context *ctx, ObjPtr obj, TermPtr term, bool create_field) {
//    if(!term->Right()) {
//        return obj;
//    } else if(term->Right()->getTermID() == TermID::FIELD && !term->Right()->Right() && create_field) {
//        // Если последнее в списке поле создается
//        return obj->push_back(Obj::CreateNone(), term->Right()->getText().c_str()).second;
//    }
//    ObjPtr result;
//    if(term->Right()->getTermID() == TermID::FIELD) {
//        result = (*obj)[term->Right()->m_text.c_str()].second;
//        return GetIndexField(ctx, result, term->Right());
//    } else if(term->Right()->getTermID() == TermID::INDEX) {
//        result = Obj::GetIndex(obj, term->Right());
//        return GetIndexField(ctx, result, term->Right());
//    }
//    LOG_RUNTIME("Fail type %s of object '%s'!", newlang::toString(term->Right()->getTermID()), term->Right()->toString().c_str());
//}

//ObjPtr NewLang::Eval(Context *ctx, TermPtr calc, bool make_function) {
//    try {
//        if(!calc) {
//            LOG_RUNTIME("Empty term evaluate!");
//        }
//
//        //    if(calc->getTermID() == TermID::OPERATOR) {
//        //        // Все операторы в отдельной функции
//        //        return EvalOp(ctx, calc);
//        //    }
//        //    if(calc->getTermID() == TermID::ITERATOR) {
//        //        // Все операторы в отдельной функции
//        //        return EvalIter(ctx, calc);
//        //    }
//
//        TermID term_id = calc->getTermID();
//        if(calc->IsLiteral() || term_id == TermID::DICT || term_id == TermID::TUPLE) { // Константа
//            ASSERT(calc->getTermID() != TermID::STRBYTE); // Нужно раскрыть переменные в строке
//            return Obj::CreateFrom(ctx, calc);
//        } else if(term_id == TermID::CALL || term_id == TermID::TERM) { // Термин или вызов функции
//
//            Object args(ctx, calc, true);
//
//            ObjPtr obj = Context::CallByName(ctx, calc->getText().c_str(), args, calc->isRef());
//            return GetIndexField(ctx, obj, calc);
//
//            //    } else if(term_id == TermID::ITERATOR) { // Итератор
//            //        ASSERT(calc->Left());
//            //        if(calc->Left()) {
//            //            ArgPtr args = Arg::CreateObject(ctx, calc);
//            //            if(calc->m_text.compare("?") == 0) {
//            //                VERIFY(args->PrepareObject(std_args_iterator_multiple.get()));
//            //            } else if(calc->m_text.compare("?!") == 0) {
//            //                VERIFY(args->PrepareObject(std_args_iterator_single.get()));
//            //            }
//            //            ObjPtr temp;
//            //            std_iterator(Obj::CreateFrom(ctx, calc).get(), args, temp);
//            //            return temp;
//            //        }
//        } else if(term_id == TermID::BLOCK) { // Последовательность операторов
//            ObjPtr temp;
//            for (TermPtr &elem : calc->BlockCode()) {
//                temp = Eval(ctx, elem);
//            }
//            return temp;
//
//        } else if(term_id == TermID::ITERATOR) {
//            if(calc->m_text.compare("?") == 0) {
//
//            }
//            ObjPtr temp;
//            for (TermPtr &elem : calc->BlockCode()) {
//                temp = Eval(ctx, elem);
//            }
//            return temp;
//
//        } else if(term_id == TermID::ASSIGN || term_id == TermID::APPEND || term_id == TermID::INDEX) {
//
//
//            ObjPtr right;
//            if(calc->Right()) {
//                right = Eval(ctx, calc->Right(), make_function);
//            } else {
//                right = Obj::CreateNone();
//            }
//
//            if(term_id == TermID::APPEND) {
//                std::string name(calc->Left()->m_text);
//                // Если создаем сессионные или глобальные объекты
//                if(name.compare("@") == 0 || name.compare("$") == 0) {
//                    if(calc->Left()->m_name.empty()) {
//                        ParserException("Append session or global term without name!", *calc->Left()->m_source, calc->Left()->m_line, calc->Left()->m_col);
//                    }
//                    if(name.compare("$") == 0) {
//                        return ctx->CreateSessionTerm(right, calc->Left()->m_name.c_str());
//                    } else if(name.compare("@") == 0) {
//                        return ctx->CreateGlobalTerm(right, calc->Left()->m_name.c_str());
//                    }
//                    LOG_RUNTIME("Fail logic");
//                }
//            }
//            // Левый операнд всегда существует
//            ObjPtr lval;
//            if(term_id == TermID::APPEND) {
//                Object args;
//                lval = Context::CallByName(ctx, calc->Left()->m_text.c_str(), args, true);
//                lval->push_back(right);
//            } else {
//                lval = Eval(ctx, calc->Left(), make_function);
//                ASSERT(lval);
//                lval->op_assign(right);
//            }
//
//            return lval;
//
//        } else if(term_id == TermID::CREATE_ONCE || term_id == TermID::CREATE_ONCE || term_id == TermID::CREATE_ONCE || term_id == TermID::CREATE_ONCE) { // Создание нового термина
//
//            // Левый операнд всегда создается
//            ObjPtr base;
//            ObjPtr lval;
//            if(calc->Left()->Right()) {
//                base = Context::CallByName(ctx, calc->Left()->getText().c_str(), Object(), true);
//                lval = GetIndexField(ctx, base, calc->Left(), true);
//            } else {
//                lval = Obj::CreateNone();
//            }
//
//
//            ObjPtr rval = Eval(ctx, calc->Right(), make_function);
//
//            lval->op_assign(rval);
//
//            if(term_id == TermID::CREATE_ONCE) {
//                lval->MakeConst();
//            }
//            if(term_id == TermID::CREATE_ONCE || term_id == TermID::CREATE_ONCE) {
//                if(lval->is_dictionary() || term_id == TermID::CREATE_ONCE) {
//                    lval->m_var_name = calc->Left()->m_text;
//                }
//                return ctx->CreateSessionTerm(lval, calc->Left()->m_text.c_str());
//            } else if(term_id == TermID::CREATE_ONCE) {
//                return ctx->CreateGlobalTerm(lval, calc->Left()->m_text.c_str());
//            }
//            return lval;
//
//        } else if(term_id == TermID::SIMPLE || term_id == TermID::FUNCTION || term_id == TermID::PURE_ONCE) {
//            if(!make_function) { // Игнорировать определения функций при выполнении
//                // Ничего не делаем
//                ObjType type;
//                switch(term_id) {
//                    case TermID::SIMPLE:
//                    case TermID::PURE_ONCE:
//                        type = ObjType::PUREFUNC;
//                        break;
//                    case TermID::FUNCTION:
//                        type = ObjType::FUNCTION;
//
//                        break;
//                    default:
//                        LOG_RUNTIME("Function type '%s' unknown '%s'", EnumStr(calc->getTermID()).c_str(), calc->toString().c_str());
//                }
//                return Obj::CreateFunc(ctx, calc->Left(), type, calc->Left()->getText().c_str());
//            }
//
//            //        std::string temp_name("runtime.temp");
//            //        std::ostringstream sstr;
//            //
//            //        MakeCppFile(calc, sstr);
//            //
//            //        std::string file_name(temp_name);
//            //        file_name.append(L".cpp");
//            //
//            //        std::ofstream file(file_name);
//            //        file << sstr.str();
//            //        file.close();
//            //
//            //        if(GccMakeModule(file_name.c_str(), temp_name.c_str())) {
//            //            if(ctx.m_global && ctx.m_global->LoadModule(temp_name.c_str()) && ctx.m_global->HasFunc(calc->Left()->GetFullName().c_str())) {
//            //                out = Obj::CreateValue(true);
//            //                return true;
//            //            }
//            //        }
//            //        out = Obj::CreateValue(false);
//            //        return false;
//        }
//
//        LOG_RUNTIME("Calculate type %s '%s' not implemented!", EnumStr(calc->getTermID()).c_str(), calc->m_text.c_str());
//    } catch (std::exception &ex) {
//        return Obj::CreateError(ex.what());
//    }
//}


//void Compiler::ReplaceSourceVariable(CompileInfo &ci, size_t count, std::string &body) {
//    std::string arg_name;
//    std::string arg_place;
//
//    for (size_t i = 0; i < count; i++) {
//        // Заменить номер аргумента
//        arg_name = "@$" + std::to_string(i);
//        arg_place = "in[" + std::to_string(i) + "]";
//        body = std::regex_replace(body, std::regex(arg_name), arg_place);
//    }
//
//    for (auto &elem : ci.arguments) {
//        // Заменить имя аргумента
//        arg_name = MakeName(elem.first);
//        arg_place = "in[\"" + arg_name + "\"]";
//        arg_name = "@$" + arg_name;
//        body = std::regex_replace(body, std::regex(arg_name), arg_place);
//    }
//}
//
//std::string Compiler::GetImpl(CompileInfo &ci, TermPtr term, std::string &output) {
//    ASSERT(0);
//
//    //    ASSERT(term);
//    //    std::string result;
//    //
//    //    std::ostringstream ostr;
//    //    std::string temp;
//    //
//    //    std::string temp2;
//    //    std::vector<std::string> iters;
//    //    TermPtr proto;
//    //    TermPtr proto2;
//    //
//    //    ci.last_type.clear();
//    //
//    //    switch(term->getTermID()) {
//    //        case TermID::NUMBER:
//    //        case TermID::INTEGER:
//    //            result = "Obj::CreateValue(" + term->getText() + ")";
//    //            output += result;
//    //            return result;
//    //
//    //        case TermID::STRCHAR:
//    //        case TermID::STRWIDE:
//    //            // Экранировать спецсимволы в символьных литералах
//    //            //Obj::CreateValue("string");
//    //            temp = std::regex_replace(term->getText(), std::regex("\n"), "\\n");
//    //            temp = std::regex_replace(temp, std::regex("\""), "@\"");
//    //            result += "Obj::CreateString(";
//    //            if(term->getTermID() == TermID::STRWIDE) {
//    //                result += "L";
//    //            }
//    //            result += "\"" + temp + "\")";
//    //            output += result;
//    //            return result;
//    //
//    //        case TermID::CREATE_ONCE:
//    //            ostr.str("");
//    //            result = MakeCppFileVariable(ci, term, ostr);
//    //            output += ostr.str();
//    //            return result;
//    //
//    //        case TermID::ARGUMENT:
//    //            result = "in[" + std::to_string(IndexArg(term)) + "]";
//    //            output += result;
//    //            return result;
//    //
//    //        case TermID::DICT:
//    //            temp = "";
//    //            for (int i = 0; i < term->size(); i++) {
//    //                if(!temp.empty()) {
//    //                    temp += ", ";
//    //                }
//    //                temp2.clear();
//    //                GetImpl(ci, (*term)[i].second, temp2);
//    //                temp += "Obj::Arg(" + temp2;
//    //                if(!term->name(i).empty()) {
//    //                    temp += ", \"" + term->name(i) + "\"";
//    //                }
//    //                temp += ")";
//    //            }
//    //            result = "Obj::CreateDict(" + temp + ")";
//    //            output += result;
//    //            return result;
//    //
//    //        case TermID::NAME:
//    //            ASSERT(!term->getName().empty() || !term->getText().empty());
//    //
//    //            if(term->isCall()) {
//    //                if((proto = ci.isArgument(term))) {
//    //                    result = "in[\"" + MakeName(term->GetFullName()) + "\"]->Call(";
//    //                    result += MakeCppFileCallArgs(ci, term, proto);
//    //                    result += ")";
//    //                } else if(isLocal(term->getText().c_str()) || isModule(term->getText().c_str())) {
//    //                    result = "Context::CallByName(ctx, \"";
//    //                    result += term->GetFullName() + "\"";
//    //                    temp = MakeCppFileCallArgs(ci, term, nullptr);
//    //                    if(!temp.empty()) {
//    //                        result += ", " + temp;
//    //                    }
//    //                    result += ")";
//    //                } else if((proto = ci.isFunction(term))) {
//    //                    //Непосредственный вызов - разрешение имени во время компиляции
//    //                    result = MakeLocalName(term->getText().c_str()) + "(ctx, ";
//    //                    result += "Obj::CreateDict(Obj::Arg()";
//    //                    temp = MakeCppFileCallArgs(ci, term, proto);
//    //                    if(!temp.empty()) {
//    //                        result += ", " + temp;
//    //                    }
//    //                    result += "))";
//    //                } else {
//    //                    //Клонирование переменной - разрешение имени во время компиляции
//    //                    result = MakeLocalName(term->getText().c_str()) + "->Call(";
//    //                    result += MakeCppFileCallArgs(ci, term, nullptr);
//    //                    result += ")";
//    //                }
//    //
//    //                output += result;
//    //                return result;
//    //            }
//    //
//    //
//    //            if(term->GetFullName().compare("_") == 0) {
//    //                //Пустой объект
//    //                ci.last_type = "_";
//    //                result = "Obj::CreateNone()";
//    //            } else if(ci.isLocalAccess(term)) {
//    //                //Доступ по имени - разрешение имени во время компиляции
//    //                result = MakeLocalName(term->GetFullName().c_str());
//    //            } else if(ci.isArgument(term)) {
//    //
//    //                result = "in[\"" + MakeName(term->GetFullName()) + "\"]";
//    //
//    //            } else if(!term->GetFullName().empty()) {
//    //                //Вызов по имении переменной (создание копии объекта) - разрешение имени во время компиляции
//    //                result = "Context::CallByName(ctx,\"" + term->GetFullName() + "\")";
//    //            } else {
//    //                //Вызов по имении переменной (создание копии объекта) - разрешение имени во время компиляции
//    //                result = "Obj::CreateNone()";
//    //            }
//    //
//    //            if(term->Right()) {
//    //                GetImpl(ci, term->Right(), result);
//    //            }
//    //
//    //            output += result;
//    //            return result;
//    //
//    //            //        case TermID::FIELD:
//    //            //            ASSERT(!term->getName().empty() || !term->getText().empty());
//    //            //            ASSERT(!term->Right());
//    //            //
//    //            //            if(!ci.m_builtin_direct->CheckDirect(ci, term, output)) {
//    //            //                output.insert(0, "(*");
//    //            //                output += ")[\"" + term->getText() + "\"]";
//    //            //            }
//    //            //            result = output;
//    //            //            return result;
//    //
//    //        case TermID::EMBED:
//    //            temp = term->getText();
//    //            ReplaceSourceVariable(ci, ci.arguments.size() + 1, temp);
//    //            output += temp;
//    //            return "";
//    //
//    //        case TermID::INT_PLUS:
//    //        case TermID::INT_MINUS:
//    //
//    //            if(term->m_right) {
//    //                GetImpl(ci, term->m_right, temp);
//    //                output += "return " + temp;
//    //
//    //            } else {
//    //                output += "return Obj::CreateNone()";
//    //            }
//    //            return "";
//    //
//    //        case TermID::ARGS:
//    //            result = "in";
//    //            output += result;
//    //            return result;
//    //
//    //        case TermID::ASSIGN:
//    //            GetImpl(ci, term->Left(), result);
//    //            output += "(*" + result + ")=";
//    //            GetImpl(ci, term->Right(), temp);
//    //            output += temp;
//    //            return result;
//    //
//    //        case TermID::APPEND:
//    //            GetImpl(ci, term->Left(), result);
//    //            result = result + "->push_back(";
//    //            GetImpl(ci, term->Right(), temp);
//    //            result += temp;
//    //            if(!term->m_name.empty()) {
//    //                result += ", \"" + term->m_name + "\"";
//    //            }
//    //            result += ")";
//    //            output += result;
//    //            return result;
//    //
//    //        case TermID::OPERATOR:
//    //            proto = ci.findObject(term->Left()->GetFullName());
//    //            if(!proto) {
//    //                proto = term->Left();
//    //            }
//    //            proto2 = ci.findObject(term->Right()->GetFullName());
//    //            if(!proto2) {
//    //                proto2 = term->Right();
//    //            }
//    //            NL_TYPECHECK(term, proto2->m_type_name, proto->m_type_name);
//    //
//    //            GetImpl(ci, term->Left(), temp2);
//    //            result = "(*" + temp2 + ")" + term->getText();
//    //            GetImpl(ci, term->Right(), temp);
//    //            result += temp + ", " + temp2;
//    //            output += result;
//    //            return result;
//    //
//    //        case TermID::BLOCK:
//    //            for (size_t i = 0; i < term->m_block.size(); i++) {
//    //                temp.clear();
//    //                GetImpl(ci, term->m_block[i], temp);
//    //                output += ci.GetIndent() + temp + ";\n";
//    //            }
//    //            return "";
//    //
//    //        case TermID::WHILE:
//    //            result += ci.GetIndent(-1) + "while(";
//    //
//    //            temp.clear();
//    //            GetImpl(ci, term->Left(), temp);
//    //            result += temp + "->GetValueAsBoolean()) {\n";
//    //
//    //            temp.clear();
//    //            GetImpl(ci, term->Right(), temp);
//    //            result += temp;
//    //
//    //            result += ci.GetIndent(-1) + "}";
//    //            output += result;
//    //            return "";
//    //
//    //        case TermID::ITERATOR:
//    //            ASSERT(term->Left());
//    //            result = BeginIterators(ci, term->Left(), output, iters);
//    //
//    //            for (int i = 0; i < iters.size(); i++) {
//    //                output += ci.GetIndent(i) + "while(!" + iters[i] + ".complete()) {\n";
//    //            }
//    //
//    //
//    //            if(ci.isArgument(term->Left())) {
//    //                temp = "in[\"" + MakeName(term->Left()->GetFullName()) + "\"]->Call(";
//    //                temp += MakeIteratorCallArgs_(ci, term->Left(), iters);
//    //                temp += ")";
//    //            } else if(isLocal(term->Left()->getText().c_str()) || isModule(term->Left()->getText().c_str())) {
//    //                temp = "Context::CallByName(ctx, \"";
//    //                temp += term->GetFullName() + "\"";
//    //                temp2 = MakeIteratorCallArgs_(ci, term->Left(), iters);
//    //                if(!temp2.empty()) {
//    //                    temp += ", " + temp2;
//    //                }
//    //                temp += ")";
//    //            } else if(ci.isFunction(term->Left())) {
//    //                //Непосредственный вызов - разрешение имени во время компиляции
//    //                temp = MakeLocalName(term->Left()->getText().c_str()) + "(ctx, ";
//    //                temp += "Obj::CreateDict(Obj::Arg()";
//    //                temp2 = MakeIteratorCallArgs_(ci, term->Left(), iters);
//    //                if(!temp2.empty()) {
//    //                    temp += ", " + temp2;
//    //                }
//    //                temp += "))";
//    //            } else {
//    //                //Клонирование переменной - разрешение имени во время компиляции
//    //                temp = MakeLocalName(term->Left()->getText().c_str()) + "->Call(";
//    //                temp += MakeIteratorCallArgs_(ci, term->Left(), iters);
//    //                temp += ")";
//    //            }
//    //
//    //            output += ci.GetIndent(iters.size()) + "ObjPtr " + result + "_eval";
//    //            output += "=" + temp + ";\n";
//    //
//    //            output += ci.GetIndent(iters.size()) + "if(" + result + "_eval->GetValueAsBoolean()){\n";
//    //
//    //            for (size_t i = 0; i < iters.size(); i++) {
//    //                // (*humans)->RefInc();
//    //                output += ci.GetIndent(iters.size()) + "(*" + iters[i] + ")->RefInc();\n";
//    //            }
//    //            // output += ci.GetIndent(iters.size()) + result + "->push_back(Obj::CreateArray(*" + iters[0] + ", *" + iters[1] + "));\n";
//    //            output += ci.GetIndent(iters.size() + 1) + result + "->push_back(Obj::CreateArray(";
//    //            for (size_t i = 0; i < iters.size(); i++) {
//    //                if(i) {
//    //                    output += ", ";
//    //                }
//    //                output += "*" + iters[i];
//    //            }
//    //            output += "));\n";
//    //
//    //            output += ci.GetIndent(iters.size()) + "}\n";
//    //
//    //
//    //            for (size_t i = iters.size() - 1; i > 0; i--) {
//    //                output += ci.GetIndent(i + 1) + iters[i] + "++;\n";
//    //                output += ci.GetIndent(i) + "}\n";
//    //                if(i) {
//    //                    output += ci.GetIndent(i) + iters[i] + ".reset();\n";
//    //                    output += ci.GetIndent(i) + iters[i - 1] + "++;\n";
//    //                }
//    //            }
//    //            output += ci.GetIndent() + "}";
//    //
//    //            return result;
//    //
//    //    }
//    //    if(term->getTermID() == TermID::FOLLOW) {
//    //        bool else_if = false;
//    //        for (size_t i = 0; i < term->m_follow.size(); i++) {
//    //
//    //            if(else_if && term->m_follow[i]->Left()) {
//    //                result += " else ";
//    //            } else {
//    //                else_if = true;
//    //            }
//    //
//    //            if(term->m_follow[i]->Left()) {
//    //                temp.clear();
//    //                GetImpl(ci, term->m_follow[i]->Left(), temp);
//    //                result += ci.GetIndent() + "if((" + temp + ")->GetValueAsBoolean()) {";
//    //            } else {
//    //                if(i == 0 || i != term->m_follow.size() - 1) {
//    //                    LOG_RUNTIME("Bad logic follow '%s'", term->toString().c_str());
//    //                }
//    //                result += " else {";
//    //            }
//    //            if(term->m_follow[i]->Right()) {
//    //                temp.clear();
//    //                GetImpl(ci, term->m_follow[i]->Right(), temp);
//    //                result += "\n" NEWLANG_INDENT_OP + ci.GetIndent() + temp + ";\n" + ci.GetIndent() + "}";
//    //            } else {
//    //                LOG_RUNTIME("Bad logic follow '%s'", term->toString().c_str());
//    //            }
//    //        }
//    //        result += "\n";
//    //        output += result;
//    //        return "";
//    //    }
//
//    LOG_RUNTIME("Can`t term type '%s' implementation '%s'!", newlang::toString(term->getTermID()), term->toString().c_str());
//}
//
//std::string Compiler::MakeIteratorCallArgs_(CompileInfo &ci, TermPtr args, std::vector<std::string> &iters) {
//    std::string result;
//    size_t iter_pos = 0;
//
//    for (int i = 0; i < args->size(); i++) {
//        std::string impl;
//
//        if (i) {
//            result += ", ";
//        }
//
//        if (args->at(i).second->getTermID() == TermID::ITERATOR) {
//            ASSERT(args->at(i).second->Left());
//            if (args->at(i).second->getText().compare("!") == 0) {
//                ASSERT(iter_pos < iters.size());
//                impl = "*" + iters[iter_pos];
//            } else {
//                LOG_RUNTIME("Iterator '%s' not implemented!", args->at(i).second->getText().c_str());
//            }
//
//            if (args->at(i).first.empty()) {
//                result += "Obj::Arg(" + impl + ")";
//            } else {
//                result += "Obj::Arg(" + impl + ", \"" + args->at(i).first + "\")";
//            }
//
//            iter_pos++;
//
//        } else {
//
//            GetImpl(ci, args->at(i).second, impl);
//            if (args->at(i).first.empty()) {
//                result += "Obj::Arg(" + impl + ")";
//            } else {
//                result += "Obj::Arg(" + impl + ", \"" + args->at(i).first + "\")";
//            }
//        }
//    }
//    return result;
//}
//
//std::string Compiler::BeginIterators(CompileInfo &ci, TermPtr args, std::string &output, std::vector<std::string> &iters) {
//
//    std::string summary = "summary";
//    output += ci.GetIndent() + "ObjPtr " + summary + "=Obj::CreateArray();\n";
//
//    // Аргументы функции с локальным доступом по имени или ииндексу
//    if (args->size()) {
//        for (int i = 0; i < args->size(); i++) {
//            if ((*args)[i].second->getTermID() == TermID::ITERATOR) {
//                std::string name;
//                if ((*args)[i].second->getText().compare("!") == 0) {
//
//                    ASSERT((*args)[i].second->Left());
//                    ASSERT(!(*args)[i].second->size()); // todo пока без обработки аргументов
//
//                    name = "iter_" + std::to_string(i + 1);
//                    iters.push_back(name);
//                    output += ci.GetIndent() + "auto " + name + "= ctx->m_info.global->select(\"" + (*args)[i].second->Left()->getText() + "\");\n";
//                } else {
//                    LOG_RUNTIME("Iterator '%s' not implemented!", (*args)[i].second->getText().c_str());
//                }
//            }
//        }
//    }
//    return summary;
//}
