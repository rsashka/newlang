//#include "pch.h"

//#include <zlib.h>
#include <zip.h>


#include "warning_push.h"

#include "llvm/IR/Metadata.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"

#include "clang/Driver/Driver.h"
#include "clang/Driver/Compilation.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "llvm/IR/Module.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/VirtualFileSystem.h"
#include "llvm/Support/TargetSelect.h"

#include "llvm-c/Core.h"
#include "llvm-c/Target.h"
#include "llvm-c/TargetMachine.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/CodeGen.h"

#include "llvm/Support/InitLLVM.h"

#include "llvm/Target/TargetMachine.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/CodeGen/CodeGenAction.h"

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"


#include "warning_pop.h"



#include "context.h"
#include "parser.h"
#include "analysis.h"

#include "system.h"

#include "term.h"
#include "runtime.h"
#include "builtin.h"

using namespace newlang;


#ifdef _MSC_VER
ObjType RunTime::m_wide_char_type = ObjType::Int16;
STATIC_ASSERT(sizeof (wchar_t) == 2);
#else
ObjType RunTime::m_wide_char_type = ObjType::Int32;
STATIC_ASSERT(sizeof (wchar_t) == 4);
#endif
ffi_type * RunTime::m_wide_char_type_ffi = nullptr;

ffi_type * RunTime::m_ffi_type_void = nullptr;
ffi_type * RunTime::m_ffi_type_uint8 = nullptr;
ffi_type * RunTime::m_ffi_type_sint8 = nullptr;
ffi_type * RunTime::m_ffi_type_uint16 = nullptr;
ffi_type * RunTime::m_ffi_type_sint16 = nullptr;
ffi_type * RunTime::m_ffi_type_uint32 = nullptr;
ffi_type * RunTime::m_ffi_type_sint32 = nullptr;
ffi_type * RunTime::m_ffi_type_uint64 = nullptr;
ffi_type * RunTime::m_ffi_type_sint64 = nullptr;
ffi_type * RunTime::m_ffi_type_float = nullptr;
ffi_type * RunTime::m_ffi_type_double = nullptr;
ffi_type * RunTime::m_ffi_type_pointer = nullptr;

RunTime::ffi_prep_cif_type *RunTime::m_ffi_prep_cif = nullptr;
RunTime::ffi_prep_cif_var_type * RunTime::m_ffi_prep_cif_var = nullptr;
RunTime::ffi_call_type * RunTime::m_ffi_call = nullptr;

//const char * RunTime::default_argv[RunTime::default_argc] = {"", "--nlc-no-runtime", "--nlc-no-dsl", "--nlc-no-embed-source"};
//const TermPtr VarScope::NonameBlock = Term::Create(parser::token_type::END, TermID::NAMESPACE, "_");

/*
 * 
 * 
 * 
 * 
 * 
 * 
 */

static char nlc_prinft_sub_buffer[4096 * 8];

extern "C" int nlc_prinft_sub_(char const *format, ...) {
    va_list args;
    va_start(args, format);

    int result = vsnprintf(nlc_prinft_sub_buffer, sizeof (nlc_prinft_sub_buffer), format, args);
    STATIC_ASSERT(sizeof (nlc_prinft_sub_buffer) == 4096 * 8);

    Logger *log = Logger::Instance();
    if (log->m_func != nullptr) {
        (*log->m_func)(log->m_func_param, LOG_LEVEL_INFO, nlc_prinft_sub_buffer, true);
    }

    //    int result = vfprintf(stdout, format, args);
    va_end(args);
    return result;
}

void RunTime::Clear() {
    m_main_ast.reset();
    m_main_runner.reset();
    m_main_module.reset();
}

TermPtr RunTime::GlobFindProto(const std::string_view name) {
    GlobItem *glob = NameFind(name.begin());
    if (glob) {
        return glob->proto;
    }
    return nullptr;
}

GlobItem * RunTime::NameFind(const char* name) {

    RunTime::iterator found;

    if (!name) {
        return nullptr;
    }

    if (isMacroName(name)) {
        LOG_RUNTIME("Macro name not allowed! '%s'", name);
    }
    //    if (isLocalName(name)) {
    //        LOG_RUNTIME("Local name not allowed! '%s'", name);
    //    }

    if (isGlobalScope(name) || isTypeName(name)) {

        found = find(name);
        if (found == end()) {
            return nullptr;
        }

    } else {

        found = find(name);
        if (found != end()) {
            //@todo  Check typename ????
            return &found->second;
            //            LOG_RUNTIME("Fail logical for name '%s' as '%s'", name, found->first.c_str());
        }

        std::string glob_name("::");

        glob_name.append(name);
        found = find(glob_name);
        if (found == end()) {
            return nullptr;
        }
    }

    ASSERT(found != end());

    //    if (!std::holds_alternative<ObjWeak>(found->second.obj)) {

    //        if (std::holds_alternative<ObjWeak>(found->second.obj)) {
    //            return std::get<ObjWeak>(found->second.obj).lock();
    //        }
    //        ASSERT(std::holds_alternative<std::vector < ObjWeak >> (found->second.obj));
    //        return std::get<std::vector < ObjWeak >> (found->second.obj)[0].lock();

    //        return std::get<std::vector < ObjWeak >> (found->second.obj)[0];

    //    } else {
    //        NL_PARSER(found->second.proto, "Global name not implemented! '%s'", found->first.c_str());
    //    }
    return &found->second;
}

TermPtr RunTime::NameRegister(bool new_only, const char *name, TermPtr proto, WeakItem obj) {

    GlobItem *item = NameFind(name);

    if (new_only && item) {
        NL_MESSAGE(LOG_LEVEL_INFO, proto, "Name '%s' already exist!", name);
        return nullptr;
    } else {
        if (item) {
            if (!Term::CheckTermEq(item->proto, proto)) {
                ASSERT(item->proto);
                NL_MESSAGE(LOG_LEVEL_INFO, proto, "The prototype '%s' differs from the first definition '%s'!",
                        name, item->proto->asTypeString().c_str());
                return nullptr;
            }
        } else {
            insert(std::pair(name, GlobItem({proto, obj})));
        }
    }
    return at(name).proto;
}

/*
 * 
 * 
 * 
 * 
 * 
 * 
 */

ObjPtr RunTime::NameGet(const char *name, bool is_raise) {

    GlobItem * ret = NameFind(name);

    if (ret) {
        if (std::holds_alternative<ObjWeak>(ret->obj)) {
            return std::get<ObjWeak>(ret->obj).lock();
        } else if (std::holds_alternative<std::vector < ObjWeak >> (ret->obj)) {
            return std::get<std::vector < ObjWeak >> (ret->obj)[0].lock();
        }
        if (is_raise) {
            NL_PARSER(ret->proto, "Global name not implemented! '%s'", name);
        }
    } else {
        if (is_raise) {
            LOG_RUNTIME("Global Name '%s' not found!", name);
        }
    }
    return nullptr;
}

//    
//    ObjPtr result = nullptr;
//    ObjPtr temp = nullptr;
//    ObjPtr args = nullptr;
//    ObjPtr value = nullptr;
//    TermPtr field = nullptr;
//    std::string full_name;
//
//    result = Obj::CreateNone();
//    result->m_is_reference = !!term->m_ref;
//
//    int64_t val_int;
//    double val_dbl;
//    ObjType type;
//    bool has_error;
//    std::vector<int64_t> sizes;
//    at::Scalar torch_scalar;
//    switch (term->getTermID()) {
//        case TermID::OP_MATH:
//        case TermID::OP_BITWISE:
//        case TermID::OP_COMPARE:
//        case TermID::OP_LOGICAL:
//            return Context::Eval(ctx, term, local_vars, eval_block, int_catch);
//
//        case TermID::INTEGER:
//
//            val_int = parseInteger(term->getText().c_str());
//            NL_TYPECHECK(term, typeFromLimit(val_int), typeFromString(term->m_type, ctx ? ctx->m_runtime.get() : nullptr)); // Соответстствует ли тип значению?
//
//            result = Obj::CreateValue(val_int);
//            result->m_var_type_current = typeFromLimit(val_int);
//            if (term->GetType()) {
//                result->m_var_type_fixed = typeFromString(term->m_type, ctx ? ctx->m_runtime.get() : nullptr);
//                result->m_var_type_current = result->m_var_type_fixed;
//            }
//            return result;
//
//        case TermID::NUMBER:
//            val_dbl = parseDouble(term->getText().c_str());
//            NL_TYPECHECK(term, typeFromLimit(val_dbl), typeFromString(term->m_type, ctx ? ctx->m_runtime.get() : nullptr)); // Соответстствует ли тип значению?
//
//            result = Obj::CreateValue(val_dbl);
//            result->m_var_type_current = typeFromLimit(val_dbl);
//            if (term->GetType()) {
//                result->m_var_type_fixed = typeFromString(term->m_type, ctx ? ctx->m_runtime.get() : nullptr);
//                result->m_var_type_current = result->m_var_type_fixed;
//            }
//            return result;
//
//        case TermID::STRWIDE:
//            return Obj::CreateString(utf8_decode(term->getText()));
//
//
//        case TermID::STRCHAR:
//            return Obj::CreateString(term->getText());
//
//        case TermID::EVAL:
//            /*
//             * subprocess.run(args, *, stdin=None, input=None, stdout=None, stderr=None, 
//             * capture_output=False, shell=False, cwd=None, timeout=None, check=False, 
//             * encoding=None, errors=None, text=None, env=None, universal_newlines=None, **other_popen_kwargs)            
//             */
//            /* class subprocess.Popen(args, bufsize = -1, executable = None, 
//             * stdin = None, stdout = None, stderr = None, 
//             * preexec_fn = None, close_fds = True, shell = False, cwd = None, 
//             * env = None, universal_newlines = None, startupinfo = None, 
//             * creationflags = 0, restore_signals = True, start_new_session = False, 
//             * pass_fds = (), *, group = None, extra_groups = None, user = None, umask = -1
//             * , encoding = None, errors = None, text = None, 
//             * pipesize = -1, process_group = None)¶
//             */
//
//
//
//            //            result = Obj::CreateType(ObjType::Eval, ObjType::Eval, true);
//            //            result->m_value = term->getText();
//            //            if (term->isCall()) {
//            LOG_RUNTIME("Not implemented!!!");
//            //            }
//            //            return Exec(ctx, term->getText().c_str(), nullptr);
//
//            /*        case TermID::FIELD:
//                        if(module && module->HasFunc(term->GetFullName().c_str())) {
//                            // Если поле является функцией и она загружена
//                            result = Obj::CreateType(Obj::Type::FUNCTION,
//               term->GetFullName().c_str()); result->m_module = module;
//                            result->m_is_const = term->m_is_const;
//                            result->m_is_ref = term->m_is_ref;
//                            return result;
//                        }
//                        if(!result) {
//                            LOG_RUNTIME("Term '%s' not found!",
//               term->toString().c_str());
//                        }
//                        return result;
//             */
//
//            //@todo Что делать с пустыми значениями? Это None ???
//        case TermID::EMPTY:
//            result->m_var_type_current = ObjType::None;
//            result->m_var_is_init = false;
//            return result;
//
//
//
//        case TermID::NEWLANG:
//        case TermID::MODULE:
//            /* Prototype \\(file=_, filter='*', load=@true, source=_)
//             * Required argument $file or $source can be only one
//             * 
//             * \\('name') - dynamic load module
//             * \\dir\name() - load module and 
//             * \\dir\name::func() - call function from module
//             * module := \\('name', @false) - Check module is loaded and don't load it
//             * 
//             */
//
//            if (term->isCall() && ExtractName(term->getText()).empty()) {
//                // Load module
//
//                return ctx->m_runtime->OpLoadModule(term);
//            }
//
//            //            if (!ctx->m_runtime->CkeckLoadModule(term)) {
//            //                NL_PARSER(term, "The '%s' module must be preloaded using the '%s()' command!", term->getText().c_str(), term->getText().c_str());
//            //            }
//
//
//
//
//            //            if (isModule(term->GetFullName())) {
//            //                if (term->isCall()) {
//            //                    /*
//            //                     * Операцию с модулем можно конкреттизировать с помощью аргумента
//            //                     * \module() -  загрузка модуля и импорт всех его объектов
//            //                     * \module("filter*") -  загрузка модуля и импорт всех его объектов по заданной маске в переданой строке
//            //                     * \module("func;ns::*;ns2::*") -  несколько масок записываются через точку с запятой
//            //                     * \module(@true) - убедиться, что модуль загружен. Если модуль не загружен, создать ошибку
//            //                     * \module(@false) - проверить, загружен ли модуль без его реальной загрузки и вернуть значение
//            //                     * \module(_) -  Выгрузка модуля
//            //                     * 
//            //                     * \\dsl() -  Единственный встроенный модуль
//            //                     */
//            //
//            //                    if (term->size() > 1) {
//            //                        NL_PARSER(term, "Only one argument can be specified for a load module!");
//            //                    }
//            //
//            //                    std::vector<std::string> filter;
//            //                    if (term->size()) {
//            //                        ASSERT(term->size() == 1);
//            //                        args = CreateRVal(ctx, term->at(0).second, local_vars);
//            //                        ASSERT(args);
//            //
//            //                        auto found = ctx->m_runtime->m_modules.find(term->getText());
//            //                        if (args->is_bool_type()) {
//            //
//            //                            if (args->GetValueAsBoolean() && found == ctx->m_runtime->m_modules.end()) {
//            //                                NL_PARSER(term, "Module %s not loaded!", term->getText().c_str());
//            //                            }
//            //                            return found == ctx->m_runtime->m_modules.end() ? Obj::Yes() : Obj::No();
//            //
//            //                        } else if (args->is_none_type()) {
//            //
//            //                            result = (found == ctx->m_runtime->m_modules.end() ? Obj::Yes() : Obj::No());
//            //                            if (found != ctx->m_runtime->m_modules.end()) {
//            //                                ctx->m_runtime->m_modules.erase(found);
//            //                            }
//            //
//            //                            ctx->CleanUp();
//            //
//            //                            return result;
//            //
//            //                        } else if (args->is_string_type()) {
//            //                            filter = Macro::SplitString(args->GetValueAsString().c_str(), ";");
//            //                        }
//            //                    }
//            //
//            //
//            //                    if (!ctx->m_runtime->CheckOrLoadModule(term->GetFullName())) {
//            //                        LOG_RUNTIME("Fail load module %s!", ExtractModuleName(term->GetFullName().c_str()).c_str());
//            //                    }
//            //
//            //
//            //                    size_t count = 0;
//            //                    auto module = ctx->m_runtime->m_modules[term->getText()];
//            //
//            //                    if (filter.empty()) {
//            //                        for (size_t i = 0; i < module->size(); i++) {
//            //                            ctx->push_front(module->at(i));
//            //                            count++;
//            //                        }
//            //                    } else {
//            //
//            //                        LOG_WARNING("Import of objects by mask does not work correctly!");
//            //
//            //                        std::vector<std::regex> regs;
//            //                        for (auto &elem : filter) {
//            //                            try {
//            //                                //                            LOG_DEBUG("From: %s", elem.c_str());
//            //
//            //                                //                            if(elem.rfind("::") != 0) {
//            //                                //                                elem.insert(0, "::");
//            //                                //                            }
//            //                                //                            elem.insert(0, term->getText());
//            //
//            //                                elem = std::regex_replace(elem, std::regex("@."), "@@.");
//            //                                elem = std::regex_replace(elem, std::regex("@*"), "(.)*");
//            //                                //                            LOG_DEBUG("To: %s", elem.c_str());
//            //                                regs.push_back(std::regex(elem));
//            //                            } catch (const std::regex_error &err) {
//            //                                LOG_RUNTIME("Regular expression for '%s' error '%s'!", elem.c_str(), err.what());
//            //                            }
//            //
//            //                        }
//            //
//            //                        for (size_t i = 0; i < module->size(); i++) {
//            //                            for (auto &elem : regs) {
//            //                                //                            LOG_DEBUG("Test: %s", module->at(i).first.c_str());
//            //                                if (std::regex_match(module->at(i).first, elem)) {
//            //                                    //                                LOG_DEBUG("Mask: %s", module->at(i).first.c_str());
//            //                                    ctx->push_front(module->at(i));
//            //                                    count++;
//            //                                    break;
//            //                                }
//            //                            }
//            //                        }
//            //                    }
//            //
//            //                    return Obj::CreateValue(count, ObjType::Integer);
//            //
//            //                } else {
//            //                    std::string name = ExtractName(term->GetFullName());
//            //                    if (isSystemName(name)) {
//            //                        std::string module = ExtractModuleName(term->GetFullName().c_str());
//            //                        auto found = ctx->m_runtime->m_modules.find(module);
//            //                        if (found != ctx->m_runtime->m_modules.end()) {
//            //                            return CheckSystemField(found->second.get(), name);
//            //                        } else {
//            //                            NL_PARSER(term, "Module %s not loaded!", name.c_str());
//            //                        }
//            //                    }
//            //                }
//            //            }
//
//            NL_PARSER(term, "Term '%s' !!!!!!!!!!!!!!!!!!!!!!!!!", term->GetFullName().c_str());
//
//            //            if (var_name.empty() && isModule(term->GetFullName()) && term->isCall()) {
//            //
//            //            }
//
//        case TermID::LOCAL:
//        case TermID::STATIC:
//        case TermID::NAME:
//        case TermID::ARGS:
//        case TermID::PARENT:
//        case TermID::INT_PLUS:
//        case TermID::INT_MINUS:
//
//
//            if (!isModuleName(term->GetFullName()) && (term->isCall() || term->isReturn())) {
//                temp = ctx->GetTerm(term->GetFullName().c_str(), term->isRef());
//
//                if (!temp) {
//                    NL_PARSER(term, "Term '%s' not found!", term->GetFullName().c_str());
//                }
//
//                args = Obj::CreateDict();
//                ctx->CreateArgs_(args, term, local_vars);
//
//                if (term->isReturn()) {
//                    return result;
//                }
//
//                result = temp->Call(ctx, args.get());
//                return result;
//            }
//
//
//            if (term->GetType()) {
//
//                result->m_var_type_current = typeFromString(term->GetType()->m_text, ctx ? ctx->m_runtime.get() : nullptr);
//                result->m_var_type_fixed = result->m_var_type_current;
//                result->m_var_is_init = false; // Нельзя считать значение
//
//                // Check BuildInType
//                has_error = false;
//                typeFromString(term->GetType()->m_text, nullptr, &has_error);
//                if (has_error) {
//                    result->m_class_name = term->GetType()->m_text;
//                }
//
//                return result;
//            }
//
//            //- **\this** - Текущий объект (**$0**)
//            //- **\sys** - Системный контекст запущенной программы (**\\**)
//            //- **\current** - Текущий модуль (** \$ **)
//            //- **\cmd** - Все аргументы выполняющегося приложения из командной строки (**@\***)
//
//            if (term->m_text.compare("_") == 0) {
//
//                result->m_var_type_current = ObjType::None;
//                return result;
//
//            } else if (term->m_text.compare("$$") == 0) {
//
//                //- **\parent** - Родительский объект (**$$**)
//
//                result->m_var_type_current = ObjType::Dictionary;
//                result->m_var_name = "$$";
//
//                ASSERT(ctx);
//
//                auto iter = ctx->begin();
//                while (iter != ctx->end()) {
//                    if (!iter->second.expired()) {
//                        result->push_back(Obj::CreateString(iter->first));
//                        iter++;
//                    } else {
//                        iter = ctx->ListType::erase(iter);
//                    }
//                }
//
//                result->m_var_is_init = true;
//                return result;
//
//            } else if (term->m_text.compare("$*") == 0) {
//
//                //- **\args** - Все аргументы функции (**$\***)
//
//                result->m_var_type_current = ObjType::Dictionary;
//                result->m_var_name = "$*";
//                result->m_var_is_init = true;
//
//                for (int i = 0; i < term->size(); i++) {
//                    if (term->name(i).empty()) {
//                        result->push_back(Context::CreateRVal(ctx, (*term)[i].second, local_vars));
//                    } else {
//                        result->push_back(Context::CreateRVal(ctx, (*term)[i].second, local_vars), term->name(i));
//                    }
//                }
//
//                return result;
//
//                //            } else if (term->m_text.compare("@@") == 0) {
//                //
//                //                if (term->m_is_call) {
//                //
//                //                    Obj args(ctx, term, true, local_vars);
//                //
//                //                    if (args.size() == 0 || !args.at(0).second->is_string_type()) {
//                //                        LOG_RUNTIME("Requires a filename in the first argument! %s", args.toString().c_str());
//                //                    }
//                //
//                //                    return ctx->ExecFile(args.at(0).second->GetValueAsString().c_str(), &args);
//                //
//                //                } else {
//                //                    return ctx->m_main_module->shared();
//                //                }
//                //
//                //            } else if (term->m_text.compare("@$") == 0) {
//                //
//                //                return ctx->m_terms->shared();
//                //
//                //            } else if (term->m_text.compare("@*") == 0) {
//                //                if (ctx->m_runtime) {
//                //                    return ctx->m_runtime->m_args;
//                //                } else {
//                //                    return Obj::CreateType(ObjType::Dictionary, ObjType::Dictionary, true);
//                //                }
//            }
//
//
//            //            if (isModule(term->GetFullName()) && term->m_right) {
//            //                bool is_call = term->m_is_call;
//            //                std::string name = term->m_text;
//            //                TermPtr field = term->m_right;
//            //                while (field) {
//            //                    if (name.compare(term->m_text) == 0) {
//            //                        name += "::";
//            //                    } else {
//            //                        name += ".";
//            //                    }
//            //                    name += field->m_text;
//            //                    term = field;
//            //                    field = field->m_right;
//            //                }
//            //                term->m_text = name;
//            //                term->m_is_call = is_call;
//            //            }
//            //
//            //            if (isModule(term->GetFullName())) { // !isVariableName(term->GetFullName())
//            //                if (term->isCall()) {
//            //                    /*
//            //                     * Операцию с модулем можно конкреттизировать с помощью аргумента
//            //                     * @module() -  загрузка модуля и импорт всех его объектов
//            //                     * @module("filter*") -  загрузка модуля и импорт всех его объектов по заданной маске в переданой строке
//            //                     * @module("func;ns::*;ns2::*") -  несколько масок записываются через точку с запятой
//            //                     * @module(\true) - убедиться, что модуль загружен. Если модуль не загружен, создать ошибку
//            //                     * @module(\false) - проверить, загружен ли модуль без его реальной загрузки и вернуть значение
//            //                     * @module(_) -  Выгрузка модуля
//            //                     * 
//            //                     * @dsl() -  Единственный встроенный модуль
//            //                     */
//            //
//            //                    //                    if(term->GetFullName().compare("@dsl")==0) {
//            //                    //                        std::string str;
//            //                    //
//            //                    //                        for (int i = 0; i < ::newlang_dsl_size; i++) {
//            //                    //                            str += ::newlang_dsl_arr[i];
//            //                    //                            str += "\n";
//            //                    //                        }
//            //                    //
//            //                    //                        m_macros.clear();
//            //                    //                        Parser::ParseAllMacros(str, &m_macros);
//            //                    //
//            //                    //                        result->m_var_type_current = ObjType::Dictionary;
//            //                    //                        result->m_var_is_init = true;
//            //                    //
//            //                    //                        for (auto &elem : m_macros) {
//            //                    //                            result->push_back(Obj::CreateString(elem.first));
//            //                    //                        }
//            //                    //                        return result;
//            //                    //                    }
//            //
//            //                    if (term->size() > 1) {
//            //                        NL_PARSER(term, "Only one argument can be specified for a load module!");
//            //                    }
//            //
//            //                    std::vector<std::string> filter;
//            //                    if (term->size()) {
//            //                        ASSERT(term->size() == 1);
//            //                        args = CreateRVal(ctx, term->at(0).second, local_vars);
//            //                        ASSERT(args);
//            //
//            //                        auto found = ctx->m_modules.find(term->getText());
//            //                        if (args->is_bool_type()) {
//            //
//            //                            if (args->GetValueAsBoolean() && found == ctx->m_modules.end()) {
//            //                                NL_PARSER(term, "Module %s not loaded!", term->getText().c_str());
//            //                            }
//            //                            return found == ctx->m_modules.end() ? Obj::Yes() : Obj::No();
//            //
//            //                        } else if (args->is_none_type()) {
//            //
//            //                            result = (found == ctx->m_modules.end() ? Obj::Yes() : Obj::No());
//            //                            if (found != ctx->m_modules.end()) {
//            //                                ctx->m_modules.erase(found);
//            //                            }
//            //
//            //                            ctx->CleanUp();
//            //
//            //                            return result;
//            //
//            //                        } else if (args->is_string_type()) {
//            //                            filter = SplitString(args->GetValueAsString().c_str(), ";");
//            //                        }
//            //                    }
//            //
//            //
//            //                    if (!ctx->CheckOrLoadModule(term->GetFullName())) {
//            //                        LOG_RUNTIME("Fail load module %s!", ExtractModuleName(term->GetFullName().c_str()).c_str());
//            //                    }
//            //
//            //
//            //                    size_t count = 0;
//            //                    auto module = ctx->m_modules[term->getText()];
//            //
//            //                    if (filter.empty()) {
//            //                        for (size_t i = 0; i < module->size(); i++) {
//            //                            ctx->push_front(module->at(i));
//            //                            count++;
//            //                        }
//            //                    } else {
//            //
//            //                        LOG_WARNING("Import of objects by mask does not work correctly!");
//            //
//            //                        std::vector<std::regex> regs;
//            //                        for (auto &elem : filter) {
//            //                            try {
//            //                                //                            LOG_DEBUG("From: %s", elem.c_str());
//            //
//            //                                //                            if(elem.rfind("::") != 0) {
//            //                                //                                elem.insert(0, "::");
//            //                                //                            }
//            //                                //                            elem.insert(0, term->getText());
//            //
//            //                                elem = std::regex_replace(elem, std::regex("@."), "@@.");
//            //                                elem = std::regex_replace(elem, std::regex("@*"), "(.)*");
//            //                                //                            LOG_DEBUG("To: %s", elem.c_str());
//            //                                regs.push_back(std::regex(elem));
//            //                            } catch (const std::regex_error &err) {
//            //                                LOG_RUNTIME("Regular expression for '%s' error '%s'!", elem.c_str(), err.what());
//            //                            }
//            //
//            //                        }
//            //
//            //                        for (size_t i = 0; i < module->size(); i++) {
//            //                            for (auto &elem : regs) {
//            //                                //                            LOG_DEBUG("Test: %s", module->at(i).first.c_str());
//            //                                if (std::regex_match(module->at(i).first, elem)) {
//            //                                    //                                LOG_DEBUG("Mask: %s", module->at(i).first.c_str());
//            //                                    ctx->push_front(module->at(i));
//            //                                    count++;
//            //                                    break;
//            //                                }
//            //                            }
//            //                        }
//            //                    }
//            //
//            //                    return Obj::CreateValue(count, ObjType::Integer);
//            //
//            //                } else {
//            //                    std::string name = ExtractName(term->GetFullName());
//            //                    if (isSystemName(name)) {
//            //                        std::string module = ExtractModuleName(term->GetFullName().c_str());
//            //                        auto found = ctx->m_modules.find(module);
//            //                        if (found != ctx->m_modules.end()) {
//            //                            return CheckSystemField(found->second.get(), name);
//            //                        } else {
//            //                            NL_PARSER(term, "Module %s not loaded!", name.c_str());
//            //                        }
//            //                    }
//            //                }
//            //            }
//
//
//
//
//            if (isLocalName(term->m_text.c_str())) {
//                full_name = MakeName(term->m_text);
//                return local_vars->at(full_name).second;
//            } else {
//                result = ctx->GetTerm(term->GetFullName().c_str(), term->isRef());
//
//                // Типы данных обрабатываются тут, а не в вызовах функций (TermID::CALL)
//                if (!result) {
//                    NL_PARSER(term, "Object '%s' not exist!", term->toString().c_str());
//                }
//                if (term->size()) {
//                    Obj args(ctx, term, true, local_vars);
//                    result = result->Call(ctx, &args);
//                }
//
//            }
//            if (!result) {
//                // Делать ислкючение или возвращать объект "ошибка" ?????
//                ASSERT(term);
//                LOG_RUNTIME("Term '%s' not found!", term->GetFullName().c_str());
//            }
//
//            field = term->m_right;
//            if (field && field->getTermID() == TermID::FIELD) {
//                while (field) {
//                    if (field->m_is_call) {
//
//                        if (result->m_class_name.empty()) {
//                            NL_PARSER(term, "Object '%s' not a class! %s", term->GetFullName().c_str(), result->m_class_name.c_str());
//                        }
//
//                        full_name = result->m_class_name;
//                        if (isTypeName(full_name)) {
//                            full_name = full_name.substr(1);
//                        }
//                        full_name += "::";
//                        full_name += field->getText();
//
//                        temp = ctx->GetTerm(full_name, true);
//                        if (!temp) {
//                            LOG_RUNTIME("Function '%s' not found!", full_name.c_str());
//                        }
//
//                        args = Obj::CreateDict();
//                        for (size_t i = 0; i < field->size(); i++) {
//                            if (field->name(i).empty()) {
//                                args->push_back(CreateRVal(ctx, (*field)[i].second, local_vars));
//                            } else {
//                                args->push_back(CreateRVal(ctx, (*field)[i].second, local_vars), field->name(i).c_str());
//                            }
//                        }
//
//                        result = temp->Call(ctx, args.get(), true, result);
//
//                    } else {
//                        result = result->at(field->getText()).second;
//                    }
//                    field = field->m_right;
//                }
//            } else if (field && field->getTermID() == TermID::INDEX) {
//                while (field) {
//                    result = result->index_get(MakeIndex(ctx, field, local_vars));
//                    field = field->m_right;
//                    ASSERT(!field); // Нужно выполнять, а не просто получать значение поля
//                }
//            } else if (field) {
//                LOG_RUNTIME("Not implemented! %s", field->toString().c_str());
//            }
//
//            return result;
//
//
//
//        case TermID::TYPE:
//
//            result = ctx->GetTerm(term->GetFullName().c_str(), term->isRef());
//
//
//            has_error = false;
//            type = typeFromString(term->GetFullName(), ctx ? ctx->m_runtime.get() : nullptr, &has_error);
//            if (has_error) {
//                LOG_RUNTIME("Type name '%s' undefined!", term->GetFullName().c_str());
//            }
//            ASSERT(result);
//            ASSERT(result->m_var_type_fixed == type);
//
//            if (result->m_var_type_fixed == ObjType::Class) {
//                //@todo Virtual
//
//            }
//
//            // Размерность, если указана
//            result->m_dimensions = Obj::CreateType(ObjType::Dictionary, ObjType::Dictionary, true);
//            for (size_t i = 0; i < term->m_dims.size(); i++) {
//                result->m_dimensions->push_back(CreateRVal(ctx, term->m_dims[i], local_vars, eval_block, int_catch));
//            }
//
//            args = Obj::CreateDict();
//            for (int64_t i = 0; i < static_cast<int64_t> (term->size()); i++) {
//
//
//                if ((*term)[i].second->getTermID() == TermID::FILLING) {
//
//                    // Заполнение значений вызовом функции
//                    // :Type(1, 2, 3, ... rand() ... );
//
//
//                    ASSERT(!(*term)[i].second->Left());
//                    ASSERT((*term)[i].second->Right());
//
//
//                    ObjPtr expr = ctx->FindTerm((*term)[i].second->Right()->GetFullName());
//                    ASSERT(expr);
//
//                    if (!(*term)[i].second->Right()->isCall()) {
//                        LOG_RUNTIME("Operator filling supported function call only!");
//                    }
//
//                    if (i + 1 != term->size()) {
//                        LOG_RUNTIME("Function filling is supported for the last argument only!");
//                    }
//
//                    if (!result->m_dimensions || !result->m_dimensions->size()) {
//                        LOG_RUNTIME("Object has no dimensions!");
//                    }
//
//                    int64_t full_size = 1;
//                    for (int dim_index = 0; dim_index < result->m_dimensions->size(); dim_index++) {
//
//                        if (!(*result->m_dimensions)[dim_index].second->is_integer()) {
//                            LOG_RUNTIME("Dimension index for function filling support integer value only!");
//                        }
//
//                        full_size *= (*result->m_dimensions)[dim_index].second->GetValueAsInteger();
//                    }
//
//                    if (full_size <= 0) {
//                        LOG_RUNTIME("Items count error for all dimensions!");
//                    }
//
//
//                    if (expr->size()) {
//                        LOG_RUNTIME("Argument in function for filling not implemented!");
//                    }
//
//                    for (int64_t dim_index = args->size(); dim_index < full_size; dim_index++) {
//                        args->push_back(expr->Call(ctx));
//                    }
//
//                    break;
//
//                } else if ((*term)[i].second->getTermID() == TermID::ELLIPSIS) {
//
//                    if (!term->name(i).empty()) {
//                        LOG_RUNTIME("Named ellipsys not implemented!");
//                    }
//
//                    if ((*term)[i].second->Right()) {
//
//                        bool named = ((*term)[i].second->Left() && (*term)[i].second->Left()->getTermID() == TermID::ELLIPSIS);
//                        ObjPtr exp = CreateRVal(ctx, (*term)[i].second->Right(), eval_block);
//
//                        if (!exp->is_dictionary_type()) {
//                            LOG_RUNTIME("Expansion operator applies to dictionary only!");
//                        }
//
//
//                        for (int index = 0; index < exp->size(); index++) {
//                            if (named) {
//                                args->push_back((*exp)[index].second, exp->name(index).empty() ? "" : exp->name(index));
//                            } else {
//                                args->push_back((*exp)[index].second);
//                            }
//                        }
//
//                        continue;
//                    }
//                }
//
//                if (term->name(i).empty()) {
//                    args->push_back(CreateRVal(ctx, (*term)[i].second, local_vars));
//                } else {
//                    args->push_back(CreateRVal(ctx, (*term)[i].second, local_vars), term->name(i).c_str());
//                }
//
//            }
//
//            result = result->Call(ctx, args.get());
//            ASSERT(result);
//
//            return result;
//
//
//        case TermID::TENSOR:
//        case TermID::DICT:
//            result->m_var_type_current = ObjType::Dictionary;
//            ctx->CreateArgs_(result, term, local_vars);
//
//            result->m_var_is_init = true;
//            if (term->getTermID() == TermID::TENSOR) {
//
//                if (!term->m_type) {
//                    result->m_var_type_fixed = ObjType::None;
//                } else {
//                    result->m_var_type_fixed = typeFromString(term->m_type->m_text, ctx ? ctx->m_runtime.get() : nullptr);
//                }
//                type = getSummaryTensorType(result.get(), result->m_var_type_fixed);
//
//                if (type != ObjType::None) {
//
//
//                    sizes = TensorShapeFromDict(result.get());
//                    result->toType_(type);
//
//                    if (!sizes.empty()) {
//                        ASSERT(result->m_tensor->defined());
//                        result->m_tensor = result->m_tensor->reshape(sizes);
//                    }
//
//
//                } else {
//                    result->m_var_is_init = false;
//                }
//                //                result->resize(0, nullptr, "");
//                result->m_var_type_current = type;
//            } else {
//                result->m_class_name = term->m_class;
//            }
//            return result;
//
//        case TermID::ARGUMENT:
//
//            val_int = IndexArg(term);
//            if (val_int < local_vars->size()) {
//                return local_vars->at(val_int).second;
//            }
//            LOG_RUNTIME("Argument '%s' not exist!", term->toString().c_str());
//
//        case TermID::BLOCK:
//            return CallBlock(ctx, term, local_vars, eval_block, CatchType::CATCH_AUTO, nullptr);
//
//        case TermID::BLOCK_TRY:
//        case TermID::BLOCK_PLUS:
//        case TermID::BLOCK_MINUS:
//            return CallBlock(ctx, term, local_vars, eval_block, int_catch, nullptr);
//
//            //        case TermID::BLOCK_PLUS:
//            //            return CallBlock(ctx, term, local_vars, CatchType::CATCH_PLUS);
//            //            
//            //        case TermID::BLOCK_MINUS:
//            //            return CallBlock(ctx, term, local_vars, CatchType::CATCH_MINUS);
//
//        case TermID::ELLIPSIS:
//            result->m_var_type_current = ObjType::Ellipsis;
//            result->m_var_type_fixed = ObjType::None;
//            result->m_var_is_init = true;
//            return result;
//
//        case TermID::RANGE:
//
//            result->m_var_type_current = ObjType::Dictionary;
//            for (int i = 0; i < term->size(); i++) {
//                ASSERT(!term->name(i).empty());
//                result->push_back(Eval(ctx, (*term)[i].second, local_vars, eval_block), term->name(i).c_str());
//            }
//
//            if (result->size() == 2) {
//                result->push_back(Obj::CreateValue(1, ObjType::None), "step");
//            }
//
//            result->m_var_type_current = ObjType::Range;
//            result->m_var_type_fixed = ObjType::Range;
//            result->m_var_is_init = true;
//
//            return result;
//
//        case TermID::RATIONAL:
//            return Obj::CreateRational(term->m_text);
//
//        case TermID::ITERATOR:
//
//            ASSERT(term->Left());
//
//            temp = Eval(ctx, term->Left(), local_vars, eval_block, CatchType::CATCH_AUTO);
//            if (!temp) {
//                LOG_RUNTIME("Term '%s' not found!", term->Left()->GetFullName().c_str());
//            }
//
//            args = Obj::CreateDict();
//            ctx->CreateArgs_(args, term, local_vars);
//
//
//            /*
//             * Создание итератора
//             * ?, ?(), ?("Фильтр"), ?(func), ?(func, args...)
//             * 
//             * Перебор элементов итератора
//             * !, !(), !(0), !(3), !(-3)
//             * 
//             * dict! и dict!(0) эквивалентны
//             * dict! -> 1,  dict! -> 2, dict! -> 3, dict! -> 4, dict! -> 5, dict! -> :IteratorEnd
//             * 
//             * Различия отрицательного размера возвращаемого словаря для итератора
//             * dict!(-1) -> (1,),  ...  dict!(-1) -> (5,),  dict!(-1) -> (:IteratorEnd,),  
//             * dict!(1) -> (1,),  ...  dict!(1) -> (5,),  dict!(1) -> (,),  
//             * dict!(-3) -> (1, 2, 3,),  dict!(-3) -> (4, 5, :IteratorEnd,)
//             * dict!(3) -> (1, 2, 3,), dict!(3) -> (4, 5,)
//             * 
//             * Операторы ?! и !? эквивалентны и возвращают текущие данные без перемещения указателя итератора.
//             * 
//             * Оператор ?? создает итератор и сразу его выполняет, возвращая все значения 
//             * в виде элементов словаря, т.е. аналог последовательности ?(LINQ); !(:Int64.__max__);
//             * 
//             * Оператор !! - сбрасывает итератор в начальное состояние и возвращает первый элемент
//             */
//
//            if (term->m_text.compare("?") == 0) {
//                return temp->IteratorMake(args.get());
//            } else if (term->m_text.compare("!") == 0) {
//                ASSERT(!args->size() && "Argument processing not implemented");
//                return temp->IteratorNext(0);
//            } else if (term->m_text.compare("!!") == 0) {
//                ASSERT(!args->size() && "Argument processing not implemented");
//                temp->IteratorReset();
//                return temp->IteratorData();
//            } else if (term->m_text.compare("!?") == 0 || term->m_text.compare("?!") == 0) {
//                return temp->IteratorData();
//            } else if (term->m_text.compare("??") == 0) {
//
//                val_int = std::numeric_limits<int64_t>::max();
//                if (args->empty() || (args->size() == 1 && args->at(0).second->is_integer())) {
//                    result = temp->IteratorMake(nullptr, false);
//                    if (args->size()) {
//                        val_int = args->at(0).second->GetValueAsInteger();
//                    }
//                } else if (args->size() == 1 && args->at(0).second->is_string_type()) {
//                    result = temp->IteratorMake(args->at(0).second->GetValueAsString().c_str(), false);
//                } else if (args->size() == 2 && args->at(0).second->is_string_type() && args->at(1).second->is_integer()) {
//                    result = temp->IteratorMake(args->at(0).second->GetValueAsString().c_str(), false);
//                    val_int = args->at(1).second->GetValueAsInteger();
//                } else {
//                    LOG_RUNTIME("Iterator`s args '%s' not allowed!", args->toString().c_str());
//                }
//                return result->IteratorNext(val_int);
//
//            } else {
//                LOG_RUNTIME("Iterator '%s' not recognized in '%s'!", term->m_text.c_str(), term->toString().c_str());
//            }
//    }
//    LOG_RUNTIME("Fail create type %s from '%s'", newlang::toString(term->getTermID()), term->toString().c_str());
//
//    return nullptr;

bool RunTime::LoadModuleFromFile(const char *term, bool init) {
    ASSERT(term);

    //    std::string name = ExtractModuleName(term);
    //    bool is_root = (name.find("\\\\") == 0);
    //    if (is_root) {
    //        LOG_WARNING("Load module from root not implemented! '%s'", name.c_str());
    //        name = name.substr(1); // remove root marker (double back slash) i.e. \\root\dir\module
    //    }
    //    std::vector<std::string> split = Macro::SplitString(name.substr(1).c_str(), "\\");
    //
    //    std::string path;
    //    for (auto &elem : split) {
    //        if (!path.empty()) {
    //            path += llvm::sys::path::get_separator();
    //        }
    //        path += elem;
    //    }
    //
    //    path += ".src";
    //
    //
    //    llvm::SmallString<1024> search_file;
    //    if (llvm::sys::path::is_relative(path)) {
    //        for (int i = 0; i < m_search_dir.size(); i++) {
    //
    //            search_file = m_search_dir[i];
    //            llvm::sys::path::append(search_file, path);
    //
    //            std::string full_path = search_file.c_str();
    //            if (llvm::sys::path::is_relative(full_path)) {
    //                full_path.insert(0, llvm::sys::path::get_separator().str());
    //                full_path.insert(0, m_work_dir);
    //            }
    //
    //            LOG_TEST("Check '%s' in file %s", name.c_str(), full_path.c_str());
    //
    //            if (llvm::sys::fs::exists(full_path)) {
    //                LOG_TEST("Module '%s' load from file '%s'!", term, full_path.c_str());
    //
    //                std::string str = ReadFile(full_path.c_str());
    //
    //                ParserPtr parser = GetParser();
    //                TermPtr ast = parser->Parse(str);
    //                Analyze(ast, ast);
    //                if (!ast->m_module) {
    //                    ast->m_module = Term::Create(parser::token_type::END, TermID::STRCHAR, name.c_str());
    //                }
    //                ModulePtr module = std::make_shared<Module>(shared_from_this(), ast);
    //
    //                module->m_md5 = parser->m_file_md5;
    //                module->m_timestamp = parser->m_timestamp;
    //                return RegisterModule(module);
    //            }
    //        }
    //    }

    LOG_ERROR("Files name"); // '%s' not found!", search_file.c_str());

    return false;


    //    ASSERT(!"Not impelmented");
    //    std::string name(module_name ? module_name : name_str);
    //    try {
    //        m_modules.insert(std::pair<std::string, Module *>(name, new Module(name_str)));
    //        void * handle = m_modules[name]->GetHandle();
    //
    //        const size_t * func_list_count = static_cast<const size_t *> (dlsym(handle, NEWLANG_PREFIX "_func_list_count"));
    //        const char ** func_list = static_cast<const char **> (dlsym(handle, NEWLANG_PREFIX "_func_list"));
    //        m_modules[name]->m_source = static_cast<const char **> (dlsym(handle, NEWLANG_PREFIX "_module_source"));
    //        m_modules[name]->m_main = reinterpret_cast<FunctionType*> (dlsym(handle, NEWLANG_PREFIX "_main_module_func"));
    //
    //        //@todo INIT MODULE
    //
    //        ObjPtr func;
    //        if(func_list_count && func_list) {
    //            TermPtr proto;
    //            Parser parser(proto);
    //            for (size_t i = 0; i < (*func_list_count); i++) {
    //                LOG_DEBUG("Load '%s' form module '%s'.", func_list[i], name_str);
    //
    //                std::string arg_name = NEWLANG_PREFIX "_";
    //                arg_name += func_list[i];
    //                arg_name += "_text";
    //
    //                const char ** func_proto = static_cast<const char **> (dlsym(handle, arg_name.c_str()));
    //                parser.Parse(*func_proto);
    //                ObjType type;
    //                switch(proto->getTermID()) {
    //                    case TermID::SIMPLE:
    //                    case TermID::PURE_ONCE:
    //                        type = ObjType::PUREFUNC;
    //                        break;
    //                    case TermID::FUNCTION:
    //                        type = ObjType::FUNCTION;
    //                        break;
    //                    default:
    //                        LOG_RUNTIME("Function type '%s' unknown '%s'", newlang::toString(proto->getTermID()), proto->toString().c_str());
    //                }
    //                func = Obj::CreateFunc(ctx, proto->Left(), type, func_list[i]);
    //                func->m_func_ptr = reinterpret_cast<void *> (dlsym(handle, MangleName(func_list[i]).c_str()));
    //                func->m_func_source = proto;
    //
    //                //@todo call init module                
    //                if(!(func->m_func_ptr && ctx->RegisterObject(func))) {
    //                    LOG_ERROR("Fail load '%s' form module '%s'.", func_list[i], name_str);
    //                } else {
    //                    m_modules[name]->Funcs()[func_list[i]] = func;
    //                }
    //            }
    //
    //            return true;
    //        }
    //        LOG_WARNING("In module '%s' functions not found!", name_str);
    //        return true;
    //    } catch (std::runtime_error &e) {
    //
    //        LOG_ERROR("%s", e.what());
    //    }
}

bool RunTime::UnLoadModule(const char *name_str, bool deinit) {
    bool result = false;
    bool is_error = false;
    //    auto it = m_modules.begin();
    //    while(it != m_modules.end()) {
    //        if(name == nullptr || it->first.compare(name) == 0) {
    //
    //            for (auto &elem : it->second->Funcs()) {
    //                if(!ctx->RemoveObject(elem.first.c_str())) {
    //                    LOG_ERROR("Fail unregister func '%s' from module '%s'.", elem.first.c_str(), it->first.c_str());
    //                    is_error = true;
    //                }
    //            }
    //            LOG_DEBUG("UnLoad module '%s'.", it->first.c_str());
    //
    //            //@todo call deinit module
    //            delete it->second;
    //            it = m_modules.erase(it);
    //            result = true;
    //        } else {
    //
    //            it++;
    //        }
    //    }
    return result && !is_error;
}

//bool NewLang::SaveContext(const char *name) {
//    std::ofstream file;
//    file.open(name, std::ios::trunc);
//    file << "#!/bin/nlc --exec\n";
//    file << "// Global user terms\n\n";
//    std::string temp;
//    std::string str;
//    for (auto &elem : * this) {
//        temp.swap(elem.second->m_var_name);
//
//        str = elem.second->toString();
//        if(str.find("&=") == 0) {
//            str = str.substr(2);
//        }
//        if(str.find("&") == 0) {
//            str = str.substr(1);
//        }
//
//        file << "@[]." << temp << " := " << str << ";\n";
//
//        temp.swap(elem.second->m_var_name);
//    }
//
//    for (auto &elem : m_global_funcs) {
//        if((elem.second->m_func_source) && ((elem.second->m_func_source)->getTermID() == TermID::SIMPLE || (elem.second->m_func_source)->getTermID() == TermID::PURE_ONCE)) {
//            TermPtr term = (elem.second->m_func_source)->Right();
//            if(!CheckClearFunction(term)) {
//                LOG_DEBUG("The function '%s' cannot be saved because it is not clean!", elem.second->m_func_source->Left()->toString().c_str());
//            }
//            file << term->toString() << ";\n";
//        }
//    }
//    file.close();
//
//    return true;
//}
//
//bool NewLang::LoadContext(Context *ctx, const char *name) {
//
//    return RunTime::Instance()->ExecModule(name, ReplaceFileExt(name, ".ctx", ".nlm").c_str(), true, ctx)->getType() != ObjType::Error;
//}

ObjPtr RunTime::ExecModule(const char *mod, const char *output, bool cached, Context * ctx) {
    /*std::string source = ReadFile(mod);
    Obj args;
    if(cached && access(output, F_OK) != -1 && LoadModule(output, true)) {
        if(m_modules[output]->m_source && *(m_modules[output]->m_source) && source.compare(*(m_modules[output]->m_source)) == 0) {
            LOG_DEBUG("Load cached module '%s'", output);
            return m_modules[output]->Main(ctx, args);
        }
        LOG_DEBUG("Disabled cached module '%s'\nsource:\n%s\nmodule:\n%s", output, source.c_str(), m_modules[output]->m_source ? *(m_modules[output]->m_source) : "nullptr");
        UnLoadModule(ctx, output);
    }
    if(NewLang::CompileModule(module, output) && LoadModule(output)) {
        return m_modules[output]->Main(ctx, args);
    }*/
    LOG_RUNTIME("Fail compile module '%s' form file '%s'.", output, mod);
    return nullptr;
}

RuntimePtr RunTime::Init(int argc, const char** argv, const char** penv) {
    //    RuntimePtr rt = std::make_shared<RunTime>();
    //    if (!rt->ParseArgs(argc, argv)) {
    //        LOG_RUNTIME("Fail parse args!");
    //    }
    //
    //    if (rt->m_load_runtime) {
    //        VERIFY(rt->RegisterBuildin(std::make_shared<newlang::runtime::Base>(rt)));
    //        VERIFY(rt->RegisterBuildin(std::make_shared<newlang::runtime::System>(rt)));
    //    }

    return RunTime::Init(MakeMainArgs(argc, argv, penv));
}

RuntimePtr RunTime::Init(StringArray args) {
    RuntimePtr rt = std::make_shared<RunTime>();


    //__STAT_RUNTIME_UNITTEST__()

    //    VERIFY(RegisterSystemBuildin("::__compare__(obj1, obj2):Int8"));
    //    VERIFY(RegisterSystemBuildin("::__equals__(obj1, obj2):Bool"));
    // __eq__ , __ne__ , __lt__ , __gt__ , __ge__ , __le__
    //__cmp__
    //  == 	__eq__()
    //  != 	__ne__()
    //  > 	__gt__()
    //  >= 	__ge__()
    //  < 	__lt__()
    //  <= 	__le__()
    // __module__, __name__, __qualname__, __doc__ and __annotations__) and have a new __wrapped__ 

    // __getattr__ __getattribute__ __setattr__ __delattr__
    // __get__(self, instance, owner=None)  __set__

    // __call__(self[, args...])
    // __getitem__(), __setitem__(), __delitem__(), 
    // __contains__

    rt->GlobalNameBuildinRegister();

    //    nlc_prinft_sub_
    //    LLVMAddSymbol("nlc_prinft_sub_", (void *) &nlc_prinft_sub_);
    VERIFY(rt->RegisterSystemFunc("::print(format:FmtChar, ... ):Int32 ::= %nlc_prinft_sub_ ..."));

    VERIFY(rt->RegisterSystemFunc("::srand(init:Int32):None ::= %srand ..."));
    VERIFY(rt->RegisterSystemFunc("::rand():Int32 ::= %rand ..."));


    if (!rt->ParseArgs(args)) {
        LOG_RUNTIME("Fail parse args!");
    }
    if (rt->m_load_runtime) {
        VERIFY(rt->RegisterBuildin(std::make_shared<newlang::runtime::Base>(rt)));
        VERIFY(rt->RegisterBuildin(std::make_shared<newlang::runtime::System>(rt)));
    }
    return rt;
}

ParserPtr RunTime::GetParser() {
    // @todo Сделать корректныую очистку состояния парсера???
    return std::make_shared<Parser>(m_macro, nullptr, m_diag, true, shared_from_this());
}

TermPtr RunTime::MakeAst(const std::string_view src, bool skip_analize) {
    TermPtr ast = GetParser()->Parse(src.begin());
    if (skip_analize) {
        return ast;
    }

    AstAnalysis analysis(*this, m_diag.get());

    if (!analysis.Analyze(ast, ast)) {
        LOG_RUNTIME("Make AST fail!");
    }
    return ast;
}

bool RunTime::ExpandFileName(std::string &filename) {
    if (std::filesystem::exists(filename)) {
        filename = std::filesystem::absolute(filename);
        return true;
    }
    return false;
}

ObjPtr RunTime::RunFile(std::string file, Obj* args) {
    if (!ExpandFileName(file)) {
        LOG_RUNTIME("File or module '%s' not found!", file.c_str());
    }
    std::string source = ReadFile(file.c_str());
    //    TermPtr ast = MakeAst(source);
    //    if (args) {
    //        args->insert(args->begin(), {
    //            "", Obj::CreateString(file)
    //        });
    //    }
    return Run(source, args);
}

// Для теста 

extern "C" char convert(char c) {
    if (c == 'A') return 'C';
    if (c == 'C') return 'G';
    if (c == 'G') return 'T';
    if (c == 'T') return 'A';
    return ' ';
}

RunTime::RunTime() :
//m_llvm_builder(LLVMCreateBuilder()),
m_macro(std::make_shared<Macro>()),
m_diag(std::make_shared<Diag>()) {

    m_assert_enable = true;
    m_load_dsl = true;
    m_embed_source = false;
    m_import_module = true;
    m_import_native = true;
    m_eval_enable = true;
    m_load_runtime = true;
    //    m_error_limit = 10;
    m_typedef_limit = 0;

    //    m_cmd_args = Obj::CreateType(ObjType::Dictionary, ObjType::Dictionary, true);
    //    m_main_args = Obj::CreateType(ObjType::Dictionary, ObjType::Dictionary, true);

    m_main_module = std::make_shared<Module>();

    LLVMLoadLibraryPermanently(nullptr);
    LLVMAddSymbol("convert", (void *) &convert); // Для теста 


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

    if (!m_ffi_handle) {
        m_ffi_handle = LoadLibrary(utf8_decode(ffi_file).c_str());
    }
    if (!m_ffi_handle) {
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

    m_wide_char_type_ffi = m_ffi_type_uint16;
    STATIC_ASSERT(sizeof (wchar_t) == 2);

#else
    //    std::string error;
    if (LLVMLoadLibraryPermanently("libffi") == 0) {
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

    m_wide_char_type_ffi = m_ffi_type_uint32;
    STATIC_ASSERT(sizeof (wchar_t) == 4);

#endif

    if (!(m_ffi_type_uint8 && m_ffi_type_sint8 && m_ffi_type_uint16 && m_ffi_type_sint16 &&
            m_ffi_type_uint32 && m_ffi_type_sint32 && m_ffi_type_uint64 && m_ffi_type_sint64 &&
            m_ffi_type_float && m_ffi_type_double && m_ffi_type_pointer && m_ffi_type_void &&
            m_ffi_prep_cif && m_ffi_prep_cif_var && m_ffi_call)) {
        LOG_RUNTIME("Fail init data from libffi!");
    }

}

void RunTime::GlobalNameBuildinRegister() {
    VERIFY(RegisterBuildinType(ObjType::None,{}));
    VERIFY(RegisterBuildinType(ObjType::Any,{}));

    VERIFY(RegisterBuildinType(ObjType::Arithmetic,{":Any"}));
    VERIFY(RegisterBuildinType(ObjType::Rational,{":Arithmetic"}));
    VERIFY(RegisterBuildinType(ObjType::Tensor,{":Arithmetic"}));

    VERIFY(RegisterBuildinType(ObjType::Integer,{":Tensor"}));
    VERIFY(RegisterBuildinType(ObjType::Bool,{":Integer"}));
    VERIFY(RegisterBuildinType(ObjType::Int8,{":Integer"}));
    VERIFY(RegisterBuildinType(ObjType::Char,{":Integer"}));
    VERIFY(RegisterBuildinType(ObjType::Byte,{":Integer"}));
    VERIFY(RegisterBuildinType(ObjType::Int16,{":Integer"}));
    VERIFY(RegisterBuildinType(ObjType::Word,{":Integer"}));
    VERIFY(RegisterBuildinType(ObjType::Int32,{":Integer"}));
    VERIFY(RegisterBuildinType(ObjType::DWord,{":Integer"}));
    VERIFY(RegisterBuildinType(ObjType::Int64,{":Integer"}));
    VERIFY(RegisterBuildinType(ObjType::DWord64,{":Integer"}));

    VERIFY(RegisterBuildinType(ObjType::Number,{":Tensor"}));
    VERIFY(RegisterBuildinType(ObjType::Float16,{":Number"}));
    VERIFY(RegisterBuildinType(ObjType::Float32,{":Number"}));
    VERIFY(RegisterBuildinType(ObjType::Float64,{":Number"}));
    VERIFY(RegisterBuildinType(ObjType::Single,{":Number"}));
    VERIFY(RegisterBuildinType(ObjType::Double,{":Number"}));

    VERIFY(RegisterBuildinType(ObjType::Complex,{":Tensor"}));
    VERIFY(RegisterBuildinType(ObjType::Complex32,{":Complex"}));
    VERIFY(RegisterBuildinType(ObjType::Complex64,{":Complex"}));

    VERIFY(RegisterBuildinType(ObjType::String,{":Any"}));
    VERIFY(RegisterBuildinType(ObjType::StrChar,{":String"}));
    VERIFY(RegisterBuildinType(ObjType::StrWide,{":String"}));
    VERIFY(RegisterBuildinType(ObjType::FmtChar,{":StrChar"}));
    VERIFY(RegisterBuildinType(ObjType::FmtWide,{":StrWide"}));

    VERIFY(RegisterBuildinType(ObjType::Dictionary,{":Any"}));
    VERIFY(RegisterBuildinType(ObjType::Interface,{":Any"}));
    VERIFY(RegisterBuildinType(ObjType::Class,{":Dictionary", ":Interface"}));

    VERIFY(RegisterBuildinType(ObjType::Pointer,{":Any"})); // Указатели на машиннозависимую реализауию объектов

    VERIFY(RegisterBuildinType(ObjType::Plain,{":Pointer"})); // Могут быть представленые в одном блоке памяти
    VERIFY(RegisterBuildinType(ObjType::Enum,{":Plain", ":Integer"}));
    VERIFY(RegisterBuildinType(ObjType::Union,{":Plain", ":Dictionary"}));
    VERIFY(RegisterBuildinType(ObjType::Struct,{":Plain", ":Class"}));


    VERIFY(RegisterBuildinType(ObjType::Eval,{":Any"})); // Может быть выполнен
    VERIFY(RegisterBuildinType(ObjType::Function,{":Eval"}));
    VERIFY(RegisterBuildinType(ObjType::NativeFunc,{":Function", ":Pointer"}));

    VERIFY(RegisterBuildinType(ObjType::Other,{":Any"}));
    VERIFY(RegisterBuildinType(ObjType::Range,{":Other"}));
    VERIFY(RegisterBuildinType(ObjType::Ellipsis,{":Other"}));

    VERIFY(RegisterBuildinType(ObjType::BLOCK,{":Eval"}));
    VERIFY(RegisterBuildinType(ObjType::BLOCK_TRY,{":Eval"}));

    VERIFY(RegisterBuildinType(ObjType::PureFunc,{":Function"}));

    VERIFY(RegisterBuildinType(ObjType::Type,{":Any"}));
    //    VERIFY(RegisterBuildinType(ObjType::Return,{":Any"}));
    //
    //    VERIFY(RegisterBuildinType(ObjType::Error,{":Return"}));
    //    VERIFY(RegisterBuildinType(ObjType::Break,{":Return"})); // Синонимы прерывания последовательности выполнения для совместимости
    //    VERIFY(RegisterBuildinType(ObjType::Continue,{":Return"})); // со стндартными алгоритмическими приемами (синтаксический сахар)
    //
    //
    //    VERIFY(RegisterBuildinType(ObjType::ErrorParser,{":Error"}));
    //    VERIFY(RegisterBuildinType(ObjType::ErrorRunTime,{":Error"}));
    //    VERIFY(RegisterBuildinType(ObjType::ErrorSignal,{":Error"}));

}

bool RunTime::RegisterSystemFunc(const char *source) {

    TermPtr ast = MakeAst(source, false);
    TermPtr term = ast->Left();
    ASSERT(term);
    ASSERT(!term->m_int_name.empty());

    ASSERT(ast->Right());
    ASSERT(ast->Right()->m_id == TermID::NATIVE);

    term->m_obj = CreateNative(term, nullptr, false, ast->Right()->m_text.substr(1).c_str());
    ASSERT(term->m_obj);

    auto base = m_buildin_obj.find(term->m_int_name);
    if (base != m_buildin_obj.end()) {
        LOG_ERROR("Fail register buildin name '%s'", term->m_int_name.c_str());
        return false;
    }
    return !!NameRegister(true, term->m_int_name.c_str(), term, term->m_obj);
}

bool RunTime::RegisterBuildinType(ObjType type, std::vector<std::string> parents) {

    std::string type_name(toString(type));

    std::string func_name = NormalizeName(type_name.substr(1));
    func_name.insert(0, "::");
    type_name = NormalizeName(type_name);

    auto base = m_buildin_obj.find(type_name);
    if (base != m_buildin_obj.end()) {
        return false;
    }

    ObjPtr result = Obj::CreateBaseType(type);
    ASSERT(result->m_var_type_fixed == type);
    ASSERT(result->m_var_type_current == ObjType::Type);
    ASSERT(!type_name.empty() && result->m_class_name.compare(type_name) == 0);
    ASSERT(result->m_class_parents.empty());

    ASSERT(std::holds_alternative<void *>(result->m_var));
    ASSERT(std::get<void *>(result->m_var));

    //    result->m_var = (void *) &Context::__make_type__;

    for (auto &parent : parents) {
        auto iter = m_buildin_obj.find(NormalizeName(parent));
        if (iter == m_buildin_obj.end()) {
            LOG_DEBUG("Parent type '%s' as '%s' not found!", parent.c_str(), NormalizeName(parent).c_str());
            return false;
        }
        for (auto &elem : result->m_class_parents) {
            ASSERT(elem);
            elem->m_class_name = newlang::NormalizeName(elem->m_class_name);
            if (!elem->m_class_name.empty() && elem->m_class_name.compare(NormalizeName(parent)) == 0) {
                LOG_DEBUG("The type '%s' already exists in the parents of '%s'!", parent.c_str(), type_name.c_str());
                return false;
            }
        }
        ASSERT(iter->first.compare(NormalizeName(parent)) == 0);
        result->m_class_parents.push_back(iter->second);
    }

    m_buildin_obj[type_name] = result;

    TermPtr type_term = Term::Create(parser::token_type::NAME, TermID::FUNCTION, type_name.c_str());
    type_term->m_int_name = type_name;
    type_term->m_obj = result;

    if (type_name.compare("None:::") == 0) {
        VERIFY(NameRegister(true, "_", type_term, result));
    }
    return NameRegister(true, type_name.c_str(), type_term, result) && NameRegister(true, type_name.insert(0, "::").c_str(), type_term, result);
}

ObjPtr RunTime::GetTypeFromString(const std::string_view type, bool *has_error) {
    if (type.empty()) {
        if (has_error) {
            *has_error = true;
            return Obj::CreateNone();
        }
        LOG_RUNTIME("Type name '%s' not found!", type.begin());
    }

    auto result_types = m_buildin_obj.find(NormalizeName(type));
    if (result_types != m_buildin_obj.end()) {
        return result_types->second;
    }

    ObjPtr found = NameGet(type.begin(), false);
    if (found) {
        return found;
    }

    //    auto result_find = find(type);
    //    if (result_find != end()) {
    //        return result_find->second.lock();
    //    }

    if (has_error) {
        *has_error = true;
        return nullptr;
    }
    LOG_RUNTIME("Type name '%s' not found!", type.begin());
}

ObjType RunTime::BaseTypeFromString(RunTime * rt, const std::string_view type_name, bool *has_error) {

    bool local_has_error = false;

    ObjType type = GetBaseTypeFromString(type_name, &local_has_error);

    if (!local_has_error) {
        return type;
    }

    if (has_error) {
        *has_error = local_has_error;
    }

    if (local_has_error && !rt) {
        LOG_RUNTIME("Type name '%s' is not calculated statically!", type_name.begin());
    }

    ObjPtr obj_type = rt->GetTypeFromString(type_name, has_error);

    if (obj_type == nullptr) {
        if (has_error) {
            *has_error = true;
            return ObjType::None;
        }
        LOG_RUNTIME("Type name '%s' not found!", type_name.begin());
    }
    return obj_type->m_var_type_fixed;
}

bool RunTime::RegisterBuildin(BuildinPtr module) {

    ASSERT(module);
    if (RegisterModule(module)) {
        return module->RegisterMacros(m_macro);
    }
    return false;
}

bool RunTime::RegisterModule(ModulePtr module) {
    ASSERT(module);

    //    if (module->m_var_type_current != ObjType::Module) {
    //        LOG_RUNTIME("Object not a module '%s'!", module->toString().c_str());
    //        return false;
    //    }

    std::string name(module->m_file);
    //    LOG_DEBUG("Load '%s'", name.c_str());
    //    name.append(module->m_class_name);

    if (m_modules.find(name) != m_modules.end()) {
        LOG_ERROR("Module name '%s' already register!", name.c_str());
        return false;
    }

    for (auto &obj : *module) {
        if (isStaticName(obj.first)) {
            if (!NameRegister(true, obj.first.c_str(), obj.second.item, obj.second.obj)) {
                LOG_ERROR("Fail register object '%s'!", obj.first.c_str());
                return false;
            }
        }
    }

    m_modules[name] = module;

    return true;
}

std::vector<std::wstring> RunTime::SelectPredict(std::string start, size_t overage_count) {

    std::vector<std::wstring> result;

    bool find_local = false;
    bool find_global = false;
    bool find_types = false;
    bool find_macro = false;

    std::string prefix;

    if (isModuleName(start)) {
        prefix = start[0];
        start = start.substr(1);
        find_global = true;
    } else if (isLocalName(start)) {
        prefix = start[0];
        start = start.substr(1);
        find_local = true;
    } else if (isMacroName(start)) {
        find_macro = true;
    } else if (isTypeName(start)) {
        find_types = true;
    } else {
        find_local = true;
        find_global = true;
        find_types = true;
        find_macro = true;
    }


    if (find_macro) {
        for (auto &elem : * this) {
            if (pred_compare(start, elem.first)) {
                result.push_back(utf8_decode(prefix + elem.first));
                if (result.size() > overage_count + 1) {
                    break;
                }
            }
        }
    }

    if (find_local) {
        //        for (int i = 0; i < size(); i++) {
        //            if (pred_compare(start, at(i).first)) {
        //                ObjPtr object = at(i).second.lock();
        //                if (object && object->is_function_type()) {
        //                    result.push_back(utf8_decode(prefix + at(i).first) + L"(");
        //                } else if (object) {
        //                    result.push_back(utf8_decode(prefix + at(i).first));
        //                }
        //                if (result.size() > overage_count + 1) {
        //                    break;
        //                }
        //            }
        //        }
    }

    if (find_global) {
        //        for (int i = 0; i < m_terms->size(); i++) {
        //            if (pred_compare(start, m_terms->at(i).first)) {
        //                if (m_terms->at(i).second->is_function_type()) {
        //                    result.push_back(utf8_decode(prefix + m_terms->at(i).first) + L"(");
        //                } else {
        //                    result.push_back(utf8_decode(prefix + m_terms->at(i).first));
        //                }
        //                if (result.size() > overage_count + 1) {
        //                    break;
        //                }
        //            }
        //        }

        //        for (auto &elem : m_funcs) {
        //
        //            if (pred_compare(start, elem.first)) {
        //                result.push_back(utf8_decode(prefix + elem.first) + L"(");
        //                if (result.size() > overage_count + 1) {
        //                    break;
        //                }
        //            }
        //        }
    }

    if (find_types) {
        for (auto &elem : m_buildin_obj) {
            if (pred_compare(start, elem.first)) {
                result.push_back(utf8_decode(elem.first));
                if (result.size() > overage_count + 1) {
                    break;
                }
            }
        }
    }
    return result;

}

bool RunTime::RegisterSystemObj(ObjPtr obj) {

    ASSERT(obj);

    //    if (IsModule(obj)) {
    //
    //        //    11111111111111
    //        //    
    //    }

    LOG_RUNTIME("RegisterSystemObj '%s' !!!!!!!!!!!!!!!!!!!!!!!!!!!!!", obj->toString().c_str());

    //    ASSERT(obj->m_prototype);

    //    std::string obj_name = obj->m_prototype->getText();
    std::string obj_name = obj->m_class_name; //m_prototype->getText();
    auto found = find(obj_name);
    if (found != end()) {
        LOG_ERROR("Object name '%s' already exist!", obj_name.c_str());
        return false;
    }

    m_sys_obj.push_back(obj);
    //    push_back(std::make_shared<GlobNameInfo>(obj->m_prototype, obj), obj_name);

    //    std::string name;
    //    for (auto &elem : *obj) {
    //        name = obj_name;
    //        name.append("::");
    //        name.append(elem.second->m_prototype->getText());
    //
    //        //        LOG_DEBUG("%s    ->    %s", name.c_str(), elem.second->m_prototype->toString().c_str());
    //
    //        auto found = find(name);
    //        if (found != end()) {
    //            LOG_ERROR("Object name '%s' already exist!", name.c_str());
    //            return false;
    //        }
    //
    //        push_back(std::make_shared<GlobNameInfo>(elem.second->m_prototype, elem.second), name);
    //
    //
    //    }


    return true;
}

ObjPtr RunTime::CreateFunction(TermPtr proto, TermPtr block) {
    ObjPtr result = Obj::CreateType(ObjType::Function, ObjType::Function, true);
    *const_cast<TermPtr *> (&result->m_prototype) = proto;
    result->m_sequence = block;
    return result;
}

ObjPtr RunTime::CreateFunction(TermPtr proto, void * addr) {
    ObjPtr result = Obj::CreateType(ObjType::Function, ObjType::Function, true);
    *const_cast<TermPtr *> (&result->m_prototype) = proto;
    result->m_var = addr;
    return result;
}

ObjPtr RunTime::CreateNative(const char *proto, const char *module, bool lazzy, const char *mangle_name) {
    TermPtr term;
    try {
        // Термин или термин + тип парсятся без ошибок
        term = Parser::ParseString(proto, nullptr); //m_named);
    } catch (std::exception &) {
        try {
            std::string func(proto);
            func += ":={}";
            term = Parser::ParseString(func, nullptr)->Left(); //m_named)->Left();
        } catch (std::exception &e) {

            LOG_RUNTIME("Fail parsing prototype '%s'!", e.what());
        }
    }
    return CreateNative(term, module, lazzy, mangle_name);
}

ObjPtr RunTime::CreateNative(TermPtr proto, const char *module, bool lazzy, const char *mangle_name) {
    ASSERT(!lazzy);

    void *addr = GetNativeAddr(mangle_name ? mangle_name : proto->m_text.c_str());
    if (!addr) {
        NL_CHECK(addr, "Error getting address '%s' from '%s'!", proto->toString().c_str(), module);
    }

    return CreateNative(proto, addr);
}

ObjPtr RunTime::CreateNative(TermPtr proto, void *addr) {

    NL_CHECK(proto, "Fail prototype native function!");
    //    NL_CHECK((module == nullptr || (module && *module == '\0')) || m_runtime,
    //            "You cannot load a module '%s' without access to the runtime context!", module);

    ObjPtr result;
    ObjType type = ObjType::None;
    if (proto->isCall()) {
        type = ObjType::NativeFunc;

        ObjType type_test;
        for (int i = 0; i < proto->size(); i++) {

            //            if (!proto->name(i).empty()) {
            //                NL_PARSER((*proto)[i].second, "Default values '%s' are not supported!", proto->name(i).c_str());
            //            }
            //
            //            if (isLocalAnyName((*proto)[i].second->m_text.c_str())) {
            //                NL_PARSER((*proto)[i].second, "Argument name expected '%s'!", (*proto)[i].second->m_text.c_str());
            //            }

            if ((*proto)[i].second->getTermID() == TermID::ELLIPSIS) {
                if (i + 1 != proto->size()) {
                    NL_PARSER((*proto)[i].second, "Ellipsys must end of argument!");
                }
            } else {
                if (!(*proto)[i].second->m_type) {
                    NL_PARSER((*proto)[i].second, "Argument type must be specified!");
                }

                type_test = GetBaseTypeFromString((*proto)[i].second->m_type->m_text.c_str());
                if (!isNativeType(type_test)) {
                    NL_PARSER((*proto)[i].second->GetType(), "Argument must be machine type! Creating a variable with type '%s' is not supported!", (*proto)[i].second->m_type->m_text.c_str());
                }
            }
        }


    } else {
        if (!proto->m_type) {
            NL_PARSER(proto, "Cannot create native variable without specifying the type!");
        }

        type = GetBaseTypeFromString(proto->m_type->m_text); //, this);
        if (!isNativeType(type)) {
            NL_PARSER(proto, "Creating a variable with type '%s' is not supported!", proto->m_type->m_text.c_str());
        }
    }

    result = Obj::CreateType(type);
    result->m_var_type_fixed = ObjType::None; // Тип определен и не может измениться в дальнейшем

    *const_cast<TermPtr *> (&result->m_prototype) = proto;
    //    result->m_func_abi = abi;

    //    if (mangle_name) {
    //        result->m_func_mangle_name = mangle_name;
    //    }
    //    if (module) {
    //        result->m_module_name = module;
    //    }
    void * ptr = nullptr;
    //    if (lazzy) {
    //        result->m_var = static_cast<void *> (nullptr);
    //    } else {
    ASSERT(std::holds_alternative<std::monostate>(result->m_var));

    ptr = addr; //GetNativeAddr(result->m_func_mangle_name.empty() ? proto->m_text.c_str() : result->m_func_mangle_name.c_str(), module);

    switch (type) {
        case ObjType::Bool:
            result->m_var = static_cast<bool *> (ptr);
            result->m_var_is_init = true;
            break;
        case ObjType::Int8:
        case ObjType::Char:
        case ObjType::Byte:
            result->m_var = static_cast<int8_t *> (ptr);
            result->m_var_is_init = true;
            break;
        case ObjType::Int16:
        case ObjType::Word:
            result->m_var = static_cast<int16_t *> (ptr);
            result->m_var_is_init = true;
            break;
        case ObjType::Int32:
        case ObjType::DWord:
            result->m_var = static_cast<int32_t *> (ptr);
            result->m_var_is_init = true;
            break;
        case ObjType::Int64:
        case ObjType::DWord64:
            result->m_var = static_cast<int64_t *> (ptr);
            result->m_var_is_init = true;
            break;
        case ObjType::Float32:
        case ObjType::Single:
            result->m_var = static_cast<float *> (ptr);
            result->m_var_is_init = true;
            break;
        case ObjType::Float64:
        case ObjType::Double:
            result->m_var = static_cast<double *> (ptr);
            result->m_var_is_init = true;
            break;

        case ObjType::NativeFunc:
        default:
            result->m_var = ptr;
            //        }
            //        result->m_var = m_runtime->GetNativeAddr(
            //                result->m_func_mangle_name.empty() ? proto->m_text.c_str() : result->m_func_mangle_name.c_str(), module);

            if (result->is_function_type() || type == ObjType::Pointer) {
                NL_CHECK(std::get<void *>(result->m_var), "Error getting address '%s'!", proto->toString().c_str());
            } else if (ptr && result->is_tensor_type()) {
                //            result->m_tensor = torch::from_blob(std::get<void *>(result->m_var),{
                //            }, toTorchType(type));
                result->m_var_is_init = true;
            } else {
                NL_PARSER(proto, "Fail CreateNative object");
            }
    }
    result->m_var_is_init = true;
    return result;
}

std::string newlang::GetFileExt(const char *str) {
    std::string filename(str);
    std::string::size_type idx = filename.rfind('.');
    if (idx != std::string::npos) {

        return filename.substr(idx);
    }
    return std::string("");
}

std::string newlang::AddDefaultFileExt(const char *str, const char *ext_default) {
    std::string filename(str);
    std::string file_ext = GetFileExt(str);
    if (file_ext.empty() && !filename.empty() && filename.compare(".") != 0) {

        filename.append(ext_default);
    }
    return filename;
}

std::string newlang::ReplaceFileExt(const char *str, const char *ext_old, const char *ext_new) {
    std::string filename(str);
    std::string file_ext = GetFileExt(str);
    if (file_ext.compare(ext_old) == 0) {
        filename = filename.substr(0, filename.length() - file_ext.length());
    }
    file_ext = GetFileExt(filename.c_str());
    if (file_ext.compare(".") != 0 && file_ext.compare(ext_new) != 0 && !filename.empty() &&
            filename.compare(".") != 0) {

        filename.append(ext_new);
    }
    return filename;
}

std::string newlang::ReadFile(const char *fileName) {
    std::ifstream f(fileName);
    std::stringstream ss;
    ss << f.rdbuf();
    //    if(f.fail()) {
    //        std::cout << "Current path is " << std::filesystem::current_path() << '\n';
    //        std::cerr << strerror(errno);
    //    }
    f.close();

    return ss.str();
}

ObjPtr RunTime::OpLoadModule(TermPtr term) {
    ASSERT(isModuleName(term->m_text));
    ASSERT(term->isCall());


    ASSERT(isModuleName(term->m_text));
    std::string name = ExtractModuleName(term->m_text.c_str());
    for (auto &elem : m_module_loader) {
        if (elem.compare(name) == 0) {
            NL_PARSER(term, "Load module '%s' recursion!", term->m_text.c_str());
        }
    }
    m_module_loader.push_back(name);




    //    if (term->size()) {
    //        NL_PARSER(term->at(0).second, "Load module from options not implemented!");
    //    }
    //
    //    if(CheckLoadModule(term)){
    //        NL_PARSER(term->at(0).second, "Load module from options not implemented!");
    //    }
    //    
    //    std::string name = ExtractModuleName(term->m_text.c_str());
    //    if (m_modules.find(name) == m_modules.end()) {
    //
    //        ModulePtr module = LoadModuleFromFile(str.c_str(), true);
    //        if (module) {
    //            m_modules[name] = std::move(module);
    //        }
    //    }
    //    return m_modules.find(name) != m_modules.end();

    return nullptr;
}

ModulePtr RunTime::CheckLoadModule(TermPtr &term) {
    ASSERT(isModuleName(term->m_text));
    std::string name = ExtractModuleName(term->m_text.c_str());

    if (m_modules.find(name) == m_modules.end()) {
        if (!LoadModuleFromFile(name.c_str(), true)) {
            NL_PARSER(term, "Load module '%s' fail!", term->m_text.c_str());
        }
    }
    return m_modules[name];
}

bool RunTime::ModuleCreate(FileModule &data, const std::string_view source) {

    TermPtr ast = MakeAst(source);
    return true;
}

bool RunTime::ModuleCreate(FileModule &data, const std::string_view module_name, const TermPtr &include, const std::string_view source, llvm::Module *bc) {

    data.name.assign(module_name);
    data.include = AstAnalysis::MakeInclude(include);
    data.source = source;
    if (bc) {
        llvm::raw_string_ostream code(data.bytecode);
        llvm::WriteBitcodeToFile(*bc, code);
    }
    return !data.name.empty() && !data.include.empty() && (data.source.empty() || data.bytecode.empty());
}

static void zip_save(zip_t *zipper, const std::string_view file, const std::string &data) {
    zip_error_t error;
    zip_source_t *zip_data = nullptr;
    if ((zip_data = zip_source_buffer_create(data.data(), data.size(), ZIP_CREATE | ZIP_TRUNCATE, &error)) == NULL) {
        LOG_RUNTIME("can't create source: %s", zip_error_strerror(&error));
    }

    if (zip_file_add(zipper, file.begin(), zip_data, ZIP_FL_ENC_UTF_8) < 0) {
        zip_source_free(zip_data);
        LOG_RUNTIME("Failed to add file '%s' to zip: %s", file.begin(), zip_strerror(zipper));
    }
    //    zip_source_commit_write(zip_data);
    //    zip_source_free(zip_data);
    zip_source_keep(zip_data);
}

bool RunTime::ModuleSave(const FileModule &data, const std::string_view filepath, const std::string_view name) {
    int errorp;
    zip_t *zipper = zip_open(filepath.begin(), ZIP_CREATE | ZIP_EXCL, &errorp);
    if (zipper == nullptr) {
        zip_error_t ziperror;
        zip_error_init_with_code(&ziperror, errorp);
        LOG_RUNTIME("Failed to open output file '%s': %s", filepath.begin(), zip_error_strerror(&ziperror));
    }

    std::string module_name(name);
    if (module_name.empty()) {
        module_name = RemoveFileExtension(BaseFileName(filepath));
    }

    std::string temp_name;
    temp_name = module_name;
    temp_name += ".inc";

    zip_save(zipper, temp_name, data.include);

    if (!data.source.empty()) {
        temp_name = module_name;
        temp_name += ".src";
        zip_save(zipper, temp_name, data.source);
    }


    if (!data.bytecode.empty()) {
        temp_name = module_name;
        temp_name += ".bc";
        zip_save(zipper, temp_name, data.bytecode);
    }

    zip_close(zipper);
    return true;
}

static bool zip_read(zip* z, const std::string_view name, std::string &data) {
    //Search for the file of given name
    struct zip_stat st;
    zip_stat_init(&st);
    zip_stat(z, name.begin(), 0, &st);

    if (!st.valid) {
        return false;
    }

    data.resize(st.size);

    //Read the compressed file
    zip_file* f = zip_fopen(z, name.begin(), 0);
    zip_fread(f, data.data(), st.size);
    zip_fclose(f);

    return true;
}

bool RunTime::ModuleRead(FileModule &data, const std::string_view filepath, const std::string_view modulename) {
    data.bytecode.clear();
    data.include.clear();
    data.source.clear();
    //    data.name.clear();

    int errorp;
    zip* z = zip_open(filepath.begin(), 0, &errorp);
    if (z == nullptr) {
        zip_error_t ziperror;
        zip_error_init_with_code(&ziperror, errorp);
        LOG_RUNTIME("Failed to open file '%s': %s", filepath.begin(), zip_error_strerror(&ziperror));
    }

    data.name.assign(modulename);
    if (data.name.empty()) {
        data.name = RemoveFileExtension(BaseFileName(filepath));
    }

    std::string temp_name = data.name;

    temp_name = data.name;
    temp_name += ".inc";

    if (!zip_read(z, temp_name, data.include)) {
        LOG_RUNTIME("Name '%s' in file module '%s' not exist!", temp_name.c_str(), filepath.begin());
    }

    temp_name = data.name;
    temp_name += ".src";

    if (!zip_read(z, temp_name, data.source)) {
        LOG_INFO("Name '%s' in file module '%s' not exist!", temp_name.c_str(), filepath.begin());
    }

    temp_name = data.name;
    temp_name += ".bc";

    if (!zip_read(z, temp_name, data.bytecode)) {
        LOG_INFO("Name '%s' in file module '%s' not exist!", temp_name.c_str(), filepath.begin());
    }

    if (data.bytecode.empty() && data.source.empty()) {
        LOG_RUNTIME("In file module '%s' not source and bytecode not exist!", filepath.begin());
    }

    //And close the archive
    zip_close(z);

    return true;
}

/*
 * Построчное выполнение не изменяет AST, а только добавляет к нему новые строки.
 * Констекст выполнения (m_main_runner) тоже остается не изменным.
 */
ObjPtr RunTime::Run(const std::string_view str, Obj* args) {
    if (!m_main_ast || m_main_ast->m_id != TermID::BLOCK) {
        m_main_ast = Term::Create(parser::token_type::END, TermID::BLOCK, "");
        m_main_runner.reset();
    }

    TermPtr ast = GetParser()->Parse(str.begin());

    m_main_ast->m_block.push_back(ast);

    if (!m_main_runner) {
        m_main_runner = std::make_shared<Context>(m_main_ast->m_int_vars, shared_from_this());
    }
    try {
        m_diag->m_error_count = 0;

        AstAnalysis analysis(*this, m_diag.get());

        analysis.CheckError(analysis.Analyze(ast, m_main_ast));
        //        AstCheckError(AstAnalyze(ast, m_main_ast));
        if (m_diag->m_error_count) {
            LOG_PARSER("fatal error: %d generated. ", m_diag->m_error_count);
        }

        return Context::Run(m_main_ast->m_block.back(), m_main_runner.get());
    } catch (...) {
        m_main_ast->m_block.pop_back();
        throw;
    }
}

/*
 * Выполенение целого AST создает полностью новый контекст выполнения.
 */
ObjPtr RunTime::Run(TermPtr ast, Obj* args) {
    m_main_ast = ast;
    m_main_runner = std::make_shared<Context>(m_main_ast->m_int_vars, shared_from_this());
    return Context::Run(m_main_ast, m_main_runner.get());
}

std::string RunTime::Escape(const std::string_view str) {
    std::string result;
    for (auto &c : str) {
        if (c == '\n') {
            result += "\\n";
        } else if (c == '\t') {
            result += "\\t";
        } else if (c == '\r') {
            result += "\\r";
        } else {
            result += c;
        }
    }
    return result;
}

std::string RunTime::NativeNameMangling(const Term *term, RunTime *rt) {
    ASSERT(term);
    //    ASSERT(isNativeName(term->m_text));
    //    if (!isStaticName(term->m_text)) {
    //        // extern "C"
    //        return term->m_text.substr(1);
    //    } else {
    //        // C++ mangling
    //        ObjType type_var = typeFromString(term->m_type, rt);
    //        if (!isNativeType(type_var)) {
    //            NL_PARSER(term->m_type, "Type name '%s' not native type!", term->m_type->m_text.c_str());
    //        }
    //        if (term->isCall()) {
    //            // Function
    //            for (auto &arg : *term) {
    //                ObjType type_arg = typeFromString(arg->m_type, rt);
    //                if (!isNativeType(type_arg)) {
    //                    NL_PARSER(arg->m_type, "Type name '%s' not native type!", arg->m_type->m_text.c_str());
    //                }
    //
    //            }
    //
    //        } else {
    //            // Variable, Struct, Union, Enum
    //        }
    //    }
    return "";
}

std::string RunTime::NativeNameMangling(std::string_view name) {
    ASSERT(isNativeName(name));
    return std::string(name.begin() + 1);
}

ObjType newlang::typeFromString(TermPtr &term, RunTime *rt, bool *has_error) {
    if (term) {
        return RunTime::BaseTypeFromString(rt, term->m_text, has_error);
    }
    return RunTime::BaseTypeFromString(rt, ":Any", has_error);
}

std::string newlang::GetDoc(std::string name) {
    return "Help system not implemented!!!!!";
}

std::string newlang::MakeConstructorName(std::string name) {
    ASSERT(isTypeName(name));
    std::string result(name.substr(1));
    result += ":";
    result += name.substr(name.rfind(":"));
    return result;
}

std::string newlang::MakeLocalName(std::string name) {
    return MangleName(MakeName(name).c_str());
}


#if defined(__x86_64__) || defined(_M_X64) || defined(__amd64__) || defined(__amd64)
ObjType RunTime::m_integer_type = ObjType::Int64;
STATIC_ASSERT(sizeof (size_t) == 8);
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
ObjType RunTime::m_integer_type = ObjType::Int32;
STATIC_ASSERT(sizeof (size_t) == 4);
#else
#error Target architecture not defined!
#endif
