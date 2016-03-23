#pragma once

#include "../Utility/Utility.h"

struct TColorData{
	// Color of data type
	std::string 	colorId;
	// The color
	int 			lowH;
	int 			highH;

	// The amount of white
	int				lowS;
	int 			highS;

	// The amount of black
	int 			lowV;
	int 			highV;
};


struct TCalibration{
	TColorData 		green;
	TColorData 		blue;
	TColorData 		purple;
	TColorData 		yellow;
	TColorData 		orange;
	TColorData 		red;
};

enum EColor
{
	ORANGE,
	BLUE,
	PURPLE,
	YELLOW,
	GREEN,
	RED
};

class CMultiTrackerApp
{
public:
	CMultiTrackerApp( char **argv );
	virtual ~CMultiTrackerApp();

	// Attibutes
	struct TTrackObject	{
		// Color string we are tracking
		TColorData				colorData;

		// 1st order spatial around x-axis - center of object
		double 					moments01;
		// 1st order spatial around y-axis - center of object
		double 					moments10;
		// Area of white of object
		double 					momentsArea;

		// Position of object in camera frame
		int 					posX;
		int 					posY;

		// Last known position of object
		int 					lastX;
		int 					lastY;
	} m_object;

	cv::SimpleBlobDetector::Params	_params;
	std::vector<TTrackObject>		m_trackedObjects;

	const int 						NUM_THREADS = 6;
	EColor 							m_colorCount;

	pthread_mutex_t					m_threadMutex;
	bool							m_isDone;

	cv::VideoCapture				m_cap;
	cv::Mat							m_origImage;

	cv::Mat 						m_imageKeypoints;
	std::string						m_controlWindow;

	// Methods
	bool Initialize();
	static void CalibrateHSV( int stateIn, void *userDataIn );

	void Run();
	bool HasNewImage();

private:
	// Attributes


	cv::Mat								_imageHSV;

	cv::Mat								_imageThreshold;
	cv::Mat								_linesImage;

	TCalibration						_calibrationData;
	cv::Ptr<cv::SimpleBlobDetector> 	_detector;

	std::vector<cv::KeyPoint>			_keyPoints;

	pthread_t							_colorThreads[];
	pthread_attr_t						_threadCalData;

	pthread_t							_cameraThread;


	bool 								_isNewImage;

	// Methods
	static void *RunColorThreads( void *interfaceIn );
	static void *RunCamThread( void *interfaceIn );
};
