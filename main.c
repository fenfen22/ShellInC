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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM "\t\r\n\a"
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
    pid_t pid, wpid;
    int status;

    pid = fork();
    if(pid = 0){

    }


}

int lsh_execute(char** args){

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