cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)

project(find)

add_executable(find find.cpp
                    actor.cpp
                    actor.h
                    environment.cpp
                    environment.h
                    platform.h
                    programconditions.cpp
                    programconditions.h
                    query.cpp
                    query.h)

install(TARGETS find
        RUNTIME DESTINATION bin)
