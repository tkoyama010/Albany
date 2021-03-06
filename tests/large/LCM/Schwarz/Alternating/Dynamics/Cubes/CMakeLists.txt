##*****************************************************************//
##    Albany 3.0:  Copyright 2016 Sandia Corporation               //
##    This Software is released under the BSD license detailed     //
##    in the file "license.txt" in the top-level Albany directory  //
##*****************************************************************//

# Copy Input file from source to binary dir
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cuboid_00.g
               ${CMAKE_CURRENT_BINARY_DIR}/cuboid_00.g COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cuboid_00.g.2.0
               ${CMAKE_CURRENT_BINARY_DIR}/cuboid_00.g.2.0 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cuboid_00.g.2.1
               ${CMAKE_CURRENT_BINARY_DIR}/cuboid_00.g.2.1 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cuboid_01.g
               ${CMAKE_CURRENT_BINARY_DIR}/cuboid_01.g COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cuboid_01.g.2.0
               ${CMAKE_CURRENT_BINARY_DIR}/cuboid_01.g.2.0 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cuboid_01.g.2.1
               ${CMAKE_CURRENT_BINARY_DIR}/cuboid_01.g.2.1 COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cuboid_00.yaml
               ${CMAKE_CURRENT_BINARY_DIR}/cuboid_00.yaml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cuboid_01.yaml
               ${CMAKE_CURRENT_BINARY_DIR}/cuboid_01.yaml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cuboids.yaml
               ${CMAKE_CURRENT_BINARY_DIR}/cuboids.yaml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/materials_00.yaml
               ${CMAKE_CURRENT_BINARY_DIR}/materials_00.yaml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/materials_01.yaml
               ${CMAKE_CURRENT_BINARY_DIR}/materials_01.yaml COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/runtest.py
               ${CMAKE_CURRENT_BINARY_DIR}/runtest.py COPYONLY)
IF(ALBANY_DTK)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/runtest_parallel.py
               ${CMAKE_CURRENT_BINARY_DIR}/runtest_parallel.py COPYONLY)
ENDIF()

execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
  ${AlbanyTPath} ${CMAKE_CURRENT_BINARY_DIR}/AlbanyT)
get_filename_component(testName ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_test(NAME Schwarz_Alternating_Dynamics_${testName} COMMAND "python" "runtest.py")
IF(ALBANY_DTK)
add_test(NAME Schwarz_Alternating_Dynamics_${testName}_Parallel COMMAND "python" "runtest_parallel.py")
ENDIF()
