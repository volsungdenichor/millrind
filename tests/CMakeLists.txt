set(CATCH_INCLUDE_DIR .)
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})


include_directories("${PROJECT_SOURCE_DIR}/include")
add_executable (tests main.cpp optional_tests.cpp)
target_link_libraries(tests Catch)