#include "CMultiTrackerApp.h"

static TColorData				_colorCal;

CMultiTrackerApp::CMultiTrackerApp( char **argv )
	: m_calibrationObjects( 0 )
	, m_isDone( false )
	, m_isNewImage( false )
	, m_track( false )
	, _recurseDir( false )
{
	m_calibrationData.blue.colorId 	= "blue";
	m_calibrationData.orange.colorId = "orange";
	m_calibrationData.purple.colorId = "purple";
	m_calibrationData.yellow.colorId = "yellow";
	m_calibrationData.green.colorId 	= "green";
	m_calibrationData.red.colorId 	= "red";

	if( !strcmp( argv[1], "vid" ) )
	{
		m_cap.open( atoi( argv[2] ) );
		m_controlWindow = argv[3];
	}else if( !strcmp(argv[2], "image" ) )
	{
		m_cap.open( atoi( argv[3] ) );
		m_controlWindow = argv[4];
		_runImage = true;
	}else if( !strcmp( argv[2], "dir" ) )
	{
		_recurseDir = true;
	}


}

CMultiTrackerApp::~CMultiTrackerApp()
{
	pthread_mutex_destroy( &m_threadMutex );

	pthread_exit( nullptr );
}

bool CMultiTrackerApp::Initialize()
{
	LOG( INFO ) << "Initializing Thread Attributes and Mutex";
	// Thread data init
	pthread_attr_init( &_threadCalData );
	pthread_attr_setdetachstate( &_threadCalData, PTHREAD_CREATE_JOINABLE );

	pthread_mutex_init( &m_threadMutex, nullptr );
	pthread_mutex_init( &m_buttonMutex, nullptr );

	LOG( INFO ) << "Parsing Calibration File";

	// Parse calibration file
	ParseJsonFile( "Config/calibration.json" );

	// We use this private class variable to disallow active changes to the calibration.
	// If we use the _calibrationData structure for calibration and adjustment in real time, we have to retrain
	// at this time.
	_colorCal.colorId = "init";

	_colorCal.lowH 	= 0;
	_colorCal.highH = 179;

	_colorCal.lowS 	= 0;
	_colorCal.highS = 255;

	_colorCal.lowV 	= 0;
	_colorCal.highV = 255;

	m_calibrationData.blue 				= _colorCal;
	m_calibrationData.blue.colorId 		= "blue";

	m_calibrationData.green 				= _colorCal;
	m_calibrationData.green.colorId 		= "green";

	m_calibrationData.orange 			= _colorCal;
	m_calibrationData.orange.colorId 	= "orange";

	m_calibrationData.purple				= _colorCal;
	m_calibrationData.purple.colorId 	= "purple";

	m_calibrationData.red 				= _colorCal;
	m_calibrationData.red.colorId 		= "red";

	m_calibrationData.yellow				= _colorCal;
	m_calibrationData.yellow.colorId 	= "yellow";

	LOG( INFO ) << "Creating Windows, Trackbars, and Buttons";

	// Set control window
	cv::namedWindow( m_controlWindow, CV_WINDOW_AUTOSIZE);

	// track bars for HSV tuning
	cv::createTrackbar( "LowH", m_controlWindow, &_colorCal.lowH, 179 );
	cv::createTrackbar( "HighH", m_controlWindow, &_colorCal.highH, 179 );

	cv::createTrackbar( "LowS", m_controlWindow, &_colorCal.lowS, 255 );
	cv::createTrackbar( "HighS", m_controlWindow, &_colorCal.highS, 255 );

	cv::createTrackbar( "LowV", m_controlWindow, &_colorCal.lowV, 255 );
	cv::createTrackbar( "HighV", m_controlWindow, &_colorCal.highV, 255 );

	// Buttons for color calibration
	cv::createButton( "Orange", CalibrateHSV, &m_calibrationData.orange, CV_PUSH_BUTTON, 0 );
	cv::createButton( "Blue", CalibrateHSV, &m_calibrationData.blue, CV_PUSH_BUTTON, 0 );

	cv::createButton( "Red", CalibrateHSV, &m_calibrationData.red, CV_PUSH_BUTTON, 0 );
	cv::createButton( "Purple", CalibrateHSV, &m_calibrationData.purple, CV_PUSH_BUTTON, 0 );

	cv::createButton( "Yellow", CalibrateHSV, &m_calibrationData.yellow, CV_PUSH_BUTTON, 0 );
	cv::createButton( "Green", CalibrateHSV, &m_calibrationData.green, CV_PUSH_BUTTON, 0 );

	cv::createButton( "Start/Stop Track", ToggleTrack, this, CV_PUSH_BUTTON, 0 );
	cv::createButton( "Load New Cal", LoadCalibration, this, CV_PUSH_BUTTON, 0 );

	cv::createButton( "Save Cal", SaveCalibration, this, CV_PUSH_BUTTON, 0 );

	return true;
}

void CMultiTrackerApp::LoadCalibration( int stateIn, void *userDataIn )
{
	CMultiTrackerApp *calData = (CMultiTrackerApp*)userDataIn;

	pthread_mutex_lock( &calData->m_buttonMutex );

	for( std::vector<TColorData>::iterator itr = calData->m_calibrationObjects.begin(); itr != calData->m_calibrationObjects.end(); ++itr )
	{
		if( itr->colorId == "red" )
		{
			itr->highH = calData->m_calibrationData.red.highH;
			itr->highS = calData->m_calibrationData.red.highS;
			itr->highV = calData->m_calibrationData.red.highV;
			itr->lowH = calData->m_calibrationData.red.lowH;
			itr->lowS= calData->m_calibrationData.red.lowS;
			itr->lowV = calData->m_calibrationData.red.lowV;
		}
		if( itr->colorId == "blue" )
		{
			itr->highH = calData->m_calibrationData.blue.highH;
			itr->highS = calData->m_calibrationData.blue.highS;
			itr->highV = calData->m_calibrationData.blue.highV;
			itr->lowH = calData->m_calibrationData.blue.lowH;
			itr->lowS= calData->m_calibrationData.blue.lowS;
			itr->lowV = calData->m_calibrationData.blue.lowV;
		}
		if( itr->colorId == "green" )
		{
			itr->highH = calData->m_calibrationData.green.highH;
			itr->highS = calData->m_calibrationData.green.highS;
			itr->highV = calData->m_calibrationData.green.highV;
			itr->lowH = calData->m_calibrationData.green.lowH;
			itr->lowS= calData->m_calibrationData.green.lowS;
			itr->lowV = calData->m_calibrationData.green.lowV;
		}
		if( itr->colorId == "yellow" )
		{
			itr->highH = calData->m_calibrationData.yellow.highH;
			itr->highS = calData->m_calibrationData.yellow.highS;
			itr->highV = calData->m_calibrationData.yellow.highV;
			itr->lowH = calData->m_calibrationData.yellow.lowH;
			itr->lowS= calData->m_calibrationData.yellow.lowS;
			itr->lowV = calData->m_calibrationData.yellow.lowV;
		}
		if( itr->colorId == "purple" )
		{
			itr->highH = calData->m_calibrationData.purple.highH;
			itr->highS = calData->m_calibrationData.purple.highS;
			itr->highV = calData->m_calibrationData.purple.highV;
			itr->lowH = calData->m_calibrationData.purple.lowH;
			itr->lowS= calData->m_calibrationData.purple.lowS;
			itr->lowV = calData->m_calibrationData.purple.lowV;
		}
		if( itr->colorId == "orange" )
		{
			itr->highH = calData->m_calibrationData.orange.highH;
			itr->highS = calData->m_calibrationData.orange.highS;
			itr->highV = calData->m_calibrationData.orange.highV;
			itr->lowH = calData->m_calibrationData.orange.lowH;
			itr->lowS= calData->m_calibrationData.orange.lowS;
			itr->lowV = calData->m_calibrationData.orange.lowV;
		}
	}

	pthread_mutex_unlock( &calData->m_buttonMutex );
}

void CMultiTrackerApp::SaveCalibration( int stateIn, void *userDataIn )
{
	CMultiTrackerApp *calData = (CMultiTrackerApp*)userDataIn;

	std::ofstream calibrationFile;

	calibrationFile.open( "calibration1.json" );

	pthread_mutex_lock( &calData->m_buttonMutex );

	LOG( INFO ) << "Saving Calibration Data";

	calibrationFile <<"{\n";

	for( std::vector<TColorData>::iterator itr = calData->m_calibrationObjects.begin(); itr != calData->m_calibrationObjects.end(); ++itr )
	{
		calibrationFile << "\"" << itr->colorId << "\":{\n";

		calibrationFile << "\"lowH\":" << itr->lowH << ",\n";


		calibrationFile << "\"lowS\":" << itr->lowS << ",\n";


		calibrationFile << "\"lowV\":" << itr->lowV << ",\n";


		calibrationFile << "\"highH\":" << itr->highH << ",\n";


		calibrationFile << "\"highS\":" << itr->highS << ",\n";


		calibrationFile << "\"highV\":" << itr->highV << "\n";

		calibrationFile <<  "},";
	}

	calibrationFile << "}";

	calibrationFile.close();

	pthread_mutex_unlock( &calData->m_buttonMutex );
}

void CMultiTrackerApp::ParseJsonFile( const std::string &fileIn )
{
	TColorData color;

	rapidjson::Value calibrationParam;

	std::ifstream calibrationFile( fileIn.c_str() );

	if( calibrationFile )
	{
		// Write to buffer
		std::stringstream dataBuf;

		dataBuf << calibrationFile.rdbuf();

		calibrationFile.close();

		// Create document
		if( _calibrationDoc.Parse<0>( dataBuf.str().c_str() ).HasParseError() )
		{
			LOG( ERROR ) << ">>>> Cannot Open Json File <<<<";

			auto errorString( "Unable to Parse .json file: " + fileIn );

			throw std::invalid_argument( errorString );
		}
	}else
	{
		LOG( ERROR ) << ">>>> Cannot Open Json File <<<<";

		auto errorString( "Unable to Parse .json file: " + fileIn );

		throw std::invalid_argument( errorString );
	}

	for(size_t i = 0; i < NUM_COLORS; ++i)
	{
		switch( i ){
		case 0:
		{
			calibrationParam = _calibrationDoc["blue"];

			color.colorId = "blue";

			break;
		}
		case 1:
		{
			calibrationParam = _calibrationDoc["orange"];

			color.colorId = "orange";

			break;
		}
		case 2:
		{
			calibrationParam = _calibrationDoc["purple"];

			color.colorId = "purple";

			break;
		}
		case 3:
		{
			calibrationParam = _calibrationDoc["yellow"];

			color.colorId = "yellow";

			break;
		}
		case 4:
		{
			calibrationParam = _calibrationDoc["green"];

			color.colorId = "green";

			break;
		}
		case 5:
		{
			calibrationParam = _calibrationDoc["red"];

			color.colorId = "red";

			break;
		}
		}

		color.lowH = calibrationParam["lowH"].GetInt();
		color.lowS = calibrationParam["lowS"].GetInt();
		color.lowV = calibrationParam["lowV"].GetInt();

		color.highH = calibrationParam["highH"].GetInt();
		color.highS = calibrationParam["highS"].GetInt();
		color.highV = calibrationParam["highV"].GetInt();

		m_calibrationObjects.push_back( color );

	}
}

void CMultiTrackerApp::CalibrateHSV( int stateIn, void *userDataIn )
{
	TColorData *calData = (TColorData*)userDataIn;

	pthread_mutex_lock( &calData->threadMutex );

	LOG( INFO ) << "Calibrating: " << calData->colorId;

	calData->lowH 	= _colorCal.lowH;
	calData->highH 	= _colorCal.highH;

	calData->lowS 	= _colorCal.lowS;
	calData->highS 	= _colorCal.highS;

	calData->lowV 	= _colorCal.lowV;
	calData->highV	= _colorCal.highV;

	pthread_mutex_unlock( &calData->threadMutex );

}

void CMultiTrackerApp::ToggleTrack( int statIn, void *userDataIn )
{
	CMultiTrackerApp *app = (CMultiTrackerApp*)userDataIn;

	pthread_mutex_lock( &app->m_buttonMutex );

	if( app->m_track )
	{
		LOG( INFO ) << "Not Tracking";

		app->m_track = false;
	}else
	{
		LOG( INFO ) << "Tracking";

		app->m_track = true;
	}

	pthread_mutex_unlock( &app->m_buttonMutex );
}

void CMultiTrackerApp::Run()
{
	LOG( INFO ) << "Launching Color Threads";

	TColorData	color;

	while( !m_isDone )
	{
		if( !m_cap.read( m_origImage ) )
		{
			LOG( ERROR ) << ">>>> Could not read from video stream: CLOSING <<<<";
			break;
		}

		cv::cvtColor( m_origImage, _imageHSV, cv::COLOR_BGR2HSV );

		// Display separate threshold window with controls
		cv::inRange( _imageHSV, cv::Scalar( _colorCal.lowH, _colorCal.lowS, _colorCal.lowV )
											, cv::Scalar( _colorCal.highH,_colorCal.highS, _colorCal.highV ), _controlImage );

		if( m_track )
		{
			// Cycle through the colors and track
			for( size_t i = 0; i < NUM_COLORS; ++i)
			{
				cv::inRange( _imageHSV, cv::Scalar( m_calibrationObjects[i].lowH,  m_calibrationObjects[i].lowS,  m_calibrationObjects[i].lowV )
													, cv::Scalar( m_calibrationObjects[i].highH, m_calibrationObjects[i].highS
													, m_calibrationObjects[i].highV ), _imageThreshold[i] );

				// Morph open - remove small objects from the foreground
				cv::erode( _imageThreshold[i], _imageThreshold[i], cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 5, 5 ) ) );
				cv::dilate( _imageThreshold[i], _imageThreshold[i], cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 5, 5 ) ) );

				// Morph close - fill small holes in foreground
				cv::dilate( _imageThreshold[i], _imageThreshold[i], cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 5, 5 ) ) );
				cv::erode( _imageThreshold[i], _imageThreshold[i], cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 5, 5 ) ) );

				std::vector<std::vector<cv::Point> > contours;
				std::vector<cv::Vec4i> heirarchy;

				findContours( _imageThreshold[i], contours, heirarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );

				double refArea = 0;

				if( heirarchy.size() > 0 )
				{
					int numObjects = heirarchy.size();

					if( numObjects < 50 )
					{
						for( size_t i = 0; i < heirarchy.size(); ++i )
						{
							// Moments ofthe threshold image for tracking
							cv::Moments imageMoments = cv::moments( (cv::Mat)contours[i] );

							if( imageMoments.m00 > 1000 )
							{
								int posX, posY;

								posX = imageMoments.m10 / imageMoments.m00;
								posY = imageMoments.m01 /imageMoments.m00;

								// Yellow
								cv::Scalar color = cv::Scalar( 94, 206, 165 );

								cv::circle( m_origImage, cv::Point( posX, posY ), sqrt( imageMoments.m00/3.14 ), color, 2, 8, 0 );

							}
						}
					}
				}// hierarchy size lif
			}// end color loop
		}// if statement end

		// Display the images
		cv::imshow( m_controlWindow, _controlImage );

		cv::imshow( "Original-Image", m_origImage );

		if( cv::waitKey( 10 ) == 27 )
		{
			LOG( INFO ) << ">>>> Exiting <<<<";

			m_isDone = true;
		}
	}
}

void *CMultiTrackerApp::RunColorThreads( void *interfaceIn )
{
	CMultiTrackerApp *app = (CMultiTrackerApp*)interfaceIn;

	return nullptr;
}
