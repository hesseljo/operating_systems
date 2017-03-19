// Name:        Joshua Hesseltine
// Course:      CS344 Program4
// Filename:    otp_enc_d.c
// Description: program will run in the background as a daemon
// Sources:  
// https://github.com/jirihnidek/daemon
// http://www.thegeekstuff.com/2012/02/c-daemon-process
// https://www.tutorialspoint.com/c_standard_library/c_function_memset.htm
// http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
// https://linux.die.net/man/3/fgets
// https://paulschreiber.com/blog/2005/10/28/simple-gethostbyname-example/
// http://fresh2refresh.com/c-programming/c-strings/c-strstr-function/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define letters 27
int lettersArray[letters][2] = {'A', 10,'B', 11,'C', 12,'D', 13,'E', 14,'F', 15,'G', 16,'H', 17,'I', 18,'J', 19,
    'K', 20,'L', 21,'M', 22,'N', 23,'O', 24,'P', 25,'Q', 26,'R', 0,'S', 1,'T', 2,'U', 3,'V', 4,'W', 5,'X', 6,'Y', 7,'Z', 8,
    ' ', 9
};

char setCharacter(char aLetter, char aKey);
int getNumber(char aLetter);
char getCharacter(int aNumber);


int main(int argc, char** argv) 
{
    int socketFD, connectSocketFD, portNumber;
    socklen_t clientSocketLength;
    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;
    
    socketFD = socket(AF_INET, SOCK_STREAM, 0); //create socket
    portNumber = atoi(argv[1]); //set port number
    memset((char *)&serverAddress, '\0', sizeof(serverAddress));
    serverAddress.sin_port = htons(portNumber); //listen
    serverAddress.sin_addr.s_addr = INADDR_ANY; //listen
    serverAddress.sin_family = AF_INET;//listen
    bind(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)); //bind settings
    
    while(1) 
    {
        static int pStatus;
        waitpid(-1, &pStatus, WNOHANG); //handle those zombies
        listen(socketFD, 5); //listen and allow up to 5 concurrent connections

        socklen_t clientSocketLength = sizeof(clientAddress);
        connectSocketFD = accept(socketFD, (struct sockaddr *) &clientAddress, &clientSocketLength);
        
        if (connectSocketFD > 0) //we have a connection!
        {
            pid_t spawnPID;
            spawnPID = fork(); //fork a new process
            
            if(spawnPID == 0) //if we have a child
            {
                
                int readValue;
                int writeValue;
                unsigned int myIndex = 0;
                
                char rBuf[10];
                char* copyBuf = malloc(sizeof(char) * 1000000);
                memset(rBuf, '\0', 10);
                memset(copyBuf, '\0', sizeof(char) * 1000000);
                
                // whois the client?
                while(strstr(copyBuf, "~~") == NULL)
                {
                    readValue = read(connectSocketFD, rBuf, 1);
                    if(readValue != -1)
                    {
                        copyBuf[myIndex] = rBuf[0];
                        myIndex++;
                    }
                }
                
                //ok we now know who the client is, choose the correct response
                if (strstr(copyBuf, "#" "~~") != NULL)
                {
 
                    writeValue = write(connectSocketFD, "&" "~~", 3); //continue
                    myIndex = 0;
                    memset(rBuf, '\0', 10);
                    memset(copyBuf, '\0', sizeof(char) * 1000000);
                
                    while (strstr(copyBuf, "~~") == NULL)
                    {
                        readValue = read(connectSocketFD, rBuf, 1);
                        if (readValue != -1)
                        {
                            copyBuf[myIndex] = rBuf[0];
                            myIndex++;
                        }
                    }
                    
                    int fileTextLength = strlen(copyBuf);
                    copyBuf[fileTextLength-1] = '\0';
                    copyBuf[fileTextLength-2] = '\0';

                    char* response = malloc(sizeof(char) * (strlen(copyBuf) + 1));
                    memset(response, '\0', sizeof(char) * (strlen(copyBuf) + 1));
                    strcpy(response, copyBuf);
                    myIndex = 0;
                    memset(rBuf, '\0', 10);
                    memset(copyBuf, '\0', sizeof(char) * 1000000);
                    
                    //read the key file
                    while(strstr(copyBuf, "~~") == NULL)
                    {
                        readValue = read(connectSocketFD, rBuf, 1);
                        if(readValue != -1)
                        {
                            copyBuf[myIndex] = rBuf[0];
                            myIndex++;
                        }
                    }
                    
                    fileTextLength = strlen(copyBuf);
                    copyBuf[fileTextLength-1] = '\0';
                    copyBuf[fileTextLength-2] = '\0';
                    char* keyResponse = malloc(sizeof(char) * (strlen(copyBuf) + 1));
                    memset(keyResponse, '\0', sizeof(char) * (strlen(copyBuf) + 1));
                    strcpy(keyResponse, copyBuf);
                    free(copyBuf);
                    
                    //write encryption data
                    for(unsigned long int i = 0 ; i < strlen(response); i++)
                    {
                        char tempCharacter = setCharacter(response[i], keyResponse[i]);
                        writeValue = write(connectSocketFD, &tempCharacter, 1);
                    }
                    writeValue = write(connectSocketFD, "~~", 2); //end value

                }
                else if (strstr(copyBuf, "$" "~~") != NULL)
                {
                    writeValue = write(connectSocketFD, "%" "~~", 3);
                    exit(1);
                }
                else
                {
                    //something went wrong
                    writeValue = write(connectSocketFD, "%" "~~", 3);
                    exit(1);
                }
                exit(0); //child process must end
            }
            
        }
    }

    close(socketFD);
    exit(0);
}

//helper function to set the characters
char setCharacter(char aLetter, char aKey)
{
    int mappedToLetter  = getNumber(aLetter);
    int mappedToKey     = getNumber(aKey);
    int addThem         = (mappedToLetter + mappedToKey);
    
    if(addThem >= letters)
    {
        addThem -= letters;
    }
    
    return getCharacter(addThem);
}

//helper function to get the number at index
int getNumber(char aLetter)
{
    for(int i = 0 ; i < letters ; i++)
    {
        if(aLetter == lettersArray[i][0])
        {
            return lettersArray[i][1];
        }
    }
    return -1;
}

//helper function to get the char at index
char getCharacter(int aNumber)
{
    for(int i = 0 ; i < letters ; i++)
    {
        if(aNumber == lettersArray[i][1])
        {
            return lettersArray[i][0];
        }
    }
    return -1;
}



