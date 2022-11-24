# test program 
add_executable(test_motor test.c ${SOURCE_FILES})
target_link_libraries(test_motor pigpio hiredis m pthread)
