// handle infinte loops, checkpatch, make README, exit and cd with args, push to github, test with tester.py, go through piazza again
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<stdint.h>
#include<sys/wait.h>
#include<errno.h>
#include<ctype.h>

#define HIST_BUFF_SIZE 5
#define MAX_ARGS 50

char* read_line();
char** get_tokens(char*, char*);
void prompt(void);
int run_exec(char**);
int execute_history(char**);
int execute_cd(char**);
int execute_exit(char**);
int numbers_only(char*);

char** history;
char** last_snapshot;
int top = -1;
int head = -1;
void initialize_hist_buff(){
        int i;
        history = malloc(HIST_BUFF_SIZE * sizeof(char*));
        if(!history)
                printf("error: malloc failed line 30");
	for(i = 0; i < HIST_BUFF_SIZE; i++)
		history[i] = NULL;
}

void populate_hist_elem(char* command){
	top = (top + 1) % HIST_BUFF_SIZE;
	if(history[top] != NULL)
		free(history[top]);
	history[top] = malloc(sizeof(char)*(1 + strlen(command)));
        if(!history[top])
                printf("error: malloc failed line 41");
        if(strcpy(history[top], command) == NULL)
                printf("error: strcpy failed on line 44");
	if(top == head)
		head = (head + 1) % HIST_BUFF_SIZE;
        if(head == -1)
                head = 0;
}

int main(){
        initialize_hist_buff();
	prompt();
}

int run_exec(char** tokens){
        int read_end = 0;
        int pipe[2];
        pid_t pid;
        while (*tokens != NULL){
                char** pipe_command = get_tokens(*tokens, " \n");
                if(strcmp(pipe_command[0], "cd") == 0){
                        execute_cd(pipe_command);
                        read_end = 0;
                        tokens++;
                        free(pipe_command);
                }
                else if(strcmp(pipe_command[0], "exit") == 0){
                        execute_exit(pipe_command);
                        free(pipe_command);
                        return 0;
                }
                else{
                        pipe(pipe);
                        if((pid = fork()) == -1){
                                printf("error: fork failed\n");
                                exit(1);
                        }
                        else if (pid == 0){
                                if(*(tokens + 1) != NULL)
                                        dup2(pipe[1], 1);
                                dup2(read_end, 0);
                                close(pipe[0]);
                                if(strcmp(pipe_command[0],"history") == 0)
                                        execute_history(pipe_command);
                                else if(strcmp(pipe_command[0], "exit") == 0)
                                        execute_exit(pipe_command);
                                else if(execv(pipe_command[0], pipe_command) < 0)
                                        printf("error: %s\n", strerror(errno));
                                exit(0);
                         }   
                         else{
                                wait(NULL);
                                free(pipe_command);
                                close(pipe[1]);
                                read_end = pipe[0]; 
                                tokens++;
                        }
                 }
        }
        return 1;
}

void prompt(){
	int status = 1;
        while(status){
                printf("$");
                char* command = read_line();
                char* store_command = malloc(sizeof(char)*(1 + strlen(command)));
                if(!store_command)
                        printf("error: malloc failed line 101");
                if(strcpy(store_command, command) == NULL)
                        printf("error: strcpy failed on line 105");
                char** tokens = get_tokens(command, "|\n");
                last_snapshot = malloc(HIST_BUFF_SIZE * sizeof(char*));
                if(!last_snapshot)
                        printf("error: malloc failed line 106");
                memcpy(last_snapshot, history, sizeof(char*)*HIST_BUFF_SIZE);
                populate_hist_elem(store_command);
                status = run_exec(tokens);
                free(tokens);
                free(command);
                free(store_command);
                free(last_snapshot);
        }
	free(history);
}

char* read_line(){
        char* line = NULL;
        ssize_t bufsize = 0;
        if(getline(&line, &bufsize, stdin) < 0)
                printf("error: %s\n", strerror(errno));
        return line;
}

char** get_tokens(char* command, char* delimiter){
        char** tokens = malloc(MAX_ARGS * sizeof(char*)); //change from 64 size
        if(!tokens)
                printf("error: malloc failed line 128");
        char* token = strtok(command, delimiter);
        int index = 0;
        while(token != NULL){
                tokens[index++] = token;
                token = strtok(NULL, delimiter);
        }

        tokens[index] = NULL;
        return tokens;
}

int execute_cd(char** params){
        printf("reaching here for cd");
        if(params[1] == NULL)
                printf("error: Expected arguments to cd\n");
        else{
                printf("file name is %s\n", params[1]);
                if(chdir(params[1]) != 0)
                        printf("error: chdir did not work correctly");
        }
	return 1;
}

void display_history(){
        if(history[0] == NULL){
                printf("%d %s\n", 0, "history");
                return;
        }
	int index = 0;
        int i;
	for(i = head; ; i = (i + 1) % HIST_BUFF_SIZE){
	        history[i][strcspn(history[i],"\n")] = '\0';
                printf("%d %s\n", index++, history[i]);
		if(i == top)
			break;
	}	
}

void clear_history(){
        int i;
	for(i = 0; i < HIST_BUFF_SIZE; i++){
		if(history[i] != NULL){
		        free(history[i]);
		        history[i] = NULL;
		        head = -1;
		        top = -1;	
                }		
	}
}

int invalid_offset(char* offset){
        if(last_snapshot[HIST_BUFF_SIZE - 1] == NULL && atoi(offset) >= top){
               printf("error: Offset value too high\n"); 
               return 1;
        }
        if(!numbers_only(offset) || atoi(offset) < 0 || atoi(offset) > HIST_BUFF_SIZE){
                printf("error: Invalid Offset\n");
                return 1;
        }
        return 0;
}

void display_history_offset(char** params){
        if(history[0] == NULL)
                return;
        if(invalid_offset(params[1]))
                return;
        int offset = atoi(params[1]);
        int start = head + offset;	
        int position = (start % HIST_BUFF_SIZE + HIST_BUFF_SIZE) % HIST_BUFF_SIZE;
        char* tokenize_command = malloc(sizeof(char)*strlen(last_snapshot[position]));
        if(!tokenize_command)
                printf("error: malloc failed line 203");
        if(strcpy(tokenize_command, last_snapshot[position]) == NULL)
                printf("error: strcpy failed on line 207");
	char** tokens = get_tokens(tokenize_command, "|\n");
        run_exec(tokens);
        free(tokenize_command);
        free(tokens);
}

int execute_history(char** params){
        if(params[1] == NULL)   
                display_history();
	else if(strcmp(params[1], "-c") == 0)
	        clear_history();
        else
                display_history_offset(params);
	return 1;
}

int execute_exit(char** params){
	return 0;
}

int numbers_only(char *s){
        while(*s){
                if (isdigit(*s++) == 0) return 0;
        }
        return 1;
}
