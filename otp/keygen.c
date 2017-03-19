// Name:        Joshua Hesseltine
// Course:      CS344 Program4
// Filename:    keygen.c
// Description: program creates a key file of specified length. 
// The characters in the file generated will be any of the 27 allowed characters, 
// generated using the standard UNIX randomization methods

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv) 
{
    if (argc < 2 || argc > 2)
    {
        fprintf(stderr, "Error: incorrect arguments\n");
        exit(1); //fail gracefully
    } 
    else if (atoi(argv[1]) <= 0)
    {
        fprintf(stderr, "Error: key is not valid\n");
        exit(1); //fail gracefully
    }
    
    time_t t; //time variable
    srand((unsigned) time(&t)); //initialize random generator
    int userKey = atoi(argv[1]); //save user input
    
    for (int i = 0 ; i < userKey ; i++)
    {
        int randomCharacter = rand() % 27; //random generated
        if(randomCharacter == 26)
        {
            printf(" "); //if 26 print extra space
        }
        else
        {
            printf("%c", (char)(randomCharacter + 65));
        }
    }
    printf("\n");
    exit(0); //end successfully
}