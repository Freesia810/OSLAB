#include "vm.h"

#include "put.h"

extern unsigned long long text_start;
extern unsigned long long rodata_start;
extern unsigned long long data_start;
extern unsigned long long _end;


#define offset (0xffffffe000000000 - 0x80000000)
#define PAGE_SIZE 0x1000

#define VPN2(va) ((va >> 30) & 0x1ff)
#define VPN1(va) ((va >> 21) & 0x1ff)
#define VPN0(va) ((va >> 12) & 0x1ff)
#define ENTRY2ADDR(en) (uint64_t*)((en >> 10) << 12)
#define MAKE_ENTRY(reserved, pa, perm) (uint64_t)(((uint64_t)reserved & 0xffc0000000000000) | (((uint64_t)pa >> 12) << 10) | ((uint64_t)(perm << 1) + 1))

#define MASK_VALID 0x1

#define PERM_X 0x4
#define PERM_W 0x2
#define PERM_R 0x1
#define PERM_U 0x8

#define KERNEL_VIRTUAL 0xffffffe000000000
#define KERNEL_PHYSICS 0x80000000
#define UART_VIRTUAL 0xffffffdf90000000
#define UART_PHYSICS 0x10000000

#define KERNEL_TEXT_VIRTUAL ((uint64_t)&text_start + offset)
#define KERNEL_TEXT_PHYSICS ((uint64_t)&text_start)
#define KERNEL_RODATA_VIRTUAL ((uint64_t)&rodata_start + offset)
#define KERNEL_RODATA_PHYSICS ((uint64_t)&rodata_start)
#define KERNEL_DATA_VIRTUAL ((uint64_t)&data_start + offset)
#define KERNEL_DATA_PHYSICS ((uint64_t)&data_start)

#define KERNEL_SIZE (uint64_t)0x1000000
#define KERNEL_TEXT_SIZE (uint64_t)((uint64_t)(&rodata_start) - (uint64_t)(&text_start))
#define KERNEL_RODATA_SIZE (uint64_t)((uint64_t)(&data_start) - (uint64_t)(&rodata_start))
#define KERNEL_DATA_SIZE (uint64_t)(KERNEL_SIZE - KERNEL_TEXT_SIZE - KERNEL_RODATA_SIZE)

#define USER_PHYSICS 0x84000000
#define USER_VIRTUAL_START 0X0
#define USER_VIRTUAL_STACK_TOP 0x0000004000000000
#define USER_SIZE 0x100000
#define USER_STACK_SIZE 0x100000


uint64_t page_num = 1;
int isVirtual = 0;

void create_mapping(uint64_t *pgtbl, uint64_t va, uint64_t pa, uint64_t sz, int perm) {
    while(sz > 0) {
        uint64_t* pgtLevel2;
        if((pgtbl[VPN2(va)] & MASK_VALID) == 1) {
            pgtLevel2 = ENTRY2ADDR(pgtbl[VPN2(va)]);
        }
        else {
            //Allocate
            if(isVirtual) {
                pgtLevel2 = (uint64_t*)((uint64_t)&_end + page_num * PAGE_SIZE - offset);
            }
            else {
                pgtLevel2 = (uint64_t*)((uint64_t)&_end + page_num * PAGE_SIZE);
            }
            page_num++;
            pgtbl[VPN2(va)] = MAKE_ENTRY(pgtbl[VPN2(va)], pgtLevel2, 0);
        }


        uint64_t* pgtLevel1;
        if((pgtLevel2[VPN1(va)] & MASK_VALID) == 1) {
            pgtLevel1 = ENTRY2ADDR(pgtLevel2[VPN1(va)]);
        }
        else {
            //Allocate
            if(isVirtual) {
                pgtLevel1 = (uint64_t*)((uint64_t)&_end + page_num * PAGE_SIZE - offset);
            }
            else {
                pgtLevel1 = (uint64_t*)((uint64_t)&_end + page_num * PAGE_SIZE);
            }
            page_num++;
            pgtLevel2[VPN1(va)] = MAKE_ENTRY(pgtLevel2[VPN1(va)], pgtLevel1, 0);
        }
        pgtLevel1[VPN0(va)] = MAKE_ENTRY(pgtLevel1[VPN0(va)], pa, perm);


        va+=PAGE_SIZE;
        pa+=PAGE_SIZE;
        sz-=PAGE_SIZE;
    }
}

void paging_init() { 
    uint64_t *pgtbl = &_end; 

    create_mapping(pgtbl, KERNEL_TEXT_VIRTUAL, KERNEL_TEXT_PHYSICS, KERNEL_TEXT_SIZE, PERM_X | PERM_R);
    create_mapping(pgtbl, KERNEL_RODATA_VIRTUAL, KERNEL_RODATA_PHYSICS, KERNEL_RODATA_SIZE, PERM_R);
    create_mapping(pgtbl, KERNEL_DATA_VIRTUAL, KERNEL_DATA_PHYSICS, KERNEL_DATA_SIZE, PERM_W | PERM_R);

    create_mapping(pgtbl, KERNEL_TEXT_PHYSICS, KERNEL_TEXT_PHYSICS, KERNEL_TEXT_SIZE, PERM_X | PERM_R);
    create_mapping(pgtbl, KERNEL_RODATA_PHYSICS, KERNEL_RODATA_PHYSICS, KERNEL_RODATA_SIZE, PERM_R);
    create_mapping(pgtbl, KERNEL_DATA_PHYSICS, KERNEL_DATA_PHYSICS, KERNEL_DATA_SIZE, PERM_W | PERM_R);

    create_mapping(pgtbl, UART_VIRTUAL, UART_PHYSICS, PAGE_SIZE, PERM_X | PERM_W | PERM_R);

    
}

uint64_t user_paging_init() {
    static uint64_t user_stack_physic = USER_PHYSICS + USER_SIZE;

    isVirtual = 1;
    uint64_t* user_pgtbl = (uint64_t*)((uint64_t)&_end + page_num * PAGE_SIZE - offset);
    page_num++;

    //copy
    for (uint64_t i = 0; i < PAGE_SIZE; i++) {
        *((unsigned char*)user_pgtbl + i) = *((unsigned char*)((uint64_t)&_end - offset) + i);
    }
    
    create_mapping(user_pgtbl, USER_VIRTUAL_START, USER_PHYSICS, USER_SIZE, PERM_U | PERM_R | PERM_W | PERM_X);
    create_mapping(user_pgtbl, USER_VIRTUAL_STACK_TOP - USER_STACK_SIZE, user_stack_physic, USER_STACK_SIZE, PERM_U | PERM_R | PERM_W);
    user_stack_physic = user_stack_physic + USER_STACK_SIZE;
    

    return (uint64_t)user_pgtbl;
}
