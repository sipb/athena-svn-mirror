/* gok-chunker.h
*
* Copyright 2002 Sun Microsystems, Inc.,
* Copyright 2002 University Of Toronto
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*/

#ifndef __GOKCHUNKER_H__
#define __GOKCHUNKER_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gnome.h>
#include "gok-key.h"
#include "gok-button.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* types of chunks */
typedef enum {
CHUNKER_RESET,
CHUNKER_NOCHUNKS,
CHUNKER_KEYS,
CHUNKER_ROWS,
CHUNKER_COLUMNS,
CHUNKER_RECURSIVE,
CHUNKER_2GROUPS,
CHUNKER_3GROUPS,
CHUNKER_4GROUPS,
CHUNKER_5GROUPS,
CHUNKER_6GROUPS
} ChunkTypes;

typedef enum {
CHUNKER_TOPTOBOTTOM_LEFTTORIGHT,
CHUNKER_TOPTOBOTTOM_RIGHTTOLEFT,
CHUNKER_BOTTOMTOTOP_LEFTTORIGHT,
CHUNKER_BOTTOMTOTOP_RIGHTTOLEFT,
CHUNKER_LEFTTORIGHT_TOPTOBOTTOM,
CHUNKER_LEFTTORIGHT_BOTTOMTOTOP,
CHUNKER_RIGHTTOLEFT_TOPTOBOTTOM,
CHUNKER_RIGHTTOLEFT_BOTTOMTOTOP
} ChunkOrder;

/*
* Each keyboard may be broken up into chunks (such as rows or columns). All the chunks
* are linked together in a double linked list. Each chunk has a pointer to the first chunkitem
* in the chunk (a chunkitem is a key). The chunkitems are stored in a double linked list.
*/
/* an item in the chunk (a key) */
/* If you add data members to this structure, initialize them in gok_chunker_init_chunkitem */
typedef struct GokChunkitem {
	struct GokChunkitem* pItemNext;
	struct GokChunkitem* pItemPrevious;
	GokKey* pKey;
} GokChunkitem;

/* a chunk (e.g. row or column) */
/* If you add data members to this structure, initialize them in gok_chunker_init_chunk */
typedef struct GokChunk {
	struct GokChunk* pChunkNext;
	struct GokChunk* pChunkPrevious;
	struct GokChunk* pChunkChild;
	gint ChunkId; /* the row or column number */
	gint Left;
	gint Right;
	gint Top;
	gint Bottom;
	struct GokChunkitem* pChunkitem;
} GokChunk;

void gok_chunker_initialize (void);
gboolean gok_chunker_chunk_all (ChunkTypes TypeChunks, ChunkOrder chunkOrder);
gboolean gok_chunker_chunk_none (void);
gboolean gok_chunker_chunk_keys (gboolean TopBottom, gboolean LeftRight);
gboolean gok_chunker_chunk_rows (gboolean TopBottom, gboolean LeftRight);
gboolean gok_chunker_chunk_columns (gboolean TopBottom, gboolean LeftRight);
void gok_chunker_init_chunk (GokChunk* pChunk);
void gok_chunker_delete_chunks (GokChunk* pChunk, gboolean bAlsoNext);
void gok_chunker_reset (void); 
void gok_chunker_next_chunk (void); 
void gok_chunker_previous_chunk (void); 
void gok_chunker_next_key (void); 
void gok_chunker_previous_key (void); 
void gok_chunker_keyup (void); 
void gok_chunker_keydown (void); 
void gok_chunker_keyleft (void);
void gok_chunker_keyright (void); 
void gok_chunker_keyhighlight (void); 
gboolean gok_chunker_wraptofirstchunk (void);
gboolean gok_chunker_wraptolastchunk (void); 
gboolean gok_chunker_wraptofirstkey (void);
gboolean gok_chunker_wraptolastkey (void); 
gboolean gok_chunker_wraptoleft (gint TrueFalse); 
gboolean gok_chunker_wraptoright (gint TrueFalse); 
void gok_chunker_move_leftright (gint TrueFalse); 
void gok_chunker_move_topbottom (gint TrueFalse); 
gboolean gok_chunker_wraptobottom (gint TrueFalse);
gboolean gok_chunker_wraptotop (gint TrueFalse);
gint gok_chunker_if_next_chunk (void); 
gint gok_chunker_if_previous_chunk (void); 
gint gok_chunker_if_next_key (void); 
gint gok_chunker_if_previous_key (void); 
gint gok_chunker_if_top (void); 
gint gok_chunker_if_bottom (void); 
gint gok_chunker_if_left (void); 
gint gok_chunker_if_right (void); 
gint gok_chunker_if_key_selected (void); 
gboolean gok_chunker_is_left (GokKey* pKey1, GokKey* pKey2);
gboolean gok_chunker_is_right (GokKey* pKey1, GokKey* pKey2);
gboolean gok_chunker_is_top (GokKey* pKey1, GokKey* pKey2);
gboolean gok_chunker_is_bottom (GokKey* pKey1, GokKey* pKey2);
void gok_chunker_counter_set (gint CounterId, gint CounterValue); 
void gok_chunker_counter_increment (gint CounterId); 
void gok_chunker_counter_decrement (gint CounterId); 
gint gok_chunker_counter_get (gint CounterId); 
void gok_chunker_state_restart (void); 
void gok_chunker_state_next (void); 
void gok_chunker_state_jump (gchar* NameState); 
gboolean gok_chunker_select_chunk (void);
void gok_chunker_highlight_chunk (GokChunk* pChunk);
void gok_chunker_highlight_chunk_with_key (GokKey* pKey);
void gok_chunker_unhighlight_chunk (GokChunk* pChunk);
void gok_chunker_highlight_center_key (void);
void gok_chunker_highlight_first_chunk (void);
void gok_chunker_highlight_first_key (void);
gboolean gok_chunker_insert_item_row_ltr (GokChunk* pChunk, GokChunkitem* pChunkitem);
gboolean gok_chunker_insert_item_row_rtl (GokChunk* pChunk, GokChunkitem* pChunkitem);
gboolean gok_chunker_insert_item_col_ttb (GokChunk* pChunk, GokChunkitem* pChunkitem);
gboolean gok_chunker_insert_item_col_btt (GokChunk* pChunk, GokChunkitem* pChunkitem);
void gok_chunker_dump_chunks (void);
void gok_chunker_dump_chunk (GokChunk* pChunk);
GokChunk* gok_chunker_chunk_group (GokChunk* pChunk, gint NumberGroups, ChunkOrder Order, gboolean bCanReorder);
void gok_chunker_add_chunkitem (GokChunk* pChunk, GokChunkitem* pChunkitem);
gint gok_chunker_count_chunkitems (GokChunk* pChunk);
GokChunk* gok_chunker_make_2_vertical (GokChunk* pChunk);
GokChunk* gok_chunker_make_2_horizontal (GokChunk* pChunk);
GokChunk* gok_chunker_make_4 (GokChunk* pChunk);
void gok_chunker_highlight_chunk_number (gint Number); 
GokChunk* gok_chunker_get_chunk (gint Number);
void gok_chunker_highlight_all_keys (void);
void gok_chunker_unhighlight_all_keys (void);
void gok_chunker_resolve_overlap (GokChunk* pChunk);
gint gok_chunker_count_keys_in_chunk (GokChunk* pChunk);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __GOKCHUNKER_H__ */
