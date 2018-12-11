#include "VideoProcessor.h"

#include <conio.h> // for getch
#include <windows.h> // for kbhit

#define FRAME_WIDTH 640
#define FRAME_HEIGHT 360

//=======================================================================
VideoProcessor::VideoProcessor()
: m_CallIt( false )
, m_Delay( -1 )
, m_DownSampleRate( 1 ) // no downsample
, m_TotalFrame( 0 )
, m_Stop( false )
, m_Digits( 0 )
, m_FrameToStop( -1 )
, m_Process( 0 )
, m_FrameProcessor( 0 )
, m_InitPosX( -1 )
, m_InitPosY( -1 )
, m_OffsetX( -1 )
, m_OffsetY( -1 )
{}

//=======================================================================
bool VideoProcessor::ReadNextFrame( cv::Mat& frame )
{
    bool ok = false;

    if( m_Images.size() == 0 )
    {
        //////////////////////////
        // it's video or webcam
        //////////////////////////
        ok = m_Capture.read( m_TmpFrame );
    }
    else
    {
        ////////////////
        // it's images
        ////////////////
        if( m_ItImg != m_Images.end() )
        {
            //printf( "%s\n", ( *m_ItImg ).c_str() ); // debug: print file path
            m_TmpFrame = cv::imread( *m_ItImg );
            m_ItImg++;

            ok = m_TmpFrame.data != 0;
        }
    }

    // whether we extract only portion of the image
    if( m_OffsetX > 0 && m_OffsetY > 0 && m_InitPosX >= 0 && m_InitPosY >= 0 )
    {
        frame = m_TmpFrame( cv::Rect( m_InitPosX, m_InitPosY, m_OffsetX, m_OffsetY ) ).clone();
    }
    else
    {
        frame = m_TmpFrame;
    }

	if (m_DownSampleRate > 1)
	{
		cv::resize(frame, frame, cv::Size(), 1.0 / m_DownSampleRate, 1.0 / m_DownSampleRate);
	}

    return ok;
}

//=======================================================================
void VideoProcessor::WriteNextFrame( cv::Mat& frame )
{
    if( m_Extension.length() )
    {
        ////////////////
        // it's images
        ////////////////

        std::stringstream ss;
        ss << m_OutputFile << std::setfill( '0' ) << std::setw( m_Digits ) << m_CurrentIndex++ << m_Extension;
        cv::imwrite( ss.str(), frame );
    }
    else
    {
        ////////////////
        // it's video
        ////////////////
        m_Writer.write( frame );
    }
}

//=======================================================================
bool VideoProcessor::SetInput( std::string filename )
{
    m_TotalFrame = 0;
    // In case a resource was already
    // associated with the VideoCapture instance
    m_Capture.release();
    m_Images.clear();

    // Open the video file
    return m_Capture.open( filename );
}

//=======================================================================
bool VideoProcessor::SetInput( int id )
{
    m_TotalFrame = 0;
    // In case a resource was already
    // associated with the VideoCapture instance
    m_Capture.release();
    m_Images.clear();

	m_Capture.set( CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH );
	m_Capture.set( CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT );

    // Open the video file
    return m_Capture.open( id );
}

//=======================================================================
void VideoProcessor::SetInput( const std::vector<std::string>& imgs )
{
    m_TotalFrame = 0;
    // In case a resource was already
    // associated with the VideoCapture instance
    m_Capture.release();

    // the input will be this vector of m_Images
    m_Images = imgs;
    m_ItImg = m_Images.begin();
}

//=======================================================================
bool VideoProcessor::SetOutput(
    const std::string& filename,
    int codec,
    double framerate,
    bool isColor )
{
    m_OutputFile = filename;
    m_Extension.clear();

    if( framerate == 0.0 )
    {
        framerate = GetFrameRate(); // same as input
    }

    char c[4];
    // use same codec as input
    if( codec == 0 )
    {
        codec = GetCodec( c );
    }

    // Open output video
    return m_Writer.open( m_OutputFile, // filename
        codec, // codec to be used
        framerate,      // frame rate of the video
        GetFrameSize(), // frame size
        isColor );       // color video?
}

//=======================================================================
bool VideoProcessor::SetOutput(
    const std::string& filename, // filename prefix
    const std::string& ext, // image file m_Extension
    int numberOfDigits,   // number of digits
    int startIndex )
{
    // number of m_Digits must be positive
    if( numberOfDigits < 0 )
    {
        return false;
    }

    // filenames and their common m_Extension
    m_OutputFile = filename;
    m_Extension = ext;

    // number of m_Digits in the file numbering scheme
    m_Digits = numberOfDigits;
    // start numbering at this index
    m_CurrentIndex = startIndex;

	return true;
}

//=======================================================================
void VideoProcessor::SetFrameProcessor( void( *frameProcessingCallback )( cv::Mat&, cv::Mat& ) )
{
    // invalidate frame processor class instance
    m_FrameProcessor = 0;
    // this is the frame processor function that will be called
    m_Process = frameProcessingCallback;
    CallProcess();
}

//=======================================================================
void VideoProcessor::SetFrameProcessor( FrameProcessor* frameProcessorPtr )
{
    // invalidate callback function
    m_Process = 0;
    // this is the frame processor instance that will be called
    m_FrameProcessor = frameProcessorPtr;

	if( m_FrameProcessor != NULL )
	{
		CallProcess();
	}
}

//=======================================================================
cv::Size VideoProcessor::GetFrameSize()
{
    if( m_Images.size() == 0 )
    {
        // get size of from the m_Capture device
        int w = static_cast<int>( m_Capture.get( CV_CAP_PROP_FRAME_WIDTH ) );
        int h = static_cast<int>( m_Capture.get( CV_CAP_PROP_FRAME_HEIGHT ) );

		if (m_DownSampleRate > 1)
		{
			w /= m_DownSampleRate;
			h /= m_DownSampleRate;
		}

        return cv::Size( w, h );
    }
    else
    {
        // if input is vector of images
        cv::Mat tmp = cv::imread( m_Images[0] );
        if( !tmp.data )
        {
            return cv::Size( 0, 0 );
        }
        else
        {
			int w = tmp.size().width;
			int h = tmp.size().height;

			if (m_DownSampleRate > 1)
			{
				w /= m_DownSampleRate;
				h /= m_DownSampleRate;
			}
			return cv::Size(w, h);
        }
    }
}

//=======================================================================
long VideoProcessor::GetFrameNumber()
{
    if( m_Images.size() == 0 )
    {
        // get info of from the m_Capture device
        long f = static_cast<long>( m_Capture.get( CV_CAP_PROP_POS_FRAMES ) );
        return f;
    }
    else
    {
        // if input is vector of m_Images
        return static_cast<long>( m_ItImg - m_Images.begin() );
    }
}

//=======================================================================
double VideoProcessor::GetPositionMS()
{
    // undefined for vector of m_Images
    if( m_Images.size() != 0 )
    {
        return 0.0;
    }

    double t = m_Capture.get( CV_CAP_PROP_POS_MSEC );
    return t;
}

//=======================================================================
double VideoProcessor::GetFrameRate()
{
    // undefined for vector of m_Images
    if( m_Images.size() != 0 )
    {
        return 0;
    }

    double r = m_Capture.get( CV_CAP_PROP_FPS );
    return r;
}

//=======================================================================
long VideoProcessor::GetTotalFrameCount()
{
    // for vector of m_Images
    if( m_Images.size() != 0 )
    {
        return static_cast<long>( m_Images.size() );
    }

    long t = static_cast<long>( m_Capture.get( CV_CAP_PROP_FRAME_COUNT ) );
    return t;
}

//=======================================================================
int VideoProcessor::GetCodec( char codec[4] )
{
    // undefined for vector of m_Images
    if( m_Images.size() != 0 )
    {
        return -1;
    }

    union
    {
        int value;
        char code[4];
    } returned;

    returned.value = static_cast<int>( m_Capture.get( CV_CAP_PROP_FOURCC ) );

    codec[0] = returned.code[0];
    codec[1] = returned.code[1];
    codec[2] = returned.code[2];
    codec[3] = returned.code[3];

    return returned.value;
}

//=======================================================================
bool VideoProcessor::SetFrameNumber( long pos )
{
    // for vector of m_Images
    if( m_Images.size() != 0 )
    {
		if (pos > m_Images.size())
		{
			return false;
		}
		else
		{
			// move to position in vector
			m_ItImg = m_Images.begin() + pos;
			return true;
		}
    }
    else
    {
        // if input is a m_Capture device
		//return m_Capture.set( CV_CAP_PROP_POS_FRAMES, pos );
		return true;
    }
}

//=======================================================================
bool VideoProcessor::SetPositionMS( double pos )
{
    // not defined in vector of m_Images
    if( m_Images.size() != 0 )
    {
        return false;
    }
    else
    {
        return m_Capture.set( CV_CAP_PROP_POS_MSEC, pos );
    }
}

//=======================================================================
bool VideoProcessor::SetRelativePosition( double pos )
{
    // for vector of m_Images
    if( m_Images.size() != 0 )
    {
        // move to position in vector
        long posI = static_cast<long>( pos*m_Images.size() + 0.5 );
        m_ItImg = m_Images.begin() + posI;

        // is it a valid position?
        return posI < m_Images.size();
    }
    else
    {
        // if input is a m_Capture device
        return m_Capture.set( CV_CAP_PROP_POS_AVI_RATIO, pos );
    }
}// SetRelativePosition

//=======================================================================
void VideoProcessor::Run()
{
    // current frame
    cv::Mat frame;
    // output frame
    cv::Mat output;

    // if no m_Capture device has been set
    if( !IsOpened() )
    {
        return;
    }

    m_Stop = false;

    while( !IsStopped() )
    {
		// if there's no window created, hit Esc on the command window should also exit
		if ( _kbhit() )
		{
			int key = _getch();
			if ( key == 27/*Esc*/ )
			{
				StopIt();
			}
		}

        // read next frame if any
        if( !ReadNextFrame( frame ) )
        {
            break;
        }

        // display input frame
        if( m_WindowNameInput.length() != 0 )
        {
            cv::imshow( m_WindowNameInput, frame );
        }

        // calling the m_Process function or method
        if( m_CallIt )
        {
            // m_Process the frame
            if( m_Process )
            {
                m_Process( frame, output );
            }
            else if( m_FrameProcessor )
            {
                m_FrameProcessor->Process( frame, output );
            }
        }
        else
        {
            output = frame;
        }

        m_TotalFrame++;
        //std::cout << m_TotalFrame << std::endl;//print the number for debug

        // write output sequence
        if( m_OutputFile.length() != 0 )
        {
            WriteNextFrame( output );
        }

        // display output frame
        if( m_WindowNameOutput.length() != 0 )
        {
            cv::imshow( m_WindowNameOutput, output );
        }

        // introduce a delay
        if( m_Delay >= 0 )
        {
            int ret = cv::waitKey( m_Delay );
            if( ret == 27/*ESC*/ )
            {
                StopIt();
            }
			//debug
			//if ( ret == 104 || ret == 72/*H/h, "home"*/ )
			//{
			//	m_FrameProcessor->m_Debug = true;
			//}
        }
        else
        {
            cv::waitKey( m_Delay );
        }

        // check if we should stop
        if( m_FrameToStop >= 0 && GetFrameNumber() == m_FrameToStop )
        {
            StopIt();
        }
    }
} // Run