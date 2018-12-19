#pragma once
#ifndef __OPENCV_STRUCTURED_LIGHT_HPP__
#define __OPENCV_STRUCTURED_LIGHT_HPP__

#include "opencv2/core.hpp"

namespace cv {
    namespace structured_light {
        //! @addtogroup structured_light
        //! @{

        //! Type of the decoding algorithm
        // other algorithms can be implemented
        enum
        {
            DECODE_3D_UNDERWORLD = 0  //!< Kyriakos Herakleous, Charalambos Poullis. "3DUNDERWORLD-SLS: An Open-Source Structured-Light Scanning System for Rapid Geometry Acquisition", arXiv preprint arXiv:1406.6595 (2014).
        };

        /** @brief Abstract base class for generating and decoding structured light patterns.
        */
        class CV_EXPORTS_W StructuredLightPattern : public virtual Algorithm
        {
        public:
            /** @brief Generates the structured light pattern to project.
            @param patternImages The generated pattern: a vector<Mat>, in which each image is a CV_8U Mat at projector's resolution.
            */
            CV_WRAP
                virtual bool generate( OutputArrayOfArrays patternImages ) = 0;

            /** @brief Decodes the structured light pattern, generating a disparity map
            @param patternImages The acquired pattern images to decode (vector<vector<Mat>>), loaded as grayscale and previously rectified.
            @param disparityMap The decoding result: a CV_64F Mat at image resolution, storing the computed disparity map.
            @param blackImages The all-black images needed for shadowMasks computation.
            @param whiteImages The all-white images needed for shadowMasks computation.
            @param flags Flags setting decoding algorithms. Default: DECODE_3D_UNDERWORLD.
            @note All the images must be at the same resolution.
            */
            CV_WRAP
                virtual bool decode( const std::vector< std::vector<Mat> >& patternImages, OutputArray disparityMap,
                    InputArrayOfArrays blackImages = noArray(),
                    InputArrayOfArrays whiteImages = noArray(),
                    int flags = DECODE_3D_UNDERWORLD ) const = 0;
        };

        //! @}

    }
}
#endif