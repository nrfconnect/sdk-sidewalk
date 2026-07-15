## 
## @file      CMakeLists.txt
## 
## @brief     Project CMake
## 
## Revised BSD License
## Copyright Semtech Corporation 2020. All rights reserved.
## 
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##     * Redistributions of source code must retain the above copyright
##       notice, this list of conditions and the following disclaimer.
##     * Redistributions in binary form must reproduce the above copyright
##       notice, this list of conditions and the following disclaimer in the
##       documentation and/or other materials provided with the distribution.
##     * Neither the name of the Semtech corporation nor the
##       names of its contributors may be used to endorse or promote products
##       derived from this software without specific prior written permission.
## 
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE LIABLE FOR ANY
## DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
## (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
## LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
## ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
## (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
## SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
## 
project(smtc_ralf VERSION 0.9.0 DESCRIPTION "Radio Abstraction Layer")
cmake_minimum_required(VERSION 3.6)

# Allow selection of radios
set(RADIO_LIST lr1110 sx126x sx1272 sx1276 sx128x)
set(RADIO sx1272 CACHE STRING "Default radio is sx1272")
set_property(CACHE RADIO PROPERTY STRINGS ${RADIO_LIST})
set_property(CACHE RADIO PROPERTY ADVANCED)

set(RADIO_DRIVER_LIST  lr1110 sx126x sx127x sx128x)
set(RADIO_DRIVER sx127x CACHE STRING "Default radio is sx127x")
set_property(CACHE RADIO_DRIVER PROPERTY STRINGS ${RADIO_DRIVER_LIST})
set_property(CACHE RADIO_DRIVER PROPERTY ADVANCED)

# Select radio driver to be used
if(${RADIO} STREQUAL sx1272 OR ${RADIO} STREQUAL sx1276)
    set(RADIO_DRIVER sx127x CACHE INTERNAL "Radio sx127x selected")
else()
    set(RADIO_DRIVER ${RADIO} CACHE INTERNAL "Radio ${RADIO} selected")
endif()

#---------------------------------------------------------------------------------------
# Target
#---------------------------------------------------------------------------------------

foreach( R ${RADIO_LIST} )
    if(${R} MATCHES ${RADIO})
        list(APPEND ${PROJECT_NAME}_SOURCES
            ${CMAKE_CURRENT_SOURCE_DIR}/ralf_${RADIO_DRIVER}.c
        )
    endif()
endforeach()

add_library(${PROJECT_NAME} OBJECT EXCLUDE_FROM_ALL ${${PROJECT_NAME}_SOURCES})

# Loops through all regions and add compile time definitions for the enabled ones.
foreach( R ${RADIO_LIST} )
    if(${R} MATCHES ${RADIO})
        string(TOUPPER ${R} RADIO_UPPERCASE)
        target_compile_definitions(${PROJECT_NAME} PUBLIC -D${RADIO_UPPERCASE})
    endif()
endforeach()

# For debug builds set the symbol DEBUG
set(CMAKE_C_FLAGS_DEBUG -DDEBUG)

foreach( R ${RADIO_LIST} )
    if(${R} MATCHES ${RADIO})
        list(APPEND ${PROJECT_NAME}_INCLUDES
            $<BUILD_INTERFACE:$<TARGET_PROPERTY:${RADIO_DRIVER}_driver,INTERFACE_INCLUDE_DIRECTORIES>>
        )
    endif()
endforeach()

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${${PROJECT_NAME}_INCLUDES}
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:smtc_ral,INTERFACE_INCLUDE_DIRECTORIES>>
)

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 11)
