#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include "instruction.h"


void usage() {
    fprintf(stderr, "Usage: ./xas file");
    exit(1);
}

// Create a struct to store labels and their locations
typedef struct {
    char* name;
    uint16_t address;
} label;

// Create an array of labels
label labelArray[100];
int labelCount = 0;

char* lowercase(char* token){
    // Switch all the characters of the token to lowercase
    for (int i = 0; token[i] != '\0'; i++){
        token[i] = tolower((unsigned char) token[i]);
    }
    return token;
}

reg_t getReg(char* reg){
    // Check if reg is actually a register, and is the right length
    reg_t ret = 0;
    if ((reg[0] != '%') || reg[1] != 'r' || (strlen(reg) != 3 )){
        return -1;
    }
    // Check if the reg number is valid
    int regNum = (reg[2] - '0');
    if (regNum > 7 || regNum < 0){
        return -1;
    }
    return (reg_t) regNum;
}

uint16_t getNum(char* imm, uint16_t bits){
    uint16_t num = 0;
    // Check that the number is actually a number
    if (imm[0] != '$'){
        return 0xFFFF;
    } else {
        // Convert the char to an int
        int num = atoi(imm + 1);
        // Find min and max values
        int minVal = -(1 << (bits - 1));
        int maxVal = (1 << (bits - 1)) - 1;
        // Check if the number is too big
        if (num < minVal || num > maxVal){
            return 0xFFFF;
        }
        // Mask the upper bits
        uint16_t mask = (1 << bits) -1;
        return (num & mask);
    }
}

uint16_t getLabel(char* labelName, uint16_t PC){
    // Iterate through the labelArray
    for (int i = 0; i < labelCount; i++){
        // Return the address if the names match
        if (strcmp(labelArray[i].name, labelName) == 0){
            return (labelArray[i].address - PC - 1);
        }
    }
    return 0xFFFF;
}

void printError(void){
    fprintf(stderr, "Error");
    exit(2);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        usage();
    }
    // Open the input file in read
    FILE* fp = fopen(argv[1], "r");

    // Set variables for the line, and PC
    uint16_t PC = 0x3000;
    char line[100];

    // Open the output file in write
    FILE* fpOut = fopen("a.obj", "wb");

    // Write the origin to the output file
    // Convert to big endian
    uint16_t orgAdr = htons(PC);
    fwrite(&orgAdr, sizeof(uint16_t), 1, fpOut);

    char* remove = " \t\n,";

    // First Pass,
    // Iterate through the lines in the file, store the names of all labels and
    // their addresses
    while (fgets(line, sizeof(line), fp) != NULL){
        // Search for comment character in line
        char* comment = strchr(line, '#');
        // If it exists, replace it with the null terminator
        if (comment != NULL){
            *comment = '\0';
        }
        // Next tokenize the line seperate the instructions and operands
        char* token = strtok(line, remove);
        // Skip empty lines and comments
        if (token == NULL){
            continue;
        }

        // Switch all the characters of the token to lowercase
        token = lowercase(token);

        // Check if the token is a label
        if (token[strlen(token) - 1] == ':'){
            // Remove the colon
            token[strlen(token) - 1] = '\0';
            // Use strdup to copy the string into memory
            labelArray[labelCount].name = strdup(token);
            labelArray[labelCount].address = PC;
            labelCount++;

            // Get the next token
            token = strtok(NULL, remove);
            // If there is nothing after the label, continue without
            // incrementing the PC
            if (token == NULL){
                continue;
            }
        }

        while (token != NULL){
            // Iterate through all tokens in the line
            token = strtok(NULL, remove);
        }
        // PC is incremented once per line (1 instruction per line)
        PC++;
    }
    // Rewind for second pass
    rewind(fp);
    // Reset PC value to origin
    PC = 0x3000;

    reg_t dst, src1, src2, base;
    uint16_t result, indirect, offset, imm, cond, jsrflag, op1, op2;

    // Second pass, convert the instructions
    while (fgets(line, sizeof(line), fp) != NULL){
        // Search for comment character in line
        char* comment = strchr(line, '#');
        // If it exists, replace it with the null terminator
        if (comment != NULL){
            *comment = '\0';
        }
        // Next tokenize the line seperate the instructions and operands
        char* token = strtok(line, remove);
        // Skip empty lines and comments
        if (token == NULL){
            continue;
        }
        // Switch all the characters of the token to lowercase
        token = lowercase(token);

        // Check if the token is a label
        if (token[strlen(token) - 1] == ':'){
            // Labels don't need to be processed on the second pass,
            // so skip over them
            token = strtok(NULL, remove);
            // If there is nothing after the label, continue without
            // incrementing the PC
            if (token == NULL){
                continue;
            }
        }
        
        // If token isn't a comment or label, it is an instruction or error
        char* inst = token;

        // Generate the 16 bit binary that corresponds to the line
        if ((strcmp(inst, "add") == 0) || (strcmp(inst, "and") == 0)){
            // Get the dst register
            token = strtok(NULL, remove);
            dst = getReg(token);
            if (dst == -1){
                printError();
            }
            // Get the src1 register
            token = strtok(NULL, remove);
            src1 = getReg(token);
            if (src1 == -1){
                printError();
            }
            // Get the src2 register/ imm5 value
            token = strtok(NULL, remove);
            src2 = getReg(token);
            if (src2 == -1){
                // If the value isn't a register, check if it is a number
                imm = getNum(token, 5);
                if (imm == 0xFFFF){
                    printError();
                } else {
                    if (strcmp(inst, "add") == 0){
                        result = emit_add_imm(dst, src1, imm);

                    } else if (strcmp(inst, "and") == 0){
                        result = emit_and_imm(dst, src1, imm);
                    }
                }
            } else {
                if (strcmp(inst, "add") == 0){
                    result = emit_add_reg(dst, src1, src2);

                } else if (strcmp(inst, "and") == 0){
                    result = emit_and_reg(dst, src1, src2);
                }
            }
            
        } else if (strncmp(inst, "br", 2) == 0){
            // Get the offset value
            token = lowercase(strtok(NULL, remove));
            offset = getLabel(token, PC);
            if (offset == 0xFFFF){
                printError();
            }
            
            if (strcmp(inst, "br") == 0){
                result = emit_br(0, 0, 0, offset);

            } else if (strcmp(inst, "brn") == 0){
                result = emit_br(1, 0, 0, (offset & 0x1FF));

            } else if (strcmp(inst, "brp") == 0){
                result = emit_br(0, 0, 1, offset);

            } else if (strcmp(inst, "brz") == 0){
                result = emit_br(0, 1, 0, offset);

            } else if (strcmp(inst, "brzp") == 0){
                result = emit_br(0, 1, 1, offset);

            } else if (strcmp(inst, "brnp") == 0){
                result = emit_br(1, 0, 1, offset);

            } else if (strcmp(inst, "brnz") == 0){
                result = emit_br(1, 1, 0, offset);

            } else if (strcmp(inst, "brnzp") == 0){
                result = emit_br(1, 1, 1, offset);
            }

        } else if (strcmp(token, "ret") == 0){
            result = emit_jmp((reg_t) 7);
        
        } else if ((strcmp(token, "jmp") == 0) || (strcmp(token, "jsrr") == 0)){
            // Get the base register
            token = lowercase(strtok(NULL, remove));
            base = getReg(token);
            if (dst == -1){
                printError();
            }
            if (strcmp(inst, "jmp") == 0){
                result = emit_jmp(base);
            } else if (strcmp(inst, "jsrr") == 0){
                result = emit_jsrr(base);    
            }

        } else if (strcmp(token, "jsr") == 0){
            // Get the offset
            token = lowercase(strtok(NULL, remove));
            offset = getLabel(token, PC);
            if (offset == 0xFFFF){
                printError();
            }
            result = emit_jsr(offset);

        } else if ((strcmp(token, "ld") == 0) || (strcmp(token, "ldi") == 0) ||
        (strcmp(token, "lea") == 0) || (strcmp(token, "ldr") == 0) ||
        (strcmp(token, "not") == 0)){
            // Get the dst register
            token = lowercase(strtok(NULL, remove));
            dst = getReg(token);
            if (dst == -1){
                printError();
            }

            if (strcmp(inst, "not") == 0){
                // Get the src1 register
                token = lowercase(strtok(NULL, remove));
                src1 = getReg(token);
                if (src1 == -1){
                    printError();
                }
                result = emit_not(dst, src1);

            } else if (strcmp(inst, "ldr") == 0){
                // Get the base register
                token = lowercase(strtok(NULL, remove));
                base = getReg(token);
                if (base == -1){
                    printError();
                }
                // Get the offset value
                token = lowercase(strtok(NULL, remove));
                offset = getNum(token, 6);
                if (offset == 0xFFFF){
                    printError();
                }
                result = emit_ldr(dst, base, offset);

            } else {
            // Handles ld, ldi, lea
                // Get the offset
                token = lowercase(strtok(NULL, remove));
                offset = getLabel(token, PC);
                if (offset == 0xFFFF){
                    printError();
                }
                if (strcmp(inst, "ld") == 0){
                    result = emit_ld(dst, offset);

                } else if (strcmp(inst, "ldi") == 0){
                    result = emit_ldi(dst, offset);  

                } else if (strcmp(inst, "lea") == 0){
                    result = emit_lea(dst, offset);    
                }
            }

        } else if ((strcmp(token, "st") == 0) || (strcmp(token, "sti") == 0) ||
        (strcmp(token, "str") == 0 )){
            // Get the src1 register
            token = lowercase(strtok(NULL, remove));
            src1 = getReg(token);
            if (src1 == -1){
                printError();
            }

            if (strcmp(inst, "str") == 0){
                // Get the base register
                token = lowercase(strtok(NULL, remove));
                base = getReg(token);
                if (base == -1){
                    printError();
                }
                // Get the offset value
                token = lowercase(strtok(NULL, remove));
                offset = getNum(token, 6);
                if (offset == 0xFFFF){
                    printError();
                }
                result = emit_str(src1, base, offset);

            } else {
            // Handles st, sti
                // Get the offset
                token = lowercase(strtok(NULL, remove));
                offset = getLabel(token, PC);
                if (offset == 0xFFFF){
                    printError();
                }
                if (strcmp(inst, "st") == 0 ){
                    result = emit_st(src1, offset);

                } else if (strcmp(inst, "sti") == 0){
                    result = emit_sti(src1, offset);
                }
            }

        } else if (strcmp(token, "getc") == 0){
            result = emit_trap(TRAP_GETC);

        } else if (strcmp(token, "putc") == 0){
            result = emit_trap(TRAP_OUT);
            
        } else if (strcmp(token, "puts") == 0){
            result = emit_trap(TRAP_PUTS);

        } else if (strcmp(token, "enter") == 0){
            result = emit_trap(TRAP_IN);

        } else if (strcmp(token, "putsp") == 0){
            result = emit_trap(TRAP_PUTSP);

        } else if (strcmp(token, "halt") == 0){
            result = emit_trap(TRAP_HALT);

        } else if (strcmp(token, "val") == 0){
            // Get the numbers
            token = lowercase(strtok(NULL, remove));
            // Check that the number is actually a number
            if (token[0] != '$'){
                printError();
            } else {
            // Convert the char to an int
            int num = atoi(token + 1);
            result = emit_value(num);
        }

        // If the line doesn't have a label or start with an instruction
        // exit with error
        } else {
            printError();
        }

        // Convert result to big endian and write to the output file
        uint16_t bigResult = htons(result);
        fwrite(&bigResult, sizeof(uint16_t), 1, fpOut);

        // PC is incremented once per line (1 instruction per line)
        PC++;
    }
    // Second pass
    fclose(fp);
    return 0;
}
