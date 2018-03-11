project(ecs_tests)

add_executable(ecs_tests
        ecs_test.cpp ../datetime.cpp)
target_link_libraries(ecs_tests gtest gtest_main)
