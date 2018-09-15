int setenvBuiltin(char ** arguments, int cnt, char * home_dir);
int unsetenvBuiltin(char ** arguments, int cnt, char * home_dir);
int getenvBuiltin(char ** arguments, int cnt, char * home_dir);
void print_jobs(char ** arguments, int cnt, char * home_dir);
int addProc(int PID, char * NAME);