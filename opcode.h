#ifndef _ZPU_OPCODE_H
#define _ZPU_OPCODE_H

// the basic opcodes
#define IS_ZPU_OPCODE_IM(x)         (((x)&0x80))
#define IS_ZPU_OPCODE_STORESP(x)    (((x)&0xE0)==0x40)
#define IS_ZPU_OPCODE_LOADSP(x)     (((x)&0xE0)==0x60)
#define IS_ZPU_OPCODE_ADDSP(x)      (((x)&0xF0)==0x10)
#define IS_ZPU_OPCODE_EMULATE(x)    (((x)&0xE0)==0x20)
#define IS_ZPU_OPCODE_POPPC(x)      ((x)==0x4)
#define IS_ZPU_OPCODE_LOAD(x)       ((x)==0x8)
#define IS_ZPU_OPCODE_STORE(x)      ((x)==0xC)
#define IS_ZPU_OPCODE_PUSHSP(x)     ((x)==0x2)
#define IS_ZPU_OPCODE_POPSP(x)      ((x)==0xD)
#define IS_ZPU_OPCODE_ADD(x)        ((x)==0x5)
#define IS_ZPU_OPCODE_AND(x)        ((x)==0x6)
#define IS_ZPU_OPCODE_OR(x)         ((x)==0x7)
#define IS_ZPU_OPCODE_NOT(x)        ((x)==0x9)
#define IS_ZPU_OPCODE_FLIP(x)       ((x)==0xA)
#define IS_ZPU_OPCODE_NOP(x)        ((x)==0xB)
#define IS_ZPU_OPCODE_BREAKPOINT(x) ((x)==0x0)

#endif
