/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _E_ADDRESSBOOK_TABLE_ADAPTER_H_
#define _E_ADDRESSBOOK_TABLE_ADAPTER_H_

#include <gal/e-table/e-table-model.h>
#include "addressbook/backend/ebook/e-book.h"
#include "addressbook/backend/ebook/e-book-view.h"
#include "addressbook/backend/ebook/e-card-simple.h"

#define E_TYPE_ADDRESSBOOK_TABLE_ADAPTER        (e_addressbook_table_adapter_get_type ())
#define E_ADDRESSBOOK_TABLE_ADAPTER(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), E_TYPE_ADDRESSBOOK_TABLE_ADAPTER, EAddressbookTableAdapter))
#define E_ADDRESSBOOK_TABLE_ADAPTER_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), E_TYPE_ADDRESSBOOK_TABLE_ADAPTER, EAddressbookTableAdapterClass))
#define E_IS_ADDRESSBOOK_TABLE_ADAPTER(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), E_TYPE_ADDRESSBOOK_TABLE_ADAPTER))
#define E_IS_ADDRESSBOOK_TABLE_ADAPTER_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), E_TYPE_ADDRESSBOOK_TABLE_ADAPTER))

/* Virtual Column list:
   0   Email
   1   Full Name
   2   Street
   3   Phone
*/

typedef struct _EAddressbookTableAdapter EAddressbookTableAdapter;
typedef struct _EAddressbookTableAdapterPrivate EAddressbookTableAdapterPrivate;
typedef struct _EAddressbookTableAdapterClass EAddressbookTableAdapterClass;

struct _EAddressbookTableAdapter {
	ETableModel parent;

	EAddressbookTableAdapterPrivate *priv;
};


struct _EAddressbookTableAdapterClass {
	ETableModelClass parent_class;
};


GType        e_addressbook_table_adapter_get_type (void);
void         e_addressbook_table_adapter_construct (EAddressbookTableAdapter *adapter,
						    EAddressbookModel *model);
ETableModel *e_addressbook_table_adapter_new (EAddressbookModel *model);

#endif /* _E_ADDRESSBOOK_TABLE_ADAPTER_H_ */