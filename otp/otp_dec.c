// Name:        Joshua Hesseltine
// Course:      CS344 Program4
// Filename:    otp_dec.c
// Description: program will connect to otp_dec_d and will ask it to decrypt ciphertext using a passed-in ciphertext and key
// Sources:  
// https://github.com/jirihnidek/daemon
// http://www.thegeekstuff.com/2012/02/c-daemon-process
// https://www.tutorialspoint.com/c_standard_library/c_function_memset.htm
// http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
// https://linux.die.net/man/3/fgets
// https://paulschreiber.com/blog/2005/10/28/simple-gethostbyname-example/
// http://fresh2refresh.com/c-programming/c-strings/c-strstr-function/
// http://beginnersbook.com/2014/01/2d-arrays-in-c-example/
// http://codereview.stackexchange.com/questions/41748/small-one-time-pad-encryption-program
// http://stackoverflow.com/questions/33156040/vernam-cipher-code

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
    int socketFD, portNumber, readValues, writeValues;  
    struct hostent *serverHostInfo;
    struct sockaddr_in serverAddress;
    unsigned int myIndex = 0;

    portNumber = atoi(argv[3]); //get port number
    inputFile = fopen(argv[1], "r"); //open file
    keyFile = fopen(argv[2], "r"); // open file
    
    //read input file 
    char* buf = malloc(sizeof(char) * 1000000);
    memset(buf, '\0', sizeof(char) * 1000000);
    fgets(buf, 1000000, inputFile);
    char* textFromInputFile = malloc(sizeof(char) * (strlen(buf) + 1));
    memset(textFromInputFile, '\0', sizeof(char) * (strlen(buf) + 1));
    strcpy(textFromInputFile, buf);

    int inputFileTextLength = strlen(textFromInputFile);    
    if(textFromInputFile[inputFileTextLength - 1] == '\n')
    {
       textFromInputFile[inputFileTextLength - 1] = '\0';
    }
    
    //read key file
    memset(buf, '\0', sizeof(char) * 1000000);
    fgets(buf, 1000000, keyFile);
    char* textFromKeyFile = malloc(sizeof(char) * (strlen(buf) + 1));
    memset(textFromKeyFile, '\0', sizeof(char) * (strlen(buf) + 1));
    strcpy(textFromKeyFile, buf);
    
    inputFileTextLength = strlen(textFromKeyFile);    
    if(textFromKeyFile[inputFileTextLength - 1] == '\n')
    {
       textFromKeyFile[inputFileTextLength - 1] = '\0';
    }
    
    free(buf); //free buffer
    
    //text from input file error
    if(strlen(textFromInputFile) > strlen(textFromKeyFile))
    {
        fprintf(stderr, "Error: key file too small\n");
        exit(1);
    }
    
    //bad characters in text from input file
    unsigned long i;
    for (i = 0 ; i < strlen(textFromInputFile) ; i ++)
    {
        if (getNumber(textFromInputFile[i]) == -1)
        {
            fprintf(stderr, "Error: bad characters in file\n");
            exit(1);
        }
    }
    
    unsigned long j;
    for (j = 0 ; j < strlen(textFromKeyFile) ; j ++)
    {
        if (getNumber(textFromKeyFile[j]) == -1)
        {
            fprintf(stderr, "Error: bad characters key\n");
            exit(1);
        }
    }
    
    socketFD = socket(AF_INET, SOCK_STREAM, 0); //create socket
    serverHostInfo = gethostbyname("localhost"); // set server host to localhost

    memset((char *)&serverAddress, '\0', sizeof(serverAddress));
    serverAddress.sin_port = htons(portNumber);
    memcpy(&serverAddress.sin_addr, serverHostInfo->h_addr_list[0], serverHostInfo->h_length);
    serverAddress.sin_family = AF_INET;
    connect(socketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)); //connect 
        
    char rBuf[10];
    char* copyBuf = malloc(sizeof(char) * 1000000);
    memset(rBuf, '\0', 1);
    memset(copyBuf, '\0', sizeof(char) * 1000000);
    
    writeValues = write(socketFD, "$" "~~", 3); //tell daemon what program we are in
    
    //read response message from server
    while(strstr(copyBuf, "~~") == NULL)
    {
        readValues = read (socketFD, rBuf, 1);
        if (readValues != -1)
        {
            copyBuf[myIndex] = rBuf[0];
            myIndex++;
        }
    }

    if (strstr(copyBuf, "&" "~~") != NULL)
    {
        //continue writing values
        writeValues = write(socketFD, textFromInputFile, strlen(textFromInputFile));
        //write encoded values for checking
        writeValues = write(socketFD, "~~", 2);
        //write to key file daemon
        writeValues = write(socketFD, textFromKeyFile, strlen(textFromKeyFile));
        //now the encoded values for the key file
        writeValues = write(socketFD, "~~", 2);

        //reset
        myIndex = 0;
        memset(rBuf, '\0', 10);
        memset(copyBuf, '\0', sizeof(char) * 1000000);
        
        //read response from daemon
        while(strstr(copyBuf, "~~") == NULL)
        {
            readValues = read(socketFD, rBuf, 1);
            if(readValues != -1)
            {
                copyBuf[myIndex] = rBuf[0];
                myIndex++;
            }
        }
        
        //erase my encoded values 
        int textInputLength = strlen(copyBuf);
        copyBuf[textInputLength-1] = '\0';
        copyBuf[textInputLength-2] = '\0';
        
        //print response 
        for(int i = 0 ; i < strlen(copyBuf) ; i++)
        {
            printf("%c", copyBuf[i]);
        }
        printf("\n"); //print new line
        
        
    }
    else if(strstr(copyBuf, "%" "~~") != NULL)
    {
        //wrong daemon exit
        fprintf(stderr, "Error: otp_dec cannot use otp_enc_d\n");
        exit(1);
    }
    else
    {
        //bad data 
        fprintf(stderr, "Error: bad data\n");
        exit(1);
    }
    
    exit(0);
}

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