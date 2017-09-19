// make README, push to github, test with tester.py, go through piazza again
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<stdint.h>
#include<sys/wait.h>
#include<errno.h>
#include<ctype.h>
#include "shell.h"

#define HIST_BUFF_SIZE 100
#define MAX_ARGS 50
#define INFINITE_LOOP_THRESHOLD 200

char **history;
char **last_snapshot;
int top = -1;
int head = -1;
int loop_number;

void initialize_hist_buff(void)
{
	int i;

	history = malloc(HIST_BUFF_SIZE * sizeof(char *));
	if (!history)
		printf("error: malloc failed line 30\n");
	for (i = 0; i < HIST_BUFF_SIZE; i++)
		history[i] = NULL;
}

void populate_hist_elem(char *command)
{
	top = (top + 1) % HIST_BUFF_SIZE;
	if (history[top] != NULL)
		free(history[top]);
	history[top] = malloc(sizeof(char)*(1 + strlen(command)));
	if (!history[top])
		printf("error: malloc failed line 41\n");
	if (strcpy(history[top], command) == NULL)
		printf("error: strcpy failed on line 44\n");
	if (top == head)
		head = (head + 1) % HIST_BUFF_SIZE;
	if (head == -1)
		head = 0;
}

int main(void)
{
	initialize_hist_buff();
	prompt();
}

int run_exec(char **tokens)
{
	pid_t pid;
	int read_end = 0;
	int pipe_ends[2];

	if (loop_number > INFINITE_LOOP_THRESHOLD) {
		printf("error: stuck in an infinite loop of history offsets\n");
		return 1;
	}
	while (*tokens != NULL) {
		char **pipe_command = get_tokens(*tokens, " \n");

		if (strcmp(pipe_command[0], "cd") == 0) {
			execute_cd(pipe_command);
			read_end = 0;
			tokens++;
			free(pipe_command);
		} else if (strcmp(pipe_command[0], "exit") == 0) {
			int return_code = execute_exit(pipe_command);

			free(pipe_command);
			return return_code;
		} else {
			pipe(pipe_ends);
			pid = fork();
			if (pid  == -1) {
				printf("error: fork failed\n");
				exit(1);
			} else if (pid == 0) {
				if (*(tokens + 1) != NULL)
					dup2(pipe_ends[1], 1);
				dup2(read_end, 0);
				close(pipe_ends[0]);
				if (strcmp(pipe_command[0], "history") == 0)
					execute_history(pipe_command);
				else if (strcmp(pipe_command[0], "exit") == 0)
					execute_exit(pipe_command);
				else if (execv(pipe_command[0], pipe_command) < 0)
					printf("error: %s\n", strerror(errno));
				exit(0);
			 } else {
				wait(NULL);
				free(pipe_command);
				close(pipe_ends[1]);
				read_end = pipe_ends[0];
				tokens++;
			}
		 }
	}
	return 1;
}

void prompt(void)
{
	int status = 1;

	loop_number = 0;
	while (status) {
		char *command = read_line();
		char *save_command = malloc(sizeof(char)*(1 + strlen(command)));

		printf("$");
		if (!save_command)
			printf("error: malloc failed line 101\n");
		if (strcpy(save_command, command) == NULL)
			printf("error: strcpy failed on line 105\n");
		char **tokens = get_tokens(command, "|\n");

		last_snapshot = malloc(HIST_BUFF_SIZE * sizeof(char *));
		if (!last_snapshot)
			printf("error: malloc failed line 106\n");
		memcpy(last_snapshot, history, sizeof(char *) * HIST_BUFF_SIZE);
		populate_hist_elem(save_command);
		status = run_exec(tokens);
		free(tokens);
		free(command);
		free(save_command);
		free(last_snapshot);
	}
	free(history);
}

char *read_line(void)
{
	char *line = NULL;
	ssize_t bufsize = 0;

	if (getline(&line, &bufsize, stdin) < 0)
		printf("error: %s\n", strerror(errno));
	return line;
}

char **get_tokens(char *command, char *delimiter)
{
	int index = 0;
	char *token = strtok(command, delimiter);
	char **tokens = malloc(MAX_ARGS * sizeof(char *));

	if (!tokens)
		printf("error: malloc failed line 128\n");
	while (token != NULL) {
		tokens[index++] = token;
		token = strtok(NULL, delimiter);
	}

	tokens[index] = NULL;
	return tokens;
}

int execute_cd(char **params)
{
	if (params[1] == NULL)
		printf("error: Expected arguments to cd\n");
	else if (params[2] != NULL)
		printf("error: Too many args passed to cd\n");
	else{
		printf("file name is %s\n", params[1]);
		if (chdir(params[1]) != 0)
			printf("error: chdir did not work correctly\n");
	}
	return 1;
}

void display_history(void)
{
	int index = 0;
	int i;

	if (history[0] == NULL) {
		printf("%d %s\n", 0, "history");
		return;
	}
	for (i = head; ; i = (i + 1) % HIST_BUFF_SIZE) {
		history[i][strcspn(history[i], "\n")] = '\0';
		printf("%d %s\n", index++, history[i]);
		if (i == top)
			break;
	}
}

void clear_history(void)
{
	int i;

	for (i = 0; i < HIST_BUFF_SIZE; i++) {
		if (history[i] != NULL) {
			free(history[i]);
			history[i] = NULL;
			head = -1;
			top = -1;
		}
	}
}

int invalid_offset(char *offset
{
	if (last_snapshot[HIST_BUFF_SIZE - 1] == NULL && atoi(offset) >= top) {
		printf("error: Offset value too high\n");
		return 1;
	}
	if (!numbers_only(offset) || atoi(offset) < 0 ||
		atoi(offset) > HIST_BUFF_SIZE) {
		printf("error: Invalid Offset\n");
		return 1;
	}
	return 0;
}

void display_history_offset(char **params)
{
	if (history[0] == NULL)
		return;
	if (invalid_offset(params[1]))
		return;
	int offset = atoi(params[1]);
	int start = head + offset;
	int position = start % HIST_BUFF_SIZE;
	char *tok_cmd = malloc(sizeof(char)*strlen(last_snapshot[position]));

	if (!tok_cmd)
		printf("error: malloc failed line 203\n");
	if (strcpy(tok_cmd, last_snapshot[position]) == NULL)
		printf("error: strcpy failed on line 207\n");
	char **tokens = get_tokens(tok_cmd, "|\n");

	loop_number++;
	run_exec(tokens);
	free(tok_cmd);
	free(tokens);
}

int execute_history(char **params)
{
	if (params[1] == NULL)
		display_history();
	else if (strcmp(params[1], "-c") == 0)
		clear_history();
	else
		display_history_offset(params);
	return 1;
}

int execute_exit(char **params)
{
	if (params[1] != NULL) {
		printf("error: Too many args passed to exit\n");
		return 1;
	}
	return 0;
}

int numbers_only(char *s)
{
	while (*s) {
		if (isdigit(*s++) == 0)
			return 0;
	}
	return 1;
}
