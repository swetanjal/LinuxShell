#include <stdio.h>
int echo_builtin(char **args, int cnt, char *home_dir)
{
	for(int i = 1; i < cnt; i++){
		printf("%s ", args[i]);
	}
	printf("\n");
	return 1;
}