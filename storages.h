#include <stdint.h>

typedef struct {
    uint8_t  valid;
    uint8_t page_num;
    uint8_t frame_num; 
} tlb_entry;


typedef struct {
    uint8_t present;    
    int16_t frame_num; 
} page_entry;

typedef struct {
    uint8_t *store_ptr;
    int count;
} frame_entry;


typedef struct {
    uint16_t addr;
    uint8_t page;
    uint8_t offset;
} sequence;