typedef struct command command;
int setenvBuiltin(char ** arguments, int cnt, char * home_dir);
int unsetenvBuiltin(char ** arguments, int cnt, char * home_dir);
int getenvBuiltin(char ** arguments, int cnt, char * home_dir);
void print_jobs(char ** arguments, int cnt, char * home_dir, command * tail);
command * addToBackground(command proc, command * tail);
command * removeFromBackground(int pid, command * tail);
void kjob(char ** arguments, int cnt, char * home_dir, command * tail);
void overkill(char ** tokens, int K, char * home_dir, command * tail);