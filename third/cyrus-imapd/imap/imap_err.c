/*
 * imap_err.c:
 * This file is automatically generated; please do not edit it.
 */
#ifdef __STDC__
#define NOARGS void
#else
#define NOARGS
#define const
#endif

static const char * const text[] = {
	"Internal Error",
	"System I/O error",
	"Operating System Error",
	"mail system storage has been exceeded",
	"Permission denied",
	"Over quota",
	"Message size exceeds fixed limit",
	"Too many user flags in mailbox",
	"Invalid namespace prefix in configuration file",
	"Mailbox has an invalid format",
	"Operation is not supported on mailbox",
	"Mailbox does not exist",
	"Mailbox already exists",
	"Invalid mailbox name",
	"Mailbox has been moved to another server",
	"Mailbox is currently reserved",
	"Mailbox is locked by POP server",
	"Unknown/invalid partition",
	"Invalid identifier",
	"Message contains NUL characters",
	"Message contains bare newlines",
	"Message contains non-ASCII characters in headers",
	"Message contains invalid header",
	"Message has no header/body separator",
	"Quota root does not exist",
	"Bad protocol",
	"Syntax error in parameters",
	"Invalid annotation entry",
	"Invalid annotation attribute",
	"Invalid annotation value",
	"Invalid server requested",
	"Server(s) unavailable to complete operation",
	"The remote Server(s) denied the operation",
	"Retry operation",
	"This mailbox hierarchy does not exist on a single backend server.",
	"Unrecognized character set",
	"Invalid user",
	"Login incorrect",
	"Anonymous login is not permitted",
	"Unsupported quota resource",
	"Authentication failed",
	"Client cancelled authentication",
	"Protocol error during authentication",
	"Mailbox is over quota",
	"Mailbox is at %d%% of quota",
	"Message %d no longer exists",
	"Unable to checkpoint \\Seen state",
	"Unable to preserve \\Seen state",
	"No matching messages",
	"No matching annotations",
	"[UNKNOWN-CTE] Can not process the binary data",
	"LOGOUT received",
	"Completed",
    0
};

struct error_table {
    char const * const * msgs;
    long base;
    int n_msgs;
};
struct et_list {
    struct et_list *next;
    const struct error_table * table;
};
extern struct et_list *_et_list;

static const struct error_table et = { text, -1904809472L, 53 };

static struct et_list link = { 0, 0 };

void initialize_imap_error_table (NOARGS) {
    if (!link.table) {
        link.next = _et_list;
        link.table = &et;
        _et_list = &link;
    }
}