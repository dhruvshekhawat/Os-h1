#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<stdint.h>
#include<sys/wait.h>

char* read_line();
char** get_tokens(char*);
void prompt(void);
int run_command(char**);
int run_exec(char**);
int execute_history(char**);
int execute_cd(char**);
int execute_exit(char**);
char** history;
int top = 0;
int main(){
	prompt();
}

void prompt(){
	history = malloc(10 * sizeof(char*));
	int status = 1;
  while(status){
    printf("> ");
    char* command = read_line();
		history[top] = malloc(sizeof(char)*strlen(command)); //being call twice?
		strcpy(history[top], command);
		top = (top + 1) % 10;
    char** tokens = get_tokens(command);
    status = run_command(tokens);
    free(command);
    free(tokens);
  }
}

char* read_line(){
  char* line;
  ssize_t bufsize = 0;
  getline(&line, &bufsize, stdin);
  return line;
}

char** get_tokens(char* command){
  char** tokens = malloc(64 * sizeof(char*));
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
    // child process

    if(execvp(*params,params) == -1){
      printf("Invalid command\n");
      exit(1);
    }
  }
  else if(pid > 0){
    // parent process
    int returnCode;
    waitpid(pid, &returnCode, 0);
    if(returnCode == 1){
      printf("Child process terminated with error");
    }
  }
  else{
    printf("Error in Forking");
  }

	return 1;
}

int execute_cd(char** params){
  if(params[1] == NULL){
    printf("Expected argument to cd. None given.");
  }
  else{
    if(chdir(params[1]) != 0){
      printf("chdir did not work correctly");
    }
  }
	return 1;
}

int execute_history(char** params){
	
 if(params[1] == NULL){
		for(int i = 0; i < top; i++){
			printf("%d %s", i, history[i]);
		}				
	}
	
	else if(strcmp(params[1], "-c") == 0){
		for(int i = 0; history[i] != NULL; i++){
			free(history[i]);	
			printf("DOne");	
		}		
	}
	else{
		for(int i = (int)(uintptr_t)params[1]; i < top; i++){
			printf("%s", history[i]);				
		}				
	}
	return 1;
}

int execute_exit(char** params){
	return 0;
}
