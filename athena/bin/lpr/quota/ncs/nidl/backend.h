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

#define VARYING 1

#define MAX_ARRAY_INDICES 10

#define for_params_and_result(x,p,r) \
        for(x=(p?p:r); \
            x; \
            x=(x==r?(parameter_t *)NULL:(x->next_param?x->next_param:r)))

#define for_marshalling_list(x,p) for(x=p;x;x=x->next_to_marshall)

typedef struct array_dimension_t {
  long num_dims;      /* number of dimensions arrays has */
  long dims[VARYING]; /* dimensions in major to minor order */
} array_dimension_t;

typedef enum {
  ins,
  outs
} param_kinds;

typedef enum {
  marshall,
  unmarshall,
  convert
} marshall_t;

typedef enum {
  auto_binding,
  implicit_binding,
  explicit_binding
} binding_style;

/*
 *
 *  inputs:
 *    astp       pointer to first binding in abstract syntax tree
 *    fid        id of file on which to write pascal insert file
 */
void PSPELL_gen_pascal_ins_file  __PROTOTYPE((binding_t *astp, FILE *fid, boolean emit_env, boolean mmmv));

void CHECKER_init __PROTOTYPE((void));

void CHECKER_check_interface __PROTOTYPE((binding_t *astp, char **import_dirs, char **def_strings));
    
void CSPELL_gen_c_ins_file __PROTOTYPE((binding_t *astp, FILE *fid, boolean emit_env, boolean mmmv));

void BACKEND_gen_c_stubs __PROTOTYPE((binding_t *, FILE *, FILE *, FILE *,
								      boolean, boolean,
								      boolean, boolean));

void BACKEND_init __PROTOTYPE((void));
    
void unimp_type_exit  __PROTOTYPE((type_kind kind, char *label));
    
void spell_name __PROTOTYPE ((FILE *fid, NAMETABLE_id_t name));
