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

#define KERNEL_VIRTUAL 0xffffffe000000000
#define KERNEL_PHYSICS 0x80000000
#define UART_PHYSICS 0x10000000

#define TEXT_VIRTUAL ((uint64_t)&text_start + offset)
#define TEXT_PHYSICS ((uint64_t)&text_start)
#define RODATA_VIRTUAL ((uint64_t)&rodata_start + offset)
#define RODATA_PHYSICS ((uint64_t)&rodata_start)
#define DATA_VIRTUAL ((uint64_t)&data_start + offset)
#define DATA_PHYSICS ((uint64_t)&data_start)

#define MAPPING_SIZE (uint64_t)0x1000000
#define TEXT_SIZE (uint64_t)((uint64_t)(&rodata_start) - (uint64_t)(&text_start))
#define RODATA_SIZE (uint64_t)((uint64_t)(&data_start) - (uint64_t)(&rodata_start))
#define DATA_SIZE (uint64_t)(MAPPING_SIZE - TEXT_SIZE - RODATA_SIZE)


#define SECTION 1


uint64_t page_num = 1;

void create_mapping(uint64_t *pgtbl, uint64_t va, uint64_t pa, uint64_t sz,
                    int perm) {
  // pgtbl 是三级页表的首地址，他已经占用了一页的大小
    // 所以后续空闲物理页面需要从 &_end + 0x1000 * n 开始分配。
    // 每次设置一个一级页表项管理 4KB 大小的映射，将 4KB 大小的 va 映射到 4KB 大小的 pa
    // 并在该过程中设置好二级和三级页表。
  while(sz > 0) {
    uint64_t* pgtLevel2;
    if((pgtbl[VPN2(va)] & MASK_VALID) == 1) {
      pgtLevel2 = ENTRY2ADDR(pgtbl[VPN2(va)]);
    }
    else {
      //Allocate
      pgtLevel2 = (uint64_t*)((uint64_t)&_end + page_num * PAGE_SIZE);
      page_num++;
      pgtbl[VPN2(va)] = MAKE_ENTRY(pgtbl[VPN2(va)], pgtLevel2, 0);
    }
    

    uint64_t* pgtLevel1;
    if((pgtLevel2[VPN1(va)] & MASK_VALID) == 1) {
      pgtLevel1 = ENTRY2ADDR(pgtLevel2[VPN1(va)]);
    }
    else {
      //Allocate
      pgtLevel1 = (uint64_t*)((uint64_t)&_end + page_num * PAGE_SIZE);
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

  #if SECTION
  create_mapping(pgtbl, TEXT_VIRTUAL, TEXT_PHYSICS, TEXT_SIZE, PERM_X | PERM_R);
  create_mapping(pgtbl, RODATA_VIRTUAL, RODATA_PHYSICS, RODATA_SIZE, PERM_R);
  create_mapping(pgtbl, DATA_VIRTUAL, DATA_PHYSICS, DATA_SIZE, PERM_W | PERM_R);

  create_mapping(pgtbl, TEXT_PHYSICS, TEXT_PHYSICS, TEXT_SIZE, PERM_X | PERM_R);
  create_mapping(pgtbl, RODATA_PHYSICS, RODATA_PHYSICS, RODATA_SIZE, PERM_R);
  create_mapping(pgtbl, DATA_PHYSICS, DATA_PHYSICS, DATA_SIZE, PERM_W | PERM_R);

  create_mapping(pgtbl, UART_PHYSICS, UART_PHYSICS, PAGE_SIZE, PERM_W | PERM_R);
  #else
  create_mapping(pgtbl, KERNEL_VIRTUAL, KERNEL_PHYSICS, MAPPING_SIZE, PERM_X | PERM_W | PERM_R);
  create_mapping(pgtbl, KERNEL_PHYSICS, KERNEL_PHYSICS, MAPPING_SIZE, PERM_X | PERM_W | PERM_R);
  create_mapping(pgtbl, UART_PHYSICS, UART_PHYSICS, PAGE_SIZE, PERM_X | PERM_W | PERM_R);
  #endif
}
