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
  //check if circular array is full
	if(top == head)
		head = (head + 1) % HIST_BUFF_SIZE;
		
  //check if circular array is empty
  if(head == -1)
      head = 0;
}

int main(){
  initialize_hist_buff();
	prompt();
}

  
void exec_pipes(char** pipe_commands){//change pipe variable names
  int len = 0;
  int status;
  char*** tokens = malloc(sizeof(char**)*100);//free this gracefully
  for(int i = 0; pipe_commands[i]!=NULL;i++){
    tokens[i] = get_tokens(pipe_commands[i], " \n");
  }
  loop_pipe(tokens);
  /*printf("LEEEEEEEEEEEEN %d\n",len);
  int pipes[(len-1)*2];
  for(int i = 0; i< len;i=i+2)
    pipe(pipes + i);
  printf("Passed this");
  for(int i=0, j=-2; i<len;i++, j=j+2){
    printf("In here i is %d, j is %d\n", i, j);
    if(fork() == 0){
      if(i == 0)
        dup2(pipes[1], 1);
      else if(i == len - 1)
          dup2(pipes[j], 0);
      else{
          printf("in elseeeee, val of j is %d\n", j);
          dup2(pipes[j], 0);
          dup2(pipes[j+3], 1);
      }
      for(int a =0; a < (len-1)*2;a++)
        close(pipes[a]);
      execvp(*tokens[i], tokens[i]);
    }
  }
  printf("OUUUUUUT HEEREEEEEEEEEEEEEEEE!");
  for(int b =0; b < (len-1)*2; b++)
    close(pipes[b]);

  for(int c = 0; c < len; c++)
    wait(&status);
  free(tokens);
  tokens = NULL;
  printf("LAST GEEEEEEEEERE");
*/}
void    loop_pipe(char ***cmd) 
{
  int   p[2];
  pid_t pid;
  int   fd_in = 0;

  while (*cmd != NULL)
    {
      pipe(p);
      if ((pid = fork()) == -1)
        {
          exit(EXIT_FAILURE);
        }
      else if (pid == 0)
        { printf("IIIIIIIIN here");
          dup2(fd_in, 0); //change the input according to the old one 
          if (*(cmd + 1) != NULL)
            dup2(p[1], 1);
          close(p[0]);
          execvp((*cmd)[0], *cmd);
          exit(EXIT_FAILURE);
        }
      else
        {
          wait(NULL);
          close(p[1]);
          fd_in = p[0]; //save the input for the next command
          cmd++;
        }
    }
}

void prompt(){
	int status = 1;
  while(status){
    printf("$");
    char* command = read_line();
    char* store_command = malloc(sizeof(char)*strlen(command));
    strcpy(store_command, command);
    if(strchr(command, '|') != NULL){
      populate_hist_elem(store_command);
      char** pipe_commands = get_tokens(command, "|\n");
      exec_pipes(pipe_commands);
      free(pipe_commands);
    }
    else{
      char** tokens = get_tokens(command, " \n");
      if(strcmp(tokens[0], "history") == 0 && tokens[1] != NULL && strcmp(tokens[1], "-c") != 0){
        status = run_command(tokens);
        populate_hist_elem(store_command);
     }
      else{
        populate_hist_elem(store_command);
        status = run_command(tokens);
      }
      free(tokens);
    }
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
		history[i][strcspn(history[i],"\n")] = '\0';
    printf("%d %s\n", index++, history[i]);
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

int invalid_offset(int offset, int start){
  if(offset < 0 || offset > 10){
    printf("Invalid Offset. Either hist buffer has elements lesser than offset or offset > 10\n");
    return 1;
  }
  return 0;
}

void display_history_offset(char** params){
	int offset = atoi(params[1]);
  int start = head + offset;		
  if(invalid_offset(offset, start))
    return;

  int position = (start % HIST_BUFF_SIZE + HIST_BUFF_SIZE) % HIST_BUFF_SIZE;
  char* tokenize_command = malloc(sizeof(char)*strlen(history[position]));
  strcpy(tokenize_command, history[position]);
	char** tokens = get_tokens(tokenize_command, " \n");
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
