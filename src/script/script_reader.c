/**
 * @file script_reader.c
 * @brief Implementation of NeoVM script reader
 */

#include "neoc/script/script_reader.h"
#include "neoc/utils/neoc_hex.h"
#include "neoc/utils/neoc_bytes_utils.h"
#include "neoc/serialization/binary_reader.h"
#include "neoc/neoc_memory.h"
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>

/* Internal helper functions */
static neoc_error_t get_operand_size_info(neoc_opcode_t opcode, size_t* fixed_size, size_t* prefix_size);
static neoc_error_t read_prefix_size(neoc_script_reader_t* reader, size_t prefix_size, size_t* result);
static const char* opcode_to_string(neoc_opcode_t opcode);

neoc_error_t neoc_script_reader_init(
    neoc_script_reader_t* reader,
    const uint8_t* script,
    size_t script_length,
    char* output_buffer,
    size_t output_size
) {
    if (!reader || !script || !output_buffer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    reader->script = script;
    reader->script_length = script_length;
    reader->position = 0;
    reader->output_buffer = output_buffer;
    reader->output_size = output_size;
    reader->output_length = 0;
    
    /* Initialize output buffer */
    if (output_size > 0) {
        output_buffer[0] = '\0';
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_script_reader_get_interop_service(
    const char* hash_string,
    neoc_interop_service_t* interop_service
) {
    if (!hash_string || !interop_service) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    /* Try to find matching interop service by hash */
    for (int i = 0; i < NEOC_INTEROP_COUNT; i++) {
        /* Get the hash for this interop service */
        uint32_t service_hash = neoc_interop_get_hash((neoc_interop_service_t)i);
        
        /* Convert hash to string for comparison */
        char service_hash_str[32];
        snprintf(service_hash_str, sizeof(service_hash_str), "%08x", service_hash);
        
        if (strcmp(hash_string, service_hash_str) == 0) {
            *interop_service = (neoc_interop_service_t)i;
            return NEOC_SUCCESS;
        }
    }
    
    return NEOC_ERROR_NOT_FOUND;
}

neoc_error_t neoc_script_reader_to_opcode_string_hex(
    const char* script_hex,
    char* output_buffer,
    size_t buffer_size
) {
    if (!script_hex || !output_buffer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    /* Convert hex string to bytes */
    size_t script_length = strlen(script_hex) / 2;
    uint8_t* script_bytes = (uint8_t*)neoc_malloc(script_length);
    if (!script_bytes) {
        return NEOC_ERROR_OUT_OF_MEMORY;
    }
    
    size_t decoded_len;
    neoc_error_t result = neoc_hex_decode(script_hex, script_bytes, script_length, &decoded_len);
    if (result != NEOC_SUCCESS) {
        neoc_free(script_bytes);
        return result;
    }
    
    /* Convert to OpCode string */
    result = neoc_script_reader_to_opcode_string(script_bytes, script_length, output_buffer, buffer_size);
    
    free(script_bytes);
    return result;
}

neoc_error_t neoc_script_reader_to_opcode_string(
    const uint8_t* script,
    size_t script_length,
    char* output_buffer,
    size_t buffer_size
) {
    if (!script || !output_buffer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    neoc_script_reader_t reader;
    neoc_error_t result = neoc_script_reader_init(&reader, script, script_length, output_buffer, buffer_size);
    if (result != NEOC_SUCCESS) {
        return result;
    }
    
    while (neoc_script_reader_has_more(&reader)) {
        uint8_t opcode_byte;
        result = neoc_script_reader_read_byte(&reader, &opcode_byte);
        if (result != NEOC_SUCCESS) {
            continue; /* Skip invalid bytes */
        }
        
        neoc_opcode_t opcode = (neoc_opcode_t)opcode_byte;
        const char* opcode_name = opcode_to_string(opcode);
        
        /* Add OpCode name to output */
        result = neoc_script_reader_append_output(&reader, opcode_name);
        if (result != NEOC_SUCCESS) {
            return result;
        }
        
        /* Handle operand data */
        size_t fixed_size = 0;
        size_t prefix_size = 0;
        result = get_operand_size_info(opcode, &fixed_size, &prefix_size);
        
        if (fixed_size > 0) {
            /* Fixed size operand */
            uint8_t operand_data[256]; /* Should be enough for any operand */
            if (fixed_size <= sizeof(operand_data)) {
                result = neoc_script_reader_read_bytes(&reader, operand_data, fixed_size);
                if (result == NEOC_SUCCESS) {
                    char hex_string[512];
                    result = neoc_hex_encode(operand_data, fixed_size, hex_string, sizeof(hex_string), false, false);
                    if (result == NEOC_SUCCESS) {
                        result = neoc_script_reader_append_formatted(&reader, " %s", hex_string);
                        if (result != NEOC_SUCCESS) {
                            return result;
                        }
                    }
                }
            }
        } else if (prefix_size > 0) {
            /* Variable size operand with prefix */
            size_t operand_size = 0;
            result = read_prefix_size(&reader, prefix_size, &operand_size);
            if (result == NEOC_SUCCESS && operand_size > 0) {
                result = neoc_script_reader_append_formatted(&reader, " %zu ", operand_size);
                if (result != NEOC_SUCCESS) {
                    return result;
                }
                
                /* Read the actual operand data */
                uint8_t* operand_data = (uint8_t*)neoc_malloc(operand_size);
                if (operand_data) {
                    result = neoc_script_reader_read_bytes(&reader, operand_data, operand_size);
                    if (result == NEOC_SUCCESS) {
                        char hex_string[512];
                        size_t max_hex_bytes = (sizeof(hex_string) - 1) / 2;
                        size_t hex_bytes = (operand_size > max_hex_bytes) ? max_hex_bytes : operand_size;
                        
                        result = neoc_hex_encode(operand_data, hex_bytes, hex_string, sizeof(hex_string), false, false);
                        if (result == NEOC_SUCCESS) {
                            result = neoc_script_reader_append_output(&reader, hex_string);
                        }
                    }
                    neoc_free(operand_data);
                }
            }
        }
        
        /* Add newline */
        result = neoc_script_reader_append_output(&reader, "\n");
        if (result != NEOC_SUCCESS) {
            return result;
        }
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_script_reader_read_byte(neoc_script_reader_t* reader, uint8_t* byte) {
    if (!reader || !byte) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (reader->position >= reader->script_length) {
        return NEOC_ERROR_END_OF_STREAM;
    }
    
    *byte = reader->script[reader->position++];
    return NEOC_SUCCESS;
}

neoc_error_t neoc_script_reader_read_bytes(neoc_script_reader_t* reader, uint8_t* buffer, size_t count) {
    if (!reader || !buffer) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    if (reader->position + count > reader->script_length) {
        return NEOC_ERROR_END_OF_STREAM;
    }
    
    memcpy(buffer, reader->script + reader->position, count);
    reader->position += count;
    
    return NEOC_SUCCESS;
}

bool neoc_script_reader_has_more(const neoc_script_reader_t* reader) {
    return reader && reader->position < reader->script_length;
}

size_t neoc_script_reader_get_position(const neoc_script_reader_t* reader) {
    return reader ? reader->position : 0;
}

neoc_error_t neoc_script_reader_append_output(neoc_script_reader_t* reader, const char* str) {
    if (!reader || !str) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    size_t str_len = strlen(str);
    if (reader->output_length + str_len + 1 > reader->output_size) {
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    
    strcat(reader->output_buffer, str);
    reader->output_length += str_len;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_script_reader_append_formatted(neoc_script_reader_t* reader, const char* format, ...) {
    if (!reader || !format) {
        return NEOC_ERROR_NULL_POINTER;
    }
    
    char temp_buffer[256];
    va_list args;
    va_start(args, format);
    int result = vsnprintf(temp_buffer, sizeof(temp_buffer), format, args);
    va_end(args);
    
    if (result < 0 || result >= (int)sizeof(temp_buffer)) {
        return NEOC_ERROR_BUFFER_TOO_SMALL;
    }
    
    return neoc_script_reader_append_output(reader, temp_buffer);
}

/* Internal helper functions */

static neoc_error_t get_operand_size_info(neoc_opcode_t opcode, size_t* fixed_size, size_t* prefix_size) {
    *fixed_size = 0;
    *prefix_size = 0;
    
    switch (opcode) {
        /* OpCodes with 1-byte operands */
        case NEOC_OP_PUSHINT8:
        case NEOC_OP_JMP:
        case NEOC_OP_JMPIF:
        case NEOC_OP_JMPIFNOT:
        case NEOC_OP_JMPEQ:
        case NEOC_OP_JMPNE:
        case NEOC_OP_JMPGT:
        case NEOC_OP_JMPGE:
        case NEOC_OP_JMPLT:
        case NEOC_OP_JMPLE:
        case NEOC_OP_CALL:
        case NEOC_OP_ENDTRY:
            *fixed_size = 1;
            break;
            
        /* OpCodes with 2-byte operands */
        case NEOC_OP_PUSHINT16:
        case NEOC_OP_JMP_L:
        case NEOC_OP_JMPIF_L:
        case NEOC_OP_JMPIFNOT_L:
        case NEOC_OP_JMPEQ_L:
        case NEOC_OP_JMPNE_L:
        case NEOC_OP_JMPGT_L:
        case NEOC_OP_JMPGE_L:
        case NEOC_OP_JMPLT_L:
        case NEOC_OP_JMPLE_L:
        case NEOC_OP_CALL_L:
        case NEOC_OP_ENDTRY_L:
            *fixed_size = 2;
            break;
            
        /* OpCodes with 4-byte operands */
        case NEOC_OP_PUSHINT32:
        case NEOC_OP_PUSHA:
        case NEOC_OP_SYSCALL:
        case NEOC_OP_ENDFINALLY:
            *fixed_size = 4;
            break;
            
        /* OpCodes with 8-byte operands */
        case NEOC_OP_PUSHINT64:
        case NEOC_OP_PUSHINT128:
        case NEOC_OP_PUSHINT256:
            *fixed_size = 8;
            break;
            
        /* OpCodes with variable-length operands */
        case NEOC_OP_PUSHDATA1:
            *prefix_size = 1;
            break;
        case NEOC_OP_PUSHDATA2:
            *prefix_size = 2;
            break;
        case NEOC_OP_PUSHDATA4:
            *prefix_size = 4;
            break;
            
        default:
            /* No operand */
            break;
    }
    
    return NEOC_SUCCESS;
}

static neoc_error_t read_prefix_size(neoc_script_reader_t* reader, size_t prefix_size, size_t* result) {
    *result = 0;
    
    for (size_t i = 0; i < prefix_size; i++) {
        uint8_t byte;
        neoc_error_t err = neoc_script_reader_read_byte(reader, &byte);
        if (err != NEOC_SUCCESS) {
            return err;
        }
        *result |= ((size_t)byte) << (i * 8);
    }
    
    return NEOC_SUCCESS;
}

static const char* opcode_to_string(neoc_opcode_t opcode) {
    switch (opcode) {
        case NEOC_OP_PUSHINT8: return "PUSHINT8";
        case NEOC_OP_PUSHINT16: return "PUSHINT16";
        case NEOC_OP_PUSHINT32: return "PUSHINT32";
        case NEOC_OP_PUSHINT64: return "PUSHINT64";
        case NEOC_OP_PUSHINT128: return "PUSHINT128";
        case NEOC_OP_PUSHINT256: return "PUSHINT256";
        /* PUSHT and PUSHF are not defined, using PUSH1 and PUSH0 instead */
        case NEOC_OP_PUSHA: return "PUSHA";
        case NEOC_OP_PUSHNULL: return "PUSHNULL";
        case NEOC_OP_PUSHDATA1: return "PUSHDATA1";
        case NEOC_OP_PUSHDATA2: return "PUSHDATA2";
        case NEOC_OP_PUSHDATA4: return "PUSHDATA4";
        case NEOC_OP_PUSHM1: return "PUSHM1";
        case NEOC_OP_PUSH0: return "PUSH0";
        case NEOC_OP_PUSH1: return "PUSH1";
        case NEOC_OP_PUSH2: return "PUSH2";
        case NEOC_OP_PUSH3: return "PUSH3";
        case NEOC_OP_PUSH4: return "PUSH4";
        case NEOC_OP_PUSH5: return "PUSH5";
        case NEOC_OP_PUSH6: return "PUSH6";
        case NEOC_OP_PUSH7: return "PUSH7";
        case NEOC_OP_PUSH8: return "PUSH8";
        case NEOC_OP_PUSH9: return "PUSH9";
        case NEOC_OP_PUSH10: return "PUSH10";
        case NEOC_OP_PUSH11: return "PUSH11";
        case NEOC_OP_PUSH12: return "PUSH12";
        case NEOC_OP_PUSH13: return "PUSH13";
        case NEOC_OP_PUSH14: return "PUSH14";
        case NEOC_OP_PUSH15: return "PUSH15";
        case NEOC_OP_PUSH16: return "PUSH16";
        case NEOC_OP_NOP: return "NOP";
        case NEOC_OP_JMP: return "JMP";
        case NEOC_OP_JMP_L: return "JMP_L";
        case NEOC_OP_JMPIF: return "JMPIF";
        case NEOC_OP_JMPIF_L: return "JMPIF_L";
        case NEOC_OP_JMPIFNOT: return "JMPIFNOT";
        case NEOC_OP_JMPIFNOT_L: return "JMPIFNOT_L";
        case NEOC_OP_JMPEQ: return "JMPEQ";
        case NEOC_OP_JMPEQ_L: return "JMPEQ_L";
        case NEOC_OP_JMPNE: return "JMPNE";
        case NEOC_OP_JMPNE_L: return "JMPNE_L";
        case NEOC_OP_JMPGT: return "JMPGT";
        case NEOC_OP_JMPGT_L: return "JMPGT_L";
        case NEOC_OP_JMPGE: return "JMPGE";
        case NEOC_OP_JMPGE_L: return "JMPGE_L";
        case NEOC_OP_JMPLT: return "JMPLT";
        case NEOC_OP_JMPLT_L: return "JMPLT_L";
        case NEOC_OP_JMPLE: return "JMPLE";
        case NEOC_OP_JMPLE_L: return "JMPLE_L";
        case NEOC_OP_CALL: return "CALL";
        case NEOC_OP_CALL_L: return "CALL_L";
        case NEOC_OP_CALLA: return "CALLA";
        case NEOC_OP_CALLT: return "CALLT";
        case NEOC_OP_ABORT: return "ABORT";
        case NEOC_OP_ASSERT: return "ASSERT";
        case NEOC_OP_THROW: return "THROW";
        case NEOC_OP_TRY: return "TRY";
        case NEOC_OP_TRY_L: return "TRY_L";
        case NEOC_OP_ENDTRY: return "ENDTRY";
        case NEOC_OP_ENDTRY_L: return "ENDTRY_L";
        case NEOC_OP_ENDFINALLY: return "ENDFINALLY";
        case NEOC_OP_RET: return "RET";
        case NEOC_OP_SYSCALL: return "SYSCALL";
        default: return "UNKNOWN";
    }
}
