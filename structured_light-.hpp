/*#ifdef __OPENCV_BUILD
#error this is a compatibility header which should not be used inside the OpenCV library
#endif*/

#include "opencv2/structured_light/structured_light.hpp"
#include "opencv2/structured_light/graycodepattern.hpp"
#include "opencv2/structured_light/sinusoidalpattern.hpp"

/** @defgroup structured_light Structured Light API
Structured light is considered one of the most effective techniques to acquire 3D models.
This technique is based on projecting a light pattern and capturing the illuminated scene
from one or more points of view. Since the pattern is coded, correspondences between image
points and points of the projected pattern can be quickly found and 3D information easily
retrieved.
One of the most commonly exploited coding strategies is based on trmatime-multiplexing. In this
case, a set of patterns  are successively projected onto the measuring surface.
The codeword for a given pixel is usually formed by  the sequence of illuminance values for that
pixel across the projected patterns. Thus, the codification is called  temporal because the bits
of the codewords are multiplexed in time @cite pattern .
In this module a time-multiplexing coding strategy based on Gray encoding is implemented following the
(stereo) approach described in 3DUNDERWORLD algorithm @cite UNDERWORLD .
For more details, see @ref tutorial_structured_light.
*/