# test program 
add_executable(test_redis test.c ${SOURCE_FILES})
target_link_libraries(test_redis pigpio hiredis m pthread)
