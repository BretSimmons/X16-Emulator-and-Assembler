#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <assert.h>
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
            token = strtoki(line, remove);
            // Anything after a label on a line, return error
            if (token != NULL){
                fprintf(stderr, "Error");
                exit(2);    
            }
            continue;
        }

        if (stringcmp(token, "add") == 0){

        } else if (stringcmp(token, "and") == 0){

        } else if (stringcmp(token, "br") == 0){

        } else if (stringcmp(token, "jmp") == 0){

        } else if (stringcmp(token, "jsr") == 0){

        } else if (stringcmp(token, "jsrr") == 0){

        } else if (stringcmp(token, "ld") == 0){

        } else if (stringcmp(token, "ldi") == 0){

        } else if (stringcmp(token, "ldr") == 0){

        } else if (stringcmp(token, "lea") == 0){

        } else if (stringcmp(token, "not") == 0){

        } else if (stringcmp(token, "st") == 0){

        } else if (stringcmp(token, "sti") == 0){

        } else if (stringcmp(token, "str") == 0){

        } else if (stringcmp(token, "getc") == 0){

        } else if (stringcmp(token, "putc") == 0){

        } else if (stringcmp(token, "puts") == 0){

        } else if (stringcmp(token, "enter") == 0){

        } else if (stringcmp(token, "putsp") == 0){
        
        } else if (stringcmp(token, "halt") == 0){
        
        // If the line doesn't have a label or start with an instruction
        // exit with error
        } else {
            fprintf(stderr, "Error");
            exit(2);
        }
    }

    // Second pass
    fclose(fp);
    return 0;
}
