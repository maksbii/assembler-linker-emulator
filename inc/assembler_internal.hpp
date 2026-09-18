#ifndef ASSEMBLER_INTERNAL_HPP
#define ASSEMBLER_INTERNAL_HPP

#include <string>
#include <map>
#include <vector>
#include "asm_types.h"
#include "registers.hpp"
#include "relocation_table.hpp"

class Section {
public:
    explicit Section(const std::string &name) : name(name), location_counter(0) {}

    const std::string &getName() const {
        return name;
    }

    long getLocationCounter() const {
        return location_counter;
    }

    const std::vector<int> &getMemory() const {
        return memory;
    }

    // Appends `count` little-endian bytes of `value` and advances the
    // location counter. Bytes beyond sizeof(value) are zero (used by
    // .skip, where value is always 0 and count can exceed 8).
    void appendBytes(long value, long count) {
        for (long i = 0; i < count; ++i) {
            unsigned char byte = (i < (long)sizeof(value)) ? (unsigned char)((value >> (8 * i)) & 0xFF) : 0;
            memory.push_back(byte);
        }
        location_counter += count;
    }

    // Encodes and appends one fixed 4-byte instruction word:
    // byte0 = OC[7:4]:MOD[3:0], byte1 = RegA[7:4]:RegB[3:0],
    // byte2 = RegC[7:4]:Disp[11:8], byte3 = Disp[7:0]. `disp` is a
    // 12-bit signed displacement, truncated to its low 12 bits here.
    void appendInstruction(unsigned oc, unsigned mod, unsigned regA, unsigned regB, unsigned regC, int disp) {
        unsigned d = (unsigned)disp & 0xFFF;
        memory.push_back((unsigned char)(((oc & 0xF) << 4) | (mod & 0xF)));
        memory.push_back((unsigned char)(((regA & 0xF) << 4) | (regB & 0xF)));
        memory.push_back((unsigned char)(((regC & 0xF) << 4) | ((d >> 8) & 0xF)));
        memory.push_back((unsigned char)(d & 0xFF));
        location_counter += 4;
    }

    // Emits an instruction with Disp=0 as a placeholder referencing a
    // literal-pool slot that doesn't exist yet, and queues that slot to be
    // materialized by flushLiteralPool(). Used for `ld`/`st` operands whose
    // value doesn't fit the 12-bit Disp field (oversized literal) or needs
    // relocation (symbol address). regA/regB/regC are caller-supplied
    // because the PC-relative base sits in different fields for `ld`
    // (RegB=pc, dest reg in RegA) vs `st` (RegA=pc, source reg in RegC).
    void appendPoolLiteral(unsigned oc, unsigned mod, unsigned regA, unsigned regB, unsigned regC, long value) {
        long instr_addr = location_counter;
        appendInstruction(oc, mod, regA, regB, regC, 0);
        pending_pool.push_back({instr_addr, false, value, ""});
    }

    void appendPoolSymbol(unsigned oc, unsigned mod, unsigned regA, unsigned regB, unsigned regC, const std::string &symbol) {
        long instr_addr = location_counter;
        appendInstruction(oc, mod, regA, regB, regC, 0);
        pending_pool.push_back({instr_addr, true, 0, symbol});
    }

    // Materializes all literal-pool slots queued by appendPoolLiteral/
    // appendPoolSymbol: appends one 4-byte word per slot to the end of this
    // section's memory (0 placeholder + a RelocationEntry for symbols, the
    // literal value directly otherwise), then backpatches each waiting
    // instruction's Disp field with pool_addr - (instr_addr + 4) -- pc is
    // assumed to already hold the address of the *next* instruction
    // (post-increment/fetch convention) at the time Disp is added to it.
    // Called at each .section boundary (for the section being left) and at
    // .end. Assumes the pool stays within +-2048 bytes of every referencing
    // instruction (no overflow check, per project's simplifying choice for
    // this one-pass assembler). The backpatch only touches the Disp_hi
    // nibble of byte2, preserving whatever RegC already holds -- RegC is 0
    // for `ld`'s pool cases but holds the source register for `st`'s.
    void flushLiteralPool(RelocationTable &relocation_table, int section_index) {
        for (const auto &entry : pending_pool) {
            long pool_addr = location_counter;
            if (entry.is_symbol) {
                relocation_table.addEntry(RelocationEntry{location_counter, section_index, entry.symbol});
                appendBytes(0, 4);
            } else {
                appendBytes(entry.value, 4);
            }

            long pc_at_instr = entry.instr_addr + 4;
            unsigned d = (unsigned)(pool_addr - pc_at_instr) & 0xFFF;
            memory[entry.instr_addr + 2] = (unsigned char)((memory[entry.instr_addr + 2] & 0xF0) | ((d >> 8) & 0xF));
            memory[entry.instr_addr + 3] = (unsigned char)(d & 0xFF);
        }
        pending_pool.clear();
    }

private:
    struct PendingLiteral {
        long instr_addr;    // start offset of the 4-byte instruction word to backpatch
        bool is_symbol;
        long value;          // literal value, if !is_symbol
        std::string symbol;  // symbol name, if is_symbol
    };

    std::string name;
    long location_counter;
    std::vector<int> memory;
    std::vector<PendingLiteral> pending_pool;
};

extern std::map<std::string, Section> sections;
extern Section *current_section;

extern DirectiveKind current_directive;

#endif /* ASSEMBLER_INTERNAL_HPP */
