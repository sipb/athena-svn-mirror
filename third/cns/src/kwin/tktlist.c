/*
 * tktlist.c
 *
 * Handle all actions of the Kerberos ticket list.
 *
 * Copyright 1994 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 */

#include "mit-copyright.h"
#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include "cns.h"
#include "kerberos.h"
#include "tktlist.h"

/*
 * Ticket information for a list line
 */
typedef struct {
	BOOL ticket;		/* TRUE if this is a real ticket */
    time_t issue_time;	/* time_t of issue */
	long lifetime;		/* Lifetime for ticket in 5 minute intervals */
	char buf[0];		/* String to display */
} TICKETINFO, *LPTICKETINFO;

/*
 * Function: Returns a standard ctime date with day of week and year
 *	removed.
 *
 * Parameters:
 *	t - time_t date to convert
 *
 * Returns: A pointer to the adjusted time value.
 */
static char *short_date(t)
	long t;
{
	static char buf[26 - 4];
	char *p;

	p = ctime(&t);
	assert(p != NULL);

	strcpy (buf, p + 4);

	buf[12] = '\0';

	return buf;

} /* short_date */


/*
 * Function: Initializes and populates the ticket list with all existing
 * 	Kerberos tickets.
 *
 * Parameters:
 * 	hwnd - the window handle of the ticket window.
 *
 * Returns: TRUE if list was successfully initialized, FALSE otherwise.
 */
BOOL ticket_init_list(
	HWND hwnd)
{
	int i;
	int ncred;
	LRESULT rc;
	char service[ANAME_SZ];
	char instance[INST_SZ];
	char realm[REALM_SZ];
	CREDENTIALS c;
	char buf[26+2 + 26+2 + ANAME_SZ+1 + INST_SZ+1 + REALM_SZ + 22];
	int l;
	LPTICKETINFO lpinfo;
	time_t expiration;

	SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);

	rc = SendMessage(hwnd, LB_GETCOUNT, 0, 0);
	assert(rc != LB_ERR);

	if (rc > 0)
		ticket_destroy(hwnd);

	while (--rc >= 0)
		SendMessage(hwnd, LB_DELETESTRING, (WPARAM) rc, 0);

	ncred = krb_get_num_cred();

	for (i = 1; i <= ncred; i++) {
		krb_get_nth_cred(service, instance, realm, i);

		krb_get_cred(service, instance, realm, &c);

		strcpy(buf, " ");

		strcat(buf, short_date(c.issue_date - kwin_get_epoch()));

		expiration = c.issue_date - kwin_get_epoch() + (long) c.lifetime * 5L * 60L;

		strcat(buf, "  ");

		strcat(buf, short_date(expiration));

		l = strlen(buf);

	  	sprintf(&buf[l], "  %s%s%s%s%s (%d)",
		 	c.service, (c.instance[0] ? "." : ""), c.instance,
		 	(c.realm[0] ? "@" : ""), c.realm, c.kvno);

		l = strlen(buf);

		lpinfo = (LPTICKETINFO) malloc(sizeof(TICKETINFO) + l + 1);
		assert(lpinfo != NULL);

		if (lpinfo == NULL)
			return FALSE;

		lpinfo->ticket = TRUE;
		lpinfo->issue_time = c.issue_date - kwin_get_epoch(); /* back to system time */
		lpinfo->lifetime = (long) c.lifetime * 5L * 60L;
		strcpy(lpinfo->buf, buf);

		rc = SendMessage(hwnd, LB_ADDSTRING, 0, (LPARAM) lpinfo);
		assert(rc >= 0);

		if (rc < 0)
			return FALSE;
	}

	if (ncred <= 0) {
		strcpy(buf, " No Tickets");

		lpinfo = (LPTICKETINFO) malloc(sizeof(TICKETINFO) + strlen(buf) + 1);
		assert(lpinfo != NULL);

		if (lpinfo == NULL)
			return FALSE;

		lpinfo->ticket = FALSE;
		strcpy (lpinfo->buf, buf);

		rc = SendMessage(hwnd, LB_ADDSTRING, 0, (LPARAM) lpinfo);
		assert(rc >= 0);
	}

	SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

	return TRUE;

} /* ticket_init_list */


/*
 * Function: Destroy the ticket list.  Make sure to delete all
 *	ticket entries created during ticket initialization.
 *
 * Parameters:
 *	hwnd - the window handle of the ticket window.
 */
void ticket_destroy(
	HWND hwnd)
{
	int i;
	int n;
	LRESULT rc;

	n = (int) SendMessage(hwnd, LB_GETCOUNT, 0, 0);

	for (i = 0; i < n; i++) {
		rc = SendMessage(hwnd, LB_GETITEMDATA, i, 0);
		assert(rc != LB_ERR);

		if (rc != LB_ERR)
			free ((void *) rc);
	}

} /* ticket_destroy */


/*
 * Function: Respond to the WM_MEASUREITEM message for the ticket list
 * 	by setting each list item up at 1/4" hight.
 *
 * Parameters:
 * 	hwnd - the window handle of the ticket window.
 *
 * 	wparam - control id of the ticket list.
 *
 * 	lparam - pointer to the MEASUREITEMSTRUCT.
 *
 * Returns: TRUE if message process, FALSE otherwise.
 */
LONG ticket_measureitem(
	HWND hwnd,
	WPARAM wparam,
	LPARAM lparam)
{
	int logpixelsy;
	LPMEASUREITEMSTRUCT lpmi;
	HDC hdc;

	lpmi = (LPMEASUREITEMSTRUCT) lparam;

	hdc = GetDC(HWND_DESKTOP);

	logpixelsy = GetDeviceCaps(hdc, LOGPIXELSY);

	ReleaseDC(HWND_DESKTOP, hdc);

	lpmi->itemHeight = logpixelsy / 4;	/* 1/4 inch */

	return TRUE;

} /* ticket_measureitem */


/*
 * Function: Respond to the WM_DRAWITEM message for the ticket list
 * 	by displaying a single list item.
 *
 * Parameters:
 * 	hwnd - the window handle of the ticket window.
 *
 * 	wparam - control id of the ticket list.
 *
 * 	lparam - pointer to the DRAWITEMSTRUCT.
 *
 * Returns: TRUE if message process, FALSE otherwise.
 */
LONG ticket_drawitem(
	HWND hwnd,
	WPARAM wparam,
	LPARAM lparam)
{
	LPDRAWITEMSTRUCT lpdi;
	BOOL rc;
	COLORREF bkcolor;
	HBRUSH hbrush;
	UINT textheight;
	UINT alignment;
	int left, top;
	BOOL b;
	LPTICKETINFO lpinfo;
	HICON hicon;
	#if 0
		COLORREF textcolor;
		COLORREF orgbkcolor;
		COLORREF orgtextcolor;
	#endif

	lpdi = (LPDRAWITEMSTRUCT) lparam;

	lpinfo = (LPTICKETINFO) lpdi->itemData;

	if (lpdi->itemAction == ODA_FOCUS)
		return TRUE;

	#if 0 

		if (lpdi->itemState & ODS_SELECTED) {
			textcolor = GetSysColor(COLOR_HIGHLIGHTTEXT);
			bkcolor = GetSysColor(COLOR_HIGHLIGHT);

			orgtextcolor = SetTextColor(lpdi->hDC, textcolor);
    		assert(textcolor != 0x80000000);

			orgbkcolor = SetBkColor(lpdi->hDC, bkcolor);
    		assert(bkcolor != 0x80000000);
		}
		else 

	#endif

	bkcolor = GetBkColor(lpdi->hDC);

	hbrush = CreateSolidBrush(bkcolor);
	assert(hbrush != NULL);

	FillRect(lpdi->hDC, &(lpdi->rcItem), hbrush);

	DeleteObject(hbrush);

	/*
	 * Display the appropriate icon
	 */
	if (lpinfo->ticket) {
		hicon = kwin_get_icon(lpinfo->issue_time + lpinfo->lifetime);

		left = lpdi->rcItem.left - (32 - ICON_WIDTH) / 2;

		top = lpdi->rcItem.top;
		top += (lpdi->rcItem.bottom - lpdi->rcItem.top - 32) / 2;

		b = DrawIcon(lpdi->hDC, left, top, hicon);
		assert(b);
	}
	
	/*
	 * Display centered string
	 */
	textheight = HIWORD(GetTextExtent(lpdi->hDC, "X", 1));

	alignment = SetTextAlign(lpdi->hDC, TA_TOP | TA_LEFT);

	if (lpinfo->ticket)
		left = lpdi->rcItem.left + ICON_WIDTH;
	else
		left = lpdi->rcItem.left;

	top = lpdi->rcItem.top;
	top += (lpdi->rcItem.bottom - lpdi->rcItem.top - textheight) / 2;

	rc = TextOut(lpdi->hDC, left, top, (LPSTR) lpinfo->buf,
			strlen((LPSTR) lpinfo->buf));
	assert(rc);

	alignment = SetTextAlign(lpdi->hDC, alignment);

	#if 0

		if (lpdi->itemState & ODS_SELECTED) {
			textcolor = SetTextColor(lpdi->hDC, orgtextcolor);
	    	assert(textcolor != 0x80000000);

			bkcolor = SetBkColor(lpdi->hDC, orgbkcolor);
	    	assert(bkcolor != 0x80000000);
		}

	#endif

	return TRUE;

} /* ticket_drawitem */