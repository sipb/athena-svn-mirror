
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



#include <stdio.h>

#if defined(MSDOS) || defined(SYS5) || defined (vaxc) || defined(_AUX_SOURCE)
# include <string.h>
#else
# include <strings.h>
#endif

#ifdef DSEE
# include "$(rpc.idl).h"
#else
# include "rpc.h"
#endif

#include "nametbl.h"
#include "ast.h"
#include "astp.h"
#include "errors.h"
#include "files.h"
#include "backend.h"
#include "sysdep.h"
#include "utils.h"

type_t *depointerize_type __PROTOTYPE((type_t *tp));
static void depointerize_routine __PROTOTYPE((routine_t *rp));
static void pspell_interface_def __PROTOTYPE((FILE *fid, binding_t *bp));
static void pspell_constant_val __PROTOTYPE((FILE *fid, constant_t *cp));
static void pspell_type_exp __PROTOTYPE ((FILE *fid, type_t *tp));
static void name_anonymous_param_types  __PROTOTYPE((FILE *fid, routine_t *rp));
static void unname_anonymous_param_types __PROTOTYPE((void));
static void pspell_constant_def __PROTOTYPE((FILE *fid, binding_t *bp));
static void pspell_type_def __PROTOTYPE((FILE *fid, binding_t *bp));
static void pspell_routine_def __PROTOTYPE((FILE *fid, binding_t *bp));
static void pspell_bindings __PROTOTYPE((FILE *fid, binding_t *bp));    
static void pspell_field __PROTOTYPE((FILE *fid, field_t *fp));
static void pspell_fields __PROTOTYPE((FILE *fid, field_t *fp));
static void pspell_enumerators __PROTOTYPE((FILE *fid, binding_t *bp));
static void pspell_tags __PROTOTYPE((FILE *fid, tag_t *tgp));
static void pspell_parameter_list __PROTOTYPE((FILE *fid, parameter_t *pp));
static void pspell_routine_ptr __PROTOTYPE((FILE *fid, routine_t *rp));
static void pspell_derived_routine_decls __PROTOTYPE ((FILE *fid));
boolean univ __PROTOTYPE((type_t *tp));
static void pspell_function_options __PROTOTYPE ((FILE *fid, type_t *tp));    
static void pspell_implicit_handle_extern  __PROTOTYPE((FILE *fid, NAMETABLE_id_t name, type_t *type));
static void pspell_epv_type_and_vars  __PROTOTYPE((FILE *fid, NAMETABLE_id_t if_name, binding_t *bp, boolean mmmv));
static void pspell_server_epv __PROTOTYPE((FILE *fid, NAMETABLE_id_t if_name, boolean mmmv));
    
type_t *depointerize_type (tp)
type_t      *tp;
{
    type_t *result;

    if (tp->kind == pointer_k) {
        result = tp->type_structure.pointer.pointee;
        free ((char *)tp);
        return result;
        }
    else
        return tp;
}


static
void depointerize_routine (rp)
routine_t *rp;
{
    parameter_t *pp;

    if (rp->pointerized) {
        rp->pointerized = false;
        for (pp = rp->parameters ; pp; pp = pp->next_param)
            if (pp->out || pp->ref) 
                pp->type = depointerize_type (pp->type);
        }
}



static
void pspell_interface_def (fid, bp)
FILE      *fid;
binding_t *bp;
{
    interface_t *ifp;
    int         family;
    int         i;
    boolean     first;

    ifp = (interface_t *) bp->binding;

    /* emit dcl of the binding info var for this interface if it has any routines*/
    if (!ifp->local_only){

        ifp->if_spec_name = NAMETABLE_add_derived_name (bp->name, "%s$if_spec");

        fprintf (fid, "var\n");
        spell_name (fid, ifp->if_spec_name);
        fprintf (fid, " : rpc_$if_spec_t := [\n");

        fprintf (fid, "  vers := %d,\n", ifp->interface_version);

        fprintf (fid, "  port := [");
        first = true;
        for (family = 0; family < socket_$num_families; family++) {
            if (first)
                first = false;
            else
                fprintf (fid, ", ");
            fprintf (fid, "%d", ifp->well_known_ports[family]);
            }
        fprintf (fid, "],\n");

        fprintf (fid, "  opcnt := %d,\n", ifp->op_count);

        fprintf (fid, "  id := [\n");
        fprintf (fid, "    time_high := 16#%lx,\n", ifp->interface_uuid.time_high);
        fprintf (fid, "    time_low := 16#%x,\n", ifp->interface_uuid.time_low);
        fprintf (fid, "    reserved := 0,\n");
        fprintf (fid, "    family := chr(16#%x),\n", ifp->interface_uuid.family);

        fprintf (fid, "    host := [");
        first = true;
        for (i = 0; i < 7; i++) {
            if (first)
                first = false;
            else
                fprintf (fid, ", ");
            fprintf (fid, "chr(16#%x)", ifp->interface_uuid.host[i]);
            }
        fprintf (fid, "]\n    ]\n  ];\n");
        }
}


static
void pspell_constant_val (fid, cp)
FILE       *fid;
constant_t *cp;
{
    char       *str;

    switch (cp->kind) {
        case nil_k:
            fprintf (fid, "nil");
            break;
        case boolean_const_k:
            if (cp->value.boolean_value)
                fprintf (fid, "true");
            else
                fprintf (fid, "false");
            break;
        case integer_k:
            fprintf (fid, "%ld", cp->value.int_val);
            break;
        case real_k:
            fprintf (fid, "%f", cp->value.real_val);
            break;
        case string_k:
            STRTAB_str_to_string (cp->value.string_val, &str);
            fprintf (fid, "'%s'", str);
            break;
        case named_const_k:
            spell_name (fid, cp->value.named_val.name);
            break;
        }
}


typedef enum {
    type_section,
    const_section,
    routine_section,
    var_section,
    no_section
    } pspell_section_kind;

static void pspell_set_section __PROTOTYPE((FILE *fid, pspell_section_kind kind));

static
pspell_section_kind current_section = no_section;


static
void pspell_set_section (fid, kind)
FILE                *fid;
pspell_section_kind kind;
{
    if (kind != current_section) {
        current_section = kind;
        switch (kind) {
            case type_section:
                fprintf (fid, "type\n");
                break;
            case const_section:
                fprintf (fid, "const\n");
                break;
            case var_section:
                fprintf (fid, "var\n");
            case routine_section:
            case no_section: break ;
            }
        }
}


static
void pspell_type_exp ();

#define MAX_ANONYMOUS 100

static parameter_t *anonymous_params [MAX_ANONYMOUS];
static int anonymous_count = 0;
static NAMETABLE_id_t interface_name;
static int type_unique_id = 0;

static
void name_anonymous_param_types (fid, rp)
FILE *fid;
routine_t *rp;
{
    char           exp[max_string_len];
    NAMETABLE_id_t new_name;
    type_t         *named_tp;
    parameter_t    *pp;
    
    for (pp = rp->parameters; pp; pp = pp->next_param)

        switch (pp->type->kind) {

            case boolean_k: 
            case byte_k:
            case character_k: 
            case small_integer_k: 
            case short_integer_k: 
            case long_integer_k: 
            case hyper_integer_k: 
            case small_unsigned_k: 
            case short_unsigned_k: 
            case long_unsigned_k: 
            case hyper_unsigned_k: 
            case small_bitset_k: 
            case short_bitset_k: 
            case long_bitset_k: 
            case small_enumeration_k: 
            case short_enumeration_k: 
            case long_enumeration_k: 
            case short_real_k: 
            case long_real_k: 
            case small_subrange_k: 
            case short_subrange_k: 
            case long_subrange_k: 
            case drep_k:
            case handle_k:
            case named_k: 

                break;

            default:

                /* invent new_name based on interface name and increasing counter */
                sprintf (exp, "%%s_%d_t", type_unique_id++);
                new_name =
                    NAMETABLE_add_derived_name (interface_name, exp);

                /* create a named type node with the new name and the old (anonymous) type */
                named_tp = (type_t *)AST_type_node (named_k);
                named_tp->type_structure.named.name = new_name;
                named_tp->type_structure.named.resolution = 
                    pp->type;

                /* declare the new type name */
                pspell_set_section (fid, type_section);
                spell_name (fid, new_name);
                fprintf (fid, " = ");
                pspell_type_exp (fid, pp->type);
                fprintf (fid, ";\n");

                /* replace pp's type with the new named type ... */
                pp->type = named_tp;

                /* ... and remember that we did this */
                if (anonymous_count < MAX_ANONYMOUS)
                    anonymous_params [anonymous_count++] = pp;
                else
                    error ("NIDL internal limit exceeded: MAX_ANONYMOUS");
            }
}

static void
unname_anonymous_param_types ()
{
    type_t *named_tp;
    parameter_t *pp;
    
    while (anonymous_count != 0) {

        pp = anonymous_params [--anonymous_count];

        named_tp = pp->type;
        pp->type = named_tp->type_structure.named.resolution;
        free ((char *)named_tp);
        }
}


static
void pspell_constant_def (fid, bp)
FILE      *fid;
binding_t *bp;
{
    pspell_set_section (fid, const_section);
    spell_name (fid, bp->name);
    fprintf (fid, " = ");
    pspell_constant_val (fid, (constant_t *)bp->binding);
    fprintf (fid, ";\n");
}


static
void pspell_field (fid, fp)
FILE    *fid;
field_t *fp;
{

    spell_name (fid, fp->name);
    fprintf (fid, " : ");
    pspell_type_exp (fid, fp->type);
    fprintf (fid, ";\n");
}


static
void pspell_enumerators (fid, ep)
FILE      *fid;
binding_t *ep; /* list of enumeration name bindings */
{
    boolean first = true;

    for (; ep; ep = ep->next) {
        if (first)
            first = false;
        else
            fprintf (fid, ",\n  ");
        spell_name (fid, ep->name);
        }
}


static
void pspell_tags (fid, tgp)
FILE  *fid;
tag_t *tgp;
{
    boolean first = true;

    for ( ; tgp; tgp = tgp->next_tag) {
        if (first)
            first = false;
        else
            fprintf (fid, ", ");
        pspell_constant_val (fid, &tgp->tag_value);
        };
    fprintf (fid, " : ");
}


static
void pspell_fields (fid, fp)
FILE    *fid;
field_t *fp;
{
    fprintf (fid, "(\n");
    for ( ; fp; fp = fp->next_field)
                pspell_field (fid, fp);
    fprintf (fid, ");\n");
}

static
void pspell_routine_ptr (fid, rp)
FILE        *fid;
routine_t   *rp;
{
    type_t      *rt; /* result type */
    boolean     function;

    depointerize_routine (rp);

    fprintf (fid, "^");

    rt = rp->result->type;
    function = (rt->kind != void_k);

    if (function)
        fprintf (fid, "function ");
    else
        fprintf (fid, "procedure ");

    pspell_parameter_list (fid,  rp->parameters);

    if (function) {
        fprintf (fid, " : ");
        pspell_type_exp (fid, rt);
        }
    fprintf (fid, " ; options(val_param, extern)");
}


static
void pspell_type_exp (fid, tp)
FILE    *fid;
type_t  *tp;
{
    component_t   *cp;
    boolean       first;
    field_t       *fp;
    array_index_t *indexp;
    variant_t     *vp;

    if (tp->type_name != NAMETABLE_NIL_ID) {
        spell_name (fid, tp->type_name);
        return;
        }

    switch (tp->kind) {

        case named_k:
            spell_name (fid, tp->type_structure.named.name);
            break;

        case small_integer_k:
            fprintf (fid, "integer8");
            break;

        case hyper_integer_k:
            fprintf (fid, "integer64");
            break;

        case small_bitset_k:
        case small_unsigned_k:
            fprintf (fid, "unsigned8");
            break;

        case hyper_unsigned_k:
            fprintf (fid, "unsigned64");
            break;

        case boolean_k:
            fprintf (fid, "boolean");
            break;

        case byte_k:
            fprintf (fid, "char");
            break;

        case short_integer_k:
            fprintf (fid, "integer");
            break;

        case long_integer_k:
        case long_enumeration_k:
            fprintf (fid, "integer32");
            break;

        case short_bitset_k:
        case long_bitset_k:
            fprintf (fid, "set of ");
            pspell_type_exp (fid, tp->type_structure.bitset.base_type);
            break;

        case small_subrange_k:
        case short_subrange_k:
        case long_subrange_k:
            pspell_constant_val (fid, tp->type_structure.subrange.lower_bound);
            fprintf (fid, "..");
            pspell_constant_val (fid, tp->type_structure.subrange.upper_bound);
            break;

        case short_unsigned_k:
            fprintf (fid, "unsigned");
            break;

        case long_unsigned_k:
            fprintf (fid, "unsigned32");
            break;

        case character_k:
            fprintf (fid, "char");
            break;

        case short_real_k:
            fprintf (fid, "real");
            break;

        case long_real_k:
            fprintf (fid, "double");
            break;

        case fixed_string_zero_k:
            fprintf (fid, "array [");
            pspell_type_exp (
                fid,
                tp->type_structure.fixed_string_zero.index->type);
            fprintf (fid, "] of char");
            break;

        case open_string_zero_k:
            fprintf (fid, "char_ptr");
            break;

        case drep_k:
            fprintf (fid, "rpc_$drep_t");
            break;

        case handle_k:
            fprintf (fid, "handle_t");
            break;

        case fixed_array_k:
            fprintf (fid, "array [");
            first = true;
            for (indexp = tp->type_structure.fixed_array.indices; indexp; indexp = indexp->next) {
                if (first)
                    first = false;
                else
                    fprintf (fid, ", ");
                pspell_type_exp (fid, indexp->type);
                }
            fprintf (fid, "] of ");
            pspell_type_exp (fid, tp->type_structure.fixed_array.elements);
            break;

        case open_array_k:
            fprintf (fid, "array [");
            pspell_constant_val (fid, tp->type_structure.open_array.lower_bound);
            fprintf (fid, "..");
            pspell_constant_val (fid, tp->type_structure.open_array.lower_bound);
            for (indexp = tp->type_structure.open_array.indices; indexp; indexp = indexp->next) {
                fprintf (fid, ", ");
                pspell_type_exp (fid, indexp->type);
                }
            fprintf (fid, "] of ");
            pspell_type_exp (fid, tp->type_structure.open_array.elements);
            break;

        case record_k:
            fprintf(fid, "record\n");
            for ( fp = tp->type_structure.record.fields; fp; fp = fp->next_field)
                pspell_field (fid, fp);
            if (tp->type_structure.record.variant) {
                    vp = tp->type_structure.record.variant;
                    fprintf (fid, "case ");
                    spell_name (fid, vp->tag_id);
                    fprintf (fid, " : ");
                    pspell_type_exp (fid, vp->tag_type);
                    fprintf (fid, " of\n");
                    for (cp = vp->components; cp; cp = cp->next_component) {
                        pspell_tags (fid, cp->tags);  
                        pspell_fields (fid, cp->fields);  
                        }
            }
            fprintf(fid, "end");
            break;

        case pointer_k:
            fprintf (fid, "^");
            pspell_type_exp (fid, tp->type_structure.pointer.pointee);
            break;

        case user_marshalled_k:
            pspell_type_exp (fid, tp->type_structure.user_marshalled.user_type);
            break;

        case void_k:
            break;

        case small_enumeration_k:
        case short_enumeration_k:
            fprintf (fid, "(");
            pspell_enumerators (fid, tp->type_structure.enumeration.enum_constants);
            fprintf (fid, ")");
            break;

        case routine_ptr_k:
            pspell_routine_ptr (fid, &tp->type_structure.routine_ptr);
            break;

        case univ_ptr_k:
            fprintf(fid, "univ_ptr") ;
            break ;

        default:
            unimp_type_exit (tp->kind, "pspell_type_exp");
            break;

        };
}


typedef enum {
    handle_type_name,
    transmit_as_type_name
    } type_name_kind;

typedef struct rtn {
    NAMETABLE_id_t name;
    type_name_kind kind;
    struct rtn *next;
    } remembered_type_name;

static void remember_type_name __PROTOTYPE((NAMETABLE_id_t name, type_name_kind kind));
    

static 
remembered_type_name *rtn_list_head = NULL;

static void
remember_type_name (name, kind)
NAMETABLE_id_t name;
type_name_kind kind;
{
    remembered_type_name *rtnp;

    rtnp = (remembered_type_name *) alloc (sizeof (remembered_type_name));
    rtnp->name = name;
    rtnp->kind = kind;
    rtnp->next = rtn_list_head;
    rtn_list_head = rtnp;
}

static void
pspell_derived_routine_decls (fid)
FILE *fid;
{
    remembered_type_name *rtnp;

    pspell_set_section (fid, routine_section);
    for (rtnp = rtn_list_head; rtnp; rtnp = rtnp->next) {
        switch (rtnp->kind) {
            case handle_type_name:

                fprintf (fid, "function ");
                spell_name (fid, rtnp->name);
                fprintf (fid, "_bind (in uh: ");
                spell_name (fid, rtnp->name);
                fprintf (fid, "): handle_t; options(val_param, extern);\n");
        
                fprintf (fid, "procedure ");
                spell_name (fid, rtnp->name);
                fprintf (fid, "_unbind (in uh: ");
                spell_name (fid, rtnp->name);
                fprintf (fid, "; in h: handle_t); options(val_param, extern);\n");
                break;
    
            case transmit_as_type_name:
        
                fprintf (fid, "procedure ");
                spell_name (fid, rtnp->name);
                fprintf (fid, "_from_xmit_rep (\n");
                fprintf (fid, "  in  xmit_object: univ_ptr;\n");
                fprintf (fid, "  out object: univ_ptr\n");
                fprintf (fid, "); options(val_param, extern);\n");
        
                fprintf (fid, "procedure ");
                spell_name (fid, rtnp->name);
                fprintf (fid, "_to_xmit_rep (\n");
                fprintf (fid, "  in  object: univ_ptr;\n");
                fprintf (fid, "  out xmit_object: univ_ptr\n");
                fprintf (fid, "); options(val_param, extern);\n");
        
                fprintf (fid, "procedure ");
                spell_name (fid, rtnp->name);
                fprintf (fid, "_free_xmit_rep (\n");
                fprintf (fid, "  in  xmit_object: univ_ptr\n");
                fprintf (fid, "); options(val_param, extern);\n");
        
                fprintf (fid, "procedure ");
                spell_name (fid, rtnp->name);
                fprintf (fid, "_free (\n");
                fprintf (fid, "  in object: univ_ptr\n");
                fprintf (fid, "); options(val_param, extern);\n");
            }
        }
}


static
void pspell_type_def (fid, bp)
FILE      *fid;
binding_t *bp;
{
    unsigned long  const_val;
    binding_t      *ec;
    type_t         *tp;

    tp = (type_t *)bp->binding;

    if (tp->kind == long_enumeration_k) {
        pspell_set_section (fid, const_section);

        const_val = 0;
        for (ec = tp->type_structure.enumeration.enum_constants; ec; ec = ec->next) {
            spell_name (fid, ec->name);
            fprintf (fid, " = %ld;\n", const_val); 
            const_val += 1;
            }
        }

    /* declare names for anonymous type exp used in parameters 
        of a routine pointer type */
    if (tp->kind == routine_ptr_k)
            name_anonymous_param_types (fid, &tp->type_structure.routine_ptr);

    pspell_set_section (fid, type_section);
    spell_name (fid, bp->name);
    fprintf (fid, " = ");

    pspell_type_exp (fid, tp);

    fprintf (fid, ";\n");

    /* remove pseudonyms for routine pointer anonymous types */
    if (tp->kind == routine_ptr_k)
            unname_anonymous_param_types ();

    /* remember name of [handle] and [transmit_as()] types so that we'll
       declare the associated routines at the end of the .ins.pas file so
       as not to disrupt type section semantics which permit a forward
       reference in one type definition to another as long as there is no
       intervening TYPE keyword */

    if ((tp->is_handle) && (tp->kind != handle_k))
        remember_type_name (bp->name, handle_type_name);

    if (tp->kind == user_marshalled_k)
        remember_type_name (bp->name, transmit_as_type_name);

}


boolean univ (tp)
type_t *tp;
{
    switch (tp->kind) {
        case named_k:
            return univ (tp->type_structure.named.resolution);
        case open_array_k:
            return true;
        default:
            return false;
        }
    /*lint -unreachable*/
}

static
void pspell_parameter_list (fid, pp)
FILE        *fid;
parameter_t *pp;
{
    boolean first = true;

    if (pp) {
        fprintf (fid, "(\n");
        for (; pp; pp = pp->next_param) {
            if (first)
                first = false;
            else {
                fprintf (fid, ";\n");
                }
            if (pp->in)
                if (pp->out)
                    fprintf (fid, "in out ");
                else
                    fprintf (fid, "in ");
            else
                fprintf (fid, "out ");
            spell_name (fid, pp->name);
            fprintf (fid, " : ");

            if (univ (pp->type))
                fprintf (fid, "univ ");

            pspell_type_exp (fid, pp->type);
            }
        fprintf (fid, "\n)");
        }
}


static
void pspell_function_options(fid, tp)
FILE      *fid;
type_t    *tp;
{             
        if (tp->kind == named_k)
            tp = tp->type_structure.named.resolution ;
        if (tp->kind == pointer_k || tp->kind == univ_ptr_k)
            fprintf (fid, "; options(val_param, extern, d0_return);\n");
        else
            fprintf (fid, "; options(val_param, extern);\n");
}

static
void pspell_routine_def (fid, bp)
FILE      *fid;
binding_t *bp;
{
    routine_t *rp;
    boolean   function; 

    rp = (routine_t *) bp->binding;

    depointerize_routine (rp);

    name_anonymous_param_types (fid, rp);

    pspell_set_section (fid, routine_section);

    function = (rp->result->type->kind != void_k);

    if (function) 
        fprintf (fid, "function ");
    else
        fprintf (fid, "procedure ");

    spell_name (fid, bp->name);

    pspell_parameter_list (fid, rp->parameters);

    if (function) {
        fprintf (fid, " : ");
        pspell_type_exp (fid, rp->result->type);
        pspell_function_options(fid, rp->result->type) ;
        }
    else
        fprintf (fid, "; options(val_param, extern);\n");

    unname_anonymous_param_types ();
}


static
void pspell_bindings (fid, bp)
FILE        *fid;
binding_t   *bp;
{
    for ( ; bp; bp = bp->next) {
        switch (bp->kind) {
            case constant_k:
                pspell_constant_def (fid, bp);
                break;
            case interface_k:
                break;
            case routine_k:
                pspell_routine_def (fid, bp);
                break;
            case type_k:
                pspell_type_def (fid, bp);
            };
        }
    pspell_derived_routine_decls (fid);
}


static
void pspell_implicit_handle_extern (fid, name, type)
FILE *fid;
NAMETABLE_id_t name;
type_t *type;
{
    pspell_set_section (fid, var_section);
    spell_name(fid, name) ;
    fprintf (fid, " : extern ");
    pspell_type_exp(fid, type) ;
    fprintf(fid, " ") ;
    fprintf (fid, ";\n") ;
}


static
void pspell_epv_type_and_vars (fid, if_name, bp, mmmv)
FILE *fid;
NAMETABLE_id_t if_name;
binding_t *bp;
boolean mmmv;
{
    NAMETABLE_id_t type_name, var_name;
    binding_t      *temp_bp;

    pspell_set_section (fid, type_section);

    /* name all anonymous parameter types */
    for (temp_bp = bp ; temp_bp; temp_bp = temp_bp->next)
        if (temp_bp->kind == routine_k)
            name_anonymous_param_types (fid, (routine_t *) temp_bp->binding);

    type_name = NAMETABLE_add_derived_name (if_name, "%s$epv_t");
    spell_name (fid, type_name);
    fprintf (fid, " = record\n");
    for (temp_bp = bp ; temp_bp; temp_bp = temp_bp->next)
        if (temp_bp->kind == routine_k) {
            spell_name (fid, temp_bp->name);
            fprintf (fid, " : ");
            pspell_routine_ptr (fid, (routine_t *) temp_bp->binding);
            fprintf (fid, ";\n");
            }
    fprintf (fid, "end;\n");

    /* unname all anonymous parameter types */
    unname_anonymous_param_types ();

    pspell_set_section (fid, var_section);
    var_name = NAMETABLE_add_derived_name (if_name, "%s$client_epv");
    spell_name (fid, var_name);
    fprintf (fid, " : extern ");
    spell_name (fid, type_name);
    fprintf (fid, ";\n");

    if (!mmmv) {
        var_name = NAMETABLE_add_derived_name (if_name, "%s$mgr_epv");
        spell_name (fid, var_name);
        fprintf (fid, " : extern ");
        spell_name (fid, type_name);
        fprintf (fid, ";\n");
        }
}


static
void pspell_server_epv (fid, if_name, mmmv)
FILE *fid;
NAMETABLE_id_t if_name;
boolean mmmv;
{
    NAMETABLE_id_t var_name;

    pspell_set_section (fid, var_section);

    var_name = NAMETABLE_add_derived_name (if_name, "%s$server_epv");
    spell_name (fid, var_name);
    if (mmmv)
        fprintf (fid, " : extern rpc_$generic_epv_t;\n");
    else
        fprintf (fid, " : extern rpc_$epv_t;\n");
}


#ifndef __STDC__
void PSPELL_gen_pascal_ins_file ( astp, fid, emit_env, mmmv)
binding_t *astp;
FILE      *fid;
boolean   emit_env;
boolean   mmmv;
#else
void PSPELL_gen_pascal_ins_file (binding_t *astp, FILE *fid, boolean emit_env, boolean mmmv)
#endif
{
    interface_t *ifp;
    import_t    *ip;
    char        leaf_name [max_string_len];
    char        leaf_ext  [max_string_len];
    char        dir_name  [max_string_len];

    NAMETABLE_id_t include_var_name ;

    interface_name = astp->name; /* global var used to gen names for anonymous types in parameter */

    include_var_name = NAMETABLE_add_derived_name (astp->name, "%s_included");

    fprintf (fid, "%%ifdef ");
    spell_name (fid, include_var_name);
    fprintf (fid, " %%then\n%%exit\n%%else\n%%var ");
    spell_name (fid, include_var_name);
    fprintf (fid, "\n%%endif\n");

    ifp = (interface_t *) astp->binding;

    fprintf (fid , "%%include 'idl_base.ins.pas';\n");

    for (ip = ifp->imports; ip; ip=ip->next_import) {
        parse_path_name (ip->file_name, dir_name, leaf_name, leaf_ext);
        if (emit_env)
            if (! contains_ev_ref(ip->file_name))
                fprintf (fid, "%%include \'$(%s.%s).ins.pas", leaf_name, leaf_ext);
            else 
                if (strlen(dir_name) > 0) 
                    fprintf(fid, "%%include \'%s/%s.ins.pas", dir_name, leaf_name);
                else
                    fprintf(fid, "%%include \'%s.ins.pas", leaf_name) ;
        else
            fprintf (fid, "%%include \'%s.ins.pas", leaf_name);
        fprintf (fid, "';\n");
        }

    pspell_interface_def (fid, astp);
    pspell_set_section (fid, no_section);
    pspell_bindings (fid, ifp->exports);

    /* emit declarations of implicit binding variable and epv's */
    if (!ifp->local_only) {
        if (ifp->implicit_handle_var != NAMETABLE_NIL_ID)
            pspell_implicit_handle_extern (fid, ifp->implicit_handle_var, ifp->implicit_handle_type);
        pspell_epv_type_and_vars (fid, astp->name, ifp->exports, mmmv);
        pspell_server_epv   (fid, astp->name, mmmv);
        }
}
