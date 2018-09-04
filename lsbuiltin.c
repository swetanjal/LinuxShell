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
		if(success == -1)
		{
			perror("Error");
			return -1;
		}
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
	if(status == NULL)
	{
		perror("Error");
		return -1;
	}
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
				DIR * ptr = opendir(tmp);
				if(ptr == NULL)
				{
					perror("Error");
					continue;
				}	
				list_directory(tmp, curr, l_flag, a_flag, 1);
			}
			else{
				DIR * ptr = opendir(tokens[i]);
				if(ptr == NULL)
				{
					perror("Error");
					continue;
				}
					
				list_directory(tokens[i], curr, l_flag, a_flag, 1);
			}
			printf("\n");
		}
		
	}
	return 0;
}