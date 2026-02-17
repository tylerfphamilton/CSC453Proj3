#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "storages.h"

// GLOBALS (I heart globals)

tlb_entry TLB[16];
page_entry PAGETABLE[256];
uint8_t **MEMORY;

//gobals for stats

int TOTAL = 0;
int TLB_HIT = 0;
int PAGE_HIT = 0;
int PAGE_FAULT = 0



uint8_t **init_mem(uint16_t framenum) {
    uint8_t **retval = malloc(framenum * sizeof(*retval));
    if (!retval) return NULL;

    for (uint16_t i = 0; i < framenum; i++) {
        retval[i] = malloc(256 * sizeof(*retval[i]));
        if (!retval[i]) return NULL;
    }
    return retval;
}


int in_tlb(uint8_t *framenum , uint8_t page){
    for (int i = 0 ; i < 16 ; i++){
        if (TLB[i].valid && TLB[i].page_num == page){
            *frame_num = TLB[i].frame_num;
            return 1;
        }
    }
    TLB_HIT++;
    *framenum = -1;
    return 0;
}


int in_pagetable(uint8_t *framenum , uint8_t page){
    *framenum = -1;
    return 0;
}

int take_from_store(uint8_t *framenum , uint8_t page , char *pra){
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
    *framenum = -1;
    return 0;
}

int get_frame_from_mem(uint8_t framenum){

    return 0;
}


int main (int argc, char* argv[]){

    // check the arguments
    if (argc > 4){
        perror("There are more than 4 arguments, must be in: memSim <reference-sequence-file.txt> <FRAMES> <PRA> form\n");
        return -1;
    }

    // checking the first argument
    char* filename = argv[1];
    FILE *fp = fopen(filename, "r"); //used stream because it's easier
    if (!fp) {
        perror("Error opening reference file");
        exit(EXIT_FAILURE);
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
    MEMORY = init_mem(frames);
    if (!MEMORY) return;

    // checking the third argument
    char* pra = argv[3];

    //MAIN LOOP - itirate through each sequence

    char line[64];  //should be more than enough and go up to the newline
    while (fgets(line, sizeof(line), fp)) {
        //extract actual sequence data (parsing)

        long logical_address = strtol(line, NULL , 10);
        uint16_t addr = (uint16_t)(logical_address & 0xFFFF);
        uint8_t page   = (addr >> 8) & 0xFF;
        uint8_t offset = addr & 0xFF;

        uint8_t framenum;

        if (!in_tlb(&framenum , page)) {
            if (!in_pagetable(&framenum , page)) {
                if (!take_from_store(&framenum , page ,pra)) {
                    printf("not in memory, I'm crine 😭\n");
                    break;
                }
            }
        }
        uint8_t *frame = MEMORY[framenum]; //this is the page, yay.
        int8_t sval  = (int8_t)frame[offset];

        //print everything
        printf("%u,%d,%u,", addr, (int)sval, (unsigned)framenum);
        for (int i = 0; i < 256; i++) {
            printf("%02X", MEMORY[frame_num][i]);
        }
        printf("\n");

    }


    fclose(fp);
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