cmake_minimum_required(VERSION 3.10)
project(FileSyncTool)

set(CMAKE_CXX_STANDARD 17)

# 设置 Boost 相关
find_package(Boost REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})

# 包含目录
include_directories(src/core src/network src/mode)

# 添加可执行文件
add_executable(FileSyncTool src/main.cpp src/core/file_watcher.cpp src/core/file_transfer.cpp src/core/conflict_resolver.cpp src/core/Loggerd.cpp)

# 链接 Boost 库
target_link_libraries(FileSyncTool ${Boost_LIBRARIES})

set_target_properties(FileSyncTool PROPERTIES
    OUTPUT_NAME "FileSyncTool"
    SUFFIX ".exe"
)