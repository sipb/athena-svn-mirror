/* $Id: JEncryption.c,v 1.1.1.1 2006-03-10 15:32:43 ghudson Exp $ */

#include <sysdep.h>
#include "include/libjwgc.h"
#ifdef USE_GPGME
#include <gpgme.h>

xode *key_list = NULL;
int num_keys = 0;

static void
print_op_info (GpgmeCtx c)
{
	char *s = gpgme_get_op_info (c, 0);

	if (!s) {
		dprintf(dGPG, "<!-- no operation info available -->");
	}
	else {
		dprintf(dGPG, "%s", s);
		free (s);
	}
}

static const char *
passphrase_cb (void *opaque, const char *desc, void **r_hd)
{
	const char *pass;

	if (!desc) {
		return NULL;
	}

	pass = (char *)jVars_get(jVarGPGPass);
	return pass;
}

char *
JSign(char *data)
{
	GpgmeCtx ctx;
	GpgmeError err;
	GpgmeData in, out;
	char *retdata;
	char buf[100];
	size_t nread;
	size_t retsz;

	retdata = (char *)malloc(sizeof(char) * 1);
	retdata[0] = '\0';
	retsz = 0;

	err = gpgme_new(&ctx);
	if (err) {
		dprintf(dGPG, "Failed to create GPGME context.\n");
		return NULL;
	}

	gpgme_set_passphrase_cb(ctx, passphrase_cb, NULL);
	gpgme_set_textmode(ctx, 1);
	gpgme_set_armor(ctx, 1);

	err = gpgme_data_new_from_mem(&in, data, strlen(data), 0);
	if (err) {
		dprintf(dGPG, "Failed to create input data.\n");
		return NULL;
	}

	err = gpgme_data_new(&out);
	if (err) {
		dprintf(dGPG, "Failed to create output data.\n");
		return NULL;
	}

	err = gpgme_op_sign(ctx, in, out, GPGME_SIG_MODE_DETACH);
	if (err) {
		dprintf(dGPG, "Detached signature failed.\n");
		return NULL;
	}

	fflush(NULL);
	print_op_info(ctx);

	err = gpgme_data_rewind(out);
	if (err) {
		dprintf(dGPG, "Failed to rewind data stream.\n");
		return NULL;
	}
	while (!(err = gpgme_data_read(out, buf, 100, &nread))) {
		retsz += nread;
		retdata = realloc(retdata, sizeof(char) * (retsz + 1));
		strcat(retdata, buf);
		retdata[retsz] = '\0';
	}

	gpgme_data_release(out);
	gpgme_data_release(in);
	gpgme_release(ctx);

	dprintf(dGPG, "Successfully signed data:\n%s\n", retdata);

	return retdata;
}

char *
JTrimPGPMessage(char *msg)
{
	char *retstr, *cp, *tmpbuf;

	if (!msg) {
		dprintf(dGPG, "Trimming empty string.\n");
		return NULL;
	}

	dprintf(dGPG, "Trimming:\n%s\n", msg);

	retstr = (char *)malloc(sizeof(char) * 1);
	retstr[0] = '\0';
	tmpbuf = strdup(msg);

	cp = tmpbuf;
	cp = strtok(cp, "\n");
	dprintf(dGPG, "Searching for BEGIN...\n", cp);
	while (cp && strncmp(cp, "-----BEGIN", 10)) {
		dprintf(dGPG, "Skipping %s\n", cp);
		cp = strtok(NULL, "\n");
	}
	if (!cp) { return NULL; }
	dprintf(dGPG, "Skipping %s\n", cp);
	cp = strtok(NULL, "\n");
	dprintf(dGPG, "Searching for end of headers...\n", cp);
	while (cp && !strchr(cp, ' ')) {
		dprintf(dGPG, "Skipping %s\n", cp);
		cp = strtok(NULL, "\n");
	}
	if (!cp) { return NULL; }
	dprintf(dGPG, "Skipping %s\n", cp);
	cp = strtok(NULL, "\n");
	dprintf(dGPG, "Collecting all until END...\n", cp);
	while (cp && strncmp(cp, "-----END", 8)) {
		dprintf(dGPG, "Keeping %s\n", cp);
		retstr = realloc(retstr, sizeof(char) * (strlen(retstr) +
					strlen(cp) + 2));
		strcat(retstr, cp);
		strcat(retstr, "\n");
		cp = strtok(NULL, "\n");
	}
	dprintf(dGPG, "Trimmed string is:\n%s\n", retstr);

	return retstr;
}

char *
JDecrypt(char *data)
{
	GpgmeCtx ctx;
	GpgmeError err;
	GpgmeData in, out;
	char *retdata;
	char *bufdata;
	char buf[100];
	size_t nread;
	size_t retsz;

	retdata = (char *)malloc(sizeof(char) * 1);
	retdata[0] = '\0';
	retsz = 0;

	err = gpgme_new(&ctx);
	if (err) {
		dprintf(dGPG, "Failed to create GPGME context.\n");
		return NULL;
	}

	gpgme_set_passphrase_cb(ctx, passphrase_cb, NULL);
	gpgme_set_textmode(ctx, 1);
	gpgme_set_armor(ctx, 1);

	bufdata = (char *)malloc(sizeof(char) * (strlen(data) + 1 + 68));
	sprintf(bufdata, "-----BEGIN PGP MESSAGE-----\nVersion: JWGC\n\n%s\n-----END PGP MESSAGE-----\n", data);
	dprintf(dGPG, "Parsing input data:\n%s\n", bufdata);
	err = gpgme_data_new_from_mem(&in, bufdata, strlen(bufdata), 0);
	if (err) {
		dprintf(dGPG, "Failed to create input data.\n");
		return NULL;
	}

	err = gpgme_data_new(&out);
	if (err) {
		dprintf(dGPG, "Failed to create output data.\n");
		return NULL;
	}

	err = gpgme_op_decrypt(ctx, in, out);
	if (err) {
		dprintf(dGPG, "Data decryption failed.\n");
		return NULL;
	}

	fflush(NULL);

	err = gpgme_data_rewind(out);
	if (err) {
		dprintf(dGPG, "Failed to rewind data stream.\n");
		return NULL;
	}
	while (!(err = gpgme_data_read(out, buf, 100, &nread))) {
		retsz += nread;
		retdata = realloc(retdata, sizeof(char) * (retsz + 1));
		strcat(retdata, buf);
		retdata[retsz] = '\0';
	}

	gpgme_data_release(out);
	gpgme_data_release(in);
	gpgme_release(ctx);

	dprintf(dGPG, "Successfully decrypted data:\n%s\n", retdata);

	return retdata;
}

char *
JEncrypt(char *data, char *recipient)
{
	GpgmeCtx ctx;
	GpgmeError err;
	GpgmeData in, out;
	GpgmeRecipients rset;
	char *retdata;
	char *bufdata;
	char buf[100];
	size_t nread;
	size_t retsz;

	retdata = (char *)malloc(sizeof(char) * 1);
	retdata[0] = '\0';
	retsz = 0;

	err = gpgme_check_engine();
	if (err) {
		dprintf(dGPG, "Failed to check GPG engine.\n");
		return NULL;
	}
	dprintf(dGPG, "Engine Info:\n%s\n", gpgme_get_engine_info());

	err = gpgme_new(&ctx);
	if (err) {
		dprintf(dGPG, "Failed to create GPGME context.\n");
		return NULL;
	}

	gpgme_set_armor(ctx, 1);

	err = gpgme_data_new_from_mem(&in, data, strlen(data), 0);
	if (err) {
		dprintf(dGPG, "Failed to create input data.\n");
		return NULL;
	}

	err = gpgme_data_new(&out);
	if (err) {
		dprintf(dGPG, "Failed to create output data.\n");
		return NULL;
	}

	err = gpgme_recipients_new(&rset);
	if (err) {
		dprintf(dGPG, "Failed to initialize recipients.\n");
		return NULL;
	}

	err = gpgme_recipients_add_name_with_validity(rset, recipient,
					GPGME_VALIDITY_FULL);
	if (err) {
		dprintf(dGPG, "Failed to add recipient.\n");
		return NULL;
	}

	err = gpgme_op_encrypt(ctx, rset, in, out);
	if (err) {
		dprintf(dGPG, "Data encryption failed.\n");
		return NULL;
	}

	print_op_info(ctx);
	fflush(NULL);

	err = gpgme_data_rewind(out);
	if (err) {
		dprintf(dGPG, "Failed to rewind data stream.\n");
		return NULL;
	}
	while (!(err = gpgme_data_read(out, buf, 100, &nread))) {
		retsz += nread;
		retdata = realloc(retdata, sizeof(char) * (retsz + 1));
		strcat(retdata, buf);
		retdata[retsz] = '\0';
	}

	gpgme_recipients_release(rset);
	gpgme_data_release(out);
	gpgme_data_release(in);
	gpgme_release(ctx);

	dprintf(dGPG, "Successfully encrypted data:\n%s\n", retdata);

	return retdata;
}

static const char *
status_string(GpgmeSigStat status)
{
	const char *s = "?";

	switch (status) {
		case GPGME_SIG_STAT_NONE:
			s = "None";
			break;

		case GPGME_SIG_STAT_NOSIG:
			s = "No Signature";
			break;

		case GPGME_SIG_STAT_GOOD:
			s = "Good";
			break;

		case GPGME_SIG_STAT_GOOD_EXP:
			s = "Good but expired";
			break;

		case GPGME_SIG_STAT_GOOD_EXPKEY:
			s = "Good but key exipired";
			break;

		case GPGME_SIG_STAT_BAD:
			s = "Bad";
			break;

		case GPGME_SIG_STAT_NOKEY:
			s = "No Key";
			break;

		case GPGME_SIG_STAT_ERROR:
			s = "Error";
			break;

		case GPGME_SIG_STAT_DIFF:
			s = "More than one signature";
			break;
	}
	return s;
}

static const char *
validity_string(GpgmeValidity val)
{
	const char *s = "?";

	switch (val) {
		case GPGME_VALIDITY_UNKNOWN:
			s = "unknown";
			break;

		case GPGME_VALIDITY_NEVER:
			s = "not trusted";
			break;

		case GPGME_VALIDITY_MARGINAL:
			s = "marginal trusted";
			break;

		case GPGME_VALIDITY_FULL:
			s = "fully trusted";
			break;

		case GPGME_VALIDITY_UNDEFINED:
		case GPGME_VALIDITY_ULTIMATE:
			break;
	}
	return s;
}

static void
print_sig_stat(GpgmeCtx ctx, GpgmeSigStat status)
{
	const char *s;
	time_t created;
	int idx;
	GpgmeKey key;

	dprintf(dGPG, "Verification Status: %s\n", status_string(status));
    
	for(idx=0; (s=gpgme_get_sig_status(ctx, idx, &status, &created));
									idx++) {
		dprintf(dGPG, "sig %d: created: %lu expires: %lu status: %s\n",
			idx, (unsigned long)created, 
			gpgme_get_sig_ulong_attr(ctx, idx, GPGME_ATTR_EXPIRE,0),
			status_string(status));
		dprintf(dGPG, "sig %d: fpr/keyid: `%s' validity: %s\n",
			idx, s,
			validity_string(gpgme_get_sig_ulong_attr
				(ctx, idx, GPGME_ATTR_VALIDITY, 0)));
		if (!gpgme_get_sig_key(ctx, idx, &key)) {
			char *p = gpgme_key_get_as_xml(key);
			dprintf(dGPG, "sig %d: key object:\n%s\n", idx, p);
			free(p);
			gpgme_key_release(key);
		}
	}
}


char *
JGetSignature(char *text, char *sig)
{
	GpgmeCtx ctx;
	GpgmeError err;
	GpgmeData gsig, gtext;
	GpgmeSigStat status;
	char *retdata;
	char *sigdata;
	char buf[100];
	size_t nread;
	size_t retsz;
	time_t created;
	int idx;
	const char *s;

	retdata = strdup("");
	retsz = 0;

	err = gpgme_new(&ctx);
	if (err) {
		dprintf(dGPG, "Failed to create GPGME context.\n");
		return NULL;
	}

	err = gpgme_data_new_from_mem(&gtext, text, strlen(text), 0);
	if (err) {
		dprintf(dGPG, "Failed to create text data.\n");
		return NULL;
	}

	sigdata = (char *)malloc(sizeof(char) * (strlen(sig) + 1 + 72));
	sprintf(sigdata, "-----BEGIN PGP SIGNATURE-----\nVersion: JWGC\n\n%s\n-----END PGP SIGNATURE-----\n", sig);
	dprintf(dGPG, "Parsing input data:\n%s\n", sigdata);
	err = gpgme_data_new_from_mem(&gsig, sigdata, strlen(sigdata), 0);
	if (err) {
		dprintf(dGPG, "Failed to create sig data.\n");
		return NULL;
	}

	err = gpgme_op_verify(ctx, gsig, gtext, &status);
	if (err) {
		dprintf(dGPG, "Data verification failed.\n");
		return NULL;
	}

	print_sig_stat(ctx, status);

	fflush(NULL);

	idx = 0;
	s = gpgme_get_sig_status(ctx, idx, &status, &created);
	if (!s) {
		dprintf(dGPG, "Unable to determine signature.\n");
		return NULL;
	}
	retdata = strdup(s);

	gpgme_data_release(gsig);
	gpgme_data_release(gtext);
	gpgme_release(ctx);

	dprintf(dGPG, "Successfully verified signature:\n%s\n", retdata);

	return retdata;
}

void
insert_into_key_list(char *jid, char *key)
{
	int i, k;
	xode s;

	num_keys++;
	key_list = realloc(key_list, sizeof(xode) * (num_keys));
	key_list[num_keys - 1] = malloc(sizeof(xode));

	for (i = 0; i < (num_keys - 1) &&
	     (strcasecmp(jid, xode_get_attrib(key_list[i], "jid")) > 0);
	     i++);

	for (k = (num_keys - 1); k > i; k--) {
		key_list[k] = key_list[k - 1];
	}

	key_list[k] = xode_new("key");
	xode_put_attrib(key_list[k], "jid", jid);
	xode_put_attrib(key_list[k], "id", key);
}

void
JUpdateKeyList(char *jid, char *textstring, char *sigstring)
{
	int i;
	char *key;

	key = JGetSignature(textstring, sigstring);
	if (!key) {
		dprintf(dGPG, "No valid key returned, skipping.\n");
		return;
	}

	dprintf(dGPG, "KeyID is %s\n", key);

	for (i = 0; i < num_keys; i++) {
		if (!strcasecmp(xode_get_attrib(key_list[i], "jid"), jid)) {
			xode_hide_attrib(key_list[i], "id");
			xode_put_attrib(key_list[i], "id", key);
			return;
		}
	}

	insert_into_key_list(jid, key);
}

char *
JGetKeyID(char *jid)
{
	int i;
	char *key;

	for (i = 0; i < num_keys; i++) {
		if (!strcasecmp(xode_get_attrib(key_list[i], "jid"), jid)) {
			key = xode_get_attrib(key_list[i], "id");
			return key;
		}
	}

	return NULL;
}
#endif /* USE_GPGME */
