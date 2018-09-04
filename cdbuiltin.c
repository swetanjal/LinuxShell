#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define MAX_SIZE 105
int cd_builtin(char **args, int cnt, char *home_dir)
{
	if(args[1] == NULL){
		//Incorrect usage, only one argument was passed.
		perror("Incorrect Usage\n");
		return -1;
	}
	if(args[1][0] == '~'){
		char *new_dir = malloc(MAX_SIZE * sizeof(char));
		new_dir[0] = '\0';
		strcat(new_dir, home_dir);
		strcat(new_dir, &args[1][1]);
		int status = chdir(new_dir);
		free(new_dir);
		if(status == 0){
			return 1;
		}
		else{
			perror("Error");
			return 0;	
		}
	}
	int status = chdir(args[1]);
	if(status == 0){
		return 1;
	}
	else{
		/*Print invalid directory*/
		perror("Error");
		return 0;
	}
}
