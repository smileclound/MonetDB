/* -*- mode:C; c-basic-offset:4; c-indentation-style:"k&r"; indent-tabs-mode:nil -*-*/

/**
 * @file
 *
 * Mnemonic XQuery Core constructor names.
 *
 * This introduces mnemonic abbreviations for the PFcore_... constructors
 * in core/core.c.
 *
 * $Id$
 */

#ifndef CORE_MNEMONIC_H
#define CORE_MNEMONIC_H

#include "core.h"

#undef nil        
#undef new_var    
#undef var        
#undef num        
#undef dec        
#undef dbl        
#undef str        
#undef seqtype    
#undef seqcast    
#undef proof      
#undef typeswitch 
#undef case_      
#undef cases      
#undef ifthenelse 
#undef int_eq     
#undef for_       
#undef let        
#undef seq        
#undef empty      
#undef true_      
#undef false_     
#undef _root
#undef locsteps   
#undef step       
#undef kindt      
#undef namet      
#undef function   
#undef apply      
#undef ebv        
#undef error      
#undef error_loc  

#define nil()                PFcore_nil ()
#define new_var(v)           PFcore_new_var (v)
#define var(v)               PFcore_var (v)
#define num(n)               PFcore_num (n)
#define dec(d)               PFcore_dec (d)
#define dbl(d)               PFcore_dbl (d)
#define str(s)               PFcore_str (s)
#define seqtype(t)           PFcore_seqtype (t)
#define seqcast(e1,e2)       PFcore_seqcast ((e1), (e2))
#define proof(e1,t,e2)       PFcore_proof ((e1), (t), (e2))
#define typeswitch(e1,e2,e3) PFcore_typeswitch ((e1), (e2), (e3))
#define case_(e1,e2)         PFcore_case ((e1), (e2))
#define cases(e1,e2)         PFcore_cases ((e1),(e2))
#define ifthenelse(e1,e2,e3) PFcore_ifthenelse ((e1), (e2), (e3))
#define int_eq(e1,e2)        PFcore_int_eq ((e1), (e2))
#define for_(e1,e2,e3,e4)    PFcore_for ((e1), (e2), (e3), (e4))
#define let(e1,e2,e3)        PFcore_let ((e1), (e2), (e3))
#define seq(e1,e2)           PFcore_seq ((e1), (e2))
#define empty()              PFcore_empty ()
#define true_()              PFcore_true ()
#define false_()             PFcore_false ()
#define _root()              PFcore_root ()
#define locsteps(e1,e2)      PFcore_locsteps ((e1), (e2))
#define step(a,e)            PFcore_step ((a), (e))
#define kindt(k,e)           PFcore_kindt ((k), (e))
#define namet(qn)            PFcore_namet (qn)
#define function(qn)         PFcore_function (qn)
#define arg(e1,e2)           PFcore_arg((e1), (e2))
#define apply(fn,e)          PFcore_apply ((fn), (e))
#define ebv(e)               PFcore_ebv (e)
#define error(s,...)         PFcore_error ((s), __VA_ARGS__)
#define error_loc(l,s,...)   PFcore_error_loc ((l), (s), __VA_ARGS__)
       
#endif 

/* vim:set shiftwidth=4 expandtab: */
