#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

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

static int cmd_si(char *args){
	char *arg = strtok(NULL, " ");
	int n;
	if(arg != NULL){
		if(sscanf(arg,"%d", &n) != 1) {
			printf("Input Error!Please input si with a integer.\n");
			return 0;
		}
	}
	else{
		n = 1;
	}
	cpu_exec(n);
	return 0;
}

static int cmd_info(char *args){
	char *arg = strtok(NULL, " ");
	if(arg == NULL) {
		printf("Input Error!");
		return 0;
	}
	if(strcmp("r",arg) == 0)
	{
		char *reg[] = {"EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI"};
		for(int i = R_EAX; i <= R_EDI;i++){
			printf("%s : 0x%08x\n", reg[i], cpu.gpr[i]._32);
		}
	}	
	else if(strcmp("w", arg) == 0)
	{

	}
	else{
		printf("Input Error ! Please input \"info\" with \"r\" or \"w\"\n");
	}
	return 0;
}

static int cmd_p(char *args){
	bool success = false;
	uint32_t value = expr(args,&success);
	if(success == false) {
		printf("Input Error!");
		cmd_c(args);
	}
	else printf("%d\n",value);

	return 0;
}

static int cmd_x(char *args){
	char *cmd = strtok(NULL, " ");
    int n;
	uint32_t addr;
	bool success = false;
	if(cmd != NULL){
		if(sscanf(cmd,"%d", &n) != 1){		//首先保证n是整数
			printf("Input Error!\n");
			return 0;
		}
	}
	else{
		printf("Input Error!\n");
		return 0;
	}
	
	char *arg = cmd + strlen(cmd) + 1;
	/*if(arg != NULL){
		if(sscanf(arg,"0x%x", &addr) != 1){
			printf("Input Error!\n");
			return 0;
		}
	}
	else{
		printf("Input Error!\n");
		return 0;
	}*/

	addr = expr(arg,&success);
	if(success == false) {
		printf("Input Error!");
		cmd_c(arg);
	}
	for(int i = 0; i < n ;i ++){
		printf("0x%07x : 0x%08x",addr + 4 * i,paddr_read(addr + 4 * i, 4));
		printf("\n");
	}
	return 0;
}

static int cmd_w(char *args){
	return 0;
}

static int cmd_d(char *args){
	return 0;
}

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Execute instructions step over [N]", cmd_si },
  { "info", "Print the status of program", cmd_info },
  { "p", "Evaluate expression", cmd_p },
  { "x", "Scan the ram", cmd_x},
  { "w","Setting watchpoint", cmd_w},
  { "d","Delete the watchpoint which seril number is [N]", cmd_d},
  /* TODO: Add more commands */
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
