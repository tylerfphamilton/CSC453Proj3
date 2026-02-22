#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "storages.h"

// GLOBALS (I heart globals)

char *PATH = "BACKING_STORE.bin";

tlb_entry TLB[16];
int TLB_IDX = 0;


page_entry PAGETABLE[256];
frame_entry *MEMORY;
int FRAMENUM = 0;
int FIFO_IDX = 0;


uint8_t *STORE = NULL;

//gobals for stats

int TOTAL = 0;
int TLB_HIT = 0;
int PAGE_HIT = 0;
int PAGE_FAULT = 0;



frame_entry *init_mem(uint16_t framenum) {
    frame_entry *m = calloc(framenum, sizeof(*m));
    if (!m) return NULL;
    for (int i = 0 ; i < framenum ; i++){
        m[i].count = -1;
    }
    return m;
}

void tlb_insert(uint8_t page, uint8_t frame) {
    TLB[TLB_IDX].valid = 1;
    TLB[TLB_IDX].page_num = page;
    TLB[TLB_IDX].frame_num = frame;
    TLB_IDX = (TLB_IDX + 1) % 16;
}



int in_tlb(uint8_t *framenum , uint8_t page){
    for (int i = 0 ; i < 16 ; i++){
        if (TLB[i].valid && TLB[i].page_num == page){
            *framenum = (uint8_t)TLB[i].frame_num;
            TLB_HIT++;
            return 1;
        }
    }
    return 0;
}


int in_memory(uint8_t *framenum, uint8_t page) {
    if (PAGETABLE[page].present) {
        *framenum = (uint8_t)PAGETABLE[page].frame_num;
        PAGE_HIT++;
        tlb_insert(page , *framenum);
        return 1;  
    }
    PAGE_FAULT++; 
    return 0;      
}


//remove present bit from pagetable and TLB 
void update_tables(uint8_t framenum){
    for (int i = 0 ; i < 16 ; i++){
        if (TLB[i].valid && TLB[i].frame_num == framenum){
            TLB[i].valid = 0;
        }
    }

    for (int j = 0 ; j < 256 ; j++){
        if (PAGETABLE[j].present && PAGETABLE[j].frame_num == framenum){
            PAGETABLE[j].present = 0;
        }
    }
}

void next_use(uint8_t page ,uint8_t frame, int cur , sequence *commands, int size){
    MEMORY[frame].count = -1;
    for (int y = cur + 1; y < size ; y++){
        if (commands[y].page == page){
            MEMORY[frame].count = y - cur;
            return;
        }
    }
}

int take_from_store(uint8_t *framenum , uint8_t page , char *pra){
    //this if statement just chooses what frame we'll evict, because it's all the same after that.

    if (strcmp(pra, "FIFO") == 0){
        // get framenumber (this is our fifo algoirthm)
        frame_entry tyler = {STORE + (page * 256) , 0};
        MEMORY[FIFO_IDX] = tyler;
        *framenum = FIFO_IDX; //update frame
        FIFO_IDX = (FIFO_IDX + 1) % FRAMENUM;
    }
    else if (strcmp(pra, "LRU") == 0){

        // call the function for LRU
        int biggest = -1;
        for (int j = 0 ; j < FRAMENUM ; j++){
            if (MEMORY[j].count > biggest){
                biggest = MEMORY[j].count;
                *framenum = (uint8_t) j;
            }
        }
        frame_entry tyler = {STORE + (page * 256) , 0};
        MEMORY[*framenum] = tyler;
    }
    else if (strcmp(pra, "OPT") == 0){
        // call the function for OPT
        int biggest = -1;
        for (int j = 0 ; j < FRAMENUM ; j++){
            if (MEMORY[j].count == -1){
                *framenum = (uint8_t) j;
                break;
            }
            if (MEMORY[j].count > biggest){
                biggest = MEMORY[j].count;
                *framenum = (uint8_t) j;
            }
        }
        frame_entry tyler = {STORE + (page * 256) , 0};
        MEMORY[*framenum] = tyler;

    }
    else {
        printf("Need to put in a different PRA, must be either FIFO or LRU or OPT \n");
        return 0;
    }

    //remove old copies in frame
    update_tables(*framenum);
    //update pagetable
    PAGETABLE[page].present = 1;
    PAGETABLE[page].frame_num = *framenum;

    //update tlb
    tlb_insert(page , (uint8_t) *framenum);

    return 1;
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
        perror("Error opening reference file, I'm crine 😭");
        exit(EXIT_FAILURE);
    }
    int size = 0;
    char line[100];
    while (fgets(line, sizeof(line), fp)) {size ++;}
    rewind(fp);
    sequence commands[size];
    for (int p = 0 ; p < size ; p++){
        if (!fgets(line, sizeof(line), fp)) return -1;
        long logical_address = strtol(line, NULL , 10);
        uint16_t addr = (uint16_t)(logical_address & 0xFFFF);
        uint8_t page   = (addr >> 8) & 0xFF;
        uint8_t offset = addr & 0xFF;
        commands[p] = (sequence) {addr , page , offset};
    }


    // frames is within the bounds (can be up to 256, which is why I decided to make it uin16_t)
    char* endptr;
    unsigned long frames_32 = strtoul(argv[2], &endptr, 10);
    uint16_t frames = (uint16_t) frames_32;

    // checking the second argument
    if (frames > 256 || frames < 1){
        printf("Incrorrect number of frames, has to be between 1 and 256, I'm crine 😭\n");
        return -1;
    }
    MEMORY = init_mem(frames);
    FRAMENUM = frames;
    if (!MEMORY) return -1;

    // checking the third argument
    char* pra = argv[3];




    //opening the store and mmaping it so we can access without a syscall
    int fd = open("BACKING_STORE.bin", O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }
    uint8_t *backing_store = mmap(NULL, 65536, PROT_READ, MAP_PRIVATE, fd, 0);

    if (backing_store == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }


    STORE = backing_store;


    //this is for LRU
    if (strcmp(pra, "LRU") == 0){
        for (int t = 0 ; t < FRAMENUM ; t++){
            MEMORY[t].count += 2;
        }
    }

    //MAIN LOOP - itirate through each sequence

    for (int c = 0 ; c < size ; c++) {
        //extract actual sequence data (parsing)
        uint16_t addr = commands[c].addr;
        uint8_t page = commands[c].page;
        uint8_t offset = commands[c].offset;


        uint8_t framenum;
        TOTAL++;
        if (!in_tlb(&framenum , page)) {
            if (!in_memory(&framenum , page)) {
                if (!take_from_store(&framenum , page ,pra)) {
                    printf("not in store, I'm crine 😭\n");
                    return -1;
                }
            }
        }
        uint8_t *frame = MEMORY[framenum].store_ptr; //this is the page, yay.
        int8_t sval  = (int8_t)frame[offset];

        //print everything
        printf("%u,%d,%u,", addr, (int)sval, (unsigned)framenum);
        for (int i = 0; i < 256; i++) {
            printf("%02X", (MEMORY[framenum].store_ptr)[i]);
        }
        printf("\n");

        if (strcmp(pra, "LRU") == 0){
            MEMORY[framenum].count = 0;
            for (int t = 0 ; t < FRAMENUM ; t++){
                MEMORY[t].count++;
            }
        } else if (strcmp(pra, "OPT") == 0){
            next_use(page , framenum , c , commands, size);
            for (int t = 0 ; t < FRAMENUM ; t++){
                if (MEMORY[t].count != -1){
                    MEMORY[t].count--;
                }
            }
        }

    }

    printf("Number of Translated Addresses = %d\n", TOTAL);
    printf("Page Faults = %d\n", PAGE_FAULT);
    printf("Page Fault Rate = %.3f\n", TOTAL ? (double)PAGE_FAULT / TOTAL : 0.0);
    printf("TLB Hits = %d\n", TLB_HIT);
    printf("TLB Misses = %d\n", TOTAL - TLB_HIT);
    printf("TLB Hit Rate = %.3f\n", TOTAL ? (double)TLB_HIT / TOTAL : 0.0);



    fclose(fp);
    printf("GOT NO ERRORS, NICE, I'm crine 😭\n");
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