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


	std::vector<TTrackObject>	m_trackedObjects;

	// Methods
	bool Initialize();
	static void CalibrateHSV( int stateIn, void *userDataIn );
	void Run();

private:
	// Attributes
	cv::VideoCapture			_cap;
	std::string					_controlWindow;

	cv::Mat						_origImage;
	cv::Mat						_imageHSV;

	cv::Mat						_imageThreshold;
	cv::Mat						_linesImage;

	TCalibration				_calibrationData;

	// Methods
};
