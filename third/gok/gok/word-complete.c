/* word-complete.c
*
* Copyright 2001,2002 Sun Microsystems, Inc.,
* Copyright 2001,2002 University Of Toronto
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
*
* To use this thing:
* - Call "WordCompleteOpen". If it returns TRUE then you're ready to go.
* - Call "WordCompletePredictString" to make the word predictions.
* - Call "WordCompleteClose" when you're done. 
* - To add a word, call "WordCompleteAddNewWord".
*
*/

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <gnome.h>
#include "word-complete.h"
#include "gok-log.h"

/* private functions */
static gchar    WordCompleteSaveDictionary     (void);
static gint     WordCompleteSaveLoop           (Node* pDictLetter, gchar* pWord, FILE* pFile);
static void     WordCompletePredictLoop        (const gchar* pWord, Node* pNode, gchar* pPrediction);
static void     WordCompleteDeleteChildren     (Node* pNode);
static gboolean WordCompleteAddWord            (GokWordComplete *complete, 
						const gchar* pWord, gint priority, gint state);
static Node*    WordCompleteAddLetter          (Node* pNode, gchar letter);
static void     WordCompleteAddPrediction      (gchar* pWord, gint priority);

/* implementations of GokWordCompleteClass virtual methods */
static gboolean WordCompleteAddNewWord         (GokWordComplete *complete, const gchar* pWord);
static gboolean WordCompleteIncrementFrequency (GokWordComplete *complete, const gchar *pWord);
static gboolean WordCompleteOpen               (GokWordComplete *complete, const gchar *dictionary);
static void     WordCompleteClose              (GokWordComplete *complete);
static gchar**  WordCompletePredictString      (GokWordComplete *complete, const gchar* pWord, gint numberPredictions);

/* a prediction */
struct PredictNode
{
	gchar word[MAXWORDLENGTH];
	gint priority;
};

typedef struct PredictNode Prediction;

typedef enum 
{
	WORD_CASE_LOWER,
	WORD_CASE_INITIAL_CAPS,
	WORD_CASE_ALL_CAPS,
	WORD_CASE_TITLE
} WordPredictionCaseType;

/* this will hold the string of predictions that is returned to the caller */
static gchar m_buffer [MAXWORDLENGTH * MAXPREDICTIONS + MAXPREDICTIONS + 1];

/* will be TRUE if "WordCompleteOpen" has been called */
static gint bWordCompleteOpened = FALSE; 

/* will be TRUE if trie has been modified after loading */
static gint bWordCompleteModified = FALSE; 

/* root node */
static Node m_FirstNode;

/* array holding all predictions found */
static Prediction m_Predictions[MAXPREDICTIONS];


/* The filename of the dictionary file */
static gchar *dictionary_full_path;

/* 
 * This macro initializes GokTrieWordComplete with the GType system 
 *   and defines ..._class_init and ..._instance_init functions 
 */
GNOME_CLASS_BOILERPLATE (GokTrieWordComplete, gok_trie_wordcomplete,
			 GokWordComplete, GOK_TYPE_WORDCOMPLETE)

static void
gok_trie_wordcomplete_instance_init (GokTrieWordComplete *complete)
{
}

static void
gok_trie_wordcomplete_class_init (GokTrieWordCompleteClass *klass)
{
	GokWordCompleteClass *word_complete_class = (GokWordCompleteClass *) klass;
	word_complete_class->open = WordCompleteOpen;
	word_complete_class->close = WordCompleteClose;
	word_complete_class->increment_word_frequency = WordCompleteIncrementFrequency;
	word_complete_class->add_new_word = WordCompleteAddNewWord;
	word_complete_class->predict_string = WordCompletePredictString;
}

/*
 * WordCompleteOpen
 *
 * Call this before using the word completor.
 * Returns TRUE if the word completor opened OK, FALSE if not.
 * Do not use the word completor if it did not open OK.
 * 
 * Reads in the dictionary file from disk. The file is straight text with the
 * fields deliminated by tabs. The fields are:
 * field 1 - the word
 * field 2 - the word's priority
 * field 3 - the word's state
 * Example: "the 500 2"
 */
static gboolean
WordCompleteOpen(GokWordComplete *complete, const gchar *directory)
{
	FILE *pFile;
	gchar buffer[100];/* will hold each line of the file */
	gchar* pWord;
	gchar* pPriority;
	gchar* pState;
	gint lengthBuffer;
	gint x;

	gok_log ("Word Complete");
	if (bWordCompleteOpened == TRUE)
	{
		/* calling "Open" after it's already been opened! */
		gok_log_x ("Calling 'Open' after WordComplete is already open!\n");
		gok_wordcomplete_reset (complete); /* reset, anyhow */
		return TRUE;
	}

	/* dictionary not modified (adding a new word causes this to be TRUE) */
	bWordCompleteModified = FALSE;

	/* initialize the first letter in the dictionary to NULL */
	m_FirstNode.letter = 0;
	m_FirstNode.priority = 0;
	m_FirstNode.state = 0;
	m_FirstNode.pNextNode = NULL;
	m_FirstNode.pFirstChildNode = NULL;

	/* open the dictionary file */
	dictionary_full_path = g_build_filename (directory, "dictionary.txt", NULL);
	pFile = fopen (dictionary_full_path, "r");
	if (pFile == NULL)
	{
		/* can't open dictionary file! */
		gok_log_x ("Can't open dictionary file in WordCompleteOpen!\n");
		return FALSE;
	}

	/* read the first line of the file... */
	if (fgets (buffer, 99, pFile) == NULL)
	{
		/* can't read in first line of the file! */
		fclose (pFile);
		return FALSE;
	}

	/* check if the first line is "WPDictFile" */
	if (strncmp (buffer, "WPDictFile", 10) != 0)
	{
		/* first line is not "WPDictFile"! */
		fclose (pFile);
		return FALSE;
	}

	/* read in the dictionary file, one line at at time */
	while (fgets (buffer, 99, pFile) != NULL)
	{
		/* FIXME: not UTF-8 safe! */
		/* mark the end of the word, priority and state */
		pWord = buffer;
		pPriority = NULL;
		pState = NULL;
		lengthBuffer = (int)strlen (buffer);
		for ( x = 0; x < lengthBuffer; x++)
		{
			if ((buffer[x] == '\t') ||
				(buffer[x] == ' ') ||
				(buffer[x] == '\n'))
			{
				buffer[x] = 0;
				if (pPriority == NULL)
				{
					pPriority = &buffer[x+1];
				}
				else if (pState == NULL)
				{
					pState = &buffer[x+1];
				}
				else
				{
					break;
				}
			}
		}

		/* add it to the array of words */
		WordCompleteAddWord (complete, pWord, atoi (pPriority), atoi (pState));
	}

	fclose (pFile);
	bWordCompleteOpened = TRUE;

	return TRUE;
}

/*
 * WordCompleteAddWord
 *
 * Adds a word to the trie structure.
 * Returns TRUE if it was added, FALSE if not.
 */
static gboolean 
WordCompleteAddWord (GokWordComplete *complete, const gchar* pWord, gint priority, gint state)
{
	/* this will eventually hold the final node (last letter) of the word */
	Node* pNode;
	gint count;
	gint lengthWord;

	pNode = &m_FirstNode;

	/* FIXME: not UTF-8 safe! */
	/* add each letter of the given word */
	lengthWord = (int)strlen (pWord);
	for (count = 0; count < lengthWord; count++)
	{
		/* holds the current node */
		pNode = WordCompleteAddLetter (pNode, pWord[count]);
		if (pNode == NULL)
		  {
		    return FALSE;
		  }
	}

	/* set the priority of the node */
	/* it indicates that this node is the end of the word*/
	pNode->priority = priority;

	/* store the state of the word */
	pNode->state = state;

	return TRUE;
}

/*
 * WordCompleteAddLetter
 *
 * Adds a letter to the trie.
 * The letter is added as a child of the given node.
 * Returns a pointer to the node that contains the given letter.
 * FIXME: Not UTF-8 safe! letter should be a gunichar or 
 * utf-8 stringlet.
 */
static Node* 
WordCompleteAddLetter (Node* pNode, gchar letter)
{
	Node* pCurrentNode;
	Node* pPreviousNode;
	Node* pNewNode;

	/* does the given node have any children? */
	if (pNode->pFirstChildNode == NULL)
	{
		/* no, add the letter as the first child node */
		pNode->pFirstChildNode = (Node*)malloc (sizeof (Node));
		if (pNode->pFirstChildNode == NULL)
		  {
		    return NULL;
		  }
		pNode->pFirstChildNode->letter = letter;
		pNode->pFirstChildNode->priority = 0;
		pNode->pFirstChildNode->state = 0;
                pNode->pFirstChildNode->pFirstChildNode = NULL;
		pNode->pFirstChildNode->pNextNode = NULL;

		return pNode->pFirstChildNode;
	}

	/* given node has children, look through them all */
	pCurrentNode = pNode->pFirstChildNode;
	pPreviousNode = pCurrentNode;
	while (pCurrentNode != NULL)
	{
		/* does this child have the same letter we're adding? */
		if (pCurrentNode->letter == letter)
		{
			/* same letter, just return a pointer to this child */
			return pCurrentNode;
		}

		pPreviousNode = pCurrentNode;
		pCurrentNode = pCurrentNode->pNextNode;
	}

	/* none of the children are the same letter that we're adding */
	/* create a new node */
	pNewNode = (Node*)malloc (sizeof (Node));
	if (pNewNode == NULL)
	  {
	    return NULL;
	  }
	pNewNode->letter = letter;
	pNewNode->priority = 0;
	pNewNode->state = 0;
	pNewNode->pFirstChildNode = NULL;
	pNewNode->pNextNode = NULL;

	/* add the new node to the list of child nodes */
	pPreviousNode->pNextNode = pNewNode;

	return pNewNode;
}

/*
 * WordCompleteClose
 *
 * Call this when done using the predictor.
 * If this is not called then memory leaks will result!
 */
static void 
WordCompleteClose(GokWordComplete *complete)
{
	Node* pDictLetter;
	Node* pLastLetter;

	/* if dictionary has been modified then save it to disk */
	if (bWordCompleteModified == TRUE)
	{
		WordCompleteSaveDictionary();
	}

	g_free (dictionary_full_path);

	/* delete all the nodes */
	pDictLetter = m_FirstNode.pFirstChildNode;
	while (pDictLetter != NULL)
	{
		if (pDictLetter->pFirstChildNode != NULL)
		{
			WordCompleteDeleteChildren (pDictLetter);
		}
		pLastLetter = pDictLetter;
		pDictLetter = pDictLetter->pNextNode;
		free (pLastLetter);
	}
	m_FirstNode.pFirstChildNode = NULL;

	bWordCompleteOpened = FALSE;
}

/*
 * WordCompleteDeleteChildren
 *
 * Deletes all the children of this node.
 * This is recursive so all the children's children are deleted too.
 */
static void 
WordCompleteDeleteChildren (Node* pParentNode)
{
	Node* pTempNode;
	Node* pNode;
	pNode = pParentNode->pFirstChildNode;
	while (pNode != NULL)
	{
		if (pNode->pFirstChildNode != NULL)
		{
			WordCompleteDeleteChildren (pNode);
		}
		pTempNode = pNode;
		pNode = pNode->pNextNode;
		free (pTempNode);
	}
	pParentNode->pFirstChildNode = NULL;
}

/**
 * DetermineCase:
 **/
static WordPredictionCaseType
DetermineCase (const gchar *pWord)
{
	gunichar c;
	const gchar *p = pWord;
	if (g_utf8_validate (pWord, -1, NULL))
	{
		c = g_utf8_get_char (pWord); 
		if (g_unichar_isupper (c)) 
		{
			int i, len = g_utf8_strlen (pWord, -1);
			if (len == 1) 
				return WORD_CASE_INITIAL_CAPS;
			for (i = 0; i < len; i++ )
			{
				p = g_utf8_offset_to_pointer (pWord, i);
				c = g_utf8_get_char (p);
				if (g_unichar_islower (c))
					return WORD_CASE_INITIAL_CAPS;
			}
			return WORD_CASE_ALL_CAPS;
		}
		else if (g_unichar_istitle (c))
		{
			/* FIXME: this isn't quite right;
			 * we're only checking the first char 
			 */
			return WORD_CASE_TITLE;
		}
	}
	return WORD_CASE_LOWER;
}

/**
 * ApplyCase:
 **/
static void
ApplyCase (gchar **pWord, WordPredictionCaseType caseType)
{
	/* 
	 * FIXME: requires a significant rework before unicode
	 * characters can work with this framework.
	 */
	gunichar c;
	int i, len;

	switch (caseType) {
	case WORD_CASE_TITLE:
	case WORD_CASE_INITIAL_CAPS:
		**pWord = g_ascii_toupper (**pWord);
		break;
	case WORD_CASE_ALL_CAPS:
		len = strlen (*pWord);
/*		fprintf (stderr, "length %d\n", len);*/
		for (i = 0; i < len; i++) 
		{
			(*pWord)[i] = g_ascii_toupper ((*pWord)[i]);
		}
		break;
	case WORD_CASE_LOWER:
/*		fprintf (stderr, "lower case\n");*/
	default:
		break;
	}
}

/**
 * WordCompleteApplyCase:
 *
 **/
static void
WordCompleteApplyCase (const gchar *pWord, gchar **pPredictions, gint num)
{
	int i;
	WordPredictionCaseType caseType = WORD_CASE_LOWER;

	caseType = DetermineCase (pWord);
/*	fprintf (stderr, "case of [%s] is %d;", pWord, caseType);*/
	if (caseType != WORD_CASE_LOWER)
	{
		gchar *p;
		ApplyCase (pPredictions, caseType);
		p = strchr (*pPredictions, '\t');
		while (p)
		{
			++p;
/*			fprintf (stderr, "applycase at %p\n", p);*/
			ApplyCase (&p, caseType);
			p = strchr (p, '\t');
		}
	}
}


/*
 * WordCompletePredict
 *
 * Makes word predictions based upon the part word given.
 * Returns an array of strings which are the predictions, or NULL if no predictions were made.
 *
 * pWord - [in] the part word we want to complete
 * numberPredictions - [in] maximum number of predictions to return
 */
static gchar**
WordCompletePredictString (GokWordComplete *complete, const gchar* pWord, gint numberPredictions)
{
	gchar **word_predict_list = NULL;

	/* make the prediction */

	gchar bufferLowercase[MAXWORDLENGTH + 1];
	gint lengthWord;
	gint count, i, j;
	gchar* pWordLowercase;
	gchar buffer [MAXWORDLENGTH + 1];

	gint bMadePrediction;
	gint highPriority;
	gint highPriorityIndex;

	/* search the trie for the given word */
	/* find the first node with the word's first letter */
	Node* pNode = m_FirstNode.pFirstChildNode;

	/* validate the given values */
	if (pWord == NULL)
	{
	  return NULL;
	}

	if ((numberPredictions < 1) ||
	   (numberPredictions > MAXPREDICTIONS))
	{
	  return NULL;
	}

	word_predict_list = g_new0 (gchar *, numberPredictions);

	/* initialize the array */
	for (i = 0; i < MAXPREDICTIONS; i++)
	{
	  m_Predictions[i].priority = 0;
	}

	/* convert the given string to lowercase (cuz our dictionary is all lower case) */
	/* FIXME: not UTF-8 ready! */
	lengthWord = strlen (pWord);
	for (count = 0; count < lengthWord; count++)
	{
	  bufferLowercase[count] = tolower (pWord[count]);
	}
	bufferLowercase[count] = 0;
	pWordLowercase = &bufferLowercase[0];

	/* search the trie for the given word */
	/* find the first node with the word's first letter */
	pNode = m_FirstNode.pFirstChildNode;
	while (pNode != NULL)
	{
		if (pNode->letter == pWordLowercase[0])
		{
			buffer[0] = pWordLowercase[0];
			buffer[1] = 0;
			pWordLowercase++;
			/* create the predictions */
			WordCompletePredictLoop (pWordLowercase, pNode, buffer);
			break;
		}
		pNode = pNode->pNextNode;
	}

	/* look through our array of predictions and find the highest priority ones */
	bMadePrediction = FALSE;

	for (i = 0; i < numberPredictions; i++)
	{
		highPriority = 0;
		highPriorityIndex = -1;
		for (j = 0; j < MAXPREDICTIONS; j++)
		{
			if (m_Predictions[j].priority > highPriority)
			{
				highPriority = m_Predictions[j].priority;
				highPriorityIndex = j;
			}
		}
		if (highPriorityIndex == -1)
		{
		  break;
		}

		/* add the prediction word to our buffer */
		word_predict_list[i] = g_strdup (m_Predictions[highPriorityIndex].word);
		m_Predictions[highPriorityIndex].priority = 0;
		bMadePrediction = TRUE;
	}

	word_predict_list[i] = NULL;

	/* were there any predictions? */
	if (bMadePrediction == FALSE)
	{
		g_free (word_predict_list);
 	        return NULL; /* indicates no predictions made */
	}

	/* post-process the predictions to match the case of the input */
	WordCompleteApplyCase (pWord, word_predict_list, numberPredictions);

	return word_predict_list; /* predictions made! */
}

/*
 * PredictLoop
 *
 * Finds any words that start with the part word given.
 * This function is recursive.
 * Each word is stored in an array by calling "AddPrediction".
 */
static void 
WordCompletePredictLoop (const gchar* pWordGiven, Node* pNode, gchar* pPrediction)
{
	if (pNode != NULL)
	{
		int length = (int)strlen (pPrediction);

		/* are we at the end of the given word? */
		if (pWordGiven[0] == 0)
		{
			/* yes, add all words under this node*/
			/* loop through all child nodes (and their children too) */
			Node* pChildNode = pNode->pFirstChildNode;
			while (pChildNode != NULL)
			{
				pPrediction[length] = pChildNode->letter;
				pPrediction[length + 1] = 0;
                           /* priority > 0 means that this is the last letter in a word */
				if (pChildNode->priority != 0)
				{
					WordCompleteAddPrediction (pPrediction, pChildNode->priority);
				}
				/* add all the children's children */
				WordCompletePredictLoop (pWordGiven, pChildNode, pPrediction);
				pChildNode = pChildNode->pNextNode;
			}
		}
		else /* not at the end of the given word */
		{
			/* follow the trie down to the the next node in the word */
			/* find the child node that has the next letter in the word */
			Node* pChildNode = pNode->pFirstChildNode;
			while (pChildNode != NULL)
			{
				/* does  child have the next letter in the given word? */
				if (pChildNode->letter == pWordGiven[0])
				{
					/* yes, move on to next letter in the given word */
					pPrediction[length] = pWordGiven[0];
					pPrediction[length + 1] = 0;
					pWordGiven++;

					/* recurse */
					WordCompletePredictLoop (pWordGiven, pChildNode, pPrediction);
					break;
				}
				pChildNode = pChildNode->pNextNode;
			}
		}

		pPrediction[length] = 0;
	}
}

/*
 * WordCompleteAddPrediction
 *
 * Adds a predicted word to an array of predicted words. This array holds all 
 * the predictions. After going through the entire trie, pick the highest
 * priority predictions from this array and return them to the caller.
 */
static void 
WordCompleteAddPrediction (gchar* pWord, gint priority)
{
    gint x;
    gint lowPriorityIndex;
    gint lowPriority;

	/* can the prediction get added to a blank spot in the array? */
	for (x = 0; x < MAXPREDICTIONS; x++)
	{
		/* yes, add it to a blank spot in the array*/
		if (m_Predictions[x].priority == 0)
		{
			m_Predictions[x].priority = priority;
			strcpy (m_Predictions[x].word, pWord);
			return;
		}
	}

	/* the array is full so replace  lowest priority prediction with this prediction */
	/* find the prediction with the lowest priority */
        lowPriorityIndex = 0;
	lowPriority = 32000;
	for (x = 0; x < MAXPREDICTIONS; x++)
	{
		if (m_Predictions[x].priority < lowPriority)
		{
			lowPriority = m_Predictions[x].priority;
			lowPriorityIndex = x;
		}
	}

	/* is the lowest priority item less than this new prediction priority? */
	if (lowPriority < priority)
	{
		/* yes, replace the low priority prediction with this prediction*/
		m_Predictions[lowPriorityIndex].priority = priority;
		strcpy (m_Predictions[lowPriorityIndex].word, pWord);
	}
	/* no, the new prediction priority was less than all predictions in the array */
	/* so it doesn't get added to the array */
}

/*
 * WordCompleteAddNewWord
 *
 */
static gboolean 
WordCompleteAddNewWord (GokWordComplete *complete, const gchar* pWord)
{
	bWordCompleteModified = TRUE;

	return WordCompleteAddWord (complete, pWord, PRIORITY_NEWWORD, STATE_TEMPORARY);
}

static Node*
WordCompleteMatchingLeaf (Node *node, const gchar *pWord)
{
	Node *child = node->pFirstChildNode;
	while (child != NULL) {
		if (pWord && *pWord && (*pWord == child->letter)) {
			if (child->priority > 0) 
				return child;
			else {
				return WordCompleteMatchingLeaf (child, ++pWord);
			}
		}
		else {
			child = child->pNextNode;
		}
	}
	return NULL;
}

/*
 * WordCompleteIncrementFrequency
 *
 */
static gboolean 
WordCompleteIncrementFrequency (GokWordComplete *complete, const gchar *pWord)
{
	Node *node;
	if (pWord) {
		node = WordCompleteMatchingLeaf (&m_FirstNode, pWord);
		if (node && node->priority) {
			++node->priority;
			bWordCompleteModified = TRUE;
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * WordCompleteSaveDictionary
 *
 * Saves the entire dictionary to disk.
 * Returns TRUE if saved OK, FALSE if dictionary was not saved.
 */
static char 
WordCompleteSaveDictionary()
{
	FILE* pFile;
	Node* pNode;
	gchar buffer [MAXWORDLENGTH];

	/* open the dictionary file */
	pFile = fopen (dictionary_full_path, "wt");
	if (pFile == NULL)
	{
	  return FALSE;
	}

	/* write an identifer line to the file */
	if (fputs ("WPDictFile\n", pFile) == EOF)
	{
	  fclose (pFile);
	  return FALSE;
	}

	fprintf (stderr, "saving dictionary...\n");
	/* iterate through each child node */
	pNode = m_FirstNode.pFirstChildNode;
	while (pNode != NULL)
	{
		buffer[0] = 0;
		if (WordCompleteSaveLoop (pNode, buffer, pFile) == FALSE)
		{
			fclose (pFile);
			return FALSE;
		}
		pNode = pNode->pNextNode;
	}

	fclose (pFile);
	return TRUE;
}

/*
 * WordCompleteSaveLoop
 *
 * Recursive loop for saving all dictionary words to disk.
 */
static int 
WordCompleteSaveLoop (Node* pNode, gchar* pBuffer, FILE* pFile)
{
    gint lengthWord;
    Node* pLetter;

    lengthWord = strlen (pBuffer);
    pBuffer[lengthWord] = pNode->letter;
    pBuffer[lengthWord+1] = 0;

	if (pNode->priority != 0) /* priority > 0 means this is the end of a word */
	{
		/* write a word to the file */
		if (fprintf (pFile, "%s\t%d\t%d\n", pBuffer, pNode->priority, pNode->state) < 0)
		{
			return FALSE;
		}
	}

	pLetter = pNode->pFirstChildNode;
	while (pLetter != NULL)
	{
		if (WordCompleteSaveLoop (pLetter, pBuffer, pFile) == FALSE)
		{
			return FALSE;
		}
		pLetter = pLetter->pNextNode;
	}

	pBuffer[lengthWord] = 0;

	return TRUE;
}
