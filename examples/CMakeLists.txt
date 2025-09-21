cmake_minimum_required(VERSION 3.16)

# Basic example
add_executable(neoc_basic basic_example.c)
target_link_libraries(neoc_basic neoc ${OPENSSL_LIBRARIES})

# Encoding example
add_executable(neoc_encoding encoding_example.c)
target_link_libraries(neoc_encoding neoc ${OPENSSL_LIBRARIES})

# Crypto example
add_executable(neoc_crypto crypto_example.c)
target_link_libraries(neoc_crypto neoc ${OPENSSL_LIBRARIES})

# Comprehensive example
add_executable(neoc_comprehensive comprehensive_example.c)
target_link_libraries(neoc_comprehensive neoc ${OPENSSL_LIBRARIES})