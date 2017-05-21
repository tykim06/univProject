#ifndef __CHAT_CMD__
#define __CHAT_CMD__

typedef void(*chat_cmd_void_fn_type)(void);

typedef struct sys_cmd_s {
	const char *cmd;
	chat_cmd_void_fn_type execute;
}sys_cmd_t;

typedef struct user_cmd_s {
	const char *cmd;
	chat_cmd_void_fn_type execute;
	const char *desc;
}user_cmd_t;

void chat_cmd_init(const sys_cmd_t *sys_cmd, const user_cmd_t *user_cmd);
void parse_sys_cmd(char *buf);
void parse_user_cmd(char *buf);

#endif