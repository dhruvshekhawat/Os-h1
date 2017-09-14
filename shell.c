#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<stdint.h>
#include<sys/wait.h>

#define HIST_BUFF_SIZE 10
char* read_line();
char** get_tokens(char*);
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

void initialize_hist_elem(char* command){
	if(history[top] != NULL){
		free(history[top]);
		history[top] = malloc(sizeof(char)*strlen(command));
  }
	else
		history[top] = malloc(sizeof(char)*strlen(command)); 
	
  strcpy(history[top], command);
}

int main(){
  initialize_hist_buff();
	prompt();
}

void prompt(){
	int status = 1;
  while(status){
    printf("> ");
    char* command = read_line();
		top = (top + 1) % HIST_BUFF_SIZE;
    initialize_hist_elem(command);
    //check if circular array is full
		if(top == head)
			head = (head + 1) % HIST_BUFF_SIZE;
		
    //check if circular array is empty
    if(head == -1)
      head = 0;
    
    char** tokens = get_tokens(command);
    status = run_command(tokens);
    free(command);
    free(tokens);
  }
	free(history);
}

char* read_line(){
  char* line;
  ssize_t bufsize = 0;
  getline(&line, &bufsize, stdin);
  return line;
}

char** get_tokens(char* command){
  char** tokens = malloc(64 * sizeof(char*)); //change from 64 size
  char* token = strtok(command, " \t\r\n\a");
  int index = 0;
  while(token != NULL){
    tokens[index++] = token;
    token = strtok(NULL, " \t\r\n\a");
  }

  tokens[index] = NULL;
  return tokens;
}

int run_command(char** params){
  if(strcmp(*params, "cd") == 0)
    return execute_cd(params);
  else if(strcmp(*params, "history") == 0)
    return execute_history(params);
  else if(strcmp(*params, "exit") == 0)
    return execute_exit(params);
  else
    return run_exec(params);  
}


int run_exec(char** params){
  pid_t pid = fork();
  if(pid == 0){
    if(execvp(*params, params) == -1){  //change call
      printf("Invalid command\n");
      exit(1);
    }
  }
  else if(pid > 0){
    int returnCode;
    waitpid(pid, &returnCode, 0);
    if(returnCode == 1)
      printf("Child process terminated with error");
   
  }
  else
    printf("Error in Forking");
  
	return 1;
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
	int index = 0;
	for(int i = head; ; i = (i + 1) % HIST_BUFF_SIZE){
		printf("%d %s", index++, history[i]);
		if(i == top)
			break;
	}				
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

void display_history_offset(char** params){
	int offset = atoi(params[1]);
  int start = top - offset + 1;		
	for(int i = (start % HIST_BUFF_SIZE + HIST_BUFF_SIZE) % HIST_BUFF_SIZE; ; i = (i + 1) % HIST_BUFF_SIZE){
		printf("%s", history[i]);				
		if(i == top)
			break;
	}				
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
