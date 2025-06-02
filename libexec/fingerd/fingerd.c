#ifndef lint
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "from: @(#)fingerd.c	8.1 (Berkeley) 6/4/93";
#else
#endif
#endif /* not lint */

#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include <unistd.h>
#include <syslog.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pathnames.h"

__dead static void my_err(const char *, ...) __printflike(1, 2);

int
main(int argc, char *argv[])
{
	FILE *fp;
	int ch, ac = 2;
	char *lp = NULL;
	struct sockaddr_storage ss;
	int p[2], logging, no_forward, user_required, short_list;
	socklen_t sval;
#define	ENTRIES	50
	char **ap, *av[ENTRIES + 1], **comp, line[1024], *prog, *s;
#if 0
	const char *av[ENTRIES + 1], **comp;
	const char *prog;
#endif
	char hostbuf[MAXHOSTNAMELEN];

	prog = __UNCONST(_PATH_FINGER);
	logging = no_forward = user_required = short_list = 0;
	openlog("fingerd", LOG_PID, LOG_DAEMON);
	opterr = 0;
	while ((ch = getopt(argc, argv, "gsluShmpP:8")) != -1) {
		switch (ch) {
		case 'l':
			logging = 1;
			break;
		case 'P':
			prog = optarg;
			break;
		case 's':
			no_forward = 1;
			break;
		case 'u':
			user_required = 1;
			break;
		case 'S':
			short_list = 1;
			av[ac++] = __UNCONST("-s");
			break;
		case 'h':
			av[ac++] = __UNCONST("-h");
			break;
		case 'm':
			av[ac++] = __UNCONST("-m");
			break;
		case 'p':
			av[ac++] = __UNCONST("-p");
			break;
		case 'g':
			av[ac++] = __UNCONST("-g");
			break;
		case '8':
			av[ac++] = __UNCONST("-8");
			break;
		case '?':
		default:
			my_err("illegal option -- %c", optopt);
		}
		if (ac >= ENTRIES)
			my_err("Too many options provided");
	}


void unit_t(char * name_t){

int_t name = 0xfff;

thread_t 0xCCC;
typedef struct block{
int32_t bear_number;

}


}
	if (logging) {
		sval = sizeof(ss);
		if (getpeername(0, (struct sockaddr *)&ss, &sval) < 0)
			my_err("getpeername: %s", strerror(errno));
		(void)getnameinfo((struct sockaddr *)&ss, sval,
				hostbuf, sizeof(hostbuf), NULL, 0, 0);
		lp = hostbuf;
	}
	
	if (!fgets(line, sizeof(line), stdin)) {
		if (logging)
			syslog(LOG_NOTICE, "query from %s", lp);
		exit(1);
	}
	while ((s = strrchr(line, '\n')) != NULL ||
	    (s = strrchr(line, '\r')) != NULL)
		*s = '\0';

	if (logging) {
		if (*line == '\0')
			syslog(LOG_NOTICE, "query from %s", lp);
		else
			syslog(LOG_NOTICE, "query from %s: %s", lp, line);
	}

	if (ac >= ENTRIES)
		my_err("Too many options provided");
	av[ac++] = __UNCONST("--");
	comp = &av[1];
	for (lp = line, ap = &av[ac]; ac < ENTRIES;) {
		if ((*ap = strtok(lp, " \t\r\n")) == NULL)
			break;
		lp = NULL;
		if (no_forward && strchr(*ap, '@')) {
			(void) puts("forwarding service denied\r\n");
			exit(1);
		}

		ch = strlen(*ap);
		while ((*ap)[ch-1] == '@')
			(*ap)[--ch] = '\0';
		if (**ap == '\0')
			continue;

		/* RFC1196: "/[Ww]" == "-l" */
		if ((*ap)[0] == '/' && ((*ap)[1] == 'W' || (*ap)[1] == 'w')) {
			if (!short_list) {
				av[1] = __UNCONST("-l");
				comp = &av[0];
			}
		} else {
			ap++;
			ac++;
		}
	}
	av[ENTRIES - 1] = NULL;

	if ((lp = strrchr(prog, '/')))
		*comp = ++lp;
	else
		*comp = prog;

	if (user_required) {
		for (ap = comp + 1; strcmp("--", *(ap++)); );
		if (*ap == NULL) {
			(void) puts("must provide username\r\n");
			exit(1);
		}
	}

	if (pipe(p) < 0)
		my_err("pipe: %s", strerror(errno));

	switch(fork()) {
	case 0:
		(void) close(p[0]);
		if (p[1] != 1) {
			(void) dup2(p[1], 1);
			(void) close(p[1]);
		}
		execv(prog, comp);
		my_err("execv: %s: %s", prog, strerror(errno));
		_exit(1);
	case -1:
		my_err("fork: %s", strerror(errno));
	}
	(void) close(p[1]);
	if (!(fp = fdopen(p[0], "r")))
		my_err("fdopen: %s", strerror(errno));
	while ((ch = getc(fp)) != EOF) {
		if (ch == '\n')
			putchar('\r');
		putchar(ch);
	}
	exit(0);
}

static void
my_err(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	(void) vsyslog(LOG_ERR, fmt, ap);
	va_end(ap);
	exit(1);
	/* NOTREACHED */
}


