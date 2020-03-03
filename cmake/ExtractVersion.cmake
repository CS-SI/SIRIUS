#
# Copyright (C) 2018 CS - Systemes d'Information (CS-SI)
#
# This file is part of Sirius
#
#     https://github.com/CS-SI/SIRIUS
#
# Sirius is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Sirius is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Sirius.  If not, see <https://www.gnu.org/licenses/>.
#

find_package(Git QUIET)
if(GIT_FOUND AND "${SIRIUS_VERSION}" STREQUAL ""
             AND "${SIRIUS_REVISION_COMMIT}" STREQUAL "")
    # try to extract version and revision commit from Git using git describe
    include(GetGitRevisionDescription)
    get_git_head_revision(SIRIUS_GIT_REFSPEC SIRIUS_REVISION_COMMIT)
    git_describe(SIRIUS_GIT_DESCRIBE --tags)
    if (SIRIUS_GIT_DESCRIBE MATCHES ".*NOTFOUND")
        message(STATUS "No recent tag in the commit history. Specify the version using SIRIUS_VERSION variable.")
        set(SIRIUS_VERSION "")
    else()
        set(SIRIUS_VERSION "${SIRIUS_GIT_DESCRIBE}")
    endif()
    if (SIRIUS_REVISION_COMMIT MATCHES ".*NOTFOUND")
        message(STATUS "No revision commit in the commit history. Specify the version using SIRIUS_REVISION_COMMIT variable.")
        set(SIRIUS_REVISION_COMMIT "")
    endif()
endif()

if ("${SIRIUS_VERSION}" STREQUAL "")
    # extract version from file VERSION
    message(STATUS "No version found. Extract version from file 'VERSION'")
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" SIRIUS_VERSION)
endif()

set(SIRIUS_EXTENDED_VERSION "${SIRIUS_VERSION}")

set(VERSION_REGEX "^([0-9]+)[.]([0-9]+)[.]([0-9]+)")
string(REGEX MATCH ${VERSION_REGEX} SIRIUS_VERSION ${SIRIUS_EXTENDED_VERSION})

if ("${SIRIUS_REVISION_COMMIT}" STREQUAL "")
    message(STATUS "No revision commit found. Setting project revision commit to 'unknown-revision-commit'")
    set(SIRIUS_REVISION_COMMIT "unknown-revision-commit")
endif()