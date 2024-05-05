#include "nlc-rt.h"
#include "module.h"

using namespace newlang;

void Module::RegisterStaticObject(Module &module, TermPtr op, bool init) {
    ASSERT(op);

    if (op->isCreate() && Term::isExportName(op->m_left)) {
        ASSERT(!op->m_left->m_int_name.empty());

        auto found = module.find(op->m_left->m_int_name);
        if (found == module.end()) {

            ObjPtr obj;
//            VarItem item;
//            item.item = op->m_left;
//            item.sync = Context::CreateSync(op->m_left);
            if (init) {
//                item.obj = Context::EvalTerm(op->m_right, nullptr, true);
                obj = Context::EvalTerm(op->m_right, nullptr, true);
            } else {
//                item.obj = Obj::CreateEmpty();
                obj = Obj::CreateEmpty();
            }
//            item.obj->m_sync = item.sync.get();
//            item.item->m_obj = item.obj;

            VarItem item = Context::CreateItem(op->m_left, std::move(obj));
            if (isGlobalScope(op->m_left->m_text) && module.m_rt) {
                module.m_rt->NameRegister(op->isCreateOnce(), op->m_left->m_int_name, item.item); //, item.obj);
            }
            module.insert(std::pair<std::string, VarItem>(op->m_left->m_int_name, std::move(item)));

        } else {
            if (op->isCreateOnce()) {
                LOG_RUNTIME("Object name '%s' (%s) already exist!", op->m_left->m_int_name.c_str(), op->m_left->toString().c_str());
            }
        }

    } else if (op->m_right) {
        RegisterStaticObject(module, op->m_right, false);
    }

    for (auto &elem : op->m_follow) {
        RegisterStaticObject(module, elem, false);
    }
    for (auto &elem : op->m_block) {
        RegisterStaticObject(module, elem, init && op->m_id == TermID::SEQUENCE);
    }
}

Module::Module(RunTime *rt, TermPtr ast) : m_rt(rt), m_ast(ast) {//: Obj(ObjType::Module) {

    if (ast) {

        ASSERT(rt);

        RegisterStaticObject(*this, ast, false);


        // Только создание пустых объектов без их инициалиазации.
        // Инициализация будет во время выполнения,
        // а к тому времени все объекты должны быть уже созданы
        //        for (auto &var_mod : ast->m_variables) {
        //            ASSERT(var_mod.second);
        ////            if (isStaticName(var_mod.second->m_text)) {
        ////
        ////                // Статические обекты
        ////                ASSERT(var_mod.second.expr);
        ////                ASSERT(var_mod.second.expr->m_right);
        ////                ObjPtr obj = RunTime::EvalStatic(var_mod.second.expr->m_right, var_mod.second.expr->IsPure());
        ////                insert({var_mod.first,
        ////                    {var_mod.second.proto, obj}});
        ////
        ////            } else {
        ////                //  Временные объекты не создаются
        ////                ASSERT(isLocalName(var_mod.second.proto->m_text));
        ////            }
        //
        //
        //            for (auto &var_local : var_mod.second->m_variables) {
        //
        ////                if (isStaticName(var_local.second.proto->m_text)) {
        ////
        ////                    // Статические обекты
        ////                    ASSERT(var_mod.second.expr);
        ////                    ObjPtr obj = RunTime::EvalStatic(var_mod.second.expr->m_right, var_mod.second.expr->IsPure());
        ////                    insert({var_mod.first,
        ////                        {var_mod.second.proto, obj}});
        ////
        ////                } else {
        ////                    //  Временные объекты не создаются
        ////                    ASSERT(isLocalName(var_local.second.proto->m_text));
        ////                }
        //            }
        //
        //        }
    }
    //            m_var_is_init = true;
}
