// Joshua Hesseltine
// CS344 Program 3
// smallsh.c 
// shell in c accepts command : [arg1 arg2 ...] [< input_file] [> output_file] [&]

//sources used:
//http://pubs.opengroup.org/onlinepubs/009695399/functions/open.html
//https://gist.github.com/aspyct/3462238
//http://stackoverflow.com/questions/33848558/catching-sigterm-in-c
//https://users.cs.cf.ac.uk/Dave.Marshall/C/node24.html
//http://stackoverflow.com/questions/4788374/writing-a-basic-shell
//http://codewiki.wikidot.com/c:system-calls:dup2
//http://www.linuxforums.org/forum/programming-scripting/40078-c-printing-linux-process-table.html
//http://stackoverflow.com/questions/15798450/open-with-o-creat-was-it-opened-or-created

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>

#define MAX_INPUT 2048
#define MAX_ARG 513 

bool runOutRedirect(char** array, int* count, char* fileOut);
bool runInputRedirect(char** array, int* count, char* fileIn);
bool runBackground(char** array, int* count);
void interruptSignal(int sigNumber); 
void stopSignal(int sigNumber);
bool noInputStr(char* input_string);
void checkProcesses(int* status);

int main() 

{
    //status handler
     static int status = 0; 
    
    //signal handlers
    signal(SIGINT, interruptSignal);
    signal(SIGTSTP, stopSignal);

    // main variables
    char userInput[MAX_INPUT];
    int count           = 0;
    int spawnId         = 0;
    bool spawn          = false;
    bool output         = false;
    bool input          = false;
    char fileOut[MAX_INPUT];
    char fileIn[MAX_INPUT];
    
    //command arguments
    char** array = malloc(MAX_ARG * sizeof(char*));
   
    // set to NULL per instructions
    int i;
    for (i = 0 ; i < MAX_ARG ; i++)
    {
        array[i] = NULL;
    }
    
    while(1)
    {
        spawn   = false;
        output  = false;
        input   = false;
        
        checkProcesses(&status);

        printf(": ");
        fflush(stdout);
        
        //read in user input and save 
        memset(userInput, '\0', MAX_INPUT);
        fgets(userInput, MAX_INPUT, stdin);
        
        //if user input is empty
        if(noInputStr(userInput))
        {
            continue;
        }

        userInput[strlen(userInput) - 1] = '\0';

        char* token;
        int cleanArray;

        //clean the array to prevent mem-leak
        for(cleanArray = 0 ; cleanArray < MAX_ARG ; cleanArray++)
        {
            if(array[cleanArray] != NULL)
            {
                free(array[cleanArray]);
                array[cleanArray] = NULL;
            }
            else
            {
                break;
            }
        }
        
        for (count = 0 ; count < MAX_ARG ; count++)
        {
            if(count == 0)
            {
                token = strtok(userInput, " ");
            }
            else
            {
                token = strtok(NULL, " ");
            }
            //done?
            if(token == NULL)
            {
                break;
            }
            
            //dynamically allocate space
            array[count] = malloc((strlen(token) + 1) * sizeof(char));
            
            //copy the string
            strcpy(array[count], token);
        }

        //validate input
        int j;
        if(strcmp(array[0], "exit") == 0)
        {
            for(j = 0 ; j < MAX_ARG ; j++)
            {
                if(array[j] != NULL)
                {
                    free(array[j]);
                    array[j] = NULL;
                }
                else
                {
                    break;
                }
            }
            free(array);
            exit(EXIT_SUCCESS);
            
        }
        else if(strcmp(array[0], "status") == 0)
        {
            //Print our exit status variable, and continue
            printf("exit value %d\n", WEXITSTATUS(status));
            fflush(stdout);
            continue;
            
        }
        else if(strcmp(array[0], "cd") == 0)
        {
            //check which dir
            if((array[1] == NULL) || \
               (strcmp(array[1], "~/") == 0) || \
               (strcmp(array[1], "~") == 0))
            {
                chdir(getenv("HOME")); // change to HOME dir
                
            }
            else
            {
                //change to user dir   
                int chdir_result = chdir(array[1]);
                if(chdir_result == -1){
                    printf("wrong directory \"%s\"\n", \
                           array[1]);
                    fflush(stdout);
                }
            }
            continue;
        }
        else if(array[0][0] == '#')
        {
            //user inputted comment line so do nothing 
            continue;
        }
        
        //check for running in background
        spawn = runBackground(array, &count);
        
        //check if we need to redirect
        output = runOutRedirect(array, &count, fileOut);
        
        //check if we need to redirect input file
        input = runInputRedirect(array, &count, fileIn);

        //clean the input line if redirect 
        int k;
        for (k = count; k < MAX_ARG; k++ ) 
        {
            if (array[k] != NULL) 
            {
                free(array[k]);
                array[k] = NULL;
            }
            else 
            {
                break;
            }
        }

        //fork new process
        spawnId = fork();
        
        if(spawnId == 0)
        {
            //entering child process
            if(spawn)
            {
                //set stdin and stdout file to /dev/null
                int rw = open("/dev/null", O_RDWR);
                int r = open("/dev/null", O_RDONLY);
                // duplicate file deccriptor
                dup2(r, 0);
                dup2(rw, 1);
            }
            
            if(input)
            {
                // open file and pass in
                int inputFileDescriptor = open(fileIn, O_RDONLY, 0644);

                if(inputFileDescriptor < 0){
                    printf("cannot open %s for input \n", fileIn);
                    fflush(stdout);
                    continue;
                }

                dup2(inputFileDescriptor, 0);
            }

            if(output)
            {
                // create output file and redirect
                int outputFileDescriptor = open(fileOut, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                if(outputFileDescriptor < 0)
                {
                    printf("failed to run: %s \n", fileIn);
                    fflush(stdout);
                    continue;
                }

                dup2(outputFileDescriptor, 1);
            }
            
            //Execute the command, including searching the path variable
            execvp(array[0], array);
            
            printf("%s: no such file or directory \n", array[0]);
            fflush(stdout);
            exit(EXIT_FAILURE);
        }
        else
        {
            //parent!!
            if(spawn)
            {
                //print prompt and flush
                printf("background pid is %d\n", spawnId);
                

                fflush(stdout);
            }
            else
            {
                // process is in background
                spawnId = waitpid(spawnId, &status, 0);
                
            } 
        } 
    }

    exit(EXIT_SUCCESS);
}


//signal handlers
void stopSignal(int sigNumber)
{
    printf("terminated by signal  %d\n", sigNumber);
    signal(SIGTSTP, stopSignal);
}

void interruptSignal(int sigNumber)
{
    printf("interrupted by signal %d\n", sigNumber);
    signal(SIGINT, interruptSignal);
}


// run in background check
//returns true or false
bool runBackground(char** array, int* count)
{
    if(strcmp(array[*count-1], "&") == 0)
    {
        *count -= 1;
        return true;
    }
    else
    {
        return false;
    }
}


// if out redirect occurs, copy the name and decrement count
// returns true or false
bool runOutRedirect(char** array, int* count, char* fileOut)
{
    memset(fileOut, '\0', MAX_INPUT);
    
    int i;
    for (i = 0 ; i < *count ; i++)
    {
        if (strcmp(array[i], ">") == 0)
        {
            strcpy(fileOut, array[i + 1]);
            *count -= 2;
            return true;
        }
    }
    return false;
}

// if a redirect from input occurs copy the the name and decrement count
// returns true or false
bool runInputRedirect(char** array, int* count, char* fileIn)
{
    memset(fileIn, '\0', MAX_INPUT);
    int i;
    for(i = 0 ; i < *count ; i++)
    {
        if(strcmp(array[i], "<") == 0)
        {
            strcpy(fileIn, array[i + 1]);
            *count -= 2;
            return true;
        }
    }
    return false;
}

// verifies whether there is input on the string or not
// returns true or false
bool noInputStr(char* input)
{

    int length = strlen(input);
    char current_char = '\0';
    int i;
    for(i = 0 ; i < length ; i++)
    {
        current_char = input[i];
        if((current_char != ' ') && (current_char != '\0') && (current_char != '\n'))
        {
            return false;
        }
    }
    return true;
}

// loop through chil processes and print
// return nothing
void checkProcesses(int* status) 
{

    int pid             = 0;
    int childrenCount   = 0;
    //print child processes 
    while(childrenCount < 100000)
    {
        pid = waitpid(-1, status, WNOHANG);
        
        if(pid > 0)
        {
            if(WIFEXITED(*status))
            {
                printf("process %d exited with exit value %d", pid, \
                        WEXITSTATUS(*status));
            }
            
            if(WIFSIGNALED(*status))
            {
                printf("process %d terminated by signal %d", pid, \
                        WTERMSIG(*status));
            }
            
            printf("\n");
            fflush(stdout);
        }
        childrenCount++;
    } 

}




