#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <stdio.h>
#include "echobuiltin.h"
#include "cdbuiltin.h"
#include "pwdbuiltin.h"
#include "execute.h"
#include "pinfobuiltin.h"
#include "remindme.h"
#include "lsbuiltin.h"
#include "clockbuiltin.h"
#define MAX_SIZE 1005

int execute(char **tokens, int cnt, char *home_dir, int bg)
{
	int argc = cnt;
	int restore_input = dup(0);
	int restore_output = dup(1);
	if(bg == 0)
	{
		/*This is a foreground process*/
		
		char input[MAX_SIZE];
		char output[MAX_SIZE][MAX_SIZE];
		int append[MAX_SIZE];
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
		int pid = fork();
		if(pid == 0){
			if(strcmp(tokens[0], "cd") == 0){
				cd_builtin(tokens, argc, home_dir);
			}
			else if(strcmp(tokens[0], "pwd") == 0){
				pwd_builtin(tokens, argc, home_dir);
			}
			else if(strcmp(tokens[0], "echo") == 0){
				echo_builtin(tokens, argc, home_dir);
			}
			else if(strcmp(tokens[0], "ls") == 0){
				ls_builtin(tokens, argc, home_dir);
			}
			else if(strcmp(tokens[0], "pinfo") == 0){
				pinfo_builtin(tokens, argc, home_dir);
			}
			else if(strcmp(tokens[0], "remindme") == 0){
				remindme(tokens, argc, home_dir);
			}
			else if(strcmp(tokens[0], "clock") == 0)
			{
				clock_builtin(tokens, argc, home_dir);
			}
			else{
				execvp(tokens[0], &tokens[0]);
				exit(1);
			}
		}
		wait(NULL);
		dup2(restore_input, 0);
		dup2(restore_output, 1);
		close(restore_input);
		close(restore_output);
	}
	else
	{
		int pid = fork();
		if(pid != 0)
			printf("%d\n", pid);
		if(pid == 0){
			if(strcmp(tokens[0], "cd") == 0){
				cd_builtin(tokens, argc, home_dir);
			}
			else if(strcmp(tokens[0], "pwd") == 0){
				pwd_builtin(tokens, argc, home_dir);
			}
			else if(strcmp(tokens[0], "echo") == 0){
				echo_builtin(tokens, argc, home_dir);
			}
			else if(strcmp(tokens[0], "ls") == 0){
				ls_builtin(tokens, argc, home_dir);
			}
			else if(strcmp(tokens[0], "pinfo") == 0){
				pinfo_builtin(tokens, argc, home_dir);
			}
			else if(strcmp(tokens[0], "remindme") == 0){
				remindme(tokens, argc, home_dir);
			}
			else if(strcmp(tokens[0], "clock") == 0)
			{
				clock_builtin(tokens, argc, home_dir);
			}
			else{
				execvp(tokens[0], &tokens[0]);
				exit(1);
			}
		}
	}
	return 0;
}
