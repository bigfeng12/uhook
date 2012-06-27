#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <getopt.h>

/*struct uhook*/
#define   UHOOK_STA_LEN	 32			/*The length of the buffer used to staorge argv*/
#define	  VERSION_LEN    128 
#define   KSYM_NAME_LEN  128
#define   MAX_ARGV_LEN	 512			/*The length of the buffer used to staorge argv*/


#define   APP_VERSION  "v0.1"
struct uhook_version{
	char		app_version[VERSION_LEN];	/*app version*/
	char		kernel_version[VERSION_LEN];	/*kernel verion like 3.3.1*/
	char		module_version[VERSION_LEN];	/*uhook kernel version*/
};
struct argv{
	int arg0, arg1, arg2, arg3, arg4;
};
struct uhook{
	char 			fun_name[KSYM_NAME_LEN];/*function name called from userspace*/
	struct argv 		argv;			/*argv of function*/
	int 			argc;			/*number of arg*/
	int 			cmd;			/*which cmd: qurey, run, query_val*/
	struct uhook_version 	version;		/*software version*/
	int			ret;			/*return value*/
	char			status[UHOOK_STA_LEN];	/*status of kernel func run*/
	unsigned long		addr;			/*Address of kernel symbal*/
};


#define   UHOOKCMD_QUERY_FUNC	1		/*CMD used to query if there is a symbal in kernel*/
#define   UHOOKCMD_QUERY_VAL	2		/*CMD used to query the value of a argument in kernel*/
#define   UHOOKCMD_RUN		3		/*CMD used to run the func in kernel*/
#define   UHOOKCMD_GET_VER	4		/*CMD used to get version of both app and kernel, uhook kernel 
						  module*/

#define	  DEV_NAME		"/dev/uhook"

static int parse_cmd(char *type)
{
	if (strcmp(type, "query") == 0) {
		return UHOOKCMD_QUERY_FUNC;
	}
	if (strcmp(type, "run") == 0) {
		return UHOOKCMD_RUN;
	}
	if (strcmp(type, "query_val") == 0) {
		return UHOOKCMD_QUERY_VAL;
	}
}

static int parse_result(struct uhook *uhook, char *cmd)
{
	printf("ret of cmd(%s):\n\t%d\n", cmd, uhook->ret);	
	printf("address of symbal(%s):\n\t0x%x\n", uhook->fun_name, uhook->addr);	
	printf("status of ksymbal(%s) run:\n\t%s\n", uhook->fun_name, uhook->status);
}

static void build_uhook(struct uhook *uhook, char *name)
{
	strcpy(uhook->fun_name, name);
}

static void print_help(){
	char *help = "uhook [-t|--type [run|query]] [-s|--symbal|-f|--func [func]]"
		     "[-v|--version]";
	printf("Usage:\n \t%s\n", help);
}

static void get_kernel_version(char *version)
{
	FILE *file;
	char *name, *tmp;
	const char *prefix = "Linux version ";

	file = fopen("/proc/version", "r");
	version[0] = '\0';
	tmp = fgets(version, VERSION_LEN, file);
	fclose(file);

	name = strstr(version, prefix);
	name += strlen(prefix);
	tmp = strchr(name, ' ');
	if (tmp)
		*tmp = '\0';
}

static void parse_version(struct uhook *uhook)
{
	printf("kernel version:\n\t%s\n", uhook->version.kernel_version);
	printf("module version:\n\t%s\n", uhook->version.module_version);
	printf("app version:\n\t%s\n", uhook->version.app_version);
}
static void cmd_get_version(struct uhook *uhook, int fd)
{
	strcpy(uhook->version.app_version, APP_VERSION);
	get_kernel_version(uhook->version.kernel_version);
	ioctl(fd, UHOOKCMD_GET_VER, uhook);
	parse_version(uhook);
}

static void cmd_process_type(struct uhook *uhook, char *type)
{
	switch(parse_cmd(type)) {
	case UHOOKCMD_QUERY_FUNC:
		uhook->cmd = UHOOKCMD_QUERY_FUNC;
		break;
	case UHOOKCMD_QUERY_VAL:
		uhook->cmd = UHOOKCMD_QUERY_VAL;
		break;
	case UHOOKCMD_RUN:
		uhook->cmd = UHOOKCMD_RUN;
		break;
	default:
		printf("Unknown type. Try uhook --help|-h to see help\n");
		exit(-1);
	}
}
/*
 * uhook -t run -f uhook_test arg1 arg2 arg3
 * 1      2  3  4    5         6    7    8
 * */

static void cmd_process_arg(struct uhook *uhook, int argct, char **argvt, int optint)
{
	int num_arg = argct - optint;
	int tmp;
	int *arg_ptr = (int *)&uhook->argv;
	
	if (num_arg > 5){	
		printf("Error: num of arg is larger than 5\n");
		exit(-1);
	}
	uhook->argc = num_arg;
	
	for (tmp = 0; tmp < num_arg; tmp++, arg_ptr++) {
		int arg = atoi(argvt[optint++]);
		memcpy(arg_ptr, &arg, sizeof(*arg_ptr));
	}
}
static void cmd_do_func(struct uhook *uhook, char *func, int uhook_fd)
{
	switch(uhook->cmd) {

	case UHOOKCMD_QUERY_FUNC:
		build_uhook(uhook, func);
		ioctl(uhook_fd, UHOOKCMD_QUERY_FUNC, uhook);
		parse_result(uhook, "query");
		break;

	case UHOOKCMD_RUN:
		build_uhook(uhook, func);
		ioctl(uhook_fd, UHOOKCMD_RUN, uhook);
		parse_result(uhook, "run");
		break;

	case UHOOKCMD_QUERY_VAL:
		build_uhook(uhook, func);
		ioctl(uhook_fd, UHOOKCMD_QUERY_VAL, uhook);
		parse_result(uhook, "query val");
		break;
		
	default:
		printf("Unkown cmd\n");
	}
}
static void bad(char *opt)
{
	printf("Bad option:\n\t%s\n", opt);
	printf("Type:\n \tuuhook -h|--help get more help\n");
}
/*
 * call type:
 * uhook    --type   	run 		func
 * argv[0] argv[1]	argv[2]		argv[3]*/
int main(int argc, char **argv)
{

	int uhook_fd = open(DEV_NAME, O_RDWR);
	struct uhook	uhook;
	int next_opt = -1;
	memset(&uhook, 0, sizeof(struct uhook));

	char *uhook_opt = "hvt:s:f:";
	char *t_opt = NULL;
	char *f_opt = NULL;
	const struct option options[] = {
		{ "help", 0, NULL, 'h' },	
		{ "version", 0, NULL, 'v' },	
		{ "type", 1, NULL, 't' },	
		{ "symbal", 1, NULL, 's' },	
		{ "func", 1, NULL, 'f' },	
		{},	
	};
	while((next_opt = getopt_long(argc, argv, uhook_opt, options, NULL)) != -1) {

		switch(next_opt) {

		case '?':
			bad(argv[optind - 1]);
			break;
		case 'v':
			cmd_get_version(&uhook, uhook_fd);
			break;
		case 't':
			t_opt = optarg;
			cmd_process_type(&uhook, t_opt);
			break;
		case 's':
		case 'f':
			f_opt = optarg;
			cmd_process_arg(&uhook, argc, argv, optind);
			cmd_do_func(&uhook, f_opt, uhook_fd);
			break;
		case 'h':
		default:
			print_help();
			break;
		}
	}
	close(uhook_fd);
	return 0;
}

