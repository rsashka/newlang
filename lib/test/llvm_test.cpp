#ifdef BUILD_UNITTEST

#include "warning_push.h"
#include <gtest/gtest.h>
#include "warning_pop.h"

#include "jit.h"

// https://github.com/rsashka/cpp-jit-llvm

using namespace newlang;

/* 
 * Функции и классы для вызова изнутри JIT
 */
namespace ns_stub {

    int func_stub(int arg1, short arg2) {
        return arg1*arg2;
    };

    int func_extern_stub() {
        return 4242;
    };

    class class_stub {
    public:
        int field_1;
        static int static_field_2;

        static class_stub * create(int a1, int a2) {
            return new class_stub(a1, a2);
        }

        class_stub() {
            printf("Call constructor class_stub()\n");
            field_1 = 0;
        }

        class_stub(int arg1, int arg2) {
            printf("Call constructor class_stub(%d, %d)\n", arg1, arg2);
            field_1 = arg1;
            static_field_2 = arg2;
        }

        virtual ~class_stub() {
            printf("Call virtual ~class_stub()\n");
        }

        int method_sum() {

            return field_1 + static_field_2;
        }

        int method_field1(int arg) {

            return field_1;
        }

        virtual double method_virt2() {

            return 999999999;
        }

        virtual float method_virt() {

            return 3.14 + field_1;
        }

        static float method_static() {
            return 3.1415;
        }
    };

    int class_stub::static_field_2 = 0;

    class class_full {
    public:

        class_full() {
        }

        int method() {
            return 42;
        }
    };
};

/*
 * Строка прототип для компиляции в JIT
 */
const char * func_text_jit = ""
        "extern \"C\" int printf(const char *, ...);\n"
        "extern \"C\" int nv_add(int a, int b) {"
        "   printf(\"call nv_add(%d, %d)\\n\", a, b);"
        "   return a + b;"
        "};\n"
        ""
        "extern \"C\" int nv_sub(int a, int b) {"
        "   printf(\"call nv_sub(%d, %d)\\n\", a, b);"
        "   return a - b;"
        "};\n"
        "extern \"C\" int run(){"
        "   nv_add(100, 123);"
        "   nv_sub(100, 123);"
        "   return 42;"
        "};\n"
        ""
        "namespace ns_stub {"
        "   class run_internal {"
        "       public:\n"
        "       run_internal(){};"
        "       int method(){"
        "           return 43;"
        "       };"
        "   };"
        "   class class_full {"
        "       public:\n"
        "       class_full();"
        "       int method();"
        "   };"
        ""
        "   class class_stub {"
        "       public:\n"
        "       static class_stub * create(int, int);"
        "       class_stub();"
        "       class_stub(int arg1, int arg2);"
        "       int method_sum();"
        "       int method_field1(int);"
        "       virtual float method_virt();"
        "   };"
        ""
        "};"
        "extern \"C\" int run_internal(){"
        "   ns_stub::run_internal cl_int;"
        "   printf(\"run_internal.method %d\\n\", cl_int.method());"
        "   return 44;"
        "};\n"
        ""
        "extern \"C\" int run_stub(){"
        "   ns_stub::class_stub *cl = ns_stub::class_stub::create(123, 123);"
        "   printf(\"class_stub.method_sum %d\\n\", cl->method_sum());"
        "   delete cl;"
        "   return 42;"
        "};\n"
        ""
        "extern \"C\" int run_extern();"
        "extern \"C\" int run_extern_stub(){"
        "   return run_extern();"
        "};\n"
        "extern \"C\" int run_virt(){"
        "   ns_stub::class_stub *cl = ns_stub::class_stub::create(124, 125);"
        "   printf(\"class_stub.method_virt %f\\n\", cl->method_virt());"
        "   delete cl;"
        "   return 0;"
        "};\n"
        "";



#define DEBUG_MSG(msg) std::cout << "[DEBUG]: "<<msg<< std::endl;

//void InitializeLLVM() {
//
//    // We have not initialized any pass managers for any device yet.
//    // Run the global LLVM pass initialization functions.
////    llvm::InitializeNativeTarget();
////    llvm::InitializeNativeTargetAsmPrinter();
////    llvm::InitializeNativeTargetAsmParser();
////    LLVMInitializeX86AsmParser();
//    LLVMInitializeNativeTarget();
//    LLVMInitializeNativeAsmParser();
//    LLVMInitializeNativeAsmPrinter();
//    //LLVMInitializeAllAsmParsers(); // Для использования ассемблерных вставок!!! (комменатрие в коде)
//
//
////    auto& Registry = *llvm::PassRegistry::getPassRegistry();
////
////    llvm::initializeCore(Registry);
////    llvm::initializeScalarOpts(Registry);
////    llvm::initializeVectorization(Registry);
////    llvm::initializeIPO(Registry);
////    llvm::initializeAnalysis(Registry);
////    llvm::initializeTransformUtils(Registry);
////    llvm::initializeInstCombine(Registry);
////    llvm::initializeTarget(Registry);
//}

std::unique_ptr<llvm::Module> CompileCpp(std::string source, std::vector<std::string> opts, std::string *asm_code = nullptr) {
    clang::CompilerInstance compilerInstance;
    auto& compilerInvocation = compilerInstance.getInvocation();


    // Диагностика работы Clang
    clang::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts = new clang::DiagnosticOptions;
    clang::TextDiagnosticPrinter *textDiagPrinter =
            new clang::TextDiagnosticPrinter(llvm::outs(), &*DiagOpts);

    clang::IntrusiveRefCntPtr<clang::DiagnosticIDs> pDiagIDs;

    clang::DiagnosticsEngine *pDiagnosticsEngine =
            new clang::DiagnosticsEngine(pDiagIDs, &*DiagOpts, textDiagPrinter);


    // Целевая платформа
    std::string triple = LLVMGetDefaultTargetTriple();

    std::vector<std::string> itemstrs;
    itemstrs.push_back(triple.insert(0, "-triple="));
    itemstrs.push_back("-xc++");
    itemstrs.push_back("-std=c++23");

    itemstrs.insert(itemstrs.end(), opts.begin(), opts.end());

    //    itemstrs.push_back("-fno-exceptions");
    //    itemstrs.push_back("-funwind-tables");

    std::vector<const char*> itemcstrs;
    for (unsigned idx = 0; idx < itemstrs.size(); idx++) {
        // note: if itemstrs is modified after this, itemcstrs will be full
        // of invalid pointers! Could make copies, but would have to clean up then...
        itemcstrs.push_back(itemstrs[idx].c_str());
//        std::cout << itemcstrs.back() << "\n";
    }

    // Компиляция из памяти
    // Send code through a pipe to stdin
    int codeInPipe[2];
    pipe2(codeInPipe, O_NONBLOCK);
    write(codeInPipe[1], source.c_str(), source.size());
    close(codeInPipe[1]); // We need to close the pipe to send an EOF
    dup2(codeInPipe[0], STDIN_FILENO);

    itemcstrs.push_back("-"); // Read code from stdin

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

    DEBUG_MSG("Using target triple: " << triple);

    LLVMContextRef ctx = LLVMContextCreate();
    std::unique_ptr<clang::CodeGenAction> action = std::make_unique<clang::EmitLLVMOnlyAction>((llvm::LLVMContext *)ctx);

    assert(compilerInstance.ExecuteAction(*action));

    // Runtime LLVM Module
    std::unique_ptr<llvm::Module> module = action->takeModule();

    assert(module);


    if (asm_code) {
        llvm::raw_string_ostream asm_stream(*asm_code);
        module->print(asm_stream, 0, false);
    }
    //AssemblyWriter




    //    LLVMContextRef context = LLVMContextCreate();
    //    LLVMModuleRef module = LLVMModuleCreateWithNameInContext("test", context);

    //    LLVMTypeRef f32 = LLVMFloatTypeInContext(context);
    //    LLVMTypeRef ftype = LLVMFunctionType(f32, 0, 0, 0);
    //    LLVMValueRef ftest = LLVMAddFunction(module, "ftest", ftype);
    //    LLVMBasicBlockRef bb = LLVMAppendBasicBlockInContext(context, ftest, "ftest");
    //    LLVMBuilderRef builder = LLVMCreateBuilderInContext(context);
    //    LLVMPositionBuilderAtEnd(builder, bb);
    //    LLVMValueRef v1 = LLVMConstReal(f32, 2.5);
    //    LLVMValueRef v2 = LLVMConstReal(f32, 4.3);
    //    LLVMValueRef result = LLVMBuildFAdd(builder, v1, v2, "f-add");
    //    LLVMBuildRet(builder, result);
    //    LLVMVerifyFunction(ftest, LLVMPrintMessageAction);

    char* errors = 0;
    //    LLVMPrintModuleToFile((LLVMModuleRef)&*module, "include_h_i_function_test.s", &errors);
    //    printf("error: %s\n", errors);
    //    LLVMDisposeMessage(errors);

    //    LLVMInitializeAllTargetInfos();
    //    LLVMInitializeAllTargets();
    //    LLVMInitializeAllTargetMCs();
    //    LLVMInitializeAllAsmParsers(); // Для использования ассемблерных вставок!!! (комменатрие в коде)
    //    LLVMInitializeAllAsmPrinters();

    //    LLVMTargetRef target;
    //    LLVMGetTargetFromTriple(LLVMGetDefaultTargetTriple(), &target, &errors);
    //    //    printf("error: %s\n", errors);
    //    LLVMDisposeMessage(errors);
    //    //    printf("target: %s, [%s], %d, %d\n", LLVMGetTargetName(target), LLVMGetTargetDescription(target), LLVMTargetHasJIT(target), LLVMTargetHasTargetMachine(target));
    //    //    printf("triple: %s\n", LLVMGetDefaultTargetTriple());
    //    //    printf("features: %s\n", LLVMGetHostCPUFeatures());
    //    LLVMTargetMachineRef machine = LLVMCreateTargetMachine(target, LLVMGetDefaultTargetTriple(), "generic", LLVMGetHostCPUFeatures(), LLVMCodeGenLevelDefault, LLVMRelocDefault, LLVMCodeModelDefault);
    //
    //    LLVMSetTarget((LLVMModuleRef)&*module, LLVMGetDefaultTargetTriple());
    //    LLVMTargetDataRef datalayout = LLVMCreateTargetDataLayout(machine);
    //    char* datalayout_str = LLVMCopyStringRepOfTargetData(datalayout);
    //    //    printf("datalayout: %s\n", datalayout_str);
    //    LLVMSetDataLayout((LLVMModuleRef)&*module, datalayout_str);
    //    LLVMDisposeMessage(datalayout_str);
    //
    //    LLVMTargetMachineEmitToFile(machine, (LLVMModuleRef)&*module, "include_h_i_function_test.o", LLVMObjectFile, &errors);
    //    //    printf("error: %s\n", errors);
    //    LLVMDisposeMessage(errors);


    // Оптимизация IR
    //    llvm::PassBuilder passBuilder;
    //    llvm::LoopAnalysisManager loopAnalysisManager;
    //    llvm::FunctionAnalysisManager functionAnalysisManager;
    //    llvm::CGSCCAnalysisManager cGSCCAnalysisManager;
    //    llvm::ModuleAnalysisManager moduleAnalysisManager;
    //
    //    passBuilder.registerModuleAnalyses(moduleAnalysisManager);
    //    passBuilder.registerCGSCCAnalyses(cGSCCAnalysisManager);
    //    passBuilder.registerFunctionAnalyses(functionAnalysisManager);
    //    passBuilder.registerLoopAnalyses(loopAnalysisManager);
    //    passBuilder.crossRegisterProxies(loopAnalysisManager, functionAnalysisManager, cGSCCAnalysisManager, moduleAnalysisManager);
    //
    //    llvm::ModulePassManager modulePassManager = passBuilder.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O0);
    //    modulePassManager.run(*module, moduleAnalysisManager);

    return module;
}

llvm::orc::ThreadSafeModule createDemoModule() {
    auto ctx = std::make_unique<llvm::LLVMContext>();
    auto M = std::make_unique<llvm::Module>("test", *ctx);

    // Create the add1 function entry and insert this entry into module M.  The
    // function will have a return type of "int" and take an argument of "int".
    llvm::Function *Add1F = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getInt32Ty(*ctx),{llvm::Type::getInt32Ty(*ctx)}, false),
    llvm::Function::ExternalLinkage, "add1", M.get());

    // Add a basic block to the function. As before, it automatically inserts
    // because of the last argument.
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*ctx, "EntryBlock", Add1F);

    // Create a basic block builder with default parameters.  The builder will
    // automatically append instructions to the basic block `BB'.
    llvm::IRBuilder<> builder(BB);

    // Get pointers to the constant `1'.
    llvm::Value *One = builder.getInt32(1);

    // Get pointers to the integer argument of the add1 function...
    assert(Add1F->arg_begin() != Add1F->arg_end()); // Make sure there's an arg
    llvm::Argument *ArgX = &*Add1F->arg_begin(); // Get the arg
    ArgX->setName("AnArg"); // Give it a nice symbolic name for fun.

    // Create the add instruction, inserting it into the end of BB.
    llvm::Value *Add = builder.CreateAdd(One, ArgX);

    // Create the return instruction and add it to the basic block
    builder.CreateRet(Add);

    return llvm::orc::ThreadSafeModule(std::move(M), std::move(ctx));
}

TEST(LLVM, JIT) {

    llvm::ExitOnError ExitOnErr;

    ASSERT_TRUE(JIT::LLVMInitialize());
    //    InitLLVM X(argc, argv);


    //    cl::ParseCommandLineOptions(argc, argv, "HowToUseLLJIT");
    //    ExitOnErr.setBanner(std::string(argv[0]) + ": ");

    std::string asm_source;

    // Create an LLJIT instance.
    auto J = ExitOnErr(llvm::orc::LLJITBuilder().create());
    //    auto M = createDemoModule();
    auto module = CompileCpp(func_text_jit,{}, &asm_source);

    std::error_code EC;
    llvm::raw_fd_ostream OS("nv_add_JIT.bc", EC, llvm::sys::fs::OpenFlags::OF_None);
    llvm::WriteBitcodeToFile(*module, OS);
    OS.flush();

    auto M = llvm::orc::ThreadSafeModule(std::move(module), std::make_unique<llvm::LLVMContext>());


    std::ofstream asm_file("nv_add_JIT.s", std::ios::trunc);
    asm_file << asm_source;
    asm_file.close();




    std::string dump;
    llvm::raw_string_ostream err(dump);


    llvm::orc::ExecutionSession &ES = J->getExecutionSession();

    //    JITDylib *plat = ES.getJITDylibByName("<Platform>");
    //    assert(plat);
    //    dump.clear();
    //    plat->dump(err);
    //    std::cout << "<Platform>:\n" << dump << "\n";
    //
    //    JITDylib *proc = ES.getJITDylibByName("<Process Symbols>");
    //    assert(proc);
    //    dump.clear();
    //    proc->dump(err);
    //    std::cout << "<Process Symbols>:\n" << dump << "\n";


    ExitOnErr(J->addIRModule(std::move(M)));
    //    ExitOnErr(J->addObjectFile(std::move(M)));
    // Adds an object file to the given JITDylib.
    //Error addObjectFile(std::unique_ptr<MemoryBuffer> Obj)




    // Функция с именем run_extern отсуствует (JIT session error: Symbols not found: [ run_extern ])
    // Подставим вместо нее указатель на другу функцию, но с таким же прототипом (func_extern_stub)
    const llvm::orc::SymbolStringPtr Foo = ES.intern("run_extern");
    const llvm::orc::ExecutorSymbolDef FooSym(llvm::orc::ExecutorAddr::fromPtr(&ns_stub::func_extern_stub), llvm::JITSymbolFlags::Exported | llvm::JITSymbolFlags::Absolute);
    auto as = llvm::orc::absoluteSymbols({
        {Foo, FooSym}
    });

    ASSERT_FALSE(J->getMainJITDylib().define(as));


    llvm::Expected<llvm::orc::ExecutorAddr> test = J->lookup("nv_add");
    ASSERT_TRUE(!!test) << "lookup error:\n" << toString(test.takeError()) << ": lookup error3\n";

    DEBUG_MSG("Retrieving nv_add/nv_sub functions...");

    auto addAddr = ExitOnErr(J->lookup("nv_add"));
    int (*add)(int, int) = addAddr.toPtr<int(int, int) >();
    ASSERT_TRUE(add);

    int res = add(40, 2);
    ASSERT_TRUE(42 == res);

    auto subAddr = ExitOnErr(J->lookup("nv_sub"));
    int (*sub)(int, int) = subAddr.toPtr<int(int, int) >();
    ASSERT_TRUE(sub);

    res = sub(50, 7);
    ASSERT_TRUE(43 == res);



    printf("Call: run_internal\n");
    auto run_internalAddr = ExitOnErr(J->lookup("run_internal"));
    int (*run_internal)() = run_internalAddr.toPtr<int() >();
    ASSERT_TRUE(run_internal);

    res = run_internal();
    ASSERT_TRUE(44 == res);


    // Линкер удаяет не используемый код, 
    // и если нет обращения к методу то его будет нельзя вызвать в JIT
    // JIT session error: Symbols not found: [ _ZN7ns_stub10class_stub6createEii, _ZN7ns_stub10class_stub10method_sumEv ]
    ns_stub::class_stub *cl = ns_stub::class_stub::create(0, 0);
    printf("Check run_stub.method %d\n", cl->method_sum());
    printf("Check run_stub.method_virt %f\n", cl->method_virt());
    delete cl;


    printf("Call: run_stub\n");
    auto run_stubAddr = ExitOnErr(J->lookup("run_stub"));
    int (*run_stub)() = run_stubAddr.toPtr<int() >();
    ASSERT_TRUE(run_stub);

    res = run_stub();
    ASSERT_TRUE(42 == res);


    printf("Call: run_extern_stub\n");
    auto run_extern_stubAddr = ExitOnErr(J->lookup("run_extern_stub"));
    int (*run_extern_stub)() = run_extern_stubAddr.toPtr<int() >();
    assert(run_extern_stub);

    res = run_extern_stub();
    ASSERT_TRUE(4242 == res);


    /*
     * 
     * Так нельзя !!!!! 
     * Виртуальные методы изнутри JIT вызываются неправильно при некорректном заголовочном файле!
     * 
     * ERROR !!!!
     * Virtual methods from within JIT are called incorrectly when the header file is incorrect!
     *      
     */

    //    printf("Call: run_virt\n");
    //    auto run_virtAddr = ExitOnErr(J->lookup("run_virt"));
    //    int (*run_virt)() = run_virtAddr.toPtr<int() >();
    //    ASSERT_TRUE(run_virt);
    //
    //    res = run_virt();
    //    ASSERT_TRUE(0 == res);
}


#include "object.h"
#include "runtime.h"

extern "C" bool include_h_i_function_test();

using namespace newlang;

TEST(LLVM, Include_h_i_GTF) {
    
    ObjPtr obj = Obj::CreateEmpty();

    ASSERT_STREQ("_", obj->toString().c_str());
    ASSERT_ANY_THROW(auto _ = static_cast<bool> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<int8_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<uint8_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<int16_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<uint16_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<int32_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<uint32_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<int64_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<uint64_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<float> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<double> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<std::string> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<std::wstring> (*obj));

    (*obj) = false;
    LOG_DEBUG("(*obj) = false;");

    ASSERT_EQ(false, static_cast<bool> (*obj));
    ASSERT_EQ(0, static_cast<int8_t> (*obj));
    ASSERT_EQ(0, static_cast<uint8_t> (*obj));
    ASSERT_EQ(0, static_cast<int16_t> (*obj));
    ASSERT_EQ(0, static_cast<uint16_t> (*obj));
    ASSERT_EQ(0, static_cast<int32_t> (*obj));
    ASSERT_EQ(0, static_cast<uint32_t> (*obj));
    ASSERT_EQ(0, static_cast<int64_t> (*obj));
    ASSERT_EQ(0, static_cast<uint64_t> (*obj));
    ASSERT_STREQ("0", static_cast<std::string> (*obj).c_str());
    ASSERT_STREQ(L"0", static_cast<std::wstring> (*obj).c_str());
    ASSERT_FLOAT_EQ(0, static_cast<float> (*obj));
    ASSERT_DOUBLE_EQ(0, static_cast<double> (*obj));

    (*obj) = true;
    LOG_DEBUG("(*obj) = true;");

    ASSERT_EQ(true, static_cast<bool> (*obj));
    ASSERT_EQ(1, static_cast<int8_t> (*obj));
    ASSERT_EQ(1, static_cast<uint8_t> (*obj));
    ASSERT_EQ(1, static_cast<int16_t> (*obj));
    ASSERT_EQ(1, static_cast<uint16_t> (*obj));
    ASSERT_EQ(1, static_cast<int32_t> (*obj));
    ASSERT_EQ(1, static_cast<uint32_t> (*obj));
    ASSERT_EQ(1, static_cast<int64_t> (*obj));
    ASSERT_EQ(1, static_cast<uint64_t> (*obj));
    ASSERT_STREQ("1", static_cast<std::string> (*obj).c_str());
    ASSERT_STREQ(L"1", static_cast<std::wstring> (*obj).c_str());
    ASSERT_FLOAT_EQ(1, static_cast<float> (*obj));
    ASSERT_DOUBLE_EQ(1, static_cast<double> (*obj));

    (*obj) = -200;
    LOG_DEBUG("(*obj) = -200;");

    ASSERT_EQ(true, static_cast<bool> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<int8_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<uint8_t> (*obj));
    ASSERT_EQ(-200, static_cast<int16_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<uint16_t> (*obj));
    ASSERT_EQ(-200, static_cast<int32_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<uint32_t> (*obj));
    ASSERT_EQ(-200, static_cast<int64_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<uint64_t> (*obj));
    ASSERT_STREQ("-200", static_cast<std::string> (*obj).c_str());
    ASSERT_STREQ(L"-200", static_cast<std::wstring> (*obj).c_str());
    ASSERT_FLOAT_EQ(-200, static_cast<float> (*obj));
    ASSERT_DOUBLE_EQ(-200, static_cast<double> (*obj));

    (*obj) = 200;
    LOG_DEBUG("(*obj) = 200;");

    ASSERT_EQ(true, static_cast<bool> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<int8_t> (*obj));
    ASSERT_EQ(200, static_cast<uint8_t> (*obj));
    ASSERT_EQ(200, static_cast<int16_t> (*obj));
    ASSERT_EQ(200, static_cast<uint16_t> (*obj));
    ASSERT_EQ(200, static_cast<int32_t> (*obj));
    ASSERT_EQ(200, static_cast<uint32_t> (*obj));
    ASSERT_EQ(200, static_cast<int64_t> (*obj));
    ASSERT_EQ(200, static_cast<uint64_t> (*obj));
    ASSERT_STREQ("200", static_cast<std::string> (*obj).c_str());
    ASSERT_STREQ(L"200", static_cast<std::wstring> (*obj).c_str());
    ASSERT_FLOAT_EQ(200, static_cast<float> (*obj));
    ASSERT_DOUBLE_EQ(200, static_cast<double> (*obj));

    ObjPtr obj2 = Obj::CreateValue(123);
    (*obj) += (*obj2);
    ASSERT_EQ(323, static_cast<int16_t> (*obj));
    ASSERT_EQ(323, static_cast<uint16_t> (*obj));
    ASSERT_STREQ("323", static_cast<std::string> (*obj).c_str());
    ASSERT_STREQ(L"323", static_cast<std::wstring> (*obj).c_str());


    ASSERT_NO_THROW(
            ASSERT_TRUE(include_h_i_function_test())
            );

}

extern "C" bool float_compare(float val1, float val2) {
    if (! (val1 == val2)) {
        LOG_RUNTIME("FAIL: float_compare %.20f %.20f!", val1, val2);
    }
    return true; //std::fabs(val1 - val2) <= std::numeric_limits<float>::epsilon();
}

extern "C" bool double_compare(double val1, double val2) {
    if (! (val1 == val2)) {
        LOG_RUNTIME("FAIL: double_compare %.20f %.20f!", val1, val2);
    }
    return true; //std::fabs(val1 - val2) <= std::numeric_limits<double>::epsilon();
}

TEST(LLVM, Include_h_i_JIT) {

    llvm::ExitOnError ExitOnErr;

    ASSERT_TRUE(JIT::LLVMInitialize());

    std::string include_test_cpp = ReadFile("../lib/test/include_test.cpp");
    ASSERT_TRUE(include_test_cpp.size() > 1000);


    std::string opt_str = ReadFile("../build_options.txt");
    ASSERT_TRUE(opt_str.size() > 100);

    std::vector<std::string> opts = RunTime::SplitChar(opt_str, " \t\r\n");

    opts.push_back("-I../lib");
    opts.push_back("-DBUILD_UNITTEST");
    opts.push_back("-DBUILD_DEBUG ");
    opts.push_back("-DLOG_LEVEL_NORMAL=LOG_LEVEL_DEBUG");

    //    opts.push_back("-emit-llvm");
    //    opts.push_back("-S");
    //    opts.push_back("-o");
    //    opts.push_back("LLVM_out.ll");
    //    -emit-llvm -S multiply.c -o multiply.ll

    //    opts.push_back("-ferror-limit=2");
    for (auto &elem : opts) {
        if (elem.find("-D") == 0) {
            elem += " ";
        }
//        std::cout << elem << "\n";
    }

    ASSERT_TRUE(opts.size() > 20);


    //InitLLVM X(argc, argv);
    //  InitializeNativeTarget();
    //  InitializeNativeTargetAsmPrinter();
    //  ThreadSafeContext context(std::make_unique<LLVMContext>());
    //
    //  ExitOnError ExitOnErr;
    //
    //  jitlink::InProcessMemoryManager MemMgr;
    //
    //  auto JTMB = ExitOnErr(JITTargetMachineBuilder::detectHost());
    //  JTMB.setCodeModel(CodeModel::Small);
    //
    //  auto jit =
    //      ExitOnErr(LLJITBuilder()
    //                    .setJITTargetMachineBuilder(std::move(JTMB))
    //                    .setObjectLinkingLayerCreator([&](ExecutionSession &ES, const Triple &TT) {
    //                      return std::make_unique<ObjectLinkingLayer>(ES, MemMgr);
    //                    })
    //                    .create());    


    std::string asm_source;
    // Create an LLJIT instance.
    auto J = ExitOnErr(llvm::orc::LLJITBuilder().create());

    auto module = CompileCpp(include_test_cpp, opts, &asm_source);
    std::error_code EC;
    llvm::raw_fd_ostream OS("Include_h_i_JIT.bc", EC, llvm::sys::fs::OpenFlags::OF_None);
    WriteBitcodeToFile(*module, OS);
    OS.flush();

    auto M = llvm::orc::ThreadSafeModule(std::move(module), std::make_unique<llvm::LLVMContext>());

    std::ofstream asm_file("Include_h_i_JIT.s", std::ios::trunc);
    asm_file << asm_source;
    asm_file.close();



    std::string dump;
    llvm::raw_string_ostream err(dump);


    llvm::orc::ExecutionSession &ES = J->getExecutionSession();


    //    // Функция с именем run_extern отсуствует (JIT session error: Symbols not found: [ run_extern ])
    //    // Подставим вместо нее указатель на другу функцию, но с таким же прототипом (func_extern_stub)
    //    const SymbolStringPtr Foo = ES.intern("include_h_i_function_test");
    //    const ExecutorSymbolDef FooSym(ExecutorAddr::fromPtr(&include_h_i_function_test), llvm::JITSymbolFlags::Exported | llvm::JITSymbolFlags::Absolute);
    //    auto as = absoluteSymbols({
    //        {Foo, FooSym}
    //    });
    //
    //    ASSERT_FALSE(J->getMainJITDylib().define(as));

    ExitOnErr(J->addIRModule(std::move(M)));

    //    auto ffi = ExitOnErr(errorOrToExpected(MemoryBuffer::getFileAsStream("include_h_i_function_test.o")));
    //    ExitOnErr(J->addObjectFile(std::move(ffi)));


    ES.dump(err);

    //    JITDylib *plat = ES.getJITDylibByName("<Platform>");
    //    assert(plat);
    //    dump.clear();
    //    plat->dump(err);
    //    std::cout << "<Platform>:\n" << dump << "\n";
    //
    //    JITDylib *proc = ES.getJITDylibByName("<Process Symbols>");
    //    assert(proc);
    //    dump.clear();
    //    proc->dump(err);
    //    std::cout << "<Process Symbols>:\n" << dump << "\n";


    llvm::Expected<llvm::orc::ExecutorAddr> test = J->lookup("include_h_i_function_test");
    ASSERT_TRUE(!!test) << "lookup error:\n" << toString(test.takeError()) << "\n" << dump << ":lookup error!!!!\n";

    auto funcAddr = ExitOnErr(J->lookup("include_h_i_function_test"));
    bool (*include_h_i_function_test)() = funcAddr.toPtr<bool() >();
    ASSERT_TRUE(include_h_i_function_test);

    ASSERT_TRUE(include_h_i_function_test());
}

#endif // BUILD_UNITTEST
