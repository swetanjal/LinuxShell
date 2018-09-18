#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#define MAX_SIZE 105
typedef struct command
{
	char name[MAX_SIZE];
	int pid;
	struct command * prev;
}command;

int setenvBuiltin(char ** arguments, int cnt, char * home_dir)
{
	if(cnt == 3)
	{
		setenv(arguments[1], arguments[2], 1);
		return 0;
	}
	else if(cnt == 2)
	{
		setenv(arguments[1], "", 1);
		return 0;
	}
	else
	{
		perror("Incorrect Usage");
		return -1;
	}
}

int getenvBuiltin(char ** arguments, int cnt, char * home_dir)
{
	if(cnt < 2)
	{
		perror("Incorrect Usage");
		return -1;
	}
	else 
		printf("%s\n", getenv(arguments[1]));
	return 0;
}

int unsetenvBuiltin(char ** arguments, int cnt, char * home_dir)
{
	if(cnt < 2)
	{
		perror("Incorrect Usage");
		return -1;
	}
	unsetenv(arguments[1]);
	return 0;
}

void rec_print(command * ptr, int cnt)
{
	if(ptr == NULL)
		return;
	rec_print(ptr->prev, cnt - 1);
	printf("[%d]%d %s\n", cnt, ptr->pid, ptr->name);
}

void print_jobs(char ** arguments, int cnt, char * home_dir, command * tail)
{
	command * starter = tail;
	int proc_cnt = 0;
	while(starter != NULL)
	{
		proc_cnt++;
		starter = starter -> prev;
	}
	starter = tail;
	rec_print(starter, proc_cnt);
}

command * addToBackground(command proc, command * tail)
{
	command * ptr = (command *)malloc(sizeof(command));
	ptr->pid = proc.pid;
	strcpy(ptr->name, proc.name);
	ptr->prev = tail;
	tail = ptr;
	return tail;
}

command * removeFromBackground(int pid, command * tail)
{
	command * ptr = tail;
	command * next = NULL;
	while(ptr -> pid != pid)
	{
		next = ptr;
		ptr = ptr -> prev;
	}
	if(ptr == tail)
	{
		tail = ptr -> prev;
		free(ptr);
	}
	else
	{
		next -> prev = ptr -> prev;
		free(ptr);
	}
	return tail;
}

void kjob(char ** tokens, int K, char * home_dir, command * tail)
{
	int proc_no = 0;
	if(K != 3)
	{
		perror("Invalid Usage");
		return;
	}
	int sig = 0;
	for(int i = 0; i < strlen(tokens[1]); i++)
	{
		if(tokens[1][i] >= '0' && tokens[1][i] <= '9')
			proc_no = (proc_no * 10) + (tokens[1][i] - '0');
		else {
			perror("Invalid datatype.");
			return;
		}
	}
	for(int i = 0; i < strlen(tokens[2]); i++)
	{
		if(tokens[2][i] >= '0' && tokens[2][i] <= '9')
			sig = (sig * 10) + (tokens[2][i] - '0');
		else{
			perror("Invalid datatype.");
			return;	
		}
	}
	command * ptr = tail;
	int c = 0;
	while(ptr != NULL){
		c++;
		ptr = ptr -> prev;
	}
	ptr = tail;
	int cnt = 0;
	//printf("%d\n", proc_no);
	while(ptr != NULL && (c - cnt) != proc_no)
	{
		cnt++;
		ptr = ptr -> prev;
	}
	if(ptr == NULL)
	{
		perror("Invalid process number.");
		return ;
	}
	int id = ptr -> pid;
	kill(id, sig);
}

void kill_proc(command * ptr)
{
	if(ptr == NULL)
		return;
	kill_proc(ptr -> prev);
	kill(ptr -> pid , 9);
}

void overkill(char ** tokens, int K, char * home_dir, command * tail)
{
	command * starter = tail;
	kill_proc(starter);
}

command * fgBuiltin(char ** arguments, int K, char * home_dir, command * tail)
{
	if(K != 2)
	{
		perror("Invalid Usage.");
		return tail;
	}
	int proc_no = atoi(arguments[1]);
	command * ptr = tail;
	int c = 0;
	while(ptr != NULL){
		c++;
		ptr = ptr -> prev;
	}
	ptr = tail;
	int cnt = 0;
	while(ptr != NULL && (c - cnt) != proc_no)
	{
		cnt++;
		ptr = ptr -> prev;
	}
	siginfo_t fgStatus;
	int val = kill(ptr->pid, SIGCONT);
	int PID = ptr -> pid;
	tail = removeFromBackground(ptr -> pid, tail);
	waitid(P_PID, PID, &fgStatus, (WUNTRACED | WNOWAIT));
	return tail;
}