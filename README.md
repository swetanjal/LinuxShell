# LinuxShell
1. make
2. ./shell

# Features

The code is written in modular style and easily is extensible. There are seperate files for functions with seperate tasks.
- main.c - Contains code which controls the main flow of the shell. Contains code to accept input, split and check if some background process ended successfully or not.
- cdbuiltin.c - Contains code implementing builtin function to change directory.
- lsbuiltin.c - Contains code implementing builtin function to list directory.
- execute.c - Executes a processes given to it along with handling whether it has to be executed in the foreground or background.
- pwdbuiltin.c - Contains code implementing builtin function to print current working directory.
- pinfobuiltin.c - Contains code implementing builtin function to print info about a given process.
- echobuiltin.c - Contains code implementing builtin function echo.
- remindme.c - Contains code implementing bonus part 1 to remind user something after certain interval of time.
- clockbuiltin.c - Contains code implementing bonus part 2.
