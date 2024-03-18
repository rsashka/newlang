#include "pch.h"

#include <context.h>
#include <runtime.h>
#include <term.h>
#include <types.h>
#include <filesystem>
#include <stdbool.h>
#include <macro.h>

using namespace newlang;

std::multimap<std::string, DocPtr> Docs::m_docs;

//ObjPtr Context::CREATE_OR_ASSIGN(Context *ctx, const TermPtr &term, Obj *local_vars, CreateMode mode) {
//    // Присвоить значение можно как одному термину, так и сразу нескольким при
//    // раскрытии словаря: var1, var2, _ = ... func(); 
//    // Первый и второй элементы словаря записывается в var1 и var2, а остальные элементы возвращаемого
//    // словаря игнорируются (если они есть)
//    // var1, var2 = ... func(); Если функция вернула словрь с двумя элементами, 
//    // то их значения записываются в var1 и var2. Если в словаре было больше двух элементов, 
//    // то первый записывается в var1, а оставшиеся в var2. !!!!!!!!!!!!!
//    // var1, ..., var2 = ... func(); 
//    // Первый элемент словаря записывается в var1, а последний в var2.
//
//    ASSERT(term && (term->getTermID() == TermID::ASSIGN ||
//            term->getTermID() == TermID::CREATE_ONCE ||
//            term->getTermID() == TermID::CREATE_OVERLAP));
//    ASSERT(term->Left());
//
//
//    std::vector<TermPtr> list_term; // Список терминов для создания/присвоения значения
//    std::vector<ObjPtr> list_obj; // Список реальных переменных ассоциированных с терминами
//
//    bool is_eval_block = true;
//
//    TermPtr next = term->Left();
//    while (next && next->getTermID() != TermID::END) {
//        if (next->isCall()) {
//            is_eval_block = false;
//        }
//        list_term.push_back(next);
//        next = next->m_list;
//    }
//
//    if (!is_eval_block && list_term.size() > 1) {
//        //@todo добавить поддержку присвоения сразу нескольким функциям
//        NL_PARSER(term, "Multiple function assignment not implemented!");
//    }
//
//    ObjPtr result;
//    bool is_ellipsis = false;
//    for (auto & elem : list_term) {
//
//        if (elem->getTermID() == TermID::ELLIPSIS) {
//
//            //@todo добавить поддержку многоточия с левой стороный оператора присвоения
//            NL_PARSER(elem, "Ellipsis on the left side in assignment not implemented!");
//
//            if (is_ellipsis) {
//                NL_PARSER(elem, "Multiple ellipsis on the left side of the assignment!");
//            }
//            is_ellipsis = true;
//            result = Obj::CreateType(ObjType::Ellipsis);
//        } else if (elem->getTermID() == TermID::NONE) {
//            result = Obj::CreateNone();
//        } else {
//            // LOG_DEBUG("find: %s", ctx->NamespaceFull(elem->GetFullName()).c_str());
//            auto found = ctx->find(elem->GetFullName());
//            if (found == ctx->end() && mode == CreateMode::ASSIGN_ONLY) {
//                NL_PARSER(elem, "Object '%s' (%s) not found!", elem->m_text.c_str(), elem->GetFullName().c_str());
//            }
//
//            if (found != ctx->end()) {
//                result = (*found).second.lock(); // Но она может быть возвращена как локальная
//            }
//
//            if (result && mode == CreateMode::CREATE_ONLY) {
//                NL_PARSER(elem, "Object '%s' (%s) already exists!", elem->m_text.c_str(), elem->GetFullName().c_str());
//            }
//
//            if (!term->Right()) { // Удаление глобальной переменной
//                ctx->ListType::erase(found);
//            } else {
//                if (!result && (mode == CreateMode::ASSIGN_ONLY)) {
//                    NL_PARSER(term->Left(), "Object '%s' (%s) not found!", term->Left()->m_text.c_str(), elem->GetFullName().c_str());
//                }
//                if (!result) {
//                    result = CreateLVal(ctx, elem, local_vars);
//                    if (!result) {
//                        NL_PARSER(term->Left(), "Fail create lvalue object!");
//                    }
//                }
//            }
//        }
//        list_obj.push_back(result);
//    }
//
//    if (!term->Right()) {
//        // Для удаления переменных все сделано
//        return result;
//    }
//
//    // Что присваиваем (правая часть выражения) - пока единичный объект
//    // @todo В будущем можно будет сделать сахар для обмена значениями при одинаковом кол-ве объектов у оператора присваивания
//    // a, b = b, a;   a, b, c = c, b, a; и т.д.
//    if (term->Right() && term->Right()->getTermID() != TermID::ELLIPSIS && term->Right()->m_list) {
//        NL_PARSER(term->Right()->Right(), "Multiple assignments not implemented!");
//    }
//
//    ObjPtr rval;
//    if (term->Right()->getTermID() == TermID::ELLIPSIS) {
//        ASSERT(term->Right()->Right());
//        rval = Eval(ctx, term->Right()->Right(), local_vars, is_eval_block, CatchType::CATCH_AUTO);
//    } else if (term->Right()->getTermID() == TermID::CLASS) {
//        ASSERT(list_obj.size() == 1);
//        // Имя класса появляется только при операции присвоения в левой части оператора
//        rval = ctx->CreateClass(term->Left()->GetFullName(), term->Right(), local_vars);
//    } else if (term->Right()->getTermID() == TermID::NATIVE) {
//        // Нативное имя допустимо только в правой части части оператора присвоения
//        ASSERT(list_obj.size() == 1);
//
//        if (term->Left()->isCall() != term->Right()->isCall()) {
//            // Нативная функция с частичным прототипом
//            TermPtr from;
//            TermPtr to;
//            if (term->Left()->isCall()) {
//                from = term->Left();
//                to = term->Right();
//            } else {
//                ASSERT(term->Right()->isCall());
//                from = term->Right();
//                to = term->Left();
//            }
//            for (int i = 0; i < from->size(); i++) {
//                to->push_back((*from)[i]);
//            }
//            to->m_is_call = from->m_is_call;
//            to->m_type = from->m_type;
//            //            to->m_type_name = from->m_type_name;
//
//        } else if (!term->Left()->isCall() && !term->Right()->isCall()) {
//            // Нативная переменная
//            if (!term->Right()->m_type) {
//                term->Right()->m_type = term->Left()->m_type;
//                //                term->Right()->m_type_name = term->Left()->m_type_name;
//            }
//
//        } else if (term->Left()->isCall() && term->Right()->isCall()) {
//            LOG_RUNTIME("Check args in native func not implemented!");
//        }
//        rval = ctx->m_runtime->CreateNative(term->Right(), nullptr, false, term->Right()->m_text.substr(1).c_str());
//
//    } else {
//        rval = Eval(ctx, term->Right(), local_vars, is_eval_block, CatchType::CATCH_AUTO);
//    }
//    if (!rval) {
//        NL_PARSER(term->Right(), "Object is missing or expression is not evaluated!");
//    }
//
//    ASSERT(list_obj.size() == list_term.size());
//
//    if (term->Right()->getTermID() == TermID::ELLIPSIS) {
//        if (rval->is_dictionary_type() || rval->is_tensor_type()) {
//            if (!rval->empty() && rval->is_scalar()) {
//                LOG_RUNTIME("Fail expand scalar!");
//            }
//            for (int i = 0; i < list_obj.size() - 1; i++) {
//                if (list_term[i]->getTermID() != TermID::NONE) {
//                    if (i < rval->size()) {
//                        list_obj[i]->SetValue_((*rval)[i].second); //->Clone()
//                    } else {
//                        list_obj[i]->SetValue_(Obj::CreateNone());
//                    }
//                }
//            }
//            if (static_cast<int64_t> (list_obj.size()) - 1 < rval->size()) {
//                // Удалить первые элементы
//                rval->resize_(-(rval->size() - (static_cast<int64_t> (list_obj.size()) - 1)), nullptr);
//            } else {
//                rval->resize_(0, nullptr);
//            }
//            list_obj[list_obj.size() - 1]->SetValue_(rval->Clone());
//
//            result = list_obj[list_obj.size() - 1];
//
//        } else {
//            LOG_RUNTIME("Fail expand type '%s'!", toString(rval->getType()));
//        }
//    } else {
//        // Присвоеить единственное значение всем элементам с левой стороны оператора присовения
//
//        for (int i = 0; i < list_obj.size(); i++) {
//            if (isTypeName(list_term[i]->m_text)) {
//
//                // Новый тип
//                //                if (ctx->m_runtime->m_types.find(list_term[i]->m_text) != ctx->m_runtime->m_types.end()) {
//                //                    LOG_RUNTIME("Type name '%s' already exists!", list_term[i]->m_text.c_str());
//                //                }
//                //
//                //                result = rval->Clone();
//                //                result->m_class_name = list_term[i]->m_text;
//                //                result->m_class_parents.push_back(rval);
//                //
//                //                ctx->m_runtime->m_types[list_term[i]->m_text] = result;
//
//                if (ctx->m_runtime->NameFind(list_term[i]->m_text.c_str()) || ctx->m_runtime->NameFind(&list_term[i]->m_text[1])) {
//                    LOG_RUNTIME("Type name '%s' already exists!", list_term[i]->m_text.c_str());
//                }
//
//                result = rval->Clone();
//
//
//                TermPtr term = *const_cast<TermPtr *> (&result->m_prototype);
//                term->m_text = list_term[i]->m_text;
//
//                result->m_class_name = list_term[i]->m_text;
//                result->m_class_parents.push_back(rval);
//
//                ctx->m_runtime->m_buildin_obj[list_term[i]->m_text] = result;
//
//                //                if (!ctx->m_runtime->NameRegister(result->m_prototype->m_text.c_str(), result->m_prototype, result)) {
//                LOG_RUNTIME("Fail register prototype '%s'!", result->m_prototype->toString().c_str());
//                //                }
//
//
//            } else if (list_term[i]->getTermID() == TermID::NONE) {
//                // Skip
//            } else {
//                if (list_term[i]->Right()) {
//                    ASSERT(list_term[i]->Right()->getTermID() == TermID::INDEX);
//                    list_obj[i]->index_set_(MakeIndex(ctx, list_term[i]->Right(), local_vars), rval);
//                } else {
//                    list_obj[i]->SetValue_(rval);
//                }
//                if (list_obj[i]->m_var_type_current == ObjType::Function && rval->is_block()) {
//                    list_obj[i]->m_var_type_current = ObjType::EVAL_FUNCTION;
//                }
//                result = list_obj[i];
//            }
//        }
//    }
//    return result;
//}

//bool Context::MatchCompare(Obj &match, ObjPtr &value, MatchMode mode, Context *ctx) {
//    switch (mode) {
//        case MatchMode::MatchEqual:
//            return match.op_equal(value);
//        case MatchMode::MatchStrict:
//            return match.op_accurate(value);
//        case MatchMode::TYPE_NAME:
//            return match.op_class_test(value, ctx);
//        case MatchMode::TYPE_EQUAL:
//            return match.op_duck_test(value, false);
//
//        case MatchMode::TYPE_STRICT:
//            return match.op_duck_test(value, true);
//    }
//    LOG_RUNTIME("Unknown pattern matching type %d!", static_cast<int> (mode));
//}
//
//bool Context::MatchEstimate(Obj &match, const TermPtr &match_item, MatchMode mode, Context *ctx, Obj * args) {
//
//    ObjPtr cond = CreateRVal(ctx, match_item, args);
//
//    if (cond->is_none_type() || MatchCompare(match, cond, mode, ctx)) {
//        return true;
//    } else {
//        for (int i = 0; i < match_item->m_follow.size(); i++) {
//
//            ASSERT(match_item->m_follow[i]);
//            cond = CreateRVal(ctx, match_item->m_follow[i], args);
//
//            if (cond->is_none_type() || MatchCompare(match, cond, mode, ctx)) {
//
//                return true;
//            }
//        }
//    }
//    return false;
//}
//
//ObjPtr Context::eval_MATCHING(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
//    /*
//     * [match] ==> { # ~> ~~> ~~~> ===>
//     *  [cond1] --> {expr};
//     *  [cond2, cond3] --> {expr};
//     *  [...] --> {expr};
//     * };
//     */
//
//    ASSERT(term->Left());
//    ASSERT(term->Right());
//
//    MatchMode mode;
//    if (term->m_text.compare("==>") == 0) {
//        mode = MatchMode::MatchEqual;
//    } else if (term->m_text.compare("===>") == 0) {
//        mode = MatchMode::MatchStrict;
//    } else if (term->m_text.compare("~>") == 0) {
//        mode = MatchMode::TYPE_NAME;
//    } else if (term->m_text.compare("~~>") == 0) {
//        mode = MatchMode::TYPE_EQUAL;
//    } else if (term->m_text.compare("~~~>") == 0) {
//        mode = MatchMode::TYPE_STRICT;
//    } else {
//        NL_PARSER(term, "Unknown pattern matching type!");
//    }
//
//    ObjPtr value = CreateRVal(ctx, term->Left(), args);
//    TermPtr list = term->Right();
//
//
//    ASSERT(list->Left());
//    ASSERT(list->Right());
//
//    ObjPtr cond = CreateRVal(ctx, list->Left(), args);
//
//    if (MatchEstimate(*cond.get(), list->Left(), mode, ctx, args)) {
//        return CreateRVal(ctx, list->Right(), args);
//    } else {
//        for (int i = 0; i < list->m_block.size(); i++) {
//
//            ASSERT(list->m_block[i]->Left());
//            ASSERT(list->m_block[i]->Right());
//
//            if (MatchEstimate(*cond.get(), list->m_block[i]->Left(), mode, ctx, args)) {
//
//                return CreateRVal(ctx, list->m_block[i]->Right(), args);
//            }
//        }
//    }
//    return Obj::CreateNone();
//}

std::string RunTime::GetLastErrorMessage() {
#ifndef _MSC_VER
    return std::string(strerror(errno));
#else
    wchar_t buffer[256];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, sizeof (buffer), NULL);

    return utf8_encode(buffer);
#endif        
}

void * RunTime::GetNativeAddr(const char *name, void * handle) {
    return GetDirectAddressFromLibrary(handle, name);
}

//void Context::ItemTensorEval_(torch::Tensor &tensor, c10::IntArrayRef shape, std::vector<Index> &ind, const int64_t pos,
//        ObjPtr &obj, ObjPtr & args) {
//    ASSERT(pos < static_cast<int64_t> (ind.size()));
//    if (pos + 1 < static_cast<int64_t> (ind.size())) {
//        for (ind[pos] = 0; ind[pos].integer() < shape[pos]; ind[pos] = ind[pos].integer() + 1) {
//            ItemTensorEval_(tensor, shape, ind, pos + 1, obj, args);
//        }
//    } else {
//
//        at::Scalar value;
//        ObjType type = fromTorchType(tensor.scalar_type());
//
//        for (ind[pos] = 0; ind[pos].integer() < shape[pos]; ind[pos] = ind[pos].integer() + 1) {
//
//            switch (type) {
//                case ObjType::Int8:
//                case ObjType::Char:
//                case ObjType::Byte:
//                case ObjType::Int16:
//                case ObjType::Word:
//                case ObjType::Int32:
//                case ObjType::DWord:
//                case ObjType::Int64:
//                case ObjType::DWord64:
//                    value = at::Scalar(obj->Call(this)->GetValueAsInteger()); // args
//                    tensor.index_put_(ind, value);
//                    break;
//                case ObjType::Float32:
//                case ObjType::Float64:
//                case ObjType::Single:
//                case ObjType::Double:
//                    value = at::Scalar(obj->Call(this)->GetValueAsNumber()); // args
//                    tensor.index_put_(ind, value);
//
//                    break;
//                default:
//                    ASSERT(!"Not implemented!");
//            }
//        }
//    }
//}
//
//void Context::ItemTensorEval(torch::Tensor &self, ObjPtr obj, ObjPtr args) {
//    if (self.dim() == 0) {
//
//        signed char *ptr_char = nullptr;
//        int16_t *ptr_short = nullptr;
//        int32_t *ptr_int = nullptr;
//        int64_t *ptr_long = nullptr;
//        float *ptr_float = nullptr;
//        double *ptr_double = nullptr;
//
//        switch (fromTorchType(self.scalar_type())) {
//            case ObjType::Int8:
//                ptr_char = self.data_ptr<signed char>();
//                ASSERT(ptr_char);
//                *ptr_char = static_cast<signed char> (obj->Call(this)->GetValueAsInteger());
//                return;
//            case ObjType::Int16:
//                ptr_short = self.data_ptr<int16_t>();
//                ASSERT(ptr_short);
//                *ptr_short = static_cast<int16_t> (obj->Call(this)->GetValueAsInteger());
//                return;
//            case ObjType::Int32:
//                ptr_int = self.data_ptr<int32_t>();
//                ASSERT(ptr_int);
//                *ptr_int = static_cast<int32_t> (obj->Call(this)->GetValueAsInteger());
//                return;
//            case ObjType::Int64:
//                ptr_long = self.data_ptr<int64_t>();
//                ASSERT(ptr_long);
//                *ptr_long = static_cast<int64_t> (obj->Call(this)->GetValueAsInteger());
//                return;
//            case ObjType::Float32:
//                ptr_float = self.data_ptr<float>();
//                ASSERT(ptr_float);
//                *ptr_float = static_cast<float> (obj->Call(this)->GetValueAsNumber());
//                return;
//            case ObjType::Float64:
//                ptr_double = self.data_ptr<double>();
//                ASSERT(ptr_double);
//                *ptr_double = static_cast<double> (obj->Call(this)->GetValueAsNumber());
//                return;
//        }
//
//        ASSERT(!"Not implemented!");
//    } else {
//
//        c10::IntArrayRef shape = self.sizes(); // Кол-во эментов в каждом измерении
//        std::vector<Index> ind(shape.size(),
//                0); // Счетчик обхода всех эелемнтов тензора
//        ItemTensorEval_(self, shape, ind, 0, obj, args);
//    }
//}
//
//std::vector<int64_t> GetTensorShape(Context *ctx, TermPtr type, Obj * local_vars, bool eval_block) {
//    std::vector<int64_t> result(type->size());
//    for (int i = 0; i < type->size(); i++) {
//        ObjPtr temp = ctx->CreateRVal(ctx, type->at(i).second, local_vars);
//        if (temp->is_integer() || temp->is_bool_type()) {
//            result[i] = temp->GetValueAsInteger();
//        } else {
//            NL_PARSER(type->at(i).second, "Measurement dimension can be an integer only!");
//        }
//        if (result[i] <= 0) {
//
//            NL_PARSER(type->at(i).second, "Dimension size can be greater than zero!");
//        }
//    }
//    return result;
//}
//
//std::vector<Index> Context::MakeIndex(Context *ctx, TermPtr term, Obj * local_vars) {
//
//    // `at::indexing::TensorIndex` is used for converting C++ tensor indices such
//    // as
//    // `{None, "...", Ellipsis, 0, true, Slice(1, None, 2), torch::tensor({1,
//    // 2})}` into its equivalent `std::vector<TensorIndex>`, so that further
//    // tensor indexing operations can be performed using the supplied indices.
//    //
//    // There is one-to-one correspondence between Python and C++ tensor index
//    // types: Python                  | C++
//    // -----------------------------------------------------
//    // `None`                  | `at::indexing::None`
//    // `Ellipsis`              | `at::indexing::Ellipsis`
//    // `...`                   | `"..."`
//    // `123`                   | `123`
//    // `True` / `False`        | `true` / `false`
//    // `:`                     | `Slice()` / `Slice(None, None)`
//    // `::`                    | `Slice()` / `Slice(None, None, None)`
//    // `1:`                    | `Slice(1, None)`
//    // `1::`                   | `Slice(1, None, None)`
//    // `:3`                    | `Slice(None, 3)`
//    // `:3:`                   | `Slice(None, 3, None)`
//    // `::2`                   | `Slice(None, None, 2)`
//    // `1:3`                   | `Slice(1, 3)`
//    // `1::2`                  | `Slice(1, None, 2)`
//    // `:3:2`                  | `Slice(None, 3, 2)`
//    // `1:3:2`                 | `Slice(1, 3, 2)`
//    // `torch.tensor([1, 2])`) | `torch::tensor({1, 2})`
//
//    std::vector<Index> result;
//
//    if (!term->size()) {
//        NL_PARSER(term, "Index not found!");
//    }
//    for (int i = 0; i < term->size(); i++) {
//        if (!term->name(i).empty() || (term->at(i).second && term->at(i).second->isString())) {
//            NL_PARSER(term, "Named index not support '%d'!", i);
//        }
//        if (!term->at(i).second) {
//            NL_PARSER(term, "Empty index '%d'!", i);
//        }
//
//        if (term->at(i).second->getTermID() == TermID::ELLIPSIS) {
//            result.push_back(Index("..."));
//        } else {
//
//            ObjPtr temp = ctx->CreateRVal(ctx, term->at(i).second, local_vars);
//
//            if (temp->is_none_type()) {
//
//                result.push_back(Index(at::indexing::None));
//            } else if (temp->is_integer() || temp->is_bool_type()) {
//
//                if (temp->is_scalar()) {
//                    result.push_back(Index(temp->GetValueAsInteger()));
//                } else if (temp->m_tensor.dim() == 1) {
//                    result.push_back(Index(temp->m_tensor));
//                } else {
//                    NL_PARSER(term->at(i).second, "Extra dimensions index not support '%d'!", i);
//                }
//            } else if (temp->is_range()) {
//
//                int64_t start = temp->at("start").second->GetValueAsInteger();
//                int64_t stop = temp->at("stop").second->GetValueAsInteger();
//                int64_t step = temp->at("step").second->GetValueAsInteger();
//
//                result.push_back(Index(at::indexing::Slice(start, stop, step)));
//            } else {
//
//                NL_PARSER(term->at(i).second, "Fail tensor index '%d'!", i);
//            }
//        }
//    }
//    return result;
//}
//
//ObjPtr Context::CreateRVal(Context *ctx, TermPtr term, Obj * local_vars, bool eval_block, CatchType int_catch) {
//
//    if (!term) {
//        ASSERT(term);
//    }
//    ASSERT(local_vars);
//
//    ObjPtr result = nullptr;
//    ObjPtr temp = nullptr;
//    ObjPtr args = nullptr;
//    ObjPtr value = nullptr;
//    TermPtr field = nullptr;
//    std::string full_name;
//
//    result = Obj::CreateNone();
//    result->m_is_reference = !!term->m_ref;
//
//    int64_t val_int;
//    double val_dbl;
//    ObjType type;
//    bool has_error;
//    std::vector<int64_t> sizes;
//    at::Scalar torch_scalar;
//    switch (term->getTermID()) {
//        case TermID::OP_MATH:
//        case TermID::OP_BITWISE:
//        case TermID::OP_COMPARE:
//        case TermID::OP_LOGICAL:
//            return Context::Eval(ctx, term, local_vars, eval_block, int_catch);
//
//        case TermID::INTEGER:
//
//            val_int = parseInteger(term->getText().c_str());
//            NL_TYPECHECK(term, typeFromLimit(val_int), typeFromString(term->m_type, ctx ? ctx->m_runtime.get() : nullptr)); // Соответстствует ли тип значению?
//
//            result = Obj::CreateValue(val_int);
//            result->m_var_type_current = typeFromLimit(val_int);
//            if (term->GetType()) {
//                result->m_var_type_fixed = typeFromString(term->m_type, ctx ? ctx->m_runtime.get() : nullptr);
//                result->m_var_type_current = result->m_var_type_fixed;
//            }
//            return result;
//
//        case TermID::NUMBER:
//            val_dbl = parseDouble(term->getText().c_str());
//            NL_TYPECHECK(term, typeFromLimit(val_dbl), typeFromString(term->m_type, ctx ? ctx->m_runtime.get() : nullptr)); // Соответстствует ли тип значению?
//
//            result = Obj::CreateValue(val_dbl);
//            result->m_var_type_current = typeFromLimit(val_dbl);
//            if (term->GetType()) {
//                result->m_var_type_fixed = typeFromString(term->m_type, ctx ? ctx->m_runtime.get() : nullptr);
//                result->m_var_type_current = result->m_var_type_fixed;
//            }
//            return result;
//
//        case TermID::STRWIDE:
//            return Obj::CreateString(utf8_decode(term->getText()));
//
//
//        case TermID::STRCHAR:
//            return Obj::CreateString(term->getText());
//
//        case TermID::EVAL:
//            /*
//             * subprocess.run(args, *, stdin=None, input=None, stdout=None, stderr=None, 
//             * capture_output=False, shell=False, cwd=None, timeout=None, check=False, 
//             * encoding=None, errors=None, text=None, env=None, universal_newlines=None, **other_popen_kwargs)            
//             */
//            /* class subprocess.Popen(args, bufsize = -1, executable = None, 
//             * stdin = None, stdout = None, stderr = None, 
//             * preexec_fn = None, close_fds = True, shell = False, cwd = None, 
//             * env = None, universal_newlines = None, startupinfo = None, 
//             * creationflags = 0, restore_signals = True, start_new_session = False, 
//             * pass_fds = (), *, group = None, extra_groups = None, user = None, umask = -1
//             * , encoding = None, errors = None, text = None, 
//             * pipesize = -1, process_group = None)¶
//             */
//
//
//
//            //            result = Obj::CreateType(ObjType::Eval, ObjType::Eval, true);
//            //            result->m_value = term->getText();
//            //            if (term->isCall()) {
//            LOG_RUNTIME("Not implemented!!!");
//            //            }
//            //            return Exec(ctx, term->getText().c_str(), nullptr);
//
//            /*        case TermID::FIELD:
//                        if(module && module->HasFunc(term->GetFullName().c_str())) {
//                            // Если поле является функцией и она загружена
//                            result = Obj::CreateType(Obj::Type::FUNCTION,
//               term->GetFullName().c_str()); result->m_module = module;
//                            result->m_is_const = term->m_is_const;
//                            result->m_is_ref = term->m_is_ref;
//                            return result;
//                        }
//                        if(!result) {
//                            LOG_RUNTIME("Term '%s' not found!",
//               term->toString().c_str());
//                        }
//                        return result;
//             */
//
//            //@todo Что делать с пустыми значениями? Это None ???
//        case TermID::EMPTY:
//            result->m_var_type_current = ObjType::None;
//            result->m_var_is_init = false;
//            return result;
//
//
//
//        case TermID::NEWLANG:
//        case TermID::MODULE:
//            /* Prototype \\(file=_, filter='*', load=@true, source=_)
//             * Required argument $file or $source can be only one
//             * 
//             * \\('name') - dynamic load module
//             * \\dir\name() - load module and 
//             * \\dir\name::func() - call function from module
//             * module := \\('name', @false) - Check module is loaded and don't load it
//             * 
//             */
//
//            if (term->isCall() && ExtractName(term->getText()).empty()) {
//                // Load module
//
//                return ctx->m_runtime->OpLoadModule(term);
//            }
//
//
//
//        case TermID::LOCAL:
//        case TermID::STATIC:
//        case TermID::NAME:
//        case TermID::ARGS:
//        case TermID::PARENT:
//        case TermID::INT_PLUS:
//        case TermID::INT_MINUS:
//
//
//            if (!isModuleName(term->GetFullName()) && (term->isCall() || term->isReturn())) {
//                temp = ctx->GetTerm(term->GetFullName().c_str(), term->isRef());
//
//                if (!temp) {
//                    NL_PARSER(term, "Term '%s' not found!", term->GetFullName().c_str());
//                }
//
//                args = Obj::CreateDict();
//                ctx->CreateArgs_(args, term, local_vars);
//
//                if (term->isReturn()) {
//                    return result;
//                }
//
//                result = temp->Call(ctx, args.get());
//                return result;
//            }
//
//
//            if (term->GetType()) {
//
//                result->m_var_type_current = typeFromString(term->GetType()->m_text, ctx ? ctx->m_runtime.get() : nullptr);
//                result->m_var_type_fixed = result->m_var_type_current;
//                result->m_var_is_init = false; // Нельзя считать значение
//
//                // Check BuildInType
//                has_error = false;
//                typeFromString(term->GetType()->m_text, nullptr, &has_error);
//                if (has_error) {
//                    result->m_class_name = term->GetType()->m_text;
//                }
//
//                return result;
//            }
//
//            //- **\this** - Текущий объект (**$0**)
//            //- **\sys** - Системный контекст запущенной программы (**\\**)
//            //- **\current** - Текущий модуль (** \$ **)
//            //- **\cmd** - Все аргументы выполняющегося приложения из командной строки (**@\***)
//
//            if (term->m_text.compare("_") == 0) {
//
//                result->m_var_type_current = ObjType::None;
//                return result;
//
//            } else if (term->m_text.compare("$$") == 0) {
//
//                //- **\parent** - Родительский объект (**$$**)
//
//                result->m_var_type_current = ObjType::Dictionary;
//                result->m_var_name = "$$";
//
//                ASSERT(ctx);
//
//                auto iter = ctx->begin();
//                while (iter != ctx->end()) {
//                    if (!iter->second.expired()) {
//                        result->push_back(Obj::CreateString(iter->first));
//                        iter++;
//                    } else {
//                        iter = ctx->ListType::erase(iter);
//                    }
//                }
//
//                result->m_var_is_init = true;
//                return result;
//
//            } else if (term->m_text.compare("$*") == 0) {
//
//                //- **\args** - Все аргументы функции (**$\***)
//
//                result->m_var_type_current = ObjType::Dictionary;
//                result->m_var_name = "$*";
//                result->m_var_is_init = true;
//
//                for (int i = 0; i < term->size(); i++) {
//                    if (term->name(i).empty()) {
//                        result->push_back(Context::CreateRVal(ctx, (*term)[i].second, local_vars));
//                    } else {
//                        result->push_back(Context::CreateRVal(ctx, (*term)[i].second, local_vars), term->name(i));
//                    }
//                }
//
//                return result;
//
//            }
//
//
//            if (isLocalName(term->m_text.c_str())) {
//                full_name = MakeName(term->m_text);
//                return local_vars->at(full_name).second;
//            } else {
//                result = ctx->GetTerm(term->GetFullName().c_str(), term->isRef());
//
//                // Типы данных обрабатываются тут, а не в вызовах функций (TermID::CALL)
//                if (!result) {
//                    NL_PARSER(term, "Object '%s' not exist!", term->toString().c_str());
//                }
//                if (term->size()) {
//                    Obj args(ctx, term, true, local_vars);
//                    result = result->Call(ctx, &args);
//                }
//
//            }
//            if (!result) {
//                // Делать ислкючение или возвращать объект "ошибка" ?????
//                ASSERT(term);
//                LOG_RUNTIME("Term '%s' not found!", term->GetFullName().c_str());
//            }
//
//            field = term->m_right;
//            if (field && field->getTermID() == TermID::FIELD) {
//                while (field) {
//                    if (field->m_is_call) {
//
//                        if (result->m_class_name.empty()) {
//                            NL_PARSER(term, "Object '%s' not a class! %s", term->GetFullName().c_str(), result->m_class_name.c_str());
//                        }
//
//                        full_name = result->m_class_name;
//                        if (isTypeName(full_name)) {
//                            full_name = full_name.substr(1);
//                        }
//                        full_name += "::";
//                        full_name += field->getText();
//
//                        temp = ctx->GetTerm(full_name, true);
//                        if (!temp) {
//                            LOG_RUNTIME("Function '%s' not found!", full_name.c_str());
//                        }
//
//                        args = Obj::CreateDict();
//                        for (size_t i = 0; i < field->size(); i++) {
//                            if (field->name(i).empty()) {
//                                args->push_back(CreateRVal(ctx, (*field)[i].second, local_vars));
//                            } else {
//                                args->push_back(CreateRVal(ctx, (*field)[i].second, local_vars), field->name(i).c_str());
//                            }
//                        }
//
//                        result = temp->Call(ctx, args.get(), true, result);
//
//                    } else {
//                        result = result->at(field->getText()).second;
//                    }
//                    field = field->m_right;
//                }
//            } else if (field && field->getTermID() == TermID::INDEX) {
//                while (field) {
//                    result = result->index_get(MakeIndex(ctx, field, local_vars));
//                    field = field->m_right;
//                    ASSERT(!field); // Нужно выполнять, а не просто получать значение поля
//                }
//            } else if (field) {
//                LOG_RUNTIME("Not implemented! %s", field->toString().c_str());
//            }
//
//            return result;
//
//
//
//        case TermID::TYPE:
//
//            result = ctx->GetTerm(term->GetFullName().c_str(), term->isRef());
//
//
//            has_error = false;
//            type = typeFromString(term->GetFullName(), ctx ? ctx->m_runtime.get() : nullptr, &has_error);
//            if (has_error) {
//                LOG_RUNTIME("Type name '%s' undefined!", term->GetFullName().c_str());
//            }
//            ASSERT(result);
//            ASSERT(result->m_var_type_fixed == type);
//
//            if (result->m_var_type_fixed == ObjType::Class) {
//                //@todo Virtual
//
//            }
//
//            // Размерность, если указана
//            result->m_dimensions = Obj::CreateType(ObjType::Dictionary, ObjType::Dictionary, true);
//            for (size_t i = 0; i < term->m_dims.size(); i++) {
//                result->m_dimensions->push_back(CreateRVal(ctx, term->m_dims[i], local_vars, eval_block, int_catch));
//            }
//
//            args = Obj::CreateDict();
//            for (int64_t i = 0; i < static_cast<int64_t> (term->size()); i++) {
//
//
//                if ((*term)[i].second->getTermID() == TermID::FILLING) {
//
//                    // Заполнение значений вызовом функции
//                    // :Type(1, 2, 3, ... rand() ... );
//
//
//                    ASSERT(!(*term)[i].second->Left());
//                    ASSERT((*term)[i].second->Right());
//
//
//                    ObjPtr expr = ctx->FindTerm((*term)[i].second->Right()->GetFullName());
//                    ASSERT(expr);
//
//                    if (!(*term)[i].second->Right()->isCall()) {
//                        LOG_RUNTIME("Operator filling supported function call only!");
//                    }
//
//                    if (i + 1 != term->size()) {
//                        LOG_RUNTIME("Function filling is supported for the last argument only!");
//                    }
//
//                    if (!result->m_dimensions || !result->m_dimensions->size()) {
//                        LOG_RUNTIME("Object has no dimensions!");
//                    }
//
//                    int64_t full_size = 1;
//                    for (int dim_index = 0; dim_index < result->m_dimensions->size(); dim_index++) {
//
//                        if (!(*result->m_dimensions)[dim_index].second->is_integer()) {
//                            LOG_RUNTIME("Dimension index for function filling support integer value only!");
//                        }
//
//                        full_size *= (*result->m_dimensions)[dim_index].second->GetValueAsInteger();
//                    }
//
//                    if (full_size <= 0) {
//                        LOG_RUNTIME("Items count error for all dimensions!");
//                    }
//
//
//                    if (expr->size()) {
//                        LOG_RUNTIME("Argument in function for filling not implemented!");
//                    }
//
//                    for (int64_t dim_index = args->size(); dim_index < full_size; dim_index++) {
//                        args->push_back(expr->Call(ctx));
//                    }
//
//                    break;
//
//                } else if ((*term)[i].second->getTermID() == TermID::ELLIPSIS) {
//
//                    if (!term->name(i).empty()) {
//                        LOG_RUNTIME("Named ellipsys not implemented!");
//                    }
//
//                    if ((*term)[i].second->Right()) {
//
//                        bool named = ((*term)[i].second->Left() && (*term)[i].second->Left()->getTermID() == TermID::ELLIPSIS);
//                        ObjPtr exp = CreateRVal(ctx, (*term)[i].second->Right(), eval_block);
//
//                        if (!exp->is_dictionary_type()) {
//                            LOG_RUNTIME("Expansion operator applies to dictionary only!");
//                        }
//
//
//                        for (int index = 0; index < exp->size(); index++) {
//                            if (named) {
//                                args->push_back((*exp)[index].second, exp->name(index).empty() ? "" : exp->name(index));
//                            } else {
//                                args->push_back((*exp)[index].second);
//                            }
//                        }
//
//                        continue;
//                    }
//                }
//
//                if (term->name(i).empty()) {
//                    args->push_back(CreateRVal(ctx, (*term)[i].second, local_vars));
//                } else {
//                    args->push_back(CreateRVal(ctx, (*term)[i].second, local_vars), term->name(i).c_str());
//                }
//
//            }
//
//            result = result->Call(ctx, args.get());
//            ASSERT(result);
//
//            return result;
//
//
//        case TermID::TENSOR:
//        case TermID::DICT:
//            result->m_var_type_current = ObjType::Dictionary;
//            ctx->CreateArgs_(result, term, local_vars);
//
//            result->m_var_is_init = true;
//            if (term->getTermID() == TermID::TENSOR) {
//
//                if (!term->m_type) {
//                    result->m_var_type_fixed = ObjType::None;
//                } else {
//                    result->m_var_type_fixed = typeFromString(term->m_type->m_text, ctx ? ctx->m_runtime.get() : nullptr);
//                }
//                type = getSummaryTensorType(result.get(), result->m_var_type_fixed);
//
//                if (type != ObjType::None) {
//
//
//                    sizes = TensorShapeFromDict(result.get());
//                    result->toType_(type);
//
//                    if (!sizes.empty()) {
//                        ASSERT(result->m_tensor.defined());
//                        result->m_tensor = result->m_tensor.reshape(sizes);
//                    }
//
//
//                } else {
//                    result->m_var_is_init = false;
//                }
//                //                result->resize(0, nullptr, "");
//                result->m_var_type_current = type;
//            } else {
//                result->m_class_name = term->m_class;
//            }
//            return result;
//
//        case TermID::ARGUMENT:
//
//            val_int = IndexArg(term);
//            if (val_int < local_vars->size()) {
//                return local_vars->at(val_int).second;
//            }
//            LOG_RUNTIME("Argument '%s' not exist!", term->toString().c_str());
//
//        case TermID::BLOCK:
//            return CallBlock(ctx, term, local_vars, eval_block, CatchType::CATCH_AUTO, nullptr);
//
//        case TermID::BLOCK_TRY:
//        case TermID::BLOCK_PLUS:
//        case TermID::BLOCK_MINUS:
//            return CallBlock(ctx, term, local_vars, eval_block, int_catch, nullptr);
//
//            //        case TermID::BLOCK_PLUS:
//            //            return CallBlock(ctx, term, local_vars, CatchType::CATCH_PLUS);
//            //            
//            //        case TermID::BLOCK_MINUS:
//            //            return CallBlock(ctx, term, local_vars, CatchType::CATCH_MINUS);
//
//        case TermID::ELLIPSIS:
//            result->m_var_type_current = ObjType::Ellipsis;
//            result->m_var_type_fixed = ObjType::None;
//            result->m_var_is_init = true;
//            return result;
//
//        case TermID::RANGE:
//
//            result->m_var_type_current = ObjType::Dictionary;
//            for (int i = 0; i < term->size(); i++) {
//                ASSERT(!term->name(i).empty());
//                result->push_back(Eval(ctx, (*term)[i].second, local_vars, eval_block), term->name(i).c_str());
//            }
//
//            if (result->size() == 2) {
//                result->push_back(Obj::CreateValue(1, ObjType::None), "step");
//            }
//
//            result->m_var_type_current = ObjType::Range;
//            result->m_var_type_fixed = ObjType::Range;
//            result->m_var_is_init = true;
//
//            return result;
//
//        case TermID::RATIONAL:
//            return Obj::CreateRational(term->m_text);
//
//        case TermID::ITERATOR:
//
//            ASSERT(term->Left());
//
//            temp = Eval(ctx, term->Left(), local_vars, eval_block, CatchType::CATCH_AUTO);
//            if (!temp) {
//                LOG_RUNTIME("Term '%s' not found!", term->Left()->GetFullName().c_str());
//            }
//
//            args = Obj::CreateDict();
//            ctx->CreateArgs_(args, term, local_vars);
//
//
//            /*
//             * Создание итератора
//             * ?, ?(), ?("Фильтр"), ?(func), ?(func, args...)
//             * 
//             * Перебор элементов итератора
//             * !, !(), !(0), !(3), !(-3)
//             * 
//             * dict! и dict!(0) эквивалентны
//             * dict! -> 1,  dict! -> 2, dict! -> 3, dict! -> 4, dict! -> 5, dict! -> :IteratorEnd
//             * 
//             * Различия отрицательного размера возвращаемого словаря для итератора
//             * dict!(-1) -> (1,),  ...  dict!(-1) -> (5,),  dict!(-1) -> (:IteratorEnd,),  
//             * dict!(1) -> (1,),  ...  dict!(1) -> (5,),  dict!(1) -> (,),  
//             * dict!(-3) -> (1, 2, 3,),  dict!(-3) -> (4, 5, :IteratorEnd,)
//             * dict!(3) -> (1, 2, 3,), dict!(3) -> (4, 5,)
//             * 
//             * Операторы ?! и !? эквивалентны и возвращают текущие данные без перемещения указателя итератора.
//             * 
//             * Оператор ?? создает итератор и сразу его выполняет, возвращая все значения 
//             * в виде элементов словаря, т.е. аналог последовательности ?(LINQ); !(:Int64.__max__);
//             * 
//             * Оператор !! - сбрасывает итератор в начальное состояние и возвращает первый элемент
//             */
//
//            if (term->m_text.compare("?") == 0) {
//                return temp->IteratorMake(args.get());
//            } else if (term->m_text.compare("!") == 0) {
//                ASSERT(!args->size() && "Argument processing not implemented");
//                return temp->IteratorNext(0);
//            } else if (term->m_text.compare("!!") == 0) {
//                ASSERT(!args->size() && "Argument processing not implemented");
//                temp->IteratorReset();
//                return temp->IteratorData();
//            } else if (term->m_text.compare("!?") == 0 || term->m_text.compare("?!") == 0) {
//                return temp->IteratorData();
//            } else if (term->m_text.compare("??") == 0) {
//
//                val_int = std::numeric_limits<int64_t>::max();
//                if (args->empty() || (args->size() == 1 && args->at(0).second->is_integer())) {
//                    result = temp->IteratorMake(nullptr, false);
//                    if (args->size()) {
//                        val_int = args->at(0).second->GetValueAsInteger();
//                    }
//                } else if (args->size() == 1 && args->at(0).second->is_string_type()) {
//                    result = temp->IteratorMake(args->at(0).second->GetValueAsString().c_str(), false);
//                } else if (args->size() == 2 && args->at(0).second->is_string_type() && args->at(1).second->is_integer()) {
//                    result = temp->IteratorMake(args->at(0).second->GetValueAsString().c_str(), false);
//                    val_int = args->at(1).second->GetValueAsInteger();
//                } else {
//                    LOG_RUNTIME("Iterator`s args '%s' not allowed!", args->toString().c_str());
//                }
//                return result->IteratorNext(val_int);
//
//            } else {
//                LOG_RUNTIME("Iterator '%s' not recognized in '%s'!", term->m_text.c_str(), term->toString().c_str());
//            }
//    }
//    LOG_RUNTIME("Fail create type %s from '%s'", newlang::toString(term->getTermID()), term->toString().c_str());
//
//    return nullptr;
//}
//
//void Context::CreateArgs_(ObjPtr &args, TermPtr &term, Obj * local_vars) {
//    for (int i = 0; i < term->size(); i++) {
//        if (term->name(i).empty()) {
//            args->push_back(Eval(this, (*term)[i].second, local_vars, true));
//        } else {
//
//            args->push_back(Eval(this, (*term)[i].second, local_vars, true), term->name(i).c_str());
//        }
//    }
//}
//
//ObjPtr Context::CreateClass(std::string class_name, TermPtr body, Obj * local_vars) {
//
//    ASSERT(false);
//    return nullptr;
//
//    //    ASSERT(body->getTermID() == TermID::CLASS);
//    //    ASSERT(body->m_base.size());
//    //
//    //    ObjPtr new_class = Obj::CreateBaseType(ObjType::Class);
//    //    new_class->m_var_name = class_name;
//    //    std::string constructor = MakeConstructorName(class_name);
//    //
//    //    // LOG_DEBUG("\nCreate class: '%s', constructor %s", class_name.c_str(), constructor.c_str());
//    //
//    //
//    //    if (class_name.find(":") == 0) {
//    //        class_name.erase(0, 1);
//    //    }
//    //
//    //    // Пройтись по всем базовым классам
//    //    for (int i = 0; i < body->m_base.size(); i++) {
//    //
//    //        ObjPtr base = GetTerm(body->m_base[i]->GetFullName().c_str(), false);
//    //
//    //        // LOG_DEBUG("Base %s: '%s' %d", body->m_base[i]->GetFullName().c_str(), base->toString().c_str(), (int) base->size());
//    //
//    //        bool has_error = false;
//    //        ObjType type = typeFromString(body->m_base[i]->GetFullName(), this, &has_error);
//    //        if (has_error) {
//    //            LOG_RUNTIME("Type name '%s' undefined!", body->m_base[i]->GetFullName().c_str());
//    //        }
//    //        ASSERT(base);
//    //        ASSERT(base->m_var_type_fixed == type);
//    //        ASSERT(base->m_class_name.compare(body->m_base[i]->GetFullName()) == 0);
//    //        ASSERT(!base->m_dimensions);
//    //
//    //        // Клонировать все методы (функции) базового класса с новыми именами
//    //        std::set<std::string> methods;
//    //        std::string base_constructor = MakeConstructorName(base->m_class_name);
//    //        // LOG_DEBUG("base_constructor %s", base_constructor.c_str());
//    //
//    //
//    //        std::string find_substr = base->m_class_name.substr(1);
//    //        find_substr += "::";
//    //
//    //        auto iter = m_terms->begin();
//    //        while (iter != m_terms->end()) {
//    //            if (iter->first.find(find_substr) == 0) {
//    //                if (base_constructor.compare(iter->first) != 0) {
//    //                    methods.insert(iter->first);
//    //                }
//    //            }
//    //            iter++;
//    //        }
//    //
//    //
//    //        std::string replace(base->m_class_name.substr(1));
//    //        replace += "::";
//    //
//    //        std::set<Variable<Obj>::PairType *> rollback;
//    //
//    //
//    //        try {
//    //
//    //            ObjPtr obj;
//    //
//    //            if (i == 0) {
//    //                // Только один конструктор у класса
//    //                obj = Obj::CreateFunc(constructor, &Obj::ConstructorStub_, ObjType::PureFunc);
//    //                push_back(obj, constructor); // weak_ptr
//    //                rollback.insert(&m_terms->push_back(obj, constructor));
//    //            }
//    //
//    //            for (auto &elem : methods) {
//    //
//    //                // LOG_DEBUG("Func: %s  Replace: %s", elem.c_str(), replace.c_str());
//    //
//    //                auto iter = m_terms->find(elem);
//    //                ASSERT(iter != m_terms->end());
//    //
//    //                obj = iter->second; //.lock();
//    //                ASSERT(obj);
//    //
//    //                std::string name(elem);
//    //                size_t pos = name.find(replace);
//    //
//    //                ASSERT(pos != name.npos);
//    //                ASSERT(pos == 0 || name[pos - 1] == ':');
//    //
//    //                name.erase(pos, replace.size() - 2);
//    //                name.insert(pos, class_name);
//    //
//    //                if (m_terms->find(name) == m_terms->end()) {
//    //                    // LOG_DEBUG("new name %s", name.c_str());
//    //                    push_back(obj, name); // weak_ptr
//    //                    rollback.insert(&m_terms->push_back(obj, name));
//    //                }
//    //            }
//    //
//    //            base->ClonePropTo(*new_class);
//    //
//    //
//    //            // Выполнить тело конструктора типа для создания новых полей и методов у создаваемого типа класса
//    //            bool is_pop_ns = m_runtime->m_macro->NamespacePush(class_name);
//    //            try {
//    //                for (int i = 0; i < body->m_block.size(); i++) {
//    //                    if (body->m_block[i]->IsCreate()) {
//    //                        ASSERT(body->m_block[i]->Left());
//    //                        if (body->m_block[i]->Left()->IsFunction() || body->m_block[i]->Left()->isCall()) {
//    //                            ObjPtr func = Eval(this, body->m_block[i], local_vars, true);
//    //
//    //                            if (body->m_block[i]->Right()->getTermID() == TermID::NONE) {
//    //                                func->m_var_type_current = ObjType::Virtual;
//    //                            }
//    //
//    //                        } else {
//    //
//    //                            std::string name = body->m_block[i]->Left()->getText();
//    //                            bool is_exists = (new_class->find(name) != new_class->end());
//    //
//    //                            if (body->m_block[i]->getText().compare("::=") == 0) {
//    //                                if (is_exists) {
//    //                                    LOG_RUNTIME("Dublicate property name '%s' in class '%s'!", name.c_str(), class_name.c_str());
//    //                                }
//    //                                new_class->push_back(nullptr, name);
//    //                            } else if (body->m_block[i]->getText().compare("=") == 0) {
//    //                                if (!is_exists) {
//    //                                    LOG_RUNTIME("Property name '%s' not found on base classes '%s'!", name.c_str(), class_name.c_str());
//    //                                }
//    //                            } else if (!is_exists) {
//    //                                new_class->push_back(nullptr, name);
//    //                            }
//    //
//    //                            new_class->at(name).second = CreateRVal(this, body->m_block[i]->Right(), local_vars, true);
//    //                        }
//    //                    } else {
//    //                        LOG_RUNTIME("Only create or assignment operators allowed! %s", body->m_block[i]->toString().c_str());
//    //                    }
//    //                }
//    //
//    //                if (is_pop_ns) {
//    //                    m_runtime->m_macro->NamespacePop();
//    //                }
//    //            } catch (...) {
//    //                if (is_pop_ns) {
//    //                    m_runtime->m_macro->NamespacePop();
//    //                }
//    //                throw;
//    //            }
//    //
//    //
//    //        } catch (...) {
//    //
//    //
//    //            for (auto &elem : rollback) {
//    //                // LOG_DEBUG("Rollback: '%s'", elem->first.c_str());
//    //                // remove(find(elem.first)); // weak_ptr
//    //                m_terms->remove(*elem);
//    //            }
//    //
//    //            throw;
//    //        }
//    //
//    //    }
//    //    new_class->m_var_is_init = true;
//    //
//    //    return new_class;
//}
//

/*
 * 
 * 
 */
ObjPtr Context::Run(TermPtr term, Context *runner) {
    if (term->isBlock()) {
        if (!runner) {
            LOG_RUNTIME("Can't calculate '%s' in static mode!", term->toString().c_str());
        }
        runner->m_latter = runner->EvalBlock_(term, term);
    } else {
        ObjPtr result = EvalTerm(term, runner);
        if (runner) {
            runner->m_latter = result;
        } else {
            return result;
        }
    }
    return runner->m_latter;
}

static bool HasReThrow(TermPtr &block, ScopeStack &stack, Obj &obj) {
    ASSERT(isInterrupt(obj.getType()));
    ASSERT(block->m_id == TermID::BLOCK || block->m_id == TermID::BLOCK_PLUS
            || block->m_id == TermID::BLOCK_MINUS || block->m_id == TermID::BLOCK_TRY);

    if (obj.m_value.empty()) {
        // Non named interrupt
        if (obj.getType() == ObjType::RetPlus && (block->m_id == TermID::BLOCK_PLUS || block->m_id == TermID::BLOCK_TRY)) {
            return false;
        } else if (obj.getType() == ObjType::RetMinus && (block->m_id == TermID::BLOCK_MINUS || block->m_id == TermID::BLOCK_TRY)) {
            return false;
        }

    } else {
        // Named interrupt

        if (obj.m_value.compare("::") == 0) {
            // Global scope - always rethrow
            return true;
        }
        std::string ns = stack.MakeNamespace(0, true);
        if (ns.rfind(obj.m_value) != std::string::npos) {
            return false;
        }

    }
    return true;
}

ObjPtr Context::EvalBlock_(TermPtr &block, TermPtr proto) {
    ASSERT(block->isBlock());
    m_latter = getNoneObj();
    ScopePush scope_block(*this, block, &proto->m_int_vars);
    try {
        size_t i = 0;
        while (i < block->m_block.size()) {

            m_latter = Run(block->m_block[i], this);

            //            LOG_DEBUG("iter: %d, value: %s", (int) i, m_latter->toString().c_str());

            if (isInterrupt(m_latter->getType())) {
                ASSERT(!m_latter->m_value.empty());
                if (CheckInterrupt(m_latter->m_value)) {
                    if (m_latter->getType() == ObjType::RetPlus) {
                        m_latter = m_latter->m_return_obj;
                        break;
                    } else if (m_latter->getType() == ObjType::RetMinus) {
                        m_latter = m_latter->m_return_obj;
                        i = 0;
                        continue;
                    } else {
                        LOG_RUNTIME("Interrupt type '%s' not implemented!", toString(m_latter->getType()));
                    }
                }
            }
            i++;
        }

    } catch (IntPlus &plus) {

        ASSERT(plus.m_obj);
        if (HasReThrow(block, *this, *plus.m_obj)) {
            throw;
        }
        m_latter = plus.m_obj->m_return_obj;

    } catch (IntMinus &minus) {

        ASSERT(minus.m_obj);
        if (HasReThrow(block, *this, *minus.m_obj)) {
            throw;
        }
        m_latter = minus.m_obj->m_return_obj;

    } catch (...) {

        throw;
    }
    return m_latter;
}

// Метод вызывается только из NewLnag кода

ObjPtr Context::Call(Context *runner, Obj &obj, TermPtr &term) {
    ObjPtr args = Obj::CreateDict();

    if (term->m_dims.empty()) {
        obj.m_dimensions = nullptr;
    } else {
        // Размерность, если указана
        obj.m_dimensions = Obj::CreateType(ObjType::Dictionary, ObjType::Dictionary, true);
        for (size_t i = 0; i < term->m_dims.size(); i++) {
            obj.m_dimensions->push_back(EvalTerm(term->m_dims[i], runner));
        }
    }

    for (int64_t i = 0; i < static_cast<int64_t> (term->size()); i++) {

        if ((*term)[i].second->getTermID() == TermID::FILLING) {

            // Заполнение значений вызовом функции
            // :Type(1, 2, 3, ... rand() ... );

            ASSERT(!(*term)[i].second->Left());
            ASSERT((*term)[i].second->Right());

            //            if (!(*term)[i].second->Right()->isCall()) {
            //                LOG_RUNTIME("Operator filling supported function call only!");
            //            }

            if (i + 1 != term->size()) {
                LOG_RUNTIME("Filling is supported for the last argument only!");
            }

            if (!obj.m_dimensions || !obj.m_dimensions->size()) {
                LOG_RUNTIME("Object has no dimensions!");
            }

            int64_t full_size = 1;
            for (int dim_index = 0; dim_index < obj.m_dimensions->size(); dim_index++) {

                if (!(*obj.m_dimensions)[dim_index].second->is_integer()) {
                    LOG_RUNTIME("Dimension index support integer value only!");
                }

                full_size *= (*obj.m_dimensions)[dim_index].second->GetValueAsInteger();
            }

            if (full_size <= 0) {
                LOG_RUNTIME("Items count '%ld' error for all dimensions!", full_size);
            }


            TermPtr fill_obj = (*term)[i].second->Right();
            ASSERT(fill_obj);

            if (fill_obj->size()) {
                LOG_RUNTIME("Argument in function for filling not implemented!");
            }

            for (int64_t dim_index = args->size(); dim_index < full_size; dim_index++) {
                args->push_back(EvalTerm(fill_obj, runner));
            }

            // Filling - last operator in the args
            break;

        } else if ((*term)[i].second->getTermID() == TermID::ELLIPSIS) {

            if (!term->name(i).empty()) {
                LOG_RUNTIME("Named ellipsys not implemented!");
            }

            if ((*term)[i].second->Right()) {

                bool named = ((*term)[i].second->Left() && (*term)[i].second->Left()->getTermID() == TermID::ELLIPSIS);
                ObjPtr exp = EvalTerm((*term)[i].second->Right(), runner);

                //                if (exp->is_funtion()) {
                //                    exp = exp->Call();
                //                }

                if (exp->is_dictionary_type()) {

                    for (int index = 0; index < exp->size(); index++) {
                        if (named) {
                            args->push_back((*exp)[index].second, exp->name(index).empty() ? "" : exp->name(index));
                        } else {
                            args->push_back((*exp)[index].second);
                        }
                    }

                } else if (exp->is_range()) {

                    if (named) {
                        LOG_RUNTIME("Named range expansion not applicable!");
                    }

                    ObjType type = getSummaryTensorType(exp.get());

                    if (isIntegralType(type, true) || isFloatingType(type) || type == ObjType::Rational) {

                        ObjPtr start = exp->at("start").second->Clone();
                        ObjPtr stop = exp->at("stop").second;
                        ObjPtr step = exp->at("step").second;

                        if (*step > Obj::CreateValue(0)) {
                            while (*start < stop) {
                                args->push_back(start->Clone());
                                *start += step;
                            }
                        } else if (*step < Obj::CreateValue(0)) {
                            while (*start > stop) {
                                args->push_back(start->Clone());
                                *start += step;
                            }
                        } else {
                            LOG_RUNTIME("Zero step in range expansion!");
                        }

                    } else {
                        LOG_RUNTIME("Range extension is only supported for integer, rational, or floating point types! (%s)", toString(type));
                    }

                } else {
                    LOG_RUNTIME("Expansion operator applies to dictionary or range only!");
                }

                continue;

            } else {
                // Заполнить до конца имеющимися значениями

                if (!obj.m_dimensions || !obj.m_dimensions->size()) {
                    LOG_RUNTIME("Object has no dimensions!");
                }

                //            if (!(*term)[i].second->Right()->isCall()) {
                //                LOG_RUNTIME("Operator filling supported function call only!");
                //            }

                if (i + 1 != term->size()) {
                    LOG_RUNTIME("Filling is supported for the last argument only!");
                }

                int64_t full_size = 1;
                for (int dim_index = 0; dim_index < obj.m_dimensions->size(); dim_index++) {

                    if (!(*obj.m_dimensions)[dim_index].second->is_integer()) {
                        LOG_RUNTIME("Dimension index support integer value only!");
                    }

                    full_size *= (*obj.m_dimensions)[dim_index].second->GetValueAsInteger();
                }

                if (full_size <= 0) {
                    LOG_RUNTIME("Items count '%ld' error for all dimensions!", full_size);
                }


                std::vector<ObjPtr> dup;

                for (size_t c = 0; c < args->size(); c++) {
                    dup.push_back(args->at(c).second);
                }

                if (!dup.size()) {
                    LOG_RUNTIME("Fill items not exist!");
                }


                if ((full_size - args->size()) % dup.size() && runner) {
                    if (runner->m_runtime->m_diag->m_fill_remainder) {
                        LOG_RUNTIME("The data is filled in with the remainder!");
                    } else {
                        LOG_WARNING("The data is filled in with the remainder!");
                        // runner->m_runtime->m_diag->Emit(Diag::DIAG_FILL_REMAINDER);
                    }
                }


                int64_t fill_index = 0;
                for (int64_t dim_index = args->size(); dim_index < full_size; dim_index++) {
                    args->push_back(dup[fill_index]->Clone());
                    fill_index++;
                    if (fill_index >= dup.size()) {
                        fill_index = 0;
                    }
                }
                // Filling - last operator in the args
                break;
            }
        }

        if (term->name(i).empty()) {
            args->push_back(EvalTerm((*term)[i].second, runner));
        } else {

            args->push_back(EvalTerm((*term)[i].second, runner), term->name(i).c_str());
        }
    }

    return Call(runner, obj, *args.get());
}

// Метод может быть вызван как из NewLnag кода, так и из кода на C/C++ (в реализации Obj::operator())

ObjPtr Context::Call(Context *runner, Obj &obj, Obj & args) {

    if (obj.is_native()) {
        return CallNative_(runner, obj, &args);
    }

    args.insert(args.begin(), std::pair<std::string, ObjPtr>("", obj.shared()));
    //    args->push_back(Obj::CreateValue(args->size()), "$#");
    //    args->push_back(args->Clone(), "$*");

    if (obj.is_string_type() || obj.is_dictionary_type()) {
        if (obj.is_string_type()) {
            if (obj.getType() == ObjType::FmtChar || obj.getType() == ObjType::FmtWide) {
                return StringPrintf(obj.GetValueAsString(), args);
            } else {
                return StringFormat(obj.GetValueAsString(), args);
            }
        } else {
            LOG_RUNTIME("Clone dict not implemented!");
        }
    }

    if (!runner) {
        LOG_RUNTIME("Call static not allowed!");
    }

    if (obj.is_function_type() || obj.is_type_name()) {

        if (obj.m_sequence) {
            return runner->EvalBlock_(obj.m_sequence, obj.m_prototype);
        } else {
            ASSERT(at::holds_alternative<void *>(obj.m_var));
            FunctionType * func_ptr = (FunctionType *) at::get<void *>(obj.m_var);
            ASSERT(func_ptr);
            return (*func_ptr)(runner, args);
        }
    }

    ASSERT(!(obj.is_string_type() || obj.is_dictionary_type()));

    return obj.Clone();
}

ObjPtr Context::CallNative_(Context *runner, Obj &obj, Obj * args) {

    ASSERT(RunTime::m_ffi_prep_cif);
    ASSERT(RunTime::m_ffi_prep_cif_var);

    ffi_cif m_cif;
    std::vector<ffi_type *> m_args_type;
    std::vector<void *> m_args_ptr;

    union VALUE {
        const void *ptr;
        size_t size;
        int64_t integer;
        double number;
        bool boolean;
    };

    std::vector<VALUE> m_args_val;
    VALUE temp;

    ASSERT(obj.m_var_type_current == ObjType::NativeFunc);
    ASSERT(obj.m_prototype);

    ASSERT(at::holds_alternative<void *>(obj.m_var));
    void * func_ptr = at::get<void *>(obj.m_var);

    if (!func_ptr) {
        LOG_RUNTIME("Dymanic load native address '%s'!", "NOT IMPLEMENTED");
    }
    //    NL_CHECK(func_ptr, "Fail load func name '%s' (%s) or fail load module '%s'!", m_prototype->m_text.c_str(),
    //            m_func_mangle_name.empty() ? m_prototype->m_text.c_str() : m_func_mangle_name.c_str(),
    //            m_module_name.empty() ? "none" : m_module_name.c_str());

    //    bool is_ellipsis = (obj.m_prototype->size() && (*obj.m_prototype)[obj.m_prototype->size() - 1].second->getTermID() == TermID::ELLIPSIS);
    //    size_t check_count = is_ellipsis ? obj.m_prototype->size() - 1 : obj.m_prototype->size();

    // Пропустить нулевой аргумент для нативных функций
    for (int i = 0; args && i < args->size(); i++) {

        ASSERT((*args)[i].second);
        if ((*args)[i].second->m_is_reference) {
            LOG_RUNTIME("Argument REFERENCE! %s", (*args)[i].second->toString().c_str());
        }

        size_t pind = i; // - 1; // Индекс прототипа на единицу меньше из-за пустого нулевого аргумента

        ObjType type = (*args)[i].second->getTypeAsLimit();
        switch (type) {
            case ObjType::Bool:
                //                if (pind < check_count) {
                //                    NL_CHECK(!isDefaultType((*obj.m_prototype)[pind].second->m_type), "Undefined type arg '%s'", (*obj.m_prototype)[pind].second->toString().c_str());
                //                    NL_CHECK(canCast(type, typeFromString((*obj.m_prototype)[pind].second->m_type, GetRT_(runner))), "Fail cast from '%s' to '%s'",
                //                            (*obj.m_prototype)[pind].second->m_type->asTypeString().c_str(), newlang::toString(type));
                //                }
                m_args_type.push_back(RunTime::m_ffi_type_uint8);
                temp.boolean = (*args)[i].second->GetValueAsBoolean();
                m_args_val.push_back(temp);
                break;

            case ObjType::Int8:
            case ObjType::Char:
            case ObjType::Byte:
                //                if (pind < check_count) {
                //                    NL_CHECK(!isDefaultType((*obj.m_prototype)[pind].second->m_type), "Undefined type arg '%s'", (*obj.m_prototype)[pind].second->toString().c_str());
                //                    NL_CHECK(canCast(type, typeFromString((*obj.m_prototype)[pind].second->m_type, GetRT_(runner))), "Fail cast from '%s' to '%s'",
                //                            (*obj.m_prototype)[pind].second->m_type->asTypeString().c_str(), newlang::toString(type));
                //                }
                m_args_type.push_back(RunTime::m_ffi_type_sint8);
                temp.integer = (*args)[i].second->GetValueAsInteger();
                m_args_val.push_back(temp);
                break;

            case ObjType::Int16:
            case ObjType::Word:
                //                if (pind < check_count) {
                //                    NL_CHECK(!isDefaultType((*obj.m_prototype)[pind].second->m_type), "Undefined type arg '%s'", (*obj.m_prototype)[pind].second->toString().c_str());
                //                    NL_CHECK(canCast(type, typeFromString((*obj.m_prototype)[pind].second->m_type, GetRT_(runner))), "Fail cast from '%s' to '%s'",
                //                            (*obj.m_prototype)[pind].second->m_type->m_text.c_str(), newlang::toString(type));
                //                }
                m_args_type.push_back(RunTime::m_ffi_type_sint16);
                temp.integer = (*args)[i].second->GetValueAsInteger();
                m_args_val.push_back(temp);
                break;

            case ObjType::Int32:
            case ObjType::DWord:
                //                if (pind < check_count) {
                //                    NL_CHECK(!isDefaultType((*obj.m_prototype)[pind].second->m_type), "Undefined type arg '%s'", (*obj.m_prototype)[pind].second->toString().c_str());
                //                    NL_CHECK(canCast(type, typeFromString((*obj.m_prototype)[pind].second->m_type, GetRT_(runner))), "Fail cast from '%s' to '%s'",
                //                            (*obj.m_prototype)[pind].second->m_type->m_text.c_str(), newlang::toString(type));
                //                }
                m_args_type.push_back(RunTime::m_ffi_type_sint32);
                temp.integer = (*args)[i].second->GetValueAsInteger();
                m_args_val.push_back(temp);
                break;

            case ObjType::Int64:
            case ObjType::DWord64:
                //                if (pind < check_count) {
                //                    NL_CHECK(!isDefaultType((*obj.m_prototype)[pind].second->m_type), "Undefined type arg '%s'", (*obj.m_prototype)[pind].second->toString().c_str());
                //                    NL_CHECK(canCast(type, typeFromString((*obj.m_prototype)[pind].second->m_type, GetRT_(runner))), "Fail cast from '%s' to '%s'",
                //                            (*obj.m_prototype)[pind].second->m_type->m_text.c_str(), newlang::toString(type));
                //                }
                m_args_type.push_back(RunTime::m_ffi_type_sint64);
                temp.integer = (*args)[i].second->GetValueAsInteger();
                m_args_val.push_back(temp);
                break;

            case ObjType::Float32:
            case ObjType::Single:
                //                if (pind < check_count) {
                //                    NL_CHECK(!isDefaultType((*obj.m_prototype)[pind].second->m_type), "Undefined type arg '%s'", (*obj.m_prototype)[pind].second->toString().c_str());
                //                    NL_CHECK(canCast(type, typeFromString((*obj.m_prototype)[pind].second->m_type, GetRT_(runner))), "Fail cast from '%s' to '%s'",
                //                            (*obj.m_prototype)[pind].second->m_type->m_text.c_str(), newlang::toString(type));
                //                }
                m_args_type.push_back(RunTime::m_ffi_type_float);
                temp.number = (*args)[i].second->GetValueAsNumber();
                m_args_val.push_back(temp);
                break;

            case ObjType::Float64:
            case ObjType::Double:
                //                if (pind < check_count) {
                //                    NL_CHECK(!isDefaultType((*obj.m_prototype)[pind].second->m_type), "Undefined type arg '%s'", (*obj.m_prototype)[pind].second->toString().c_str());
                //                    NL_CHECK(canCast(type, typeFromString((*obj.m_prototype)[pind].second->m_type, GetRT_(runner))), "Fail cast from '%s' to '%s'",
                //                            (*obj.m_prototype)[pind].second->m_type->m_text.c_str(), newlang::toString(type));
                //                }
                m_args_type.push_back(RunTime::m_ffi_type_double);
                temp.number = (*args)[i].second->GetValueAsNumber();
                m_args_val.push_back(temp);
                break;

            case ObjType::StrChar:
            {
                //                if (pind < check_count) {
                //                    NL_CHECK(!isDefaultType((*obj.m_prototype)[pind].second->m_type), "Undefined type arg '%s'", (*obj.m_prototype)[pind].second->toString().c_str());
                //                    ObjType target = typeFromString((*obj.m_prototype)[pind].second->m_type, GetRT_(runner));
                //                    if (!canCast(type, target)) {
                //                        if ((target == ObjType::Int8 || target == ObjType::Char || target == ObjType::Byte)
                //                                && isStringChar(type) && (*args)[i].second->size() == 1) {
                //
                //                            m_args_type.push_back(RunTime::m_ffi_type_sint8);
                //                            temp.integer = (*args)[i].second->m_value[0];
                //                            m_args_val.push_back(temp);
                //                            break;
                //
                //                        }
                //                        LOG_RUNTIME("Fail cast from '%s' to '%s'", toString(type), toString(target));
                //                    }
                //                }
                m_args_type.push_back(RunTime::m_ffi_type_pointer);
                temp.ptr = (*args)[i].second->m_value.c_str();
                m_args_val.push_back(temp);
                break;
            }
            case ObjType::StrWide:
                //                if (pind < check_count) {
                //                    NL_CHECK(!isDefaultType((*obj.m_prototype)[pind].second->m_type), "Undefined type arg '%s'", (*obj.m_prototype)[pind].second->toString().c_str());
                //                    ObjType target = typeFromString((*obj.m_prototype)[pind].second->m_type, GetRT_(runner));
                //                    if (!canCast(type, target)) {
                //                        if (target == RunTime::m_wide_char_type && isStringWide(type) && (*args)[i].second->size() == 1) {
                //
                //                            m_args_type.push_back(RunTime::m_wide_char_type_ffi);
                //                            temp.integer = (*args)[i].second->m_string[0];
                //                            m_args_val.push_back(temp);
                //                            break;
                //
                //                        }
                //                        LOG_RUNTIME("Fail cast from '%s' to '%s'", toString(type), toString(target));
                //                    }
                //                }
                m_args_type.push_back(RunTime::m_ffi_type_pointer);
                temp.ptr = (*args)[i].second->m_string.c_str();
                m_args_val.push_back(temp);
                break;

            case ObjType::Pointer:
                //                if (pind < check_count) {
                //                    NL_CHECK(!isDefaultType((*obj.m_prototype)[pind].second->m_type), "Undefined type arg '%s'", (*obj.m_prototype)[pind].second->toString().c_str());
                //                    NL_CHECK(canCast(type, typeFromString((*obj.m_prototype)[pind].second->m_type, GetRT_(runner))), "Fail cast from '%s' to '%s'",
                //                            (*obj.m_prototype)[pind].second->m_type->m_text.c_str(), newlang::toString(type));
                //                }
                m_args_type.push_back(RunTime::m_ffi_type_pointer);

                if (at::holds_alternative<void *>((*args)[i].second->m_var)) {
                    temp.ptr = at::get<void *>((*args)[i].second->m_var);
                    //                } else if (args[i].second->m_var_type_fixed == ObjType::None || args[i].second->m_var_type_current == ObjType::None ) {
                    //                    temp.ptr = nullptr;
                } else {
                    LOG_RUNTIME("Fail convert arg '%s' to pointer!", (*args)[i].second->toString().c_str());
                }

                m_args_val.push_back(temp);
                break;

            default:
                LOG_RUNTIME("Native arg type '%s' not implemented!", newlang::toString((*args)[i].second->getType()));
        }
        //        if (pind < check_count && (*obj.m_prototype)[pind].second->GetType()) {
        //            if ((*obj.m_prototype)[pind].second->GetType()->m_text.compare(newlang::toString(ObjType::FmtChar)) == 0) {
        //                NL_CHECK(newlang::ParsePrintfFormat(args, i), "Fail format string or type args!");
        //            }
        //        }
    }

    for (size_t i = 0; i < m_args_val.size(); i++) {
        m_args_ptr.push_back((void *) &m_args_val[i]);
    }

    NL_CHECK(!isDefaultType(obj.m_prototype->m_type), "Undefined return type '%s'", obj.m_prototype->toString().c_str());

    VALUE res_value;
    ffi_type *result_ffi_type = nullptr;

    ObjType return_type = RunTime::BaseTypeFromString(GetRT_(runner), obj.m_prototype->m_type->m_text);

    switch (return_type) {
        case ObjType::Bool:
            result_ffi_type = RunTime::m_ffi_type_uint8;
            break;

        case ObjType::Int8:
        case ObjType::Char:
        case ObjType::Byte:
            result_ffi_type = RunTime::m_ffi_type_sint8;
            break;

        case ObjType::Int16:
        case ObjType::Word:
            result_ffi_type = RunTime::m_ffi_type_sint16;
            break;

        case ObjType::Int32:
        case ObjType::DWord:
            result_ffi_type = RunTime::m_ffi_type_sint32;
            break;

        case ObjType::Int64:
        case ObjType::DWord64:
            result_ffi_type = RunTime::m_ffi_type_sint64;
            break;

        case ObjType::Float32:
        case ObjType::Single:
            result_ffi_type = RunTime::m_ffi_type_float;
            break;

        case ObjType::Float64:
        case ObjType::Double:
            result_ffi_type = RunTime::m_ffi_type_double;
            break;

        case ObjType::Pointer:
        case ObjType::StrChar:
        case ObjType::StrWide:
            result_ffi_type = RunTime::m_ffi_type_pointer;
            break;

        case ObjType::None:
            result_ffi_type = RunTime::m_ffi_type_void;
            break;

        default:
            LOG_RUNTIME("Native return type '%s' not implemented!", obj.m_prototype->m_type->asTypeString().c_str());
    }

    //    ASSERT(ctx->m_func_abi == FFI_DEFAULT_ABI); // Нужны другие типы вызовов ???
    if (RunTime::m_ffi_prep_cif(&m_cif, FFI_DEFAULT_ABI, static_cast<unsigned int> (m_args_type.size()), result_ffi_type, m_args_type.data()) == FFI_OK) {

        ASSERT(obj.m_prototype->m_type);

        RunTime::m_ffi_call(&m_cif, FFI_FN(func_ptr), &res_value, m_args_ptr.data());

        if (result_ffi_type == RunTime::m_ffi_type_void) {
            return Obj::CreateNone();
        } else if (result_ffi_type == RunTime::m_ffi_type_uint8) {
            // Возвращаемый тип может быть как Byte, так и Bool
            return Obj::CreateValue(static_cast<uint8_t> (res_value.integer), return_type);
        } else if (result_ffi_type == RunTime::m_ffi_type_sint8) {
            return Obj::CreateValue(static_cast<int8_t> (res_value.integer), return_type);
        } else if (result_ffi_type == RunTime::m_ffi_type_sint16) {
            return Obj::CreateValue(static_cast<int16_t> (res_value.integer), return_type);
        } else if (result_ffi_type == RunTime::m_ffi_type_sint32) {
            return Obj::CreateValue(static_cast<int32_t> (res_value.integer), return_type);
        } else if (result_ffi_type == RunTime::m_ffi_type_sint64) {
            return Obj::CreateValue(res_value.integer, return_type);
        } else if (result_ffi_type == RunTime::m_ffi_type_float) {
            return Obj::CreateValue(res_value.number, return_type);
        } else if (result_ffi_type == RunTime::m_ffi_type_double) {
            return Obj::CreateValue(res_value.number, return_type);
        } else if (result_ffi_type == RunTime::m_ffi_type_pointer) {
            if (return_type == ObjType::StrChar) {
                return Obj::CreateString(reinterpret_cast<const char *> (res_value.ptr));
            } else if (return_type == ObjType::StrWide) {
                return Obj::CreateString(reinterpret_cast<const wchar_t *> (res_value.ptr));
            } else if (return_type == ObjType::Pointer) {
                ObjPtr result = Obj::CreateBaseType(return_type); //RunTime::GetTypeFromString(GetRT_(runner), obj.m_prototype->m_type->m_text.c_str());
                result->m_var = (void *) res_value.ptr;
                result->m_var_is_init = true;
                return result;
            } else {
                LOG_RUNTIME("Error result type '%s' or not implemented!", obj.m_prototype->m_type->m_text.c_str());
            }
        } else {
            LOG_RUNTIME("Native return type '%s' not implemented!", obj.m_prototype->m_type->m_text.c_str());
        }
    }

    LOG_RUNTIME("Fail native call '%s'!", obj.toString().c_str());

    return Obj::CreateNone();
}

ObjPtr Context::CreateArgs_(ObjPtr &args, TermPtr &term, Context * runner) {
    for (int i = 0; i < term->size(); i++) {
        ASSERT((*term)[i].second);
        if (term->name(i).empty()) {
            args->push_back(EvalTerm((*term)[i].second, runner));
        } else {

            args->push_back(EvalTerm((*term)[i].second, runner), term->name(i).c_str());
        }
    }
    return args;
}

ObjPtr Context::CreateDict(TermPtr &term, Context * runner) {
    ObjPtr result = Obj::CreateDict();

    CreateArgs_(result, term, runner);
    result->m_var_is_init = true;

    if (term->getTermID() == TermID::DICT) {

        result->m_var_type_fixed = ObjType::Class;
        result->m_class_name = term->m_class;
    }

    return result;
}

ObjPtr Context::CreateRange(TermPtr &term, Context * runner) {
    ObjPtr result = Obj::CreateDict();

    ASSERT(term->m_type);
    ObjType type = GetBaseTypeFromString(term->m_type->m_text);
    for (int i = 0; i < term->size(); i++) {
        ASSERT(!term->name(i).empty());
        result->push_back(EvalTerm((*term)[i].second, runner), term->name(i).c_str());
        result->back().second->toType_(type);
    }
    if (result->size() == 2) {
        result->push_back(Obj::CreateValue(1, type), "step");
    }
    result->m_var_type_current = ObjType::Range;
    result->m_var_type_fixed = ObjType::Range;
    result->m_var_is_init = true;

    return result;
}

ObjPtr Context::CreateTensor(TermPtr &term, Context * runner) {

    ASSERT(term->getTermID() == TermID::TENSOR);
    ASSERT(term->m_class.empty());

    ObjPtr result = CreateDict(term, runner);
    if (!term->m_type) {
        result->m_var_type_fixed = ObjType::None;
    } else {
        result->m_var_type_fixed = GetBaseTypeFromString(term->m_type->m_text); //, GetRT_(runner)
    }
    ObjType type = getSummaryTensorType(result.get(), result->m_var_type_fixed);

    if (type != ObjType::None) {

        std::vector<int64_t> sizes = TensorShapeFromDict(result.get());
        result->toType_(type);

        if (!sizes.empty()) {
            ASSERT(result->m_tensor.defined());
            result->m_tensor = result->m_tensor.reshape(sizes);
        }

    } else {
        result->m_var_is_init = false;
    }
    result->m_var_type_current = type;

    return result;
}

ObjPtr Context::StringFormat(std::string_view format, Obj & args) {
    ASSERT(0);

    return nullptr;
}

ObjPtr Context::StringPrintf(std::string_view format, Obj & args) {
    ASSERT(0);

    return nullptr;
}

ObjPtr Context::EvalTerm(TermPtr term, Context * runner, bool rvalue) {
    ASSERT(term);

    try {
        // Static calculate
        switch (term->m_id) {
            case TermID::INTEGER:
            {
                int64_t val_int = parseInteger(term->getText().c_str());
                NL_TYPECHECK(term, typeFromLimit(val_int), typeFromString(term->m_type, GetRT_(runner))); // Соответстствует ли тип значению?
                ObjPtr result = Obj::CreateValue(val_int);
                result->m_var_type_current = typeFromLimit(val_int);
                if (term->GetType() && !isDefaultType(term->GetType())) {
                    result->m_var_type_fixed = typeFromString(term->m_type, GetRT_(runner));
                    result->m_var_type_current = result->m_var_type_fixed;
                }
                return result;
            }
            case TermID::NUMBER:
            {
                double val_dbl = parseDouble(term->getText().c_str());
                NL_TYPECHECK(term, typeFromLimit(val_dbl), typeFromString(term->m_type, GetRT_(runner))); // Соответстствует ли тип значению?
                ObjPtr result = Obj::CreateValue(val_dbl);
                result->m_var_type_current = typeFromLimit(val_dbl);
                if (term->GetType() && !isDefaultType(term->GetType())) {
                    result->m_var_type_fixed = typeFromString(term->m_type, GetRT_(runner));
                    result->m_var_type_current = result->m_var_type_fixed;
                }
                return result;
            }
            case TermID::RATIONAL:
            {
                ObjPtr result = Obj::CreateRational(term->m_text);
                result->m_var_type_fixed = result->m_var_type_current;
                return result;
            }
            case TermID::STRWIDE:
            {
                ObjPtr result = Obj::CreateString(utf8_decode(term->getText()));
                ASSERT(result);
                if (term->isCall()) {
                    // Format string
                    result = Call(runner, *result, term);
                }
                return result;
            }
            case TermID::STRCHAR:
            {
                ObjPtr result = Obj::CreateString(term->getText());
                ASSERT(result);
                if (term->isCall()) {
                    // Format string
                    result = Call(runner, *result, term);
                }
                return result;
            }

            case TermID::TENSOR:
                return CreateTensor(term, runner);
            case TermID::DICT:
                return CreateDict(term, runner);
            case TermID::RANGE:
                return CreateRange(term, runner);
            case TermID::OP_MATH:
                return EvalOpMath_(term, runner);
            case TermID::OP_COMPARE:
                return EvalOpCompare_(term, runner);
            case TermID::OP_LOGICAL:
                return EvalOpLogical_(term, runner);
            case TermID::OP_BITWISE:
                return EvalOpBitwise_(term, runner);
            case TermID::WHILE:
                return EvalWhile_(term, runner);
            case TermID::DOWHILE:
                return EvalDoWhile_(term, runner);
            case TermID::FOLLOW:
                return EvalFollow_(term, runner);
            case TermID::NATIVE:
                return Obj::CreateBool(RunTime::GetDirectAddressFromLibrary(nullptr, term->m_text.c_str()));
            case TermID::ELLIPSIS:
                return getEllipsysObj();
            case TermID::END:
                return Obj::CreateNone();
        }

        if (!runner) {
            NL_PARSER(term, "The term type '%s' is not calculated statistically!", newlang::toString(term->getTermID()));
        }

        if (term->isCreate()) {
            return runner->EvalCreate_(term);
        } else if (term->isNamed()) {

            TermPtr found = runner->GetObject(term, runner->m_runtime);
            if (!found->m_obj) {
                NL_PARSER(term, "Object '%s' not calculated!", term->m_text.c_str());
            }

            if (term->isCall()) {
                // Вызов функции или клонирование объекта
                ASSERT(found->m_obj);
                return Call(runner, *found->m_obj, term);
            } else if (rvalue && term->m_right) {

                if (term->m_right->m_id == TermID::INDEX) {
                    return GetIndexValue(term, found->m_obj, runner);
                } else if (term->m_right->m_id == TermID::FIELD) {
                    return GetFieldValue(term, found->m_obj, runner);
                } else {
                    NL_PARSER(term, "Eval type '%s' not implemented!", toString(term->m_right->m_id));
                }

            } else {
                return found->m_obj;
            }

        } else {

            switch (term->m_id) {

                case TermID::ITERATOR:
                    return runner->EvalIterator_(term);
                case TermID::EVAL:
                    return runner->EvalEval_(term);
                case TermID::INT_PLUS:
                case TermID::INT_MINUS:
                case TermID::INT_REPEAT:
                    return runner->EvalInterrupt_(term);

            }

            //        if (term->GetType() && !isDefaultType(term->GetType())) {
            //            term->m_obj->m_var_type_fixed = typeFromString(term->m_type, m_runtime.get());
            //            term->m_obj->m_var_type_current = term->m_obj->m_var_type_fixed;
            //        }
        }

        LOG_RUNTIME("EvalTerm_ for type '%s'(%s) not implemented!", newlang::toString(term->m_id), term->m_text.c_str());

    } catch (std::exception & ex) {
        NL_PARSER(term, "%s", ex.what());
    }
}

ObjPtr Context::CreateNative_(TermPtr &proto, const char *module, bool lazzy, const char *mangle_name) {
    ObjPtr result = m_runtime->CreateNative(proto, module, lazzy, mangle_name);
    result->m_ctx = this;

    return result;
}

ObjPtr Context::EvalCreate_(TermPtr & op) {
    ASSERT(op);
    ASSERT(op->isCreate());
    ASSERT(op->m_left);

    ArrayTermType l_vars = op->m_left->CreateArrayFromList();

    bool is_ellipsis = false;
    TermPtr var_found;
    for (auto &elem : l_vars) {

        if (elem->getTermID() == TermID::ELLIPSIS) {

            //@todo добавить поддержку многоточия с левой стороный оператора присвоения
            // NL_PARSER(elem, "Ellipsis on the left side in assignment not implemented!");

            //  Игнорировать несколько объектов
            elem->m_obj = getEllipsysObj();
            if (is_ellipsis) {
                NL_PARSER(elem, "Multiple ellipsis on the left side of the assignment!");
            }
            is_ellipsis = true;

        } else if (elem->isNone()) {

            //  Игнорировать один объект
            elem->m_obj = getNoneObj();

        } else {

            var_found = GetObject(elem, m_runtime);

            if (op->isCreateOnce() && var_found && var_found->m_obj) {
                NL_PARSER(elem, "Object '%s' already exist", elem->m_text.c_str());
            } else if (op->getTermID() == TermID::ASSIGN && !(var_found && var_found->m_obj)) {
                NL_PARSER(elem, "Object '%s' not exist!", elem->m_text.c_str());
            }

            if (var_found) {
                if (var_found->isCall() && var_found->m_obj) {
                    NL_PARSER(elem, "The function cannot be overridden! '%s'", var_found->toString().c_str());
                }
                elem->m_int_name = var_found->m_int_name;
            }
        }
    }

    m_latter = AssignVars_(l_vars, op->m_right, op->isPure());
    for (auto &elem : l_vars) {
        if (isGlobalScope(elem->m_int_name)) {
            m_runtime->NameRegister(op->isCreateOnce(), elem->m_int_name.c_str(), elem, elem->m_obj);
        }
    }
    return m_latter;
}

ObjPtr Context::AssignVars_(ArrayTermType &l_vars, const TermPtr &r_term, bool is_pure) {

    if (r_term->getTermID() == TermID::NATIVE) {

        ASSERT(l_vars.size() == 1);
        //        ASSERT(vars[0]->m_obj);
        //        ASSERT(r_term->m_obj);

        l_vars[0]->m_obj = CreateNative_(l_vars[0], nullptr, false, r_term->m_text.substr(1).c_str());
        //        vars[0]->m_obj = r_term->m_obj;
        m_latter = l_vars[0]->m_obj;

    } else if (r_term->getTermID() == TermID::ELLIPSIS) {
        // При раскрытии словаря присвоить значение можно как одному, так и сразу нескольким терминам: 
        // var1, var2, _ = ... func(); Первый и второй элементы записывается в var1 и var2, 
        // а все остальные игнорируются (если они есть)
        // var1, var2 = ... func(); Если функция вернула словарь с двумя элементами, 
        // то их значения записываются в var1 и var2. Если в словаре было больше двух элементов, 
        // то первый записывается в var1, а все оставшиеся элементы в var2. !!!!!!!!!!!!!
        // _, var1, ..., var2 = ... func(); 
        // Первый элемент словаря игнорируется, второй записывается в var1, а последний в var2.

        //@todo добавить поддержку многоточия с левой стороный оператора присвоения
        NL_PARSER(r_term, "ELLIPSIS NOT implemented!");

    } else if (r_term->getTermID() == TermID::FILLING) {
        // Заполнение переменных значениями последовательным вызовом фукнции?

        //@todo добавить поддержку многоточия с левой стороный оператора присвоения
        NL_PARSER(r_term, "FILLING NOT implemented!");

    } else if (r_term->isBlock()) {

        if (l_vars.size() > 1) {
            //@todo добавить поддержку присвоения сразу нескольким функциям
            NL_PARSER(r_term, "Multiple function assignment not implemented!");
        }
        ASSERT(l_vars.size() == 1);

        if (!l_vars[0]->isCall()) {
            NL_PARSER(l_vars[0], "Function name expected!");
        }

        ASSERT(!l_vars[0]->m_right);

        l_vars[0]->m_obj = m_runtime->CreateFunction(l_vars[0], r_term);
        m_latter = l_vars[0]->m_obj;

    } else {

        ArrayTermType r_vars = r_term->CreateArrayFromList();
        if (r_vars.empty()) {
            // Delete vars
            NL_PARSER(r_term, "NOT implemented!");
            //            m_latter = getNoneObj();

        } else if (r_vars.size() == 1) {

            m_latter = Run(r_vars[0], this);
            for (auto &elem : l_vars) {
                if (elem->getTermID() == TermID::ELLIPSIS) {
                    NL_PARSER(elem, "Ellipses unexpected!");
                } else if (elem->isNone()) {
                    NL_PARSER(elem, "None var unexpected!");
                } else if (elem->m_right) {

                    if (elem->m_right->m_id == TermID::INDEX) {
                        SetIndexValue(elem, m_latter, this);
                    } else if (elem->m_right->m_id == TermID::FIELD) {
                        SetFieldValue(elem, m_latter, this);
                    } else {
                        NL_PARSER(elem, "Eval type '%s' not implemented!", toString(elem->m_right->m_id));
                    }

                } else {
                    elem->m_obj = m_latter;
                }
            }

        } else {
            // Что присваиваем (правая часть выражения)
            // @todo В будущем можно будет сделать сахар для обмена значениями при одинаковом кол-ве объектов у оператора присваивания
            // a, b = b, a;   a, b, c = c, b, a; и т.д.

            if (l_vars.size() != r_vars.size()) {
                NL_PARSER(r_term, "Fail count right values! Expected one or %d.", (int) l_vars.size());
            }
            for (size_t i = 0; i < l_vars.size(); i++) {
                if (l_vars[i]->getTermID() == TermID::ELLIPSIS) {
                    NL_PARSER(l_vars[i], "Ellipses unexpected!");
                } else if (l_vars[i]->isNone()) {
                    NL_PARSER(l_vars[i], "None var unexpected!");
                } else {

                    m_latter = Run(r_vars[i], this);
                    l_vars[i]->m_obj = m_latter;
                }
            }
        }
    }

    return m_latter;



    //    std::vector<TermPtr> list_term; // Список терминов для создания/присвоения значения
    //    std::vector<ObjPtr> list_obj; // Список реальных переменных ассоциированных с терминами
    //
    //    bool is_eval_block = true;
    //
    //    TermPtr next = term->Left();
    //    while (next && next->getTermID() != TermID::END) {
    //        if (next->isCall()) {
    //            is_eval_block = false;
    //        }
    //        list_term.push_back(next);
    //        next = next->m_list;
    //    }
    //
    //
    //    ObjPtr result;
    //    bool is_ellipsis = false;
    //    for (auto & elem : list_term) {
    //
    //        if (elem->getTermID() == TermID::ELLIPSIS) {
    //
    //            //@todo добавить поддержку многоточия с левой стороный оператора присвоения
    //            NL_PARSER(elem, "Ellipsis on the left side in assignment not implemented!");
    //
    //            if (is_ellipsis) {
    //                NL_PARSER(elem, "Multiple ellipsis on the left side of the assignment!");
    //            }
    //            is_ellipsis = true;
    //            result = Obj::CreateType(ObjType::Ellipsis);
    //        } else if (elem->getTermID() == TermID::NONE) {
    //            result = Obj::CreateNone();
    //        } else {
    //            // LOG_DEBUG("find: %s", ctx->NamespaceFull(elem->GetFullName()).c_str());
    //            auto found = ctx->find(elem->GetFullName());
    //            if (found == ctx->end() && mode == CreateMode::ASSIGN_ONLY) {
    //                NL_PARSER(elem, "Object '%s' (%s) not found!", elem->m_text.c_str(), elem->GetFullName().c_str());
    //            }
    //
    //            if (found != ctx->end()) {
    //                result = (*found).second.lock(); // Но она может быть возвращена как локальная
    //            }
    //
    //            if (result && mode == CreateMode::CREATE_ONLY) {
    //                NL_PARSER(elem, "Object '%s' (%s) already exists!", elem->m_text.c_str(), elem->GetFullName().c_str());
    //            }
    //
    //            if (!term->Right()) { // Удаление глобальной переменной
    //                ctx->ListType::erase(found);
    //            } else {
    //                if (!result && (mode == CreateMode::ASSIGN_ONLY)) {
    //                    NL_PARSER(term->Left(), "Object '%s' (%s) not found!", term->Left()->m_text.c_str(), elem->GetFullName().c_str());
    //                }
    //                if (!result) {
    //                    result = CreateLVal(ctx, elem, local_vars);
    //                    if (!result) {
    //                        NL_PARSER(term->Left(), "Fail create lvalue object!");
    //                    }
    //                }
    //            }
    //        }
    //        list_obj.push_back(result);
    //    }
    //
    //    if (!term->Right()) {
    //        // Для удаления переменных все сделано
    //        return result;
    //    }

    //    // Что присваиваем (правая часть выражения) - пока единичный объект
    //    // @todo В будущем можно будет сделать сахар для обмена значениями при одинаковом кол-ве объектов у оператора присваивания
    //    // a, b = b, a;   a, b, c = c, b, a; и т.д.
    //    if (term->Right() && term->Right()->getTermID() != TermID::ELLIPSIS && term->Right()->m_list) {
    //        NL_PARSER(term->Right()->Right(), "Multiple assignments not implemented!");
    //    }

    //    ObjPtr rval;
    //    if (term->Right()->getTermID() == TermID::ELLIPSIS) {
    //        ASSERT(term->Right()->Right());
    //        rval = Eval(ctx, term->Right()->Right(), local_vars, is_eval_block, CatchType::CATCH_AUTO);
    //    } else if (term->Right()->getTermID() == TermID::CLASS) {
    //        ASSERT(list_obj.size() == 1);
    //        // Имя класса появляется только при операции присвоения в левой части оператора
    //        rval = ctx->CreateClass(term->Left()->GetFullName(), term->Right(), local_vars);
    //    } else if (term->Right()->getTermID() == TermID::NATIVE) {
    //        // Нативное имя допустимо только в правой части части оператора присвоения
    //        ASSERT(list_obj.size() == 1);
    //
    //        if (term->Left()->isCall() != term->Right()->isCall()) {
    //            // Нативная функция с частичным прототипом
    //            TermPtr from;
    //            TermPtr to;
    //            if (term->Left()->isCall()) {
    //                from = term->Left();
    //                to = term->Right();
    //            } else {
    //                ASSERT(term->Right()->isCall());
    //                from = term->Right();
    //                to = term->Left();
    //            }
    //            for (int i = 0; i < from->size(); i++) {
    //                to->push_back((*from)[i]);
    //            }
    //            to->m_is_call = from->m_is_call;
    //            to->m_type = from->m_type;
    //            //            to->m_type_name = from->m_type_name;
    //
    //        } else if (!term->Left()->isCall() && !term->Right()->isCall()) {
    //            // Нативная переменная
    //            if (!term->Right()->m_type) {
    //                term->Right()->m_type = term->Left()->m_type;
    //                //                term->Right()->m_type_name = term->Left()->m_type_name;
    //            }
    //
    //        } else if (term->Left()->isCall() && term->Right()->isCall()) {
    //            LOG_RUNTIME("Check args in native func not implemented!");
    //        }
    //        rval = ctx->m_runtime->CreateNative(ctx, term->Right(), nullptr, false, term->Right()->m_text.substr(1).c_str());
    //
    //    } else {
    //        rval = Eval(ctx, term->Right(), local_vars, is_eval_block, CatchType::CATCH_AUTO);
    //    }
    //    if (!rval) {
    //        NL_PARSER(term->Right(), "Object is missing or expression is not evaluated!");
    //    }
    //
    //    ASSERT(list_obj.size() == list_term.size());
    //
    //    if (term->Right()->getTermID() == TermID::ELLIPSIS) {
    //        if (rval->is_dictionary_type() || rval->is_tensor_type()) {
    //            if (!rval->empty() && rval->is_scalar()) {
    //                LOG_RUNTIME("Fail expand scalar!");
    //            }
    //            for (int i = 0; i < list_obj.size() - 1; i++) {
    //                if (list_term[i]->getTermID() != TermID::NONE) {
    //                    if (i < rval->size()) {
    //                        list_obj[i]->SetValue_((*rval)[i].second); //->Clone()
    //                    } else {
    //                        list_obj[i]->SetValue_(Obj::CreateNone());
    //                    }
    //                }
    //            }
    //            if (static_cast<int64_t> (list_obj.size()) - 1 < rval->size()) {
    //                // Удалить первые элементы
    //                rval->resize_(-(rval->size() - (static_cast<int64_t> (list_obj.size()) - 1)), nullptr);
    //            } else {
    //                rval->resize_(0, nullptr);
    //            }
    //            list_obj[list_obj.size() - 1]->SetValue_(rval->Clone());
    //
    //            result = list_obj[list_obj.size() - 1];
    //
    //        } else {
    //            LOG_RUNTIME("Fail expand type '%s'!", toString(rval->getType()));
    //        }
    //    } else {
    //        // Присвоеить единственное значение всем элементам с левой стороны оператора присовения
    //
    //        for (int i = 0; i < list_obj.size(); i++) {
    //            if (isTypeName(list_term[i]->m_text)) {
    //
    //                // Новый тип
    //                //                if (ctx->m_runtime->m_types.find(list_term[i]->m_text) != ctx->m_runtime->m_types.end()) {
    //                //                    LOG_RUNTIME("Type name '%s' already exists!", list_term[i]->m_text.c_str());
    //                //                }
    //                //
    //                //                result = rval->Clone();
    //                //                result->m_class_name = list_term[i]->m_text;
    //                //                result->m_class_parents.push_back(rval);
    //                //
    //                //                ctx->m_runtime->m_types[list_term[i]->m_text] = result;
    //
    //                if (ctx->m_runtime->NameFind(list_term[i]->m_text.c_str()) || ctx->m_runtime->NameFind(&list_term[i]->m_text[1])) {
    //                    LOG_RUNTIME("Type name '%s' already exists!", list_term[i]->m_text.c_str());
    //                }
    //
    //                result = rval->Clone();
    //
    //
    //                TermPtr term = *const_cast<TermPtr *> (&result->m_prototype);
    //                term->m_text = list_term[i]->m_text;
    //
    //                result->m_class_name = list_term[i]->m_text;
    //                result->m_class_parents.push_back(rval);
    //
    //                ctx->m_runtime->m_types[list_term[i]->m_text] = result;
    //
    //                //                if (!ctx->m_runtime->NameRegister(result->m_prototype->m_text.c_str(), result->m_prototype, result)) {
    //                LOG_RUNTIME("Fail register prototype '%s'!", result->m_prototype->toString().c_str());
    //                //                }
    //
    //
    //            } else if (list_term[i]->getTermID() == TermID::NONE) {
    //                // Skip
    //            } else {
    //                if (list_term[i]->Right()) {
    //                    ASSERT(list_term[i]->Right()->getTermID() == TermID::INDEX);
    //                    list_obj[i]->index_set_(MakeIndex(ctx, list_term[i]->Right(), local_vars), rval);
    //                } else {
    //                    list_obj[i]->SetValue_(rval);
    //                }
    //                if (list_obj[i]->m_var_type_current == ObjType::Function && rval->is_block()) {
    //                    list_obj[i]->m_var_type_current = ObjType::EVAL_FUNCTION;
    //                }
    //                result = list_obj[i];
    //            }
    //        }
    //    }
    //    return result;
}

ObjPtr Context::SetIndexValue(TermPtr &term, ObjPtr &value, Context * runner) {

    ASSERT(term->m_right);

    TermPtr index = term->m_right;
    ASSERT(index->size());

    switch (term->m_id) {
        case TermID::DICT:
        case TermID::STRCHAR:
        case TermID::STRWIDE:
            if (index->size() > 1) {
                NL_PARSER(index, "Strings and dictionaries do not support multiple dimensions!");
            }
        case TermID::TENSOR:
            break;

        case TermID::NAME:
        case TermID::LOCAL:
        case TermID::TYPE:
        case TermID::ARGS:
        case TermID::ARGUMENT:
        case TermID::STATIC:
            break;

        default:
            NL_PARSER(term, "Term type '%s' not indexable!", toString(term->m_id));
    }

    ObjPtr obj = CheckObjTerm_(term, runner, false);
    ASSERT(obj);
    ObjPtr ind = CheckObjTerm_(index->at(0).second, runner);
    ASSERT(ind);

    if (isStringChar(obj->getType())) {

        if (canCast(value->getType(), ObjType::Int8)) {
            obj->index_set_({ind->GetValueAsInteger()}, value);
        } else {
            NL_PARSER(term, "Fail cast from '%s' to ':Int8'", toString(value->getType()));
        }
        return value;

    } else if (isStringWide(obj->getType())) {

        if (canCast(value->getType(), RunTime::m_wide_char_type)) {
            obj->index_set_({ind->GetValueAsInteger()}, value);
        } else {
            NL_PARSER(term, "Fail cast from '%s' to '%s'", toString(value->getType()), newlang::toString(RunTime::m_wide_char_type));
        }
        return value;
    }
    NL_PARSER(term, "Index type '%s' not implemented!", toString(obj->getType()));
}

ObjPtr Context::GetIndexValue(TermPtr &term, ObjPtr &obj, Context * runner) {
    ASSERT(term->m_right);

    TermPtr index = term->m_right;
    ASSERT(index->size());

    switch (term->m_id) {
        case TermID::DICT:
        case TermID::STRCHAR:
        case TermID::STRWIDE:
            if (index->size() > 1) {
                NL_PARSER(index, "Strings and dictionaries do not support multiple dimensions!");
            }
        case TermID::TENSOR:
            break;

        case TermID::NAME:
        case TermID::LOCAL:
        case TermID::TYPE:
        case TermID::ARGS:
        case TermID::ARGUMENT:
        case TermID::STATIC:
            break;

        default:
            NL_PARSER(term, "Term type '%s' not indexable!", toString(term->m_id));
    }

    ObjPtr ind = CheckObjTerm_(index->at(0).second, runner);
    ASSERT(ind);

    if (isStringChar(obj->getType())) {
        return obj->index_get({ind->GetValueAsInteger()});
    }

    NL_PARSER(term, "Index type '%s' not implemented!", toString(obj->getType()));
}

ObjPtr Context::GetFieldValue(TermPtr &term, ObjPtr &value, Context * runner) {
    NL_PARSER(term, "GetFieldValue not implemented!");
}

ObjPtr Context::SetFieldValue(TermPtr &term, ObjPtr &value, Context * runner) {
    NL_PARSER(term, "SetFieldValue not implemented!");
}

/*
 * 
 * 
 * 
 */

ObjPtr Context::EvalOpLogical_(TermPtr & op, Context * runner) {
    ASSERT(op);
    if (op->m_id == TermID::OP_MATH || op->m_id == TermID::OP_LOGICAL) {
        //        if(op->m_text.compare(==))
    }

    NL_PARSER(op, "EvalOpOther '%s' not implemented!", op->m_text.c_str());
}

ObjPtr Context::EvalOpMath_(TermPtr & op, Context * runner) {
    ASSERT(op);
    ASSERT(op->m_id == TermID::OP_MATH);
    ASSERT(op->m_left);
    ASSERT(op->m_right);
    CheckObjTerm_(op->m_left, runner);
    CheckObjTerm_(op->m_right, runner);

    ObjPtr result = op->m_left->m_obj;
    if (op->m_text.rfind("=") == op->m_text.size() - 2) {
        result = result->Clone();
    }

    if (op->m_text.find("+") == 0) {
        (*result) += op->m_right->m_obj;
    } else if (op->m_text.find("-") == 0) {
        (*result) -= op->m_right->m_obj;
    } else if (op->m_text.find("*") == 0) {
        (*result) *= op->m_right->m_obj;
    } else if (op->m_text.find("//") == 0) {
        result->op_div_ceil_(op->m_right->m_obj);
    } else if (op->m_text.find("/") == 0) {
        (*result) /= op->m_right->m_obj;
    } else if (op->m_text.find("%") == 0) {
        (*result) %= op->m_right->m_obj;
    } else {

        NL_PARSER(op, "Math operator '%s' not implemented!", op->m_text.c_str());
    }
    return result;
}

ObjPtr Context::EvalOpCompare_(TermPtr & op, Context * runner) {
    ASSERT(op);
    ASSERT(op->m_id == TermID::OP_COMPARE);
    ASSERT(op->m_left);
    ASSERT(op->m_right);
    CheckObjTerm_(op->m_left, runner);
    CheckObjTerm_(op->m_right, runner);

    ObjPtr result;

    if (op->m_text.compare("==") == 0) {
        result = Obj::CreateBool(op->m_left->m_obj->op_equal(op->m_right->m_obj));
    } else if (op->m_text.compare("!=") == 0) {
        result = Obj::CreateBool(!op->m_left->m_obj->op_equal(op->m_right->m_obj));
    } else if (op->m_text.compare("<") == 0) {
        result = Obj::CreateBool((*op->m_left->m_obj) < op->m_right->m_obj);
    } else if (op->m_text.compare("<=") == 0) {
        result = Obj::CreateBool((*op->m_left->m_obj) <= op->m_right->m_obj);
    } else if (op->m_text.compare(">") == 0) {
        result = Obj::CreateBool((*op->m_left->m_obj) > op->m_right->m_obj);
    } else if (op->m_text.compare(">=") == 0) {
        result = Obj::CreateBool((*op->m_left->m_obj) >= op->m_right->m_obj);
    } else if (op->m_text.compare("===") == 0) {
        result = Obj::CreateBool(op->m_left->m_obj->op_accurate(op->m_right->m_obj));
    } else if (op->m_text.compare("!==") == 0) {
        result = Obj::CreateBool(!op->m_left->m_obj->op_accurate(op->m_right->m_obj));
    } else {

        NL_PARSER(op, "Compare operator '%s' not implemented!", op->m_text.c_str());
    }
    return result;
}

ObjPtr Context::EvalWhile_(TermPtr & term, Context * runner) {
    ASSERT(term->Left());
    ASSERT(term->Right());

    ObjPtr result = Obj::CreateNone();
    ObjPtr cond = Run(term->Left(), runner);
    if (!cond->GetValueAsBoolean() && !term->m_follow.empty()) {
        // else
        ASSERT(term->m_follow.size() == 1);
        ASSERT(term->m_follow[0]->Left()->m_id == TermID::ELLIPSIS);
        result = Run(term->m_follow[0]->Right(), runner);
    } else {
        while (cond->GetValueAsBoolean()) {

            result = Run(term->Right(), runner);

            //            if (result->GetType() == ObjType::RetPlus) {
            //                if (result->) {
            //                }
            //                break;
            //            } else if (is_interrupt || result->op_class_test(Return::Continue, ctx)) {
            //                continue;
            //            }

            cond = Run(term->Left(), runner);
        }
    }
    return result;
}

ObjPtr Context::EvalDoWhile_(TermPtr & term, Context * runner) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    ObjPtr result;
    ObjPtr cond;
    do {

        result = Run(term->Left(), runner);
        cond = Run(term->Right(), runner);

        //        if (is_interrupt && cond->op_class_test(Return::Break, ctx)) {
        //            break;
        //        } else if (is_interrupt && cond->op_class_test(Return::Continue, ctx)) {
        //            continue;
        //        }

    } while (cond->GetValueAsBoolean());

    return result;
}

ObjPtr Context::EvalFollow_(TermPtr & term, Context * runner) {
    /*
     * [cond] --> {expr};
     * [cond] --> {expr}, [...] --> {else};
     * 
     */

    for (int64_t i = 0; i < static_cast<int64_t> (term->m_follow.size()); i++) {

        ASSERT(term->m_follow[i]);
        ASSERT(term->m_follow[i]->Left());

        bool condition = false;
        if (term->m_follow[i]->Left()->getTermID() == TermID::ELLIPSIS) {
            ASSERT(i + 1 == term->m_follow.size());
            condition = true;
        } else {
            ObjPtr cond = Run(term->m_follow[i]->Left(), runner);
            condition = cond->GetValueAsBoolean();
        }

        if (condition) {

            return Run(term->m_follow[i]->Right(), runner);
        }
    }
    return Obj::CreateNone();
}

ObjPtr Context::EvalEval_(TermPtr & term) {

    if (!m_runtime->m_eval_enable) {
        NL_PARSER(term, "The eval operator cannot be used! Use flag: --nlc-eval-enable");
    }

    /*
     * subprocess.run(args, *, stdin=None, input=None, stdout=None, stderr=None, 
     * capture_output=False, shell=False, cwd=None, timeout=None, check=False, 
     * encoding=None, errors=None, text=None, env=None, universal_newlines=None, **other_popen_kwargs)            
     */
    /* class subprocess.Popen(args, bufsize = -1, executable = None, 
     * stdin = None, stdout = None, stderr = None, 
     * preexec_fn = None, close_fds = True, shell = False, cwd = None, 
     * env = None, universal_newlines = None, startupinfo = None, 
     * creationflags = 0, restore_signals = True, start_new_session = False, 
     * pass_fds = (), *, group = None, extra_groups = None, user = None, umask = -1
     * , encoding = None, errors = None, text = None, 
     * pipesize = -1, process_group = None)¶
     */


    // we use std::array rather than a C-style array to get all the
    // C++ array convenience functions
    std::array<char, 128> buffer;
    std::string result;

    // popen() receives the command and parameter "r" for read,
    // since we want to read from a stream.
    // by using unique_ptr, the pipe object is automatically cleaned
    // from memory once we've read all the data from the pipe.
    std::unique_ptr<FILE, decltype(&pclose) > pipe(popen(term->getText().c_str(), "r"), pclose);

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return Obj::CreateString(result);
}

ObjPtr Context::EvalIterator_(TermPtr & op) {
    ASSERT(op);
    ASSERT(op->m_left);
    if (op->m_text.compare("?!") == 0 || op->m_text.compare("!?") == 0) {
        if (isReservedName(op->m_left->m_text)) {
            ObjPtr result;
            if (op->m_left->m_text.compare("$") == 0) {

                result = Obj::CreateDict();
                for (auto &elem : getStorage_()) {
                    ASSERT(elem.second);
                    result->push_back(Obj::CreateString(elem.second->m_text));
                }
                return result;

            } else if (op->m_left->m_text.compare("@") == 0) {

                result = Obj::CreateDict();
                if (m_runtime && m_runtime->m_macro) {
                    for (auto &elem : *(m_runtime->m_macro)) {
                        result->push_back(Obj::CreateString(elem.first));
                    }
                }
                return result;

            } else if (op->m_left->m_text.compare("$^") == 0) {
                return m_latter;
            } else if (op->m_left->m_text.compare("@::") == 0) {

                result = Obj::CreateDict();
                for (auto &elem : m_static) {
                    result->push_back(Obj::CreateString(elem.first));
                }
                return result;

            } else if (op->m_left->m_text.compare("%") == 0) {
            } else if (op->m_left->m_text.compare("$$") == 0) {
            } else if (op->m_left->m_text.compare("@$") == 0) {
            }
        }
        NL_PARSER(op, "Eval iterator for reserved word '%s' not implemented!", op->m_left->m_text.c_str());
    } else {

        NL_PARSER(op, "Eval iterator '%s' not implemented!", op->m_text.c_str());
    }
    NL_PARSER(op, "Eval iterator not implemented!");
}

ObjPtr Context::EvalOpBitwise_(TermPtr & op, Context * runner) {

    ASSERT(op);
    NL_PARSER(op, "Bitwise operator '%s' not implemented!", op->m_text.c_str());
}

//ObjPtr Context::EvalOp_(TermPtr & op) {
//    ASSERT(op);
//    ObjPtr result;
//    //    switch (obj->getTermID()) {
//    //        case TermID::INT_PLUS:
//    //            result = Obj::CreateString(obj->m_namespace ? obj->m_namespace->m_text : "");
//    //            result->m_var_type = ObjType::RetPlus;
//    //            break;
//    //        case TermID::INT_MINUS:
//    //            result = Obj::CreateString(obj->m_namespace ? obj->m_namespace->m_text : "");
//    //            result->m_var_type = ObjType::RetMinus;
//    //            break;
//    //        case TermID::INT_REPEAT:
//    //            result = Obj::CreateString(obj->m_namespace ? obj->m_namespace->m_text : "");
//    //            result->m_var_type = ObjType::RetRepeat;
//    //            break;
//    //        default:
//    //            NL_PARSER(term, "EvalOp for type '%s' not implemented!", toString(obj->getTermID()));
//    //    }
//    return result;
//}


//VERIFY(CreateMacro("@@ exit(code) @@ ::= @@ :: ++ @$code ++ @@"));
//VERIFY(CreateMacro("@@ abort() @@ ::= @@ :: -- @@"));

ObjPtr Context::EvalInterrupt_(TermPtr & term) {
    ASSERT(term);
    ObjType type;
    switch (term->m_id) {
        case TermID::INT_PLUS:
            type = ObjType::RetPlus;
            break;
        case TermID::INT_MINUS:
            type = ObjType::RetMinus;
            break;
        case TermID::INT_REPEAT:
            type = ObjType::RetRepeat;
            break;
        default:
            NL_PARSER(term, "Term '%s' is not interrupt!", toString(term->m_id));
    }
    ObjPtr result = Obj::CreateType(type, type, true);
    if (term->Right()) {
        result->m_return_obj = EvalTerm(term->Right(), this);
    } else {
        result->m_return_obj = Obj::CreateNone();
    }
    if (!term->m_namespace || term->m_namespace->m_text.compare("::") == 0) {
        switch (type) {
            case ObjType::RetPlus:
                throw IntPlus(result);
            case ObjType::RetMinus:
                throw IntMinus(result);
            case ObjType::RetRepeat:
                NL_PARSER(term, "Type RetRepeat not implemented!");
            default:
                NL_PARSER(term, "Type '%s' is not interrupt!", toString(type));
        }
    }
    result->m_value = term->m_namespace->m_text;
    return result;
}

