# cmp
The IA-32 cmp instruction compares two operands by performing a subtraction but discards the result, only using the outcome to set the status flags (carry, zero, sign, overflow) in the EFLAGS register. These flags are then used by subsequent conditional jump instructions (je, jne, jg, etc.)

`reg`, `reg` / `reg`, `imm`
