#pragma once
#ifndef INCLUDED_NEWLANG_CONTEXT_
#define INCLUDED_NEWLANG_CONTEXT_

#include "term.h"

namespace newlang {

    /*
     * Context - место хранения локальных (автоматических) переменных
     * Module - место хранения статических переменных модуля
     * RunTime - место хранения глобальных статических переменных
     */

    /* 
     * Выполняет модуль (файл/ast)
     */
    struct VarItem {
        TermPtr item;
        ObjPtr obj;
        std::unique_ptr<Sync> sync;
#ifdef BUILD_DEBUG
        void * term_check;
        void * obj_check;
#endif        
    };

    struct VarScope {
        TermID id;
        std::string ns;
        std::map<std::string, VarItem> vars;
    };

    class Context : public std::vector<VarScope>, public std::enable_shared_from_this<Context> {
        friend class CtxPush;

    public:
        RunTime *m_runtime;
        ObjPtr m_latter;
        Module &m_static;

        Context(Module &module, RunTime *rt);

        virtual ~Context() {
        }

        static ObjPtr Run(TermPtr ast, Context * runner);

        static ObjPtr EvalTerm(TermPtr term, Context *runner, bool rvalue = true);

        static std::unique_ptr<Sync> CreateSync(const TermPtr &term);

        static VarItem CreateItem(TermPtr term, ObjPtr obj);

        VarItem * FindVarItem(const TermPtr &term) {
            if (!isInternalName(term->m_int_name)) {
                LOG_RUNTIME("'%s' is not an internal name!", term->m_text.c_str());
            }
            return FindVarItem(term->m_int_name);
        }
        VarItem * FindVarItem(const std::string_view name);


        TermPtr GetObject(const std::string_view int_name);
        TermPtr FindInternalName(const std::string_view int_name);


        static ObjPtr StringFormat(std::string_view format, Obj & args);
        static ObjPtr StringPrintf(std::string_view format, Obj & args);

        /**
         * Метод вызывается только из NewLnag кода
         * @param runner
         * @param obj
         * @param term
         * @return 
         */
        static ObjPtr Call(Context *runner, Obj &obj, TermPtr & term);
        /**
         * Метод может быть вызван как из NewLnag кода, так и из кода на C/C++ (в реализации Obj::operator())
         * @param runner
         * @param obj
         * @param term
         * @return 
         */
        static ObjPtr Call(Context *runner, Obj &obj, Obj & args);

        std::string Dump();

    protected:

        void PushScope(const TermID id, const std::string_view &name) {
            std::string ns;
            if (!name.empty()) {
                ns = name;
                if (ns.rfind("::") != ns.size() - 2) {
                    ns += "::";
                }
            }
            push_back({id, ns,
                {}});
        }

        void PopScope() {
            ASSERT(size());
            pop_back();
        }

        bool FindScope(const std::string_view &name) {
            auto iter = rbegin();
            while (iter != rend()) {
                if (iter->ns.compare(name.begin()) == 0) {
                    return iter == rbegin();
                }
                iter++;
            }
            LOG_RUNTIME("Named block '%s' not found!", name.begin());
        }

        static bool HasReThrow(TermPtr &block, Context &stack, Obj &obj);

        std::string MakeNamespace(int skip, bool is_global) {
            std::string result;
            auto iter = rbegin();
            if (skip > 0) {
                iter += skip;
            }
            int count = 0;
            while (iter != rend()) {
                if (result.find("::") == 0) {
                    break;
                }
                if (skip < 0 && count == -skip) {
                    break;
                }
                if (!iter->ns.empty() && (!is_global || !isdigit(iter->ns[0]))) {
                    // The namespace is always padded with ::
                    result.insert(0, iter->ns);
                }
                iter++;
                count++;
            }
            return result;
        }

        static RunTime * GetRT_(Context * runner) {
            return runner ? runner->m_runtime : nullptr;
        }

        ObjPtr CreateNative_(TermPtr &proto, const char *module, bool lazzy, const char *mangle_name);
        static ObjPtr CallNative_(Context *runner, Obj & obj, Obj *args = nullptr);

        /**
         * Выполняет одну операцию
         * @param op
         * @param run
         * @return 
         */
        static ObjPtr CheckObjTerm_(TermPtr & term, Context * runner, bool rvalue = true) {
            if (!term->m_obj) {
                term->m_obj = EvalTerm(term, runner, rvalue);
            }
            return term->m_obj;
        }

        static ObjPtr CreateArgs_(TermPtr &term, Context * runner);
        static ObjPtr CreateDict(TermPtr &term, Context * runner);
        static ObjPtr CreateRange(TermPtr &term, Context * runner);
        static ObjPtr CreateTensor(TermPtr &term, Context * runner);

        static ObjPtr SetIndexValue(TermPtr &term, ObjPtr &value, Context * runner);
        static ObjPtr SetFieldValue(TermPtr &term, ObjPtr &value, Context * runner);
        static ObjPtr GetIndexValue(TermPtr &term, ObjPtr &value, Context * runner);
        static ObjPtr GetFieldValue(TermPtr &term, ObjPtr &value, Context * runner);


        /* 
         * a, b, c :=  value
         * a() :=  {  }     func
         * 
         * a, b, c :=  ... dict     
         * a, b :=  b, a    swap
         * 
         * a() :=  {  }     func
         * && a() :=  {  }  async func     val :=  *a();  **( val := *a()){   },[...] {  };   -> val :=  await  a() ??????????????????????
         * a() :=  %() {  } coro  co_yeld, co_return,  co_wait  ????????????????
         */
        ObjPtr EvalCreate_(TermPtr & op);

        ObjPtr EvalCreateAsValue_(TermPtr & op);
        ObjPtr EvalCreateAsFunc_(TermPtr & op);
        ObjPtr EvalCreateAsEllipsis_(TermPtr & op);
        ObjPtr EvalCreateAsFilling_(TermPtr & op);

        void EvalLeftVars_(ArrayTermType &vars, const TermPtr &op);
        //        ObjPtr AssignVars_(ArrayTermType &vars, const TermPtr &r_term, bool is_pure);



        ObjPtr EvalIterator_(TermPtr & term);
        ObjPtr EvalInterrupt_(TermPtr & term);

        ObjPtr EvalOp_(TermPtr & op);

        static ObjPtr EvalOpMath_(TermPtr &op, Context * runner);
        static ObjPtr EvalOpLogical_(TermPtr &op, Context * runner);
        static ObjPtr EvalOpCompare_(TermPtr &op, Context * runner);
        static ObjPtr EvalOpBitwise_(TermPtr &op, Context * runner);
        static ObjPtr EvalRange_(TermPtr &op, Context * runner);

        ObjPtr EvalTake_(TermPtr & op);
        ObjPtr EvalWhile_(TermPtr & op);
        ObjPtr EvalDoWhile_(TermPtr & op);

        static ObjPtr EvalFollow_(TermPtr & op, Context * runner);


        ObjPtr EvalTryBlock_(TermPtr &block, StorageTerm & storage);
        ObjPtr EvalEval_(TermPtr & op);

    };

    class CtxPush {
    public:
        Context &m_ctx;

        CtxPush(Context &ctx, const TermID id, const TermPtr & ns) : m_ctx(ctx) {
            m_ctx.PushScope(id, ns ? ns->m_text : "");
        }

        ~CtxPush() {
            m_ctx.PopScope();
        }
    };

}
#endif //INCLUDED_NEWLANG_CONTEXT_
