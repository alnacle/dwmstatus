/*
 * See LICENSE file 
 */
#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <stdarg.h>
#include <X11/Xlib.h>

void die(const char* format, ...) {
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);
    exit(EXIT_FAILURE);
}

char *
smprintf(char *fmt, ...)
{
	va_list fmtargs;
	char *ret;
	int len;

	va_start(fmtargs, fmt);
	len = vsnprintf(NULL, 0, fmt, fmtargs);
	va_end(fmtargs);

	ret = malloc(++len);
	if (ret == NULL) {
		perror("malloc");
		exit(1);
	}

	va_start(fmtargs, fmt);
	vsnprintf(ret, len, fmt, fmtargs);
	va_end(fmtargs);

	return ret;
}
int 
getconnections() {
    FILE *pp;
    char *line;
    char buf[1000];
    pp = popen("netstat -ant | grep ESTABLISHED | wc -l ", "r");

    if (pp != NULL) {
        line = fgets(buf, sizeof buf, pp);
        pclose(pp);
    }
    return atoi(line);
}

char *
getdatetime() {
	char *buf;
	time_t result;
	struct tm *resulttm;

	if((buf = malloc(sizeof(char)*65)) == NULL) {
		die("Cannot allocate memory for buf.\n");
	}
	result = time(NULL);
	resulttm = localtime(&result);
	if(resulttm == NULL) {
		die("Error getting localtime.\n");
	}
	if(!strftime(buf, sizeof(char)*65-1, "%a %b %d %H:%M:%S", resulttm)) {
		die("strftime is 0.\n");
	}
	
	return buf;
}

int 
getbattery() {
    FILE *pp;
    char *line;
    char buf[1000];

    pp = popen("apm -l", "r");
    if (pp != NULL) {
        line = fgets(buf, sizeof buf, pp);
        pclose(pp);
    }
    return atoi(line);
}

char *
getload(void)
{
	double avgs[3];

	if (getloadavg(avgs, 3) < 0) {
		perror("getloadavg");
		exit(1);
	}

	return smprintf("%.2f %.2f %.2f", avgs[0], avgs[1], avgs[2]);
}

int main(void) {
	char *status;
	char *datetime;
    char *loadavg;

	int battery;
    int connects;

    Display *dpy;

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "Cannot open display.\n");
		return 1;
	}

	if((status = malloc(200)) == NULL)
		exit(1);
	
    char buf[200];

	for (;;sleep(1)) {
		datetime = getdatetime();
        loadavg  = getload();
		battery  = getbattery();
        connects = getconnections();

		snprintf(buf, 200, "TCP: %d | %d%% | %s | %s", 
                               connects, battery, loadavg, datetime);

		free(datetime);

    	XStoreName(dpy, DefaultRootWindow(dpy), buf);
	    XSync(dpy, False);
	}

	free(status);
	XCloseDisplay(dpy);

	return 0;
}

