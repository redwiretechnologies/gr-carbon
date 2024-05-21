# Copyright 2020 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Check if there is C++ code at all
########################################################################
if(NOT carbon_sources)
    MESSAGE(STATUS "No C++ sources... skipping python bindings")
    return()
endif(NOT carbon_sources)

########################################################################
# Check for pygccxml
########################################################################
GR_PYTHON_CHECK_MODULE_RAW(
    "pygccxml"
    "import pygccxml"
    PYGCCXML_FOUND
    )

include(GrPybind)

########################################################################
# Python Bindings
########################################################################

list(APPEND carbon_python_files
    fmcomms5_source_python.cc
    python_bindings.cc)

GR_PYBIND_MAKE_OOT(carbon
   ../../..
   gr::carbon
   "${carbon_python_files}")

# copy bindings extension for use in QA test module
add_custom_command(TARGET carbon_python POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:carbon_python>
    ${PROJECT_BINARY_DIR}/test_modules/gnuradio/carbon/
)

install(TARGETS carbon_python DESTINATION ${GR_PYTHON_DIR}/gnuradio/carbon COMPONENT pythonapi)
