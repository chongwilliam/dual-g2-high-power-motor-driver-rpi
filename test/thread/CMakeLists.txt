# test program 
add_executable(test_thread test.c ${SOURCE_FILES})
target_link_libraries(test_thread pigpio hiredis m pthread)
