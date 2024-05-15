cmake_minimum_required(VERSION 3.5)

project(torrent-client LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

file(GLOB CPR_HEADERS /Users/ilushka/TORRENT/torrent-client/cpr/include*.h)
file(GLOB CPR_SOURCES /Users/ilushka/TORRENT/torrent-client/cpr/cpr/*.cpp)

# Добавляем исполняемый файл torrent-client
add_executable(torrent-client
    main.cpp
    sha1.h sha1.cpp
    bencode.h bencode.cpp
    torrentfile.h torrentfile.cpp
    piece.h piece.cpp
    utils.h utils.cpp
    tester.h tester.cpp
    connect.h connect.cpp
    bittorrentmessage.h bittorrentmessage.cpp
    torrentclient.h torrentclient.cpp
    peerretriever.h peerretriever.cpp
    peerconnection.h peerconnection.cpp
    piecemanager.h piecemanager.cpp
    SharedQueue.h
    SharedQueue.h
    torrentclientui.h torrentclientui.cpp

    logo_rb.png
    logo_rb.ico
    logo_rb.icns
    log.h log.cpp
    ${CPR_HEADERS}
    ${CPR_SOURCES}
)

target_link_libraries(torrent-client PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    curl
)

target_include_directories(torrent-client PRIVATE
    /Users/ilushka/TORRENT/torrent-client/cpr/include
/Users/ilushka/TORRENT/torrent-client/cpr/cpr)

include(GNUInstallDirs)
install(TARGETS torrent-client
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

