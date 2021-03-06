#pragma once
#include <vector>
#include<map>
using namespace std;

class GrayCode
{
public:
    GrayCode();

    bool GeneratePattern();

    bool DecodeTwoDir(
        const vector<vector<cv::Mat> >& captured,
		const vector<cv::Mat>& whiteImages,
		const vector<cv::Mat>& blackImages,
        const bool debug );

	bool DecodeColImgOnly(
		const vector<vector<cv::Mat> >& captured,
		const vector<cv::Mat>& whiteImages,
		const vector<cv::Mat>& blackImages,
		const bool debug );

	void ComputeNumPatternImgs(const int w, const int h );

	int GetWidth()
	{
		return m_ProjectorWidth;
	}

	int GetHeight()
	{
		return m_ProjectorHeight;
	}

	const vector<cv::Mat>& GetPattern()
	{
		return m_Pattern;
	}

    void SetWhiteThresh( const int w )
    {
        m_WhiteThresh = w;
    }

    void SetBlackThresh( const int b )
    {
        m_BlackThresh = b;
    }

	int GetNumPatternImgs()
	{
		return m_NumPatternImgs;
	}

	cv::Mat GetDisparityMap()
	{
		return m_DisparityMap;
	}

    void SetPath( const string path )
    {
        m_Path = path;
    }

	void SetCorners( const vector<vector<cv::Point>>& roiCorners )
	{
		m_ROICorners = roiCorners;
	}

	bool GetColImgOnly()
	{
		return m_ColImgOnly;
	}

	void SetColImgOnly( const bool ok )
	{
		m_ColImgOnly = ok;
	}

private:
	bool IsInsideROI( const int r, const int c, const vector<cv::Point>& pts );

    // convert Gray code to decimal
    int GrayToDec( const std::vector<uchar>& gray ) const;

    //for a (r,c) pixel of the camera returns the corresponding projector pixel by calculating the decimal number
    bool GetProjPixel(
        const vector<cv::Mat>& captured,
        const int r,
		const int c,
		int& rowDec, // out
		int& colDec ); // out

	//for a (r,c) pixel of the camera returns the corresponding projector pixel by calculating the decimal number
	bool GetProjPixelColImgOnly(
		const vector<cv::Mat>& captured,
		const int r,
		const int c,
		int& colDec ); // out

    void GenerateShadowMask(
		const vector<cv::Mat>& whiteImg,
		const vector<cv::Mat>& blackImg,
        const bool debug );

	void SetProjectorWidth( const int w ) // called in ComputeNumPatternImgs
	{
		m_ProjectorWidth = w;
	}

	void SetProjectorHeight( const int h ) // called in ComputeNumPatternImgs
	{
		m_ProjectorHeight = h;
	}

	int RowColToIdx( const int r, const int c, const int w )
	{
		return r * w + c;
	}

	void IdxToRowCol( const int idx, int& row, int& col , const int w )
	{
		col = idx % w;
		row = idx / w;
	}

    bool FindCorrespondance(
        const vector<cv::Mat>& whiteImages,
        const std::map< int, int >& leftCam,
        const std::multimap< int, int >& rightCam
    );

    bool FindCorrespondanceDebug(
        const vector<cv::Mat>& whiteImages,
        const std::map< int, int >& leftCam,
        const std::multimap< int, int >& rightCam
    );

	bool FindCorrespondanceColImgOnly(
		const vector<cv::Mat>& whiteImages,
		const vector<std::map< int, int >>& leftCam,
		const vector<std::multimap< int, int >>& rightCam
	);

	bool FindCorrespondanceDebugColImgOnly(
		const vector<cv::Mat>& whiteImages,
		const vector<std::map< int, int >>& leftCam,
		const vector<std::multimap< int, int >>& rightCam
	);

    static void OnMouse( int event, int x, int y, int f, void* data );

    int					m_ProjectorWidth; // projector resolution
    int					m_ProjectorHeight; // projector resolution
	int					m_ImgWidth; // being set in GenerateShadowMask
	int					m_ImgHeight;
	int					m_NumColImgs;
	int					m_NumRowImgs;
	int					m_NumPatternImgs;
	vector<cv::Mat>		m_Pattern;
	cv::Mat				m_BlackImage;
	cv::Mat				m_WhiteImage;
    int                 m_BlackThresh;
    int                 m_WhiteThresh;
    vector<cv::Mat>     m_ShadowMask; // 0: is shadow; 0: otherwise
    cv::Mat             m_DisparityMap;
    string              m_Path;
	vector<vector<cv::Point>>	m_ROICorners; // each img has 2 corners
	bool				m_ColImgOnly;
}; // GrayCode
