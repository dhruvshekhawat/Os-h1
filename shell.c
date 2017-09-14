#include<stdio.h>
#include<unistd.h>
#include<string.h>

char* read_line();
char** get_tokens(char*);
char** history;
int top = 0;
int main(){
	prompt();
}

void prompt(){
	history = malloc(100 * sizeof(char*));
	int status = 1;
  while(status){
    printf("> ");
    char* command = read_line();
		history[top] = malloc(sizeof(char)*strlen(command));
		strcpy(history[top++], command);
		printf("%s", history[0]);
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
    execute_cd(params);
  else if(strcmp(*params, "history") == 0)
    execute_history(params);
  else if(strcmp(*params, "exit") == 0)
    execute_exit(params);
  else
    run_exec(params);  
}


int run_exec(char** params){
  pid_t pid = fork();
  if(pid == 0){
    // child process

    if(execvp(*params,params) == -1){
      printf("Exec failed!");
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
}

int execute_history(char** params){
	if(params[1] == NULL){
		printf("IN hereerer");
		for(int i = 0; i < top; i++){
			printf("%s\n", history[i]);
		}				
	}
	else{
		for(int i = params[1]; i < top; i++){
			printf("%s\n", history[i]);				
		}				
	}
}

int execute_exit(char** params){
	return 0;
}
