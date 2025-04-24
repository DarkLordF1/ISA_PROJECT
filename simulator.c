// simulator.c — Full SIMP Simulator Implementation
// Based on the SIMP ISA spec from the course PDF

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define MEM_SIZE     4096        // words
#define IOREG_COUNT  23
#define MAX_IRQ2     10000       // max irq2 events

// I/O register indices
enum {
    IO_IRQ0ENABLE=0, IO_IRQ1ENABLE, IO_IRQ2ENABLE,
    IO_IRQ0STATUS, IO_IRQ1STATUS, IO_IRQ2STATUS,
    IO_IRQHANDLER, IO_IRQRETURN,
    IO_CLKS, IO_LEDS,
    IO_DISPLAY7SEG, IO_TIMERENABLE, IO_TIMERCURRENT, IO_TIMERMAX,
    IO_DISKCMD, IO_DISKSECTOR, IO_DISKBUFFER, IO_DISKSTATUS,
    IO_MONITORADDR, IO_MONITORDATA, IO_MONITORCMD
};

// Global state
static uint32_t memory[MEM_SIZE];
static uint32_t io_regs[IOREG_COUNT];
static uint32_t regs[16];
static uint32_t disk[128*128];      // support at least 4 sectors (4*128 words)
static uint32_t irq2_times[MAX_IRQ2];
static int irq2_count = 0;
static uint32_t pc = 0;             // program counter

// Output files
static FILE *f_memout, *f_regout, *f_trace, *f_hwtrace;
static FILE *f_cycles, *f_leds, *f_7seg, *f_diskout, *f_monitor;

// Helper: load hex words from text file into buffer
static void load_hex(const char *path, uint32_t *buf, int max) {
    FILE *f = fopen(path, "r");
    if (!f) { perror(path); exit(1); }
    for (int i = 0; i < max; i++) {
        if (fscanf(f, "%x", &buf[i]) != 1) break;
    }
    fclose(f);
}

// Map IO index to name for hwtrace
static const char *io_name(int idx) {
    switch(idx) {
        case IO_IRQ0ENABLE: return "irq0enable";
        case IO_IRQ1ENABLE: return "irq1enable";
        case IO_IRQ2ENABLE: return "irq2enable";
        case IO_IRQ0STATUS:return "irq0status";
        case IO_IRQ1STATUS:return "irq1status";
        case IO_IRQ2STATUS:return "irq2status";
        case IO_IRQHANDLER:return "irqhandler";
        case IO_IRQRETURN:return "irqreturn";
        case IO_CLKS:return "clks";
        case IO_LEDS:return "leds";
        case IO_DISPLAY7SEG:return "display7seg";
        case IO_TIMERENABLE:return "timerenable";
        case IO_TIMERCURRENT:return "timercurrent";
        case IO_TIMERMAX:return "timermax";
        case IO_DISKCMD:return "diskcmd";
        case IO_DISKSECTOR:return "disksector";
        case IO_DISKBUFFER:return "diskbuffer";
        case IO_DISKSTATUS:return "diskstatus";
        case IO_MONITORADDR:return "monitoraddr";
        case IO_MONITORDATA:return "monitordata";
        case IO_MONITORCMD:return "monitorcmd";
        default: return "unknown";
    }
}

int main(int argc, char **argv) {
    if (argc != 14) {
        fprintf(stderr, "Usage: %s memin.txt diskin.txt irq2in.txt memout.txt regout.txt trace.txt hwregtrace.txt cycles.txt leds.txt display7seg.txt diskout.txt monitor.txt monitor.yuv\n", argv[0]);
        return 1;
    }
    // 1) Open all output files
    f_memout  = fopen(argv[4], "w");
    f_regout  = fopen(argv[5], "w");
    f_trace   = fopen(argv[6], "w");
    f_hwtrace = fopen(argv[7], "w");
    f_cycles  = fopen(argv[8], "w");
    f_leds    = fopen(argv[9], "w");
    f_7seg    = fopen(argv[10], "w");
    f_diskout = fopen(argv[11], "w");
    f_monitor = fopen(argv[12], "w");
    if (!f_memout||!f_regout||!f_trace||!f_hwtrace||!f_cycles||!f_leds||!f_7seg||!f_diskout||!f_monitor) {
        perror("opening output files"); exit(1);
    }

    // 2) Load initial memory, disk, irq2 event times
    load_hex(argv[1], memory, MEM_SIZE);
    load_hex(argv[2], disk, 128*4);
    FILE *f_irq2 = fopen(argv[3], "r");
    if (!f_irq2) { perror(argv[3]); exit(1);}    
    while (irq2_count < MAX_IRQ2 && fscanf(f_irq2, "%u", &irq2_times[irq2_count])==1) irq2_count++;
    fclose(f_irq2);

    // 3) Clear registers and IO
    memset(regs, 0, sizeof(regs));
    memset(io_regs,0,sizeof(io_regs));

    uint64_t cycle = 0;
    size_t irq2_idx = 0;
    bool in_irq = false;

    // 4) Main loop
    while (1) {
        if (cycle < 20) {
            fprintf(stderr, "DEBUG cycle=%llu, PC=%u\\n", cycle, pc);
        }
    
        // A) Increment clock
        io_regs[IO_CLKS] = (uint32_t)cycle;
        fprintf(f_cycles, "%08X\n", (uint32_t)cycle);

        // B) Timer
        if (io_regs[IO_TIMERENABLE]) {
            io_regs[IO_TIMERCURRENT]++;
            if (io_regs[IO_TIMERCURRENT] == io_regs[IO_TIMERMAX]) {
                io_regs[IO_TIMERCURRENT] = 0;
                io_regs[IO_IRQ0STATUS] = 1;
            }
        }
        // C) External irq2
        if (irq2_idx < irq2_count && irq2_times[irq2_idx] == cycle) {
            io_regs[IO_IRQ2STATUS] = 1;
            irq2_idx++;
        }
        // D) LED & 7seg trace
        fprintf(f_leds, "%08X %08X\n", (uint32_t)cycle, io_regs[IO_LEDS]);
        fprintf(f_7seg, "%08X %08X\n",(uint32_t)cycle, io_regs[IO_DISPLAY7SEG]);

        // E) Interrupt check
        uint32_t irq_mask = (io_regs[IO_IRQ0ENABLE]&io_regs[IO_IRQ0STATUS]) |
                             (io_regs[IO_IRQ1ENABLE]&io_regs[IO_IRQ1STATUS]) |
                             (io_regs[IO_IRQ2ENABLE]&io_regs[IO_IRQ2STATUS]);
        if (irq_mask && !in_irq) {
            io_regs[IO_IRQRETURN] = (pc + 1);
            pc = io_regs[IO_IRQHANDLER];
            in_irq = true;
            cycle++;
            continue;
        }

        // F) Fetch instruction
        uint32_t word1 = memory[pc];
        uint8_t opcode = (word1 >> 24) & 0xFF;
        uint8_t rd     = (word1 >> 20) & 0x0F;
        uint8_t rs     = (word1 >> 16) & 0x0F;
        uint8_t rt     = (word1 >> 12) & 0x0F;
        bool    bigimm = ((word1 >> 8) & 1) != 0;
        int32_t imm;
        int     inst_size = bigimm ? 2 : 1;
        if (bigimm) {
            imm = (int32_t)memory[pc+1];
        } else {
            imm = (int8_t)(word1 & 0xFF);
        }
        uint32_t next_pc = pc + inst_size;

        // G) Execute
        uint32_t hw_data;
        switch (opcode) {
            // ALU
            case 0: regs[rd] = regs[rs] + regs[rt]; break; // add
            case 1: regs[rd] = regs[rs] - regs[rt]; break; // sub
            case 2: regs[rd] = regs[rs] * regs[rt]; break; // mul
            case 3: regs[rd] = regs[rs] & regs[rt]; break; // and
            case 4: regs[rd] = regs[rs] | regs[rt]; break; // or
            case 5: regs[rd] = regs[rs] ^ regs[rt]; break; // xor
            case 6: regs[rd] = regs[rs] << regs[rt]; break; // sll
            case 7: regs[rd] = (int32_t)regs[rs] >> regs[rt]; break; // sra
            case 8: regs[rd] = regs[rs] >> regs[rt]; break; // srl
            // branches
            case 9: if (regs[rs] == regs[rt]) { pc = regs[rd]; inst_size=0; } break; // beq
            case 10: if (regs[rs] != regs[rt]){ pc = regs[rd]; inst_size=0; } break; // bne
            case 11: if ((int32_t)regs[rs] < (int32_t)regs[rt]){ pc = regs[rd]; inst_size=0;} break; // blt
            case 12: if ((int32_t)regs[rs] > (int32_t)regs[rt]){ pc = regs[rd]; inst_size=0;} break; // bgt
            case 13: if ((int32_t)regs[rs] <= (int32_t)regs[rt]){ pc = regs[rd]; inst_size=0;} break; // ble
            case 14: if ((int32_t)regs[rs] >= (int32_t)regs[rt]){ pc = regs[rd]; inst_size=0;} break; // bge
            // jal
            case 15:
                regs[rd] = next_pc;
                pc = regs[rs];
                inst_size = 0;
                break;
            // lw/sw
            case 16:
                regs[rd] = memory[ regs[rs] + regs[rt] + imm ];
                break;
            case 17:
                memory[ regs[rs] + regs[rt] + imm ] = regs[rd];
                break;
            // reti
            case 18:
                pc = io_regs[IO_IRQRETURN];
                in_irq = false;
                inst_size = 0;
                break;
            // in
            case 19:
                hw_data = io_regs[ regs[rs] + regs[rt] + imm ];
                regs[rd] = hw_data;
                fprintf(f_hwtrace, "%08X READ  %s %08X\n", (uint32_t)cycle,
                        io_name(regs[rs]+regs[rt]+imm), hw_data);
                break;
            // out
            case 20:
                io_regs[ regs[rs] + regs[rt] + imm ] = regs[rd];
                hw_data = regs[rd];
                fprintf(f_hwtrace, "%08X WRITE %s %08X\n", (uint32_t)cycle,
                        io_name(regs[rs]+regs[rt]+imm), hw_data);
                break;
            // halt
            case 21:
                goto done;
            default:
                fprintf(stderr, "Unknown opcode %u at PC=%u\n", opcode, pc);
                exit(1);
        }

        // H) Instruction trace: CYCLE PC INST R0…R15
        fprintf(f_trace, "%u %u %u %08X", (uint32_t)cycle, pc, opcode, word1);
        for (int r = 0; r < 16; r++) fprintf(f_trace, " %08X", regs[r]);
        fprintf(f_trace, "\n");

        // Advance PC
        pc += inst_size;
        cycle++;
        if (cycle > 1000000) {
            fprintf(stderr, "ERROR: cycle limit exceeded\n");
            break;
        }
    
    }

done:
    // 5) Write memory and regs outputs
    for (int i = 0; i < MEM_SIZE; i++) fprintf(f_memout, "%08X\n", memory[i]);
    for (int r = 0; r < 16;    r++) fprintf(f_regout, "%08X\n", regs[r]);

    // 6) Dump final disk state (sector4..)
    for (int i = 0; i < 128*4; i++) fprintf(f_diskout, "%08X\n", disk[i]);

    // 7) Monitor and yuv output left as exercise (use monitoraddr/monitordata log)

    return 0;
}
