#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*struct uhook*/
#define   MAX_ARGV_LEN	 512			/*The length of the buffer used to staorge argv*/
#define   UHOOK_STA_LEN	 32			/*The length of the buffer used to staorge argv*/
#define   KSYM_NAME_LEN  128
struct uhook{
	char 		fun_name[KSYM_NAME_LEN];/*function name called from userspace*/
	char 		argv[MAX_ARGV_LEN];	/*argv of function*/
	int 		argc;			/*number of arg*/
	int		ret;			/*return value*/
	char		status[UHOOK_STA_LEN];	/*status of kernel func run*/
	unsigned long	addr;			/*Address of kernel symbal*/
};


#define   UHOOKCMD_QUERY_FUNC	1		/*CMD used to query if there is a symbal in kernel*/
#define   UHOOKCMD_QUERY_VAL	2		/*CMD used to query the value of a argument in kernel*/
#define   UHOOKCMD_RUN		3		/*CMD used to run the func in kernel*/

#define	  DEV_NAME		"/dev/uhook"

static int parse_cmd(char *cmd)
{
	if (strcmp(cmd, "query") == 0) {
		return UHOOKCMD_QUERY_FUNC;
	}
	if (strcmp(cmd, "run") == 0) {
		return UHOOKCMD_RUN;
	}
	if (strcmp(cmd, "query_val") == 0) {
		return UHOOKCMD_QUERY_VAL;
	}
}

static int parse_result(struct uhook *uhook, char *cmd)
{
	printf("The ret of cmd(%s) is %d\n", cmd, uhook->ret);	
	printf("The address of symbal(%s) is 0x%x\n", uhook->fun_name, uhook->addr);	
	printf("The status of ksymbal(%s) run is %s\n", uhook->fun_name, uhook->status);
}

static void build_uhook(struct uhook *uhook, char *name)
{
	strcpy(uhook->fun_name, name);
}
/*
 * call type:
 * uhook    --type   	run 		func
 * argv[0] argv[1]	argv[2]		argv[3]*/
int main(int argc, char **argv)
{

	int uhook_fd = open(DEV_NAME, O_RDWR);
	struct uhook	uhook;
	memset(&uhook, 0, sizeof(struct uhook));

	switch(parse_cmd(argv[2])) {
		case UHOOKCMD_QUERY_FUNC:
			build_uhook(&uhook, argv[3]);
			ioctl(uhook_fd, UHOOKCMD_QUERY_FUNC, &uhook);
			parse_result(&uhook, "query");
			break;
		case UHOOKCMD_RUN:
			build_uhook(&uhook, argv[3]);
			ioctl(uhook_fd, UHOOKCMD_RUN, &uhook);
			parse_result(&uhook, "run");
			break;
		case UHOOKCMD_QUERY_VAL:
			build_uhook(&uhook, argv[3]);
			ioctl(uhook_fd, UHOOKCMD_QUERY_VAL, &uhook);
			parse_result(&uhook, "query val");
			break;
			
		default:
			printf("Unkown cmd\n");
	}
	close(uhook_fd);
	return 0;
}

