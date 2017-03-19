// Name:        Joshua Hesseltine
// Course:      CS344 Program4
// Filename:    otp_dec_d.c
// Description: 
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

int getNumber(char aLetter);
char getCharacter(int aNumber);
char setCharacter(char aLetter, char aKey);

int main(int argc, char** argv) 
{
    int socketFD, connectSocketFD, portNumber;   
    socklen_t clientSocketLength;
    struct sockaddr_in serverAddress;
    struct sockaddr_in clientAddress;

    socketFD = socket(AF_INET, SOCK_STREAM, 0);//create socket
    portNumber = atoi(argv[1]);//set port

    memset((char *)&serverAddress, '\0', sizeof(serverAddress));
    serverAddress.sin_port = htons(portNumber);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_family = AF_INET;
    bind(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)); //bind the server from listening 

    
    while(1)
    {
        static int pStatus;
        waitpid(-1, &pStatus, WNOHANG); //kill those zombies!
        listen(socketFD, 5); //listen up to 5 concurrent connections

        socklen_t clientSocketLength = sizeof(clientAddress);
        connectSocketFD = accept(socketFD, (struct sockaddr *) &clientAddress, &clientSocketLength); //accept the connection
        
        if (connectSocketFD > 0) // we have a connection!
        {
            pid_t spawnPID;
            spawnPID = fork(); // fork it!
            
            if (spawnPID == 0) //child process
            {
                int readValue, writeValue;
                unsigned int myIndex = 0;
                
                char rBuf[10];
                char* copyBuf = malloc(sizeof(char) * 1000000);
                memset(rBuf, '\0', 10);
                memset(copyBuf, '\0', sizeof(char) * 1000000);
                
                // figure out who we are talking to 
                while(strstr(copyBuf, "~~") == NULL)
                {
                    readValue = read(connectSocketFD, rBuf, 1);
                    if(readValue != -1)
                    {
                        copyBuf[myIndex] = rBuf[0];
                        myIndex++;
                    }
                }
                
                if(strstr(copyBuf, "$" "~~") != NULL)
                {
                  
                    writeValue = write(connectSocketFD, "&" "~~", 3);
                    //clear 
                    myIndex = 0;
                    memset(rBuf, '\0', 10);
                    memset(copyBuf, '\0', sizeof(char) * 1000000);
                
                    //read 
                    while(strstr(copyBuf, "~~") == NULL)
                    {
                        readValue = read(connectSocketFD, rBuf, 1);
                        if(readValue != -1)
                        {
                            copyBuf[myIndex] = rBuf[0];
                            myIndex++;
                        }
                    }
                    
                    //set markers to null
                    int input_string_length = strlen(copyBuf);
                    copyBuf[input_string_length-1] = '\0';
                    copyBuf[input_string_length-2] = '\0';
                    
                    //save input text
                    char* input_text = malloc(sizeof(char) *(strlen(copyBuf) + 1));
                    memset(input_text, '\0', sizeof(char) * (strlen(copyBuf) + 1));
                    strcpy(input_text, copyBuf);
                    
                    //clear
                    myIndex = 0;
                    memset(rBuf, '\0', 10);
                    memset(copyBuf, '\0', sizeof(char) * 1000000);
                    
                    // read the key file
                    while(strstr(copyBuf, "~~") == NULL)
                    {
                        readValue = read(connectSocketFD, rBuf, 1);
                        if(readValue != -1)
                        {
                            copyBuf[myIndex] = rBuf[0];
                            myIndex++;
                        }
                    }
                    
                    //set my code to null
                    input_string_length = strlen(copyBuf);
                    copyBuf[input_string_length-1] = '\0';
                    copyBuf[input_string_length-2] = '\0';
                    
                    //save key file 
                    char* key_text = malloc(sizeof(char) * (strlen(copyBuf) + 1));
                    memset(key_text, '\0', sizeof(char) * (strlen(copyBuf) + 1));
                    strcpy(key_text, copyBuf);
                    free(copyBuf);
                    
                    //write the decrypted data out
                    for (unsigned long int i = 0 ; i < strlen(input_text); i++)
                    {
                        char tempCharacter = setCharacter(input_text[i], key_text[i]);
                        writeValue = write(connectSocketFD, &tempCharacter, 1);
                    }
                    
                    //write my code 
                    writeValue = write(connectSocketFD, "~~", 2);
                    
                    
                }
                else if(strstr(copyBuf, "#" "~~") != NULL)
                {
                    //something went wrong
                    writeValue = write(connectSocketFD, "%" "~~", 3);
                    exit(1);
                }
                else
                {
                    fprintf(stderr, "Error: bad characters\n");
                    writeValue = write(connectSocketFD, "%" "~~", 3);
                    exit(1);
                }
                
                exit(0); //child process has to die
            }
            
        }
    }

    close(socketFD);
    exit(0);
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
    for (int i = 0 ; i < letters ; i++)
    {
        if(aNumber == lettersArray[i][1])
        {
            return lettersArray[i][0];
        }
    }
    return -1;
}

//helper function to set the characters
char setCharacter(char aLetter, char aKey)
{
    int mappedToLetter  = getNumber(aLetter);
    int mappedToKey     = getNumber(aKey);
    int minus           = (mappedToLetter - mappedToKey);
    
    if(minus < 0)
    {
        minus += letters;
    }
    
    return getCharacter(minus);
}