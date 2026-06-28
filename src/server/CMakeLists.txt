aux_source_directory(. SRC_LIST)

# 指定可生成文件
add_executable(ChatServer ${SRC_LIST})
# 指定库文件
target_link_libraries(ChatServer muduo_net muduo_base pthread)