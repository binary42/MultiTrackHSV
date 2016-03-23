// Inspired by http://opencv-srf.blogspot.com/2010/09/object-detection-using-color-seperation.html
#include "CMultiTrackerApp.h"

static 	TColorData				_colorCal;

CMultiTrackerApp::CMultiTrackerApp( char **argv )
	: _cap( atoi( argv[1] ) )
	, _controlWindow( argv[2] )
	, m_trackedObjects( 0 )
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

}

bool CMultiTrackerApp::Initialize()
{

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

	// Init detector with params
	_detector = cv::SimpleBlobDetector::create( _params );
	// _detector->detect( image, keyPoints ); // detect blob

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

	// Set control window
	cv::namedWindow( _controlWindow, CV_WINDOW_AUTOSIZE);

	// track bars for HSV tuning
	cv::createTrackbar( "LowH", _controlWindow, &_colorCal.lowH, 179 );
	cv::createTrackbar( "HighH", _controlWindow, &_colorCal.highH, 179 );

	cv::createTrackbar( "LowS", _controlWindow, &_colorCal.lowS, 255 );
	cv::createTrackbar( "HighS", _controlWindow, &_colorCal.highS, 255 );

	cv::createTrackbar( "LowV", _controlWindow, &_colorCal.lowV, 255 );
	cv::createTrackbar( "HighV", _controlWindow, &_colorCal.highV, 255 );

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

	LOG( INFO ) << "Calibratiing: " << calData->colorId;

	calData->lowH 	= _colorCal.lowH;
	calData->highH 	= _colorCal.highH;

	calData->lowS 	= _colorCal.lowS;
	calData->highS 	= _colorCal.highS;

	calData->lowV 	= _colorCal.lowV;
	calData->highV	= _colorCal.highV;

}

void CMultiTrackerApp::Run()
{
//	while( true )
//	{
//		if( !_cap.read( _origImage ) )
//		{
//			LOG( ERROR ) << ">>>> Could not read from video stream: CLOSING <<<<";
////			break;
//		}
		cv::Mat _origImage = cv::imread( "roboops_test_image.png", cv::IMREAD_COLOR );

		cv::cvtColor( _origImage, _imageHSV, cv::COLOR_BGR2HSV );

//		cv::threshold( _imageHSV, _imageThreshold, 127, 255, cv::THRESH_TOZERO );

		cv::inRange( _imageHSV, cv::Scalar( 110, 50, 50 ), cv::Scalar( 179, 255, 255 ), _imageThreshold );

		// Morph open - remove small objects from the foreground
		cv::erode( _imageThreshold, _imageThreshold, cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 5, 5 ) ) );
		cv::dilate( _imageThreshold, _imageThreshold, cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 5, 5 ) ) );

		// Morph close - fill small holes in foreground
		cv::dilate( _imageThreshold, _imageThreshold, cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 5, 5 ) ) );
		cv::erode( _imageThreshold, _imageThreshold, cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 5, 5 ) ) );

		// Detect the blobs
		_detector->detect( _imageThreshold, _keyPoints );

		// Draw blobs as red circles
		// flags ensures the size of the circle corresponds to the size of the blob
		cv::Mat _imageKeypoints;

		cv::drawKeypoints( _imageThreshold, _keyPoints, _imageKeypoints, cv::Scalar( 0, 255, 0 ), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

		// Moments ofthe threshold image for tracking
		cv::Moments imageMoments = cv::moments( _imageThreshold );
		int posX, posY, lastX, lastY;

		if( imageMoments.m00 > 10000 )
		{
			posX = imageMoments.m10 / imageMoments.m00;
			posY = imageMoments.m01 /imageMoments.m00;

			if( lastX >= 0 && lastY >=0 && posX >= 0 && posY >= 0 )
			{
				// Draw a line from previous pose to current pose
				cv::line( _linesImage, cv::Point( posX, posY )
							, cv::Point( lastX, lastY ), cv::Scalar( 0, 0, 255 ), 2 );
			}

			lastX = posX;
			lastY = posY;

			cv::Scalar color = cv::Scalar( 94, 206, 165 );

			cv::circle( _origImage, cv::Point( posX, posY ), sqrt( imageMoments.m00/3.14 )/10, color, 1.0, 8, 0 );
		}

//		_linesImage = cv::Mat::zeros( _origImage.size(), CV_8UC3 );
//		for( std::vector<TTrackObject>::iterator itr = m_trackedObjects.begin(); itr != m_trackedObjects.end(); ++itr)
//		{
//
//
//			// Convert from BGR to HSV
//			cv::cvtColor( _origImage, _imageHSV, cv::COLOR_BGR2HSV );
//
//			// Threshold the image values
//			cv::inRange( _imageHSV, cv::Scalar( itr->colorData.lowH, itr->colorData.lowS, itr->colorData.lowV ), cv::Scalar( itr->colorData.highH,
//																					itr->colorData.highS, itr->colorData.highV ), _imageThreshold );
//
//			// Morph open - remove small objects from the foreground
//			cv::erode( _imageThreshold, _imageThreshold, cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 5, 5 ) ) );
//			cv::dilate( _imageThreshold, _imageThreshold, cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 5, 5 ) ) );
//
//			// Morph close - fill small holes in foreground
//			cv::dilate( _imageThreshold, _imageThreshold, cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 5, 5 ) ) );
//			cv::erode( _imageThreshold, _imageThreshold, cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size( 5, 5 ) ) );
//
//			// Moments ofthe threshold image for tracking
//			cv::Moments imageMoments = cv::moments( _imageThreshold );
//
//			m_trackedObjects[0].moments01 = imageMoments.m01;
//			m_trackedObjects[0].moments10 = imageMoments.m10;
//			m_trackedObjects[0].momentsArea = imageMoments.m00;
//
//			// Noise threshold
//			if( m_trackedObjects[0].momentsArea > 1000 )
//			{
//				m_trackedObjects[0].posX = m_trackedObjects[0].moments10 / m_trackedObjects[0].momentsArea;
//				m_trackedObjects[0].posY = m_trackedObjects[0].moments01 / m_trackedObjects[0].momentsArea;
//
//				if( m_trackedObjects[0].lastX >= 0 && m_trackedObjects[0].lastY >=0 && m_trackedObjects[0].posX >= 0 && m_trackedObjects[0].posY >= 0 )
//				{
//					// Draw a line from previous pose to current pose
//					cv::line( _linesImage, cv::Point( m_trackedObjects[0].posX, m_trackedObjects[0].posY )
//								, cv::Point( m_trackedObjects[0].lastX, m_trackedObjects[0].lastY ), cv::Scalar( 0, 0, 255 ), 2 );
//				}
//
//				m_trackedObjects[0].lastX = m_trackedObjects[0].posX;
//				m_trackedObjects[0].lastY = m_trackedObjects[0].posY;
//
//				cv::Scalar color = cv::Scalar( 94, 206, 165 );
//
//				cv::circle( _origImage, cv::Point( m_trackedObjects[0].posX, m_trackedObjects[0].posY ), sqrt( m_trackedObjects[0].momentsArea/3.14 )/10, color, 0.5, 8, 0 );
//			}
//		}
		// Display the images
		cv::imshow( _controlWindow, _imageThreshold );

		// Display the combined image
		_origImage = _origImage + _linesImage;

		cv::imshow( "Original-Image", _origImage );
		cv::imshow( "Blob-Image", _imageKeypoints );

		if( cv::waitKey( 0 ) == 27 )
		{
			LOG( INFO ) << ">>>> Exiting <<<<";
//			break;
		}
//	}
}
