/* gok-chunker.c
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

/* 
* SPECIAL NOTE:
* gok-chunker needs a rewrite/overhaul/refactor
* we have too many globals (i.e. more than 0)
* we have semi-redundancy among the globals
*
* Also, we need to deprecate nav by "key" -- everything should be a chunk.
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "gok-chunker.h"
#include "main.h"
#include "gok-scanner.h"
#include "gok-settings-dialog.h"
#include "gok-log.h"
#include "gok-feedback.h"
#include <cspi/spi.h>
#include <gtk/gtk.h>

#define CHUNK_HORIZONTAL 0
#define CHUNK_VERTICAL 1

/* private prototypes */
gboolean gok_chunker_chunk_each_key (GokKeyboard* keyboard, ChunkOrder chunkOrder);


/* cell that is currently highlighted */
static gint m_CellHighlightedRow;
static gint m_CellHighlightedColumn;

/* chunk that is currently highlighted */
static GokChunk* m_pChunkHighlighted;

/* chunkitem (a key) that is highlighted */
static GokChunkitem* m_pChunkitemHighlighted;

/* flag will be TRUE if the currently highlighted chunk has been selected */
static gboolean g_bChunkSelected;

/* type of chunks in use */
static ChunkTypes m_ChunkType;
static ChunkOrder m_ChunkOrder;

/* if set then disabled keys are not included in chunks */
static gboolean m_bIgnoreDisabledInChunks = TRUE;

/* maximum number of counters that may be used in the access methods */
#define MAX_COUNTERS 5

/* counters used in the access methods */
static gint m_Counters [MAX_COUNTERS];

/**
* gok_chunker_init_chunk
* @pChunk: Pointer to the chunk that will be initialized.
*
* Initializes a GOK chunk. This must be called after creating any chunk.
**/
void gok_chunker_init_chunk (GokChunk* pChunk)
{
	g_assert (pChunk != NULL);

	pChunk->pChunkNext = NULL;
	pChunk->pChunkPrevious = NULL;
	pChunk->pChunkChild = NULL;
	pChunk->ChunkId = -1;
	pChunk->pChunkitem = NULL;
	pChunk->Left = -1;
	pChunk->Right = -1;
	pChunk->Top = -1;
	pChunk->Bottom = -1;
}

/**
* gok_chunker_init_chunkitem
* @pChunkitem: Pointer to the chunk item that will be initialized.
*
* Initializes a GOK chunk item. This must be called after creating any chunk item.
**/
void gok_chunker_init_chunkitem (GokChunkitem* pChunkitem)
{
	g_assert (pChunkitem != NULL);

	pChunkitem->pItemNext = NULL;
	pChunkitem->pItemPrevious = NULL;
	pChunkitem->pKey = NULL;
}

/**
* gok_chunker_initialize
*
* Initializes the chunker. This must be called before using the chunker.
**/
void gok_chunker_initialize ()
{
	/* initialize global data */
	gok_feedback_set_highlighted_key (NULL);
	gok_feedback_set_selected_key (NULL);
	m_pChunkHighlighted = NULL;
	m_CellHighlightedRow = -1;
	m_CellHighlightedColumn = -1;
	m_ChunkType = CHUNKER_NOCHUNKS;
	m_ChunkOrder = CHUNKER_TOPTOBOTTOM_LEFTTORIGHT;
	g_bChunkSelected = FALSE;
	m_pChunkitemHighlighted = NULL;
}

/**
* gok_chunker_chunk_none
*
* Makes sure all the keyboards have no chunks.
*
* returns: TRUE if everything went OK, FALSE if there was any problem.
**/
gboolean gok_chunker_chunk_none ()
{	
	gok_chunker_initialize();
	return gok_chunker_chunk_all(CHUNKER_NOCHUNKS, 
		CHUNKER_TOPTOBOTTOM_LEFTTORIGHT);
}


/**
* gok_chunker_chunk_keys
* @LeftRight: Direction the keys are ordered. 
* @TopBottom: Direction the rows are ordered.
*
* Breaks all the keyboards into rows and keys.
*
* returns: TRUE if everything went OK, FALSE if there was any problem.
**/
gboolean gok_chunker_chunk_keys (gboolean TopBottom, gboolean LeftRight)
{
	gint orderChunks;

	if (LeftRight == 0)
	{
		if (TopBottom == 0)
		{
			orderChunks = CHUNKER_TOPTOBOTTOM_LEFTTORIGHT;
		}
		else
		{
			orderChunks = CHUNKER_BOTTOMTOTOP_LEFTTORIGHT;
		}
	}
	else
	{
		if (TopBottom == 0)
		{
			orderChunks = CHUNKER_TOPTOBOTTOM_RIGHTTOLEFT;
		}
		else
		{
			orderChunks = CHUNKER_BOTTOMTOTOP_RIGHTTOLEFT;
		}
	}
	return gok_chunker_chunk_all (CHUNKER_KEYS, orderChunks);
}

/**
* gok_chunker_chunk_rows
* @LeftRight: Direction the keys are ordered. 
* @TopBottom: Direction the rows are ordered.
*
* Breaks all the keyboards into rows and keys.
*
* returns: TRUE if everything went OK, FALSE if there was any problem.
**/
gboolean gok_chunker_chunk_rows (gboolean TopBottom, gboolean LeftRight)
{
	gint orderChunks;

	if (LeftRight == 0)
	{
		if (TopBottom == 0)
		{
			orderChunks = CHUNKER_TOPTOBOTTOM_LEFTTORIGHT;
		}
		else
		{
			orderChunks = CHUNKER_BOTTOMTOTOP_LEFTTORIGHT;
		}
	}
	else
	{
		if (TopBottom == 0)
		{
			orderChunks = CHUNKER_TOPTOBOTTOM_RIGHTTOLEFT;
		}
		else
		{
			orderChunks = CHUNKER_BOTTOMTOTOP_RIGHTTOLEFT;
		}
	}
	return gok_chunker_chunk_all (CHUNKER_ROWS, orderChunks);
}

/**
* gok_chunker_chunk_columns
* @LeftRight: Direction the keys are ordered. 
* @TopBottom: Direction the rows are ordered.
*
* Breaks all the keyboards into rows and keys.
*
* returns: TRUE if everything went OK, FALSE if there was any problem.
**/
gboolean gok_chunker_chunk_columns (gboolean LeftRight, gboolean TopBottom)
{
	gint orderChunks;
	
	if (LeftRight == 0)
	{
		if (TopBottom == 0)
		{
			orderChunks = CHUNKER_TOPTOBOTTOM_LEFTTORIGHT;
		}
		else
		{
			orderChunks = CHUNKER_BOTTOMTOTOP_LEFTTORIGHT;
		}
	}
	else
	{
		if (TopBottom == 0)
		{
			orderChunks = CHUNKER_TOPTOBOTTOM_RIGHTTOLEFT;
		}
		else
		{
			orderChunks = CHUNKER_BOTTOMTOTOP_RIGHTTOLEFT;
		}
	}
	return gok_chunker_chunk_all (CHUNKER_COLUMNS, orderChunks);
}
/**
* gok_chunker_chunk_all
* @TypeChunks: The type of chunks required e.g. rows or columns. 
* 			(See the enum 'ChunkTypes' for possible values.)
* @OrderChunks: Order of the chunks e.g. top to bottom or left to right. 
*			(See the enum 'ChunkOrder' for possible values.)
*
* Breaks all the keyboards into chunks.
*
* returns: TRUE if everything went OK, FALSE if there was any problem.
**/
gboolean gok_chunker_chunk_all (ChunkTypes TypeChunks, ChunkOrder OrderChunks)
{
	GokKeyboard* pKeyboard;
	gboolean returnValue = TRUE;
	
	gok_log_enter();

	/* store the chunk type and chunk order */
	m_ChunkType = TypeChunks;
	m_ChunkOrder = OrderChunks;

	/* set all the keyboards as requiring chunking */
	pKeyboard = gok_main_get_first_keyboard();
	while (pKeyboard != NULL)
	{
		pKeyboard->bRequiresChunking = TRUE;
		pKeyboard = pKeyboard->pKeyboardNext;
	}

	m_pChunkHighlighted = NULL;

	gok_log_leave();
	return returnValue;
}

/**
* gok_chunker_dump_chunks
*
* Diagnostic function that prints all the chunks to stdout.
**/
void gok_chunker_dump_chunks ()
{
	GokKeyboard* pKeyboard;
	
	pKeyboard = gok_main_get_first_keyboard();
	while (pKeyboard != NULL)
	{
		gok_log ("++++++keyboard: %s\n", pKeyboard->Name);
		gok_chunker_dump_chunk (pKeyboard->pChunkFirst);

		pKeyboard = pKeyboard->pKeyboardNext;
	}
}

/**
* gok_chunker_dump_chunk
* @pChunk: Pointer to the chunk that gets the dump.
*
* Diagnostic function that prints the chunk to stdout.
**/
void gok_chunker_dump_chunk (GokChunk* pChunk)
{
	GokChunkitem* pChunkitem;
	while (pChunk != NULL)
	{
		gok_log ("----chunk:%d\n", pChunk->ChunkId);
		pChunkitem = pChunk->pChunkitem;
		while (pChunkitem != NULL)
		{
			gok_log ("%s\n", gok_key_get_label (pChunkitem->pKey));
			pChunkitem = pChunkitem->pItemNext;
		}
	
		if (pChunk->pChunkChild != NULL)
		{
			gok_log (">>>>>Child<<<<<<<\n");
			gok_chunker_dump_chunk (pChunk->pChunkChild);
			gok_log (">>>>>Child END<<<<<<<\n");
		}
		
		pChunk = pChunk->pChunkNext;
	}
}

/**
* gok_chunker_chunk
* @pKeyboard: Pointer to the keyboard that will be broken ginto chunks.
*
* Breaks the keyboard ginto chunks (rows, columns etc.). The list of chunks is stored
* on the keyboard
*
* returns: TRUE if keyboard could be broken into chunks, FALSE if not.
**/
gboolean gok_chunker_chunk (GokKeyboard* pKeyboard)
{
	gboolean returnValue = TRUE;

	/* remove highlighting from the currently highlighted chunk */
	if (m_pChunkHighlighted != NULL)
	{
		gok_chunker_unhighlight_chunk (m_pChunkHighlighted);
	}

	/* remove any current chunks on the keyboard */
	gok_chunker_delete_chunks (pKeyboard->pChunkFirst, TRUE);
	pKeyboard->pChunkFirst = NULL; /* very important */
	
	/* what type of chunks are required? */
	switch (m_ChunkType)
	{
		case CHUNKER_NOCHUNKS:
			/* keyboard is not broken into chunks (e.g. direct selection) */
			break;

		case CHUNKER_KEYS:
			/* todo: implement ordering */
			returnValue = gok_chunker_chunk_each_key (pKeyboard, (ChunkOrder)m_ChunkOrder);
			break;

		case CHUNKER_ROWS:
			if ((m_ChunkOrder == CHUNKER_TOPTOBOTTOM_LEFTTORIGHT) ||
				(m_ChunkOrder == CHUNKER_TOPTOBOTTOM_RIGHTTOLEFT))
			{
				returnValue = gok_chunker_chunk_rows_ttb (pKeyboard, m_ChunkOrder);
			}
			else if ((m_ChunkOrder == CHUNKER_BOTTOMTOTOP_LEFTTORIGHT) ||
						(CHUNKER_BOTTOMTOTOP_RIGHTTOLEFT))
			{
				returnValue = gok_chunker_chunk_rows_btt (pKeyboard, m_ChunkOrder);
			}
			else
			{
				gok_log_x ("Warning: Invalid ChunkOrder (%d) in gok_chunker_chunk1 case CHUNKER_ROWS!\n", m_ChunkOrder);
			}
			break;

		case CHUNKER_COLUMNS:
			if ((m_ChunkOrder == CHUNKER_TOPTOBOTTOM_LEFTTORIGHT) ||
				(m_ChunkOrder == CHUNKER_BOTTOMTOTOP_LEFTTORIGHT))
			{
				returnValue = gok_chunker_chunk_cols_ltr (pKeyboard, m_ChunkOrder);
			}
			else if ((m_ChunkOrder == CHUNKER_TOPTOBOTTOM_RIGHTTOLEFT) ||
						(CHUNKER_BOTTOMTOTOP_RIGHTTOLEFT))
			{
				returnValue = gok_chunker_chunk_cols_rtl (pKeyboard, m_ChunkOrder);
			}
			else
			{
				gok_log_x ("Warning: Invalid ChunkOrder (%d) in gok_chunker_chunk2 case CHUNKER_COLUMNS!\n", m_ChunkOrder);
			}
			break;

		case CHUNKER_RECURSIVE:
			returnValue = gok_chunker_chunk_recursive (pKeyboard, m_ChunkOrder, 4);
			break;

		case CHUNKER_2GROUPS:
			break;

		case CHUNKER_3GROUPS:
			break;

		case CHUNKER_4GROUPS:
			break;

		case CHUNKER_5GROUPS:
			break;

		case CHUNKER_6GROUPS:
			break;

		default:
			/* shouldn't reach here! */
			gok_log_x ("Warning: Default hit in gok_chunker_chunk! m_ChunkType = %d\n", m_ChunkType);
			returnValue = FALSE;
			break;
	}

	if (returnValue == FALSE)
	{
		gok_log_x ("Warning: Keyboard '%s' failed gok_chunker_chunk!\n", pKeyboard->Name);
	}

	return returnValue;
}

/* private helper */
GokChunk*
gok_chunker_get_bestchunk (GokChunk* chunk, ChunkOrder order) 
{
	GokChunk* bestChunk;
	GokKey* bestKey;
	GokKey* key;
	gboolean better;
	
	gok_log_enter();
	bestChunk = chunk;
	bestKey = bestChunk->pChunkitem->pKey;
	
	while ((chunk = chunk->pChunkNext) != NULL) {
		better = FALSE;
		key = chunk->pChunkitem->pKey;
		switch (order) {
			case CHUNKER_TOPTOBOTTOM_LEFTTORIGHT:
				if ((key->Top < bestKey->Top) ||
					((key->Top == bestKey->Top) && 
					 (key->Left < bestKey->Left))) {
						 better = TRUE;
				}
				break;	
			case CHUNKER_TOPTOBOTTOM_RIGHTTOLEFT:
				if ((key->Top < bestKey->Top) ||
					((key->Top == bestKey->Top) && 
					 (key->Left > bestKey->Left))) {
						 better = TRUE;
				}
				break;	
			case CHUNKER_BOTTOMTOTOP_LEFTTORIGHT:
				if ((key->Top > bestKey->Top) ||
					((key->Top == bestKey->Top) && 
					 (key->Left < bestKey->Left))) {
						 better = TRUE;
				}
				break;	
			case CHUNKER_BOTTOMTOTOP_RIGHTTOLEFT:
				if ((key->Top > bestKey->Top) ||
					((key->Top == bestKey->Top) && 
					 (key->Left > bestKey->Left))) {
						 better = TRUE;
				}
				break;	
		}
		if (better == TRUE) {
			bestChunk = chunk;
			bestKey = bestChunk->pChunkitem->pKey;
		}
	}
	gok_log_leave();
	return bestChunk;
}

/* private helper */
void
gok_chunker_chunk_move_before (GokChunk* gc1, GokChunk* gc2) 
{
	gok_log("inserting [%s] before [%s]",
		gok_key_get_label(gc2->pChunkitem->pKey),
		gok_key_get_label(gc1->pChunkitem->pKey));

	/* assume gc2 is later than gc1 in the list */
	/* remove gc2*/
	if (gc2->pChunkPrevious != NULL) { 
		gc2->pChunkPrevious->pChunkNext = gc2->pChunkNext;
	}
	if (gc2->pChunkNext != NULL) {
		gc2->pChunkNext->pChunkPrevious = gc2->pChunkPrevious;
	}
	/* insert gc2 before gc1 */
	if (gc1->pChunkPrevious != NULL) { 
		gc1->pChunkPrevious->pChunkNext = gc2; 
	}
	gc2->pChunkPrevious = gc1->pChunkPrevious;
	gc2->pChunkNext = gc1;
	gc1->pChunkPrevious = gc2;
}

		
/* private helper */
GokChunk*
gok_chunker_sort_key_chunks (GokChunk* headChunk, ChunkOrder chunkOrder)
{
	GokChunk* chunk;
	GokChunk* bestChunk;
	GokChunk* newHeadChunk;
	gok_log_enter();
	chunk = headChunk;
	newHeadChunk = NULL;
	while (chunk != NULL) {
		gok_log ("chunk [%s]",gok_key_get_label(chunk->pChunkitem->pKey));
		bestChunk = gok_chunker_get_bestchunk(chunk, chunkOrder);
		if (newHeadChunk == NULL) { newHeadChunk = bestChunk; /* done once */}
		if (bestChunk != chunk) {
			gok_chunker_chunk_move_before (chunk, bestChunk);
		}
		else {
			chunk = chunk->pChunkNext;
		}
	}
	gok_log_leave();
	return newHeadChunk;
}

/**
* gok_chunker_chunk_each_key
* @keyboard: Pointer to the keyboard that will be broken ginto chunks of rows.
* @chunkOrder: Defines the row order (left to right or right to left). See enum ChunkOrder.
*
* Chunk entire keyboard into single key chunks.
*
* returns: TRUE if the keyboard was broken into keys, FALSE if not
**/
gboolean
gok_chunker_chunk_each_key (GokKeyboard* keyboard, ChunkOrder chunkOrder)
{
	GokKey* key;
	GokChunk* chunkNew;
	GokChunk* chunkLast;
	GokChunk* chunkFirst;
	GokChunkitem* chunkitemNew;
	gint i;
	
	gok_log_enter();
	
	chunkFirst = NULL;
	chunkLast = NULL;
	
	/* iterate through all the keys on the keyboard */
	key = keyboard->pKeyFirst;
	i = 0;
	while (key != NULL)
	{
		g_assert (key->pButton != NULL);

		/* ignore keys that are disabled */
		if ((m_bIgnoreDisabledInChunks == TRUE) &&
			(strcmp (gtk_widget_get_name (key->pButton), "StyleButtonDisabled") == 0))
		{
			key = key->pKeyNext;
			continue;
		}
		/* create a new chunk for each key */
		chunkNew = (GokChunk*) g_malloc (sizeof (GokChunk));
		gok_chunker_init_chunk (chunkNew);
		chunkNew->ChunkId = i++;
		chunkNew->pChunkPrevious = chunkLast;
		if (chunkFirst == NULL) {
			chunkFirst = chunkNew;
		}
		else {
			chunkLast->pChunkNext = chunkNew;
		}
		/* create a chunkitem for each chunk */
		chunkitemNew = (GokChunkitem*) g_malloc (sizeof (GokChunkitem));
		gok_chunker_init_chunkitem (chunkitemNew);
		chunkitemNew->pKey = key;
		chunkNew->pChunkitem = chunkitemNew;
		gok_log("chunked key [%s]",gok_key_get_label(key));

		chunkLast = chunkNew;
		key = key->pKeyNext;
	}
	keyboard->pChunkFirst = gok_chunker_sort_key_chunks (chunkFirst, chunkOrder);
	gok_log_leave();
	return TRUE;
}

/**
* gok_chunker_chunk_rows_ttb
* @pKeyboard: Pointer to the keyboard that will be broken ginto chunks of rows.
* @ChunkOrder: Defines the row order (left to right or right to left). See enum ChunkOrder.
*
* Keys are placed in the row in which they appear - starting at the top row and working
* towards the bottom row. If a key spans more than one row it will be placed in the 
* topmost row it occupies.
* The given keyboard should not have any chunks (call gok_chunker_delete_chunks
* before calling this).
*
* returns: TRUE if the keyboard was broken ginto rows, FALSE if not
**/
gboolean gok_chunker_chunk_rows_ttb (GokKeyboard* pKeyboard, gint ChunkOrder)
{
	GokKey* pKey;
	GokChunk* pChunk;
	GokChunk* pChunkNew;
	GokChunk* pChunkFirst;
	GokChunkitem* pChunkitemNew;

	pChunkFirst = NULL;

	/* iterate through all the keys on the keyboard */
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		g_assert (pKey->pButton != NULL);

		/* ignore keys that are disabled */
		if ((m_bIgnoreDisabledInChunks == TRUE) &&
			(strcmp (gtk_widget_get_name (pKey->pButton), "StyleButtonDisabled") == 0))
		{
			pKey = pKey->pKeyNext;
			continue;
		}

		/* create a new chunkitem for each key */
		pChunkitemNew = (GokChunkitem*) g_malloc (sizeof (GokChunkitem));

		/* initialize the chunk item*/
		gok_chunker_init_chunkitem (pChunkitemNew);
		pChunkitemNew->pKey = pKey;

		/* find the chunk (row) in which to place the chunkitem */
		if (pChunkFirst == NULL)
		{

			/* no chunks yet so create the first one */
			pChunkNew = (GokChunk*) g_malloc (sizeof (GokChunk));

			gok_chunker_init_chunk (pChunkNew);
			pChunkNew->pChunkitem = pChunkitemNew;
			pChunkNew->ChunkId = pKey->Top;
			pChunkFirst = pChunkNew;
		}
		else /* chunk gets added to the list of chunks */
		{
			pChunk = pChunkFirst;
			while (pChunk != NULL)
			{
				/* does this chunk have the same row as the key? */
				if (pKey->Top == pChunk->ChunkId)
				{
					/* yes, add the key to the chunk */
					if (ChunkOrder == CHUNKER_TOPTOBOTTOM_LEFTTORIGHT)
					{
						gok_chunker_insert_item_row_ltr (pChunk, pChunkitemNew);
					}
					else
					{
						gok_chunker_insert_item_row_rtl (pChunk, pChunkitemNew);
					}
					break;
				}

				/* does current key go on a chunk (row) less than the current row? */
				else if (pKey->Top < pChunk->ChunkId)
				{
					/* chunk is added before the current chunk */
					/* create a new chunk (row) for the chunkitem (key) */
					pChunkNew = (GokChunk*) g_malloc (sizeof (GokChunk));

					gok_chunker_init_chunk (pChunkNew);
					pChunkNew->pChunkitem = pChunkitemNew;
					pChunkNew->ChunkId = pKey->Top;
					pChunkNew->pChunkPrevious = pChunk->pChunkPrevious;
					pChunkNew->pChunkNext = pChunk;
					if (pChunk->pChunkPrevious != NULL)
					{
						pChunk->pChunkPrevious->pChunkNext = pChunkNew;
					}
					pChunk->pChunkPrevious = pChunkNew;

					if (pChunkFirst == pChunk)
					{
						pChunkFirst = pChunkNew;
					}

					break;
				}

				/* current key is in a following chunk (row) */
				/* does it appear before the next row? */
				else if ((pChunk->pChunkNext == NULL) ||
							(pKey->Top < pChunk->pChunkNext->ChunkId))
				{
					/* create a new chunk (row) for the chunkitem (key) */
					pChunkNew = (GokChunk*) g_malloc (sizeof (GokChunk));

					gok_chunker_init_chunk (pChunkNew);
					pChunkNew->pChunkitem = pChunkitemNew;
					pChunkNew->ChunkId = pKey->Top;
					pChunkNew->pChunkPrevious = pChunk;
					pChunkNew->pChunkNext = pChunk->pChunkNext;
					if (pChunk->pChunkNext != NULL)
					{
						pChunk->pChunkNext->pChunkPrevious = pChunkNew;
					}
					pChunk->pChunkNext = pChunkNew;

					break;
				}

				pChunk = pChunk->pChunkNext;
			}
		}

		pKey = pKey->pKeyNext;
	}

	pKeyboard->pChunkFirst = pChunkFirst;
	return TRUE;
}

/**
* gok_chunker_chunk_rows_btt
* @pKeyboard: Pointer to the keyboard that will be broken ginto chunks of rows.
* @ChunkOrder: Defines the row order (left to right or right to left). See enum ChunkOrder.
*
* Keys are placed in the row in which they appear - starting at the bottom row and working
* towards the top row. If a key spans more than one row it will be placed in the 
* lowest row it occupies.
* The given keyboard should not have any chunks (call gok_chunker_delete_chunks
* before calling this).
*
* returns: TRUE if the keyboard was broken ginto rows, FALSE if not
**/
gboolean gok_chunker_chunk_rows_btt (GokKeyboard* pKeyboard, gint ChunkOrder)
{
	GokKey* pKey;
	GokChunk* pChunk;
	GokChunk* pChunkNew;
	GokChunk* pChunkFirst;
	GokChunkitem* pChunkitemNew;

	pChunkFirst = NULL;

	/* iterate through all the keys on the keyboard */
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		g_assert (pKey->pButton != NULL);

		/* ignore keys that are disabled */
		if ((m_bIgnoreDisabledInChunks == TRUE) &&
			 (strcmp (gtk_widget_get_name (pKey->pButton), "StyleButtonDisabled") == 0))
		{
			pKey = pKey->pKeyNext;
			continue;
		}

		/* create a new chunkitem for each key */
		pChunkitemNew = (GokChunkitem*) g_malloc (sizeof (GokChunkitem));

		/* initialize the chunk item */
		gok_chunker_init_chunkitem (pChunkitemNew);
		pChunkitemNew->pKey = pKey;

		/* find the chunk (row) in which to place the chunkitem */
		if (pChunkFirst == NULL)
		{
			/* no chunks yet so create the first one */
			pChunkNew = (GokChunk*) g_malloc (sizeof (GokChunk));

			gok_chunker_init_chunk (pChunkNew);
			pChunkNew->pChunkitem = pChunkitemNew;
			pChunkNew->ChunkId = pKey->Bottom;
			pChunkFirst = pChunkNew;
		}
		else /* chunk gets added to the list of chunks */
		{
			pChunk = pChunkFirst;
			while (pChunk != NULL)
			{
				/* does this chunk have the same row as the key? */
				if (pKey->Bottom == pChunk->ChunkId)
				{
					/* yes, add the key to the chunk */
					if (ChunkOrder == CHUNKER_BOTTOMTOTOP_LEFTTORIGHT)
					{
						gok_chunker_insert_item_row_ltr (pChunk, pChunkitemNew);
					}
					else
					{
						gok_chunker_insert_item_row_rtl (pChunk, pChunkitemNew);
					}
					break;
				}

				/* does current key go on a chunk (row) less than the current row? */
				else if (pKey->Bottom > pChunk->ChunkId)
				{
					/* chunk is added before the current chunk */
					/* create a new chunk (row) for the chunkitem (key) */
					pChunkNew = (GokChunk*)g_malloc (sizeof (GokChunk));

					gok_chunker_init_chunk (pChunkNew);
					pChunkNew->pChunkitem = pChunkitemNew;
					pChunkNew->ChunkId = pKey->Bottom;
					pChunkNew->pChunkPrevious = pChunk->pChunkPrevious;
					pChunkNew->pChunkNext = pChunk;
					if (pChunk->pChunkPrevious != NULL)
					{
						pChunk->pChunkPrevious->pChunkNext = pChunkNew;
					}
					pChunk->pChunkPrevious = pChunkNew;

					if (pChunkFirst == pChunk)
					{
						pChunkFirst = pChunkNew;
					}
					break;
				}

				/* current key is in a following chunk (row) */
				/* does it appear before the next row? */
				else if ((pChunk->pChunkNext == NULL) ||
							(pKey->Bottom > pChunk->pChunkNext->ChunkId))
				{
					/* create a new chunk (row) for the chunkitem (key) */
					pChunkNew = (GokChunk*) g_malloc (sizeof (GokChunk));

					gok_chunker_init_chunk (pChunkNew);
					pChunkNew->pChunkitem = pChunkitemNew;
					pChunkNew->ChunkId = pKey->Bottom;
					pChunkNew->pChunkPrevious = pChunk;
					pChunkNew->pChunkNext = pChunk->pChunkNext;
					if (pChunk->pChunkNext != NULL)
					{
						pChunk->pChunkNext->pChunkPrevious = pChunkNew;
					}
					pChunk->pChunkNext = pChunkNew;

					break;
				}
				pChunk = pChunk->pChunkNext;
			}
		}

		pKey = pKey->pKeyNext;
	}

	pKeyboard->pChunkFirst = pChunkFirst;
	return TRUE;
}

/**
* gok_chunker_chunk_cols_ltr
* @pKeyboard: Pointer to the keyboard that will be broken ginto chunks of rows.
* @ChunkOrder: Defines the column order (top to bottom or bottom to top). See 
* enum ChunkOrder.
*
* Keys are placed in the column in which they appear - starting at the left column and working
* towards the right row. If a key spans more than one column it will be placed in the 
* leftmost columns it occupies.
* The given keyboard should not have any chunks (call gok_chunker_delete_chunks
* before calling this).
*
* returns: TRUE if the keyboard was broken ginto columns, FALSE if not
**/
gboolean gok_chunker_chunk_cols_ltr (GokKeyboard* pKeyboard, gint ChunkOrder)
{
	GokKey* pKey;
	GokChunk* pChunk;
	GokChunk* pChunkNew;
	GokChunk* pChunkFirst;
	GokChunkitem* pChunkitemNew;

	pChunkFirst = NULL;

	/* iterate through all the keys on the keyboard */
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		g_assert (pKey->pButton != NULL);

		/* ignore keys that are disabled */
		if ((m_bIgnoreDisabledInChunks == TRUE) &&
			 (strcmp (gtk_widget_get_name (pKey->pButton), "StyleButtonDisabled") == 0))
		{
			pKey = pKey->pKeyNext;
			continue;
		}

		/* create a new chunkitem for each key */
		pChunkitemNew = (GokChunkitem*) g_malloc (sizeof (GokChunkitem));

		/* initialize the chunk item*/
		gok_chunker_init_chunkitem (pChunkitemNew);
		pChunkitemNew->pKey = pKey;

		/* find the chunk (row) in which to place the chunkitem */
		if (pChunkFirst == NULL)
		{

			/* no chunks yet so create the first one */
			pChunkNew = (GokChunk*) g_malloc (sizeof (GokChunk));

			gok_chunker_init_chunk (pChunkNew);
			pChunkNew->pChunkitem = pChunkitemNew;
			pChunkNew->ChunkId = pKey->Left;
			pChunkFirst = pChunkNew;
		}
		else /* chunk gets added to the list of chunks */
		{
			pChunk = pChunkFirst;
			while (pChunk != NULL)
			{
				/* does this chunk have the same column as the key? */
				if (pKey->Left == pChunk->ChunkId)
				{
					/* yes, add the key to the chunk */
					if (ChunkOrder == CHUNKER_TOPTOBOTTOM_LEFTTORIGHT)
					{
						gok_chunker_insert_item_col_ttb (pChunk, pChunkitemNew);
					}
					else
					{
						gok_chunker_insert_item_col_btt (pChunk, pChunkitemNew);
					}
					break;
				}

				/* does current key go on a chunk (column) less than the current column? */
				else if (pKey->Left < pChunk->ChunkId)
				{
					/* chunk is added before the current chunk */
					/* create a new chunk (column) for the chunkitem (key) */
					pChunkNew = (GokChunk*) g_malloc (sizeof (GokChunk));

					gok_chunker_init_chunk (pChunkNew);
					pChunkNew->pChunkitem = pChunkitemNew;
					pChunkNew->ChunkId = pKey->Left;
					pChunkNew->pChunkPrevious = pChunk->pChunkPrevious;
					pChunkNew->pChunkNext = pChunk;
					if (pChunk->pChunkPrevious != NULL)
					{
						pChunk->pChunkPrevious->pChunkNext = pChunkNew;
					}
					pChunk->pChunkPrevious = pChunkNew;

					if (pChunkFirst == pChunk)
					{
						pChunkFirst = pChunkNew;
					}

					break;
				}

				/* current key is in a following chunk (column) */
				/* does it appear before the next column? */
				else if ((pChunk->pChunkNext == NULL) ||
							(pKey->Left < pChunk->pChunkNext->ChunkId))
				{
					/* create a new chunk (column) for the chunkitem (key) */
					pChunkNew = (GokChunk*) g_malloc (sizeof (GokChunk));

					gok_chunker_init_chunk (pChunkNew);
					pChunkNew->pChunkitem = pChunkitemNew;
					pChunkNew->ChunkId = pKey->Left;
					pChunkNew->pChunkPrevious = pChunk;
					pChunkNew->pChunkNext = pChunk->pChunkNext;
					if (pChunk->pChunkNext != NULL)
					{
						pChunk->pChunkNext->pChunkPrevious = pChunkNew;
					}
					pChunk->pChunkNext = pChunkNew;

					break;
				}

				pChunk = pChunk->pChunkNext;
			}
		}

		pKey = pKey->pKeyNext;
	}

	pKeyboard->pChunkFirst = pChunkFirst;
	return TRUE;
}

/**
* gok_chunker_chunk_cols_rtl
* @pKeyboard: Pointer to the keyboard that will be broken ginto chunks of columns.
* @ChunkOrder: Defines the column order (top to bottom or bottom to top). See 
* enum ChunkOrder.
*
* Keys are placed in the column in which they appear - starting at the right row 
* and working towards the left column. If a key spans more than one row it will be 
* placed in the rightmost row it occupies.
* The given keyboard should not have any chunks (call gok_chunker_delete_chunks
* before calling this).
*
* returns: TRUE if the keyboard was broken ginto columns, FALSE if not
**/
gboolean gok_chunker_chunk_cols_rtl (GokKeyboard* pKeyboard, gint ChunkOrder)
{
	GokKey* pKey;
	GokChunk* pChunk;
	GokChunk* pChunkNew;
	GokChunk* pChunkFirst;
	GokChunkitem* pChunkitemNew;

	pChunkFirst = NULL;

	/* iterate through all the keys on the keyboard */
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		g_assert (pKey->pButton != NULL);

		/* ignore keys that are disabled */
		if ((m_bIgnoreDisabledInChunks == TRUE) &&
			 (strcmp (gtk_widget_get_name (pKey->pButton), "StyleButtonDisabled") == 0))
		{
			pKey = pKey->pKeyNext;
			continue;
		}

		/* create a new chunkitem for each key */
		pChunkitemNew = (GokChunkitem*) g_malloc (sizeof (GokChunkitem));

		/* initialize the chunk item */
		gok_chunker_init_chunkitem (pChunkitemNew);
		pChunkitemNew->pKey = pKey;

		/* find the chunk (row) in which to place the chunkitem */
		if (pChunkFirst == NULL)
		{
			/* no chunks yet so create the first one */
			pChunkNew = (GokChunk*) g_malloc (sizeof (GokChunk));

			gok_chunker_init_chunk (pChunkNew);
			pChunkNew->pChunkitem = pChunkitemNew;
			pChunkNew->ChunkId = pKey->Right;
			pChunkFirst = pChunkNew;
		}
		else /* chunk gets added to the list of chunks */
		{
			pChunk = pChunkFirst;
			while (pChunk != NULL)
			{
				/* does this chunk have the same column as the key? */
				if (pKey->Right == pChunk->ChunkId)
				{
					/* yes, add the key to the chunk */
					if (ChunkOrder == CHUNKER_TOPTOBOTTOM_RIGHTTOLEFT)
					{
						gok_chunker_insert_item_col_ttb (pChunk, pChunkitemNew);
					}
					else
					{
						gok_chunker_insert_item_col_btt (pChunk, pChunkitemNew);
					}
					break;
				}

				/* does current key go on a chunk (column) less than the current column? */
				else if (pKey->Right > pChunk->ChunkId)
				{
					/* chunk is added before the current chunk */
					/* create a new chunk (column) for the chunkitem (key) */
					pChunkNew = (GokChunk*) g_malloc (sizeof (GokChunk));

					gok_chunker_init_chunk (pChunkNew);
					pChunkNew->pChunkitem = pChunkitemNew;
					pChunkNew->ChunkId = pKey->Right;
					pChunkNew->pChunkPrevious = pChunk->pChunkPrevious;
					pChunkNew->pChunkNext = pChunk;
					if (pChunk->pChunkPrevious != NULL)
					{
						pChunk->pChunkPrevious->pChunkNext = pChunkNew;
					}
					pChunk->pChunkPrevious = pChunkNew;

					if (pChunkFirst == pChunk)
					{
						pChunkFirst = pChunkNew;
					}
					break;
				}

				/* current key is in a following chunk (column) */
				/* does it appear before the next column? */
				else if ((pChunk->pChunkNext == NULL) ||
							(pKey->Right > pChunk->pChunkNext->ChunkId))
				{
					/* create a new chunk (row) for the chunkitem (key) */
					pChunkNew = (GokChunk*) g_malloc (sizeof (GokChunk));

					gok_chunker_init_chunk (pChunkNew);
					pChunkNew->pChunkitem = pChunkitemNew;
					pChunkNew->ChunkId = pKey->Right;
					pChunkNew->pChunkPrevious = pChunk;
					pChunkNew->pChunkNext = pChunk->pChunkNext;
					if (pChunk->pChunkNext != NULL)
					{
						pChunk->pChunkNext->pChunkPrevious = pChunkNew;
					}
					pChunk->pChunkNext = pChunkNew;

					break;
				}
				pChunk = pChunk->pChunkNext;
			}
		}

		pKey = pKey->pKeyNext;
	}

	pKeyboard->pChunkFirst = pChunkFirst;
	return TRUE;
}

/**
* gok_chunker_chunk_recursive
* @pKeyboard: Pointer to the keyboard that will be broken into chunks.
* @ChunkOrder: Defines the column order (top to bottom or bottom to top). See 
* enum ChunkOrder.
* @Groups: Number of groups the chunk should be broken into.
*
* Breaks the keyboard into recursive groups until a group consists of only one key.
*
* returns: TRUE if the keyboard was broken ginto chunks, FALSE if not.
**/
gboolean gok_chunker_chunk_recursive (GokKeyboard* pKeyboard, gint ChunkOrder, gint Groups)
{
	GokKey* pKey;
	GokChunk* pChunkTemp;
	GokChunk* pChunkChunked;
	GokChunkitem* pChunkitemNew;
	GokChunkitem* pChunkitemPrevious;

	pChunkTemp = (GokChunk*)g_malloc (sizeof (GokChunk));
	gok_chunker_init_chunk (pChunkTemp);
	pChunkTemp->ChunkId = 0;
	pChunkTemp->Left = 0;
	pChunkTemp->Right = gok_keyboard_get_number_columns (pKeyboard);
	pChunkTemp->Top = 0;
	pChunkTemp->Bottom = gok_keyboard_get_number_rows (pKeyboard);

	/* create a chunk item for all the keys on the keyboard */
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		g_assert (pKey->pButton != NULL);

		/* ignore keys that are disabled */
		if ((m_bIgnoreDisabledInChunks == TRUE) &&
			 (strcmp (gtk_widget_get_name (pKey->pButton), "StyleButtonDisabled") == 0))
		{
			pKey = pKey->pKeyNext;
			continue;
		}

		pChunkitemNew = (GokChunkitem*) g_malloc (sizeof (GokChunkitem));
		gok_chunker_init_chunkitem (pChunkitemNew);
		pChunkitemNew->pKey = pKey;

		if (pChunkTemp->pChunkitem == NULL)
		{
			pChunkTemp->pChunkitem = pChunkitemNew;
		}
		else
		{
			pChunkitemNew->pItemPrevious = pChunkitemPrevious;
			pChunkitemPrevious->pItemNext = pChunkitemNew;
		}
		
		pChunkitemPrevious = pChunkitemNew;
		pKey = pKey->pKeyNext;
	}

	pChunkChunked = gok_chunker_chunk_group (pChunkTemp, Groups, ChunkOrder, TRUE);
	pKeyboard->pChunkFirst = pChunkChunked;

	gok_chunker_delete_chunks (pChunkTemp, FALSE);

	return TRUE;
}

/**
* gok_chunker_chunk_group
* @pChunk: Pointer to the list of chunks that will be broken into groups.
* @NumberGroups: Number of groups the chunks should be broken into.
* @Order: The order in which the chunks should be created. (see enum ChunkOrder)
* @bCanReorder: TRUE if the chunks can be reordered.
*
* Breaks the given list of chunks into 2 or more groups.
*
* Returns: A pointer to the first chunk in the list of chunks. Returns
* NULL if the given chunk can't be divided into chunks.
**/
GokChunk* gok_chunker_chunk_group (GokChunk* pChunk, gint NumberGroups, ChunkOrder Order, gboolean bCanReorder)
{
	GokChunk* pFirstChunk;
	GokChunk* pChunkLoop;
	GokChunk* pChunkTemp;

	gok_log_enter();
	
	g_assert (pChunk != NULL);
	pFirstChunk = NULL;
	
	if ((NumberGroups != 2) &&
		(NumberGroups != 4))
	{
		gok_log_x ("Must be 2 or 4 items per group!");
		return NULL;
	}
	
	if (gok_chunker_count_chunkitems (pChunk) <= 1)
	{
		return NULL;
	}

	if (NumberGroups == 2)
	{
		/* are the 2 groups horizontal or vertical/ */
		if ((Order == CHUNKER_TOPTOBOTTOM_LEFTTORIGHT) ||
			(Order == CHUNKER_TOPTOBOTTOM_RIGHTTOLEFT) ||
			(Order == CHUNKER_BOTTOMTOTOP_LEFTTORIGHT) ||
			(Order == CHUNKER_BOTTOMTOTOP_RIGHTTOLEFT))
		{
			/* groups are horizontal */
			/* does the chunk span more than one row? */
			if (pChunk->Bottom <= (pChunk->Top + 1))
			{
				/* no, only one row so can't be chunked horizontal */
				/* start chunking vertically */
				if (bCanReorder == TRUE)
				{
					pFirstChunk = gok_chunker_chunk_group (pChunk, NumberGroups, CHUNKER_LEFTTORIGHT_TOPTOBOTTOM, FALSE);
				}
			}
			else
			{
				pFirstChunk = gok_chunker_make_2_horizontal (pChunk);
			}
		}
		else
		{
			/* groups are vertical */
			/* does the chunk span more than one column? */
			if (pChunk->Right <= (pChunk->Left + 1))
			{
				/* no, only one column so can't be chunked vertical */
				/* start chunking horizontally */
				if (bCanReorder == TRUE)
				{
					pFirstChunk = gok_chunker_chunk_group (pChunk, NumberGroups, CHUNKER_BOTTOMTOTOP_RIGHTTOLEFT, FALSE);
				}
			}
			else
			{
				pFirstChunk = gok_chunker_make_2_vertical (pChunk);
			}
		}
	}			
	else /* making 4 chunks */
	{
		pFirstChunk = gok_chunker_make_4 (pChunk);
	}

	/* remove any empty chunks */
	pChunkLoop = pFirstChunk;
	while (pChunkLoop != NULL)
	{
		pChunkTemp = pChunkLoop;
		pChunkLoop = pChunkLoop->pChunkNext;

		if (gok_chunker_count_chunkitems (pChunkTemp) < 1)
		{
			if (pChunkTemp->pChunkPrevious != NULL)
			{
				pChunkTemp->pChunkPrevious->pChunkNext = pChunkTemp->pChunkNext;
			}
			if (pChunkTemp->pChunkNext != NULL)
			{
				pChunkTemp->pChunkNext->pChunkPrevious = pChunkTemp->pChunkPrevious;
			}
			if (pChunkTemp == pFirstChunk)
			{
				if (pChunkTemp->pChunkNext != NULL)
				{
					pFirstChunk = pChunkTemp->pChunkNext;
				}
				else
				{
					pFirstChunk = pChunkTemp->pChunkPrevious;
				}
			}
			
			gok_chunker_delete_chunks (pChunkTemp, FALSE);
			
		}
	}

	/* subdivide each chunk */
	pChunkLoop = pFirstChunk;
	while (pChunkLoop != NULL)
	{
		pChunkLoop->pChunkChild = gok_chunker_chunk_group (pChunkLoop, NumberGroups, Order, TRUE);
		pChunkLoop = pChunkLoop->pChunkNext;
	}
	
	gok_log_leave();
	return pFirstChunk;
}

/**
* gok_chunker_make_2_horizontal
* @pChunk: Pointer to the chunks that will be split.
*
* Create 2 chunks that split the given chunk horizontally.
*
* returns: A pointer to the first chunk, NULL if none could be created.
**/
GokChunk* gok_chunker_make_2_horizontal (GokChunk* pChunk)
{
	GokChunk* pChunk1;
	GokChunk* pChunk2;
	GokChunkitem* pChunkitem;
	GokChunkitem* pChunkitemNew;
	gint midA;
	gint valueTemp;
	
	/* create 2 chunks */
	pChunk1 = (GokChunk*)g_malloc (sizeof (GokChunk));
	gok_chunker_init_chunk (pChunk1);
	pChunk1->ChunkId = 1;
	pChunk2 = (GokChunk*)g_malloc (sizeof (GokChunk));
	gok_chunker_init_chunk (pChunk2);
	pChunk2->ChunkId = 2;
		
	pChunk1->pChunkNext = pChunk2;
	pChunk2->pChunkPrevious = pChunk1;
				
	/* calculate the mid point of the 2 groups */
	valueTemp = (pChunk->Bottom - pChunk->Top) / 2;
	valueTemp += ((pChunk->Bottom - pChunk->Top) - (valueTemp * 2));
	midA = pChunk->Top + valueTemp;
				
	pChunk1->Top = pChunk->Top;
	pChunk1->Bottom = midA;
	pChunk1->Left = pChunk->Left;
	pChunk1->Right = pChunk->Right;
	pChunk2->Top = midA;
	pChunk2->Bottom = pChunk->Bottom;
	pChunk2->Left = pChunk->Left;
	pChunk2->Right = pChunk->Right;

	/* move all the chunkitems into the 2 groups */
	pChunkitem = pChunk->pChunkitem;
	while (pChunkitem != NULL)
	{
		/* create a new chunk item */
		pChunkitemNew = (GokChunkitem*) g_malloc (sizeof (GokChunkitem));
		gok_chunker_init_chunkitem (pChunkitemNew);
		pChunkitemNew->pKey = pChunkitem->pKey;
	
		/* move the chunk item into one of the groups */
		if (pChunkitem->pKey->Top < midA)
		{
			gok_chunker_add_chunkitem (pChunk1, pChunkitemNew);
		}
		else
		{
			gok_chunker_add_chunkitem (pChunk2, pChunkitemNew);
		}
					
		pChunkitem = pChunkitem->pItemNext;
	}
	
	return pChunk1;
}

/**
* gok_chunker_make_2_vertical
* @pChunk: Pointer to the chunk that will be split.
*
* Creates 2 chunks that split the given chunk vertically.
*
* returns: A pointer to the first chunk, NULL if it couldn't be created.
**/
GokChunk* gok_chunker_make_2_vertical (GokChunk* pChunk)
{
	GokChunk* pChunk1;
	GokChunk* pChunk2;
	GokChunkitem* pChunkitem;
	GokChunkitem* pChunkitemNew;
	gint midA;
	gint valueTemp;
	
	/* create 2 chunks */
	pChunk1 = (GokChunk*)g_malloc (sizeof (GokChunk));
	gok_chunker_init_chunk (pChunk1);
	pChunk1->ChunkId = 1;
	pChunk2 = (GokChunk*)g_malloc (sizeof (GokChunk));
	gok_chunker_init_chunk (pChunk2);
	pChunk2->ChunkId = 2;
		
	pChunk1->pChunkNext = pChunk2;
	pChunk2->pChunkPrevious = pChunk1;
				
	/* calculate the mid point of the 2 groups */
	valueTemp = (pChunk->Right - pChunk->Left) / 2;
	valueTemp += ((pChunk->Right - pChunk->Left) - (valueTemp * 2));
	midA = pChunk->Left + valueTemp;

	pChunk1->Left = pChunk->Left;
	pChunk1->Right = midA;
	pChunk1->Top = pChunk->Top;
	pChunk1->Bottom = pChunk->Bottom;
	pChunk2->Left = midA;
	pChunk2->Right = pChunk->Right;
	pChunk2->Top = pChunk->Top;
	pChunk2->Bottom = pChunk->Bottom;
			
	/* move all the chunkitems into the 2 groups */
	pChunkitem = pChunk->pChunkitem;
	while (pChunkitem != NULL)
	{
		/* create a new chunk item */
		pChunkitemNew = (GokChunkitem*) g_malloc (sizeof (GokChunkitem));
		gok_chunker_init_chunkitem (pChunkitemNew);
		pChunkitemNew->pKey = pChunkitem->pKey;

		/* move the chunk item into one of the groups */
		if (pChunkitem->pKey->Left < midA)
		{
			gok_chunker_add_chunkitem (pChunk1, pChunkitemNew);
		}
		else
		{
			gok_chunker_add_chunkitem (pChunk2, pChunkitemNew);
		}
				
		pChunkitem = pChunkitem->pItemNext;
	}

	return pChunk1;
}

/**
* gok_chunker_make_4
* @pChunk: Pointer to the first in a list of chunks that will be split.
*
* Create 4 chunks from the given chunk. The 4 chunks are ordered like:
* 1 2
* 3 4
*
* returns: A pointer to the first chunk, NULL if none could be created.
**/
GokChunk* gok_chunker_make_4 (GokChunk* pChunk)
{
	GokChunk* pChunk1;
	GokChunk* pChunk2;
	GokChunk* pChunk3;
	GokChunk* pChunk4;
	GokChunkitem* pChunkitem;
	GokChunkitem* pChunkitemNew;
	gint midHoriz;
	gint midVert;
	gint valueTemp;

	/* create 4 chunks */
	pChunk1 = (GokChunk*)g_malloc (sizeof (GokChunk));
	gok_chunker_init_chunk (pChunk1);
	pChunk1->ChunkId = 1;
	
	pChunk2 = (GokChunk*)g_malloc (sizeof (GokChunk));
	gok_chunker_init_chunk (pChunk2);
	pChunk2->ChunkId = 2;
	pChunk1->pChunkNext = pChunk2;
	pChunk2->pChunkPrevious = pChunk1;

	pChunk3 = (GokChunk*)g_malloc (sizeof (GokChunk));
	gok_chunker_init_chunk (pChunk3);
	pChunk3->ChunkId = 3;
	pChunk2->pChunkNext = pChunk3;
	pChunk3->pChunkPrevious = pChunk2;

	pChunk4 = (GokChunk*)g_malloc (sizeof (GokChunk));
	gok_chunker_init_chunk (pChunk4);
	pChunk4->ChunkId = 4;
	pChunk3->pChunkNext = pChunk4;
	pChunk4->pChunkPrevious = pChunk3;
				
	/* calculate the horizontal mid point of the groups */
	valueTemp = (pChunk->Bottom - pChunk->Top) / 2;
	valueTemp += ((pChunk->Bottom - pChunk->Top) - (valueTemp * 2));
	midHoriz = pChunk->Top + valueTemp;

	/* calculate the vertical mid point of the groups */
	valueTemp = (pChunk->Right - pChunk->Left) / 2;
	valueTemp += ((pChunk->Right - pChunk->Left) - (valueTemp * 2));
	midVert = pChunk->Left + valueTemp;

	pChunk1->Top = pChunk->Top;
	pChunk1->Bottom = midHoriz;
	pChunk1->Left = pChunk->Left;
	pChunk1->Right = midVert;
	
	pChunk2->Top = pChunk->Top;
	pChunk2->Bottom = midHoriz;
	pChunk2->Left = midVert;
	pChunk2->Right = pChunk->Right;

	pChunk3->Top = midHoriz;
	pChunk3->Bottom = pChunk->Bottom;
	pChunk3->Left = pChunk->Left;
	pChunk3->Right = midVert;
	
	pChunk4->Top = midHoriz;
	pChunk4->Bottom = pChunk->Bottom;
	pChunk4->Left = midVert;
	pChunk4->Right = pChunk->Right;

	/* move all the chunkitems into the groups */
	pChunkitem = pChunk->pChunkitem;
	while (pChunkitem != NULL)
	{
		/* create a new chunk item */
		pChunkitemNew = (GokChunkitem*) g_malloc (sizeof (GokChunkitem));
		gok_chunker_init_chunkitem (pChunkitemNew);
		pChunkitemNew->pKey = pChunkitem->pKey;
	
		/* move the chunk item into one of the groups */
		if (pChunkitem->pKey->Top < midHoriz)
		{
			if (pChunkitem->pKey->Left < midVert)
			{
				gok_chunker_add_chunkitem (pChunk1, pChunkitemNew);
			}
			else
			{
				gok_chunker_add_chunkitem (pChunk2, pChunkitemNew);
			}
		}
		else
		{
			if (pChunkitem->pKey->Left < midVert)
			{
				gok_chunker_add_chunkitem (pChunk3, pChunkitemNew);
			}
			else
			{
				gok_chunker_add_chunkitem (pChunk4, pChunkitemNew);
			}
		}
					
		pChunkitem = pChunkitem->pItemNext;
	}

	gok_chunker_resolve_overlap (pChunk1);
	
	return pChunk1;
}

/**
* gok_chunker_resolve_overlap
* @pChunk: Pointer to the list of chunke sthat need to be resolved.
*
* If one of the chunks has more than one item, move an item to another chunk.
**/
void gok_chunker_resolve_overlap (GokChunk* pChunk)
{
	GokChunk* pChunkTest;
	GokChunk* pChunkOverlap;
	GokChunk* pChunkEmpty;
	gint countValidGroups;
	
	/* count the number of valid groups */
	pChunkTest = pChunk;
	pChunkEmpty = NULL;
	countValidGroups = 0;
	pChunkEmpty = NULL;
	while (pChunkTest != NULL)
	{
		if (gok_chunker_count_chunkitems (pChunkTest) > 0)
		{
			countValidGroups++;
			pChunkOverlap = pChunk;
		}
		else
		{
			/* keep track of the first empty chunk */
			if (pChunkEmpty == NULL)
			{
				pChunkEmpty = pChunkTest;
			}
		}

		pChunkTest = pChunkTest->pChunkNext;
	}
	
	/* if all the keys are in one group then move one key to another group */
	if ((countValidGroups == 1) &&
		(gok_chunker_count_chunkitems (pChunkOverlap) > 1))
	{
		pChunkEmpty->pChunkitem = pChunkOverlap->pChunkitem;
		pChunkOverlap->pChunkitem = pChunkEmpty->pChunkitem->pItemNext;
		pChunkOverlap->pChunkitem->pItemPrevious = NULL;
		pChunkEmpty->pChunkitem->pItemNext = NULL;
	}
}

/**
* gok_chunker_count_chunkitems
* @pChunk: Pointer to the Chunk that will be counted.
*
* Counts the number of items in the given chunk.
*
* returns: The number of items in the chunk.
**/
gint gok_chunker_count_chunkitems (GokChunk* pChunk)
{
	GokChunkitem* pItem;
	gint count = 0;
	
	if (pChunk == NULL)
	{
		return 0;
	}

	pItem = pChunk->pChunkitem;
	while (pItem != NULL)
	{
		count++;
		pItem = pItem->pItemNext;
	}

	return count;
}

/**
* gok_chunker_add_chunkitem
* @pChunk: Pointer to the chunk that will contain the new chunkitem.
* @pChunkitem: Pointer to the chunkitem that gets added to the chunk.
*
* Adds a chunkitem (key) to a chunk (group). The new chunkitem will be added
* as the last chunkitem in the list.
**/
void gok_chunker_add_chunkitem (GokChunk* pChunk, GokChunkitem* pChunkitemNew)
{
	GokChunkitem* pChunkitem;
	
	g_assert (pChunk != NULL);
	g_assert (pChunkitemNew != NULL);
	
	if (pChunk->pChunkitem == NULL)
	{
		pChunk->pChunkitem = pChunkitemNew;
		return;
	}
	
	pChunkitem = pChunk->pChunkitem;
	while (pChunkitem->pItemNext != NULL)
	{
		pChunkitem = pChunkitem->pItemNext;
	}
	
	pChunkitem->pItemNext = pChunkitemNew;
	pChunkitemNew->pItemPrevious = pChunkitem;
}

/**
* gok_chunker_insert_item_row_ltr
* @pChunk: Pointer to the first chunk that will contain the new chunkitem.
* @pChunkitem: Pointer to the chunkitem that gets added to the chunk.
*
* Adds a chunkitem (key) to a chunk (row). The new chunkitem will be added in the
* sequence left to right (the leftmost key is the first key in the sequence).
*
* returns: TRUE if the chunkitem was added to the chunk, FALSE if not.
**/
gboolean gok_chunker_insert_item_row_ltr (GokChunk* pChunk, GokChunkitem* pChunkitem)
{
	GokChunkitem* pItemCurrent;

	g_assert (pChunk != NULL);
	g_assert (pChunkitem != NULL);
	g_assert (pChunkitem->pKey != NULL);

	/* does the chunk have no items? */
	if (pChunk->pChunkitem == NULL)
	{
		/* yes, add the chunkitem as the first item on the chunk */
		pChunk->pChunkitem = pChunkitem;
		return TRUE;
	}

	/* insert the new chunkitem ginto the list of chunkitems on the chunk */
	pItemCurrent = pChunk->pChunkitem;
	while (pItemCurrent != NULL)
	{
		g_assert (pItemCurrent->pKey != NULL);
		/* does the new item go before the current item? */
		if ((pChunkitem->pKey->Left < pItemCurrent->pKey->Left) ||
			(pChunkitem->pKey->Left == pItemCurrent->pKey->Left))
		{
			/* yes, insert the new item before the current item */
			pChunkitem->pItemNext = pItemCurrent;
			pChunkitem->pItemPrevious = pItemCurrent->pItemPrevious;
			if (pItemCurrent->pItemPrevious != NULL)
			{
				pItemCurrent->pItemPrevious->pItemNext = pChunkitem;
			}
			pItemCurrent->pItemPrevious = pChunkitem;

			if (pChunk->pChunkitem == pItemCurrent)
			{
				pChunk->pChunkitem = pChunkitem;
			}
			break;
		}

		/* does this new item go after the current item? */
		else if ((pItemCurrent->pItemNext == NULL) ||
					(pItemCurrent->pItemNext->pKey->Left > pChunkitem->pKey->Left))
		{
			/* yes, insert the new item after the current item */
			pChunkitem->pItemNext = pItemCurrent->pItemNext;
			pChunkitem->pItemPrevious = pItemCurrent;
			if (pItemCurrent->pItemNext != NULL)
			{
				pItemCurrent->pItemNext->pItemPrevious = pChunkitem;
			}
			pItemCurrent->pItemNext = pChunkitem;
			break;
		}
		pItemCurrent = pItemCurrent->pItemNext;
	}

	/* if pItemCurrent is NULL then new item was not added! */
	g_assert (pItemCurrent != NULL);

	return TRUE;
}

/**
* gok_chunker_insert_item_row_rtl
* @pChunk: Pointer to the first chunk that will contain the new chunkitem.
* @pChunkitem: Pointer to the chunkitem that gets added to the chunk.
*
* Adds a chunkitem (key) to a chunk (row). The new chunkitem will be added in the
* sequence right to left (the rightmost key is the first key in the sequence).
*
* returns: TRUE if the chunkitem was added to the chunk, FALSE if not.
**/
gboolean gok_chunker_insert_item_row_rtl (GokChunk* pChunk, GokChunkitem* pChunkitem)
{
	GokChunkitem* pItemCurrent;

	g_assert (pChunk != NULL);
	g_assert (pChunkitem != NULL);
	g_assert (pChunkitem->pKey != NULL);

	/* does the chunk have no items? */
	if (pChunk->pChunkitem == NULL)
	{
		/* yes, add the chunkitem as the first item on the chunk */
		pChunk->pChunkitem = pChunkitem;
		return TRUE;
	}

	/* insert the new chunkitem ginto the list of chunkitems on the chunk */
	pItemCurrent = pChunk->pChunkitem;
	while (pItemCurrent != NULL)
	{
		g_assert (pItemCurrent->pKey != NULL);
		/* does the new item go before the current item? */
		if ((pChunkitem->pKey->Right > pItemCurrent->pKey->Right) ||
			(pChunkitem->pKey->Right == pItemCurrent->pKey->Right))
		{
			/* yes, insert the new item before the current item */
			pChunkitem->pItemNext = pItemCurrent;
			pChunkitem->pItemPrevious = pItemCurrent->pItemPrevious;
			if (pItemCurrent->pItemPrevious != NULL)
			{
				pItemCurrent->pItemPrevious->pItemNext = pChunkitem;
			}
			pItemCurrent->pItemPrevious = pChunkitem;

			if (pChunk->pChunkitem == pItemCurrent)
			{
				pChunk->pChunkitem = pChunkitem;
			}
			break;
		}

		/* does this new item go after the current item? */
		else if ((pItemCurrent->pItemNext == NULL) ||
					(pItemCurrent->pItemNext->pKey->Left < pChunkitem->pKey->Left))
		{
			/* yes, insert the new item after the current item */
			pChunkitem->pItemNext = pItemCurrent->pItemNext;
			pChunkitem->pItemPrevious = pItemCurrent;
			if (pItemCurrent->pItemNext != NULL)
			{
				pItemCurrent->pItemNext->pItemPrevious = pChunkitem;
			}
			pItemCurrent->pItemNext = pChunkitem;
			break;
		}
		pItemCurrent = pItemCurrent->pItemNext;
	}

	/* if pItemCurrent is NULL then new item was not added! */
	g_assert (pItemCurrent != NULL);

	return TRUE;
}

/**
* gok_chunker_insert_item_col_ttb
* @pChunk: Pointer to the first chunk that will contain the new chunkitem.
* @pChunkitem: Pointer to the chunkitem that gets added to the chunk.
*
* Adds a chunkitem (key) to a chunk (column). The new chunkitem will be added in the
* sequence top to bottom (the topmost key is the first key in the sequence).
*
* returns: TRUE if the chunkitem was added to the chunk, FALSE if not.
**/
gboolean gok_chunker_insert_item_col_ttb (GokChunk* pChunk, GokChunkitem* pChunkitem)
{
	GokChunkitem* pItemCurrent;

	g_assert (pChunk != NULL);
	g_assert (pChunkitem != NULL);
	g_assert (pChunkitem->pKey != NULL);

	/* does the chunk have no items? */
	if (pChunk->pChunkitem == NULL)
	{
		/* yes, add the chunkitem as the first item on the chunk */
		pChunk->pChunkitem = pChunkitem;
		return TRUE;
	}

	/* insert the new chunkitem ginto the list of chunkitems on the chunk */
	pItemCurrent = pChunk->pChunkitem;
	while (pItemCurrent != NULL)
	{
		g_assert (pItemCurrent->pKey != NULL);
		/* does the new item go before the current item? */
		if ((pChunkitem->pKey->Top < pItemCurrent->pKey->Top) ||
			(pChunkitem->pKey->Top == pItemCurrent->pKey->Top))
		{
			/* yes, insert the new item before the current item */
			pChunkitem->pItemNext = pItemCurrent;
			pChunkitem->pItemPrevious = pItemCurrent->pItemPrevious;
			if (pItemCurrent->pItemPrevious != NULL)
			{
				pItemCurrent->pItemPrevious->pItemNext = pChunkitem;
			}
			pItemCurrent->pItemPrevious = pChunkitem;

			if (pChunk->pChunkitem == pItemCurrent)
			{
				pChunk->pChunkitem = pChunkitem;
			}
			break;
		}

		/* does this new item go after the current item? */
		else if ((pItemCurrent->pItemNext == NULL) ||
					(pItemCurrent->pItemNext->pKey->Top > pChunkitem->pKey->Top))
		{
			/* yes, insert the new item after the current item */
			pChunkitem->pItemNext = pItemCurrent->pItemNext;
			pChunkitem->pItemPrevious = pItemCurrent;
			if (pItemCurrent->pItemNext != NULL)
			{
				pItemCurrent->pItemNext->pItemPrevious = pChunkitem;
			}
			pItemCurrent->pItemNext = pChunkitem;
			break;
		}
		pItemCurrent = pItemCurrent->pItemNext;
	}

	/* if pItemCurrent is NULL then new item was not added! */
	g_assert (pItemCurrent != NULL);

	return TRUE;
}

/**
* gok_chunker_insert_item_col_btt
* @pChunk: Pointer to the first chunk that will contain the new chunkitem.
* @pChunkitem: Pointer to the chunkitem that gets added to the chunk.
*
* Adds a chunkitem (key) to a chunk (column). The new chunkitem will be added in the
* sequence bottom to top (the bottommost key is the first key in the sequence).
*
* returns: TRUE if the chunkitem was added to the chunk, FALSE if not.
**/
gboolean gok_chunker_insert_item_col_btt (GokChunk* pChunk, GokChunkitem* pChunkitem)
{
	GokChunkitem* pItemCurrent;

	g_assert (pChunk != NULL);
	g_assert (pChunkitem != NULL);
	g_assert (pChunkitem->pKey != NULL);

	/* does the chunk have no items? */
	if (pChunk->pChunkitem == NULL)
	{
		/* yes, add the chunkitem as the first item on the chunk */
		pChunk->pChunkitem = pChunkitem;
		return TRUE;
	}

	/* insert the new chunkitem ginto the list of chunkitems on the chunk */
	pItemCurrent = pChunk->pChunkitem;
	while (pItemCurrent != NULL)
	{
		g_assert (pItemCurrent->pKey != NULL);
		/* does the new item go before the current item? */
		if ((pChunkitem->pKey->Bottom > pItemCurrent->pKey->Bottom) ||
			(pChunkitem->pKey->Bottom == pItemCurrent->pKey->Bottom))
		{
			/* yes, insert the new item before the current item */
			pChunkitem->pItemNext = pItemCurrent;
			pChunkitem->pItemPrevious = pItemCurrent->pItemPrevious;
			if (pItemCurrent->pItemPrevious != NULL)
			{
				pItemCurrent->pItemPrevious->pItemNext = pChunkitem;
			}
			pItemCurrent->pItemPrevious = pChunkitem;

			if (pChunk->pChunkitem == pItemCurrent)
			{
				pChunk->pChunkitem = pChunkitem;
			}
			break;
		}

		/* does this new item go after the current item? */
		else if ((pItemCurrent->pItemNext == NULL) ||
					(pItemCurrent->pItemNext->pKey->Top < pChunkitem->pKey->Top))
		{
			/* yes, insert the new item after the current item */
			pChunkitem->pItemNext = pItemCurrent->pItemNext;
			pChunkitem->pItemPrevious = pItemCurrent;
			if (pItemCurrent->pItemNext != NULL)
			{
				pItemCurrent->pItemNext->pItemPrevious = pChunkitem;
			}
			pItemCurrent->pItemNext = pChunkitem;
			break;
		}
		pItemCurrent = pItemCurrent->pItemNext;
	}

	/* if pItemCurrent is NULL then new item was not added! */
	g_assert (pItemCurrent != NULL);

	return TRUE;
}

/**
* gok_chunker_delete_chunks
* @pChunk: Pointer to the first chunk in the list of chunks that gets deleted.
* @bAlsoNext: If TRUE then all chunks following the one given will be deleted.
*
* Deletes a chunk and, optionally, any following chunks.
**/
void gok_chunker_delete_chunks (GokChunk* pChunk, gboolean bAlsoNext)
{
	GokChunk* pChunkTemp;
	GokChunkitem* pChunkitem;
	GokChunkitem* pChunkitemTemp;

	gok_log_enter();
	while (pChunk != NULL)
	{
		/* delete all the chunk items */
		pChunkitem = pChunk->pChunkitem;
		while (pChunkitem != NULL)
		{
			pChunkitemTemp = pChunkitem;
			pChunkitem = pChunkitem->pItemNext;
			g_free (pChunkitemTemp);
		}

		pChunkTemp = pChunk;
		pChunk = pChunk->pChunkNext;

		/* delete any children of the chunk */
		if (pChunkTemp->pChunkChild != NULL)
		{
			gok_chunker_delete_chunks (pChunkTemp->pChunkChild, TRUE);
		}
		
		/* defensive */
		if (pChunkTemp == m_pChunkHighlighted) {
			gok_chunker_unhighlight_chunk (m_pChunkHighlighted);
			m_pChunkHighlighted = NULL;
		}
		
		g_free (pChunkTemp);
		
		/* keep deleting the following chunks? */
		if (bAlsoNext == FALSE)
		{
			/* nope, stop deleting */
			break;
		}
	}
	gok_log_leave();
}

/**
* gok_chunker_reset
*
**/
void gok_chunker_reset ()
{
	/* FIXME: remove all existing chunks ?? */
	/* and call gok_chunker_initialize  ?? */
}

/**
* gok_chunker_next_chunk
*
* Highlightes the next chunk.
**/
void gok_chunker_next_chunk ()
{
	GokChunk* pChunkNext;
	
	if (m_pChunkHighlighted == NULL)
	{
		gok_log_x ("Warning: m_pChunkHighlighted is NULL!\n");
	}
	else
	{
		if (m_pChunkHighlighted->pChunkNext == NULL)
		{
			gok_log_x ("Warning: m_pChunkHighlighted->pChunkNext is NULL!\n");
		}
		else
		{
			pChunkNext = m_pChunkHighlighted->pChunkNext;
			gok_chunker_unhighlight_chunk (m_pChunkHighlighted);
			gok_chunker_highlight_chunk (pChunkNext);
		}
	}
}

/**
* gok_chunker_previous_chunk
*
* Highlightes the previous chunk.
**/
void gok_chunker_previous_chunk ()
{
	GokChunk* pChunkPrevious;
	
	if (m_pChunkHighlighted == NULL)
	{
		gok_log_x ("Warning: gok_chunker_previous_chunk failed because m_pChunkHighlighted is NULL!\n");
	}
	else
	{
		if (m_pChunkHighlighted->pChunkPrevious == NULL)
		{
			gok_log_x ("Warning: gok_chunker_previous_chunk failed because m_pChunkHighlighted->pChunkNext is NULL!\n");
		}
		else
		{
			pChunkPrevious = m_pChunkHighlighted->pChunkPrevious;
			gok_chunker_unhighlight_chunk (m_pChunkHighlighted);
			gok_chunker_highlight_chunk (pChunkPrevious);
		}
	}
}

/**
* gok_chunker_next_key
*
* Highlightes the next key in the chunk.
**/
void gok_chunker_next_key ()
{
	g_assert (m_pChunkitemHighlighted != NULL);

	if (m_pChunkitemHighlighted->pItemNext == NULL)
	{
		gok_log_x ("Warning: gok_chunker_next_key failed because m_pChunkitemHighlighted->pItemNext is NULL!\n");
	}
	else
	{
		gok_feedback_unhighlight (m_pChunkitemHighlighted->pKey, FALSE);
		m_pChunkitemHighlighted = m_pChunkitemHighlighted->pItemNext;
		gok_feedback_highlight (m_pChunkitemHighlighted->pKey, FALSE);

		/* this is now the selected key */
		gok_feedback_set_selected_key (m_pChunkitemHighlighted->pKey);
	}
}

/**
* gok_chunker_previous_key
*
* Highlightes the previous key in the selected chunk.
**/
void gok_chunker_previous_key ()
{
	g_assert (m_pChunkitemHighlighted != NULL);

	if (m_pChunkitemHighlighted->pItemPrevious == NULL)
	{
		gok_log_x ("Warning: gok_chunker_previous_key failed because m_pChunkitemHighlighted->pItemPrevious is NULL!\n");
	}
	else
	{
		gok_feedback_unhighlight (m_pChunkitemHighlighted->pKey, FALSE);
		m_pChunkitemHighlighted = m_pChunkitemHighlighted->pItemPrevious;
		gok_feedback_highlight (m_pChunkitemHighlighted->pKey, FALSE);

		/* this is now the selected key */
		gok_feedback_set_selected_key (m_pChunkitemHighlighted->pKey);
	}
}

/**
* gok_chunker_keyup
*
* Highlights and selects the key to the top of the currently highlighted key.
* This does NOT wrap to the bottom side of the keyboard (call gok_chunker_wraptobottom).
**/
void gok_chunker_keyup ()
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;
	GokKey* pKeyToTop;
	GokKey* pKeyHighlighted;

fprintf(stderr, "gok_chunker_keyup\n");
	pKeyHighlighted = gok_feedback_get_highlighted_key();

	pKeyboard = gok_main_get_current_keyboard();
	g_assert (pKeyboard != NULL);

	/* find the key that is in the next row above */
	pKeyToTop = NULL;
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		/* ignore the key currently highlighted */
		if (pKey != pKeyHighlighted)
		{
			/* is the key on the same column as the current highlight? */
			if ((pKey->Right >= m_CellHighlightedColumn) &&
				(pKey->Left < m_CellHighlightedColumn))
			{
				if (gok_chunker_is_top (pKey, /*gok_feedback_get_selected_key*/gok_feedback_get_highlighted_key()) == TRUE)
				{
					if ((pKeyToTop == NULL) ||
						(gok_chunker_is_top (pKeyToTop, pKey) == TRUE))
					{
						pKeyToTop = pKey;
					}
				}
			}
		}
		pKey = pKey->pKeyNext;
	}

	if (pKeyToTop != NULL)
	{
		gok_feedback_unhighlight (pKeyHighlighted, FALSE);
		gok_feedback_highlight (pKeyToTop, FALSE);	
		gok_chunker_highlight_chunk_with_key (pKeyToTop);
		gok_feedback_set_selected_key (pKeyToTop);
		gok_feedback_set_highlighted_key (pKeyToTop);
		m_CellHighlightedRow = pKeyToTop->Bottom;
	}
}

/**
* gok_chunker_keydown
*
* Highlights and selects the right to the bottom of the currently highlighted key.
* This does NOT wrap to the top side of the keyboard (call gok_chunker_wraptotop).
**/
void gok_chunker_keydown ()
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;
	GokKey* pKeyToBottom;
	GokKey* pKeyHighlighted;

	pKeyHighlighted = gok_feedback_get_highlighted_key();

	pKeyboard = gok_main_get_current_keyboard();
	g_assert (pKeyboard != NULL);

	/* find the key that is in the next row down */
	pKeyToBottom = NULL;
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		/* ignore the key currently highlighted */
		if (pKey != pKeyHighlighted)
		{
			/* is the key on the same column as the current highlight? */
			if ((pKey->Right >= m_CellHighlightedColumn) &&
				(pKey->Left < m_CellHighlightedColumn))
			{
				if (gok_chunker_is_bottom (pKey, gok_feedback_get_selected_key()) == TRUE)
				{
					if ((pKeyToBottom == NULL) ||
						(gok_chunker_is_bottom (pKeyToBottom, pKey) == TRUE))
					{
						pKeyToBottom = pKey;
					}
				}
			}
		}
		pKey = pKey->pKeyNext;
	}

	if (pKeyToBottom != NULL)
	{
		gok_feedback_unhighlight (pKeyHighlighted, FALSE);
		gok_feedback_highlight (pKeyToBottom, FALSE);	
		gok_chunker_highlight_chunk_with_key (pKeyToBottom);
		gok_feedback_set_selected_key (pKeyToBottom);
		m_CellHighlightedRow = pKeyToBottom->Top + 1;
	}
}

/**
* gok_chunker_keyleft
*
* Highlights and selects the key to the left of the currently highlighted key.
* This does NOT wrap to the right side of the keyboard (call gok_chunker_wraptoright).
**/
void gok_chunker_keyleft ()
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;
	GokKey* pKeyToLeft;
	GokKey* pKeyHighlighted;

	pKeyHighlighted = gok_feedback_get_highlighted_key();

	pKeyboard = gok_main_get_current_keyboard();
	g_assert (pKeyboard != NULL);

	/* find the key that is in the next column to the left */
	pKeyToLeft = NULL;
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		/* ignore the key currently highlighted */
		if (pKey != pKeyHighlighted)
		{
			/* is the key on the same row as the current highlight? */
			if ((pKey->Bottom >= m_CellHighlightedRow) &&
				(pKey->Top < m_CellHighlightedRow))
			{
				if (gok_chunker_is_left (pKey, gok_feedback_get_selected_key()) == TRUE)
				{
					if ((pKeyToLeft == NULL) ||
						(gok_chunker_is_left (pKeyToLeft, pKey) == TRUE))
					{
						pKeyToLeft = pKey;
					}
				}
			}
		}
		pKey = pKey->pKeyNext;
	}

	if (pKeyToLeft != NULL)
	{
		gok_feedback_unhighlight (pKeyHighlighted, FALSE);
		gok_feedback_highlight (pKeyToLeft, FALSE);	
		gok_chunker_highlight_chunk_with_key (pKeyToLeft);
		gok_feedback_set_selected_key (pKeyToLeft);
		gok_feedback_set_highlighted_key (pKeyToLeft);
		m_CellHighlightedColumn = pKeyToLeft->Right;
	}
}

/**
* gok_chunker_keyright
*
* Highlights and selects the right to the right of the currently highlighted key.
* This does NOT wrap to the left side of the keyboard (call gok_chunker_wraptoleft).
**/
void gok_chunker_keyright ()
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;
	GokKey* pKeyToRight;
	GokKey* pKeyHighlighted;
	
	gok_log_enter();
	pKeyHighlighted = gok_feedback_get_highlighted_key();

	pKeyboard = gok_main_get_current_keyboard();
	g_assert (pKeyboard != NULL);

	/* find the key that is in the next column to the right */
	pKeyToRight = NULL;
	pKey = pKeyboard->pKeyFirst;

	while (pKey != NULL)
	{
		/* ignore the key currently highlighted */
		if (pKey != pKeyHighlighted)
		{
			/* is the key on the same row as the current highlight? */
			if ((pKey->Bottom >= m_CellHighlightedRow) &&
				(pKey->Top < m_CellHighlightedRow))
			{
				if (gok_chunker_is_right (pKey, pKeyHighlighted) == TRUE)
				{
					if ((pKeyToRight == NULL) ||
						(gok_chunker_is_right (pKeyToRight, pKey) == TRUE))
					{
						pKeyToRight = pKey;
					}
				}
			}
		}
		pKey = pKey->pKeyNext;
	}

	if (pKeyToRight != NULL)
	{
		gok_feedback_unhighlight (pKeyHighlighted, FALSE);
		gok_feedback_highlight (pKeyToRight, FALSE);	
		gok_chunker_highlight_chunk_with_key (pKeyToRight);
		gok_feedback_set_selected_key (pKeyToRight);
		gok_feedback_set_highlighted_key (pKeyToRight);
		m_CellHighlightedColumn = pKeyToRight->Left + 1;
	}
	gok_log_leave();
}

/**
* gok_chunker_highlight_chunk
* @pChunk: Pointer to the chunk that will be highlighted.
*
* Highlightes the given chunk.
* Updates m_pChunkHighlighted with the chunk given.
**/
void gok_chunker_highlight_chunk (GokChunk* pChunk)
{
	GokChunkitem* pChunkitem;

	g_assert (pChunk != NULL);
	gok_log_enter();

	pChunkitem = pChunk->pChunkitem;
	while (pChunkitem != NULL)
	{
		if (pChunkitem->pKey != NULL)
		{
			gok_feedback_highlight (pChunkitem->pKey, FALSE);
		}
		pChunkitem = pChunkitem->pItemNext;
	}

	m_pChunkHighlighted = pChunk;

	/* no key selected if a chunk is  highlighted */
	gok_feedback_set_selected_key (NULL);
	gok_log_leave();
}

/**
* gok_chunker_unhighlight_chunk
* @pChunk: Pointer to the chunk that will be unhighlighed (made normal).
*
* Unhighlightes the given chunk.
* Updates m_pChunkHighlighted.
**/
void gok_chunker_unhighlight_chunk (GokChunk* pChunk)
{
	GokChunkitem* pChunkitem;

	g_assert (pChunk != NULL);

	pChunkitem = pChunk->pChunkitem;
	while (pChunkitem != NULL)
	{
		if (pChunkitem->pKey != NULL)
		{
			gok_feedback_unhighlight (pChunkitem->pKey, FALSE);
		}
		pChunkitem = pChunkitem->pItemNext;
	}
	
	/* update this global */
	m_pChunkHighlighted = NULL;
}

/**
* gok_chunker_highlight_first_chunk
*
* Highlightes the first chunk on the current keyboard.
* Updates m_pChunkHighlighted.
**/
void gok_chunker_highlight_first_chunk ()
{
	GokKeyboard* pKeyboard;

	gok_log_enter();
	
	/* unhighlight any keys that are highlighted */
	gok_chunker_unhighlight_all_keys();

	/* highlight the first chunk */
	pKeyboard = gok_main_get_current_keyboard ();
	if (pKeyboard != NULL)
	{
		m_pChunkHighlighted = pKeyboard->pChunkFirst;
		if (pKeyboard->pChunkFirst != NULL)
		{
			gok_chunker_highlight_chunk (pKeyboard->pChunkFirst);
			/* if this is a single key chunk then set selected key 
			   which is a kludge for directed scanning - fixme */
			if (pKeyboard->pChunkFirst->pChunkNext == NULL) {
				if (pKeyboard->pChunkFirst->pChunkitem->pItemNext == NULL) {
					if (pKeyboard->pChunkFirst->pChunkitem->pKey != NULL) {
						gok_feedback_set_selected_key (pKeyboard->pChunkFirst->pChunkitem->pKey);
					}
				}
			}
		}
	}
	
	gok_log_leave();
}

/**
* gok_chunker_highlight_all_keys
*
* Highlightes all keys on the current keyboard.
**/
void gok_chunker_highlight_all_keys ()
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;
	
	pKeyboard = gok_main_get_current_keyboard ();
	if (pKeyboard != NULL)
	{
		pKey = pKeyboard->pKeyFirst;
		while (pKey != NULL)
		{
			gok_feedback_highlight (pKey, FALSE);
			pKey = pKey->pKeyNext;
		}
	}
}

/**
* gok_chunker_unhighlight_all_keys
*
* Unhighlightes all keys on the current keyboard.
**/
void gok_chunker_unhighlight_all_keys ()
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;
	
	gok_log_enter();
	pKeyboard = gok_main_get_current_keyboard ();
	if (pKeyboard != NULL)
	{
		pKey = pKeyboard->pKeyFirst;
		while (pKey != NULL)
		{
			gok_feedback_unhighlight (pKey, FALSE);
			pKey = pKey->pKeyNext;
		}
	}
	gok_log_leave();
}

/**
* gok_chunker_highlight_first_key
*
* Highlightes the first key in the selected chunk.
* Updates m_pChunkitemHighlighted.
**/
void gok_chunker_highlight_first_key ()
{
	if (m_pChunkHighlighted == NULL) {
		gok_log_x( "aborting - no highlighted chunk to scan");
		return;
	}
	
	m_pChunkitemHighlighted = m_pChunkHighlighted->pChunkitem;
	g_assert (m_pChunkitemHighlighted != NULL);
	g_assert (m_pChunkitemHighlighted->pKey != NULL);

	gok_chunker_unhighlight_chunk (m_pChunkHighlighted);

	gok_feedback_highlight (m_pChunkitemHighlighted->pKey, FALSE);

	/* this is now the selected key */
	gok_feedback_set_selected_key (m_pChunkitemHighlighted->pKey);
}

/**
* gok_chunker_highlight_chunk_with_key 
*
* Highlights the chunk with the specified key
* We'll need this helper function until we convert the navigate by key
* implementations to navigate by chunks... e.g. keyup, keydown....
* NOTE: for now we assume one key per chunk.
**/
void 
gok_chunker_highlight_chunk_with_key (GokKey* pKey) {
	GokKeyboard* pkb = NULL;
	GokChunk* pchunk = NULL;
	
	gok_log_enter();
	pkb = gok_main_get_current_keyboard();
	g_assert(pkb != NULL);
	pchunk = pkb->pChunkFirst;
	while (pchunk != NULL) {
		g_assert (pchunk->pChunkitem != NULL);
		/* for now assume one key per chunk see head comment */
		if (pchunk->pChunkitem->pKey == pKey) {
			gok_chunker_highlight_chunk(pchunk);
			break;
		}
		pchunk = pchunk->pChunkNext;
	}
	gok_log_leave();
}

/**
* gok_chunker_highlight_center_key
*
* Highlightes the center key on the current keyboard.
**/
void gok_chunker_highlight_center_key ()
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;
	gint x;
	gint centerRow;
	gint centerRowAlternate;
	gint centerColumn;
	gint centerColumnAlternate;
	GokKey* pKeyCenter;
	GokKey* pKeyCenter1;
	GokKey* pKeyCenter2;
	GokKey* pKeyCenter3;
	GokKey* pKeyCenter4;
	gint rowsAway1, columnsAway1;
	gint rowsAway2, columnsAway2;
	gint rowsAway3, columnsAway3;
	gint rowsAway4, columnsAway4;
	gint totalAway;

	pKeyCenter = NULL;

	/* get the current keyboard */
	pKeyboard = gok_main_get_current_keyboard();
	if (pKeyboard == NULL)
	{
		/* keyboard may be NULL if this is called before keyboards are read */
		return;
	}

	/* make sure the keyboard has at least one key */
	if (pKeyboard->pKeyFirst == NULL)
	{
		return;
	}

	/* remove the highlighting from any keys that are currently highlighted */
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		gok_feedback_unhighlight (pKey, FALSE);
		pKey = pKey->pKeyNext;
	}

	/* find the center key */
	/* first, find the keyboard cell that is at the center */
	x = gok_keyboard_get_number_rows (pKeyboard);
	centerRow =  x / 2 + (x - ((x / 2) * 2));
	/* there may be an even number of rows or columns so store the 'alternate' row
		or column that is at the center */
	if (((x/2) * 2) == x)
	{
		centerRowAlternate = centerRow + 1;
	}
	else
	{
		centerRowAlternate = -1; /* no alternate */
	}

	x = gok_keyboard_get_number_columns (pKeyboard);
	centerColumn =  x / 2 + (x - ((x / 2) * 2));
	if (((x/2) * 2) == x)
	{
		centerColumnAlternate = centerColumn + 1;
	}
	else
	{
		centerColumnAlternate = -1; /* no alternate */
	}

	pKeyCenter1 = NULL;
	pKeyCenter2 = NULL;
	pKeyCenter3 = NULL;
	pKeyCenter4 = NULL;

	/* get the key closest to the center */
	pKeyCenter = gok_chunker_find_center (pKeyboard, centerRow, centerColumn, &rowsAway1, &columnsAway1);
	totalAway = rowsAway1 + columnsAway1;

	/* save the cell at the center of the keyboard */
	m_CellHighlightedRow = centerRow;
	m_CellHighlightedColumn = centerColumn;

	/* is the key directly at the center? */
	if ((rowsAway1 != 0) ||
		(columnsAway1 != 0))
	{
		/* no, is there an alternate center row? */
		if (centerRowAlternate != -1)
		{
			/* try to find the key closest to the center using the alternate row */
			pKeyCenter2 = gok_chunker_find_center (pKeyboard, centerRowAlternate, centerColumn, &rowsAway2, &columnsAway2);
		}

		/* is the key directly at the center? */
		if ((rowsAway2 == 0) &&
			(columnsAway2 == 0))
		{
			/* yes, this key is at the center */
			pKeyCenter = pKeyCenter2;
		
			/* save the cell at the center of the keyboard */
			m_CellHighlightedRow = centerRowAlternate;
			m_CellHighlightedColumn = centerColumn;
		}
		else
		{
			/* no, but is this key closer to the center then the first one? */
			if ((rowsAway2 + columnsAway2) < (rowsAway1 + columnsAway1))
			{
				/* yes, this one is closer so make it the center key */
				pKeyCenter = pKeyCenter2;
				totalAway = rowsAway2 + columnsAway2;
			}

			/* is there an alternate center column? */
			if (centerColumnAlternate != -1)
			{
				/* try to find the key closest to the center using the alternate row */
				pKeyCenter3 = gok_chunker_find_center (pKeyboard, centerRow, centerColumnAlternate, &rowsAway3, &columnsAway3);
			}

			/* is the key directly at the center? */
			if ((rowsAway3 == 0) &&
				(columnsAway3 == 0))
			{
				/* yes, this key is at the center */
				pKeyCenter = pKeyCenter3;

				/* save the cell at the center of the keyboard */
				m_CellHighlightedRow = centerRow;
				m_CellHighlightedColumn = centerColumnAlternate;
			}
			else
			{
				/* no, but is this key closer to the center then the first one? */
				if ((rowsAway3 + columnsAway3) < totalAway)
				{
					/* yes, this one is closer so make it the center key */
					pKeyCenter = pKeyCenter3;
					totalAway = rowsAway3 + columnsAway3;
				}

				/* is there both an alternate row and an alternate column? */
				if ((centerColumnAlternate != -1) &&
					(centerRowAlternate != -1))
				{
					/* try to find the key closest to the center using the alternate row */
					pKeyCenter4 = gok_chunker_find_center (pKeyboard, centerRowAlternate, centerColumnAlternate, &rowsAway4, &columnsAway4);

					/* is the key directly at the center? */
					if ((rowsAway4 == 0) &&
						(columnsAway4 == 0))
					{
						/* yes, this key is at the center */
						pKeyCenter = pKeyCenter4;

						/* save the cell at the center of the keyboard */
						m_CellHighlightedRow = centerRowAlternate;
						m_CellHighlightedColumn = centerColumnAlternate;
					}
					else
					{
						if ((rowsAway4 + columnsAway4) < totalAway)
						{
							/* yes, this one is closer so make it the center key */
							pKeyCenter = pKeyCenter4;
						}

						/* the key selected is not at the center of the keyboard */
						/* store the center cell of the key */
						if (pKeyCenter->Top >= centerRow)
						{
							m_CellHighlightedRow = pKeyCenter->Top + 1;
						}
						else
						{
							m_CellHighlightedRow = pKeyCenter->Bottom;
						}
						if (pKeyCenter->Right >= centerColumn)
						{
							m_CellHighlightedColumn = pKeyCenter->Left + 1;
						}
						else
						{
							m_CellHighlightedColumn = pKeyCenter->Right;
						}
					}
				}
			}
		}
	}

	/* highlight and select the center key */
	if (pKeyCenter != NULL)
	{
		gok_chunker_highlight_chunk_with_key (pKeyCenter);
		gok_feedback_highlight (pKeyCenter, FALSE);	
		gok_feedback_set_highlighted_key (pKeyCenter);
		gok_feedback_set_selected_key (pKeyCenter); /* bad? */
	}
}

/**
* gok_chunker_find_center
* @pKeyboard: Keyboard that you want to find the center of.
* @CenterRow: Center row of the keyboard.
* @CenterColumn: Center column of the keyboard.
* @pRowsDistant: (out) Key returned is this number of rows away from the center.
* @pColumnsDistant: (out) Key returned is this number of columns away from the center.
*
* Check pRowsDistant and pColumnsDistant to find out how close this key is to the 
* center. If both pRowsDistant and pColumnsDistant are 0 then the key returned is 
* directly at the center.
*
* Returns: Pointer to the key that is closest to the center of the keyboard.
**/
GokKey* gok_chunker_find_center (GokKeyboard* pKeyboard, gint CenterRow, gint CenterColumn, gint* pRowsDistant, gint* pColumnsDistant)
{
	GokKey* pKeyCenter;
	GokKey* pKey;
	gint x;
	gint closestKey;
	gint rowsAway, columnsAway;

	pKeyCenter = NULL;
	closestKey = 1000;

	/* check all the keys */
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		if (pKey->Right == CenterColumn)
		{
			rowsAway = 0;
		}
		else 
		{
			if (pKey->Right > CenterColumn)
			{
				if (pKey->Left < CenterColumn)
				{
					rowsAway = 0;
				}
				else
				{
					rowsAway = (pKey->Left - CenterColumn) + 1;
				}
			}
			else
			{
				rowsAway = CenterColumn - pKey->Right;
			}
		}

		if (pKey->Bottom == CenterRow)
		{
			columnsAway = 0;
		}
		else 
		{
			if (pKey->Bottom > CenterRow)
			{
				if (pKey->Top < CenterRow)
				{
					columnsAway = 0;
				}
				else
				{
					columnsAway = (pKey->Top - CenterRow) + 1;
				}
			}
			else
			{
				columnsAway = CenterRow - pKey->Bottom;
			}
		}

		x = columnsAway + rowsAway;
		if (x < closestKey)
		{
			closestKey = x;
			pKeyCenter = pKey;
			*pRowsDistant = rowsAway;
			*pColumnsDistant = columnsAway;
		}

		pKey = pKey->pKeyNext;
	}

	return pKeyCenter;
}

/**
* gok_chunker_keyhighlight
*
* Highlights the given key.
*
* NOT USED  (deprecate?)
**/
void gok_chunker_keyhighlight()
{
	gint MouseX, MouseY;
	GokKeyboard* pKeyboard;
	GokKey* pKey;
	GokKey* pKeySelected;
	GokKey* pKeyHighlighted;

	pKeyHighlighted = gok_feedback_get_highlighted_key();

	/* get location of the mouse pointer */
	gok_scanner_get_pointer_location (&MouseX, &MouseY);

	/* find the topmost key that is highlighted */
	pKeyboard = gok_main_get_current_keyboard();
	g_assert (pKeyboard != NULL);

	pKeySelected = NULL;
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		if ((pKey->TopWin <= MouseY) &&
				(pKey->BottomWin >= MouseY) &&
				(pKey->LeftWin <= MouseX) &&
				(pKey->RightWin >= MouseX))
		{
			pKeySelected = pKey;
		}
		pKey = pKey->pKeyNext;
	}

	/* update m_pKeyHighlighted with the key that is highlighted */
	/* Note: m_pKeyHighlighted may be NULL */
	if (pKeySelected != NULL)
	{
		if (pKeySelected != pKeyHighlighted)
		{
			if (pKeyHighlighted != NULL)
			{
				gok_feedback_unhighlight (pKeyHighlighted, FALSE);
			}
			gok_feedback_highlight (pKeySelected, FALSE);
		}
	}
	else if (pKeyHighlighted != NULL)
	{
		gok_feedback_unhighlight (pKeyHighlighted, FALSE);
	}

	gok_feedback_set_highlighted_key (pKeySelected);
}

/**
* gok_chunker_wraptofirstchunk
*
* Highlights the first chunk in the list of chunks 
*
* returns: TRUE if successful, FALSE if not.
**/
gboolean gok_chunker_wraptofirstchunk ()
{
	GokChunk* pChunkFirst;

	if (m_pChunkHighlighted == NULL)
	{
		gok_log_x ("Warning: m_pChunkHighlighted is NULL!\n");
		return FALSE;
	}

	/* if this is the first chunk then stay here */
	if (m_pChunkHighlighted->pChunkPrevious == NULL)
	{
		return TRUE;
	}
	
	pChunkFirst = m_pChunkHighlighted->pChunkPrevious;
	gok_chunker_unhighlight_chunk (m_pChunkHighlighted);
	
	while (pChunkFirst->pChunkPrevious != NULL)
	{
		pChunkFirst = pChunkFirst->pChunkPrevious;
	}
	
	gok_chunker_highlight_chunk (pChunkFirst);
	return TRUE;
}

/**
* gok_chunker_wraptolastchunk
*
* Highlights the last chunk in the list of chunks 
*
* returns: TRUE if successful, FALSE if not.
**/
gboolean gok_chunker_wraptolastchunk ()
{
	GokChunk* pChunkLast;
	
	if (m_pChunkHighlighted == NULL)
		{
		gok_log_x ("Warning: m_pChunkHighlighted is NULL!\n");
		return FALSE;
	}

	/* if this is the last chunk then stay here */
	if (m_pChunkHighlighted->pChunkNext == NULL)
	{
		return TRUE;
	}
	
	pChunkLast = m_pChunkHighlighted->pChunkNext;
	gok_chunker_unhighlight_chunk (m_pChunkHighlighted);

	while (pChunkLast->pChunkNext != NULL)
	{
		pChunkLast = pChunkLast->pChunkNext;
	}
	
	gok_chunker_highlight_chunk (pChunkLast);
	return TRUE;
}

/**
* gok_chunker_wraptofirstkey
*
* Highlights the first key in the list of keys.
*
* returns: TRUE if successful, FALSE if not.
**/
gboolean gok_chunker_wraptofirstkey ()
{
	if (m_pChunkitemHighlighted == NULL)
	{
		gok_log_x ("Warning: gok_chunker_wraptofirstkey failed because m_pChunkitemHighlighted is NULL!\n");
		return FALSE;
	}

	g_assert (m_pChunkitemHighlighted->pKey != NULL);
	gok_feedback_unhighlight (m_pChunkitemHighlighted->pKey, FALSE);

	while (m_pChunkitemHighlighted->pItemPrevious != NULL)
	{
		m_pChunkitemHighlighted = m_pChunkitemHighlighted->pItemPrevious;
	}

	g_assert (m_pChunkitemHighlighted->pKey != NULL);
	gok_feedback_highlight (m_pChunkitemHighlighted->pKey, FALSE);
	gok_feedback_set_selected_key (m_pChunkitemHighlighted->pKey);

	return TRUE;
}

/**
* gok_chunker_wraptolastkey
*
* Highlights the last key in the list of keys. 
*
* returns: TRUE if successful, FALSE if not.
**/
gboolean gok_chunker_wraptolastkey ()
{
	if (m_pChunkitemHighlighted == NULL)
	{
		gok_log_x ("Warning: gok_chunker_wraptolastkey failed because m_pChunkHighlighted is NULL!\n");
		return FALSE;
	}

	gok_feedback_unhighlight (m_pChunkitemHighlighted->pKey, FALSE);
	while (m_pChunkitemHighlighted->pItemNext != NULL)
	{
		m_pChunkitemHighlighted = m_pChunkitemHighlighted->pItemNext;
	}
	gok_feedback_highlight (m_pChunkitemHighlighted->pKey, FALSE);
	gok_feedback_set_selected_key (m_pChunkitemHighlighted->pKey);

	return TRUE;
}

/**
* gok_chunker_wraptotop
* @bTrueFalse: unused
*
* Highlights the topmost key that is on the same column as the currently selected 
* key. 
*
* returns: TRUE if successful, FALSE if not.
**/
gboolean gok_chunker_wraptotop (gint TrueFalse)
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;
	GokKey* pTopmostKey;

	/* find the topmost key */
	pKeyboard = gok_main_get_current_keyboard();
	g_assert (pKeyboard != NULL);

	pTopmostKey = NULL;
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		/* is the key on the same column as the current highlight? */
		if ((pKey->Right >= m_CellHighlightedColumn) &&
			(pKey->Left < m_CellHighlightedColumn))
		{
			/* yes, it's on the same column */

			if (pTopmostKey == NULL)
			{
				pTopmostKey = pKey;
			}
			else
			{
				if (gok_chunker_is_bottom (pTopmostKey, pKey) == TRUE)
				{
					pTopmostKey = pKey;
				}
			}
		}
		pKey = pKey->pKeyNext;
	}

	if (pTopmostKey == NULL)
	{
		gok_log_x ("Warning: Can't find topmost key in gok_chunker_wraptotop\n");
		return FALSE;
	}
	else
	{
		gok_feedback_unhighlight (gok_feedback_get_highlighted_key(), FALSE);
		gok_feedback_highlight (pTopmostKey, FALSE);
		gok_chunker_highlight_chunk_with_key (pTopmostKey);
		gok_feedback_set_selected_key (pTopmostKey);
		m_CellHighlightedRow = pTopmostKey->Top + 1;
		return TRUE;
	}
}

/**
* gok_chunker_wraptobottom
* @bTrueFalse: unused
*
* Highlights the bottommost key that is on the same column as the currently selected key.
*
* returns: TRUE if successful, FALSE if not.
**/
gboolean gok_chunker_wraptobottom (gint TrueFalse)
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;
	GokKey* pBottommostKey;

	pKeyboard = gok_main_get_current_keyboard();
	g_assert (pKeyboard != NULL);

	/* find the bottom key */
	pBottommostKey = NULL;
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		/* is the key on the same column as the current highlight? */
		if ((pKey->Right >= m_CellHighlightedColumn) &&
			(pKey->Left < m_CellHighlightedColumn))
		{
			/* yes, it's on the same column */

			if (pBottommostKey == NULL)
			{
				pBottommostKey = pKey;
			}
			else
			{
				if (gok_chunker_is_top (pBottommostKey, pKey) == TRUE)
				{
					pBottommostKey = pKey;
				}
			}
		}
		pKey = pKey->pKeyNext;
	}

	if (pBottommostKey == NULL)
	{
		gok_log_x ("Warning: Can't find bottommost key in gok_chunker_wraptobottom!\n");
		return FALSE;
	}
	else
	{
		gok_feedback_unhighlight (gok_feedback_get_highlighted_key(), FALSE);
		gok_feedback_highlight (pBottommostKey, FALSE);	
		gok_chunker_highlight_chunk_with_key (pBottommostKey);
		gok_feedback_set_selected_key (pBottommostKey);
		m_CellHighlightedRow = pBottommostKey->Bottom;
		return TRUE;
	}
}

/**
* gok_chunker_wraptoleft
* @TrueFalse: unused.
*
* Highlights the leftmost key that is on the same row as the currently selected key.
*
* returns: TRUE if successful, FALSE if not.
**/
gboolean gok_chunker_wraptoleft (gint TrueFalse)
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;
	GokKey* pLeftmostKey;

	pKeyboard = gok_main_get_current_keyboard();
	g_assert (pKeyboard != NULL);

	/* find the leftmost key */
	pLeftmostKey = NULL;
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		/* is the key on the same row as the current highlight? */
		if ((pKey->Bottom >= m_CellHighlightedRow) &&
			(pKey->Top < m_CellHighlightedRow))
		{
			/* yes, it's on the same row */

			if (pLeftmostKey == NULL)
			{
				pLeftmostKey = pKey;
			}
			else
			{
				if (gok_chunker_is_right (pLeftmostKey, pKey) == TRUE)
				{
					pLeftmostKey = pKey;
				}
			}
		}
		pKey = pKey->pKeyNext;
	}

	if (pLeftmostKey == NULL)
	{
		gok_log_x ("Warning: Can't find leftmost key in gok_chunker_wraptoleft!\n");
		return FALSE;
	}
	else
	{
		gok_feedback_unhighlight (gok_feedback_get_highlighted_key(), FALSE);
		gok_feedback_highlight (pLeftmostKey, FALSE);	
		gok_chunker_highlight_chunk_with_key (pLeftmostKey);
		gok_feedback_set_selected_key (pLeftmostKey);
		m_CellHighlightedColumn = pLeftmostKey->Left + 1;
		return TRUE;
	}
}

/**
* gok_chunker_wraptoright
* @TrueFalse: unused
*
* Highlights the rightmost key that is on the same row as the currently selected key.
*
* returns: TRUE if successful, FALSE if not.
**/
int gok_chunker_wraptoright (gint TrueFalse)
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;
	GokKey* pRightmostKey;

	pKeyboard = gok_main_get_current_keyboard();
	g_assert (pKeyboard != NULL);

	/* find the right key */
	pRightmostKey = NULL;
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		/* is the key on the same row as the current highlight? */
		if ((pKey->Bottom >= m_CellHighlightedRow) &&
			(pKey->Top < m_CellHighlightedRow))
		{
			/* yes, it's on the same row */

			if (pRightmostKey == NULL)
			{
				pRightmostKey = pKey;
			}
			else
			{
				if (gok_chunker_is_left (pRightmostKey, pKey) == TRUE)
				{
					pRightmostKey = pKey;
				}
			}
		}
		pKey = pKey->pKeyNext;
	}

	if (pRightmostKey == NULL)
	{
		gok_log_x ("Warning: Can't find rightmost key in gok_chunker_wraptoright!\n");
		return FALSE;
	}
	else
	{
		gok_feedback_unhighlight (gok_feedback_get_highlighted_key(), FALSE);
		gok_feedback_highlight (pRightmostKey, FALSE);	
		gok_chunker_highlight_chunk_with_key (pRightmostKey);
		gok_feedback_set_selected_key (pRightmostKey);
		m_CellHighlightedColumn = pRightmostKey->Right;
		return TRUE;
	}
}

/**
* gok_chunker_is_left
* @pKey1: Key you want to test.
* @pKey2: Key you want to compare against.
*
* Tests if a key is left of another key. This does NOT test if the keys are on the same row.
* This should be called when traversing a row from right to left.
*
* Returns: TRUE if pKey1 is left of pKey2, FALSE if pKey2 is left of pKey1.
**/
gboolean gok_chunker_is_left (GokKey* pKey1, GokKey* pKey2)
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;

	if (pKey1->Right < pKey2->Right)
	{
		return TRUE;
	}

	if (pKey1->Right > pKey2->Right)
	{
		return FALSE;
	}

	/* right sides are the same */
	/* shorter keys are more left */
	if (pKey1->Left < pKey2->Left)
	{
		return TRUE;
	}

	/* longer keys are more right */
	if (pKey1->Left > pKey2->Left)
	{
		return FALSE;
	}

	/* both sides are the same */
	/* key that is underneath is more left */
	pKeyboard = gok_main_get_current_keyboard();
	g_assert (pKeyboard != NULL);

	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		if (pKey == pKey1)
		{
			return TRUE;
		}
		if (pKey == pKey2)
		{
			return FALSE;
		}
		pKey = pKey->pKeyNext;
	}

	gok_log_x ("Warning: Can't find keys in gok_chunker_is_left!\n");
	return TRUE;
}

/**
* gok_chunker_is_right
* @pKey1: Key you want to test.
* @pKey2: Key you want to compare against.
*
* Tests if a key is right of another key. This does NOT test if the keys are on the same row.
* This should be called when traversing a row from left to right.
*
* Returns: TRUE if pKey1 is right of pKey2, FALSE if pKey2 is left of pKey1.
**/
gboolean gok_chunker_is_right (GokKey* pKey1, GokKey* pKey2)
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;

	g_assert(pKey1);
	g_assert(pKey2);
	
	if (pKey1->Left > pKey2->Left)
	{
		return TRUE;
	}

	if (pKey1->Left < pKey2->Left)
	{
		return FALSE;
	}

	/* left sides are the same */
	/* shorter keys are more right */
	if (pKey1->Right > pKey2->Right)
	{
		return TRUE;
	}

	/* longer keys are more left */
	if (pKey1->Right < pKey2->Right)
	{
		return FALSE;
	}

	/* both sides are the same */
	/* key that is on top is more right */
	pKeyboard = gok_main_get_current_keyboard();
	g_assert (pKeyboard != NULL);

	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		if (pKey == pKey1)
		{
			return FALSE;
		}
		if (pKey == pKey2)
		{
			return TRUE;
		}
		pKey = pKey->pKeyNext;
	}

	gok_log_x ("Warning: Can't find keys in gok_chunker_is_right!\n");
	return TRUE;
}

/**
* gok_chunker_is_top
* @pKey1: Key you want to test.
* @pKey2: Key you want to compare against.
*
* Tests if a key is top of another key. This does NOT test if the keys are on the same column.
* This should be called when traversing a column from bottom to top.
*
* Returns: TRUE if pKey1 is higher than pKey2, FALSE if pKey2 is higher than pKey1.
**/
gboolean gok_chunker_is_top (GokKey* pKey1, GokKey* pKey2)
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;

	if (pKey1->Bottom < pKey2->Bottom)
	{
		return TRUE;
	}

	if (pKey1->Bottom > pKey2->Bottom)
	{
		return FALSE;
	}

	/* bottom sides are the same */
	/* shorter keys are more top */
	if (pKey1->Top < pKey2->Top)
	{
		return TRUE;
	}

	/* longer keys are more bottom */
	if (pKey1->Top > pKey2->Top)
	{
		return FALSE;
	}

	/* both sides are the same */
	/* key that is underneath is more top */
	pKeyboard = gok_main_get_current_keyboard();
	g_assert (pKeyboard != NULL);

	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		if (pKey == pKey1)
		{
			return TRUE;
		}
		if (pKey == pKey2)
		{
			return FALSE;
		}
		pKey = pKey->pKeyNext;
	}

	gok_log_x ("Warning: Can't find keys in gok_chunker_is_top!\n");
	return TRUE;
}

/**
* gok_chunker_is_bottom
* @pKey1: Key you want to test.
* @pKey2: Key you want to compare against.
*
* Tests if a key is bottom of another key. This does NOT test if the keys are on the same column.
* This should be called when traversing a column from top to bottom.
*
* Returns: TRUE if pKey1 is bottom of pKey2, FALSE if pKey2 is bottom of pKey1.
**/
gboolean gok_chunker_is_bottom (GokKey* pKey1, GokKey* pKey2)
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;

	if (pKey1->Top > pKey2->Top)
	{
		return TRUE;
	}

	if (pKey1->Top < pKey2->Top)
	{
		return FALSE;
	}

	/* top sides are the same */
	/* shorter keys are more bottom */
	if (pKey1->Bottom > pKey2->Bottom)
	{
		return TRUE;
	}

	/* longer keys are more top */
	if (pKey1->Bottom < pKey2->Bottom)
	{
		return FALSE;
	}

	/* both sides are the same */
	/* key that is stacked on top is more bottom */
	pKeyboard = gok_main_get_current_keyboard();
	g_assert (pKeyboard != NULL);

	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		if (pKey == pKey1)
		{
			return FALSE;
		}
		if (pKey == pKey2)
		{
			return TRUE;
		}
		pKey = pKey->pKeyNext;
	}

	gok_log_x ("Warning: Can't find keys in gok_chunker_is_bottom!\n");
	return TRUE;
}

/**
* gok_chunker_move_leftright
* @TrueFalse: unused
*
* Not implemented yet.
**/
void gok_chunker_move_leftright (gint TrueFalse)
{
	gok_log_x ("gok_chunker_move_ight not implemented yet!\n");
}

/**
* gok_chunker_move_topbottom
* @TrueFalse: unused
*
* Not implemented yet.
**/
void gok_chunker_move_topbottom (gint TrueFalse)
{
	gok_log_x ("gok_chunker_move_topbottom not implemented yet!\n");
}

/**
* gok_chunker_if_next_chunk
*
* Tests if there is another chunk after the current chunk.
*
* returns: TRUE if there is a next chunk, FALSE if not.
**/
gboolean gok_chunker_if_next_chunk ()
{
	g_assert (m_pChunkHighlighted != NULL);
	return (m_pChunkHighlighted->pChunkNext != NULL) ? TRUE : FALSE;
}

/**
* gok_chunker_if_previous_chunk
*
* Tests if there is another chunk before the current chunk.
*
* returns: TRUE if there is a previous chunk, FALSE if not.
**/
gboolean gok_chunker_if_previous_chunk ()
{
	g_assert (m_pChunkHighlighted != NULL);
	return (m_pChunkHighlighted->pChunkPrevious != NULL) ? TRUE : FALSE;
}

/**
* gok_chunker_if_next_key
*
* Tests if there is another key after the current key.
*
* returns: TRUE if there is a next key, FALSE if not.
**/
gboolean gok_chunker_if_next_key ()
{
	g_assert (m_pChunkitemHighlighted != NULL);
	return (m_pChunkitemHighlighted->pItemNext != NULL) ? TRUE : FALSE;
}

/**
* gok_chunker_if_previous_key
*
* Tests if there is another key before the current key.
*
* returns: TRUE if there is a key chunk, FALSE if not.
**/
gboolean gok_chunker_if_previous_key ()
{
	g_assert (m_pChunkitemHighlighted != NULL);
	return (m_pChunkitemHighlighted->pItemPrevious != NULL) ? TRUE : FALSE;
}

/**
* gok_chunker_if_top
*
* Tests if there is a key to the top of the currently highlighted key.
*
* returns: TRUE if there is a key to the top, FALSE if not.
**/
gboolean gok_chunker_if_top ()
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;
	GokKey* pKeySelected;

	pKeySelected = gok_feedback_get_selected_key();

	/* looking for the top key */
	/* if no key highlighted then can't find a key to the top */
	if (pKeySelected == NULL)
	{
		gok_log_x ("Warning: No key highlighted in gok_chunker_if_top!\n");
		return FALSE;
	}

	pKeyboard = gok_main_get_current_keyboard();
	g_assert (pKeyboard != NULL);

	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		if (pKey != pKeySelected)
		{
			/* is the key on the same column as the current highlight? */
			if ((pKey->Right >= m_CellHighlightedColumn) &&
				(pKey->Left < m_CellHighlightedColumn))
			{
				/* yes, it's on the same row */
				if (gok_chunker_is_top (pKey, pKeySelected) == TRUE)
				{
					return TRUE;
				}
			}
		}
		pKey = pKey->pKeyNext;
	}
	return FALSE;
}

/**
* gok_chunker_if_bottom
*
* Tests if there is a key to the bottom of the currently highlighted key.
*
* returns: TRUE if there is a key to the bottom, FALSE if not.
**/
gboolean gok_chunker_if_bottom ()
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;
	GokKey* pKeySelected;

	pKeySelected = gok_feedback_get_selected_key();

	/* looking for the bottom key */
	/* if no key highlighted then can't find a key to the bottom */
	if (pKeySelected == NULL)
	{
		gok_log_x ("Warning: No key highlighted in gok_chunker_if_bottom!\n");
		return FALSE;
	}

	pKeyboard = gok_main_get_current_keyboard();
	g_assert (pKeyboard != NULL);

	/* find the key that is in the next row to the bottom */
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		if (pKey != pKeySelected)
		{
			/* is the key on the same column as the current highlight? */
			if ((pKey->Right >= m_CellHighlightedColumn) &&
				(pKey->Left < m_CellHighlightedColumn))
			{
				/* yes, it's on the same column */
				if (gok_chunker_is_bottom (pKey, pKeySelected) == TRUE)
				{
					return TRUE;
				}
			}
		}
		pKey = pKey->pKeyNext;
	}
	return FALSE;
}

/**
* gok_chunker_if_left
*
* Tests if there is a key to the left of the currently highlighted key.
*
* returns: TRUE if there is a key to the left, FALSE if not.
**/
gboolean gok_chunker_if_left ()
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;
	GokKey* pKeySelected;
	
	pKeySelected = gok_feedback_get_selected_key();
	
	/* if no key highlighted then can't find a key to the left */
	if (pKeySelected == NULL)
	{
		gok_log_x ("Warning: No key highlighted in gok_chunker_if_left!\n");
		return FALSE;
	}

	pKeyboard = gok_main_get_current_keyboard();
	g_assert (pKeyboard != NULL);

	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		if (pKey != pKeySelected)
		{
			/* is the key on the same row as the current highlight? */
			if ((pKey->Bottom >= m_CellHighlightedRow) &&
				(pKey->Top < m_CellHighlightedRow))
			{
				/* yes, it's on the same row */
				if (gok_chunker_is_left (pKey, pKeySelected) == TRUE)
				{
					return TRUE;
				}
			}
		}
		pKey = pKey->pKeyNext;
	}
	return FALSE;
}

/**
* gok_chunker_if_right
*
* Tests if there is a key to the right of the currently highlighted key.
*
* returns: TRUE if there is a key to the right, FALSE if not.
**/
gboolean gok_chunker_if_right ()
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;
	GokKey* pKeySelected;
	gboolean bFoundHighlightKey;

	/* TODO: sort out selected vs highlighted distinction? */
	/* pKeySelected = gok_feedback_get_selected_key(); */
	pKeySelected = gok_feedback_get_highlighted_key();
	
	/* if no key highlighted then can't find a key to the right */
	if (pKeySelected == NULL)
	{
		gok_log_x ("Warning: No key highlighted in gok_chunker_if_right!\n");
		return FALSE;
	}

	pKeyboard = gok_main_get_current_keyboard();
	g_assert (pKeyboard != NULL);

	/* find the key that is in the next column to the right */
	bFoundHighlightKey = FALSE;
	pKey = pKeyboard->pKeyFirst;
	while (pKey != NULL)
	{
		if (pKey == pKeySelected)
		{
			bFoundHighlightKey = TRUE;
		}
		else
		{
			/* is the key on the same row as the current highlight? */
			if ((pKey->Bottom >= m_CellHighlightedRow) &&
				(pKey->Top < m_CellHighlightedRow))
			{
				/* yes, it's on the same row */
				if (gok_chunker_is_right (pKey, pKeySelected) == TRUE)
				{
					return TRUE;
				}
			}
		}
		pKey = pKey->pKeyNext;
	}
	return FALSE;
}

/**
* gok_chunker_if_key_selected
*
* Returns: Alwasy 0.
**/
int gok_chunker_if_key_selected ()
{
	return 0;
}

/**
* gok_chunker_counter_set
* @CounterId: Counter you want to set.
* @CounterValue: Value you want the counter set to.
*
**/
void gok_chunker_counter_set (gint CounterId, gint CounterValue)
{
	if ((CounterId < 0) ||
	(CounterId >= MAX_COUNTERS))
	{
		gok_log_x ("counter value %d out of range!", CounterId);
		return;
	}
	
	m_Counters[CounterId] = CounterValue;
}

/**
* gok_chunker_counter_increment
* @CounterId: Counter you want to increment.
*
**/
void gok_chunker_counter_increment (gint CounterId)
{
	if ((CounterId < 0) ||
	(CounterId >= MAX_COUNTERS))
	{
		gok_log_x ("counter value %d out of range!", CounterId);
		return;
	}
	
	m_Counters[CounterId]++;
}

/**
* gok_chunker_counter_decrement
* @CounterId: Counter you want to decrement.
*
**/
void gok_chunker_counter_decrement (gint CounterId)
{
	if ((CounterId < 0) ||
	(CounterId >= MAX_COUNTERS))
	{
		gok_log_x ("counter value %d out of range!", CounterId);
		return;
	}
	
	m_Counters[CounterId]--;
}

/**
* gok_chunker_counter_get
* @CounterId: Counter you want to get the value of.
*
* Returns the value of the specified counter.
**/
gint gok_chunker_counter_get (gint CounterId)
{
	if ((CounterId < 0) ||
	(CounterId >= MAX_COUNTERS))
	{
		gok_log_x ("counter value %d out of range!", CounterId);
		return 0;
	}
	return m_Counters[CounterId];
}


/**
* gok_chunker_state_restart
*
* Move to the first state.
**/
void gok_chunker_state_restart ()
{
	GokAccessMethod* pAccessMethod;

	gok_repeat_stop();
	gok_repeat_disarm();
	pAccessMethod = gok_scanner_get_current_access_method();
	g_assert (pAccessMethod != NULL);
	gok_scanner_change_state (pAccessMethod->pStateFirst, pAccessMethod->Name);
}

/**
* gok_chunker_state_next:
*
* Move to the next state. Move to the first state if at the last state.
**/
void gok_chunker_state_next ()
{
	GokScannerState* pState;
	GokAccessMethod* pAccessMethod;

	pAccessMethod = gok_scanner_get_current_access_method();
	pState = gok_scanner_get_current_state();
	g_assert (pState != NULL);
	
	/* is there a next state? */
	if (pState->pStateNext != NULL)
	{
		/* yes, move to the next state */
		gok_scanner_change_state (pState->pStateNext, pAccessMethod->Name);
		return;
	}

	/* no next state so move to the first state */
	g_assert (pAccessMethod != NULL);
	gok_scanner_change_state (pAccessMethod->pStateFirst, pAccessMethod->Name);
}

/**
* gok_chunker_state_jump:
* @NameState: Name of the state you want to jump to.
*
* Change state to the given state.
**/
void gok_chunker_state_jump (gchar* NameState)
{
	GokScannerState* pState;
	GokAccessMethod* pAccessMethod;
	
	g_assert (NameState != NULL);

	/* find the state with the given name */
	pAccessMethod = gok_scanner_get_current_access_method();
	g_assert (pAccessMethod != NULL);
	
	pState = pAccessMethod->pStateFirst;
	while (pState != NULL)
	{
		if ((pState->NameState != NULL) &&
			(strcmp (pState->NameState, NameState) == 0))
		{
			gok_scanner_change_state (pState, pAccessMethod->Name);
			return;
		}
		pState = pState->pStateNext;
	}

	gok_log_x ("Warning: Can't find state %s in gok_chunker_state_jump!\n", NameState);
}

/**
* gok_chunker_select_chunk:
*
* Sets the current chunk as the selected chunk.
*
* returns: The number of keys in the selected chunk.
**/
gboolean gok_chunker_select_chunk ()
{
	gint numKeysInChunk;
	
	if (m_pChunkHighlighted == NULL)
	{
		gok_log_x ("Warning: gok_chunker_select_chunk failed because m_pChunkHighlighted is NULL!\n");
		return 0;
	}

	g_bChunkSelected = TRUE;
	
	/* count the number of keys in the chunk */
	numKeysInChunk = gok_chunker_count_keys_in_chunk (m_pChunkHighlighted);
	
	/* if there is only one key then make it the selected key */
	if (numKeysInChunk == 1)
	{
		gok_feedback_set_selected_key (m_pChunkHighlighted->pChunkitem->pKey);
		g_bChunkSelected = FALSE; /* TODO: what if keys are chunks? */
	}
	
	return numKeysInChunk;
}

/**
* gok_chunker_count_keys_in_chunk
* @pChunk: Count the keys in this chunk.
*
* Returns: The number of keys in the chunk.
**/
gint gok_chunker_count_keys_in_chunk (GokChunk* pChunk)
{
	gint count;
	GokChunkitem* pChunkitem;
	
	g_assert (pChunk != NULL);
	
	count = 0;
	pChunkitem = pChunk->pChunkitem;
	while (pChunkitem != NULL)
	{
		count++;
		pChunkitem = pChunkitem->pItemNext;
	}
	
	return count;
}

/**
* gok_chunker_highlight_chunk_number
* @Number: Id number of the chunk you want highlighted.
*
* Highlight the chunk identified by a number.
**/
void gok_chunker_highlight_chunk_number (gint Number)
{
	GokChunk* pChunk;
	
	pChunk = gok_chunker_get_chunk (Number);

	if (m_pChunkHighlighted != NULL)
	{
		gok_chunker_unhighlight_chunk (m_pChunkHighlighted);
	}
	else
	{
		gok_chunker_unhighlight_all_keys();
	}
	
	if (pChunk != NULL)
	{
		if (pChunk->pChunkChild == NULL)
		{
			gok_feedback_set_selected_key (pChunk->pChunkitem->pKey);
			gok_keyboard_output_selectedkey ();
			gok_chunker_highlight_all_keys();
		}
		else
		{
			gok_chunker_highlight_chunk (pChunk);
		}
	}
}

/**
* gok_chunker_get_chunk
* @Number: Number of the chunk you are looking for.
*
* Finds a child chunk of the currently highlighted chunk.
*
* returns: A pointer to the chunk, NULL if not found.
**/
GokChunk* gok_chunker_get_chunk (gint Number)
{
	GokKeyboard* pKeyboard;
	GokChunk* pChunk;

	/* is there currently a chunk highlighted/ */
	if (m_pChunkHighlighted == NULL)
	{
		/* no, get the first chunk in the list of chunks */
		pKeyboard = gok_main_get_current_keyboard();
		pChunk = pKeyboard->pChunkFirst;
	}
	else
	{
		/* yes, find the key in the child chunk of the chunk highlighed */
		pChunk = m_pChunkHighlighted->pChunkChild;
	}
	
	while (pChunk != NULL)
	{
		if (pChunk->ChunkId == Number)
		{
			return pChunk;
		}
		pChunk = pChunk->pChunkNext;
	}
	gok_log_x ("Can't find chunk number '%d' in any chunk!", Number);

	return NULL;
}
