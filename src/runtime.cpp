#include "pch.h"
#include "system.h"

#include <term.h>
#include <runtime.h>
#include <builtin.h>
#include <system.h>

using namespace newlang;

const char * RunTime::default_argv[RunTime::default_argc] = {"", "-nlc-no-runtime", "-nlc-no-dsl"};
const TermPtr NsStack::NonameBlock = Term::Create(parser::token_type::END, TermID::NAMESPACE, "_");

ObjType newlang::typeFromString(const std::string type_arg, RuntimePtr rt, bool *has_error) {

    std::string type(type_arg);

    if (type.find("~") != std::string::npos) {
        type.erase(std::remove(type.begin(), type.end(), '~'), type.end());
    }

    if (rt) {
        return rt->BaseTypeFromString(type, has_error);
    }

#define DEFINE_CASE(name, _)                    \
    else if (type.compare(":"#name) == 0) {     \
        return ObjType:: name;                  \
    }

    if (type.empty()) {
        return ObjType::None;
    } else if (type.compare("_") == 0) {
        return ObjType::None;
    }
    NL_TYPES(DEFINE_CASE)
#undef DEFINE_CASE

    if (has_error) {
        *has_error = true;

        return ObjType::None;
    }
    LOG_RUNTIME("Undefined type name '%s'!", type.c_str());
}

/*
 * 
 * 
 * 
 * 
 * 
 * 
 */
TermPtr RunTime::GlobFindProto(const char *name) {
    GlobItem *glob = NameFind(name);
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

    //    if (!at::holds_alternative<ObjWeak>(found->second.obj)) {

    //        if (at::holds_alternative<ObjWeak>(found->second.obj)) {
    //            return at::get<ObjWeak>(found->second.obj).lock();
    //        }
    //        ASSERT(at::holds_alternative<std::vector < ObjWeak >> (found->second.obj));
    //        return at::get<std::vector < ObjWeak >> (found->second.obj)[0].lock();

    //        return at::get<std::vector < ObjWeak >> (found->second.obj)[0];

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

//bool RunTime::NameRegister(const char * glob_name, TermPtr proto, WeakItem obj) {
//
//    ASSERT(proto);
//
//    if (isMacroName(glob_name)) {
//        NL_PARSER(proto, "Macro name not allowed! '%s'", glob_name);
//    }
//
//    if (NameFind(glob_name)) {
//        NL_MESSAGE(LOG_LEVEL_INFO, proto, "Name '%s' already exist!", glob_name);
//        return false;
//    }
//
//    insert(std::pair(glob_name, GlobItem({proto, obj})));
//
//    return true;
//}

/*
 * 
 * 
 * 
 */

TermPtr NameList::NameFind(const char* name) {

    NameList::iterator found;

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
            return found->second.proto;
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

    //    if (!at::holds_alternative<ObjWeak>(found->second.obj)) {

    //        if (at::holds_alternative<ObjWeak>(found->second.obj)) {
    //            return at::get<ObjWeak>(found->second.obj).lock();
    //        }
    //        ASSERT(at::holds_alternative<std::vector < ObjWeak >> (found->second.obj));
    //        return at::get<std::vector < ObjWeak >> (found->second.obj)[0].lock();

    //        return at::get<std::vector < ObjWeak >> (found->second.obj)[0];

    //    } else {
    //        NL_PARSER(found->second.proto, "Global name not implemented! '%s'", found->first.c_str());
    //    }
    return found->second.proto;
}

TermPtr NameList::NameRegister(bool new_only, const char *name, TermPtr proto) {

    TermPtr item = NameFind(name);

    if (new_only && item) {
        NL_MESSAGE(LOG_LEVEL_INFO, proto, "Name '%s' already exist!", name);
        return nullptr;
    } else {
        if (item) {
            if (!Term::CheckTermEq(item, proto)) {
                ASSERT(item);
                NL_MESSAGE(LOG_LEVEL_INFO, proto, "The prototype '%s' differs from the first definition '%s'!",
                        name, item->asTypeString().c_str());
                return nullptr;
            }
        } else {
            insert(std::pair(name, NameItem({proto, nullptr})));
        }
    }
    return at(name).proto;
}

//TermPtr NameList::NameRegister(const char * glob_name, TermPtr proto){
//
//    ASSERT(proto);
//
//    if (isMacroName(glob_name)) {
//        NL_PARSER(proto, "Macro name not allowed! '%s'", glob_name);
//    }
//
//    if (NameFind(glob_name)) {
//        NL_MESSAGE(LOG_LEVEL_INFO, proto, "Name '%s' already exist!", glob_name);
//        return false;
//    }
//
//    insert(std::pair(glob_name, NameItem({proto, nullptr})));
//
//    return at(glob_name).item;
//}

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
        if (at::holds_alternative<ObjWeak>(ret->obj)) {
            return at::get<ObjWeak>(ret->obj).lock();
        } else if (at::holds_alternative<std::vector < ObjWeak >> (ret->obj)) {
            return at::get<std::vector < ObjWeak >> (ret->obj)[0].lock();
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

std::string RunTime::Dump() {
    std::string result;

    for (auto &elem : * this) {
        result += '\n';
        result += elem.first;
    }

    return result;
}

ObjPtr RunTime::EvalStatic(const TermPtr term, bool pure) {
    if (term->IsLiteral()) {

        return Context::CreateRVal(nullptr, term);

    } else if (term->getTermID() == TermID::NATIVE) {

        //@todo Add check mangled name for C++ lang
        return Obj::CreateValue(reinterpret_cast<int64_t> (RunTime::GetNativeAddr(&term->m_text[1], nullptr)), ObjType::Int64);

    } else if (term->getTermID() == TermID::OPERATOR) {

        ASSERT(term->Left());
        ASSERT(term->Right());

        ObjPtr left = Context::CreateRVal(nullptr, term->Left());
        ObjPtr right = Context::CreateRVal(nullptr, term->Right());

        if (term->m_text.compare("==") == 0) {
            return Obj::CreateBool(left->op_equal(right));
        } else if (term->m_text.compare("===") == 0) {
            return Obj::CreateBool(left->op_accurate(right));
        } else if (term->m_text.compare("<") == 0) {
            return Obj::CreateBool(*left < *right);
        } else if (term->m_text.compare("<=") == 0) {
            return Obj::CreateBool(*left <= *right);
        } else if (term->m_text.compare(">") == 0) {
            return Obj::CreateBool(*left > *right);
        } else if (term->m_text.compare(">=") == 0) {
            return Obj::CreateBool(*left >= *right);
        } else if (term->m_text.compare("!=") == 0) {
            return Obj::CreateBool(!left->op_equal(right));
        } else if (term->m_text.compare("!==") == 0) {
            return Obj::CreateBool(!left->op_accurate(right));
        }
        NL_PARSER(term, "Operator '%s' not implemented in static calculate!", term->m_text.c_str());
    }
    NL_PARSER(term, "Expression '%s' not static calculate!", term->toString().c_str());
}

bool RunTime::LoadModuleFromFile(const char *term, bool init) {
    ASSERT(term);

    std::string name = ExtractModuleName(term);
    bool is_root = (name.find("\\\\") == 0);
    if (is_root) {
        LOG_WARNING("Load module from root not implemented! '%s'", name.c_str());
        name = name.substr(1); // remove root marker (double back slash) i.e. \\root\dir\module
    }
    std::vector<std::string> split = Macro::SplitString(name.substr(1).c_str(), "\\");

    std::string path;
    for (auto &elem : split) {
        if (!path.empty()) {
            path += llvm::sys::path::get_separator();
        }
        path += elem;
    }

    path += ".nlp";


    llvm::SmallString<1024> search_file;
    if (llvm::sys::path::is_relative(path)) {
        for (int i = 0; i < m_search_dir.size(); i++) {

            search_file = m_search_dir[i];
            llvm::sys::path::append(search_file, path);

            std::string full_path = search_file.c_str();
            if (llvm::sys::path::is_relative(full_path)) {
                full_path.insert(0, llvm::sys::path::get_separator().str());
                full_path.insert(0, m_work_dir);
            }

            LOG_TEST("Check '%s' in file %s", name.c_str(), full_path.c_str());

            if (llvm::sys::fs::exists(full_path)) {
                LOG_TEST("Module '%s' load from file '%s'!", term, full_path.c_str());

                std::string str = ReadFile(full_path.c_str());

                ParserPtr parser = GetParser();
                TermPtr ast = parser->Parse(str);
                AstAnalyze(ast, false);
                if (!ast->m_module) {
                    ast->m_module = Term::Create(parser::token_type::END, TermID::STRCHAR, name.c_str());
                }
                ModulePtr module = std::make_shared<Module>(shared_from_this(), ast);

                module->m_md5 = parser->m_file_md5;
                module->m_timestamp = parser->m_timestamp;
                return RegisterModule(module);
            }
        }
    }

    LOG_ERROR("Files name '%s' not found!", search_file.c_str());

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

bool CheckClearFunction(TermPtr term) {
    if (term && term->getTermID() == TermID::SOURCE) {
        return false;
    } else if (term && term->getTermID() == TermID::BLOCK) {
        for (int i = 0; i < (int) term->m_block.size(); i++) {
            if (!CheckClearFunction(term->m_block[i])) {
                return false;
            }
        }
    } else if (term && term->Right()) {
        TermPtr temp = term->Right();
        while (temp) {
            if (term->getTermID() == TermID::SOURCE) {
                return false;
            }
            if (term->Right()) {
                temp = term->Right();
            } else {

                break;
            }
        }
    }
    return true;
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

RuntimePtr RunTime::Init(std::vector<const char *> args, bool ignore_error) {
    return RunTime::Init(args.size(), args.data(), ignore_error);
}

RuntimePtr RunTime::Init(int argc, const char** argv, bool ignore_error) {
    RuntimePtr rt = std::make_shared<RunTime>();
    if (!rt->ParseArgs(argc, argv)) {
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

RunTime::RunTime() :
m_llvm_builder(LLVMCreateBuilder()),
m_macro(std::make_shared<Macro>()),
m_diag(std::make_shared<Diag>()) {

    m_assert_enable = true;
    m_load_dsl = true;
    m_load_runtime = true;
    m_error_limit = 10;
    m_typedef_limit = 0;

    m_args = Obj::CreateType(ObjType::Dictionary, ObjType::Dictionary, true);

    m_main_module = std::make_shared<Module>();

    LLVMLoadLibraryPermanently(nullptr);

    GlobalNameBuildinRegister();
}

void RunTime::GlobalNameBuildinRegister() {
    VERIFY(RegisterTypeHierarchy(ObjType::None,{}));
    VERIFY(RegisterTypeHierarchy(ObjType::Any,{}));

    VERIFY(RegisterTypeHierarchy(ObjType::Arithmetic,{":Any"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Rational,{":Arithmetic"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Tensor,{":Arithmetic"}));

    VERIFY(RegisterTypeHierarchy(ObjType::Integer,{":Tensor"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Bool,{":Integer"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Int8,{":Integer"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Char,{":Integer"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Byte,{":Integer"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Int16,{":Integer"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Word,{":Integer"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Int32,{":Integer"}));
    VERIFY(RegisterTypeHierarchy(ObjType::DWord,{":Integer"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Int64,{":Integer"}));
    VERIFY(RegisterTypeHierarchy(ObjType::DWord64,{":Integer"}));

    VERIFY(RegisterTypeHierarchy(ObjType::Number,{":Tensor"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Float16,{":Number"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Float32,{":Number"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Float64,{":Number"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Single,{":Number"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Double,{":Number"}));

    VERIFY(RegisterTypeHierarchy(ObjType::Complex,{":Tensor"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Complex32,{":Complex"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Complex64,{":Complex"}));

    VERIFY(RegisterTypeHierarchy(ObjType::String,{":Any"}));
    VERIFY(RegisterTypeHierarchy(ObjType::StrChar,{":String"}));
    VERIFY(RegisterTypeHierarchy(ObjType::StrWide,{":String"}));
    VERIFY(RegisterTypeHierarchy(ObjType::FmtChar,{":String"}));

    VERIFY(RegisterTypeHierarchy(ObjType::Dictionary,{":Any"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Interface,{":Any"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Class,{":Dictionary", ":Interface"}));

    VERIFY(RegisterTypeHierarchy(ObjType::Pointer,{":Any"})); // Указатели на машиннозависимую реализауию объектов

    VERIFY(RegisterTypeHierarchy(ObjType::Plain,{":Pointer"})); // Могут быть представленые в одном блоке памяти
    VERIFY(RegisterTypeHierarchy(ObjType::Enum,{":Plain", ":Integer"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Union,{":Plain", ":Dictionary"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Struct,{":Plain", ":Class"}));


    VERIFY(RegisterTypeHierarchy(ObjType::Eval,{":Any"})); // Может быть выполнен
    VERIFY(RegisterTypeHierarchy(ObjType::Function,{":Eval"}));
    VERIFY(RegisterTypeHierarchy(ObjType::NativeFunc,{":Function", ":Pointer"}));

    VERIFY(RegisterTypeHierarchy(ObjType::Other,{":Any"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Range,{":Other"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Ellipsis,{":Other"}));

    VERIFY(RegisterTypeHierarchy(ObjType::BLOCK,{":Eval"}));
    VERIFY(RegisterTypeHierarchy(ObjType::BLOCK_TRY,{":Eval"}));

    VERIFY(RegisterTypeHierarchy(ObjType::PureFunc,{":Function"}));

    VERIFY(RegisterTypeHierarchy(ObjType::Type,{":Any"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Return,{":Any"}));

    VERIFY(RegisterTypeHierarchy(ObjType::Error,{":Return"}));
    VERIFY(RegisterTypeHierarchy(ObjType::Break,{":Return"})); // Синонимы прерывания последовательности выполнения для совместимости
    VERIFY(RegisterTypeHierarchy(ObjType::Continue,{":Return"})); // со стндартными алгоритмическими приемами (синтаксический сахар)


    VERIFY(RegisterTypeHierarchy(ObjType::ErrorParser,{":Error"}));
    VERIFY(RegisterTypeHierarchy(ObjType::ErrorRunTime,{":Error"}));
    VERIFY(RegisterTypeHierarchy(ObjType::ErrorSignal,{":Error"}));






    //    VERIFY(RegisterSystemBuildin("::__compare__(obj1, obj2):Int8"));
    //    VERIFY(RegisterSystemBuildin("::__equals__(obj1, obj2):Bool"));
}

bool RunTime::RegisterSystemBuildin(const char *text) {
    //    return RegisterSystemObj(CreateNative(Term::Create());
    return false;
}

bool RunTime::RegisterTypeHierarchy(ObjType type, std::vector<std::string> parents) {
    //            std::array < std::string, sizeof...(parents) > list = {parents...};

    std::string type_name(toString(type));
    auto base = m_types.find(type_name);
    if (base != m_types.end()) {
        return false;
    }

    ObjPtr result = Obj::CreateBaseType(type);
    ASSERT(result->m_var_type_fixed == type);
    ASSERT(result->m_var_type_current == ObjType::Type);
    ASSERT(!type_name.empty() && result->m_class_name.compare(type_name) == 0);
    ASSERT(result->m_class_parents.empty());

    for (auto &parent : parents) {
        auto iter = m_types.find(parent);
        if (iter == m_types.end()) {
            LOG_DEBUG("Parent type '%s' not found!", parent.c_str());
            return false;
        }
        for (auto &elem : result->m_class_parents) {
            ASSERT(elem);
            if (!elem->m_class_name.empty() && elem->m_class_name.compare(parent) == 0) {
                LOG_DEBUG("The type '%s' already exists in the parents of '%s'!", parent.c_str(), type_name.c_str());
                return false;
            }
        }
        ASSERT(iter->first.compare(parent) == 0);
        result->m_class_parents.push_back(iter->second);
    }

    m_types[type_name] = result;

    if (type_name.compare(":None") == 0) {
        VERIFY(NameRegister(true, "_", Term::Create(parser::token_type::NAME, TermID::TYPE, "_"), result));
    }

    return NameRegister(true, &type_name.c_str()[1], Term::Create(parser::token_type::NAME, TermID::TYPE, &type_name.c_str()[1]), result)
            && NameRegister(true, type_name.c_str(), Term::Create(parser::token_type::NAME, TermID::TYPE, type_name.c_str()), result);
}

ObjPtr RunTime::GetTypeFromString(const std::string & type, bool *has_error) {
    if (type.empty()) {
        if (has_error) {
            *has_error = true;
            return Obj::CreateNone();
        }
        LOG_RUNTIME("Type name '%s' not found!", type.c_str());
    }

    auto result_types = m_types.find(type);
    if (result_types != m_types.end()) {
        return result_types->second;
    }

    ObjPtr found = NameGet(type.c_str(), false);
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
    LOG_RUNTIME("Type name '%s' not found!", type.c_str());
}

ObjType RunTime::BaseTypeFromString(const std::string & type, bool *has_error) {
    ObjPtr obj_type = GetTypeFromString(type, has_error);

    if (obj_type == nullptr) {
        if (has_error) {
            *has_error = true;
            return ObjType::None;
        }
        LOG_RUNTIME("Type name '%s' not found!", type.c_str());
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
        for (auto &elem : m_types) {
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

//GlobNamePtr RunTime::FindObject(const char *name) {
//    if (!name) {
//        return nullptr;
//    }
//
//    if (isLocalAny(name)) {
//
//        auto found = find(name);
//        if (found != end()) {
//            return found->second;
//        }
//        return nullptr;
//    }
//
//    auto found = find(name);
//    if (found != end()) {
//        return found->second;
//    }
//
//    std::string find_name("@");
//
//    find_name.append(name);
//    found = find(find_name);
//    if (found != end()) {
//        return found->second;
//    }
//
//    find_name[0] = '$';
//    found = find(find_name);
//    if (found != end()) {
//        return found->second;
//    }
//
//    find_name[0] = ':';
//    found = find(find_name);
//    if (found != end()) {
//        return found->second;
//    }
//
//    return nullptr;
//}

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

ObjPtr RunTime::CreateNative(Context *ctx, const char *proto, const char *module, bool lazzy, const char *mangle_name) {
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
    return CreateNative(ctx, term, module, lazzy, mangle_name);
}

ObjPtr RunTime::CreateNative(Context *ctx, TermPtr proto, const char *module, bool lazzy, const char *mangle_name) {
    ASSERT(!lazzy);

    void *addr = GetNativeAddr(mangle_name ? mangle_name : proto->m_text.c_str(), module);
    NL_CHECK(addr, "Error getting address '%s' from '%s'!", proto->toString().c_str(), module);

    return RunTime::CreateNative(proto, addr, ctx);
}

ObjPtr RunTime::CreateNative(TermPtr proto, void *addr, Context *ctx) {

    NL_CHECK(proto, "Fail prototype native function!");
    //    NL_CHECK((module == nullptr || (module && *module == '\0')) || m_runtime,
    //            "You cannot load a module '%s' without access to the runtime context!", module);

    ObjPtr result;
    ObjType type = ObjType::None;
    if (proto->isCall()) {
        type = ObjType::NativeFunc;

        ObjType type_test;
        for (int i = 0; i < proto->size(); i++) {

            if (!proto->name(i).empty()) {
                NL_PARSER((*proto)[i].second, "Default values '%s' are not supported!", proto->name(i).c_str());
            }

            if (isLocalAnyName((*proto)[i].second->m_text.c_str())) {
                NL_PARSER((*proto)[i].second, "Argument name expected '%s'!", (*proto)[i].second->m_text.c_str());
            }

            if ((*proto)[i].second->getTermID() == TermID::ELLIPSIS) {
                if (i + 1 != proto->size()) {
                    NL_PARSER((*proto)[i].second, "Ellipsys must end of argument!");
                }
            } else {
                if (!(*proto)[i].second->m_type) {
                    NL_PARSER((*proto)[i].second, "Argument type must be specified!");
                }

                type_test = typeFromString((*proto)[i].second->m_type->m_text, ctx ? ctx->m_runtime : nullptr);
                if (!isNativeType(type_test)) {
                    NL_PARSER((*proto)[i].second->GetType(), "Argument must be machine type! Creating a variable with type '%s' is not supported!", (*proto)[i].second->m_type->m_text.c_str());
                }
            }
        }


    } else if (proto->getTermID() == TermID::NAME) {
        if (!proto->m_type) {
            NL_PARSER(proto, "Cannot create native variable without specifying the type!");
        }

        type = typeFromString(proto->m_type->m_text); //, this);
        if (!isNativeType(type)) {
            NL_PARSER(proto, "Creating a variable with type '%s' is not supported!", proto->m_type->m_text.c_str());
        }
    } else {
        NL_PARSER(proto, "Native type arg undefined!");
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
    ASSERT(at::holds_alternative<at::monostate>(result->m_var));

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
                NL_CHECK(at::get<void *>(result->m_var), "Error getting address '%s'!", proto->toString().c_str());
            } else if (ptr && result->is_tensor_type()) {
                //            result->m_tensor = torch::from_blob(at::get<void *>(result->m_var),{
                //            }, toTorchType(type));
                result->m_var_is_init = true;
            } else {
                NL_PARSER(proto, "Fail CreateNative object");
            }
    }
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

bool newlang::CheckCharModuleName(const char *name) {
    for (int i = 0; name && name[i]; i++) {
        // Module name - backslash, underscore, lowercase English letters or number
        if (!(name[i] == '\\' || name[i] == '_' || islower(name[i]) || isdigit(name[i]))) {
            return false;
        }
    }
    return name;
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
            NL_PARSER(term, "Load module fail!");
        }
    }
    return m_modules[name];
}

/*
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */

bool NsStack::NameMacroExpand(TermPtr term) {
    ASSERT(term);
    if (term->m_text.find("@::") == 0) {
        if (term->m_text.size() == 3) {
            term->m_text = GetNamespace();
        } else {
            if (GetNamespace().empty()) {
                term->m_text = term->m_text.replace(0, 3, "$$");
            } else {
                term->m_text = term->m_text.replace(0, 1, GetNamespace());
            }
        }
        //        } else if (term->m_text.find("@$$") == 0) {
        //            term->m_text = term->m_text.replace(0, 3, Get__BLOCK__());
    } else if (term->m_id == TermID::NAMESPACE) {
        if (term->m_text.compare("@__CLASS__") == 0) {
            term->m_text = Get__CLASS__();
        } else if (term->m_text.compare("@__NAMESPACE__") == 0) {
            term->m_text = GetNamespace();
        } else if (term->m_text.compare("@__FUNC_BLOCK__") == 0) {
            term->m_text = Get__BLOCK__();
        } else if (term->m_text.compare("@__FUNCTION__") == 0) {
            term->m_text = Get__FUNCTION__();
        } else if (term->m_text.compare("@__FUNCDNAME__") == 0) {
            term->m_text = Get__FUNCDNAME__();
        } else if (term->m_text.compare("@__FUNCSIG__") == 0) {
            term->m_text = Get__FUNCSIG__();
        } else {
            NL_MESSAGE(LOG_LEVEL_INFO, term, "Fail NAMESPACE prefix '%s'!", term->m_text.c_str());

            return false;
        }
        term->m_id = TermID::STRCHAR;
    }
    return true;
}

bool NsStack::LookupName(TermPtr term, RunTime * rt) {

    if (term && !term->m_text.empty() && term->m_text[0] == '@') {
        if (m_op && m_op->IsCreate() && term->m_text.compare("@$$") == 0) {
            if (m_root->m_variables.empty() && !m_root->m_module) {
                if (!m_op->m_right || !CheckCharModuleName(m_op->m_right->m_text.c_str())) {
                    NL_MESSAGE(LOG_LEVEL_INFO, m_op, "Set module name - backslash, underscore, lowercase English letters or number!");
                    return false;
                }
                m_root->m_module = m_op->m_right;
            } else {
                NL_MESSAGE(LOG_LEVEL_INFO, m_op, "Setting the module name must be the first statement once!");
                return false;
            }
        } else if (!NameMacroExpand(term)) {
            return false;
        }
    }

    //// error: no member named 'isCreate' in 'newlang::Term'; did you mean 'IsCreate'?
    TermPtr proto;
    //    NameItem *item = nullptr;

    if (m_op && m_op->IsCreate()) {

        if (term->m_id == TermID::FIELD) {

            // Field class or module
            LOG_RUNTIME("Not TermID::FIELD implemented! '%s'", term->toString().c_str());

        } else if (isLocalName(term->m_text)) {
            // Full name include namespace
            term->m_text.insert(0, GetNamespace());
        } else if (!isGlobalScope(term->m_text) && !isModuleScope(term->m_text)) {
            if (term->isCall()) {
                term->m_text.insert(0, "$"); // Default hiden function name
            }
            term->m_text.insert(0, "$"); // Default local variable name
            term->m_text.insert(0, GetNamespace());
        } else {
            ASSERT(isGlobalScope(term->m_text) || isModuleScope(term->m_text));

            if (!isStaticName(term->m_text) && !GetNamespace().empty()) {
                // Expand full name
                term->m_text.insert(0, "::");
                term->m_text.insert(0, GetNamespace());
            }
        }

        if (m_op->m_id == TermID::CREATE_ONCE || m_op->m_id == TermID::PURE_OVERLAP) {

            if (isLocalName(term->m_text)) {

                if (m_function) {
                    proto = m_function->m_variables.NameRegister(true, term->m_text.c_str(), term);
                } else if (m_class) {
                    proto = m_class->m_variables.NameRegister(true, term->m_text.c_str(), term);
                } else {
                    ASSERT(m_root);
                    proto = m_root->m_variables.NameRegister(true, term->m_text.c_str(), term);
                }

            } else if (isModuleScope(term->m_text)) {

                proto = m_root->m_variables.NameRegister(true, term->m_text.c_str(), term);

            } else if (isGlobalScope(term->m_text)) {
                if (!rt) {
                    LOG_RUNTIME("Global table name not exists!");
                }
                proto = rt->NameRegister(true, term->m_text.c_str(), term);
            } else {
                LOG_RUNTIME("Qualifier name '%s' not recognized!", term->toString().c_str());
            }

        } else if (m_op->m_id == TermID::CREATE_OVERLAP || m_op->m_id == TermID::PURE_ONCE) {

            if (isLocalName(term->m_text)) {

                if (m_function) {
                    proto = m_function->m_variables.NameRegister(false, term->m_text.c_str(), term);
                } else if (m_class) {
                    proto = m_class->m_variables.NameRegister(false, term->m_text.c_str(), term);
                } else {
                    ASSERT(m_root);
                    proto = m_root->m_variables.NameRegister(false, term->m_text.c_str(), term);
                }

            } else if (isModuleScope(term->m_text)) {
                //                item = m_root->m_variables.NameRegister(false, term->m_text.c_str(), term);
            } else if (isGlobalScope(term->m_text)) {
                if (!rt) {
                    LOG_RUNTIME("Global table name not exists!");
                }
                proto = rt->NameRegister(false, term->m_text.c_str(), term);
            } else {
                LOG_RUNTIME("Qualifier name '%s' not recognized!", term->toString().c_str());
            }

        } else if (m_op->m_id == TermID::ASSIGN) {

            if (isLocalName(term->m_text)) {

                if (m_function) {
                    proto = m_function->m_variables.NameFind(term->m_text.c_str());
                } else if (m_class) {
                    proto = m_class->m_variables.NameFind(term->m_text.c_str());
                } else {
                    ASSERT(m_root);
                    proto = m_root->m_variables.NameFind(term->m_text.c_str());
                }

            } else if (isModuleScope(term->m_text)) {
                proto = m_root->m_variables.NameFind(term->m_text.c_str());
            } else if (isGlobalScope(term->m_text)) {
                if (!rt) {
                    LOG_RUNTIME("Global table name not exists!");
                }
                proto = rt->GlobFindProto(term->m_text.c_str());
            } else {
                LOG_RUNTIME("Qualifier name '%s' not recognized!", term->toString().c_str());
            }

        } else {
            LOG_RUNTIME("CREATE '%s' not implemented!", m_op ? m_op->toString().c_str() : term->toString().c_str());
        }

    } else if (term->getTermID() == TermID::MODULE) {
        if (term->isCall()) {
            // Skip load module
            // @todo Load module args
            ASSERT(term->size() == 0);
        }

        ModulePtr module = rt ? rt->CheckLoadModule(term) : nullptr;
        if (!module) {
            return false;
        }
        if (term->m_right) {
            proto = rt->GlobFindProto(term->m_right->m_text.c_str());
        } else {
            // Module exist - all ok!
            return true;
        }
    }

    if (!proto) {
        return false; // Name not found
    }
    // Совместимость типов у переменных при повторных присвоениях
    if (rt && !rt->CheckType(proto, term)) {
        return false;
    }

    if (m_op && m_op->m_right) {
        // Совместимость типов правого операнда с типом переменной
        if (!rt->CheckType(proto, m_op->m_right)) {
            return false;
        }
        if (m_op->getTermID() == TermID::CREATE_ONCE || m_op->getTermID() == TermID::PURE_OVERLAP) {
            //            item->
        }
    }
    return true;
}

std::string NsStack::Get__CLASS__() {
    if (m_class) {

        return m_class->m_text;
    }
    return "";
}

std::string NsStack::Get__FUNCTION__() {
    if (m_function) {

        return m_function->m_text;
    }
    return "";
}

std::string NsStack::Get__FUNCDNAME__() {
    if (m_function) {

        return m_function->m_text;
    }
    return "";
}

std::string NsStack::Get__FUNCSIG__() {
    if (m_function) {

        return m_function->m_text;
    }
    return "";
}

std::string NsStack::Get__BLOCK__() {
    if (m_blocks.size()) {

        return m_blocks.back();
    }
    return "";
}

bool NsStack::LookupBlock(TermPtr &term) {
    if (!term || term->m_text.empty() || term->m_text.compare("::") == 0) {
        return true;
    }
    for (auto &elem : m_blocks) {
        if (elem.compare(term->m_text) == 0) {
            return true;
        }
    }
    NL_MESSAGE(LOG_LEVEL_INFO, term, "Lookup block '%s' fail!%s", term->m_text.c_str(), GetOfferBlock().c_str());

    return false;
}

class ParserError : public Return {
public:

    ParserError(std::string msg) : Return(msg) {
    }

    virtual ~ParserError() {
    }

};

void RunTime::AstCheckError(bool force) {
    if (m_error_limit && m_error_count >= m_error_limit) {
        LOG_CUSTOM_ERROR(ParserError, "fatal error: too many errors emitted %d, stopping now [-nlc-error-limit=]", m_error_count);
    } else if (force && m_error_count) {

        LOG_CUSTOM_ERROR(ParserError, "fatal error: %d generated. ", m_error_count);
    }
}

bool RunTime::AstAnalyze(TermPtr &term, bool is_main) {
    m_error_count = 0;
    NsStack stack(term);
    if (is_main) {
        // Main module name - empty string
        term->m_module = Term::Create(parser::token_type::END, TermID::STRCHAR, "");
    }
    try {
        AstRecursiveAnalyzer(term, stack);
        AstCheckError(true);
    } catch (ParserError err) {
        return false;
    }
    return true;
}

void RunTime::AstRecursiveAnalyzer(TermPtr &term, NsStack &stack) {
    if (term->IsLiteral()) {

        return;

    } else if (term->getTermID() == TermID::STATIC || term->getTermID() == TermID::LOCAL || term->getTermID() == TermID::NAME
            || term->getTermID() == TermID::NAMESPACE || term->getTermID() == TermID::FIELD || term->getTermID() == TermID::MODULE) {

        if (!stack.LookupName(term, this)) {
            NL_MESSAGE(LOG_LEVEL_INFO, term, "NameLookup fail for '%s'!%s", term->m_text.c_str(), stack.GetOffer().c_str());
            m_error_count++;
            AstCheckError(false);
        }

    } else if (term->IsInterrupt()) {

        if (!stack.NameMacroExpand(term->m_namespace)) {
            m_error_count++;
            AstCheckError(false);
        }
        if (!stack.LookupBlock(term->m_namespace)) {
            m_error_count++;
            AstCheckError(false);
        }

    } else if (term->IsCreate()) {

        if (!AstCreateOp(term, stack)) {
            m_error_count++;
            AstCheckError(false);
        }

        //        if (term->m_left->isCall()) {
        //            if (!(term->getTermID() == TermID::CREATE_ONCE || term->getTermID() == TermID::PURE_OVERLAP)) {
        //                NL_MESSAGE(LOG_LEVEL_INFO, term, "Allowed to create functions only by one-time initialization using '::='!");
        //                m_error_count++;
        //                AstCheckError(false);
        //            }
        //        }
        //
        //        stack.m_op = term;
        //        AstRecursiveAnalyzer(term->m_left, stack);
        //
        //        if (term->m_right->getTermID() == TermID::CLASS) {
        //
        //            stack.SetClass(term->m_left);
        //            AstRecursiveAnalyzer(term->m_right, stack);
        //            stack.SetClass(nullptr);
        //
        //        } else if (term->m_left->isCall()) {
        //
        //            stack.SetFunc(term->m_left);
        //            AstRecursiveAnalyzer(term->m_right, stack);
        //            stack.SetFunc(nullptr);
        //
        //        } else {
        //            AstRecursiveAnalyzer(term->m_right, stack);
        //        }
        //
        //        stack.m_op = nullptr;

    } else if (term->getTermID() == TermID::OPERATOR) {

        AstRecursiveAnalyzer(term->m_left, stack);
        AstRecursiveAnalyzer(term->m_right, stack);

        if (!CheckType(term->m_left, term->m_right)) {
            m_error_count++;
            AstCheckError(false);
        }

    } else if (term->IsBlock()) {
        bool is_pop = !!term->m_namespace;
        if (is_pop) {
            stack.NamespacePush(term->m_namespace->m_text);
        }

        for (auto &elem : term->m_block) {
            AstRecursiveAnalyzer(elem, stack);
        }
        if (is_pop) {
            stack.NamespacePop();
        }
    } else if (term->getTermID() == TermID::FOLLOW) {
        ASSERT(term->m_right);
        AstRecursiveAnalyzer(term->m_right, stack);
        //        for (auto &elem : term->m_follow) {
        //            ASSERT(elem);
        //            AstExpandNamespace(elem, stack);
        //        }
    } else {

        LOG_TEST("Skip ExpandNamespace %s: '%s'", toString(term->getTermID()), term->toString().c_str());
    }
}

bool RunTime::CheckType(TermPtr &left, TermPtr &right) {

    if (left && right) {
        if (!right->m_type && !left->m_type) {
            return true;
        }
        if (!left->m_type && right->m_type) {
            left->m_type = right->m_type;
            return true;
        } else if (left->m_type && !right->m_type) {
            // Mark dinamic type check cast ??????????????????????????????????
            return true;
        }
        if (right->m_type && left->m_type) {
            if (!canCast(typeFromString(right->m_type), typeFromString(left->m_type))) {
                if (canCast(typeFromString(left->m_type), typeFromString(right->m_type)) && isDefaultType(left->m_type)) {
                    // UpCast default type
                    left->m_type = right->m_type;
                } else {
                    NL_MESSAGE(LOG_LEVEL_INFO, right, "Fail cast type %s to %s.", right->m_type->asTypeString().c_str(), left->m_type->asTypeString().c_str());
                    return false;
                }
            }
            return true;
        }
        LOG_TEST("Skip CheckType left '%s' and  right '%s'", left->toString().c_str(), right->toString().c_str());
        LOG_TEST("Skip CheckType %s and %s", toString(left->getTermID()), right->toString().c_str());

        return false;
    }
    return false;
}

bool AstCreateVar(TermPtr &var, TermPtr &value, NsStack &stack);
bool AstAssignVar(TermPtr &var, TermPtr &value, NsStack &stack);

bool RunTime::AstCreateOp(TermPtr &op, NsStack &stack) {
    return false;
    //    TermPtr variable = op->m_left;
    //    TermPtr value = op;
    //    int right_count = 0;
    //
    //    while (variable) {
    //        if (value->m_right) {
    //            value = value->m_right;
    //            right_count++;
    //        } else if (right_count) {
    //        }
    //        if (!AstCreateVar(variable, value, stack)) {
    //            return false;
    //        }
    //        variable = op->m_left;
    //    }
    //
    //
    //    if (op->isCreateOnce()) {
    //    } else if (op->isCreateOverlap()) {
    //    } else {
    //        ASSERT(op->isCreate());
    //    }
    //
    //    if (term->m_left->isCall()) {
    //        if (!(term->getTermID() == TermID::CREATE_ONCE || term->getTermID() == TermID::PURE_OVERLAP)) {
    //            NL_MESSAGE(LOG_LEVEL_INFO, term, "Allowed to create functions only by one-time initialization using '::='!");
    //            m_error_count++;
    //            AstCheckError(false);
    //        }
    //    }
    //
    //    stack.m_op = term;
    //    AstRecursiveAnalyzer(term->m_left, stack);
    //
    //    if (term->m_right->getTermID() == TermID::CLASS) {
    //
    //        stack.SetClass(term->m_left);
    //        AstRecursiveAnalyzer(term->m_right, stack);
    //        stack.SetClass(nullptr);
    //
    //    } else if (term->m_left->isCall()) {
    //
    //        stack.SetFunc(term->m_left);
    //        AstRecursiveAnalyzer(term->m_right, stack);
    //        stack.SetFunc(nullptr);
    //
    //    } else {
    //        AstRecursiveAnalyzer(term->m_right, stack);
    //    }
    //
    //    stack.m_op = nullptr;

}
