# test program 
add_executable(test_encoder test.c ${SOURCE_FILES})
target_link_libraries(test_encoder pigpio hiredis m pthread)
