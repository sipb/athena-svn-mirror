/*  balanced_trees.c, /us/lib/ddslib/src, pato, 05/26/88
**      Symbol table manager
**
** ========================================================================== 
** Confidential and Proprietary.  Copyright 1988 by Apollo Computer Inc.,
** Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
** Copyright Laws Of The United States.
** 
** Apollo Computer Inc. reserves all rights, title and interest with respect
** to copying, modification or the distribution of such software programs
** and associated documentation, except those rights specifically granted
** by Apollo in a Product Software Program License, Source Code License
** or Commercial License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between
** Apollo and Licensee.  Without such license agreements, such software
** programs may not be used, copied, modified or distributed in source
** or object code form.  Further, the copyright notice must appear on the
** media, the supporting documentation and packaging as set forth in such
** agreements.  Such License Agreements do not grant any rights to use
** Apollo Computer's name or trademarks in advertising or publicity, with
** respect to the distribution of the software programs without the specific
** prior written permission of Apollo.  Trademark agreements may be obtained
** in a separate Trademark License Agreement.
** ========================================================================== 
**
** History:
**  05/25/88    pato    import from registry library (and remove dependencies)
**  03/20/88    pato    make sure files are closed after being read.
**  03/18/88    pato    change file creation protection
**  01/29/88    pato    add an external clear_database operation
**  01/18/88    pato    read files in smaller chunks, and use task_$yield
**  12/18/87    pato    add mem_$ ops to enable garbage collection.
**  12/10/87    pato    use rsdb_file_$... instead of open and stat.
*/

/* 
** Roughly modeled on the algorithms in Aho Hopcroft and Ullman.  - modified so 
** that 3 node splitting is handled by the recursive implementation of
** search_insert
*/

#include "std.h"

#ifdef MSDOS
#  include "b_trees.h"
#else
#  include "balanced_trees.h"
#endif

#define private static
#define public

#define FULLWORD_SIZE 4     /* Must be a power of 2 */

typedef struct mem_$buf *mem_$ptr;
struct mem_$buf {
    mem_$ptr    next;
    char        *free;
    long        space_avail;
};

typedef struct data_node {
    int     size;
    sym_$datum_t key;
    sym_$datum_t data;
} data_node, *data_handle;

typedef struct tree_node *node_handle;
typedef struct tree_node {
    node_handle lson;
    node_handle mson;
    node_handle rson;
    data_handle lkey;
    data_handle mkey;
} tree_node;

struct sym_$tree_root {
    mem_$ptr    mem;
    node_handle tree;
};

/*
** Static info used by update and node_allocation routines.
*/
static  node_handle     td_$implant;
static  sym_$datum_t    *td_$key;
static  sym_$datum_t    *td_$data;
static  boolean         td_$insert;
static  boolean         td_$replace;
static  sym_$tree_handle td_$root;

typedef enum { IMPLANT, NOT_FOUND, REPLACED, DUPLICATE, } result_t; 

#define IS_3NODE(P) ((P)->rson != NULL)
#define IS_LEAF(P) ((P)->lson == NULL)
#define IS_DELETED(P) ((P)->mson == NULL)
#define NOT_DELETED(P) ((P)->mson = (P))
#define MAKE_DELETED(P) ((P)->mson = NULL)

#define MIN(A,B) ((A) < (B) ? (A) : (B))

#define alloc_tree_node() ((sym_$tree_handle) mem_$alloc(sizeof(tree_node)))

#ifdef __STDC__
static struct sym_$tree_root *mem_$alloc_db(void);
static struct mem_$buf *mem_$_alloc(size_t size);
static char *mem_$alloc(struct sym_$tree_root *db,size_t size);
static void mem_$free(struct sym_$tree_root *db);
static unsigned char key_less_equal(struct sym_$datum *k1,struct sym_$datum *k2);
static unsigned char key_less(struct sym_$datum *k1,struct sym_$datum *k2);
static unsigned char key_equal(struct sym_$datum *k1,struct sym_$datum *k2);
static void store_new_data(struct data_node *new,struct sym_$datum *key,struct sym_$datum *data);
static struct data_node *new_data_node(struct sym_$datum *key,struct sym_$datum *data);
static struct data_node *replace_data_node(struct data_node *old_data,struct sym_$datum *key,struct sym_$datum *data);
static struct tree_node *new_tree_node(struct tree_node *lson,struct tree_node *mson,unsigned char add_data);
static result_t update(struct tree_node *tree);
static result_t search_next(struct tree_node *root,struct sym_$datum *key,struct tree_node * *result_node);
static result_t search(struct tree_node *tree,struct sym_$datum *key,struct tree_node * *result_node);
#endif

/*
** Memory Allocation Rountines
*/

private sym_$tree_handle mem_$alloc_db ()
{
    sym_$tree_handle db;

    db = (sym_$tree_handle) malloc(sizeof(*db));
    db->mem = NULL;
    db->tree = NULL;

    return db;
}

private mem_$ptr    mem_$_alloc ( size )
    size_t    size;
{
#define ALLOC_BLKSIZE (16*1024) /* Must be a power of 2 */
    mem_$ptr    p;
    size_t      alloc_size;


    alloc_size = size + sizeof(*p);
    alloc_size = (alloc_size + (ALLOC_BLKSIZE - 1)) & ~(ALLOC_BLKSIZE - 1);

    p = (mem_$ptr) malloc(alloc_size);
    if (p == NULL)
        return NULL;

    p->next         = NULL;
    p->space_avail  = (alloc_size) - sizeof(*p);
    p->free         = (char *) (p+1);

    return p;
}

private char * mem_$alloc ( db, size )
    sym_$tree_handle db;
    size_t           size;
{
    mem_$ptr    p;
    char        *ret_ptr = NULL;

    /*
    ** Round size up to a fullword boundary
    */
    size = (size + (FULLWORD_SIZE - 1)) & ~(FULLWORD_SIZE - 1);

    if (db->mem == NULL) {
        db->mem = mem_$_alloc(size);
        if (db->mem == NULL)
            return ret_ptr;
    }

    for (p = db->mem; p->next != NULL && p->space_avail < size; p = p->next);

    if (p->space_avail < size) {
        p->next = mem_$_alloc(size);
        if (p->next == NULL)
            return ret_ptr;

        p = p->next;
    }

    p->space_avail -= size;
    ret_ptr = p->free;
    p->free += size;

    return ret_ptr;
}

private void mem_$free ( db )
    sym_$tree_handle db;
{
    mem_$ptr    p;
    mem_$ptr    next_p;

    if (db != NULL) {
        for (p = db->mem; p != NULL; p = next_p) {
            next_p = p->next;
            free(p);
        }
        db->mem  = NULL;
        db->tree = NULL;
    }
}




private boolean key_less_equal ( k1, k2 )
    sym_$datum_t *k1;
    sym_$datum_t *k2;
{
    int result = -1;    /* An empty key is less than anything */
    unsigned char *p1;
    unsigned char *p2;
    int len;

    len = MIN(k1->dsize, k2->dsize);

    p1 = (unsigned char *) k1->dptr;
    p2 = (unsigned char *) k2->dptr;
    while (len--) {
        result = *p1++ - *p2++;
        if (result != 0)
            break;
    }

    if (result < 0)
        return true;
    if (result == 0) {
        if (k1->dsize <= k2->dsize)
            return true;
    }
    return false;
}

private boolean key_less ( k1, k2 )
    sym_$datum_t *k1;
    sym_$datum_t *k2;
{
    int result = -1;    /* An empty key is less than anything */
    unsigned char *p1;
    unsigned char *p2;
    int  len;

    len = MIN(k1->dsize, k2->dsize);

    p1 = (unsigned char *) k1->dptr;
    p2 = (unsigned char *) k2->dptr;
    while (len--) {
        result = *p1++ - *p2++;
        if (result != 0)
            break;
    }

    if (result == 0) {
        if (k1->dsize < k2->dsize) {
            return true;
        } else {
            return false;
        }
    } else if (result > 0) {
        return false;
    }

    return true;
}

private boolean key_equal ( k1, k2 )
    sym_$datum_t *k1;
    sym_$datum_t *k2;
{
    int result;

    if (k1->dsize != k2->dsize)
        return false;

    result = memcmp(k1->dptr, k2->dptr, k1->dsize);
    if (result != 0)
        return false;

    return true;
}

private void store_new_data ( new, key, data )
    data_handle     new;
    sym_$datum_t    *key;
    sym_$datum_t    *data;
{
    new->key.dsize = key->dsize;
    new->key.dptr  = (char *) (new + 1);
    bcopy(key->dptr, new->key.dptr, new->key.dsize);
    new->data.dsize = data->dsize;
    new->data.dptr  = new->key.dptr + new->key.dsize;
    bcopy(data->dptr, new->data.dptr, new->data.dsize);
} 

private data_handle new_data_node ( key, data )
    sym_$datum_t *key;
    sym_$datum_t *data;
{
    data_handle new;
    size_t      size;

    size = sizeof(data_node) + key->dsize + data->dsize;
    new = (data_handle) mem_$alloc(td_$root, size);
    new->size = size;
    store_new_data(new, key, data);

    return new;
}

private data_handle replace_data_node ( old_data, key, data )
    data_handle     old_data;
    sym_$datum_t    *key;
    sym_$datum_t    *data;
{
    int size;

    size = sizeof(data_node) + key->dsize + data->dsize;
    if (size <= old_data->size) {
        store_new_data(old_data, key, data);
        return old_data;
    } else {
        return new_data_node(key, data);
    }
}

private node_handle new_tree_node ( lson, mson, add_data )
    node_handle lson;
    node_handle mson;
    boolean     add_data;
{
    node_handle implant;

    implant = (node_handle) mem_$alloc(td_$root, sizeof(tree_node));
    implant->rson = NULL;
    implant->lson = lson;
    implant->mson = mson;
    if (IS_DELETED(implant)) {
        NOT_DELETED(implant);
    }
    if (add_data) {
        implant->mkey = implant->lkey = new_data_node(td_$key, td_$data);
    } else {
        implant->mkey = implant->lkey = NULL;
    }
    return implant;
}


/*
** update is NOT reentrant for multi-tasking.  It must be called with an
**  appropriate mutual exclusion lock in place.  (It uses a static data block
**  to control its actions.)
*/

private result_t update ( tree )
    node_handle tree;
{
    result_t    retval;
    tree_node   *split_node;
    tree_node   *t;
    enum {left, middle, right} branch;

    if (tree == NULL) {
        if (td_$insert) {
            td_$implant = new_tree_node(NULL, NULL, true);
            return IMPLANT;
        } else {
            return NOT_FOUND;
        }
    }

    if (IS_LEAF(tree)) {
        if (key_equal(td_$key, &(tree->lkey->key))) {
            td_$implant = tree;
            if (td_$replace || (td_$insert && IS_DELETED(tree))) {
                tree->lkey = tree->mkey
                    = replace_data_node(tree->lkey, td_$key, td_$data);
                NOT_DELETED(tree);
                return REPLACED;
            } else if (IS_DELETED(tree)) {
                return NOT_FOUND;
            } else {                
                return DUPLICATE;
            }
        } else {
            if (td_$insert) {
                td_$implant = new_tree_node(NULL, NULL, true);
                return IMPLANT;
            } else {
                return NOT_FOUND;
            }
        }
    } else {
        if (key_less_equal(td_$key, &(tree->lkey->key))) {
            branch = left;
            retval = update(tree->lson);
        } else if (!IS_3NODE(tree) 
                        || key_less_equal(td_$key, &(tree->mkey->key))) {
            branch = middle;
            retval = update(tree->mson);
        } else {
            branch = right;
            retval = update(tree->rson);
        }

        if (retval == IMPLANT) {
            if (IS_3NODE(tree)) {
                split_node = new_tree_node(NULL, NULL, false);

                if (key_less_equal(&(td_$implant->mkey->key), 
                                            &(tree->lson->mkey->key)))
                    branch = left;
                else if (key_less_equal(&(td_$implant->mkey->key), 
                                            &(tree->mson->mkey->key)))
                    branch = middle;
                else
                    branch = right;

                if (branch == left || branch == middle) {
                    split_node->lson = tree->mson;
                    split_node->mson = tree->rson;

                    if (branch == left) {
                        tree->mson = tree->lson;
                        tree->lson = td_$implant;
                    } else {
                        tree->mson = td_$implant;
                    }
                } else {
                    if (key_less_equal(&(tree->rson->mkey->key), 
                                            &(td_$implant->mkey->key))) {
                        split_node->lson = tree->rson;
                        split_node->mson = td_$implant;
                    } else {
                        split_node->lson = td_$implant;
                        split_node->mson = tree->rson;
                    }
                }

                for (t = split_node->lson; IS_3NODE(t); t = t->rson);
                split_node->lkey = t->mkey;
                for (t = split_node->mson; IS_3NODE(t); t = t->rson);
                split_node->mkey = t->mkey;

                for (t = tree->lson; IS_3NODE(t); t = t->rson);
                tree->lkey = t->mkey;
                for (t = tree->mson; IS_3NODE(t); t = t->rson);
                tree->mkey = t->mkey;

                tree->rson = NULL;
                td_$implant = split_node;
                return IMPLANT;

            } else {
                if (branch == left) {
                    tree->rson = tree->mson;
                    if (key_less_equal(&(tree->lson->mkey->key), 
                                            &(td_$implant->mkey->key))) {
                        tree->mson = td_$implant;
                    } else {
                        tree->mson = tree->lson;
                        tree->lson = td_$implant;
                    }
                    for (t = tree->lson; IS_3NODE(t); t = t->rson);
                    tree->lkey = t->mkey;
                    for (t = tree->mson; IS_3NODE(t); t = t->rson);
                    tree->mkey = t->mkey;
                } else if (branch == middle) {
                    if (key_less_equal(&(tree->mson->mkey->key), 
                                            &(td_$implant->mkey->key))) {
                        tree->rson = td_$implant;
                    } else {
                        tree->rson = tree->mson;
                        tree->mson = td_$implant;
                    }
                    for (t = tree->mson; IS_3NODE(t); t = t->rson);
                    tree->mkey = t->mkey;
                } else {
                    tree->rson = td_$implant;
                }
                return REPLACED;
            }
        }
    }

    return retval;
}

private result_t search_next ( root, key, result_node )
    node_handle root;
    sym_$datum_t     *key;
    node_handle *result_node;
{
    node_handle tree;

    if (root == NULL) {
        return NOT_FOUND;
    }

    while (true) {
        tree = root;

        while (!IS_LEAF(tree)) {
            if (key_less(key, &(tree->lkey->key))) {
                tree = tree->lson;
            } else if (!IS_3NODE(tree) || key_less(key, &(tree->mkey->key))) {
                tree = tree->mson;
            } else {
                tree = tree->rson;
            }
        }

        if (key_less(key, &(tree->lkey->key))) {
            if (IS_DELETED(tree)) {
                key = &tree->lkey->key;
            } else {
                *result_node = tree;
                return DUPLICATE;
            }
        } else {
            return NOT_FOUND;
        }
    }

    /* NOTREACHED */
}

private result_t search ( tree, key, result_node )
    node_handle tree;
    sym_$datum_t     *key;
    node_handle *result_node;
{
    if (tree == NULL) {
        return NOT_FOUND;
    }

    while (!IS_LEAF(tree)) {
        if (key_less_equal(key, &(tree->lkey->key))) {
            tree = tree->lson;
        } else if (!IS_3NODE(tree) || key_less_equal(key, &(tree->mkey->key))) {
            tree = tree->mson;
        } else {
            tree = tree->rson;
        }
    }

    if (key_equal(key, &(tree->lkey->key))) {
        if (IS_DELETED(tree)) {
            return NOT_FOUND;
        } else {
            *result_node = tree;
            return DUPLICATE;
        }
    } else {
        return NOT_FOUND;
    }

    /* NOTREACHED */
}

public long sym_$insert_data ( replace_ok, db, new_key, new_data )
    long             replace_ok;
    sym_$tree_handle *db;
    sym_$datum_t     *new_key;
    sym_$datum_t     *new_data;
{
    result_t    result;
    node_handle t;

    if (*db == NULL) {
        *db = mem_$alloc_db();
    }

    td_$insert  = true;
    td_$implant = NULL;
    td_$replace = replace_ok;
    td_$root    = *db;

    td_$key  = new_key;
    td_$data = new_data;

    result = update(td_$root->tree);
    if (result == IMPLANT) {
        if (td_$root->tree == NULL) {
            td_$root->tree = td_$implant;
        } else {
            t = td_$root->tree;
            if (key_less_equal(&(t->mkey->key), &(td_$implant->mkey->key))) {
                td_$root->tree = new_tree_node(t, td_$implant, false);
            } else {
                td_$root->tree = new_tree_node(td_$implant, t, false);
            }
            for (t = td_$root->tree->lson; IS_3NODE(t); t = t->rson);
            td_$root->tree->lkey = t->mkey;
            for (t = td_$root->tree->mson; IS_3NODE(t); t = t->rson);
            td_$root->tree->mkey = t->mkey;
        }
        return true;
    } else if (result == REPLACED) {
        return true;
    }
    return false;
}

public sym_$datum_t * sym_$fetch_data ( db, search_key )
    sym_$tree_handle db;
    sym_$datum_t     *search_key;
{
    node_handle result_node;
    result_t    result;
    sym_$datum_t     *search_data = NULL;

    if (db == NULL) {
        return search_data;
    }

    result = search(db->tree, search_key, &result_node);

    if (result == DUPLICATE) {
        search_data = &(result_node->mkey->data);
    }

    return search_data;
}

public sym_$datum_t * sym_$fetch_next ( db, search_key, next_key )
    sym_$tree_handle db;
    sym_$datum_t     *search_key;
    sym_$datum_t     **next_key;
{
    node_handle result_node;
    result_t    result;
    sym_$datum_t     *search_data = NULL;

    if (db == NULL) {
        return search_data;
    }

    *next_key = NULL;
    result = search_next(db->tree, search_key, &result_node);

    if (result == DUPLICATE) {
        search_data = &(result_node->mkey->data);
        *next_key = &(result_node->mkey->key);
    }

    return search_data;
}

public long sym_$delete_data ( db, search_key )
    sym_$tree_handle db;
    sym_$datum_t     *search_key;
{
    node_handle result_node;
    result_t    result;

    if (db == NULL) {
        return false;
    }
    result = search(db->tree, search_key, &result_node);

    if (result == DUPLICATE) {
        MAKE_DELETED(result_node);
        return true;
    }

    return false;
}

public void sym_$clear_database ( db )
    sym_$tree_handle *db;
{
    if ((*db) != NULL) {
        mem_$free(*db);
    }
}
