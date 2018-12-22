#pragma once
#include <vector>
using namespace std;

class GrayCode
{
public:
    GrayCode();

    void GeneratePattern();

    bool Decode(
        const vector<vector<cv::Mat> >& captured,
        const vector<cv::Mat>& blackImages,
        const vector<cv::Mat>& whiteImages );

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

private:

    // convert Gray code to decimal
    int GrayToDec( const std::vector<uchar>& gray ) const;

    //for a (x,y) pixel of the camera returns the corresponding projector pixel by calculating the decimal number
    bool GetProjPixel(
        const vector<cv::Mat>& captured,
        int x,
        int y,
        cv::Point& projPix );

    void GenerateShadowMask(
        const vector<cv::Mat>& blackImg,
        const vector<cv::Mat>& whiteImg );

	void SetWidth( const int w )
	{
		m_ProjectorWidth = w;
	}

	void SetHeight( const int h )
	{
		m_ProjectorHeight = h;
	}

    int					m_ProjectorWidth; // projector resolution
    int					m_ProjectorHeight; // projector resolution
	int					m_NumColImgs;
	int					m_NumRowImgs;
	int					m_NumPatternImgs;
	vector<cv::Mat>		m_Pattern;
	cv::Mat				m_BlackImage;
	cv::Mat				m_WhiteImage;
    int                 m_BlackThresh;
    int                 m_WhiteThresh;
    vector<cv::Mat>     m_ShadowMask;
    cv::Mat             m_DisparityMap;
}; // GrayCode
