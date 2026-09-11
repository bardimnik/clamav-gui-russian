find_package(Qt${QT_MAJOR_VERSION} REQUIRED COMPONENTS Test)

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

target_link_libraries( ClamAV-GUI-Test
    PRIVATE
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_VERSION}::Gui
    Qt${QT_VERSION}::Widgets
    Qt${QT_MAJOR_VERSION}::Test    
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)

enable_testing(true)

add_test(NAME ClamAV-GUI-Test COMMAND ClamAV-GUI-Test)