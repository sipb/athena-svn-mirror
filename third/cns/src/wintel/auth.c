/*
 * Implements Kerberos 4 authentication
 */

#include <windows.h>
#include <time.h>
#include <string.h>
#include "winsock.h"
#include "telopts.h"
#include "kerberos.h"
#include "telnet.h"

/*
 * Contants
 */
	#define IS						0
	#define SEND					1
	#define REPLY					2
	#define NAME					3

	#define AUTH_NULL				0
	#define KERBEROS_V4				1
	#define KERBEROS_V5				2
	#define SPX						3
	#define RSA            			6
	#define LOKI           			10

	#define AUTH					0
	#define REJECT					1
	#define ACCEPT					2
	#define CHALLENGE				3
	#define RESPONSE				4

	#define AUTH_WHO_MASK		1
	#define AUTH_CLIENT_TO_SERVER   0
	#define AUTH_SERVER_TO_CLIENT   1

	#define AUTH_HOW_MASK		2
	#define AUTH_HOW_ONE_WAY        0
	#define AUTH_HOW_MUTUAL         2

	#define KRB_SERVICE_NAME   "rcmd"

/*
 * Globals
 */
	static CREDENTIALS cred;
	BOOL encrypt_enable;

/*
 * Function: Enable or disable the encryption process.
 *
 * Parameters:
 *	enable - TRUE to enable, FALSE to disable.
 */
static void auth_encrypt_enable(
	BOOL enable)
{
	encrypt_enable = enable;

} /* auth_encrypt_enable */


/*
 * Function: Abort the authentication process
 *
 * Parameters:
 *	ks - kstream to send abort message to.
 */
static void auth_abort(
	kstream ks,
	char *errmsg,
	int r)
{
    char buf[9];
	char errbuf[256];

	wsprintf(buf, "%c%c%c%c%c%c%c%c", IAC, SB, AUTHENTICATION, IS, AUTH_NULL, AUTH_NULL, IAC, SE);
	TelnetSend(ks, (LPSTR)buf, 8, 0);

	if (errmsg != NULL) {
		strcpy(errbuf, errmsg);

		if (r != KSUCCESS) {
			strcat(errbuf, "\n");
			lstrcat(errbuf, krb_get_err_text(r));
		}

		MessageBox(HWND_DESKTOP, errbuf, "Kerberos authentication failed!", MB_OK | MB_ICONEXCLAMATION);
	}

} /* auth_abort */


/*
 * Function: Copy data to buffer, doubling IAC character if present.
 *
 * Parameters:
 *	kstream - kstream to send abort message to.
 */
static int copy_for_net(
	unsigned char *to,
	unsigned char *from,
	int c)
{
	int n;

	n = c;

	while (c-- > 0) {
		if ((*to++ = *from++) == IAC) {
			n++;
			*to++ = IAC;
		}
	}

	return n;

} /* copy_for_net */


/*
 * Function: Parse authentication send command
 *
 * Parameters:
 *	ks - kstream to send abort message to.
 *
 *  parsedat - the sub-command data.
 *
 *	end_sub - index of the character in the 'parsedat' array which
 *		is the last byte in a sub-negotiation
 *
 * Returns: Kerberos error code.
 */
static int auth_send(
	kstream ks,
	unsigned char *parsedat,
	int end_sub)
{
    char buf[256];
	char *realm;
	char *pname;
	int r;
	KTEXT_ST auth;
	int how;
	char instance[INST_SZ];
	int i;

	how = -1;

	for (i = 2; i+1 <= end_sub; i += 2) {
		if (parsedat[i] == KERBEROS_V4)
			if ((parsedat[i+1] & AUTH_WHO_MASK) == AUTH_CLIENT_TO_SERVER) {
				how = parsedat[i+1] & AUTH_HOW_MASK;
				break;
			}
	}

	if (how == -1) {
		auth_abort(ks, NULL, 0);
		return KFAILURE;
	}

	memset(instance, 0, sizeof(instance));

	if (realm = krb_get_phost(szHostName))
	  	lstrcpy(instance, realm);

	realm = krb_realmofhost(szHostName);

	if (!realm) {
		strcpy(buf, "Can't find realm for host \"");
		strcat(buf, szHostName);
		strcat(buf, "\"");
		auth_abort(ks, buf, 0);
		return KFAILURE;
	}

	r = krb_mk_req(&auth, KRB_SERVICE_NAME, instance, realm, 0);

	if (r == 0)
		r = krb_get_cred(KRB_SERVICE_NAME, instance, realm, &cred);

	if (r) {
		strcpy(buf, "Can't get \"");
		strcat(buf, KRB_SERVICE_NAME);
		if (instance[0] != 0) {
		  	strcat(buf, ".");
			lstrcat(buf, instance);
		}
		strcat(buf, "@");
		lstrcat(buf, realm);
		strcat(buf, "\" ticket");
		auth_abort(ks, buf, r);
		return r;
	}

	wsprintf(buf, "%c%c%c%c", IAC, SB, AUTHENTICATION, NAME);

	if (szUserName[0])
		pname = szUserName;
	else
		pname = cred.pname;

	lstrcpy(&buf[4], pname);

	wsprintf(&buf[lstrlen(pname)+4], "%c%c", IAC, SE);

	TelnetSend(ks, (LPSTR)buf, lstrlen(pname)+6, 0);

	wsprintf(buf, "%c%c%c%c%c%c%c", IAC, SB, AUTHENTICATION, IS,
		KERBEROS_V4, how|AUTH_CLIENT_TO_SERVER, AUTH);

	auth.length = copy_for_net(&buf[7], auth.dat, auth.length);

	wsprintf(&buf[auth.length+7], "%c%c", IAC, SE);

	TelnetSend(ks, (LPSTR)buf, auth.length+9, 0);

	return KSUCCESS;

}	/* auth_send */


/*
 * Function: Parse authentication reply command
 *
 * Parameters:
 *	ks - kstream to send abort message to.
 *
 *  parsedat - the sub-command data.
 *
 *	end_sub - index of the character in the 'parsedat' array which
 *		is the last byte in a sub-negotiation
 *
 * Returns: Kerberos error code.
 */
static int auth_reply(
	kstream ks,
	unsigned char *parsedat,
	int end_sub)
{
	des_cblock session_key;
	des_key_schedule sched;
	time_t t;
	int x;
    char buf[256];
	static des_cblock challenge;
	int i;

	if (end_sub < 4)
		return KFAILURE;
		
	if (parsedat[2] != KERBEROS_V4)
		return KFAILURE;

	if (parsedat[4] == REJECT) {
		buf[0] = 0;

		for (i = 5; i <= end_sub; i++) {
			if (parsedat[i] == IAC)
				break;

			buf[i-5] = parsedat[i];

			buf[i-4] = 0;
		}

		if (!buf[0])
			strcpy(buf,
			       "Authentication rejected by remote machine!");

		MessageBox(NULL, buf, NULL, MB_OK | MB_ICONEXCLAMATION);

		return KFAILURE;
	}

	if (parsedat[4] == ACCEPT) {

		if ((parsedat[3] & AUTH_HOW_MASK) == AUTH_HOW_ONE_WAY)
			return KSUCCESS;

		if ((parsedat[3] & AUTH_HOW_MASK) != AUTH_HOW_MUTUAL)
			return KFAILURE;

		des_key_sched(cred.session, sched);

		t = time(NULL);

		memcpy(challenge, &t, 4);

		memcpy(&challenge[4], &t, 4);

		des_ecb_encrypt(&challenge, &session_key, sched, 1);

		/*
		* Increment the challenge by 1, and encrypt it for
		* later comparison.
		*/
		for (i = 7; i >= 0; --i) {
			x = (unsigned int)challenge[i] + 1;

			challenge[i] = x;	/* ignore overflow */

			if (x < 256)		/* if no overflow, all done */
				break;
		}

		des_ecb_encrypt(&challenge, &challenge, sched, 1);

		wsprintf(buf, "%c%c%c%c%c%c%c", IAC, SB, AUTHENTICATION, IS,
			KERBEROS_V4, AUTH_CLIENT_TO_SERVER|AUTH_HOW_MUTUAL, CHALLENGE);

		memcpy(&buf[7], session_key, 8);

		wsprintf(&buf[15], "%c%c", IAC, SE);

		TelnetSend(ks, (LPSTR)buf, 17, 0);

		return KSUCCESS;
	}

	if (parsedat[4] == RESPONSE) {

		if (end_sub < 12)
			return KFAILURE;

		if (memcmp(&parsedat[5], challenge, sizeof(challenge)) != 0) {
	    	MessageBox(NULL, "Remote machine is being impersonated!",
			   NULL, MB_OK | MB_ICONEXCLAMATION);

			return KFAILURE;
		}


		return KSUCCESS;
	}
	
	return KFAILURE;

} /* auth_reply */


/*
 * Function: Parse the athorization sub-options and reply.
 *
 * Parameters:
 *	ks - kstream to send abort message to.
 *
 *	parsedat - sub-option string to parse.
 *
 *	end_sub - last charcter position in parsedat.
 */
void auth_parse(
	kstream ks,
	unsigned char *parsedat,
	int end_sub)
{
	if (parsedat[1] == SEND)
		auth_send(ks, parsedat, end_sub);

	if (parsedat[1] == REPLY)
		auth_reply(ks, parsedat, end_sub);

} /* auth_parse */


/*
 * Function: Initialization routine called kstream encryption system.
 *
 * Parameters:
 *	str - kstream to send abort message to.
 *
 *  data - user data.
 */
int INTERFACE auth_init(
	kstream str,
	kstream_ptr data)
{
	return 0;

} /* auth_init */


/*
 * Function: Destroy routine called kstream encryption system.
 *
 * Parameters:
 *	str - kstream to send abort message to.
 *
 *  data - user data.
 */
void INTERFACE auth_destroy(
	kstream str)
{
} /* auth_destroy */


/*
 * Function: Callback to encrypt a block of characters
 *
 * Parameters:
 *	out - return as pointer to converted buffer.
 *
 *  in - the buffer to convert
 *
 *  str - the stream being encrypted
 *
 * Returns: number of characters converted.
 */
int INTERFACE auth_encrypt(
	struct kstream_data_block *out,
	struct kstream_data_block *in,
	kstream str)
{
	out->ptr = in->ptr;

	out->length = in->length;

	return(out->length);

} /* auth_encrypt */


/*
 * Function: Callback to decrypt a block of characters
 *
 * Parameters:
 *	out - return as pointer to converted buffer.
 *
 *  in - the buffer to convert
 *
 *  str - the stream being encrypted
 *
 * Returns: number of characters converted.
 */
int INTERFACE auth_decrypt(
	struct kstream_data_block *out,
	struct kstream_data_block *in,
	kstream str)
{
	out->ptr = in->ptr;

	out->length = in->length;

	return(out->length);

} /* auth_decrypt */
