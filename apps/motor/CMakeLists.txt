# test program 
add_executable(motor_driver driver.c ${SOURCE_FILES})
target_link_libraries(motor_driver pigpio hiredis m pthread)
