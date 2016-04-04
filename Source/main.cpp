/**============================================================================
// Name        : CMultiTrackerApp.cpp
// Author      : J. H. Neilan
// Version     : 0.5
// Copyright   : BSD
// Description : Histogram MultiTracker
============================================================================
*/

#include <stdio.h>
#include <stdlib.h>

#include "Utility/Utility.h"
#include "Interfaces/CMultiTrackerApp.h"

INITIALIZE_EASYLOGGINGPP

void InitializeLogger()
{
    // Specifiy the location of the configuration file
	el::Configurations config( "Config/logger.config" );

	// Reconfigure the logger
	el::Loggers::reconfigureAllLoggers( config );
}

void DisplayUsage()
{
	LOG( INFO ) << ">>>>=== Missing Arguments ===<<<<";

	LOG( INFO ) << "Usage: ./MultiTrackerApp [mode] [window name/directory/filename]\n"
	 << " \nWhere [mode] is either 'image' or 'vid' or 'dir'\n"
	 << "If [mode] is vid: input the device number of the camera stream followed by the output window name. i.e. ./MutiTrackerApp vid 0 TestOutput\n"
	 << "If [mode] is image: input the image location. i.e. ./MultiTrackerApp image /home/test.png.\n"
	 << "If [mode] is dir: input the location to recurse. i.e. /home/pictures";

	LOG( INFO ) << ">>>>=========================<<<<";
}

int main( int argc, char **argv ) {

	//Initialize easylogging
	InitializeLogger();

	//TODO - Use boost cmdline args

	if( argc < 3 )
	{
		DisplayUsage();

		return EXIT_FAILURE;
	}
	else
	{
		std::unique_ptr<CMultiTrackerApp> trackerApp( new CMultiTrackerApp( argv ) );

		if( !trackerApp->Initialize() )
		{
			LOG( INFO ) << ">>>> Application Failed To Initialize <<<<";
			return false;
		}

		trackerApp->Run();

		LOG( INFO ) << ">>>> Application Closing <<<<";

		return EXIT_SUCCESS;
	}
}
