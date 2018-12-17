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

	void WriteCaliResults();
	void ReadCaliResults();

	void WriteNumCaliImgs();
	void ReadNumCaliImgs();

	void SetCaptureAndCali( const bool ok )
	{
		m_CaptureAndCali = ok;
	}

	void SetDoCali( const bool ok )
	{
		m_DoCali = ok;
	}

	bool FindChessboard( const cv::Mat& img, const bool writeImg );

	void Calibrate();
	void WriteCaliImg( const cv::Mat& img );

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
	cv::Mat				m_IntrinsicMat;
	cv::Mat				m_DistCoeff;
	cv::Size			m_ImgSiz;
	bool				m_CaptureAndCali; // do calibration right after capturing all the imgs?
	bool				m_DoCali;
};//Calibrator