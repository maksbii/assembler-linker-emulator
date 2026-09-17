#ifndef REGISTERS_HPP
#define REGISTERS_HPP

/*
 * GPR and CSR index constants, per spec: gpr index N for register rN
 * (r0 hardwired to 0, r14 aliased as sp, r15 aliased as pc); csr indices
 * are status=0, handler=1, cause=2. Shared with the emulator, which uses
 * the same indices to address its register file.
 */

enum GprIndex : unsigned {
    REG_SP = 14,
    REG_PC = 15,
};

enum CsrIndex : unsigned {
    CSR_STATUS  = 0,
    CSR_HANDLER = 1,
    CSR_CAUSE   = 2,
};

#endif /* REGISTERS_HPP */
