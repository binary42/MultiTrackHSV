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
	cv::SimpleBlobDetector::Params	_params;
	std::vector<TColorData>		m_calibrationObjects;

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

	TCalibration					m_calibrationData;

	pthread_mutex_t					m_buttonMutex;

	// Methods
	bool Initialize();
	void ParseJsonFile( const std::string &fileIn );

	static void SaveCalibration( int stateIn, void *userDataIn );
	static void CalibrateHSV( int stateIn, void *userDataIn );

	static void LoadCalibration( int stateIn, void *userDataIn );

	void Run();


private:
	// Attributes
	cv::Mat								_imageHSV;

	cv::Mat								_imageThreshold[NUM_COLORS];
	cv::Mat								_imageThresholdSum;
	cv::Mat								_controlImage;
	cv::Mat								_linesImage;



	pthread_t							_colorThreads[NUM_THREADS];
	pthread_attr_t						_threadCalData;

	pthread_t							_cameraThread;

	bool								_runImage;
	bool								_recurseDir;

	rapidjson::Document					_calibrationDoc;


	// Methods
	static void *RunColorThreads( void *interfaceIn );
	static void ToggleTrack( int statIn, void *userData );
};
