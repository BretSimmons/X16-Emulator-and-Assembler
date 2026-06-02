#include <stdio.h>
#include <stdlib.h>
#include "bits.h"
#include "control.h"
#include "instruction.h"
#include "x16.h"
#include "trap.h"
#include "decode.h"


// Update condition code based on result
void update_cond(x16_t* machine, reg_t reg) {
    uint16_t result = x16_reg(machine, reg);
    if (result == 0) {
        x16_set(machine, R_COND, FL_ZRO);
    } else if (is_negative(result)) {
        x16_set(machine, R_COND, FL_NEG);
    } else {
        x16_set(machine, R_COND, FL_POS);
    }
}



// Execute a single instruction in the given X16 machine. Update
// memory and registers as required. PC is advanced as appropriate.
// Return 0 on success, or -1 if an error or HALT is encountered.
int execute_instruction(x16_t* machine) {
    // Fetch the instruction and advance the program counter
    uint16_t pc = x16_pc(machine);
    uint16_t instruction = x16_memread(machine, pc);
    x16_set(machine, R_PC, pc + 1);

    if (LOG) {
        fprintf(LOGFP, "0x%x: %s\n", pc, decode(instruction));
    }

    // Variables we might need in various instructions
    reg_t dst, src1, src2, base;
    uint16_t result, indirect, offset, imm, cond, jsrflag, op1, op2;

    // Decode the instruction
    uint16_t opcode = getopcode(instruction);
    switch (opcode) {
        case OP_ADD:
            // Extract the SR1 and DR registers
            uint16_t SR1 = ((instruction >> 6) & 0b111);
            uint16_t DR = ((instruction >> 9) & 0b111);
            // Get the value from SR1
            uint16_t sr1Val = x16_reg(machine, SR1);
            uint16_t drVal = 0;
            // Check if bit 5 is 0
            if (((instruction >> 5) & 1) == 0){
                // Extract the SR2 register
                uint16_t SR2 = (instruction & (0b111));
                // Get the value from SR2
                uint16_t sr2Val = x16_reg(machine, SR2);
                drVal = sr1Val + sr2Val;

            } else {
                // Sign extend imm5
                uint16_t imm5 = (instruction & (0b11111));
                drVal = sr1Val + sign_extend(imm5, 5);
            }
            // Set the DR register
            x16_set(machine, DR, drVal);
            // Update the condition flags for the DR register
            update_cond(machine, DR);
            break;

        case OP_AND:
            break;

        case OP_NOT:
            break;

        case OP_BR:
            break;

        case OP_JMP:
            break;

        case OP_JSR:
            break;

        case OP_LD:
            break;

        case OP_LDI:
            break;

        case OP_LDR:
            break;

        case OP_LEA:
            break;

        case OP_ST:
            break;

        case OP_STI:
            break;

        case OP_STR:
            break;

        case OP_TRAP:
            // Execute the trap -- do not rewrite
            return trap(machine, instruction);

        case OP_RES:
        case OP_RTI:
        default:
            // Bad codes, never used
            abort();
    }

    return 0;
}
