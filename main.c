#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main (int argc, char* argv[]){

    // check the arguments
    if (argc > 4){
        perror("There are more than 4 arguments, must be in: memSim <reference-sequence-file.txt> <FRAMES> <PRA> form\n");
        return -1;
    }

    char* filename = argv[1];
    int fd = open(filename, O_RDONLY);

    // check open() error
    if (fd == -1){
        perror("There was an error opening the file\n");
    }

    // next we are going to read from the descriptor
    

    return 0;

}