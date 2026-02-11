#include <stdio.h>



int main (int argc, char argv[]){

    // check the arguments
    if (argc > 4){
        perror("There are more than 4 arguments, must be in: memSim <reference-sequence-file.txt> <FRAMES> <PRA> form\n");
        return -1;
    }



}