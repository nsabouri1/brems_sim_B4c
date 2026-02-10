#----------------------------------------------------------------------------
# Setup the project
#
cmake_minimum_required(VERSION 3.16...3.27)
project(B4c)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# See the documentation for a guide on how to enable/disable specific components
#
find_package(Geant4 REQUIRED ui_all vis_all)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Add the executable, use our local headers, and link it to the Geant4 libraries
#
add_executable(brems_sim_b4c main.cc ${sources} ${headers})
target_include_directories(brems_sim_b4c PRIVATE include)
target_link_libraries(brems_sim_b4c PRIVATE ${Geant4_LIBRARIES})
# Copy macro files to the build directory when they are currently in macros subfolder
# This is useful for running the simulation directly from the build directory
# without needing to specify the path to the macros.
# The macros are used to run the simulation and visualize the results.
# The macros are copied to the build directory so that they can be run directly 
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/macros DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build B4c. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(EXAMPLEB4C_SCRIPTS
  exampleB4c.out
  exampleB4.in
  gui.mac
  init_vis.mac
  plotHisto.C
  plotNtuple.C
  run1.mac
  run2.mac
  vis.mac
  )

#foreach(_script ${EXAMPLEB4C_SCRIPTS})
  # configure_file(
   # ${PROJECT_SOURCE_DIR}/${_script}
   # ${PROJECT_BINARY_DIR}/${_script}
   # COPYONLY
   # )
#endforeach()
