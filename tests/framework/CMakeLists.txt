# 测试框架构建配置
# 配置Google Test、Google Mock和测试工具

cmake_minimum_required(VERSION 3.16)

# 查找Google Test
find_package(GTest REQUIRED)
if(NOT GTest_FOUND)
    # 如果没有找到系统安装的GTest，从源码构建
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    # 设置gtest_force_shared_crt为ON以Windows兼容性
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# 查找Google Benchmark (用于性能测试)
find_package(benchmark QUIET)
if(NOT benchmark_FOUND)
    message(STATUS "Google Benchmark not found. Performance tests will be limited.")
endif()

# 创建测试框架库
add_library(test_framework STATIC
    TestMain.cpp
)

# 设置包含目录
target_include_directories(test_framework
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
)

# 链接Google Test
target_link_libraries(test_framework
    PUBLIC
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        GTest::gmock_main
)

# 如果找到Google Benchmark，添加到构建中
if(benchmark_FOUND)
    target_link_libraries(test_framework PUBLIC benchmark::benchmark)
endif()

# 设置编译选项
target_compile_options(test_framework PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>  # MSVC: 启用警告并视为错误
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -Werror>  # GCC/Clang
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Werror>
)

# 启用代码覆盖率 (GCC/Clang)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_framework PRIVATE --coverage)
    target_link_options(test_framework PRIVATE --coverage)
endif()

# 设置输出目录
set_target_properties(test_framework PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# 安装规则 (可选)
install(TARGETS test_framework
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(FILES
    TestUtils.h
    TestFixtures.h
    DESTINATION include/test_framework
)
