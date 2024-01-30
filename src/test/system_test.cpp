#include "pch.h"
#include "runtime.h"

#ifdef UNITTEST

#include <warning_push.h>
#include <gtest/gtest.h>
#include <warning_pop.h>

#include <system.h>

using namespace newlang;
using namespace newlang::runtime;

class SystemTest : public ::testing::Test {
protected:



    std::vector<std::string> m_postlex;

    utils::Logger::FuncCallback *m_log_callback_save;
    void *m_log_callback_arg_save;
    std::string m_output;

    static void LoggerCallback(void *param, utils::Logger::LogLevelType level, const char * str, bool flush) {
        SystemTest *p = static_cast<SystemTest *> (param);
        fprintf(stdout, "%s", str);
        if (flush) {
            fflush(stdout);
        }
        if (p) {
            p->m_output += str;
        }
    }

    void SetUp() {
        utils::Logger::Instance()->SaveCallback(m_log_callback_save, m_log_callback_arg_save);
        utils::Logger::Instance()->Clear();
        utils::Logger::Instance()->SetCallback(&LoggerCallback, this);
    }

    void TearDown() {
        utils::Logger::Instance()->SetCallback(m_log_callback_save, m_log_callback_arg_save);
    }

    TermPtr Parse(std::string str, MacroPtr buffer = nullptr, DiagPtr diag = nullptr) {
        m_postlex.clear();
        ast = Parser::ParseString(str, buffer, &m_postlex, diag);
        return ast;
    }

    int Count(TermID token_id) {
        int result = 0;
        for (int c = 0; c < ast->size(); c++) {
            if ((*ast)[c].second->m_id == token_id) {
                result++;
            }
        }
        return result;
    }

    std::string LexOut() {
        std::string result;
        for (int i = 0; i < m_postlex.size(); i++) {
            if (!result.empty()) {
                result += " ";
            }
            result += m_postlex[i];
        }
        trim(result);
        return result;
    }

    TermPtr ast;
};


/*
 
   :System().run("echo", "Hello from the other side!");

    os := :System();
    
    home_dir := os.run("cd", "~");
    printf("`cd ~` ran with exit code %d", os.exitcode());

    unknown_dir := os.run("cd", "doesnotexist");
    printf("`cd doesnotexis` ran with exit code %d", os.exitcode());

 * 
    `echo Hello from the other side!`;

 * 
    `echo Hello from the other side!`;
    res := $^;
    printf("`cd doesnotexis` ran with exit code %d", res.exitcode());

 * 
 */


//#include <cstdio>
//#include <iostream>
//#include <memory>
//#include <stdexcept>
//#include <string>
//#include <array>

//std::string exec(const char* cmd) {
//    std::array<char, 128> buffer;
//    std::string result;
//    std::unique_ptr<FILE, decltype(&pclose) > pipe(popen(cmd, "r"), pclose);
//    if (!pipe) {
//        throw std::runtime_error("popen() failed!");
//    }
//    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
//        result += buffer.data();
//    }
//    return result;
//}


//    os.system(command) - исполняет системную команду, возвращает код её завершения (в случае успеха 0).
//
//    
//    os.chdir(path) - смена текущей директории.
//    os.chmod(path, mode, *, dir_fd=None, follow_symlinks=True) - смена прав доступа к объекту (mode - восьмеричное число).
//    os.chown(path, uid, gid, *, dir_fd=None, follow_symlinks=True) - меняет id владельца и группы (Unix).
//    os.getcwd() - текущая рабочая директория.
//    os.link(src, dst, *, src_dir_fd=None, dst_dir_fd=None, follow_symlinks=True) - создаёт жёсткую ссылку.
//    os.listdir(path=".") - список файлов и директорий в папке.
//    os.mkdir(path, mode=0o777, *, dir_fd=None) - создаёт директорию. OSError, если директория существует.
//    os.makedirs(path, mode=0o777, exist_ok=False) - создаёт директорию, создавая при этом промежуточные директории.
//    os.remove(path, *, dir_fd=None) - удаляет путь к файлу.
//    os.rename(src, dst, *, src_dir_fd=None, dst_dir_fd=None) - переименовывает файл или директорию из src в dst.
//    os.renames(old, new) - переименовывает old в new, создавая промежуточные директории.
//    os.replace(src, dst, *, src_dir_fd=None, dst_dir_fd=None) - переименовывает из src в dst с принудительной заменой.
//    os.rmdir(path, *, dir_fd=None) - удаляет пустую директорию.
//    os.removedirs(path) - удаляет директорию, затем пытается удалить родительские директории, и удаляет их рекурсивно, пока они пусты.
//    os.symlink(source, link_name, target_is_directory=False, *, dir_fd=None) - создаёт символическую ссылку на объект.
//    os.sync() - записывает все данные на диск (Unix).
//    os.truncate(path, length) - обрезает файл до длины length.
//    os.utime(path, times=None, *, ns=None, dir_fd=None, follow_symlinks=True) - модификация времени последнего доступа и изменения файла. Либо times - кортеж (время доступа в секундах, время изменения в секундах), либо ns - кортеж (время доступа в наносекундах, время изменения в наносекундах).
//    os.walk(top, topdown=True, onerror=None, followlinks=False) - генерация имён файлов в дереве каталогов, сверху вниз (если topdown равен True), либо снизу вверх (если False). Для каждого каталога функция walk возвращает кортеж (путь к каталогу, список каталогов, список файлов).

TEST_F(SystemTest, MethodExist) {

    struct Method {
        std::string name;
        ObjPtr res;
    };

    std::vector<Method> list;

    list.push_back({"getname", nullptr});
    //    list.push_back("getlogin");
    //    list.push_back("getenv");
    //    list.push_back("getpid");
    //    list.push_back("getuname");
    std::string callname;
    for (auto &elem : list) {
        callname = "System::";
        callname += elem.name;
        callname += "()";
    }

    //    CALSS_METHOD(System, getname);

    ObjPtr arg = Obj::CreateDict();
    arg->push_back(Obj::Arg());
    arg->push_back(Obj::Arg("ls -l"));

    ObjPtr ret = System::system(nullptr, *arg);

    std::cout << ret->toString().c_str();

}

TEST_F(SystemTest, StaticAssert) {

    RuntimePtr rt = RunTime::Init();

    ASSERT_TRUE(rt);
    ASSERT_TRUE(rt->m_macro);
    ASSERT_FALSE(rt->m_macro->empty());
    ASSERT_FALSE(rt->m_macro->find("static_assert") == rt->m_macro->end()) << rt->m_macro->Dump();



    TermPtr term = rt->GetParser()->Parse("@static_assert(1)");
    ASSERT_TRUE(term);
    ASSERT_TRUE(term->getTermID() == TermID::END);

    ASSERT_ANY_THROW(rt->GetParser()->Parse("@static_assert(0)"));
    ASSERT_ANY_THROW(rt->GetParser()->Parse("@static_assert(0.0)"));
    ASSERT_ANY_THROW(rt->GetParser()->Parse("@static_assert(0\\1)"));
    ASSERT_ANY_THROW(rt->GetParser()->Parse("@static_assert('')"));
    ASSERT_ANY_THROW(rt->GetParser()->Parse("@static_assert(\"\")"));
    ASSERT_ANY_THROW(rt->GetParser()->Parse("@static_assert($local)"));
    ASSERT_ANY_THROW(rt->GetParser()->Parse("@static_assert(::global)"));
    ASSERT_ANY_THROW(rt->GetParser()->Parse("@static_assert(@macro)"));

    ASSERT_NO_THROW(rt->GetParser()->Parse("@static_assert(1)"));
    ASSERT_NO_THROW(rt->GetParser()->Parse("@static_assert(1.0)"));
    ASSERT_NO_THROW(rt->GetParser()->Parse("@static_assert(1\\1)"));
    ASSERT_NO_THROW(rt->GetParser()->Parse("@static_assert(' ')"));
    ASSERT_NO_THROW(rt->GetParser()->Parse("@static_assert(\" \")"));

    ASSERT_NO_THROW(rt->GetParser()->Parse("@static_assert(%printf)"));

    ASSERT_ANY_THROW(rt->GetParser()->Parse("@static_assert(%FAIL_NATIVE_NAME)"));
    int FAIL_NATIVE_NAME;
    LLVMAddSymbol("FAIL_NATIVE_NAME", (void *) &FAIL_NATIVE_NAME);
    ASSERT_NO_THROW(rt->GetParser()->Parse("@static_assert(%FAIL_NATIVE_NAME)"));

    ASSERT_NO_THROW(rt->GetParser()->Parse("@static_assert(1==1)"));
    ASSERT_NO_THROW(rt->GetParser()->Parse("@static_assert(1.0===1.0)"));
    ASSERT_NO_THROW(rt->GetParser()->Parse("@static_assert(1\\1 <= 10)"));
    ASSERT_NO_THROW(rt->GetParser()->Parse("@static_assert(''=='')"));
    ASSERT_NO_THROW(rt->GetParser()->Parse("@static_assert(\"str\"=='str')"));

    ASSERT_ANY_THROW(rt->GetParser()->Parse("@static_assert(\"str2222\"=='str')"));

}

TEST_F(SystemTest, Assert) {

    RuntimePtr rt = RunTime::Init();

    ASSERT_TRUE(rt);
    ASSERT_TRUE(rt->m_assert_enable);

    ASSERT_TRUE(rt->m_macro);
    ASSERT_FALSE(rt->m_macro->empty());
    ASSERT_FALSE(rt->m_macro->find("assert") == rt->m_macro->end()) << rt->m_macro->Dump();
    ASSERT_FALSE(rt->m_macro->find("verify") == rt->m_macro->end()) << rt->m_macro->Dump();

    TermPtr term;

    ASSERT_NO_THROW(
            term = rt->GetParser()->Parse("  :Bool(1)");
            ) << rt->Dump();

    ASSERT_NO_THROW(
            term = rt->GetParser()->Parse("@assert(1+1, 2, 'str')");
            ) << rt->Dump();
    ASSERT_TRUE(term);
    ASSERT_TRUE(term->getTermID() == TermID::FOLLOW) << term->toString();
    ASSERT_STREQ("[:Bool(1 + 1) == 0]-->{::Base::__assert_abort__(\"1+1\", 1 + 1, 2, 'str');};", term->toString().c_str());

    Context ctx1(rt);
    Obj args;
    ObjPtr result;

    ASSERT_NO_THROW(
            result = ctx1.Eval(&ctx1, term, &args, true);
            ) << rt->Dump();

    ASSERT_NO_THROW(
            term = rt->GetParser()->Parse("@verify(1)");
            ) << rt->Dump();
    ASSERT_TRUE(term);
    ASSERT_TRUE(term->getTermID() == TermID::FOLLOW) << term->toString();
    ASSERT_STREQ("[:Bool(1) == 0]-->{::Base::__assert_abort__(\"1\", 1);};", term->toString().c_str());

    ASSERT_NO_THROW(
            result = ctx1.Eval(&ctx1, term, &args, true);
            ) << rt->Dump();

    term = rt->GetParser()->Parse("@verify(0, 3+4, '555')");
    ASSERT_TRUE(term);
    ASSERT_TRUE(term->getTermID() == TermID::FOLLOW) << term->toString();
    ASSERT_STREQ("[:Bool(0) == 0]-->{::Base::__assert_abort__(\"0\", 0, 3 + 4, '555');};", term->toString().c_str());

    ASSERT_ANY_THROW(
            result = ctx1.Eval(&ctx1, term, &args, true);
            ) << rt->Dump();



    rt = RunTime::Init({"path", "-nlc-no-assert"});
    ASSERT_FALSE(rt->m_assert_enable);

    ASSERT_TRUE(rt->m_macro);
    ASSERT_FALSE(rt->m_macro->empty());
    ASSERT_FALSE(rt->m_macro->find("assert") == rt->m_macro->end()) << rt->m_macro->Dump();
    ASSERT_FALSE(rt->m_macro->find("verify") == rt->m_macro->end()) << rt->m_macro->Dump();



    ASSERT_NO_THROW(
            term = rt->GetParser()->Parse("@assert(1+1, 2, 'str'); 33+44; 55");
            ) << rt->Dump();
    ASSERT_TRUE(term);
    ASSERT_TRUE(term->getTermID() == TermID::BLOCK) << toString(term->getTermID()) << " " << term->toString();
    ASSERT_STREQ("{_; 33 + 44; 55;}", term->toString().c_str());

    Context ctx2(rt);
    ASSERT_NO_THROW(
            result = ctx2.Eval(&ctx1, term, &args, true);
            ) << rt->Dump();

    ASSERT_NO_THROW(
            term = rt->GetParser()->Parse("@verify(1+1, 'message'); 33+44");
            );
    ASSERT_TRUE(term);
    ASSERT_TRUE(term->getTermID() == TermID::BLOCK) << term->toString();
    ASSERT_STREQ("{1 + 1; 33 + 44;}", term->toString().c_str());

    ASSERT_NO_THROW(
            result = ctx2.Eval(&ctx1, term, &args, true);
            );
    ASSERT_NO_THROW(
            term = rt->GetParser()->Parse("@verify(0+0)");
            );
    ASSERT_TRUE(term);
    ASSERT_TRUE(term->getTermID() == TermID::OPERATOR) << term->toString();
    ASSERT_STREQ("0 + 0", term->toString().c_str());

    ASSERT_NO_THROW(
            result = ctx2.Eval(&ctx1, term, &args, true);
            );
}

//TEST_F(SystemTest, Base) {
//
//    //    struct Method {
//    //        std::string name;
//    //        ObjPtr res;
//    //    };
//    //
//    //    std::vector<Method> list;
//    //
//    //    list.push_back({"getname", nullptr});
//    //    //    list.push_back("getlogin");
//    //    //    list.push_back("getenv");
//    //    //    list.push_back("getpid");
//    //    //    list.push_back("getuname");
//    //    std::string callname;
//    //    for (auto &elem : list) {
//    //        callname = "System::";
//    //        callname += elem.name;
//    //        callname += "()";
//    //    }
//
//    //    CALSS_METHOD(System, getname);
//
//    ObjPtr arg = Obj::CreateDict();
//    arg->push_back(Obj::Arg());
//    arg->push_back(Obj::Arg("ls -l"));
//
//    ObjPtr ret = Base::__pragma_assert__(nullptr, *arg);
//
//    std::cout << ret->toString().c_str();
//
//   RuntimePtr rt = RunTime::Init();
//   
////   rt->m_macro.find("@assert")
//   
//
//
//
//}

//TEST_F(SystemTest, MethodRun) {
//
//    ObjPtr sys = std::make_shared<System>();
//
//}

//TEST_F(SystemTest, Runtime) {
//
//    RuntimePtr rt = RunTime::Init();
//    for (auto &elem : *rt) {
//        ASSERT_TRUE(elem.second) << elem.first;
//        ASSERT_TRUE(elem.second->proto) << elem.first;
//        std::cout << elem.first << " -> " << elem.second->proto->toString() << "\n";
//    }
//
//    Context ctx(rt);
//
//    TermPtr term = Parser::ParseString(":System::getlogin()", nullptr);
//    ASSERT_TRUE(term);
//    std::cout << "\nTerm: " << term->toString() << "!!!!!!!!!!!!!!!!!!!!!!!!!!11\n";
//
//    ObjPtr result = ctx.Run(term, nullptr);
//    ASSERT_TRUE(result);
//
//    std::cout << result->toString() << "!!!!!!!!!!!!!!!!!!!!!!!!!!11";
//
//}

TEST_F(SystemTest, Native) {

    RuntimePtr rt = RunTime::Init();
    Context ctx(rt);

    ASSERT_ANY_THROW(ctx.ExecStr("time(:Pointer):Int32 := %time"));
    ASSERT_ANY_THROW(ctx.ExecStr("time(arg):Int32 := %time"));
    ASSERT_ANY_THROW(ctx.ExecStr("time(arg:Rational):Int32 := %time"));

    ObjPtr usleep = ctx.ExecStr("usleep(usec:DWord64):Int32 := %usleep");
    ASSERT_TRUE(usleep);

    ObjPtr time = ctx.ExecStr("time(ptr:Pointer):Int32 := %time");
    ASSERT_TRUE(time);
    
    ASSERT_TRUE(time->m_prototype);
    ASSERT_EQ(1, time->m_prototype->size());
    ASSERT_STREQ("ptr", (*time->m_prototype)[0].second->m_text.c_str());
    
    ASSERT_TRUE((*time->m_prototype)[0].second->m_type);
    ASSERT_STREQ(":Pointer", (*time->m_prototype)[0].second->m_type->m_text.c_str());
    
    ASSERT_TRUE(time->m_prototype->m_type);
    ASSERT_STREQ(":Int32", time->m_prototype->m_type->m_text.c_str());

//    ASSERT_STREQ("%time(ptr:Pointer):Int32", time->m_prototype->toString().c_str())<< time->toString();

    ObjPtr res;

    ASSERT_NO_THROW(
            res = time->Call(&ctx, Obj::ArgNull());
            ) << time->m_prototype->toString();

    std::cout << time->toString();

    usleep->Call(&ctx, Obj::Arg(1000000));

    ObjPtr res2 = time->Call(&ctx, Obj::ArgNull());

    std::cout << "\nres1: " << res->toString() << "\n";
    std::cout << "res2: " << res2->toString() << "\n";
    (*res2) -= res;
    std::cout << "res2: " << res2->toString() << "\n";

    std::cout << "\nusleep: " << res2->toString() << "\n";

    ObjPtr srand = ctx.ExecStr("srand(val:Int32):None := %srand");
    ASSERT_TRUE(srand);

    srand->Call(&ctx, Obj::Arg(time->Call(&ctx, Obj::ArgNull())));

    ObjPtr rand = ctx.ExecStr("rand():Int32 := %rand");
    ASSERT_TRUE(rand);

    std::cout << rand->toString() << " -> " << rand->Call(&ctx) << ", " << rand->Call(&ctx) << "\n";

}

/**
    rand():Int32 := %rand;
    usleep(usec:DWord64):None := %usleep;
    printf(format:FmtChar, ...):Int32 := %printf;

 */

TEST_F(SystemTest, Logger) {
    //    std::cout << exec("ls -l");
    //    std::system("ls -l > temp/test.txt"); // executes the UNIX command "ls -l >test.txt"
    //    std::cout << std::ifstream("temp/test.txt").rdbuf();
}


#endif // UNITTEST