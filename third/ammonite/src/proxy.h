/* config settings */

#ifndef _PROXY_H_
#define _PROXY_H_

#include <stdio.h>

typedef struct {
	FILE *logf;
	char *logf_name;
	char *target_path;
	char *upstream_host;
	int upstream_port;
	gboolean specified_upstream_host;
	int local_port;
	gboolean append_log;
	gboolean use_stderr;
	gboolean use_ssl;
	gboolean use_oaf;
	gboolean quit;
#ifndef NO_DEBUG_MIRRORING
	char *mirror_log_dir;
#endif
} Config;

struct stat_s {
	unsigned long int num_reqs;
	unsigned long int num_cons;
	unsigned long int num_badcons;
	unsigned long int num_opens;
	unsigned long int num_listens;
	unsigned long int num_tx;
	unsigned long int num_rx;
	unsigned long int num_garbage;
	unsigned long int num_idles;
	unsigned long int num_refused;
};

extern Config config;
extern struct stat_s stats;

#define PORT_START	11600
#define PORT_END	11900


#endif	/* _PROXY_H_ */
