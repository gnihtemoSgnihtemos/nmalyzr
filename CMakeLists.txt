
cmake_minimum_required (VERSION 2.8.4)

project (nmalyzr)

# Boost references
#------------------------------------------------------------------------------------------------
set(BOOST_COMPONENTS
    boost_program_options 
    boost_iostreams 
    boost_regex
)

set(BOOST_DIR "$ENV{BOOST_DIR}")
if(NOT "${BOOST_DIR}" STREQUAL "")
    set(BOOST_INCLUDE "$ENV{BOOST_INCLUDE}" )
    if("${BOOST_INCLUDE}" STREQUAL "")
        set(BOOST_INCLUDE "${BOOST_DIR}/boost")
    endif()
    set(BOOST_LIB_PATH "$ENV{BOOST_LIB_PATH}")
    if("${BOOST_LIB_PATH}" STREQUAL "")
        set(BOOST_LIB_PATH "${BOOST_DIR}/stage/lib")
    endif()
else()
    find_package(boost COMPONENTS ${BOOST_COMPONENTS})
    if(Boost_FOUND)      
        set(BOOST_INCLUDE ${Boost_INCLUDE_DIRS})
        set(BOOST_COMPONENTS ${Boost_LIBRARIES})
    else()
        message(FATAL_ERROR "Cannot find necessary boost libraries: ${BOOST_COMPONENTS}.")
    endif()
endif()

# POCO references
#------------------------------------------------------------------------------------------------
set(POCO_COMPONENTS
    PocoFoundation 
    PocoXML 
)

set(POCO_INCLUDE "$ENV{POCO_INCLUDE}")
if("${POCO_INCLUDE}" STREQUAL "")
    set(POCO_INCLUDE "/usr/local/include")
endif()

set(POCO_LIB_PATH "$ENV{POCO_LIB_PATH}")
if("${POCO_LIB_PATH}" STREQUAL "")
    set(POCO_LIB_PATH "/usr/local/lib")
endif()

include_directories(${BOOST_INCLUDE})
include_directories(${POCO_INCLUDE})
link_directories(
    ${BOOST_LIB_PATH}
    ${POCO_LIB_PATH}
)
    
file(GLOB nmalyzr_src src/*.cpp)

add_executable(nmalyzr ${nmalyzr_src})
target_link_libraries(nmalyzr ${BOOST_COMPONENTS} ${POCO_COMPONENTS})

install (TARGETS nmalyzr DESTINATION /usr/bin)
install (FILES "man/nmalyzr.1.gz" DESTINATION /usr/share/man/man1)