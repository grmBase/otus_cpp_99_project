cmake_minimum_required(VERSION 3.2)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})


# имя проекта
project(mrpc_boost VERSION ${PROJECT_VESRION})



# настройки для boost-а:
#set(Boost_USE_STATIC_LIBS OFF) 
#set(Boost_USE_MULTITHREADED ON)  
#set(Boost_USE_STATIC_RUNTIME OFF) 


SET (Boost_FIND_REQUIRED TRUE)
SET (Boost_FIND_QUIETLY TRUE)
SET (Boost_DEBUG FALSE)
set (Boost_USE_MULTITHREADED TRUE)
set (Boost_USE_STATIC_LIBS TRUE)


FIND_PACKAGE(Boost COMPONENTS)



include_directories(${Boost_INCLUDE_DIR})



# имя основной утилиты будет таким же как и имя проекта?
add_library(mrpc_lib SHARED

  server.cpp
  tcp_connect.cpp
  iocp_block.cpp

  sync_console.cpp
  #../test/global_place.cpp
)



set_target_properties(mrpc_lib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)



target_include_directories(mrpc_lib PUBLIC
  ${PROJECT_SOURCE_DIR}/../
)



# пока не буду делать библиотеку, но по идее потом может и надо буует
#add_library(helloworld lib.cpp)
#add_executable(test_version test_version.cpp)

target_include_directories(mrpc_lib
    PRIVATE "${CMAKE_BINARY_DIR}"
)



target_link_libraries(mrpc_lib 
  ${Boost_LIBRARIES}
)





# для linux нужно ещё pthread:
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(mrpc_lib 
    pthread
  )
endif ()




if (MSVC)
    target_compile_options(mrpc_lib PRIVATE
        /W4
    )
else ()
    #target_compile_options(mrpc_lib PRIVATE
    #    -Wall -Wextra -pedantic -Werror
    #)
endif()



install(TARGETS mrpc_lib RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT example@example.com)

include(CPack)

