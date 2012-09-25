
cmake_minimum_required (VERSION 2.8.4)

project (nmalyzr)

set(BoostDir /opt/boost_1_48_0)

link_directories(${BoostDir}/stage/lib)

include_directories(${BoostDir}/boost) 
    
file(GLOB nmalyzr_src src/*.cpp)

add_executable(nmalyzr ${nmalyzr_src})

target_link_libraries(nmalyzr -lboost_program_options -lboost_iostreams -lboost_regex)

install (TARGETS nmalyzr DESTINATION /usr/bin)
install (FILES "man/nmalyzr.1.gz" DESTINATION /usr/share/man/man1)