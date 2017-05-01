#ifndef _ZPU_VM_H
#define _ZPU_VM_H

#include "opcode.h"
#include <stdint.h>
#include <stdlib.h>
#include <endian.h>

#define ZPU_FLAG_IDIM           ((uint8_t)0x1)
typedef uint32_t    uintzpu_t;
typedef int32_t     intzpu_t;
union zpuv_union {
    uintzpu_t v;
};
typedef union zpuv_union zpuv;
#define ZPU_WORD_SIZE           (sizeof(uintzpu_t))

#ifndef ZPU_LITTLE_ENDIAN
#define zpu2h(x) (be32toh((x).v))
#define h2zpu(x) ((zpuv)(htobe32(x)))
#endif

#ifdef ZPU_LITTLE_ENDIAN
#define zpu2h(x) (le32toh((x).v))
#define h2zpu(x) ((zpuv)(htole32(x)))
#endif

#define ZPU_NAIVE_MEM

struct zpuvm_memblock_struct {
    uintzpu_t   base;
    uintzpu_t   size;
    void*       mem;
};
typedef struct zpuvm_memblock_struct zpuvm_memblock;
typedef zpuvm_memblock* zpuvm_memblock_t;

struct zpuvm_struct {
    zpuvm_memblock*     memblock;
    uintzpu_t           blocks;
    uintzpu_t           pc;
    uintzpu_t           sp;
    uintzpu_t           flags;
};
typedef struct zpuvm_struct zpuvm;
typedef zpuvm* zpuvm_t;

void zpuvm_init(zpuvm_t vm, zpuvm_memblock* mem, uintzpu_t blocks, uintzpu_t pc, uintzpu_t sp);
void zpuvm_memblock_init(zpuvm_memblock* memblock, void* mem, uintzpu_t base, uintzpu_t size);
int zpuvm_step(zpuvm_t vm);

#endif
