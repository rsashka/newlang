#ifdef BUILD_UNITTEST

#include "warning_push.h"
#include <gtest/gtest.h>
#include "warning_pop.h"



#include "jit.h"
#include "term.h"
#include "runtime.h"

using namespace newlang;

TEST(JIT, Function) {
    //    std::filesystem::create_directories("temp");
    //    ASSERT_TRUE(std::filesystem::is_directory("temp"));
    //    //std::remove("temp/brother.sh.temp.nlm");

    TermPtr term = Parser::ParseString("func():Int32");
    term->m_int_name = "func::";
    std::string src;
    src = JIT::MakeFunctionPrototype(term, "");
    ASSERT_STREQ("newlang::ObjPtr _$$_func$$(newlang::Context *ctx, newlang::Obj &args)", src.c_str()) << src;
    src = JIT::MakeFunctionPrototype(term, "\\\\dir\\module");
    ASSERT_STREQ("newlang::ObjPtr _$dir$module$_func$$(newlang::Context *ctx, newlang::Obj &args)", src.c_str()) << src;

    term->push_back(Term::CreateName("arg"));
    term->push_back(Term::CreateName("arg2"));
    src = JIT::MakeFunctionPrototype(term, "");
    ASSERT_STREQ("newlang::ObjPtr _$$_func$$(newlang::Context *ctx, newlang::Obj &args)", src.c_str()) << src;
    src = JIT::MakeFunctionPrototype(term, "\\\\dir\\module2");
    ASSERT_STREQ("newlang::ObjPtr _$dir$module2$_func$$(newlang::Context *ctx, newlang::Obj &args)", src.c_str()) << src;
}






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

#include  <iostream>
#include <fcntl.h>

using namespace llvm;
using namespace llvm::sys;

void writeModuleToFile(llvm::Module *module) {
    auto TargetTriple = LLVMGetDefaultTargetTriple();
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();

    std::string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);
    auto CPU = "generic";
    auto Features = "";

    TargetOptions opt;
    auto RM = std::optional<Reloc::Model>();
    auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

    module->setDataLayout(TargetMachine->createDataLayout());
    module->setTargetTriple(TargetTriple);

    auto Filename = "output.o";
    std::error_code EC;
    raw_fd_ostream dest(Filename, EC, sys::fs::OF_None);

    legacy::PassManager pass;
    auto FileType = CodeGenFileType::ObjectFile;

    if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        errs() << "TargetMachine can't emit a file of this type";
        return;
    }
    pass.run(*module);
    dest.flush();

    IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts = new clang::DiagnosticOptions;
    clang::TextDiagnosticPrinter *DiagClient = new clang::TextDiagnosticPrinter(errs(), &*DiagOpts);
    IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID(new clang::DiagnosticIDs());
    clang::DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagClient);
    clang::driver::Driver TheDriver("/uslkjhklhjkr/bin/clang++-12ssss", TargetTriple, Diags);

    auto args = ArrayRef<const char *>{"-g", "output.o", "-o", "xxxxxxxxxxxxxxxxxxxxx"};

    std::unique_ptr<clang::driver::Compilation> C(TheDriver.BuildCompilation(args));

    if (C && !C->containsError()) {
        SmallVector<std::pair<int, const clang::driver::Command *>, 4> FailingCommands;
        TheDriver.ExecuteCompilation(*C, FailingCommands);
    }

//    remove(Filename);
}

std::unique_ptr<llvm::Module> CompileCpp2(std::string source, std::vector<std::string> opts, std::string *asm_code = nullptr) {
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
        std::cout << itemcstrs.back() << "\n";
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

    std::cout << "Using target triple: " << triple;

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

const char * source = ""
        "#include <stdio.h>\n"
        "#include \"sys/time.h\"\n"
        "\n"
        "\n"
        "int main(int argc, char* argv[], char* envp[]) {\n"
        "\n"
        "   // disable unused warning\n"
        "    ((void)(argc)); ((void)(argv)); ((void)(envp));\n"
        "   printf(\"Hello, world!!!!!!!!!!!!!!!!!!\\n\");\n"
        "\n"
        "   return 0;\n"
        "};";

TEST(JIT, Embed) {

    std::vector<std::string> argsX{
        //        "-std=c++23", "-stdlib=libc++",
        //        "--print-supported-cpus",
        "-I", "/usr/include/x86_64-linux-gnu/",
        "-I", "/usr/lib/gcc/x86_64-linux-gnu/11/include",
        "-I", "/usr/include",
        "-I", "/usr/include/linux",
        "-I", "/usr/local/include",
        "-o", "hello_embeddddddddddddddddddddd",
        //        "-x", "c++",
        "hello_embed.cpp"};

    auto module = CompileCpp2(source,argsX);

    writeModuleToFile(module.get());

}



//TEST(JIT, Embed) {
//
//
//    std::filesystem::path path = std::filesystem::path("../examples/hello_embed.src");
//    ASSERT_TRUE(std::filesystem::exists(path));
//
//    std::ifstream file(path, std::ios::in | std::ios::binary);
//
//    const auto sz = std::filesystem::file_size(path);
//    ASSERT_TRUE(sz);
//
//    std::string source(sz, '\0');
//    file.read(source.data(), sz);
//    ASSERT_TRUE(file) << "error: only " << file.gcount() << " could be read";
//    file.close();
//
//    TermPtr ast = Parser::ParseString(source);
//
//    JIT trans;
//    std::string hello_embed = trans.MakeApplicationSource(ast);
//
//    if (!std::filesystem::exists(std::filesystem::path("temp"))) {
//        std::filesystem::create_directories(std::filesystem::path("temp"));
//    }
//    ASSERT_TRUE(std::filesystem::exists(std::filesystem::path("temp")));
//    ASSERT_TRUE(std::filesystem::is_directory(std::filesystem::path("temp")));
//
//    std::ofstream ofile(std::filesystem::path("temp/hello_embed.cpp"), std::ios::out | std::ios::trunc);
//    ofile << hello_embed;
//    ofile.close();
//
//    std::string bin_out;
//    ASSERT_TRUE(trans.MakeCppExec(hello_embed, "temp/hello_embed")) << hello_embed;
//
//    ASSERT_TRUE(std::filesystem::exists(std::filesystem::path("temp/hello_embed")));
//    
//
//}








////bool simple_int_filter(Context *ctx, Object &in, ObjPtr & out) {
////    if(!ctx || !ctx->Current()->self) {
////        return false;
////    }
////    for (int i = 0; i < in.size(); i++) {
////        if(ctx->Current()->self->op_equal(in[i])) {
////            return false;
////        }
////    }
////    return true;
////}
//
/////*
//// * Основонй тест соответствия между компиляцией и интерпретацией текста
//// программы
//// * Реализован как набор выражений, возвращающих результат.
//// * В начале все выражения выпоняются интерпретатором, тестируются на
//// корректность и собираются в массив.
//// * Далее выражения записываюстя в исходный фай в виде модуля, который
//// компилируется, загружается и выполняется.
//// * Результатом его выполнения так же является массив.
//// * В итоге два маасива сравниваются между собой по элементно.
//// */
//// TEST(NewLang, CompileEvalConsistency) {
////
////    struct TestItem {
////        const char *result;
////        const char *eval;
////    };
////
////    // Выполняются построчно
////    std::vector<TestItem> eval_items = {
////        //        {"(val=456,)", "field_test $= (val=456,)"},
////        //        {"(val=456,)", "field_test()"},
////        //        {"123", "field_test.name := 123"},
////        //        {"(val=456, name=123,)", "field_test()"},
////        {"123.456", "float #= 123.456"},
////        {"муж.", "м #= \"муж.\""},
////        {"жен.", "ж #= \"жен.\""},
////        {"human=@term(пол=, parent=[,])", "human   @=  term(пол=,    parent  =
////        [,]   )"},
////        {"Tom2=human(пол=\"муж.\", parent=[,])", "Tom2 $= @human(пол=м)"},
////        {"Janna2=human(пол=\"жен.\", parent=[,])", "Janna2 $= @human(пол=ж)"},
////        {"Jake2=human(пол=\"муж.\", parent=[&Tom2, &Janna2,])", "Jake2 $=
////        @human(м, [  &Tom2, &Janna2, ] )"},
////        {"Tim2=human(пол=\"муж.\", parent=[&Tom2,])", "Tim2$=@human(пол=м,
////        parent=[&Tom2,])"},
////        //        {"brother={}", "brother(test1, test2) :&&= $test1!=$test2,
////        $test1.sex==м, intersec($test1.parent, $test2.parent)"},
////    };
////    // Возвращаются в виде элементов массива
////    std::vector<TestItem> array_items = {
////        {"123.456", "float"},
////        {"муж.", "м"},
////        {"жен.", "ж"},
////        {"100", "100"},
////        {"300", "100+200"},
////        {"100", "min(200,100,300)"},
////    };
////
////    RuntimePtr opts = NewLang::Init();
////    Context ctx(opts);
////    Parser parser;
////
////
////    // Литерал
////    ASSERT_STREQ("100", opts->Eval(&ctx, "100")->GetValueAsString().c_str());
////    // Оператор (символ +)
////    ASSERT_STREQ("300", opts->Eval(&ctx,
////    "100+200")->GetValueAsString().c_str());
////    // Вызов встроенной функции
////    Args min_args(Obj::CreateValue(200), Obj::CreateValue(100),
////    Obj::CreateValue(300)); // min(200,100,300) ASSERT_STREQ("100",
////    Context::CallByName(&ctx, "min", min_args)->GetValueAsString().c_str());
////
////    ObjPtr min_ret = newlang_min(&ctx, Args::Arg(Obj::CreateValue(200)),
////    Args::Arg(Obj::CreateValue(100)), Args::Arg(Obj::CreateValue(300)));
////    ASSERT_TRUE(min_ret);
////    ASSERT_STREQ("100", min_ret->GetValueAsString().c_str());
////
////
////    ASSERT_THROW(
////            opts->Eval(&ctx, "name2"),
////            std::runtime_error);
////    ASSERT_THROW(
////            opts->Eval(&ctx, "name3"),
////            std::runtime_error);
////
////    ASSERT_EQ(0, opts->m_user_terms.size());
////    ASSERT_TRUE(ctx.CreateGlobalTerm(Obj::CreateValue(111), "name1"));
////    ASSERT_EQ(1, opts->m_user_terms.size());
////    ASSERT_TRUE(opts->Eval(&ctx, "name2 $= 222; name3 $=
////    333")->GetValueAsString().c_str());
////
////    ASSERT_EQ(0, ctx.Current()->session.find("name2")->second->RefCount());
////    ASSERT_STREQ("222", opts->Eval(&ctx,
////    "name2")->GetValueAsString().c_str()); ASSERT_EQ(0,
////    ctx.Current()->session.find("name2")->second->RefCount());
////
////    ASSERT_STREQ("333", opts->Eval(&ctx,
////    "name3")->GetValueAsString().c_str());
////
////
////    Obj::Ptr field = Obj::CreateNone();
////    field->ItemAdd(Obj::CreateValue("FIELD1"), "field1");
////    field->ItemAdd(Obj::CreateValue("FIELD2"), "field2");
////    ASSERT_THROW(
////            Obj::GetField(field, "not found"),
////            std::runtime_error);
////
////    ASSERT_STREQ("FIELD1", Obj::GetField(field,
////    "field1")->GetValueAsString().c_str());
////
////    Obj::Ptr obj = Obj::CreateDict(Args::Arg("f", field),
////    Args::Arg("f2", field));
////
////    ASSERT_STREQ("FIELD1", Obj::GetField(Obj::GetField(obj, "f"),
////    "field1")->GetValueAsString().c_str()); ASSERT_STREQ("FIELD2",
////    Obj::GetField(Obj::GetField(obj, "f2"),
////    "field2")->GetValueAsString().c_str());
////
////    ASSERT_STREQ("(val=456,)", opts->Eval(&ctx, "field $=
////    (val=456,)")->GetValueAsString().c_str()); ASSERT_STREQ("(val=456,)",
////    opts->Eval(&ctx, "field")->GetValueAsString().c_str());
////    ASSERT_STREQ("123", opts->Eval(&ctx, "field.name :=
////    123")->GetValueAsString().c_str()); ASSERT_STREQ("(val=456, name=123,)",
////    opts->Eval(&ctx, "field")->GetValueAsString().c_str());
////
////
////
////
////    ASSERT_EQ(1, opts->m_user_terms.size());
////    ASSERT_STREQ("test=@term(пол=, parent=[,])", opts->Eval(&ctx, "test   @=
////    term(пол=,    parent  =   [,]   )")->GetValueAsString().c_str());
////    ASSERT_EQ(2, opts->m_user_terms.size());
////    ASSERT_STREQ("name1", opts->m_user_terms[0]->getName().c_str());
////
////    ASSERT_EQ(4, ctx.Current()->session.size());
////    ASSERT_EQ(0, ctx.Current()->session.find("name2")->second->RefCount());
////    ASSERT_STREQ("222",
////    ctx.Current()->session.find("name2")->second->toString().c_str());
////    ASSERT_STREQ("Tom=test(пол=222, parent=[,])", opts->Eval(&ctx, "Tom $=
////    @test(пол=name2)")->GetValueAsString().c_str()); ASSERT_EQ(0,
////    ctx.Current()->session.find("name2")->second->RefCount());
////
////    ASSERT_STREQ("[&Tom,]", opts->Eval(&ctx,
////    "ref$=[&Tom,]")->GetValueAsString().c_str()); ASSERT_STREQ("Tim=test(пол=,
////    parent=[&Tom,])", opts->Eval(&ctx, "Tim $=
////    @test(parent=[&Tom,])")->GetValueAsString().c_str());
////    ASSERT_STREQ("brother={}", opts->Eval(&ctx, "brother(test1, test2) :&&=
////    $test1!=$test2, $test1.sex==м, intersec($test1.parent,
////    $test2.parent)")->GetValueAsString().c_str());
////
////
////    std::string str = "#!./dist/Debug/GNU-Linux/nlc --exec\n"
////            "// Test source code for verify consistency between compilation
////            and evaluation modes\n"
////            "\n";
////
////    for (auto item : eval_items) {
////        ASSERT_STREQ(item.result, opts->Eval(&ctx,
////        item.eval)->GetValueAsString().c_str()); str += item.eval; str +=
////        ";\n";
////    }
////
////    str += "\nconsistency $= (,);\n";
////    for (auto item : array_items) {
////        ASSERT_STREQ(item.result, opts->Eval(&ctx,
////        item.eval)->GetValueAsString().c_str()); str += "consistency[] := ";
////        str += item.eval;
////        str += ";\n";
////    }
////
////    str += "\n$$ := consistency;\n";
////
////    std::filesystem::create_directories("temp");
////    ASSERT_TRUE(std::filesystem::is_directory("temp"));
////
////    std::ofstream out("temp/consistency.temp.sh");
////    out << str;
////    out.close();
////
////
////    Context ctx_module(NewLang::Init());
////    ObjPtr result_module = NewLang::ExecModule("temp/consistency.temp.sh",
////    "temp/consistency.temp.nlm", false, &ctx_module);
////    ASSERT_TRUE(result_module);
////
////
////    RuntimePtr copy = NewLang::Init();
////    Context ctx_copy(copy);
////    ObjPtr result_eval = copy->Eval(&ctx_copy, str.c_str());
////
////    ASSERT_TRUE(result_module->getItemCount());
////    ASSERT_EQ(array_items.size(), result_module->getItemCount());
////
////    ASSERT_TRUE(result_eval->getItemCount());
////    ASSERT_EQ(array_items.size(), result_eval->getItemCount());
////
////    RuntimePtr copy2 = NewLang::Init();
////    Context ctx_copy2(copy2);
////    ObjPtr res;
////    for (int i = 0; i < eval_items.size(); i++) {
////        res = copy2->Eval(&ctx_copy2, eval_items[i].eval);
////        ASSERT_TRUE(res);
////        ASSERT_STREQ(eval_items[i].result, res->GetValueAsString().c_str()) <<
////        eval_items[i].result << " != " << res->GetValueAsString();
////    }
////
////    for (int i = 0; i < array_items.size(); i++) {
////
////        res = copy2->Eval(&ctx_copy2, array_items[i].eval);
////        ASSERT_TRUE(res);
////        ASSERT_STREQ(array_items[i].result, res->GetValueAsString().c_str())
////        << array_items[i].result << " != " << res->GetValueAsString();
////
////        ObjPtr temp = result_module->getItem(i);
////        ASSERT_TRUE(temp->op_eq(res, true)) << i << "  " << res->toString();
////        temp = result_eval->getItem(i);
////        ASSERT_TRUE(temp->op_eq(res, true)) << i << "  " << res->toString();
////    }
////
////
////    std::filesystem::create_directories("temp");
////    ASSERT_TRUE(std::filesystem::is_directory("temp"));
////    //std::remove("temp/brother.sh.temp.nlm");
////}
//
//// TEST(NewLang, JitLLVM) {
////
////     RuntimePtr opts = NewLang::Init();
////     Context ctx(opts);
////          const char *src = "\
////#include <stdio.h>\n\
////#include <string.h>\n\
////extern \"C\" int jit_print(const char *str) {\n\
////    printf(\"%s\\n\", str);\n\
////    return strlen(str);\n\
////}\n";
////
////     llvm::ExecutionEngine * e = opts->JITCompileCPP(src);
////
////
////     typedef int (*JIT_PRINT)(const char *);
////
////     e = opts->JITCompileCPP(src);
////     ASSERT_TRUE(e);
////     JIT_PRINT jit_print = reinterpret_cast<JIT_PRINT>
////     (opts->m_jit->GetFunction("jit_print")); ASSERT_TRUE(jit_print);
////     ASSERT_EQ(0, jit_print(""));
////     ASSERT_EQ(9, jit_print("jit_print"));
////
////
////
////    std::filesystem::create_directories("temp");
////    ASSERT_TRUE(std::filesystem::is_directory("temp"));
////
////     //    std::ofstream out("temp/native.temp.src");
////     //    out << "native(str=\"default arg\") $= { %{ $$=$str; printf(\"%s\",
////     static_cast<char *>($str)); %} };";
////     //    out.close();
////     //
////     //    ASSERT_TRUE(opts->CompileFunctions("native(str=\"default arg\") $=
////     { %{ $$=$str; printf(\"%s\", static_cast<char *>($str)); %} };"));
////     //    ASSERT_TRUE(opts->CompileModule("temp/native.temp.src",
////     "temp/native.temp.nlm"));
////     //    ASSERT_TRUE(opts->LoadModule("temp/native.temp.nlm"));
////     //
////     //    EXPECT_STREQ("default arg", opts->Eval(&ctx,
////     "native()")->GetValueAsString().c_str());
////     //    EXPECT_STREQ("default arg", opts->Eval(&ctx,
////     "\\native()")->GetValueAsString().c_str());
////     //    EXPECT_STREQ("default arg", opts->Eval(&ctx,
////     "\\native()")->GetValueAsString().c_str());
////     //    EXPECT_STREQ("", opts->Eval(&ctx,
////     "\\native(\"\")")->GetValueAsString().c_str());
////     //    EXPECT_STREQ("Hello, world!\n", opts->Eval(&ctx, "\\native(\"Hello,
////     world!\\n\")")->GetValueAsString().c_str());
//// }
//
//TEST(Compiler, DISABLED_Function) {
//
//    const char *func_text =
//            "func_sum(arg1, arg2) :- {$arg1 + $arg2;};\n"
//            "func_call(arg1, arg2) :- {func_sum($arg1, $arg2);};";
//
//    TermPtr funcs;
//    Parser parser(funcs);
//
//    parser.Parse(func_text);
//    ASSERT_TRUE(funcs);
//    ASSERT_EQ(TermID::BLOCK, funcs->getTermID());
//    ASSERT_EQ(2, funcs->BlockCode().size());
//
//    TermPtr func1 = funcs->BlockCode()[0];
//
//    ASSERT_EQ(TermID::PURE_ONCE, func1->getTermID());
//    ASSERT_TRUE(func1->Left());
//    ASSERT_EQ(2, func1->Left()->size());
//
//    ASSERT_TRUE(func1->Right());
//    TermPtr body1 = func1->Right();
//
//    //    ASSERT_EQ(TermID::LOCAL, body1->getTermID());
//    //    ASSERT_TRUE(body1->Left());
//    //    ASSERT_TRUE(body1->Right());
//    //    ASSERT_STREQ(":=", body1->getText().c_str());
//    //    ASSERT_EQ(TermID::RESULT, body1->Left()->getTermID());
//    //    ASSERT_STREQ("$$", body1->Left()->getText().c_str());
//    //    ASSERT_EQ(TermID::OPERATOR, body1->Right()->getTermID());
//
//    TermPtr op = body1->Right();
//    //    ASSERT_TRUE(op);
//    //    ASSERT_TRUE(op->Left());
//    //    ASSERT_TRUE(op->Right());
//    //    ASSERT_EQ(TermID::TERM, op->Left()->getTermID());
//    //    ASSERT_STREQ("+", op->getText().c_str());
//    //    ASSERT_STREQ("$arg1", op->Left()->getText().c_str());
//    //    ASSERT_STREQ("$arg2", op->Right()->getText().c_str());
//
//
//    TermPtr func2 = funcs->BlockCode()[1];
//
//    ASSERT_EQ(TermID::PURE_ONCE, func2->getTermID());
//    ASSERT_TRUE(func2->Left());
//    ASSERT_EQ(2, func2->Left()->size());
//
//    //    ASSERT_TRUE(func2->Right());
//    //    TermPtr body2 = func2->Right();
//    //    ASSERT_TRUE(body2);
//    //
//    //    ASSERT_TRUE(body2->Left());
//    //    ASSERT_TRUE(body2->Right());
//    //    ASSERT_EQ(TermID::LOCAL, body2->getTermID());
//    //    ASSERT_STREQ(":=", body2->getText().c_str());
//    //    ASSERT_EQ(TermID::RESULT, body2->Left()->getTermID());
//    //    ASSERT_STREQ("$$", body2->Left()->getText().c_str());
//    //
//    //    op = body2->Right();
//    //    ASSERT_TRUE(op);
//    //    ASSERT_FALSE(op->Left());
//    //    ASSERT_FALSE(op->Right());
//    //    ASSERT_EQ(TermID::CALL, op->getTermID());
//    //    ASSERT_STREQ("func_sum", op->getText().c_str());
//    //    ASSERT_EQ(2, op->size());
//    //    ASSERT_STREQ("$arg1", (*op)[0]->getText().c_str());
//    //    ASSERT_STREQ("$arg2", (*op)[1]->getText().c_str());
//
//
//    std::ostringstream sstr;
//    ASSERT_TRUE(Compiler::MakeCppFile(funcs, sstr)); // << sstr.str();
//
//
//    std::filesystem::create_directories("temp");
//    ASSERT_TRUE(std::filesystem::is_directory("temp"));
//
//    std::ofstream file("temp/function_test.temp.cpp");
//    file << sstr.str();
//    file.close();
//}
//
//TEST(Compiler, DISABLED_FuncsTypes) {
//
//    /*
//     * - Проверка типов аргументов при вызове функций
//     * - Проверка типов возвращаемых значений у функций
//     */
//
//    RuntimePtr opts = RunTime::Init();
//    Context ctx(opts);
//
//#define FUNC_ARG "func_arg(arg1: Int8, arg2): Int8 := { $arg1+$arg2; };"
//#define FUNC_RES "func_res(arg1: Int8, arg2: Int32): Integer := { $arg2+=$arg1; };"
//
//    TermPtr func;
//    Parser parser(func);
//
//    std::ostringstream sstr;
//
//    // Не соответствие типа функции в операторе
//    parser.Parse(FUNC_ARG FUNC_RES "\n$res:Int8 := func_arg(100, 100); $res += func_res(100, 100);");
//    sstr.str("");
//    ASSERT_THROW(Compiler::MakeCppFile(func, sstr, nullptr, &ctx), Return) << sstr.str();
//
//    // Компилится без ошибок
//    parser.Parse(FUNC_ARG "\nfunc_arg(Int8(100), 100);");
//    sstr.str("");
//    ASSERT_NO_THROW(Compiler::MakeCppFile(func, sstr, nullptr, &ctx)) << sstr.str();
//
//    // Не соответствие типа первого аргумента
//    parser.Parse(FUNC_ARG "\nfunc_arg(1000, 100);");
//    sstr.str("");
//    ASSERT_THROW(Compiler::MakeCppFile(func, sstr, nullptr, &ctx), Return) << sstr.str();
//
//    // Не соответствие типа функции
//    parser.Parse(FUNC_ARG FUNC_RES "\n$res:Int8 := func_res(100, 1000);");
//    sstr.str("");
//    ASSERT_THROW(Compiler::MakeCppFile(func, sstr, nullptr, &ctx), Return) << sstr.str();
//
//    // Не соответствие типа функции в операторе
//    parser.Parse(FUNC_ARG FUNC_RES "\n$res:Int8 := func_arg(100, 100); $res += func_res(100, 100);");
//    sstr.str("");
//    ASSERT_THROW(Compiler::MakeCppFile(func, sstr, nullptr, &ctx), Return) << sstr.str();
//
//    // Нет типа у $res как в предыдщем случае
//    parser.Parse(FUNC_ARG FUNC_RES "\n$res := func_arg(100, 100); $res += func_res(100, 100);");
//    sstr.str("");
//    ASSERT_NO_THROW(Compiler::MakeCppFile(func, sstr, nullptr, &ctx)) << sstr.str();
//
//    // Тип есть, но делается каст возвращаемого типа у функции
//    parser.Parse(FUNC_ARG FUNC_RES "\n$res: Int8 := func_arg(100, 100); $res += Int8(func_res(100, 100));");
//    sstr.str("");
//    ASSERT_NO_THROW(Compiler::MakeCppFile(func, sstr, nullptr, &ctx)) << sstr.str();
//
//
//    std::filesystem::create_directories("temp");
//    ASSERT_TRUE(std::filesystem::is_directory("temp"));
//
//
//    std::ofstream file("temp/call_types.temp.cpp");
//    file << sstr.str();
//    file.close();
//
//    std::string out;
//    int exit_code;
//    ASSERT_TRUE(Compiler::GccMakeModule("temp/call_types.temp.cpp",
//            "temp/call_types.temp.nlm", nullptr,
//            &out, &exit_code))
//            << exit_code << " " << out;
//
//    ASSERT_TRUE(ctx.m_runtime->LoadModule(ctx, "call_types.temp.nlm", false));
//
//    // Переполнение байтовой переменной $res во время выполнения последнего оператора "+="
//    //    Obj args;
//    //    ASSERT_TRUE(ctx.m_runtime->m_modules["temp/call_types.temp.nlm"]);
//    //    ASSERT_NO_THROW(
//    //            ctx.m_runtime->m_modules["temp/call_types.temp.nlm"]->Main(&ctx, args));
//    //@todo Контроль переполнения при операциях для типизированных переменных
//    //????????????????
//    //@todo Такой же как и для остальных операций
//    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//    // ASSERT_THROW(opts->m_modules["temp/call_types.temp.nlm"]->Main(&ctx,
//    // args), newlang_exception);
//
//#undef FUNC_ARG
//#undef FUNC_RES
//
//}

#endif // BUILD_UNITTEST
