#pragma once
#include "LiveViewProcessor.h"
#include "GrayCode.h"
#include "Exporter.h"

class Calibrator : public LiveViewProcessor
{
public:
	Calibrator::Calibrator();//ctor

	void CaptureAndClibrate( const bool captureRoi );

	void LiveView();

    void Scan( const bool captureRoi );

	void Init()
	{
		LiveViewProcessor::Init();

		m_InputFileName.clear();
		m_OutputFileName.clear();
		m_ImagePoints.clear();
		SetDoCali( false );
		m_ROICorners.clear();

	}

	static void OnMouse( int event, int x, int y, int f, void* data );

    void Generate3D( const bool debug );

    int GetNumCaliImgs() const
    {
        return m_NumCaliImgs;
    }

	void SetNumCaliImgs( int s )
	{
		m_NumCaliImgs = s;
	}

    void SetCaliPatternWidth( const int w )
    {
		m_CalibPatternWidth = w;
    }

    void SetCaliPatternHeight( const int h )
    {
		m_CalibPatternHeight = h;
    }

	void SetBlockSize( const float s )
	{
		m_BlockSize = s;
	}

	void SetInputFileName( vector<string> s )
	{
		m_InputFileName = s;
	}

	void SetOutputFileName( vector<string> s )
	{
		m_OutputFileName = s;
	}

	void SetPath( string s )
	{
		m_Path = s;
        m_GrayCode.SetPath( s );
	}

	void WriteSingleCamCaliResults( const double err );
	void ReadSingleCamCaliResults(
		const string path,
		cv::Mat& intrinsicMat,
		cv::Mat& distCoeff );

	void WriteStereoCamCaliResults( const double err );
	void ReadStereoCamCaliResults(
		const string path,
		cv::Mat& R,
		cv::Mat& T,
		cv::Mat& E,
		cv::Mat& F );

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
	void WriteImg(
        const string& fileName,
        const cv::Mat& img,
        const int idx );

	void WriteCaliWithCirclesImg( const string& fileName, const cv::Mat& img );
	void SetProjectorDimension( const int w, const int h )
	{
		m_GrayCode.ComputeNumPatternImgs( w, h );
	}

	void SetProjWinName( const string s )
	{
		m_ProjWinName = s;
	}

	int ReadNumPatterns();

	bool GeneratePattern()
	{
		return m_GrayCode.GeneratePattern();
	}

	void WriteRoi( const vector<vector<cv::Point>>& corners )
	{
		// write Roi
		std::stringstream roiPath;
		roiPath << m_Path << "Roi.xml";

		cv::FileStorage fs( roiPath.str(), cv::FileStorage::WRITE );
		fs << "leftCamCorner1" << corners[0][0]
			<< "leftCamCorner2" << corners[0][1]
			<< "rightCamCorner1" << corners[1][0]
			<< "rightCamCorner2" << corners[1][1];

		fs.release();
	}

	void ReadRoi()
	{
		m_ROICorners.clear();

		std::stringstream roiPath;
		roiPath << m_Path << "Roi.xml";

		cv::FileStorage fs;
		fs.open( roiPath.str(), cv::FileStorage::READ );

		vector<cv::Point> pts;
		cv::Point pt;

		fs["leftCamCorner1"] >> pt;
		pts.push_back( pt );

		fs["leftCamCorner2"] >> pt;
		pts.push_back( pt );

		m_ROICorners.push_back( pts );
		pts.clear();

		fs["rightCamCorner1"] >> pt;
		pts.push_back( pt );

		fs["rightCamCorner2"] >> pt;
		pts.push_back( pt );

		m_ROICorners.push_back( pts );

		fs.release();

		m_GrayCode.SetCorners( m_ROICorners );
	}

	void SetColImgOnly( const bool ok )
	{
		m_GrayCode.SetColImgOnly( ok );
	}

private:
	void ReprojectImageTo3D(
		const cv::Mat& colorImg,
		const cv::Mat& disp /*CV_32S*/,
		const cv::Mat& Q,
		vector<cv::Point3d>& pointcloud,
		vector<cv::Point3i>& colors,
        vector<cv::Vec2f>& textureCoord );

	void DisplayAndSaveComposite(
		const string& rectifyWinName,
		const cv::Mat& tmpLeft,
		const cv::Mat& tempRight );
	void WriteNumPatterns();

    void DisplayFrame(
		const vector<string>& winName,
		const vector<cv::Mat>& output,
		const bool captureRoi = false );

	bool CaptureOptions( vector<cv::Mat>& frame, vector<cv::Mat>& output );

	int					m_NumCaliImgs;
	int					m_CalibPatternWidth; // number of column of inner corners
	int					m_CalibPatternHeight; // number of row of inner corners
	float				m_BlockSize; // physical size of a chessboard block, in mm
    vector<string>		m_InputFileName;
	vector<string>		m_OutputFileName;
	string				m_Path;
	vector<vector<vector<cv::Point2f>>>	    m_ImagePoints; // chessboard corners in 2D img coord; #caliImgs of #source of #corerns in a img
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
	string				m_ProjWinName;
	vector<vector<cv::Point>>	m_ROICorners; // each img has 2 corners
};//Calibrator