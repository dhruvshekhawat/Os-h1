char *read_line(void);
char **get_tokens(char *a, char *b);
void prompt(void);
int run_exec(char **a);
int execute_history(char **a);
int execute_cd(char **a);
int execute_exit(char **a);
int numbers_only(char *a);
