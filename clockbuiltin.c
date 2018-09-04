#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#define MAX_SIZE 105
int clock_builtin(char **tokens, int argc, char *home_dir)
{
	if(argc != 5){
		perror("Invalid Usage\n");
		return -1;
	}
	int time_limit = 0;
	for(int i = 0; i < strlen(tokens[4]); i++){
		if(tokens[4][i] >= '0' && tokens[4][i] <= '9')
		time_limit = (time_limit * 10) + (tokens[4][i] - '0');
		else
		{
			perror("Invalid number in argument 5");
			return -1;
		}
	}
	int __time = 0;
	for(int i = 0; i < strlen(tokens[2]); i++){
		if(tokens[2][i] >= '0' && tokens[2][i] <= '9')
		__time = (__time * 10) + (tokens[2][i] - '0');
		else
		{
			perror("Invalid number in argument 3");
			return -1;
		}		
	}
	for(int i = 1; i <= time_limit; i += __time)
	{
		time_t t = time(NULL);
		char * res = malloc(MAX_SIZE * sizeof(char));
		strftime(res, MAX_SIZE, "%b%e %Y, %H:%M:%S ", localtime(&t));
		printf("%s\n", res);
		sleep(__time);
	}
	return 0;
}