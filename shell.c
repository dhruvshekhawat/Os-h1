#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<stdint.h>
#include<sys/wait.h>

#define HIST_BUFF_SIZE 10
#define MAX_ARGS 50

char* read_line();
char** get_tokens(char*, char*);
void prompt(void);
int run_command(char**);
int run_exec(char**);
int execute_history(char**);
int execute_cd(char**);
int execute_exit(char**);

char** history;
int top = -1;
int head = -1;

void initialize_hist_buff(){
	history = malloc(HIST_BUFF_SIZE * sizeof(char*));
	for(int i = 0; i < HIST_BUFF_SIZE; i++)
		history[i] = NULL;
}

void populate_hist_elem(char* command){
	top = (top + 1) % HIST_BUFF_SIZE;
	if(history[top] != NULL){
		free(history[top]);
		history[top] = malloc(sizeof(char)*strlen(command));
  }
	else
		history[top] = malloc(sizeof(char)*strlen(command)); 
  strcpy(history[top], command);
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
  char*** pipe_commands = malloc(sizeof(char**)*100);
  for(int i = 0; tokens[i] != NULL;i++)
    pipe_commands[i] = get_tokens(tokens[i], " \n");
  int p[2];
  pid_t pid;
  int fd_in = 0;
  while (*pipe_commands != NULL){
    pipe(p);
    if((pid = fork()) == -1)
      exit(EXIT_FAILURE);
    else if (pid == 0){
      dup2(fd_in, 0);
      if(*(pipe_commands + 1) != NULL)
        dup2(p[1], 1);
      close(p[0]);
      execvp((*pipe_commands)[0], *pipe_commands);
      exit(EXIT_FAILURE);
    }
    else{
      wait(NULL);
      close(p[1]);
      fd_in = p[0]; 
      pipe_commands++;
    }
  }
  //free(pipe_commands);
  return 1;
}

void prompt(){
	int status = 1;
  while(status){
    printf("$");
    char* command = read_line();
    char* store_command = malloc(sizeof(char)*strlen(command));
    strcpy(store_command, command);
    char** tokens = get_tokens(command, "|\n");
    status = run_command(tokens);
    populate_hist_elem(store_command);
    free(tokens);
    free(command);
    free(store_command);
  }
	free(history);
}

char* read_line(){
  char* line;
  ssize_t bufsize = 0;
  getline(&line, &bufsize, stdin);
  return line;
}

char** get_tokens(char* command, char* delimiter){
  char** tokens = malloc(MAX_ARGS * sizeof(char*)); //change from 64 size
  char* token = strtok(command, delimiter);
  int index = 0;
  while(token != NULL){
    tokens[index++] = token;
    token = strtok(NULL, delimiter);
  }

  tokens[index] = NULL;
  return tokens;
}

int run_command(char** params){
  char* duplicate = malloc(sizeof(char)*strlen(params[0]));
  strcpy(duplicate, params[0]);
  char** builtin = get_tokens(duplicate," \n");
  if(strcmp(*builtin, "cd") == 0)
    return execute_cd(builtin);
  else if(strcmp(*builtin, "history") == 0)
    return execute_history(builtin);
  else if(strcmp(*builtin, "exit") == 0)
    return execute_exit(builtin);
  else
    return run_exec(params);  
}

int execute_cd(char** params){
  if(params[1] == NULL)
    printf("Expected argument to cd. None given.");
  
  else{
    if(chdir(params[1]) != 0)
      printf("chdir did not work correctly");
    
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
  if(history[HIST_BUFF_SIZE - 1] == NULL)
    i = 0;
  else
    i = (head + 1) % HIST_BUFF_SIZE;
	for(; ; i = (i + 1) % HIST_BUFF_SIZE){
		history[i][strcspn(history[i],"\n")] = '\0';
    printf("%d %s\n", index++, history[i]);
		if(i == top)
			break;
	}	
  printf("%d %s\n", index, "history");
}

void clear_history(){
	for(int i = 0; i < HIST_BUFF_SIZE; i++){
		if(history[i] != NULL){
		  free(history[i]);
		  history[i] = NULL;
		  head = -1;
		  top = -1;	
	  }		
	}
}

int invalid_offset(int offset, int start){
  if(offset < 0 || offset > HIST_BUFF_SIZE){
    printf("Invalid Offset. Either hist buffer has elements lesser than offset or offset > 10\n");
    return 1;
  }
  return 0;
}

void display_history_offset(char** params){
  if(history[0] == NULL)
    return;
	int offset = atoi(params[1]);
  int start = head + offset;		
  if(invalid_offset(offset, start))
    return;

  int position = (start % HIST_BUFF_SIZE + HIST_BUFF_SIZE) % HIST_BUFF_SIZE;
  char* tokenize_command = malloc(sizeof(char)*strlen(history[position]));
  strcpy(tokenize_command, history[position]);
	char** tokens = get_tokens(tokenize_command, "|\n");
  run_command(tokens);
  free(tokenize_command);
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
