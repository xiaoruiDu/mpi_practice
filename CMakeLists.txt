cmake_minimum_required(VERSION 3.15.2)

project(main)


find_package(nlohmann_json 3.2.0 REQUIRED)


add_executable(${PROJECT_NAME} main.cpp)

add_subdirectory(include/)

target_include_directories(${PROJECT_NAME} 
    PUBLIC include/
    PUBLIC include/
    
    )

target_link_directories(${PROJECT_NAME} 
    PRIVATE include/
    PRIVATE include/
    )


target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json )





