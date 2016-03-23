cmake_minimum_required(VERSION 2.8)
project( MultiTrackerApp )

set(TEMP_CXX_FLAGS  ${CMAKE_CXX_FLAGS} )
set(CMAKE_CXX_FLAGS "-std=c++11 -O3 ${TEMP_CXX_FLAGS}")  # TODO: Up the optimization level
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

message(STATUS "")
message(STATUS "///////////////////////////////////////////////////////////////")
message(STATUS "/////////////// CMAKE MESSAGES ////////////////////////////////")
message(STATUS "///////////////////////////////////////////////////////////////")

message(STATUS "Compiler flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "Linker flags:   ${CMAKE_EXE_LINKER_FLAGS}")

### including project headers ###
include_directories( ${CMAKE_SOURCE_DIR}/Libraries/Opencv3.1/include )
include_directories( ${CMAKE_SOURCE_DIR}/Libraries/EasyLogging )
include_directories( ${CMAKE_SOURCE_DIR}/Libraries )
include_directories( ${CMAKE_SOURCE_DIR}/Source )
include_directories( ${CMAKE_SOURCE_DIR}/Source/Utility )

# Specify the directories where libraries are located.  
link_directories( ${CMAKE_SOURCE_DIR}/Libraries/Opencv3.1/lib  )

# Creates the executable file with the source files
# TODO: Possibly use GLOB to specify source files
add_executable( MultiTrackerApp Source/main.cpp 
                    Source/Interfaces/CMultiTrackerApp.cpp
)
message(STATUS "lib:" ${CMAKE_SOURCE_DIR})
target_link_libraries( MultiTrackerApp	   opencv_shape
		                                   opencv_stitching
		                                   opencv_objdetect 
		                                   opencv_videostab  
		                                   opencv_calib3d
		                                   opencv_features2d
		                                   opencv_videoio
		                                   opencv_imgcodecs 
		                                   opencv_video 
		                                   opencv_photo
		                                   opencv_ml  
		                                   opencv_flann 
		                                   opencv_hal
		                                   opencv_viz
		                                   opencv_superres
		                                   opencv_core
						   				   opencv_highgui
										   opencv_imgproc
						   				   
					           			   pthread
)

### PostBuild Copy Files ###
file( COPY Config DESTINATION "/home/borotech/Development/repos/MultiTrackerApp/build" )

message(STATUS "///////////////////////////////////////////////////////////////")
message(STATUS "/////////////// END CMAKE MESSAGES ////////////////////////////")
message(STATUS "///////////////////////////////////////////////////////////////")
