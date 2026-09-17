#ifndef OPCODES_HPP
#define OPCODES_HPP

/*
 * Raw 4-bit OC (opcode) and MOD (modifier) values from the fixed 4-byte
 * instruction encoding: OC[7:4] MOD[3:0] | RegA[7:4] RegB[3:0] |
 * RegC[7:4] Disp[11:8] | Disp[7:0]. These are the machine-level values
 * that go straight into Section::appendInstruction, as distinct from the
 * asm_types.h OpCode enum (which is the assembler-mnemonic level, e.g.
 * OPC_BEQ/OPC_BNE/OPC_JMP all share raw OC_JMP with different MOD).
 *
 * Shared with the emulator, which decodes the same bit patterns.
 */

enum RawOpcode : unsigned {
    OC_HALT  = 0x0,
    OC_INT   = 0x1,
    OC_CALL  = 0x2,
    OC_JMP   = 0x3,
    OC_XCHG  = 0x4,
    OC_ARITH = 0x5,
    OC_LOGIC = 0x6,
    OC_SHIFT = 0x7,
    OC_STORE = 0x8,
    OC_LOAD  = 0x9,
};

enum CallMod : unsigned {
    MOD_CALL_DIRECT   = 0x0, // push pc; pc <= gpr[A]+gpr[B]+D
    MOD_CALL_INDIRECT = 0x1, // push pc; pc <= mem32[gpr[A]+gpr[B]+D]
};

enum JumpMod : unsigned {
    MOD_JMP_DIRECT     = 0x0, // pc <= gpr[A]+D
    MOD_BEQ_DIRECT     = 0x1, // if (gpr[B]==gpr[C]) pc <= gpr[A]+D
    MOD_BNE_DIRECT     = 0x2,
    MOD_BGT_DIRECT     = 0x3,
    MOD_JMP_INDIRECT   = 0x8, // pc <= mem32[gpr[A]+D]
    MOD_BEQ_INDIRECT   = 0x9,
    MOD_BNE_INDIRECT   = 0xA,
    MOD_BGT_INDIRECT   = 0xB,
};

enum ArithMod : unsigned {
    MOD_ARITH_ADD = 0x0,
    MOD_ARITH_SUB = 0x1,
    MOD_ARITH_MUL = 0x2,
    MOD_ARITH_DIV = 0x3,
};

enum LogicMod : unsigned {
    MOD_LOGIC_NOT = 0x0,
    MOD_LOGIC_AND = 0x1,
    MOD_LOGIC_OR  = 0x2,
    MOD_LOGIC_XOR = 0x3,
};

enum ShiftMod : unsigned {
    MOD_SHIFT_SHL = 0x0,
    MOD_SHIFT_SHR = 0x1,
};

enum StoreMod : unsigned {
    MOD_STORE_DIRECT   = 0x0, // mem32[gpr[A]+gpr[B]+D] <= gpr[C]
    MOD_STORE_PUSH     = 0x1, // gpr[A] <= gpr[A]+D; mem32[gpr[A]] <= gpr[C]
    MOD_STORE_INDIRECT = 0x2, // mem32[mem32[gpr[A]+gpr[B]+D]] <= gpr[C]
};

enum LoadMod : unsigned {
    MOD_LOAD_CSRRD             = 0x0, // gpr[A] <= csr[B]
    MOD_LOAD_IMM_DISP          = 0x1, // gpr[A] <= gpr[B]+D
    MOD_LOAD_MEM               = 0x2, // gpr[A] <= mem32[gpr[B]+gpr[C]+D]
    MOD_LOAD_MEM_POSTINC       = 0x3, // gpr[A] <= mem32[gpr[B]]; gpr[B] <= gpr[B]+D
    MOD_LOAD_CSRWR             = 0x4, // csr[A] <= gpr[B]
    MOD_LOAD_CSRWR_OR_DISP     = 0x5, // csr[A] <= csr[B]|D
    MOD_LOAD_CSRRD_MEM         = 0x6, // csr[A] <= mem32[gpr[B]+gpr[C]+D]
    MOD_LOAD_CSRRD_MEM_POSTINC = 0x7, // csr[A] <= mem32[gpr[B]]; gpr[B] <= gpr[B]+D
};

#endif /* OPCODES_HPP */
