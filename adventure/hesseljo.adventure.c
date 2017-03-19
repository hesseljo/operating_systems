/*
Name:			    Joshua Hesseltine
Assignment:		    cs344 Program 2
Filename:			hesseljo.adventure
Description:	    plays the game and save time to file in working directory

Online resources used:

http://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
https://users.cs.cf.ac.uk/Dave.Marshall/C/node20.html
http://stackoverflow.com/questions/11291154/save-file-listing-into-array-or-something-else-c
http://www.sanfoundry.com/c-programming-examples-file-handling/
http://stackoverflow.com/questions/20157648/fopen-with-integer-in-file-name
http://stackoverflow.com/questions/19117131/get-list-of-file-names-and-store-them-in-array-on-linux-using-c
http://stackoverflow.com/questions/10446526/get-last-modified-time-of-file-in-linux
http://stackoverflow.com/questions/13542345/how-to-convert-st-mtime-which-get-from-stat-function-to-string-or-char
https://www.tutorialspoint.com/c_standard_library/c_function_strstr.htm
http://stackoverflow.com/questions/3138600/correct-use-of-stat-on-c
http://pubs.opengroup.org/onlinepubs/009695399/functions/pthread_mutex_timedlock.html
http://stackoverflow.com/questions/14888027/mutex-lock-threads
http://stackoverflow.com/questions/10161425/posix-c-threads-mutex-example-dont-work-as-expected
*/


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h> 

char* ROOM_FILES[] = {
    "room1",
    "room2",
    "room3",
    "room4",
    "room5",
    "room6",
    "room7"
};

char WORKING_DIRECTORY[255];
char NEW_WORKING_DIRECTORY[255];
char* BEGIN_HERE_FILENAME;
char MAZE_LOG[255][15];
unsigned int COUNT = 0;
int MAX_SO_FAR = -1;
bool GAME_OVER = false;
bool CORRECT_INPUT = false;
bool PRINT_ME = true;
bool SET_TIME = false;
char* TIME_FILENAME = "currentTime.txt";
FILE *FILE_POINTER;
FILE *FILE_POINTER_TIME;
FILE *openFile(char *filename, char *node);
pthread_t TIME_ID_THREAD;
pthread_mutex_t TIME_MUTEX;
void printRoom(void);
void printEnd(void);
void getRoomName(char* roomFilename, char* roomNameBuffer);
int getIndex(char* roomName);
bool checkEnd(char* roomFilename);
void printTime(void);
void* getTime(void *arguments);
void getFileCreationTime(char* path);
void getNewWorkingDirectory(void);

int main(int argc, char** argv)
{
    char inputBuffer[255];
    getNewWorkingDirectory();
    assert(pthread_mutex_init(&TIME_MUTEX, NULL) == 0);
    BEGIN_HERE_FILENAME = ROOM_FILES[0];

    while(!GAME_OVER)
    {
        // print function call that displays current room
        printRoom();
        
        while(!CORRECT_INPUT)
        {
                memset(inputBuffer, '\0', 255);
                printf("\nWHERE TO? >");
                fgets(inputBuffer, 255, stdin);
                inputBuffer[strlen(inputBuffer)-1] = '\0';
                
                int rIndex = getIndex(inputBuffer);
                if(rIndex != -1) //change rooms and increment
                {
                    //set input room and increment count
                    strcpy(MAZE_LOG[COUNT], inputBuffer);
                    COUNT++;
                    BEGIN_HERE_FILENAME = ROOM_FILES[rIndex];
                    CORRECT_INPUT = true;
                    
                }
                else if(strcmp(inputBuffer, "time") == 0) // check if user inputted "time"
                {
                    printTime(); 
                }
                else // check if we are in the end room
                {
                printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
                CORRECT_INPUT = true;
                }    
                GAME_OVER = checkEnd(BEGIN_HERE_FILENAME);  
        }
        CORRECT_INPUT = false;
    }   
printEnd();
return (EXIT_SUCCESS);
}

void getNewWorkingDirectory(void)
{
const char needle[15] = "hesseljo.rooms."; // string to look for in strstr function
struct stat attr; //struct for stat()
char saveName[255]; //copies directory with largest st_mtime

    DIR           *d;
    struct dirent *dir;
    d = opendir(".");
    if (d) 
    {
        while ((dir = readdir(d)) != NULL)  
        {
            if (strstr(dir->d_name, needle) != NULL) 
            {
                stat(dir->d_name, &attr);
                    int t = attr.st_mtime;
                    if (t > MAX_SO_FAR) 
                    {
                        MAX_SO_FAR = t;
                        strcpy(saveName, dir->d_name);
                        memset(NEW_WORKING_DIRECTORY, '\0', 255);
                strcat(NEW_WORKING_DIRECTORY, saveName);
                }
            }
        } 
    }   		
closedir(d);
}

void printRoom(void)
{
    char line[255];
    char* token;
    memset(line, '\0', 255);

    FILE_POINTER = openFile(BEGIN_HERE_FILENAME, "r");
    
    if(PRINT_ME)
    {
      printf("CURRENT LOCATION: ");
      PRINT_ME = false;
    }
    else
    {
       printf("\nCURRENT LOCATION: "); 
    }
    
    // remove and print room name
    fgets(line, 255, FILE_POINTER);
    token = strtok(line, ":");
    token = strtok(NULL, " \n");
    
    bool one = true;
    while(token != NULL)
    {
            if(one)
            {
              one = false;
            }
            else
            {
              printf(" ");  
            }
           printf("%s", token);
           token = strtok(NULL, " \n");
    }
    printf("\nPOSSIBLE CONNECTIONS: ");
    
    // now loop through the connections and print each name
    bool printC = true;
    bool printOneC = false;
    while (printC)
    {
        fgets(line, 255, FILE_POINTER);
        token = strtok(line, " ");
        
        if (strcmp(token, "CONNECTION") != 0)
        {
            printC = false;
            break;
        }
        
        if (printOneC)
        {
            printf(", ");
        }
        else
        {
            printOneC = true;
        }
        
        token = strtok(NULL, ":");
        token = strtok(NULL, " \n");
        
        bool one = true;
        while (token != NULL)
        {
                if (one)
                {
                    one = false;
                }
                else
                {
                  printf(" ");  
                }

               printf("%s", token);
               token = strtok(NULL, " \n");
        }
    }
    printf(".");
    fclose(FILE_POINTER);
}

void printEnd(void)
{
    printf("\nYOU HAVE FOUND THE END ROOM. CONGRATULATIONS!");
    printf("\nYOU TOOK %u STEPS. YOUR PATH TO VICTORY WAS:", COUNT);
    
    // print the maze steps in order
    int i;
    for (i = 0 ; i < COUNT ; i++)
    {
        printf("\n%s", MAZE_LOG[i]);
    }
    printf("\n");
}

void printTime(void)
{    
    //create thread
    assert(pthread_create(&TIME_ID_THREAD, NULL, getTime, NULL) == 0);
    assert(pthread_join(TIME_ID_THREAD, NULL) == 0);

    // set and set time written to false
    pthread_mutex_lock(&TIME_MUTEX);
    bool atRuntime = SET_TIME;
    SET_TIME = false;
    pthread_mutex_unlock(&TIME_MUTEX);
    
    //you have to assert that the time was written
    assert(atRuntime);
    
    //now open file and print it
    FILE_POINTER = openFile(TIME_FILENAME, "r");
    assert(FILE_POINTER != NULL);
    
    printf("\n");
    char c = fgetc(FILE_POINTER);
    while(c != EOF)
    {
        printf("%c", c);
        c = fgetc(FILE_POINTER);
    }

    printf("\n");
    fclose(FILE_POINTER);
    
}

void* getTime(void *arguments)
{
    time_t beforeTransformOfTime;
    struct tm *timeS;
    char buffer_size = 50;
    char buffer[buffer_size];
    memset(buffer, '\0', buffer_size);
    
    // get the current time from OS
    time(&beforeTransformOfTime);
    timeS = localtime(&beforeTransformOfTime);
    
    //format time 
    strftime(buffer, buffer_size, "%-l:%M%P, %A, %B %-e, %Y", timeS);
    
    //save to file
    FILE_POINTER_TIME = openFile(TIME_FILENAME, "w");
    fprintf(FILE_POINTER_TIME, "%s", buffer);
    fclose(FILE_POINTER_TIME);
    
    // lock and unlock the mutex
    pthread_mutex_lock(&TIME_MUTEX);
    SET_TIME = true;
    pthread_mutex_unlock(&TIME_MUTEX);
    return NULL;
}

// simple strcat function for file in working directory
FILE *openFile(char *filename, char *node)
{
    char fileDir[255];
    memset(fileDir, '\0', 255);
    strcat(fileDir, NEW_WORKING_DIRECTORY);
    strcat(fileDir, "/");
    strcat(fileDir, filename);
    return fopen(fileDir, node);
}

int getIndex(char* roomName)
{
    char num_rooms = sizeof(ROOM_FILES) / sizeof(ROOM_FILES[0]);
    char roomNameBuffer[255];
    
    //find the room name and get it's index
    int i;
    for(i = 0 ; i < num_rooms ; i++)
    {
        memset(roomNameBuffer, '\0', 255);
        getRoomName(ROOM_FILES[i], roomNameBuffer);
        
        if(strcmp(roomNameBuffer, roomName) == 0)
        {
           return i; 
        }
    }
    return -1;
}

void getRoomName(char* roomFilename, char* roomNameBuffer)
{
    char line[255];
    char* token;
    memset(line, '\0', 255);
    FILE_POINTER = openFile(roomFilename, "r");
    
    // return if not a room name
    if(FILE_POINTER == NULL)
    {
        strcpy(roomNameBuffer, " ");
        return;
    }
    
    // get room name in file and hanle the whitespace
    while(fgets(line, 255, FILE_POINTER) != NULL)
    {
        token = strtok(line, ":");
        bool one = true;
        
        if(strcmp("ROOM NAME", token) == 0)
        {
            token = strtok(NULL, " \n");
            while(token != NULL)
            {
                if(one)
                {
                    one = false;
                }
                else
                {
                  strcat(roomNameBuffer, " ");  
                }
                
               strcat(roomNameBuffer, token);
               token = strtok(NULL, " \n");
            }
            break;
        }
    }
    
    fclose(FILE_POINTER);
}

bool checkEnd(char* roomFilename){
    char line[255];
    char* token;
    memset(line, '\0', 255);
    FILE_POINTER = openFile(roomFilename, "r");
    
    //find type and check if it's the end
    while(fgets(line, 255, FILE_POINTER) != NULL)
    {
        token = strtok(line, ":");
        if(strcmp("ROOM TYPE", token) == 0)
        {
            token = strtok(NULL, " \n");
            if(strcmp(token, "END_ROOM") == 0)
            {
                fclose(FILE_POINTER);  
                return true;
            }
        }
    }
    fclose(FILE_POINTER);
    return false;
}



