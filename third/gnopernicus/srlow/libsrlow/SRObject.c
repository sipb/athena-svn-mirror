/* SRObject.c
 *
 * Copyright 2001, 2002 Sun Microsystems, Inc.,
 * Copyright 2001, 2002 BAUM Retec, A.G.
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

#include "SRObject.h"
#include "SREvent.h"
#include "cspi/spi.h"
#include <stdio.h>
#include <string.h>

#define SRLOW_FEW_CHILDREN 3

/*
 *
 * Screen Reader Point Object's methods
 *
 */

/**
 * sr_point_get_x:
 * @point: a pointer to the #SRPoint object to query.
 * @x: a pointer to @gint32 to store the x coordinate of point.
 *
 * Get the x coordonate of @point.
 *
 * Returns: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean
sr_point_get_x (const SRPoint *point,
		gint32 *x)
{
    srl_return_val_if_fail (point && x, FALSE);
    
    *x = point->x;

    return TRUE;
}

/**
 * sr_point_get_y:
 * @point: a pointer to the #SRPoint object to query.
 * @y: a pointer to @gint32 to store the y coordinate of point.
 *
 * Get the y coordonate of @point.
 *
 * Returns: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean 
sr_point_get_y (const SRPoint *point,
		gint32 *y)
{
    srl_return_val_if_fail (point && y, FALSE);

    *y = point->y;

    return TRUE;
}

/**
 * sr_point_set_x:
 * @point: a pointer to the #SRPoint object to set.
 * @x: a @gint32 storing the x coordinate of point.
 *
 * Set the x coordonate of @point.
 *
 * Returns: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean
sr_point_set_x (SRPoint *point,
		gint32 x)
{
    srl_return_val_if_fail (point, FALSE);

    point->x = x;

    return TRUE;
}

/**
 * sr_point_set_y:
 * @point: a pointer to the #SRPoint object to set.
 * @y: a @gint32 storing the y coordinate of point.
 *
 * Get the y coordonate of @point.
 *
 * Returns: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean 
sr_point_set_y (SRPoint *point,
		gint32 y)
{
    srl_return_val_if_fail (point, FALSE);

    point->y = y;

    return TRUE;
}





/*
 *
 * Screen Reader Rectangle Object's methods
 *
 */

/**
 * sr_rectangle_get_x:
 * @rect: a pointer to the #SRRectangle object to query.
 * @x: a pointer to @gint32 to store the x coordinate of rectangle.
 *
 * Get the x coordonate of @rect.
 *
 * Returns: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean
sr_rectangle_get_x (const SRRectangle *rect,
		    gint32 *x)
{
    srl_return_val_if_fail (rect && x, FALSE);

    *x = rect->x;

    return TRUE;
}

/**
 * sr_rectangle_get_y:
 * @rect: a pointer to the #SRRectangle object to query.
 * @y: a pointer to @gint32 to store the y coordinate of rectangle.
 *
 * Get the y coordonate of @rect.
 *
 * Returns: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean 
sr_rectangle_get_y (const SRRectangle *rect,
		    gint32 *y)
{
    srl_return_val_if_fail (rect && y, FALSE);

    *y = rect->y;

    return TRUE;
}

/**
 * sr_rectangle_get_width:
 * @rect: a pointer to the #SRRectangle object to query.
 * @width: a pointer to @gint32 to store the width dimension of rectangle.
 *
 * Get the width dimension of @rect.
 *
 * Returns: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean
sr_rectangle_get_width (const SRRectangle *rect,
			gint32 *width)
{
    srl_return_val_if_fail (rect && width, FALSE);

    *width = rect->width;

    return TRUE;
}

/**
 * sr_rectangle_get_height:
 * @rect: a pointer to the #SRRectangle object to query.
 * @height: a pointer to @gint32 to store the height dimension of rectangle.
 *
 * Get the height dimension of @rect.
 *
 * Returns: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean
sr_rectangle_get_height (const SRRectangle *rect,
			 gint32 *height)
{
    srl_return_val_if_fail (rect && height, FALSE);

    *height = rect->height;

    return TRUE;
}




static AccessibleCoordType 
sr_2_acc_coord (SRCoordinateType type)
{
    switch (type)
    {
        case SR_COORD_TYPE_WINDOW:
    	    return SPI_COORD_TYPE_WINDOW;
	case SR_COORD_TYPE_SCREEN:
	    return SPI_COORD_TYPE_SCREEN;
	default:
	    srl_assert_not_reached ();
	    break;
    }
    return SPI_COORD_TYPE_SCREEN;
}


static AccessibleTextBoundaryType 
sr_2_acc_tb (SRTextBoundaryType type)
{
    switch (type)
    {
	case SR_TEXT_BOUNDARY_CHAR:
	    return SPI_TEXT_BOUNDARY_CHAR;
	case SR_TEXT_BOUNDARY_WORD:
	    return SPI_TEXT_BOUNDARY_WORD_START;
	case SR_TEXT_BOUNDARY_SENTENCE:
	    return SPI_TEXT_BOUNDARY_SENTENCE_START;
	case SR_TEXT_BOUNDARY_LINE:
	    return SPI_TEXT_BOUNDARY_LINE_START;
	default:
	    srl_assert_not_reached ();
	    break;
    }
    return SPI_TEXT_BOUNDARY_CHAR;
}


#ifdef SRL_DEBUG
extern GList *srl_str_list;
void 
SR_freeString (gchar *str)
{
    GList *found;
    
    srl_return_if_fail (str);
    
    found = g_list_find (srl_str_list, str);
    if (!found)
    {
	srl_debug ("\nWrong trying to remove next string");
	srl_debug (str ? str : "");
    }
/*    fprintf (stderr, "\nRE:%xp---%s", (unsigned int)str, str);*/
    srl_str_list = g_list_remove (srl_str_list, str);
    g_free (str);
}

void 
SR_strfreev (gchar **str)
{
    gint i;
    
    srl_return_if_fail (str);
    
    for (i = 0; str[i]; i++)
	SR_freeString (str[i]);
    g_free (str);
}

gchar *
SR_strdup (gchar *str)
{
    gchar *new_str;
    new_str = g_strdup (str);
/*    fprintf (stderr, "\nAL:%xp---%s", (unsigned int)new_str, new_str);*/
    srl_str_list = g_list_append (srl_str_list, new_str);
    return new_str;
}
#else
#define SR_strdup g_strdup
#endif

/*
 *
 * Screen Reader Specializations
 *
 */
typedef SRLong SRSpecialization;
#define SR_IS_NOTHING		(0)
#define SR_IS_ACTION		(1)
#define SR_IS_COMPONENT		(1<<1)
#define SR_IS_EDITABLE_TEXT	(1<<2)
#define SR_IS_HYPERTEXT		(1<<3)
#define SR_IS_IMAGE		(1<<4)
#define SR_IS_SELECTION		(1<<5)
#define SR_IS_TABLE		(1<<6)
#define SR_IS_TEXT		(1<<7)
#define SR_IS_VALUE		(1<<8)


/*
 *
 * Screen Reader Object Object
 *
 */
/**
 * SR_OBJECT:
 * @role: role of object.
 * @name: name of object.
 * @description: description of object.
 * @index_in_parent: index in parent.
 * @children_count: number of children.
 * @acc: pointer to an accessible object from which this screen reader object cames
 * @specialization: #SRSpecialization type.
*/
struct _SRObject
{
    GObject                     parent;

    SRObjectRoles		role;
    gchar			*reason;
    Accessible			*acc;
    GArray			*children;
    gchar			*text;
    gchar			*name;
    guint                       manages_descendants:1;
};


struct _SRObjectClass
{
  GObjectClass parent;

  gboolean (* is_action)	(const SRObject *obj, SRLong index);
  gboolean (* is_component)	(const SRObject *obj, SRLong index);
  gboolean (* is_editable_text)	(const SRObject *obj, SRLong index); 
  gboolean (* is_hypertext)	(const SRObject *obj, SRLong index);
  gboolean (* is_image)		(const SRObject *obj, SRLong index);
  gboolean (* is_selection)	(const SRObject *obj, SRLong index);
  gboolean (* is_table)		(const SRObject *obj, SRLong index);
  gboolean (* is_text)		(const SRObject *obj, SRLong index);
  gboolean (* is_value)		(const SRObject *obj, SRLong index);
  
  gboolean (* get_role) 	(const SRObject *obj,
				 SRObjectRoles *role, SRLong index);
  gboolean (* get_role_name)	(const SRObject *obj,
				 gchar **role_name, SRLong index);
  gboolean (* get_name) 	(const SRObject *obj,
				 gchar **name, SRLong index);
  gboolean (* get_description) 	(const SRObject *obj,
				 gchar **description, SRLong index);
  gboolean (* get_parent) 	(const SRObject *obj,
				 SRObject **parent);
  gboolean (* get_index_in_parent) (const SRObject *obj,
				    guint32 *index);
  gboolean (* get_children_count)  (const SRObject *obj,
				    guint32 *count);
  gboolean (* get_i_child)	(const SRObject *obj,
				 SRLong index, 
				 SRObject **child);
  gboolean (* get_location)	(SRObject *obj,
				 SRCoordinateType type, 
				 SRRectangle *location,
				 SRLong index);	
  gboolean (* get_state)        (const SRObject *obj,
				 SRState *state, SRLong index);
  gboolean (* get_relation)     (const SRObject *obj,
				 SRRelation *relation, SRLong index);				 
  gboolean (* get_layer)        (const SRObject *obj,
				 SRObjectLayer *layer, SRLong index);
  gboolean (* get_MDIZOrder)     (const SRObject *obj,
				  short *MDIZOrder, SRLong index);
  gboolean (* manages_descendants) (const SRObject *obj);
};


static AccessibleValue *
get_value_from_acc (Accessible *acc)
{
    AccessibleValue *value = NULL;
    
    srl_return_val_if_fail (acc, NULL);
    srl_return_val_if_fail (Accessible_isValue (acc), NULL);
    
    value = Accessible_getValue (acc);
    srl_debug (value);
    
    return value;    
}

static AccessibleImage *
get_image_from_acc (Accessible *acc)
{
    AccessibleImage *image = NULL;
    
    srl_return_val_if_fail (acc, NULL);
    
    if (Accessible_isImage (acc))
    {
        image = Accessible_getImage (acc);
    }
    else
    {
	if (Accessible_getRole (acc) == SPI_ROLE_TABLE_CELL)
	{
	    Accessible *child;
	    child = Accessible_getChildAtIndex (acc, 0);
	    if (child && Accessible_isImage (child))
		image = Accessible_getImage (child);
	    if (child)
		Accessible_unref (child);
	}
    }
    srl_debug (image);
    return image;    
}

static AccessibleAction *
get_action_from_acc (Accessible *acc)
{
    AccessibleAction *action = NULL;
    srl_return_val_if_fail (acc, NULL);

    if (Accessible_isAction (acc))
    {
	action = Accessible_getAction (acc);
    }
    else if (Accessible_getRole (acc) == SPI_ROLE_TABLE_CELL)
    {
	Accessible *child;
	child = Accessible_getChildAtIndex (acc, 1);
	if (child && Accessible_isAction (child))
	    action = Accessible_getAction (child);
	if (child)
	Accessible_unref (child);
    }
    srl_debug (action);
    return action;    
}


static AccessibleText *
get_text_from_acc (Accessible *acc)
{
    AccessibleText *text = NULL;
    srl_return_val_if_fail (acc, NULL);
    if (Accessible_isText (acc))
    {
        text = Accessible_getText (acc);
    }
    else
    {
	if (Accessible_getRole (acc) == SPI_ROLE_TABLE_CELL)
	{
	    Accessible *child;
	    child = Accessible_getChildAtIndex (acc, 1);
	    if (child && Accessible_isText (child))
		text = Accessible_getText (child);
	    if (child)
		Accessible_unref (child);
	}
	else if (Accessible_getRole (acc) == SPI_ROLE_COMBO_BOX)
	{
	    Accessible *child;
	    child = Accessible_getChildAtIndex (acc, 1);
	    if (child && Accessible_isText (child))
		text = Accessible_getText (child);
	    if (child)
		Accessible_unref (child);
	}
    }
    srl_debug (text);
    return text;
}


/*
 *
 * Screen Reader Object's methods
 *
 */

/**
 * sro_init:
 * @obj: a pointer to the #SRObject structure on which to operate.
 *
 * Make all the initialization for @obj.
 *
 * Returns: #TRUE if successful, #FALSE otherwise.
 *
**/
static gboolean 
sro_init (SRObject *obj)
{
    srl_return_val_if_fail (obj, FALSE);
    obj->role = SR_ROLE_UNKNOWN;
    obj->acc = NULL;
    obj->children = NULL;
    obj->text = NULL;
    obj->name = NULL;
    obj->reason = NULL;
    obj->manages_descendants = FALSE;
    return TRUE;
}



/**
 * sro_terminate:
 * @obj: a pointer to the #SRObject structure on wich to operate.
 *
 * Free all memory used by @obj stucture and it fields.
 *
 * Returns: #TRUE if successful, #FALSE otherwise.
 *
**/
static void 
sro_terminate (GObject *gobj)
{
    SRObject *obj;
    srl_return_if_fail (gobj);
    
    obj = SR_OBJECT (gobj);

    if (obj->acc)
    	Accessible_unref (obj->acc);
    if (obj->children)
    {
	int i;
	for (i = 0; i < obj->children->len; i++)
	{
	    Accessible *child;
	    child = g_array_index (obj->children, Accessible *, i);
	    srl_debug (child);
	    if (child)
		Accessible_unref (child);
	}
	g_array_free (obj->children, TRUE);
    }
    if (obj->reason)
	g_free (obj->reason);
    if (obj->text)
	SR_freeString (obj->text);
    if (obj->name)
	SR_freeString (obj->name);
}


void 
sro_add_reference (SRObject *obj)
{
    srl_return_if_fail (obj);
    srl_return_if_fail (SR_IS_OBJECT (obj));
    
    g_object_ref (obj);
}

#ifdef SRL_DEBUG
extern GList *srl_obj_list;
#endif


void 
sro_release_reference (SRObject *obj)
{
    srl_return_if_fail (obj);
    srl_return_if_fail (SR_IS_OBJECT (obj));
#ifdef SRL_DEBUG    
    if (G_OBJECT (obj)->ref_count == 1)
	srl_obj_list = g_list_remove (srl_obj_list, obj);
#endif    
    g_object_unref (obj);
}

/**
 * sro_new:
 *
 * Reserve memory space for storing a #SRObject object.
 *
 * Returns: a valid pointer to #SR_OBJECT structure if successful, otherwise NULL.
**/
SRObject* 
sro_new ()
{
#ifdef SRL_DEBUG
    SRObject *obj;
    obj = g_object_new (SR_TYPE_OBJECT, NULL);
    srl_obj_list = g_list_append (srl_obj_list, obj);
    return obj;
#else
    return g_object_new (SR_TYPE_OBJECT, NULL);	
#endif
} 



/**
 * sro_get_role:
 * @obj: a pointer to the #SRObject structure to query.
 * @role: pointer to the #SRObjectRole.
 *
 * Get the #SRObjectRole for @obj.
 *
 * Return: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean 
sro_get_role (const SRObject *obj, 
	      SRObjectRoles *role,
	      SRLong index)
{
  SRObjectClass *klass;

  srl_return_val_if_fail (SR_IS_OBJECT (obj), FALSE);

  klass = SR_OBJECT_GET_CLASS (obj);
  if (klass->get_role)
	  return (klass->get_role) (obj, role, index);
  else
	  return FALSE;
}

gboolean
sro_manages_descendants (SRObject *obj)
{
  SRObjectClass *klass;

  srl_return_val_if_fail (SR_IS_OBJECT (obj), FALSE);

  klass = SR_OBJECT_GET_CLASS (obj);
  if (klass->manages_descendants)
	  return (klass->manages_descendants) (obj);
  else
	  return FALSE;
}


gboolean 
sro_get_state (const SRObject *obj, 
	       SRState *state, 
	       SRLong index)
{
  SRObjectClass *klass;

  srl_return_val_if_fail (SR_IS_OBJECT (obj), FALSE);

  klass = SR_OBJECT_GET_CLASS (obj);
  if (klass->get_state)
	  return (klass->get_state) (obj, state, index);
  else
	  return FALSE;
}

gboolean 
sro_get_relation (const SRObject *obj, 
		  SRRelation *relation,
		  SRLong index)
{
  SRObjectClass *klass;

  srl_return_val_if_fail (SR_IS_OBJECT (obj), FALSE);

  klass = SR_OBJECT_GET_CLASS (obj);
  if (klass->get_relation)
	  return (klass->get_relation) (obj, relation, index);
  else
	  return FALSE;
}


/**
 * sro_get_name:
 * @obj: a pointer to the #SRObject structure to query.
 * @name: a pointer to a pointer to char.
 *
 * Get a pointer to the name for a @obj.
 *
 * Return: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean 
sro_get_name (const SRObject *obj, 
	      char **name, 
	      SRLong index)
{
  SRObjectClass *klass;

  srl_return_val_if_fail (SR_IS_OBJECT (obj), FALSE);

  klass = SR_OBJECT_GET_CLASS (obj);
  if (klass->get_name)
	  return (klass->get_name) (obj, name, index);
  else
	  return FALSE;
}

/**
 * sro_get_description:
 * @obj: a pointer to the #SRObject structure to query.
 * @description: a pointer to a pointer to char.
 *
 * Get a pointer to the description for @obj.
 *
 * Return: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean 
sro_get_description (const SRObject *obj,
		     char **description,
		     SRLong index)
{
  SRObjectClass *klass;

  srl_return_val_if_fail (SR_IS_OBJECT (obj), FALSE);

  klass = SR_OBJECT_GET_CLASS (obj);
  if (klass->get_description)
	  return (klass->get_description) (obj, description, index);
  else
	  return FALSE;
}

 
static SRRelation
get_relation_from_acc (Accessible *acc)
{
    AccessibleRelation **acc_relation;
    SRRelation relation;
    int i;

    srl_return_val_if_fail (acc, SR_RELATION_NONE);
    
    relation = SR_RELATION_NONE;
    acc_relation = Accessible_getRelationSet (acc);
    if (!acc_relation)
	return SR_RELATION_NONE;
    
    for (i = 0; acc_relation[i]; i++) 
    {
	AccessibleRelationType type;
	type = AccessibleRelation_getRelationType (acc_relation[i]);
	switch (type)
	{
	    case SPI_RELATION_CONTROLLED_BY:
		relation |= SR_RELATION_CONTROLLED_BY;
		break;
	    case SPI_RELATION_CONTROLLER_FOR:
		relation |= SR_RELATION_CONTROLLER_FOR;
		break;
	    case SPI_RELATION_MEMBER_OF:
		relation |= SR_RELATION_MEMBER_OF;
		break;
	    case SPI_RELATION_EXTENDED:
		relation |= SR_RELATION_EXTENDED;
		break;
	    case SPI_RELATION_LABEL_FOR:
	    case SPI_RELATION_LABELED_BY:
		break;
	    default:
		srl_assert_not_reached ();
		break;
	}
	AccessibleRelation_unref (acc_relation[i]);
    }
    g_free (acc_relation);
    return relation;
}


Accessible *
sro_get_acc_at_index (const SRObject *obj, const gint index)
{
    Accessible *acc = NULL;
    srl_return_val_if_fail (obj, NULL);
    
    if (index == SR_INDEX_CONTAINER)
	acc = obj->acc;
    else
    {
	if (obj->children && 0 <= index && index < obj->children->len)
	    acc = g_array_index (obj->children, Accessible *, index);
	else
	    acc = Accessible_getChildAtIndex (obj->acc, index);
    }

    return acc;
}

static gboolean
sro_default_manages_descendants (const SRObject *obj)
{
  return obj->manages_descendants;
}

static gboolean 
sro_default_get_relation (const SRObject *obj, 
	    		  SRRelation *relation,
			  SRLong index)
{
    Accessible *acc;
    gboolean rv = FALSE;
    
    if (relation)
	*relation = SR_RELATION_NONE;

    srl_return_val_if_fail (obj && relation, FALSE);

    acc = sro_get_acc_at_index (obj, index);

    if (acc)
    {
	*relation = get_relation_from_acc (acc);
	rv = TRUE;
    }
    
    return rv;	
}


static SRState 
get_state_from_acc (Accessible *acc)
{
    AccessibleStateSet *state_acc;
    SRState state_sr;
    
    srl_return_val_if_fail (acc, SR_STATE_INVALID);
    
    state_acc = Accessible_getStateSet (acc);
    if (!state_acc)
	return SR_STATE_INVALID;
    
    state_sr = SR_STATE_INVALID;
    if (AccessibleStateSet_contains (state_acc, SPI_STATE_ACTIVE))
        state_sr |= SR_STATE_ACTIVE;
    if (AccessibleStateSet_contains (state_acc, SPI_STATE_CHECKED))
        state_sr |= SR_STATE_CHECKED | SR_STATE_CHECKABLE;
    if (AccessibleStateSet_contains (state_acc, SPI_STATE_COLLAPSED))
        state_sr |= SR_STATE_COLLAPSED;
    if (AccessibleStateSet_contains (state_acc, SPI_STATE_EDITABLE))
        state_sr |= SR_STATE_EDITABLE;
    if (AccessibleStateSet_contains (state_acc, SPI_STATE_EXPANDABLE))
        state_sr |= SR_STATE_EXPANDABLE;	        
    if (AccessibleStateSet_contains (state_acc, SPI_STATE_EXPANDED))
        state_sr |= SR_STATE_EXPANDED;	    
    if (AccessibleStateSet_contains (state_acc, SPI_STATE_FOCUSABLE))
        state_sr |= SR_STATE_FOCUSABLE;	        
    if (AccessibleStateSet_contains (state_acc, SPI_STATE_FOCUSED))
        state_sr |= SR_STATE_FOCUSED;	    
    if (AccessibleStateSet_contains (state_acc, SPI_STATE_MODAL))
        state_sr |= SR_STATE_MODAL;	    
    if (AccessibleStateSet_contains (state_acc, SPI_STATE_PRESSED))
        state_sr |= SR_STATE_PRESSED;	    
    if (AccessibleStateSet_contains (state_acc, SPI_STATE_SELECTED))
        state_sr |= SR_STATE_SELECTED;	    
    if (AccessibleStateSet_contains (state_acc, SPI_STATE_VISIBLE))
        state_sr |= SR_STATE_VISIBLE;	    
    if (AccessibleStateSet_contains (state_acc, SPI_STATE_SHOWING))
        state_sr |= SR_STATE_SHOWING;	    
    if (AccessibleStateSet_contains (state_acc, SPI_STATE_ICONIFIED))
        state_sr |= SR_STATE_MINIMIZED;	    
    if (AccessibleStateSet_contains (state_acc, SPI_STATE_ENABLED))
	state_sr |= SR_STATE_ENABLED;	    
    AccessibleStateSet_unref (state_acc);
    
    if ((state_sr & SR_STATE_CHECKABLE) != SR_STATE_CHECKABLE)
    {
	switch (Accessible_getRole (acc))
	{
	    case SPI_ROLE_RADIO_MENU_ITEM:
	    case SPI_ROLE_CHECK_MENU_ITEM:
	    case SPI_ROLE_TOGGLE_BUTTON:
	    case SPI_ROLE_CHECK_BOX:
	    case SPI_ROLE_RADIO_BUTTON:	    
	        state_sr |= SR_STATE_CHECKABLE;
		break;
	    case SPI_ROLE_TABLE_CELL:
		if (Accessible_isAction (acc))
		{
		    AccessibleAction *action;
		    long i;
		    action = Accessible_getAction (acc);
		    if (action)
		    {
			for (i = 0; i < AccessibleAction_getNActions (action); i++)
			{
		    	    gchar *tmp;
		    	    tmp = AccessibleAction_getName (action, i);
		    	    if (tmp && strcmp (tmp, "toggle") == 0)
		    		state_sr |= SR_STATE_CHECKABLE;
			    SPI_freeString (tmp);
			}
		    }
		    if (action)
			AccessibleAction_unref (action);
		}
		break;
	    default:
		break;
	}
    }
    return state_sr;
}


static gboolean 
sro_default_get_state (const SRObject *obj, 
	    	       SRState *state,
	    	       SRLong index)
{
    Accessible *acc;
    gboolean rv = FALSE;
    if (state)
	*state  = SR_STATE_INVALID;
    srl_return_val_if_fail (obj && state, FALSE);

    acc = sro_get_acc_at_index (obj, index);
    
    if (acc)
    {    
	*state = get_state_from_acc (acc); 
	rv = TRUE;
    }
    
    return rv;	
}



static gchar *
get_name_from_label_rel (Accessible *acc)
{
    gchar *name = NULL, *old;
    AccessibleRelation **relation;
    int i;
    
    srl_return_val_if_fail (acc, NULL);
    relation = Accessible_getRelationSet (acc);
    if (!relation)
	return NULL;
    for (i = 0; relation[i]; ++i)
    {
        AccessibleRelationType type;
        type = AccessibleRelation_getRelationType (relation[i]);
        if (type == SPI_RELATION_LABELED_BY)
        {
	    gint j, cnt;
	    
	    cnt = AccessibleRelation_getNTargets (relation[i]);
	    for (j = 0; j < cnt; j++)
	    {
    		Accessible *label;
		label = AccessibleRelation_getTarget (relation[i], j);
		if (label)
		{
		    gchar *tmp;
		    tmp = Accessible_getName (label);
		    if (tmp && tmp[0])
		    {
			if (name)
			{
			    gchar *old = name;
			    name = g_strconcat (name, " ", tmp, NULL);
			    g_free (old);
			}
			else
			    name = g_strdup (tmp);
		    }
		    SPI_freeString (tmp);
		    Accessible_unref (label);
		}
	    }
	}
    }
    for (i = 0; relation[i]; ++i)
        AccessibleRelation_unref (relation[i]);
    g_free (relation);
    old = name;
    name = SR_strdup (name);
    g_free (old);
    return name;
}

gboolean 
sro_get_reason (const SRObject *obj, 
	    	 char **reason)
{
    if (reason)
	*reason = NULL;
    srl_return_val_if_fail (obj && reason, FALSE);
    
    if (obj->reason)
	*reason = SR_strdup (obj->reason);
    
    return *reason ? TRUE : FALSE;
}

/**
 * sro_get_name:
 * @obj: a pointer to the #SRObject structure to query.
 * @name: a pointer to a pointer to char.
 *
 * Get a pointer to the name for a @obj.
 *
 * Return: #TRUE if successful, #FALSE otherwise.
 *
**/
static gboolean 
sro_default_get_name (const SRObject *obj, 
	    	      char **name,
		      SRLong index)
{
    gchar *name_ = NULL;
    Accessible *acc = NULL;
    SRObjectRoles role = SR_ROLE_UNKNOWN;
    if (name)
	*name = NULL;
    srl_return_val_if_fail (obj && name, FALSE);
    
    acc = sro_get_acc_at_index (obj, index);

    sro_get_role (obj, &role, index);

    switch (role)
    {
    	case SR_ROLE_TABLE_COLUMNS_HEADER:
	    {
	        Accessible *parent=NULL, *header=NULL;
		AccessibleTable *table = NULL;
	
		parent = Accessible_getParent (acc);
		if (parent)
		    table = Accessible_getTable (parent);
		if (table)
	    	{
		    long col;
		    
		    col = AccessibleTable_getColumnAtIndex (table,
				Accessible_getIndexInParent (acc));
		    if (col>=0)
			header = AccessibleTable_getColumnHeader (table, col);
		}	
		if (header)
		{
		    gchar *tmp;
		    tmp = Accessible_getName (header);
		    if (tmp && tmp[0])
			name_ = SR_strdup (tmp);
		    SPI_freeString (tmp);    
		}	
		if (parent)
		    Accessible_unref (parent);
		if (table)
		    Accessible_unref (table);
		if (header) 
		    Accessible_unref (header);
	    }
	    break;
	case SR_ROLE_TABLE:
	case SR_ROLE_TREE_TABLE:
	    {
	        gchar *name2;
		Accessible *caption = NULL;
		AccessibleTable *table = NULL;
	
		name2 = get_name_from_label_rel (acc);
		table = Accessible_getTable (acc);
		if (table)
		    caption = AccessibleTable_getCaption (table);
		if (caption)
		{
	    	    gchar *tmp;
	    	    tmp = Accessible_getName (caption);
	    	    if (tmp && tmp[0])
		    {
			if (name2)
			{
			    gchar *tmp2;
			    tmp2 = g_strconcat (name2, " ", tmp, NULL);
			    name_ = SR_strdup (tmp2);
			    SPI_freeString (tmp2);
			}
			else
		    	    name_ = SR_strdup (tmp);
		    }
		    SPI_freeString (tmp);
		}
		if (!name_ && name2)
		    name_ = SR_strdup (name2);	
		if (table)
		    AccessibleTable_unref (table);
		if (caption)
		    Accessible_unref (caption); 
		if (!name_)
		{
		    gchar *tmp;
		    tmp = Accessible_getName (acc);
		    if (tmp && tmp[0])
			name_ = SR_strdup (tmp);
		    SPI_freeString (tmp);	
		}       
	    }
	    break;
	case SR_ROLE_COMBO_BOX: /* bug 158055 */
	    {
		AccessibleText *text;
		gchar *name2, *tmp;
		name2 = get_name_from_label_rel (acc);
		tmp = Accessible_getName (acc);
		text = get_text_from_acc (acc);
		if (!text)
		{ 
		    if (tmp && tmp[0])
		    {
			if (name2)
			{
			    gchar *tmp2;
		    	    tmp2 = g_strconcat (name2, " ", tmp, NULL);
			    name_ = SR_strdup (tmp2);
			    SR_freeString (name2);
			    g_free (tmp2);
			}
			else
			    name_ = SR_strdup (tmp);
		    }
		    else
			name_ = name2;
		}
		else
		{
		    if (name2)
			name_ = name2;
		    else
			if (tmp && tmp[0])
			    name_ = SR_strdup (tmp);
		    AccessibleText_unref (text);
		}
		SPI_freeString (tmp);
	    }
	    break;
	case SR_ROLE_TEXT_SL:
	case SR_ROLE_TEXT_ML:
/*	case SR_ROLE_COMBO_BOX: bug 158055 */
	case SR_ROLE_SPIN_BUTTON:
	case SR_ROLE_SLIDER:
	    name_ = get_name_from_label_rel (acc);
	    if (!name_)
	    {
	    	gchar *tmp;
		tmp = Accessible_getName (acc);
		if (tmp && tmp[0])
		    name_ = SR_strdup (tmp);
		SPI_freeString (tmp);
	    }
	    break;
	case SR_ROLE_PUSH_BUTTON:
	    {
		gchar *tmp, *name2;
		name2 = get_name_from_label_rel (acc);
		tmp = Accessible_getName (acc);
		if (tmp && tmp[0])
		{
		    if (name2)
		    {
			gchar *tmp2;
		    	tmp2 = g_strconcat (name2, " ", tmp, NULL);
			name_ = SR_strdup (tmp2);
			SR_freeString (name2);
		    }
		    else
			name_ = SR_strdup (tmp);
		}
		else
		{
		    gint i, cnt;
		    gchar *cname = g_strdup ("");
		    cnt = Accessible_getChildCount (acc);
		    for (i = 0; i< cnt; i++)
		    {
			Accessible *child = Accessible_getChildAtIndex (acc, i);
			if (child)
			{
			    gchar *t,*tname = Accessible_getName (child);
			    t = cname;
			    cname = g_strconcat (cname, cname[0] ? " " : "", tname, NULL);
			    g_free (t);
			    Accessible_unref (child);
			    SPI_freeString (tname);
			}
		    }
		    if (cname && cname[0])
		    {
			gchar *tmp2;
		    	tmp2 = g_strconcat (name2 ? name2 : "", name2 ? " ": "", cname, NULL);
			name_ = SR_strdup (tmp2);
		    }
		    else
			name_ = name2;
		    g_free (cname);
		}
		SPI_freeString (tmp);
	    }
	    break;
	case SR_ROLE_RADIO_BUTTON:
	case SR_ROLE_CHECK_BOX:
	    {
		gchar *tmp, *name2;
		name2 = get_name_from_label_rel (acc);
		tmp = Accessible_getName (acc);
		if (tmp && tmp[0])
		{
		    if (name2)
		    {
			gchar *tmp2;
		    	tmp2 = g_strconcat (name2, " ", tmp, NULL);
			name_ = SR_strdup (tmp2);
			SR_freeString (name2);
		    }
		    else
			name_ = SR_strdup (tmp);
		}
		else
		{
		    name_ = name2;
		}
		SPI_freeString (tmp);
	    }
	    break;
	case SR_ROLE_TABLE_CELL:
	case SR_ROLE_TABLE_LINE:
	    {
		gchar *tmp; 
		Accessible *cell = NULL;
		srl_debug (Accessible_getChildCount (acc) == 0 ||
			    Accessible_getChildCount (acc) == 2);
		if (Accessible_getChildCount (acc) == 2)
		    cell = Accessible_getChildAtIndex (acc, 1);
		else
		{
		    cell = acc;
		    Accessible_ref (cell);
		}
		    
		tmp = Accessible_getName (cell);
		if (tmp && tmp[0])
		{
		    name_ = SR_strdup (tmp);
		}
		SPI_freeString (tmp);
		if (cell)
		    Accessible_unref (cell);
	    }
	    break;
	case SR_ROLE_LABEL:
	    {
		sro_text_get_text_from_caret ((SRObject *)obj, SR_TEXT_BOUNDARY_LINE,
					&name_, SR_INDEX_CONTAINER);
	    }
	    break;
	case SR_ROLE_ICON:
	    {
		gchar *desc, *name, *tmp;
		sro_image_get_description ((SRObject*)obj, &desc, SR_INDEX_CONTAINER);
		name = Accessible_getName (acc);
		if (name && desc)
		    name_ = g_strconcat (name, " ", desc, NULL);
		else 
		    name_ = g_strdup (name ? name : desc);
		if (desc)
		    SR_freeString (desc);
		SPI_freeString (name);
		tmp = name_;
		name_ = SR_strdup (tmp);
		g_free (tmp);
	    }
	    break;
	case SR_ROLE_POPUP_MENU:
	    {
		gchar *tmp;
		tmp = Accessible_getName (acc);
		if (tmp && tmp[0])
		    name_ = SR_strdup (tmp);
		else
		    name_ = SR_strdup (("Context Menu")); /*FIXME: should be marked for translation*/
		SPI_freeString (tmp);
	    }
	    break;
	default:
	    {
		gchar *tmp;
		tmp = Accessible_getName (acc);
		if (tmp && tmp[0])
		    name_ = SR_strdup (tmp);
		SPI_freeString (tmp);
	    }
	    break;
    }

    *name = name_;
    if (!(*name) && obj->name)
	*name = SR_strdup (obj->name);
    if (!(*name))
    {
	gchar *tmp = Accessible_getDescription (acc);
	if (tmp && tmp[0])
	    *name = SR_strdup (tmp);
	SPI_freeString (tmp);
    }
    return *name ? TRUE : FALSE;
}

/**
 * sro_get_column_header:
 * @obj; a pointer to the #SRObject structure to query.
 * @header_name: a pointer to a pointer to char.
 *
 * Get a pointer to the header name for a @obj.
 *
 * Return: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean
sro_get_column_header (const SRObject *obj,
                       char **header_name,
		       SRLong index)
{
    gchar *name_ = NULL;
    Accessible *acc = NULL, *parent = NULL, *header = NULL;
    AccessibleTable *table = NULL;
    if (header_name)
	*header_name = NULL;
    srl_return_val_if_fail (obj && header_name, FALSE);	
    
    acc = sro_get_acc_at_index (obj, index);
    
    switch (obj->role)
    {
	case SR_ROLE_TABLE:
	case SR_ROLE_TREE_TABLE:
	{
	    long index = -1;
	    Accessible *child = NULL;
	    AccessibleSelection *selection;
	    selection = Accessible_getSelection (acc);
 	    if (selection)
	    {
		gint i, cnt;
		cnt = AccessibleSelection_getNSelectedChildren (selection);		    
		if (cnt == 1)
		{
		    child = AccessibleSelection_getSelectedChild (selection, 0);
		    if (child && (Accessible_getRole (child) == SPI_ROLE_TABLE_CELL))
		    {
			table = Accessible_getTable (acc);
			if (table)
			    index = AccessibleTable_getColumnAtIndex (table, 
					    Accessible_getIndexInParent (child));
		    }
		    Accessible_unref (child);
		}
		else
		{
		    for (i = 0; i < cnt; i++ )
		    {
			child = AccessibleSelection_getSelectedChild (selection, i);
			if (child && (Accessible_getRole (child) == SPI_ROLE_TABLE_CELL))
			{
			    AccessibleStateSet *states;
			    states = Accessible_getStateSet (child);
			    if (AccessibleStateSet_contains (states, SPI_STATE_FOCUSED))
			    {
				table = Accessible_getTable (acc);
				if (table)
				{
				    index = AccessibleTable_getColumnAtIndex (table,
				        		Accessible_getIndexInParent (child));
				    AccessibleStateSet_unref (states);
				    Accessible_unref (child);
				    break;
				}    		
			    }	
			    AccessibleStateSet_unref (states);    
			}
			Accessible_unref (child);
		    }	
		}	    
		if (index >= 0)
		    header = AccessibleTable_getColumnHeader (table, index);  
		AccessibleSelection_unref (selection);
	    }	
	}
	break;
	default:
	{	
	    parent = Accessible_getParent (acc);
	    if (parent)
		table = Accessible_getTable (parent);
	    if (table)
	    {  
		long col;
		col = AccessibleTable_getColumnAtIndex (table,
				    Accessible_getIndexInParent (acc));
		if (col >= 0)
		    header = AccessibleTable_getColumnHeader (table, col);
	    }
	}
	break;
    } 

    if (header)
    {
	AccessibleStateSet *set;
	set = Accessible_getStateSet (header);
	/* present the column header only if it is displayed on the screen */
	if (AccessibleStateSet_contains (set, SPI_STATE_SHOWING))
	{
	    gchar *tmp;
	    tmp = Accessible_getName (header);
	    if (tmp && tmp[0])
		name_ = SR_strdup (tmp);
	    SPI_freeString (tmp);    
	}
	AccessibleStateSet_unref (set);
    }
    if (parent)
	Accessible_unref (parent);
    if (table)
	Accessible_unref (table);
    if (header)
	Accessible_unref (header);
    if (name_ && name_[0])		
	*header_name = name_;
    
    return *header_name ? TRUE : FALSE;
    
}

/**
 * sro_get_row_header:
 * @obj: a pointer to the #SRObject structure to query.
 * @header_name: a pointer to a pointer to char.
 *
 * Get a pointer to the table for @obj.
 *
 * Return: #TRUE if successful, #FALSE otherwise.
 *
**/ 
gboolean
sro_get_row_header (const SRObject *obj,
                    char **header_name,
		    SRLong index)
{
    gchar *name_ = NULL;
    Accessible *acc = NULL, *parent = NULL, *header = NULL;
    AccessibleTable *table = NULL;
    if (header_name)
	*header_name = NULL;
    srl_return_val_if_fail (obj && header_name, FALSE);	
    
    acc = sro_get_acc_at_index (obj, index);
    
    switch (obj->role)
     {    
	case SR_ROLE_TABLE:
	case SR_ROLE_TREE_TABLE:
	{
	
	    long index = -1;
	    Accessible *child = NULL;
	    AccessibleSelection *selection;
	    selection = Accessible_getSelection (acc);
	    if (selection)
	    {
		gint i, cnt;
		cnt = AccessibleSelection_getNSelectedChildren (selection);
		if (cnt == 1)
		{
		    child = AccessibleSelection_getSelectedChild (selection, 0);
		    if (child && (Accessible_getRole (child) == SPI_ROLE_TABLE_CELL))
	    {
		table = Accessible_getTable (acc);
			if (table)
			    index = AccessibleTable_getRowAtIndex (table,
					Accessible_getIndexInParent (child));
		    }
		    Accessible_unref (child);
		}
		else
		{
		    for (i = 0; i < cnt; i++)
		    {
			child = AccessibleSelection_getSelectedChild (selection, i);
			if (child && (Accessible_getRole (child) == SPI_ROLE_TABLE_CELL))
			{
			    AccessibleStateSet *states;
			    states = Accessible_getStateSet (child);
    			    if (AccessibleStateSet_contains (states, SPI_STATE_FOCUSED))
			    {
				table = Accessible_getTable (acc);
				if (table)
				{
				    index = AccessibleTable_getRowAtIndex (table,
						Accessible_getIndexInParent (child));	    
				    AccessibleStateSet_unref (states);
				    Accessible_unref (child);
				    break;
				}
			    }
			    AccessibleStateSet_unref (states);
			}
			Accessible_unref (child);
		    }
		}    	    	    		
		if (index >= 0)
		    header = AccessibleTable_getRowHeader (table, index);
		if (header)
		{
		    gchar *tmp;
		    tmp = Accessible_getName (header);
		    if (tmp && tmp[0])
			name_ = SR_strdup (tmp);
		    SPI_freeString (tmp);
		}
		AccessibleSelection_unref (selection);
	    }
	}
	break;
	default:	
	{    
	    parent = Accessible_getParent (acc);
	    if (parent)
		table = Accessible_getTable (parent);
	    if (table)
	    {
		long row;
		row = AccessibleTable_getRowAtIndex (table,
			    Accessible_getIndexInParent (acc));	    
		if (row >= 0)
		    header = AccessibleTable_getRowHeader (table, row);	    
	    }	 
	    if (header)
	    {
		gchar *tmp;
		tmp = Accessible_getName (header);
		if (tmp && tmp[0])
		    name_ = SR_strdup (tmp);
		SPI_freeString (tmp);        
	    }
	}
	break;
     }
     if (parent)
 	Accessible_unref (parent);
     if (table)
	AccessibleTable_unref (table);
     if (header)
	Accessible_unref (header);
				
    if (name_ && name_[0])
	*header_name = name_;
		
     return *header_name ? TRUE : FALSE;
 }
 

/**
 * sro_get_cell:
 * @obj: a pointer to the #SRObject structure to query.
 * @cell: a pointer to a pointer to char.
 *
 * Get a pointer to the table for @obj.
 *
 * Return: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean
sro_get_cell (const SRObject *obj,
	      char **cell,
	      SRLong index)
{
    gchar *name_ = "";
    Accessible *acc = NULL, *child = NULL;
    
    if (cell)
	*cell = NULL;
    srl_return_val_if_fail (obj && cell, FALSE);
    
    acc = sro_get_acc_at_index (obj, index);
    
    switch (obj->role)
    {
	case SR_ROLE_TABLE:
	case SR_ROLE_TREE_TABLE:
	{
	    AccessibleSelection *selection;
	    selection = Accessible_getSelection (acc);
	    if (selection)
	    {
		gint i, cnt;
		cnt = AccessibleSelection_getNSelectedChildren (selection);
		if (cnt == 1)
		{
		    child = AccessibleSelection_getSelectedChild (selection, 0);
		    if (child && (Accessible_getRole (child) == SPI_ROLE_TABLE_CELL))
		    {
			glong childcnt = 0;
			childcnt = Accessible_getChildCount (child);
			/*cell has no children*/
			if (childcnt == 0)
			{
			    gchar *tmp = NULL;
			    tmp = Accessible_getName (child);
			    if (tmp && tmp[0])
				name_ = g_strdup (tmp);
			    SPI_freeString (tmp);         
			}	
			/*there is a container cell*/
			else
			{
			    for (i = 0; i < childcnt; i++ )
			    {
				Accessible *cell_child = NULL;
				cell_child = Accessible_getChildAtIndex (child, i);
				if (cell_child)
				{
				    gchar* tmp = NULL;
				    tmp = Accessible_getName (cell_child);
				    if (tmp && tmp[0])
					name_ = g_strconcat (name_, " ", tmp, NULL);
				    SPI_freeString (tmp);     
				    Accessible_unref (cell_child);
				}
			    }
			}
		    }
		    Accessible_unref (child);    
		}
		else
		{
		    for (i = 0; i < cnt; i++)
		    {
			child = AccessibleSelection_getSelectedChild (selection, i);
			if (child && (Accessible_getRole (child) == SPI_ROLE_TABLE_CELL))
			{
			    AccessibleStateSet *states;
			    states = Accessible_getStateSet (child); 
			    if (AccessibleStateSet_contains (states, SPI_STATE_FOCUSED))
			    {
				glong childcnt = 0;
				childcnt = Accessible_getChildCount (child);
				/*cell has no children*/
				if (childcnt == 0)
				{
				    gchar *tmp = NULL;
				    tmp = Accessible_getName (child);
				    if (tmp && tmp[0])
					name_ = g_strdup (tmp);
				    SPI_freeString (tmp);         
				}	
				/*there is a container cell*/
				else
				{
				    for (i = 0; i < childcnt; i++ )
				    {
					Accessible *cell_child = NULL;
					cell_child = Accessible_getChildAtIndex (child, i);
					if (cell_child)
					{
					    gchar* tmp = NULL;
					    tmp = Accessible_getName (cell_child);
					    if (tmp && tmp[0])
						name_ = g_strconcat (name_, " ", tmp, NULL);
					    SPI_freeString (tmp);     
					    Accessible_unref (cell_child);
					}
				    }
				}
				Accessible_unref (child);
				break;	
			    }
    			    AccessibleStateSet_unref (states);
			}
			Accessible_unref (child);
		    }
		}
	    }	    
	    AccessibleSelection_unref (selection);
	}
	break;
	
	default:
	{
	    Accessible *parent = NULL, *table = NULL;
	    gint i;
	    
	    parent = Accessible_getParent (acc);
	    if (parent)
		table = Accessible_getTable (parent);
	    if (table)
	    {
		glong colcnt, rowno;
		colcnt = AccessibleTable_getNColumns (table);
		rowno = AccessibleTable_getRowAtIndex (table,
		          Accessible_getIndexInParent (acc));
		if (rowno >= 0 && colcnt > 0)
		{
		    /*single column table: present the single cell in the line*/
		    if (colcnt == 1)
		    {
			child = AccessibleTable_getAccessibleAt (table, rowno, 0);
			if (child)
			{
			    glong childcnt = 0;
			    childcnt = Accessible_getChildCount (child);
			    /*cell has no children*/
			    if (childcnt == 0)
			    {
				gchar *tmp = NULL;
				tmp = Accessible_getName (child);
				if (tmp && tmp[0])
				    name_ = g_strdup (tmp);
				SPI_freeString (tmp);         
			    }	
			    /*there is a container cell*/
			    else
			    {
				for (i = 0; i < childcnt; i++ )
				{
				    Accessible *cell_child = NULL;
				    cell_child = Accessible_getChildAtIndex (child, i);
				    if (cell_child)
				    {
					gchar* tmp = NULL;
					tmp = Accessible_getName (cell_child);
					if (tmp && tmp[0])
					    name_ = g_strconcat (name_, " ", tmp, NULL);
					SPI_freeString (tmp);     
					Accessible_unref (cell_child);
				    }
				}
			    }	
			}    
			Accessible_unref (child);
		    }
		    /*the whole row is selected*/
		    else if (AccessibleTable_isRowSelected (table, rowno))
		    {
			glong focus_index = -1;
			for (i = 0; i < colcnt; i++)
			{
			    child = AccessibleTable_getAccessibleAt (table, rowno, i);
			    if (child)
			    {
				AccessibleStateSet *states;
				states = Accessible_getStateSet (child); 
				if (AccessibleStateSet_contains (states, SPI_STATE_FOCUSED))
				{
				    focus_index = i;
				    Accessible_unref (child);
				    break;	
				}
    				AccessibleStateSet_unref (states);
			    }
			    Accessible_unref (child);
			}
			
			/* Three cells (if exist) will be presented:the currently 
			   focused cell, and the cells before/after this cell */
			if (focus_index >= 0)
			{   
			    for (i = focus_index - 1; i <= focus_index + 1; i++)
			    {
				if (i >= 0)
				{
				    child = AccessibleTable_getAccessibleAt (table, rowno, i);
				    if (child)
				    {
					glong childcnt = 0;
					childcnt = Accessible_getChildCount (child);
					/*cell has no children*/
					if (childcnt == 0)
					{
					    gchar *tmp = NULL;
					    tmp = Accessible_getName (child);
					    if (tmp && tmp[0])
						name_ = g_strconcat (name_, " ", tmp, NULL);
					    SPI_freeString (tmp);	
					}
					/*there is a container cell*/
					else
					{
					    gint j;
					    for (j = 0; j < childcnt; j++)
					    {
						Accessible *cell_child = NULL;
						cell_child = Accessible_getChildAtIndex (child, j);
						if (cell_child)
						{
						    gchar *tmp2 = NULL;
						    tmp2 = Accessible_getName (cell_child);
						    if (tmp2 && tmp2[0])
							name_ = g_strconcat (name_, " ", tmp2, NULL);
						    SPI_freeString (tmp2);
						    Accessible_unref (cell_child);	
						}
					    }
					}
				    }    
				    Accessible_unref (child);
				}    
			    }
			}	    
		    }
		    /* Partial row is selected*/
		    else
		    {
			for (i = 0; i < colcnt; i++)
			{
			    child = AccessibleTable_getAccessibleAt (table, rowno, i);
			    if (child)
			    {
				AccessibleStateSet *states;
				states = Accessible_getStateSet (child); 
				if (AccessibleStateSet_contains (states, SPI_STATE_FOCUSED))
				{
				    gchar *tmp;
				    tmp = Accessible_getName (child);
				    if (tmp && tmp[0])
					name_ = SR_strdup (tmp);
				    SPI_freeString (tmp);
				    Accessible_unref (child);
				    break;	
				}
    				AccessibleStateSet_unref (states);
			    }
			    Accessible_unref (child);
    			}
		    }
		}	  
	    }	
	    if (table)
		AccessibleTable_unref (table);
	    if (parent)
		Accessible_unref (parent);
	}
	break;
    }
    		
    if (name_ && name_[0])
	*cell = name_;
	
    return *cell ? TRUE : FALSE;		
}	

/**
 * sro_get_description:
 * @obj: a pointer to the #SRObject structure to query.
 * @description: a pointer to a pointer to char.
 *
 * Get a pointer to the description for @obj.
 *
 * Return: #TRUE if successful, #FALSE otherwise.
 *
**/
static gboolean 
sro_default_get_description (const SRObject *obj, 
			     char **description,
			     SRLong index)
{
    gchar *description_ = NULL;
    Accessible *acc = NULL;
    SRObjectRoles role;
    gchar *tmp, *name;
    
    if (description)
	*description = NULL;

    srl_return_val_if_fail (obj && description, FALSE);
    
    acc = sro_get_acc_at_index (obj, index);

    sro_get_role (obj, &role, index);

    
    tmp = Accessible_getDescription (acc);
    name = Accessible_getName (acc);
    
    if (!(name && name[0]))
	return FALSE;
    
    if (tmp && tmp[0])
    {
	if ((name && name[0]) && g_strcasecmp (name, tmp) == 0)
	    return FALSE;
        description_ = SR_strdup (tmp);
    }
    SPI_freeString (name);
    SPI_freeString (tmp);
	
    *description = description_;
    return description_ ? TRUE : FALSE;
}

static gboolean
get_layer_from_acc (Accessible *acc,  
		    SRObjectLayer *layer)
{
    Accessible *comp;
    
    srl_return_val_if_fail (acc && Accessible_isComponent (acc), FALSE);
    
    comp = Accessible_getComponent (acc);
    if (!comp)
	return FALSE;
	    
    *layer = (SRObjectLayer)AccessibleComponent_getLayer  (comp);
    AccessibleComponent_unref (comp);
    
    return TRUE;
}

/**
 * sro_get_default_layer:
 * @obj: a pointer to the #SRObject structure to query.
 * @description: a pointer to SRObjectLayer.
 *
 * Get a pointer to the layer for @obj.
 *
 * Return: #TRUE if successful, #FALSE otherwise.
 *
**/
static gboolean 
sro_default_get_layer (const SRObject *obj, 
		       SRObjectLayer *layer,
		       SRLong index)
{
    gboolean rv = FALSE;
    Accessible *child;	    

    srl_return_val_if_fail (obj && layer, FALSE);
    srl_return_val_if_fail (sro_is_component (obj, index), FALSE);
/*
    if (index == SR_INDEX_CONTAINER)
    {
	switch (obj->role)
	{
	    case SR_ROLE_STATUS_BAR:
	    case SR_ROLE_TABLE_LINE:
	    case SR_ROLE_TABLE_COLUMNS_HEADER:
		if (obj->children)
		    rv = get_layer_from_array_of_acc (obj->children, layer);
		break;
	    default:
		if (obj->acc)
		    rv = get_layer_from_acc (obj->acc, layer);
		break;	    
	}
    }
    else
    {
	Accessible *child;	    
*/    
        child = sro_get_acc_at_index (obj, index);
	if (child)
	    rv = get_layer_from_acc (child, layer);
/*    }*/
    return rv;
}


static gboolean
get_MDIZOrder_from_acc (Accessible *acc,  
			short *MDIZOrder)
{
    Accessible *comp;
    
    srl_return_val_if_fail (acc && Accessible_isComponent (acc), FALSE);
    
    comp = Accessible_getComponent (acc);
    if (!comp)
	return FALSE;
	    
    *MDIZOrder = AccessibleComponent_getMDIZOrder  (comp);
    AccessibleComponent_unref (comp);
    
    return TRUE;
}


/**
 * sro_get_default_MDIZOrder:
 * @obj: a pointer to the #SRObject structure to query.
 * @description: a pointer to short.
 *
 * Get a pointer to the MDIZorder for @obj.
 *
 * Return: #TRUE if successful, #FALSE otherwise.
 *
**/
static gboolean 
sro_default_get_MDIZOrder (const SRObject *obj, 
		    	   short *MDIZOrder,
		    	   SRLong index)
{
    gboolean rv = FALSE;
    Accessible *child;	    
    
    srl_return_val_if_fail (obj && MDIZOrder, FALSE);
    srl_return_val_if_fail (sro_is_component (obj, index), FALSE);
/*
    if (index == SR_INDEX_CONTAINER)
    {

	switch (obj->role)
	{
	    case SR_ROLE_STATUS_BAR:
	    case SR_ROLE_TABLE_LINE:
	    case SR_ROLE_TABLE_COLUMNS_HEADER:
		if (obj->children)
		    rv = get_MDIZOrder_from_array_of_acc (obj->children, MDIZOrder);
		break;
	    default:
		if (obj->acc)
		    rv = get_MDIZOrder_from_acc (obj->acc, MDIZOrder);
		break;	    
	}
    }
    else
    {
	Accessible *child;	    
*/
        child = sro_get_acc_at_index (obj, index);
	if (child)
	    rv = get_MDIZOrder_from_acc (child, MDIZOrder);
/*    }*/
    return rv;
}


static struct
{
    AccessibleRole  acc_role;
    SRObjectRoles   sr_role;
}acc_sr_role[] =
{
    {	SPI_ROLE_INVALID, 		SR_ROLE_INVALID 		},
    {	SPI_ROLE_ACCEL_LABEL,		SR_ROLE_ACCELERATOR_LABEL 	},
    {	SPI_ROLE_ALERT,			SR_ROLE_ALERT			},
    {	SPI_ROLE_ANIMATION, 		SR_ROLE_ANIMATION		},
    {	SPI_ROLE_ARROW,			SR_ROLE_ARROW 			},
    {	SPI_ROLE_CALENDAR,		SR_ROLE_CALENDAR 		},
    {	SPI_ROLE_CANVAS,		SR_ROLE_CANVAS 			},
    {	SPI_ROLE_CHECK_MENU_ITEM,	SR_ROLE_CHECK_MENU_ITEM 		},
    {	SPI_ROLE_CHECK_BOX,		SR_ROLE_CHECK_BOX 		},
    {	SPI_ROLE_COLOR_CHOOSER,		SR_ROLE_COLOR_CHOOSER 		},
    {	SPI_ROLE_COLUMN_HEADER,		SR_ROLE_COLUMN_HEADER 		},
    {	SPI_ROLE_COMBO_BOX,		SR_ROLE_COMBO_BOX 		},
    {   SPI_ROLE_DATE_EDITOR,		SR_ROLE_DATE_EDITOR 		},
    {   SPI_ROLE_DESKTOP_ICON,		SR_ROLE_DESKTOP_ICON 		},
    {   SPI_ROLE_DESKTOP_FRAME,		SR_ROLE_DESKTOP_FRAME 		},
    {   SPI_ROLE_DIAL,			SR_ROLE_DIAL 			},
    {   SPI_ROLE_DIALOG,		SR_ROLE_DIALOG 			},
    {	SPI_ROLE_DIRECTORY_PANE,	SR_ROLE_DIRECTORY_PANE		},
    {   SPI_ROLE_FILE_CHOOSER,		SR_ROLE_FILE_CHOOSER 		},
    {   SPI_ROLE_FILLER,		SR_ROLE_FILLER 			},
    {   SPI_ROLE_FONT_CHOOSER,		SR_ROLE_FONT_CHOOSER 		},
    {   SPI_ROLE_FRAME,			SR_ROLE_FRAME 			},
    {   SPI_ROLE_GLASS_PANE,	        SR_ROLE_GLASS_PANE 		},
    {   SPI_ROLE_HTML_CONTAINER,     	SR_ROLE_HTML_CONTAINER 		},
    { 	SPI_ROLE_ICON,			SR_ROLE_ICON 			},
    {	SPI_ROLE_IMAGE,			SR_ROLE_IMAGE 			},
    {	SPI_ROLE_INTERNAL_FRAME,	SR_ROLE_INTERNAL_FRAME 		},
    {   SPI_ROLE_LABEL,			SR_ROLE_LABEL 			},
    {	SPI_ROLE_LAYERED_PANE,		SR_ROLE_LAYERED_PANE 		},
    {	SPI_ROLE_LIST,			SR_ROLE_LIST 			},
    {	SPI_ROLE_LIST_ITEM,		SR_ROLE_LIST_ITEM 		},
    {	SPI_ROLE_MENU,			SR_ROLE_MENU 			},
    {   SPI_ROLE_MENU_BAR,		SR_ROLE_MENU_BAR 		},
    { 	SPI_ROLE_MENU_ITEM,     	SR_ROLE_MENU_ITEM 		},
    { 	SPI_ROLE_OPTION_PANE,		SR_ROLE_OPTION_PANE 		},
    { 	SPI_ROLE_PAGE_TAB,		SR_ROLE_PAGE_TAB 		},
    {	SPI_ROLE_PAGE_TAB_LIST,     	SR_ROLE_PAGE_TAB_LIST 		},
    {	SPI_ROLE_PANEL,		    	SR_ROLE_PANEL 			},
    { 	SPI_ROLE_PASSWORD_TEXT,     	SR_ROLE_PASSWORD_TEXT 		},
    { 	SPI_ROLE_POPUP_MENU,     	SR_ROLE_POPUP_MENU 		},
    { 	SPI_ROLE_PROGRESS_BAR,     	SR_ROLE_PROGRESS_BAR 		},
    {	SPI_ROLE_PUSH_BUTTON,    	SR_ROLE_PUSH_BUTTON 		},
    { 	SPI_ROLE_RADIO_BUTTON,     	SR_ROLE_RADIO_BUTTON 		},
    {	SPI_ROLE_RADIO_MENU_ITEM,     	SR_ROLE_RADIO_MENU_ITEM	 	},
    { 	SPI_ROLE_ROOT_PANE,     	SR_ROLE_ROOT_PANE 		},
    { 	SPI_ROLE_ROW_HEADER,     	SR_ROLE_ROW_HEADER 		},
    { 	SPI_ROLE_SCROLL_BAR,     	SR_ROLE_SCROLL_BAR 		},
    { 	SPI_ROLE_SCROLL_PANE,     	SR_ROLE_SCROLL_PANE 		},
    { 	SPI_ROLE_SEPARATOR,     	SR_ROLE_SEPARATOR 		},
    { 	SPI_ROLE_SLIDER,     		SR_ROLE_SLIDER 			},
    { 	SPI_ROLE_SPIN_BUTTON,     	SR_ROLE_SPIN_BUTTON 		},
    { 	SPI_ROLE_SPLIT_PANE,     	SR_ROLE_SPLIT_PANE 		},
    { 	SPI_ROLE_STATUS_BAR,     	SR_ROLE_STATUS_BAR 		},  
    { 	SPI_ROLE_TABLE,     		SR_ROLE_TABLE 			},
    { 	SPI_ROLE_TABLE_CELL,     	SR_ROLE_TABLE_CELL 		},
    { 	SPI_ROLE_TABLE_COLUMN_HEADER,	SR_ROLE_TABLE_COLUMN_HEADER 	},
    { 	SPI_ROLE_TABLE_ROW_HEADER,	SR_ROLE_TABLE_ROW_HEADER 	},
    { 	SPI_ROLE_TEAROFF_MENU_ITEM,	SR_ROLE_MENU_ITEM	 	},
    { 	SPI_ROLE_TERMINAL,		SR_ROLE_TERMINAL	 	},
    {	SPI_ROLE_TEXT,			SR_ROLE_TEXT_ML			},
    { 	SPI_ROLE_TOGGLE_BUTTON,		SR_ROLE_TOGGLE_BUTTON 		},
    { 	SPI_ROLE_TOOL_BAR,		SR_ROLE_TOOL_BAR 		},
    { 	SPI_ROLE_TOOL_TIP,		SR_ROLE_TOOL_TIP 		},
    { 	SPI_ROLE_TREE,			SR_ROLE_TREE 			},
    { 	SPI_ROLE_TREE_TABLE,		SR_ROLE_TREE_TABLE		},
    { 	SPI_ROLE_UNKNOWN,     		SR_ROLE_UNKNOWN 		},
    {	SPI_ROLE_VIEWPORT,	     	SR_ROLE_VIEWPORT 		},
    { 	SPI_ROLE_WINDOW,		SR_ROLE_WINDOW 			},
    {	SPI_ROLE_EDITBAR,		SR_ROLE_EDITBAR			},
    {	SPI_ROLE_EXTENDED,		SR_ROLE_EXTENDED		},
};

gint
sr_acc_get_link_index (Accessible *acc)
{
    AccessibleHypertext *hyper;
    gint rv = -1;
		
    hyper = Accessible_getHypertext (acc);
    if (hyper)
    {
	gint cnt = AccessibleHypertext_getNLinks (hyper);
	if (cnt > 0)
	{
	    AccessibleText *text = Accessible_getText (acc);
	    if (text)
	    {
	        long cp = AccessibleText_getCaretOffset (text);
	        rv = AccessibleHypertext_getLinkIndex (hyper, cp);
		AccessibleText_unref (text);
	    }
	}
	AccessibleHypertext_unref (hyper);
    }

    return rv;
}

static SRObjectRoles
get_role_from_acc (Accessible *acc, 
		    SRObjectType type)
{
    AccessibleRole role_;
    Accessible *parent;
    SRObjectRoles role = SR_ROLE_UNKNOWN;
    
    srl_return_val_if_fail (acc, SR_ROLE_UNKNOWN);
    role_ = Accessible_getRole (acc);
    
    parent = Accessible_getParent (acc);
    if (parent)
    {
	if (Accessible_isTable (parent) && 
	    role_ != SPI_ROLE_TABLE_COLUMN_HEADER &&
	    role_ != SPI_ROLE_COLUMN_HEADER)
	{
	    AccessibleTable *table = Accessible_getTable (acc);
	    if (table)
	    {
		long index = Accessible_getIndexInParent (acc);
		if (AccessibleTable_getRowAtIndex (table, index) >= 0 &&
			AccessibleTable_getColumnAtIndex (table, index) >= 0)
		    role_ = SPI_ROLE_TABLE_CELL;
		AccessibleTable_unref (table);
	    }
	}
	Accessible_unref (parent);
    }
    switch (role_)
    {
	case SPI_ROLE_TABLE_CELL:
	    if (type == SR_OBJ_TYPE_VISUAL)
	    {
		int i;
		AccessibleRole role_acc;
		role_acc = Accessible_getRole (acc);
		for (i = 0; i <= G_N_ELEMENTS (acc_sr_role); i++) 
		{
    		    if (acc_sr_role[i].acc_role == role_acc)
    		    {
			role = acc_sr_role[i].sr_role;
			break;
		    }
		}
	    }
	    else if (type == SR_OBJ_TYPE_PROCESSED)
		role = SR_ROLE_TABLE_LINE;
	    else
		srl_assert_not_reached ();
	    break;
	case SPI_ROLE_TABLE_COLUMN_HEADER:
	case SPI_ROLE_COLUMN_HEADER:
	    if (type == SR_OBJ_TYPE_VISUAL)
	    	role = SR_ROLE_TABLE_COLUMN_HEADER;
	    else if (type == SR_OBJ_TYPE_PROCESSED)
		role = SR_ROLE_TABLE_COLUMNS_HEADER;
	    else
		srl_assert_not_reached ();	
	    break;
/*FIXME: for tree item objects */
	case SPI_ROLE_LABEL:
	    {
		Accessible *parent;
		parent = acc;
		Accessible_ref (parent);
		while (parent && Accessible_getRole (parent) != SPI_ROLE_TREE)
		{
		    Accessible *tmp = Accessible_getParent (parent);
		    Accessible_unref (parent);
		    parent = tmp;
		}
		if (parent)
		{
		    Accessible_unref (parent);
		    role = SR_ROLE_TREE_ITEM;
		}
		else
		{
		    role = SR_ROLE_LABEL;
		}
	    }
	    break;
/*FIXME: end */
	case SPI_ROLE_TEXT:
	    {
		role = SR_ROLE_TEXT_ML;
		if (sr_acc_get_link_index (acc) >= 0)
		    role = SR_ROLE_LINK;
		else
		{
		    AccessibleStateSet *state = Accessible_getStateSet (acc);
		    if (state)
		    {
			if (AccessibleStateSet_contains (state, SPI_STATE_SINGLE_LINE))
			    role = SR_ROLE_TEXT_SL;
			AccessibleStateSet_unref (state);
		    }
		}
	    }
	    break;
	default:
	    {
	        int i;
		AccessibleRole role_acc;
		role_acc = Accessible_getRole (acc);
		for (i = 0; i <= G_N_ELEMENTS (acc_sr_role); i++) 
		{
    		    if (acc_sr_role[i].acc_role == role_acc)
    		    {
			role = acc_sr_role[i].sr_role;
			break;
		    }
		}
	    }
	    break;
    }
    srl_debug (role != SR_ROLE_UNKNOWN);
    return role;
}

/**
 * sro_get_role:
 * @obj: a pointer to the #SRObject structure to query.
 * @role: pointer to the #SRObjectRole.
 *
 * Get the #SRObjectRole for @obj.
 *
 * Return: #TRUE if successful, #FALSE otherwise.
 *
**/
static gboolean 
sro_default_get_role (const SRObject *obj, 
	    	      SRObjectRoles *role,
	    	      SRLong index)
{    
    if (role)
	*role = SR_ROLE_UNKNOWN;
    srl_return_val_if_fail (obj && role, FALSE);
    
    if (index == SR_INDEX_CONTAINER)
    {
	*role = obj->role;
    }
    else
    {
	Accessible *child;
	child = sro_get_acc_at_index (obj, index); 
	if (child)
	    *role = get_role_from_acc (child, SR_OBJ_TYPE_VISUAL);
    }
    
    return TRUE;	
}


static void
get_sro_role (SRObject *obj, 
	      SRObjectType type)
{
    Accessible *acc;
    srl_return_if_fail (obj);
    srl_debug (obj->role == SR_ROLE_UNKNOWN);
    
    acc = sro_get_acc_at_index (obj, SR_INDEX_CONTAINER);
        
    if (acc)
	obj->role = get_role_from_acc (acc, type);
        
    srl_debug (obj->role != SR_ROLE_UNKNOWN);
}




static SRSpecialization
get_specialization_from_acc_real (Accessible *acc)
{
    SRSpecialization specialization;

    srl_return_val_if_fail (acc, SR_IS_NOTHING);
    
    specialization = SR_IS_NOTHING;
    
    if (Accessible_isAction (acc))
    	specialization |= SR_IS_ACTION;
    if (Accessible_isComponent (acc))
    	specialization |= SR_IS_COMPONENT;    
    if (Accessible_isEditableText (acc))
    	specialization |= SR_IS_EDITABLE_TEXT;
    if (Accessible_isHypertext (acc))
	specialization |= SR_IS_HYPERTEXT;
    if (Accessible_isImage (acc))
    {
	AccessibleImage *image;
	long x, y;
	image = Accessible_getImage (acc);
	if (image)
	{
	    AccessibleImage_getImagePosition (image, &x, &y, SPI_COORD_TYPE_SCREEN);
	    if ((x != G_MINLONG) && (y != G_MINLONG))
		    specialization |= SR_IS_IMAGE;
	    AccessibleImage_unref (image);
	}
    }
    if (Accessible_isSelection (acc))
    	specialization |= SR_IS_SELECTION;
    if (Accessible_isTable (acc))
    	specialization |= SR_IS_TABLE;
    if (Accessible_isText (acc))
    	specialization |= SR_IS_TEXT;
    if (Accessible_isValue (acc))
    	specialization |= SR_IS_VALUE;

    return specialization;
}

static SRSpecialization
get_specialization_from_acc (Accessible *acc)
{
    SRSpecialization specialization;
    srl_return_val_if_fail (acc, SR_IS_NOTHING);
    specialization = SR_IS_NOTHING;
    if (Accessible_getRole (acc) == SPI_ROLE_TABLE_CELL)
    {
    	if (Accessible_getChildCount (acc) == 2)
	{
	    Accessible *child1, *child2;
	    SRSpecialization s1, s2;
	    /* the cell which has two children must be only COMPONENT */
	    srl_debug ((specialization & ~SR_IS_COMPONENT) == 0);
	    child1 = Accessible_getChildAtIndex (acc, 0);
	    child2 = Accessible_getChildAtIndex (acc, 1);
	    s1 = s2 = SR_IS_NOTHING;
	    if (child1)
		s1 = get_specialization_from_acc_real (child1);
	    if (child2)
		s2 = get_specialization_from_acc_real (child2);
	    /* the specialization of both children must be different, except COMPONENT */
	    srl_debug (((s1 ^ SR_IS_COMPONENT) & (s2 ^ SR_IS_COMPONENT)) == 0);
	    specialization |= s1;
	    specialization |= s2;
	    if (child1)
		Accessible_unref (child1);
	    if (child2)
		Accessible_unref (child2);
	} 
	else
	{
	    specialization = get_specialization_from_acc_real (acc);
	}
    }
    else
    {
	specialization = get_specialization_from_acc_real (acc);
    }
    srl_debug (specialization != SR_IS_NOTHING);
    return specialization;
}


static gboolean 
get_acc_child_with_role_from_acc (Accessible *acc, GArray **array, 
		       AccessibleRole role, gint level, gboolean stop);

static gboolean
srl_acc_manages_descendants (Accessible *acc);

static gboolean
srl_table_is_on_screen_cell_at (AccessibleTable *table,
				gint row,
				gint col)
{
    Accessible *child;
    gboolean rv = FALSE;

    srl_assert (table);

    child = AccessibleTable_getAccessibleAt (table, row, col);
    if (child)
    {
	AccessibleStateSet *state;
	state = Accessible_getStateSet (child);
	if (state)
	{
	    rv = AccessibleStateSet_contains (state, SPI_STATE_VISIBLE) &&
		    AccessibleStateSet_contains (state, SPI_STATE_SHOWING);
	    AccessibleStateSet_unref (state);
	}
	Accessible_unref (child);
    }

    return rv;
}

static gboolean
srl_table_get_visible_range_from_cell (Accessible *cell,
					GArray *children)
{
    Accessible *parent;
    AccessibleTable *table;
    gint start, end, row, col, index, cnt, i;

    srl_assert (cell && children);

    parent = Accessible_getParent (cell);
    srl_return_val_if_fail (parent, FALSE);

    table = Accessible_getTable (parent);
    index = Accessible_getIndexInParent (cell);
    row = AccessibleTable_getRowAtIndex (table, index);
    srl_return_val_if_fail (row >= 0, FALSE);
    col = AccessibleTable_getColumnAtIndex (table, index);
    start = end = col;
    
    for (start--; start >= 0; start--)
    {
	if (!srl_table_is_on_screen_cell_at (table, row, start))
	    break;
    }
    start++;
    start = MAX (0, start);
    
    cnt = AccessibleTable_getNColumns (table);
    for (end++; end < cnt; end++)
    {
    	if (!srl_table_is_on_screen_cell_at (table, row, end))
	    break;
    }
    end = MIN (end, cnt);

    for (i = start; i < end; i++)
    {
	Accessible *child;
	child = AccessibleTable_getAccessibleAt (table, row, i);
	g_array_append_val (children, child);
    }

    Accessible_unref (parent);
    AccessibleTable_unref (table);

    return TRUE;
}


static gboolean
get_sro_children (SRObject *obj)
{
    srl_return_val_if_fail (obj && obj->acc, FALSE);
    srl_debug (obj->children == NULL);

    if (srl_acc_manages_descendants (obj->acc))
    {
        obj->manages_descendants = TRUE;
        return FALSE;
    }
    
    switch (obj->role)
    {
    	case SR_ROLE_TABLE:
	case SR_ROLE_TREE_TABLE:
	    {		
		AccessibleTable *table;
		table = Accessible_getTable (obj->acc);
		if (table)
		{
		    Accessible *header;
		    int i;
		    long n_child;
	
		    n_child = AccessibleTable_getNRows (table);
		    header = AccessibleTable_getColumnHeader (table, 0);
		    if (header)
			n_child++;
		    obj->children = g_array_sized_new (FALSE, FALSE, sizeof (Accessible*), n_child);
		    if (header)
			g_array_append_val (obj->children, header);
		
		    for (i = 0; i < AccessibleTable_getNRows (table); i++)
		    {
			Accessible *row;
			row = AccessibleTable_getAccessibleAt (table, i, 0);
			if (row)
			    g_array_append_val (obj->children, row);
		    }
		    AccessibleTable_unref (table);
		}
	    }
	    break;
	case SR_ROLE_TABLE_LINE:
	    {
	    	obj->children = g_array_sized_new (FALSE, FALSE, sizeof (Accessible*), 1);
		srl_table_get_visible_range_from_cell (obj->acc, obj->children);
	    }
	    break;
	case SR_ROLE_TABLE_COLUMNS_HEADER:
	    {
		int i;
		Accessible *parent = NULL;
		AccessibleTable *table = NULL;
		parent = Accessible_getParent (obj->acc);
		if (parent && Accessible_isTable (parent))
		    table = Accessible_getTable (parent);
		if (table)
		{
		    long n_child;
		    n_child = AccessibleTable_getNColumns (table);
		    obj->children = g_array_sized_new (FALSE, FALSE, sizeof (Accessible*), n_child);
		    for (i = 0; i < n_child; i++)
		    {
			Accessible *header;
			header = AccessibleTable_getColumnHeader (table, i);
			if (header)
			    g_array_append_val (obj->children, header);
		    }
		}
		if (table)
		    AccessibleTable_unref (table);
		if (parent)
		    Accessible_unref (parent);    
	    }
	    break;
	case SR_ROLE_TOOL_BAR:
	    {
		long cc, i;
		cc = Accessible_getChildCount (obj->acc);
		obj->children = g_array_sized_new (FALSE, FALSE, sizeof (Accessible*), cc);
		/* Assumption: We assume that ROLE_TOOL_BAR never 
		   applies to a MANAGES_DESCENDANTS object */
		for (i = 0; i < cc; i++)
		{
		    Accessible *child;
		    child = Accessible_getChildAtIndex (obj->acc, i);
		    if (child)
		    {
		        GArray *tmp;
			int j;
		
			tmp = g_array_sized_new (FALSE, FALSE, sizeof (Accessible*), 1);
			get_acc_child_with_role_from_acc (child, &tmp, SPI_ROLE_PUSH_BUTTON, -1, FALSE);
			for (j = 0; j < tmp->len; j++)
			{
			    Accessible *button;
			    button = g_array_index (tmp, Accessible *, j);
			    if ((get_state_from_acc (button) & SR_STATE_VISIBLE) == SR_STATE_VISIBLE)
				g_array_append_val (obj->children, button);
			    else
				Accessible_unref (button);
			}
			g_array_free (tmp, TRUE);
			Accessible_unref (child);
		    }
		}
	    }
	    break;
	case SR_ROLE_MENU_BAR:
	case SR_ROLE_MENU:
	    {
		long cc, i;
		cc = Accessible_getChildCount (obj->acc);
		/* Assumption: for menus, MANAGES_DESCENDANTS == false */
		obj->children = g_array_sized_new (FALSE, FALSE, sizeof (Accessible*), cc);
		for (i = 0; i < cc; i++)
		{
		    Accessible *child;
		    child = Accessible_getChildAtIndex (obj->acc, i);
		    if (child)
		    {
			if ((get_state_from_acc (child) & SR_STATE_VISIBLE) == SR_STATE_VISIBLE
				    && Accessible_getRole (child) != SPI_ROLE_SEPARATOR)
			    g_array_append_val (obj->children, child);
			else
			    Accessible_unref (child);
		    }
		}
	    }
	    break;
	default:
	    obj->children = g_array_sized_new (FALSE, FALSE, sizeof (Accessible*), 1);
	    g_array_append_val (obj->children, obj->acc);
	    Accessible_ref (obj->acc);
	    break;
    }
    srl_debug (obj->children);
    return TRUE;
}



/**
 * get_sro_from accessible:
 * @obj: returned #SRObject structure filled with nedded information.
 * @acc: pointer to the #Accessible object.
 *
 * Returns: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean
sro_get_from_accessible (Accessible *acc,
			 SRObject **obj,
			 SRObjectType type)
{
    if (obj)
	*obj = NULL;

    srl_return_val_if_fail (obj && acc, FALSE);

    srl_debug (Accessible_getRole (acc) != SPI_ROLE_UNKNOWN);

    /*FIXME: next lines must be valid?????*/    
/*    if (Accessible_getRole (acc) == SPI_ROLE_UNKNOWN)
	return FALSE;
*/
    *obj = sro_new ();
    if (!*obj)
	return FALSE;
    (*obj)->acc = acc;
    Accessible_ref (acc);
    
    get_sro_role (*obj, type);
    get_sro_children (*obj);

    return TRUE;
}

gboolean 
sro_get_from_accessible_event (Accessible *acc,
				gchar *event, 
				SRObject **obj)
{ 
    gboolean rv;
    if (obj)
	*obj = NULL;
    srl_return_val_if_fail (obj && acc && event, FALSE);

    rv = sro_get_from_accessible (acc, obj, SR_OBJ_TYPE_PROCESSED);
    if (rv)
	(*obj)->reason = g_strdup (event);
    return rv;
}





/**
 * sro_get_parent:
 * @obj: a pointer to the #SRObject structure to query.
 * @parent: returned parent of @obj.
 *
 * Get a pointer to parent #SRObject stucture for @obj.
 *
 * Return: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean 
sro_default_get_parent (const SRObject *obj, 
			SRObject **parent)
{
    gboolean rv = FALSE;
    if (parent)
	*parent = NULL;        
    srl_return_val_if_fail (obj && parent, FALSE);

    switch (obj->role)
    {
	case SR_ROLE_TABLE_CELL:
	case SR_ROLE_COLUMN_HEADER:
	    rv = sro_get_from_accessible (obj->acc, parent, SR_OBJ_TYPE_PROCESSED);
	    break;
	default:
	    {
		Accessible *acc_parent;
		acc_parent = Accessible_getParent (obj->acc);
		if (acc_parent)
		{
    		    rv = sro_get_from_accessible (acc_parent, parent, SR_OBJ_TYPE_VISUAL);	    
		    Accessible_unref (acc_parent);
		}
	    }
	    break;
    }
    return rv;	
}

/**
 * sro_get_index_in_parent:
 * @obj: a pointer to the #SRObject structure to query.
 * @index: a pointer to #guint32.
 *
 * Get index in parent for @obj.
 *
 * Return: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean 
sro_default_get_index_in_parent (const SRObject *obj,
				 guint32 *index)
{
    if (index)
	*index = -1;
    srl_return_val_if_fail (obj && index, FALSE);
    
    switch (obj->role)
    {
	case SR_ROLE_TABLE_LINE:
	    {
		Accessible *parent;
		AccessibleTable *table = NULL;
		parent = Accessible_getParent (obj->acc);
		if (parent && Accessible_isTable (parent))
		    table = Accessible_getTable (parent);
		
		if (table)
		{
		    Accessible *header;
		    *index= AccessibleTable_getRowAtIndex (table, Accessible_getIndexInParent (obj->acc));
		    header = AccessibleTable_getColumnHeader (table, 0);
		    if (header)
		    {
			(*index)++;
			Accessible_unref (header);
		    }
		}
		if (table)		
		    AccessibleTable_unref (table);
		if (parent)
		    Accessible_unref (parent);
	    }
	    break;
	case SR_ROLE_TABLE_COLUMNS_HEADER:
	    *index = 0;
	    break;
	default:
	    *index = Accessible_getIndexInParent (obj->acc);
	    break;
    }
    
    return TRUE;	
}

/**
 * sro_get_children_count:
 * @obj: a pointer to the #SRObject structure to query.
 * @count: a pointer to #guint32.
 *
 * Get the number of children for @obj.
 *
 * Return: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean 
sro_default_get_children_count (const SRObject *obj,
				guint32 *count)
{
    if (count)
	*count = -1;
    srl_return_val_if_fail (obj && count, FALSE);	        
    if (obj->children)  
	*count = obj->children->len;
    else
    	*count = Accessible_getChildCount (obj->acc);
	
    return TRUE;	
}

/**
 * sro_get_name:
 * @obj: a pointer to the #SRObject structure to query.
 * @child: a pointer to a pointer to the #SRObject.
 * @index: a #long indicating which children is specified.
 *
 * Get a #SRObject for the index-nt child of @obj.
 *
 * Return: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean 
sro_default_get_i_child (const SRObject *obj, 
			 long index,
			 SRObject **child)
{
    gboolean rv = FALSE;
    Accessible *acc_child = NULL;
    if (child)
	*child = NULL;
    srl_return_val_if_fail (obj && child, FALSE);
    srl_return_val_if_fail (index >= 0, FALSE);
    
    acc_child = sro_get_acc_at_index (obj, index);
    
    switch (obj->role)
    {
	case SR_ROLE_TABLE:
	case SR_ROLE_TREE_TABLE:
	    rv = sro_get_from_accessible (acc_child, child, SR_OBJ_TYPE_PROCESSED);
	    break;
	default:
	    rv = sro_get_from_accessible (acc_child, child, SR_OBJ_TYPE_VISUAL);
	    break;
    }
    return rv;	
}


/**
 * sro_get_index_in_parent:
 * @obj: a pointer to the #SRObject structure to query.
 * @index: a pointer to #guint32.
 *
 * Get index in parent for @obj.
 *
 * Return: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean 
sro_get_index_in_parent (const SRObject *obj,
			 guint32 *index)
{
  SRObjectClass *klass;

  srl_return_val_if_fail (SR_IS_OBJECT (obj), FALSE);

  klass = SR_OBJECT_GET_CLASS (obj);
  if (klass->get_index_in_parent)
	  return (klass->get_index_in_parent) (obj, index);
  else
	  return FALSE;
}

/**
 * sro_get_children_count:
 * @obj: a pointer to the #SRObject structure to query.
 * @count: a pointer to #guint32.
 *
 * Get the number of children for @obj.
 *
 * Return: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean 
sro_get_children_count (const SRObject *obj,
			 guint32 *count)
{
  SRObjectClass *klass;

  srl_return_val_if_fail (SR_IS_OBJECT (obj), FALSE);

  klass = SR_OBJECT_GET_CLASS (obj);
  if (klass->get_children_count)
	  return (klass->get_children_count) (obj, count);
  else
	  return FALSE;
}

/**
 * sro_get_i_child:
 * @obj: a pointer to the #SRObject structure to query.
 * @child: a pointer to a pointer to the #SRObject.
 * @index: a #long indicating which children is specified.
 *
 * Get a #SRObject for the index-nt child of @obj.
 *
 * Return: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean 
sro_get_i_child (const SRObject *obj, 
		 long index,
		 SRObject **child)
{
  SRObjectClass *klass;

  srl_return_val_if_fail (SR_IS_OBJECT (obj), FALSE);

  klass = SR_OBJECT_GET_CLASS (obj);
  if (klass->get_i_child)
	  return (klass->get_i_child) (obj, index, child);
  else
	  return FALSE;
}



char *sr_role_name[] =
{
	"invalid",
	"alert",
	"canvas",
	"check-box",
	"color-chooser",
	"column-header",
	"combo-box",
    	"desktop-icon",
    	"desktop-frame",
    	"dialog",
    	"directory-pane",
    	"file-chooser",
    	"filler",
/*	"focus traversable",*/
    	"frame",
	"glass-pane",
    	"HTML-container",
	"icon",
    	"internal-frame",
    	"label",
    	"layered-pane",
    	"link",
	"list",
    	"list-item",
    	"menu",
    	"menu-bar",
    	"menu-item",
    	"check-menu-item",
    	"radio-menu-item",
    	"option-pane",
    	"page-tab",
    	"page-tab-list",
    	"panel",
    	"password-text",
    	"popup-menu",
    	"progress-bar",
    	"push-button",
    	"radio-button",
    	"root-pane",
    	"row-header",
    	"scroll-bar",
    	"scroll-pane",
    	"separator",
    	"slider",
    	"split-pane",
	"status-bar",
    	"table",
    	"table-cell",
    	"table-column-header",
    	"table-row-header",
/*	"tearoff-menu-item",*/
    	"multi-line-text",
	"single-line-text",
    	"toggle-button",
    	"tool-bar",
    	"tool-tip",
    	"tree",
	"tree-item",
	"tree-table",
    	"unknown",
    	"viewport",
    	"window",
	"accelerator-label",
	"animation",
	"arrow",
	"calendar",
	"date-editor",
    	"dial",
    	"drawing-area",
    	"font-chooser",
    	"image",
/*   	"radio menu item",*/
	"spin-button",
	"terminal",
	"extended",
	"table-line",
	"table-columns-header",
	"title-bar",
	"edit-bar"
};



/**
 * sro_get_role_name:
 * @obj: a pointer to a #SRObject structure to query.
 * @role_name: a pointer to a pointer to char.
 *
 * Get the name for @obj role.
 *
 * Returns: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean 
sro_get_role_name (const SRObject *obj, 
		   char **role_name,
		   SRLong index)
{    	
  SRObjectClass *klass;

  srl_return_val_if_fail (SR_IS_OBJECT (obj), FALSE);

  klass = SR_OBJECT_GET_CLASS (obj);
  if (klass->get_role_name)
	  return (klass->get_role_name) (obj, role_name, index);
  else
	  return FALSE;
}

/**
 * sro_get_role_name:
 * @obj: a pointer to a #SRObject structure to query.
 * @role_name: a pointer to a pointer to char.
 *
 * Get the name for @obj role.
 *
 * Returns: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean 
sro_default_get_role_name (const SRObject *obj, 
			   char **role_name,
			   SRLong index)
{    	
    SRObjectRoles role = SR_ROLE_UNKNOWN;
    int index_ = 0;
    if (role_name)
	*role_name = NULL;    
    srl_return_val_if_fail (obj && role_name, FALSE);
	
    sro_get_role(obj, &role, index);
    if (0 <= role && role < G_N_ELEMENTS (sr_role_name))
    {
	index_ = role;
    }
    
    if (role != SR_ROLE_UNKNOWN && role != SR_ROLE_EXTENDED)
	*role_name = SR_strdup (sr_role_name[index_]);
    else
    {
	gchar *tmp;
	Accessible *acc;
	acc = sro_get_acc_at_index (obj, index);
	tmp = Accessible_getRoleName (acc);
	if (tmp && tmp[0])
	    *role_name = SR_strdup (tmp);
	else
	    *role_name = SR_strdup ("unknown");
	SPI_freeString (tmp);
    }
    return *role_name ? TRUE : FALSE;
}

gboolean
sro_get_app_name (SRObject *obj,
                  gchar **name,
		  SRLong index)
{
    Accessible *acc = NULL;
    gchar *name_ = NULL;
    acc = sro_get_acc_at_index (obj, index);
    Accessible_ref (acc);
    srl_return_val_if_fail (acc, FALSE);
    
    while (acc && !Accessible_isApplication(acc))
    {
	Accessible *tmp = acc;
	acc = Accessible_getParent (acc);
	Accessible_unref (tmp);
    }
    name_ = Accessible_getName (acc);
    Accessible_unref (acc);
    
    *name = SR_strdup (name_);
    return *name ? TRUE : FALSE;
}		   

gboolean 
sro_get_window_name (SRObject *obj,
		     gchar **role,
		     gchar **name,
		     SRLong index)
{
    Accessible *acc = NULL;
    gchar *name_ = NULL, *role_ = NULL;
    acc = sro_get_acc_at_index (obj, index);
    Accessible_ref (acc);
    srl_return_val_if_fail (acc, FALSE);
    
    role_ = Accessible_getRoleName (acc);
    
    while (acc && strcmp (role_, "frame") && strcmp (role_, "dialog"))
    {
	Accessible *tmp = acc;
	acc = Accessible_getParent (acc);
	role_ = Accessible_getRoleName (acc);
	Accessible_unref (tmp);
    }
    
    if (strcmp (role_, "frame") && strcmp (role_, "dialog"))
	return FALSE;
    name_ = Accessible_getName (acc);
    Accessible_unref (acc);
    	
    *role = SR_strdup (role_);
    *name = SR_strdup (name_);
    
    return *role ? TRUE : FALSE;
}		     

/**
 * sro_get_location:
 * @obj: a pointer to a #SRObject structure to query.
 * @type: a #SRCoordinateType .
 * @location: pointer to the #SRLocation object.
 *
 * Get location for @obj in @type screen reader coordinate type.
 *
 * Returns: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean 
sro_get_location (SRObject *obj, 
		  SRCoordinateType type, 
		  SRRectangle *location,
		  SRLong index)
{
  SRObjectClass *klass;

  srl_return_val_if_fail (SR_IS_OBJECT (obj), FALSE);

  klass = SR_OBJECT_GET_CLASS (obj);
  if (klass->get_location)
	  return (klass->get_location) (obj, type, location, index);
  else
	  return FALSE;
}

static gboolean
get_location_from_acc (Accessible *acc, 
		       AccessibleCoordType type, 
		       SRRectangle *location)
{
    Accessible *comp;
    long int x, y, width, height;
    
    srl_return_val_if_fail (acc && Accessible_isComponent (acc), FALSE);
    srl_return_val_if_fail (location, FALSE);
    
    comp = Accessible_getComponent (acc);
    if (!comp)
	return FALSE;
	    
    AccessibleComponent_getExtents  (comp, &x, &y, &width, &height, type);
    AccessibleComponent_unref (comp);
    location->x = x;
    location->y = y;
    location->width  = width;
    location->height = height;
    return TRUE;
}

/**
 * sro_get_layer:
 * @obj: a pointer to a #SRObject structure to query.
 * @layer: pointer to the #SRObjectLayer object.
 *
 * Get layer for @obj.
 *
 * Returns: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean 
sro_get_layer (SRObject *obj, 
     	       SRObjectLayer *layer,
	       SRLong index)
{
  SRObjectClass *klass;

  srl_return_val_if_fail (SR_IS_OBJECT (obj), FALSE);

  klass = SR_OBJECT_GET_CLASS (obj);
  if (klass->get_layer)
	  return (klass->get_layer) (obj, layer, index);
  else
	  return FALSE;
}


/**
 * sro_get_MDIZOrder:
 * @obj: a pointer to a #SRObject structure to query.
 * @MDIZOrder: pointer to a short indicating the stacking order of the 
 *             object in the MDI layer, or -1 if it is not in MDI layer.
 *		( Bigger Z-order means nearer to the top)
 *
 * Get MDIZOrder for @obj.(this info might be valid if the layer is 
 * SR_LAYER_MDI and it has to be valid if SR_LAYER_WINDOW
 *
 * Returns: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean 
sro_get_MDIZOrder (SRObject *obj, 
     	    	   short *MDIZOrder,
	    	   SRLong index)
{
  SRObjectClass *klass;

  srl_return_val_if_fail (SR_IS_OBJECT (obj), FALSE);

  klass = SR_OBJECT_GET_CLASS (obj);
  if (klass->get_MDIZOrder)
	  return (klass->get_MDIZOrder) (obj, MDIZOrder, index);
  else
	  return FALSE;
}

static gboolean
get_location_from_array_of_acc (GArray *array, 
		    		AccessibleCoordType type, 
		    		SRRectangle *location)
{
    Accessible * acc;
    SRRectangle location_;
    
    srl_return_val_if_fail (array && array->len > 0, FALSE);
    srl_return_val_if_fail (location, FALSE);

    acc = g_array_index (array, Accessible *, 0);
    if (!acc)
	return FALSE;
    get_location_from_acc (acc, type, &location_);
    location->x = location_.x;
    location->y = location_.y;

    acc = g_array_index (array, Accessible *, array->len - 1);
    if (!acc)
	return FALSE;
    get_location_from_acc (acc, type, &location_);
    location->width = location_.x + location_.width - location->x;
    location->height = location_.height;
    
    return TRUE;
}

/**
 * sro_get_location:
 * @obj: a pointer to a #SRObject structure to query.
 * @type: a #SRCoordinateType .
 * @location: pointer to the #SRLocation object.
 *
 * Get location for @obj in @type screen reader coordinate type.
 *
 * Returns: #TRUE if successful, #FALSE otherwise.
 *
**/
gboolean 
sro_default_get_location (SRObject *obj, 
		  SRCoordinateType type, 
		  SRRectangle *location,
		  SRLong index)
{
    gboolean rv = FALSE;
    AccessibleCoordType acc_type;
	
    srl_return_val_if_fail (obj && location, FALSE);
    srl_return_val_if_fail (sro_is_component (obj, index), FALSE);

    acc_type = sr_2_acc_coord (type);
    if (index == SR_INDEX_CONTAINER)
    {
	switch (obj->role)
	{
	    case SR_ROLE_STATUS_BAR:
	    case SR_ROLE_TABLE_LINE:
	    case SR_ROLE_TABLE_COLUMNS_HEADER:
		if (obj->children)
		    rv = get_location_from_array_of_acc (obj->children, acc_type, location);
		else
		    rv = get_location_from_acc (obj->acc, acc_type, location);
		break;
	    case SR_ROLE_TITLE_BAR:
		{
		    Accessible *child;
		    sru_assert (Accessible_getChildCount (obj->acc) == 1);
		    child = Accessible_getChildAtIndex (obj->acc, 0);
		    if (child)
		    {
			SRRectangle location1;
			rv = get_location_from_acc (obj->acc, acc_type, location)
				&&get_location_from_acc (child, acc_type, &location1);
			if (rv)
			    location->height -= location1.height;
		    	Accessible_unref (child);
		    }
		}
		break;
	    default:
		if (obj->acc)
		    rv = get_location_from_acc (obj->acc, acc_type, location);
		break;	    
	}
    }
    else
    {
	Accessible *child;	    
        child = sro_get_acc_at_index (obj, index);
	if (child)
	    rv = get_location_from_acc (child, acc_type, location);
    }
    return rv;
}

static SRSpecialization 
get_sro_specialization (const SRObject *obj, SRLong index)
{
    SRSpecialization specialization = SR_IS_NOTHING;
    srl_return_val_if_fail (obj, SR_IS_NOTHING);

    specialization = SR_IS_NOTHING;
    if (index == SR_INDEX_CONTAINER)
    {
	switch (obj->role)
	{
	    case SR_ROLE_COMBO_BOX:
		if (obj->acc)
		    specialization = get_specialization_from_acc (obj->acc);
		specialization |= SR_IS_TEXT;
		break;	    
	    case SR_ROLE_STATUS_BAR:
	    case SR_ROLE_TABLE_COLUMNS_HEADER:
	    case SR_ROLE_TABLE_LINE:
		specialization = SR_IS_COMPONENT;
		break;
	    default:
		if (obj->acc)
		    specialization = get_specialization_from_acc (obj->acc);
		break;
	}
    }
    else
    {
	Accessible *child;
	child = sro_get_acc_at_index (obj, index);
	if (child)
	    specialization = get_specialization_from_acc (child);
    }
    srl_debug (specialization != SR_IS_NOTHING);
    return specialization;
}



gboolean 
sro_default_is_action (const SRObject *obj, SRLong index) 
{
    srl_return_val_if_fail (obj, FALSE);

    return (
		(get_sro_specialization (obj, index) & SR_IS_ACTION) ? TRUE : FALSE 
	    );
}

gboolean 
sro_default_is_component (const SRObject *obj, SRLong index) 
{
    srl_return_val_if_fail (obj, FALSE);
    
    return (
		(get_sro_specialization (obj, index) & SR_IS_COMPONENT) ? TRUE : FALSE 
	    );
}

gboolean 
sro_default_is_editable_text (const SRObject *obj, SRLong index) 
{
    srl_return_val_if_fail (obj, FALSE);
    
    return (
		(get_sro_specialization (obj, index) & SR_IS_EDITABLE_TEXT) ? TRUE : FALSE 
	    );
}

gboolean 
sro_default_is_hypertext (const SRObject *obj, SRLong index) 
{
    srl_return_val_if_fail (obj, FALSE);
    
    return (
		(get_sro_specialization (obj, index) & SR_IS_HYPERTEXT) ? TRUE : FALSE 
	    );
}


gboolean 
sro_default_is_image (const SRObject *obj, SRLong index) 
{
    srl_return_val_if_fail (obj, FALSE);
    
    return (
		(get_sro_specialization (obj, index) & SR_IS_IMAGE) ? TRUE : FALSE 
	    );
}

gboolean 
sro_default_is_selection (const SRObject *obj, SRLong index) 
{
    srl_return_val_if_fail (obj, FALSE);
    
    return (
		(get_sro_specialization (obj, index) & SR_IS_SELECTION) ? TRUE : FALSE 
	    );
}

gboolean 
sro_default_is_table (const SRObject *obj, SRLong index) 
{
    srl_return_val_if_fail (obj, FALSE);

    return (
		(get_sro_specialization (obj, index) & SR_IS_TABLE) ? TRUE : FALSE 
	    );
}


gboolean 
sro_default_is_text (const SRObject *obj, SRLong index) 
{
    srl_return_val_if_fail (obj, FALSE);

    return (
		(get_sro_specialization (obj, index) & SR_IS_TEXT) ? TRUE : FALSE 
	    );
}

gboolean 
sro_default_is_value (const SRObject *obj, SRLong index) 
{
    srl_return_val_if_fail (obj, FALSE);
    
    return (
		(get_sro_specialization (obj, index) & SR_IS_VALUE) ? TRUE : FALSE 
	    );
}




gboolean 
sro_is_action (const SRObject *obj, SRLong index) 
{
  SRObjectClass *klass;

  srl_return_val_if_fail (SR_IS_OBJECT (obj), FALSE);

  klass = SR_OBJECT_GET_CLASS (obj);
  if (klass->is_action)
	  return (klass->is_action) (obj, index);
  else
	  return FALSE;
}

gboolean 
sro_is_component (const SRObject *obj, SRLong index) 
{
  SRObjectClass *klass;

  srl_return_val_if_fail (SR_IS_OBJECT (obj), FALSE);

  klass = SR_OBJECT_GET_CLASS (obj);
  if (klass->is_component)
	  return (klass->is_component) (obj, index);
  else
	  return FALSE;
}

gboolean 
sro_is_editable_text (const SRObject *obj, SRLong index) 
{
  SRObjectClass *klass;

  srl_return_val_if_fail (SR_IS_OBJECT (obj), FALSE);

  klass = SR_OBJECT_GET_CLASS (obj);
  if (klass->is_editable_text)
	  return (klass->is_editable_text) (obj, index);
  else
	  return FALSE;
}

gboolean 
sro_is_hypertext (const SRObject *obj, SRLong index) 
{
  SRObjectClass *klass;

  srl_return_val_if_fail (SR_IS_OBJECT (obj), FALSE);

  klass = SR_OBJECT_GET_CLASS (obj);
  if (klass->is_hypertext)
	  return (klass->is_hypertext) (obj, index);
  else
	  return FALSE;
}


gboolean 
sro_is_image (const SRObject *obj, SRLong index) 
{
  SRObjectClass *klass;

  srl_return_val_if_fail (SR_IS_OBJECT (obj), FALSE);

  klass = SR_OBJECT_GET_CLASS (obj);
  if (klass->is_image)
	  return (klass->is_image) (obj, index);
  else
	  return FALSE;
}

gboolean 
sro_is_selection (const SRObject *obj, SRLong index) 
{
  SRObjectClass *klass;

  srl_return_val_if_fail (SR_IS_OBJECT (obj), FALSE);

  klass = SR_OBJECT_GET_CLASS (obj);
  if (klass->is_selection)
	  return (klass->is_selection) (obj, index);
  else
	  return FALSE;
}

gboolean 
sro_is_table (const SRObject *obj, SRLong index) 
{
  SRObjectClass *klass;

  srl_return_val_if_fail (SR_IS_OBJECT (obj), FALSE);

  klass = SR_OBJECT_GET_CLASS (obj);
  if (klass->is_table)
	  return (klass->is_table) (obj, index);
  else
	  return FALSE;
}


gboolean 
sro_is_text (const SRObject *obj, SRLong index) 
{
  SRObjectClass *klass;

  srl_return_val_if_fail (SR_IS_OBJECT (obj), FALSE);

  klass = SR_OBJECT_GET_CLASS (obj);
  if (klass->is_text)
	  return (klass->is_text) (obj, index);
  else
	  return FALSE;
}

gboolean 
sro_is_value (const SRObject *obj, SRLong index) 
{
  SRObjectClass *klass;

  srl_return_val_if_fail (SR_IS_OBJECT (obj), FALSE);

  klass = SR_OBJECT_GET_CLASS (obj);
  if (klass->is_value)
	  return (klass->is_value) (obj, index);
  else
	  return FALSE;
}

gboolean 
sro_action_get_count (SRObject *obj,
		      SRLong *count,
		      SRLong index)
{
    Accessible *acc;
    AccessibleAction *acc_action;

    if (count)
	*count = -1;
    srl_return_val_if_fail (obj && count, FALSE);
    srl_return_val_if_fail (sro_is_action (obj, index), FALSE);
    
    acc = sro_get_acc_at_index (obj, index);
    if (!acc)
	return FALSE;
    
    acc_action = get_action_from_acc (acc);
    if (!acc_action)
	return FALSE;
	
    *count = AccessibleAction_getNActions (acc_action);
    AccessibleAction_unref (acc_action);
        
    return TRUE;
}		    

gboolean
sro_action_get_name (SRObject *obj,
		     SRLong index,
		     gchar **name,
		     SRLong index_obj)
{
    Accessible *acc;
    AccessibleAction *acc_action;
    
    if (name)
	*name = NULL;
    srl_return_val_if_fail (obj && name, FALSE);
    srl_return_val_if_fail (sro_is_action (obj, index_obj), FALSE);

    acc = sro_get_acc_at_index (obj, index_obj);    
    if (!acc)
	return FALSE;
    
    acc_action = get_action_from_acc (acc);
    if (!acc_action)
	return FALSE;
    
    if (0 <= index && index < AccessibleAction_getNActions (acc_action))
    {
	gchar *tmp;
	tmp = AccessibleAction_getName (acc_action, index);
        *name = (tmp && tmp[0]) ? SR_strdup (tmp) : NULL;
        SPI_freeString (tmp);
    }
    AccessibleAction_unref (acc_action);

    return *name ? TRUE : FALSE;
}		    

gboolean
sro_action_get_key (SRObject *obj,
		    SRLong index,
		    gchar **key,
		    SRLong index_obj)
{
    Accessible *acc;
    AccessibleAction *acc_action;
    
    if (key)
	*key = NULL;
        
    srl_return_val_if_fail (obj && key, FALSE);
    srl_return_val_if_fail (sro_is_action (obj, index_obj), FALSE);
    
    acc = sro_get_acc_at_index (obj, index_obj);
    if (!acc)
	return FALSE;
    
    acc_action = get_action_from_acc (acc);    
    if (!acc_action)
	return FALSE;
    if (0 <= index && index < AccessibleAction_getNActions (acc_action))
    {
	gchar *tmp;
	tmp = AccessibleAction_getKeyBinding (acc_action, index);
        *key = (tmp && tmp[0]) ? SR_strdup (tmp) : NULL;
        SPI_freeString (tmp);
    }
    AccessibleAction_unref (acc_action);

    return *key ? TRUE : FALSE;
}		    


gboolean
sro_action_get_description (SRObject *obj,
			    SRLong index,
			    gchar **description,
			    SRLong index_obj)
{
    Accessible *acc;
    AccessibleAction *acc_action;

    if (description)
	*description = NULL;
    
    srl_return_val_if_fail (obj && description, FALSE);
    srl_return_val_if_fail (sro_is_action (obj, index_obj), FALSE);
    
    acc = sro_get_acc_at_index (obj, index_obj);
    if (!acc)
	return FALSE;
    
    acc_action = get_action_from_acc (acc);
    if (!acc_action)
	return FALSE;
	
    if (0 <= index && index < AccessibleAction_getNActions (acc_action))
    {
        gchar *tmp;
	tmp = AccessibleAction_getDescription (acc_action, index);
        *description = (tmp && tmp[0]) ? SR_strdup (tmp) : NULL;
        SPI_freeString (tmp);
    }
    AccessibleAction_unref (acc_action);
    
    return *description ? TRUE : FALSE;
}		    



gboolean
sro_action_do_action (SRObject *obj,
			gchar *action,
			SRLong index_obj)
{
    Accessible *acc;
    AccessibleAction *acc_action;
    gint cnt, i;
    gboolean rv = FALSE;

    srl_return_val_if_fail (obj && action, FALSE);
/*
    srl_return_val_if_fail (sro_is_action (obj, index_obj), FALSE);
*/    
    acc = sro_get_acc_at_index (obj, index_obj);
    if (!acc)
	return FALSE;
    
    acc_action = get_action_from_acc (acc);
    if (!acc_action)
	return FALSE;
	
    cnt = AccessibleAction_getNActions (acc_action);
    for (i = 0; i < cnt; i++)
    {
        gchar *tmp;
	tmp = AccessibleAction_getName (acc_action, i);
	
        if (tmp && strcmp (tmp, action) == 0)
	{
	    rv = AccessibleAction_doAction (acc_action, i);
	}
        SPI_freeString (tmp);
	if (rv)
	    break;
    }
    AccessibleAction_unref (acc_action);
    
    return rv;
}		    




gboolean 
sro_image_get_description (SRObject *obj, 
			   gchar **description,
			   SRLong index) 
{
    Accessible *acc;
    AccessibleImage *acc_image;
    gchar *tmp;
    
    if (description)
	*description = NULL;
    
    srl_return_val_if_fail (obj && description, FALSE);
    srl_return_val_if_fail (sro_is_image (obj, index), FALSE);
    
    acc = sro_get_acc_at_index (obj,index);
    if (!acc)
	return FALSE;
    
    acc_image = get_image_from_acc (acc);
    if (!acc_image)
	return FALSE;
	
    tmp = AccessibleImage_getImageDescription (acc_image);
    *description = (tmp && tmp[0]) ? SR_strdup (tmp) : NULL;
    SPI_freeString (tmp);
    AccessibleImage_unref (acc_image);

    return *description ? TRUE : FALSE;
}			   


gboolean 
sro_image_get_location (SRObject *obj, 
			 SRCoordinateType type, 
			 SRRectangle *location,
			 SRLong index) 
{
    Accessible *acc;
    AccessibleImage *acc_image;
    long int x, y, w, h;
    AccessibleCoordType acc_type;

    srl_return_val_if_fail (obj && location, FALSE);
    srl_return_val_if_fail (sro_is_image (obj, index), FALSE);
    
    acc = sro_get_acc_at_index (obj, index);
    if (!acc)
	return FALSE;
    
    acc_image = get_image_from_acc (acc);
    if (!acc_image)
	return FALSE;
    acc_type = sr_2_acc_coord (type);
    AccessibleImage_getImageExtents (acc_image, &x, &y, &w, &h, acc_type);
    AccessibleImage_unref (acc_image);
    
    location->x = x;
    location->y = y;
    location->width = w;
    location->height = h;
    
    return TRUE;
}




gboolean
sro_value_get_min_val (SRObject *obj,
		       gdouble *min,
		       SRLong index)
{
    Accessible *acc;
    AccessibleValue *acc_value;

    srl_return_val_if_fail (obj && min, FALSE);
    srl_return_val_if_fail (sro_is_value (obj, index), FALSE);
    
    acc = sro_get_acc_at_index (obj, index);
    if (!acc)
	return FALSE;
    
    acc_value = get_value_from_acc (acc);
    if (!acc_value)
	return FALSE;

    *min = AccessibleValue_getMinimumValue (acc_value);
    AccessibleValue_unref (acc_value);
    
    return TRUE;   
}

gboolean
sro_value_get_max_val (SRObject *obj,
		       gdouble *max,
		       SRLong index)
{
    Accessible *acc;
    AccessibleValue *acc_value;

    srl_return_val_if_fail (obj && max, FALSE);
    srl_return_val_if_fail (sro_is_value (obj, index), FALSE);

    acc = sro_get_acc_at_index (obj, index);
    if (!acc)
	return FALSE;
    
    acc_value = get_value_from_acc (acc);
    if (!acc_value)
	return FALSE;

    *max = AccessibleValue_getMaximumValue (acc_value);
    AccessibleValue_unref (acc_value);
    
    return TRUE;   
}


gboolean
sro_value_get_crt_val (SRObject *obj,
		       gdouble *crt,
		       SRLong index)
{
    Accessible *acc;
    AccessibleValue *acc_value;

    srl_return_val_if_fail (obj && crt, FALSE);
    srl_return_val_if_fail (sro_is_value (obj, index), FALSE);

    acc = sro_get_acc_at_index (obj, index);
    if (!acc)
	return FALSE;
    
    acc_value = get_value_from_acc (acc);
    if (!acc_value)
	return FALSE;

    *crt = AccessibleValue_getCurrentValue (acc_value);
    AccessibleValue_unref (acc_value);
        
    return TRUE;   
}


static gboolean
get_text_range_from_offset (AccessibleText *text,
			    SRTextBoundaryType type,
			    SRLong offset,
			    SRLong *start,
			    SRLong *end)
{
    AccessibleTextBoundaryType acc_type_text;
    gchar *temp;
    long int start_range, end_range;
    
    srl_return_val_if_fail (text && start && end, FALSE);
    
    if (!(0 <= offset && offset <= AccessibleText_getCharacterCount (text)))
	return FALSE;
        
    *start = *end = -1;
    acc_type_text = sr_2_acc_tb (type);
    temp = AccessibleText_getTextAtOffset (text, offset, acc_type_text, 
						&start_range, &end_range);
    if (temp)
    {
	int i;
	i = 0;
	while (temp[i] == '\n' || (type != SR_TEXT_BOUNDARY_LINE && temp[i] == ' '))
	{
	    start_range++;
	    i++;
	}
	if (start_range > offset)
	{
	    start_range = offset;
	    end_range = offset + 1;
	}	    
    }
    	    
    *start = start_range;
    *end = end_range;
    SPI_freeString (temp);
    return TRUE;
}		


gboolean
sro_is_word_navigation (SRObject *obj,
                        SRLong crt_offset,
			SRLong last_offset,
			SRLong index)
{
    Accessible *acc = NULL;
    AccessibleText *acc_text = NULL;
    SRLong start_crt_word, end_crt_word, start_last_word, end_last_word,
           start_crt_line, end_crt_line, start_last_line, end_last_line;
    gchar *temp = NULL;
    gboolean rv;
    
    srl_return_val_if_fail (obj, FALSE);
    srl_return_val_if_fail (sro_is_text (obj, index), FALSE);
    
    acc = sro_get_acc_at_index (obj, index);
    if (!acc)
	return FALSE;
	
    acc_text = get_text_from_acc (acc);
    if (!acc_text)
	return FALSE;
	
    temp = AccessibleText_getTextAtOffset (acc_text, crt_offset, SPI_TEXT_BOUNDARY_WORD_START, 
						&start_crt_word, &end_crt_word);	
    temp = AccessibleText_getTextAtOffset (acc_text, last_offset, SPI_TEXT_BOUNDARY_WORD_START,   
						&start_last_word, &end_last_word);						
    temp = AccessibleText_getTextAtOffset (acc_text, crt_offset, SPI_TEXT_BOUNDARY_LINE_START, 
						&start_crt_line, &end_crt_line);	
    temp = AccessibleText_getTextAtOffset (acc_text, last_offset, SPI_TEXT_BOUNDARY_LINE_START,   
						&start_last_line, &end_last_line);												
    if ((start_crt_word == end_last_word  && start_crt_word >= start_crt_line) || /* down navigation */
        (start_last_word == end_crt_word && start_last_word >= start_last_line))  /* up navigation */
	rv = TRUE;
    else
	rv = FALSE;		
	
    AccessibleText_unref (acc_text);						
    SPI_freeString (temp);
    
    return rv;
}			

gboolean
sro_text_get_abs_offset (SRObject *obj, 
			 SRLong *offset,
			 SRLong index)
{
    Accessible *acc;
    AccessibleText *acc_text;
    
    if (offset)
	*offset = -1;
    srl_return_val_if_fail (obj && offset, FALSE);
    srl_return_val_if_fail (sro_is_text (obj, index), FALSE);

    acc = sro_get_acc_at_index (obj, index);
    if (!acc)
	return FALSE;
	
    acc_text = get_text_from_acc (acc);
    if (!acc_text)
	return FALSE;
    
    *offset = AccessibleText_getCaretOffset (acc_text);
    
    AccessibleText_unref (acc_text);
    return TRUE;
}
			
gboolean
sro_text_is_same_line (SRObject *obj,
		       SRLong offset,
		       SRLong index)
{
    Accessible *acc;
    AccessibleText *acc_text;
    SRLong crt_offset, start, end;
    
    srl_return_val_if_fail (obj, FALSE);
    srl_return_val_if_fail (sro_is_text (obj, index), FALSE);

    acc = sro_get_acc_at_index (obj, index);
    if (!acc)
	return FALSE;
	
    acc_text = get_text_from_acc (acc);
    if (!acc_text)
	return FALSE;

    crt_offset = AccessibleText_getCaretOffset (acc_text);
    get_text_range_from_offset (acc_text, SR_TEXT_BOUNDARY_LINE, crt_offset, &start, &end);
    
    AccessibleText_unref (acc_text);
    
    return (start <= offset) && (offset < end) ? TRUE : FALSE;
}


gboolean
sro_text_get_caret_offset (SRObject *obj,
			   SRLong *line_offset, 
			   SRLong index)
{
    Accessible *acc;
    AccessibleText *acc_text;
    SRLong offset, start, end;
    if (line_offset)
	*line_offset = -1;
    srl_return_val_if_fail (obj && line_offset, FALSE);
    srl_return_val_if_fail (sro_is_text (obj, index), FALSE);

    acc = sro_get_acc_at_index (obj, index);
    if (!acc)
	return FALSE;
	
    acc_text = get_text_from_acc (acc);
    if (!acc_text)
	return FALSE;
    
    offset = AccessibleText_getCaretOffset (acc_text);
    get_text_range_from_offset (acc_text, SR_TEXT_BOUNDARY_LINE, offset, &start, &end);

    AccessibleText_unref (acc_text);
    
    *line_offset = offset - start;
        
    return TRUE;
}

gboolean 
sro_text_get_caret_location (SRObject *obj, 
			     SRCoordinateType type, 
			     SRRectangle *location,
			     SRLong index) 
{
    Accessible *acc;
    AccessibleText *acc_text;
    long int x, y, w, h, offset;
    AccessibleCoordType acc_type;

    srl_return_val_if_fail (obj && location, FALSE);
    srl_return_val_if_fail (sro_is_text (obj, index), FALSE);
    
    acc = sro_get_acc_at_index (obj, index);
    if (!acc)
	return FALSE;
	
    acc_text = get_text_from_acc (acc);
    if (!acc_text)
	return FALSE;

    acc_type = sr_2_acc_coord (type);
    offset = AccessibleText_getCaretOffset (acc_text);
    if (offset == AccessibleText_getCharacterCount (acc_text) && offset > 0)
	offset--;

    AccessibleText_getCharacterExtents (acc_text, offset, &x, &y, &w, &h, acc_type);
    AccessibleText_unref (acc_text);
    location->x = x;
    location->y = y;
    location->width = w;
    location->height = h;
    return TRUE;
}


gboolean
sro_text_set_caret_offset (SRObject *obj,
			   SRLong line_offset, 
			   SRLong index)
{
    Accessible *acc;
    AccessibleText *acc_text;
    SRLong offset, start, end;
    gboolean rv;
    
    srl_return_val_if_fail (obj, FALSE);
    srl_return_val_if_fail (sro_is_text (obj, index), FALSE);

    acc = sro_get_acc_at_index (obj, index);
    if (!acc)
	return FALSE;
	
    acc_text = get_text_from_acc (acc);
    if (!acc_text)
	return FALSE;
    
    offset = AccessibleText_getCaretOffset (acc_text);
    get_text_range_from_offset (acc_text, SR_TEXT_BOUNDARY_LINE, offset, &start, &end);

    line_offset += start;
    line_offset = line_offset > end ? end : line_offset;
    
    rv = AccessibleText_setCaretOffset (acc_text, line_offset);    
        
    AccessibleText_unref (acc_text);
    return rv;
}





#if 0
static inline int 
tb_2_index (SRTextBoundaryType type)
{
    switch (type)
    {
	case SR_TEXT_BOUNDARY_CHAR:
	    return 0;
	case SR_TEXT_BOUNDARY_WORD:
	    return 1;
	case SR_TEXT_BOUNDARY_SENTENCE:
	    return 2;
	case SR_TEXT_BOUNDARY_LINE:
	    return 3;
	default:
	    srl_assert_not_reached ();
	    break;
    }
    return 0;
}
#endif

gboolean 
sro_text_get_text_from_caret (SRObject *obj, 
			      SRTextBoundaryType type,
			      gchar **text,
			      SRLong index)
{
    Accessible *acc;
    AccessibleText *acc_text;
    SRLong offset, start, end;
    gchar *tmp;
    if (text)
	*text = NULL;
    srl_return_val_if_fail (obj && text, FALSE);
    srl_return_val_if_fail (sro_is_text (obj, index), FALSE);
    
    acc = sro_get_acc_at_index (obj, index);
    if (!acc)
	return FALSE;
	
    acc_text = get_text_from_acc (acc);
    if (!acc_text)
	return FALSE;
    
    offset = AccessibleText_getCaretOffset (acc_text);
    get_text_range_from_offset (acc_text, type, offset, &start, &end);
    tmp = AccessibleText_getText (acc_text, start, end);
    *text = (tmp && tmp[0]) ? SR_strdup (tmp) : NULL;
    SPI_freeString (tmp);
    AccessibleText_unref (acc_text);
    
    return *text ? TRUE : FALSE;    
}						    
		

#define SR_BOLD 	700
#define SR_COLOR_DIF	100
#define SR_COLOR_MAX	65535
#define SR_COLOR_MIN	0

typedef struct _SRCcolor
{
    long r;
    long g;
    long b;
    char *name;
}SRColor;

static SRColor colors[] =
{
    { SR_COLOR_MIN, SR_COLOR_MIN, SR_COLOR_MIN,	"white"},
    { SR_COLOR_MAX, SR_COLOR_MIN, SR_COLOR_MIN,	"red"},
    { SR_COLOR_MIN, SR_COLOR_MAX, SR_COLOR_MIN,	"green"},
    { SR_COLOR_MIN, SR_COLOR_MIN, SR_COLOR_MAX,	"blue"},
    { SR_COLOR_MAX, SR_COLOR_MAX, SR_COLOR_MAX,	"black"},
};
    
static gchar *
sra_get_color (gchar *color)
{
    gchar *tmp;
    long r = 0, g = 0, b = 0;
    int i;
    
    srl_return_val_if_fail (color, NULL);
    r = atol (color);
    tmp =strstr (color, ",");
    if (tmp)
	g = atol (tmp + 1);
    if (tmp)
	tmp =strstr (tmp + 1, ",");
    if (tmp)
	b = atol (tmp + 1);
    
    for (i = 0; i < G_N_ELEMENTS (colors); i++)
	if (   (colors[i].r - SR_COLOR_DIF <= r && colors[i].r + SR_COLOR_DIF >= r)
	    && (colors[i].g - SR_COLOR_DIF <= g && colors[i].g + SR_COLOR_DIF >= g)
	    && (colors[i].b - SR_COLOR_DIF <= b && colors[i].b + SR_COLOR_DIF >= b))
	return g_strdup (colors[i].name);
    return g_strdup (color);
}


static SRTextAttribute
sra_prelucrare (SRTextAttribute attr)
{
    int offset = 0;
    gchar val[1000];
    
    srl_return_val_if_fail (attr, NULL);

    while (*attr)
    {
	gchar *tmp, *tmp2;
	int was_null = 0;
	tmp = strstr (attr, ":") + 1;
	tmp2 = strstr (tmp, "; ");
/*FIXME: in java attrs are separated by ",". in gtk+ are separated by ";". */
	if (!tmp2)
	    tmp2 = strstr (tmp, ", ");
	if (!tmp2)
	{
	    tmp2 = tmp + strlen (tmp);
	    was_null = 1;
	}
	*tmp2 = '\0';
	if (g_ascii_strncasecmp (attr, "weight", tmp - attr - 1) == 0) 
	{
	    if (atoi (tmp) >= SR_BOLD)
	        offset += sprintf (val+offset, ",  bold:true");
	    else
		offset += sprintf (val+offset, ",  bold:false");
	}
	else if (g_ascii_strncasecmp (attr, "fg-stipple", tmp - attr - 1) == 0) 
	{
	    offset += sprintf (val+offset, ",  foreground-stipple:%s", tmp);
	}
	else if (g_ascii_strncasecmp (attr, "bg-stipple", tmp - attr - 1) == 0) 
	{
	    offset += sprintf (val+offset, ",  background-stipple:%s", tmp);
	}
	else if (g_ascii_strncasecmp (attr, "fg-color", tmp - attr - 1) == 0) 
	{
	    gchar *color;
	    color = sra_get_color (tmp);
	    offset += sprintf (val+offset, ",  foreground-color:%s", color);
	    g_free (color);
	}
	else if (g_ascii_strncasecmp (attr, "bg-color", tmp - attr - 1) == 0) 
	{
	    gchar *color;
	    color = sra_get_color (tmp);
	    offset += sprintf (val+offset, ",  background-color:%s", color);
	    g_free (color);
	}
	else if (g_ascii_strncasecmp (attr, "family-name", tmp - attr - 1) == 0) 
	{
	    offset += sprintf (val+offset, ",  font-name:%s", tmp);
	}
	else if (g_ascii_strncasecmp (attr, "style", tmp - attr - 1) == 0) 
	{
	    if (g_ascii_strcasecmp (tmp, "italic") == 0)
		offset += sprintf (val+offset, ",  italic:true");
	    else
		offset += sprintf (val+offset, ",  style:%s", tmp);		
	}
	else
	{
	    offset += sprintf (val+offset, ",  %s", attr);
	}
	attr = tmp2;
	if (!was_null)
	{
	    *tmp2 = ';';
	    attr++;
	    while (*attr == ' ')
		attr++;
	}
    }

    return SR_strdup (val + 3);
}



static gboolean
get_text_attributes_from_range (AccessibleText *text,
				SRLong start, 
				SRLong end,
				SRTextAttribute **attr)
{
    int n_selections, i;
    SRLong diff = start;
    GSList *list = NULL;
    int is_selection;
    
    if (attr)
	*attr = NULL;
    
    srl_return_val_if_fail (text && attr, FALSE);
    if (start >= end)
	return FALSE;
    if (0 > start || end > AccessibleText_getCharacterCount (text))
	return FALSE;

    n_selections = AccessibleText_getNSelections (text);
    i = 0;
    while (start < end)
    {
	gchar *tmp, tmp2[50];
	long int ss, es, sa, ea;
	SRLong start_t, end_t;
	SRTextAttribute attr_range, attr_range_tmp;
	is_selection = 0;
	end_t = end;

	if (i < n_selections)
	{
	    AccessibleText_getSelection (text, i, &ss, &es);    
	    if (es <= start)
	    {
		i++;
		continue;
	    }
	    if (ss <= start)
	    	is_selection = 1;
	    else
	    	end_t = ss;
	}
	
	tmp = AccessibleText_getAttributes (text, start, &sa, &ea);
	if (start < sa || start > ea)
	    break; 

	start_t = start;
	end_t = MIN (end_t, end);		
	end_t = MIN (end_t, ea);	

	if (is_selection)
	{
	    if (ss > start)
		end_t = MIN (ss, end_t);
	    else
		end_t = MIN (es, end_t);
	}
	sprintf (tmp2, "start:%ld;  end:%ld", start_t - diff, end_t - diff);
	if (is_selection && ss < end && es >= start)
	{
	    if (tmp && tmp[0])
	    	attr_range = g_strconcat (tmp2, ";  selected:true;  ", tmp, NULL);
	    else
		attr_range = g_strconcat (tmp2, ";  selected:true", NULL);
	}
	else
	{
	    if (tmp && tmp[0])
	    	attr_range = g_strconcat (tmp2, ";  ", tmp, NULL);
	    else
		attr_range = g_strconcat (tmp2, NULL);
	}
	attr_range_tmp = sra_prelucrare (attr_range);
	list = g_slist_append (list, attr_range_tmp);
	g_free (attr_range);
	SPI_freeString (tmp);
	start = end_t;
    }
    
    *attr = (SRTextAttribute *) g_malloc ((g_slist_length (list) + 1) * sizeof (SRTextAttribute));
    
    for (i = 0; i < g_slist_length (list); i++)
    {
	(*attr)[i] = (gchar *) g_slist_nth_data (list, i);
    }
    (*attr)[g_slist_length (list)] = NULL;
    g_slist_free (list);
    return TRUE;
}


gboolean 
sro_text_get_text_attr_from_caret (SRObject *obj, 
				   SRTextBoundaryType type,
				   SRTextAttribute **attr,
				   SRLong index)
{
    Accessible *acc;
    AccessibleText *acc_text;
    SRLong offset, start, end;
    if (attr)
	*attr = NULL;
    srl_return_val_if_fail (obj && attr, FALSE);
    srl_return_val_if_fail (sro_is_text (obj, index), FALSE);
        
    acc = sro_get_acc_at_index (obj, index);
    if (!acc)
	return FALSE;
    acc_text = get_text_from_acc (acc);
    if (!acc_text)
	return FALSE;
    
    offset = AccessibleText_getCaretOffset (acc_text);
    get_text_range_from_offset (acc_text, type, offset, &start, &end);

    if (start < end)
        get_text_attributes_from_range (acc_text, start, end, attr);	
        
    AccessibleText_unref (acc_text);
    return *attr ? TRUE : FALSE;    
}


gboolean 
sro_text_get_text_location_from_caret (SRObject *obj, 
				       SRTextBoundaryType type_text,
				       SRCoordinateType type_coord,
				       SRRectangle *location,
				       SRLong index)
{
    Accessible *acc;
    AccessibleText *acc_text;
    SRLong offset, start, end;
    long int x, y, width, height;
    AccessibleCoordType acc_type_coord;
    gboolean rv = FALSE;
    
    srl_return_val_if_fail (obj && location, FALSE);
    srl_return_val_if_fail (sro_is_text (obj, index), FALSE);
    
    location->x = location->y = -1;
    location->height = location->width = 0;
        
    if (type_text == SR_TEXT_BOUNDARY_SENTENCE)
	return FALSE;
    
    acc = sro_get_acc_at_index(obj, index);
    if (!acc)
	return FALSE;
    acc_text = get_text_from_acc (acc);
    if (!acc_text)
	return FALSE;
    
    offset = AccessibleText_getCaretOffset (acc_text);
    get_text_range_from_offset (acc_text, type_text, offset, &start, &end);

    acc_type_coord = sr_2_acc_coord (type_coord);

    if (start < end)
    {    
	AccessibleText_getCharacterExtents (acc_text, start,
		        &x, &y, &width, &height, acc_type_coord);
	location->x = x;
	location->y = y;
	location->height = height;
	AccessibleText_getCharacterExtents (acc_text, end - 1,
			&x, &y, &width, &height, acc_type_coord);
        location->width = x - location->x + width;
	rv = TRUE;
    }
    
    AccessibleText_unref (acc_text);
    return rv;    
}				        


gboolean 
sro_text_get_line_offset_from_point (SRObject *obj,
				      const SRPoint *point,
				      SRCoordinateType type,
				      SRLong *line_offset,
				      SRLong index)
{
    Accessible *acc;
    AccessibleText *acc_text;
    AccessibleCoordType acc_type_coord;
    SRLong offset, start, end;

    if (line_offset)
	*line_offset = -1;    
    srl_return_val_if_fail (obj && point && line_offset, FALSE);
    srl_return_val_if_fail (sro_is_text (obj, index), FALSE);
    
    acc = sro_get_acc_at_index(obj, index);
    if (!acc)
	return FALSE;
    acc_text = get_text_from_acc (acc);
    if (!acc_text)
	return FALSE;

    acc_type_coord = sr_2_acc_coord (type);
    offset = AccessibleText_getOffsetAtPoint (acc_text,
		    point->x, point->y, acc_type_coord);
    
    get_text_range_from_offset (acc_text, SR_TEXT_BOUNDARY_LINE, offset, &start, &end);

    *line_offset = offset - start;
    
    return TRUE;
}

gboolean 
sro_text_get_text_from_point (SRObject *obj, 
			      const SRPoint *point,
			      SRCoordinateType type_coord,
			      SRTextBoundaryType type_text,
			      gchar **text,
			      SRLong index)
{
    Accessible *acc;
    AccessibleText *acc_text;
    AccessibleCoordType acc_type_coord;
    SRLong offset, start, end;
    gchar *tmp = NULL;
    if (text)
	*text = NULL;    
    srl_return_val_if_fail (obj && point && text, FALSE);
    srl_return_val_if_fail (sro_is_text (obj, index), FALSE);
    
    acc = sro_get_acc_at_index(obj, index);
    if (!acc)
	return FALSE;
    acc_text = get_text_from_acc (acc);
    if (!acc_text)
	return FALSE;

    acc_type_coord = sr_2_acc_coord (type_coord);
    offset = AccessibleText_getOffsetAtPoint (acc_text,
		    point->x, point->y, acc_type_coord);
    
    get_text_range_from_offset (acc_text, SR_TEXT_BOUNDARY_LINE, offset, &start, &end);
    if (start < end)
	tmp = AccessibleText_getText (acc_text, start, end);
    
    *text = (tmp && tmp[0]) ? SR_strdup (tmp) : NULL;
    
    return *text ? TRUE : FALSE;
}						    

    
gboolean 
sro_text_get_text_attr_from_point (SRObject *obj,
				   const SRPoint *point, 
				   SRCoordinateType type_coord,
				   SRTextBoundaryType type_text,
				   SRTextAttribute **attr,
				   SRLong index)
{
    Accessible *acc;
    AccessibleText *acc_text;
    SRLong offset, start, end;
    AccessibleCoordType acc_type_coord;
    if (attr)
	*attr = NULL;    
    srl_return_val_if_fail (obj && point && attr, FALSE);
    srl_return_val_if_fail (sro_is_text (obj, index), FALSE);
        
    acc = sro_get_acc_at_index(obj, index);
    if (!acc)
	return FALSE;
    acc_text = get_text_from_acc (acc);
    if (!acc_text)
	return FALSE;
    
    acc_type_coord = sr_2_acc_coord (type_coord);
    offset = AccessibleText_getOffsetAtPoint (acc_text,
		    point->x, point->y, acc_type_coord);
    
    get_text_range_from_offset (acc_text, type_text, offset, &start, &end);

    if (start < end)
        get_text_attributes_from_range (acc_text, start, end, attr);
    	
    AccessibleText_unref (acc_text);
    return *attr ? TRUE : FALSE;    
}	
					     
gboolean 
sro_text_get_text_location_from_point (SRObject *obj, 
				       const SRPoint *point,
				       SRCoordinateType type_coord,
				       SRTextBoundaryType type_text,
				       SRRectangle *location,
				       SRLong index)
{
    gboolean rv = FALSE;
    Accessible *acc;
    AccessibleText *acc_text;
    SRLong offset, start, end;
    long int x, y, width, height;
    AccessibleCoordType acc_type_coord;
    AccessibleTextBoundaryType acc_type_text;
        
    srl_return_val_if_fail (obj && point && location, FALSE);
    srl_return_val_if_fail (sro_is_text (obj, index), FALSE);
    
    location->x = location->y = -1;
    location->height = location->width = 0;
        
    if (type_text == SR_TEXT_BOUNDARY_SENTENCE)
	return FALSE;
    
    acc = sro_get_acc_at_index (obj, index);
    if (!acc)
	return FALSE;
    acc_text = get_text_from_acc (acc);
    if (!acc_text)
	return FALSE;
    
    acc_type_coord = sr_2_acc_coord (type_coord);
    acc_type_text = sr_2_acc_tb (type_text);
    offset = AccessibleText_getOffsetAtPoint (acc_text,
		    point->x, point->y, acc_type_coord);

    get_text_range_from_offset (acc_text, type_text, offset, &start, &end);
    if (start < end)
    {    
    
	AccessibleText_getCharacterExtents (acc_text, start,
		        &x, &y, &width, &height, acc_type_coord);
	location->x = x;
	location->y = y;
	location->height = height;
	AccessibleText_getCharacterExtents (acc_text, end - 1,
			&x, &y, &width, &height, acc_type_coord);
        location->width = x - location->x + width;
	rv = TRUE;
    }
    
    AccessibleText_unref (acc_text);
    
    return rv;    
}				        



static void
sr_object_class_init (SRObjectClass *klass)
{
  GObjectClass *class_obj;
  
  class_obj = G_OBJECT_CLASS (klass);
  class_obj->dispose = sro_terminate;
  klass->is_action = sro_default_is_action;
  klass->is_component = sro_default_is_component;
  klass->is_editable_text = sro_default_is_editable_text;
  klass->is_hypertext = sro_default_is_hypertext;
  klass->is_image = sro_default_is_image;
  klass->is_selection = sro_default_is_selection;
  klass->is_table = sro_default_is_table;
  klass->is_text = sro_default_is_text;
  klass->is_value = sro_default_is_value;
  klass->get_role = sro_default_get_role;
  klass->get_role_name = sro_default_get_role_name;
  klass->get_name = sro_default_get_name;
  klass->get_description = sro_default_get_description;
  klass->get_parent = sro_default_get_parent;
  klass->get_index_in_parent = sro_default_get_index_in_parent;
  klass->get_children_count = sro_default_get_children_count;
  klass->get_i_child = sro_default_get_i_child;
  klass->get_location = sro_default_get_location;
  klass->get_state = sro_default_get_state;
  klass->get_relation = sro_default_get_relation;
  klass->get_layer = sro_default_get_layer;
  klass->get_MDIZOrder = sro_default_get_MDIZOrder;
  klass->manages_descendants = sro_default_manages_descendants;
}

static void
sr_object_instance_init (SRObject *object)
{
  sro_init (object);	
}

GType
sro_get_type   (void)
{
  static GType type = 0;

  if (!type)
    {
      static const GTypeInfo typeInfo =
      {
        sizeof (SRObjectClass),
        (GBaseInitFunc) NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc) sr_object_class_init,
        (GClassFinalizeFunc) NULL,
        NULL,
        sizeof (SRObject),
        0,
        (GInstanceInitFunc) sr_object_instance_init,
      } ;
      type = g_type_register_static (G_TYPE_OBJECT, "SRObject", &typeInfo, 0) ;
    }
  return type;
}




gboolean
sra_get_attribute_value (SRTextAttribute text_attr,
	    		 gchar *attr,
	    		 gchar **val)
{
    gchar *tmp, *tmp2, *val_tmp = NULL;
    if (val)
	*val = NULL;
    
    if (!(text_attr && attr && val))
	return FALSE;
    
    tmp = strstr (text_attr, attr);
    if (tmp && tmp[strlen (attr)] == ':')
    {
        tmp = strstr (tmp, ":");
	tmp++;
	tmp2 = strstr (tmp, ",  ");
	if (tmp2)
	    val_tmp = g_strndup (tmp, tmp2 - tmp);
	else
	    val_tmp = g_strdup (tmp);
    }
    if (val_tmp)
    {
	*val = SR_strdup (val_tmp);
	g_free (val_tmp);
    }
    
    return *val ? TRUE : FALSE;
}

gboolean
sra_get_attribute_values_string (SRTextAttribute text_attr,
	    			 gchar *attr,
	    			 gchar **val)
{
    gchar *val_tmp;
    gchar val_tmp2[1000];
    
    if (val)
	*val = NULL;
	
    srl_return_val_if_fail (text_attr && val, FALSE);
    
    if (!attr)
    {
	gchar *tmp;
	tmp = strstr (text_attr, "end");
	tmp = strstr (tmp, ", ");
	if (tmp)
	    val_tmp = tmp + 3;
	else
	    val_tmp = NULL;	
    }
    else
    {
	int offset = 0;
	gchar *attr_;
	if (attr[strlen (attr) - 1] != ':' )
	    attr_ = g_strconcat (attr, ":", NULL);
	else
	    attr_ = g_strdup (attr);
	attr = attr_;
	while (*attr)
	{
	    gchar *tmp, *tmp2;
	    tmp = strstr (attr, ":");
	    *tmp = '\0';
	    sra_get_attribute_value (text_attr, attr, &tmp2);
	    srl_return_val_if_fail (tmp2, FALSE);
	    offset += sprintf (val_tmp2+offset, ",  %s:%s", attr, tmp2);
	    SR_freeString (tmp2);
	    *tmp = ':';
	    attr = tmp + 1;	    
	}
	val_tmp = val_tmp2 + 3;
	g_free (attr_);
    }
    
    if (!val_tmp)
    	*val = g_strdup ( "No available attributes");
    else
    	*val = g_strdup (val_tmp);

    val_tmp = *val;
    if (val_tmp && val_tmp[0])
    {
	val_tmp = g_strdelimit (val_tmp, ":", ' ');
        *val = SR_strdup (val_tmp);
    }
    else
	*val = NULL;
	
    g_free (val_tmp);
    
    return *val ? TRUE : FALSE;
}



static gchar*
prel_key_binding (gchar *key)
{
    gchar new_key[50];
    gchar *tmp_key = new_key;
    gchar *tmp;

    if (!key || key[0] == '\0')
	return NULL;
    
    for (tmp = strstr (key, "<"); tmp; tmp = strstr (key, "<"))
    {
	gchar *tmp2 = strstr (tmp, ">");
	if (!tmp2)
	    return NULL;
	*tmp2 = '\0';
	tmp_key = g_stpcpy (tmp_key, tmp + 1);
	tmp_key = g_stpcpy (tmp_key, " ");	
	*tmp2 = '>';
	key = tmp2;
    } 
    if (*key == '>')
	key++;
    tmp = g_strdup (key);
    tmp_key = g_stpcpy (tmp_key, g_strdelimit (tmp, ":", ' '));
    g_free (tmp);

    return SR_strdup (new_key);
}

gboolean
sro_get_shortcut (SRObject *obj,
		  gchar **shortcut,
		  SRLong index)
{
    gboolean rv = FALSE;
    AccessibleAction *acc_action;
    long acc_count, i;
    Accessible *acc;
    if (shortcut)
	*shortcut = NULL;    
    srl_return_val_if_fail (obj && shortcut, FALSE);
/*FIXME*/
/*    srl_return_val_if_fail (sro_is_action (obj, index), FALSE);*/
    if (!sro_is_action (obj, index))
	return FALSE;

    acc = sro_get_acc_at_index (obj, index);
    if (!acc)
	return FALSE;
    acc_action = get_action_from_acc (acc);
    if (!acc_action)
	return FALSE;
    acc_count = AccessibleAction_getNActions (acc_action);
    for (i = 0; !rv && i < acc_count; ++i)
    {
        gchar *tmp;
        tmp = AccessibleAction_getKeyBinding (acc_action, i);
        if (tmp && tmp[0])
        {
	    SRObjectRoles role;
	    sro_get_role (obj, &role, index);
	    if (   role == SR_ROLE_PUSH_BUTTON
		    || role == SR_ROLE_CHECK_BOX
		    || role == SR_ROLE_RADIO_BUTTON)
	    {
		gchar *tmp2, *tmp3;
		tmp3 = g_strdup (tmp);
		tmp2 = strstr (tmp3, ";");
		if (tmp2)
		    *tmp2 = '\0'; 
	        *shortcut = prel_key_binding (tmp3);
		if (*shortcut)
	    	    rv = TRUE;
		g_free (tmp3);
	    }
	    else
	    {
		gchar *tmp2 = tmp;
		tmp2 = strstr (tmp2, ";");
		if (tmp2 )
		{
		    tmp2 = strstr (tmp2 + 1, ";");
		}
		if (tmp2)
		{
		    *shortcut = prel_key_binding (tmp2 + 1);
		    if (*shortcut)
		        rv = TRUE;
		}
		if (!rv)
		{
		    tmp2 = strstr (tmp, ";");
		    if (tmp2)
		    	*tmp2 = '\0';
		    *shortcut = prel_key_binding (tmp);
		    if (*shortcut)
			rv = TRUE;
		}
	    }
	}
	SPI_freeString (tmp);
    }
    AccessibleAction_unref (acc_action);
    
    return rv;    
}

gboolean
sro_get_accelerator (SRObject *obj,
		     gchar **accelerator,
		     SRLong index)
{
    gboolean rv = FALSE;
    AccessibleAction *acc_action;
    Accessible *acc;
    long acc_count, i;

    if (accelerator)
	*accelerator = NULL;
    
    srl_return_val_if_fail (obj && accelerator, FALSE);
/*FIXME*/
/*    srl_return_val_if_fail (sro_is_action (obj, index), FALSE);*/
    if (!sro_is_action (obj, index))
	return FALSE;

    acc = sro_get_acc_at_index(obj, index);
    if (!acc)
	return FALSE;
    acc_action = get_action_from_acc (acc);
    if (!acc_action)
	return FALSE;

    acc_count = AccessibleAction_getNActions (acc_action);

    for (i = 0; !rv && i < acc_count; ++i)
    {
        gchar *tmp;
        tmp = AccessibleAction_getKeyBinding (acc_action, i);

	if (tmp && tmp[0])
	{
	    gchar *tmp2 = tmp;
	    gchar *tmp3 = NULL;
    
	    tmp2 = strstr (tmp2, ";");
	    if (tmp2)
		tmp3 = strstr (tmp2 + 1, ";");
	    if (tmp2 && tmp3)
	    {
	        *tmp3 = '\0';
	        *accelerator = prel_key_binding (tmp2 + 1);
	        if (*accelerator)
		    rv = TRUE;
		*tmp3 = ';';
	    }
	}
	SPI_freeString (tmp);
    }
    AccessibleAction_unref (acc_action);
    
    return rv;
}


gboolean
sro_text_get_attributes_at_index (SRObject *obj,
				  SRLong index,
				  SRTextAttribute **index_attr,
				  SRLong index_obj)
{
    Accessible *acc;
    AccessibleText *acc_text;
    SRLong offset, start, end;
    if (index_attr)
	*index_attr = NULL;
    
    srl_return_val_if_fail (obj && index_attr, FALSE);
    srl_return_val_if_fail (sro_is_text (obj, index_obj), FALSE);

    acc = sro_get_acc_at_index (obj, index_obj);
    if (!acc)
	return FALSE;
    acc_text = get_text_from_acc (acc);
    if (!acc_text)
	return FALSE;
    
    offset = AccessibleText_getCaretOffset (acc_text);
    get_text_range_from_offset (acc_text, SR_TEXT_BOUNDARY_LINE, offset, &start, &end);
    if (index <= end - start)
    {
	if (index + start == AccessibleText_getCharacterCount (acc_text))
	{
	    *index_attr = NULL;
	}
	else
	{
	    index += start;
	    get_text_attributes_from_range (acc_text, index, index + 1, index_attr);
	}
    }
    AccessibleText_unref (acc_text);
    return *index_attr ? TRUE : FALSE;    
}

gboolean
sro_text_get_location_at_index (SRObject *obj,
				  SRLong index,
				  SRRectangle *location,
				  SRLong index_obj)
{
    gboolean rv = FALSE;
    Accessible *acc;
    AccessibleText *acc_text;
    SRLong offset, start, end;
    
    srl_return_val_if_fail (obj && location, FALSE);
    srl_return_val_if_fail (sro_is_text (obj, index_obj), FALSE);
    
    acc = sro_get_acc_at_index (obj, index_obj);
    if (!acc)
	return FALSE;
    acc_text = get_text_from_acc (acc);
    if (!acc_text)
	return FALSE;
    
    offset = AccessibleText_getCaretOffset (acc_text);
    get_text_range_from_offset (acc_text, SR_TEXT_BOUNDARY_LINE, offset, &start, &end);
    if (index <= end - start)
    {
        if (index + start == AccessibleText_getCharacterCount (acc_text))
	{
	    location->x = -1;
	    location->y = -1;
	    location->height = 0;
	    location->width = 0;
	}
	else
	{
	    long int x, y, height,width;
	    index += start;
	    AccessibleText_getCharacterExtents (acc_text, index, &x, &y, 
		    &width, &height, SPI_COORD_TYPE_SCREEN);
	    location->x = x;
	    location->y = y;
	    location->height = height;
	    location->width = width;
	    rv = TRUE;
	}
    }
    
    AccessibleText_unref (acc_text);
    
    return rv;
}


gboolean
sro_text_get_char_at_index (SRObject *obj,
			    SRLong index,
			    gchar *chr,
			    SRLong index_obj)
{
    Accessible *acc;
    AccessibleText *acc_text;
    SRLong offset, start, end;
    if (chr)
	*chr = '\0';    
    srl_return_val_if_fail (obj && chr, FALSE);
    srl_return_val_if_fail (sro_is_text (obj, index_obj), FALSE);

    acc = sro_get_acc_at_index (obj, index_obj);
    if (!acc)
	return FALSE;
    acc_text = get_text_from_acc (acc);
    if (!acc_text)
	return FALSE;
    
    offset = AccessibleText_getCaretOffset (acc_text);
    get_text_range_from_offset (acc_text, SR_TEXT_BOUNDARY_LINE, offset, &start, &end);
    if (index <= end - start)
    {
	if (index + start == AccessibleText_getCharacterCount (acc_text))
	{
	    *chr = '\0';
	}
	else
	{
	    gchar *tmp;
	    tmp = AccessibleText_getText (acc_text, index, index + 1);
	    *chr = tmp[0];
	    SPI_freeString (tmp);
	}
    }
    AccessibleText_unref (acc_text);
    
    return *chr ? TRUE : FALSE;
}

gboolean 
sro_tree_item_get_level (SRObject *obj,
			 SRLong *level,
			 SRLong index)
{
    Accessible *parent;
    if (level)
	*level = 0;
    srl_return_val_if_fail (obj && level, FALSE);
    srl_return_val_if_fail (obj->role == SR_ROLE_TREE_ITEM, FALSE);

    parent = sro_get_acc_at_index (obj, index);;
    Accessible_ref (parent);
    while (parent && (Accessible_getRole (parent) != SPI_ROLE_TREE))
    {
        Accessible *tmp = Accessible_getParent (parent);
        Accessible_unref (parent);
        parent = tmp;
	(*level)++;
    }

    if (parent)
	Accessible_unref (parent);
    return TRUE;
}


gboolean 
sro_text_get_selections (SRObject *obj,
			 gchar ***selections,
			 SRLong index)
{
    Accessible *acc;
    AccessibleText *acc_text;
    long n_sel;
    if (selections)
	*selections = NULL;
            
    srl_return_val_if_fail (obj && selections, FALSE);
    srl_return_val_if_fail (sro_is_text (obj, index), FALSE);

    acc = sro_get_acc_at_index (obj, index);
    if (!acc)
	return FALSE;
    acc_text = get_text_from_acc (acc);
    if (!acc_text)
	return FALSE;

    n_sel = AccessibleText_getNSelections (acc_text);
    if (n_sel > 0)
    {
	long i;
	*selections = (gchar**) g_malloc ((n_sel + 1) * sizeof (gchar*));
	for (i = 0; i < n_sel; i++)
	{
	    gchar *tmp;
	    long int start, end;
	    AccessibleText_getSelection (acc_text, i, &start, &end);
	    tmp = AccessibleText_getText (acc_text, start, end);
	    (*selections)[i] = SR_strdup (tmp);
	    SPI_freeString (tmp);
	}
	(*selections)[n_sel] = NULL;
    }

    AccessibleText_unref (acc_text);    
    return *selections ? TRUE : FALSE;    
}


gboolean 
sro_text_get_difference (SRObject *obj,
			 gchar **difference,
			 SRLong index)
{
    
    if (difference)
	*difference = NULL;
            
    srl_return_val_if_fail (obj && difference, FALSE);
    srl_return_val_if_fail (sro_is_text (obj, index), FALSE);

    if (obj->text)
	*difference = SR_strdup (obj->text);
    return *difference ? TRUE : FALSE;    
}


gboolean
sro_get_objs_for_relation (SRObject *obj,
			   SRRelation type,
			   SRObject ***targets,
			   SRLong index)
{
    AccessibleRelationType acc_type = SPI_RELATION_NULL;
    AccessibleRelation **acc_relation;
    GSList *list;
    Accessible *acc;
    int i;
    if (targets)
	*targets = NULL;
    
    srl_return_val_if_fail (obj && targets, FALSE);

    acc = sro_get_acc_at_index (obj, index);
    if (!acc)
	return FALSE;
    if ((get_relation_from_acc(acc) & type) != type)
	return FALSE;
    
    switch (type)
    {
	case SR_RELATION_CONTROLLED_BY:
	    acc_type = SPI_RELATION_CONTROLLED_BY;
	    break;
	case SR_RELATION_CONTROLLER_FOR:
	    acc_type = SPI_RELATION_CONTROLLER_FOR;
	    break;
	case SR_RELATION_MEMBER_OF:
	    acc_type = SPI_RELATION_MEMBER_OF;
	    break;
	case SR_RELATION_EXTENDED:
	    acc_type = SPI_RELATION_EXTENDED;
	    break;
	default:
	    srl_assert_not_reached ();
	    break;
    }
    
    
    acc_relation = Accessible_getRelationSet (acc);
    if (!acc_relation)
	return FALSE;
    
    list = NULL;
    for (i = 0; acc_relation[i]; i++) 
    {
	if (acc_type ==AccessibleRelation_getRelationType (acc_relation[i]))
	{
	    int n_rel, j;
	    n_rel = AccessibleRelation_getNTargets (acc_relation[i]);
	    for (j = 0; j < n_rel; j++)
	    {
		Accessible *dest;
		dest = AccessibleRelation_getTarget (acc_relation[i], j);
		if (dest)
		{
		    SRObject *obj_;
		    sro_get_from_accessible (dest, &obj_, SR_OBJ_TYPE_VISUAL);
		    list = g_slist_append (list, obj_);
		    Accessible_unref (dest);
		}
	    }	    
	}
	AccessibleRelation_unref (acc_relation[i]);
    }
    g_free (acc_relation);
    
    if (g_slist_length (list) != 0)
    {
	*targets = (SRObject **) g_malloc ((g_slist_length (list) + 1) * sizeof (SRObject*));
	for (i = 0; i < g_slist_length (list); i++)
    	    (*targets)[i] = (SRObject*) g_slist_nth_data (list, i);
	(*targets)[g_slist_length (list)] = NULL;
    }
    
    return TRUE;
}

gboolean
sro_get_index_in_group (SRObject *obj,
			SRLong *index,
			SRLong index_obj)
{
    AccessibleRelation **acc_relation;
    Accessible *acc;
    int i;
    if (index)
	*index = -1;    
    srl_return_val_if_fail (obj &&index, FALSE);
    
    acc = sro_get_acc_at_index (obj, index_obj);
    if (!acc)
	return FALSE;
    if ((get_relation_from_acc(acc) & SR_RELATION_MEMBER_OF) == SR_RELATION_MEMBER_OF)
	return FALSE;
    
    acc_relation = Accessible_getRelationSet (obj->acc);
    if (!acc_relation)
	return FALSE;
    
    for (i = 0; acc_relation[i]; i++) 
    {
	if (AccessibleRelation_getRelationType (acc_relation[i]) == SPI_RELATION_MEMBER_OF)
	{
	    int n_rel, j;
	    n_rel = AccessibleRelation_getNTargets (acc_relation[i]);
	    for (j = 0; j < n_rel; j++)
	    {
		Accessible *dest;
		dest = AccessibleRelation_getTarget (acc_relation[i], j);
		if (acc == dest)
		    *index = j;
		if (dest)
		    Accessible_unref (dest);
	    }	    
	}
	AccessibleRelation_unref (acc_relation[i]);
    }
    g_free (acc_relation);
    
    return TRUE;
}

static gboolean
srl_acc_manages_descendants (Accessible *acc) 
{
    gboolean rv = FALSE;
    AccessibleStateSet *states = Accessible_getStateSet (acc);

    if (states)
    {
	rv = AccessibleStateSet_contains (states, SPI_STATE_MANAGES_DESCENDANTS);
	AccessibleStateSet_unref (states);
    }

    return rv;
}

static gboolean
get_acc_child_with_role_from_acc (Accessible *acc, 
		    		  GArray **array, 
		    		  AccessibleRole role,
		    		  gint level,
		    		  gboolean stop/* stop if find one */)
{
    int i;
    int count;
    
    srl_return_val_if_fail (acc && array && *array, FALSE);
    srl_return_val_if_fail (level >= -1, FALSE);
    
    if (level == 0)
	return TRUE;
	
    if (Accessible_getRole (acc) == role)
    {
	*array = g_array_append_val (*array, acc);
	Accessible_ref (acc);
    }
    
    if (stop && (*array)->len != 0)
	return TRUE;

    count = Accessible_getChildCount (acc);

    if (count > SRLOW_FEW_CHILDREN && srl_acc_manages_descendants (acc))
        return TRUE;

    for (i = 0; i < count; i++)
    {
        Accessible *child;
        child = Accessible_getChildAtIndex (acc, i);
        if (child)
	{
    	    get_acc_child_with_role_from_acc (child, array, role, (level == -1) ? -1 : level - 1, stop);
	    Accessible_unref (child);
	}
    }
    
    return TRUE;
}

static gboolean
get_acc_with_role_from_main_widget (Accessible *acc, 
			       GArray **array, 
			       AccessibleRole role,
			       gint level,
			       gboolean stop /* stop if find one */)
{    
    srl_return_val_if_fail (acc && array && *array, FALSE);
    srl_return_val_if_fail (level >= 0 || level == -1, FALSE);
    
#ifdef SRL_DEBUG
    {
	Accessible *parent;
	parent = Accessible_getParent (acc);
	if (parent)
	{
	    srl_assert (Accessible_isApplication (parent));
	    Accessible_unref (parent);
	}
    }
#endif

    get_acc_child_with_role_from_acc (acc, array, role, level, stop);

    return TRUE;
}




static Accessible *
get_menu_from_main_widget (Accessible *acc)
{
    Accessible *menu = NULL;
    GArray *array;

    srl_return_val_if_fail (acc, FALSE);
        
    array = g_array_sized_new (FALSE, FALSE, sizeof (Accessible*), 1);
    get_acc_with_role_from_main_widget (acc, &array, SPI_ROLE_MENU_BAR, -1, TRUE);
        
    if (array->len == 1)
	menu = g_array_index (array, Accessible *, 0);
	
    g_array_free (array, TRUE);
    
    return menu;
}



static Accessible *
get_toolbar_from_main_widget (Accessible *acc)
{
    Accessible *toolbar = NULL;
    GArray *array;

    srl_return_val_if_fail (acc, FALSE);
        
    array = g_array_sized_new (FALSE, FALSE, sizeof (Accessible*), 1);
    get_acc_with_role_from_main_widget (acc, &array, SPI_ROLE_TOOL_BAR, -1, TRUE);
        
    if (array->len == 1)
	toolbar = g_array_index (array, Accessible *, 0);
    
    g_array_free (array, TRUE);
    
    if (toolbar)
    {
	if ((get_state_from_acc (toolbar) & SR_STATE_VISIBLE) != SR_STATE_VISIBLE)
	{
	    Accessible_unref (toolbar);
	    toolbar = NULL;
	}
    }
    
    return toolbar;
}

static gboolean
get_statusbar_from_main_widget (Accessible *acc, GArray **status)
{
    srl_return_val_if_fail (acc && status && *status, FALSE);
    
    get_acc_with_role_from_main_widget (acc, status, SPI_ROLE_STATUS_BAR, -1, FALSE);

    return TRUE;
}


static gboolean
acc_has_location (Accessible *acc,
		  AccessibleCoordType type,
		  SRRectangle *location)
{
    SRRectangle location_;
    srl_return_val_if_fail (acc, FALSE);
    srl_return_val_if_fail (location, FALSE);

    get_location_from_acc (acc, type, &location_);
    
    return (   location->x == location_.x 
	    && location->y == location_.y
	    && location->width == location_.width
	    && location->height == location_.height) ? TRUE : FALSE;
}

static Accessible *
get_parent_with_location (Accessible *acc,
			  AccessibleCoordType type,
			  SRRectangle *location)
{
    Accessible *parent;
    
    srl_return_val_if_fail (acc, FALSE);
    srl_return_val_if_fail (location, FALSE);
    
    parent = NULL;
    Accessible_ref (acc);
    while (!parent)
    {
	Accessible *tmp;
	
	if (!Accessible_isComponent (acc))
	    break;

	if (acc_has_location (acc, SPI_COORD_TYPE_SCREEN, location))
	{
	    parent = acc;
	    Accessible_ref (parent);
	}

	tmp = Accessible_getParent (acc);
	Accessible_unref (acc);
	acc = tmp;
    }
    Accessible_unref (acc);
    return parent;
}    

static Accessible *
get_statusbar_parent (GArray *status)
{
    Accessible *child, *parent = NULL;
    SRRectangle location;
    
    srl_return_val_if_fail (status && status->len > 0, FALSE);
    	    
    get_location_from_array_of_acc (status, SPI_COORD_TYPE_SCREEN, &location);
        		
    child = g_array_index (status, Accessible *, 0);
    if (child)
	parent = get_parent_with_location (child, SPI_COORD_TYPE_SCREEN, &location);
    
    return parent;
}

static Accessible *
get_main_widget_from_acc (Accessible *acc)
{
    Accessible *crt;
    
    srl_return_val_if_fail (acc, FALSE);
    srl_return_val_if_fail (!Accessible_isApplication (acc), FALSE);
    crt = acc;
    Accessible_ref (crt);
    for ( ; ; )
    {
	Accessible *parent;
	parent = Accessible_getParent (crt);
	if (parent && Accessible_isApplication (parent))
	{
	    Accessible_unref (parent);
	    return crt;
	}
	if (!parent)
	{
	    srl_warning ("no object wich is application in parent line");
	    return crt;
	}
	Accessible_unref (crt);
	crt = parent;
    }
    srl_assert_not_reached ();
    return NULL;
}


extern Accessible * srl_last_edit;

gboolean 
sro_get_sro (SRObject *obj1,
	     SRNavigationDir dir,
	     SRObject **obj2,
	     SRNavigationMode mode)
{
    if (obj2)
        *obj2 = NULL;
    srl_return_val_if_fail (obj1 && obj2, FALSE);
    
    switch (dir)
    {
	case SR_NAV_NEXT:
	case SR_NAV_PREV:
	    {
	    	Accessible *parent;
		long index = -1;
		parent = Accessible_getParent (obj1->acc);
		if (parent)
		{
		    if (Accessible_isApplication (parent))
		    {
			switch (mode)
			{
			    case SR_NAV_MODE_WINDOW:
				Accessible_unref (parent);
				parent = NULL;
				break;
			    case SR_NAV_MODE_APPLICATION:
				index = Accessible_getIndexInParent (obj1->acc);
				index += dir == SR_NAV_NEXT ? 1 : -1;
				break;
			    case SR_NAV_MODE_DESKTOP:
				index = Accessible_getIndexInParent (obj1->acc);
				index += dir == SR_NAV_NEXT ? 1 : -1;
				if (0 > index || index >= Accessible_getChildCount (parent))
				{
				    gint desktop_cnt, i;
				    desktop_cnt = SPI_getDesktopCount ();

				    for (i = 0; i < desktop_cnt; i++)
				    {
					Accessible *desktop;
					gint j, cnt;
					desktop = SPI_getDesktop (i);
					if (!desktop)
					    continue;
					cnt = Accessible_getChildCount (desktop);
					for (j = 0; j < cnt; j++)
					{
					    Accessible *child;
					    child = Accessible_getChildAtIndex (desktop, j);
					    if (!child)
						continue;
					    if (child == parent)
					    {
						if (index == -1)
						{
						    if (j == 0)
						    {
							Accessible_unref (parent);
							parent = NULL;
						    }
						    else
						    {
							parent = Accessible_getChildAtIndex (desktop, j - 1);
							index = Accessible_getChildCount (parent) - 1;
						    }
						}
						else
						{
						    if (j == cnt - 1)
						    {
						    	Accessible_unref (parent);
							parent = NULL;
						    }
						    else
						    {
							parent = Accessible_getChildAtIndex (desktop, j + 1);
							index = 0;
						    }
						    
						}
						break;
					    }
					    Accessible_unref (child);
					}					
					Accessible_unref (desktop);
				    }
				}
				break;
			    default:
				srl_assert_not_reached ();
				break;
			}
		    }
		    else
		    {
		    	index = Accessible_getIndexInParent (obj1->acc);
			index += dir == SR_NAV_NEXT ? 1 : -1;
		    }
		}
		if (parent)
		{
		    if (0 <= index && index < Accessible_getChildCount (parent))
		    {
			Accessible *next;
			next = Accessible_getChildAtIndex (parent, index);
			if (next)
			{
			    sro_get_from_accessible (next, obj2, SR_OBJ_TYPE_VISUAL);
			    Accessible_unref (next);
			}
		    }
		    Accessible_unref (parent);
		}		
	    }
	    break;
	case SR_NAV_FIRST:
	case SR_NAV_LAST:
	    {
	    	Accessible *parent;
		long index = -1;
		parent = Accessible_getParent (obj1->acc);
		if (parent)
		{
		    if (Accessible_isApplication (parent))
		    {
			switch (mode)
			{
			    case SR_NAV_MODE_WINDOW:
				Accessible_unref (parent);
				parent = NULL;
				break;
			    case SR_NAV_MODE_APPLICATION:
				index = dir == SR_NAV_FIRST ? 0 : Accessible_getChildCount (parent) - 1;
				if (index == Accessible_getIndexInParent (obj1->acc))
				{
				    Accessible_unref (parent);
				    parent = NULL;
				}
				break;
			    case SR_NAV_MODE_DESKTOP:
				Accessible_unref (parent);
				parent = NULL;
				if (dir == SR_NAV_FIRST)
				{
				    Accessible *desktop;
				    desktop = SPI_getDesktop (0);
				    if (desktop)
				    {
					Accessible *app;
					app = Accessible_getChildAtIndex (desktop, 0);
					if (app)
					{
					    Accessible *child;
					    child = Accessible_getChildAtIndex (app, 0);
					    if (child)
					    {
						if (child != obj1->acc)
						{
						    parent = app;
						    Accessible_ref (parent);
						    index = 0;
						}
						Accessible_unref (child);
					    }
					    Accessible_unref (app);
					}
					Accessible_unref (desktop);
				    }				
				}
				else if (dir == SR_NAV_LAST)
				{
				    Accessible *desktop;
				    /*desktop = SPI_getDesktop (SPI_getDesktopCount () - 1);*/
				    /* for now no support for multiple desktop */
				    desktop = SPI_getDesktop (0);
				    if (desktop)
				    {
					Accessible *app;
					app = Accessible_getChildAtIndex (desktop, Accessible_getChildCount (desktop) - 1);
					if (app)
					{
					    Accessible *child;
					    child = Accessible_getChildAtIndex (app, Accessible_getChildCount (app) - 1);
					    if (child)
					    {
						if (child != obj1->acc)
						{
						    parent = app;
						    Accessible_ref (parent);
						    index = Accessible_getChildCount (app) - 1;
						}
						Accessible_unref (child);
					    }
					    Accessible_unref (app);
					}
					Accessible_unref (desktop);
				    }				
				}
				else
				    srl_assert_not_reached ();
				break;
			    default:
				srl_assert_not_reached ();
			}
		    }
		    else
		    {
			index = dir == SR_NAV_FIRST ? 0 : Accessible_getChildCount (parent) - 1;
			if (index == Accessible_getIndexInParent (obj1->acc))
			{
			    Accessible_unref (parent);
			    parent = NULL;
			}
		    }
		}
		if (parent)
		{
		    if (0 <= index && index < Accessible_getChildCount (parent))
		    {
			Accessible *next;
			next = Accessible_getChildAtIndex (parent, index);
			if (next)
			{
			    sro_get_from_accessible (next, obj2, SR_OBJ_TYPE_VISUAL);
			    Accessible_unref (next);
			}
		    }
		    Accessible_unref (parent);
		}		
	    }
	    break;
	
	case SR_NAV_PARENT:
	    {
		Accessible *parent;
		parent = Accessible_getParent (obj1->acc);
		if (parent)
		{
		    if (!Accessible_isApplication (parent))
			sro_get_from_accessible (parent, obj2, SR_OBJ_TYPE_VISUAL);	
		    Accessible_unref (parent);
		}
	    }
	    break;
	case SR_NAV_CHILD:
	    {
		long cc;
		cc = Accessible_getChildCount (obj1->acc);
		if (cc)
		{
		    Accessible *child;
		    child = Accessible_getChildAtIndex (obj1->acc, 0);
		    if (child)
		    {
			sro_get_from_accessible (child, obj2, SR_OBJ_TYPE_VISUAL);
			Accessible_unref (child);
		    }
		}
	    }
	    break;
	case SR_NAV_TITLE:
	    {
		Accessible *main_widget;
		main_widget = get_main_widget_from_acc (obj1->acc);
		if (main_widget)
		{
		    *obj2 = sro_new ();
		    (*obj2)->acc = main_widget;
		    (*obj2)->role = SR_ROLE_TITLE_BAR;
		    get_sro_children (*obj2);
		}
	    }
	    break;
	case SR_NAV_WINDOW:
	    {
		Accessible *main_widget;
		main_widget = get_main_widget_from_acc (obj1->acc);
		if (main_widget)
		{
		    sro_get_from_accessible (main_widget, obj2, SR_OBJ_TYPE_VISUAL);
		    Accessible_unref (main_widget);
		}
	    }
	    break;
	case SR_NAV_MENU:
	    {
	    	Accessible *main_widget, *menu = NULL;
		main_widget = get_main_widget_from_acc (obj1->acc);
		if (main_widget)
		{
		    menu = get_menu_from_main_widget (main_widget);
		    Accessible_unref (main_widget);
		}
		if (menu)
		{
		    sro_get_from_accessible (menu, obj2, SR_OBJ_TYPE_VISUAL);
		    Accessible_unref (menu);
		}
	    }
	    break;
	case SR_NAV_GROUP:
	    break;
	case SR_NAV_TOOLBAR:
	    {
		Accessible *main_widget, *toolbar = NULL;
		main_widget = get_main_widget_from_acc (obj1->acc);
		if (main_widget)
		{
		    toolbar = get_toolbar_from_main_widget (main_widget);
		    Accessible_unref (main_widget);
		}
		if (toolbar)
		{
		    sro_get_from_accessible (toolbar, obj2, SR_OBJ_TYPE_VISUAL);
		    Accessible_unref (toolbar);
		}
	    }
	    break;
	case SR_NAV_STATUSBAR:
	    {
		Accessible *main_widget;
		GArray *status;
		
		main_widget = get_main_widget_from_acc (obj1->acc);
		if (main_widget)
		{
		    status = g_array_sized_new (FALSE, FALSE, sizeof (Accessible*), 5);
		    get_statusbar_from_main_widget (main_widget, &status);
		    Accessible_unref (main_widget);
		}
		if (status->len != 0)
		{
		    *obj2 = sro_new ();
		    (*obj2)->acc = get_statusbar_parent (status);
		    (*obj2)->role = SR_ROLE_STATUS_BAR;
		    (*obj2)->children = status;
		}
		else
		{
		    g_array_free (status, TRUE);
		}
	    }
	    break;
	case SR_NAV_CARET:
	    if (srl_last_edit)
		sro_get_from_accessible (srl_last_edit, obj2, SR_OBJ_TYPE_VISUAL);
	    break;

    }
    if (*obj2)
	(*obj2)->reason = SR_strdup ("present");
    return *obj2 ? TRUE : FALSE;
}


gboolean
sro_set_difference (SRObject *obj,
		    gchar *difference)
{
    srl_return_val_if_fail (obj, FALSE);
    
    if (difference)
	obj->text = SR_strdup (difference);
    return TRUE;
}		 

gboolean
sro_set_name (SRObject *obj,
	      gchar *name)
{
    srl_return_val_if_fail (obj, FALSE);
    
    if (name)
	obj->name = SR_strdup (name);
    return TRUE;
}		 

static gboolean
add_role (GArray *array, gchar *role)
{
    gint i;
    SRRoleCnt *role_cnt;
    srl_return_val_if_fail (array, FALSE);
    
    for (i = 0; i < array->len; i++)
    {
	role_cnt = g_array_index (array, SRRoleCnt *, i);
	if (role_cnt && strcmp (role_cnt->role, role) == 0)
	{
	    role_cnt->cnt++;
	    return TRUE;
	}
    }
    role_cnt = (SRRoleCnt *) g_malloc (sizeof (SRRoleCnt));
    if (!role_cnt)
	return FALSE;
    role_cnt->role = SR_strdup (role);
    role_cnt->cnt = 1;
    g_array_append_val (array, role_cnt);
    
    return TRUE;    
}

static gboolean
acc_has_stop_role (Accessible *acc)
{
    static SRObjectRoles stop_role[] =
	{
	    SR_ROLE_TREE_TABLE,
	    SR_ROLE_PAGE_TAB_LIST,
	    SR_ROLE_STATUS_BAR,
	    SR_ROLE_TOOL_BAR,
	    SR_ROLE_MENU_BAR,
	    SR_ROLE_TEXT_SL,
	    SR_ROLE_TEXT_ML,
	    SR_ROLE_LABEL,
	    SR_ROLE_TABLE,
	    SR_ROLE_PUSH_BUTTON,
	    SR_ROLE_CHECK_BOX,
	    SR_ROLE_SLIDER,
	    SR_ROLE_COMBO_BOX,
	    SR_ROLE_SPIN_BUTTON,
	    SR_ROLE_LIST,
	    SR_ROLE_RADIO_BUTTON,
	};
    gint i;
    SRObjectRoles role;
    
    srl_return_val_if_fail (acc, FALSE);
    role = get_role_from_acc (acc, SR_OBJ_TYPE_VISUAL);
    
    for (i = 0; i < G_N_ELEMENTS (stop_role); i++)
    {
	if (role == stop_role[i])
	    return TRUE;
    }
    
    return FALSE;
} 
    

static gboolean
sro_get_surroundings_from_acc (Accessible *acc,
		    	       GArray **surroundings)
{
    SRLong cnt, i;
    srl_return_val_if_fail (acc && surroundings && *surroundings, FALSE);

    cnt = Accessible_getChildCount (acc);

    if (cnt <= SRLOW_FEW_CHILDREN || !srl_acc_manages_descendants (acc))
    {
        for (i = 0; i < cnt; i++)
	{
	    Accessible *child;
	    child = Accessible_getChildAtIndex (acc, i);
	    if (child)
	    {
	        SRObjectRoles role;
		role = get_role_from_acc (child, SR_OBJ_TYPE_VISUAL);
		
		if (acc_has_stop_role (child))
	        {
		    gchar *tmp;
		    tmp = Accessible_getRoleName (child);
		    if (tmp)
		        add_role (*surroundings, tmp);
		    SPI_freeString (tmp);
		}
		else
		    sro_get_surroundings_from_acc (child, surroundings);
		Accessible_unref (child);
	    }
	}
    }
    return TRUE;
}


    
gboolean
sro_get_surroundings (SRObject *obj,
		      GArray **surroundings)
{
    Accessible *acc, *widget;
    gchar *role;
    
    if (surroundings)
	*surroundings = NULL;

    srl_return_val_if_fail (obj && surroundings, FALSE);

    *surroundings = g_array_new (TRUE, TRUE, sizeof (SRRoleCnt *));
    if (!(*surroundings))
	return FALSE;

    acc = obj->acc;
    widget = get_main_widget_from_acc (acc);
    if (!widget)
    {
	g_array_free (*surroundings, FALSE);
	*surroundings = NULL;
	return FALSE;
    }

    role = Accessible_getRoleName (widget);
    if (role)
	add_role (*surroundings, role);
    else
	add_role (*surroundings, "unknown");
    SPI_freeString (role);    
    
    sro_get_surroundings_from_acc (widget, surroundings);

    Accessible_unref (widget);
    
    return TRUE;
}

static gboolean
sro_get_hierarchy_from_acc (Accessible *acc,
		    	       GNode **hierarchy)
{
    SRLong cnt, i;
    SRObject *sro;
    GNode *crt;
    srl_return_val_if_fail (acc && hierarchy, FALSE);

    if (sro_get_from_accessible (acc, &sro, SR_OBJ_TYPE_VISUAL))
    {
	crt = g_node_new (sro);
        if (!crt)
	{
	    sro_release_reference (sro);
	    return FALSE;
	}
    }

    cnt = Accessible_getChildCount (acc);

    if ((cnt <= SRLOW_FEW_CHILDREN) || !srl_acc_manages_descendants (acc))
        /* if nchildren is small, it's more efficient not to check state */
    {
        for (i = 0; i < cnt; i++)
	{
	     Accessible *child;
	     child = Accessible_getChildAtIndex (acc, i);
	     if (child)
	     {
		 sro_get_hierarchy_from_acc (child, &crt);
		 Accessible_unref (child);
	     }
	}
    }
    
    if (*hierarchy)
	g_node_append (*hierarchy, crt);
    else
	*hierarchy = crt;
    
    return *hierarchy ? TRUE : FALSE;
}
    
gboolean
sro_get_window_hierarchy (SRObject *obj,
		      GNode **hierarchy)
{
    Accessible *acc, *widget;
        
    if (hierarchy)
	*hierarchy = NULL;

    srl_return_val_if_fail (obj && hierarchy, FALSE);

    acc = obj->acc;
    widget = get_main_widget_from_acc (acc);
    if (!widget)
	return FALSE;

    sro_get_hierarchy_from_acc (widget, hierarchy);

    Accessible_unref (widget);
    
    return *hierarchy ? TRUE : FALSE;
}



#define SR_RECT_IN		1
#define SR_RECT_OUT		2
#define SR_RECT_INTERSECT	3
#define SR_RECT_UNDEF		4


static gint
rect_rect_position (SRRectangle *rect1,
		     SRRectangle *rect2)
{
    gint cnt1_x, cnt1_y, cnt2_x, cnt2_y;
    srl_return_val_if_fail (rect1 && rect2, SR_RECT_UNDEF);
    
    cnt1_x = cnt1_y = cnt2_x = cnt2_y = 0;
    if (rect2->x <= rect1->x && rect1->x <= rect2->x + rect2->width)
	cnt1_x++;
    
    if (rect2->x <= rect1->x + rect1->width && rect1->x + rect1->width <= rect2->x + rect2->width)
	cnt1_x++;

    if (rect2->y <= rect1->y && rect1->y <= rect2->y + rect2->height)
	cnt1_y++;
    
    if (rect2->y <= rect1->y + rect1->height && rect1->y + rect1->height <= rect2->y + rect2->height)
	cnt1_y++;
    
    if (rect1->x <= rect2->x && rect2->x <= rect1->x + rect1->width)
        cnt2_x++;

    if (rect1->x <= rect2->x + rect2->width && rect2->x + rect2->width <= rect1->x + rect1->width)
	cnt2_x++;

    if (rect1->y <= rect2->y && rect2->y <= rect1->y + rect1->height)
	cnt2_y++;
    
    if (rect1->y <= rect2->y + rect2->height && rect2->y + rect2->height <= rect1->y + rect1->height)
	cnt2_y++;

    if (cnt1_x == 2 && cnt1_y == 2)
	return SR_RECT_IN;
    if (cnt1_x && cnt1_y)
	return SR_RECT_INTERSECT;
    if (cnt2_x && cnt2_y)
	return SR_RECT_INTERSECT;
    return SR_RECT_OUT;
}

static gboolean
get_sros_in_rectangle_from_acc (Accessible *acc, 
			       SRRectangle *area,
			       GArray **array,
			       gboolean intersect)
{
    SRLong cnt, i;
    gboolean recurse_down = TRUE;

    srl_return_val_if_fail (acc && area && array && *array, FALSE);
    
    cnt = Accessible_getChildCount (acc);
    /* fprintf (stderr, "5408: child-count %d", cnt); */
    for (i = 0; i < cnt; i++)
    {
        Accessible *child;
        child = Accessible_getChildAtIndex (acc, i);
        if (child)
	{
	    if (Accessible_isComponent (child))
	    {
		AccessibleStateSet *states;
		states = Accessible_getStateSet (child);
		if (states &&
		    AccessibleStateSet_contains (states, SPI_STATE_VISIBLE)    &&
		    !AccessibleStateSet_contains (states, SPI_STATE_ICONIFIED) &&
		    !AccessibleStateSet_contains (states, SPI_STATE_HORIZONTAL) &&		    
		    !AccessibleStateSet_contains (states, SPI_STATE_VERTICAL))
		{
		    AccessibleComponent *comp;
		    comp = Accessible_getComponent (child);
		    if (comp)
		    {
			long int x, y, width, height;
			gint rez;
			SRRectangle rect;
			SRObject *sro;
		
			AccessibleComponent_getExtents (comp , 
							&x, 
							&y, 
							&width, 
							&height, 
							SPI_COORD_TYPE_SCREEN);
/*
			fprintf (stderr, "\n%s %d : MDIZOrder %d",__FILE__,__LINE__,
				AccessibleComponent_getMDIZOrder (comp));						
*/			rect.x = x;
			rect.y = y;
			rect.width = width;
			rect.height = height;
			rez = rect_rect_position (&rect, area);
			if ((rez == SR_RECT_IN) ||
			    (rez == SR_RECT_INTERSECT && intersect))
			    if (sro_get_from_accessible (child, &sro, SR_OBJ_TYPE_VISUAL))
				g_array_append_val (*array, sro);
			
			AccessibleComponent_unref (comp);
		    }
		}
		if (states) 
		{
		    recurse_down = 
		      !AccessibleStateSet_contains (states, 
						    SPI_STATE_MANAGES_DESCENDANTS);
		    AccessibleStateSet_unref (states);
		}
	    }
	    if (recurse_down) 
	        get_sros_in_rectangle_from_acc (child, area, array, intersect);
	    Accessible_unref (child);
	}
    }
    return TRUE;
}

static gboolean
get_sros_in_rectangle_from_app (Accessible *app, 
			       SRRectangle *area,
			       GArray **array,
			       gboolean intersect)
{
    srl_return_val_if_fail (app && area && array && *array, FALSE);
    
    if (!Accessible_isApplication (app))
	return FALSE;

    get_sros_in_rectangle_from_acc (app, area, array, intersect);

    return TRUE;
}

static gboolean
get_sros_in_rectangle_from_desktop (Accessible *desktop, 
			       SRRectangle *area,
			       GArray **array,
			       gboolean intersect)
{
    SRLong cnt, i;
    srl_return_val_if_fail (desktop && area && array && *array, FALSE);
    
    cnt = Accessible_getChildCount (desktop);
    for (i = 0; i < cnt; i++)
    {
        Accessible *child;
        child = Accessible_getChildAtIndex (desktop, i);
        if (child)
	{
	    if (Accessible_isApplication (child))
		get_sros_in_rectangle_from_app (child, area, array, intersect);
	    else
		get_sros_in_rectangle_from_desktop (child, area, array, intersect);
	    Accessible_unref (child);
	}
    }
    return TRUE;
}

/* UNUSED ??? */
#ifdef NEVER
gboolean
sro_get_sros_from_rectangle (SRRectangle *area,
			    GArray **array,
			    gboolean intersect)
{
    gint desktop_cnt, i;
    if (array)
	*array = NULL;
    srl_return_val_if_fail (area && array, FALSE);

    *array = g_array_new (TRUE, TRUE, sizeof (SRObject *));
    if (!(*array))
	return FALSE;

    desktop_cnt = SPI_getDesktopCount ();

    for (i = 0; i < desktop_cnt; i++)
    {
	Accessible *desktop;
	desktop = SPI_getDesktop (i);
	if (desktop)
	{
	    get_sros_in_rectangle_from_desktop (desktop, area, array, intersect);
	    Accessible_unref (desktop);
	}
    }
    return TRUE;
}
#endif


typedef gboolean (*SRLMatchFunction) (Accessible *acc, gpointer data);
typedef gboolean (*SRLTraverseChildOfFunction) (Accessible *acc, gpointer data);


#define SRL_DIRECTION_NEXT	1
#define SRL_DIRECTION_PREVIOUS	2

#define SRL_TRAVERSE_CHILDREN	4
#define SRL_TRAVERSE_PARENT	8
#define SRL_TRAVERSE_SIBLINGS	16

#define SRL_SCOPE_WINDOW	32
#define SRL_SCOPE_APPLICATION	64
#define SRL_SCOPE_DESKTOP	128

static gboolean srl_stop_action = FALSE;

/* function travers all above or below children an index in parent */
static gboolean
srl_traverse_in_parent (Accessible *parent, 
			Accessible **ret,
			guint32 index,
			gint flags,
			SRLMatchFunction match_func,
			gpointer data1,
			SRLTraverseChildOfFunction trav_func,
			gpointer data2)
{
    gint32 i, start, end, step;
    srl_assert (parent && ret && match_func && trav_func);

/*
    fprintf (stderr, "\nNAME:%s --- %s", Accessible_getName (parent),
			Accessible_getRoleName (parent));
*/
    if (srl_stop_action)
	return FALSE;

    if (!trav_func (parent, data2))
	return FALSE;

    if (!(flags & SRL_TRAVERSE_SIBLINGS))
	return FALSE;

    start = index;
    end = flags & SRL_DIRECTION_PREVIOUS ? 0 : Accessible_getChildCount (parent);
    step = flags & SRL_DIRECTION_PREVIOUS ? -1 : 1;

    /* fprintf (stderr, "5596: start %d, end %d", start, end); */
    for (i = start; (i < end && step == 1) || (i >= end && step == -1); i += step)
    {
	Accessible *child;

	if (srl_stop_action)
	    break;

    	child = Accessible_getChildAtIndex (parent, i);
	if (!child)
	    continue;
	if (match_func (child, data1))
	{
	    *ret = child;
	    Accessible_ref (*ret);
	}
	
	if (!(*ret) && (flags & SRL_TRAVERSE_CHILDREN) && 
	    !srl_acc_manages_descendants (child))
	{
	    srl_traverse_in_parent (child, ret, 
			flags & SRL_DIRECTION_PREVIOUS ? Accessible_getChildCount (child) - 1 : 0,
			flags,
			match_func, data1,
			trav_func, data2);

	}

	Accessible_unref (child);
	if (*ret)
	    break;
    }
    return *ret ? TRUE : FALSE;
}

static gboolean
srl_traverse_application (Accessible *app,
	    		  Accessible **ret,
			  gint index,
	    		  gint flags,
	    		  SRLMatchFunction match_func,
	    		  gpointer data1,
	    		  SRLTraverseChildOfFunction trav_func,
	    		  gpointer data2)
{
    gboolean rv = FALSE;
    Accessible *desktop;
    guint32 cnt, i, start, end, step;

    srl_assert (app && ret && match_func && trav_func && 
			Accessible_isApplication (app));
    
    if (srl_stop_action)
	return FALSE;

    if (flags & SRL_SCOPE_WINDOW)
	return FALSE;

    if (!rv)
	rv = srl_traverse_in_parent (app, ret, 
		        flags & SRL_DIRECTION_PREVIOUS ? index - 1 : index + 1,
		        flags,
		        match_func, data1,
		        trav_func, data2);
    if (flags & SRL_SCOPE_APPLICATION)
	return rv;
    if (rv)
	return TRUE;

    desktop = SPI_getDesktop (0);
    if (!desktop)
        return FALSE;
    cnt = Accessible_getChildCount (desktop);
    for (i = 0; i < cnt; i++)
    {
        Accessible *appt;
        appt = Accessible_getChildAtIndex (desktop, i);
	Accessible_unref (appt);
	if (app == appt)
	    break;
    }
	    
    start = i;
    end = flags & SRL_DIRECTION_PREVIOUS ? 0 : cnt;
    step = flags & SRL_DIRECTION_PREVIOUS ? -1 : 1;
    for (i = start + step; (i < end && step == 1) || (i >= end && step == -1); i += step)
    {
        Accessible *appt;
	if (srl_stop_action)
	    break;
	appt = Accessible_getChildAtIndex (desktop, i);
	rv = srl_traverse_in_parent (appt, ret, 
		    flags & SRL_DIRECTION_PREVIOUS ? Accessible_getChildCount (app) - 1 : 0,
		    flags,
		    match_func, data1,
		    trav_func, data2);
	Accessible_unref (appt);
	if (rv)
	    break;
    }	
    
    Accessible_unref (desktop);
    return rv;
}


static gboolean
srl_traverse (Accessible *acc,
	      Accessible **ret,
	      gint flags,
	      SRLMatchFunction match_func,
	      gpointer data1,
	      SRLTraverseChildOfFunction trav_func,
	      gpointer data2)
{
    gboolean rv = FALSE;
    Accessible *parent;

    srl_assert (acc && ret && match_func && trav_func);
    
    if (srl_stop_action)
	return FALSE;

    if (!rv && (flags & SRL_TRAVERSE_CHILDREN) && !srl_acc_manages_descendants (acc))
    {
	rv = srl_traverse_in_parent (acc, ret, 
			    flags & SRL_DIRECTION_PREVIOUS ? Accessible_getChildCount (acc) - 1 : 0,
			    flags,
			    match_func, data1,
			    trav_func, data2);
    }

    if (!(flags & SRL_TRAVERSE_PARENT))
	return rv;

    parent = acc;
    Accessible_ref (parent);
    while (!rv)
    {
	guint32 index;
	Accessible *tmp;
	
	if (srl_stop_action)
	    break;

	tmp = parent;
	index = Accessible_getIndexInParent (parent);
	parent = Accessible_getParent (parent);
	Accessible_unref (tmp);
	if (Accessible_isApplication (parent))
	{
	    rv = srl_traverse_application (parent, ret, index, flags,
	    		  match_func, data1,
	    		  trav_func, data2);
	}    
	else if (!srl_acc_manages_descendants (acc))
	{
	    rv = srl_traverse_in_parent (parent, ret, 
			    flags & SRL_DIRECTION_PREVIOUS ? index - 1 : index + 1,
			    flags,
			    match_func, data1,
			    trav_func, data2);	    
	}
	if (!parent || rv || Accessible_isApplication (parent))
	    break;
    }

    if (parent)
        Accessible_unref (parent);

    return rv;
}


#define SRL_CASE_SENSITIVE	1
#define SRL_CASE_OR		2
#define SRL_CASE_AND		4
#define SRL_CASE_BOLD		8
#define SRL_CASE_ITALIC		16
#define SRL_CASE_UNDERLINE	32
#define SRL_CASE_SELECTED	64
#define SRL_CASE_STRIKETHROUGH	128


static gboolean
srl_find_string (gchar *text1,
		 gchar *text2,
		 gint flags, 
		 SRLong *index)
{
    gchar *index_;
    gchar *s1, *s2;
    
    srl_assert (text1 && text2 && index);

    if (flags & SRL_CASE_SENSITIVE)
    {
	s1 = g_strdup (text1);
	s2 = g_strdup (text2);
    }
    else
    {
	s1 = g_utf8_strup (text1, -1);
	s2 = g_utf8_strup (text2, -1);
    }

    index_ = strstr (s1, s2);
    *index = index_ - s1;

    g_free (s1);
    g_free (s2);

    return index_ ? TRUE : FALSE;
}

static gboolean
srl_acc_has_real_text (Accessible *acc,
		       gchar *text,
		       gint flags,
		       SRLong *index)
{
    gboolean rv = FALSE;
    long int x1, y1, x2, y2, y;
    long cnt;
    AccessibleText *acc_text;
    AccessibleComponent *acc_comp;
    
    srl_assert (acc &&text && index);
    
    if (srl_stop_action)
	return FALSE;

    acc_text = Accessible_getText (acc);
    acc_comp = Accessible_getComponent (acc);
    
    cnt = AccessibleText_getCharacterCount (acc_text);
    AccessibleComponent_getExtents (acc_comp, &x1, &y1, &x2, &y2, 
				SPI_COORD_TYPE_SCREEN);
    x2 += x1;
    y2 += y1;
    
    y = y1;
    while (y < y2 && !rv)
    {
	long start, end;
	long int xt, yt, wt, ht;
	gchar *text2;
	SRLong index_;
	if (srl_stop_action)
	    break;
	start = AccessibleText_getOffsetAtPoint (acc_text, x1, y,
			    SPI_COORD_TYPE_SCREEN);
	end   = AccessibleText_getOffsetAtPoint (acc_text, x2, y,
			    SPI_COORD_TYPE_SCREEN);
	AccessibleText_getCharacterExtents (acc_text, start, &xt, &yt, &wt, &ht,
			    SPI_COORD_TYPE_SCREEN);
	start = MAX (start, *index);
	end = MAX (end, *index);
	
	text2 = NULL;
	if (start < end)
	    text2 = AccessibleText_getText (acc_text, start, end);
	if (text2)
	    rv = srl_find_string (text2, text, flags, &index_);
	if (rv)
	    *index = start + index_;
/*	if (rv)
	    fprintf (stderr, "\nSTART:%ld\nEND:%ld\nFIND:%ld\nNEW:%ld", start, end, index_, *index);
*/	SPI_freeString (text2);
	y += ht;
	if (end >=cnt)
	    break;
	/* DR: how to avoid case in which app crashes */
    }

    if (acc_text)
	AccessibleText_unref (acc_text);
    if (acc_comp)
	AccessibleComponent_unref (acc_comp);
    return rv;
}


static gboolean
srl_acc_has_name (Accessible *acc,
		  gchar *text,
		  gint flags,
		  SRLong *index)
{
    gboolean rv = FALSE;
    gchar *name;
    SRLong index_;
    
    srl_assert (acc &&text && index);
    
    if (srl_stop_action)
	return FALSE;

    name = Accessible_getName (acc);
    if (name)
	rv = srl_find_string (g_utf8_offset_to_pointer (name, *index), text, flags, &index_);
    if (rv)
	*index += index_;

    SPI_freeString (name);
    
    return rv;
}

typedef struct
{
    gchar *text;
    SRLong start;
    gint flags;
}SRLTextFind;

static gboolean
srl_acc_has_text (Accessible *acc,
		  gpointer data)
{
    gboolean rv = FALSE;
    SRLTextFind* ft;
    
    ft = (SRLTextFind*)data;
    srl_assert (acc && ft && ft->text);

    if (Accessible_isText (acc))
	rv = srl_acc_has_real_text (acc, ft->text, ft->flags, &ft->start);
    else
    	rv = srl_acc_has_name (acc, ft->text, ft->flags, &ft->start);

    return rv;
}

static gboolean
srl_acc_has_image (Accessible *acc,
	    	   gpointer data)
{
    gboolean rv = FALSE;
    AccessibleImage *image;
    Accessible *parent;
    AccessibleComponent *comp;
    srl_assert (acc);

    if (!(get_specialization_from_acc (acc) & SR_IS_IMAGE))
	return FALSE;
	
    image = get_image_from_acc (acc);
    parent = Accessible_getParent (acc);
    comp = NULL;
    if (parent)
	comp = Accessible_getComponent (parent);
    if (image && comp)
    {
	long int x1, y1, w1, h1, x2, y2, w2, h2;
	AccessibleImage_getImageExtents (image, &x2, &y2, &w2, &h2, SPI_COORD_TYPE_SCREEN);
	AccessibleComponent_getExtents (comp, &x1, &y1, &w1, &h1, SPI_COORD_TYPE_SCREEN);
	if ((x1 <= x2) && (x1 + w1 >= x2) && (y1 <= y2) && (y1 + h1 >= y2))
	    rv = TRUE;
	else if ((x1 <= x2 + w2) && (x1 + w1 >= x2 + w2) && (y1 <= y2) && (y1 + h1 >= y2))
	    rv = TRUE;
	else if ((x1 <= x2) && (x1 + w1 >= x2) && (y1 <= y2 + h2) && (y1 + h1 >= y2 + h2))
	    rv = TRUE;
	else if ((x1 <= x2 + w2) && (x1 + w1 >= x2 + w2) && (y1 <= y2 + h2) && (y1 + h1 >= y2 + h2))
	    rv = TRUE;
    }
    
    if (image)
	AccessibleImage_unref (image);
    if (parent)
	Accessible_unref (parent);    
    if (comp)
	AccessibleComponent_unref (comp);
    return rv;
}

static gboolean
srl_acc_has_real_attributes (Accessible *acc,
		    	     gint flags,
		    	     SRLong *index)
{
    gboolean rv = FALSE;
    long int x1, y1, x2, y2, y;
    long cnt;
    AccessibleText *acc_text;
    AccessibleComponent *acc_comp;
    
    srl_assert (acc && index);
    
    if (srl_stop_action)
	return FALSE;

    acc_text = Accessible_getText (acc);
    acc_comp = Accessible_getComponent (acc);
    
    cnt = AccessibleText_getCharacterCount (acc_text);
    AccessibleComponent_getExtents (acc_comp, &x1, &y1, &x2, &y2, 
				SPI_COORD_TYPE_SCREEN);
    x2 += x1;
    y2 += y1;
    
    y = y1;
    while (y < y2 && !rv)
    {
	long start, end;
	long int xt, yt, wt, ht;
	SRTextAttribute *attr;
	
	if (srl_stop_action)
	    break;
	start = AccessibleText_getOffsetAtPoint (acc_text, x1, y,
			    SPI_COORD_TYPE_SCREEN);
	end   = AccessibleText_getOffsetAtPoint (acc_text, x2, y,
			    SPI_COORD_TYPE_SCREEN);
	AccessibleText_getCharacterExtents (acc_text, start, &xt, &yt, &wt, &ht,
			    SPI_COORD_TYPE_SCREEN);
	start = MAX (start, *index);
	end = MAX (end, *index);
	
	get_text_attributes_from_range (acc_text, start, end, &attr);
	if (attr)
	{
	    gint i;
	    for (i = 0; attr[i]; i++)
	    {
		gchar *tmp;
		gint rez;
		rez = 0;
		if (flags & SRL_CASE_BOLD)
		{
		    if (sra_get_attribute_value (attr[i], "bold", &tmp))
		    {
			if (strcmp (tmp, "true") == 0)
			    rez |= SRL_CASE_BOLD;    
			SR_freeString (tmp);
		    }
		}
		if (flags & SRL_CASE_ITALIC)
		{
		    if (sra_get_attribute_value (attr[i], "italic", &tmp))
		    {
			if (strcmp (tmp, "true") == 0)
			    rez |= SRL_CASE_ITALIC;    
			SR_freeString (tmp);
		    }
		}
		if (flags & SRL_CASE_UNDERLINE)
		{
		    if (sra_get_attribute_value (attr[i], "underline", &tmp))
		    {
			rez |= SRL_CASE_UNDERLINE;    
			SR_freeString (tmp);
		    }
		}
		if (flags & SRL_CASE_SELECTED)
		{
		    if (sra_get_attribute_value (attr[i], "selected", &tmp))
		    {
			if (strcmp (tmp, "true") == 0)
			    rez |= SRL_CASE_SELECTED;    
			SR_freeString (tmp);
		    }
		}
		if (flags & SRL_CASE_STRIKETHROUGH)
		{
		    if (sra_get_attribute_value (attr[i], "strikethrough", &tmp))
		    {
			if (strcmp (tmp, "true") == 0)
			    rez |= SRL_CASE_STRIKETHROUGH;    
			SR_freeString (tmp);
		    }
		}
		
		if (flags & SRL_CASE_AND)
		    rv = rez == ((flags & SRL_CASE_BOLD)	|
				 (flags & SRL_CASE_ITALIC) 	|
				 (flags & SRL_CASE_UNDERLINE)	| 
				 (flags & SRL_CASE_SELECTED) 	|
				 (flags & SRL_CASE_STRIKETHROUGH));
		else
		    rv = rez != 0;

		if (rv)
		{
		    if (sra_get_attribute_value (attr[i], "end", &tmp))
		    {
			*index = start + atol (tmp) + 1;    
			SR_freeString (tmp);
		    }
		    break;
		}		
	    }
	    sra_free (attr);
	}

	y += ht;
	if (end >=cnt)
	    break;
	/* DR: how to avoid case in which app crashes */
    }

    if (acc_text)
	AccessibleText_unref (acc_text);
    if (acc_comp)
	AccessibleComponent_unref (acc_comp);

    return rv;
}



static gboolean
srl_acc_has_attributes (Accessible *acc,
			gpointer data)
{
    gboolean rv = FALSE;
    SRLTextFind* ft;
    
    ft = (SRLTextFind*)data;
    srl_assert (acc && ft);

    if (Accessible_isText (acc))
	rv = srl_acc_has_real_attributes (acc, ft->flags, &ft->start);

    return rv;
}



#if 0
static gboolean
srl_true (Accessible *acc,
	  gpointer data)
{
    return TRUE;
}


static gboolean
srl_false (Accessible *acc,
	   gpointer data)
{
    return FALSE;
}
#endif

static gboolean
srl_is_visible_on_screen (Accessible *acc,
			  gpointer data)
{
    gboolean rv = FALSE;
    AccessibleStateSet *state;
    srl_assert (acc);
    
    state = Accessible_getStateSet (acc);
    if (!state)
	return FALSE;
	
    if (AccessibleStateSet_contains (state, SPI_STATE_VISIBLE) &&
	AccessibleStateSet_contains (state, SPI_STATE_SHOWING))
    {
	rv = TRUE;
    }   
    AccessibleStateSet_unref (state);
    
    if (!rv)
	rv = Accessible_isApplication (acc);
    
    return rv;
}


gboolean
sro_get_next_text (SRObject *obj,
		   gchar *text_,
		   SRObject **next,
		   SRNavigationMode mode)
{
    Accessible *ret;
    SRLTextFind tf;
    static Accessible *last = NULL;
    static SRLong index = 0;
    gint flags;
    
    if (next)
	*next = NULL;
    
    srl_return_val_if_fail (obj && text_ && next, FALSE);

    tf.text = g_utf8_strchr (text_, -1, ':');

    if (!tf.text)
	return FALSE;

    tf.flags = 0;
    /* 14 = strlen ("CASE_SENSITIVE") */
    if (tf.text - text_ == 14)
	tf.flags |= SRL_CASE_SENSITIVE;
    
    tf.text ++;
    if (!tf.text[0])
	return FALSE;

    tf.start = index + 1;
    
    ret = NULL;
    if (srl_acc_has_text (obj->acc, &tf))
    {
	ret = obj->acc;
	Accessible_ref (ret);
    }
    
    flags = 0;
    if (mode == SR_NAV_MODE_WINDOW)
	flags |= SRL_SCOPE_WINDOW;
    else if (mode == SR_NAV_MODE_APPLICATION)
	flags |= SRL_SCOPE_APPLICATION;
    else if (mode == SR_NAV_MODE_DESKTOP)
	flags |= SRL_SCOPE_DESKTOP;
    else
	srl_assert_not_reached ();

    if (!ret)
    {
	tf.start = 0;
	srl_traverse (obj->acc,
			&ret,
	    		flags | SRL_DIRECTION_NEXT | SRL_TRAVERSE_CHILDREN | SRL_TRAVERSE_PARENT | SRL_TRAVERSE_SIBLINGS,
			srl_acc_has_text,
	    		&tf,
	    		srl_is_visible_on_screen,
	    		NULL);
    }

    if (ret)
    {
	last = ret;
	index = tf.start;
	sro_get_from_accessible (ret, next, SR_OBJ_TYPE_PROCESSED);
	Accessible_unref (ret);
    }
    
    if (*next)
    {
	g_free ((*next)->reason);
	(*next)->reason = g_strdup ("present");
    }

    return (*next) ? TRUE : FALSE;    
}

gboolean
sro_get_next_image (SRObject *obj,
		    SRObject **next,
		    SRNavigationMode mode)
{
    gboolean rv = FALSE;
    gint flags;
    Accessible *ret;

    if (next)
	*next = NULL;
    srl_return_val_if_fail (obj && next, FALSE);

    flags = 0;
    if (mode == SR_NAV_MODE_WINDOW)
	flags |= SRL_SCOPE_WINDOW;
    else if (mode == SR_NAV_MODE_APPLICATION)
	flags |= SRL_SCOPE_APPLICATION;
    else if (mode == SR_NAV_MODE_DESKTOP)
	flags |= SRL_SCOPE_DESKTOP;
    else
	srl_assert_not_reached ();

    ret = NULL;
    rv = srl_traverse (obj->acc,
			&ret,
	    		flags | SRL_DIRECTION_NEXT | SRL_TRAVERSE_CHILDREN | SRL_TRAVERSE_PARENT | SRL_TRAVERSE_SIBLINGS,
			srl_acc_has_image,
	    		NULL,
	    		srl_is_visible_on_screen,
	    		NULL);

    if (ret)
    {
	rv = sro_get_from_accessible (ret, next, SR_OBJ_TYPE_PROCESSED);
	Accessible_unref (ret);
    }

    if (rv)
    {
	g_free ((*next)->reason);
	(*next)->reason = g_strdup ("present");
    }

    return rv;    
}

gboolean
sro_get_next_attributes (SRObject *obj,
			 gchar *attr,
			 SRObject **next,
			 SRNavigationMode mode)
{
    Accessible *ret;
    SRLTextFind tf;
    static Accessible *last = NULL;
    static SRLong index = 0;
    gint flags;
    gchar *tmp;
    
    if (next)
	*next = NULL;
    
    srl_return_val_if_fail (obj && attr && next, FALSE);

    tmp = g_utf8_strchr (attr, -1, ':');

    if (!tmp)
	return FALSE;

    tf.flags = 0;
    /* 2 = strlen ("OR") */
    if (tmp - attr == 2)
	tf.flags |= SRL_CASE_OR;
    else
	tf.flags |= SRL_CASE_AND;

    if (strstr (attr, "BOLD"))
	tf.flags |= SRL_CASE_BOLD;
    if (strstr (attr, "ITALIC"))
	tf.flags |= SRL_CASE_ITALIC;
    if (strstr (attr, "UNDERLINE"))
	tf.flags |= SRL_CASE_UNDERLINE;
    if (strstr (attr, "SELECTED"))
	tf.flags |= SRL_CASE_SELECTED;
    if (strstr (attr, "STRIKETHROUGH"))
	tf.flags |= SRL_CASE_STRIKETHROUGH;

    tf.start = index + 1;

    ret = NULL;
    if (srl_acc_has_attributes (obj->acc, &tf))
    {
	ret = obj->acc;
	Accessible_ref (ret);
    }
    
    flags = 0;
    if (mode == SR_NAV_MODE_WINDOW)
	flags |= SRL_SCOPE_WINDOW;
    else if (mode == SR_NAV_MODE_APPLICATION)
	flags |= SRL_SCOPE_APPLICATION;
    else if (mode == SR_NAV_MODE_DESKTOP)
	flags |= SRL_SCOPE_DESKTOP;
    else
	srl_assert_not_reached ();

    if (!ret)
    {
	tf.start = 0;
	srl_traverse (obj->acc,
			&ret,
	    		flags | SRL_DIRECTION_NEXT | SRL_TRAVERSE_CHILDREN | SRL_TRAVERSE_PARENT | SRL_TRAVERSE_SIBLINGS,
			srl_acc_has_attributes,
	    		&tf,
	    		srl_is_visible_on_screen,
	    		NULL);
    }

    if (ret)
    {
	last = ret;
	index = tf.start;
	sro_get_from_accessible (ret, next, SR_OBJ_TYPE_PROCESSED);
	Accessible_unref (ret);
    }
    
    if (*next)
    {
	g_free ((*next)->reason);
	(*next)->reason = g_strdup ("present");
    }

    return (*next) ? TRUE : FALSE;    
}


static gchar*
sro_get_text_from_acc (Accessible *acc,
		       gchar *prev)
{
    gchar *rv = prev;
    
    srl_assert (acc);

    if (Accessible_isText (acc) && Accessible_getRole (acc) != SPI_ROLE_PUSH_BUTTON)
    {
	gchar *name = Accessible_getName (acc);
	if (name && name[0])
	{
	    rv = g_strconcat (prev ? prev: "", prev ? " ": "", name, NULL);
	    g_free (prev);
	}
	SPI_freeString (name);
    }
    else
    {
	gint i, cnt;
	cnt = Accessible_getChildCount (acc);
	for (i = 0; i < cnt; i++)
	{
	    Accessible *child = Accessible_getChildAtIndex (acc, i);
	    if (child)
	    {
		rv = sro_get_text_from_acc (child, rv);
		Accessible_unref (child);
	    }
	}
    }    

    return rv;    
}

static gchar*
sro_get_button_from_acc (Accessible *acc,
			 gchar *prev)
{
    gchar *rv = prev;
    
    srl_assert (acc);

    if (Accessible_getRole (acc) == SPI_ROLE_PUSH_BUTTON)
    {
	gchar *name = Accessible_getName (acc);
	if (name && name[0])
	{
	    rv = g_strconcat (prev ? prev: "", prev ? " ": "", name, NULL);
	    g_free (prev);
	}
	SPI_freeString (name);
    }
    else
    {
	gint i, cnt;
	cnt = Accessible_getChildCount (acc);
	for (i = 0; i < cnt; i++)
	{
	    Accessible *child = Accessible_getChildAtIndex (acc, i);
	    if (child)
	    {
		rv = sro_get_button_from_acc (child, rv);
		Accessible_unref (child);
	    }
	}
    }    

    return rv;    
}

gboolean
sro_alert_get_info (SRObject *obj,
		    gchar **title,
		    gchar **text,
		    gchar **button)
{
    gchar *title_, *text_, *button_;

    srl_assert (obj && title && text && button);
    srl_assert (obj->role == SR_ROLE_ALERT);

    *title = *text = *button = NULL;
    title_  = Accessible_getName (obj->acc);
    if (title_ && title_[0])
	*title = SR_strdup (title_);
    SPI_freeString (title_);
    
    text_   = sro_get_text_from_acc (obj->acc, NULL);
    if (text_ && text_[0])
	*text = SR_strdup (text_);
    g_free (text_);
    
    button_ = sro_get_button_from_acc (obj->acc, NULL);
    if (button_ && button_[0])
	*button = SR_strdup (button_);
    g_free (button_);

    return TRUE;
}

Accessible *
sro_get_acc (SRObject *obj)
{
    return obj->acc;
}
