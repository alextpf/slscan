#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/structured_light.hpp>

#include "GrayCode.h"

//======================================
GrayCode::GrayCode()
    : m_Width( 0 )
    , m_Height( 0 )
{}

//======================================
void GrayCode::GeneratePattern()
{
    cv::structured_light::GrayCodePattern::Params params;
}// GeneratePattern
