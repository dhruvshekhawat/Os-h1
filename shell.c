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
int main(){
	prompt();
}

void prompt(){
	history = malloc(HIST_BUFF_SIZE * sizeof(char*));
	for(int i = 0; i < HIST_BUFF_SIZE; i++)
		history[i] = NULL;
	int status = 1;
  while(status){
    printf("> ");
    char* command = read_line();
		top = (top + 1) % HIST_BUFF_SIZE;
		if(history[top] != NULL){
			free(history[top]);
			history[top] = malloc(sizeof(char)*strlen(command));
		}
		else{
			history[top] = malloc(sizeof(char)*strlen(command)); //being call twice?
		}
		strcpy(history[top], command);
		if(top == head){
			head = (head + 1) % HIST_BUFF_SIZE;
		}
    if(head == -1){
      head = 0;
    }
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

    if(execvp(*params,params) == -1){  //change call
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
		int index = 0;
		for(int i = head; ; i = (i + 1) % HIST_BUFF_SIZE){
			printf("%d %s", index++, history[i]);
			if(i == top)
				break;
		}				
	}
	
	else if(strcmp(params[1], "-c") == 0){
		for(int i = 0; i < HIST_BUFF_SIZE; i++){
			if(history[i] != NULL){
			free(history[i]);
			history[i] = NULL;
			head = -1;
			top = -1;	
		}		
		}
	}
	else{
		int offset = atoi(params[1]);
		int start = top - offset + 1;		
		for(int i = (start % HIST_BUFF_SIZE + HIST_BUFF_SIZE) % HIST_BUFF_SIZE; ; i = (i + 1) % HIST_BUFF_SIZE){
			printf("%s", history[i]);				
			if(i == top)
				break;
		}				
	}
	return 1;
}

int execute_exit(char** params){
	return 0;
}
