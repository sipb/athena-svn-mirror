/* $Id: libjwgc.h,v 1.1.1.2 2006-03-24 16:59:39 ghudson Exp $ */

#ifndef _LIBJWGC_H_
#define _LIBJWGC_H_ 1

#include <sysdep.h>
#include "libxode.h"
#include "libjwgc_types.h"
#include "libjwgc_debug.h"
#include "libjabber_types.h"



/* --------------------------------------------------------- */
/* JAuth.c                                                   */
/* Auth string support                                       */
/*                                                           */
/* --------------------------------------------------------- */
void	JGenerateAuth();
int	JCheckAuth(char *str);
char	*JGetAuth();



/* --------------------------------------------------------- */
/* JContact.c                                                */
/* Contact management                                        */
/*                                                           */
/* --------------------------------------------------------- */
int contact_status_change(jabpacket packet);
char *find_nickname_from_jid(char *jid);
char *find_jid_from_nickname(char *nickname);
void list_contacts(jwgconn jwg, char *matchstr, int strictmatch, int skipnotavail);
void list_contacts_bygroup(jwgconn jwg, char *matchstr, int strictmatch, int skipnotavail);
void list_agents(xode x);
int update_contact_status(char *jid, char *status, char *resource);
void update_nickname(char *target, char *nickname);
void update_group(char *target, char *group);
void remove_from_contact_list(char *contact);
int contact_exists(char *jid);
int test_match(char *matchstr, xode contact, int exactmatch);
char *find_match(char *searchstr);
void insert_into_agent_list(char *jid, char *name, char *service, int flags);



/* --------------------------------------------------------- */
/* JEncryption.c                                             */
/* GPG encryption functions                                  */
/* --------------------------------------------------------- */
#ifdef USE_GPGME
char	*JDecrypt(char *data);
char	*JEncrypt(char *data, char *recipient);
char	*JSign(char *data);
char	*JGetKeyID(char *jid);
char	*JGetSignature(char *text, char *sig);
char	*JTrimPGPMessage(char *msg);
void	JUpdateKeyList(char *jid, char *textstring, char *sigstring);
#endif /* USE_GPGME */



/* --------------------------------------------------------- */
/* JExpat.c                                                  */
/* Expat functions                                           */
/* --------------------------------------------------------- */
void	expat_startElement(void* userdata, const char* name, const char** atts);
void	expat_endElement(void* userdata, const char* name);
void	expat_charData(void* userdata, const char* s, int len);
xode	xode_str(char *str, int len);
xode	xode_file(char *file);
int	xode2file(char *file, xode node);
void	xode_put_expat_attribs(xode owner, const char** atts);



/* --------------------------------------------------------- */
/* JFile.c                                                   */
/* Communication record file functions                       */
/* --------------------------------------------------------- */
int	JSaveAuthPort();
int	JGetAuthPort();
void	JClearAuthPort();
int	JSetupComm();
int	JConnect();
void	JCleanupSocket();



/* --------------------------------------------------------- */
/* JForm.c                                                   */
/* Form handling routines                                    */
/* --------------------------------------------------------- */
xode	JFormHandler(xode form);



/* --------------------------------------------------------- */
/* JHashTable.c                                              */
/* Hash table functions                                      */
/* --------------------------------------------------------- */
NAMED	*lookup(HASH_TABLE *table, KEY name, size_t createSize);
void	hashTableInit(HASH_TABLE *);
void	hashTableDestroy(HASH_TABLE *);
void	hashTableIterInit(HASH_TABLE_ITER *, const HASH_TABLE *);
NAMED	*hashTableIterNext(HASH_TABLE_ITER *);



/* --------------------------------------------------------- */
/* JNetSock.c                                                */
/* Network socket routines                                   */
/* --------------------------------------------------------- */
#ifndef WIN32
int	make_netsocket(u_short port, char *host, int type);
int	get_netport();
struct in_addr	*make_addr(char *host);
int	set_fd_close_on_exec(int fd, int flag);
#endif



/* --------------------------------------------------------- */
/* JSha.c                                                    */
/* SHA calculations                                          */
/* --------------------------------------------------------- */
void	j_shaInit(j_SHA_CTX *ctx);
void	j_shaUpdate(j_SHA_CTX *ctx, unsigned char *dataIn, int len);
void	j_shaFinal(j_SHA_CTX *ctx, unsigned char hashout[20]);
void	j_shaBlock(unsigned char *dataIn, int len, unsigned char hashout[20]);
char	*j_shahash(char *str);
void	j_shahash_r(const char* str, char hashbuf[41]);



/* --------------------------------------------------------- */
/* JStr.c                                                    */
/* String management routines                                */
/* --------------------------------------------------------- */
char	*j_strdup(const char *str);
	/* provides NULL safe strdup wrapper */
char	*j_strcat(char *dest, char *txt);
	/* strcpy() clone */
int	j_strcmp(const char *a, const char *b);
	/* provides NULL safe strcmp wrapper */
int	j_strcasecmp(const char *a, const char *b);
	/* provides NULL safe strcasecmp wrapper */
int	j_strncmp(const char *a, const char *b, int i);
	/* provides NULL safe strncmp wrapper */
int	j_strncasecmp(const char *a, const char *b, int i);
	/* provides NULL safe strncasecmp wrapper */
int	j_strlen(const char *a);
	/* provides NULL safe strlen wrapper */
int	j_atoi(const char *a, int def);
	/* checks for NULL and uses default instead, convienence */
void	str_b64decode(char *str);
	/* what it says */
void	trim_message(char *str);
	/* trim whitespace from end of string */
xode_spool	spool_new(xode_pool p);
	/* create a string pool */
void	spooler(xode_spool s, ...);
	/* append all the char* args to the pool, terminate args with s again */
char	*spool_print(xode_spool s);
	/* return a big string */
void	spool_add(xode_spool s, char *str);
	/* add a single char to the pool */
char	*spools(xode_pool p, ...);
	/* wrap all the spooler stuff in one function, the happy fun ball! */
char	*spools(xode_pool p, ...);
	/* wrap all the spooler stuff in one function, the happy fun ball! */
/*char	*unicode_to_str(char *s); */
int	unicode_to_str(const char *in, char **out);
	/* converts a string from unicode to the local charset */
/*char	*str_to_unicode(char *s); */
int	str_to_unicode(const char *in, char **out);
	/* converts a string from the local charset to unicode */
/*char	*str_clear_unprintable(char *s); */
int	str_clear_unprintable(const char *in, char **out); 
	/* converts all unprintable characters to spaces */



/* --------------------------------------------------------- */
/* JVariables.c                                              */
/* Variable management routines                              */
/* --------------------------------------------------------- */
int	jVars_set_personal_var(char *var, char *value);
int	jVars_inset_personal_var(char *var);
int	jVars_init();
void	jVars_set_error(char *errstr);
char	*jVars_get_error();
void	jVars_set_change_handler(jVar jvar, void (*handler) ());
void	jVars_set_check_handler(jVar jvar, int (*handler) ());
void	jVars_set_show_handler(jVar jvar, int (*handler) ());
void	jVars_set_defaults_handler(void (*handler) ());
void	*jVars_get(jVar name);
char	*jVars_show(jVar name);
int	jVars_set(jVar name, void *setting);
char	*jVars_itos(jVar jvar);
jVar	jVars_stoi(char *jvar);
void	jVars_read_defaults();



/* --------------------------------------------------------- */
/* JXMLComm.c                                                */
/* XML communication routines                                */
/* --------------------------------------------------------- */
jwgpacket	jwgpacket_new(xode x);
jwgpacket	jwgpacket_reset(jwgpacket p);

jwgconn	jwg_new();
void	jwg_delete(jwgconn j);
void	jwg_packet_handler(jwgconn j, jwgconn_packet_h h);
void	jwg_start(jwgconn j);
void	jwg_stop(jwgconn j);
int	jwg_getfd(jwgconn j);
void	jwg_send(jwgconn j, xode x);
void	jwg_servsend(jwgconn j, xode x);
void	jwg_send_raw(jwgconn j, const char *str);
void	jwg_servsend_raw(jwgconn j, const char *str);
void	jwg_recv(jwgconn j);
void	jwg_servrecv(jwgconn j);
void	jwg_servsuccess(jwgconn jwg, char *text);
void	jwg_serverror(jwgconn jwg, char *text);
void	jwg_poll(jwgconn j, int timeout);
void	jwg_servpoll(jwgconn j, int timeout);
jwgconn	jwg_server();


#endif /* _LIBJWGC_H_ */
