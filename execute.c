#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
int execute(char **tokens, int cnt, char *home_dir, int bg)
{
	if(bg == 0)
	{
		/*This is a foreground process*/
		int pid = fork();
		if(pid == 0){
			execvp(tokens[0], &tokens[0]);
			exit(1);
		}
		wait(NULL);
	}
	else
	{
		int pid = fork();
		if(pid != 0)
			printf("%d\n", pid);
		if(pid == 0){
			execvp(tokens[0], &tokens[0]);
			exit(1);	
		}
	}
	return 0;
}
