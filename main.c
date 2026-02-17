#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

int main (int argc, char* argv[]){

    // check the arguments
    if (argc > 4){
        perror("There are more than 4 arguments, must be in: memSim <reference-sequence-file.txt> <FRAMES> <PRA> form\n");
        return -1;
    }

    // checking the first argument
    char* filename = argv[1];
    int fd = open(filename, O_RDONLY);

    // check open() error
    if (fd == -1){
        perror("There was an error opening the file");
        return -1;
    }

    // frames is within the bounds (can be up to 256, which is why I decided to make it uin16_t)
    char* endptr;
    unsigned long frames_32 = strtoul(argv[2], &endptr, 10);
    uint16_t frames = (uint16_t) frames_32;

    // checking the second argument
    if (frames > 256 || frames < 1){
        printf("Incrorrect number of frames, has to be between 1 and 256\n");
        return -1;
    }

    // checking the third argument
    char* pra = argv[3];
    
    if (strcmp(pra, "FIFO") == 0){

        // call the function for FIFO


    }
    else if (strcmp(pra, "LRU") == 0){

        // call the function for LRU
        

    }
    else if (strcmp(pra, "OPT") == 0){

        // call the function for OPT
        

    }
    else {
        printf("Need to put in a different PRA, must be either FIFO or LRU or OPT\n");
        return -1;
    }

    printf("GOT NO ERRORS, NICE\n");
    return 0;

}



/*
while (read reference sequence){

    -parse line
    -if (in tlb){
        print line;
        continue;
    }
    -if (in physical memory , check page table){
        give page \n
        continue
    }
    -say there was a page fault
    -pull it up from store

}


page table

[
[logical , physical]
[0 , 5]
[1 , 3]
[2 , 21]
]


*/