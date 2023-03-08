
cpmaddpackage(NAME nonius
  GITHUB_REPOSITORY libnonius/nonius
  GIT_TAG devel
  DOWNLOAD_ONLY TRUE)
if(nonius_ADDED)
  add_library(nonius INTERFACE)
  target_include_directories(nonius INTERFACE ${nonius_SOURCE_DIR}/include)
endif()

add_executable(bench parser_variants.cpp)
target_link_libraries(bench PRIVATE nonius async_json::async_json)
target_compile_features(bench PRIVATE cxx_std_20)

find_program(MOLD_FOUND mold) 
if(MOLD_FOUND)
  target_link_options(bench PRIVATE -fuse-ld=mold -Wl,--icf=all -Wl,--print-icf-sections)
endif()

