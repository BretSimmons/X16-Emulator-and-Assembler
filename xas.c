#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <assert.h>
#include <math.h>
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
        return -1;
    } else {
        // Check if neg
        bool neg = false;
        if (imm[1] == '-'){
            neg = true;
        }
        // Convert the char to an int
        uint16_t num = atoi(imm + 1);
        // Find min and max values
        uint16_t minVal = -(1U << (bits - 1));
        uint16_t maxVal = (1U << (bits - 1)) - 1;
        // Check if the number is too big
        if (num < minVal || num > maxVal){
            return 0xFFFF;
        }
        // 2's compliment bit conversion
        for (int i = 0; i < bits - 1; i++){
            if (num & (1U << i) == 0){
                num |= (1U << i);
            }
        }
        // Mask the upper bits
        uint16_t mask = (1 << bits) -1;
        return (num & mask);

    }
}

void printError(void){
    fprintf(stderr, "Error");
    exit(2);
}

int main(int argc, char** argv) {
    if (argc != 2) {
        usage();
    }
    // Open the file in read
    FILE* fp = fopen(argv[1], "r");

    // Set variables for the line, and PC
    uint16_t PC = 0x3000;
    char line[100];

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
        // Check if the token is a label
        if (token[strlen(token) - 1] == ':'){
            // Remove the colon
            token[strlen(token) - 1] = '\0';
            // Use strdup to copy the string into memory
            labelArray[labelCount].name = strdup(token);
            labelArray[labelCount].address = PC;
            labelCount++;
        }
        while(token != NULL){
            printf("%s\n", token);
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
        // Check if the token is a label
        if (token[strlen(token) - 1] == ':'){
            // Labels don't need to be processed on the second pass,
            // so skip over them
            token = strtok(line, remove);
            // Anything after a label on a line, return error
            if (token != NULL){
                fprintf(stderr, "Error");
                exit(2);    
            }
            continue;
        }

        if (strcmp(token, "add") == 0){
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
                    result = emit_add_imm(dst, src1, imm);
                }
            } else {
                result = emit_add_reg(dst, src1, src2);
            }
        } else if (strcmp(token, "and") == 0){

        } else if (strcmp(token, "br") == 0){

        } else if (strcmp(token, "jmp") == 0){

        } else if (strcmp(token, "jsr") == 0){

        } else if (strcmp(token, "jsrr") == 0){

        } else if (strcmp(token, "ld") == 0){

        } else if (strcmp(token, "ldi") == 0){

        } else if (strcmp(token, "ldr") == 0){

        } else if (strcmp(token, "lea") == 0){

        } else if (strcmp(token, "not") == 0){

        } else if (strcmp(token, "st") == 0){

        } else if (strcmp(token, "sti") == 0){

        } else if (strcmp(token, "str") == 0){

        } else if (strcmp(token, "getc") == 0){

        } else if (strcmp(token, "putc") == 0){

        } else if (strcmp(token, "puts") == 0){

        } else if (strcmp(token, "enter") == 0){

        } else if (strcmp(token, "putsp") == 0){
        
        } else if (strcmp(token, "halt") == 0){
        
        // If the line doesn't have a label or start with an instruction
        // exit with error
        } else {
            printError(); 
        }
    }
    // Second pass
    fclose(fp);
    return 0;
}
