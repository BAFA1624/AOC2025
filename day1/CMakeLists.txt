add_executable(day1 day1.cpp)
target_compile_features(day1 PUBLIC ${DEFAULT_COMPILE_FEATURES})
target_include_directories(day1 PRIVATE ${INCLUDE_DIRS})
