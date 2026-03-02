cmake_minimum_required(VERSION 3.20)
project(Mcaster1DNAS_GUI VERSION 2.5.3 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ── Qt6 ────────────────────────────────────────────────────────────────────────
find_package(Qt6 REQUIRED COMPONENTS
    Core Widgets Network Xml WebEngineWidgets Svg
)
qt_standard_project_setup()

# ── Build-time flag: excludes main() from src/main.c ──────────────────────────
add_compile_definitions(MCASTER_BUILD_GUI HAVE_CONFIG_H)

# ── Root & src include paths (config.h lives in the autoconf build root) ───────
set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(SRC_DIR      "${PROJECT_ROOT}/src")

# ── Isolate config.h to avoid VERSION file shadowing C++20 <version> header ───
# macOS has a case-insensitive filesystem: adding the project root to -I would
# cause the repo's "VERSION" file to satisfy #include <version> before libc++.
# Fix: copy just config.h into a dedicated build-dir subdirectory.
set(CONFIG_H_SRCDIR "${CMAKE_BINARY_DIR}/autoconf_include")
file(MAKE_DIRECTORY "${CONFIG_H_SRCDIR}")
configure_file("${PROJECT_ROOT}/config.h"   "${CONFIG_H_SRCDIR}/config.h"   COPYONLY)
configure_file("${PROJECT_ROOT}/git_hash.h" "${CONFIG_H_SRCDIR}/git_hash.h" COPYONLY)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}  # server_bridge.h and other macos/ headers
    ${CONFIG_H_SRCDIR}           # config.h only — no VERSION collision
    ${SRC_DIR}
    ${SRC_DIR}/avl
    ${SRC_DIR}/thread
    ${SRC_DIR}/httpp
    ${SRC_DIR}/net
    ${SRC_DIR}/log
    ${SRC_DIR}/timing
)

# ── System library discovery (works on macOS/Linux) ───────────────────────────
find_package(PkgConfig REQUIRED)

# On Apple Silicon, Homebrew installs keg-only packages under /opt/homebrew.
# pkg-config must find Homebrew's libxslt and libxml2 BEFORE the system copies
# (/usr/lib/libxslt.1.dylib is compiled against system libxml2 ~2.9).
# Using system libxslt + Homebrew libxml2 causes ABI mismatch → SIGSEGV in
# xmlFreeID because the xmlBuf / xmlID struct layouts changed in libxml2 2.12+.
if(APPLE)
    execute_process(
        COMMAND brew --prefix
        OUTPUT_VARIABLE HOMEBREW_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(ENV{PKG_CONFIG_PATH}
        "${HOMEBREW_PREFIX}/lib/pkgconfig:${HOMEBREW_PREFIX}/opt/libxslt/lib/pkgconfig:${HOMEBREW_PREFIX}/opt/libxml2/lib/pkgconfig:${HOMEBREW_PREFIX}/opt/openssl@3/lib/pkgconfig:${HOMEBREW_PREFIX}/opt/curl/lib/pkgconfig:${HOMEBREW_PREFIX}/opt/libyaml/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}"
    )
endif()

pkg_check_modules(LIBXML2   REQUIRED libxml-2.0)
pkg_check_modules(LIBXSLT   REQUIRED libxslt)
pkg_check_modules(OPENSSL   REQUIRED openssl)
pkg_check_modules(LIBCURL   REQUIRED libcurl)
pkg_check_modules(LIBYAML   REQUIRED yaml-0.1)
pkg_check_modules(LIBOGG    REQUIRED ogg)
pkg_check_modules(LIBVORBIS REQUIRED vorbis)
# speex — Homebrew doesn't ship a .pc file; probe then fall back to find_library
pkg_check_modules(LIBSPEEX speex)
if(NOT LIBSPEEX_FOUND)
    find_library(SPEEX_LIB NAMES speex
        HINTS /opt/homebrew/lib /usr/local/lib /usr/lib)
    find_path(SPEEX_INC speex/speex.h
        HINTS /opt/homebrew/include /usr/local/include /usr/include)
    if(NOT SPEEX_LIB)
        message(FATAL_ERROR "speex not found. Run: brew install speex")
    endif()
    set(LIBSPEEX_LIBRARIES    "${SPEEX_LIB}")
    set(LIBSPEEX_INCLUDE_DIRS "${SPEEX_INC}")
    message(STATUS "Found speex via find_library: ${SPEEX_LIB}")
endif()

# theora — may also lack a .pc on some platforms; same fallback pattern
pkg_check_modules(LIBTHEORA theora)
if(NOT LIBTHEORA_FOUND)
    find_library(THEORA_LIB NAMES theora
        HINTS /opt/homebrew/lib /usr/local/lib /usr/lib)
    find_path(THEORA_INC theora/theora.h
        HINTS /opt/homebrew/include /usr/local/include /usr/include)
    if(NOT THEORA_LIB)
        message(FATAL_ERROR "theora not found. Run: brew install theora")
    endif()
    set(LIBTHEORA_LIBRARIES    "${THEORA_LIB}")
    set(LIBTHEORA_INCLUDE_DIRS "${THEORA_INC}")
    message(STATUS "Found theora via find_library: ${THEORA_LIB}")
endif()

include_directories(
    ${LIBXML2_INCLUDE_DIRS}
    ${LIBXSLT_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIRS}
    ${LIBCURL_INCLUDE_DIRS}
    ${LIBYAML_INCLUDE_DIRS}
    ${LIBOGG_INCLUDE_DIRS}
    ${LIBVORBIS_INCLUDE_DIRS}
    ${LIBSPEEX_INCLUDE_DIRS}
    ${LIBTHEORA_INCLUDE_DIRS}
)

# ── Server core C sources (all of src/ + sub-libraries) ───────────────────────
# Excludes test.c files from sub-libraries (not needed in production build)
set(SERVER_SOURCES
    # ── sub-libraries ──────────────────────────────────────────────────────────
    ${SRC_DIR}/avl/avl.c
    ${SRC_DIR}/thread/thread.c
    ${SRC_DIR}/httpp/httpp.c
    ${SRC_DIR}/net/resolver.c
    ${SRC_DIR}/net/sock.c
    ${SRC_DIR}/log/log.c
    ${SRC_DIR}/timing/timing.c

    # ── server core ────────────────────────────────────────────────────────────
    # main.c is included: its main() is guarded by #ifndef MCASTER_BUILD_GUI
    ${SRC_DIR}/main.c
    ${SRC_DIR}/admin.c
    ${SRC_DIR}/auth.c
    ${SRC_DIR}/auth_cmd.c
    ${SRC_DIR}/auth_htpasswd.c
    ${SRC_DIR}/auth_url.c
    ${SRC_DIR}/cfgfile.c
    ${SRC_DIR}/cfgfile_yaml.c
    ${SRC_DIR}/client.c
    ${SRC_DIR}/connection.c
    ${SRC_DIR}/event.c
    ${SRC_DIR}/flv.c
    ${SRC_DIR}/fnmatch.c
    # fnmatch_loop.c is NOT compiled directly — it is #include-d by fnmatch.c
    # as a template (CHAR/INT macros are defined by fnmatch.c for each inclusion).
    ${SRC_DIR}/format.c
    ${SRC_DIR}/format_ebml.c
    ${SRC_DIR}/format_flac.c
    ${SRC_DIR}/format_kate.c
    ${SRC_DIR}/format_midi.c
    ${SRC_DIR}/format_mp3.c
    ${SRC_DIR}/format_ogg.c
    ${SRC_DIR}/format_opus.c
    ${SRC_DIR}/format_skeleton.c
    ${SRC_DIR}/format_speex.c
    ${SRC_DIR}/format_theora.c
    ${SRC_DIR}/format_vorbis.c
    ${SRC_DIR}/fserve.c
    ${SRC_DIR}/global.c
    ${SRC_DIR}/icy2_meta.c
    ${SRC_DIR}/logging.c
    ${SRC_DIR}/md5.c
    ${SRC_DIR}/mpeg.c
    ${SRC_DIR}/params.c
    ${SRC_DIR}/refbuf.c
    ${SRC_DIR}/sighandler.c
    ${SRC_DIR}/slave.c
    ${SRC_DIR}/songdata_api.c
    ${SRC_DIR}/source.c
    ${SRC_DIR}/ssl_gen.c
    ${SRC_DIR}/stats.c
    ${SRC_DIR}/util.c
    ${SRC_DIR}/xslt.c
    ${SRC_DIR}/yp.c
)

# ── Qt resources (icons + images compiled into the binary) ────────────────────
qt_add_resources(QRC_SOURCES "resources/mcaster1.qrc")

# ── GUI C++ sources ────────────────────────────────────────────────────────────
set(GUI_SOURCES
    main.cpp
    MainWindow.cpp
    MainWindow.h
    ServerThread.cpp
    ServerThread.h
    StatsPoller.cpp
    StatsPoller.h
    LogTab.cpp
    LogTab.h
    StatsTab.cpp
    StatsTab.h
    ConfigTab.cpp
    ConfigTab.h
    TrayIcon.cpp
    TrayIcon.h
    ConsoleTab.cpp
    ConsoleTab.h
    ConfigDialog.cpp
    ConfigDialog.h
    HtmlTab.h
    server_bridge.h
    ${QRC_SOURCES}
)

# ── App bundle / executable ────────────────────────────────────────────────────
if(APPLE)
    set(APP_ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/mcaster1.icns")
    if(EXISTS ${APP_ICON_FILE})
        set_source_files_properties(${APP_ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
        list(APPEND GUI_SOURCES ${APP_ICON_FILE})
    endif()

    qt_add_executable(Mcaster1DNAS
        MACOSX_BUNDLE
        ${GUI_SOURCES}
        ${SERVER_SOURCES}
    )
    set_target_properties(Mcaster1DNAS PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER  "com.mcaster1.Mcaster1DNAS"
        MACOSX_BUNDLE_BUNDLE_NAME     "Mcaster1DNAS"
        MACOSX_BUNDLE_BUNDLE_VERSION  "2.5.3"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "2.5.3"
        MACOSX_BUNDLE_INFO_PLIST      "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
    )
else()
    # Linux: standard executable
    qt_add_executable(Mcaster1DNAS
        ${GUI_SOURCES}
        ${SERVER_SOURCES}
    )
endif()

# ── Post-build: copy docs/ into the app bundle (macOS) or alongside binary ─────
# On macOS, applicationDirPath() returns Contents/MacOS/ — copy docs there.
# On Linux, copy docs/ next to the binary in the build output directory.
if(APPLE)
    # docs go into Contents/Resources/ — Contents/MacOS/ is for executables only,
    # and codesign rejects non-code files placed there.
    add_custom_command(TARGET Mcaster1DNAS POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_ROOT}/docs"
            "$<TARGET_BUNDLE_DIR:Mcaster1DNAS>/Contents/Resources/docs"
        COMMENT "Copying docs/ into app bundle Resources"
    )
else()
    add_custom_command(TARGET Mcaster1DNAS POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_ROOT}/docs"
            "$<TARGET_FILE_DIR:Mcaster1DNAS>/docs"
        COMMENT "Copying docs/ alongside binary"
    )
endif()

# ── Library search paths (pkg-config LIBRARY_DIRS → linker -L flags) ──────────
# pkg_check_modules sets LIBRARIES to bare names (ssl, xml2, …) without -L paths.
# target_link_directories adds those paths so the linker can find the dylibs.
target_link_directories(Mcaster1DNAS PRIVATE
    ${LIBXML2_LIBRARY_DIRS}
    ${LIBXSLT_LIBRARY_DIRS}
    ${OPENSSL_LIBRARY_DIRS}
    ${LIBCURL_LIBRARY_DIRS}
    ${LIBYAML_LIBRARY_DIRS}
    ${LIBOGG_LIBRARY_DIRS}
    ${LIBVORBIS_LIBRARY_DIRS}
    ${LIBSPEEX_LIBRARY_DIRS}
    ${LIBTHEORA_LIBRARY_DIRS}
)

# ── Link Qt6 + all server libraries ───────────────────────────────────────────
target_link_libraries(Mcaster1DNAS PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Xml
    Qt6::WebEngineWidgets
    Qt6::Svg
    ${LIBXML2_LIBRARIES}
    ${LIBXSLT_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${LIBCURL_LIBRARIES}
    ${LIBYAML_LIBRARIES}
    ${LIBOGG_LIBRARIES}
    ${LIBVORBIS_LIBRARIES}
    ${LIBSPEEX_LIBRARIES}
    ${LIBTHEORA_LIBRARIES}
    pthread
)

# ── macOS-specific: link against system frameworks ─────────────────────────────
if(APPLE)
    target_link_libraries(Mcaster1DNAS PRIVATE
        "-framework CoreFoundation"
        "-framework AppKit"
    )
endif()

# ── Pass pkg-config CFLAGS (include dirs + compile flags) into the build ───────
target_compile_options(Mcaster1DNAS PRIVATE
    ${LIBXML2_CFLAGS_OTHER}
    ${LIBXSLT_CFLAGS_OTHER}
    ${OPENSSL_CFLAGS_OTHER}
    ${LIBCURL_CFLAGS_OTHER}
    ${LIBYAML_CFLAGS_OTHER}
)


# ── Install (for Linux packaging) ─────────────────────────────────────────────
if(NOT APPLE)
    install(TARGETS Mcaster1DNAS DESTINATION bin)
endif()
