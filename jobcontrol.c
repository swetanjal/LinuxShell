#include <stdlib.h>
#include <stdio.h>
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