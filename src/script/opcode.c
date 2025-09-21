/**
 * @file opcode.c
 * @brief Implementation of Neo VM operation codes
 */

#include "neoc/script/opcode.h"
#include <stddef.h>

typedef struct {
    neoc_opcode_t opcode;
    const char* name;
    int operand_size;
} opcode_info_t;

static const opcode_info_t opcode_table[] = {
    // Constants
    {NEOC_OP_PUSHINT8, "PUSHINT8", 1},
    {NEOC_OP_PUSHINT16, "PUSHINT16", 2},
    {NEOC_OP_PUSHINT32, "PUSHINT32", 4},
    {NEOC_OP_PUSHINT64, "PUSHINT64", 8},
    {NEOC_OP_PUSHINT128, "PUSHINT128", 16},
    {NEOC_OP_PUSHINT256, "PUSHINT256", 32},
    {NEOC_OP_PUSHA, "PUSHA", 4},
    {NEOC_OP_PUSHNULL, "PUSHNULL", 0},
    {NEOC_OP_PUSHDATA1, "PUSHDATA1", -1}, // Variable based on first byte
    {NEOC_OP_PUSHDATA2, "PUSHDATA2", -2}, // Variable based on first 2 bytes
    {NEOC_OP_PUSHDATA4, "PUSHDATA4", -4}, // Variable based on first 4 bytes
    {NEOC_OP_PUSHM1, "PUSHM1", 0},
    {NEOC_OP_PUSH0, "PUSH0", 0},
    {NEOC_OP_PUSH1, "PUSH1", 0},
    {NEOC_OP_PUSH2, "PUSH2", 0},
    {NEOC_OP_PUSH3, "PUSH3", 0},
    {NEOC_OP_PUSH4, "PUSH4", 0},
    {NEOC_OP_PUSH5, "PUSH5", 0},
    {NEOC_OP_PUSH6, "PUSH6", 0},
    {NEOC_OP_PUSH7, "PUSH7", 0},
    {NEOC_OP_PUSH8, "PUSH8", 0},
    {NEOC_OP_PUSH9, "PUSH9", 0},
    {NEOC_OP_PUSH10, "PUSH10", 0},
    {NEOC_OP_PUSH11, "PUSH11", 0},
    {NEOC_OP_PUSH12, "PUSH12", 0},
    {NEOC_OP_PUSH13, "PUSH13", 0},
    {NEOC_OP_PUSH14, "PUSH14", 0},
    {NEOC_OP_PUSH15, "PUSH15", 0},
    {NEOC_OP_PUSH16, "PUSH16", 0},
    
    // Flow control
    {NEOC_OP_NOP, "NOP", 0},
    {NEOC_OP_JMP, "JMP", 1},
    {NEOC_OP_JMP_L, "JMP_L", 4},
    {NEOC_OP_JMPIF, "JMPIF", 1},
    {NEOC_OP_JMPIF_L, "JMPIF_L", 4},
    {NEOC_OP_JMPIFNOT, "JMPIFNOT", 1},
    {NEOC_OP_JMPIFNOT_L, "JMPIFNOT_L", 4},
    {NEOC_OP_JMPEQ, "JMPEQ", 1},
    {NEOC_OP_JMPEQ_L, "JMPEQ_L", 4},
    {NEOC_OP_JMPNE, "JMPNE", 1},
    {NEOC_OP_JMPNE_L, "JMPNE_L", 4},
    {NEOC_OP_JMPGT, "JMPGT", 1},
    {NEOC_OP_JMPGT_L, "JMPGT_L", 4},
    {NEOC_OP_JMPGE, "JMPGE", 1},
    {NEOC_OP_JMPGE_L, "JMPGE_L", 4},
    {NEOC_OP_JMPLT, "JMPLT", 1},
    {NEOC_OP_JMPLT_L, "JMPLT_L", 4},
    {NEOC_OP_JMPLE, "JMPLE", 1},
    {NEOC_OP_JMPLE_L, "JMPLE_L", 4},
    {NEOC_OP_CALL, "CALL", 1},
    {NEOC_OP_CALL_L, "CALL_L", 4},
    {NEOC_OP_CALLA, "CALLA", 0},
    {NEOC_OP_CALLT, "CALLT", 2},
    {NEOC_OP_ABORT, "ABORT", 0},
    {NEOC_OP_ASSERT, "ASSERT", 0},
    {NEOC_OP_THROW, "THROW", 0},
    {NEOC_OP_TRY, "TRY", 2}, // catch offset + finally offset
    {NEOC_OP_TRY_L, "TRY_L", 8},
    {NEOC_OP_ENDTRY, "ENDTRY", 1},
    {NEOC_OP_ENDTRY_L, "ENDTRY_L", 4},
    {NEOC_OP_ENDFINALLY, "ENDFINALLY", 0},
    {NEOC_OP_RET, "RET", 0},
    {NEOC_OP_SYSCALL, "SYSCALL", 4},
    
    // Stack
    {NEOC_OP_DEPTH, "DEPTH", 0},
    {NEOC_OP_DROP, "DROP", 0},
    {NEOC_OP_NIP, "NIP", 0},
    {NEOC_OP_XDROP, "XDROP", 0},
    {NEOC_OP_CLEAR, "CLEAR", 0},
    {NEOC_OP_DUP, "DUP", 0},
    {NEOC_OP_OVER, "OVER", 0},
    {NEOC_OP_PICK, "PICK", 0},
    {NEOC_OP_TUCK, "TUCK", 0},
    {NEOC_OP_SWAP, "SWAP", 0},
    {NEOC_OP_ROT, "ROT", 0},
    {NEOC_OP_ROLL, "ROLL", 0},
    {NEOC_OP_REVERSE3, "REVERSE3", 0},
    {NEOC_OP_REVERSE4, "REVERSE4", 0},
    {NEOC_OP_REVERSEN, "REVERSEN", 0},
    
    // Slot
    {NEOC_OP_INITSSLOT, "INITSSLOT", 1},
    {NEOC_OP_INITSLOT, "INITSLOT", 2},
    {NEOC_OP_LDSFLD0, "LDSFLD0", 0},
    {NEOC_OP_LDSFLD1, "LDSFLD1", 0},
    {NEOC_OP_LDSFLD2, "LDSFLD2", 0},
    {NEOC_OP_LDSFLD3, "LDSFLD3", 0},
    {NEOC_OP_LDSFLD4, "LDSFLD4", 0},
    {NEOC_OP_LDSFLD5, "LDSFLD5", 0},
    {NEOC_OP_LDSFLD6, "LDSFLD6", 0},
    {NEOC_OP_LDSFLD, "LDSFLD", 1},
    {NEOC_OP_STSFLD0, "STSFLD0", 0},
    {NEOC_OP_STSFLD1, "STSFLD1", 0},
    {NEOC_OP_STSFLD2, "STSFLD2", 0},
    {NEOC_OP_STSFLD3, "STSFLD3", 0},
    {NEOC_OP_STSFLD4, "STSFLD4", 0},
    {NEOC_OP_STSFLD5, "STSFLD5", 0},
    {NEOC_OP_STSFLD6, "STSFLD6", 0},
    {NEOC_OP_STSFLD, "STSFLD", 1},
    {NEOC_OP_LDLOC0, "LDLOC0", 0},
    {NEOC_OP_LDLOC1, "LDLOC1", 0},
    {NEOC_OP_LDLOC2, "LDLOC2", 0},
    {NEOC_OP_LDLOC3, "LDLOC3", 0},
    {NEOC_OP_LDLOC4, "LDLOC4", 0},
    {NEOC_OP_LDLOC5, "LDLOC5", 0},
    {NEOC_OP_LDLOC6, "LDLOC6", 0},
    {NEOC_OP_LDLOC, "LDLOC", 1},
    {NEOC_OP_STLOC0, "STLOC0", 0},
    {NEOC_OP_STLOC1, "STLOC1", 0},
    {NEOC_OP_STLOC2, "STLOC2", 0},
    {NEOC_OP_STLOC3, "STLOC3", 0},
    {NEOC_OP_STLOC4, "STLOC4", 0},
    {NEOC_OP_STLOC5, "STLOC5", 0},
    {NEOC_OP_STLOC6, "STLOC6", 0},
    {NEOC_OP_STLOC, "STLOC", 1},
    {NEOC_OP_LDARG0, "LDARG0", 0},
    {NEOC_OP_LDARG1, "LDARG1", 0},
    {NEOC_OP_LDARG2, "LDARG2", 0},
    {NEOC_OP_LDARG3, "LDARG3", 0},
    {NEOC_OP_LDARG4, "LDARG4", 0},
    {NEOC_OP_LDARG5, "LDARG5", 0},
    {NEOC_OP_LDARG6, "LDARG6", 0},
    {NEOC_OP_LDARG, "LDARG", 1},
    {NEOC_OP_STARG0, "STARG0", 0},
    {NEOC_OP_STARG1, "STARG1", 0},
    {NEOC_OP_STARG2, "STARG2", 0},
    {NEOC_OP_STARG3, "STARG3", 0},
    {NEOC_OP_STARG4, "STARG4", 0},
    {NEOC_OP_STARG5, "STARG5", 0},
    {NEOC_OP_STARG6, "STARG6", 0},
    {NEOC_OP_STARG, "STARG", 1},
    
    // Splice
    {NEOC_OP_NEWBUFFER, "NEWBUFFER", 0},
    {NEOC_OP_MEMCPY, "MEMCPY", 0},
    {NEOC_OP_CAT, "CAT", 0},
    {NEOC_OP_SUBSTR, "SUBSTR", 0},
    {NEOC_OP_LEFT, "LEFT", 0},
    {NEOC_OP_RIGHT, "RIGHT", 0},
    
    // Bitwise logic
    {NEOC_OP_INVERT, "INVERT", 0},
    {NEOC_OP_AND, "AND", 0},
    {NEOC_OP_OR, "OR", 0},
    {NEOC_OP_XOR, "XOR", 0},
    {NEOC_OP_EQUAL, "EQUAL", 0},
    {NEOC_OP_NOTEQUAL, "NOTEQUAL", 0},
    
    // Arithmetic
    {NEOC_OP_SIGN, "SIGN", 0},
    {NEOC_OP_ABS, "ABS", 0},
    {NEOC_OP_NEGATE, "NEGATE", 0},
    {NEOC_OP_INC, "INC", 0},
    {NEOC_OP_DEC, "DEC", 0},
    {NEOC_OP_ADD, "ADD", 0},
    {NEOC_OP_SUB, "SUB", 0},
    {NEOC_OP_MUL, "MUL", 0},
    {NEOC_OP_DIV, "DIV", 0},
    {NEOC_OP_MOD, "MOD", 0},
    {NEOC_OP_POW, "POW", 0},
    {NEOC_OP_SQRT, "SQRT", 0},
    {NEOC_OP_MODMUL, "MODMUL", 0},
    {NEOC_OP_MODPOW, "MODPOW", 0},
    {NEOC_OP_SHL, "SHL", 0},
    {NEOC_OP_SHR, "SHR", 0},
    {NEOC_OP_NOT, "NOT", 0},
    {NEOC_OP_BOOLAND, "BOOLAND", 0},
    {NEOC_OP_BOOLOR, "BOOLOR", 0},
    {NEOC_OP_NZ, "NZ", 0},
    {NEOC_OP_NUMEQUAL, "NUMEQUAL", 0},
    {NEOC_OP_NUMNOTEQUAL, "NUMNOTEQUAL", 0},
    {NEOC_OP_LT, "LT", 0},
    {NEOC_OP_LE, "LE", 0},
    {NEOC_OP_GT, "GT", 0},
    {NEOC_OP_GE, "GE", 0},
    {NEOC_OP_MIN, "MIN", 0},
    {NEOC_OP_MAX, "MAX", 0},
    {NEOC_OP_WITHIN, "WITHIN", 0},
    
    // Compound
    {NEOC_OP_PACKMAP, "PACKMAP", 0},
    {NEOC_OP_PACKSTRUCT, "PACKSTRUCT", 0},
    {NEOC_OP_PACK, "PACK", 0},
    {NEOC_OP_UNPACK, "UNPACK", 0},
    {NEOC_OP_NEWARRAY0, "NEWARRAY0", 0},
    {NEOC_OP_NEWARRAY, "NEWARRAY", 0},
    {NEOC_OP_NEWARRAY_T, "NEWARRAY_T", 1},
    {NEOC_OP_NEWSTRUCT0, "NEWSTRUCT0", 0},
    {NEOC_OP_NEWSTRUCT, "NEWSTRUCT", 0},
    {NEOC_OP_NEWMAP, "NEWMAP", 0},
    {NEOC_OP_SIZE, "SIZE", 0},
    {NEOC_OP_HASKEY, "HASKEY", 0},
    {NEOC_OP_KEYS, "KEYS", 0},
    {NEOC_OP_VALUES, "VALUES", 0},
    {NEOC_OP_PICKITEM, "PICKITEM", 0},
    {NEOC_OP_APPEND, "APPEND", 0},
    {NEOC_OP_SETITEM, "SETITEM", 0},
    {NEOC_OP_REVERSEITEMS, "REVERSEITEMS", 0},
    {NEOC_OP_REMOVE, "REMOVE", 0},
    {NEOC_OP_CLEARITEMS, "CLEARITEMS", 0},
    {NEOC_OP_POPITEM, "POPITEM", 0},
    
    // Types
    {NEOC_OP_ISNULL, "ISNULL", 0},
    {NEOC_OP_ISTYPE, "ISTYPE", 1},
    {NEOC_OP_CONVERT, "CONVERT", 1},
};

static const int opcode_table_size = sizeof(opcode_table) / sizeof(opcode_table[0]);

const char* neoc_opcode_get_name(neoc_opcode_t opcode) {
    for (int i = 0; i < opcode_table_size; i++) {
        if (opcode_table[i].opcode == opcode) {
            return opcode_table[i].name;
        }
    }
    return "UNKNOWN";
}

int neoc_opcode_get_operand_size(neoc_opcode_t opcode) {
    for (int i = 0; i < opcode_table_size; i++) {
        if (opcode_table[i].opcode == opcode) {
            return opcode_table[i].operand_size;
        }
    }
    return 0;
}
