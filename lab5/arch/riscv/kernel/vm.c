#include "vm.h"

#include "buddy.h"
#include "put.h"
#include "slub.h"

extern unsigned long long text_start;
extern unsigned long long rodata_start;
extern unsigned long long data_start;
extern unsigned long long _end;

#define VPN2(va) ((va >> 30) & 0x1ff)
#define VPN1(va) ((va >> 21) & 0x1ff)
#define VPN0(va) ((va >> 12) & 0x1ff)
#define ENTRY2ADDR(en) (uint64_t*)((en >> 10) << 12)
#define MAKE_ENTRY(reserved, pa, perm) (uint64_t)(((uint64_t)reserved & 0xffc0000000000000) | (((uint64_t)pa >> 12) << 10) | ((uint64_t)(perm << 1) + 1))
#define MASK_VALID 0x1


#define KERNEL_VIRTUAL 0xffffffe000000000
#define KERNEL_PHYSICS 0x80000000
#define UART_PHYSICS 0x10000000

#define KERNEL_TEXT_VIRTUAL isVirtual? (uint64_t)&text_start: ((uint64_t)&text_start + offset)
#define KERNEL_TEXT_PHYSICS isVirtual? (uint64_t)((uint64_t)&text_start - offset): ((uint64_t)&text_start)
#define KERNEL_RODATA_VIRTUAL isVirtual? (uint64_t)&rodata_start: ((uint64_t)&rodata_start + offset)
#define KERNEL_RODATA_PHYSICS isVirtual? (uint64_t)((uint64_t)&rodata_start - offset): ((uint64_t)&rodata_start)
#define KERNEL_DATA_VIRTUAL isVirtual? (uint64_t)&data_start: ((uint64_t)&data_start + offset)
#define KERNEL_DATA_PHYSICS isVirtual? (uint64_t)((uint64_t)&data_start - offset): ((uint64_t)&data_start)

#define KERNEL_SIZE (uint64_t)0x1000000
#define KERNEL_TEXT_SIZE (uint64_t)((uint64_t)(&rodata_start) - (uint64_t)(&text_start))
#define KERNEL_RODATA_SIZE (uint64_t)((uint64_t)(&data_start) - (uint64_t)(&rodata_start))
#define KERNEL_DATA_SIZE (uint64_t)(KERNEL_SIZE - KERNEL_TEXT_SIZE - KERNEL_RODATA_SIZE)

#define offset (KERNEL_VIRTUAL - KERNEL_PHYSICS)

int isVirtual = 0;
uint64_t *kernel_pgtbl;

void create_mapping(uint64_t *pgtbl, uint64_t va, uint64_t pa, uint64_t sz, int perm) {
    while(sz > 0) {
        uint64_t* pgtLevel2;

        if((pgtbl[VPN2(va)] & MASK_VALID) == 1) {
            pgtLevel2 = ENTRY2ADDR(pgtbl[VPN2(va)]);
        }
        else {
            //Allocate
            if(isVirtual) {
                pgtLevel2 = (uint64_t*)((uint64_t)alloc_pages(1) - offset);
            }
            else {
                pgtLevel2 = (uint64_t*)alloc_pages(1);
            }
            pgtbl[VPN2(va)] = MAKE_ENTRY(pgtbl[VPN2(va)], pgtLevel2, 0);
        }


        uint64_t* pgtLevel1;
        if((pgtLevel2[VPN1(va)] & MASK_VALID) == 1) {
            pgtLevel1 = ENTRY2ADDR(pgtLevel2[VPN1(va)]);
        }
        else {
            //Allocate
            if(isVirtual) {
                pgtLevel1 = (uint64_t*)((uint64_t)alloc_pages(1) - offset);
            }
            else {
                pgtLevel1 = (uint64_t*)alloc_pages(1);
            }
            pgtLevel2[VPN1(va)] = MAKE_ENTRY(pgtLevel2[VPN1(va)], pgtLevel1, 0);
        }
        pgtLevel1[VPN0(va)] = MAKE_ENTRY(pgtLevel1[VPN0(va)], pa, perm);


        va+=PAGE_SIZE;
        pa+=PAGE_SIZE;
        sz-=PAGE_SIZE;
    }
}

void paging_init() {
    init_buddy_system();

    alloc_pages(((uint64_t)((uint64_t)&_end - (uint64_t)&text_start) - 1)/PAGE_SIZE + 1);

    kernel_pgtbl = (uint64_t*)alloc_pages(1); 


    create_mapping(kernel_pgtbl, KERNEL_TEXT_VIRTUAL, KERNEL_TEXT_PHYSICS, KERNEL_TEXT_SIZE, PERM_X | PERM_R);
    create_mapping(kernel_pgtbl, KERNEL_RODATA_VIRTUAL, KERNEL_RODATA_PHYSICS, KERNEL_RODATA_SIZE, PERM_R);
    create_mapping(kernel_pgtbl, KERNEL_DATA_VIRTUAL, KERNEL_DATA_PHYSICS, KERNEL_DATA_SIZE, PERM_W | PERM_R);

    create_mapping(kernel_pgtbl, KERNEL_TEXT_PHYSICS, KERNEL_TEXT_PHYSICS, KERNEL_TEXT_SIZE, PERM_X | PERM_R);
    create_mapping(kernel_pgtbl, KERNEL_RODATA_PHYSICS, KERNEL_RODATA_PHYSICS, KERNEL_RODATA_SIZE, PERM_R);
    create_mapping(kernel_pgtbl, KERNEL_DATA_PHYSICS, KERNEL_DATA_PHYSICS, KERNEL_DATA_SIZE, PERM_W | PERM_R);

    create_mapping(kernel_pgtbl, UART_PHYSICS, UART_PHYSICS, PAGE_SIZE, PERM_X | PERM_W | PERM_R);
    

    uint64_t satp = (uint64_t)(0x8000000000000000 + (uint64_t)(((uint64_t)kernel_pgtbl) >> 12));
    asm("ld t0, %0" : : "m"(satp));
    asm("csrw satp, t0");
    asm("sfence.vma");

    isVirtual = 1;
}

uint64_t task_paging_init() {
    uint64_t* task_pgtbl = (uint64_t*)((uint64_t)alloc_pages(1) - offset);

    create_mapping(task_pgtbl, KERNEL_TEXT_VIRTUAL, KERNEL_TEXT_PHYSICS, KERNEL_TEXT_SIZE, PERM_X | PERM_R);
    create_mapping(task_pgtbl, KERNEL_RODATA_VIRTUAL, KERNEL_RODATA_PHYSICS, KERNEL_RODATA_SIZE, PERM_R);
    create_mapping(task_pgtbl, KERNEL_DATA_VIRTUAL, KERNEL_DATA_PHYSICS, KERNEL_DATA_SIZE, PERM_W | PERM_R);

    create_mapping(task_pgtbl, KERNEL_TEXT_PHYSICS, KERNEL_TEXT_PHYSICS, KERNEL_TEXT_SIZE, PERM_X | PERM_R);
    create_mapping(task_pgtbl, KERNEL_RODATA_PHYSICS, KERNEL_RODATA_PHYSICS, KERNEL_RODATA_SIZE, PERM_R);
    create_mapping(task_pgtbl, KERNEL_DATA_PHYSICS, KERNEL_DATA_PHYSICS, KERNEL_DATA_SIZE, PERM_W | PERM_R);

    create_mapping(task_pgtbl, UART_PHYSICS, UART_PHYSICS, PAGE_SIZE, PERM_X | PERM_W | PERM_R);

    

    return (uint64_t)task_pgtbl;
}