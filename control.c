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
    reg_t src1, src2, dr, dst, base;
    uint16_t val, mem, imm5, offset6, PCoffset9, PCoffset11, cond,
    src1Val, src2Val, drVal, baseVal, baseMem;

    // Decode the instruction
    uint16_t opcode = getopcode(instruction);
    switch (opcode) {
        case OP_ADD:
            // Extract the src1 and dr registers
            src1 = getbits(instruction, 6, 3);
            dr = getbits(instruction, 9, 3);
            // Get the value from src1
            src1Val = x16_reg(machine, src1);
            drVal = 0;
            // Check if bit 5 is 0
            if (getbit(instruction, 5) == 0){
                // Extract the src2 register
                src2 = getbits(instruction, 0, 3);
                // Get the value from src2
                src2Val = x16_reg(machine, src2);
                drVal = src1Val + src2Val;

            } else {
                // Extract and sign extend imm5
                imm5 = getbits(instruction, 0, 5);
                drVal = src1Val + sign_extend(imm5, 5);
            }
            // Set the dr register
            x16_set(machine, dr, drVal);
            // Update the condition flags for the dr register
            update_cond(machine, dr);
            break;

        case OP_AND:
            // Extract the src1 and dr registers
            src1 = getbits(instruction, 6, 3);
            dr = getbits(instruction, 9, 3);
            // Get the value from src1
            src1Val = x16_reg(machine, src1);
            drVal = 0;
            if ((getbit(instruction, 5) & 1) == 0){
                // Extract the src2 register
                src2 = getbits(instruction, 0, 3);
                // Get the value from src2
                src2Val = x16_reg(machine, src2);
                drVal = src1Val & src2Val;
            } else{
                // Extract and sign extend imm5
                imm5 = getbits(instruction, 0, 5);
                drVal = src1Val & sign_extend(imm5, 5);
            }
            // Set the dr register
            x16_set(machine, dr, drVal);
            // Update the condition flags for the dr register
            update_cond(machine, dr);
            break;

        case OP_NOT:
            // Extract the src1 and dr registers
            src1 = getbits(instruction, 6, 3);
            dr = getbits(instruction, 9, 3);
            // Get the value from src1
            src1Val = x16_reg(machine, src1);
            drVal = ~src1Val;
            // Set the dr register
            x16_set(machine, dr, drVal);
            // Update the condition flags for the dr register
            update_cond(machine, dr);
            break;

        case OP_BR:
            // Exract n, z, and p
            uint16_t n = getbit(instruction, 11);
            uint16_t z = getbit(instruction, 10);
            uint16_t p = getbit(instruction, 9);

            // Get the condition flag
            cond = x16_cond(machine);
            // Check if any of the bits match the condition flag
            if ((n && (cond == FL_NEG))
            || (z && (cond == FL_ZRO))
            || (p && (cond == FL_POS)
            // Also branch if no condition flags are set
            || ((n + z + p) == 0))){
                // Extract the offset and sign extend it
                PCoffset9 = getbits(instruction, 0, 9);
                PCoffset9 = sign_extend(PCoffset9, 9);
                // Get the pc counter
                pc = x16_reg(machine, R_PC);
                // Add the offset to the pc
                x16_set(machine, R_PC, pc + PCoffset9);
            }
            break;

        case OP_JMP:
            // Extract the base register
            base = getbits(instruction, 6, 3);
            // JMP case
            if (base != 0b111){
                // Get the value from base
                baseVal = x16_reg(machine, base);
                // Set the pc to the base register
                x16_set(machine, R_PC, baseVal);
            // RET case
            } else{
                // Retrieve the return address from r7
                dst = x16_reg(machine, R_R7);
                // Set the PC to the return address
                x16_set(machine, R_PC, dst);
            }
            break;

        case OP_JSR:
            // Save the increment pc to r7
            pc = x16_reg(machine, R_PC);
            x16_set(machine, R_R7, pc);
            // JSR
            if (getbit(instruction, 11)){
                // Extract PCoffset11
                PCoffset11 = getbits(instruction, 0, 11);
                // Sign extend PCoffset11
                PCoffset11 = sign_extend(PCoffset11, 11);
                // Add value to the incremented pc
                x16_set(machine, R_PC, pc + PCoffset11);
            // JSRR
            } else {
                // Extract the base register
                base = getbits(instruction, 6, 3);
                // Get the value from the base register
                baseVal = x16_reg(machine, base);
                // Load into the pc
                x16_set(machine, R_PC, baseVal);
            }
            break;

        case OP_LD:
            // Extract PCoffset9 and sign extend it
            PCoffset9 = getbits(instruction, 0, 9);
            PCoffset9 = sign_extend(PCoffset9, 9);
            // Get the pc and add the offset
            pc = x16_reg(machine, R_PC);
            pc += PCoffset9;
            // Get the contents of memory at this address
            val = x16_memread(machine, pc);
            // Extract dr register
            dr = getbits(instruction, 9, 3);
            // Load into dr
            x16_set(machine, dr, val);
            // Set the condition codes based on val
            update_cond(machine, dr);
            break;

        case OP_LDI:
            // Extract PCoffset9 and sign extend it
            PCoffset9 = getbits(instruction, 0, 9);
            PCoffset9 = sign_extend(PCoffset9, 9);
            // Get the pc and add the offset
            pc = x16_reg(machine, R_PC);
            pc += PCoffset9;
            // Get the memory stored at this point in memory
            mem = x16_memread(machine, pc);
            // Get the val stored indirectly in memory
            val = x16_memread(machine, mem);
            // Extract dr register
            dr = getbits(instruction, 9, 3);
            // Load into dr
            x16_set(machine, dr, val);
            // Set the condition codes based on val
            update_cond(machine, dr);
            break;

        case OP_LDR:
            // Extract offset6 and sign extend it
            offset6 = getbits(instruction, 0, 6);
            offset6 = sign_extend(offset6, 6);
            // Extract base and the address stored there, then add offset
            base = getbits(instruction, 6, 3);
            baseMem = x16_reg(machine, base);
            baseMem += offset6;
            // Get the value from the memory address
            baseVal = x16_memread(machine, baseMem);
            // Extract dr
            dr = getbits(instruction, 9, 3);
            // Load the value into dr
            x16_set(machine, dr, baseVal);
            // Set the condition codes based on baseVal
            update_cond(machine, dr);
            break;

        case OP_LEA:
            // Extract PCoffset9 and sign extend it
            PCoffset9 = getbits(instruction, 0, 9);
            PCoffset9 = sign_extend(PCoffset9, 9);
            // Get the pc and add PCoffset9
            pc = x16_reg(machine, R_PC);
            pc += PCoffset9;
            // Extract dr
            dr = getbits(instruction, 9, 3);
            // Load address into dr
            x16_set(machine, dr, pc);
            // Set the condition codes based on pcVal
            update_cond(machine, dr);
            break;

        case OP_ST:
            // Extract sr and its value
            src1 = getbits(instruction, 9, 3);
            src1Val = x16_reg(machine, src1);
            // Extract PCoffset9 and sign extend it
            PCoffset9 = getbits(instruction, 0, 9);
            PCoffset9 = sign_extend(PCoffset9, 9);
            // Get the pc and add PCoffset9
            pc = x16_reg(machine, R_PC);
            pc += PCoffset9;
            // Write sr val to the memory location in pc
            x16_memwrite(machine, pc, src1Val);
            break;

        case OP_STI:
            // Extract sr and its value
            src1 = getbits(instruction, 9, 3);
            src1Val = x16_reg(machine, src1);
            // Extract PCoffset9 and sign extend it
            PCoffset9 = getbits(instruction, 0, 9);
            PCoffset9 = sign_extend(PCoffset9, 9);
            // Get the pc and add PCoffset9
            pc = x16_reg(machine, R_PC);
            pc += PCoffset9;
            // Get the address stored at the address contained in the pc
            mem = x16_memread(machine, pc);
            // Write sr val to the memory location in PC
            x16_memwrite(machine, mem, src1Val);
            break;

        case OP_STR:
            // Extract sr and its value
            src1 = getbits(instruction, 9, 3);
            src1Val = x16_reg(machine, src1);
            // Extract base and its value
            base = getbits(instruction, 6, 3);
            baseVal = x16_reg(machine, base);
            // Extract offset6 and sign extend it
            offset6 = getbits(instruction, 0, 6);
            offset6 = sign_extend(offset6, 6);
            // Add offset6 to base
            baseVal += offset6;
            // Write sr val to the memory location in pc
            x16_memwrite(machine, baseVal, src1Val);
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
