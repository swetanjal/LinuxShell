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
#define MAX_SIZE 105

int pinfo_builtin(char **tokens, int cnt, char *home_dir)
{
	long long int pid = 0;
	if(cnt == 1){
		pid = getpid();
	}
	else
	{
		for(int i = 0; i < strlen(tokens[1]); i++)
		{
			if(tokens[1][i] >= '0' && tokens[1][i] <= '9')
			pid = (pid * 10) + (tokens[1][i] - '0');
			else{
				perror("Invalid process id!");
				return -1;
			}
		}
	}
	
	char exec[MAX_SIZE]; char stat[MAX_SIZE];
	exec[0] = '/';
	exec[1] = 'p';
	exec[2] = 'r';
	exec[3] = 'o';
	exec[4] = 'c';
	exec[5] = '/';
	exec[6] = '\0';
	stat[0] = '/';
	stat[1] = 'p';
	stat[2] = 'r';
	stat[3] = 'o';
	stat[4] = 'c';
	stat[5] = '/';
	stat[6] = '\0';
	char *EXEC = "/exe\0";
	char *STAT = "/stat\0";
	long long copy = pid;
	int len = 0;
	while(copy > 0){
		copy /= 10;
		len++;
	}
	char PID[MAX_SIZE];
	copy = pid;
	for(int i = 0; i < len; i++){
		PID[len - 1 - i] = (char)(48 + (copy % 10));
		copy /= 10;
	}
	PID[len] = '\0';
	strcat(exec, PID);
	strcat(stat, PID);
	strcat(exec, EXEC);
	strcat(stat, STAT);
	char link[MAX_SIZE];
	for(int i = 0; i < MAX_SIZE; i++)
		link[i] = '\0';
	int val = readlink(exec, link, MAX_SIZE);
	FILE * fd = fopen(stat, "r");
	if(fd != NULL){
		char  b[MAX_SIZE];
		fscanf(fd, " %4096s", b);
		fscanf(fd, " %4096s", b);
		fscanf(fd, " %4096s", b);
		printf("pid -- %lld\n", pid);
		printf("Process Status -- %s\n", b);
		for(int i=0; i<20; ++i) fscanf(fd, " %4096s", b);
		printf("Virtual Memory -- %s\n", b);
	}
	else
	{
		perror("Error");
		return -1;
		//Error: Invalid Process.
	}
	if(val == -1){
		//Error : Read Link Error
		perror("Error");
		return -1;
	}
	else
	{
		printf("Executable path -- %s\n", link);
	}
	return 1;
}