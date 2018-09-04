#include <unistd.h>
#include <stdio.h>
#define MAX_SIZE 105
int pwd_builtin(char **args, int cnt, char *home_dir)
{
	char pwd[MAX_SIZE];
	char * status = getcwd(pwd, MAX_SIZE);
	if(status == NULL){
		/*Print unable to get pwd*/
		perror("Error");
		return 0;
	}
	else{
		printf("%s\n", pwd);
		return 1;
	}
}