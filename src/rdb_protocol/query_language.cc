#include "rdb_protocol/query_language.hpp"

#include "errors.hpp"
#include <boost/make_shared.hpp>
#include <math.h>

#include "http/json.hpp"

#define CHECK_WELL_DEFINED(x) if (!is_well_defined(x)) { return false; }

bool is_well_defined(const VarTermTuple &v) {
    return is_well_defined(v.term());
}

bool is_well_defined(const Term &t) {
    rassert(t.has_type());
    rassert(Term_TermType_IsValid(t.type()));

    if (t.has_var()) {
        if (t.type() != Term::VAR) {
            return false;
        } else {
            //no other way for this to fail
        }
    }

    if (t.has_let()) {
        if (t.type() != Term::LET) {
            return false;
        } else {
            for (int i = 0; i < t.let().binds_size(); ++i) {
                CHECK_WELL_DEFINED(t.let().binds(i));
            }
        }
        CHECK_WELL_DEFINED(t.let().expr());
    }

    if (t.has_call()) {
        if (t.type() != Term::CALL) {
            return false;
        } else {
            CHECK_WELL_DEFINED(t.call().builtin());
            for (int i = 0; i < t.call().args_size(); ++i) {
                CHECK_WELL_DEFINED(t.call().args(i));
            }
        }
    }

    if (t.has_if_()) {
        if (t.type() != Term::IF) {
            return false;
        } else {
            CHECK_WELL_DEFINED(t.if_().test());
            CHECK_WELL_DEFINED(t.if_().true_branch());
            CHECK_WELL_DEFINED(t.if_().false_branch());
        }
    }

    if (t.has_try_()) {
        if (t.type() != Term::TRY) {
            return false;
        } else {
            CHECK_WELL_DEFINED(t.try_().try_term());
            CHECK_WELL_DEFINED(t.try_().var_and_catch_term().term());
        }
    }

    if (t.has_error() && t.type() != Term::ERROR) {
        return false;
    }

    if (t.has_number() && t.type() != Term::NUMBER) {
        return false;
    }

    if (t.has_valuestring() && t.type() != Term::STRING) {
        return false;
    }

    if (t.has_valuebool() && t.type() != Term::BOOL) {
        return false;
    }

    if (t.array_size() > 0 && t.type() != Term::ARRAY) {
        return false;
    }

    if (t.map_size() > 0 && t.type() != Term::MAP) {
        return false;
    }

    if (t.has_view_as_stream() && t.type() != Term::VIEWASSTREAM) {
        return false;
    }

    if (t.has_get_by_key()) {
        if (t.type() != Term::GETBYKEY) {
            return false;
        } else {
            CHECK_WELL_DEFINED(t.get_by_key().key());
        }
    }

    return true;
}

bool is_well_defined(const Builtin &b) {
    rassert(b.has_type());
    rassert(Builtin_BuiltinType_IsValid(b.type()));

    if (b.has_attr() && b.type() != Builtin::GETATTR && b.type() != Builtin::HASATTR) {
        return false;
    }

    if (b.attrs_size() > 0 && b.type() != Builtin::PICKATTRS) {
        return false;
    }

    if (b.has_filter()) {
        if (b.type() != Builtin::FILTER) {
            return false;
        } else {
            CHECK_WELL_DEFINED(b.filter().predicate());
        }
    }

    if (b.has_map()) {
        if (b.type() != Builtin::MAP) {
            return false;
        } else {
            CHECK_WELL_DEFINED(b.map().mapping());
        }
    }

    if (b.has_concat_map()) {
        if (b.type() != Builtin::CONCATMAP) {
            return false;
        } else {
            CHECK_WELL_DEFINED(b.concat_map().mapping());
        }
    }

    if (b.has_order_by()) {
        if (b.type() != Builtin::ORDERBY) {
            return false;
        } else {
            CHECK_WELL_DEFINED(b.order_by().mapping());
        }
    }

    if (b.has_distinct()) {
        if (b.type() != Builtin::DISTINCT) {
            return false;
        } else {
            CHECK_WELL_DEFINED(b.distinct().mapping());
        }
    }

    if (b.has_limit() && b.type() != Builtin::LIMIT) {
        return false;
    }

    if (b.has_reduce()) {
        if (b.type() != Builtin::REDUCE) {
            return false;
        } else {
            CHECK_WELL_DEFINED(b.reduce().reduction());
        }
    }

    if (b.has_grouped_map_reduce()) {
        if (b.type() != Builtin::GROUPEDMAPREDUCE) {
            return false;
        } else {
            CHECK_WELL_DEFINED(b.grouped_map_reduce().group_mapping());
            CHECK_WELL_DEFINED(b.grouped_map_reduce().map_reduce().change_mapping());
            CHECK_WELL_DEFINED(b.grouped_map_reduce().map_reduce().reduction());
        }
    }

    if (b.has_map_reduce()) {
        if (b.type() != Builtin::MAPREDUCE) {
            return false;
        } else {
            CHECK_WELL_DEFINED(b.map_reduce().change_mapping());
            CHECK_WELL_DEFINED(b.map_reduce().reduction());
        }
    }

    return true;
}

bool is_well_defined(const Reduction &r) {
    CHECK_WELL_DEFINED(r.base())
    CHECK_WELL_DEFINED(r.body())
    return true;
}

bool is_well_defined(const Mapping &m) {
    CHECK_WELL_DEFINED(m.body());
    return true;
}

bool is_well_defined(const Predicate &p) {
    CHECK_WELL_DEFINED(p.body());
    return true;
}

bool is_well_defined(const View &v) {
    if (v.has_table() && v.type() != View::TABLE) {
        return false;
    }

    if (v.has_filter_view()) {
        if (v.type() != View::FILTERVIEW) {
            return false;
        } else {
            CHECK_WELL_DEFINED(v.filter_view().view());
            CHECK_WELL_DEFINED(v.filter_view().predicate());
        }
    }

    if (v.has_range_view()) {
        if (v.type() != View::RANGEVIEW) {
            return false;
        } else {
            CHECK_WELL_DEFINED(v.range_view().view());
            CHECK_WELL_DEFINED(v.range_view().lowerbound());
            CHECK_WELL_DEFINED(v.range_view().upperbound());
        }
    }

    return true;
}

bool is_well_defined(const ReadQuery &r) {
    return is_well_defined(r.term());
}

bool is_well_defined(const WriteQuery &w) {
    rassert(w.has_type());
    rassert(WriteQuery_WriteQueryType_IsValid(w.type()));

    if (w.has_update()) {
        if (w.type() != WriteQuery::UPDATE) {
            return false;
        } else {
            CHECK_WELL_DEFINED(w.update().view());
            CHECK_WELL_DEFINED(w.update().mapping());
        }
    }

    if (w.has_delete_()) {
        if (w.type() != WriteQuery::DELETE) {
            return false;
        } else {
            CHECK_WELL_DEFINED(w.delete_().view());
        }
    }

    if (w.has_mutate()) {
        if (w.type() != WriteQuery::MUTATE) {
            return false;
        } else {
            CHECK_WELL_DEFINED(w.mutate().view());
            CHECK_WELL_DEFINED(w.mutate().mapping());
        }
    }

    if (w.has_insert()) {
        if (w.type() != WriteQuery::INSERT) {
            return false;
        } else {
            for (int i = 0; i < w.insert().terms_size(); ++i) {
                CHECK_WELL_DEFINED(w.insert().terms(i));
            }
        }
    }

    if (w.has_insert_stream()) {
        if (w.type() != WriteQuery::INSERTSTREAM) {
            return false;
        } else {
            CHECK_WELL_DEFINED(w.insert_stream().stream());
        }
    }

    if (w.has_for_each()) {
        if (w.type() != WriteQuery::FOREACH) {
            return false;
        } else {
            CHECK_WELL_DEFINED(w.for_each().stream());
            for (int i = 0; i < w.for_each().queries_size(); ++i) {
                CHECK_WELL_DEFINED(w.for_each().queries(i));
            }
        }
    }

    if (w.has_point_update()) {
        if (w.type() != WriteQuery::POINTUPDATE) {
            return false;
        } else {
            CHECK_WELL_DEFINED(w.point_update().key());
            CHECK_WELL_DEFINED(w.point_update().mapping());
        }
    }

    if (w.has_point_delete()) {
        if (w.type() != WriteQuery::POINTDELETE) {
            return false;
        } else {
            CHECK_WELL_DEFINED(w.point_delete().key());
        }
    }

    if (w.has_point_mutate()) {
        if (w.type() != WriteQuery::POINTMUTATE) {
            return false;
        } else {
            CHECK_WELL_DEFINED(w.point_mutate().key());
        }
    }

    return true;
}

bool is_well_defined(const Query &q) {
    if (q.has_read_query()) {
        if (q.type() != Query::READ) {
            return false;
        } else {
            CHECK_WELL_DEFINED(q.read_query());
        }
    }

    if (q.has_write_query()) {
        if (q.type() != Query::WRITE) {
            return false;
        } else {
            CHECK_WELL_DEFINED(q.write_query());
        }
    }

    return true;
}

namespace query_language {
const type_t get_type(const Term &t, variable_type_scope_t *scope) {
    switch (t.type()) {
        case Term::VAR:
            if (scope->is_in_scope(t.var())) {
                return scope->get(t.var());
            } else {
                return error_t(strprintf("Symbol %s is not in scope\n", t.var().c_str()));
            }
            break;
        case Term::LET:
            {
                scope->push(); //create a new scope
                for (int i = 0; i < t.let().binds_size(); ++i) {
                    scope->put_in_scope(t.let().binds(i).var(), get_type(t.let().binds(i).term(), scope));
                }
                type_t res = get_type(t.let().expr(), scope);
                scope->pop();
                return res;
                break;
            }
        case Term::CALL:
            {
                function_t signature = get_type(t.call().builtin(), scope);
                if (!signature.is_variadic()) {
                    int n_args = signature.get_n_args();
                    if (t.call().args_size() > n_args) {
                        return error_t(strprintf("Too many arguments passed to function. Expected %d but got %d", n_args, t.call().args_size())); //TODO would be nice to have function names attached to errors
                    } else if (t.call().args_size() < n_args) {
                        return error_t(strprintf("Too few  arguments passed to function. Expected %d but got %d", n_args, t.call().args_size())); //TODO would be nice to have function names attached to errors
                    }
                }
                for (int i = 0; i < t.call().args_size(); ++i) {
                    if (!(get_type(t.call().args(i), scope) == signature.get_arg_type())) {
                        return error_t("Type mismatch in function call\n"); //Need descriptions of types to give a more informative message
                    }
                }
                return signature.get_return_type();
                break;
            }
        case Term::IF:
            {
                if (!(get_type(t.if_().test(), scope) == Type::JSON)) {
                    return error_t("Test in an if must be JSON\n");
                }
                type_t true_branch  = get_type(t.if_().true_branch(), scope),
                       false_branch = get_type(t.if_().false_branch(), scope);
                if (!(true_branch == false_branch)) {
                    return error_t("Mismatch between true and false branch types.");
                } else {
                    return true_branch;
                }
                break;
            }
        case Term::TRY:
            {
                type_t try_type = get_type(t.try_().try_term(), scope);
                scope->push();
                scope->put_in_scope(t.try_().var_and_catch_term().var(), Type::ERROR);
                type_t catch_type = get_type(t.try_().var_and_catch_term().term(), scope);
                scope->pop();
                if (!(try_type == catch_type)) {
                    return error_t("Mismatch between try and catch branch types.");
                } else {
                    return try_type;
                }
                break;
            }
        case Term::ERROR:
            return Type::ERROR;
        case Term::NUMBER:
        case Term::STRING:
        case Term::JSON:
        case Term::BOOL:
        case Term::JSON_NULL:
        case Term::ARRAY:
        case Term::MAP:
            return Type::JSON;
        case Term::VIEWASSTREAM:
            if (get_type(t.view_as_stream(), scope) == Type::VIEW) {
                return Type::STREAM;
            } else {
                return type_t(error_t());
            }
            break;
        case Term::GETBYKEY:
            if (get_type(t.get_by_key().key(), scope) == Type::JSON) {
                return Type::JSON;
            } else {
                return error_t("Key must be a json value.");
            }
            break;
        default:
            crash("unreachable");
            break;
    }
    crash("unreachable");
}

function_t::function_t(const type_t& _arg_type, int _n_args, const type_t& _return_type)
    : arg_type(_arg_type), n_args(_n_args), return_type(_return_type) { }

const type_t& function_t::get_arg_type() const {
    return arg_type;
}

const type_t& function_t::get_return_type() const {
    return return_type;
}

bool function_t::is_variadic() const {
    return n_args == -1;
}

int function_t::get_n_args() const {
    return n_args;
}

const function_t get_type(const Builtin &b, variable_type_scope_t *) {
    switch (b.type()) {
        //JSON -> JSON
        case Builtin::NOT:
        case Builtin::GETATTR:
        case Builtin::HASATTR:
        case Builtin::PICKATTRS:
        case Builtin::ARRAYLENGTH:
        case Builtin::JAVASCRIPT:
            return function_t(Type::JSON, 1, Type::JSON);
            break;
        case Builtin::MAPMERGE:
        case Builtin::ARRAYAPPEND:
        case Builtin::ARRAYCONCAT:
        case Builtin::ARRAYNTH:
        case Builtin::MODULO:
            return function_t(Type::JSON, 2, Type::JSON);
            break;
        case Builtin::ADD:
        case Builtin::SUBTRACT:
        case Builtin::MULTIPLY:
        case Builtin::DIVIDE:
        case Builtin::COMPARE:
        case Builtin::ANY:
        case Builtin::ALL:
            return function_t(Type::JSON, -1, Type::JSON);  // variadic JSON type
            break;
        case Builtin::ARRAYSLICE:
            return function_t(Type::JSON, 3, Type::JSON);
            break;
        case Builtin::FILTER:
        case Builtin::MAP:
        case Builtin::CONCATMAP:
        case Builtin::ORDERBY:
        case Builtin::DISTINCT:
        case Builtin::LIMIT:
            return function_t(Type::STREAM, 1, Type::STREAM);
            break;
        case Builtin::LENGTH:
        case Builtin::NTH:
        case Builtin::STREAMTOARRAY:
        case Builtin::REDUCE:
        case Builtin::GROUPEDMAPREDUCE:
        case Builtin::MAPREDUCE:
            return function_t(Type::STREAM, 1, Type::JSON);
            break;
        case Builtin::UNION:
            return function_t(Type::STREAM, 2, Type::STREAM);
            break;
        case Builtin::ARRAYTOSTREAM:
        case Builtin::JAVASCRIPTRETURNINGSTREAM:
            return function_t(Type::JSON, 1, Type::STREAM);
            break;
        default:
            crash("unreachable");
            break;
    }
}

const type_t get_type(const Reduction &r, variable_type_scope_t *scope) {
    if (!(get_type(r.base(), scope) == Type::JSON)) {
        return type_t(error_t());
    }

    new_scope_t scope_maker(scope);
    scope->put_in_scope(r.var1(), Type::JSON);
    scope->put_in_scope(r.var2(), Type::JSON);

    if (!(get_type(r.body(), scope) == Type::JSON)) {
        return type_t(error_t());
    } else {
        return Type::JSON;
    }
}

const type_t get_type(const Mapping &m, variable_type_scope_t *scope) {
    new_scope_t scope_maker(scope);
    scope->put_in_scope(m.arg(), Type::JSON);

    if (!(get_type(m.body(), scope) == Type::JSON)) {
        return type_t(error_t());
    } else {
        return Type::JSON;
    }
}

const type_t get_type(const Predicate &p, variable_type_scope_t *scope) {
    new_scope_t scope_maker(scope);
    scope->put_in_scope(p.arg(), Type::JSON);

    if (!(get_type(p.body(), scope) == Type::JSON)) {
        return type_t(error_t());
    } else {
        return Type::JSON;
    }
}

const type_t get_type(const View &v, variable_type_scope_t *scope) {
    switch(v.type()) {
        case View::TABLE:
            //no way for this to be incorrect
            return Type::VIEW;
            break;
        case View::FILTERVIEW:
            if (get_type(v.filter_view().view(), scope) == Type::VIEW &&
                get_type(v.filter_view().predicate(), scope) == Type::JSON) {
                return Type::VIEW;
            }
            break;
        case View::RANGEVIEW:
            if (get_type(v.range_view().view(), scope) == Type::VIEW &&
                get_type(v.range_view().lowerbound(), scope) == Type::JSON &&
                get_type(v.range_view().upperbound(), scope) == Type::JSON) {
                return Type::READ;
            }
            break;
        default:
            crash("Unreachable");
    }
    crash("Unreachable");
}

const type_t get_type(const ReadQuery &r, variable_type_scope_t *scope) {
    type_t res = get_type(r.term(), scope);
    if (res == Type::JSON ||
        res == Type::STREAM) {
        return Type::READ;
    } else {
        return error_t("ReadQueries must produce either JSON or a STREAM.");
    }
}

const type_t get_type(const WriteQuery &w, variable_type_scope_t *scope) {
    switch (w.type()) {
        case WriteQuery::UPDATE:
            if (get_type(w.update().view(), scope) == Type::VIEW &&
                get_type(w.update().mapping(), scope) == Type::JSON) {
                return Type::WRITE;
            }
            break;
        case WriteQuery::DELETE:
            if (get_type(w.update().view(), scope) == Type::VIEW) {
                return Type::WRITE;
            }
            break;
        case WriteQuery::MUTATE:
            if (get_type(w.mutate().view(), scope) == Type::VIEW &&
                get_type(w.mutate().mapping(), scope) == Type::JSON) {
                return Type::WRITE;
            }
            break;
        case WriteQuery::INSERT:
            for (int i = 0; i < w.insert().terms_size(); ++i) {
                if (!(get_type(w.insert().terms(i), scope) == Type::JSON)) {
                    return type_t(error_t("Trying to insert a non JSON term"));
                }
            }
            return Type::WRITE;
            break;
        case WriteQuery::INSERTSTREAM:
            if (!(get_type(w.insert_stream().stream(), scope) == Type::STREAM)) {
                return Type::WRITE;
            }
            break;
        case WriteQuery::FOREACH:
            {
                if (!(get_type(w.for_each().stream(), scope) == Type::STREAM)) {
                    return type_t(error_t("Must pass a stream in to a FOREACH query."));
                }

                new_scope_t scope_maker(scope);
                scope->put_in_scope(w.for_each().var(), Type::JSON);
                for (int i = 0; i < w.for_each().queries_size(); ++i) {
                    if (!(get_type(w.for_each().queries(i), scope) == Type::WRITE)) {
                        return type_t(error_t("Queries passed to a foreach must all be write queries\n"));
                    }
                }
                return Type::WRITE;
            }
            break;
        case WriteQuery::POINTUPDATE:
            if (!(get_type(w.point_update().key(), scope) == Type::JSON) ||
                !(get_type(w.point_update().mapping(), scope) == Type::JSON)) {
                return type_t(error_t("Key and mapping must both be of type JSON\n"));
            }
            return Type::WRITE;
            break;
        case WriteQuery::POINTDELETE:
            if (!(get_type(w.point_delete().key(), scope) == Type::JSON)) {
                return type_t(error_t("Key must be of type JSON\n"));
            }
            return Type::WRITE;
            break;
        case WriteQuery::POINTMUTATE:
            if (!(get_type(w.point_mutate().key(), scope) == Type::JSON) ||
                !(get_type(w.point_mutate().mapping(), scope) == Type::JSON)) {
                return type_t(error_t("Key and mapping must both be of type JSON\n"));
            }
            return Type::WRITE;
            break;
        default:
            break;
    }
    crash("Unreachable");
}

const type_t get_type(const Query &q, variable_type_scope_t *scope) {
    switch (q.type()) {
        case Query::READ:
            if (!(get_type(q.read_query(), scope) == Type::READ)) {
                return type_t(error_t("Malformed read."));
            }
            return Type::QUERY;
            break;
        case Query::WRITE:
            if (!(get_type(q.write_query(), scope) == Type::WRITE)) {
                return type_t(error_t("Malformed write."));
            }
            return Type::QUERY;
            break;
        default:
            crash("unreachable");
    }
    crash("unreachable");
}

Response eval(const Query &q, runtime_environment_t *env) {
    switch (q.type()) {
        case Query::READ:
            return eval(q.read_query(), env);
            break;
        case Query::WRITE:
            return eval(q.write_query(), env);
            break;
        default:
            crash("unreachable");
    }
    crash("unreachable");
}

Response eval(const ReadQuery &r, runtime_environment_t *env) THROWS_ONLY(runtime_exc_t) {
    Response res;
    boost::shared_ptr<scoped_cJSON_t> json = eval(r.term(), env);

    res.set_status_code(0);
    res.set_token(0);
    res.add_response(cJSON_print_std_string(json->get()));
    return res;
}

void insert(namespace_repo_t<rdb_protocol_t>::access_t ns_access, boost::shared_ptr<scoped_cJSON_t> data, runtime_environment_t *env) {
    if (!cJSON_GetObjectItem(data->get(), "id")) {
        throw runtime_exc_t("Must have a field named id.");
    }

    rdb_protocol_t::write_t write(rdb_protocol_t::point_write_t(store_key_t(cJSON_print_std_string(cJSON_GetObjectItem(data->get(), "id"))), data));
    ns_access.get_namespace_if()->write(write, order_token_t::ignore, &env->interruptor);
}

Response eval(const WriteQuery &w, runtime_environment_t *env) THROWS_ONLY(runtime_exc_t) {
    switch (w.type()) {
        case WriteQuery::UPDATE:
            break;
        case WriteQuery::DELETE:
            break;
        case WriteQuery::MUTATE:
            break;
        case WriteQuery::INSERT:
            {
                namespace_repo_t<rdb_protocol_t>::access_t ns_access = eval(w.insert().table_ref(), env);
                for (int i = 0; i < w.insert().terms_size(); ++i) {
                    boost::shared_ptr<scoped_cJSON_t> data = eval(w.insert().terms(i), env);

                    insert(ns_access, data, env);
                }
                Response res;
                res.set_status_code(0);
                res.set_token(0);
                res.add_response(strprintf("Inserted %d rows.", w.insert().terms_size()));
                return res;
            }
            break;
        case WriteQuery::INSERTSTREAM:
            break;
        case WriteQuery::FOREACH:
            break;
        case WriteQuery::POINTUPDATE:
            {
                //First we need to grab the value the easiest way to do this is to just construct a term and evaluate it.
                Term get;
                get.set_type(Term::GETBYKEY);
                Term::GetByKey get_by_key;
                *get_by_key.mutable_table_ref() = w.point_update().table_ref();
                get_by_key.set_attrname(w.point_update().attrname());
                *get_by_key.mutable_key() = w.point_update().key();
                *get.mutable_get_by_key() = get_by_key;

                rassert(is_well_defined(get));

                boost::shared_ptr<scoped_cJSON_t> original_val = eval(get, env);
                new_val_scope_t scope_maker(&env->scope);
                env->scope.put_in_scope(w.point_update().mapping().arg(), original_val);

                boost::shared_ptr<scoped_cJSON_t> new_val = eval(w.point_update().mapping().body(), env);

                /* Now we insert the new value. */
                namespace_repo_t<rdb_protocol_t>::access_t ns_access = eval(w.point_update().table_ref(), env);

                insert(ns_access, new_val, env);

                Response res;
                res.set_status_code(0);
                res.set_token(0);
                res.add_response("Updated 1 rows.");
                return res;
            }
            break;
        case WriteQuery::POINTDELETE:
            break;
        case WriteQuery::POINTMUTATE:
            break;
        default:
            unreachable();
            break;
    }

    Response res;
    res.set_status_code(-3);
    res.set_token(0);
    res.add_response("Unimplemented");
    return res;
}

boost::shared_ptr<scoped_cJSON_t> eval(const Term &t, runtime_environment_t *env) THROWS_ONLY(runtime_exc_t) {
    switch (t.type()) {
        case Term::VAR:
            return env->scope.get(t.var());
            break;
        case Term::LET:
            {
                // Push the scope
                variable_val_scope_t::new_scope_t new_scope(&env->scope);
                // Go through the bindings in a let and add them one by one
                for (int i = 0; i < t.let().binds_size(); ++i) {
                    // TODO: we should evaluate the binding term right
                    // here to avoid multiple reevaluations during
                    // expression evaluation.
                    env->scope.put_in_scope(t.let().binds(i).var(),
                                            eval(t.let().binds(i).term(), env));
                }
                return eval(t.let().expr(), env);;
            }
            break;
        case Term::CALL:
            return eval(t.call(), env);
            break;
        case Term::IF:
            {
                boost::shared_ptr<scoped_cJSON_t> test = eval(t.if_().test(), env);
                if (test->get()->type != cJSON_True && test->get()->type != cJSON_False) {
                    throw runtime_exc_t("The IF test must evaluate to a boolean.");
                }

                boost::shared_ptr<scoped_cJSON_t> res;
                if (test->get()->type == cJSON_True) {
                    res = eval(t.if_().true_branch(), env);
                } else {
                    res = eval(t.if_().false_branch(), env);
                }
                return res;
            }
            break;
        case Term::TRY:
            crash("unimplemented");
            break;
        case Term::ERROR:
            crash("unimplemented");
            break;
        case Term::NUMBER:
            {
                return boost::shared_ptr<scoped_cJSON_t>(new scoped_cJSON_t(cJSON_CreateNumber(t.number())));
            }
            break;
        case Term::STRING:
            {
                return boost::shared_ptr<scoped_cJSON_t>(new scoped_cJSON_t(cJSON_CreateString(t.valuestring().c_str())));
            }
            break;
        case Term::JSON:
            return boost::shared_ptr<scoped_cJSON_t>(new scoped_cJSON_t(cJSON_Parse(t.jsonstring().c_str())));
            break;
        case Term::BOOL:
            {
                return boost::shared_ptr<scoped_cJSON_t>(new scoped_cJSON_t(cJSON_CreateBool(t.valuebool())));
            }
            break;
        case Term::JSON_NULL:
            {
                return boost::shared_ptr<scoped_cJSON_t>(new scoped_cJSON_t(cJSON_CreateNull()));
            }
            break;
        case Term::ARRAY:
            {
                boost::shared_ptr<scoped_cJSON_t> res(new scoped_cJSON_t(cJSON_CreateArray()));
                for (int i = 0; i < t.array_size(); ++i) {
                    cJSON_AddItemToArray(res->get(), eval(t.array(i), env)->release());
                }
                return res;
            }
            break;
        case Term::MAP:
            {
                boost::shared_ptr<scoped_cJSON_t> res(new scoped_cJSON_t(cJSON_CreateObject()));
                for (int i = 0; i < t.map_size(); ++i) {
                    std::string item_name(t.map(i).var());
                    cJSON_AddItemToObject(res->get(), item_name.c_str(), eval(t.map(i).term(), env)->release());
                }
                return res;
            }
            break;
        case Term::VIEWASSTREAM:
            crash("unimplemented");
            break;
        case Term::GETBYKEY:
            {
                boost::optional<std::pair<namespace_id_t, deletable_t<namespace_semilattice_metadata_t<rdb_protocol_t> > > > namespace_info =
                    env->semilattice_metadata->get().rdb_namespaces.get_namespace_by_name(t.get_by_key().table_ref().table_name());

                if (!namespace_info) {
                    throw runtime_exc_t(strprintf("Namespace %s either not found, ambigious or namespace metadata in conflict.", t.get_by_key().table_ref().table_name().c_str()));
                }

                if (t.get_by_key().attrname() != "id") {
                    throw runtime_exc_t(strprintf("Attribute: %s is not the primary key and thus cannot be selected upon.", t.get_by_key().attrname().c_str()));
                }

                namespace_repo_t<rdb_protocol_t>::access_t ns_access = eval(t.get_by_key().table_ref(), env);

                boost::shared_ptr<scoped_cJSON_t> key = eval(t.get_by_key().key(), env);
                rdb_protocol_t::read_t read(rdb_protocol_t::point_read_t(store_key_t(cJSON_print_std_string(key->get()))));
                rdb_protocol_t::read_response_t res = ns_access.get_namespace_if()->read(read, order_token_t::ignore, &env->interruptor);

                rdb_protocol_t::point_read_response_t *p_res = boost::get<rdb_protocol_t::point_read_response_t>(&res.response);
                return p_res->data;
                break;
            }
        default:
            crash("unreachable");
            break;
    }
    crash("unreachable");
}

boost::shared_ptr<scoped_cJSON_t> eval(const Term::Call &c, runtime_environment_t *env) THROWS_ONLY(runtime_exc_t) {
    switch (c.builtin().type()) {
        //JSON -> JSON
        case Builtin::NOT:
            {
                boost::shared_ptr<scoped_cJSON_t> data = eval(c.args(0), env);

                if (data->get()->type == cJSON_False) {
                    data->get()->type = cJSON_True;
                } else if (data->get()->type == cJSON_True) {
                    data->get()->type = cJSON_False;
                } else {
                    throw runtime_exc_t("Not can only be called on a boolean");
                }
                return data;
            }
            break;
        case Builtin::GETATTR:
            {
                boost::shared_ptr<scoped_cJSON_t> data = eval(c.args(0), env);

                if (!data->get()->type == cJSON_Object) {
                    throw runtime_exc_t("Data must be an object");
                }

                boost::shared_ptr<scoped_cJSON_t> attr
                    = shared_scoped_json(cJSON_DeepCopy(cJSON_GetObjectItem(data->get(), c.builtin().attr().c_str())));

                if (!attr->get()) {
                    throw runtime_exc_t("Failed to find attribute");
                } else {
                    return attr;
                }
            }
            break;
        case Builtin::HASATTR:
            {
                boost::shared_ptr<scoped_cJSON_t> data = eval(c.args(0), env);

                if (!data->get()->type == cJSON_Object) {
                    throw runtime_exc_t("Data must be an object");
                }

                cJSON *attr = cJSON_GetObjectItem(data->get(), c.builtin().attr().c_str());

                if (attr) {
                    return shared_scoped_json(cJSON_CreateTrue());
                } else {
                    return shared_scoped_json(cJSON_CreateFalse());
                }
            }
            break;
        case Builtin::PICKATTRS:
            {
                boost::shared_ptr<scoped_cJSON_t> data = eval(c.args(0), env);

                if (!data->get()->type == cJSON_Object) {
                    throw runtime_exc_t("Data must be an object");
                }

                boost::shared_ptr<scoped_cJSON_t> res = shared_scoped_json(cJSON_CreateObject());

                for (int i = 0; i < c.builtin().attrs_size(); ++i) {
                    cJSON *item = cJSON_DeepCopy(cJSON_GetObjectItem(data->get(), c.builtin().attrs(i).c_str()));
                    if (!item) {
                        throw runtime_exc_t("Attempting to pick non existant attribute.");
                    } else {
                        cJSON_AddItemToObject(res->get(), item->string, item);
                    }
                }
                return res;
            }
            break;
        case Builtin::MAPMERGE:
            {
                boost::shared_ptr<scoped_cJSON_t> left  = eval(c.args(0), env),
                                                  right = eval(c.args(1), env);
                if (left->get()->type != cJSON_Object) {
                    throw runtime_exc_t("Data must be an object");
                }

                if (right->get()->type != cJSON_Object) {
                    throw runtime_exc_t("Data must be an object");
                }

                boost::shared_ptr<scoped_cJSON_t> res(new scoped_cJSON_t(cJSON_DeepCopy(left->get())));
                
                // Extend with the right side (and overwrite if necessary)
                for(int i = 0; i < cJSON_GetArraySize(right->get()); i++) {
                    cJSON *item = cJSON_GetArrayItem(right->get(), i);
                    cJSON_DeleteItemFromObject(res->get(), item->string);
                    cJSON_AddItemToObject(res->get(), item->string, cJSON_DeepCopy(item));
                }
                
                return res;
            }
            break;
        case Builtin::ARRAYAPPEND:
            {
                // Check first arg type
                boost::shared_ptr<scoped_cJSON_t> array  = eval(c.args(0), env);
                if (array->get()->type != cJSON_Array) {
                    throw runtime_exc_t("The first argument must be an array.");
                }
                boost::shared_ptr<scoped_cJSON_t> res(new scoped_cJSON_t(cJSON_DeepCopy(array->get())));
                cJSON_AddItemToArray(res->get(), eval(c.args(1), env)->release());
                return res;
            }
            break;
        case Builtin::ARRAYCONCAT:
            {
                // Check first arg type
                boost::shared_ptr<scoped_cJSON_t> array1  = eval(c.args(0), env);
                if (array1->get()->type != cJSON_Array) {
                    throw runtime_exc_t("The first argument must be an array.");
                }
                // Check second arg type
                boost::shared_ptr<scoped_cJSON_t> array2  = eval(c.args(1), env);
                if (array2->get()->type != cJSON_Array) {
                    throw runtime_exc_t("The first argument must be an array.");
                }
                // Create new array and deep copy all the elements
                boost::shared_ptr<scoped_cJSON_t> res(new scoped_cJSON_t(cJSON_CreateArray()));
                for(int i = 0; i < cJSON_GetArraySize(array1->get()); i++) {
                    cJSON_AddItemToArray(res->get(), cJSON_DeepCopy(cJSON_GetArrayItem(array1->get(), i)));
                }
                for(int j = 0; j < cJSON_GetArraySize(array2->get()); j++) {
                    cJSON_AddItemToArray(res->get(), cJSON_DeepCopy(cJSON_GetArrayItem(array2->get(), j)));
                }

                return res;
            }
            break;
        case Builtin::ARRAYSLICE:
            {
                // Check first arg type
                boost::shared_ptr<scoped_cJSON_t> array  = eval(c.args(0), env);
                if (array->get()->type != cJSON_Array) {
                    throw runtime_exc_t("The first argument must be an array.");
                }

                // Check second arg type
                boost::shared_ptr<scoped_cJSON_t> start  = eval(c.args(1), env);
                if (start->get()->type != cJSON_Number) {
                    throw runtime_exc_t("The second argument must be an integer.");
                }
                float float_start = start->get()->valuedouble;
                int int_start = (int)float_start;
                if (float_start != int_start) {
                    throw runtime_exc_t("The second argument must be an integer.");
                }
                if (int_start < 0) {
                    throw runtime_exc_t("The second argument cannot be smaller than zero.");
                }
                if (int_start > cJSON_GetArraySize(array->get())) {
                    throw runtime_exc_t("The second argument cannot be greater than the size of the array.");
                }

                // Check third arg type
                boost::shared_ptr<scoped_cJSON_t> end  = eval(c.args(2), env);
                if (end->get()->type != cJSON_Number) {
                    throw runtime_exc_t("The third argument must be an integer.");
                }
                float float_end = end->get()->valuedouble;
                int int_end = (int)float_end;
                if (float_end != int_end) {
                    throw runtime_exc_t("The third argument must be an integer.");
                }
                if (int_end < 0) {
                    throw runtime_exc_t("The third argument cannot be smaller than zero.");
                }
                if (int_end > cJSON_GetArraySize(array->get())) {
                    throw runtime_exc_t("The third argument cannot be greater than the size of the array.");
                }

                // Create a new array and slice the elements into it
                if (int_start > int_end) {
                    throw runtime_exc_t("The second argument cannot be greater than the third argument.");
                }
                boost::shared_ptr<scoped_cJSON_t> res(new scoped_cJSON_t(cJSON_CreateArray()));
                for(int i = int_start; i < int_end; i++) {
                    cJSON_AddItemToArray(res->get(), cJSON_DeepCopy(cJSON_GetArrayItem(array->get(), i)));
                }

                return res;
            }
            break;
        case Builtin::ARRAYNTH:
            {
                // Check first arg type
                boost::shared_ptr<scoped_cJSON_t> array  = eval(c.args(0), env);
                if (array->get()->type != cJSON_Array) {
                    throw runtime_exc_t("The first argument must be an array.");
                }

                // Check second arg type
                boost::shared_ptr<scoped_cJSON_t> index  = eval(c.args(1), env);
                if (index->get()->type != cJSON_Number) {
                    throw runtime_exc_t("The second argument must be an integer.");
                }
                float float_index = index->get()->valuedouble;
                int int_index = (int)float_index;
                if (float_index != int_index) {
                    throw runtime_exc_t("The second argument must be an integer.");
                }

                // Size arg
                if (int_index >= cJSON_GetArraySize(array->get())) {
                    throw runtime_exc_t("The second argument must be an integer.");
                }

                return boost::shared_ptr<scoped_cJSON_t>(new scoped_cJSON_t(cJSON_DeepCopy(cJSON_GetArrayItem(array->get(),
                                                                                                              int_index))));
            }
            break;
        case Builtin::ARRAYLENGTH:
            {
                // Check first arg type
                boost::shared_ptr<scoped_cJSON_t> array  = eval(c.args(0), env);
                if (array->get()->type != cJSON_Array) {
                    throw runtime_exc_t("The first argument must be an array.");
                }
                return boost::shared_ptr<scoped_cJSON_t>(new scoped_cJSON_t(cJSON_CreateNumber(cJSON_GetArraySize(array->get()))));
            }
            break;
        case Builtin::ADD:
            {
                double result = 0.0;

                for (int i = 0; i < c.args_size(); ++i) {
                    boost::shared_ptr<scoped_cJSON_t> arg = eval(c.args(i), env);
                    if (arg->get()->type != cJSON_Number) {
                        throw runtime_exc_t("All operands to ADD must be numbers.");
                    }
                    result += arg->get()->valuedouble;
                }

                boost::shared_ptr<scoped_cJSON_t> res(new scoped_cJSON_t(cJSON_CreateNumber(result)));
                return res;
            }
            break;
        case Builtin::SUBTRACT:
            {
                double result = 0.0;

                if (c.args_size() > 0) {
                    boost::shared_ptr<scoped_cJSON_t> arg = eval(c.args(0), env);
                    if (arg->get()->type != cJSON_Number) {
                        throw runtime_exc_t("All operands to SUBTRACT must be numbers.");
                    }
                    if (c.args_size() == 1) {
                        result = -arg->get()->valuedouble;  // (- x) is negate
                    } else {
                        result = arg->get()->valuedouble;
                    }

                    for (int i = 1; i < c.args_size(); ++i) {
                        boost::shared_ptr<scoped_cJSON_t> arg = eval(c.args(i), env);
                        if (arg->get()->type != cJSON_Number) {
                            throw runtime_exc_t("All operands to SUBTRACT must be numbers.");
                        }
                        result -= arg->get()->valuedouble;
                    }
                }

                boost::shared_ptr<scoped_cJSON_t> res(new scoped_cJSON_t(cJSON_CreateNumber(result)));
                return res;
            }
            break;
        case Builtin::MULTIPLY:
            {
                double result = 1.0;

                for (int i = 0; i < c.args_size(); ++i) {
                    boost::shared_ptr<scoped_cJSON_t> arg = eval(c.args(i), env);
                    if (arg->get()->type != cJSON_Number) {
                        throw runtime_exc_t("All operands of MULTIPLY must be numbers.");
                    }
                    result *= arg->get()->valuedouble;
                }

                boost::shared_ptr<scoped_cJSON_t> res(new scoped_cJSON_t(cJSON_CreateNumber(result)));
                return res;
            }
            break;
        case Builtin::DIVIDE:
            {
                double result = 0.0;

                if (c.args_size() > 0) {
                    boost::shared_ptr<scoped_cJSON_t> arg = eval(c.args(0), env);
                    if (arg->get()->type != cJSON_Number) {
                        throw runtime_exc_t("All operands to SUBTRACT must be numbers.");
                    }
                    if (c.args_size() == 1) {
                        result = 1.0 / arg->get()->valuedouble;  // (/ x) is reciprocal
                    } else {
                        result = arg->get()->valuedouble;
                    }

                    for (int i = 1; i < c.args_size(); ++i) {
                        boost::shared_ptr<scoped_cJSON_t> arg = eval(c.args(i), env);
                        if (arg->get()->type != cJSON_Number) {
                            throw runtime_exc_t("All operands to DIVIDE must be numbers.");
                        }
                        result /= arg->get()->valuedouble;
                    }
                }

                boost::shared_ptr<scoped_cJSON_t> res(new scoped_cJSON_t(cJSON_CreateNumber(result)));
                return res;
            }
            break;
        case Builtin::MODULO:
            {
                boost::shared_ptr<scoped_cJSON_t> lhs = eval(c.args(0), env),
                    rhs = eval(c.args(1), env);
                if (lhs->get()->type != cJSON_Number || rhs->get()->type != cJSON_Number) {
                    throw runtime_exc_t("Both operands to MOD must be numbers.");
                }

                boost::shared_ptr<scoped_cJSON_t> res(new scoped_cJSON_t(cJSON_CreateNumber(fmod(lhs->get()->valuedouble, rhs->get()->valuedouble))));
                return res;
            }
            break;
        case Builtin::COMPARE:
            {
                bool result = true;

                boost::shared_ptr<scoped_cJSON_t> lhs = eval(c.args(0), env);

                int type = lhs->get()->type;

                if (type != cJSON_Number &&
                    type != cJSON_String &&
                    type != cJSON_True &&
                    type != cJSON_False)
                {
                    throw runtime_exc_t("Comparison is undefined for this type.");
                }

                for (int i = 1; i < c.args_size(); ++i) {
                    boost::shared_ptr<scoped_cJSON_t> rhs = eval(c.args(i), env);

                    if (type == cJSON_Number) {
                        if (rhs->get()->type != type) {
                            throw runtime_exc_t("Cannot compare these types.");
                        }

                        double left = lhs->get()->valuedouble;
                        double right = rhs->get()->valuedouble;

                        switch (c.builtin().comparison()) {
                        case Builtin_Comparison_EQ:
                            result = (left == right);
                            break;
                        case Builtin_Comparison_NE:
                            result = (left != right);
                            break;
                        case Builtin_Comparison_LT:
                            result = (left < right);
                            break;
                        case Builtin_Comparison_LE:
                            result = (left <= right);
                            break;
                        case Builtin_Comparison_GT:
                            result = (left > right);
                            break;
                        case Builtin_Comparison_GE:
                            result = (left >= right);
                            break;
                        default:
                            crash("Unknown comparison operator.");
                            break;
                        }
                    } else if (type == cJSON_String) {
                        if (rhs->get()->type != type) {
                            throw runtime_exc_t("Cannot compare these types.");
                        }

                        char *left = lhs->get()->valuestring;
                        char *right = rhs->get()->valuestring;

                        int res = strcmp(left, right);
                        switch (c.builtin().comparison()) {
                        case Builtin_Comparison_EQ:
                            result = (res == 0);
                            break;
                        case Builtin_Comparison_NE:
                            result = (res != 0);
                            break;
                        case Builtin_Comparison_LT:
                            result = (res < 0);
                            break;
                        case Builtin_Comparison_LE:
                            result = (res <= 0);
                            break;
                        case Builtin_Comparison_GT:
                            result = (res > 0);
                            break;
                        case Builtin_Comparison_GE:
                            result = (res >= 0);
                            break;
                        default:
                            crash("Unknown comparison operator.");
                            break;
                        }
                    } else { // cJSON_True / cJSON_False
                        if (rhs->get()->type != cJSON_True && rhs->get()->type != cJSON_False) {
                            throw runtime_exc_t("Cannot compare these types.");
                        }

                        int lefttype = lhs->get()->type;
                        int righttype = rhs->get()->type;

                        bool eq = (lefttype == righttype);
                        bool lt = (lefttype == cJSON_False && righttype == cJSON_True);
                        bool gt = (lefttype == cJSON_True && righttype == cJSON_False);

                        switch (c.builtin().comparison()) {
                        case Builtin_Comparison_EQ:
                            result = eq;
                            break;
                        case Builtin_Comparison_NE:
                            result = !eq;
                            break;
                        case Builtin_Comparison_LT:
                            result = lt;
                            break;
                        case Builtin_Comparison_LE:
                            // False is always <= any bool
                            result = eq && lt;
                            break;
                        case Builtin_Comparison_GT:
                            result = gt;
                            break;
                        case Builtin_Comparison_GE:
                            result = eq && gt;
                            break;
                        default:
                            crash("Unknown comparison operator.");
                            break;
                        }
                    }

                    if (!result) {
                        break;
                    }

                    lhs = rhs;
                }

                boost::shared_ptr<scoped_cJSON_t> res(new scoped_cJSON_t(cJSON_CreateBool(result)));
                return res;
            }
            break;
        case Builtin::FILTER:
            crash("Not implemented");
            break;
        case Builtin::MAP:
            crash("Not implemented");
            break;
        case Builtin::CONCATMAP:
            crash("Not implemented");
            break;
        case Builtin::ORDERBY:
            crash("Not implemented");
            break;
        case Builtin::DISTINCT:
            crash("Not implemented");
            break;
        case Builtin::LIMIT:
            crash("Not implemented");
            break;
        case Builtin::LENGTH:
            crash("Not implemented");
            break;
        case Builtin::UNION:
            crash("Not implemented");
            break;
        case Builtin::NTH:
            crash("Not implemented");
            break;
        case Builtin::STREAMTOARRAY:
            crash("Not implemented");
            break;
        case Builtin::ARRAYTOSTREAM:
            crash("Not implemented");
            break;
        case Builtin::REDUCE:
            crash("Not implemented");
            break;
        case Builtin::GROUPEDMAPREDUCE:
            crash("Not implemented");
            break;
        case Builtin::JAVASCRIPT:
            crash("Not implemented");
            break;
        case Builtin::JAVASCRIPTRETURNINGSTREAM:
            crash("Not implemented");
            break;
        case Builtin::MAPREDUCE:
            crash("Not implemented");
            break;
        case Builtin::ALL:
            {
                bool result = true;

                for (int i = 0; i < c.args_size(); ++i) {
                    boost::shared_ptr<scoped_cJSON_t> arg = eval(c.args(i), env);
                    if (arg->get()->type != cJSON_False && arg->get()->type != cJSON_True) {
                        throw runtime_exc_t("All operands to ALL must be booleans.");
                    }
                    if (arg->get()->type != cJSON_True) {
                        result = false;
                        break;
                    }
                }

                boost::shared_ptr<scoped_cJSON_t> res(new scoped_cJSON_t(cJSON_CreateBool(result)));
                return res;
            }
            break;
        case Builtin::ANY:
            {
                bool result = false;

                for (int i = 0; i < c.args_size(); ++i) {
                    boost::shared_ptr<scoped_cJSON_t> arg = eval(c.args(i), env);
                    if (arg->get()->type != cJSON_False && arg->get()->type != cJSON_True) {
                        throw runtime_exc_t("All operands to ANY must be booleans.");
                    }
                    if (arg->get()->type == cJSON_True) {
                        result = true;
                        break;
                    }
                }

                boost::shared_ptr<scoped_cJSON_t> res(new scoped_cJSON_t(cJSON_CreateBool(result)));
                return res;
            }
            break;
        default:
            crash("unreachable");
            break;
    }
    crash("unreachable");
}

namespace_repo_t<rdb_protocol_t>::access_t eval(const TableRef &t, runtime_environment_t *env) THROWS_ONLY(runtime_exc_t) {
    boost::optional<std::pair<namespace_id_t, deletable_t<namespace_semilattice_metadata_t<rdb_protocol_t> > > > namespace_info =
        env->semilattice_metadata->get().rdb_namespaces.get_namespace_by_name(t.table_name());

    if (namespace_info) {
        return namespace_repo_t<rdb_protocol_t>::access_t(env->ns_repo, namespace_info->first, &env->interruptor);
    } else {
        throw runtime_exc_t(strprintf("Namespace %s either not found, ambigious or namespace metadata in conflict.", t.table_name().c_str()));
    }
}

} //namespace query_language
