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
#include "echobuiltin.h"
#include "cdbuiltin.h"
#include "pwdbuiltin.h"
#include "execute.h"
#include "pinfobuiltin.h"
#include "remindme.h"
#include "lsbuiltin.h"
#include "clockbuiltin.h"
#define MAX_SIZE 105
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

int background_processes;

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

void handler(int sig)
{
	fflush(stdout);
	pid_t pid = wait(NULL);
	if(pid != -1){
		printf("Process with pid %d exited normally.\n", pid);
		--background_processes;
	}
	return;
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
			}
			else if(input_string[cnt - 2] == '&'){
				input_string[cnt - 2] = '\0';	
				background = 1;
				//background_processes++;
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
			if(background_processes > 0)
			{
				signal(SIGCHLD, handler);
			}
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