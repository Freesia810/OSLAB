#pragma once

#include "sched.h"

extern unsigned long long _end;

#define VM_START 0xffffffe000000000

#define PERM_X 0x4
#define PERM_W 0x2
#define PERM_R 0x1

void create_mapping(uint64_t *pgtbl, uint64_t va, uint64_t pa, uint64_t sz,
                    int perm);
void paging_init();
uint64_t task_paging_init();
