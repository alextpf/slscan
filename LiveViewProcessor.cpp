#include "LiveViewProcessor.h"

#include <conio.h> // for getch
#include <windows.h> // for kbhit

#define FRAME_WIDTH 640
#define FRAME_HEIGHT 360

//=======================================================================
LiveViewProcessor::LiveViewProcessor()
: m_CallIt( false )
, m_Delay( -1 )
, m_DownSampleRate( 1 ) // no downsample
, m_TotalFrame( 0 )
, m_Stop( false )
, m_Digits( 0 )
, m_FrameToStop( -1 )
, m_Process( 0 )
, m_FrameProcessor( 0 )
, m_NumCam( 0 )
{}

//=======================================================================
bool LiveViewProcessor::ReadNextFrame( vector<cv::Mat>& frame )
{
	bool ok( true );
	for ( int i = 0; i < m_NumCam; i++ )
	{
		cv::Mat tmp;

		if ( m_Images[i].size() == 0 )
		{
			//////////////////////////
			// it's video or webcam
			//////////////////////////
			ok = ok && m_Capture[i].read( tmp );
		}
		else
		{
			////////////////
			// it's images
			////////////////
			if ( m_ItImg[i] != m_Images[i].end() )
			{
				//printf( "%s\n", ( *m_ItImg[i] ).c_str() ); // debug: print file path
				tmp = cv::imread( *(m_ItImg[i]) );
				m_ItImg[i]++;

				ok = ok && tmp.data != 0;
			}
		}

		if ( ok && m_DownSampleRate > 1 )
		{
			cv::resize( tmp, tmp, cv::Size(), 1.0 / m_DownSampleRate, 1.0 / m_DownSampleRate );
		}

		frame.push_back( tmp );
	}
    return ok;
}

//=======================================================================
void LiveViewProcessor::WriteNextFrame( vector<cv::Mat>& frame )
{
    if( m_Extension.length() )
    {
        ////////////////
        // it's images
        ////////////////
		for ( int i = 0; i < m_NumCam; i++ )
		{
			std::stringstream ss;
			ss << m_OutputFile[i] << std::setfill( '0' ) << std::setw( m_Digits ) << m_CurrentIndex++ << m_Extension;
			cv::imwrite( ss.str(), frame[i] );
		}
    }
}

//=======================================================================
bool LiveViewProcessor::SetInput( vector<std::string> filename /*of video*/ )
{
    m_TotalFrame = 0;

    // In case a resource was already
    // associated with the VideoCapture instance
	bool ok( true );

	for ( int i = 0; i < m_NumCam; i++ )
	{
		m_Capture[i].release();
		m_Images[i].clear();

		// Open the video file
		ok = ok && m_Capture[i].open( filename[i] );
	}

	return ok;
}

//=======================================================================
bool LiveViewProcessor::SetInput( vector<int> id /*webcam id*/)
{
    m_TotalFrame = 0;
	bool ok( true );

    // In case a resource was already
    // associated with the VideoCapture instance
	for ( int i = 0; i < m_NumCam; i++ )
	{
		m_Capture[i].release();
		m_Images[i].clear();
		m_Capture[i].set( CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH );
		m_Capture[i].set( CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT );
		ok = ok && m_Capture[i].open( id[i] );
	}

    // Open the video file
    return ok;
}

//=======================================================================
void LiveViewProcessor::SetInput( const vector<vector<std::string>>& imgs /*sets of img sequence*/ )
{
    m_TotalFrame = 0;

    for( int i = 0; i < m_NumCam; i++ )
    {
        // In case a resource was already
        // associated with the VideoCapture instance
        m_Capture[i].release();

        // the input will be this vector of m_Images
        m_Images[i] = imgs[i];
        m_ItImg[i] = m_Images[i].begin();
    }
}

//=======================================================================
bool LiveViewProcessor::SetOutput(
    const vector<std::string>& filename, // filename prefix
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
    for( int i = 0; i < m_NumCam; i++ )
    {
        m_OutputFile[i] = filename[i];
    }

    m_Extension = ext;

    // number of m_Digits in the file numbering scheme
    m_Digits = numberOfDigits;
    // start numbering at this index
    m_CurrentIndex = startIndex;

	return true;
}

//=======================================================================
void LiveViewProcessor::SetFrameProcessor( void( *frameProcessingCallback )( vector<cv::Mat>&, vector<cv::Mat>& ) )
{
    // invalidate frame processor class instance
    m_FrameProcessor = 0;
    // this is the frame processor function that will be called
    m_Process = frameProcessingCallback;
    CallProcess();
}

//=======================================================================
void LiveViewProcessor::SetFrameProcessor( FrameProcessor* frameProcessorPtr )
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
cv::Size LiveViewProcessor::GetFrameSize()
{
    if( m_Images[0].size() == 0 )
    {
        // get size of from the m_Capture device
        int w = static_cast<int>( m_Capture[0].get( CV_CAP_PROP_FRAME_WIDTH ) );
        int h = static_cast<int>( m_Capture[0].get( CV_CAP_PROP_FRAME_HEIGHT ) );

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
        cv::Mat tmp = cv::imread( m_Images[0][0] );
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
long LiveViewProcessor::GetFrameNumber()
{
    if( m_Images[0].size() == 0 )
    {
        // get info of from the m_Capture device
        long f = static_cast<long>( m_Capture[0].get( CV_CAP_PROP_POS_FRAMES ) );
        return f;
    }
    else
    {
        // if input is vector of m_Images
        return static_cast<long>( m_ItImg[0] - m_Images[0].begin() );
    }
}

//=======================================================================
double LiveViewProcessor::GetPositionMS()
{
    // undefined for vector of m_Images
    if( m_Images[0].size() != 0 )
    {
        return 0.0;
    }

    double t = m_Capture[0].get( CV_CAP_PROP_POS_MSEC );
    return t;
}

//=======================================================================
double LiveViewProcessor::GetFrameRate()
{
    // undefined for vector of m_Images
    if( m_Images[0].size() != 0 )
    {
        return 0;
    }

    double r = m_Capture[0].get( CV_CAP_PROP_FPS );
    return r;
}

//=======================================================================
long LiveViewProcessor::GetTotalFrameCount()
{
    // for vector of m_Images
    if( m_Images[0].size() != 0 )
    {
        return static_cast<long>( m_Images[0].size() );
    }

    long t = static_cast<long>( m_Capture[0].get( CV_CAP_PROP_FRAME_COUNT ) );
    return t;
}

//=======================================================================
int LiveViewProcessor::GetCodec( char codec[4] )
{
    // undefined for vector of m_Images
    if( m_Images[0].size() != 0 )
    {
        return -1;
    }

    union
    {
        int value;
        char code[4];
    } returned;

    returned.value = static_cast<int>( m_Capture[0].get( CV_CAP_PROP_FOURCC ) );

    codec[0] = returned.code[0];
    codec[1] = returned.code[1];
    codec[2] = returned.code[2];
    codec[3] = returned.code[3];

    return returned.value;
}

//=======================================================================
bool LiveViewProcessor::SetFrameNumber( long pos )
{
    bool ok( true );

    for( int i = 0; i < m_NumCam; i++ )
    {
        // for vector of m_Images
        if( m_Images[i].size() != 0 )
        {
            if( pos > m_Images[0].size() )
            {
                return false;
            }
            else
            {
                // move to position in vector
                m_ItImg[i] = m_Images[0].begin() + pos;
            }
        }
        else
        {
            // if input is a m_Capture device or video
            ok = ok && m_Capture[i].set( CV_CAP_PROP_POS_FRAMES, pos );
        }
    }

    return ok;
}

//=======================================================================
bool LiveViewProcessor::SetPositionMS( double pos /*in ms, for vid or cam*/)
{
    // not defined in vector of m_Images
    if( m_Images[0].size() != 0 )
    {
        return false;
    }
    else
    {
        for( int i = 0; i < m_NumCam; i++ )
        {
            return m_Capture[i].set( CV_CAP_PROP_POS_MSEC, pos );
        }
    }
}

//=======================================================================
bool LiveViewProcessor::SetRelativePosition( double pos /*in %*/)
{
    bool ok( true );

    for( int i = 0; i < m_NumCam; i++ )
    {
        // for vector of m_Images
        if( m_Images[i].size() != 0 )
        {
            // move to position in vector
            long posI = static_cast<long>( pos * m_Images[i].size() + 0.5 );
            m_ItImg[i] = m_Images[i].begin() + posI;

            // is it a valid position?
            ok = ok && posI < m_Images[i].size();
        }
        else
        {
            // if input is a m_Capture device
            ok = ok && m_Capture[i].set( CV_CAP_PROP_POS_AVI_RATIO, pos );
        }
    }

    return ok;
}// SetRelativePosition

//=======================================================================
void LiveViewProcessor::Run()
{
    // current frame
    vector<cv::Mat> frame;

    // output frame
    vector<cv::Mat> output;

    // if no m_Capture device has been set
    if( !IsOpened() )
    {
        return;
    }

    m_Stop = false;

    while( !IsStopped() )
    {
        // if there's no window created, hit Esc on the command window should also exit
        if( _kbhit() )
        {
            int key = _getch();
            if( key == 27/*Esc*/ )
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
        for( int i = 0; i < m_NumCam; i++ )
        {
            if( m_WindowNameInput[i].length() != 0 )
            {
                cv::imshow( m_WindowNameInput[i], frame[i] );
            }
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

        // write output sequence
        if( m_OutputFile[0].length() != 0 )
        {
            WriteNextFrame( output );
        }

        // display output frame
        for( int i = 0; i < m_NumCam; i++ )
        {
            if( m_WindowNameOutput[i].length() != 0 )
            {
                cv::imshow( m_WindowNameOutput[i], output[i] );
            }
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