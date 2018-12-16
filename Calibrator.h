#pragma once
#include "LiveViewProcessor.h"

class Calibrator : public LiveViewProcessor
{
public:
	Calibrator::Calibrator();//ctor

	void Run() override;

    int GetNumCaliImgs() const
    {
        return m_NumCaliImgs;
    }

    void SetWidth( const int w )
    {
		m_Width = w;
    }

    void SetHeight( const int h )
    {
		m_Height = h;
    }

	void SetBlockSize( const float s )
	{
		m_BlockSize = s;
	}

	void SetFileName( string s )
	{
		m_FileName = s;
	}

private:
	void CaptureOptions( vector<cv::Mat>& frame, vector<cv::Mat>& output );

	int					m_NumCaliImgs;
	vector<cv::Point2f> m_Corners;
	int					m_Width; // number of column
	int					m_Height; // number of row
	float				m_BlockSize; // physical size of a chessboard block, in mm
	string				m_FileName;
	vector<vector<cv::Point2f> >	m_ImagePoints; // chessboard corners in 2D img coord
	vector<vector<cv::Point3f> >	m_ObjectPoints; // chessboard corners in 3D obj coord
};//Calibrator