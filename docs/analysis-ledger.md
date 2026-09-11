# Analysis & Validation Failure Ledger

This document tracks all unresolved targets, switch tables, function boundaries, invalid instructions, and codegen validation failures discovered during ReXGlue analysis passes.

## Pass 1: Strict Baseline Analysis
- Timestamp: Pending execution
- Target: default.xex (2955F2E2BE61EC1948CD2FD3538AD45BEB04772F5EBD5E0FB1BFDE484748E5A7)
- Codegen Exit Code: Pending
- Raw Output Log: (Captured below once run)

### Unresolved Function Boundaries
| Target Address | Caller Address / Source | Classification | Evidence / Instruction | Config Fix | Status |
|---|---|---|---|---|---|

### Switch Tables / Computed Branches (bctr)
| Branch PC | Target Addresses / Table Range | Classification | Evidence | Config Fix | Status |
|---|---|---|---|---|---|

### Materialized Function Pointers / Vtables
| Pointer Address | Referencing Data/Code PC | Type | Class / Subsystem | Status |
|---|---|---|---|---|

### Invalid Instructions / Data-as-Code
| Address Range | False Instruction / Data Content | Reason | Config Fix | Status |
|---|---|---|---|---|
