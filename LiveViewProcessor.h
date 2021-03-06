#pragma once

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;

// The frame processor interface
class FrameProcessor
{
public:
	// processing method
	virtual void Process( cv::Mat &input, cv::Mat &output ) = 0;
}; // class FrameProcessor

class TwoFrameProcessor
{
public:
	// processing method
	virtual void Process( vector<cv::Mat> &input, vector<cv::Mat> &output ) = 0;
}; // class TwoFrameProcessor

class LiveViewProcessor
{
public:
    enum WEB_CAM_ID {
        DEFAULT_CAM,
        LEFT_CAM,
        RIGHT_CAM
    };

    // Constructor setting the default values
    LiveViewProcessor();

	void Init();

    // set the name of the video file
    bool SetInput( vector<string> filename /*video names*/ );

    // set the camera ID
    bool SetInput( vector<WEB_CAM_ID> id /*webcam id*/ );

    // set the vector of input m_Images
    void SetInput( const vector<vector<string>>& imgs /*sets of img sequence*/ );

    // set the output as a series of image files
    // extension must be ".jpg", ".bmp" ...
    bool SetOutput(
        const vector<string>& filename, // filename prefix
        const string& ext, // image file extension
        int numberOfDigits,   // number of digits
        int startIndex = 0 );

    // set the callback function that will be called for each frame
    void SetOneFrameProcessor( void( *frameProcessingCallback ) ( cv::Mat&, cv::Mat& ) );

    // set the instance of the class that implements the FrameProcessor interface
    void SetOneFrameProcessor( FrameProcessor* frameProcessorPtr );

	// set the callback function that will be called for each frame
	void SetTwoFrameProcessor( void( *frameProcessingCallback ) ( vector<cv::Mat>&, vector<cv::Mat>& ) );

	// set the instance of the class that implements the TwoFrameProcessor interface
	void SetTwoFrameProcessor( TwoFrameProcessor* frameProcessorPtr );

    // stop streaming at this frame number
    void StopAtFrameNo( long frame ) {

        m_FrameToStop = frame;
    }

    // Process callback to be called
    void CallProcess()
    {
        m_CallIt = true;
    }

    // do not call Process callback
    void DontCallProcess()
    {
        m_CallIt = false;
    }

    // to display the processed frames
    void DisplayInput( vector<string> wn )
    {
        for( int i = 0; i < m_NumSource; i++ )
        {
            m_WindowNameInput.push_back( wn[i] );
            cv::namedWindow( m_WindowNameInput[i] );
        }
    }

    // to display the processed frames
    void DisplayOutput( vector<string> wn )
    {
        for( int i = 0; i < wn.size(); i++ )
        {
            m_WindowNameOutput.push_back( wn[i] );
			cv::namedWindow( m_WindowNameOutput[i] );
        }
    }

    // do not display the processed frames
    void DontDisplay()
    {
        for( int i = 0; i < m_NumSource; i++ )
        {
            cv::destroyWindow( m_WindowNameInput[i] );
            cv::destroyWindow( m_WindowNameOutput[i] );
            m_WindowNameInput[i].clear();
            m_WindowNameOutput[i].clear();
        }
    }

    // set a m_Delay between each frame
    // 0 means wait at each frame
    // negative means no m_Delay
    void SetDelay( int d )
    {
        m_Delay = d;
    }

    // return the size of the video frame
    cv::Size GetFrameSize();

    // return the frame number of the next frame
    long GetFrameNumber();

    // return the position in ms
    double GetPositionMS();

    // return the frame rate
    double GetFrameRate();

    // return the number of frames in video
    long GetTotalFrameCount();

    // get the codec of input video
    int GetCodec( char codec[4] );

    // go to this frame number
    bool SetFrameNumber( long pos );

    // go to this position
    bool SetPositionMS( double pos /*in ms, for vid or cam*/ );

    // go to this position expressed in fraction of total film length
    bool SetRelativePosition( double pos /*in %*/ );

    // Stop the processing
    void StopIt()
    {
        m_Stop = true;
    }

    // Is the Process stopped?
    bool IsStopped()
    {
        return m_Stop;
    }

    // Is a m_Capture device opened?
    bool IsOpened()
    {
		// either one of the camera should be open

		int s = static_cast<int>( m_Capture.size() );

		bool isOpen( true );

		for ( int i = 0; i < s; i++ )
		{
			isOpen = isOpen && ( m_Capture[i].isOpened() || !m_Images[i].empty() );
		}

        return isOpen;
    }

    // to grab (and Process) the frames of the sequence
    virtual void Run();

    // the image will be down-sampled by 1/t in both width and height
    void SetDownSampleRate( unsigned int t )
    {
        m_DownSampleRate = t;
    }

    // Get down-sample rate. The image is down-sampled by 1/t in both width and height
    unsigned int GetDownSampleRate()
    {
        return m_DownSampleRate;
    }

	vector<string> GetInputWinName()
	{
		return m_WindowNameInput;
	}

	void SetNumSource( const int n )
	{
		m_NumSource = n;
	}

	void SetFileNameNumDigits( int s )
	{
		m_Digits = s;
	}

	int SetFileNameNumDigits()
	{
		return m_Digits;
	}

    void SetScaleFactorForShow( const float s )
    {
        m_ScaleFactorForShow = s;
    }

protected:

    // to get the next frame
    bool ReadNextFrame( vector<cv::Mat>& frame );

    // to write the output frame
    void WriteNextFrame( vector<cv::Mat>& frame );

    // the callback function to be called
    // for the processing of each frame
    void( *m_OneFrameProcess ) ( cv::Mat&, cv::Mat& );

    void( *m_TwoFrameProcess ) ( vector<cv::Mat>&, vector<cv::Mat>& );

	int                         m_NumSource;

    // the OpenCV video m_Capture object
	vector<cv::VideoCapture>    m_Capture;

    // the pointer to the class implementing
    // the FrameProcessor interface
    FrameProcessor *            m_OneFrameProcessor;
	TwoFrameProcessor *         m_TwoFrameProcessor;

    // a bool to determine if the
    // Process callback will be called
    bool                        m_CallIt;

    // Input display window name
	vector<string>              m_WindowNameInput;

    // Output display window name
	vector<string>              m_WindowNameOutput;

    // delay between each frame processing
    int                         m_Delay;

    // whether to down sample the image to save computation power
    unsigned int                m_DownSampleRate;

    // number of frames
    long                        m_TotalFrame;

    // stop at this frame number
    long                        m_FrameToStop;

    // to stop the processing
    bool                        m_Stop;

	// vector of image filename to be used as input
	vector<vector<string>>      m_Images;

	// image vector iterator
	vector<vector<string>::const_iterator> m_ItImg;

    // output filename
	vector<string>              m_OutputFile;

    // current index for output Images
    vector<int>                 m_CurrentIndex;

    // number of digits in output image filename
    int                         m_Digits;

    // extension of output Images
    string                      m_Extension;

    float                       m_ScaleFactorForShow; // scale for showing purpose
}; // class LiveViewProcessor

