#include "chat_cmd.h"
#include <string.h>
#include <stdio.h>

static const sys_cmd_t *p_sys_cmd;
static const user_cmd_t *p_user_cmd;

void chat_cmd_init(const sys_cmd_t *sys_cmd, const user_cmd_t *user_cmd) {
	p_sys_cmd = sys_cmd;
	p_user_cmd = user_cmd;
}

void parse_sys_cmd(char *buf) {
	unsigned int i=0;
	while((p_sys_cmd+i)->execute) {
		if(strncmp((p_sys_cmd+i)->cmd, buf, strlen((p_sys_cmd+i)->cmd)) == 0) {
			(p_sys_cmd+i)->execute(buf);
			return;
		}
		i++;
	}
}

void parse_user_cmd(char *buf) {
	unsigned int i=0;
	while((p_user_cmd+i)->execute) {
		if(strcmp((p_user_cmd+i)->cmd, buf) == 0) {
			(p_user_cmd+i)->execute();
			return;
		}
		i++;
	}
	printf("wrong command, more info enter :help\n");
}