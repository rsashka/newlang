#include "pch.h"
#include "system.h"

#include <term.h>
#include <runtime.h>
#include <builtin.h>

using namespace newlang;

const char * RunTime::default_argv[RunTime::default_argc] = {"", "-nlc-no-runtime", "-nlc-no-dsl"};

GlobNameItem * GlobNameList::GlobalNameFind(const char* name) {

    RunTime::iterator found;

    if (!name) {
        return nullptr;
    }

    if (isMacroName(name)) {
        LOG_RUNTIME("Macro name not allowed! '%s'", name);
    }
    if (isLocalName(name)) {
        LOG_RUNTIME("Local name not allowed! '%s'", name);
    }

    if (isGlobalName(name) || isTypeName(name)) {

        found = find(name);
        if (found == end()) {
            return nullptr;
        }

    } else {

        found = find(name);
        if (found != end()) {
            LOG_RUNTIME("Fail logical for name '%s'", name);
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

bool GlobNameList::GlobalNameRegister(TermPtr term, WeakItem obj) {

    ASSERT(term);

    std::string name = term->getText();

    if (isMacroName(name)) {
        NL_PARSER(term, "Macro name not allowed! '%s'", name.c_str());
    }
    if (isLocalName(name)) {
        NL_PARSER(term, "Local name not allowed! '%s'", name.c_str());
    }


    if (!isTypeName(name) && !GlobalNameFind(name.c_str())) {
        name.insert(0, "::");
    }

    if (GlobalNameFind(name.c_str())) {
        NL_PARSER(term, "GlobalName '%s' already register!", name.c_str());
    }

    insert(std::pair(name, GlobNameItem({term, obj})));

    return true;
}


//bool RunTime::NameAnalisysItem_(TermPtr lval, TermPtr rval) {
//    switch (rval->getTermID()) {
//        case TermID::NAME:
//        case TermID::MACRO:
//        case TermID::LOCAL:
//        case TermID::MODULE:
//        case TermID::NATIVE:
//            return true;
//    }
//    switch (lval->getTermID()) {
//        case TermID::MODULE:
//        case TermID::NATIVE:
//            LOG_ERROR("Term '%s' as lval not allowed!", lval->toString().c_str());
//            return false;
//        case TermID::NAME:
//        case TermID::MACRO:
//        case TermID::LOCAL:
//            return true;
//    }
//    return true;
//}
//
//bool RunTime::NameAnalisys_(TermPtr term) {
//    if (term->IsCreate()) {
//        return NameAnalisysItem_(term->Left(), term->Right());
//    }
//    return true;
//}
//
//bool RunTime::NameAnalisys(TermPtr ast) {
//    ASSERT(ast);
//    if (ast->IsBlock()) {
//        for (auto &elem : ast->m_block) {
//            if (!NameAnalisys(ast)) {
//                return false;
//            }
//        }
//    } else {
//        return NameAnalisys_(ast);
//    }
//    return true;
//}

ModulePtr RunTime::LoadModule(const char *term, bool init) {
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

            //            //            LOG_DEBUG("Check '%s' in file %s", name_str, full_path.c_str());
            //
            //            if (llvm::sys::fs::exists(full_path)) {
            //                LOG_DEBUG("Module '%s' load from file '%s'!", term, full_path.c_str());
            //
            //                ModulePtr module = std::make_shared<Module>();
            //                if (module->Load(ctx, full_path.c_str(), false)) {
            //
            //                    ctx.m_terms = module.get();
            //                    ctx.ExecStr(module->m_source);
            //                    ctx.m_terms = ctx.m_main_module.get();
            //
            //                    for (int i = 0; i < module->size(); i++) {
            //                        if (!isModule(module->at(i).first)) {
            //                            if (module->at(i).first.compare("::") != 0) {
            //                                module->at(i).first.insert(0, "::");
            //                            }
            //                            module->at(i).first.insert(0, name);
            //                        }
            //                    }
            //
            //                    return module;
            //                }
            //            }


        }
    }

    LOG_ERROR("Module name '%s' or file '%s' not found!", term, search_file.c_str());

    return nullptr;


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
    //                    case TermID::PUREFUNC:
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
    return nullptr;
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
//        if((elem.second->m_func_source) && ((elem.second->m_func_source)->getTermID() == TermID::SIMPLE || (elem.second->m_func_source)->getTermID() == TermID::PUREFUNC)) {
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

RuntimePtr RunTime::Init(int argc, const char** argv, bool ignore_error) {
    RuntimePtr rt = std::make_shared<RunTime>();
    if (!rt->ParseArgs(argc, argv)) {
        LOG_RUNTIME("Fail parse args!");
    }
    return rt;
}

RunTime::RunTime() : GlobNameList(),
m_llvm_builder(LLVMCreateBuilder()),
m_macro(std::make_shared<Macro>()),
m_diag(std::make_shared<Diag>()),
m_parser(m_macro, nullptr, m_diag, true) {

    m_args = Obj::CreateType(ObjType::Dictionary, ObjType::Dictionary, true);

    m_main_module = std::make_shared<Module>();

    LLVMLoadLibraryPermanently(nullptr);

    GlobalNameRegisterTypes();

    VERIFY(RegisterModule(std::make_shared<newlang::runtime::System>()));
}

void RunTime::GlobalNameRegisterTypes() {
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

    return true;
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

    ObjPtr found = GlobalNameGet(type.c_str(), false);
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

bool RunTime::RegisterModule(ModulePtr module) {
    ASSERT(module);

    //    if (module->m_var_type_current != ObjType::Module) {
    //        LOG_RUNTIME("Object not a module '%s'!", module->toString().c_str());
    //        return false;
    //    }
    //
    //    std::string name("__buildin__::");
    //    name.append(module->m_class_name);
    //
    //    if (m_modules.find(name) != m_modules.end()) {
    //        LOG_ERROR("Module name '%s' already register!", name.c_str());
    //        return false;
    //    }
    //
    //    for (auto &obj : *module) {
    //        if (obj.second->is_function_type()) {
    //            if (!GlobalNameRegister(obj.second->m_prototype, obj.second)) {
    //                LOG_ERROR("Fail register prototype '%s'!", obj.second->m_prototype->toString().c_str());
    //                return false;
    //            }
    //        }
    //    }
    //
    //    m_modules[name] = module;

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

    void *addr = GetNativeAddr(mangle_name ? mangle_name : proto->m_text.c_str(), module);
    NL_CHECK(addr, "Error getting address '%s' from '%s'!", proto->toString().c_str(), module);

    return RunTime::CreateNative(proto, addr);
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

            if (!proto->name(i).empty()) {
                NL_PARSER((*proto)[i].second, "Default values '%s' are not supported!", proto->name(i).c_str());
            }

            if (isLocalAnyName((*proto)[i].second->m_text.c_str())) {
                NL_PARSER((*proto)[i].second, "Argument name expected '%s'!", (*proto)[i].second->m_text.c_str());
            }

            if ((*proto)[i].second->m_type_name.empty()) {
                NL_PARSER((*proto)[i].second, "Argument type must be specified!");
            }

            type_test = typeFromString((*proto)[i].second->m_type_name); //, this);
            if (!isNativeType(type_test)) {
                NL_PARSER((*proto)[i].second->GetType(), "Argument must be machine type! Creating a variable with type '%s' is not supported!", (*proto)[i].second->m_type_name.c_str());
            }
        }


    } else if (proto->getTermID() == TermID::NAME) {
        if (proto->m_type_name.empty()) {
            NL_PARSER(proto, "Cannot create native variable without specifying the type!");
        }

        type = typeFromString(proto->m_type_name); //, this);
        if (!isNativeType(type)) {
            NL_PARSER(proto, "Creating a variable with type '%s' is not supported!", proto->m_type_name.c_str());
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
            break;
        case ObjType::Int8:
        case ObjType::Char:
        case ObjType::Byte:
            result->m_var = static_cast<int8_t *> (ptr);
            break;
        case ObjType::Int16:
        case ObjType::Word:
            result->m_var = static_cast<int16_t *> (ptr);
            break;
        case ObjType::Int32:
        case ObjType::DWord:
            result->m_var = static_cast<int32_t *> (ptr);
            break;
        case ObjType::Int64:
        case ObjType::DWord64:
            result->m_var = static_cast<int64_t *> (ptr);
            break;
        case ObjType::Float32:
        case ObjType::Single:
            result->m_var = static_cast<float *> (ptr);
            break;
        case ObjType::Float64:
        case ObjType::Double:
            result->m_var = static_cast<double *> (ptr);
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

bool RunTime::CheckOrLoadModule(std::string str) {
    ASSERT(isModuleName(str));


    std::string name = ExtractModuleName(str.c_str());
    if (m_modules.find(name) == m_modules.end()) {

        ModulePtr module = LoadModule(str.c_str(), true);
        if (module) {
            m_modules[name] = std::move(module);
        }
    }
    return m_modules.find(name) != m_modules.end();
}

bool RunTime::CheckNativeName(TermPtr &term, void * obj) {
    RunTime * rt = static_cast<RunTime *> (obj);

    if (term) {

    }
    return false;
}

bool RunTime::AppendAndCheckGlobalName(TermPtr &term, void * obj) {
    RunTime * rt = static_cast<RunTime *> (obj);

    if (term) {

    }
    return false;
}


