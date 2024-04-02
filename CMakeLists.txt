# CMakeList.txt: проект CMake для ParserCombinations; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.12)

# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("ParserCombinators")

option(BUILD_TESTING "Build unit tests" ON)

# Добавьте источник в исполняемый файл этого проекта.
add_executable (${PROJECT_NAME} main.cpp ParserCombinators.cpp ParserCombinators.h)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
if(BUILD_TESTING)
  #enable_testing()

  # add dependencies
  include(cmake/CPM.cmake)
  #CPMAddPackage("gh:doctest/doctest@2.4.11")
  CPMAddPackage(NAME  doctest
    VERSION         2.4.11
    GIT_REPOSITORY  https://github.com/doctest/doctest.git
    GIT_SHALLOW     TRUE
    DONWLOAD_ONLY   TRUE
)
    
  add_executable(${PROJECT_NAME}_test test/test.cpp ParserCombinators.cpp ParserCombinators.h)
  set_property(TARGET ${PROJECT_NAME}_test PROPERTY CXX_STANDARD 23)
  add_test(${PROJECT_NAME}_test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}_test)

  if(MSVC)
    target_compile_options(${PROJECT_NAME}_test PRIVATE /W4 /permissive-)
  else()
    target_compile_options(${PROJECT_NAME}_test PRIVATE -Wall -Wextra -Werror)
  endif()

  target_link_libraries(${PROJECT_NAME}_test doctest::doctest)

endif()
