/*
   file: lert.h
   mike barker
   Dec. 2, 1994
 */

/*
  highly likely to need changing
 */

/* for /etc/services lookup */
#define LERT_SERVED  "lert"
#define LERT_PROTO   "udp"

/* for hesiod resolution */
#define LERT_SERVER  "lert"
#define LERT_TYPE    "sloc"

/* and when all else fails, hardcode! */
#define LERT_HOME    "minos"

#define LERT_PORT     3717
#define LERT_SERVICE "daemon"

/* where the data base and log are */
#define LERTS_DATA   "/var/ops/lert/lertdata"
#define LERTS_LOG    "/var/ops/lert/lertlog"
#define LERTS_SRVTAB    "/var/ops/lert/srvtab"

/* the base name of the displayed files */
#define LERTS_MSG_FILES   "/afs/athena/system/config/lert/lert"
#define LERTS_MSG_SUBJECT   "/afs/athena/system/config/lert/lertsub"
#define LERTS_DEF_SUBJECT  "Message from Lert"

/* how many times should we send it */
#define RETRIES      5
#define LERT_TIMEOUT  1

/* should the server produce a message every time a user hits it? */
#define LOGGING      3

/*
  not as likely to need changing
 */

/* for various types of delivery */
#define LERT_CAT      0
#define LERT_Z        1
#define LERT_MAIL     2
#define LERT_HANDLE   3

#define LERT_VERSION '1'
#define LERT_ASKS    '0'

/* various categories of response from server
   bad means something strange in instance, realm, etc...
   free means you aren't in the database!
   msg means lert is going to pinch you
   and sick means lert doesn't understand her data...
 */

#define LERT_BAD     '0'
#define LERT_FREE    '1'
#define LERT_MSG     '2'
#define LERT_SICK    '3'

#define LERT_GOTCHA     0
#define LERT_NO_DB     -1
#define LERT_NOT_IN_DB -2

/* 
  conceivably a future version might use more bytes
  currently:
  client -> server
  [0] version
  [1] stop msgs flag 0 or 1
  [2] reserved for future
  [3] reserved for future
  [4 on] authentication, etc.
 */

#define LERT_LENGTH   4

/*
  currently:
  server->client
  [0] LERT_VERSION
  [1] code response
  [2 on] data
 */

#define LERT_CHECK 2

/* bombout codes */
#define ERR_KERB_PHOST    1
#define ERR_HOSTNAME      2
#define ERR_SOCKET        3
#define ERR_CONNECT       4
#define ERR_SEND          5
#define ERR_RCV           6
#define ERR_VERSION       7
#define ERR_SERVER        8
#define ERR_KERB_REALM    9
#define ERR_KERB_FAKE     10
#define ERR_KERB_CRED     11

#define ERR_KERB_AUTH   109
#define ERR_TIMEOUT     110

#define ERR_USER        201
#define LERT_NO_SOCK     202

#define NO_PROCS        301
#define ERR_MEMORY      302
#define ERR_SERVED      303

/* for ease of use */
#ifndef FALSE
#define FALSE  0
#endif
#ifndef TRUE
#define TRUE  1
#endif
