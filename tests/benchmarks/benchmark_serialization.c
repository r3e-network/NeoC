/**
 * @file benchmark_serialization.c
 * @brief Performance benchmarks for serialization operations
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h>
#include "neoc/neoc.h"
#include "neoc/serialization/binary_reader.h"
#include "neoc/serialization/binary_writer.h"
#include "neoc/types/hash160.h"
#include "neoc/types/hash256.h"
#include "neoc/types/contract_parameter.h"
#include "neoc/types/stack_item.h"
#include "neoc/transaction/transaction.h"

#define ITERATIONS 10000
#define WARMUP_ITERATIONS 1000

// Timing utilities
typedef struct {
    const char *name;
    clock_t start;
    clock_t end;
    int iterations;
} benchmark_t;

static void benchmark_start(benchmark_t *bench, const char *name, int iterations) {
    bench->name = name;
    bench->iterations = iterations;
    bench->start = clock();
}

static void benchmark_end(benchmark_t *bench) {
    bench->end = clock();
    double cpu_time_used = ((double)(bench->end - bench->start)) / CLOCKS_PER_SEC;
    double ops_per_sec = bench->iterations / cpu_time_used;
    double us_per_op = (cpu_time_used * 1000000) / bench->iterations;
    
    printf("%-35s: %10.2f ops/sec, %8.2f μs/op (%d iterations in %.3fs)\n",
           bench->name, ops_per_sec, us_per_op, bench->iterations, cpu_time_used);
}

// Benchmark binary writer operations
static void benchmark_binary_writer(void) {
    printf("\n=== Binary Writer Benchmarks ===\n");
    benchmark_t bench;
    
    // Warmup
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        neoc_binary_writer_t *writer = NULL;
        neoc_binary_writer_create(&writer);
        neoc_binary_writer_write_u8(writer, 0xFF);
        neoc_binary_writer_free(writer);
    }
    
    // Benchmark writer creation/destruction
    benchmark_start(&bench, "BinaryWriter Create/Destroy", ITERATIONS);
    for (int i = 0; i < ITERATIONS; i++) {
        neoc_binary_writer_t *writer = NULL;
        neoc_error_t err = neoc_binary_writer_create(&writer);
        assert(err == NEOC_SUCCESS);
        neoc_binary_writer_free(writer);
    }
    benchmark_end(&bench);
    
    // Create writer for operations
    neoc_binary_writer_t *writer = NULL;
    neoc_error_t err = neoc_binary_writer_create(&writer);
    assert(err == NEOC_SUCCESS);
    
    // Benchmark writing bytes
    benchmark_start(&bench, "Write U8", ITERATIONS * 10);
    for (int i = 0; i < ITERATIONS * 10; i++) {
        err = neoc_binary_writer_write_u8(writer, i & 0xFF);
        assert(err == NEOC_SUCCESS);
    }
    benchmark_end(&bench);
    
    // Reset writer
    neoc_binary_writer_free(writer);
    neoc_binary_writer_create(&writer);
    
    // Benchmark writing integers
    benchmark_start(&bench, "Write U32", ITERATIONS * 5);
    for (int i = 0; i < ITERATIONS * 5; i++) {
        err = neoc_binary_writer_write_u32_le(writer, i);
        assert(err == NEOC_SUCCESS);
    }
    benchmark_end(&bench);
    
    // Reset writer
    neoc_binary_writer_free(writer);
    neoc_binary_writer_create(&writer);
    
    // Benchmark writing variable integers
    benchmark_start(&bench, "Write VarInt", ITERATIONS * 5);
    for (int i = 0; i < ITERATIONS * 5; i++) {
        err = neoc_binary_writer_write_var_int(writer, i);
        assert(err == NEOC_SUCCESS);
    }
    benchmark_end(&bench);
    
    // Reset writer
    neoc_binary_writer_free(writer);
    neoc_binary_writer_create(&writer);
    
    // Benchmark writing byte arrays
    uint8_t data[256];
    for (int i = 0; i < sizeof(data); i++) {
        data[i] = i;
    }
    
    benchmark_start(&bench, "Write Bytes (256 bytes)", ITERATIONS);
    for (int i = 0; i < ITERATIONS; i++) {
        err = neoc_binary_writer_write_bytes(writer, data, sizeof(data));
        assert(err == NEOC_SUCCESS);
    }
    benchmark_end(&bench);
    
    // Get serialized data
    uint8_t *output = NULL;
    size_t output_len = 0;
    err = neoc_binary_writer_to_array(writer, &output, &output_len);
    assert(err == NEOC_SUCCESS);
    
    free(output);
    neoc_binary_writer_free(writer);
}

// Benchmark binary reader operations
static void benchmark_binary_reader(void) {
    printf("\n=== Binary Reader Benchmarks ===\n");
    benchmark_t bench;
    
    // Prepare test data
    uint8_t test_data[4096];
    for (int i = 0; i < sizeof(test_data); i++) {
        test_data[i] = i & 0xFF;
    }
    
    // Benchmark reader creation/destruction
    benchmark_start(&bench, "BinaryReader Create/Destroy", ITERATIONS);
    for (int i = 0; i < ITERATIONS; i++) {
        neoc_binary_reader_t *reader = NULL;
        neoc_error_t err = neoc_binary_reader_create(test_data, sizeof(test_data), &reader);
        assert(err == NEOC_SUCCESS);
        neoc_binary_reader_free(reader);
    }
    benchmark_end(&bench);
    
    // Create reader for operations
    neoc_binary_reader_t *reader = NULL;
    neoc_error_t err = neoc_binary_reader_create(test_data, sizeof(test_data), &reader);
    assert(err == NEOC_SUCCESS);
    
    // Benchmark reading bytes
    benchmark_start(&bench, "Read U8", ITERATIONS * 10);
    for (int i = 0; i < ITERATIONS * 10; i++) {
        uint8_t value;
        err = neoc_binary_reader_read_u8(reader, &value);
        assert(err == NEOC_SUCCESS);
    }
    benchmark_end(&bench);
    
    // Reset reader
    neoc_binary_reader_free(reader);
    neoc_binary_reader_create(test_data, sizeof(test_data), &reader);
    
    // Benchmark reading integers
    benchmark_start(&bench, "Read U32", ITERATIONS * 5);
    for (int i = 0; i < ITERATIONS * 5; i++) {
        uint32_t value;
        err = neoc_binary_reader_read_u32_le(reader, &value);
        assert(err == NEOC_SUCCESS);
    }
    benchmark_end(&bench);
    
    // Prepare data with variable integers
    neoc_binary_writer_t *writer = NULL;
    neoc_binary_writer_create(&writer);
    for (int i = 0; i < 1000; i++) {
        neoc_binary_writer_write_var_int(writer, i);
    }
    uint8_t *var_int_data = NULL;
    size_t var_int_len = 0;
    neoc_binary_writer_to_array(writer, &var_int_data, &var_int_len);
    
    // Reset reader with variable integer data
    neoc_binary_reader_free(reader);
    neoc_binary_reader_create(var_int_data, var_int_len, &reader);
    
    // Benchmark reading variable integers
    benchmark_start(&bench, "Read VarInt", ITERATIONS);
    for (int i = 0; i < ITERATIONS; i++) {
        // Reset position for each iteration
        if (i % 1000 == 0) {
            neoc_binary_reader_free(reader);
            neoc_binary_reader_create(var_int_data, var_int_len, &reader);
        }
        uint64_t value;
        err = neoc_binary_reader_read_var_int(reader, &value);
        assert(err == NEOC_SUCCESS);
    }
    benchmark_end(&bench);
    
    free(var_int_data);
    neoc_binary_writer_free(writer);
    neoc_binary_reader_free(reader);
}

// Benchmark hash serialization
static void benchmark_hash_serialization(void) {
    printf("\n=== Hash Serialization Benchmarks ===\n");
    benchmark_t bench;
    
    // Create test hashes
    neoc_hash160_t *hash160 = NULL;
    neoc_error_t err = neoc_hash160_from_string("0x969a77db482f74ce27105f760efa139223431394", &hash160);
    assert(err == NEOC_SUCCESS);
    
    neoc_hash256_t *hash256 = NULL;
    err = neoc_hash256_from_string("0x0000000000000000000000000000000000000000000000000000000000000001", &hash256);
    assert(err == NEOC_SUCCESS);
    
    neoc_binary_writer_t *writer = NULL;
    neoc_binary_writer_create(&writer);
    
    // Benchmark Hash160 serialization
    benchmark_start(&bench, "Hash160 Serialize", ITERATIONS);
    for (int i = 0; i < ITERATIONS; i++) {
        err = neoc_hash160_serialize(hash160, writer);
        assert(err == NEOC_SUCCESS);
    }
    benchmark_end(&bench);
    
    // Get serialized data
    uint8_t *data = NULL;
    size_t data_len = 0;
    neoc_binary_writer_to_array(writer, &data, &data_len);
    
    // Benchmark Hash160 deserialization
    neoc_binary_reader_t *reader = NULL;
    benchmark_start(&bench, "Hash160 Deserialize", ITERATIONS);
    for (int i = 0; i < ITERATIONS; i++) {
        neoc_binary_reader_create(data, 20, &reader);
        neoc_hash160_t *temp = NULL;
        err = neoc_hash160_deserialize(reader, &temp);
        assert(err == NEOC_SUCCESS);
        neoc_hash160_free(temp);
        neoc_binary_reader_free(reader);
    }
    benchmark_end(&bench);
    
    free(data);
    neoc_binary_writer_free(writer);
    neoc_binary_writer_create(&writer);
    
    // Benchmark Hash256 serialization
    benchmark_start(&bench, "Hash256 Serialize", ITERATIONS);
    for (int i = 0; i < ITERATIONS; i++) {
        err = neoc_hash256_serialize(hash256, writer);
        assert(err == NEOC_SUCCESS);
    }
    benchmark_end(&bench);
    
    // Get serialized data
    neoc_binary_writer_to_array(writer, &data, &data_len);
    
    // Benchmark Hash256 deserialization
    benchmark_start(&bench, "Hash256 Deserialize", ITERATIONS);
    for (int i = 0; i < ITERATIONS; i++) {
        neoc_binary_reader_create(data, 32, &reader);
        neoc_hash256_t *temp = NULL;
        err = neoc_hash256_deserialize(reader, &temp);
        assert(err == NEOC_SUCCESS);
        neoc_hash256_free(temp);
        neoc_binary_reader_free(reader);
    }
    benchmark_end(&bench);
    
    free(data);
    neoc_binary_writer_free(writer);
    neoc_hash160_free(hash160);
    neoc_hash256_free(hash256);
}

// Benchmark contract parameter serialization
static void benchmark_contract_parameter(void) {
    printf("\n=== Contract Parameter Benchmarks ===\n");
    benchmark_t bench;
    
    // Create various parameter types
    neoc_contract_parameter_t *int_param = NULL;
    neoc_error_t err = neoc_contract_parameter_create_integer(42, &int_param);
    assert(err == NEOC_SUCCESS);
    
    neoc_contract_parameter_t *bool_param = NULL;
    err = neoc_contract_parameter_create_boolean(true, &bool_param);
    assert(err == NEOC_SUCCESS);
    
    neoc_contract_parameter_t *string_param = NULL;
    err = neoc_contract_parameter_create_string("Hello, Neo!", &string_param);
    assert(err == NEOC_SUCCESS);
    
    uint8_t bytes[32];
    for (int i = 0; i < 32; i++) bytes[i] = i;
    neoc_contract_parameter_t *byte_param = NULL;
    err = neoc_contract_parameter_create_byte_array(bytes, sizeof(bytes), &byte_param);
    assert(err == NEOC_SUCCESS);
    
    neoc_binary_writer_t *writer = NULL;
    
    // Benchmark integer parameter serialization
    benchmark_start(&bench, "ContractParameter (Integer) Serialize", ITERATIONS);
    for (int i = 0; i < ITERATIONS; i++) {
        neoc_binary_writer_create(&writer);
        err = neoc_contract_parameter_serialize(int_param, writer);
        assert(err == NEOC_SUCCESS);
        neoc_binary_writer_free(writer);
    }
    benchmark_end(&bench);
    
    // Benchmark boolean parameter serialization
    benchmark_start(&bench, "ContractParameter (Boolean) Serialize", ITERATIONS);
    for (int i = 0; i < ITERATIONS; i++) {
        neoc_binary_writer_create(&writer);
        err = neoc_contract_parameter_serialize(bool_param, writer);
        assert(err == NEOC_SUCCESS);
        neoc_binary_writer_free(writer);
    }
    benchmark_end(&bench);
    
    // Benchmark string parameter serialization
    benchmark_start(&bench, "ContractParameter (String) Serialize", ITERATIONS);
    for (int i = 0; i < ITERATIONS; i++) {
        neoc_binary_writer_create(&writer);
        err = neoc_contract_parameter_serialize(string_param, writer);
        assert(err == NEOC_SUCCESS);
        neoc_binary_writer_free(writer);
    }
    benchmark_end(&bench);
    
    // Benchmark byte array parameter serialization
    benchmark_start(&bench, "ContractParameter (ByteArray) Serialize", ITERATIONS);
    for (int i = 0; i < ITERATIONS; i++) {
        neoc_binary_writer_create(&writer);
        err = neoc_contract_parameter_serialize(byte_param, writer);
        assert(err == NEOC_SUCCESS);
        neoc_binary_writer_free(writer);
    }
    benchmark_end(&bench);
    
    // Cleanup
    neoc_contract_parameter_free(int_param);
    neoc_contract_parameter_free(bool_param);
    neoc_contract_parameter_free(string_param);
    neoc_contract_parameter_free(byte_param);
}

int main(void) {
    printf("=================================================\n");
    printf("      NeoC SDK Serialization Benchmarks\n");
    printf("=================================================\n");
    printf("CPU: Performance measured in operations per second\n");
    printf("All times are CPU time, not wall clock time\n");
    
    // Initialize NeoC
    neoc_error_t err = neoc_init();
    assert(err == NEOC_SUCCESS);
    
    // Run benchmarks
    benchmark_binary_writer();
    benchmark_binary_reader();
    benchmark_hash_serialization();
    benchmark_contract_parameter();
    
    // Cleanup
    neoc_cleanup();
    
    printf("\n=================================================\n");
    printf("               Benchmarks Complete\n");
    printf("=================================================\n");
    
    return 0;
}