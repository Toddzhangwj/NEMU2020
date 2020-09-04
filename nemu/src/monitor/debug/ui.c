#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si", "just only", cmd_si },
	{ "info", "information", cmd_info},
	{ "x", "memory", cmd_x},
	{ "p", "expression", cmd_p},
	{ "w", "watchpoint", cmd_w},
	{ "d", "delete watchpoint", cmd_d},
	 /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

static int cmd_si(char *args) {
	/**/
	if(args == NULL)
	{
		cpu_exec(1);
	}
	else{
		int num;
		sscanf(args,"%d",&num);
		cpu_exec(num);
	}
	return 0;
}

static int cmd_info(char *args) {

	if(strcmp(args,"r") == 0) {
		int i;
		for( i=0;i<8;i++) {
			printf("the %s reg is 0x%08x\n",regsl[i],reg_l(i));	
		}
	}
	else if(strcmp(args,"w")==0) {
		info_wp();
	}
	return 0;
}

static int cmd_x(char *args) {
	int num,addr;
	if(args==NULL) assert(0);
	sscanf(args,"%d %x",&num,&addr);
	int i;
	for(i=0;i<num;i++) {
		printf("0x%08x ",swaddr_read(addr,4));
		addr+=4;
	}
	printf("\n");
	return 0;
}

static int cmd_p(char *args) {
	bool op;
	int ans = expr(args,&op);
	if(op)
		printf("0x%08x is %d\n",ans,ans);
	else assert (0);
	return 0;
}

static int cmd_w(char *args) {
	WP *wp = new_wp();
	bool suc;
	printf("watchpoint %d: %s\n",wp->NO,args);
	wp->val = expr(args,&suc);
	if(!suc) assert(0);
	strcpy(wp->str,args);
	printf("the val is %d\n",wp->val);		
	return 0;	
}

static int cmd_d(char *args) {
	int num;
	sscanf(args,"%d",&num);
	delete_wp(num);	
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
