# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (C) 2026 Lúcio de Araújo
#
# Every C/C++ source of the Aquorbium starts with its licence (2026-10-22):
# run as `cmake -DROOT=<project> -P tests/LicenseHeaders.cmake` (a ctest).
file(GLOB_RECURSE sources RELATIVE "${ROOT}"
    "${ROOT}/src/*.h" "${ROOT}/src/*.hpp" "${ROOT}/src/*.cpp"
    "${ROOT}/tests/*.h" "${ROOT}/tests/*.hpp" "${ROOT}/tests/*.cpp"
    "${ROOT}/tools/*.h" "${ROOT}/tools/*.hpp" "${ROOT}/tools/*.cpp")
set(missing "")
foreach(source IN LISTS sources)
    file(STRINGS "${ROOT}/${source}" first LIMIT_COUNT 1)
    if(NOT first STREQUAL "// SPDX-License-Identifier: AGPL-3.0-or-later")
        list(APPEND missing "${source}")
    endif()
endforeach()
list(LENGTH sources total)
if(missing)
    list(JOIN missing "\n  " listed)
    message(FATAL_ERROR "without the AGPL-3.0-or-later header:\n  ${listed}")
endif()
message(STATUS "license headers: all ${total} sources")
