add_executable(basic_usage basic_usage.cpp)
target_link_libraries(basic_usage PRIVATE columnar)

add_executable(filtering filtering.cpp)
target_link_libraries(filtering PRIVATE columnar)

add_executable(particle_analysis particle_analysis.cpp)
target_link_libraries(particle_analysis PRIVATE columnar)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../tests/data
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})