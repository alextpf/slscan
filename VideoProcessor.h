#pragma once

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// The frame processor interface
class FrameProcessor
{
public:
	FrameProcessor() : m_Debug( false )
	{}

	// processing method
    virtual void Process( cv::Mat &input, cv::Mat &output ) = 0;
	bool m_Debug;
}; // class FrameProcessor

class VideoProcessor
{
public:
    // Constructor setting the default values
    VideoProcessor();

    // set the name of the video file
    bool SetInput( std::string filename );

    // set the camera ID
    bool SetInput( int id );

    // set the vector of input m_Images
    void SetInput( const std::vector<std::string>& imgs );

    // set the output video file
    // by default the same parameters than input video will be used
    bool SetOutput(
        const std::string& filename,
        int codec = 0,
        double framerate = 0.0,
        bool isColor = true );

    // set the output as a series of image files
    // extension must be ".jpg", ".bmp" ...
    bool SetOutput(
        const std::string& filename, // filename prefix
        const std::string& ext, // image file extension
        int numberOfDigits = 3,   // number of digits
        int startIndex = 0 );

    // set the callback function that will be called for each frame
    void SetFrameProcessor( void( *frameProcessingCallback ) ( cv::Mat&, cv::Mat& ) );

    // set the instance of the class that implements the FrameProcessor interface
    void SetFrameProcessor( FrameProcessor* frameProcessorPtr );

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
    void DisplayInput( std::string wn )
    {
        m_WindowNameInput = wn;
        cv::namedWindow( m_WindowNameInput );
    }

    // to display the processed frames
    void DisplayOutput( std::string wn )
    {
        m_WindowNameOutput = wn;
        cv::namedWindow( m_WindowNameOutput );
    }

    // do not display the processed frames
    void DontDisplay()
    {
        cv::destroyWindow( m_WindowNameInput );
        cv::destroyWindow( m_WindowNameOutput );
        m_WindowNameInput.clear();
        m_WindowNameOutput.clear();
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
    bool SetPositionMS( double pos );

    // go to this position expressed in fraction of total film length
    bool SetRelativePosition( double pos );

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
        return m_Capture.isOpened() || !m_Images.empty();
    }

    void SetInitPosX( int x )
    {
        m_InitPosX = x;
    }

    void SetInitPosY( int y )
    {
        m_InitPosY = y;
    }

    void SetOffsetX( int x )
    {
        m_OffsetX = x;
    }

    void SetOffsetY( int y )
    {
        m_OffsetX = y;
    }

    // to grab (and Process) the frames of the sequence
    void Run();

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

	std::string GetInputWinName()
	{
		return m_WindowNameInput;
	}

private:

    // the OpenCV video m_Capture object
    cv::VideoCapture m_Capture;

    // the callback function to be called
    // for the processing of each frame
    void( *m_Process ) ( cv::Mat&, cv::Mat& );

    // the pointer to the class implementing
    // the FrameProcessor interface
    FrameProcessor *m_FrameProcessor;

    // a bool to determine if the
    // Process callback will be called
    bool m_CallIt;

    // Input display window name
    std::string m_WindowNameInput;

    // Output display window name
    std::string m_WindowNameOutput;

    // delay between each frame processing
    int m_Delay;

    // whether to down sample the image to save computation power
    unsigned int m_DownSampleRate;

    // number of frames
    long m_TotalFrame;

    // stop at this frame number
    long m_FrameToStop;

    // to stop the processing
    bool m_Stop;

    // vector of image filename to be used as input
    std::vector<std::string> m_Images;

    // image vector iterator
    std::vector<std::string>::const_iterator m_ItImg;

    // the OpenCV video writer object
    cv::VideoWriter m_Writer;

    // output filename
    std::string m_OutputFile;

    // current index for output Images
    int m_CurrentIndex;

    // number of digits in output image filename
    int m_Digits;

    // extension of output Images
    std::string m_Extension;

    // extract only portion of the image
    int m_InitPosX;

    int m_InitPosY;

    int m_OffsetX; //

    int m_OffsetY; //

    cv::Mat m_TmpFrame; // tmp frame

    // to get the next frame
    // could be: video file; camera; vector of m_Images
    bool ReadNextFrame( cv::Mat& frame );

    // to write the output frame
    // could be: video file or m_Images
    void WriteNextFrame( cv::Mat& frame );

}; // class VideoProcessor

