#ifndef RELOCATION_TABLE_HPP
#define RELOCATION_TABLE_HPP

#include <string>
#include <vector>

struct RelocationEntry {
    long offset;             // offset within the section's memory where the patch goes
    int sectionIndex;        // symbol table index (type SCTN) of the section this belongs to
    std::string symbolName;  // symbol whose final address gets patched in
};

class RelocationTable {
public:
    void addEntry(const RelocationEntry &entry) {
        entries.push_back(entry);
    }

    const std::vector<RelocationEntry> &getEntries() const {
        return entries;
    }

private:
    std::vector<RelocationEntry> entries;
};

#endif /* RELOCATION_TABLE_HPP */
