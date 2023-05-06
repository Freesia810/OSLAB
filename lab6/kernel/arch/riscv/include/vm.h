#ifndef VM_H
#define VM_H

#include "sched.h"

#define offset (0xffffffe000000000 - 0x80000000)

typedef unsigned long long uint64_t;

void create_mapping(uint64_t *pgtbl, uint64_t va, uint64_t pa, uint64_t sz,
                    int perm);
void paging_init();
uint64_t user_paging_init();

#endif
