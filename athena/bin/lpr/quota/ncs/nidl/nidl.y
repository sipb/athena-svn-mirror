%{

/*
 * ========================================================================== 
 * Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
 * Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
 * Copyright Laws Of The United States.
 *
 * Apollo Computer Inc. reserves all rights, title and interest with respect 
 * to copying, modification or the distribution of such software programs and
 * associated documentation, except those rights specifically granted by Apollo
 * in a Product Software Program License, Source Code License or Commercial
 * License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between Apollo and 
 * Licensee.  Without such license agreements, such software programs may not
 * be used, copied, modified or distributed in source or object code form.
 * Further, the copyright notice must appear on the media, the supporting
 * documentation and packaging as set forth in such agreements.  Such License
 * Agreements do not grant any rights to use Apollo Computer's name or trademarks
 * in advertising or publicity, with respect to the distribution of the software
 * programs without the specific prior written permission of Apollo.  Trademark 
 * agreements may be obtained in a separate Trademark License Agreement.
 * ========================================================================== 
 */



#ifdef DSEE
#include "$(base.idl).h"
#else
#include "base.h"
#endif

#ifdef vms
#  include <types.h>
#else
#  include <sys/types.h>
#endif

#include <stdio.h>
#include <string.h>
#include "idl_base.h"
#include "nametbl.h"
#include "errors.h"
#include "ast.h"
#include "astp.h"
#include "utils.h"
#include "sysdep.h"

#define YYDEBUG 1


extern boolean search_attributes_table ; 
extern boolean search_pas_keywords ;
extern boolean search_c_keywords ;

int yyparse __PROTOTYPE((void));
int yylex __PROTOTYPE((void));

%}


        /*   Declaration of yylval, yyval                   */
%union
{
    NAMETABLE_id_t         y_id ;          /* Identifier           */
    STRTAB_str_t           y_string ;      /* String               */
    long                   y_ival ;        /* Integer constant     */
    binding_t*             y_binding ;     /* Binding node         */
    import_t*              y_import ;      /* Import node          */
    constant_t*            y_constant;     /* Constant node        */
    parameter_t*           y_parameter ;   /* Parameter node       */
    structure_t*           y_type ;        /* Type node            */
    field_t*               y_field ;       /* Field node           */
    variant_t*             y_variant ;     /* Record variant part  */
    structure_t*           y_routine ;     /* Routine node         */
    structure_t*           y_interface ;   /* Interface node       */
    component_t*           y_component ;   /* Union components     */
    tag_t*                 y_tag ;         /* Union tags           */
    char*                  y_comment ;     /* Echoed comment       */
    declarator_t*          y_declarator ;  /* Declarator info      */
    array_index_t*         y_index ;       /* Array index info     */
    uuid_$t                y_uuid ;        /* Universal UID        */
    type_attr_t*           y_type_attr ;   /* Type attribute       */

}

/********************************************************************/
/*                                                                  */
/*          Tokens used by the IDC parser.                          */
/*                                                                  */
/********************************************************************/


/* Keywords                 */

%token ARRAY_KW
%token BITSET_KW
%token BOOLEAN_KW
%token BYTE_KW
%token CHAR_KW
%token CASE_KW
%token COMM_STATUS_KW
%token CONST_KW
%token DOUBLE_KW
%token END_KW
%token FROM_KW
%token FUNCTION_KW
%token IDEMPOTENT_KW
%token IN_KW
%token IMPLICIT_HANDLE_KW
%token IMPORT_KW
%token INCLUDE_KW
%token INTEGER_KW
%token INTEGER8_KW
%token INTEGER32_KW
%token INTEGER64_KW
%token INTERFACE_KW
%token LAST_IS_KW
%token LOCAL_KW
%token MAYBE_KW
%token MAX_IS_KW
%token NIL_KW
%token OF_KW
%token OTHERWISE_KW
%token OUT_KW
%token PORT_KW
%token PROCEDURE_KW
%token RECORD_KW   
%token REAL_KW
%token REMOTE_KW
%token SET_KW 
%token STRING0_KW 
%token TAG_IS_KW
%token TYPE_KW
%token UUID_KW 
%token UNION_KW            
%token UNSIGNED_KW
%token UNSIGNED8_KW
%token UNSIGNED32_KW
%token UNSIGNED64_KW
%token VERSION_KW
%token HYPER_KW
%token LONG_KW
%token SHORT_KW
%token FLOAT_KW
%token VOID_KW
%token SECURE_KW
%token SMALL_KW
%token SWITCH_KW
%token TYPEDEF_KW
%token STRUCT_KW
%token ENUM_KW
%token INT_KW
%token REF_KW
%token VAL_KW
%token OPTION_KW
%token DREP_T_KW
%token HANDLE_T_KW
%token HANDLE_KW
%token UUID_REP
%token TRANSMIT_AS_KW
%token TRUE_KW
%token FALSE_KW
%token BROADCAST_KW
%token UNIV_PTR_KW

/*  Punctuation             */

%token COLON
%token COMMA
%token DOTDOT
%token EQUAL
%token HAT
%token LBRACE
%token LBRACKET
%token LPAREN
%token RBRACE
%token RBRACKET
%token RPAREN
%token SEMI
%token STAR
%token AMPER

/*  Tokens setting yylval   */

%token <y_id>      IDENTIFIER
%token <y_string>  STRING
%token <y_ival>    INTEGER_NUMERIC

%start interface

%%

/********************************************************************/
/*                                                                  */
/*          Syntax description and actions for IDL                  */
/*                                                                  */
/********************************************************************/

interface:
        interface_attributes INTERFACE_KW IDENTIFIER interface_tail
        {
            structure_t*    interface_node_ptr ;
            binding_t*      binding_node_ptr ;                              
            
            interface_node_ptr = $<y_interface>4 ;
            AST_set_interface_attributes(&interface_node_ptr->interface);
            binding_node_ptr = AST_binding_node($<y_id>3, interface_k, interface_node_ptr) ;
            the_interface    = binding_node_ptr ;
        }
    ;

interface_tail:
        pascal_interface_tail
    |   c_interface_tail
    ;

pascal_interface_tail:
        pas_interface_marker interface_body pas_interface_close
        { $<y_interface>$ = $<y_interface>2; }
    ;

pas_interface_marker:
        SEMI
        { search_pas_keywords = true ; }
    ;

pas_interface_close:
        END_KW SEMI
    ;    

interface_attributes: 
        attribute_opener interface_attr_list attribute_closer
    |   /* Nothing */
    ;

attribute_opener:
        LBRACKET
        { search_attributes_table = true ; }
    ;

attribute_closer:
        RBRACKET
        { search_attributes_table = false ; }
    ;

interface_attr_list:
        interface_attr
    |   interface_attr_list COMMA interface_attr
    ;

interface_attr:
        IMPLICIT_HANDLE_KW LPAREN IDENTIFIER COLON builtin_type_exp RPAREN
        {                                                  
             structure_t *type_node_p ;
             interface_implicit_handle_var  = $<y_id>3 ;
             type_node_p = $<y_type>5;
             interface_implicit_handle_type = &type_node_p -> type ;
        }
    |   IMPLICIT_HANDLE_KW LPAREN c_simple_type_spec IDENTIFIER  RPAREN
        { 
             structure_t *type_node_p ;
             interface_implicit_handle_var  = $<y_id>4 ;
             type_node_p =  $<y_type>3;
             interface_implicit_handle_type = &type_node_p -> type ;
        }
    |   UUID_KW UUID_REP
        { 
            interface_uuid = $<y_uuid>2;
        }
    |   PORT_KW LPAREN port_list RPAREN
        { 
        }
    |   VERSION_KW LPAREN INTEGER_NUMERIC RPAREN
        { 
            interface_version = $<y_ival>3 ;
        }
    |   LOCAL_KW
        {
            interface_is_local = true ;
        }
    ;

port_list: 
        port_spec
    |   port_list COMMA port_spec
    ;

port_spec:
        IDENTIFIER COLON LBRACKET INTEGER_NUMERIC RBRACKET
        {
            if (interface_port_count > socket_$num_families)
                error("Too many ports\n") ;
            interface_ports[interface_port_count].family_name = $<y_id>1;
            interface_ports[interface_port_count].port_number = $<y_ival>4;
            interface_port_count++ ;
        }
    ;

interface_body:
        exports
        { 
            $<y_interface>$ = AST_interface_node((import_t *) NULL, $<y_binding>1 ) ;
        }        
    |   imports exports
        { 
            $<y_interface>$ = AST_interface_node($<y_import>1, $<y_binding>2) ;
        }
    |   /* nothing */
        {
            $<y_interface>$ = AST_interface_node((import_t *)NULL, (binding_t *)NULL) ;
        }
    ;

imports:            
        import 
    |   imports import
        {
            $<y_import>$ = AST_concat_imports($<y_import>1,  $<y_import>2); 
        }
    ;

import:
        IMPORT_KW import_list SEMI
        { 
            $<y_import>$ = $<y_import>2 ;
        }
    ;

import_list:
        import
    |   import_list COMMA import
        { 
             $<y_import>$ = AST_concat_imports($<y_import>1, $<y_import>3); 
        }
    ;

import:
        STRING
        { 
            $<y_import>$ = AST_import_node($<y_string>1) ;
        }
    ;

exports:            
        export
    |   exports export 
        {
            $<y_binding>$ = AST_concat_exports($<y_binding>1, $<y_binding>2) ;
        }
    ;
     
export:
        CONST_KW const_defs
        { $<y_binding>$ = $<y_binding>2 ;}
    |   TYPE_KW  type_defs  
        { $<y_binding>$ = $<y_binding>2 ;}
    |   proc_def            
    |   func_def 
    ;

const_defs:
        const_def 
    |   const_defs const_def
        {
            $<y_binding>$ = AST_concat_exports($<y_binding>1, $<y_binding>2) ;
        }
    ;

const_def:
        IDENTIFIER EQUAL const_exp  SEMI
        {             
            $<y_binding>$ = AST_binding_node ($<y_id>1, constant_k, (structure_t *) $<y_constant>3) ;
        }    
    ;

const_exp:    
        INTEGER_NUMERIC 
        { 
            constant_t*     constant_node_ptr ;

            constant_node_ptr = (constant_t *) alloc(sizeof(constant_t)) ;

            constant_node_ptr->kind          = integer_k ;
            constant_node_ptr->value.int_val = $<y_ival>1 ;
            $<y_constant>$                   = constant_node_ptr ;
        }
    |   IDENTIFIER 
        {
            $<y_constant>$ = AST_named_const($<y_id>1) ;
        }
    |   STRING 
        { 
            constant_t*     constant_node_ptr ;

            constant_node_ptr = (constant_t *) alloc(sizeof(constant_t)) ;

            constant_node_ptr->kind             = string_k ;
            constant_node_ptr->value.string_val = $<y_string>1 ;
            $<y_constant>$                      = constant_node_ptr ;
        }
    |   NIL_KW
        {
            $<y_constant>$ = AST_constant_node(nil_k) ;
        }

    |   TRUE_KW
        {
            $<y_constant>$     = AST_boolean_constant(true) ;
        }

    |   FALSE_KW
        {
            $<y_constant>$     = AST_boolean_constant(false) ;
        }
    ;

type_defs:
        type_def
    |   type_defs type_def
        { 
            $<y_binding>$ = AST_concat_exports($<y_binding>1, $<y_binding>2) ;
        }
    ;

type_def: 
        IDENTIFIER EQUAL attributed_type SEMI 
        { 
            $<y_binding>$ = AST_binding_node($<y_id>1, type_k, $<y_type>3) ;
        }
    ;

attributed_type:   
        type_attribute_list  type_exp 
        { 
            structure_t*  tnp = $<y_type>2 ;
            $<y_type>$    = AST_set_type_attrs(tnp, $<y_type_attr>1) ;
        }
    |   type_exp 
    ;

type_attribute_list:
        attribute_opener type_attributes attribute_closer
        { $<y_type_attr>$ = $<y_type_attr>2;}
    ;

type_attributes:
        type_attribute 
    |   type_attributes COMMA type_attribute 
        { $<y_type_attr>$ = AST_concat_type_attrs($<y_type_attr>1, $<y_type_attr>3); }
    ;

type_attribute:
        LAST_IS_KW LPAREN IDENTIFIER RPAREN 
        { 
            type_attr_t * attr_node_p ;
            attr_node_p = AST_attribute_node(last_is_name);
            attr_node_p -> the_attr.last_is = $<y_id>3;
            $<y_type_attr>$ = attr_node_p ;
        }
    |   MAX_IS_KW LPAREN IDENTIFIER RPAREN
        { 
            type_attr_t * attr_node_p ;
            attr_node_p = AST_attribute_node(max_is_name);
            attr_node_p -> the_attr.max_is = $<y_id>3;
            $<y_type_attr>$ = attr_node_p ;
        }
    |   HANDLE_KW
        { 
            type_attr_t * attr_node_p ;
            attr_node_p = AST_attribute_node(is_handle);
            $<y_type_attr>$ = attr_node_p ;
        }
    |   TRANSMIT_AS_KW LPAREN IDENTIFIER RPAREN
        { 
            type_attr_t * attr_node_p ;
            attr_node_p = AST_attribute_node(xmit_name);
            attr_node_p -> the_attr.xmit_type_name = $<y_id>3;
            $<y_type_attr>$ = attr_node_p; ;
        }
    ;

type_exp:
       simple_type_exp
    |  structured_type_exp 
    ;

simple_type_exp:   
        builtin_type_exp
    |   enumerated_type_exp
    |   subrange_type_exp
    ;                   

builtin_type_exp:
        BOOLEAN_KW
        { $<y_type>$ = AST_type_node(boolean_k) ; }
    |   BYTE_KW
        { $<y_type>$ = AST_type_node(byte_k) ; }
    |   CHAR_KW
        { $<y_type>$ = AST_type_node(character_k) ; }
    |   INTEGER_KW
        { $<y_type>$ = AST_type_node(short_integer_k) ; }
    |   INTEGER8_KW
        { $<y_type>$ = AST_type_node(small_integer_k) ; }
    |   INTEGER32_KW
        { $<y_type>$ = AST_type_node(long_integer_k) ; }
    |   INTEGER64_KW
        { $<y_type>$ = AST_type_node(hyper_integer_k) ; }
    |   UNSIGNED_KW
        { $<y_type>$ = AST_type_node(short_unsigned_k) ; }
    |   UNSIGNED8_KW
        { $<y_type>$ = AST_type_node(small_unsigned_k) ; }
    |   UNSIGNED32_KW
        { $<y_type>$ = AST_type_node(long_unsigned_k) ; }
    |   UNSIGNED64_KW
        { $<y_type>$ = AST_type_node(hyper_unsigned_k) ; }
    |   REAL_KW
        { $<y_type>$ = AST_type_node(short_real_k) ; }
    |   DOUBLE_KW
        { $<y_type>$ = AST_type_node(long_real_k) ; }   
    |   HANDLE_T_KW
        { $<y_type>$ = AST_type_node(handle_k) ; }
    |   DREP_T_KW
        { $<y_type>$ = AST_type_node(drep_k) ; }
    |   IDENTIFIER
        { $<y_type>$ = AST_named_type($<y_id>1) ; }
    |   UNIV_PTR_KW
        { $<y_type>$ = AST_type_node(univ_ptr_k) ; }

    ;

enumerated_type_exp: 
    |   LPAREN enum_ids RPAREN
        {                                 
             $<y_type>$ = AST_enumerator_node($<y_binding>2, short_enumeration_k) ;
        }
    ;    

enum_ids:
        enum_id
    |   enum_ids COMMA enum_id
        {
            $<y_binding>$ = AST_concat_exports($<y_binding>1, $<y_binding>3) ;
        }
    ;

enum_id:
        IDENTIFIER  
        {
            $<y_binding>$  = AST_enum_constant_binding($<y_id>1) ;
        }
    ;                                       
subrange_type_exp:
        const_exp DOTDOT const_exp 
        {                                  
            $<y_type>$ = AST_subrange_node($<y_constant>1, $<y_constant>3);
        }
    ;

structured_type_exp:
        open_array_type_exp
    |   fixed_array_type_exp
    |   ptr_type_exp
    |   record_type_exp
    |   set_type_exp
    |   proc_ptr_type_exp 
    |   func_ptr_type_exp
    |   string0_type_exp 
    ;

open_array_type_exp:
        ARRAY_KW LBRACKET open_array_index RBRACKET OF_KW type_exp 
        {
            structure_t*    element_type_p ;
            constant_t*     lower_bound_p ;

            element_type_p = $<y_type>6 ;
            lower_bound_p  = $<y_constant>3 ;
            $<y_type>$     = AST_open_array_node(lower_bound_p,
                                                    (array_index_t *)NULL,
                                                    &element_type_p->type);
        }
    |   ARRAY_KW LBRACKET open_array_index COMMA fixed_array_indices RBRACKET OF_KW type_exp 
        {
            structure_t*    element_type_p ;
            array_index_t*  indices_p;
            constant_t*     lower_bound_p ;

            element_type_p = $<y_type>8 ;
            indices_p      = $<y_index>5 ;
            lower_bound_p  = $<y_constant>3 ;
            $<y_type>$     = AST_open_array_node(lower_bound_p,
                                                    indices_p,
                                                    &element_type_p->type);
        }
    ;

fixed_array_type_exp:
        ARRAY_KW LBRACKET fixed_array_indices RBRACKET OF_KW type_exp 
        {                               
            structure_t*    element_type_p ;
            array_index_t*  indices_p ;

            element_type_p = $<y_type>6 ;
            indices_p      = $<y_index>3 ;
            $<y_type>$     = AST_fixed_array_node(indices_p,
                                                  &element_type_p->type) ;
        }
    ; 

open_array_index:
        const_exp DOTDOT STAR
        {
        $<y_constant>$ = $<y_constant>1 ;
        }
    ;

fixed_array_indices:
        fixed_array_index

    |   fixed_array_indices COMMA fixed_array_index
        {
        $<y_index>$ = AST_concat_indices ($<y_index>1, $<y_index>3);
        }
    ;

fixed_array_index:
        simple_type_exp
        {
        $<y_index>$ = AST_index_node ((type_t *) $<y_type>1, false);
        }
    ;

ptr_type_exp:
        HAT simple_type_exp
        {                    
            structure_t*    type_node_p ;
            type_node_p = $<y_type>2 ;
            $<y_type>$  = AST_pointer_node(&type_node_p->type) ;
        }
    ;

set_type_exp:
        SET_KW OF_KW simple_type_exp
        {
            structure_t*    type_node_p ;
            type_node_p = $<y_type>3 ;
            $<y_type>$  = AST_bitset_node(&type_node_p->type, true, true) ;
        }
    ;

record_type_exp:
        RECORD_KW record_body END_KW
        { $<y_type>$ = $<y_type>2 ;}
    ;
record_body:
        field_specs
        { $<y_type>$ = AST_record_node($<y_field>1, (variant_t *)NULL); }
    |   field_specs SEMI 
        { $<y_type>$ = AST_record_node($<y_field>1, (variant_t *)NULL); }
    |   field_specs SEMI variant_dcl
        { 
            $<y_type>$ = AST_record_node($<y_field>1, $<y_variant>3);
        }
    |   variant_dcl 
        { 
            $<y_type>$ = AST_record_node((field_t *)NULL, $<y_variant>1);
        } 
    ;

id_colon_frob:
        IDENTIFIER COLON
        {
        $<y_id>$ = $<y_id>1;
        }
    ;

variant_dcl:
        CASE_KW IDENTIFIER COLON simple_type_exp OF_KW union_components 
        {
            structure_t*    type_node_p ;
            type_node_p = $<y_type>4 ;

            $<y_variant>$  = AST_variant_node(NAMETABLE_NIL_ID,
                                              $<y_id>2,
                                              &type_node_p->type,
                                              $<y_component>6) ;
        }
    | id_colon_frob CASE_KW IDENTIFIER COLON simple_type_exp OF_KW union_components 
        {
            structure_t*    type_node_p ;
            type_node_p = $<y_type>5 ;

            $<y_variant>$ = AST_variant_node($<y_id>1,
                                             $<y_id>3,
                                             &type_node_p->type,
                                             $<y_component>7) ;
        }
    ;

field_specs:
        field_spec 
    |   field_specs SEMI field_spec 
        {
            $<y_field>$ = AST_concat_field_nodes($<y_field>1, $<y_field>3) ;
        }
    ;

field_spec:
        field_id_list attributed_type 
        {                     
           structure_t* tnp  = $<y_type>2 ;
           $<y_field>$ = AST_propagate_field_type($<y_field>1,&tnp->type) ;
        }
    |   field_attrs field_id_list attributed_type 
        {                     
           structure_t* tnp  = $<y_type>2 ;
           $<y_field>$ = AST_propagate_field_type($<y_field>1,&tnp->type) ;
        }
    ;

field_attrs:
        attribute_opener field_attr_list attribute_closer
    ;

field_attr_list:
        field_attr
    |   field_attr_list COMMA field_attr
    ;

field_attr:
        LAST_IS_KW LPAREN IDENTIFIER RPAREN 
        { 
            last_is_of_field = $<y_id>3;
        }

    |   MAX_IS_KW LPAREN IDENTIFIER RPAREN 
        { 
            max_is_of_field = $<y_id>3;
        }
    ;


field_id_list:
        id_colon_frob
        {                     
           $<y_field>$ = AST_field_node($<y_id>1) ;
        }
    |   field_id COMMA field_id_list 
        {
            $<y_field>$ = AST_concat_field_nodes($<y_field>1, $<y_field>3) ;
        }
    ;

field_id:
        IDENTIFIER
        { 
            $<y_field>$ = AST_field_node($<y_id>1) ;
        }
    ;

union_components:
        union_component    
    |   union_components union_component
        {  
            $<y_component>$ = AST_concat_components($<y_component>1, $<y_component>2);
        }
    ;

union_component:
        union_tag COLON LPAREN field_specs RPAREN SEMI
        {
            $<y_component>$ = AST_component_node(NAMETABLE_NIL_ID, $<y_tag>1, $<y_field>4) ;
        }
    |   union_tag COLON LPAREN field_specs SEMI RPAREN SEMI
        {
            $<y_component>$ = AST_component_node(NAMETABLE_NIL_ID, $<y_tag>1, $<y_field>4) ;
        }
    |   union_tag COLON LPAREN RPAREN SEMI 
        {
            $<y_component>$ = AST_component_node(NAMETABLE_NIL_ID, $<y_tag>1, (field_t *)NULL) ;
        }
    |   union_tag COLON IDENTIFIER COLON LPAREN field_specs RPAREN SEMI
        {
            $<y_component>$ = AST_component_node($<y_id>3, $<y_tag>1, $<y_field>6) ;
        }
    |   union_tag COLON IDENTIFIER COLON LPAREN field_specs SEMI RPAREN SEMI
        {
            $<y_component>$ = AST_component_node($<y_id>3, $<y_tag>1, $<y_field>6) ;
        }
    |   union_tag COLON IDENTIFIER COLON LPAREN RPAREN SEMI 
        {
            $<y_component>$ = AST_component_node($<y_id>3, $<y_tag>1, (field_t *)NULL) ;
        }

    ;

union_tag:
        tag
    |   union_tag COMMA tag 
        {
            $<y_tag>$ = AST_concat_tags($<y_tag>1, $<y_tag>3);
        }
    ;

tag:
        const_exp
        {
            $<y_tag>$ = AST_tag_node($<y_constant>1) ;
        }
    ;
proc_ptr_type_exp:
        HAT PROCEDURE_KW parameter_list 
    {
           structure_t*     void_type_node  = AST_type_node(void_k) ;

        $<y_type>$ = AST_routine_ptr_node($<y_parameter>3, &void_type_node->type);
    }
    |   HAT PROCEDURE_KW 
    {
           structure_t*     void_type_node  = AST_type_node(void_k) ;

        $<y_type>$ = AST_routine_ptr_node((parameter_t *)NULL, &void_type_node->type);
    }
    ;



func_ptr_type_exp:
        HAT FUNCTION_KW parameter_list COLON type_exp 
    {
           structure_t*     result_type_p  = $<y_type>5;
        $<y_type>$ = AST_routine_ptr_node($<y_parameter>3, &result_type_p->type);
    }
    ; 


string0_type_exp:
        STRING0_KW LBRACKET const_exp RBRACKET
        {   $<y_type>$ = AST_fixed_string_zero_node($<y_constant>3) ; }
    ; 


proc_def:
        proc_header IDENTIFIER SEMI proc_options
        {  
           structure_t*     routine_node_p ;
           structure_t*     void_type_node  = AST_type_node(void_k) ;

           routine_node_p   = AST_routine_node((parameter_t *)NULL, $<y_id>2, &void_type_node->type);
           (void) AST_set_routine_attrs(&routine_node_p->routine) ;
           $<y_binding>$    = AST_binding_node($<y_id>2, routine_k, routine_node_p);
        }
    |   proc_header IDENTIFIER parameter_list SEMI proc_options
        { 
           structure_t*     routine_node_p ;
           structure_t*     void_type_node  = AST_type_node(void_k) ;
           routine_node_p   = AST_routine_node($<y_parameter>3, $<y_id>2, &void_type_node->type);
           (void) AST_set_routine_attrs(&routine_node_p->routine) ;
           $<y_binding>$    = AST_binding_node($<y_id>2, routine_k, routine_node_p);
        }
    ;

proc_header:
        routine_attribute_list PROCEDURE_KW
    |   PROCEDURE_KW
    ;

proc_options:
        list_directed_options
    |   option_directed_options
    |   /* nothing */
    ;

list_directed_options:
        list_option_element 
    |   list_directed_options list_option_element
    ;

list_option_element:
        IDENTIFIER SEMI 
    ;

option_directed_options:
        OPTION_KW LPAREN options_list RPAREN SEMI
    ;

options_list:
        IDENTIFIER
    |   options_list COMMA IDENTIFIER
    ;

func_def:
        function_header IDENTIFIER COLON type_exp SEMI
        {  
           structure_t*     result_type_p  = $<y_type>4;
           structure_t*     routine_node_p ;

           routine_node_p            = AST_routine_node((parameter_t *) NULL, 
                                                        $<y_id>2,
                                                        &result_type_p->type);

           (void) AST_set_routine_attrs(&routine_node_p->routine) ;
           $<y_binding>$ = AST_binding_node($<y_id>2, routine_k, routine_node_p);
        }
    |   function_header IDENTIFIER parameter_list COLON type_exp SEMI
        {  
           structure_t*     result_type_p  = $<y_type>5;
           structure_t*     routine_node_p ;

           routine_node_p            = AST_routine_node($<y_parameter>3 , 
                                                        $<y_id>2,
                                                        &result_type_p->type);

           (void) AST_set_routine_attrs(&routine_node_p->routine) ;
           $<y_binding>$ = AST_binding_node($<y_id>2, routine_k, routine_node_p);
        }
    ;

function_header:
        routine_attribute_list FUNCTION_KW ;
    |   FUNCTION_KW
    ;

routine_attribute_list:
        attribute_opener routine_attributes attribute_closer
    ;

routine_attributes:
        routine_attribute
    |   routine_attributes COMMA routine_attribute 
    ;
     
routine_attribute:
        SECURE_KW
        { routine_secure = true ;}
    |   IDEMPOTENT_KW
        { routine_idempotent = true ;}
    |   MAYBE_KW
        { routine_maybe = true ;}
    |   BROADCAST_KW
        { routine_broadcast = true ;}
    ;

parameter_list:
        LPAREN parameters RPAREN
        { 
            $<y_parameter>$ = $<y_parameter>2 ;
        }
    ;

parameters:
        parameter_spec
    |   parameters SEMI parameter_spec
        {   
            $<y_parameter>$ = AST_concat_parameters($<y_parameter>1, $<y_parameter>3) ;
        }
    |   /* nothing */
        { $<y_parameter>$ = (parameter_t *)NULL; }
    ;

parameter_spec:
        parameter_ids COLON attributed_type
        { 
            structure_t* tnp = $<y_type>3 ;
            (void) AST_propagate_param_attrs($<y_parameter>1, &tnp->type) ;
            $<y_parameter>$ = AST_propagate_param_type($<y_parameter>1, &tnp->type) ;
        }
    ;

parameter_ids:
        parameter_attrs parameter_id_list 
        {
            $<y_parameter>$ = $<y_parameter>2 ;
        }
    ;

parameter_id_list:
        parameter_id 
    |   parameter_id_list COMMA parameter_id
        { 
            $<y_parameter>$ = AST_concat_parameters($<y_parameter>1, $<y_parameter>3) ;
        }
    ;

parameter_id:
        IDENTIFIER 
        { 
            $<y_parameter>$ = AST_parameter_node($<y_id>1) ;
        }
        
    ;

parameter_attrs:
        parameter_class attribute_opener parameter_attr_list attribute_closer
    |   parameter_class
    ;

parameter_attr_list:
        parameter_attr
    |   field_attr
    |   parameter_attr_list COMMA parameter_attr
    |   parameter_attr_list COMMA field_attr
    ;

parameter_attr:
        COMM_STATUS_KW
        {
            comm_status_parameter = true;
        }

    |   IN_KW
        {
            in_parameter = true;
        }

    |   OUT_KW
        {
            out_parameter = true;
        }

    |   REF_KW
        {
            ref_parameter = true;
        }
    ;

parameter_class:
        IN_KW REF_KW
        {
            in_parameter = ref_parameter = true;
        }
         
    |   IN_KW OUT_KW
        {
            in_parameter = out_parameter = true;
        }

    |   IN_KW
        {
            in_parameter = true;
        }

    |   OUT_KW
        {
            out_parameter = true;
        }

    ;
  
/**********************************************************************/

c_interface_tail:          
        c_interface_marker c_interface_body c_interface_close
        { $<y_interface>$ = $<y_interface>2; }
    ;

c_interface_marker:
        LBRACE
        { search_c_keywords   = true ;
          search_pas_keywords = false ; }
    ;

c_interface_close:            
        RBRACE
        { search_c_keywords   = false ;
          search_pas_keywords = true  ;}
    ;     

c_attribute_opener:
        LBRACKET
        { search_attributes_table = true ;}
    ;

c_attribute_closer:
        RBRACKET
        { search_attributes_table = false ;}
    ;

c_interface_body:
        c_exports
        { 
            $<y_interface>$ = AST_interface_node((import_t *) NULL, $<y_binding>1 ) ;
        }
    |   c_imports c_exports 
        { 
            $<y_interface>$ = AST_interface_node($<y_import>1, $<y_binding>2) ; 
        }
      |   /* nothing */
        {
            $<y_interface>$ = AST_interface_node((import_t *)NULL, (binding_t *)NULL) ;
        }
  ;

c_imports:  
        c_import
    |   c_imports c_import
        { 
             $<y_import>$ = AST_concat_imports($<y_import>1, $<y_import>2); 
        }
    ;

c_import:
        IMPORT_KW STRING SEMI
        { 
            $<y_import>$ = AST_import_node($<y_string>2) ;
        }
    ;

c_exports:
        c_export 
    |   c_exports  c_export
        {
            $<y_binding>$ = AST_concat_exports($<y_binding>1, $<y_binding>2) ;
        }

    ;
        
c_export:
        c_type_dcl      SEMI
    |   c_const_dcl     SEMI 
    |   c_operation_dcl SEMI 
    ;

c_const_dcl:
        CONST_KW c_type_spec IDENTIFIER EQUAL c_const_exp
        { $<y_binding>$ = AST_binding_node($<y_id>3, constant_k, (structure_t *)$<y_constant>5) ; }
    ;


c_const_exp:    
        INTEGER_NUMERIC 
        { 
            constant_t*     constant_node_ptr ;

            constant_node_ptr = (constant_t *) alloc(sizeof(constant_t)) ;

            constant_node_ptr->kind          = integer_k ;
            constant_node_ptr->value.int_val = $<y_ival>1 ;
            $<y_constant>$                   = constant_node_ptr ;
        }
    |   IDENTIFIER 
        {
            constant_t*     constant_node_ptr ;

            constant_node_ptr = AST_named_const($<y_id>1) ;
            $<y_constant>$             = constant_node_ptr ;
        }
    |   STRING 
        { 
            constant_t*     constant_node_ptr ;

            constant_node_ptr = (constant_t *) alloc(sizeof(constant_t)) ;

            constant_node_ptr->kind             = string_k ;
            constant_node_ptr->value.string_val = $<y_string>1 ;
            $<y_constant>$                      = constant_node_ptr ;
        }
    |   NIL_KW
        {
            $<y_constant>$ = AST_constant_node(nil_k) ;
        }
        
    |   TRUE_KW
        {
            $<y_constant>$  = AST_boolean_constant(true) ;
        }

    |   FALSE_KW
        {
            $<y_constant>$  = AST_boolean_constant(false) ;
        }
    ;

c_type_dcl:
        TYPEDEF_KW c_type_declarator
        {
           $<y_binding>$      = AST_declarators_to_bindings($<y_declarator>2)  ;
        }
    ;

c_type_declarator:
        c_attributed_type_spec c_declarators
        {                           
            structure_t*    type_node_ptr ;
        
            type_node_ptr = $<y_type>1 ;
            $<y_declarator>$  = AST_prop_declarator_type(&type_node_ptr->type, $<y_declarator>2) ;
        }

    |   c_type_spec c_declarators
        { 
            structure_t*    type_node_ptr ;
        
            type_node_ptr = $<y_type>1 ;
            $<y_declarator>$  = AST_prop_declarator_type(&type_node_ptr->type, $<y_declarator>2) ;
        }
    ;
     

c_attributed_type_spec:
        c_attribute_opener c_type_attributes c_attribute_closer c_type_spec 
        {    $<y_type>$ = AST_set_type_attrs($<y_type>4, $<y_type_attr>2) ;}
    ;

c_type_attributes:
        c_type_attribute 
    |   c_type_attributes COMMA c_type_attribute 
        { $<y_type_attr>$ = AST_concat_type_attrs($<y_type_attr>1, $<y_type_attr>3); }
    ;

c_type_attribute:              
        LAST_IS_KW LPAREN IDENTIFIER RPAREN 
        { 
            type_attr_t * attr_node_p ;
            attr_node_p = AST_attribute_node(last_is_name);
            attr_node_p -> the_attr.last_is = $<y_id>3;
            $<y_type_attr>$ = attr_node_p ;
        }
    |   MAX_IS_KW LPAREN IDENTIFIER RPAREN 
        { 
            type_attr_t * attr_node_p ;
            attr_node_p = AST_attribute_node(max_is_name);
            attr_node_p -> the_attr.max_is = $<y_id>3;
            $<y_type_attr>$ = attr_node_p ;
        }
    |   HANDLE_KW
        { 
            type_attr_t * attr_node_p ;
            attr_node_p = AST_attribute_node(is_handle);
            $<y_type_attr>$ = attr_node_p ;
        }
    |   TRANSMIT_AS_KW LPAREN IDENTIFIER RPAREN
        { 
            type_attr_t * attr_node_p ;
            attr_node_p = AST_attribute_node(xmit_name);
            attr_node_p -> the_attr.xmit_type_name = $<y_id>3;
            $<y_type_attr>$ = attr_node_p; ;
        }
    ;


c_type_spec:
        c_simple_type_spec
    |   c_constructed_type_spec
    ;

c_simple_type_spec:
        c_floating_point_type_spec
    |   c_integer_type_spec            
    |   c_char_type_spec
    |   c_boolean_type_spec
    |   c_byte_type_spec
    |   c_void_type_spec
    |   c_named_type_spec
    |   c_handle_type_spec
    |   c_drep_type_spec
    ;

c_constructed_type_spec:       
        c_struct_type_spec
    |   c_union_type_spec 
    |   c_enum_type_spec
    |   c_set_type_spec
    |   c_string0_type_spec
    ;

c_named_type_spec:
        IDENTIFIER
        { $<y_type>$ = AST_named_type($<y_id>1) ; }  
    ;

c_floating_point_type_spec:
        FLOAT_KW
        { $<y_type>$ = AST_type_node(short_real_k); }
    |   DOUBLE_KW
        { $<y_type>$ = AST_type_node(long_real_k); }
    ;
     
c_integer_size_spec:
        SMALL_KW
        { int_size = small_int; }
    |   SHORT_KW
        { int_size = short_int; }
    |   LONG_KW
        { int_size = long_int; }
    |   HYPER_KW
        { int_size = hyper_int; }
    ;

c_integer_modifiers:
        c_integer_size_spec
    |   UNSIGNED_KW
        { int_signed = false; }
    |   UNSIGNED_KW c_integer_size_spec
        { int_signed = false; }
    |   c_integer_size_spec UNSIGNED_KW
        { int_signed = false; }
    ;

c_integer_type_spec:
        INT_KW
        { $<y_type>$ = AST_integer_type_node(); }
    |   c_integer_modifiers
        { $<y_type>$ = AST_integer_type_node(); }
    |   c_integer_modifiers INT_KW
        { $<y_type>$ = AST_integer_type_node(); }
    ;

c_char_type_spec:
        CHAR_KW
        { $<y_type>$ = AST_type_node(character_k) ;} 
    ;

c_boolean_type_spec:
        BOOLEAN_KW
        { $<y_type>$ = AST_type_node(boolean_k) ;}
    ;

c_byte_type_spec:
        BYTE_KW
        { $<y_type>$ = AST_type_node(byte_k) ;} 
    ;

c_void_type_spec:
        VOID_KW 
        { $<y_type>$ = AST_type_node(void_k) ;}
    ;

c_drep_type_spec:
        DREP_T_KW
        { $<y_type>$ = AST_type_node(drep_k) ; }
    ;

c_handle_type_spec:
       HANDLE_T_KW
        { $<y_type>$ = AST_type_node(handle_k) ; }
    ;

c_struct_type_spec:
        STRUCT_KW c_struct_body 
        { $<y_type>$ = $<y_type>2 ;}
    ;

c_struct_body:
        LBRACE c_member_list RBRACE
        { $<y_type>$ = AST_record_node($<y_field>2, (variant_t *)NULL) ; }
    ;

c_union_type_spec:
        c_union_header LBRACE c_union_body RBRACE
        {
            structure_t*    record_node_ptr ;

            record_node_ptr = $<y_type>1 ;
            record_node_ptr->type.type_structure.record.variant->components = $<y_component>3; 
        }
    ;

c_union_header:
        UNION_KW SWITCH_KW LPAREN c_simple_type_spec IDENTIFIER RPAREN
        {
        $<y_type>$ = AST_record_node((field_t *)NULL, AST_variant_node (NAMETABLE_NIL_ID, $<y_id>5, (type_t *)$<y_type>4, (component_t *)NULL));
        }
    |   UNION_KW SWITCH_KW LPAREN c_simple_type_spec IDENTIFIER RPAREN IDENTIFIER
        {
        $<y_type>$ = AST_record_node((field_t *)NULL, AST_variant_node ($<y_id>7, $<y_id>5, (type_t *)$<y_type>4, (component_t *)NULL));
        }
    ;

c_union_body:
        c_union_case 
    |   c_union_body c_union_case
        { $<y_component>$ = AST_concat_components($<y_component>1, $<y_component>2);}
    ;

c_union_case:                                                                             
        c_union_case_list c_member
        {
        $<y_component>$ = AST_component_node($<y_field>2->name, $<y_tag>1, $<y_field>2) ;
        }
    ;
                          
c_union_case_list:
        c_union_case_tag
    |   c_union_case_list c_union_case_tag
        { $<y_tag>$ = AST_concat_tags($<y_tag>1, $<y_tag>2) ; }
    ;

c_union_case_tag:
        CASE_KW c_const_exp COLON
        { $<y_tag>$ = AST_component_tag_node($<y_constant>2) ; }
    ;


c_member_list:
        c_member
    |   c_member_list c_member
        { $<y_field>$ = AST_concat_field_nodes($<y_field>1, $<y_field>2) ; }
    ;

c_member:
        c_attributed_type_spec c_member_attribute_list c_declarators SEMI
        {
            structure_t* member_type_p;

            member_type_p = $<y_type>1;

            (void) AST_prop_declarator_type(&member_type_p->type, $<y_declarator>3);
            $<y_field>$ = AST_declarators_to_fields($<y_declarator>3);
        }
    |   c_attributed_type_spec  c_declarators SEMI
        {
            structure_t* member_type_p;

            member_type_p = $<y_type>1;

            (void) AST_prop_declarator_type(&member_type_p->type, $<y_declarator>2);
            $<y_field>$ = AST_declarators_to_fields($<y_declarator>2);
        }
    |   c_type_spec c_member_attribute_list c_declarators SEMI
        {
            structure_t* member_type_p;

            member_type_p = $<y_type>1;

            (void) AST_prop_declarator_type(&member_type_p->type, $<y_declarator>3);
            $<y_field>$ = AST_declarators_to_fields($<y_declarator>3);
        }
    |   c_type_spec c_declarators SEMI
        {
            structure_t* member_type_p;

            member_type_p = $<y_type>1;

            (void) AST_prop_declarator_type(&member_type_p->type, $<y_declarator>2);
            $<y_field>$ = AST_declarators_to_fields($<y_declarator>2);
        }
    ;

c_member_attribute_list:
        c_attribute_opener c_member_attributes c_attribute_closer
    ;

c_member_attributes:
        c_member_attribute
    |   c_member_attributes COMMA c_member_attribute 
    ;

c_member_attribute:
        LAST_IS_KW LPAREN IDENTIFIER RPAREN 
        { 
            last_is_of_field = $<y_id>3;
        }
    |   MAX_IS_KW LPAREN IDENTIFIER RPAREN 
        { 
            max_is_of_field = $<y_id>3;
        }
    ;

c_enum_type_spec:
        ENUM_KW c_enum_body 
        {                                 
             $<y_type>$ = AST_enumerator_node($<y_binding>2, long_enumeration_k) ;
        }
    |   LONG_KW ENUM_KW c_enum_body
        {
             $<y_type>$ = AST_enumerator_node($<y_binding>3, long_enumeration_k) ;
        }
    |   SHORT_KW ENUM_KW c_enum_body
        {
             $<y_type>$ = AST_enumerator_node($<y_binding>3, short_enumeration_k) ;
        }
    ;

c_enum_body:
        LBRACE c_enum_ids RBRACE
        {
            $<y_binding>$ = $<y_binding>2 ;
        }
    ;

c_enum_ids:
    |   c_enum_id
    |   c_enum_ids COMMA c_enum_id
        {
            $<y_binding>$ = AST_concat_exports($<y_binding>1, $<y_binding>3) ;
        }
    ;

c_enum_id:
        IDENTIFIER
        {
            $<y_binding>$  = AST_enum_constant_binding($<y_id>1) ;
        }
    ;

c_set_type_spec:
        BITSET_KW c_type_spec
        {
            structure_t* type_node_p ;
            type_node_p = $<y_type>2 ;
            $<y_type>$  = AST_bitset_node(&type_node_p->type, false, false) ;
        }
    |   LONG_KW BITSET_KW c_type_spec
        {
            structure_t* type_node_p ;
            type_node_p = $<y_type>3 ;
            $<y_type>$  = AST_bitset_node(&type_node_p->type, false, false) ;
        }
    |   SHORT_KW BITSET_KW c_type_spec
        {
            structure_t* type_node_p ;
            type_node_p = $<y_type>3 ;
            $<y_type>$  = AST_bitset_node(&type_node_p->type, false, true) ;
        }
    ;

c_string0_type_spec:
        STRING0_KW LBRACKET c_const_exp RBRACKET
        {   $<y_type>$ = AST_fixed_string_zero_node($<y_constant>3) ; }
    ;

c_declarators:
        c_declarator
    |   c_declarators COMMA c_declarator 
        { $<y_declarator>$ = AST_concat_declarators($<y_declarator>1, $<y_declarator>3) ;}
    ;
  
c_declarator:  
        c_simple_declarator
    |   c_complex_declarator
    ;

c_simple_declarator:
        IDENTIFIER 
        { $<y_declarator>$ = AST_declarator_node($<y_id>1, void_k, (array_index_t *) NULL) ;}
    ;

c_complex_declarator:
        c_pointer_declarator
    |   c_array_declarator
    |   c_function_ptr_declarator
    |   c_reference_declarator
    ;
                                              
c_pointer_declarator:
        STAR IDENTIFIER 
        { $<y_declarator>$ = AST_declarator_node($<y_id>2, pointer_k, (array_index_t *) NULL) ;}
    |   STAR CONST_KW IDENTIFIER
    |   CONST_KW STAR IDENTIFIER
    ;

c_reference_declarator:
        AMPER IDENTIFIER
        { $<y_declarator>$ = AST_declarator_node($<y_id>2, reference_k, (array_index_t *) NULL) ;}
    |   AMPER CONST_KW IDENTIFIER
    |   CONST_KW AMPER IDENTIFIER
    ;    

c_array_declarator:
        IDENTIFIER LBRACKET RBRACKET
        { $<y_declarator>$ = AST_declarator_node($<y_id>1, open_array_k, (array_index_t *)NULL) ;}

    |   IDENTIFIER LBRACKET STAR RBRACKET
        { $<y_declarator>$ = AST_declarator_node($<y_id>1, open_array_k, (array_index_t *)NULL) ;}

    |   IDENTIFIER LBRACKET RBRACKET c_fixed_array_indices
        { $<y_declarator>$ = AST_declarator_node($<y_id>1, open_array_k, $<y_index>4) ;}

    |   IDENTIFIER LBRACKET STAR RBRACKET c_fixed_array_indices
        { $<y_declarator>$ = AST_declarator_node($<y_id>1, open_array_k, $<y_index>5) ;}

    |    IDENTIFIER c_fixed_array_indices
        { $<y_declarator>$ = AST_declarator_node($<y_id>1, fixed_array_k, $<y_index>2) ;}
    ;

c_fixed_array_indices:
        c_fixed_array_index

    |   c_fixed_array_indices c_fixed_array_index
        {
            $<y_index>$ = AST_concat_indices ($<y_index>1, $<y_index>2);
        }
    ;

c_fixed_array_index:
        LBRACKET const_exp RBRACKET
        {
            $<y_index>$ = AST_index_node_c ($<y_constant>2);
        }
    ;

c_function_ptr_declarator:
        c_function_ptr_hdr c_parameter_dcls
        {   declarator_t    *declarator_p ;
            declarator_p     = $<y_declarator>1 ;
            declarator_p->routine_params = $<y_parameter>2 ;
            $<y_declarator>$ = declarator_p ;
        }
    ;

c_function_ptr_hdr:
        LPAREN STAR IDENTIFIER RPAREN 
        { $<y_declarator>$ = AST_declarator_node($<y_id>3, routine_ptr_k, (array_index_t *) NULL) ;}
    ;

c_operation_dcl:
        c_routine_attribute_list c_simple_type_spec IDENTIFIER  c_parameter_dcls
        {
            structure_t*    routine_node_p ;
            structure_t*    result_p ;

            result_p       = $<y_type>2 ;
            AST_depointerize_params($<y_parameter>4) ;
            routine_node_p = AST_routine_node($<y_parameter>4, $<y_id>3, &result_p->type) ;
            (void) AST_set_routine_attrs(&routine_node_p->routine) ;
            $<y_binding>$  = AST_binding_node($<y_id>3, routine_k, routine_node_p);   
        }
    |   c_simple_type_spec IDENTIFIER  c_parameter_dcls
        {
            structure_t*    routine_node_p ;
            structure_t*    result_p ;

            result_p       = $<y_type>1 ;
            AST_depointerize_params($<y_parameter>3) ;
            routine_node_p = AST_routine_node($<y_parameter>3, $<y_id>2, &result_p->type) ;
            (void) AST_set_routine_attrs(&routine_node_p->routine) ;
            $<y_binding>$  = AST_binding_node($<y_id>2, routine_k, routine_node_p);   
        }
    |   IDENTIFIER  c_parameter_dcls
        {   structure_t*    integer_type_p = AST_type_node(long_integer_k) ;
            structure_t*    routine_node_p ; 
            AST_depointerize_params($<y_parameter>2) ;
            routine_node_p = AST_routine_node($<y_parameter>2, NAMETABLE_NIL_ID, &integer_type_p->type) ;
            (void) AST_set_routine_attrs(&routine_node_p->routine) ;
            $<y_binding>$  = AST_binding_node($<y_id>1, routine_k, routine_node_p);   
        }
    ;

c_routine_attribute_list:
        c_attribute_opener c_routine_attributes c_attribute_closer
    ;

c_routine_attributes:
        c_routine_attribute
    |   c_routine_attributes COMMA c_routine_attribute 
    ;
     
c_routine_attribute:
        SECURE_KW
        { routine_secure = true ;}
    |   IDEMPOTENT_KW
        { routine_idempotent = true ;}
    |   MAYBE_KW
        { routine_maybe = true ;}
    |   BROADCAST_KW
        { routine_broadcast = true ;}
    ;

c_parameter_dcls:
        LPAREN c_param_list RPAREN
        { $<y_parameter>$ = $<y_parameter>2 ; }
    ;

c_param_list:
        c_param_dcl
    |   c_param_list COMMA c_param_dcl
        { $<y_parameter>$ = AST_concat_parameters($<y_parameter>1, $<y_parameter>3) ; }
    |   /* nothing */
        { $<y_parameter>$ = (parameter_t *)NULL; }
    ;

c_param_dcl:
        c_attributed_type_spec c_param_attribute_list c_declarator
        {
            parameter_t*    param_list_p ;
            structure_t*    param_type_p ;
                                          
            param_type_p    = $<y_type>1 ;

	    (void) AST_prop_declarator_type(&param_type_p->type, $<y_declarator>3) ;
            param_list_p    = AST_declarators_to_params($<y_declarator>3) ;
            $<y_parameter>$ = AST_propagate_param_attrs(param_list_p, &param_type_p->type) ;

        }
    |   c_attributed_type_spec  c_declarator
        {
            parameter_t*    param_list_p ;
            structure_t*    param_type_p ;
                                          
            param_type_p    = $<y_type>1 ;

	    (void) AST_prop_declarator_type(&param_type_p->type, $<y_declarator>2) ;
            param_list_p    = AST_declarators_to_params($<y_declarator>2) ;
            $<y_parameter>$ = AST_propagate_param_attrs(param_list_p, &param_type_p->type) ;

        }
    |   c_type_spec c_param_attribute_list c_declarator
        {
            parameter_t*    param_list_p ;
            structure_t*    param_type_p ;
                                        
            param_type_p    = $<y_type>1 ;
	    (void) AST_prop_declarator_type(&param_type_p->type, $<y_declarator>3) ;
            param_list_p    = AST_declarators_to_params($<y_declarator>3) ;
            $<y_parameter>$ = AST_propagate_param_attrs(param_list_p, &param_type_p->type) ;
       }
    ;

c_param_attribute_list:
        c_attribute_opener c_param_attributes c_attribute_closer
    ;

c_param_attributes:
        c_param_attribute
    |   c_member_attribute
    |   c_param_attributes COMMA c_param_attribute 
    |   c_param_attributes COMMA c_member_attribute 
    ;

c_param_attribute:
        IN_KW
        {
            in_parameter    = true ;
            $<y_type_attr>$ = (type_attr_t *)NULL ;
        }

    |   OUT_KW
        {
            out_parameter    = true ;
            requires_pointer = true ;
            $<y_type_attr>$  = (type_attr_t *)NULL ;
        }
    |   COMM_STATUS_KW
        {
            comm_status_parameter = true ;
            $<y_type_attr>$      = (type_attr_t *)NULL ;
        }
    ;
%%
