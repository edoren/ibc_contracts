add_contract(ibc.proxy ibc.proxy src/ibc.proxy.cpp)
target_include_directories(ibc.proxy.wasm
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../ibc.chain/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../ibc.chain/src
    ${CMAKE_CURRENT_SOURCE_DIR}/../ibc.token/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../ibc.token/src)

set_target_properties(ibc.proxy.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
