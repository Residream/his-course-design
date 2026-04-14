cmake_minimum_required(VERSION 3.10)
project(HIS C)

set(CMAKE_C_STANDARD 11)

# 编译选项
if(MSVC)
    add_compile_options(/W4 /WX)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wall -Wextra -Werror -Wno-stringop-truncation -Wno-unused-result)
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang|AppleClang")
    add_compile_options(-Wall -Wextra -Werror -Wno-unused-result)
else()
    add_compile_options(-Wall -Wextra -Werror)
endif()

# # Windows 控制台中文输出
# if(WIN32)
#     add_compile_options(-finput-charset=UTF-8 -fexec-charset=GBK)
# endif()

# 头文件目录
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

# 收集所有源文件（递归搜索子目录）
file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.c
)

# 生成可执行文件
add_executable(his ${SOURCES})
