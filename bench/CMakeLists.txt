cmake_minimum_required(VERSION 3.2)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})


# имя проекта
project(mrpc_boost VERSION ${PROJECT_VESRION})


SET (Boost_FIND_REQUIRED TRUE)
SET (Boost_FIND_QUIETLY TRUE)
SET (Boost_DEBUG FALSE)
set (Boost_USE_MULTITHREADED TRUE)
set (Boost_USE_STATIC_LIBS TRUE)


FIND_PACKAGE(Boost COMPONENTS)

find_package(benchmark REQUIRED)


include_directories(${Boost_INCLUDE_DIR})



# имя основной утилиты будет таким же как и имя проекта?
add_executable(mrpc_bench

  main.cpp
  ../test/appl.cpp
  ../test/global_place.cpp
  ../test/tst_custom_drv.cpp
)



set_target_properties(mrpc_bench PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)



target_include_directories(mrpc_bench PUBLIC
${PROJECT_SOURCE_DIR}/./
)



# пока не буду делать библиотеку, но по идее потом может и надо буует
#add_library(helloworld lib.cpp)
#add_executable(test_version test_version.cpp)

target_include_directories(mrpc_bench
    PRIVATE "${CMAKE_BINARY_DIR}"
)



target_link_libraries(mrpc_bench 

  ${Boost_LIBRARIES}
  benchmark::benchmark

  mrpc_lib
)



# для linux нужно ещё pthread:
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(mrpc_bench 
    pthread
  )
endif ()




if (MSVC)
    target_compile_options(mrpc_bench PRIVATE
        /W4
    )
else ()
    #target_compile_options(mrpc_bench PRIVATE
    #    -Wall -Wextra -pedantic -Werror
    #)
endif()


# регистрируем тест:
# пока закоментировал - тест подвисает
# add_test(mrpc_boost mrpc_bench)

