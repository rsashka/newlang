
#include "warning_push.h"

#include <clang/AST/ASTConcept.h>
#include <clang/Sema/SemaConcept.h>
#include <clang/Driver/Driver.h>
#include <clang/Driver/Compilation.h>

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/CodeGen.h>

#include <llvm/Support/Signals.h>

#include <llvm/Analysis/AliasAnalysis.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>

#include "warning_pop.h"

#include "jit.h"
#include "runtime.h"
#include "term.h"
#include "analysis.h"
#include "module.h"

#include "build_options.data"
#include "include_h_i.data"

//inline constexpr unsigned long newlang_build_options_size = 30;
//inline std::string newlang_build_options_arr[newlang_build_options_size];
//inline constexpr unsigned long newlang_include_h_i_size = 111295;
//inline std::string newlang_include_h_i_arr[newlang_include_h_i_size]



using namespace newlang;

JIT * JIT::m_instance = nullptr;

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

/*
 * 
 * 
 */
//std::unique_ptr<llvm::Module> JIT::MakeLLVMModule(std::string_view source, const std::vector<std::string> extra_opts, std::string *asm_code) {
//
//    //        std::unique_ptr<llvm::Module> CompileCpp(std::string source, std::vector<std::string> opts, std::string *asm_code = nullptr) {
//    clang::CompilerInstance compilerInstance;
//    auto& compilerInvocation = compilerInstance.getInvocation();
//
//
////    // Диагностика работы Clang
////    clang::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts = new clang::DiagnosticOptions;
////    clang::TextDiagnosticPrinter *textDiagPrinter =
////            new clang::TextDiagnosticPrinter(llvm::outs(), &*DiagOpts);
////
////    clang::IntrusiveRefCntPtr<clang::DiagnosticIDs> pDiagIDs;
////
////    clang::DiagnosticsEngine *pDiagnosticsEngine =
////            new clang::DiagnosticsEngine(pDiagIDs, &*DiagOpts, textDiagPrinter);
//
//
//    // Целевая платформа
//    std::string triple = LLVMGetDefaultTargetTriple();
//
//    std::vector<std::string> run_opts;
//    run_opts.push_back(triple.insert(0, "-triple="));
//
//    run_opts.push_back("-xc++");
//
//    run_opts.push_back("-DBUILD_UNITTEST");
//    run_opts.push_back("-DBUILD_DEBUG ");
//    run_opts.push_back("-DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG");
//
//    std::string build_options_string;
//    for (size_t i = 0; i < newlang_build_options_size; i++) {
//        build_options_string += newlang_build_options_arr[i];
//        build_options_string += " ";
//    }
//    std::vector<std::string> build_opts = RunTime::SplitChar(build_options_string, " \t\r\n");
//    build_options_string.clear();
//
//    run_opts.insert(run_opts.end(), build_opts.begin(), build_opts.end());
//    run_opts.insert(run_opts.end(), extra_opts.begin(), extra_opts.end());
//
//    for (auto &elem : run_opts) {
//        if (elem.find("-D") == 0) {
//            elem += " ";
//        }
//        //        std::cout << elem << "\n";
//    }
//
//    std::vector<const char*> itemcstrs;
//    for (unsigned idx = 0; idx < run_opts.size(); idx++) {
//        // note: if itemstrs is modified after this, itemcstrs will be full
//        // of invalid pointers! Could make copies, but would have to clean up then...
//        itemcstrs.push_back(run_opts[idx].c_str());
//        //        std::cout << itemcstrs.back() << "\n";
//    }
//
//    // Компиляция из памяти
//    // Send code through a pipe to stdin
//    int codeInPipe[2];
//    pipe2(codeInPipe, O_NONBLOCK);
//    write(codeInPipe[1], source.begin(), source.size());
//    close(codeInPipe[1]); // We need to close the pipe to send an EOF
//    dup2(codeInPipe[0], STDIN_FILENO);
//
//    itemcstrs.push_back("-"); // Read code from stdin
//
//    clang::CompilerInvocation::CreateFromArgs(compilerInvocation,
//            llvm::ArrayRef<const char *>(itemcstrs.data(),
//            itemcstrs.size()), *pDiagnosticsEngine);
//
//    auto& languageOptions = compilerInvocation.getLangOpts();
//    auto& preprocessorOptions = compilerInvocation.getPreprocessorOpts();
//    auto& targetOptions = compilerInvocation.getTargetOpts();
//
//    auto& frontEndOptions = compilerInvocation.getFrontendOpts();
//    //    frontEndOptions.ShowStats = true;
//
//    auto& headerSearchOptions = compilerInvocation.getHeaderSearchOpts();
//    //    headerSearchOptions.Verbose = true;
//
//    auto& codeGenOptions = compilerInvocation.getCodeGenOpts();
//
//
//    targetOptions.Triple = LLVMGetDefaultTargetTriple();
//    compilerInstance.createDiagnostics(textDiagPrinter, false);
//
//    //    DEBUG_MSG("Using target triple: " << triple);
//
//    LLVMContextRef ctx = LLVMContextCreate();
//    std::unique_ptr<clang::CodeGenAction> action = std::make_unique<clang::EmitLLVMOnlyAction>((llvm::LLVMContext *)ctx);
//
//    assert(compilerInstance.ExecuteAction(*action));
//
//    // Runtime LLVM Module
//    std::unique_ptr<llvm::Module> module = action->takeModule();
//
//    assert(module);
//
//
//    if (asm_code) {
//        llvm::raw_string_ostream asm_stream(*asm_code);
//        module->print(asm_stream, 0, false);
//    }
//    //AssemblyWriter
//
//    // Оптимизация IR
//    //    llvm::PassBuilder passBuilder;
//    //    llvm::LoopAnalysisManager loopAnalysisManager;
//    //    llvm::FunctionAnalysisManager functionAnalysisManager;
//    //    llvm::CGSCCAnalysisManager cGSCCAnalysisManager;
//    //    llvm::ModuleAnalysisManager moduleAnalysisManager;
//    //
//    //    passBuilder.registerModuleAnalyses(moduleAnalysisManager);
//    //    passBuilder.registerCGSCCAnalyses(cGSCCAnalysisManager);
//    //    passBuilder.registerFunctionAnalyses(functionAnalysisManager);
//    //    passBuilder.registerLoopAnalyses(loopAnalysisManager);
//    //    passBuilder.crossRegisterProxies(loopAnalysisManager, functionAnalysisManager, cGSCCAnalysisManager, moduleAnalysisManager);
//    //
//    //    llvm::ModulePassManager modulePassManager = passBuilder.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O0);
//    //    modulePassManager.run(*module, moduleAnalysisManager);
//
//    return module;
//}

std::unique_ptr<llvm::Module> JIT::MakeLLVMModule(const std::string_view source, std::vector<std::string> extra_opts, std::string temp_dir) {

    clang::CompilerInstance compilerInstance;
    auto& compilerInvocation = compilerInstance.getInvocation();


    //    // Диагностика работы Clang
    //    clang::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts = new clang::DiagnosticOptions;
    //    clang::TextDiagnosticPrinter *textDiagPrinter =
    //            new clang::TextDiagnosticPrinter(llvm::outs(), &*DiagOpts);
    //
    //    clang::IntrusiveRefCntPtr<clang::DiagnosticIDs> pDiagIDs;
    //
    //    clang::DiagnosticsEngine *pDiagnosticsEngine =
    //            new clang::DiagnosticsEngine(pDiagIDs, &*DiagOpts, textDiagPrinter);


    // Целевая платформа
    std::string triple = LLVMGetDefaultTargetTriple();

    std::vector<std::string> run_opts;
    run_opts.push_back(triple);
    run_opts.back().insert(0, "-triple=");

    run_opts.push_back("-xc++");

    //    run_opts.push_back("-DBUILD_UNITTEST");
    //    run_opts.push_back("-DBUILD_DEBUG");
    //    run_opts.push_back("-DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG");
    //

    std::string build_options_string;
    for (size_t i = 0; i < newlang_build_options_size; i++) {
        build_options_string += newlang_build_options_arr[i];
        build_options_string += " ";
    }
    std::vector<std::string> build_opts = RunTime::SplitChar(build_options_string, " \t\r\n");
    build_options_string.clear();

    run_opts.insert(run_opts.end(), build_opts.begin(), build_opts.end());
    run_opts.insert(run_opts.end(), extra_opts.begin(), extra_opts.end());

    for (auto &elem : run_opts) {
        if (elem.find("-D") == 0) {
            elem += " ";
        }
        //        std::cout << elem << "\n";
    }


    //    std::vector<std::string> itemstrs;
    //    itemstrs.push_back(triple.insert(0, "-triple="));
    //    itemstrs.push_back("-xc++");
    //    itemstrs.push_back("-std=c++23");
    //
    //    itemstrs.insert(itemstrs.end(), opts.begin(), opts.end());


    // Компиляция из памяти
    // Send code through a pipe to stdin
    int codeInPipe[2];
    pipe2(codeInPipe, O_NONBLOCK);

    int writen;
    int pipe_sz_save = fcntl(codeInPipe[1], F_GETPIPE_SZ, 0);

    std::string temp_file;
    std::string obj_file;
    int fd_temp = 0;
    if (pipe_sz_save / 2 < source.size()) {

        close(codeInPipe[1]);

        temp_file = temp_dir.empty() ? m_temp_dir : temp_dir;
        temp_file += "/module_XXXXXX";
        fd_temp = mkstemp(temp_file.data());
        if (fd_temp < 0) {
            LOG_RUNTIME("Fail create temp file %s!", temp_file.c_str());
        }

        writen = write(fd_temp, source.begin(), source.size());
        close(fd_temp);
        if (writen != source.size()) {
            LOG_RUNTIME("Write error to temp file %s!", temp_file.c_str());
        }

        obj_file = temp_file;
        obj_file += ".o";

        run_opts.push_back("-o");
        run_opts.push_back(obj_file);
        run_opts.push_back(temp_file.c_str());

    } else {

        //    int pipe_sz = fcntl(codeInPipe[1], F_SETPIPE_SZ, 65536 * 4);

        int writen = write(codeInPipe[1], source.begin(), source.size());

        if (writen != source.size()) {
            LOG_RUNTIME("Write PIPE error: only %d of required %d bytes were written!", writen, (int) source.size());
        }

        close(codeInPipe[1]); // We need to close the pipe to send an EOF
        dup2(codeInPipe[0], STDIN_FILENO);

        run_opts.push_back("-"); // Read code from stdin

    }

    std::vector<const char*> itemcstrs;
    for (unsigned idx = 0; idx < run_opts.size(); idx++) {
        // note: if itemstrs is modified after this, itemcstrs will be full
        // of invalid pointers! Could make copies, but would have to clean up then...
        itemcstrs.push_back(run_opts[idx].c_str());
        //        std::cout << itemcstrs.back() << "\n";
    }


    clang::CompilerInvocation::CreateFromArgs(compilerInvocation,
            llvm::ArrayRef<const char *>(itemcstrs.data(),
            itemcstrs.size()), *pDiagnosticsEngine);

    auto& languageOptions = compilerInvocation.getLangOpts();
    auto& preprocessorOptions = compilerInvocation.getPreprocessorOpts();
    auto& targetOptions = compilerInvocation.getTargetOpts();

    auto& frontEndOptions = compilerInvocation.getFrontendOpts();
    //    frontEndOptions.ShowStats = true;

    auto& headerSearchOptions = compilerInvocation.getHeaderSearchOpts();
    //    headerSearchOptions.Verbose = true;

    auto& codeGenOptions = compilerInvocation.getCodeGenOpts();


    targetOptions.Triple = LLVMGetDefaultTargetTriple();
    compilerInstance.createDiagnostics(textDiagPrinter, false);

    //    std::cout << "Using target triple: " << triple;

    LLVMContextRef ctx = LLVMContextCreate();
    std::unique_ptr<clang::CodeGenAction> action = std::make_unique<clang::EmitLLVMOnlyAction>((llvm::LLVMContext *)ctx);

    if (!compilerInstance.ExecuteAction(*action)) {
        LOG_RUNTIME("Compile error!");
    }

    // Runtime LLVM Module
    std::unique_ptr<llvm::Module> module = action->takeModule();

    if (!module) {
        LOG_RUNTIME("Fail create module!");
    }

    std::filesystem::remove(temp_file);


    //    if (asm_code) {
    //        llvm::raw_string_ostream asm_stream(*asm_code);
    //        module->print(asm_stream, 0, false);
    //    }

    return module;
    //
    //    triple = LLVMGetDefaultTargetTriple();
    //    LLVMInitializeAllTargetInfos();
    //    LLVMInitializeAllTargets();
    //    LLVMInitializeAllTargetMCs();
    //    LLVMInitializeAllAsmParsers();
    //    LLVMInitializeAllAsmPrinters();
    //
    //    std::string Error;
    //    auto Target = llvm::TargetRegistry::lookupTarget(triple, Error);
    //
    //    if (!Target) {
    //        LOG_ERROR("\n%s\n", Error.c_str());
    //        return false;
    //    }
    //
    //    auto CPU = "generic";
    //    auto Features = "";
    //
    //    llvm::TargetOptions opt;
    //    auto RM = std::optional<llvm::Reloc::Model>();
    //    auto TargetMachine = Target->createTargetMachine(triple, CPU, Features, opt, RM);
    //
    //    module->setDataLayout(TargetMachine->createDataLayout());
    //    module->setTargetTriple(triple);
    //
    //    auto Filename = obj_file.empty() ? "output.o" : obj_file;
    //    std::error_code EC;
    //    llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);
    //
    //    llvm::legacy::PassManager pass;
    //    auto FileType = llvm::CodeGenFileType::ObjectFile;
    //
    //    if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
    //        llvm::errs() << "TargetMachine can't emit a file of this type";
    //        return false;
    //    }
    //    pass.run(*module);
    //    dest.flush();
    //
    //    //    //    llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts2 = new clang::DiagnosticOptions;
    //    //    //    clang::TextDiagnosticPrinter *DiagClient = new clang::TextDiagnosticPrinter(llvm::errs(), &*DiagOpts2);
    //    //    //    llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID(new clang::DiagnosticIDs());
    //    //    //    clang::DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagClient);
    //    //    clang::driver::Driver TheDriver("", triple, *pDiagnosticsEngine);
    //    //
    //    //    auto args = llvm::ArrayRef<const char *>{"-g", Filename.c_str(), "-o", execname.begin()};
    //    //
    //    //    std::unique_ptr<clang::driver::Compilation> C(TheDriver.BuildCompilation(args));
    //    //
    //    //    if (C && !C->containsError()) {
    //    //        llvm::SmallVector<std::pair<int, const clang::driver::Command *>, 4> FailingCommands;
    //    //        TheDriver.ExecuteCompilation(*C, FailingCommands);
    //    //    }
    //
    //    return true;
}

bool JIT::MakeObjFile(const std::string_view filename, llvm::Module &module, const std::vector<std::string> opts) {

    LLVMInitializeJIT();

    std::string triple = LLVMGetDefaultTargetTriple();

    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(triple, Error);

    if (!Target) {
        LOG_ERROR("\n%s\n", Error.c_str());
        return false;
    }

    auto CPU = "generic";
    auto Features = "";

    llvm::TargetOptions opt;
    auto RM = std::optional<llvm::Reloc::Model>();
    auto TargetMachine = Target->createTargetMachine(triple, CPU, Features, opt, RM);

    module.setDataLayout(TargetMachine->createDataLayout());
    module.setTargetTriple(triple);

    //    auto Filename = obj_file.empty() ? "output.o" : obj_file;
    std::error_code EC;
    llvm::raw_fd_ostream dest(filename.begin(), EC, llvm::sys::fs::OF_None);

    llvm::legacy::PassManager pass;
    auto FileType = llvm::CodeGenFileType::ObjectFile;

    if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        llvm::errs() << "TargetMachine can't emit a file of this type";
        return false;
    }
    pass.run(module);
    dest.flush();
    return true;
}

bool JIT::LinkObjToExec(const std::string_view execname, std::vector<std::string> objs, std::vector<std::string> opts) {

    clang::driver::Driver TheDriver("", LLVMGetDefaultTargetTriple(), *pDiagnosticsEngine);


    objs.insert(objs.begin(), opts.begin(), opts.end());

    objs.insert(objs.begin(), "-g");
    objs.push_back("-o");
    objs.push_back(execname.begin());



    std::vector<const char*> args;
    for (unsigned idx = 0; idx < objs.size(); idx++) {
        // note: if itemstrs is modified after this, itemcstrs will be full
        // of invalid pointers! Could make copies, but would have to clean up then...
        args.push_back(objs[idx].c_str());
        //        std::cout << args.back() << "\n";
    }

    //    auto args = llvm::ArrayRef<const char *>{"-g", Filename.c_str(), "-o", execname.begin()};
    //    auto args = llvm::ArrayRef<const char *>(objs.begin(), objs.end());

    std::unique_ptr<clang::driver::Compilation> C(TheDriver.BuildCompilation(args));

    if (C && !C->containsError()) {
        llvm::SmallVector<std::pair<int, const clang::driver::Command *>, 4> FailingCommands;
        TheDriver.ExecuteCompilation(*C, FailingCommands);
    }

    return true;
}

/*
 * 
 * 
 * 
 */


std::string JIT::ExtractFunctionDecls(const TermPtr &term, const std::string_view module) {
    std::string result;
    if (term->isBlock()) {
        for (auto &elem : term->m_block) {
            result += ExtractFunctionDecls(elem, module);
        }
    } else {
        if (term->isCreate() && term->m_left->isCall()) {
            result += "// ";
            result += MakeCommentPlace(term->m_left);
            result += "\n";
            result += "extern \"C\" ";
            result += MakeFunctionPrototype(term->m_left, module);
            result += ";\n";
        }
    }
    return result;
}

std::string JIT::MakeFunctionPrototype(const TermPtr &func, const std::string_view module) {
    ASSERT(func->isCall());
    if (func->m_int_name.empty()) {
        NL_PARSER(func, "Has no internal name! AST analysis required!");
    }
    std::string result(NEWLANG_NS "::ObjPtr ");
    result += func->m_int_name.getMangledName(module);
    result += "(" NEWLANG_NS "::Context *ctx, " NEWLANG_NS "::Obj &args)";
    return result;
}

std::string JIT::MakeCommentPlace(const TermPtr &term) {
    std::string comment = term->toString();
    if (term->m_line) {
        comment += " at line: ";
        comment += std::to_string(term->m_line);
    }
    return RegExpInlineComment(comment);
}

std::string JIT::RegExpInlineComment(const std::string_view src) {
    std::string comment(src);
    comment = std::regex_replace(comment, std::regex("\n"), "\\n");
    comment = std::regex_replace(comment, std::regex("\""), "@\"");
    return comment;
}

TermPtr JIT::MainArgs() {
    TermPtr result = Term::CreateDict();
    result->push_back(Term::CreateName("argc"));
    result->back().second->m_type = Term::CreateName(":Int32", TermID::TYPE);
    result->push_back(Term::CreateName("argv"));
    result->back().second->m_type = Term::CreateName(":StrChar", TermID::TYPE);
    result->push_back(Term::CreateName("penv"));
    result->back().second->m_type = Term::CreateName(":StrChar", TermID::TYPE);
    return result;
}

std::string JIT::MakeApplicationSource(const TermPtr &ast) {
    std::string result;
    if (!ast->isBlock() && ast->m_id == TermID::EMBED) {
        std::string source = ReplaceObjectInEmbedSource(ast->m_text, m_includes, MainArgs());
        if (source.compare(ast->m_text) == 0) {
            // Simple embedded (no libnlc-rt)
            result = MakeMainEmbed(source, m_includes);
        } else {
            // Need link libnlc-rt
            if (!m_link_rt) {
                LOG_RUNTIME("Options '--nlc-no-link-rt' not allowed!");
            }
            result = MakeMain(m_includes);
        }
    } else {
        if (!m_link_rt) {
            LOG_RUNTIME("Options '--nlc-no-link-rt' not allowed!");
        }
        //        if (!m_rt->m_link_rt || !m_rt->m_link_jit) {
        //            LOG_RUNTIME("Options '--nlc-no-link-rt' or '--nlc-no-link-jit' not allowed!");
        //        }
        // Need link libnlc-rt   and   libnlc-jit
        LOG_RUNTIME("Not implemented!!!");
    }
    return result;
}

std::string JIT::MakeMain(const std::vector<std::string> &include) {
    std::string result;
    result += NEWLANG_INDENT_OP NEWLANG_INDENT_OP "return newlang::RunMain(argc, argv, penv);";
    return MakeMainEmbed(result, include);
}

std::string JIT::MakeMainEmbed(const std::string_view embed_source, const std::vector<std::string> &include) {
    std::string result;
    result += "/*\n* Generate NewLang JIT::MakeMainEmbed";
    result += "\n* " VERSION_SOURCE_FULL_ID;
    result += "\n* at: ";
    result += Parser::GetCurrentTimeStamp();
    result += "\n*/\n\n";

    std::string filename;
    for (auto &elem : include) {
        result += "#include ";
        if (elem.find("\\") == 0) {
            result += "<>\n";
            filename = elem.substr(1);
        } else {
            result += "\"\"\n";
            filename = elem;
        }
        //        filename = std::regex_replace(filename, std::regex("\\\\\\\\"), "\\");
        std::replace(filename.begin(), filename.end(), '\\', '/');
        result = result.insert(result.size() - 2, filename);
    }
    result += "\n\nnamespace newlang {\n";
    result += NEWLANG_INDENT_OP "int RunMain(const int argc, const char** argv, const char** penv);\n";
    result += "}\n\n";
    result += "int main(const int argc, const char* argv[], const char* penv[]) {\n\n";
    result += NEWLANG_INDENT_OP NEWLANG_INDENT_OP "// disable unused warning\n";
    result += NEWLANG_INDENT_OP NEWLANG_INDENT_OP "((void)(argc)); ((void)(argv)); ((void)(penv));\n\n";
    result += embed_source;
    result += NEWLANG_INDENT_OP NEWLANG_INDENT_OP "\n" NEWLANG_INDENT_OP NEWLANG_INDENT_OP "return 0;\n};\n\n";
    return result;
}

/*
 *  @\\stdio.h     -->  #include <stdio.h>
 *  @\sys\time.h   -->  #include "sys/time.h"
 */
std::string JIT::ReplaceObjectInEmbedSource(const std::string_view embed, std::vector<std::string> &include, const TermPtr args) {
    std::string result(embed);

    size_t pos = result.find("@\\");
    while (pos != std::string::npos) {
        include.push_back(result.substr(pos + 2, result.find_first_of(" \t\n\r*/$#\"'[]%;:", pos + 2) - pos - 2));
        pos = result.find("@\\", pos + 2);
    }

    if (args) {
        std::string arg_name;
        std::string arg_place;
        for (size_t i = 0; i < args->size(); i++) {
            // Заменить номер аргумента
            arg_name = "@$" + std::to_string(i);
            arg_place = "in[" + std::to_string(i) + "]";
            result = std::regex_replace(result, std::regex(arg_name), arg_place);
        }

        for (auto &elem : *args) {
            // Заменить имя аргумента
            arg_name = MakeName(elem.first);
            arg_place = "in[\"" + arg_name + "\"]";
            arg_name = "@$" + arg_name;
            result = std::regex_replace(result, std::regex(arg_name), arg_place);
        }
    }
    return result;
}

std::string JIT::MakeCodeModule(const TermPtr &ast, const std::string_view module, bool is_main) {
    std::string result;
    for (size_t i = 0; i < newlang_include_h_i_size; i++) {
        result += newlang_include_h_i_arr[i];
        result += "\n";
    }

    result += "/*\n* Generate NewLang JIT::MakeCodeModule";
    result += "\n* " VERSION_SOURCE_FULL_ID;
    result += "\n* at: ";
    result += Parser::GetCurrentTimeStamp();
    result += "\n*/\n\n";

    //    result += "#include <memory>\n";
    //    result += "\n";
    //    
    //    result += "namespace newlang {\n";
    //    result += "  class Obj;\n";
    //    result += "  class Context;\n";
    //    result += "  typedef std::shared_ptr<Obj> ObjPtr;\n";
    //    result += "};\n\n";

    result += ExtractFunctionDecls(ast, module);

    result += ExtractStaticVars(ast, module);

    TermPtr func_module = Term::CreateName("__module__");
    func_module->m_is_call = true;
    func_module->m_int_name = NormalizeName(func_module->m_text);

    result += MakeFunctionPrototype(func_module, module);
    result += " {\n";
    result += MakeBodyFunction(ast);
    result += "  return nullptr;\n";
    result += "}\n";

    MakeFunctionRecursive_(ast, result, module);

    result += "/*\n* End generate JIT::MakeCodeModule";
    result += "\n* " VERSION_SOURCE_FULL_ID;
    result += "\n* at: ";
    result += Parser::GetCurrentTimeStamp();
    result += "\n*/\n\n";
    return result;
}

std::string JIT::MakeCodeRepl(const std::string_view source, const std::string_view func_name) {

    TermPtr ast = MakeAst(source);

    std::string result;
    for (size_t i = 0; i < newlang_include_h_i_size; i++) {
        result += newlang_include_h_i_arr[i];
        result += "\n";
    }

    result += "/*\n* Generate NewLang JIT::MakeCodeRepl";
    result += "\n* " VERSION_SOURCE_FULL_ID;
    result += "\n* at: ";
    result += Parser::GetCurrentTimeStamp();
    result += "\n* from source:\n*  ";
    result += RegExpInlineComment(source);
    result += "\n*/\n\n";

    result += "using namespace newlang;\n\n";

    //    //    result += ExtractFunctionDecls(ast, module);
    //    //    result += ExtractStaticVars(ast, module);
    //
    //    TermPtr func_module = Term::CreateName(func_name);
    //    func_module->m_is_call = true;
    //    func_module->m_int_name = NormalizeName(func_module->m_text);
    //
    //    result += MakeFunctionPrototype(func_module, module);
    //    result += " {\n";
    //    result += MakeBodyFunction(ast);
    //    result += "  return JitLastResult(nullptr);\n";
    //    result += "}\n";
    //
    //    MakeFunctionRecursive_(ast, result, module);
    //
    //    result += "/*\n* End generate NewLang JIT::MakeCodeRepl";
    //    result += "\n* " VERSION_SOURCE_FULL_ID;
    //    result += "\n* at: ";
    //    result += Parser::GetCurrentTimeStamp();
    //    result += "\n* from source:\n*  ";
    //    result += RegExpInlineComment(source);
    //    result += "\n*/\n\n";
    return result;
}

std::string JIT::ExtractStaticVars(const TermPtr &ast, const std::string_view module) {
    return "";
}

std::string JIT::MakeCodeFunction(const TermPtr &term) {
    return "";
}

std::string JIT::MakeFuncDeclarations_(const TermPtr &term) {
    return "";
}

std::string JIT::MakeBodyFunction(const TermPtr &ast) {
    return "";
}

void JIT::MakeFunctionRecursive_(const TermPtr &term, std::string &output, const std::string_view module) {
    //    std::string result;
    if (term->isBlock()) {
        for (auto &elem : term->m_block) {
            MakeFunctionRecursive_(elem, output, module);
        }
    } else {
        if (term->isCreate()) {
            if (term->m_left->isCall()) { // Function
                output += "// ";
                output += MakeCommentPlace(term);
                output += "\n\n";
                output += "extern \"C\" ";
                output += MakeFunctionPrototype(term->m_left, module);
                output += " {\n";
                output += MakeBodyFunction(term->m_right);
                output += "}\n";
            }
        }
    }
    //    return result;
}

std::string CompileInfo::GetIndent(int64_t offset) {
    return repeat(std::string(NEWLANG_INDENT_OP), indent + offset);
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
//
//llvm::ExecutionEngine * Compiler::JITCompileCPP(const char* source, const char *file_name) {
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
//Compiler::Compiler(RuntimePtr rt) : m_rt(rt) {
//    if (!m_rt) {
//        m_rt = RunTime::Init();
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

LLVMGenericValueRef GetGenericValueRef(Obj &obj, LLVMTypeRef type) {
    if (type == LLVMInt1Type() || type == LLVMInt8Type() || type == LLVMInt16Type() || type == LLVMInt32Type() || type == LLVMInt64Type()) {
        return LLVMCreateGenericValueOfInt(type, obj.GetValueAsInteger(), true);
    } else if (type == LLVMFloatType() || type == LLVMDoubleType()) {
        return LLVMCreateGenericValueOfFloat(type, obj.GetValueAsNumber());
    } else if (type == LLVMPointerType(LLVMInt8Type(), 0)) {
        if (obj.getType() == ObjType::StrChar || obj.getType() == ObjType::FmtChar) {
            return LLVMCreateGenericValueOfPointer((void *) obj.m_value.c_str());
        } else if (obj.getType() == ObjType::Pointer) {
            ASSERT(std::holds_alternative<void *>(obj.m_var));
            return LLVMCreateGenericValueOfPointer(std::get<void *>(obj.m_var));
        }
    } else if (type == LLVMPointerType(LLVMInt32Type(), 0)) {
        if (obj.getType() == ObjType::StrWide || obj.getType() == ObjType::FmtWide) {

            return LLVMCreateGenericValueOfPointer((void *) obj.m_string.c_str());
        }
    }
    LOG_RUNTIME("Not support LLVM type '%s'", newlang::toString(obj.m_var_type_current));
}

ObjPtr CreateFromGenericValue(ObjType type, LLVMGenericValueRef ref, LLVMTypeRef llvm_type) {
    if (type == ObjType::None) {
        return Obj::CreateNone();
    } else if (isIntegralType(type, true)) {
        int64_t value = LLVMGenericValueToInt(ref, true);
        unsigned width = LLVMGenericValueIntWidth(ref);
        return Obj::CreateValue(value, type);
    } else if (isFloatingType(type)) {
        return Obj::CreateValue((double) LLVMGenericValueToFloat(llvm_type, ref), type);
    } else if (type == ObjType::StrChar || type == ObjType::FmtChar) {
        if (llvm_type == LLVMInt8Type()) {
            return Obj::CreateString(std::string(1, (char) LLVMGenericValueToInt(ref, false)));
        } else {
            //@todo Нужна проверка на тип LLVM данных?
            return Obj::CreateString(std::string((const char *) LLVMGenericValueToPointer(ref)));
        }
    } else if (type == ObjType::StrWide || type == ObjType::FmtWide) {
        if (llvm_type == LLVMInt32Type()) {
            return Obj::CreateString(std::wstring(1, (wchar_t) LLVMGenericValueToInt(ref, false)));
        } else {
            //@todo Нужна проверка на тип LLVM данных?
            return Obj::CreateString(std::wstring((const wchar_t *)LLVMGenericValueToPointer(ref)));
        }
    } else if (type == ObjType::Pointer) {

        ObjPtr result = Obj::CreateType(type, ObjType::None, true);
        result->m_var = LLVMGenericValueToPointer(ref);

        return result;
    }
    LOG_RUNTIME("Create to type '%s' form LLVM type not implemented!", newlang::toString(type));
}

LLVMTypeRef toLLVMType(ObjType t, bool none_if_error) {
    switch (t) {
        case ObjType::None:
            return LLVMVoidType();
        case ObjType::Bool:
            return LLVMInt1Type();
        case ObjType::Int8:
        case ObjType::Char:
        case ObjType::Byte:
            return LLVMInt8Type();
        case ObjType::Int16:
        case ObjType::Word:
            return LLVMInt16Type();
        case ObjType::Int32:
        case ObjType::DWord:
            return LLVMInt32Type();
        case ObjType::Int64:
        case ObjType::DWord64:
        case ObjType::Integer:
            return LLVMInt64Type();
        case ObjType::Float32:
        case ObjType::Single:
        case ObjType::Tensor:
            return LLVMFloatType();
        case ObjType::Float64:
        case ObjType::Double:
        case ObjType::Number:
            return LLVMDoubleType();

        case ObjType::Pointer:
        case ObjType::StrChar:
        case ObjType::FmtChar:
            return LLVMPointerType(LLVMInt8Type(), 0);

#ifdef _MSC_VER                
        case ObjType::StrWide:
        case ObjType::FmtWide:
            return LLVMPointerType(LLVMInt16Type(), 0);
#else
        case ObjType::StrWide:
        case ObjType::FmtWide:
            return LLVMPointerType(LLVMInt32Type(), 0);
#endif

    }
    LOG_RUNTIME("Can`t convert type '%s' to LLVM type!", toString(t));
}

ParserPtr JIT::GetParser() {
    // @todo Сделать корректныую очистку состояния парсера???
    return std::make_shared<JitParser>(m_macro, nullptr, m_diag, true, this);
}

ObjPtr JIT::RunFile(std::string file, Obj* args) {
    if (!RunTime::ExpandFileName(file)) {
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

/*
 * Построчное выполнение не изменяет AST, а только добавляет к нему новые строки.
 * Констекст выполнения (m_main_runner) тоже остается не изменным.
 */
ObjPtr JIT::Run(const std::string_view str, Obj* args) {

    if (m_module.empty()) {
        m_main_ast = Term::Create(TermID::SEQUENCE, "");
        m_module.push_back(std::make_shared<Module>(this, m_main_ast));
    }
    if (m_ctx.empty()) {
        m_ctx.push_back(std::make_shared<Context>(*m_module.begin()->get(), this));
    }

    TermPtr temp = GetParser()->Parse(str.begin());
    ASSERT((*m_module.begin())->m_ast);
    (*m_module.begin())->m_ast->m_block.push_back(temp);


    try {
        m_diag->m_error_count = 0;

        AstAnalysis analysis(*this, m_diag.get());

        analysis.Analyze((*m_module.begin())->m_ast->m_block.back(), GetAst());

        if (m_diag->m_error_count) {
            LOG_PARSER("fatal error: %d generated. ", m_diag->m_error_count);
        }

        return Context::Execute((*m_module.begin())->m_ast->m_block.back(), &GetCtx());

    } catch (...) {
        (*m_module.begin())->m_ast->m_block.pop_back();

        throw;
    }
    return m_latter;
}

ObjPtr JIT::Run(Module *module, Obj* args) {
    LOG_RUNTIME("Run module not implemented!");
    return nullptr;
}

TermPtr JIT::MakeAstParser(const std::string_view src, bool skip_analize) {
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

TermPtr JIT::MakeAst(const std::string_view src, bool skip_analize) {
    TermPtr ast = Parser::ParseString(src.begin());
    if (skip_analize) {
        return ast;
    }

    AstAnalysis analysis(*this, m_diag.get());

    if (!analysis.Analyze(ast, ast)) {
        LOG_RUNTIME("Make AST fail!");
    }
    return ast;
}

bool JIT::ModuleCreate(FileModule &data, const std::string_view source) {

    TermPtr ast = MakeAst(source, false);
    return true;
}

bool JIT::ModuleCreate(FileModule &data, const std::string_view module_name, const TermPtr &include, const std::string_view source, llvm::Module *bc) {

    data.name.assign(module_name);
    data.include = AstAnalysis::MakeInclude(include);
    data.source = source;
    if (bc) {
        llvm::raw_string_ostream code(data.bytecode);
        llvm::WriteBitcodeToFile(*bc, code);
    }
    return !data.name.empty() && !data.include.empty() && (data.source.empty() || data.bytecode.empty());
}

JIT::JIT(const StringArray &args) : RunTime(args), m_macro(std::make_shared<Macro>()) { //m_rt(rt), 

    LLVMInitialize();

    m_repl_count = 0;
    // Диагностика работы Clang
    DiagOpts = new clang::DiagnosticOptions;
    textDiagPrinter = new clang::TextDiagnosticPrinter(llvm::outs(), &*DiagOpts);
    pDiagnosticsEngine = new clang::DiagnosticsEngine(pDiagIDs, &*DiagOpts, textDiagPrinter);


    if (m_load_dsl) {

        VERIFY(CreateMacro("@@ true @@ ::= 1"));
        VERIFY(CreateMacro("@@ yes @@ ::= 1"));
        VERIFY(CreateMacro("@@ false @@ ::= 0"));
        VERIFY(CreateMacro("@@ no @@ ::= 0"));

        VERIFY(CreateMacro("@@ if( ... ) @@ ::= @@ [ @$... ] --> @@"));
        VERIFY(CreateMacro("@@ elif( ... ) @@ ::= @@ ,[ @$... ] --> @@"));
        VERIFY(CreateMacro("@@ else @@ ::= @@ ,[...] --> @@"));

        VERIFY(CreateMacro("@@ while( ... ) @@ ::= @@ [ @$... ] <-> @@"));
        VERIFY(CreateMacro("@@ dowhile( ... ) @@ ::= @@ <-> [ @$... ] @@"));
        VERIFY(CreateMacro("@@ loop @@ ::= @@ [ 1 ] <-> @@"));


        VERIFY(CreateMacro("@@ break $label @@ ::= @@ @$label :: ++ @@"));
        VERIFY(CreateMacro("@@ continue $label @@ ::= @@ @$label :: -- @@"));
        VERIFY(CreateMacro("@@ return $... @@ ::= @@ $:: ++ @$... ++ @@"));
        VERIFY(CreateMacro("@@ throw $... @@ ::= @@ -- @$... -- @@"));

        VERIFY(CreateMacro("@@ match( ... ) @@ ::= @@ [ @$... ] @__PRAGMA_EXPECTED__( @\\ =>, @\\ ==>, @\\ ===>, @\\ ~>, @\\ ~~>, @\\ ~~~> ) @@"));
        VERIFY(CreateMacro("@@ case( ... ) @@ ::= @@ [ @$... ] --> @@"));
        VERIFY(CreateMacro("@@ default @@ ::= @@ [...] --> @@"));


        VERIFY(CreateMacro("@@ this @@ ::= @@ $0 @@ ##< This object (self)"));
        VERIFY(CreateMacro("@@ self @@ ::= @@ $0 @@ ##< This object (self)"));
        VERIFY(CreateMacro("@@ super @@ ::= @@ $$ @@ ##< Super (parent) class or function"));
        VERIFY(CreateMacro("@@ latter @@ ::= @@ $^ @@  ##< Result of the last operation"));


        VERIFY(CreateMacro("@@ try @@ ::= @@ [ {*  @__PRAGMA_EXPECTED__( @\\ { ) @@"));
        VERIFY(CreateMacro("@@ catch(...) @@ ::= @@ *} ] : < @$... > ~> @@"));
        VERIFY(CreateMacro("@@ forward @@ ::= @@ +- $^ -+ @@  ##< Forward latter result or exception"));


        VERIFY(CreateMacro("@@ iter( obj, ... ) @@ ::= @@ @$obj ? (@$...) @@"));
        VERIFY(CreateMacro("@@ next( obj, ... ) @@ ::= @@ @$obj ! (@$...) @@"));
        VERIFY(CreateMacro("@@ curr( obj ) @@ ::= @@ @$obj !? @@"));
        VERIFY(CreateMacro("@@ first( obj ) @@ ::= @@ @$obj !! @@"));
        VERIFY(CreateMacro("@@ all( obj ) @@ ::= @@ @$obj ?? @@"));

        VERIFY(CreateMacro("@@ and @@ ::= @@ && @@"));
        VERIFY(CreateMacro("@@ or @@ ::= @@ || @@"));
        VERIFY(CreateMacro("@@ xor @@ ::= @@ ^^ @@"));
        VERIFY(CreateMacro("@@ not(value) @@ ::= @@ (:Bool(@$value)==0) @@"));

        //                    VERIFY(CreateMacro("@@ root() @@ ::= @@ @# @\\\\ @@"));
        //                    VERIFY(CreateMacro("@@ module() @@ ::= @@ @# $\\\\ @@"));

        //                    VERIFY(CreateMacro("@@ namespace() @@ ::= @@ @# @:: @@"));
        VERIFY(CreateMacro("@@ module() @@ ::= @@ @$$ @@"));
        VERIFY(CreateMacro("@@ static @@ ::= @@ @:: @@"));
        VERIFY(CreateMacro("@@ package $name @@ ::= @@  @$$ = @# @$name @@"));
        VERIFY(CreateMacro("@@ declare( obj ) @@ ::= @@ @$obj ::= ... @@  ##< Forward declaration of the object"));

        VERIFY(CreateMacro("@@ using(...) @@ ::= @@ ... = @$... @@"));

        VERIFY(CreateMacro("@@ typedef(cnt) @@ ::= @@ @__PRAGMA_TYPE_DEFINE__(@$cnt) @@ ##< Disable warning when defining a type inside a namespace"));

        VERIFY(CreateMacro("@@ coroutine @@ ::= @@ __ANNOTATION_SET__(coroutine) @@"));
        VERIFY(CreateMacro("@@ co_yield  $val  @@ ::= @@ __ANNOTATION_CHECK__(coroutine) $:: :: -- @$val -- @@"));
        VERIFY(CreateMacro("@@ co_await        @@ ::= @@ __ANNOTATION_CHECK__(coroutine) $:: :: +- @@"));
        VERIFY(CreateMacro("@@ co_return $val  @@ ::= @@ __ANNOTATION_CHECK__(coroutine) $:: :: ++ @$val ++ @@"));

        VERIFY(CreateMacro("@@ exit(code) @@ ::= @@ :: ++ @$code ++ @@"));
        VERIFY(CreateMacro("@@ abort() @@ ::= @@ :: -- @@"));

/*
 * @time call(123);
 *  {
 *      __start_point__ := ::Base::microsec_point();
 *      call(123);
 *      ::Base::microsec_point( __start_point__ );
 *  }
 */
        VERIFY(CreateMacro("@@ timeit( call, ... ) @@ ::= @@ { __start_point__ := ::Base::__timeit__();  @$call ;  ::Base::__timeit__( __start_point__, @# @$call, @$... ); } @@  ##< Measure execution time of function call"));
        
    }


    // @assert(value, ...)
    // @static_assert(value, ...)
    // @verify(value, ...)
    //
    // @__PRAGMA_ASSERT__( is_runtime, is_always, value, val_string, ... )
    //
    // @@ static_assert(value, ...) @@ ::= @@ @__PRAGMA_ASSERT__(0, 0, value, @# value, @$... ) @@;
    // @@ assert(value, ...) @@ ::= @@ @__PRAGMA_ASSERT__(1, 0, value, @# value, @$... ) @@;
    // @@ verify(value, ...) @@ ::= @@ @__PRAGMA_ASSERT__(1, 1, value, @# value, @$... ) @@;

    //    // @__PRAGMA_ASSERT__ replase to __pragma_assert__ in @ref Parser::PragmaEval
    //    // @__PRAGMA_STATIC_ASSERT__
    //    CALSS_METHOD(Base, __assert_abort__);
    //    VERIFY(CreateMethod("__assert_abort__(...):None", __assert_abort__));

    VERIFY(CreateMacro("@@ static_assert(...) @@ ::= @@ @__PRAGMA_STATIC_ASSERT__(@$... ) @@"));

    if (m_assert_enable) {
        VERIFY(CreateMacro("@@ assert(value, ...) @@ ::= @@ [:Bool(@$value)==0]-->{ ::Base::__assert_abort__(@# @$value, @$value, @$... ) } @@"));
        VERIFY(CreateMacro("@@ verify(value, ...) @@ ::= @@ [:Bool(@$value)==0]-->{ ::Base::__assert_abort__(@# @$value, @$value, @$... ) } @@"));
    } else {
        VERIFY(CreateMacro("@@ assert(value, ...) @@ ::= @@ (_) @@"));
        VERIFY(CreateMacro("@@ verify(value, ...) @@ ::= @@ (@$value) @@"));
    }
}

ObjPtr newlang::JitLastResult(ObjPtr val) {
    JIT * jit = JIT::Instance();
    if (val) {
        jit->m_latter = val;
    }
    return jit->m_latter;
}

bool JIT::CreateMacro(const std::string_view text) {
    TermPtr m = Parser::ParseTerm(text.begin(), m_macro, false);
    return true;
}

/* Override *weak* Logger method for print call stack by PrintStackTrace */
std::string Logger::GetStackTrace() {
    std::string result("\n");
    llvm::raw_string_ostream stack(result);
    llvm::sys::PrintStackTrace(stack, 1);
    return result;
}

TermPtr JitParser::ParseFile(const std::string_view filename) {

    llvm::SmallVector<char> path;
    if (!llvm::sys::fs::real_path(filename, path)) {
        m_filename = llvm::StringRef(path.data(), path.size());
    } else {
        m_filename = filename;
    }

    llvm::sys::fs::file_status fs;
    std::error_code ec = llvm::sys::fs::status(filename, fs);
    if (!ec) {
        time_t temp = llvm::sys::toTimeT(fs.getLastModificationTime());
        struct tm * timeinfo;
        timeinfo = localtime(&temp);
        m_file_time = asctime(timeinfo);
        m_file_time = m_file_time.substr(0, 24); // Remove \n on the end line
    }

    auto md5 = llvm::sys::fs::md5_contents(filename);
    if (md5) {
        llvm::SmallString<32> hash;
        llvm::MD5::stringifyResult(*md5, hash);
        m_file_md5 = hash.c_str();
    }

    int fd = open(filename.begin(), O_RDONLY);
    if (fd < 0) {
        LOG_RUNTIME("Error open file '%s'", filename.begin());
    }

    struct stat sb;
    fstat(fd, &sb);

    std::string data;
    data.resize(sb.st_size);

    read(fd, const_cast<char*> (data.data()), sb.st_size);
    close(fd);

    return Parse(data);
}

TermPtr JitParser::LoadIfModule(const TermPtr & term) {
    if (term->m_id == TermID::MODULE) {
        NL_PARSER(term, "Fail load module '%s'!", term->toString().c_str());
    }
    return term;
}

TermPtr JitParser::CheckLoadModule(const TermPtr & term) {
    if (!CheckCharModuleName(term->m_text.c_str())) {
        NL_PARSER(term, "Module name - backslash, underscore, lowercase English letters or number!");
    }
    //    if (m_rt && !m_rt->CheckLoadModule(term)) {
    NL_PARSER(term, "Fail load module '%s'!", term->toString().c_str());
    //    }
    return term;
}

void * RunTime::GetNativeAddress(void * handle, const std::string_view name) {
    LLVMLoadLibraryPermanently(nullptr);
    return LLVMSearchForAddressOfSymbol(name.begin());
}

/*
 * 
 * 
 * 
 * 
 *  
 */

//TermPtr JIT::GetObject(const std::string_view int_name) {
//
//    //    std::string int_name;
//    //    if (isInternalName(int_name)) {
//    //    }
//    //    int_name = NormalizeName(int_name);
//    //    ASSERT();
//    //
//    //    std::string int_name;
//    //    
//    //    if (term->isNone()) {
//    //        term->m_int_name = "_";
//    //    }
//    if (int_name.empty()) {
//        LOG_RUNTIME("Has no internal name! AST analysis required!");
//    }
//
//    TermPtr result;
//    if (int_name.compare("_") == 0) {
//        result = Term::CreateNone();
//    } else if (int_name.compare("$^") == 0) {
//        result = Term::CreateNone();
//        result->m_obj = m_latter;
//    } else {
//        auto found = find(int_name.begin());
//        if(found != end()){
//            result = found->second.item;
//        }
////        result = FindInternalName(int_name, m_rt.get());
//        if (!result && m_rt && (isGlobalScope(int_name) || isTypeName(int_name))) {
//            result = m_rt->GlobFindProto(int_name);
//        }
//    }
//    if (!result) {
//        //#ifdef BUILD_UNITTEST
//        //            if (term->m_text.compare("__STAT_RUNTIME_UNITTEST__") == 0) {
//        //                ASSERT(term->isCall());
//        //                ASSERT(term->size() == 2);
//        //                return Obj::CreateValue(m_rt->__STAT_RUNTIME_UNITTEST__(
//        //                        parseInteger(term->at(0).second->m_text.c_str()),
//        //                        parseInteger(term->at(1).second->m_text.c_str())));
//        //            }
//        //#endif
//        LOG_RUNTIME("Object with internal name '%s' not found!", int_name.begin());
//    }
//
//    return result;
//}
//
//ObjPtr JIT::EvalCreate_(TermPtr & op) {
//    ASSERT(op);
//    ASSERT(op->isCreate());
//    ASSERT(op->m_left);
//
//    ArrayTermType l_vars = op->m_left->CreateArrayFromList();
//
//    bool is_ellipsis = false;
//    TermPtr var_found;
//    for (auto &elem : l_vars) {
//
//        if (elem->getTermID() == TermID::ELLIPSIS) {
//
//            //@todo добавить поддержку многоточия с левой стороный оператора присвоения
//            // NL_PARSER(elem, "Ellipsis on the left side in assignment not implemented!");
//
//            //  Игнорировать несколько объектов
//            elem->m_obj = getEllipsysObj();
//            if (is_ellipsis) {
//                NL_PARSER(elem, "Multiple ellipsis on the left side of the assignment!");
//            }
//            is_ellipsis = true;
//
//        } else if (elem->isNone()) {
//
//            //  Игнорировать один объект
//            elem->m_obj = getNoneObj();
//
//        } else {
//
//            var_found = GetObject(elem->m_int_name);
//
//            if (op->isCreateOnce() && var_found && var_found->m_obj) {
//                NL_PARSER(elem, "Object '%s' already exist", elem->m_text.c_str());
//            } else if (op->getTermID() == TermID::ASSIGN && !(var_found && var_found->m_obj)) {
//                NL_PARSER(elem, "Object '%s' not exist!", elem->m_text.c_str());
//            }
//
//            if (var_found) {
//                if (var_found->isCall() && var_found->m_obj) {
//                    NL_PARSER(elem, "The function cannot be overridden! '%s'", var_found->toString().c_str());
//                }
//                elem = var_found;
//                //                elem->m_int_name = var_found->m_int_name;
//                //                elem->m_obj = var_found->m_obj;
//                //                LOG_TEST("0: %s = %s (%p)", elem->m_text.c_str(), elem->m_obj ? elem->m_obj->toString().c_str() : "nullptr", elem->m_obj.get());
//            }
//        }
//    }
//
//    m_latter = AssignVars_(l_vars, op->m_right, op->isPure());
//    for (auto &elem : l_vars) {
//
//        //        LOG_TEST("4: %s = %s (%p)", elem->m_text.c_str(), elem->m_obj ? elem->m_obj->toString().c_str() : "nullptr", elem->m_obj.get());
//
//        if (isGlobalScope(elem->m_int_name)) {
//            m_rt->NameRegister(op->isCreateOnce(), elem->m_int_name.c_str(), elem, elem->m_obj);
//        }
//    }
//    return m_latter;
//}
//
//ObjPtr JIT::AssignVars_(ArrayTermType &l_vars, const TermPtr &r_term, bool is_pure) {
//
//    ASSERT(l_vars.size());
//    if (r_term->getTermID() == TermID::NATIVE) {
//
//        ASSERT(l_vars.size() == 1);
//        //        ASSERT(vars[0]->m_obj);
//        //        ASSERT(r_term->m_obj);
//
//        l_vars[0]->m_obj = m_rt->CreateNative(l_vars[0], nullptr, false, r_term->m_text.substr(1).c_str());
//        //        vars[0]->m_obj = r_term->m_obj;
//        m_latter = l_vars[0]->m_obj;
//
//    } else if (r_term->getTermID() == TermID::ELLIPSIS) {
//        // При раскрытии словаря присвоить значение можно как одному, так и сразу нескольким терминам: 
//        // var1, var2, _ = ... func(); Первый и второй элементы записывается в var1 и var2, 
//        // а все остальные игнорируются (если они есть)
//        // var1, var2 = ... func(); Если функция вернула словарь с двумя элементами, 
//        // то их значения записываются в var1 и var2. Если в словаре было больше двух элементов, 
//        // то первый записывается в var1, а все оставшиеся элементы в var2. !!!!!!!!!!!!!
//        // item, dict = ... dict; Первый элементы записывается в item и удаялется из dict 
//
//        // _, var1, ..., var2 = ... func(); 
//        // Первый элемент словаря игнорируется, второй записывается в var1, а последний в var2.
//
//        //@todo добавить поддержку многоточия с левой стороный оператора присвоения
//
//        bool is_named = !!r_term->m_left;
//        if (is_named) {
//            NL_PARSER(r_term, "Named ELLIPSIS NOT implemented!");
//            ASSERT(r_term->m_left->getTermID() == TermID::ELLIPSIS);
//        }
//
//        bool is_last = false;
//        ObjPtr right_obj = CheckObjTerm_(r_term->m_right, this);
//        for (size_t i = 0; i < l_vars.size(); i++) {
//            if (l_vars[i]->isNone()) {
//                // Ignore position
//                continue;
//            }
//            if (i + 1 == l_vars.size()) {
//                is_last = true;
//            }
//
//            //            LOG_TEST("1: %s = %s (%p)", l_vars[i]->m_text.c_str(), l_vars[i]->m_obj ? l_vars[i]->m_obj->toString().c_str() : "nullptr", l_vars[i]->m_obj.get());
//
//            if (i < right_obj->size()) {
//                if (is_last) {
//                    if (l_vars[i]->m_int_name.compare(r_term->m_right->m_int_name) == 0) {
//                        // Остаток элементов присваивается тому же словарю
//                        l_vars[i]->m_obj = right_obj;
//                    } else {
//                        l_vars[i]->m_obj = right_obj->Clone();
//                    }
//                    l_vars[i]->m_obj->erase(0, l_vars.size() - 1);
//                } else {
//                    l_vars[i]->m_obj = (*right_obj)[i].second;
//                }
//            } else {
//                if (is_last) {
//                    if (l_vars[i]->m_int_name.compare(r_term->m_right->m_int_name) == 0) {
//                        // Остаток элементов присваивается тому же словарю
//                        l_vars[i]->m_obj = right_obj;
//                    } else {
//                        l_vars[i]->m_obj = right_obj->Clone();
//                    }
//                    l_vars[i]->m_obj->erase(0, 0);
//                } else {
//                    l_vars[i]->m_obj = Obj::CreateNone();
//                }
//            }
//            //            LOG_TEST("2: %s = %s (%p)", l_vars[i]->m_text.c_str(), l_vars[i]->m_obj ? l_vars[i]->m_obj->toString().c_str() : "nullptr", l_vars[i]->m_obj.get());
//        }
//        m_latter = l_vars[l_vars.size() - 1]->m_obj;
//
//    } else if (r_term->getTermID() == TermID::FILLING) {
//        // Заполнение переменных значениями последовательным вызовом фукнции?
//
//        //@todo добавить поддержку многоточия с левой стороный оператора присвоения
//        NL_PARSER(r_term, "FILLING NOT implemented!");
//
//    } else if (r_term->isBlock()) {
//
//        if (l_vars.size() > 1) {
//            //@todo добавить поддержку присвоения сразу нескольким функциям
//            NL_PARSER(r_term, "Multiple function assignment not implemented!");
//        }
//        ASSERT(l_vars.size() == 1);
//
//        if (!l_vars[0]->isCall()) {
//            NL_PARSER(l_vars[0], "Function name expected!");
//        }
//
//        ASSERT(!l_vars[0]->m_right);
//
//        l_vars[0]->m_obj = m_rt->CreateFunction(l_vars[0], r_term);
//        m_latter = l_vars[0]->m_obj;
//
//    } else {
//
//        ArrayTermType r_vars = r_term->CreateArrayFromList();
//        if (r_vars.empty()) {
//            // Delete vars
//            NL_PARSER(r_term, "NOT implemented!");
//            //            m_latter = getNoneObj();
//
//        } else if (r_vars.size() == 1) {
//
//            m_latter = Run(r_vars[0], this);
//            for (auto &elem : l_vars) {
//                if (elem->getTermID() == TermID::ELLIPSIS) {
//                    NL_PARSER(elem, "Ellipses unexpected!");
//                } else if (elem->isNone()) {
//                    NL_PARSER(elem, "None var unexpected!");
//                } else if (elem->m_right) {
//
//                    if (elem->m_right->m_id == TermID::INDEX) {
//                        SetIndexValue(elem, m_latter, this);
//                    } else if (elem->m_right->m_id == TermID::FIELD) {
//                        SetFieldValue(elem, m_latter, this);
//                    } else {
//                        NL_PARSER(elem, "Eval type '%s' not implemented!", toString(elem->m_right->m_id));
//                    }
//
//                } else {
//                    elem->m_obj = m_latter;
//                }
//            }
//
//        } else {
//            // Что присваиваем (правая часть выражения)
//            // @todo В будущем можно будет сделать сахар для обмена значениями при одинаковом кол-ве объектов у оператора присваивания
//            // a, b = b, a;   a, b, c = c, b, a; и т.д.
//
//            if (l_vars.size() != r_vars.size()) {
//                NL_PARSER(r_term, "Fail count right values! Expected one or %d.", (int) l_vars.size());
//            }
//            for (size_t i = 0; i < l_vars.size(); i++) {
//                if (l_vars[i]->getTermID() == TermID::ELLIPSIS) {
//                    NL_PARSER(l_vars[i], "Ellipses unexpected!");
//                } else if (l_vars[i]->isNone()) {
//                    NL_PARSER(l_vars[i], "None var unexpected!");
//                } else {
//
//                    m_latter = Run(r_vars[i], this);
//                    l_vars[i]->m_obj = m_latter;
//                }
//            }
//        }
//    }
//
//    return m_latter;
//
//
//
//}
//
//ObjPtr JIT::SetIndexValue(TermPtr &term, ObjPtr &value, Context * runner) {
//
//    ASSERT(term->m_right);
//
//    TermPtr index = term->m_right;
//    ASSERT(index->size());
//
//    switch (term->m_id) {
//        case TermID::DICT:
//        case TermID::STRCHAR:
//        case TermID::STRWIDE:
//            if (index->size() > 1) {
//                NL_PARSER(index, "Strings and dictionaries do not support multiple dimensions!");
//            }
//        case TermID::TENSOR:
//            break;
//
//        case TermID::NAME:
//        case TermID::LOCAL:
//        case TermID::TYPE:
//        case TermID::ARGS:
//        case TermID::ARGUMENT:
//        case TermID::STATIC:
//            break;
//
//        default:
//            NL_PARSER(term, "Term type '%s' not indexable!", toString(term->m_id));
//    }
//
//    ObjPtr obj = CheckObjTerm_(term, runner, false);
//    ASSERT(obj);
//    ObjPtr ind = CheckObjTerm_(index->at(0).second, runner);
//    ASSERT(ind);
//
//    if (isStringChar(obj->getType())) {
//
//        if (canCast(value->getType(), ObjType::Int8)) {
//            obj->index_set_({ind->GetValueAsInteger()}, value);
//        } else {
//            NL_PARSER(term, "Fail cast from '%s' to ':Int8'", toString(value->getType()));
//        }
//        return value;
//
//    } else if (isStringWide(obj->getType())) {
//
//        if (canCast(value->getType(), m_rt->m_wide_char_type)) {
//            obj->index_set_({ind->GetValueAsInteger()}, value);
//        } else {
//            NL_PARSER(term, "Fail cast from '%s' to '%s'", toString(value->getType()), newlang::toString(m_rt->m_wide_char_type));
//        }
//        return value;
//    }
//    NL_PARSER(term, "Index type '%s' not implemented!", toString(obj->getType()));
//}
//
//ObjPtr JIT::GetIndexValue(TermPtr &term, ObjPtr &obj, Context * runner) {
//    ASSERT(term->m_right);
//
//    TermPtr index = term->m_right;
//    ASSERT(index->size());
//
//    switch (term->m_id) {
//        case TermID::DICT:
//        case TermID::STRCHAR:
//        case TermID::STRWIDE:
//            if (index->size() > 1) {
//                NL_PARSER(index, "Strings and dictionaries do not support multiple dimensions!");
//            }
//        case TermID::TENSOR:
//            break;
//
//        case TermID::NAME:
//        case TermID::LOCAL:
//        case TermID::TYPE:
//        case TermID::ARGS:
//        case TermID::ARGUMENT:
//        case TermID::STATIC:
//            break;
//
//        default:
//            NL_PARSER(term, "Term type '%s' not indexable!", toString(term->m_id));
//    }
//
//    ObjPtr ind = CheckObjTerm_(index->at(0).second, runner);
//    ASSERT(ind);
//
//    if (isStringChar(obj->getType())) {
//        return obj->index_get({ind->GetValueAsInteger()});
//    }
//
//    NL_PARSER(term, "Index type '%s' not implemented!", toString(obj->getType()));
//}
//
//ObjPtr JIT::GetFieldValue(TermPtr &term, ObjPtr &value, Context * runner) {
//    NL_PARSER(term, "GetFieldValue not implemented!");
//}
//
//ObjPtr JIT::SetFieldValue(TermPtr &term, ObjPtr &value, Context * runner) {
//    NL_PARSER(term, "SetFieldValue not implemented!");
//}
