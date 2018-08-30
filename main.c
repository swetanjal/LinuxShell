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
/*To be changed*/
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
char **lsh_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}
/************/

int echo_builtin(char **args, int cnt, char *home_dir)
{
	for(int i = 1; i < cnt; i++){
		printf("%s ", args[i]);
	}
	printf("\n");
}

int cd_builtin(char **args, int cnt, char *home_dir)
{
	if(args[1] == NULL){
		//Incorrect usage, only one argument was passed.

	}
	int status = chdir(args[1]);
	if(status == 0)
		return 1;
	else{
		/*Print invalid directory*/
		return 0;
	}
}

int pwd_builtin(char **args, int cnt, char *home_dir)
{
	char pwd[MAX_SIZE];
	char * status = getcwd(pwd, MAX_SIZE);
	if(status == NULL){
		/*Print unable to get pwd*/
		return 0;
	}
	else{
		printf("%s\n", pwd);
		return 1;
	}
}

char* input()
{
	char * line;
	size_t n;
	getline(&line, &n, stdin);
	return line;
}

int arg_count(char **tokens)
{
	int cnt = 0;
	while(tokens[cnt++] != NULL);
	cnt = cnt - 1;
	return cnt;
}

typedef struct{
	char *name;
	char **args;
}command;

int execute(char **tokens, int cnt, char *home_dir, int bg)
{
	if(bg == 0)
	{
		/*This is a foreground process*/
		int pid = fork();
		if(pid == 0){
			execvp(tokens[0], &tokens[0]);
		}
		wait(NULL);
	}
	return 0;
}


int ls_builtin(char **tokens, int cnt, char *home_dir)
{
	int compare(const char *str1, const char *str2)
	{
		return strcmp(str1, str2);
	}
	int __sort(const struct dirent **name1, const struct dirent **name2)
	{
		return compare((*name1)->d_name, (*name2)->d_name);
	}
	int longlist(char *file)
	{
		/*Long Lists file*/
		struct stat element;
		int success = stat(file, &element);
		if(S_ISDIR(element.st_mode))
			printf("d");
		else
			printf("-");
		
		if(element.st_mode & S_IRUSR)
			printf("r");
		else
			printf("-");
		if(element.st_mode & S_IWUSR)
			printf("w");
		else
			printf("-");
		if(element.st_mode & S_IXUSR)
			printf("x");
		else
			printf("-");

		if(element.st_mode & S_IRGRP)
			printf("r");
		else
			printf("-");
		if(element.st_mode & S_IWGRP)
			printf("w");
		else
			printf("-");
		if(element.st_mode & S_IXGRP)
			printf("x");
		else
			printf("-");

		if(element.st_mode & S_IROTH)
			printf("r");
		else
			printf("-");
		if(element.st_mode & S_IWOTH)
			printf("w");
		else
			printf("-");
		if(element.st_mode & S_IXOTH)
			printf("x");
		else
			printf("-");
		printf("\t%3ld\t", element.st_nlink);
		printf("%10s\t", getpwuid(element.st_uid)->pw_name);
		printf("%10s\t", getgrgid(element.st_gid)->gr_name);
		printf("%5ld\t", element.st_size);
		char *c_time_string = ctime(&element.st_mtim.tv_sec);
		c_time_string[strlen(c_time_string) - 1] = '\0';
		printf("%s\t", c_time_string);
		return 1;
	}
	void list_directory(char *dir, char *pwd, int L, int A, int not_pwd)
	{
		/*If not_pwd is 1, it means that the directory we are printing is not the pwd.*/
		struct dirent **nameList;
		int dir_cnt =scandir(dir, &nameList, NULL, __sort);
		for(int i = 0; i < dir_cnt; i++){
			
			if(A == 0){
				if(nameList[i]->d_name[0] != '.'){
					if(L == 1){
						char path[MAX_SIZE];
						path[0] = '\0';
						strcat(path, pwd);
						strcat(path, "/");
						if(not_pwd){
							strcat(path, dir);
							strcat(path, "/");
						}
						strcat(path, nameList[i]->d_name);
						longlist(path);
					}
					printf("%s ", nameList[i]->d_name);
					if(L == 1)
					printf("\n");
				}
			}
			else
			{
				if(L == 1){
					char path[MAX_SIZE];
					path[0] = '\0';
					strcat(path, pwd);
					strcat(path, "/");
					if(not_pwd){
						strcat(path, dir);
						strcat(path, "/");
					}
					strcat(path, nameList[i]->d_name);
					longlist(path);
				}
				printf("%s ", nameList[i]->d_name);
				if(L == 1)
					printf("\n");	
			}
		}
		if(L == 0)
			printf("\n");
	}
	int l_flag = 0;
	int a_flag = 0;
	int many = 0;
	for (int i = 1; i < cnt; ++i)
	{
		if(tokens[i][0] == '-')
		{
			int len = 1;
			while(tokens[i][len] != '\0')
			{
				if(tokens[i][len] == 'l')
					l_flag = 1;
				if(tokens[i][len] == 'a')
					a_flag = 1;
				len++;
			}
		}
		else
			many++;
	}
	char curr[MAX_SIZE];
	char * status = getcwd(curr, MAX_SIZE);
	if(many == 0)
	{	
		list_directory(curr, curr, l_flag, a_flag, 0);
		return 1;
	}
	for(int i = 1; i < cnt; ++i)
	{
		if(tokens[i][0] != '-')
		{
			if(many > 1)
				printf("%s:\n", tokens[i]);
			list_directory(tokens[i], curr, l_flag, a_flag, 1);
			printf("\n");
		}
		
	}
	return 0;
}

void shell_loop()
{
	char *system_name = (char *)malloc(MAX_SIZE * sizeof(char));
	gethostname(system_name, MAX_SIZE);
	char *username = getenv("USER");
	while(1)
	{
		printf("<%s@%s:~> ", username, system_name);
		char *input_string = input();
		int background = 0;
		int cnt = 0;
		while(input_string[cnt] != '\0')
		{
			cnt++;
		}
		if(input_string[cnt - 1] == '&')
			background = 1;
		char **tokens = lsh_split_line(input_string);
		int argc = arg_count(tokens);
		execute(tokens, argc, "lol", background);
		if(background){
			execute(tokens, argc, "lol", background);
		}
		if(strcmp(tokens[0], "cd") == 0){
			cd_builtin(tokens, argc, "lol");
		}
		else if(strcmp(tokens[0], "pwd") == 0){
			pwd_builtin(tokens, argc, "lol");
		}
		else if(strcmp(tokens[0], "echo") == 0){
			echo_builtin(tokens, argc, "lol");
		}
		else if(strcmp(tokens[0], "ls") == 0){
			ls_builtin(tokens, argc, "lol");
		}
		else{
			execute(tokens, argc, "lol", background);
		}
		free(input_string);
		free(tokens);
	}
}

int main()
{
	shell_loop();
	return 0;
}