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

	pthread_mutex_t		threadMutex;
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

	static const int 				NUM_THREADS = 1;
	EColor 							m_colorCount;

	static const int				NUM_COLORS = 6;

	pthread_mutex_t					m_threadMutex;
	bool							m_isDone;

	cv::VideoCapture				m_cap;
	cv::Mat							m_origImage;

	cv::Mat 						m_imageKeypoints;
	std::string						m_controlWindow;

	bool 							m_isNewImage;
	bool							m_track;

	pthread_mutex_t					m_buttonMutex;

	// Methods
	bool Initialize();
	static void CalibrateHSV( int stateIn, void *userDataIn );
	void Run();


private:
	// Attributes
	cv::Mat								_imageHSV;

	cv::Mat								_imageThreshold[NUM_COLORS];
	cv::Mat								_imageThresholdSum;
	cv::Mat								_controlImage;
	cv::Mat								_linesImage;

	TCalibration						_calibrationData;

	pthread_t							_colorThreads[NUM_THREADS];
	pthread_attr_t						_threadCalData;

	pthread_t							_cameraThread;


	// Methods
	static void *RunColorThreads( void *interfaceIn );
	static void ToggleTrack( int statIn, void *userData );
};
