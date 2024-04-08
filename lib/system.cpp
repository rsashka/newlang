//#include "pch.h"

#include <sys/sysinfo.h>
#include <sys/time.h>

#include "warning_push.h"
#include <torch/torch.h>
#include <ATen/ATen.h>
#include "warning_pop.h"

#include "system.h"
#include "runtime.h"

using namespace newlang;
using namespace runtime;

ObjType getSummaryTensorType(Obj *obj, ObjType start);
ObjPtr CreateTensor(torch::Tensor tensor);

bool Buildin::AddMethod(const char * name, ObjPtr obj) {
//    if (find(name) != end()) {
//        LOG_ERROR("Name '%s' already exist!", name);
//        return false;
//    }
//    std::string fullname(m_file);
//    fullname += "::";
//    fullname += name;
//
//    TermPtr proto = *const_cast<TermPtr *> (&obj->m_prototype);
//    proto->m_text = fullname;
//    proto->m_int_name = NormalizeName(fullname);
//    //    proto->m_obj = obj;
//
//    insert({fullname,
//        {proto, obj}});

    LOG_RUNTIME("New method '%s'!", name);//fullname.c_str());
    return true;
}

bool Buildin::CreateMethodNative(const char * proto, void * addr) {
    TermPtr term = Parser::ParseTerm(proto, nullptr, false);

    if (!term) {
        LOG_ERROR("Fail parse '%s'!", proto);
        return false;
    }
    ASSERT(0);
//    return AddMethod(term->getText().c_str(), m_runtime->CreateNative(term, addr));

}

bool Buildin::CreateMethod(const char * proto, FunctionType & func, ObjType type) {
    ASSERT(proto);

    TermPtr term = Parser::ParseTerm(proto, nullptr, false);

    if (!term) {
        LOG_ERROR("Fail parse '%s'!", proto);
        return false;
    }
    ASSERT(0);
//    return AddMethod(term->getText().c_str(), m_runtime->CreateFunction(term, (void *) &func)); //, type
}

bool Buildin::CreateProperty(const char * proto, ObjPtr obj) {
    return true;
}

/*
 * 
 * :Base ::= :Class() {
 * 
 * };
 * 
 */
CALSS_METHOD(Base, __assert_abort__) {

    //    ARG_TEST_COUNT(1);
    std::string message;
    for (int i = 1; i < in.size(); i++) {
        if (in.at(i).second) {
            message += in.at(i).second->toString();
        } else {
            message += "nullptr";
        }
    }

    LOG_RUNTIME("Assert abort '%s'!", message.c_str());
    return Obj::CreateNone();
}

/*
 * 
 * :System ::= :Class() {
 * 
 *      getname():String ::= {%
 * 
 *          char hostname[HOST_NAME_MAX];
 *          ::gethostname(hostname, sizeof (hostname));
 * 
 *          return Obj::CreateString(hostname);
 * 
 *      %}
 * 
 *      getenv(name:String):String ::= {%
 *          ARG_TEST_COUNT(1);
 *          ARG_TEST_TYPE(1, ObjType::String);
 * 
 *          return Obj::CreateString(::getenv(static_cast<const char *>($name)));
 *      %}
 * 
 * };
 * 
 */
CALSS_METHOD(System, getname) {

    char hostname[HOST_NAME_MAX];
    gethostname(hostname, sizeof (hostname));

    return Obj::CreateString(hostname);
}

CALSS_METHOD(System, getlogin) {

    char username[LOGIN_NAME_MAX];
    getlogin_r(username, sizeof (username));

    return Obj::CreateString(username);
}

#define ARG_TEST_COUNT(cnt) if (in.size() != cnt + 1) { \
                                LOG_RUNTIME("Only %d argument expected!", cnt); \
                            }

//#define ARG_POS_EXIST(pos)  if (in.size() >= pos) { \
//                                LOG_RUNTIME("Arguments %d missing!", pos); \
//                            }

CALSS_METHOD(System, getenv) {

    ARG_TEST_COUNT(1);
    if (!in.at(1).second || !in.at(1).second->is_string_type()) {
        LOG_RUNTIME("String type argument expected, not '%s'!", newlang::toString(in.at(1).second->m_var_type_current));
    }

    return Obj::CreateString(std::getenv(in.at(1).second->GetValueAsString().c_str()));
}

CALSS_METHOD(System, getuname) {

    struct utsname *buf = nullptr;
    if (0 == uname(buf) && buf) {

        ObjPtr result = Obj::CreateDict();

        result->push_back(Obj::CreateString(buf->sysname), "sysname");
        result->push_back(Obj::CreateString(buf->nodename), "nodename");
        result->push_back(Obj::CreateString(buf->release), "release");
        result->push_back(Obj::CreateString(buf->version), "version");
        result->push_back(Obj::CreateString(buf->machine), "machine");

        return result;
    }

    LOG_RUNTIME("Fail call uname()");
}

CALSS_METHOD(System, getsysinfo) {

    struct sysinfo *info = nullptr;
    if (0 == sysinfo(info) && info) {

        ObjPtr result = Obj::CreateDict();

        result->push_back(Obj::CreateValue(info->uptime), "uptime");
        //        torch::Tensor ten=torch::from_blob(info->loads, {3}).clone(); //torch::tensor({info->loads[0], info->loads[1], info->loads[2]});//torch::Tensor(info->loads, 3, at::ScalarType::Long);
        result->push_back(CreateTensor(torch::from_blob(info->loads, {
            3
        })), "loads");
        result->push_back(Obj::CreateValue(info->totalram), "totalram");
        result->push_back(Obj::CreateValue(info->freeram), "freeram");
        result->push_back(Obj::CreateValue(info->sharedram), "sharedram");
        result->push_back(Obj::CreateValue(info->bufferram), "bufferram");
        result->push_back(Obj::CreateValue(info->totalswap), "totalswap");
        result->push_back(Obj::CreateValue(info->freeswap), "freeswap");
        result->push_back(Obj::CreateValue(info->procs), "procs");
        result->push_back(Obj::CreateValue(info->totalhigh), "totalhigh");
        result->push_back(Obj::CreateValue(info->freehigh), "freehigh");
        result->push_back(Obj::CreateValue(info->mem_unit), "mem_unit");

        return result;
    }

    LOG_RUNTIME("Fail call sysinfo()");
}

CALSS_METHOD(System, getmillisec) {
    struct timeval tp;
    ::gettimeofday(&tp, NULL);
    return Obj::CreateValue(tp.tv_sec * 1000 + tp.tv_usec / 1000);
}

CALSS_METHOD(System, gettimeofday) {

    struct timeval tp;
    ::gettimeofday(&tp, NULL);

    ObjPtr result = Obj::CreateDict();
    result->push_back(Obj::CreateValue(tp.tv_sec), "tv_sec");
    result->push_back(Obj::CreateValue(tp.tv_usec), "tv_usec");
    return result;
}

CALSS_METHOD(System, getcwd) {
    char dir[PATH_MAX];
    ::getcwd(dir, PATH_MAX);
    return Obj::CreateString(dir);
}


#if defined(WIN) && (_MSC_VER >= 1900)
extern "C" __p__environ();
#else
extern "C" char ** environ;
#endif

CALSS_METHOD(System, getenviron) {

    char ** env;
#if defined(WIN) && (_MSC_VER >= 1900)
    env = *__p__environ();
#else
    env = environ;
#endif

    std::vector<std::string> split;
    ObjPtr result = Obj::CreateDict();
    for (; env && *env; ++env) {
        split = RunTime::SplitString(*env, "=");
        result->push_back(Obj::CreateString(&(*env)[split[0].size() + 1]), split[0]);
    }
    return result;
}

CALSS_METHOD(System, system) {

    ARG_TEST_COUNT(1);
    if (!in.at(1).second || !in.at(1).second->is_string_type()) {
        LOG_RUNTIME("String type argument expected, not '%s'!", newlang::toString(in.at(1).second->m_var_type_current));
    }

    // we use std::array rather than a C-style array to get all the
    // C++ array convenience functions
    std::array<char, 128> buffer;
    std::string result;

    // popen() receives the command and parameter "r" for read,
    // since we want to read from a stream.
    // by using unique_ptr, the pipe object is automatically cleaned
    // from memory once we've read all the data from the pipe.
    std::unique_ptr<FILE, decltype(&pclose) > pipe(popen(in.at(1).second->GetValueAsString().c_str(), "r"), pclose);

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return Obj::CreateString(result);
}

//bool Buildin::CreateMacro(const char * text) {
//
//    //                try {
//    //                    MacroPtr macro;
//    //                                    TermPtr m = Parser::ParseTerm(text, nulltr, false);
//    //                } catch (...) {
//    //                    return false;
//    //                }
//    //                m_macro_list.push_back(text);
//    if (m_runtime) {
//        TermPtr m = Parser::ParseTerm(text, m_runtime->m_macro, false);
//    }
//    return true;
//}

Base::Base(RuntimePtr rt) : Buildin(rt, ObjType::Base, "::Base") {

    bool dsl_enable = true;
    if (rt) {
        dsl_enable = rt->m_load_dsl;
    }

//    if (dsl_enable) {
//
//        VERIFY(CreateMacro("@@ true @@ ::= 1"));
//        VERIFY(CreateMacro("@@ yes @@ ::= 1"));
//        VERIFY(CreateMacro("@@ false @@ ::= 0"));
//        VERIFY(CreateMacro("@@ no @@ ::= 0"));
//
//        VERIFY(CreateMacro("@@ if( ... ) @@ ::= @@ [ @$... ] --> @@"));
//        VERIFY(CreateMacro("@@ elif( ... ) @@ ::= @@ ,[ @$... ] --> @@"));
//        VERIFY(CreateMacro("@@ else @@ ::= @@ ,[...] --> @@"));
//
//        VERIFY(CreateMacro("@@ while( ... ) @@ ::= @@ [ @$... ] <-> @@"));
//        VERIFY(CreateMacro("@@ dowhile( ... ) @@ ::= @@ <-> [ @$... ] @@"));
//        VERIFY(CreateMacro("@@ loop @@ ::= @@ [ 1 ] <-> @@"));
//
//
//        VERIFY(CreateMacro("@@ break $label @@ ::= @@ @$label :: ++ @@"));
//        VERIFY(CreateMacro("@@ continue $label @@ ::= @@ @$label :: -- @@"));
//        VERIFY(CreateMacro("@@ return( result ) @@ ::= @@ @__FUNC_BLOCK__ ++ @$result ++ @@"));
//        VERIFY(CreateMacro("@@ throw( result ) @@ ::= @@ -- @$result -- @@"));
//
//        VERIFY(CreateMacro("@@ match( ... ) @@ ::= @@ [ @$... ] @__PRAGMA_EXPECTED__( @\\ =>, @\\ ==>, @\\ ===>, @\\ ~>, @\\ ~~>, @\\ ~~~> ) @@"));
//        VERIFY(CreateMacro("@@ case( ... ) @@ ::= @@ [ @$... ] --> @@"));
//        VERIFY(CreateMacro("@@ default @@ ::= @@ [...] --> @@"));
//
//
//        VERIFY(CreateMacro("@@ this @@ ::= @@ $0 @@ ##< This object (self)"));
//        VERIFY(CreateMacro("@@ self @@ ::= @@ $0 @@ ##< This object (self)"));
//        VERIFY(CreateMacro("@@ super @@ ::= @@ $$ @@ ##< Super (parent) class or function"));
//        VERIFY(CreateMacro("@@ latter @@ ::= @@ $^ @@  ##< Result of the last operation"));
//
//
//        VERIFY(CreateMacro("@@ try @@ ::= @@ [ {*  @__PRAGMA_EXPECTED__( @\\ { ) @@"));
//        VERIFY(CreateMacro("@@ catch(...) @@ ::= @@ *} ] : < @$... > ~> @@"));
//        VERIFY(CreateMacro("@@ forward @@ ::= @@ +- $^ -+ @@  ##< Forward latter result or exception"));
//
//
//        VERIFY(CreateMacro("@@ iter( obj, ... ) @@ ::= @@ @$obj ? (@$...) @@"));
//        VERIFY(CreateMacro("@@ next( obj, ... ) @@ ::= @@ @$obj ! (@$...) @@"));
//        VERIFY(CreateMacro("@@ curr( obj ) @@ ::= @@ @$obj !? @@"));
//        VERIFY(CreateMacro("@@ first( obj ) @@ ::= @@ @$obj !! @@"));
//        VERIFY(CreateMacro("@@ all( obj ) @@ ::= @@ @$obj ?? @@"));
//
//        VERIFY(CreateMacro("@@ and @@ ::= @@ && @@"));
//        VERIFY(CreateMacro("@@ or @@ ::= @@ || @@"));
//        VERIFY(CreateMacro("@@ xor @@ ::= @@ ^^ @@"));
//        VERIFY(CreateMacro("@@ not(value) @@ ::= @@ (:Bool(@$value)==0) @@"));
//
//        //                    VERIFY(CreateMacro("@@ root() @@ ::= @@ @# @\\\\ @@"));
//        //                    VERIFY(CreateMacro("@@ module() @@ ::= @@ @# $\\\\ @@"));
//
//        //                    VERIFY(CreateMacro("@@ namespace() @@ ::= @@ @# @:: @@"));
//        VERIFY(CreateMacro("@@ module() @@ ::= @@ @$$ @@"));
//        VERIFY(CreateMacro("@@ static @@ ::= @@ @:: @@"));
//        VERIFY(CreateMacro("@@ package $name @@ ::= @@  @$$ = @# @$name @@"));
//        VERIFY(CreateMacro("@@ declare( obj ) @@ ::= @@ @$obj ::= ... @@  ##< Forward declaration of the object"));
//
//        VERIFY(CreateMacro("@@ using(...) @@ ::= @@ ... = @$... @@"));
//
//        VERIFY(CreateMacro("@@ typedef(cnt) @@ ::= @@ @__PRAGMA_TYPE_DEFINE__(@$cnt) @@ ##< Disable warning when defining a type inside a namespace"));
//
//        VERIFY(CreateMacro("@@ coroutine @@ ::= @@ __ANNOTATION_SET__(coroutine) @@"));
//        VERIFY(CreateMacro("@@ co_yield  $val  @@ ::= @@ __ANNOTATION_CHECK__(coroutine) @__FUNC_BLOCK__ :: -- @$val -- @@"));
//        VERIFY(CreateMacro("@@ co_await        @@ ::= @@ __ANNOTATION_CHECK__(coroutine) @__FUNC_BLOCK__ :: +- @@"));
//        VERIFY(CreateMacro("@@ co_return $val  @@ ::= @@ __ANNOTATION_CHECK__(coroutine) @__FUNC_BLOCK__ :: ++ @$val ++ @@"));
//
//        VERIFY(CreateMacro("@@ exit(code) @@ ::= @@ :: ++ @$code ++ @@"));
//        VERIFY(CreateMacro("@@ abort() @@ ::= @@ :: -- @@"));
//
//
//
//    }


    bool assert_enable = true;
    if (rt) {
        assert_enable = rt->m_assert_enable;
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

    // @__PRAGMA_ASSERT__ replase to __pragma_assert__ in @ref Parser::PragmaEval
    // @__PRAGMA_STATIC_ASSERT__
    VERIFY(CreateMethod("__assert_abort__(...):None", __assert_abort__));

//    VERIFY(CreateMacro("@@ static_assert(...) @@ ::= @@ @__PRAGMA_STATIC_ASSERT__(@$... ) @@"));
//
//    if (assert_enable) {
//        VERIFY(CreateMacro("@@ assert(value, ...) @@ ::= @@ [:Bool(@$value)==0]-->{ ::Base::__assert_abort__(@# @$value, @$value, @$... ) } @@"));
//        VERIFY(CreateMacro("@@ verify(value, ...) @@ ::= @@ [:Bool(@$value)==0]-->{ ::Base::__assert_abort__(@# @$value, @$value, @$... ) } @@"));
//    } else {
//        VERIFY(CreateMacro("@@ assert(value, ...) @@ ::= @@ (_) @@"));
//        VERIFY(CreateMacro("@@ verify(value, ...) @@ ::= @@ (@$value) @@"));
//    }


}
