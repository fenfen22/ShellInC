/*
A shell does three things in its lifetime.
(1) Initialize: a typical shell would read and execute its configuration files. These would change aspects of the shell's behavior.
(2) Interpret: the shell reads commands from stdin (which could be interactive, or a file) and executes them.
(3) Terminate: after its commands are executed, the shell executes any shutdown commands, frees up any memory, and terminates.

It's an important architecture, that's the basic lifetime of the program.
*/

/*
This shell would be very simple, as there won't be any configuration files and there won't be any shutdown command. We will just call the looping fuction and then terminate.
*/

/*
What does the shell do during its loop? a simple way to handle commands is with three steps:
(1) Read: Read the command from standard input.
(2) Parse: Separate the command string into a program and arguments.
(3) Excute: Run the parsed command.
*/
#include <stdio.h>              //fprintf(), printf(), stderr, perror()
#include <stdlib.h>             //malloc(), realloc(), free(), exit(), execvp(), EXIT_SUCCESS, EXIT_FAILURE
#include <sys/wait.h>           //waitpid() and associated macros
#include <unistd.h>             //chdir(), fork(), exec(), pid_t
#include <string.h>             //strcmp(), strtok()
#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM "\t\r\n\a"

//running gcc -o main main.c to compile it, and then ./main to run it on a Linux Machine

/*
function: lsh_read_line
We don't know ahead of time how much text a user will enter in their shell, so need to start with a block, 
if they do exceed it, reallocate with more space.
*/

char *lsh_read_line(void){
    char* line = NULL;
    size_t bufsize = 0;    //They are using ssize_t

    if(getline(&line, &bufsize, stdin) == -1){      //getline(array of characters, number of characters, terminator)
        if(feof(stdin)){        //receive a EOF(end of file)
            exit(EXIT_SUCCESS);
        }
        else{
            perror("readline");
            exit(EXIT_FAILURE);
        }

    }
    return line;

}



/*
we wll simply use whitespace to separate arguments from each other, that is mean we won't allow quoting or 
backslash escaping in our command line arguments
*/
char **lsh_split_line(char* line){
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));        //array of pointers
    char * token;           

    if(!tokens){
        fprintf(stderr,"lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);        //it returns a pointer to the first token 
    //strtok() actually does it return pointers to within the string you give it, and place \0 bytes at the end of each token

    while(token != NULL){       //the process repeats until no token is returned by strtok
        tokens[position] = token;       //we store each pointer in an array (buffer) of character pointers.
        position++;

        if(position >= bufsize){
            bufsize += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));      //we reallocate the array of pointers if necessary
            if(!tokens){
                fprintf(stderr,"lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, LSH_TOK_DELIM);        //we null-terminate the list of tokens, because no tokens will be returned by strtok.
    }
    tokens[position] = NULL;
    return tokens;

}

/*
two ways of starting processes on Unix. The first one is by being init.

when a Unix computer boots, its kernel is loaded. Once it is loaded and initialized, the kernel
starts only one process, which is called init. This process runs for the intire length of time that
the computer is on, and it manages loading up the rest of the processes that you need for your cpomputer to be useful.

most programs aren't init, the other one is the fork() system call.

when this function is called, the OS makes a duplicate of the process and starts them both running. The original process is called the "parent"
and the new one is called the "child". fork() returns 0 to the child process, and it returns to the parent the process ID (PID) of its child.
This mean that the only way for new processes is to start it by an exiting one duplicating itself.

exec() system call, it could replace the current running program with an entirely new one, this mean that when you call exec, the OS stops your process,
loads up the new program, and starts that one in its place. A process never returns from an exec() call (unless that's an error).

Now, we are going to build blocks for how most programs are run on Unix.
First, an existing process forks itself into two separate ones. Then, the child uses exec() to replace itself with a new program.
The parent process can continue doing other things, and it can keep tags on its children, using the system call wait()
*/

int lsh_launch(char** args){
    //pid_t data type stands for process identification and it is used to represent process ids
    pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid == 0){
        //children
        if(execvp(args[0], args) == -1){        //if the exec system call returns -1 or if it returns, we know there was an error
            perror("lsh");      //we use perror to print the system's error message, along with our program name
        }
        exit(EXIT_FAILURE);     //then, we exit so the shell can keep running.
    }
    else if (pid < 0)
    {
        //error forking
        perror("lsh");  
    }
    else{           //fork() execute successfully
        //parent process
        do{
            wpid = waitpid(pid, &status, WUNTRACED);
        }while(!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

/*
most commands execute by a shell are programs, but not all of them, some of them are built right into the shell.

these commands could only cange the shell's operation if they were implemented within the shell proces itself.\

the shell process itself needs to execute chdir(), so that its own current directory is updated, then, when it launches child processes, 
they will inherit that directory too.

Now, we are going to add some commands to the shell itself, like cd, exit and help.
*/
int lsh_cd(char** args);
int lsh_help(char**args);
int lsh_exit(char** args);      //forward declarations

//an array of builtin command names
char * builtin_str[] = {
    "cd",
    "help",
    "exit"
};

//an array of their corresponding functions
int (*builtin_func[]) (char**) = {      //it is an array of function pointers (that take array of strings and return an int)
    &lsh_cd,
    &lsh_help,
    &lsh_exit
};

int lsh_num_builtis(){
    return sizeof(builtin_str) / sizeof(char*);
}

int lsh_cd(char** args){        //implement cd
    if(args[1] == NULL){        //if its second argument does not exist, then print an error message.
        fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    }
    else{
        if(chdir(args[1]) != 0)     //call chdir(), check for errors, and returns
            perror("lsh");
    }
    return 1;
}


//the help function prints a nice message and the names of all the buitins.
int lsh_help(char** args){
    int i;
    printf("LSH\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for(i = 0; i < lsh_num_builtis(); i++){
        printf(" %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;

}

//the exit function returns 0, as a signal for the command loop to terminate.
int lsh_exit(char** args){
    return 0;
}


//this function will either launch a builtin, or a process.
int lsh_execute(char** args){
    int i;

    if(args[0] == NULL){
        // an empty command was entered
        return 1;
    }

    for(i = 0; i < lsh_num_builtis(); i++){
        if(strcmp(args[0], builtin_str[i]) == 0){ //to check if the command equals each builtin
            return (*builtin_func[i])(args);   //if so, run it
        }
    }
    return lsh_launch(args);    //if doesn't match a builtin, it calls lsh_launch() to launch the process.

}

void lsh_loop(void)
{
    char *line;
    char **args;
    int status;

    //the do-while loop is more convienient for checking the status variable, 
    //because it executes once before checking its value.
    do{
        printf("> ");       //print a prompt
        line = lsh_read_line();     //call a function to read a line
        args = lsh_split_line(line);        //call a function to split the line into args
        status = lsh_execute(args);         //excute the args

        free(line);
        free(args);         //free the line and arguments that we created earlier.
    }while(status);         //using a status variable returned by lsh_executed() to determine when to exit.


}

int main(int argc, char **argv)
{
    // TODO: Load confi files, if any. 
    // TODO: Run command loop.
    lsh_loop();

    // TODO: Perform any shutdown/clearup

    return EXIT_SUCCESS;


}