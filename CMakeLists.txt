cmake_minimum_required(VERSION 3.2)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})


# имя проекта
project(mrpc_boost VERSION ${PROJECT_VESRION})






# имя основной утилиты будет таким же как и имя проекта?
add_executable(mrpc_boost

  lib/server.cpp
  lib/tcp_connect.cpp

  appl/appl.cpp
  appl/global_place.cpp
  appl/sync_console.cpp
  appl/tst_custom_drv.cpp
)



set_target_properties(mrpc_boost PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)



target_include_directories(mrpc_boost PUBLIC
${PROJECT_SOURCE_DIR}/./
)



# пока не буду делать библиотеку, но по идее потом может и надо буует
#add_library(helloworld lib.cpp)
#add_executable(test_version test_version.cpp)

target_include_directories(mrpc_boost
    PRIVATE "${CMAKE_BINARY_DIR}"
)





# для linux нужно ещё pthread:
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(mrpc_boost 
    pthread
  )
endif ()




if (MSVC)
    target_compile_options(mrpc_boost PRIVATE
        /W4
    )
else ()
    #target_compile_options(mrpc_boost PRIVATE
    #    -Wall -Wextra -pedantic -Werror
    #)
endif()



install(TARGETS mrpc_boost RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT example@example.com)

include(CPack)

