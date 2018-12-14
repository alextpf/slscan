#include "Calibrator.h"

//=======================================================================
Calibrator::Calibrator()
	: LiveViewProcessor()
	, m_NumCaliImgs(0)
{}

//=======================================================================
void Calibrator::Run()
{
	// current frame
	vector<cv::Mat> frame;

	// output frame
	vector<cv::Mat> output;

	// if no m_Capture device has been set
	if ( !IsOpened() )
	{
		return;
	}

    for( int i = 0; i < m_NumSource; i++ )
    {
        frame.push_back( cv::Mat() );
        output.push_back( cv::Mat() );
    }

	m_Stop = false;

	while ( !IsStopped() )
	{
		// read next frame if any
		if ( !ReadNextFrame( frame ) )
		{
			break;
		}

		// display input frame
        if( m_WindowNameInput.size() > 0 )
        {
            for( int i = 0; i < m_NumSource; i++ )
            {
                cv::imshow( m_WindowNameInput[i], frame[i] );
            }
		}

		// check whether capture or start calibration
		CaptureOptions( frame, output );

		output = frame;

		// display output frame
        if( m_WindowNameOutput.size() > 0 )
        {
            for( int i = 0; i < m_NumSource; i++ )
            {
                cv::imshow( m_WindowNameOutput[i], output[i] );
            }
		}

		// check if we should stop
		if ( m_FrameToStop >= 0 && GetFrameNumber() == m_FrameToStop )
		{
			StopIt();
		}
	}
} // Run

//=======================================================================
void Calibrator::CaptureOptions( vector<cv::Mat>& frame, vector<cv::Mat>& output )
{
	int ret = cv::waitKey( m_Delay );
	if ( ret == 27/*ESC*/ )
	{
		StopIt();
	}

	if ( ret == 67/*'C'*/ || ret == 99 /*'c'*/ )
	{
		// capture, and find chessboard corners
		/*cv::Mat in = frame[0];
		cv::Mat out = output[0];
		m_ChessboardFinder.Process( in, out );

        // show the corners on output window
        cv::imshow( m_WindowNameOutput[0], out );
        */
		bool needAnswer( true );
		while ( needAnswer )
		{
			int ans = cv::waitKey( 0 ); // wait indefinitely for an answer: "a" (accept) or "r" (reject)

			if ( ans == 65/*'A'*/ || ans == 97/*'a'*/ )
			{
				// we take it, now store the parameters and image
				m_NumCaliImgs++;
				needAnswer = false;
			}
			else if ( ans == 82/*'R'*/ || ans == 114/*'r'*/ )
			{
				needAnswer = false;
			}
		}
	}
	else if ( ret == 27/*ESC*/ )
	{
		// use all the captured imgs for calibration
		StopIt();
	}
} // CaptureOptions