#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h> // for bool type

#define MAX_LINE_LEN 512
#define MAX_LINES 4096
#define MAX_LABELS 100

// Label representation
typedef struct {
    char name[64];
    int address;
} Label;

Label labels[MAX_LABELS];
int label_count = 0;
char cleaned_lines[MAX_LINES][MAX_LINE_LEN];
int cleaned_line_count = 0;

// Trim leading/trailing whitespace
void trim(char *str) {
    char *start = str;
    while (isspace((unsigned char)*start)) start++;
    if (start != str) memmove(str, start, strlen(start) + 1);
    char *end = str + strlen(str) - 1;
    while (end >= str && isspace((unsigned char)*end)) *end-- = '\0';
}

// Remove comments and trim
void preprocess_line(char *line) {
    char *comment = strchr(line, '#');
    if (comment) *comment = '\0';
    trim(line);
}

bool is_label(const char *line) {
    return strchr(line, ':') != NULL && !isspace((unsigned char)line[0]);
}

void add_label(const char *name, int address) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].name, name) == 0) return;
    }
    strcpy(labels[label_count].name, name);
    labels[label_count].address = address;
    label_count++;
}

int lookup_label(const char *name) {
    for (int i = 0; i < label_count; i++) {
        if (strcmp(labels[i].name, name) == 0)
            return labels[i].address;
    }
    fprintf(stderr, "Error: Undefined label %s\n", name);
    exit(1);
}

bool needs_bigimm(const char *imm) {
    if (isalpha((unsigned char)imm[0])) return true;
    long val = (strncmp(imm, "0x", 2) == 0 || strncmp(imm, "0X", 2) == 0)
               ? strtol(imm, NULL, 16)
               : strtol(imm, NULL, 10);
    return (val > 127 || val < -128);
}

int parse_imm(const char *token, bool *is_label_ref) {
    if (isalpha((unsigned char)token[0])) {
        *is_label_ref = true;
        return lookup_label(token);
    } else if (strncmp(token, "0x", 2) == 0 || strncmp(token, "0X", 2) == 0) {
        return (int)strtol(token, NULL, 16);
    } else {
        return atoi(token);
    }
}

int parse_register(const char *token) {
    if (token[0] != '$') return -1;
    struct { const char *name; int num; } regs[] = {
        {"$zero",0},{"$imm",1},{"$v0",2},{"$a0",3},{"$a1",4},{"$a2",5},{"$a3",6},
        {"$t0",7},{"$t1",8},{"$t2",9},{"$s0",10},{"$s1",11},{"$s2",12},{"$gp",13},
        {"$sp",14},{"$ra",15}
    };
    for (int i = 0; i < sizeof(regs)/sizeof(*regs); i++)
        if (strcmp(token, regs[i].name) == 0)
            return regs[i].num;
    return -1;
}

int get_opcode(const char *op) {
    const char *names[] = {"add","sub","mul","and","or","xor","sll","sra","srl",
                           "beq","bne","blt","bgt","ble","bge","jal","lw","sw",
                           "reti","in","out","halt"};
    for (int i = 0; i < 22; i++)
        if (strcmp(op, names[i]) == 0)
            return i;
    return -1;
}

// Write one or two words for an instruction
void encode_and_write_inst(FILE *fout, int opcode, int rd, int rs, int rt,
                           int imm, bool bigimm) {
    uint32_t word = (opcode & 0xFF) << 24  |
                    (rd     & 0xF ) << 20  |
                    (rs     & 0xF ) << 16  |
                    (rt     & 0xF ) << 12  |
                    (bigimm ? 1 : 0) <<  8  |
                    (!bigimm ? (imm & 0xFF) : 0);
    fprintf(fout, "%08X\n", word);
    if (bigimm) fprintf(fout, "%08X\n", (uint32_t)imm);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s prog.asm memin.txt\n", argv[0]);
        return 1;
    }

    FILE *fin = fopen(argv[1], "r");
    if (!fin) { perror("open asm"); return 1; }

    char line[MAX_LINE_LEN];
    int mem_address = 0;

    // Pass 1: record labels and measure space
    while (fgets(line, sizeof(line), fin)) {
        preprocess_line(line);
        if (!*line) continue;
        if (is_label(line)) {
            char *lbl = strtok(line, ":");
            trim(lbl);
            add_label(lbl, mem_address);
            continue;
        }
        strcpy(cleaned_lines[cleaned_line_count], line);
        if (strncmp(line, ".word", 5) == 0) {
            char addrstr[64], valstr[64];
            sscanf(line + 5, "%63s %63s", addrstr, valstr);
            bool dummy = false;
            int addr = parse_imm(addrstr, &dummy);
            mem_address = (addr >= mem_address ? addr + 1 : mem_address + 1);
        } else {
            char immstr[64];
            sscanf(line, "%*s %*[^,], %*[^,], %*[^,], %63s", immstr);
            mem_address += needs_bigimm(immstr) ? 2 : 1;
        }
        cleaned_line_count++;
    }
    fclose(fin);

    FILE *fout = fopen(argv[2], "w");
    if (!fout) { perror("open memin"); return 1; }

    int out_addr = 0;
    for (int i = 0; i < cleaned_line_count; i++) {
        char *ln = cleaned_lines[i];
        if (strncmp(ln, ".word", 5) == 0) {
            char addrstr[64], valstr[64];
            sscanf(ln + 5, "%63s %63s", addrstr, valstr);
            bool dummy = false;
            int addr = parse_imm(addrstr, &dummy);
            int val  = parse_imm(valstr, &dummy);
            while (out_addr < addr) {
                fprintf(fout, "00000000\n");
                out_addr++;
            }
            fprintf(fout, "%08X\n", (uint32_t)val);
            out_addr++;
        } else {
            char op[16], rd[16], rs[16], rt[16], immstr[64];
            sscanf(ln, "%15s %15[^,], %15[^,], %15[^,], %63s",
                   op, rd, rs, rt, immstr);
            bool label_ref = false;
            int imm = parse_imm(immstr, &label_ref);
            bool big = needs_bigimm(immstr);
            encode_and_write_inst(fout,
                get_opcode(op),
                parse_register(rd),
                parse_register(rs),
                parse_register(rt),
                imm, big);
            out_addr += big ? 2 : 1;
        }
    }

    fclose(fout);
    return 0;
}
