/* auto-generated by config2header 1.5 */

/* DO NOT EDIT */

/* THIS FILE AUTOMATICALLY GENERATED BY config2header 1.5 */

#include <config.h>

#include <sys/types.h>

#ifdef HAVE_UNISTD_H

#include <unistd.h>

#endif

#include <syslog.h>

#include <com_err.h>

#include <stdlib.h>

#include <string.h>

#include "imapopts.h"



struct imapopt_s imapopts[] =

{

    { IMAPOPT_ZERO, "", 0, { NULL }, OPT_NOTOPT },

  { IMAPOPT_ADMINS, "admins", 0, (union config_value)((const char *) ""), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_ALLOWALLSUBSCRIBE, "allowallsubscribe", 0, (union config_value)((int) 0), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_ALLOWANONYMOUSLOGIN, "allowanonymouslogin", 0, (union config_value)((int) 0), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_ALLOWAPOP, "allowapop", 0, (union config_value)((int) 1), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_ALLOWNEWNEWS, "allownewnews", 0, (union config_value)((int) 0), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_ALLOWPLAINTEXT, "allowplaintext", 0, (union config_value)((int) 1), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_ALLOWUSERMOVES, "allowusermoves", 0, (union config_value)((int) 0), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_ALTNAMESPACE, "altnamespace", 0, (union config_value)((int) 0), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_ANNOTATION_DB, "annotation_db", 0, (union config_value)((const char *) "skiplist"), OPT_STRINGLIST, { { "berkeley" , IMAP_ENUM_ZERO }, { "skiplist" , IMAP_ENUM_ZERO },  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_AUTOCREATEQUOTA, "autocreatequota", 0, (union config_value)((int) 0), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_CONFIGDIRECTORY, "configdirectory", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_DEBUG_COMMAND, "debug_command", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_DEFAULTACL, "defaultacl", 0, (union config_value)((const char *) "anyone lrs"), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_DEFAULTDOMAIN, "defaultdomain", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_DEFAULTPARTITION, "defaultpartition", 0, (union config_value)((const char *) "default"), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_DELETERIGHT, "deleteright", 0, (union config_value)((const char *) "c"), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_DUPLICATE_DB, "duplicate_db", 0, (union config_value)((const char *) "berkeley-nosync"), OPT_STRINGLIST, { { "berkeley" , IMAP_ENUM_ZERO }, { "berkeley-nosync" , IMAP_ENUM_ZERO }, { "skiplist" , IMAP_ENUM_ZERO },  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_DUPLICATESUPPRESSION, "duplicatesuppression", 0, (union config_value)((int) 1), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_FOOLSTUPIDCLIENTS, "foolstupidclients", 0, (union config_value)((int) 0), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_FULLDIRHASH, "fulldirhash", 0, (union config_value)((int) 0), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_HASHIMAPSPOOL, "hashimapspool", 0, (union config_value)((int) 0), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_IDLESOCKET, "idlesocket", 0, (union config_value)((const char *) "{configdirectory}/socket/idle"), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_IGNOREREFERENCE, "ignorereference", 0, (union config_value)((int) 0), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_IMAPIDLEPOLL, "imapidlepoll", 0, (union config_value)((int) 60), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_IMAPIDRESPONSE, "imapidresponse", 0, (union config_value)((int) 1), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_IMPLICIT_OWNER_RIGHTS, "implicit_owner_rights", 0, (union config_value)((const char *) "lca"), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_LDAP_BASE, "ldap_base", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_LDAP_FILTER, "ldap_filter", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_LDAP_SERVERS, "ldap_servers", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_LDAP_SASL, "ldap_sasl", 0, (union config_value)((int) 1), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_LDAP_SASL_AUTHC, "ldap_sasl_authc", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_LDAP_SASL_AUTHZ, "ldap_sasl_authz", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_LDAP_SASL_MECH, "ldap_sasl_mech", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_LDAP_SASL_PASSWORD, "ldap_sasl_password", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_LDAP_SASL_REALM, "ldap_sasl_realm", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_LDAP_MEMBER_ATTRIBUTE, "ldap_member_attribute", 0, (union config_value)((const char *) "memberOf"), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_LMTP_DOWNCASE_RCPT, "lmtp_downcase_rcpt", 0, (union config_value)((int) 0), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_LMTP_OVER_QUOTA_PERM_FAILURE, "lmtp_over_quota_perm_failure", 0, (union config_value)((int) 0), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_LMTPSOCKET, "lmtpsocket", 0, (union config_value)((const char *) "{configdirectory}/socket/lmtp"), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_LOGINREALMS, "loginrealms", 0, (union config_value)((const char *) ""), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_LOGINUSEACL, "loginuseacl", 0, (union config_value)((int) 0), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_LOGTIMESTAMPS, "logtimestamps", 0, (union config_value)((int) 0), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_MAILNOTIFIER, "mailnotifier", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_MAXMESSAGESIZE, "maxmessagesize", 0, (union config_value)((int) 0), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_MBOXLIST_DB, "mboxlist_db", 0, (union config_value)((const char *) "skiplist"), OPT_STRINGLIST, { { "flat" , IMAP_ENUM_ZERO }, { "berkeley" , IMAP_ENUM_ZERO }, { "skiplist" , IMAP_ENUM_ZERO },  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_MUPDATE_CONNECTIONS_MAX, "mupdate_connections_max", 0, (union config_value)((int) 128), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_MUPDATE_AUTHNAME, "mupdate_authname", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_MUPDATE_PASSWORD, "mupdate_password", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_MUPDATE_PORT, "mupdate_port", 0, (union config_value)((int) 3905), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_MUPDATE_REALM, "mupdate_realm", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_MUPDATE_RETRY_DELAY, "mupdate_retry_delay", 0, (union config_value)((int) 20), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_MUPDATE_SERVER, "mupdate_server", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_MUPDATE_WORKERS_START, "mupdate_workers_start", 0, (union config_value)((int) 5), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_MUPDATE_WORKERS_MINSPARE, "mupdate_workers_minspare", 0, (union config_value)((int) 2), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_MUPDATE_WORKERS_MAXSPARE, "mupdate_workers_maxspare", 0, (union config_value)((int) 10), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_MUPDATE_WORKERS_MAX, "mupdate_workers_max", 0, (union config_value)((int) 50), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_MUPDATE_USERNAME, "mupdate_username", 0, (union config_value)((const char *) ""), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_NETSCAPEURL, "netscapeurl", 0, (union config_value)((const char *) "http://asg.web.cmu.edu/cyrus/imapd/netscape-admin.html"), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_NEWSMASTER, "newsmaster", 0, (union config_value)((const char *) "news"), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_NEWSPEER, "newspeer", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_NEWSPOSTUSER, "newspostuser", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_NEWSPREFIX, "newsprefix", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_NOTIFYSOCKET, "notifysocket", 0, (union config_value)((const char *) "{configdirectory}/socket/notify"), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_PLAINTEXTLOGINPAUSE, "plaintextloginpause", 0, (union config_value)((int) 0), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_POPEXPIRETIME, "popexpiretime", 0, (union config_value)((int) -1), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_POPMINPOLL, "popminpoll", 0, (union config_value)((int) 0), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_POPTIMEOUT, "poptimeout", 0, (union config_value)((int) 10), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_POSTMASTER, "postmaster", 0, (union config_value)((const char *) "postmaster"), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_POSTSPEC, "postspec", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_POSTUSER, "postuser", 0, (union config_value)((const char *) ""), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_PROXY_AUTHNAME, "proxy_authname", 0, (union config_value)((const char *) "proxy"), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_PROXY_PASSWORD, "proxy_password", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_PROXY_REALM, "proxy_realm", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_PROXYD_ALLOW_STATUS_REFERRAL, "proxyd_allow_status_referral", 0, (union config_value)((int) 0), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_PROXYSERVERS, "proxyservers", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_PTSCACHE_DB, "ptscache_db", 0, (union config_value)((const char *) "berkeley"), OPT_STRINGLIST, { { "berkeley" , IMAP_ENUM_ZERO }, { "skiplist" , IMAP_ENUM_ZERO },  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_PTSCACHE_TIMEOUT, "ptscache_timeout", 0, (union config_value)((int) 10800), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_PTSKRB5_SLASHTODOT, "ptskrb5_slashtodot", 0, (union config_value)((int) 0), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_QUOTAWARN, "quotawarn", 0, (union config_value)((int) 90), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_QUOTAWARNKB, "quotawarnkb", 0, (union config_value)((int) 0), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_REJECT8BIT, "reject8bit", 0, (union config_value)((int) 0), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_RFC2046_STRICT, "rfc2046_strict", 0, (union config_value)((int) 0), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_RFC3028_STRICT, "rfc3028_strict", 0, (union config_value)((int) 1), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_SASL_MAXIMUM_LAYER, "sasl_maximum_layer", 0, (union config_value)((int) 256), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_SASL_MINIMUM_LAYER, "sasl_minimum_layer", 0, (union config_value)((int) 0), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_SEENSTATE_DB, "seenstate_db", 0, (union config_value)((const char *) "skiplist"), OPT_STRINGLIST, { { "flat" , IMAP_ENUM_ZERO }, { "berkeley" , IMAP_ENUM_ZERO }, { "skiplist" , IMAP_ENUM_ZERO },  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_SENDMAIL, "sendmail", 0, (union config_value)((const char *) "/usr/lib/sendmail"), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_SERVERNAME, "servername", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_SHAREDPREFIX, "sharedprefix", 0, (union config_value)((const char *) "Shared Folders"), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_SIEVE_MAXSCRIPTSIZE, "sieve_maxscriptsize", 0, (union config_value)((int) 32), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_SIEVE_MAXSCRIPTS, "sieve_maxscripts", 0, (union config_value)((int) 5), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_SIEVEDIR, "sievedir", 0, (union config_value)((const char *) "/usr/sieve"), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_SIEVENOTIFIER, "sievenotifier", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_SIEVEUSEHOMEDIR, "sieveusehomedir", 0, (union config_value)((int) 0), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_SINGLEINSTANCESTORE, "singleinstancestore", 0, (union config_value)((int) 1), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_SKIPLIST_UNSAFE, "skiplist_unsafe", 0, (union config_value)((int) 0), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_SOFT_NOAUTH, "soft_noauth", 0, (union config_value)((int) 1), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_SRVTAB, "srvtab", 0, (union config_value)((const char *) ""), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_SUBSCRIPTION_DB, "subscription_db", 0, (union config_value)((const char *) "flat"), OPT_STRINGLIST, { { "flat" , IMAP_ENUM_ZERO }, { "berkeley" , IMAP_ENUM_ZERO }, { "skiplist" , IMAP_ENUM_ZERO },  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_SYSLOG_PREFIX, "syslog_prefix", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_TEMP_PATH, "temp_path", 0, (union config_value)((const char *) "/tmp"), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_TIMEOUT, "timeout", 0, (union config_value)((int) 30), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_TLS_CA_FILE, "tls_ca_file", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_TLS_CA_PATH, "tls_ca_path", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_TLSCACHE_DB, "tlscache_db", 0, (union config_value)((const char *) "berkeley-nosync"), OPT_STRINGLIST, { { "berkeley" , IMAP_ENUM_ZERO }, { "berkeley-nosync" , IMAP_ENUM_ZERO }, { "skiplist" , IMAP_ENUM_ZERO },  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_TLS_CERT_FILE, "tls_cert_file", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_TLS_CIPHER_LIST, "tls_cipher_list", 0, (union config_value)((const char *) "DEFAULT"), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_TLS_KEY_FILE, "tls_key_file", 0, (union config_value)((const char *) NULL), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_TLS_REQUIRE_CERT, "tls_require_cert", 0, (union config_value)((int) 0), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_TLS_SESSION_TIMEOUT, "tls_session_timeout", 0, (union config_value)((int) 1440), OPT_INT, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_UMASK, "umask", 0, (union config_value)((const char *) "077"), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_USERNAME_TOLOWER, "username_tolower", 0, (union config_value)((int) 1), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_USERPREFIX, "userprefix", 0, (union config_value)((const char *) "Other Users"), OPT_STRING, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_UNIX_GROUP_ENABLE, "unix_group_enable", 0, (union config_value)((int) 1), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_UNIXHIERARCHYSEP, "unixhierarchysep", 0, (union config_value)((int) 0), OPT_SWITCH, {  { NULL, IMAP_ENUM_ZERO } } },
  { IMAPOPT_VIRTDOMAINS, "virtdomains", 0, (union config_value)((enum enum_value) IMAP_ENUM_VIRTDOMAINS_OFF), OPT_ENUM, { { "off" , IMAP_ENUM_VIRTDOMAINS_OFF }, { "userid" , IMAP_ENUM_VIRTDOMAINS_USERID }, { "on" , IMAP_ENUM_VIRTDOMAINS_ON },  { NULL, IMAP_ENUM_ZERO } } },

  { IMAPOPT_LAST, NULL, 0, { NULL }, OPT_NOTOPT }

};



/* c code goes here */


