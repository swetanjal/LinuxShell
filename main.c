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
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

int background_processes;

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

char **split(char * line)
{
	char ** tokens = malloc(MAX_SIZE * sizeof(char*));
	char * token;
	token = strtok(line, ";\n");
	int count = 0;
	while(token != NULL)
	{
		tokens[count++] = token;
		token = strtok(NULL, ";\n");
	}
	tokens[count++] = NULL;
	return tokens;
}

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

void handler(int sig)
{
	fflush(stdout);
	pid_t pid = wait(NULL);
	if(pid != -1){
		printf("Process with pid %d exited normally.\n", pid);
		--background_processes;
	}
	return;
}

int execute(char **tokens, int cnt, char *home_dir, int bg)
{
	int status;
	if(bg == 0)
	{
		/*This is a foreground process*/
		int pid = fork();
		if(pid == 0){
			execvp(tokens[0], &tokens[0]);
			exit(1);
		}
		//waitpid(pid, &status, WUNTRACED);
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
		//waitpid(pid, &status, WNOHANG);
	}
	return 0;
}


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
		//Error: Invalid Process.
	}
	if(val == -1){
		//Error : Read Link Error
		return 1;
	}
	else
	{
		printf("Executable path -- %s\n", link);
	}
	return 1;
}

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
			if(tokens[i][0] == '~'){
				char tmp[MAX_SIZE];
				for(int i = 0; i < MAX_SIZE; i++)
					tmp[i] = '\0';
				strcat(tmp, home_dir);
				strcat(tmp, &tokens[i][1]);
				list_directory(tmp, curr, l_flag, a_flag, 1);
			}
			else
				list_directory(tokens[i], curr, l_flag, a_flag, 1);
			printf("\n");
		}
		
	}
	return 0;
}
char rel[MAX_SIZE];

void abs_to_rel(char *home_dir, char *path)
{
	rel[0] = '~';
	rel[1] = '/';
	for(int i = 2; i <= strlen(path) - strlen(home_dir) + 1; i++)
		rel[i] = path[i - 1 + strlen(home_dir)];
	rel[strlen(path) - strlen(home_dir) + 2] = '\0';
}

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

void shell_loop()
{
	char *system_name = (char *)malloc(MAX_SIZE * sizeof(char));
	gethostname(system_name, MAX_SIZE);
	char *username = getenv("USER");
	char *home_dir = (char *)malloc(MAX_SIZE * sizeof(char));
	getcwd(home_dir, MAX_SIZE);
	while(1)
	{
		char *curr_dir = (char *)malloc(MAX_SIZE * sizeof(char));
		char *stat = getcwd(curr_dir, MAX_SIZE);
		abs_to_rel(home_dir, curr_dir);
		printf("<%s@%s:%s> ", username, system_name, rel);
		char *input_str = input();
		char **commands = split(input_str);
		int test_cases = arg_count(commands);
		
		for(int ccc = 0; ccc < test_cases; ccc++){
		char *input_string = commands[ccc];
		int background = 0;
		int cnt = 0;
		while(input_string[cnt] != '\0')
		{
			cnt++;
		}
		if(input_string[cnt - 1] == '&'){
			input_string[cnt - 1] = '\0';
			background = 1;
		}
		else if(input_string[cnt - 2] == '&'){
			input_string[cnt - 2] = '\0';	
			background = 1;
			//background_processes++;
		}
		char **tokens = lsh_split_line(input_string);
		int argc = arg_count(tokens);
		if(background){
			background_processes++;
			execute(tokens, argc, home_dir, background);
		}
		else if(strcmp(tokens[0], "cd") == 0){
			cd_builtin(tokens, argc, home_dir);
		}
		else if(strcmp(tokens[0], "pwd") == 0){
			pwd_builtin(tokens, argc, home_dir);
		}
		else if(strcmp(tokens[0], "echo") == 0){
			echo_builtin(tokens, argc, home_dir);
		}
		else if(strcmp(tokens[0], "ls") == 0){
			ls_builtin(tokens, argc, home_dir);
		}
		else if(strcmp(tokens[0], "pinfo") == 0){
			pinfo_builtin(tokens, argc, home_dir);
		}
		else if(strcmp(tokens[0], "remindme") == 0){
			remindme(tokens, argc, home_dir);
		}
		else if(strcmp(tokens[0], "clock") == 0)
		{
			clock_builtin(tokens, argc, home_dir);
		}
		else{
			execute(tokens, argc, home_dir, background);
		}
		if(background_processes > 0)
		{
			signal(SIGCHLD, handler);
		}
		//free(input_string);
		//free(tokens);
		}
		free(input_str);
		free(commands);
	}
}

int main()
{
	shell_loop();
	return 0;
}