#include "vm.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
/* Compiling using gcc with -O3, enabling opcode emulation gives about
 * 30 host cycles per instruction performance.
 * Not too bad.
 */

void zpuvm_init(zpuvm* vm, zpuvm_memblock* mem, uintzpu_t blocks, uintzpu_t pc, uintzpu_t sp) {
    vm->blocks      = blocks;
    vm->memblock    = mem;
    vm->pc          = pc;
    vm->sp          = sp;
    vm->flags       = 0;
    return;
}

void zpuvm_memblock_init(zpuvm_memblock* memblock, void* mem, uintzpu_t base, uintzpu_t size) {
    memblock->base = base;
    memblock->size = size;
    memblock->mem = mem;
    return;
}

inline static zpuv zpu_mem_read(zpuvm* vm, uintzpu_t addr) { 
    #ifndef ZPU_NAIVE_MEM
    for (uintzpu_t i = 0; i < vm->blocks; i++) {
        zpuvm_memblock* m = vm->memblock + i;
        if (addr >= m->base && addr - m->base < m->size) {
            return *((zpuv*)((char*)m->mem + (addr - m->base)));
        }
    }
    return h2zpu(0);
    #endif
    #ifdef ZPU_NAIVE_MEM
    return *((zpuv*)((char*)vm->memblock->mem + addr));
    #endif
}

inline static uint8_t zpu_mem_read_opcode(zpuvm* vm, uintzpu_t addr) { 
    for (uintzpu_t i = 0; i < vm->blocks; i++) {
        zpuvm_memblock* m = vm->memblock + i;
        if (addr >= m->base && addr - m->base < m->size) {
            return *((uint8_t*)((char*)m->mem + (addr - m->base)));
        }
    }
    return 0;
}

inline static void zpu_mem_set(zpuvm* vm, uintzpu_t addr, zpuv v) {
    for (uintzpu_t i = 0; i < vm->blocks; i++) {
        zpuvm_memblock* m = vm->memblock + i;
        if (addr >= m->base && addr - m->base < m->size) {
            *((zpuv*)((char*)m->mem + (addr - m->base))) = v;
            return;
        }
    }
    return;
}

inline static void zpu_stack_set(zpuvm* vm, zpuv x) {
    zpu_mem_set(vm, vm->sp, x);
}

inline static void zpu_stack_push(zpuvm* vm, zpuv x) {
    vm->sp -= ZPU_WORD_SIZE;
    zpu_stack_set(vm, x);
}

inline static zpuv zpu_stack_peek(zpuvm* vm) {
    return zpu_mem_read(vm, vm->sp);
}

inline static zpuv zpu_stack_pop(zpuvm* vm) {
    zpuv v = zpu_stack_peek(vm);
    vm->sp += ZPU_WORD_SIZE;
    return v;
}

int zpuvm_step(zpuvm* vm) {
    const uint8_t opcode = zpu_mem_read_opcode(vm, vm->pc);
    // OPCODE ----- IM
    if (IS_ZPU_OPCODE_IM(opcode)) 
    {
        bool idim = vm->flags & ZPU_FLAG_IDIM;
        if (idim) 
        { // IDIM flag set, shift and append
            uintzpu_t x = ((zpu2h(zpu_stack_peek(vm))) << 7) | (opcode & 0x7F);
            zpu_stack_set(vm, h2zpu(x));
        } else 
        { // sign extend and set, sign extend 7 bits to zpu bits (32?)
            uintzpu_t x = (uintzpu_t)((int8_t)((opcode & 0x7F) | ((opcode << 1) & 0x80)));
            zpu_stack_push(vm, h2zpu(x));
        }
        // set IDIM flag
        vm->flags |= ZPU_FLAG_IDIM;
        vm->pc++;
    } 
    // OPCODE ----- STORESP
    else if (IS_ZPU_OPCODE_STORESP(opcode)) 
    {
        uintzpu_t addr = vm->sp + ((uintzpu_t)((opcode & 0x1F) ^ 0x10)) * ZPU_WORD_SIZE;
        zpuv v = zpu_stack_pop(vm);
        // pop value from stack top and store value to addr
        zpu_mem_set(vm, addr, v);
        // unset IDIM flag
        vm->flags &= ~ZPU_FLAG_IDIM;
        vm->pc++;
    }
    else if (IS_ZPU_OPCODE_LOADSP(opcode))
    {
        // TODO: VERIFY MASK!
        uintzpu_t addr = vm->sp + ((uintzpu_t)((opcode & 0x1F) ^ 0x10)) * ZPU_WORD_SIZE;
        zpu_stack_push(vm, zpu_mem_read(vm, addr));
        // unset IDIM flag
        vm->flags &= ~ZPU_FLAG_IDIM;
        vm->pc++;
    }
    else if (IS_ZPU_OPCODE_ADDSP(opcode))
    {
        uintzpu_t addr = vm->sp + (uintzpu_t)(opcode & 0xF) * ZPU_WORD_SIZE;
        uintzpu_t src = zpu2h(zpu_mem_read(vm, addr));
        uintzpu_t dst = zpu2h(zpu_stack_peek(vm));
        zpu_stack_set(vm, h2zpu(src + dst));
        // unset IDIM flag
        vm->flags &= ~ZPU_FLAG_IDIM;
        vm->pc++;
    }
    else if (IS_ZPU_OPCODE_POPPC(opcode))
    {
        // unset IDIM flag
        vm->flags &= ~ZPU_FLAG_IDIM;
        vm->pc = zpu2h(zpu_stack_pop(vm));
    }
    else if (IS_ZPU_OPCODE_LOAD(opcode))
    {
        uintzpu_t addr = zpu2h(zpu_stack_peek(vm));
        // TODO: verify addr to be 32bit aligned.
        zpu_stack_set(vm, zpu_mem_read(vm, addr));
        // unset IDIM flag
        vm->flags &= ~ZPU_FLAG_IDIM;
        vm->pc++;
    }
    else if (IS_ZPU_OPCODE_STORE(opcode))
    {
        uintzpu_t addr = zpu2h(zpu_stack_pop(vm));
        // TODO: verify addr to be 32bit aligned.
        zpuv v = zpu_stack_pop(vm);
        zpu_mem_set(vm, addr, v);
        // unset IDIM flag
        vm->flags &= ~ZPU_FLAG_IDIM;
        vm->pc++;
    }
    else if (IS_ZPU_OPCODE_PUSHSP(opcode))
    {
        zpu_stack_push(vm, h2zpu(vm->sp));
        vm->flags &= ~ZPU_FLAG_IDIM;
        vm->pc++;
    }
    else if (IS_ZPU_OPCODE_POPSP(opcode))
    {
        vm->sp = zpu2h(zpu_stack_pop(vm));
        vm->flags &= ~ZPU_FLAG_IDIM;
        vm->pc++;
    }
    else if (IS_ZPU_OPCODE_ADD(opcode))
    {
        uintzpu_t u1 = zpu2h(zpu_stack_pop(vm));
        uintzpu_t u2 = zpu2h(zpu_stack_peek(vm));
        zpu_stack_set(vm, h2zpu(u1 + u2));
        vm->flags &= ~ZPU_FLAG_IDIM;
        vm->pc++;
    }
    else if (IS_ZPU_OPCODE_AND(opcode))
    {
        uintzpu_t u1 = zpu2h(zpu_stack_pop(vm));
        uintzpu_t u2 = zpu2h(zpu_stack_peek(vm));
        zpu_stack_set(vm, h2zpu(u1 & u2));
        vm->flags &= ~ZPU_FLAG_IDIM;
        vm->pc++;
    }
    else if (IS_ZPU_OPCODE_OR(opcode))
    {
        uintzpu_t u1 = zpu2h(zpu_stack_pop(vm));
        uintzpu_t u2 = zpu2h(zpu_stack_peek(vm));
        zpu_stack_set(vm, h2zpu(u1 | u2));
        vm->flags &= ~ZPU_FLAG_IDIM;
        vm->pc++;
    }
    else if (IS_ZPU_OPCODE_NOT(opcode))
    {
        uintzpu_t v = zpu2h(zpu_stack_peek(vm));
        zpu_stack_set(vm, h2zpu(~v));
        vm->flags &= ~ZPU_FLAG_IDIM;
        vm->pc++;
    }
    else if (IS_ZPU_OPCODE_FLIP(opcode))
    {
        uintzpu_t v = zpu2h(zpu_stack_peek(vm));
        uintzpu_t v_rev = v;
        uintzpu_t count = sizeof(v) * 8 - 1;
        v >>= 1;
        while (v) {
            v_rev <<= 1;
            v_rev |= v & 0x1;
            v >>= 1;
            count--;
        }
        v_rev <<= count;
        zpu_stack_set(vm, h2zpu(v_rev));
        vm->flags &= ~ZPU_FLAG_IDIM;
        vm->pc++;
    }
    else if (IS_ZPU_OPCODE_NOP(opcode))
    {
        vm->flags &= ~ZPU_FLAG_IDIM;
        vm->pc++;
    }
    else if (IS_ZPU_OPCODE_EMULATE(opcode))
    {
        uintzpu_t addr = (uintzpu_t)(opcode & 0x1F) * 32;
        zpu_stack_push(vm, h2zpu(vm->pc+1));
        // unset IDIM flag
        vm->flags &= ~ZPU_FLAG_IDIM;
        vm->pc = addr;
    }
    else
    { // unsupported
        vm->pc++;
    }
    return 0;
}
