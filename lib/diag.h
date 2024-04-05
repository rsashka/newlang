#ifndef NEWLANG_DIAG_H_
#define NEWLANG_DIAG_H_

//#include "pch.h"

#include "nlc-rt.h"
//#include <variable.h>

#include "warning_push.h"
#include "parser.yy.h"
#include "warning_pop.h"

namespace newlang {

    /*
    -Werror     Turn warnings into errors.
    -Werror=foo Turn warning “foo” into an error.
    -Wno-error=foo  Turn warning “foo” into a warning even if -Werror is specified.
    -Wfoo       Enable warning “foo”. See the diagnostics reference for a complete list of the warning flags that can be specified in this way.
    -Wno-foo    Disable warning “foo”.
    -w          Disable all diagnostics.
    
     * For "-Wfoo"
     * -Werror=foo
     * -Wno-error=foo
     * -Wno-foo
     * 
     * -Wclang-error
     * For "-Wclang-*"
     * -Werror=clang-*
     * -Wno-error=clang-*
     * -Wno-clang-*

     */
    class Diag {
    public:


#define DIAG_STATE(_) \
        _(none)     /* not exist */ \
        _(flag)     /* set flag only (-w, -Weverything, -pedantic,  -fdiagnostics-format=clang/msvc/vi  ...) */ \
        _(option)   /* enable or disable diagnostic (-f[no-]caret-diagnostics, -f[no-]show-source-location ...) */ \
        _(ignored)  /* mutable diagnostic status -Wfoo, -Wno-foo, -Werror=foo, -Wno-error=foo */ \
        _(warning)  /* mutable diagnostic status -Wfoo, -Wno-foo, -Werror=foo, -Wno-error=foo */ \
        _(error)    /* mutable diagnostic status -Wfoo, -Wno-foo, -Werror=foo, -Wno-error=foo */

        enum class State : uint8_t {
#define DEFINE_ENUM(name) name,
            DIAG_STATE(DEFINE_ENUM)
#undef DEFINE_ENUM
        };

        inline static const char * toString(State state) {
            switch (state) {

#define DEFINE_CASE(name)   \
  case State::name:     \
    return #name;
                    DIAG_STATE(DEFINE_CASE)
#undef DEFINE_CASE

                default:
                    return nullptr;
            }
        }

        struct DataDiag {
            std::string Name;
            Diag::State State;
            const char *Desc;
        };

        typedef std::vector<DataDiag> DiagListType;
        typedef std::vector<DiagListType> DiagStackType;


        inline static const char * DIAG_MACRO_NOT_FOUND = "-Wmacro-not-found";
        inline static const char * DIAG_MACRO_STORAGE_NOT_EXIST = "-Wmacro-storage-not-exist";
        inline static const char * DIAG_ERROR_LIMIT = "-ferror-limit="; //-ferror-limit=20
        inline static const char * DIAG_EXTRA_TOKENS = "-Wextra-tokens";
        inline static const char * DIAG_FILL_REMAINDER = "-Wfill-remainder";
        

        inline static bool IsClang(const std::string name) {
            return name.find("-clang") == 0;
        }

        inline static bool IsTurnError(const std::string name) {
            return name.find("-Werror=") == 0;
        }

        inline static bool IsTurnNoError(const std::string name) {
            return name.find("-Wno-error=") == 0;
        }

        inline static bool IsDisable(const std::string name) {
            return name.find("-Wno-") == 0;
        }

        inline static bool IsEnable(const std::string name) {
            return name.find("-W") == 0 && !IsDisable(name)&& !IsTurnError(name)&& !IsTurnNoError(name);
        }

        inline static std::string RemoveDiagPrefix(const std::string name) {

            std::string result(IsClang(name) ? name.substr(6) : name); // Remove prefix "-clang"

            if (IsEnable(result)) {
                return result.substr(2); // Remove prefix "-W"
            }

            ASSERT(!IsDisable(result));
            ASSERT(!IsTurnError(result));
            ASSERT(!IsTurnNoError(result));

            return result; // Return any options ???
        }

        static std::string ChangeState(const std::string name, State from_state, State to_state);

        //-Werror: Turn warnings into errors.
        //-Werror=foo: Turn warning "foo" into an error.
        //-Wno-error=foo: Turn warning "foo" into an warning even if -Werror is specified.

        //-Wno-foo: Disable warning foo
        //-Wfoo: Enable warning foo
        //-w: Disable all warnings.

        //-Wno-error

        static DiagPtr Init(int argc = 0, const char** argv = nullptr) {
            DiagPtr diag = std::make_shared<Diag>();
            if (!diag->ParseArgs(argc, argv)) {
                LOG_RUNTIME("Fail parse args!");
            }
            return diag;
        }

        void Push(const TermPtr term = nullptr);
        void Pop(const TermPtr term = nullptr);
        bool Apply(const char *name, State state, const TermPtr term = nullptr);

        Diag::State Test(const char *name);
        bool TestIgnore(const char *name);
        bool Emit(const char *name, const TermPtr term = nullptr);

        bool Register(const char *name, State state, const char *desc = nullptr);

        virtual ~Diag() {
        }

        Diag();

//        SCOPE(private) :

        bool ParseArgs(int argc, const char** argv) {

            for (int i = 0; i < argc; i++) {

                //                m_args->push_back(Obj::CreateString(argv[i]));
                //
                //                if (strstr(argv[i], "--nlc-") == argv[i]) {
                //                    if (strstr(argv[i], "--nlc-search=") == argv[i]) {
                //                        std::string list(argv[i]);
                //                        list = list.substr(strlen("--nlc-search="));
                //                        m_search_dir = Context::SplitString(list.c_str(), ";");
                //                    } else if (strstr(argv[i], "--nlc-search=") == argv[i]) {
                //                    } else {
                //                        LOG_RUNTIME("System arg '%s' not found!", argv[i]);
                //                    }
                //                }

            }


            //            llvm::SmallString<1024> path;
            //            auto error = llvm::sys::fs::current_path(path);
            //            if (error) {
            //                LOG_RUNTIME("%s", error.message().c_str());
            //            }
            //            m_work_dir = path.c_str();
            //            // LOG_DEBUG("work_dir: %s", m_work_dir.c_str());
            //
            //            path = llvm::sys::fs::getMainExecutable(nullptr, nullptr);
            //            // LOG_DEBUG("%s", path.c_str());
            //
            //            llvm::sys::path::remove_filename(path);
            //            m_exec_dir = path.c_str();
            //            // LOG_DEBUG("exec_dir: %s", m_exec_dir.c_str());
            //
            //            m_search_dir.push_back(m_work_dir);
            //            m_search_dir.push_back(m_exec_dir);

            return true;
        }

        DiagStackType m_diag_stack;
    
        int m_error_limit;
        int m_error_count;
        bool m_fill_remainder;


//        Diag();
        Diag(const Diag&) = delete;
        const Diag& operator=(const Diag&) = delete;

    };


} // namespace example

#endif // NEWLANG_DIAG_H_
