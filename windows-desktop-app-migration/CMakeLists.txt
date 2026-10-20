cmake_minimum_required(VERSION 3.20)
project(EnterpriseOperationsWorkbench LANGUAGES CXX)

add_library(workbench_service
    src/workbench_service.cpp
)
target_include_directories(workbench_service PUBLIC include)
target_compile_features(workbench_service PUBLIC cxx_std_17)

if(WIN32)
    add_executable(enterprise_operations_workbench WIN32
        src/main.cpp
    )
    target_compile_features(enterprise_operations_workbench PRIVATE cxx_std_17)
    target_compile_definitions(enterprise_operations_workbench PRIVATE
        UNICODE
        _UNICODE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        WINVER=0x0601
        _WIN32_WINNT=0x0601
    )
    target_link_libraries(enterprise_operations_workbench PRIVATE
        workbench_service
        advapi32
        comctl32
        ole32
        shell32
        uuid
    )
else()
    message(STATUS "Building the portable demo service and tests; the Win32 UI requires Windows.")
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(workbench_service_tests
        tests/workbench_service_tests.cpp
    )
    target_link_libraries(workbench_service_tests PRIVATE workbench_service)
    add_test(NAME workbench_service_tests COMMAND workbench_service_tests)
endif()
