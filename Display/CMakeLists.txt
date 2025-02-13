cmake_minimum_required(VERSION 3.16)

project(RoomBookerDisplay VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
)

qt_add_executable(RoomBookerDisplay
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    widgets/bookingwidget.h widgets/bookingwidget.cpp
    settingstrings.h
    bookingdisplay.h bookingdisplay.cpp
    widgets/bookinginfo.h widgets/bookinginfo.cpp
    widgets/bookingname.h widgets/bookingname.cpp
    widgets/bookingstatus.h widgets/bookingstatus.cpp
    resources.qrc
    datafetchinghandler.h datafetchinghandler.cpp
    datatypes.h
    widgets/upcomingbookings.h widgets/upcomingbookings.cpp
    settings/settingspopup.h settings/settingspopup.cpp
    settings/textsettingedit.h settings/textsettingedit.cpp
    widgets/clickableicon.h widgets/clickableicon.cpp
    settings/colorsettingedit.h settings/colorsettingedit.cpp
    settings/fontsettingedit.h settings/fontsettingedit.cpp
    settings/iconsettingedit.h settings/iconsettingedit.cpp
    settings/integersettingedit.h settings/integersettingedit.cpp
)


target_link_libraries(RoomBookerDisplay PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(RoomBookerDisplay PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

find_package(Qt6 REQUIRED COMPONENTS Network)
target_link_libraries(RoomBookerDisplay PRIVATE Qt6::Network)

include(GNUInstallDirs)
install(TARGETS RoomBookerDisplay
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(RoomBookerDisplay)
