/*
 * nntp_err.c:
 * This file is automatically generated; please do not edit it.
 */
#ifdef __STDC__
#define NOARGS void
#else
#define NOARGS
#define const
#endif

static const char * const text[] = {
	"No newsgroups header in article",
	"Error parsing newgroups header",
	"Already have this article",
	"Error transferring this article",
	"Unknown control message",
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

static const struct error_table et = { text, -1567905280L, 5 };

static struct et_list link = { 0, 0 };

void initialize_nntp_error_table (NOARGS) {
    if (!link.table) {
        link.next = _et_list;
        link.table = &et;
        _et_list = &link;
    }
}