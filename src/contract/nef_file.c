/**
 * @file nef_file.c
 * @brief Neo Executable Format (NEF) file implementation
 */

#include "neoc/contract/nef_file.h"
#include "neoc/crypto/neoc_hash.h"
#include "neoc/serialization/binary_writer.h"
#include "neoc/serialization/binary_reader.h"
#include "neoc/neoc_memory.h"
#include <string.h>
#include <stdio.h>

// NEF file structure
struct neoc_nef_file_t {
    neoc_nef_header_t header;
    neoc_nef_method_token_t *tokens;
    size_t token_count;
    size_t token_capacity;
    uint8_t *script;
    size_t script_size;
    neoc_hash256_t checksum;
    bool checksum_valid;
};

// Helper to calculate checksum
static neoc_error_t calculate_checksum(const neoc_nef_file_t *nef, neoc_hash256_t *checksum) {
    // Calculate checksum of header + tokens + script
    neoc_binary_writer_t *writer = NULL;
    neoc_error_t err = neoc_binary_writer_create(1024, true, &writer);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Write magic
    err = neoc_binary_writer_write_uint32(writer, nef->header.magic);
    if (err != NEOC_SUCCESS) {
        neoc_binary_writer_free(writer);
        return err;
    }
    
    // Write compiler (64 bytes)
    err = neoc_binary_writer_write_bytes(writer, (uint8_t*)nef->header.compiler, 64);
    if (err != NEOC_SUCCESS) {
        neoc_binary_writer_free(writer);
        return err;
    }
    
    // Write source length and source
    uint8_t source_len = strlen(nef->header.source);
    err = neoc_binary_writer_write_byte(writer, source_len);
    if (err != NEOC_SUCCESS) {
        neoc_binary_writer_free(writer);
        return err;
    }
    
    if (source_len > 0) {
        err = neoc_binary_writer_write_bytes(writer, (uint8_t*)nef->header.source, source_len);
        if (err != NEOC_SUCCESS) {
            neoc_binary_writer_free(writer);
            return err;
        }
    }
    
    // Write reserved bytes
    err = neoc_binary_writer_write_bytes(writer, nef->header.reserved, 2);
    if (err != NEOC_SUCCESS) {
        neoc_binary_writer_free(writer);
        return err;
    }
    
    // Write method tokens count
    err = neoc_binary_writer_write_byte(writer, (uint8_t)nef->token_count);
    if (err != NEOC_SUCCESS) {
        neoc_binary_writer_free(writer);
        return err;
    }
    
    // Write method tokens
    for (size_t i = 0; i < nef->token_count; i++) {
        // Write hash
        uint8_t hash_bytes[20];
        neoc_hash160_to_bytes(&nef->tokens[i].hash, hash_bytes, sizeof(hash_bytes));
        err = neoc_binary_writer_write_bytes(writer, hash_bytes, 20);
        if (err != NEOC_SUCCESS) {
            neoc_binary_writer_free(writer);
            return err;
        }
        
        // Write method
        uint8_t method_len = strlen(nef->tokens[i].method);
        err = neoc_binary_writer_write_byte(writer, method_len);
        if (err != NEOC_SUCCESS) {
            neoc_binary_writer_free(writer);
            return err;
        }
        
        err = neoc_binary_writer_write_bytes(writer, (uint8_t*)nef->tokens[i].method, method_len);
        if (err != NEOC_SUCCESS) {
            neoc_binary_writer_free(writer);
            return err;
        }
        
        // Write parameters count
        err = neoc_binary_writer_write_uint16(writer, nef->tokens[i].parameters_count);
        if (err != NEOC_SUCCESS) {
            neoc_binary_writer_free(writer);
            return err;
        }
        
        // Write has return value
        err = neoc_binary_writer_write_byte(writer, nef->tokens[i].has_return_value ? 1 : 0);
        if (err != NEOC_SUCCESS) {
            neoc_binary_writer_free(writer);
            return err;
        }
        
        // Write call flags
        err = neoc_binary_writer_write_byte(writer, nef->tokens[i].call_flags);
        if (err != NEOC_SUCCESS) {
            neoc_binary_writer_free(writer);
            return err;
        }
    }
    
    // Write reserved2
    err = neoc_binary_writer_write_byte(writer, nef->header.reserved2);
    if (err != NEOC_SUCCESS) {
        neoc_binary_writer_free(writer);
        return err;
    }
    
    // Write script
    err = neoc_binary_writer_write_var_bytes(writer, nef->script, nef->script_size);
    if (err != NEOC_SUCCESS) {
        neoc_binary_writer_free(writer);
        return err;
    }
    
    // Get serialized data
    uint8_t *data = NULL;
    size_t data_size = 0;
    err = neoc_binary_writer_to_array(writer, &data, &data_size);
    if (err != NEOC_SUCCESS) {
        neoc_binary_writer_free(writer);
        return err;
    }
    
    // Calculate double SHA256
    uint8_t checksum_bytes[32];
    err = neoc_sha256_double(data, data_size, checksum_bytes);
    if (err == NEOC_SUCCESS) {
        err = neoc_hash256_from_bytes(checksum, checksum_bytes);
    }
    
    neoc_free(data);
    neoc_binary_writer_free(writer);
    
    return err;
}

neoc_error_t neoc_nef_file_create(const char *compiler,
                                   const char *source,
                                   const uint8_t *script,
                                   size_t script_size,
                                   neoc_nef_file_t **nef) {
    if (!compiler || !script || !nef) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (script_size > NEF_MAX_SCRIPT_SIZE) {
        return neoc_error_set(NEOC_ERROR_INVALID_SIZE, "Script too large");
    }
    
    if (strlen(compiler) > 63) {
        return neoc_error_set(NEOC_ERROR_INVALID_SIZE, "Compiler name too long");
    }
    
    if (source && strlen(source) > NEF_MAX_SOURCE_SIZE) {
        return neoc_error_set(NEOC_ERROR_INVALID_SIZE, "Source URL too long");
    }
    
    *nef = neoc_calloc(1, sizeof(neoc_nef_file_t));
    if (!*nef) {
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate NEF file");
    }
    
    // Set header
    (*nef)->header.magic = NEF_MAGIC;
    strncpy((*nef)->header.compiler, compiler, 63);
    (*nef)->header.compiler[63] = '\0';
    
    if (source) {
        strncpy((*nef)->header.source, source, NEF_MAX_SOURCE_SIZE);
        (*nef)->header.source[NEF_MAX_SOURCE_SIZE] = '\0';
    }
    
    // Initialize tokens array
    (*nef)->token_capacity = 10;
    (*nef)->tokens = neoc_calloc((*nef)->token_capacity, sizeof(neoc_nef_method_token_t));
    if (!(*nef)->tokens) {
        neoc_free(*nef);
        *nef = NULL;
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate tokens");
    }
    
    // Copy script
    (*nef)->script = neoc_malloc(script_size);
    if (!(*nef)->script) {
        neoc_free((*nef)->tokens);
        neoc_free(*nef);
        *nef = NULL;
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate script");
    }
    
    memcpy((*nef)->script, script, script_size);
    (*nef)->script_size = script_size;
    
    // Calculate checksum
    neoc_error_t err = calculate_checksum(*nef, &(*nef)->checksum);
    if (err != NEOC_SUCCESS) {
        neoc_free((*nef)->script);
        neoc_free((*nef)->tokens);
        neoc_free(*nef);
        *nef = NULL;
        return err;
    }
    
    (*nef)->checksum_valid = true;
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_nef_file_add_method_token(neoc_nef_file_t *nef,
                                             const neoc_hash160_t *hash,
                                             const char *method,
                                             uint16_t parameters_count,
                                             bool has_return_value,
                                             uint8_t call_flags) {
    if (!nef || !hash || !method) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (strlen(method) > 31) {
        return neoc_error_set(NEOC_ERROR_INVALID_SIZE, "Method name too long");
    }
    
    if (nef->token_count >= NEF_MAX_TOKENS) {
        return neoc_error_set(NEOC_ERROR_INVALID_SIZE, "Too many method tokens");
    }
    
    // Resize array if needed
    if (nef->token_count >= nef->token_capacity) {
        size_t new_capacity = nef->token_capacity * 2;
        if (new_capacity > NEF_MAX_TOKENS) {
            new_capacity = NEF_MAX_TOKENS;
        }
        
        neoc_nef_method_token_t *new_tokens = neoc_realloc(nef->tokens,
                                                             new_capacity * sizeof(neoc_nef_method_token_t));
        if (!new_tokens) {
            return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to resize tokens");
        }
        
        nef->tokens = new_tokens;
        nef->token_capacity = new_capacity;
    }
    
    // Add token
    neoc_nef_method_token_t *token = &nef->tokens[nef->token_count];
    memcpy(&token->hash, hash, sizeof(neoc_hash160_t));
    strncpy(token->method, method, 31);
    token->method[31] = '\0';
    token->parameters_count = parameters_count;
    token->has_return_value = has_return_value;
    token->call_flags = call_flags;
    
    nef->token_count++;
    
    // Recalculate checksum
    return calculate_checksum(nef, &nef->checksum);
}

neoc_error_t neoc_nef_file_get_script(const neoc_nef_file_t *nef,
                                       const uint8_t **script,
                                       size_t *script_size) {
    if (!nef || !script || !script_size) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    *script = nef->script;
    *script_size = nef->script_size;
    
    return NEOC_SUCCESS;
}

const char* neoc_nef_file_get_compiler(const neoc_nef_file_t *nef) {
    return nef ? nef->header.compiler : NULL;
}

const char* neoc_nef_file_get_source(const neoc_nef_file_t *nef) {
    return (nef && nef->header.source[0]) ? nef->header.source : NULL;
}

neoc_error_t neoc_nef_file_get_checksum(const neoc_nef_file_t *nef,
                                         neoc_hash256_t *checksum) {
    if (!nef || !checksum) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    memcpy(checksum, &nef->checksum, sizeof(neoc_hash256_t));
    return NEOC_SUCCESS;
}

bool neoc_nef_file_verify_checksum(const neoc_nef_file_t *nef) {
    if (!nef) {
        return false;
    }
    
    neoc_hash256_t calculated;
    neoc_error_t err = calculate_checksum(nef, &calculated);
    if (err != NEOC_SUCCESS) {
        return false;
    }
    
    return neoc_hash256_equal(&calculated, &nef->checksum);
}

size_t neoc_nef_file_get_token_count(const neoc_nef_file_t *nef) {
    return nef ? nef->token_count : 0;
}

neoc_error_t neoc_nef_file_get_token(const neoc_nef_file_t *nef,
                                      size_t index,
                                      const neoc_nef_method_token_t **token) {
    if (!nef || !token) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (index >= nef->token_count) {
        return neoc_error_set(NEOC_ERROR_OUT_OF_BOUNDS, "Index out of bounds");
    }
    
    *token = &nef->tokens[index];
    return NEOC_SUCCESS;
}

neoc_error_t neoc_nef_file_to_bytes(const neoc_nef_file_t *nef,
                                     uint8_t **data,
                                     size_t *data_size) {
    if (!nef || !data || !data_size) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    neoc_binary_writer_t *writer = NULL;
    neoc_error_t err = neoc_binary_writer_create(1024, true, &writer);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    // Write magic
    err = neoc_binary_writer_write_uint32(writer, nef->header.magic);
    if (err != NEOC_SUCCESS) {
        neoc_binary_writer_free(writer);
        return err;
    }
    
    // Write compiler (64 bytes)
    uint8_t compiler_buf[64] = {0};
    size_t compiler_len = strlen(nef->header.compiler);
    if (compiler_len > sizeof(compiler_buf) - 1) {
        compiler_len = sizeof(compiler_buf) - 1;
    }
    memcpy(compiler_buf, nef->header.compiler, compiler_len);
    err = neoc_binary_writer_write_bytes(writer, compiler_buf, 64);
    if (err != NEOC_SUCCESS) {
        neoc_binary_writer_free(writer);
        return err;
    }
    
    // Write source
    uint8_t source_len = strlen(nef->header.source);
    err = neoc_binary_writer_write_byte(writer, source_len);
    if (err != NEOC_SUCCESS) {
        neoc_binary_writer_free(writer);
        return err;
    }
    
    if (source_len > 0) {
        err = neoc_binary_writer_write_bytes(writer, (uint8_t*)nef->header.source, source_len);
        if (err != NEOC_SUCCESS) {
            neoc_binary_writer_free(writer);
            return err;
        }
    }
    
    // Write reserved bytes
    err = neoc_binary_writer_write_bytes(writer, nef->header.reserved, 2);
    if (err != NEOC_SUCCESS) {
        neoc_binary_writer_free(writer);
        return err;
    }
    
    // Write method tokens count
    err = neoc_binary_writer_write_byte(writer, (uint8_t)nef->token_count);
    if (err != NEOC_SUCCESS) {
        neoc_binary_writer_free(writer);
        return err;
    }
    
    // Write method tokens
    for (size_t i = 0; i < nef->token_count; i++) {
        // Write hash
        uint8_t hash_bytes[20];
        neoc_hash160_to_bytes(&nef->tokens[i].hash, hash_bytes, sizeof(hash_bytes));
        err = neoc_binary_writer_write_bytes(writer, hash_bytes, 20);
        if (err != NEOC_SUCCESS) {
            neoc_binary_writer_free(writer);
            return err;
        }
        
        // Write method
        uint8_t method_len = strlen(nef->tokens[i].method);
        err = neoc_binary_writer_write_byte(writer, method_len);
        if (err != NEOC_SUCCESS) {
            neoc_binary_writer_free(writer);
            return err;
        }
        
        err = neoc_binary_writer_write_bytes(writer, (uint8_t*)nef->tokens[i].method, method_len);
        if (err != NEOC_SUCCESS) {
            neoc_binary_writer_free(writer);
            return err;
        }
        
        // Write parameters count
        err = neoc_binary_writer_write_uint16(writer, nef->tokens[i].parameters_count);
        if (err != NEOC_SUCCESS) {
            neoc_binary_writer_free(writer);
            return err;
        }
        
        // Write has return value
        err = neoc_binary_writer_write_byte(writer, nef->tokens[i].has_return_value ? 1 : 0);
        if (err != NEOC_SUCCESS) {
            neoc_binary_writer_free(writer);
            return err;
        }
        
        // Write call flags
        err = neoc_binary_writer_write_byte(writer, nef->tokens[i].call_flags);
        if (err != NEOC_SUCCESS) {
            neoc_binary_writer_free(writer);
            return err;
        }
    }
    
    // Write reserved2
    err = neoc_binary_writer_write_byte(writer, nef->header.reserved2);
    if (err != NEOC_SUCCESS) {
        neoc_binary_writer_free(writer);
        return err;
    }
    
    // Write script
    err = neoc_binary_writer_write_var_bytes(writer, nef->script, nef->script_size);
    if (err != NEOC_SUCCESS) {
        neoc_binary_writer_free(writer);
        return err;
    }
    
    // Write checksum
    uint8_t checksum_bytes[32];
    neoc_hash256_to_bytes(&nef->checksum, checksum_bytes, sizeof(checksum_bytes));
    err = neoc_binary_writer_write_bytes(writer, checksum_bytes, 32);
    if (err != NEOC_SUCCESS) {
        neoc_binary_writer_free(writer);
        return err;
    }
    
    // Get result
    err = neoc_binary_writer_to_array(writer, data, data_size);
    neoc_binary_writer_free(writer);
    
    return err;
}

neoc_error_t neoc_nef_file_to_file(const neoc_nef_file_t *nef,
                                    const char *filename) {
    if (!nef || !filename) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    uint8_t *data = NULL;
    size_t data_size = 0;
    neoc_error_t err = neoc_nef_file_to_bytes(nef, &data, &data_size);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    FILE *file = fopen(filename, "wb");
    if (!file) {
        neoc_free(data);
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Failed to open file");
    }
    
    size_t written = fwrite(data, 1, data_size, file);
    fclose(file);
    neoc_free(data);
    
    if (written != data_size) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Failed to write file");
    }
    
    return NEOC_SUCCESS;
}

neoc_error_t neoc_nef_file_from_bytes(const uint8_t *data,
                                       size_t data_size,
                                       neoc_nef_file_t **nef) {
    if (!data || !nef) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    if (data_size < sizeof(neoc_nef_header_t)) {
        return neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Data too small for NEF header");
    }
    
    neoc_binary_reader_t *reader = NULL;
    neoc_error_t err = neoc_binary_reader_create(data, data_size, &reader);
    if (err != NEOC_SUCCESS) {
        return err;
    }
    
    *nef = neoc_calloc(1, sizeof(neoc_nef_file_t));
    if (!*nef) {
        neoc_binary_reader_free(reader);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate NEF file");
    }
    
    // Read header
    err = neoc_binary_reader_read_uint32(reader, &(*nef)->header.magic);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    // Verify magic number
    if ((*nef)->header.magic != NEF_MAGIC) {
        err = neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Invalid NEF magic number");
        goto cleanup;
    }
    
    // Read compiler (fixed 64 bytes)
    err = neoc_binary_reader_read_bytes(reader, (uint8_t*)(*nef)->header.compiler, 64);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    // Read source length and source string
    uint8_t source_len;
    err = neoc_binary_reader_read_byte(reader, &source_len);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    if (source_len > 0) {
        if (source_len > 0 && source_len >= 255) {
            err = neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Source string too long");
            goto cleanup;
        }
        err = neoc_binary_reader_read_bytes(reader, (uint8_t*)(*nef)->header.source, source_len);
        if (err != NEOC_SUCCESS) goto cleanup;
        (*nef)->header.source[source_len] = '\0';
    }
    
    // Read reserved bytes
    err = neoc_binary_reader_read_bytes(reader, (*nef)->header.reserved, 2);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    // Read method tokens count
    uint8_t token_count;
    err = neoc_binary_reader_read_byte(reader, &token_count);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    (*nef)->token_count = token_count;
    
    // Read method tokens
    if (token_count > 0) {
        (*nef)->tokens = neoc_calloc(token_count, sizeof(neoc_nef_method_token_t));
        if (!(*nef)->tokens) {
            err = neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate tokens");
            goto cleanup;
        }
        
        for (size_t i = 0; i < token_count; i++) {
            // Read hash
            uint8_t hash_bytes[20];
            err = neoc_binary_reader_read_bytes(reader, hash_bytes, 20);
            if (err != NEOC_SUCCESS) goto cleanup;
            neoc_hash160_from_bytes(&(*nef)->tokens[i].hash, hash_bytes);
            
            // Read method length and method string
            uint8_t method_len;
            err = neoc_binary_reader_read_byte(reader, &method_len);
            if (err != NEOC_SUCCESS) goto cleanup;
            
            if (method_len >= sizeof((*nef)->tokens[i].method)) {
                err = neoc_error_set(NEOC_ERROR_INVALID_FORMAT, "Method name too long");
                goto cleanup;
            }
            
            err = neoc_binary_reader_read_bytes(reader, (uint8_t*)(*nef)->tokens[i].method, method_len);
            if (err != NEOC_SUCCESS) goto cleanup;
            (*nef)->tokens[i].method[method_len] = '\0';
            
            // Read parameters count
            err = neoc_binary_reader_read_uint16(reader, &(*nef)->tokens[i].parameters_count);
            if (err != NEOC_SUCCESS) goto cleanup;
            
            // Read has return value
            uint8_t has_return;
            err = neoc_binary_reader_read_byte(reader, &has_return);
            if (err != NEOC_SUCCESS) goto cleanup;
            (*nef)->tokens[i].has_return_value = (has_return != 0);
            
            // Read call flags
            err = neoc_binary_reader_read_byte(reader, &(*nef)->tokens[i].call_flags);
            if (err != NEOC_SUCCESS) goto cleanup;
        }
    }
    
    // Read reserved2
    err = neoc_binary_reader_read_byte(reader, &(*nef)->header.reserved2);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    // Read script
    err = neoc_binary_reader_read_var_bytes(reader, &(*nef)->script, &(*nef)->script_size);
    if (err != NEOC_SUCCESS) goto cleanup;
    
    // Read checksum
    uint8_t checksum_bytes[32];
    err = neoc_binary_reader_read_bytes(reader, checksum_bytes, 32);
    if (err != NEOC_SUCCESS) goto cleanup;
    neoc_hash256_from_bytes(&(*nef)->checksum, checksum_bytes);
    
    neoc_binary_reader_free(reader);
    return NEOC_SUCCESS;
    
cleanup:
    neoc_binary_reader_free(reader);
    neoc_nef_file_free(*nef);
    *nef = NULL;
    return err;
}

neoc_error_t neoc_nef_file_from_file(const char *filename,
                                      neoc_nef_file_t **nef) {
    if (!filename || !nef) {
        return neoc_error_set(NEOC_ERROR_INVALID_ARGUMENT, "Invalid arguments");
    }
    
    FILE *file = fopen(filename, "rb");
    if (!file) {
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Failed to open file");
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size <= 0) {
        fclose(file);
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Invalid file size");
    }
    
    uint8_t *data = neoc_malloc(file_size);
    if (!data) {
        fclose(file);
        return neoc_error_set(NEOC_ERROR_MEMORY, "Failed to allocate buffer");
    }
    
    size_t read = fread(data, 1, file_size, file);
    fclose(file);
    
    if (read != (size_t)file_size) {
        neoc_free(data);
        return neoc_error_set(NEOC_ERROR_INVALID_STATE, "Failed to read file");
    }
    
    neoc_error_t err = neoc_nef_file_from_bytes(data, file_size, nef);
    neoc_free(data);
    
    return err;
}

void neoc_nef_file_free(neoc_nef_file_t *nef) {
    if (!nef) return;
    
    neoc_free(nef->tokens);
    neoc_free(nef->script);
    neoc_free(nef);
}
