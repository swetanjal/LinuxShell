#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pwd.h>
#include <time.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "echobuiltin.h"
#include "cdbuiltin.h"
#include "pwdbuiltin.h"
#include "pinfobuiltin.h"
#include "remindme.h"
#include "lsbuiltin.h"
#include "clockbuiltin.h"
#include "jobcontrol.h"
#define MAX_SIZE 105
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
typedef struct command
{
	char name[MAX_SIZE];
	int pid;
	struct command * prev;
}command;

int proc_cnt;
command * background_last = NULL;
command * foreground_last = NULL;
int background_processes;

void quit()
{
	int pid = getpid();
	kill(pid, 9);
}

command * fgBuiltin(char ** arguments, int K, char * home_dir, command * tail)
{
	if(K != 2)
	{
		perror("Invalid Usage.");
		return background_last;
	}
	int proc_no = atoi(arguments[1]);
	command * ptr = background_last;
	int c = 0;
	while(ptr != NULL){
		c++;
		ptr = ptr -> prev;
	}
	ptr = background_last;
	int cnt = 0;
	while(ptr != NULL && (c - cnt) != proc_no)
	{
		cnt++;
		ptr = ptr -> prev;
	}
	if(ptr != NULL){
		int val = kill(ptr->pid, SIGCONT);
		if(val == -1)
		{
			perror("Kill error.");
			return tail;
		}
		siginfo_t fgStatus;
		int PID = ptr -> pid;
		foreground_last = ptr;
		background_last = removeFromBackground(ptr -> pid, background_last);
		//wait(NULL);

		waitid(P_PID, PID, &fgStatus, (WUNTRACED | WNOWAIT));
		free(ptr);
	}
	else{
		free(ptr);
		perror("Invalid process number");
	}
	return background_last;
}
void sigintHandler(int sig)
{
	if(foreground_last != NULL)
		kill(foreground_last -> pid, SIGINT);
}

void sigtstpHandler(int sig)
{
	if(foreground_last != NULL)
		kill(foreground_last -> pid, SIGTSTP);
}

void childEndHandler(int sig)
{
	fflush(stdout);
	pid_t pid;
	int status;
	while((pid = waitpid(-1, &status, (WNOHANG | WUNTRACED))) > 0)
	{
		if(foreground_last != NULL && foreground_last->pid == pid)
		{
			if(WIFEXITED(status)){

			}
			else if(WIFSTOPPED(status)) {
				printf("%s with pid %d stopped\n", foreground_last -> name, pid);
				command G;
				strcpy(G.name, foreground_last->name);
				G.pid = pid;
				background_last = addToBackground(G, background_last);
			}
			else if(WIFSIGNALED(status)){
				printf("%s with pid %d terminated\n", foreground_last -> name, pid);
			}
			//free(foreground_last);
			foreground_last = NULL;
		}
		else if(background_last != NULL){
			/*Check for pid in background linked list.*/
			command * ptr = background_last;
			int found = 0;
			while(ptr != NULL)
			{
				if(ptr -> pid == pid)
				{
					found = 1;
					break;
				}
				ptr = ptr -> prev;
			}
			if(found){
				if(WIFEXITED(status))
				{
					printf("Process with pid %d exited normally.\n", pid);
					background_last = removeFromBackground(pid, background_last);
				}
				else if(WIFSIGNALED(status))
				{
					printf("Process with pid %d terminated normally.\n", pid);
					background_last = removeFromBackground(pid, background_last);	
				}
			}
		}
	}	
}

int execute(char **tokens, int cnt, char *home_dir, int bg)
{
	int restore_input = dup(0);
	int restore_output = dup(1);
	int pos = cnt;
	if(bg == 0)
	{
		/*This is a foreground process*/
		
		char input[MAX_SIZE];
		char output[MAX_SIZE][MAX_SIZE];
		int append[MAX_SIZE];
		//int pos = cnt;
		for(int i = 0; i < cnt; i++)
		{
			if(strcmp(">", tokens[i]) == 0 || strcmp(">>", tokens[i]) == 0 || strcmp("<", tokens[i]) == 0){
				pos = i;
				break;
			}
		}
		int output_counter = 0;
		for(int i = 0; i < MAX_SIZE; i++){
			input[i] = '\0';
			output[i][i] = '\0';
			for(int j = i + 1; j < MAX_SIZE; j++)
				output[i][j] = '\0';
		}
		for(int i = 0; i < cnt; i++){
			if(strcmp(tokens[i], ">") == 0)
			{
				append[output_counter] = 0;
				tokens[i] = NULL;
				strcpy(output[output_counter++], tokens[i + 1]);
			}
			else if(strcmp(tokens[i], ">>") == 0)
			{
				append[output_counter] = 1;
				tokens[i] = NULL;
				strcpy(output[output_counter++], tokens[i + 1]);
			}
			else if(strcmp(tokens[i], "<") == 0)
			{
				tokens[i] = NULL;
				strcpy(input, tokens[i + 1]);
			}
		}
		if(input[0] != '\0')
		{
			int fd = open(input, O_RDONLY);
			if(fd < 0)
			{
				perror("Invalid File");
			}
			else
			{
				dup2(fd, 0);
			}
			close(fd);
			//From now on we can use 0 as fd.
		}
		for(int i = 0; i < output_counter; i++)
		{
			int fd;
			if(append[i] == 0)
				fd = open(output[i], O_WRONLY | O_TRUNC | O_CREAT ,0644);
			else
				fd = open(output[i], O_WRONLY | O_APPEND | O_CREAT ,0644);
			if(fd < 0)
			{
				perror("Error");
				continue;
			}
			dup2(fd, STDOUT_FILENO);
			close(fd);
		}
		if(strcmp(tokens[0], "cd") == 0){
			cd_builtin(tokens, pos, home_dir);
		}
		else if(strcmp(tokens[0], "pwd") == 0){
			pwd_builtin(tokens, pos, home_dir);
		}
		else if(strcmp(tokens[0], "echo") == 0){
			echo_builtin(tokens, pos, home_dir);
		}
		else if(strcmp(tokens[0], "ls") == 0){
			ls_builtin(tokens, pos, home_dir);
		}
		else if(strcmp(tokens[0], "pinfo") == 0){
			pinfo_builtin(tokens, pos, home_dir);
		}
		else if(strcmp(tokens[0], "remindme") == 0){
			remindme(tokens, pos, home_dir);
		}
		else if(strcmp(tokens[0], "clock") == 0)
		{
			clock_builtin(tokens, pos, home_dir);
		}
		else if(strcmp(tokens[0], "setenv") == 0)
		{
			setenvBuiltin(tokens, pos, home_dir);
		}
		else if(strcmp(tokens[0], "unsetenv") == 0)
		{
			unsetenvBuiltin(tokens, pos, home_dir);
		}
		else if(strcmp(tokens[0], "getenv") == 0)
		{
			getenvBuiltin(tokens, pos, home_dir);
		}
		else if(strcmp(tokens[0], "jobs") == 0)
		{
			print_jobs(tokens, pos, home_dir, background_last);	
		}
		else if(strcmp(tokens[0], "kjob") == 0)
		{
			kjob(tokens, pos, home_dir, background_last);
		}
		else if(strcmp(tokens[0], "overkill") == 0)
		{
			overkill(tokens, pos, home_dir, background_last);
		}
		else if(strcmp(tokens[0], "fg") == 0)
		{
			background_last = fgBuiltin(tokens, pos, home_dir, background_last);
		}
		else if(strcmp(tokens[0], "quit") == 0){
			quit();
		}
		else{
			int pid = fork();
			command * f = (command *)malloc(sizeof(command));
			strcpy(f -> name, tokens[0]);
			f -> pid = pid;
			foreground_last = f;
			
			if(pid == 0){
				execvp(tokens[0], &tokens[0]);
				exit(1);
			}
			else{
				siginfo_t fgStatus;
				waitid(P_PID, pid, &fgStatus, (WUNTRACED | WNOWAIT));
				//wait(NULL);
			}
		}
		dup2(restore_input, 0);
		dup2(restore_output, 1);
		close(restore_input);
		close(restore_output);
	}
	else
	{
		if(strcmp(tokens[0], "cd") == 0){
			cd_builtin(tokens, pos, home_dir);
		}
		else if(strcmp(tokens[0], "pwd") == 0){
			pwd_builtin(tokens, pos, home_dir);
		}
		else if(strcmp(tokens[0], "echo") == 0){
			echo_builtin(tokens, pos, home_dir);
		}
		else if(strcmp(tokens[0], "ls") == 0){
			ls_builtin(tokens, pos, home_dir);
		}
		else if(strcmp(tokens[0], "pinfo") == 0){
			pinfo_builtin(tokens, pos, home_dir);
		}
		else if(strcmp(tokens[0], "remindme") == 0){
			remindme(tokens, pos, home_dir);
		}
		else if(strcmp(tokens[0], "clock") == 0)
		{
			clock_builtin(tokens, pos, home_dir);
		}
		else if(strcmp(tokens[0], "setenv") == 0)
		{
			setenvBuiltin(tokens, pos, home_dir);
		}
		else if(strcmp(tokens[0], "unsetenv") == 0)
		{
			unsetenvBuiltin(tokens, pos, home_dir);
		}
		else if(strcmp(tokens[0], "getenv") == 0)
		{
			getenvBuiltin(tokens, pos, home_dir);
		}
		else if(strcmp(tokens[0], "jobs") == 0)
		{
			print_jobs(tokens, pos, home_dir, background_last);	
		}
		else if(strcmp(tokens[0], "kjob") == 0)
		{
			kjob(tokens, pos, home_dir, background_last);
		}
		else if(strcmp(tokens[0], "overkill") == 0)
		{
			overkill(tokens, pos, home_dir, background_last);
		}
		else if(strcmp(tokens[0], "fg") == 0)
		{
			background_last = fgBuiltin(tokens, pos, home_dir, background_last);
		}
		else if(strcmp(tokens[0], "quit") == 0){
			quit();
		}
		else{
			int pid = fork();
			if(pid != 0)
				printf("%d\n", pid);
			command proc;
			strcpy(proc.name, tokens[0]);
			proc.pid = pid;
			background_last = addToBackground(proc, background_last);
			proc_cnt++;
			if(pid == 0){
				execvp(tokens[0], &tokens[0]);
				exit(1);
			}
		}
	}
	return 0;
}

char **lsh_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

char **split(char * line)
{
	char ** tokens = malloc(MAX_SIZE * sizeof(char*));
	char * token;
	token = strtok(line, ";\n");
	int count = 0;
	while(token != NULL)
	{
		tokens[count++] = token;
		token = strtok(NULL, ";\n");
	}
	tokens[count++] = NULL;
	return tokens;
}

char **pipe_split(char * line)
{
	char ** tokens = malloc(MAX_SIZE * sizeof(char*));
	char * token;
	token = strtok(line, "|\n");
	int count = 0;
	while(token != NULL)
	{
		tokens[count++] = token;
		token = strtok(NULL, "|\n");
	}
	tokens[count++] = NULL;
	return tokens;	
}

char* input()
{
	char * line;
	size_t n;
	getline(&line, &n, stdin);
	return line;
}

int arg_count(char **tokens)
{
	int cnt = 0;
	while(tokens[cnt++] != NULL);
	cnt = cnt - 1;
	return cnt;
}



char rel[MAX_SIZE];

void abs_to_rel(char *home_dir, char *path)
{
	rel[0] = '~';
	rel[1] = '/';
	for(int i = 2; i <= strlen(path) - strlen(home_dir) + 1; i++)
		rel[i] = path[i - 1 + strlen(home_dir)];
	rel[strlen(path) - strlen(home_dir) + 2] = '\0';
}

void process_pipes(char * str, int len, char *home_dir)
{
	char ** tokens = pipe_split(str);
	int cnt = 0;
	while(tokens[cnt++] != NULL);
	cnt--;	
	int fd = 0;
	int restore_input = dup(0), restore_output = dup(1);
	//printf("%d\n", cnt);
	for(int i = 0; i < cnt; i++)
	{
		int pipefd[2];
		pipe(pipefd);
		char ** token_pipe = lsh_split_line(tokens[i]);
		int c = 0;
		while(token_pipe[c++] != NULL);
		c--;
		int ccc = 0;
		while(token_pipe[ccc] == NULL){
			ccc++;
		}
		int child = fork();
		
		if(child == 0){
			dup2(fd , 0);
			if(tokens[i + 1] != NULL)dup2(pipefd[1],1);
			else
				dup2(restore_output, 1);
			close(pipefd[0]);
			execute(&token_pipe[ccc], c, home_dir, 0);
			exit(1);
		}
		else
		{
			wait(NULL);
			close(pipefd[1]);
			fd = pipefd[0];
		}
		free(token_pipe);
	}

	dup2(restore_input, 0);
	dup2(restore_output, 1);
	close(restore_output);
	close(restore_input);
}

void shell_loop()
{
	char *system_name = (char *)malloc(MAX_SIZE * sizeof(char));
	gethostname(system_name, MAX_SIZE);
	char *username = getenv("USER");
	char *home_dir = (char *)malloc(MAX_SIZE * sizeof(char));
	getcwd(home_dir, MAX_SIZE);
	signal(SIGCHLD, childEndHandler);
	signal(SIGINT, sigintHandler);
	signal(SIGTSTP, sigtstpHandler);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGCHLD, childEndHandler);
	while(1)
	{
		char *curr_dir = (char *)malloc(MAX_SIZE * sizeof(char));
		getcwd(curr_dir, MAX_SIZE);

		abs_to_rel(home_dir, curr_dir);
		printf("<%s@%s:%s> ", username, system_name, rel);
		char *input_str = input();
		char **commands = split(input_str);
		int test_cases = arg_count(commands);
		
		for(int ccc = 0; ccc < test_cases; ccc++){
			
			char *input_string = commands[ccc];
			int background = 0;
			int cnt = 0;
			
			while(input_string[cnt] != '\0')
			{
				cnt++;
			}
			if(input_string[cnt - 1] == '&'){
				input_string[cnt - 1] = '\0';
				background = 1;

				//proc_cnt++;
			}
			else if(input_string[cnt - 2] == '&'){
				input_string[cnt - 2] = '\0';	
				background = 1;
				//proc_cnt++;
			}
			int pipe = 0;
			for(int i = 0; i < cnt; i++)
			{
				if(input_string[i] == '|'){
					pipe = 1;
					break;
				}
			}
			if(pipe)
			{
				process_pipes(input_string, cnt, home_dir);
				continue;
			}
			char **tokens = lsh_split_line(input_string);
			int argc = arg_count(tokens);
			if(background){
				background_processes++;
				execute(tokens, argc, home_dir, background);
			}
			else{
				execute(tokens, argc, home_dir, background);
			}
			signal(SIGCHLD, childEndHandler);
			signal(SIGINT, sigintHandler);
			signal(SIGTSTP, sigtstpHandler);
			signal(SIGTTOU, SIG_IGN);
		}
		free(input_str);
		free(commands);
	}
}

int main()
{
	shell_loop();
	return 0;
}