find_package(GTest REQUIRED)

add_executable(ClamAV-GUI-Test
    ${CMAKE_SOURCE_DIR}/src/setupfilehandler.cpp
    ${CMAKE_SOURCE_DIR}/src/setupfilehandler.h
    setupfilehandlertest.cpp
)

target_include_directories( ClamAV-GUI-Test
    PRIVATE
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(ClamAV-GUI-Test
    PRIVATE
    GTest::gtest GTest::gtest_main
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
    Qt${QT_MAJOR_VERSION}::Core
)

enable_testing(true)

add_test(NAME ClamAV-GUI-Test COMMAND ClamAV-GUI-Test)