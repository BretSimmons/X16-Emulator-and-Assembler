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
                // Extract and sign extend imm5
                uint16_t imm5 = (instruction & (0b11111));
                drVal = sr1Val + sign_extend(imm5, 5);
            }
            // Set the DR register
            x16_set(machine, DR, drVal);
            // Update the condition flags for the DR register
            update_cond(machine, DR);
            break;

        case OP_AND:
            // Extract the SR1 and DR registers
            SR1 = getbits(instruction, 6, 3);
            DR = getbits(instruction, 9, 3);
            // Get the value from SR1
            sr1Val = x16_reg(machine, SR1);
            drVal = 0;
            if (((instruction >> 5) & 1) == 0){
                // Extract the SR2 register
                uint16_t SR2 = (instruction & 0b111);
                // Get the value from SR2
                uint16_t sr2Val = x16_reg(machine, SR2);
                drVal = sr1Val & sr2Val;
            } else{
                // Extract and sign extend imm5
                uint16_t imm5 = (instruction & (0b11111));
                drVal = sr1Val & sign_extend(imm5, 5);
            }
            // Set the DR register
            x16_set(machine, DR, drVal);
            // Update the condition flags for the DR register
            update_cond(machine, DR);
            break;

        case OP_NOT:
            // Extract the SR and DR registers
            uint16_t SR = getbits(instruction, 6, 3);
            DR = getbits(instruction, 9, 3);
            // Get the value from the SR
            uint16_t srVal = x16_reg(machine, SR);
            drVal = ~srVal;
            // Set the DR register
            x16_set(machine, DR, drVal);
            // Update the condition flags for the DR register
            update_cond(machine, DR);
            break;

        case OP_BR:
            // Exract n, z, and p
            uint16_t n = ((instruction >> 11) & 1);
            uint16_t z = ((instruction >> 10) & 1);
            uint16_t p = ((instruction >> 9) & 1);

            // Get the condition flag
            uint16_t flag = x16_cond(machine);
            // Check if any of the bits match the condition flag
            if ((n && (flag == FL_NEG)) 
            || (z && (flag == FL_ZRO))
            || (p && (flag == FL_POS)
            // Also branch if no condition flags are set
            || ((n + z + p) == 0))){
                // Extract the offset and sign extend it
                uint16_t PCoffset9 = (instruction & 0x1FF);
                PCoffset9 = sign_extend(PCoffset9, 9);
                // Get the PC counter
                uint16_t curPC = x16_reg(machine, R_PC);
                // Add the offset to the PC
                x16_set(machine, R_PC, curPC + PCoffset9);
            }
            break;

        case OP_JMP:
            // Extract the BaseR register
            uint16_t BaseR = ((instruction >> 6)& 0b111);
            // JMP case
            if (BaseR != 0b111){
                // Get the value from BaseR
                uint16_t baseRVal = x16_reg(machine, BaseR);
                // Set the PC to the base register
                x16_set(machine, R_PC, baseRVal);
            // RET case
            } else{
                // Retrieve the return address from r7
                uint16_t retADR = x16_reg(machine, R_R7);
                // Set the PC to the return address
                x16_set(machine, R_PC, retADR);
            }
            break;

        case OP_JSR:
            // Save the increment PC to r7
            uint16_t curPC = x16_reg(machine, R_PC);
            x16_set(machine, R_R7, curPC);
            // JSR
            if ((instruction >> 11) & 1){
                // Extract PCoffset11
                uint16_t PCoffset11 = (instruction & 0x7FF);
                // Sign extend PCoffset11
                PCoffset11 = sign_extend(PCoffset11, 11);
                // Add value to the incremented PC
                x16_set(machine, R_PC, curPC + PCoffset11);
            // JSRR
            } else {
                // Extract the base register
                uint16_t BaseR = ((instruction >> 6) & 0b111);
                // Get the value from the base register
                uint16_t baseRVal = x16_reg(machine, BaseR);
                // Load into the PC
                x16_set(machine, R_PC, baseRVal);
            }
            break;

        case OP_LD:
            // Extract PCoffset9 and sign extend it
            uint16_t PCoffset9 = getbits(instruction, 0, 9);
            PCoffset9 = sign_extend(PCoffset9, 9);
            // Get the PC and add the offset
            uint16_t PC = x16_reg(machine, R_PC);
            PC += PCoffset9;
            // Get the contents of memory at this address
            uint16_t val = x16_memread(machine, PC);
            // Extract DR register
            DR = getbits(instruction, 9, 3);
            // Load into DR
            x16_set(machine, DR, val);
            // Set the condition codes based on val
            update_cond(machine, DR);
            break;

        case OP_LDI:
            // Extract PCoffset9 and sign extend it
            PCoffset9 = getbits(instruction, 0, 9);
            PCoffset9 = sign_extend(PCoffset9, 9);
            // Get the PC and add the offset
            PC = x16_reg(machine, R_PC);
            PC += PCoffset9;
            // Get the memory stored at this point in memory
            uint16_t mem = x16_memread(machine, PC);
            // Get hte val stored indirectly in memory
            val = x16_memread(machine, mem);
            // Extract DR register
            DR = getbits(instruction, 9, 3);
            // Load into DR
            x16_set(machine, DR, val);
            // Set the condition codes based on val
            update_cond(machine, DR);
            break;

        case OP_LDR:
            // Extract offset6 and sign extend it
            uint16_t offset6 = getbits(instruction, 0, 6);
            offset6 = sign_extend(offset6, 6);
            // Extract BaseR and the address stored there, then add offset
            BaseR = getbits(instruction, 6, 3);
            uint16_t BaseRMem = x16_reg(machine, BaseR);
            BaseRMem += offset6;
            // Get the value from the memory address
            uint16_t BaseRVal = x16_memread(machine, BaseRMem);
            // Extract DR
            DR = getbits(instruction, 9, 3);
            // Load the value into DR
            x16_set(machine, DR, BaseRVal);
            // Set the condition codes based on BaseRval
            update_cond(machine, DR);
            break;

        case OP_LEA:
            // Extract PCoffset9 and sign extend it
            PCoffset9 = getbits(instruction, 0, 9);
            PCoffset9 = sign_extend(PCoffset9, 9);
            // Get the PC and add PCoffset9
            PC = x16_reg(machine, R_PC);
            PC += PCoffset9;
            // Extract DR
            DR = getbits(instruction, 9, 3);
            // Load address into DR
            x16_set(machine, DR, PC);
            // Set the condition codes based on pcVal
            update_cond(machine, DR);
            break;

        case OP_ST:
            // Extract SR and its value
            SR = getbits(instruction, 9, 3);
            srVal = x16_reg(machine, SR);
            // Extract PCoffset9 and sign extend it
            PCoffset9 = getbits(instruction, 0, 9);
            PCoffset9 = sign_extend(PCoffset9, 9);
            // Get the PC and add PCoffset9
            PC = x16_reg(machine, R_PC);
            PC += PCoffset9;
            // Write SR val to the memory location in PC
            x16_memwrite(machine, PC, srVal);
            break;

        case OP_STI:
            // Extract SR and its value
            SR = getbits(instruction, 9, 3);
            srVal = x16_reg(machine, SR);
            // Extract PCoffset9 and sign extend it
            PCoffset9 = getbits(instruction, 0, 9);
            PCoffset9 = sign_extend(PCoffset9, 9);
            // Get the PC and add PCoffset9
            PC = x16_reg(machine, R_PC);
            PC += PCoffset9;
            // Get the address stoerd at the address contained in the PC
            uint16_t memAdr = x16_memread(machine, PC);
            // Write SR val to the memory location in PC
            x16_memwrite(machine, memAdr, srVal);
            break;

        case OP_STR:
            // Extract SR and its value
            SR = getbits(instruction, 9, 3);
            srVal = x16_reg(machine, SR);
            // Extract BaseR and its value
            BaseR = getbits(instruction, 6, 3);
            BaseRVal = x16_reg(machine, BaseR);
            // Extract offset6 and sign extend it
            offset6 = getbits(instruction, 0, 6);
            offset6 = sign_extend(offset6, 6);
            // Add offset6 to BaseR
            BaseRVal += offset6;
            // Write SR val to the memory location in PC
            x16_memwrite(machine, BaseRVal, srVal);
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
