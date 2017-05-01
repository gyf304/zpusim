#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "vm.h"
#include "cmdline.h"

#define MIN_MEM_SIZE 0x10000
#define DEFAULT_MEM_SIZE 0x10000

int main(int argc, char *argv[]) {
    struct gengetopt_args_info ai;
    if (cmdline_parser (argc, argv, &ai) != 0) {
        return -1;
    }

    char* filename = ai.filename_arg;
    uintzpu_t main_mem_size = DEFAULT_MEM_SIZE;
    uintzpu_t print_stack_size = 0;
    size_t max_cycles = 0;
    bool peek = false;
    uintzpu_t peek_addr = 0;
    bool silent = false;

    if (ai.mem_given && ai.mem_arg >= MIN_MEM_SIZE) {
        main_mem_size = (uintzpu_t)ai.mem_arg;
    }
    if (ai.stack_given && ai.stack_arg > 0) {
        print_stack_size = (uintzpu_t)ai.stack_arg;
    }
    if (ai.cycles_given && ai.cycles_arg > 0) {
        max_cycles = (size_t)ai.cycles_arg;
    }
    if (ai.peek_given && ai.peek_arg >= 0 && ai.peek_arg < main_mem_size) {
        peek = true;
        peek_addr = (uintzpu_t)ai.peek_arg;
    }
    if (max_cycles && ai.silent_flag) {
        silent = true;
    }

    // FILE LOADING
    FILE* f = fopen (filename, "rb");
    if (!f) {
        fprintf(stderr, "File? \n");
        return -1;
    }
    fseek (f, 0, SEEK_END);
    size_t length = ftell(f);
    if (length > main_mem_size) {
        fclose(f);
        fprintf(stderr, "File too large for given memory size! \n");
        return -1;
    }
    fseek (f, 0, SEEK_SET);

    zpuvm vm;
    zpuvm_memblock memblock[3];
    void* mem[1];
    mem[0] = malloc(main_mem_size);
    if (!mem[0]) {
        fclose(f);
        fprintf(stderr, "Mem allocation error\n");
        return -1;
    }
    fread(mem[0], 1, length, f);
    fclose(f);
    if (!silent) {
        #ifdef ZPU_LITTLE_ENDIAN
        const char endianess[] = "little endian";
        #endif
        #ifndef ZPU_LITTLE_ENDIAN
        const char endianess[] = "big endian";
        #endif
        printf("ZPUSIM: %zu bit %s ZPU, %u bytes of mem", ZPU_WORD_SIZE * 8, endianess, main_mem_size);
        if (print_stack_size) printf(", printing upto %d deep in stack", print_stack_size);
        if (peek) printf(", poking 0x%08x", peek_addr);
        printf(".\n");
    }
    zpuvm_memblock_init(&(memblock[0]), mem[0], 0x0, main_mem_size);
    zpuvm_init(&vm, memblock, 1, 0, main_mem_size);
    size_t cycles = 1;
    while(1) {
        unsigned char op = *((unsigned char*)(mem[0] + vm.pc));
        uintzpu_t v = zpu2h(*((zpuv*)(mem[0] + 0x8000)));
        if (!silent) {
            printf("cycle         %10zu    ", cycles);
            printf("opcode          0x%02x\n", op);
            printf("pc_before     0x%08x    ", vm.pc);
            printf("sp_before   %08x \n", vm.sp);
        }
        zpuvm_step(&vm);
        if (!silent) {
            printf("pc_after      0x%08x    ", vm.pc);
            printf("sp_after    %08x \n", vm.sp);
            if (peek) {
                printf("peek_after    0x%08x <= 0x%08x\n", peek_addr, zpu2h(*((zpuv*)(mem[0] + peek_addr))));
            }
            if (print_stack_size) {
                uintzpu_t sp = vm.sp;
                uintzpu_t sc = print_stack_size;
                printf("STACK LAYOUT (After execution):\n");
                while(sc && sp <= main_mem_size - ZPU_WORD_SIZE) {
                    uintzpu_t st = zpu2h(*((zpuv*)(mem[0] + sp)));
                    printf("0x%08x <= 0x%08x\n", sp, st);
                    sp += ZPU_WORD_SIZE;
                    sc--;
                }
            }
        }
        if (max_cycles) {
            if (cycles >= max_cycles) break;
            if (!silent) {
                printf("\n");
            }
        } else {
            printf("Press enter for next cycle.\n");
            getchar();
        }
        cycles++;
    }
    free(mem[0]);
    return 0;
}
