# This is a build file for Huggle (used with cmake)
cmake_minimum_required (VERSION 2.8.7)

if (NOT HUGGLE_CMAKE)
    message(FATAL_ERROR "This cmake file can't be used on its own, it must be included from parent folder")
endif()

if (LINUX_SNAP)
    INSTALL(FILES devtools.js DESTINATION bin/extensions)
else()
    INSTALL(FILES devtools.js DESTINATION share/huggle/extensions)
endif()
