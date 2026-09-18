#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <string>
#include <vector>

typedef enum LocalityKind {
    LOCAL,
    GLOBAL,
    EXTERN
} LocalityKind;

typedef enum SymbolType {
    SCTN,
    NOTYP
};

struct SymbolTableEntry {
    long value;
    std::string symbolName;
    int sectionIndex;
    LocalityKind locality;
    SymbolType type;
    bool defined;
    std::vector <int> references;
};

class SymbolTable {
public:
    SymbolTable() {
        entries.push_back(SymbolTableEntry{ 0, "", 0, LOCAL, NOTYP, false, {} });
    }

    bool checkSymbolDefinition(const std::string &name) {
        for (const auto &entry : entries) {
            if (entry.symbolName == name && entry.defined) {
                return true;
            }
        }
        return false;
    }

    // NOTYP-only: sections have their own SCTN-typed rows (see
    // getSectionIndex) and must not collide with the symbol namespace.
    bool checkEntry(const std::string &name) {
        for (const auto &entry : entries) {
            if (entry.symbolName == name && entry.type == NOTYP) {
                return true;
            }
        }
        return false;
    }

    // NOTYP-only, same scoping as checkEntry. Returns nullptr if the symbol
    // has no entry yet. Lets callers read back a symbol's resolved
    // value/sectionIndex/locality (e.g. to decide whether it's usable as an
    // absolute displacement constant) instead of just checking booleans.
    SymbolTableEntry *getEntry(const std::string &name) {
        for (auto &entry : entries) {
            if (entry.symbolName == name && entry.type == NOTYP) {
                return &entry;
            }
        }
        return nullptr;
    }

    bool isDeclaredExtern(const std::string &name) {
        for (const auto &entry : entries) {
            if (entry.symbolName == name && entry.type == NOTYP) {
                return entry.locality == EXTERN;
            }
        }
        return false;
    }

    int getSectionIndex(const std::string &name) {
        for (int i = 0; i < entries.size(); ++i) {
            if (entries[i].symbolName == name && entries[i].type == SCTN) {
                return i;
            }
        }
        return -1; // Section not found
    }

    void defineSymbol(const std::string &name, int sectionIndex, long value) {
        for (auto &entry : entries) {
            if (entry.symbolName == name && entry.type == NOTYP) {
                entry.defined = true;
                entry.sectionIndex = sectionIndex;
                entry.value = value;
                return;
            }
        }
    }

    void setLocality(const std::string &name, LocalityKind locality) {
        for (auto &entry : entries) {
            if (entry.symbolName == name && entry.type == NOTYP) {
                entry.locality = locality;
                return;
            }
        }
    }

    void addEntry(const SymbolTableEntry &entry) {
        entries.push_back(entry);
    }

private:
    std::vector<SymbolTableEntry> entries;

};

#endif /* SYMBOL_TABLE_HPP */
