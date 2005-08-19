/**
 * @file
 *
 * Creation of algebra translations for XQuery built-in functions.
 * The representation of such functions was extended by a function
 * pointer that references the functions defined in this file.
 *
 * Copyright Notice:
 * -----------------
 *
 * The contents of this file are subject to the Pathfinder Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License.  You may obtain a copy of the License at
 * http://monetdb.cwi.nl/Legal/PathfinderLicense-1.1.html
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the Pathfinder system.
 *
 * The Initial Developer of the Original Code is the Database &
 * Information Systems Group at the University of Konstanz, Germany.
 * Portions created by the University of Konstanz are Copyright (C)
 * 2000-2005 University of Konstanz.  All Rights Reserved.
 *
 * $Id$
 */

/* always include "pathfinder.h", first! */
#include "pathfinder.h"

#include "builtins.h"

#include "logical.h"
#include "logical_mnemonic.h"

/**
 * Worker function to construct algebra implementation of binary
 * functions (e.g., arithmetic functions: op:numeric-add(),
 * op:numeric-subtract(); comparison functions: op:eq(); boolean
 * functions: op:and();...).
 *
 * env,loop,delta: e1 => q1,delta1   env,loop,delta1: e2 => q2,delta2
 * ------------------------------------------------------------------
 *                 env,loop,delta: (e1 + e2) =>
 * (proj_iter,pos,item:res(
 *   OP_res<item,item1>(
 *     cast_item,t (q1)
 *     |X| (iter,iter1)
 *     (proj_iter1:iter,item1:item (cast_item,t (q2)))))
 *  ,
 *  delta2
 * )
 *
 * (where OP is the algebra operator that implements the function,
 * and t is the implementation type that is used)
 *
 * @param t    Algebra type that should be used for this operator.
 *             All arguments will be cast to that type before invoking
 *             the actual algebra operator. This makes these
 *             operations always monomorphic (and thus reasonably
 *             efficient).
 * @param op   Algebra tree node constructor for the algebra operator
 *             that shall be used for this operation. (Pick any of the
 *             construction functions in algebra.c.)
 * @param args Builtin function argument list as in the calling
 *             conventions for algebra implementations in the PFfun_t
 *             struct (field @c alg). (Our implementations for these
 *             functions do not require document representation and/or
 *             the loop relation. They are thus missing here.)
 */
static struct PFla_pair_t
bin_arith (PFalg_simple_type_t t,
           PFla_op_t *(*OP) (const PFla_op_t *, PFalg_att_t,
                             PFalg_att_t, PFalg_att_t),
           struct PFla_pair_t *args)
{
    return (struct PFla_pair_t) {
	.rel = project (OP (eqjoin (cast (args[0].rel, "item", t),
				    project (cast (args[1].rel,
						   "item", t),
					     proj ("iter1", "iter"),
					     proj ("item1", "item")),
				    "iter",
				    "iter1"),
			    "res", "item", "item1"),
			proj ("iter", "iter"),
			proj ("pos", "pos"),
			proj ("item", "res")),
	.frag = PFla_empty_set () };
}


/**
 * Worker function to construct algebra implementation of unary
 * functions (e.g., fn:not(), op:numeric-unary-plus(),...).
 *
 * env,loop,delta: e1 => q1,delta1
 * ------------------------------------------------------------------
 *                 env,loop,delta: e1 =>
 * (proj_iter,pos,item:res(OP_res<item>(cast_item,t (q1)))
 *  ,
 *  delta2
 * )
 *
 * (where OP is the algebra operator that implements the function,
 * and t is the implementation type that is used)
 *
 * @param t    Algebra type that should be used for this operator.
 *             All arguments will be cast to that type before invoking
 *             the actual algebra operator. This makes these
 *             operations always monomorphic (and thus reasonably
 *             efficient).
 * @param op   Algebra tree node constructor for the algebra operator
 *             that shall be used for this operation. (Pick any of the
 *             construction functions in algebra.c.)
 * @param args Builtin function argument list as in the calling
 *             conventions for algebra implementations in the PFfun_t
 *             struct (field @c alg). (Our implementations for these
 *             functions do not require document representation and/or
 *             the loop relation. They are thus missing here.)
 */
static struct PFla_pair_t
un_func (PFalg_simple_type_t t,
	 PFla_op_t *(*OP) (const PFla_op_t *, PFalg_att_t, PFalg_att_t),
	 struct PFla_pair_t *args)
{
    return (struct PFla_pair_t) {
	.rel = project (OP (cast (args[0].rel, "item", t),
			    "res", "item"),
			proj ("iter", "iter"),
			proj ("pos", "pos"),
			proj ("item", "res")),
	.frag = PFla_empty_set () };
}


/**
 * Algebra implementation for op:numeric-add(integer?,integer?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_numeric_add_int (const PFla_op_t *loop __attribute__((unused)),
                          struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_int, PFla_add, args);
}

/**
 * Algebra implementation for op:numeric-add(decimal?,decimal?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_numeric_add_dec (const PFla_op_t *loop __attribute__((unused)),
                          struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_dec, PFla_add, args);
}

/**
 * Algebra implementation for op:numeric-add(double?,double?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_numeric_add_dbl (const PFla_op_t *loop __attribute__((unused)),
                          struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand 
                    "__attribute__((unused))" */
    return bin_arith (aat_dbl, PFla_add, args);
}


/**
 * Algebra implementation for op:numeric-subtract(integer?,integer?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_numeric_subtract_int (const PFla_op_t *loop __attribute__((unused)),
                               struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand 
                    "__attribute__((unused))" */
    return bin_arith (aat_int, PFla_subtract, args);
}

/**
 * Algebra implementation for op:numeric-subtract(decimal?,decimal?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_numeric_subtract_dec (const PFla_op_t *loop __attribute__((unused)),
                               struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_dec, PFla_subtract, args);
}

/**
 * Algebra implementation for op:numeric-subtract(double?,double?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_numeric_subtract_dbl (const PFla_op_t *loop __attribute__((unused)),
                               struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_dbl, PFla_subtract, args);
}


/**
 * Algebra implementation for op:numeric-multiply(integer?,integer?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_numeric_multiply_int (const PFla_op_t *loop __attribute__((unused)),
                               struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_int, PFla_multiply, args);
}

/**
 * Algebra implementation for op:numeric-multiply(decimal?,decimal?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_numeric_multiply_dec (const PFla_op_t *loop __attribute__((unused)),
                               struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_dec, PFla_multiply, args);
}

/**
 * Algebra implementation for op:numeric-multiply(double?,double?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_numeric_multiply_dbl (const PFla_op_t *loop __attribute__((unused)),
                               struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_dbl, PFla_multiply, args);
}


/**
 * Algebra implementation for op:numeric-divide(decimal?,decimal?)
 * @see bin_arith()
 *
 * NB: A function for the division of two integer operators is required
 * because, according to the XQuery specifications, the division of two
 * integers returns a decimal number, i.e. we let the two operands be
 * promoted to decimal and use "PFbui_op_numeric_divide_dec".     
 */
struct PFla_pair_t
PFbui_op_numeric_divide_dec (const PFla_op_t *loop __attribute__((unused)),
                             struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_dec, PFla_divide, args);
}

/**
 * Algebra implementation for op:numeric-divide(double?,double?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_numeric_divide_dbl (const PFla_op_t *loop __attribute__((unused)),
                             struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_dbl, PFla_divide, args);
}


/**
 * Algebra implementation for op:numeric-integer-divide(integer?,integer?)
 * @see bin_arith()
 *
 * NB: ($a idiv $b) <=> ($a div $b) cast as xs:integer
 */
struct PFla_pair_t
PFbui_op_numeric_idivide_int (const PFla_op_t *loop __attribute__((unused)),
			      struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return (struct PFla_pair_t) {
	.rel = cast (bin_arith (aat_int, PFla_divide, args).rel,
		     "item", aat_int),
	.frag = PFla_empty_set () };
}

/**
 * Algebra implementation for op:numeric-integer-divide(decimal?,decimal?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_numeric_idivide_dec (const PFla_op_t *loop __attribute__((unused)),
                             struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return (struct PFla_pair_t) {
	.rel = cast (bin_arith (aat_dec, PFla_divide, args).rel,
		     "item", aat_int),
	.frag = PFla_empty_set () };
}

/**
 * Algebra implementation for op:numeric-integer-divide(double?,double?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_numeric_idivide_dbl (const PFla_op_t *loop __attribute__((unused)),
			      struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return (struct PFla_pair_t) {
	.rel = cast (bin_arith (aat_dbl, PFla_divide, args).rel,
		     "item", aat_int),
	.frag = PFla_empty_set () };
}


/**
 * Algebra implementation for op:numeric-mod(integer?,integer?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_numeric_modulo_int (const PFla_op_t *loop __attribute__((unused)),
                             struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_int, PFla_modulo, args);
}

/**
 * Algebra implementation for op:numeric-mod(decimal?,decimal?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_numeric_modulo_dec (const PFla_op_t *loop __attribute__((unused)),
                             struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_dec, PFla_modulo, args);
}

/**
 * Algebra implementation for op:numeric-mod(double?,double?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_numeric_modulo_dbl (const PFla_op_t *loop __attribute__((unused)),
                             struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_dbl, PFla_modulo, args);
}


/**
 * Algebra implementation for op:gt(integer?,integer?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_gt_int (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_int, PFla_gt, args);
}

/**
 * Algebra implementation for op:gt(decimal?,decimal?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_gt_dec (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_dec, PFla_gt, args);
}

/**
 * Algebra implementation for op:gt(double?,double?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_gt_dbl (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_dbl, PFla_gt, args);
}

/**
 * Algebra implementation for op:gt(boolean?,boolean?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_gt_bln (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_bln, PFla_gt, args);
}

/**
 * Algebra implementation for op:gt(string?,string?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_gt_str (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_str, PFla_gt, args);
}


/**
 * Algebra implementation for op:ge(integer?,integer?)
 * @see un_func() and bin_arith()
 */
struct PFla_pair_t
PFbui_op_ge_int (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void)loop; /* pacify picky compilers that do not understand
                   "__attribute__((unused))" */
    return un_func (aat_bln, PFla_not,
        (struct PFla_pair_t []) { bin_arith (aat_int, PFla_gt,
                  (struct PFla_pair_t []) { args[1], args[0] }) });
}

/**
 * Algebra implementation for op:ge(decimal?,decimal?)
 * @see un_func() and bin_arith()
 */
struct PFla_pair_t
PFbui_op_ge_dec (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return un_func (aat_bln, PFla_not,
        (struct PFla_pair_t []) { bin_arith (aat_dec, PFla_gt,
                  (struct PFla_pair_t []) { args[1], args[0] }) });
}

/**
 * Algebra implementation for op:ge(double?,double?)
 * @see un_func() and bin_arith()
 */
struct PFla_pair_t
PFbui_op_ge_dbl (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return un_func (aat_bln, PFla_not,
        (struct PFla_pair_t []) { bin_arith (aat_dbl, PFla_gt,
                  (struct PFla_pair_t []) { args[1], args[0] }) });
}

/**
 * Algebra implementation for op:ge(boolean?,boolean?)
 * @see un_func() and bin_arith()
 */
struct PFla_pair_t
PFbui_op_ge_bln (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return un_func (aat_bln, PFla_not,
        (struct PFla_pair_t []) { bin_arith (aat_bln, PFla_gt,
                  (struct PFla_pair_t []) { args[1], args[0] }) });
}

/**
 * Algebra implementation for op:ge(string?,string?)
 * @see un_func() and bin_arith()
 */
struct PFla_pair_t
PFbui_op_ge_str (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return un_func (aat_bln, PFla_not,
        (struct PFla_pair_t []) { bin_arith (aat_str, PFla_gt,
                  (struct PFla_pair_t []) { args[1], args[0] }) });
}


/**
 * Algebra implementation for op:lt(integer?,integer?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_lt_int (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return bin_arith (aat_int, PFla_gt,
                      (struct PFla_pair_t []) { args[1], args[0] });
}

/**
 * Algebra implementation for op:lt(decimal?,decimal?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_lt_dec (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return bin_arith (aat_dec, PFla_gt,
                      (struct PFla_pair_t []) { args[1], args[0] });
}

/**
 * Algebra implementation for op:lt(double?,double?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_lt_dbl (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return bin_arith (aat_dbl, PFla_gt,
                      (struct PFla_pair_t []) { args[1], args[0] });
}

/**
 * Algebra implementation for op:lt(boolean?,boolean?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_lt_bln (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_bln, PFla_gt,
                      (struct PFla_pair_t []) { args[1], args[0] });
}

/**
 * Algebra implementation for op:lt(string?,string?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_lt_str (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return bin_arith (aat_str, PFla_gt,
                      (struct PFla_pair_t []) { args[1], args[0] });
}


/**
 * Algebra implementation for op:le(integer?,integer?)
 * @see un_func() and bin_arith()
 */
struct PFla_pair_t
PFbui_op_le_int (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return un_func (aat_bln, PFla_not,
        (struct PFla_pair_t []) { bin_arith (aat_int, PFla_gt, args) });
}

/**
 * Algebra implementation for op:le(decimal?,decimal?)
 * @see un_func() and bin_arith()
 */
struct PFla_pair_t
PFbui_op_le_dec (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return un_func (aat_bln, PFla_not,
        (struct PFla_pair_t []) { bin_arith (aat_dec, PFla_gt, args) });
}

/**
 * Algebra implementation for op:le(double?,double?)
 * @see un_func() and bin_arith()
 */
struct PFla_pair_t
PFbui_op_le_dbl (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return un_func (aat_bln, PFla_not,
        (struct PFla_pair_t []) { bin_arith (aat_dbl, PFla_gt, args) });
}

/**
 * Algebra implementation for op:le(boolean?,boolean?)
 * @see un_func() and bin_arith()
 */
struct PFla_pair_t
PFbui_op_le_bln (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return un_func (aat_bln, PFla_not,
        (struct PFla_pair_t []) { bin_arith (aat_bln, PFla_gt, args) });
}

/**
 * Algebra implementation for op:le(string?,string?)
 * @see un_func() and bin_arith()
 */
struct PFla_pair_t
PFbui_op_le_str (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return un_func (aat_bln, PFla_not,
        (struct PFla_pair_t []) { bin_arith (aat_str, PFla_gt, args) });
}


/**
 * Algebra implementation for op:eq(integer?,integer?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_eq_int (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_int, PFla_eq, args);
}

/**
 * Algebra implementation for op:eq(decimal?,decimal?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_eq_dec (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_dec, PFla_eq, args);
}

/**
 * Algebra implementation for op:eq(double?,double?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_eq_dbl (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_dbl, PFla_eq, args);
}

/**
 * Algebra implementation for op:eq(boolean?,boolean?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_eq_bln (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_bln, PFla_eq, args);
}

/**
 * Algebra implementation for op:eq(string?,string?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_eq_str (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_str, PFla_eq, args);
}


/**
 * Algebra implementation for op:ne(integer?,integer?)
 * @see un_func() and bin_arith()
 */
struct PFla_pair_t
PFbui_op_ne_int (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return un_func (aat_bln, PFla_not,
        (struct PFla_pair_t []) { bin_arith (aat_int, PFla_eq, args) });
}

/**
 * Algebra implementation for op:ne(decimal?,decimal?)
 * @see un_func() and bin_arith()
 */
struct PFla_pair_t
PFbui_op_ne_dec (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return un_func (aat_bln, PFla_not,
        (struct PFla_pair_t []) { bin_arith (aat_dec, PFla_eq, args) });
}

/**
 * Algebra implementation for op:ne(double?,double?)
 * @see un_func() and bin_arith()
 */
struct PFla_pair_t
PFbui_op_ne_dbl (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return un_func (aat_bln, PFla_not,
        (struct PFla_pair_t []) { bin_arith (aat_dbl, PFla_eq, args) });
}

/**
 * Algebra implementation for op:ne(boolean?,boolean?)
 * @see un_func() and bin_arith()
 */
struct PFla_pair_t
PFbui_op_ne_bln (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    
    return un_func (aat_bln, PFla_not,
        (struct PFla_pair_t []) { bin_arith (aat_bln, PFla_eq, args) });
}

/**
 * Algebra implementation for op:ne(string?,string?)
 * @see un_func() and bin_arith()
 */
struct PFla_pair_t
PFbui_op_ne_str (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return un_func (aat_bln, PFla_not,
        (struct PFla_pair_t []) { bin_arith (aat_str, PFla_eq, args) });
}

/**
 * Algebra implementation for fn:not (boolean) as boolean
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_fn_not_bln (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return un_func (aat_bln, PFla_not, args);
}

/**
 * Algebra implementation for op:or (boolean, boolean) as boolean
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_or_bln (const PFla_op_t *loop __attribute__((unused)),
                 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_bln, PFla_or, args);
}

/**
 * Algebra implementation for op:and (boolean, boolean) as boolean
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_and_bln (const PFla_op_t *loop __attribute__((unused)),
		  struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_bln, PFla_and, args);
}


/**
 * Algebra implementation for <code>fn:boolean (xs:boolean)</code>.
 *
 * If the operand is a single boolean value, the function returns
 * the boolean value itself.
 */
struct PFla_pair_t
PFbui_fn_boolean_bln (const PFla_op_t *loop __attribute__((unused)),
                       struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return args[0];
}


/**
 * Algebra implementation for op:is-same-node (node?, node?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_is_same_node (const PFla_op_t *loop __attribute__((unused)),
		       struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_node, PFla_eq, args);
}

/**
 * Algebra implementation for op:node-before (node?, node?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_node_before (const PFla_op_t *loop __attribute__((unused)),
		      struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return bin_arith (aat_node, PFla_gt,
		      (struct PFla_pair_t []) { args[1], args[0] });
}

/**
 * Algebra implementation for op:node-after (node?, node?)
 * @see bin_arith()
 */
struct PFla_pair_t
PFbui_op_node_after (const PFla_op_t *loop __attribute__((unused)),
		     struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */
    return bin_arith (aat_node, PFla_gt, args);
}


/**
 * Algebra implementation for op:union (node*, node*)
 *
 * Constructs a sequence containing every node that occurs in the
 * values of either the first or the second parameter, eliminating
 * duplicate nodes. Nodes are returned in document order. Two nodes
 * are equal if they are op:is-same-node().
 */
struct PFla_pair_t
PFbui_op_union (const PFla_op_t *loop __attribute__((unused)),
		struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return (struct  PFla_pair_t) {
        .rel = rownum (
	           distinct (
		       disjunion (
			   project (args[0].rel,
				    proj ("iter", "iter"),
				    proj ("item", "item")),
			   project (args[1].rel,
				    proj ("iter", "iter"),
				    proj ("item", "item")))),
		   "pos", sortby ("item"), NULL),
        .frag = PFla_set_union (args[0].frag, args[1].frag) };
}

/**
 * Algebra implementation for op:intersect (node*, node*)
 *
 * Constructs a sequence containing every node that occurs in the
 * values of both the first and the second parameter, eliminating
 * duplicate nodes. Nodes are returned in document order. If either
 * operand is the empty sequence, the empty sequence is returned.
 * Two nodes are equal if they are op:is-same-node().
 */
struct PFla_pair_t
PFbui_op_intersect (const PFla_op_t *loop __attribute__((unused)),
		    struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return (struct  PFla_pair_t) {
        .rel = rownum (
	           distinct (
		       intersect (
			   project (args[0].rel,
				    proj ("iter", "iter"),
				    proj ("item", "item")),
			   project (args[1].rel,
				    proj ("iter", "iter"),
				    proj ("item", "item")))),
		   "pos", sortby ("item"), NULL),
        .frag = PFla_set_union (args[0].frag, args[1].frag) };
}

/**
 * Algebra implementation for op:except (node*, node*)
 *
 * Constructs a sequence containing every node that occurs in the
 * value of the first parameter, but not in the value of the second
 * parameter, eliminating duplicate nodes. Nodes are returned in
 * document order. If the first parameter is the empty sequence, the
 * empty sequence is returned. If the second parameter is the empty
 * sequence, the first parameter is returned. Two nodes are equal if
 * they are op:is-same-node().
 */
struct PFla_pair_t
PFbui_op_except (const PFla_op_t *loop __attribute__((unused)),
		 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return (struct  PFla_pair_t) {
        .rel = rownum (
	           distinct (
		       difference (
			   project (args[0].rel,
				    proj ("iter", "iter"),
				    proj ("item", "item")),
			   project (args[1].rel,
				    proj ("iter", "iter"),
				    proj ("item", "item")))),
		   "pos", sortby ("item"), NULL),
	/* result nodes can only originate from first argument TODO */
        .frag = args[0].frag };
}


/**
 *
 * Th fs:item-sequence-to-node-sequence function converts a sequence
 * of item values to nodes (see FS, Section 6.1.6).
 *
 * intput:
 *          iter | pos | item
 *         -------------------
 *               |     | "a"
 *               |     | 42
 *               |     | <foo/>
 *               |     | 1.2
 *
 * - insert new, consecutive row numbering of "pos" column
 * - select those rows that have type "node" (part1)
 * - select the remaining rows (part2)
 * - convert all items in part2 into strings
 * - concatenate consecutive strings by putting a space between them;
 *   (e.g. "a" . " " . "42"); we must introduce a new operator for
 *   this step
 * - create text nodes from the (concatenated) strings; IMPORTANT:
 *   textnode() function was generalized to retain "pos" numbering
 * - add the new fragment of text nodes (frag) to the .frag field
 *   together with those nodes we had in the very beginning (those
 *   in part1, e.g. <foo/>)
 * - project frag on "iter", "pos", "item"
 * - form union of projection result and part1
 * - sort result on "pos" column to restore original sort order
 */
struct PFla_pair_t
PFbui_pf_item_seq_to_node_seq (const PFla_op_t *loop __attribute__((unused)),
			       struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    /*
     * insert new, consecutive row numbering of "pos" column and
     * carry out type test on "node" type
     */
    PFla_op_t *sort = type (rownum (args[0].rel, "pos1",
				     sortby ("pos"), "iter"),
			     "res", "item", aat_node);

    /* select those rows that have type "node" (part1) */
    PFla_op_t *part1 = project (select_ (sort, "res"),
				 proj ("iter", "iter"),
				 proj ("pos", "pos1"),
				 proj ("item", "item"));

    /* select the remaining rows (part2) */
    PFla_op_t *part2 = project (select_ (not (sort, "res1", "res"), "res1"),
				 proj ("iter", "iter"),
				 proj ("pos", "pos1"),
				 proj ("item", "item"));

    /*
     * convert all items in part2 into strings and concatenate
     * consecutive strings (by putting a space between them;
     * (e.g. "a" . " " . "42"); create text nodes from the
     * (concatenated) strings
     */
    PFla_op_t *t_nodes = textnode (
                              strconcat (
				  cast (part2, "item", aat_str)));

    /* get the roots of the new text nodes, form union of roots and
     * part1, and sort result on "pos" column
     */
    return (struct  PFla_pair_t) {
                 .rel = project (
                            rownum (
				disjunion (
				    roots (t_nodes),
				    part1),
				"pos1", sortby ("pos"), "iter"),
			    proj ("iter", "iter"),
			    proj ("pos", "pos1"),
			    proj ("item", "item")),
		 /* union of those nodes we had in the very beginning
		  * (those in part1) and those produced by text node
		  * creation
		  */
                 .frag = PFla_set_union (args[0].frag,
					  PFla_set (fragment (t_nodes)))};
}


/**
 * Merge adjacent textnodes into one text node and delete empty text
 * nodes.
 *
 * Input: iter | pos | item table where all items are of type node.
 * Introduce new algebra operator which takes the current document and
 * the current algebra representation. It merges consecutive text nodes
 * (with same "iter" and consecutive "pos" values). If a text node
 * is empty, it is discarded.
 * The output are an algebra representation of all nodes (old and new,
 * i.e. unmerged and merged) and a fragment representation of the newly
 * created nodes only.
 */
struct PFla_pair_t
PFbui_pf_merge_adjacent_text_nodes (
        const PFla_op_t *loop __attribute__((unused)),
        struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    PFla_op_t *merged
        = merge_adjacent (PFla_set_to_la (args[0].frag), args[0].rel);

    return (struct  PFla_pair_t) {
                 .rel = roots (merged),
		 /* form union of old and new fragment */
		 .frag = PFla_set_union (args[0].frag,
					  PFla_set (fragment (merged))) };
}


/**
 * The fs:distinct-doc-order function sorts its input sequence of
 * nodes by document order and removes duplicates.
 */
struct PFla_pair_t
PFbui_pf_distinct_doc_order (const PFla_op_t *loop __attribute__((unused)),
			     struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return (struct  PFla_pair_t) {
                 .rel = rownum (
                            distinct (
				project (args[0].rel,
					 proj ("iter", "iter"),
					 proj ("item", "item"))),
			    "pos", sortby ("item"), "iter"),
                 .frag = args[0].frag };
}


/**
 * Algebra implementation for <code>fn:boolean (xs:boolean?)</code>.
 *
 * If the operand is an optional boolean value, the function returns
 * the value itself if there is one, and @c false if the operand is
 * the empty sequence.
 *
 *              env,loop,delta: e1 => q1,delta1
 *  --------------------------------------------------------------
 *             env,loop,delta: fn:boolean(e1) =>
 * ( q1 U
 *   /                                                               pos|item\
 *  |loop \ (proj_iter (q1 |X| (iter,iter1) proj_iter1:iter loop)) X ---+---- |
 *   \                                                                1 | fal/
 * ,
 *   delta1
 * )
 */
struct PFla_pair_t
PFbui_fn_boolean_optbln (const PFla_op_t *loop __attribute__((unused)),
			 struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return (struct PFla_pair_t) {
	.rel = disjunion (args[0].rel,
			  cross (
			      difference (loop,
					  project (eqjoin (
						       args[0].rel,
						       project (loop,
								proj ("iter1",
								      "iter")),
						       "iter", "iter1"),
						   proj ("iter", "iter"))),
			      lit_tbl (attlist ("pos", "item"),
				       tuple (lit_nat (1), lit_bln (false))))),
	.frag = PFla_empty_set () };
}

/**
 * @bug FIXME: This needs to be changed. It's just to far off the XQuery
 *      semantics.
 */
struct PFla_pair_t
PFbui_fn_boolean_item (const PFla_op_t *loop __attribute__((unused)),
                       struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return (struct PFla_pair_t) {
	.rel = cross (
                   disjunion (
		       cross (
			   distinct (project (args[0].rel,
					      proj ("iter", "iter"))),
			   lit_tbl (attlist ("item"),
				    tuple (lit_bln (true)))),
		       cross (
			   difference (
			       loop,
			       project (args[0].rel, proj ("iter",
							   "iter"))),
			   lit_tbl (attlist ("item"),
				    tuple (lit_bln (false))))),
		   lit_tbl (attlist ("pos"), tuple (lit_nat (1)))),
	.frag = PFla_empty_set ()};
}

/**
 * Algebra implementation for <code>fn:empty(item*)</code>.
 *
 *            env,loop,delta: e1 => (q1, delta1)
 *  ---------------------------------------------------------------------
 *             env,loop,delta: fn:empty (e1) =>
 * (
 *  / //                      itm\     /                        itm\\    pos \
 * | || dist (proj_iter q1) X --- | U | (loop \ proj_iter q1) X --- || X ---  |
 *  \ \\                      fal/     \                        tru//     1  /
 * ,
 *  delta1
 * )
 */
struct PFla_pair_t
PFbui_fn_empty (const PFla_op_t *loop __attribute__((unused)),
                struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return (struct PFla_pair_t) {
	.rel = cross (
                   disjunion (
		       cross (
			   distinct (project (args[0].rel,
					      proj ("iter", "iter"))),
			   lit_tbl (attlist ("item"),
				    tuple (lit_bln (false)))),
		       cross (
			   difference (
			       loop,
			       project (args[0].rel,
					proj ("iter", "iter"))),
			   lit_tbl (attlist ("item"),
				    tuple (lit_bln (true))))),
		   lit_tbl (attlist ("pos"), tuple (lit_nat (1)))),
	.frag = PFla_empty_set ()};
}

/**
 * Built-in function <code>fn:doc(xs:string?)</code>: Return root
 * node of document that is found under given URI. Return empty
 * sequence if the argument is the empty sequence.
 *
 * The latter (the ``empty sequence'') case is actually nice for
 * us. No need to access the loop relation in any way.
 */
struct PFla_pair_t
PFbui_fn_doc (const PFla_op_t *loop __attribute__((unused)),
              struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    PFla_op_t *doc = doc_tbl (project (args[0].rel,
                                       proj ("iter", "iter"),
                                       proj ("item", "item")));

    return (struct PFla_pair_t) {
        .rel  = cross (lit_tbl (attlist ("pos"), tuple (lit_nat (1))),
                       roots (doc)),
        .frag = PFla_set (fragment (doc)) };
}

/**
 * Built-in functin <code>pf:typed-value</code>.
 */
struct PFla_pair_t
PFbui_pf_string_value (const PFla_op_t *loop __attribute__((unused)),
                       struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return (struct PFla_pair_t) {
        .rel  = string_value (PFla_set_to_la (args[0].frag), args[0].rel),
        .frag = PFla_empty_set () };
}

/**
 * Build up operator tree for built-in function 'fn:typed-value'.
 */
struct PFla_pair_t
PFbui_op_typed_value (const PFla_op_t *loop __attribute__((unused)),
                      struct PFla_pair_t *args)
{
    (void) loop; /* pacify picky compilers that do not understand
                    "__attribute__((unused))" */

    return args[0];
}

/**
 * Build up operator tree for built-in function 'fn:count'.
 *
 *                env,loop: e => (q,delta)
 *  -------------------------------------------------------------------
 *                         env,loop: fn:count(e) =>
 *  //                             \     //                  \    item\\    pos
 * ||count_item/iter (proj_iter (q))| U ||loop \ proj_iter (q)| X ---- || X ---,
 *  \\                             /     \\                  /      0 //     1
 *                                    ()
 */
struct PFla_pair_t
PFbui_fn_count (const PFla_op_t *loop __attribute__((unused)),
                struct PFla_pair_t *args)
{
    return (struct PFla_pair_t) {
        .rel = cross (
                disjunion (
                    count (project (args[0].rel, proj ("iter", "iter")),
                           "item", "iter"),
                    cross (
                        difference (
                            loop,
                            project (args[0].rel, proj ("iter", "iter"))),
                        lit_tbl (attlist ("item"), tuple (lit_int (0))))),
                lit_tbl (attlist ("pos"), tuple (lit_nat (1)))),
        .frag = PFla_empty_set () };
}

/* vim:set shiftwidth=4 expandtab: */
