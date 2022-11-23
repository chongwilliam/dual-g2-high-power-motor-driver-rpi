# test program 
add_executable(test test.c ${SOURCE_FILES})
target_link_libraries(test pigpio hiredis m pthread)
