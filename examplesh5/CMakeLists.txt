set(CMAKE_AUTOMOC ON)

set(EXAMPLESH5_LINK_LIBRARIES
  Qt5::Core
  Qt5::Widgets
  Brion
  Brain
  prefr
)

if (ZEQ_FOUND)
  list(APPEND EXAMPLESH5_LINK_LIBRARIES zeq zeqHBP ${CMAKE_THREAD_LIBS_INIT})
endif()

if (GMRVZEQ_FOUND)
  list( APPEND EXAMPLESH5_LINK_LIBRARIES gmrvzeq )
endif()

list( APPEND EXAMPLESH5_LINK_LIBRARIES ${HDF5_LIBRARIES})

add_executable( loadh5 testh5.cpp )
target_link_libraries( loadh5 ${EXAMPLESH5_LINK_LIBRARIES} )

add_executable( simDataH5 simData.cpp )
target_link_libraries( simDataH5 ${EXAMPLESH5_LINK_LIBRARIES} sumrice )


  
   
                       