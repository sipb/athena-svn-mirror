/*
 * Copyright Milan Technology Corp. 1991, 1992
 */

/*
 * fpfilter.c -- FastPort communications interface program. See man page for
 * complete set of options generates fpcomm for Adobe compatibility
 */

char           *VERSION = "@(#)fpfilter.c	2.2 10/15/92";

#include "std.h"
#include "dp.h"
#include "errors.h"
#include "udp.h"

char           *g_filter_name;	/* The invocation name */
s_options       g_opt;		/* the options struct */
FILE           *g_errorLog=0;	/* log file for errors */

long            fin_bytes;	/* Total number of bytes */
long            act_bytes_sent = 0;	/* Number of bytes actually sent */
long            int_bytes;	/* Number of par bytes initially */

/* UDP Status packet type */

extern udp_status_packet udp_stat;

int             status_id = 0;

/* SIGALARM interrupt handler. */
int             g_alarmed = 0;
#ifdef ANSI
void
to_alarm(int foo)
#else
void
to_alarm(foo)
int foo;
#endif
{
#ifndef ESIX
	signal(SIGALRM, to_alarm);
#endif
	g_alarmed = 1;
}

#ifdef ANSI
void
add_file(file_obj_ptr * flist, char *file_to_add)
#else
void
add_file(flist, file_to_add)
	file_obj_ptr   *flist;
	char           *file_to_add;
#endif
{
	file_obj_ptr    node, runner;
	node = (file_obj_ptr) malloc(sizeof(file_obj_t));
	node->next = 0;
	strncpy(node->file_name, file_to_add, MAXNAMELEN);

	if (!*flist)
		*flist = node;
	else {
		runner = *flist;
		while (runner->next)
			runner = runner->next;
		runner->next = node;
	}
}

#ifdef ANSI
int             sendfile
                (int sock, char *file_to_send, int output_dest, int check_postscript, char *printer_name)
#else
int
sendfile(sock, file_to_send, output_dest, check_postscript, printer_name)
	int             sock;
	char           *file_to_send;
	int             output_dest;
	int             check_postscript;
	char           *printer_name;
#endif
{
	int             fd;
	int             numchars, num_sent, total_sent;
	int             length, filetype;
	char            inputbuffer[MAX_BUFFER];
	char           *databuffer;
	char            ff = 12;
	char            first_chars[MAX_MAGIC_HEADER];
	char            error_string[MAXSTRNGLEN];
	int             write_error_sent = 0;
	int             i;

	pause();
	if (strcmp(file_to_send, "STDIN")) {
		if ((fd = open(file_to_send, O_RDONLY, 0400)) < 0)
			return (1);
	} else
		fd = 0;

	while ((numchars = read(fd, inputbuffer, MAX_BUFFER)) > 0) {
		total_sent = 0;

		databuffer = (g_opt.mapflg) ?
			expand_buffer(inputbuffer, numchars, &numchars) : inputbuffer;
#ifndef SLOWSCO
		/* Perform normal write operation */
		write_buffer(sock, databuffer, numchars, printer_name, output_dest);
		check_input(sock, output_dest, 0);
#ifdef SCO
		check_write(sock);
#endif

#else
		/*
		 * write only one byte to avoid any flow control problem
		 * which SCO seems to handle pretty badly
		 */
		for (i = 0; i < numchars; i++) {
			check_write(sock);
			write_buffer(sock, &databuffer[i], 1, printer_name, output_dest);
		}
#endif
	}
	check_input(sock, output_dest, 2);
	if (g_opt.ff_flag) {
		write(sock, &ff, 1);
		act_bytes_sent += 1;
	}
	close(fd);
	return (numchars);
}

/*
 * write a buffer to the socket sock On blocks it checks to see what error
 * has occurred through both the data channel and udp status All alarms and
 * alarm logic are handled here. Note--the alarm handling routine re-arms the
 * timer alarm
 */

#ifdef ANSI
void
write_buffer(int sock, char *databuffer, int numchars, char *printer_name, int output_dest)
#else
void
write_buffer(sock, databuffer, numchars, printer_name, output_dest)
	int             sock;
	char           *databuffer;
	int             numchars;
	char           *printer_name;
	int             output_dest;
#endif
{
	char            error_string[MAXSTRNGLEN];
	int             total_sent = 0;
	int             num_sent;
	int             orig_total;

#ifndef ESIX
	signal(SIGALRM, to_alarm);
#endif

#ifdef BSD
	alarm(60);
#endif
	orig_total = numchars;
	while ((num_sent = write(sock, databuffer + total_sent, numchars)) != numchars) {
		if (errno != EINTR && num_sent < 0)
			error_notify(ERR_WRITE, 0);

		if (errno == EINTR) {
			error_string[0] = 0;
			get_printer_status(printer_name, error_string, g_opt.dataport);
#ifdef BSD
			update_status_file(error_string, status_id, CREATE);
#endif
			error_notify(ERR_GENERIC, error_string);
			check_input(sock, output_dest, 0);
			continue;
		}
		check_input(sock, output_dest, 0);
		if (num_sent > 0) {
			numchars -= num_sent;
			total_sent += num_sent;
		}
#ifdef BSD
		if (numchars == 0) {
			break;
		} else
			alarm(10);
#endif
	}
#ifdef BSD
	alarm(0);		/* turn off the alarm after the write */
#endif

	/* Update # of characters sent so far */

	act_bytes_sent += orig_total;
}


/*
 * simply checks the read side of sock to see if there is any data to read
 * back and writes it to output_dest
 */

#ifdef ANSI
void
check_input(int sock, int output_dest, int time_out)
#else
void
check_input(sock, output_dest, time_out)
	int             sock, output_dest, time_out;
#endif
{
	struct timeval  timeout;
	fd_set          readfd;
	char            inchar;
	int             num_chars;
	int             sel_val;
	char            buf[MAX_BUFFER];
	char            error_string[MAXSTRNGLEN];
	int             count = 0;

	FD_ZERO(&readfd);

	timeout.tv_sec = time_out;
	timeout.tv_usec = 0;
	FD_SET(sock, &readfd);
	while (sel_val = select(sock + 1, &readfd, 0, 0, &timeout)) {
		if (sel_val < 0)
			break;
		if (FD_ISSET(sock, &readfd)) {
			if ((num_chars = read(sock, &inchar, 1)) >= 0) {
				if (count < MAX_BUFFER - 1)
					buf[count++] = inchar;
				else {
					buf[count] = (char) 0;
					error_notify(ERR_GENERIC, buf);
					count = 0;
				}
			} else
				break;
		}
	}
	if (count > 0) {
		buf[count] = (char) 0;
		error_notify(ERR_GENERIC, buf);
	}
}

#ifdef ANSI
void
send_control_d(int sock)
#else
void
send_control_d(sock)
	int             sock;
#endif
{
	char            dchar = 4;	/* control-d if a postscript printer */
	write(sock, &dchar, 1);

	/* Keep track of # of chars actually written so far */

	act_bytes_sent += 1;
}

#ifdef ANSI
char           *
expand_buffer(char *data, int length, int *retlength)
#else
char           *
expand_buffer(data, length, retlength)
	char           *data;
	int             length;
	int            *retlength;
#endif
{
	static char     return_buff[2 * MAX_BUFFER];
	int             temp, i;
	for (i = 0, temp = 0; i < length; i++) {
		if (data[i] == '\n') {
			return_buff[temp++] = '\r';
			return_buff[temp++] = '\n';
		} else {
			return_buff[temp++] = data[i];
		}
	}
	*retlength = temp;
	return (return_buff);
}

/*
 * send_banner() we need this since transcript writes a file .banner that we
 * need to write to the printer
 * 
 * ADOBE: assume that previous filters will handle BANNERPRO=file and write it
 * to .banner
 */

#ifdef ANSI
void
send_banner(int sock)
#else
void
send_banner(sock)
	int             sock;
#endif
{
	int             bfile;
	char            input_b[512];
	int             c_read, total = 0, num_sent = 0;

	if ((bfile = open(".banner", O_RDONLY, 0600)) < 0)
		return;
	while ((c_read = read(bfile, input_b, 512)) > 0) {
		act_bytes_sent += c_read;
		while ((num_sent = write(sock, input_b + total, c_read))
		       != c_read) {
			if (num_sent < 0)
				return;
			if (!num_sent)
				return;
			total += num_sent;
			c_read -= num_sent;
		}
	}
	unlink(".banner");
}

/*
 * Get the next printer from the linked list of printers and return the
 * printer in first argument. Returns a zero if all OK, else returns -1.
 */

#ifdef ANSI
int
getNextPrinter(hsw_PCONFIG ** current_ptr, struct sockaddr_in * server)
#else
int
getNextPrinter(current_ptr, server)
	hsw_PCONFIG   **current_ptr;
	struct sockaddr_in *server;
#endif
{
	static int      valid_host_name = 0;
	struct hostent *hp;
	if (!*current_ptr)
		*current_ptr = g_opt.prt_list;
	else {
		*current_ptr = (*current_ptr)->next_printer;
		if (!*current_ptr)
			*current_ptr = g_opt.prt_list;
	}

	while (*current_ptr) {
		if (hp = gethostbyname((*current_ptr)->printer_name)) {
			valid_host_name++;
			bcopy((char *) hp->h_addr, (char *) &server->sin_addr, hp->h_length);
			server->sin_port = htons((*current_ptr)->ptype);
			return (0);
		} else {
			char            error_string[MAXSTRNGLEN];
			sprintf(error_string, "host name lookup failed for %s",
				(*current_ptr)->printer_name);
			error_notify(ERR_GENERIC, error_string);
			if (!(*current_ptr)->next_printer) {
				if (!valid_host_name) {
					error_notify(ERR_GENERIC, "fpfilter: exit, no valid hosts specified");
					exit(1);
				}
			}
		}
		*current_ptr = (*current_ptr)->next_printer;
	}
	if (valid_host_name)
	  return(1);
	return(-1);
}

#ifdef ANSI
main(int argc, char **argv)
#else
main(argc, argv)
	int             argc;
	char          **argv;
#endif
{
	struct sockaddr_in server;
	hsw_PCONFIG    *current_ptr;	/* Current printer */
	char            c, *name, *temp, *cp;
	char           *Autoconfig;
	char            error_string[MAXSTRNGLEN];
	int             val;
	int             counter = 0;


	/*
	 * set to stdout or stderr depending on from where the program is
	 * called
	 */
	int             output_dest;

	int             sock, sockops, optval;
	int             was_waiting_for_printer = 0;
	int		got_status = 0;


#ifndef HPOLD
	struct linger   linger_struct;
#else
	int             lingerval = 1;
#endif

#ifdef DEBUG
	if (!access("/tmp/fpdebug", F_OK)) {
		syslog(LOG_ERR, "%s %d\n", argv[0], getpid());
		pause();
	}
#endif

#ifndef ESIX
#ifdef BSD
	signal(SIGALRM, to_alarm);
	signal(SIGCLD, sig_child);
#else
	signal(SIGCLD, SIG_IGN);
	sigignore(SIGPIPE);
#endif
#endif

	setDefaults(&g_opt);

	checkForCurrentDir(argc, argv);

	hsw_Getprinterconfig();

#ifdef BSD
	if (g_opt.acctg)
		do_acctg(argv[8]);
#endif

	/* If an error file needs to be opened, do so */
	if (g_opt.notify_type.file)
		if (!(g_errorLog = fopen(g_opt.notify_type.filename, "a")))
			g_opt.notify_type.file = 0;

	/* override with command line options */
	output_dest = parseCommandLineArgs(argc, argv);

	if (!g_opt.prt_list) {
		char            printer_name[MAXNAMELEN];
		get_printername(printer_name);
		g_opt.prt_list =
			form_printer_list(printer_name,
				    g_opt.prt_list, g_opt.dataport, APPEND);
	}
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		error_notify(ERR_DOMAIN, 0);
	server.sin_family = AF_INET;

	current_ptr = 0;
	if (getNextPrinter(&current_ptr, &server) == -1)
		error_notify(ERR_NORESPONSE, 0);
	g_alarmed = 0;
	for (;;) {		/* loop as long as we don't get a fatal error
				 * this allows engine errors to be ridden
				 * through */
		char            status_message[MAXSTRNGLEN];
		char            temp_printer[MAXSTRNGLEN];
		int             connect_error_sent = 0;

#ifdef BSD
		sprintf(status_message, "printing to %s port on %s\n",
			g_opt.dataport == PARALLEL ? "parallel" : "serial",
			current_ptr->printer_name);
		update_status_file(status_message, status_id++, CREATE);
#endif

		strcpy(temp_printer, current_ptr->printer_name);

#ifdef BSD
		alarm(30);
#endif

		if (connect(sock, (struct sockaddr *) & server, sizeof(server)) < 0) {
#ifdef BSD
			alarm(0);
#endif
			close(sock);
			if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
				error_notify(ERR_DOMAIN, 0);

			sprintf(error_string, "Error on connect (errno = %d)\nPrinter Status from Fastport:\n", errno);
			was_waiting_for_printer = 1;
			get_printer_status(current_ptr->printer_name, error_string, g_opt.dataport);
#ifdef BSD
			update_status_file(error_string, status_id, APPEND);
#endif

#ifdef BSD
			if (!connect_error_sent) {
				connect_error_sent = 1;
				error_notify(ERR_GENERIC, error_string);
			}
#else
			if (!connect_error_sent) {
				if (got_printer_error(error_string)) {
					connect_error_sent = 1;
					error_notify(ERR_GENERIC, error_string);
				}
			}
#endif
			if (getNextPrinter(&current_ptr, &server) == -1) {
			  error_notify(ERR_NORESPONSE,0);
			  exit(1);
			}
		} else
			break;
	}

#ifdef BSD
	alarm(0);
#endif

	status_id++;
#ifdef BSD
	if (was_waiting_for_printer) {
		sprintf(error_string, "printing to %s port on %s\n", g_opt.dataport == PARALLEL ? "parallel" : "serial",
			current_ptr->printer_name);
		update_status_file(error_string, status_id, CREATE);
	}
#endif

	if (g_opt.closewait) {
		/*
	 	* Now that you have connected to the printer, check the printer
	 	* status
	 	*/

		if (! get_printer_status(current_ptr->printer_name, error_string, g_opt.dataport))
			got_status = 0;
		else {
		        got_status = 1;
	 		/* Check the number of bytes written so far on the serial and
	 		 * parallel port */

			if (g_opt.dataport == PARALLEL)
				int_bytes = ntohl(udp_stat.parallel_bytes);	/* Initial # of parallel
								 * bytes */
			if (g_opt.dataport == SERIAL)
				int_bytes = ntohl(udp_stat.serial_bytes);	/* Initial # of serial
								 * bytes */
		}

	}

#ifndef HPOLD
	linger_struct.l_onoff = 1;
	linger_struct.l_linger = LINGERVAL;

#ifndef ATT
#ifdef __STDC__	
	sockops = setsockopt(sock, SOL_SOCKET, SO_LINGER,
			     (const char *)&linger_struct,
			     sizeof(linger_struct));
#else
	sockops = setsockopt(sock, SOL_SOCKET, SO_LINGER,
			     (char *)&linger_struct,
			     sizeof(linger_struct));
#endif /* __STDC__ */	
	
	if (sockops < 0)
		fprintf(stderr, "setsockopt returned %d \n", sockops);
#endif
#else
	sockops = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &lingerval,
			     sizeof(lingerval));
	if (sockops < 0)
		fprintf(stderr, "setsockopt returned %d for keepalive\n", sockops);
#endif

	optval = 1;
#ifdef __STDC__	
	sockops = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE,
			     (const char *)&optval,
			     sizeof(optval));
#else	
	sockops = setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE,
			     (char *)&optval,
			     sizeof(optval));
#endif /* __STDC__ */	
	if (sockops < 0)
		fprintf(stderr, "setsockopt returned %d for keepalive\n", sockops);

	if (g_opt.start_string) {
		write(sock, g_opt.start_string, strlen(g_opt.start_string));
		act_bytes_sent += strlen(g_opt.start_string);
	}
	if (g_opt.send_startfile)
		sendfile(sock, g_opt.start_file, output_dest, 0, current_ptr->printer_name);

	/*
	 * postscript printers really like control-d's.  So, if there was an
	 * engine problem this first control d will resync.  The last one
	 * will leave it that way.
	 */
	if (g_opt.use_control_d)
		send_control_d(sock);
	/*
	 * check to see if it should send a banner page
	 */
	if ((g_opt.real_filter || g_opt.dobanner) && g_opt.adobe.banner_first)
		send_banner(sock);
	/*
	 * if we have a list of files just send them one at a time
	 */
	if (g_opt.file_list) {
		while (g_opt.file_list) {
			if (sendfile(sock, g_opt.file_list->file_name,
				     output_dest, g_opt.check_postscript, current_ptr->printer_name))
				error_notify(ERR_SNDFILE, 0);
			g_opt.file_list = g_opt.file_list->next;
		}
		if ((g_opt.real_filter || g_opt.dobanner) && g_opt.adobe.banner_last)
			send_banner(sock);
		if (g_opt.send_endfile)
			sendfile(sock, g_opt.end_file, output_dest, 0, current_ptr->printer_name);
		if (g_opt.end_string) {
			write(sock, g_opt.end_string, strlen(g_opt.end_string));
			act_bytes_sent += strlen(g_opt.end_string);
		}
		if (g_opt.use_control_d)
			send_control_d(sock);
	} else {
		/*
		 * Reading from standard input can either be through lpr or
		 * just piping into the filter from the shell
		 */

		sendfile(sock, "STDIN", output_dest, g_opt.check_postscript, current_ptr->printer_name);
		if ((g_opt.real_filter || g_opt.dobanner) && g_opt.adobe.banner_last)
			send_banner(sock);

		if (g_opt.send_endfile)
			sendfile(sock, g_opt.end_file, output_dest, 0, current_ptr->printer_name);


		if (g_opt.end_string) {
			write(sock, g_opt.end_string, strlen(g_opt.end_string));
			act_bytes_sent += strlen(g_opt.end_string);
		}
		if (g_opt.use_control_d)
			send_control_d(sock);
	}

	/* Now that you have finished printing, check the status again */

	if (g_opt.closewait) {
		int sent_status = 0;
	        if (! got_status ) {
		    sleep(10);
		} else 
		do {
			error_string[0] = 0;
			get_printer_status(current_ptr->printer_name, error_string, g_opt.dataport);
			if (counter > 0) {
				if ((! sent_status)  && (g_opt.dataport == PARALLEL) ) {
					sent_status = 1;
					if (got_printer_error(error_string)) {
						error_notify(ERR_GENERIC, error_string);
					}
				}
				check_input(sock, output_dest, 0);
				sleep(1);
			}

			/*
			 * Check the number of bytes written so far on the
			 * serial and parallel port
			 */
			if (g_opt.dataport == PARALLEL)
				fin_bytes = ntohl(udp_stat.parallel_bytes);	/* Final # of par bytes */
			if (g_opt.dataport == SERIAL)
				fin_bytes = ntohl(udp_stat.serial_bytes);
			counter++;
		}
		while ((fin_bytes < (int_bytes + act_bytes_sent)) && (counter < 60));

	}
	close(sock);
	if (g_opt.use_printer_classes) {
		char            message[MAXSTRNGLEN];
		sprintf(message, "The file was printed on %s \n", current_ptr->printer_name);
		error_notify(ERR_GENERIC, message);
	}
	if (g_errorLog)
		fclose(g_errorLog);
	exit(0);
}

#ifdef BSD
void
#ifdef ANSI
update_status_file(char *message, int status_num, int style)
#else
update_status_file(message, status_num, style)
	char           *message;
	int             status_num;
	int             style;
#endif
{
	int             sf;
	static int      last_id = -1;

	if (style == APPEND)
		style = O_APPEND;
	else
		style = 0;

	if ((sf = open("./status", O_RDWR | style, 0600)) >= 0) {
		if (style == CREATE)
			ftruncate(sf, 0);
		write(sf, message, strlen(message));
		close(sf);
	} else {
		if (status_num != last_id) {
			error_notify(ERR_GENERIC, message);
			last_id = status_id;
		}
	}
}

#endif


#ifdef SCO

void
check_write(sock)
	int             sock;
{

	struct timeval  timeout;
	fd_set          writefd;
	int             sel_val;
	struct pollfd   pollfds[20];
	int             i;

	for (i = 0; i < 20; i++)
		pollfds[i].events = 0;

	pollfds[sock].events = POLLOUT;
	pollfds[sock].fd = sock;

	while (1) {
		if (poll(pollfds, 20, -1) < 0) {
			perror("Poll failed");
			exit(1);
		}
		if (pollfds[sock].revents == POLLOUT)
			return;
	}
}

#endif

int
#ifdef ANSI
got_printer_error(char *error_string)
#else
got_printer_error(error_string)
	char           *error_string;
#endif
{
	char           *c_ptr;
	if (is_substring("OFFLINE", error_string))
		return (1);
	if (is_substring("FAULT", error_string))
		return (1);
	if (is_substring("PAPER", error_string))
		return (1);
	return (0);
}

int
#ifdef ANSI
is_substring(char *sub, char *src)
#else
is_substring(sub, src)
	char           *sub, *src;
#endif
{
	while (*src) {
		if (!strncmp(src, sub, strlen(sub)))
			return (1);
		src++;
	}
	return (0);
}

/*
 * This routine performs accounting. Here we look at the '.banner' file
 * generated by the output filter in the spool directory and append the
 * contents of this file to the accounting file whose name and path are
 * passed as the 8th argument to the filter program. This SHOULD be used only
 * when you are printing through 'lpr' command and only on BSD systems.
 */

#ifdef BSD
void
do_acctg(file)
	char           *file;
{
	FILE           *acctg, *banner;
	char           *str;

	str = (char *) malloc(80);

	if ((acctg = fopen(file, "a")) == NULL) {
		error_notify(ERR_GENERIC, "can not open accounting file");
		return;
	}
	if ((banner = fopen("./.acct", "r")) == NULL){
		error_notify(ERR_GENERIC, "can not perform accounting !");
		return;
	}
	fgets(str, 80, banner);
	fputs(str, acctg);
	fclose(banner);
	fclose(acctg);
}
#endif
