// Name:        Joshua Hesseltine
// Course:      CS344 Program4
// Filename:    otp_enc.c
// Description: program connects to otp_enc_d, and asks it to perform a one-time pad style encryption
// Sources:  
// https://www.tutorialspoint.com/c_standard_library/c_function_memset.htm
// http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
// https://linux.die.net/man/3/fgets
// https://paulschreiber.com/blog/2005/10/28/simple-gethostbyname-example/
// http://fresh2refresh.com/c-programming/c-strings/c-strstr-function/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define letters 27
int lettersArray[letters][2] = {'A', 10,'B', 11,'C', 12,'D', 13,'E', 14,'F', 15,'G', 16,'H', 17,'I', 18,'J', 19,
    'K', 20,'L', 21,'M', 22,'N', 23,'O', 24,'P', 25,'Q', 26,'R', 0,'S', 1,'T', 2,'U', 3,'V', 4,'W', 5,'X', 6,'Y', 7,'Z', 8,
    ' ', 9
};
 
int getNumber(char aLetter);

int main(int argc, char** argv) 
{
    FILE *inputFile;
    FILE *keyFile;
    int socketFD, portNumber, readValue, writeValue;
    unsigned int myIndex;
    struct hostent *serverHostInfo;
    struct sockaddr_in serverAddress;

    portNumber = atoi(argv[3]);

    //open files
    inputFile = fopen(argv[1], "r");
    keyFile = fopen(argv[2], "r");
    
    //read inputFile
    char* buf = malloc(sizeof(char) * 1000000);
    memset(buf, '\0', sizeof(char) * 1000000);
    fgets(buf, 1000000, inputFile);

    //copy text in inputFile
    char* fileText = malloc(sizeof(char) * (strlen(buf) + 1));
    memset(fileText, '\0', sizeof(char) * (strlen(buf) + 1));
    strcpy(fileText, buf);
    
    int fileTextLength = strlen(fileText);    
    if(fileText[fileTextLength - 1] == '\n'){
       fileText[fileTextLength - 1] = '\0';
    }
    
    //read keyFile
    memset(buf, '\0', sizeof(char) * 1000000);
    fgets(buf, 1000000, keyFile);
    
    char* keyFileText = malloc(sizeof(char) * (strlen(buf) + 1));
    // copy text
    memset(keyFileText, '\0', sizeof(char) * (strlen(buf) + 1));
    strcpy(keyFileText, buf);
    
    fileTextLength = strlen(keyFileText);    
    if (keyFileText[fileTextLength - 1] == '\n')
    {
       keyFileText[fileTextLength - 1] = '\0';
    }
    free(buf); //clear buffer
    
    // exit if key file is too short
    if(strlen(fileText) > strlen(keyFileText))
    {
        fprintf(stderr, "Error: key file too short\n");
        exit(1);
    }
    
    //exit if bad characters exist in input file
    unsigned long i;
    for (i = 0 ; i < strlen(fileText) ; i ++)
    {
        if (getNumber(fileText[i]) == -1)
        {
            fprintf(stderr, "Error: bad characters\n");
            exit(1);
        }
    } //end for
        
    //exit if bad characters exist in key file
    unsigned long j;
    for (j = 0 ; j < strlen(keyFileText) ; j ++)
    {
        if (getNumber(keyFileText[j]) == -1)
        {
            fprintf(stderr, "Error: bad characters in key file\n");
            exit(1);
        }
    } //end for
    
    socketFD = socket(AF_INET, SOCK_STREAM, 0); //create listening socket
    serverHostInfo = gethostbyname("localhost"); //set serverHost to local host
    memset((char *)&serverAddress, '\0', sizeof(serverAddress));
    serverAddress.sin_port = htons(portNumber);
    memcpy(&serverAddress.sin_addr, serverHostInfo->h_addr_list[0], serverHostInfo->h_length);
    serverAddress.sin_family = AF_INET;
    connect(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)); //create server connection
         
    char rBuf[10];
    char* cBuf = malloc(sizeof(char) * 1000000);
    memset(rBuf, '\0', 1);
    memset(cBuf, '\0', sizeof(char) * 1000000);
    
    // now write message
    writeValue = write(socketFD, "#" "~~", 3);

    //read response message from the server
    while(strstr(cBuf, "~~") == NULL)
    {
        readValue = read(socketFD, rBuf, 1);
        if(readValue != -1)
        {
            cBuf[myIndex] = rBuf[0];
            myIndex++;
        }
    }

    
    if(strstr(cBuf, "&" "~~") != NULL)
    {

        writeValue = write(socketFD, fileText, strlen(fileText)); //write to daemon
        writeValue = write(socketFD, "~~", 2); //write end values
        writeValue = write(socketFD, keyFileText, strlen(keyFileText)); //write to keyfile
        writeValue = write(socketFD, "~~", 2); //write end values again

        myIndex = 0; //reset baseline
        memset(rBuf, '\0', 10);
        memset(cBuf, '\0', sizeof(char) * 1000000);
        //read in the daemon response message
        while(strstr(cBuf, "~~") == NULL)
        {
            readValue = read(socketFD, rBuf, 1);
            if(readValue != -1) //check if we have something
            {
                cBuf[myIndex] = rBuf[0];
                myIndex++;
            }
        }

        int input_string_length = strlen(cBuf);
        cBuf[input_string_length-1] = '\0';
        cBuf[input_string_length-2] = '\0';
    
        for (int i = 0 ; i < strlen(cBuf) ; i++)
        {
            printf("%c", cBuf[i]); //print new buf
        }
        printf("\n"); //print new line
        
    } //end if
    else if (strstr(cBuf, "%" "~~") != NULL) //tried to connect to the wrong daemon
    {
        fprintf(stderr, "Error: otp_enc cannot use otp_dec_d\n");
        exit(1);
    } //end else if
    
    exit(0);
}

int getNumber(char aLetter)
{
    int i;
    for (i = 0 ; i < letters ; i++)
    {
        if (aLetter == lettersArray[i][0])
        {
            return lettersArray[i][1];
        }
    }
    return -1;
}