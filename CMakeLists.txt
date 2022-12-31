# CMakeLists.txt
#
# CMake file for the Paho C++ demo application with sqlite.
#
#*******************************************************************************
# This is part of the Paho MQTT C++ client library.
#
# Copyright (c) 2016-2019
# 
# All rights reserved. This program and the accompanying materials
# are made available under the terms of the Eclipse Public License v1.0
# and Eclipse Distribution License v1.0 which accompany this distribution.
# 
# The Eclipse Public License is available at
#   http://www.eclipse.org/legal/epl-v10.html
# and the Eclipse Distribution License is available at
#   http://www.eclipse.org/org/documents/edl-v10.php.
#*******************************************************************************/

cmake_minimum_required(VERSION 3.15)
set(CMAKE_DEBUG_POSTFIX d)
project(mqtt_demo)

## --- Library dependencies ---

set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

## include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

## link directories
link_directories(${PROJECT_SOURCE_DIR}/lib)

add_executable(data_publish src/data_publish.cpp
        src/sqliteclient/sqliteclient.h
        src/sqliteclient/sqliteclient.cpp
        src/sqliteclient/sql.h)


add_executable(sqlite_test "")
target_sources(sqlite_test
        PRIVATE
        test/sqlite_test.cpp
        src/sqliteclient/sqliteclient.h
        src/sqliteclient/sqliteclient.cpp)

## link binaries
target_link_libraries(data_publish -lpaho-mqttpp3 -lpaho-mqtt3a -lpaho-mqtt3as -lsqlite3 -lcbor)
target_link_libraries(sqlite_test)


