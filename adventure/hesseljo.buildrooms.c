/*
Name:			Joshua Hesseltine
Assignment:		cs344 Program 2
Filename:		hesseljo.buildrooms
Description:	generates 7 different room files, one room per file, in a directory called 
                "hesseljo.rooms.<PROCESS ID>" Each room has a Room Name, at least 3 outgoing connections and 
                at most 6 outgoing connections, where the number of outgoing connections is random from a
                room to other rooms, and a room type. The connections from one room to the others are randomly assigned. 

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
http://www.ibm.com/support/knowledgecenter/ssw_i5_54/ifs/rzaaxcfunctionexample.htm
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>
#include <time.h>

char* ROOM_FILES[] = {
    "room1",
    "room2",
    "room3",
    "room4",
    "room5",
    "room6",
    "room7"
};

char *MY_ROOM_NAMES[10] = {
    "groot",
    "starlord",
    "drax",
    "gamora",
    "thanos",
    "rocket",
    "ronan",
    "nebula",
    "yondu",
    "collector"
};

char* DIRECTORY_KEY = "hesseljo.rooms.";
char WORKING_DIRECTORY[255];
FILE *FILE_POINTER;
FILE *openFile(char *filename, char *mode);
void createRoom(char *roomFilename, char *roomName, char* roomType);
void createConnection(char *roomFilename, char *addRoomName);
unsigned int getConnection(char *roomFilename);
int returnIndex(char* roomFilename, int* list);
bool checkConnectionToRoom(char* roomName);
void getRoomName(char* roomFilename, char* roomNameBuffer);
int getIndex(char* roomName);

int main() 
{
    bool start = false;
    bool end = false;
	time_t t; 
    char pid_buffer[20];
    memset(pid_buffer, '\0', 20);
    memset(WORKING_DIRECTORY, '\0', 255);
    sprintf(pid_buffer, "%d", getpid());
    strcat(WORKING_DIRECTORY, DIRECTORY_KEY);
    strcat(WORKING_DIRECTORY, pid_buffer);
    mkdir(WORKING_DIRECTORY, 0770);
    srand((unsigned) time(&t)); // seeds to ensure random rooms every run
    
    char rooms = sizeof(ROOM_FILES) / sizeof(ROOM_FILES[0]);

    int i;
    for (i = 0 ; i < rooms ; i++)
    {
        int randomNumber = rand() % 10;
        char* currentRoom = MY_ROOM_NAMES[randomNumber];
        
        //if current room is used, create a new room
        while(checkConnectionToRoom(currentRoom))
        {
           randomNumber = rand() % 10;
           currentRoom = MY_ROOM_NAMES[randomNumber];
        }
        
        // first if create the start room
        // second if  create the end room
        // else create all the mid rooms
        if(!start)
        {
            FILE_POINTER = openFile(ROOM_FILES[i], "w");
            assert(FILE_POINTER != NULL);
            fputs("ROOM NAME: ", FILE_POINTER);
            fputs(currentRoom, FILE_POINTER);
            fputs("\n", FILE_POINTER);
            fputs("ROOM TYPE: ", FILE_POINTER);
            fputs("START_ROOM", FILE_POINTER);
            fclose(FILE_POINTER);
            start = true;
        }
        else if(!end)
        {
            FILE_POINTER = openFile(ROOM_FILES[i], "w");
            assert(FILE_POINTER != NULL);
            fputs("ROOM NAME: ", FILE_POINTER);
            fputs(currentRoom, FILE_POINTER);
            fputs("\n", FILE_POINTER);
            fputs("ROOM TYPE: ", FILE_POINTER);
            fputs("END_ROOM", FILE_POINTER);
            fclose(FILE_POINTER);
            end = true;
        }
        else
        {
            FILE_POINTER = openFile(ROOM_FILES[i], "w");
            assert(FILE_POINTER != NULL);
            fputs("ROOM NAME: ", FILE_POINTER);
            fputs(currentRoom, FILE_POINTER);
            fputs("\n", FILE_POINTER);
            fputs("ROOM TYPE: ", FILE_POINTER);
            fputs("MID_ROOM", FILE_POINTER);
            fclose(FILE_POINTER);
        }
    }
    
    // now create the connections for each room
    for(i = 0 ; i < rooms ; i++)
    {
        char buffer[255];
        int usedRooms[6];
        
        //make already connected rooms unusable
        int l;
        for (l = 0 ; l < 6 ; l++)
        {
            usedRooms[l] = -1;
        }
        
        // + 3 is minimum number of connectiosns between rooms
        int numberOfTotalConnections = (rand() % 4) + 3 - \
            getConnection(ROOM_FILES[i]);
        
        //create unique connections for each room
        int j;
        for (j = 0 ; j < numberOfTotalConnections ; j++)
        {
            int addThisRoom;
            bool unused_found = false;
            
            //create new connections
            while(!unused_found)
            {
                addThisRoom = rand() % 7;
                
                // don't add connections to the same room
                if(addThisRoom == i)
                {
                    continue;
                }
                
								// check if connection exists
                bool already_exists = false;
                returnIndex(ROOM_FILES[i], usedRooms);                
                
                int k;
                for(k = 0 ; k < 6 ; k++)
                {
                    if(usedRooms[k] == addThisRoom)
                    {
                        already_exists = true;
                        break;
                    }
                }
                
                // maximum number of connections is 6
                if(getConnection(\
                        ROOM_FILES[addThisRoom]) == 6)
                {
                    already_exists = true;
                }
                
                //set to exit
                if(!already_exists)
                {
                    unused_found = true;   
                }                
            }
            
            // add forward connection
            memset(buffer, '\0', 255);
            getRoomName(ROOM_FILES[addThisRoom], buffer);
            createConnection(ROOM_FILES[i], buffer);
            
            //add backwards connection
            memset(buffer, '\0', 255);
            getRoomName(ROOM_FILES[i], buffer);
            createConnection(ROOM_FILES[addThisRoom], buffer);
        }
    }
    return 0;
}

// find the new connection
// interate over room name
// save the connection, and create it between rooms
void createConnection(char *roomFilename, char *addRoomName)
{
    char stringBuffer[255];
    char roomTypeBuffer[255];
    unsigned int newline_count = 0;
    unsigned int injectLine = 0;
    long insertion_position = 0;
    memset(stringBuffer, '\0', 255);
    memset(roomTypeBuffer, '\0', 255);

    injectLine = getConnection(roomFilename);
    injectLine++; 
    
    FILE_POINTER = openFile(roomFilename, "r+");
    
    while(newline_count != injectLine)
    {
        if(fgetc(FILE_POINTER) == '\n')
        {
            newline_count++;
        }
    }
    
    //save positions and backup
    insertion_position = ftell(FILE_POINTER);
    fgets(roomTypeBuffer, 255, FILE_POINTER);
    fseek(FILE_POINTER, insertion_position, SEEK_SET);
    
    // save new connection
    sprintf(stringBuffer, "CONNECTION %u: %s\n", injectLine, \
            addRoomName);
    fputs(stringBuffer, FILE_POINTER);
    fputs(roomTypeBuffer, FILE_POINTER);
    fclose(FILE_POINTER);  
}

//count the number of connections
unsigned int getConnection(char *roomFilename)
{
    char myLine[255];
    char* token;
    unsigned int roomsCount = 0;
    
    memset(myLine, '\0', 255);
    
    FILE_POINTER = openFile(roomFilename, "r");
    
    while(fgets(myLine, 255, FILE_POINTER) != NULL)
    {
        token = strtok(myLine, " ");
        
        if(strcmp("CONNECTION", token) == 0)
        {
            roomsCount++;
        }
    }
    fclose(FILE_POINTER);
    return roomsCount;
}

int returnIndex(char* roomFilename, int* list)
{
    char myLine[255];
    char usedNamesArray[6][15];
    char* token;
    int index = 0;
    
    memset(myLine, '\0', 255);
    
    FILE_POINTER = openFile(roomFilename, "r");
    
    //get all the connections inside this file
    while(fgets(myLine, 255, FILE_POINTER) != NULL)
    {
        token = strtok(myLine, " ");
        
        if(strcmp("CONNECTION", token) == 0)
        {
          token = strtok(NULL, ":");
          token = strtok(NULL, " \n");
          bool first_entry = true;
          
          while(token != NULL)
          {
              if(first_entry)
              {
                  first_entry = false;
              }
              else
              {
               strcat(usedNamesArray[index], " "); 
              }

             strcat(usedNamesArray[index], token);
             token = strtok(NULL, " \n");
          }
          index++; //increment index
        }
    }
    
    fclose(FILE_POINTER);
    
    //loop through names and return index
    int i;
    for(i = 0 ; i < index ; i++)
    {
        list[i] = getIndex(usedNamesArray[i]);
    }
    
    return index;
}

//loop through file type and check to see if it's been used already
bool checkConnectionToRoom(char* roomName)
{
    char rooms = sizeof(ROOM_FILES) / sizeof(ROOM_FILES[0]);
    char roomNameBuffer[255];
    
    int i;
    for(i = 0 ; i < rooms ; i++)
    {
        memset(roomNameBuffer, '\0', 255);
        getRoomName(ROOM_FILES[i], roomNameBuffer);

        if(strcmp(roomNameBuffer, roomName) == 0)
        {
            return true;
        }
    }
    return false;
}

void getRoomName(char* roomFilename, char* roomNameBuffer)
{
    char myLine[255];
    char* token;
    
    memset(myLine, '\0', 255);
    FILE_POINTER = openFile(roomFilename, "r");

    //if file is gone return 
    if(FILE_POINTER == NULL)
    {
      strcpy(roomNameBuffer, "__NOT_A_ROOM_NAME__");
      return;
    }
    
    //find the room name and handle extra spaces
    while(fgets(myLine, 255, FILE_POINTER) != NULL)
    {
        token = strtok(myLine, ":");
        bool first_entry = true;
        
        if(strcmp("ROOM NAME", token) == 0)
        {
            token = strtok(NULL, " \n");
            while(token != NULL)
            {
              if(first_entry)
              {
                  first_entry = false;
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

int getIndex(char* roomName)
{
    char rooms = sizeof(ROOM_FILES) / sizeof(ROOM_FILES[0]);
    char roomNameBuffer[255];
    
    // loop till room name is found and return it's index
    int i;
    for(i = 0 ; i < rooms ; i++)
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

// simplified strcat function for file in working directory
FILE *openFile(char *filename, char *mode)
{
    char fileDir[255];
    memset(fileDir, '\0', 255);
    strcat(fileDir, WORKING_DIRECTORY);
    strcat(fileDir, "/");
    strcat(fileDir, filename);
    return fopen(fileDir, mode);
}
