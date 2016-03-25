// Inspired by http://opencv-srf.blogspot.com/2010/09/object-detection-using-color-seperation.html
#include "CMultiTrackerApp.h"

static TColorData				_colorCal;

CMultiTrackerApp::CMultiTrackerApp( char **argv )
	: m_cap( atoi( argv[1] ) )
	, m_controlWindow( argv[2] )
	, m_trackedObjects( 0 )
	, m_isDone( false )
	, m_isNewImage( false )
{
	_calibrationData.blue.colorId 	= "blue";
	_calibrationData.orange.colorId = "orange";
	_calibrationData.purple.colorId = "purple";
	_calibrationData.yellow.colorId = "yellow";
	_calibrationData.green.colorId 	= "green";
	_calibrationData.red.colorId 	= "red";


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

	LOG( INFO ) << "Initializing Blob Parameters";

	// Set blob parameters
	_params.minThreshold 		= 10;
	_params.maxThreshold 		= 200;

	_params.filterByArea 		= true;
	_params.minArea 			= 1500;

	_params.filterByCircularity = true;
	_params.minCircularity 		= 0.1;

	_params.filterByConvexity 	= true;
	_params.minConvexity 		= 0.87;

	_params.filterByInertia 	= true;
	_params.minInertiaRatio 	= 0.01;

	LOG( INFO ) << "Creating Blob Detector";

	// Init detector with params
	_detector = cv::SimpleBlobDetector::create( _params );

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

	_calibrationData.blue 			= _colorCal;
	_calibrationData.blue.colorId 	= "blue";

	_calibrationData.green 	= _colorCal;
	_calibrationData.green.colorId 	= "green";

	_calibrationData.orange = _colorCal;
	_calibrationData.orange.colorId 	= "orange";

	_calibrationData.purple = _colorCal;
	_calibrationData.purple.colorId 	= "purple";

	_calibrationData.red 	= _colorCal;
	_calibrationData.red.colorId 	= "red";

	_calibrationData.yellow	= _colorCal;
	_calibrationData.yellow.colorId 	= "yellow";

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
	cv::createButton( "Orange", CalibrateHSV, &_calibrationData.orange, CV_PUSH_BUTTON, 0 );
	cv::createButton( "Blue", CalibrateHSV, &_calibrationData.blue, CV_PUSH_BUTTON, 0 );

	cv::createButton( "Red", CalibrateHSV, &_calibrationData.red, CV_PUSH_BUTTON, 0 );
	cv::createButton( "Purple", CalibrateHSV, &_calibrationData.purple, CV_PUSH_BUTTON, 0 );

	cv::createButton( "Yellow", CalibrateHSV, &_calibrationData.yellow, CV_PUSH_BUTTON, 0 );
	cv::createButton( "Green", CalibrateHSV, &_calibrationData.green, CV_PUSH_BUTTON, 0 );

	return true;
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

void CMultiTrackerApp::Run()
{
	LOG( INFO ) << "Launching Color Threads";

//	for( size_t t; t < (size_t)NUM_THREADS; ++t)
//	{
//		int ret = pthread_create( &_colorThreads[t], &_threadCalData, RunColorThreads, (void*)this );
//		if( ret )
//		{
//			LOG( ERROR ) << ">>>> Error: Thread create for thread: " << t << " Falied: " << ret << " <<<<";
//		}
//	}



	while( !m_isDone )
	{
		if( !m_cap.read( m_origImage ) )
		{
			LOG( ERROR ) << ">>>> Could not read from video stream: CLOSING <<<<";
			break;
		}
		if( !m_origImage.empty() )
		{
			m_isNewImage = true;
		}else
		{
			m_isNewImage = false;
		}

		cv::cvtColor( m_origImage, _imageHSV, cv::COLOR_BGR2HSV );

		TColorData color;

//		_imageThresholdSum = cv::Mat::zeros( m_origImage.size(), CV_8UC1 );

		_keyPointsSum = cv::Mat::zeros( m_origImage.size(), CV_8UC3 );

		for( size_t i = 0; i < 5; ++i)
		{
			switch( i ){
				case EColor::BLUE:
				{
					color = _calibrationData.blue;
					break;
				}
				case EColor::ORANGE:
				{
					color = _calibrationData.orange;
					break;
				}
				case EColor::GREEN:
				{
					color = _calibrationData.green;
					break;
				}
				case EColor::RED:
				{
					color = _calibrationData.red;
					break;
				}
				case EColor::PURPLE:
				{
					color = _calibrationData.purple;
					break;
				}
				case EColor::YELLOW:
				{
					color = _calibrationData.yellow;
					break;
				}
				default:
				{
					color = _calibrationData.blue;
					break;
				}
			}

			pthread_mutex_lock( &_calibrationData.blue.threadMutex );

			cv::inRange( _imageHSV, cv::Scalar( color.lowH, color.lowS, color.lowV ), cv::Scalar( color.highH, color.highS, color.highV ), _imageThreshold[i] );

			pthread_mutex_unlock( &_calibrationData.blue.threadMutex );

			cv::inRange( _imageHSV, cv::Scalar( _colorCal.lowH, _colorCal.lowS, _colorCal.lowV ), cv::Scalar( _colorCal.highH, _colorCal.highS, _colorCal.highV ), _controlImage );

			// Morph open - remove small objects from the foreground
			cv::erode( _imageThreshold[i], _imageThreshold[i], cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 5, 5 ) ) );
			cv::dilate( _imageThreshold[i], _imageThreshold[i], cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 5, 5 ) ) );

			// Morph close - fill small holes in foreground
			cv::dilate( _imageThreshold[i], _imageThreshold[i], cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 5, 5 ) ) );
			cv::erode( _imageThreshold[i], _imageThreshold[i], cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 5, 5 ) ) );


			// Detect the blobs
			_detector->detect( _imageThreshold[i], _keyPoints );

			// Draw blobs with circles
			// flags ensures the size of the circle corresponds to the size of the blob
			cv::drawKeypoints( _imageThreshold[i], _keyPoints ,m_imageKeypoints[i], cv::Scalar( 0, 255, 0 ), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

			//_keyPoints.clear();

			// Moments ofthe threshold image for tracking
			cv::Moments imageMoments = cv::moments( _imageThreshold[i] );

			int posX, posY, lastX, lastY;
			// Filtering noise again with # .m00 area
			if( imageMoments.m00 > 10000 )
			{
				TTrackObject object;

				object.posX = imageMoments.m10 / imageMoments.m00;
				object.posY = imageMoments.m01 /imageMoments.m00;

				object.lastX = object.posX;
				object.lastY = object.posY;

				// Yellow
				cv::Scalar color = cv::Scalar( 94, 206, 165 );

				cv::circle( m_origImage, cv::Point( posX, posY ), sqrt( imageMoments.m00/3.14 )/10, color, 1.5, 8, 0 );
			}



//			_imageThresholdSum += _imageThreshold[i];

			_keyPointsSum += m_imageKeypoints[i];
		}

		// Display the images
		cv::imshow( m_controlWindow, _controlImage );

		cv::imshow( "Original-Image", m_origImage );

		cv::imshow( "Blob-Image", _keyPointsSum );

		if( cv::waitKey( 10 ) == 27 )
			{
				LOG( INFO ) << ">>>> Exiting <<<<";

				m_isDone = true;
			}

	}

	// pthread status and join wait
//	void *status;
//
//	pthread_attr_destroy( &_threadCalData );
//
//	for( size_t t; t < (size_t)NUM_THREADS; ++t)
//	{
//		int ret = pthread_join( _colorThreads[t], &status );
//
//		if( ret )
//		{
//			LOG( ERROR ) << "Error Join Thread: " << ret;
//		}
//
//		LOG( INFO ) << "Completed join with thread: " << (long)status;
//	}
}

void *CMultiTrackerApp::RunColorThreads( void *interfaceIn )
{
	CMultiTrackerApp *app = (CMultiTrackerApp*)interfaceIn;

//	while( !app->m_isDone )
//	{
//
//		LOG(INFO)<<"HERE";
//		if( app->m_isNewImage )
//		{
////			for( size_t i = 0; i < 6; ++i)
////			{
////			pthread_mutex_lock( &app->m_threadMutex );
//
//				cv::cvtColor( app->m_origImage, app->_imageHSV, cv::COLOR_BGR2HSV );
//
//				cv::inRange( app->_imageHSV, cv::Scalar( 110, 50, 50 ), cv::Scalar( 179, 255, 255 ), app->_imageThreshold );
//
//				// Morph open - remove small objects from the foreground
//				cv::erode( app->_imageThreshold, app->_imageThreshold, cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 5, 5 ) ) );
//				cv::dilate( app->_imageThreshold, app->_imageThreshold, cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 5, 5 ) ) );
//
//				// Morph close - fill small holes in foreground
//				cv::dilate( app->_imageThreshold, app->_imageThreshold, cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 5, 5 ) ) );
//				cv::erode( app->_imageThreshold, app->_imageThreshold, cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 5, 5 ) ) );
//
//				// Detect the blobs
//				app->_detector->detect( app->_imageThreshold, app->_keyPoints );
//
//				// Draw blobs with circles
//				// flags ensures the size of the circle corresponds to the size of the blob
//				cv::drawKeypoints( app->_imageThreshold,app->_keyPoints,app->m_imageKeypoints, cv::Scalar( 0, 255, 0 ), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS );
//
//				// Moments ofthe threshold image for tracking
//				cv::Moments imageMoments = cv::moments( app->_imageThreshold );
//
//				int posX, posY, lastX, lastY;
//				// Filtering noise again with # .m00 area
//				if( imageMoments.m00 > 10000 )
//				{
//					posX = imageMoments.m10 / imageMoments.m00;
//					posY = imageMoments.m01 /imageMoments.m00;
//
//					lastX = posX;
//					lastY = posY;
//
//					cv::Scalar color = cv::Scalar( 94, 206, 165 );
//
//					cv::circle( app->m_origImage, cv::Point( posX, posY ), sqrt( imageMoments.m00/3.14 )/10, color, 1.0, 8, 0 );
//				}
////			}
//
//				// Display the images
////						cv::imshow( app->m_controlWindow, app->_imageThreshold );
//
////						cv::imshow( "Original-Image", app->m_origImage );
//
//						cv::imshow( "Blob-Image", app->m_imageKeypoints );
//
//						if( cv::waitKey( 0 ) == 27 )
//								{
////									LOG( INFO ) << ">>>> Exiting <<<<";
//
////									app->m_isDone = true;
//								}
//
//				// Unlock image data
//						pthread_mutex_unlock( &app->m_threadMutex );


//		}





//	}

	return nullptr;
}
