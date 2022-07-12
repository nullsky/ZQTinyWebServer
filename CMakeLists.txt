cmake_minimum_required(VERSION 3.21)
project(ZQTinyWebServer)

set(CMAKE_CXX_STANDARD 14)

# 设置别名
set(MYSQL_HEADER /usr/local/mysql/include)
set(MYSQL_LIB /usr/local/mysql/lib)

include_directories("./lock" "./log" "./sql")
# 添加 MySQL 连接头文件
include_directories(${MYSQL_HEADER})
# 添加 MySQL 外部库
link_libraries(${MYSQL_LIB})

add_executable(ZQTinyWebServer main.cpp lock/locker.h log/block_queue.h log/log.cpp log/log.h sql/sql_connection_pool.cpp sql/sql_connection_pool.h threadPool/thread_pool.h http/HttpConn.cpp http/HttpConn.h)

# 添加链接库(必须在add_executable之后)
target_link_libraries(ZQTinyWebServer ${MYSQL_LIB}/libmysqlclient.a)
target_link_libraries(ZQTinyWebServer ${MYSQL_LIB}/libmysqlservices.a)
target_link_libraries(ZQTinyWebServer ${MYSQL_LIB}/libssl.dylib)
target_link_libraries(ZQTinyWebServer ${MYSQL_LIB}/libcrypto.dylib)