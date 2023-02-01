find_package(PkgConfig)

PKG_CHECK_MODULES(PC_GR_CARBON gnuradio-carbon)

FIND_PATH(
    GR_CARBON_INCLUDE_DIRS
    NAMES gnuradio/carbon/api.h
    HINTS $ENV{CARBON_DIR}/include
        ${PC_CARBON_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    GR_CARBON_LIBRARIES
    NAMES gnuradio-carbon
    HINTS $ENV{CARBON_DIR}/lib
        ${PC_CARBON_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          )

include("${CMAKE_CURRENT_LIST_DIR}/gnuradio-carbonTarget.cmake")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GR_CARBON DEFAULT_MSG GR_CARBON_LIBRARIES GR_CARBON_INCLUDE_DIRS)
MARK_AS_ADVANCED(GR_CARBON_LIBRARIES GR_CARBON_INCLUDE_DIRS)
