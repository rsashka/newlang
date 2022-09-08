#include "pch.h"

#ifdef UNITTEST

#include "term.h"

#include <warning_push.h>
#include <gtest/gtest.h>
#include <warning_pop.h>

#include <newlang.h>
#include <builtin.h>
#include <dlfcn.h>


using namespace newlang;

TEST(Compiler, EvalExample) {

    TermPtr p;
    Parser parser(p);
    parser.Parse("newlang(cpp);\n#Simple example\n%{printf(\"Hello world!\");%}");
    ASSERT_EQ(2, p->BlockCode().size());

    TermPtr op = p->BlockCode()[0];
    ASSERT_TRUE(op);

    ASSERT_EQ(TermID::CALL, op->GetTokenID());
    ASSERT_STREQ("newlang", op->getText().c_str());
    ASSERT_EQ(1, op->size());
    ASSERT_STREQ("cpp", (*op)[0].second->getText().c_str());

    op = p->BlockCode()[1];
    ASSERT_EQ(TermID::SOURCE, op->GetTokenID());
    ASSERT_STREQ("printf(\"Hello world!\");", op->getText().c_str());
    ASSERT_EQ(0, op->size());
}


#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/Builtins.h"
#include "clang/AST/Mangle.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/FrontendTool/Utils.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "llvm/IR/LLVMContext.h"




#include <llvm/InitializePasses.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/raw_ostream.h>
#include "llvm/Support/CommandLine.h"

#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/Core.h"
#include "llvm/ExecutionEngine/Orc/ExecutionUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"

#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/FileSystemOptions.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/HeaderSearchOptions.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/PreprocessorOptions.h>
#include <clang/Parse/ParseAST.h>
#include <clang/Sema/Sema.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/ASTConsumer.h>
#include <c++/9/bits/stringfwd.h>
#include <c++/9/bits/stl_bvector.h>


#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

class MyPrinter : public clang::ast_matchers::MatchFinder::MatchCallback {
public:

    clang::MangleContext *m_mangle_ctx;
    std::vector<std::string > mangle_list;

    MyPrinter() : m_mangle_ctx(nullptr) {
    }

    ~MyPrinter() {
        if(m_mangle_ctx) {
            delete m_mangle_ctx;
            m_mangle_ctx = nullptr;
        }
    }

    virtual void run(const clang::ast_matchers::MatchFinder::MatchResult &Result) {

        clang::ASTContext *ctx = Result.Context;
        ASSERT(ctx);

        if(!m_mangle_ctx) {
            m_mangle_ctx = ctx->createMangleContext();
        }
        ASSERT(m_mangle_ctx);


        std::string name;
        llvm::raw_string_ostream raw(name);

        //        llvm::outs() << Result.Nodes.getMap().size() << " nodes matched\n";
        //        Context->DumpRecordLayout()        


        if(const auto decl = Result.Nodes.getNodeAs<clang::Decl>("decl")) {
            name = decl->getDeclKindName();
            if(name.compare("LinkageSpec") == 0
                    || name.compare("Namespace") == 0
                    || name.compare("ParmVar") == 0
                    || name.compare("Empty") == 0
                    || name.compare("TranslationUnit") == 0
                    || name.compare("Typedef") == 0
                    ) {
                return;
            }
            name.clear();
        }

        if(const auto decl = Result.Nodes.getNodeAs<clang::CXXRecordDecl>("decl")) {
            // Ignore
            // Определение класса, ссылка на класс (предварительное определение), в том числе скрытая
            if(decl->isReferenced() || decl->isImplicit() || decl->isClass()) {
                return;
            }
        }

        if(const auto decl = Result.Nodes.getNodeAs<clang::CXXDestructorDecl>("decl")) {
            //            decl->dump();
            //
            //            ASSERT(mangler->shouldMangleDeclName(decl));
            //            mangler->mangleCXXDtorThunk(clang::GlobalDecl(decl), raw);
            //            LOG_DEBUG("CXXDestructorDecl  %s", name.c_str());
            //            
        } else if(const auto decl = Result.Nodes.getNodeAs<clang::CXXConstructorDecl>("decl")) {
            //            decl->dump();
            //
            //            ASSERT(mangler->shouldMangleDeclName(decl));
            //            mangler->mangleCtorBlock(decl, raw);
            //            LOG_DEBUG("CXXConstructorDecl  %s", name.c_str());
            //
        } else if(const auto decl = Result.Nodes.getNodeAs<clang::FunctionDecl>("decl")) {
            if(m_mangle_ctx->shouldMangleCXXName(decl)) {
                m_mangle_ctx->mangleCXXName(decl, raw);

                if(name.compare("_Znwm") == 0
                        || name.compare("_Znam") == 0
                        || name.compare("_ZdlPv") == 0
                        || name.compare("_ZdaPv") == 0
                        || name.rfind("SERKS_") != std::string::npos //class_body::operator=(class_body const&)
                        ) {
                    return;
                }
            } else {
                name = decl->getNameAsString();
            }
            //                LOG_DEBUG("FunctionDecl %s", name.c_str());
            mangle_list.push_back(name);

        } else if(const auto decl = Result.Nodes.getNodeAs<clang::VarDecl>("decl")) {
            if(m_mangle_ctx->shouldMangleCXXName(decl)) {
                m_mangle_ctx->mangleCXXName(decl, raw);
            } else {
                name = decl->getNameAsString();
            }
            //                LOG_DEBUG("%s", decl->getNameAsString().c_str());
            mangle_list.push_back(name);

        } else if(const auto decl = Result.Nodes.getNodeAs<clang::FieldDecl>("decl")) {
            if(m_mangle_ctx->shouldMangleCXXName(decl)) {
                m_mangle_ctx->mangleCXXName(decl, raw);
            } else {
                name = decl->getNameAsString();
            }
            //                LOG_DEBUG("FieldDecl %s", name.c_str());
            mangle_list.push_back(name);

        } else if(const auto decl = Result.Nodes.getNodeAs<clang::CXXRecordDecl>("decl")) {
            decl->dump();
            LOG_RUNTIME("CXXRecordDecl");
        } else if(const auto decl = Result.Nodes.getNodeAs<clang::Decl>("decl")) {
            LOG_RUNTIME("DeclKindName: %s", decl->getDeclKindName());
        } else {
            for (const auto &elem : Result.Nodes.getMap()) {
                LOG_DEBUG("%s", elem.first.c_str());
            }
            LOG_RUNTIME("ELSE");
        }
    }
};



const char * func_text = \
            "int func(int arg, char, signed char, unsigned char);\n"
        "int func(signed int arg, unsigned int arg2, signed short int arg3, unsigned short int arg4);\n"
        "int func(const signed int arg, const unsigned int arg2);\n"
        "int func(int &arg, int &&arg2, int *arg3); //, int **arg4, int **arg5, int ****arg6 \n"
        "int func(const int &arg, const int &&arg2, const int *arg3); //, const int **arg4, const int **arg5, const int ****arg6 \n"
        "void func(int arg, double arg2);\n"
        "extern \"C\" int printf(const char * format, ...);\n"
        "const int value_const=0;\n"
        "namespace n { const int value_const=0; };\n"
        "int value_value;\n"
        "extern \"C\" int value_c;\n"
        "namespace n { int value; };\n"
        "class header_only;\n"
        "class class_body{\n"
        "  int field;\n"
        "  class_body(){};\n"
        "  virtual ~class_body(){};\n"
        "  void method_header();\n"
        "  void method_body(){};\n"
        "  static header_only * static_call(header_only *);"
        "};\n"
        "namespace name {\n"
        "int func(int arg);\n"
        "class ns_class {"
        "  void method();"
        "};"
        "};\n";

std::vector <const char *> mangle_name = {
    "_Z4funcicah",
    "_Z4funcijst",
    "_Z4funcij",
    "_Z4funcRiOiPi", //_Z4funcRiOiPiPS1_S2_PPS2_
    "_Z4funcRKiOS_PS_", //_Z4funcRKiOS_PS_PS2_S3_PPS3_
    "_Z4funcid",
    "printf",
    "_ZL11value_const",
    "_ZN1nL11value_constE",
    "value_value",
    "value_c",
    "_ZN1n5valueE",
    "_ZN10class_body5fieldE",
    "_ZN10class_body13method_headerEv",
    "_ZN10class_body11method_bodyEv",
    "_ZN10class_body11static_callEP11header_only",
    "_ZN4name4funcEi",
    "_ZN4name8ns_class6methodEv",
};

#define ATTR _GLIBCXX_VISIBILITY(default)

/* ::func(arg:Int32, arg:Int8, arg:Char, arg:Byte): Int32*/
ATTR int func1(int arg, char c1, signed char c2, unsigned char c3) {
    return arg;
}

/* ::func(arg:Int32, arg2:Dword, arg3:Int16, arg4:Word): Int32*/
ATTR int func2(signed int arg, unsigned int arg2, signed short int arg3, unsigned short int arg4) {
    return arg;
}

/* ::func(arg:&&Int32, arg2:&&&Int32, arg3:&Int32): Int32*/
ATTR int func3(int &arg, int &&arg2, int *arg3) {
    return arg;
}

/* ::func(arg:&&Int32^, arg2:&&&Int32^, arg3:&Int32^): Int32*/
ATTR int func4(const int &arg, const int &&arg2, const int *arg3) {
    return arg;
}

/* ::func(arg:Int32 arg2:Double): None*/
ATTR void func5(signed long long int arg, double arg2) {
}

/* ::value_const: Int32^ */
namespace n {
    ATTR const int value_const = 0;
}
/* ::value_const: Int32 */
ATTR int value_value;
/* value_c: Int32 */
extern "C" {
    ATTR int value_c;
}

/* ::n::value: Int32 */
namespace n {
    ATTR int value;
};

/*
 * 
 */
struct ImportType {
    void * ptr;
    const char * name;
    const char * mangle;
    const char * proto;
};
std::vector <ImportType> import_names = {
    {(void *) &value_c, "value_c:Int32", "value_c", "signed int value_c;"},
    {(void *) &func1, "::func1(arg:Int32, arg:Int8, arg:Char, arg:Byte): Int32", "_Z5func1icah", "signed int func1(signed int, char, signed char, unsigned char);"},
    {(void *) &func2, "::func2(arg:Int32, arg2:DWord, arg3:Int16, arg4:Word): Int32", "_Z5func2ijst", "signed int func2(signed int, unsigned int, signed short, unsigned short);"},
    {(void *) &func3, "::func3(arg:&&Int32, arg2:&&&Int32, arg3:&Int32): Int32", "_Z5func3RiOiPi", "signed int func3(signed int &, signed int &&, signed int *);"},
    {(void *) &func4, "::func4(arg:&&Int32^, arg2:&&&Int32^, arg3:&Int32^): Int32", "_Z5func4RKiOS_PS_", "signed int func4(const signed int &, const signed int &&, const signed int *);"},
    {(void *) &func5, "::func5(arg:Int64, arg2:Double):None", "_Z5func5xd", "void func5(signed long long int, double);"}, // 
    //    {(void *) &n::value_const, "::value_const: Int32^", "_ZN1nL11value_constE", ""}, //_ZN1nL11value_constE   value_const
    {(void *) &value_value, "value_value:Int32", "value_value", "signed int value_value;"},
    //    {(void *) &n::value, "::n::value:Int32", "_ZN1n5valueE", ""},
};

std::string toCXXProto(std::string name) {
    std::string result;
    name += ":={}";
    TermPtr term = Parser::ParseString(name, nullptr);
    if(term->Left()) {
        term = term->Left();

        if(!term->GetType()) {
            LOG_RUNTIME("Term %s type not defined!", term->toString().c_str());
        }

        result = term->m_text;

        if(!term->m_namespace.empty()) {

            //            LOG_DEBUG("%s", term->m_namespace.c_str());
        }

        ObjType type = typeFromString(term->GetType()->m_text, nullptr, nullptr);
        result.insert(0, " ");
        result.insert(0, toCXXType(type, true));

        if(term->m_is_call) {
            result += "(";
            for (int i = 0; i < term->size(); i++) {
                if(i) {
                    result += ", ";
                }
                TermPtr arg = term->at(i).second;

                if(!arg->GetType()) {
                    LOG_RUNTIME("Type arg %d not defined '%s'", i, term->toString().c_str());
                }

                if(arg->GetType()->m_is_const) {
                    result += "const ";
                }

                // LOG_DEBUG("%s", arg->m_type_name.c_str());
                ObjType arg_type = typeFromString(arg->GetType()->m_text, nullptr, nullptr);
                result += toCXXType(arg_type, true);
                if(arg->GetType()->m_ref) {
                    result += " ";
                    result += toCXXRef(arg->GetType()->m_ref->m_text);
                }
            }
            result += ")";
        }
        result += ";";
    }
    return result;
}

std::vector<std::string> toMangledName(std::vector<std::string> names) {

    std::string input_source;
    for (auto &elem : names) {
        input_source += toCXXProto(elem);
        input_source += "\n";
    }

    clang::CompilerInstance compilerInstance;

    constexpr int COUNT = 3;
    int argc = COUNT;
    const char *argv[COUNT] = {"", "fake.cpp", "--"};

    static llvm::cl::OptionCategory MyToolCategory("my-tool options");
    auto OptionsParser = clang::tooling::CommonOptionsParser::create(argc, argv, MyToolCategory, llvm::cl::OneOrMore);
    if(!OptionsParser) {
        LOG_RUNTIME("Fail create OptionsParser!");
    }

    clang::tooling::ClangTool Tool(OptionsParser->getCompilations(), OptionsParser->getSourcePathList());

    Tool.mapVirtualFile("fake.cpp", clang::StringRef(input_source));

    MyPrinter Printer;
    auto functionMatcher = clang::ast_matchers::decl(clang::ast_matchers::anything()).bind("decl");

    clang::ast_matchers::MatchFinder Finder;
    Finder.addMatcher(functionMatcher, &Printer);

    if(0 != Tool.run(clang::tooling::newFrontendActionFactory(&Finder).get())) {
        LOG_RUNTIME("Fail run FrontendAction!");
    }

    if(Printer.mangle_list.size() != names.size()) {
        LOG_RUNTIME("Fail matching result!");
    }
    return Printer.mangle_list;
}

inline std::string toMangledName(std::string name) {
    std::vector<std::string> names({name});
    return toMangledName(names)[0];
}

TEST(Compiler, MangleName) {

    EXPECT_STREQ("newlang_min", MangleName("мин").c_str()) << MangleName("мин");
    EXPECT_STREQ("newlang_maks", MangleName("макс").c_str()) << MangleName("макс");


    clang::CompilerInstance compilerInstance;

    constexpr int COUNT = 3;
    int argc = COUNT;
    const char *argv[COUNT] = {"", "fake.cpp", "--"};

    static llvm::cl::OptionCategory MyToolCategory("my-tool options");
    auto OptionsParser = clang::tooling::CommonOptionsParser::create(argc, argv, MyToolCategory, llvm::cl::OneOrMore);
    ASSERT_TRUE(!!OptionsParser);

    clang::tooling::ClangTool Tool(OptionsParser->getCompilations(), OptionsParser->getSourcePathList());

    Tool.mapVirtualFile("fake.cpp", clang::StringRef(func_text));

    MyPrinter Printer;
    auto functionMatcher = clang::ast_matchers::decl(clang::ast_matchers::anything()).bind("decl");

    clang::ast_matchers::MatchFinder Finder;
    Finder.addMatcher(functionMatcher, &Printer);

    ASSERT_EQ(0, Tool.run(clang::tooling::newFrontendActionFactory(&Finder).get()));

    ASSERT_EQ(Printer.mangle_list.size(), mangle_name.size());
    for (int i = 0; i < mangle_name.size(); i++) {
        EXPECT_STREQ(mangle_name[i], Printer.mangle_list[i].c_str()) << i;
    }

}

TEST(Compiler, ParserNS) {
    ASSERT_FALSE(LLVMSearchForAddressOfSymbol("value_c"));
    ASSERT_EQ(0, LLVMLoadLibraryPermanently(nullptr));
    ASSERT_TRUE(LLVMSearchForAddressOfSymbol("value_c"));
    ASSERT_TRUE(LLVMSearchForAddressOfSymbol("_Z5func5xd"));
    ASSERT_TRUE(LLVMSearchForAddressOfSymbol("_ZN1n5valueE"));


    for (int i = 0; i < import_names.size(); i++) {
        EXPECT_TRUE(LLVMSearchForAddressOfSymbol(import_names[i].mangle)) << "i: " << i << " -> '" << import_names[i].mangle << "'";
        EXPECT_STREQ(toCXXProto(import_names[i].name).c_str(), import_names[i].proto) << import_names[i].name;
        EXPECT_STREQ(toMangledName(import_names[i].name).c_str(), import_names[i].mangle) << import_names[i].name;
    }
}

void test_void_func() {
}
const int test_void_val = 0;

TEST(LLVM, Symbols) {

    auto rt = RunTime::Init();


    EXPECT_TRUE(LLVMSearchForAddressOfSymbol("printf"));
    EXPECT_TRUE(LLVMSearchForAddressOfSymbol("fopen"));

    EXPECT_FALSE(LLVMSearchForAddressOfSymbol("test_void_func"));
    EXPECT_FALSE(LLVMSearchForAddressOfSymbol("test_void_val"));

    LLVMAddSymbol("test_void_func", (void *) &test_void_func);
    LLVMAddSymbol("test_void_val", (void *) &test_void_val);

    EXPECT_TRUE(LLVMSearchForAddressOfSymbol("test_void_func"));
    EXPECT_TRUE(LLVMSearchForAddressOfSymbol("test_void_val"));

    EXPECT_EQ(&test_void_func, LLVMSearchForAddressOfSymbol("test_void_func"));
    EXPECT_EQ(&test_void_val, LLVMSearchForAddressOfSymbol("test_void_val"));
}



#define ASSERT_STRSTART(base, val) ASSERT_TRUE(str_cmp_strart(base, val))<<"  \""<<base<<"\"\n  \""<<val<<"\""
#define EXPECT_STRSTART(base, val) EXPECT_TRUE(str_cmp_strart(base, val))<<"  \""<<base<<"\"\n  \""<<val<<"\""

bool str_cmp_strart(const char *base_str, const char *cmp_str) {
    std::string base(base_str);
    std::string cmp(cmp_str);
    for (size_t i = 0; i < base.size() && i < cmp.size(); i++) {
        if(base[i] != cmp[i]) {

            return false;
        }
    }
    return base.size() <= cmp.size();
}

//bool simple_int_filter(Context *ctx, Object &in, ObjPtr & out) {
//    if(!ctx || !ctx->Current()->self) {
//        return false;
//    }
//    for (int i = 0; i < in.size(); i++) {
//        if(ctx->Current()->self->op_equal(in[i])) {
//            return false;
//        }
//    }
//    return true;
//}

///*
// * Основонй тест соответствия между компиляцией и интерпретацией текста
// программы
// * Реализован как набор выражений, возвращающих результат.
// * В начале все выражения выпоняются интерпретатором, тестируются на
// корректность и собираются в массив.
// * Далее выражения записываюстя в исходный фай в виде модуля, который
// компилируется, загружается и выполняется.
// * Результатом его выполнения так же является массив.
// * В итоге два маасива сравниваются между собой по элементно.
// */
// TEST(NewLang, CompileEvalConsistency) {
//
//    struct TestItem {
//        const char *result;
//        const char *eval;
//    };
//
//    // Выполняются построчно
//    std::vector<TestItem> eval_items = {
//        //        {"(val=456,)", "field_test $= (val=456,)"},
//        //        {"(val=456,)", "field_test()"},
//        //        {"123", "field_test.name := 123"},
//        //        {"(val=456, name=123,)", "field_test()"},
//        {"123.456", "float #= 123.456"},
//        {"муж.", "м #= \"муж.\""},
//        {"жен.", "ж #= \"жен.\""},
//        {"human=@term(пол=, parent=[,])", "human   @=  term(пол=,    parent  =
//        [,]   )"},
//        {"Tom2=human(пол=\"муж.\", parent=[,])", "Tom2 $= @human(пол=м)"},
//        {"Janna2=human(пол=\"жен.\", parent=[,])", "Janna2 $= @human(пол=ж)"},
//        {"Jake2=human(пол=\"муж.\", parent=[&Tom2, &Janna2,])", "Jake2 $=
//        @human(м, [  &Tom2, &Janna2, ] )"},
//        {"Tim2=human(пол=\"муж.\", parent=[&Tom2,])", "Tim2$=@human(пол=м,
//        parent=[&Tom2,])"},
//        //        {"brother={}", "brother(test1, test2) :&&= $test1!=$test2,
//        $test1.sex==м, intersec($test1.parent, $test2.parent)"},
//    };
//    // Возвращаются в виде элементов массива
//    std::vector<TestItem> array_items = {
//        {"123.456", "float"},
//        {"муж.", "м"},
//        {"жен.", "ж"},
//        {"100", "100"},
//        {"300", "100+200"},
//        {"100", "min(200,100,300)"},
//    };
//
//    RuntimePtr opts = NewLang::Init();
//    Context ctx(opts);
//    Parser parser;
//
//
//    // Литерал
//    ASSERT_STREQ("100", opts->Eval(&ctx, "100")->GetValueAsString().c_str());
//    // Оператор (символ +)
//    ASSERT_STREQ("300", opts->Eval(&ctx,
//    "100+200")->GetValueAsString().c_str());
//    // Вызов встроенной функции
//    Args min_args(Obj::CreateValue(200), Obj::CreateValue(100),
//    Obj::CreateValue(300)); // min(200,100,300) ASSERT_STREQ("100",
//    Context::CallByName(&ctx, "min", min_args)->GetValueAsString().c_str());
//
//    ObjPtr min_ret = newlang_min(&ctx, Args::Arg(Obj::CreateValue(200)),
//    Args::Arg(Obj::CreateValue(100)), Args::Arg(Obj::CreateValue(300)));
//    ASSERT_TRUE(min_ret);
//    ASSERT_STREQ("100", min_ret->GetValueAsString().c_str());
//
//
//    ASSERT_THROW(
//            opts->Eval(&ctx, "name2"),
//            std::runtime_error);
//    ASSERT_THROW(
//            opts->Eval(&ctx, "name3"),
//            std::runtime_error);
//
//    ASSERT_EQ(0, opts->m_user_terms.size());
//    ASSERT_TRUE(ctx.CreateGlobalTerm(Obj::CreateValue(111), "name1"));
//    ASSERT_EQ(1, opts->m_user_terms.size());
//    ASSERT_TRUE(opts->Eval(&ctx, "name2 $= 222; name3 $=
//    333")->GetValueAsString().c_str());
//
//    ASSERT_EQ(0, ctx.Current()->session.find("name2")->second->RefCount());
//    ASSERT_STREQ("222", opts->Eval(&ctx,
//    "name2")->GetValueAsString().c_str()); ASSERT_EQ(0,
//    ctx.Current()->session.find("name2")->second->RefCount());
//
//    ASSERT_STREQ("333", opts->Eval(&ctx,
//    "name3")->GetValueAsString().c_str());
//
//
//    Obj::Ptr field = Obj::CreateNone();
//    field->ItemAdd(Obj::CreateValue("FIELD1"), "field1");
//    field->ItemAdd(Obj::CreateValue("FIELD2"), "field2");
//    ASSERT_THROW(
//            Obj::GetField(field, "not found"),
//            std::runtime_error);
//
//    ASSERT_STREQ("FIELD1", Obj::GetField(field,
//    "field1")->GetValueAsString().c_str());
//
//    Obj::Ptr obj = Obj::CreateDict(Args::Arg("f", field),
//    Args::Arg("f2", field));
//
//    ASSERT_STREQ("FIELD1", Obj::GetField(Obj::GetField(obj, "f"),
//    "field1")->GetValueAsString().c_str()); ASSERT_STREQ("FIELD2",
//    Obj::GetField(Obj::GetField(obj, "f2"),
//    "field2")->GetValueAsString().c_str());
//
//    ASSERT_STREQ("(val=456,)", opts->Eval(&ctx, "field $=
//    (val=456,)")->GetValueAsString().c_str()); ASSERT_STREQ("(val=456,)",
//    opts->Eval(&ctx, "field")->GetValueAsString().c_str());
//    ASSERT_STREQ("123", opts->Eval(&ctx, "field.name :=
//    123")->GetValueAsString().c_str()); ASSERT_STREQ("(val=456, name=123,)",
//    opts->Eval(&ctx, "field")->GetValueAsString().c_str());
//
//
//
//
//    ASSERT_EQ(1, opts->m_user_terms.size());
//    ASSERT_STREQ("test=@term(пол=, parent=[,])", opts->Eval(&ctx, "test   @=
//    term(пол=,    parent  =   [,]   )")->GetValueAsString().c_str());
//    ASSERT_EQ(2, opts->m_user_terms.size());
//    ASSERT_STREQ("name1", opts->m_user_terms[0]->getName().c_str());
//
//    ASSERT_EQ(4, ctx.Current()->session.size());
//    ASSERT_EQ(0, ctx.Current()->session.find("name2")->second->RefCount());
//    ASSERT_STREQ("222",
//    ctx.Current()->session.find("name2")->second->toString().c_str());
//    ASSERT_STREQ("Tom=test(пол=222, parent=[,])", opts->Eval(&ctx, "Tom $=
//    @test(пол=name2)")->GetValueAsString().c_str()); ASSERT_EQ(0,
//    ctx.Current()->session.find("name2")->second->RefCount());
//
//    ASSERT_STREQ("[&Tom,]", opts->Eval(&ctx,
//    "ref$=[&Tom,]")->GetValueAsString().c_str()); ASSERT_STREQ("Tim=test(пол=,
//    parent=[&Tom,])", opts->Eval(&ctx, "Tim $=
//    @test(parent=[&Tom,])")->GetValueAsString().c_str());
//    ASSERT_STREQ("brother={}", opts->Eval(&ctx, "brother(test1, test2) :&&=
//    $test1!=$test2, $test1.sex==м, intersec($test1.parent,
//    $test2.parent)")->GetValueAsString().c_str());
//
//
//    std::string str = "#!./dist/Debug/GNU-Linux/nlc --exec\n"
//            "// Test source code for verify consistency between compilation
//            and evaluation modes\n"
//            "\n";
//
//    for (auto item : eval_items) {
//        ASSERT_STREQ(item.result, opts->Eval(&ctx,
//        item.eval)->GetValueAsString().c_str()); str += item.eval; str +=
//        ";\n";
//    }
//
//    str += "\nconsistency $= (,);\n";
//    for (auto item : array_items) {
//        ASSERT_STREQ(item.result, opts->Eval(&ctx,
//        item.eval)->GetValueAsString().c_str()); str += "consistency[] := ";
//        str += item.eval;
//        str += ";\n";
//    }
//
//    str += "\n$$ := consistency;\n";
//
//    std::filesystem::create_directories("temp");
//    ASSERT_TRUE(std::filesystem::is_directory("temp"));
//
//    std::ofstream out("temp/consistency.temp.sh");
//    out << str;
//    out.close();
//
//
//    Context ctx_module(NewLang::Init());
//    ObjPtr result_module = NewLang::ExecModule("temp/consistency.temp.sh",
//    "temp/consistency.temp.nlm", false, &ctx_module);
//    ASSERT_TRUE(result_module);
//
//
//    RuntimePtr copy = NewLang::Init();
//    Context ctx_copy(copy);
//    ObjPtr result_eval = copy->Eval(&ctx_copy, str.c_str());
//
//    ASSERT_TRUE(result_module->getItemCount());
//    ASSERT_EQ(array_items.size(), result_module->getItemCount());
//
//    ASSERT_TRUE(result_eval->getItemCount());
//    ASSERT_EQ(array_items.size(), result_eval->getItemCount());
//
//    RuntimePtr copy2 = NewLang::Init();
//    Context ctx_copy2(copy2);
//    ObjPtr res;
//    for (int i = 0; i < eval_items.size(); i++) {
//        res = copy2->Eval(&ctx_copy2, eval_items[i].eval);
//        ASSERT_TRUE(res);
//        ASSERT_STREQ(eval_items[i].result, res->GetValueAsString().c_str()) <<
//        eval_items[i].result << " != " << res->GetValueAsString();
//    }
//
//    for (int i = 0; i < array_items.size(); i++) {
//
//        res = copy2->Eval(&ctx_copy2, array_items[i].eval);
//        ASSERT_TRUE(res);
//        ASSERT_STREQ(array_items[i].result, res->GetValueAsString().c_str())
//        << array_items[i].result << " != " << res->GetValueAsString();
//
//        ObjPtr temp = result_module->getItem(i);
//        ASSERT_TRUE(temp->op_eq(res, true)) << i << "  " << res->toString();
//        temp = result_eval->getItem(i);
//        ASSERT_TRUE(temp->op_eq(res, true)) << i << "  " << res->toString();
//    }
//
//
//    //std::remove("temp/brother.sh.temp.nlm");
//}

// TEST(NewLang, JitLLVM) {
//
//     RuntimePtr opts = NewLang::Init();
//     Context ctx(opts);
//          const char *src = "\
//#include <stdio.h>\n\
//#include <string.h>\n\
//extern \"C\" int jit_print(const char *str) {\n\
//    printf(\"%s\\n\", str);\n\
//    return strlen(str);\n\
//}\n";
//
//     llvm::ExecutionEngine * e = opts->JITCompileCPP(src);
//
//
//     typedef int (*JIT_PRINT)(const char *);
//
//     e = opts->JITCompileCPP(src);
//     ASSERT_TRUE(e);
//     JIT_PRINT jit_print = reinterpret_cast<JIT_PRINT>
//     (opts->m_jit->GetFunction("jit_print")); ASSERT_TRUE(jit_print);
//     ASSERT_EQ(0, jit_print(""));
//     ASSERT_EQ(9, jit_print("jit_print"));
//
//
//
//    std::filesystem::create_directories("temp");
//    ASSERT_TRUE(std::filesystem::is_directory("temp"));
//
//     //    std::ofstream out("temp/native.temp.nlp");
//     //    out << "native(str=\"default arg\") $= { %{ $$=$str; printf(\"%s\",
//     static_cast<char *>($str)); %} };";
//     //    out.close();
//     //
//     //    ASSERT_TRUE(opts->CompileFunctions("native(str=\"default arg\") $=
//     { %{ $$=$str; printf(\"%s\", static_cast<char *>($str)); %} };"));
//     //    ASSERT_TRUE(opts->CompileModule("temp/native.temp.nlp",
//     "temp/native.temp.nlm"));
//     //    ASSERT_TRUE(opts->LoadModule("temp/native.temp.nlm"));
//     //
//     //    EXPECT_STREQ("default arg", opts->Eval(&ctx,
//     "native()")->GetValueAsString().c_str());
//     //    EXPECT_STREQ("default arg", opts->Eval(&ctx,
//     "@native()")->GetValueAsString().c_str());
//     //    EXPECT_STREQ("default arg", opts->Eval(&ctx,
//     "@native()")->GetValueAsString().c_str());
//     //    EXPECT_STREQ("", opts->Eval(&ctx,
//     "@native(\"\")")->GetValueAsString().c_str());
//     //    EXPECT_STREQ("Hello, world!\n", opts->Eval(&ctx, "@native(\"Hello,
//     world!\\n\")")->GetValueAsString().c_str());
// }

TEST(Compiler, DISABLED_Function) {

    const char *func_text =
            "func_sum(arg1, arg2) :- {$arg1 + $arg2;};\n"
            "func_call(arg1, arg2) :- {func_sum($arg1, $arg2);};";

    TermPtr funcs;
    Parser parser(funcs);

    parser.Parse(func_text);
    ASSERT_TRUE(funcs);
    ASSERT_EQ(TermID::BLOCK, funcs->GetTokenID());
    ASSERT_EQ(2, funcs->BlockCode().size());

    TermPtr func1 = funcs->BlockCode()[0];

    ASSERT_EQ(TermID::PUREFUNC, func1->GetTokenID());
    ASSERT_TRUE(func1->Left());
    ASSERT_EQ(2, func1->Left()->size());

    ASSERT_TRUE(func1->Right());
    TermPtr body1 = func1->Right();

    //    ASSERT_EQ(TermID::LOCAL, body1->getTermID());
    //    ASSERT_TRUE(body1->Left());
    //    ASSERT_TRUE(body1->Right());
    //    ASSERT_STREQ(":=", body1->getText().c_str());
    //    ASSERT_EQ(TermID::RESULT, body1->Left()->getTermID());
    //    ASSERT_STREQ("$$", body1->Left()->getText().c_str());
    //    ASSERT_EQ(TermID::OPERATOR, body1->Right()->getTermID());

    TermPtr op = body1->Right();
    //    ASSERT_TRUE(op);
    //    ASSERT_TRUE(op->Left());
    //    ASSERT_TRUE(op->Right());
    //    ASSERT_EQ(TermID::TERM, op->Left()->getTermID());
    //    ASSERT_STREQ("+", op->getText().c_str());
    //    ASSERT_STREQ("$arg1", op->Left()->getText().c_str());
    //    ASSERT_STREQ("$arg2", op->Right()->getText().c_str());


    TermPtr func2 = funcs->BlockCode()[1];

    ASSERT_EQ(TermID::PUREFUNC, func2->GetTokenID());
    ASSERT_TRUE(func2->Left());
    ASSERT_EQ(2, func2->Left()->size());

    //    ASSERT_TRUE(func2->Right());
    //    TermPtr body2 = func2->Right();
    //    ASSERT_TRUE(body2);
    //
    //    ASSERT_TRUE(body2->Left());
    //    ASSERT_TRUE(body2->Right());
    //    ASSERT_EQ(TermID::LOCAL, body2->getTermID());
    //    ASSERT_STREQ(":=", body2->getText().c_str());
    //    ASSERT_EQ(TermID::RESULT, body2->Left()->getTermID());
    //    ASSERT_STREQ("$$", body2->Left()->getText().c_str());
    //
    //    op = body2->Right();
    //    ASSERT_TRUE(op);
    //    ASSERT_FALSE(op->Left());
    //    ASSERT_FALSE(op->Right());
    //    ASSERT_EQ(TermID::CALL, op->getTermID());
    //    ASSERT_STREQ("func_sum", op->getText().c_str());
    //    ASSERT_EQ(2, op->size());
    //    ASSERT_STREQ("$arg1", (*op)[0]->getText().c_str());
    //    ASSERT_STREQ("$arg2", (*op)[1]->getText().c_str());


    std::ostringstream sstr;
    ASSERT_TRUE(NewLang::MakeCppFile(funcs, sstr)); // << sstr.str();


    std::filesystem::create_directories("temp");
    ASSERT_TRUE(std::filesystem::is_directory("temp"));

    std::ofstream file("temp/function_test.temp.cpp");
    file << sstr.str();
    file.close();
}

TEST(Compiler, DISABLED_FuncsTypes) {

    /*
     * - Проверка типов аргументов при вызове функций
     * - Проверка типов возвращаемых значений у функций
     */

    RuntimePtr opts = RunTime::Init();
    Context ctx(opts);

#define FUNC_ARG "func_arg(arg1: Int8, arg2): Int8 := { $arg1+$arg2; };"
#define FUNC_RES "func_res(arg1: Int8, arg2: Int32): Integer := { $arg2+=$arg1; };"

    TermPtr func;
    Parser parser(func);

    std::ostringstream sstr;

    // Не соответствие типа функции в операторе
    parser.Parse(FUNC_ARG FUNC_RES "\n$res:Int8 := func_arg(100, 100); $res += func_res(100, 100);");
    sstr.str("");
    ASSERT_THROW(NewLang::MakeCppFile(func, sstr, nullptr, &ctx), Interrupt) << sstr.str();

    // Компилится без ошибок
    parser.Parse(FUNC_ARG "\nfunc_arg(Int8(100), 100);");
    sstr.str("");
    ASSERT_NO_THROW(NewLang::MakeCppFile(func, sstr, nullptr, &ctx)) << sstr.str();

    // Не соответствие типа первого аргумента
    parser.Parse(FUNC_ARG "\nfunc_arg(1000, 100);");
    sstr.str("");
    ASSERT_THROW(NewLang::MakeCppFile(func, sstr, nullptr, &ctx), Interrupt) << sstr.str();

    // Не соответствие типа функции
    parser.Parse(FUNC_ARG FUNC_RES "\n$res:Int8 := func_res(100, 1000);");
    sstr.str("");
    ASSERT_THROW(NewLang::MakeCppFile(func, sstr, nullptr, &ctx), Interrupt) << sstr.str();

    // Не соответствие типа функции в операторе
    parser.Parse(FUNC_ARG FUNC_RES "\n$res:Int8 := func_arg(100, 100); $res += func_res(100, 100);");
    sstr.str("");
    ASSERT_THROW(NewLang::MakeCppFile(func, sstr, nullptr, &ctx), Interrupt) << sstr.str();

    // Нет типа у $res как в предыдщем случае
    parser.Parse(FUNC_ARG FUNC_RES "\n$res := func_arg(100, 100); $res += func_res(100, 100);");
    sstr.str("");
    ASSERT_NO_THROW(NewLang::MakeCppFile(func, sstr, nullptr, &ctx)) << sstr.str();

    // Тип есть, но делается каст возвращаемого типа у функции
    parser.Parse(FUNC_ARG FUNC_RES "\n$res: Int8 := func_arg(100, 100); $res += Int8(func_res(100, 100));");
    sstr.str("");
    ASSERT_NO_THROW(NewLang::MakeCppFile(func, sstr, nullptr, &ctx)) << sstr.str();


    std::filesystem::create_directories("temp");
    ASSERT_TRUE(std::filesystem::is_directory("temp"));


    std::ofstream file("temp/call_types.temp.cpp");
    file << sstr.str();
    file.close();

    std::string out;
    int exit_code;
    ASSERT_TRUE(NewLang::GccMakeModule("temp/call_types.temp.cpp",
            "temp/call_types.temp.nlm", nullptr,
            &out, &exit_code))
            << exit_code << " " << out;

    ASSERT_TRUE(ctx.m_runtime->LoadModule("call_types.temp.nlm", false, &ctx));

    // Переполнение байтовой переменной $res во время выполнения последнего оператора "+="
    //    Obj args;
    //    ASSERT_TRUE(ctx.m_runtime->m_modules["temp/call_types.temp.nlm"]);
    //    ASSERT_NO_THROW(
    //            ctx.m_runtime->m_modules["temp/call_types.temp.nlm"]->Main(&ctx, args));
    //@todo Контроль переполнения при операциях для типизированных переменных
    //????????????????
    //@todo Такой же как и для остальных операций
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // ASSERT_THROW(opts->m_modules["temp/call_types.temp.nlm"]->Main(&ctx,
    // args), newlang_exception);

#undef FUNC_ARG
#undef FUNC_RES

}
#endif // UNITTEST