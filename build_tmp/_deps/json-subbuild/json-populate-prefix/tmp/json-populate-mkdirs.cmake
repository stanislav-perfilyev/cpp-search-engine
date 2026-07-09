# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/mnt/d/claude/Projects/Устройство на работу/GitHub-repos/cpp-search-engine/build_tmp/_deps/json-src"
  "/mnt/d/claude/Projects/Устройство на работу/GitHub-repos/cpp-search-engine/build_tmp/_deps/json-build"
  "/mnt/d/claude/Projects/Устройство на работу/GitHub-repos/cpp-search-engine/build_tmp/_deps/json-subbuild/json-populate-prefix"
  "/mnt/d/claude/Projects/Устройство на работу/GitHub-repos/cpp-search-engine/build_tmp/_deps/json-subbuild/json-populate-prefix/tmp"
  "/mnt/d/claude/Projects/Устройство на работу/GitHub-repos/cpp-search-engine/build_tmp/_deps/json-subbuild/json-populate-prefix/src/json-populate-stamp"
  "/mnt/d/claude/Projects/Устройство на работу/GitHub-repos/cpp-search-engine/build_tmp/_deps/json-subbuild/json-populate-prefix/src"
  "/mnt/d/claude/Projects/Устройство на работу/GitHub-repos/cpp-search-engine/build_tmp/_deps/json-subbuild/json-populate-prefix/src/json-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/mnt/d/claude/Projects/Устройство на работу/GitHub-repos/cpp-search-engine/build_tmp/_deps/json-subbuild/json-populate-prefix/src/json-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/mnt/d/claude/Projects/Устройство на работу/GitHub-repos/cpp-search-engine/build_tmp/_deps/json-subbuild/json-populate-prefix/src/json-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
