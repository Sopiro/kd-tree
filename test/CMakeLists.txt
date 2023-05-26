add_executable(test
    doctest.h
    test.cpp
)

set_target_properties(test PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

target_include_directories(test PUBLIC ../include)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    doctest.h
    test.cpp
)