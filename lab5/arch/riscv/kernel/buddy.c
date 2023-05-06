#include "buddy.h"
#include "put.h"

#define PAGE_SIZE 4096
#define offset (0xffffffe000000000 - 0x80000000)
#define BUDDY_PHYSIC_START (uint64_t)0x80000000
#define BUDDY_VIRTUAL_START (uint64_t)0xffffffe000000000
extern int isVirtual;

typedef unsigned long long uint64_t;

buddy buddy_system;

void init_buddy_system(void) {
    buddy_system.size = 4096;

    buddy_system.bitmap[1] = 4096;
    for(int i = 2; i <= 8191; i++) {
        if(i % 2 == 0) {
            //偶数
            buddy_system.bitmap[i] = buddy_system.bitmap[i/2]/2;
        }
        else {
            //奇数
            buddy_system.bitmap[i] = buddy_system.bitmap[(i-1)/2]/2;
        }
    }
}
void *alloc_pages(int npages) {
    if(npages > buddy_system.bitmap[1] || npages < 1) {
        return (void*)0;
    }

    int align_page_num = npages - 1;
    align_page_num |= align_page_num >> 1;
    align_page_num |= align_page_num >> 2;
    align_page_num |= align_page_num >> 4;
    align_page_num |= align_page_num >> 8;
    align_page_num |= align_page_num >> 16;
    align_page_num++;

    int origin_page_num = 4096;
    int target_index = 1;
    uint64_t addr = isVirtual? BUDDY_VIRTUAL_START: BUDDY_PHYSIC_START;

    while (1) {
        if(buddy_system.bitmap[2 * target_index] >= align_page_num) {
        target_index = 2 * target_index;
        }
        else if(buddy_system.bitmap[2 * target_index + 1] >= align_page_num) {
            target_index = 2 * target_index + 1;
            addr += (origin_page_num / 2) * PAGE_SIZE;
        }
        else {
            break;
        }
        origin_page_num /= 2;
    }

    for(int i = target_index; i >= 1; i = i / 2) {
        buddy_system.bitmap[i] -= align_page_num;
    }


    return (void*)addr;
}
void free_pages(void* addr) {
    int page_num = isVirtual? (((uint64_t)addr - BUDDY_VIRTUAL_START) >> 12): (((uint64_t)addr - BUDDY_PHYSIC_START) >> 12);

    int index = buddy_system.size + page_num; //树叶的起始index
    int origin_size = 1;
    int isFound = 0;
    for(int index = buddy_system.size + page_num;;index = index / 2) {
        if(buddy_system.bitmap[index] != 0 && !isFound) {
            origin_size <<= 1;
        }
        else {
            isFound = 1;
            buddy_system.bitmap[index] += origin_size;
            if(index == 1) {
                break;
            }
        }
    }
}