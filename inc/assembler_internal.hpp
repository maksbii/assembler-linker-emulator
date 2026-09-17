#ifndef ASSEMBLER_INTERNAL_HPP
#define ASSEMBLER_INTERNAL_HPP

#include <string>
#include <map>
#include <vector>
#include "asm_types.h"

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

private:
    std::string name;
    long location_counter;
    std::vector<int> memory;
};

extern std::map<std::string, Section> sections;
extern Section *current_section;

extern DirectiveKind current_directive;

#endif /* ASSEMBLER_INTERNAL_HPP */
