/* -*- c-basic-offset:4; c-indentation-style:"k&r"; indent-tabs-mode:nil -*- */

/**
 * @file
 *
 * Resolve XML namespaces (NS) in the abstract syntax tree.
 *
 * $Id$
 */

#ifndef NS_H
#define NS_H

#include "pathfinder.h"

/** representation of an XML Namespace */
typedef struct PFns_t PFns_t;

/**
 * Representation of an XML NS:
 * (1) @a ns:  the namespace prefix
 * (2) @a uri: the URI @a ns has been mapped to,
 *             either via an XQuery namespace declaration, e.g.
 *
 *                           declare namespace foo = "http://bar"
 *                
 *             or a namespace declaration (xmlns) attribute, e.g.,
 *
 *                           <a xmlns:foo="http://bar"> ... </a>
 *
 *             or by definition, see the W3C standard documents, e.g.
 *             W3C XQuery, 4.1 (Namespace Declarations):
 *                          
 *                    "xs" |-> "http://www.w3.org/2001/XMLSchema"
 */
struct PFns_t {
  char *ns;    /**< namespace prefix */
  char *uri;   /**< URI this namespace has been mapped to */
};

/*
 * XML NS that are predefined for any query (may be used without
 * prior declaration) in XQuery, see W3C XQuery, 4.1
 */
/** Predefined Namespace 'xml' for any query */
extern PFns_t PFns_xml;
/** Predefined Namespace 'xs' (XML Schema) for any query */
extern PFns_t PFns_xs; 
/** Predefined Namespace 'xsi' (XML Schema Instance) for any query */
extern PFns_t PFns_xsi;

/**
 * XQuery default function namespace (fn:..., this may be overridden 
 * via `default function namespace = "..."')
 * (see W3C XQuery 1.0 and XPath 2.0 Function and Operators, 1.5).
 */
extern PFns_t PFns_fn;

/**
 * XQuery operator namespace (op:...)
 * (see W3C XQuery 1.0 and XPath 2.0 Function and Operators, 1.5).
 */
extern PFns_t PFns_op;

/** 
 * Pathfinder's own internal NS (pf:...).
 */ 
extern PFns_t PFns_pf;

/**
 * Wildcard namespace (used in QNames of the form *:loc)
 */
extern PFns_t PFns_wild;

/** 
 * NS equality (URI-based, then prefix-based)
 */
int PFns_eq (PFns_t, PFns_t);

#endif /* NS_H */


/* vim:set shiftwidth=4 expandtab: */
