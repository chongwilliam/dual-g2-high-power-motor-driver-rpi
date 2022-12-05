# test program 
add_executable(test_timer test.c ${SOURCE_FILES})
target_link_libraries(test_timer pigpio hiredis m pthread)
