// Autogenerated by convert_protofile.py.
// Do not edit this file directly.
// The template for this file is located at:
// ../../../../../../../../templates/AstSubclass.java
package com.rethinkdb.ast.gen;

import com.rethinkdb.ast.helper.Arguments;
import com.rethinkdb.ast.helper.OptArgs;
import com.rethinkdb.ast.RqlAst;
import com.rethinkdb.proto.TermType;
import java.util.*;



public class DayOfWeek extends RqlQuery {


    public DayOfWeek(java.lang.Object arg) {
        this(new Arguments(arg), null);
    }
    public DayOfWeek(Arguments args, OptArgs optargs) {
        this(null, args, optargs);
    }
    public DayOfWeek(RqlAst prev, Arguments args, OptArgs optargs) {
        this(prev, TermType.DAY_OF_WEEK, args, optargs);
    }
    protected DayOfWeek(RqlAst previous, TermType termType, Arguments args, OptArgs optargs){
        super(previous, termType, args, optargs);
    }


    /* Static factories */
    public static DayOfWeek fromArgs(Object... args){
        return new DayOfWeek(new Arguments(args), null);
    }


}