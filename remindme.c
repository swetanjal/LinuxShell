#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
int remindme(char **tokens, int cnt, char *home_dir)
{
	//printf("REMINDME!!!\n");
	int time = 0;
	if(cnt < 3)
	{
		//Invalid Usage
		perror("Invalid Usage of command");
		return -1;
	}
	for(int i = 0; i < strlen(tokens[1]); i++)
	{
		if(tokens[1][i] >= '0' && tokens[1][i] <= '9')
		{
			time = (time * 10) + (tokens[1][i] - '0');
		}
		else
		{
			//Invalid number
			perror("Invalid number in second argument!");
			return -1;
		}
	}
	int pid = fork();
	if(pid == 0){
		sleep(time);
		printf("Reminder: ");
		for(int i = 2; i < cnt; i++)
			printf("%s ", tokens[i]);
		printf("\n");
	}
	return 1;
}