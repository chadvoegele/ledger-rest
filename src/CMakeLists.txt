include(../cmake/find_boost.cmake)
include(../cmake/find_microhttpd.cmake)

find_library(LEDGER_LIB NAMES "libledger.so" PATHS "/usr/lib")
find_path(LEDGER_INCLUDE NAMES "ledger/")

find_path(UTF8CPP_INCLUDE NAMES "utf8/" PATHS "/usr/include/utf8cpp")

find_library(GNUTLS_LIB NAMES "libgnutls.so" PATHS "/usr/lib")
find_path(GNUTLS_INCLUDE NAMES "gnutls/gnutls.h")

include_directories(${SRC_DIR} ${Boost_INCLUDE_DIRS} ${LEDGER_INCLUDE}
  ${UTF8CPP_INCLUDE} ${GNUTLS_INCLUDE})

add_library(${PROJECT_NAME} STATIC
  uri_parser.cpp mhd.cpp ledger_rest.cpp http.cpp logger.cpp
  runnable.cpp runner.cpp stderr_logger.cpp ledger_rest_runnable.cpp
  file_reader.cpp args.cpp ledger_rest_args.cpp mhd_args.cpp
  signal_handler.cpp json_parser.cpp)
set_cpp14(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} ${LEDGER_LIB} ${GNUTLS_LIB} ${Boost_LIBRARIES})

set(EXE_TARGET "${PROJECT_NAME}-bin")
add_executable(${EXE_TARGET} main.cpp)
set_cpp14(${EXE_TARGET})
set_property(TARGET ${EXE_TARGET} PROPERTY OUTPUT_NAME ${PROJECT_NAME})
target_link_libraries(${EXE_TARGET} ${PROJECT_NAME} ${MICROHTTPD_LIB} ${Boost_LIBRARIES})

install(TARGETS ${PROJECT_NAME} ${EXE_TARGET}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
install(FILES ledger_rest.h http.h logger.h uri_parser.h mhd.h
          runnable.h runner.h stderr_logger.h ledger_rest_runnable.h
          file_reader.h args.h ledger_rest_args.h mhd_args.h
          signal_handler.h json_parser.h
        DESTINATION include/${PROJECT_NAME})
