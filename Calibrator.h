#pragma once
#include "LiveViewProcessor.h"
#include "GrayCode.h"

class Calibrator : public LiveViewProcessor
{
public:
	Calibrator::Calibrator();//ctor

	void CaptureAndClibrate();

    void Scan();

    int GetNumCaliImgs() const
    {
        return m_NumCaliImgs;
    }

	void SetNumCaliImgs( int s )
	{
		m_NumCaliImgs = s;
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

	void SetFileName( vector<string> s )
	{
		m_FileName = s;
	}

	void SetPath( string s )
	{
		m_Path = s;
	}

	void WriteSingleCamCaliResults();
	void ReadSingleCamCaliResults(
		const string path,
		cv::Mat& intrinsicMat,
		cv::Mat& distCoeff );

	void WriteStereoCamCaliResults();

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

	bool FindChessboard( const vector<cv::Mat>& imgs, const bool writeImg );

	void Calibrate();
	void WriteCaliImg( const string& fileName, const cv::Mat& img );
	void WriteCaliWithCirclesImg( const string& fileName, const cv::Mat& img );

    void SetProjectorWidth( const int w )
    {
        m_GrayCode.SetWidth( w );
    }

    void SetProjectorHeight( const int h )
    {
        m_GrayCode.SetHeight( h );
    }

private:
	void CaptureOptions( vector<cv::Mat>& frame, vector<cv::Mat>& output );

	int					m_NumCaliImgs;
	int					m_Width; // number of column of inner corners
	int					m_Height; // number of row of inner corners
	float				m_BlockSize; // physical size of a chessboard block, in mm
    vector<string>		m_FileName;
	string				m_Path;
	vector<vector<vector<cv::Point2f>>>	    m_ImagePoints; // chessboard corners in 2D img coord; #caliImgs of #source of #corerns in a img
	vector<vector<cv::Point3f>>	            m_ObjectPoints; // chessboard corners in 3D obj coord
	cv::Mat				m_IntrinsicMat;
	cv::Mat				m_DistCoeff;
	cv::Size			m_ImgSiz;
	bool				m_CaptureAndCali; // do calibration right after capturing all the imgs?
	bool				m_DoCali;
	cv::Mat				m_R; // rotation between 2 cams
	cv::Mat				m_T; // translation between 2 cams
	cv::Mat				m_E; // essential matrix
	cv::Mat				m_F; // fundamental matrix
    GrayCode            m_GrayCode;
};//Calibrator