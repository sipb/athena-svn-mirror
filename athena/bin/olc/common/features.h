#define BUFLEN		10000	/* size of buffer for server info */
#define MAX_KWD_LEN	30	/* maximum length of keyword */
#define MAX_VER_LEN	3	/* maximum length (# of digits) of version # */

struct info {
    char *keyword;
    char *value;
};

