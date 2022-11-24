# test program 
add_executable(test_filter test.c ${SOURCE_FILES})
target_link_libraries(test_filter pigpio hiredis m pthread)
