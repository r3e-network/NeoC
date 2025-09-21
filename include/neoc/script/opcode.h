/**
 * @file opcode.h
 * @brief Neo VM operation codes
 */

#ifndef NEOC_OPCODE_H
#define NEOC_OPCODE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Neo VM operation codes
 */
typedef enum {
    // Constants
    NEOC_OP_PUSHINT8 = 0x00,
    NEOC_OP_PUSHINT16 = 0x01,
    NEOC_OP_PUSHINT32 = 0x02,
    NEOC_OP_PUSHINT64 = 0x03,
    NEOC_OP_PUSHINT128 = 0x04,
    NEOC_OP_PUSHINT256 = 0x05,
    NEOC_OP_PUSHA = 0x0A,
    NEOC_OP_PUSHNULL = 0x0B,
    NEOC_OP_PUSHDATA1 = 0x0C,
    NEOC_OP_PUSHDATA2 = 0x0D,
    NEOC_OP_PUSHDATA4 = 0x0E,
    NEOC_OP_PUSHM1 = 0x0F,
    NEOC_OP_PUSH0 = 0x10,
    NEOC_OP_PUSH1 = 0x11,
    NEOC_OP_PUSH2 = 0x12,
    NEOC_OP_PUSH3 = 0x13,
    NEOC_OP_PUSH4 = 0x14,
    NEOC_OP_PUSH5 = 0x15,
    NEOC_OP_PUSH6 = 0x16,
    NEOC_OP_PUSH7 = 0x17,
    NEOC_OP_PUSH8 = 0x18,
    NEOC_OP_PUSH9 = 0x19,
    NEOC_OP_PUSH10 = 0x1A,
    NEOC_OP_PUSH11 = 0x1B,
    NEOC_OP_PUSH12 = 0x1C,
    NEOC_OP_PUSH13 = 0x1D,
    NEOC_OP_PUSH14 = 0x1E,
    NEOC_OP_PUSH15 = 0x1F,
    NEOC_OP_PUSH16 = 0x20,
    
    // Flow control
    NEOC_OP_NOP = 0x21,
    NEOC_OP_JMP = 0x22,
    NEOC_OP_JMP_L = 0x23,
    NEOC_OP_JMPIF = 0x24,
    NEOC_OP_JMPIF_L = 0x25,
    NEOC_OP_JMPIFNOT = 0x26,
    NEOC_OP_JMPIFNOT_L = 0x27,
    NEOC_OP_JMPEQ = 0x28,
    NEOC_OP_JMPEQ_L = 0x29,
    NEOC_OP_JMPNE = 0x2A,
    NEOC_OP_JMPNE_L = 0x2B,
    NEOC_OP_JMPGT = 0x2C,
    NEOC_OP_JMPGT_L = 0x2D,
    NEOC_OP_JMPGE = 0x2E,
    NEOC_OP_JMPGE_L = 0x2F,
    NEOC_OP_JMPLT = 0x30,
    NEOC_OP_JMPLT_L = 0x31,
    NEOC_OP_JMPLE = 0x32,
    NEOC_OP_JMPLE_L = 0x33,
    NEOC_OP_CALL = 0x34,
    NEOC_OP_CALL_L = 0x35,
    NEOC_OP_CALLA = 0x36,
    NEOC_OP_CALLT = 0x37,
    NEOC_OP_ABORT = 0x38,
    NEOC_OP_ASSERT = 0x39,
    NEOC_OP_THROW = 0x3A,
    NEOC_OP_TRY = 0x3B,
    NEOC_OP_TRY_L = 0x3C,
    NEOC_OP_ENDTRY = 0x3D,
    NEOC_OP_ENDTRY_L = 0x3E,
    NEOC_OP_ENDFINALLY = 0x3F,
    NEOC_OP_RET = 0x40,
    NEOC_OP_SYSCALL = 0x41,
    
    // Stack
    NEOC_OP_DEPTH = 0x43,
    NEOC_OP_DROP = 0x45,
    NEOC_OP_NIP = 0x46,
    NEOC_OP_XDROP = 0x48,
    NEOC_OP_CLEAR = 0x49,
    NEOC_OP_DUP = 0x4A,
    NEOC_OP_OVER = 0x4B,
    NEOC_OP_PICK = 0x4D,
    NEOC_OP_TUCK = 0x4E,
    NEOC_OP_SWAP = 0x50,
    NEOC_OP_ROT = 0x51,
    NEOC_OP_ROLL = 0x52,
    NEOC_OP_REVERSE3 = 0x53,
    NEOC_OP_REVERSE4 = 0x54,
    NEOC_OP_REVERSEN = 0x55,
    
    // Slot
    NEOC_OP_INITSSLOT = 0x56,
    NEOC_OP_INITSLOT = 0x57,
    NEOC_OP_LDSFLD0 = 0x58,
    NEOC_OP_LDSFLD1 = 0x59,
    NEOC_OP_LDSFLD2 = 0x5A,
    NEOC_OP_LDSFLD3 = 0x5B,
    NEOC_OP_LDSFLD4 = 0x5C,
    NEOC_OP_LDSFLD5 = 0x5D,
    NEOC_OP_LDSFLD6 = 0x5E,
    NEOC_OP_LDSFLD = 0x5F,
    NEOC_OP_STSFLD0 = 0x60,
    NEOC_OP_STSFLD1 = 0x61,
    NEOC_OP_STSFLD2 = 0x62,
    NEOC_OP_STSFLD3 = 0x63,
    NEOC_OP_STSFLD4 = 0x64,
    NEOC_OP_STSFLD5 = 0x65,
    NEOC_OP_STSFLD6 = 0x66,
    NEOC_OP_STSFLD = 0x67,
    NEOC_OP_LDLOC0 = 0x68,
    NEOC_OP_LDLOC1 = 0x69,
    NEOC_OP_LDLOC2 = 0x6A,
    NEOC_OP_LDLOC3 = 0x6B,
    NEOC_OP_LDLOC4 = 0x6C,
    NEOC_OP_LDLOC5 = 0x6D,
    NEOC_OP_LDLOC6 = 0x6E,
    NEOC_OP_LDLOC = 0x6F,
    NEOC_OP_STLOC0 = 0x70,
    NEOC_OP_STLOC1 = 0x71,
    NEOC_OP_STLOC2 = 0x72,
    NEOC_OP_STLOC3 = 0x73,
    NEOC_OP_STLOC4 = 0x74,
    NEOC_OP_STLOC5 = 0x75,
    NEOC_OP_STLOC6 = 0x76,
    NEOC_OP_STLOC = 0x77,
    NEOC_OP_LDARG0 = 0x78,
    NEOC_OP_LDARG1 = 0x79,
    NEOC_OP_LDARG2 = 0x7A,
    NEOC_OP_LDARG3 = 0x7B,
    NEOC_OP_LDARG4 = 0x7C,
    NEOC_OP_LDARG5 = 0x7D,
    NEOC_OP_LDARG6 = 0x7E,
    NEOC_OP_LDARG = 0x7F,
    NEOC_OP_STARG0 = 0x80,
    NEOC_OP_STARG1 = 0x81,
    NEOC_OP_STARG2 = 0x82,
    NEOC_OP_STARG3 = 0x83,
    NEOC_OP_STARG4 = 0x84,
    NEOC_OP_STARG5 = 0x85,
    NEOC_OP_STARG6 = 0x86,
    NEOC_OP_STARG = 0x87,
    
    // Splice
    NEOC_OP_NEWBUFFER = 0x88,
    NEOC_OP_MEMCPY = 0x89,
    NEOC_OP_CAT = 0x8B,
    NEOC_OP_SUBSTR = 0x8C,
    NEOC_OP_LEFT = 0x8D,
    NEOC_OP_RIGHT = 0x8E,
    
    // Bitwise logic
    NEOC_OP_INVERT = 0x90,
    NEOC_OP_AND = 0x91,
    NEOC_OP_OR = 0x92,
    NEOC_OP_XOR = 0x93,
    NEOC_OP_EQUAL = 0x97,
    NEOC_OP_NOTEQUAL = 0x98,
    
    // Arithmetic
    NEOC_OP_SIGN = 0x99,
    NEOC_OP_ABS = 0x9A,
    NEOC_OP_NEGATE = 0x9B,
    NEOC_OP_INC = 0x9C,
    NEOC_OP_DEC = 0x9D,
    NEOC_OP_ADD = 0x9E,
    NEOC_OP_SUB = 0x9F,
    NEOC_OP_MUL = 0xA0,
    NEOC_OP_DIV = 0xA1,
    NEOC_OP_MOD = 0xA2,
    NEOC_OP_POW = 0xA3,
    NEOC_OP_SQRT = 0xA4,
    NEOC_OP_MODMUL = 0xA5,
    NEOC_OP_MODPOW = 0xA6,
    NEOC_OP_SHL = 0xA8,
    NEOC_OP_SHR = 0xA9,
    NEOC_OP_NOT = 0xAA,
    NEOC_OP_BOOLAND = 0xAB,
    NEOC_OP_BOOLOR = 0xAC,
    NEOC_OP_NZ = 0xB1,
    NEOC_OP_NUMEQUAL = 0xB3,
    NEOC_OP_NUMNOTEQUAL = 0xB4,
    NEOC_OP_LT = 0xB5,
    NEOC_OP_LE = 0xB6,
    NEOC_OP_GT = 0xB7,
    NEOC_OP_GE = 0xB8,
    NEOC_OP_MIN = 0xB9,
    NEOC_OP_MAX = 0xBA,
    NEOC_OP_WITHIN = 0xBB,
    
    // Compound
    NEOC_OP_PACKMAP = 0xBE,
    NEOC_OP_PACKSTRUCT = 0xBF,
    NEOC_OP_PACK = 0xC0,
    NEOC_OP_UNPACK = 0xC1,
    NEOC_OP_NEWARRAY0 = 0xC2,
    NEOC_OP_NEWARRAY = 0xC3,
    NEOC_OP_NEWARRAY_T = 0xC4,
    NEOC_OP_NEWSTRUCT0 = 0xC5,
    NEOC_OP_NEWSTRUCT = 0xC6,
    NEOC_OP_NEWMAP = 0xC8,
    NEOC_OP_SIZE = 0xCA,
    NEOC_OP_HASKEY = 0xCB,
    NEOC_OP_KEYS = 0xCC,
    NEOC_OP_VALUES = 0xCD,
    NEOC_OP_PICKITEM = 0xCE,
    NEOC_OP_APPEND = 0xCF,
    NEOC_OP_SETITEM = 0xD0,
    NEOC_OP_REVERSEITEMS = 0xD1,
    NEOC_OP_REMOVE = 0xD2,
    NEOC_OP_CLEARITEMS = 0xD3,
    NEOC_OP_POPITEM = 0xD4,
    
    // Types
    NEOC_OP_ISNULL = 0xD8,
    NEOC_OP_ISTYPE = 0xD9,
    NEOC_OP_CONVERT = 0xDB
} neoc_opcode_t;

/**
 * @brief Get the name of an opcode
 * 
 * @param opcode The opcode
 * @return The name of the opcode (do not free)
 */
const char* neoc_opcode_get_name(neoc_opcode_t opcode);

/**
 * @brief Get the size of an opcode's operand
 * 
 * @param opcode The opcode
 * @return The operand size in bytes
 */
int neoc_opcode_get_operand_size(neoc_opcode_t opcode);

#ifdef __cplusplus
}
#endif

#endif // NEOC_OPCODE_H
