cmake_minimum_required(VERSION 3.5)

project(torrent-client LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# Добавляем все исходные файлы CPR напрямую в проект
file(GLOB CPR_HEADERS /Users/ilushka/mycpr/cpr/include/*.h)
file(GLOB CPR_SOURCES /Users/ilushka/mycpr/cpr/cpr/*.cpp)

# Добавляем исполняемый файл torrent-client
add_executable(torrent-client
    main.cpp
    sha1.h sha1.cpp
    bencode.h bencode.cpp
    torrentfileparser.h torrentfileparser.cpp
    piece.h piece.cpp
    utils.h utils.cpp
    torrentfile.h torrentfile.cpp

    tester.h tester.cpp
    connect.h connect.cpp
    torrentclient.h torrentclient.cpp
    peerretriever.h peerretriever.cpp
    peerconnection.h peerconnection.cpp
    piecemanager.h piecemanager.cpp

    SharedQueue.h
    logo_rb.PNG
logo_rb.ico
    bittorrentmessage.h bittorrentmessage.cpp
    ${CPR_HEADERS}  # Включаем заголовочные файлы CPR
    ${CPR_SOURCES}
    torrentclientui.h torrentclientui.cpp
    # Включаем исходные файлы CPR
)

target_link_libraries(torrent-client PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    curl
)

target_include_directories(torrent-client PRIVATE
    /Users/ilushka/mycpr/cpr/include
    /Users/ilushka/mycpr/cpr/cpr
)

include(GNUInstallDirs)
install(TARGETS torrent-client
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
