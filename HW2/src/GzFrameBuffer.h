#ifndef __GZ_FRAME_BUFFER_H_
#define __GZ_FRAME_BUFFER_H_

#include "GzCommon.h"
#include "GzImage.h"
#include <vector>
#include <cmath>
using namespace std;

//Frame buffer with Z-buffer -------------------------------------------------
class GzFrameBuffer {
public:
	//The common interface
	void initFrameSize(GzInt width, GzInt height);
	GzImage toImage();

	void clear(GzFunctional buffer);
	void setClearColor(const GzColor& color);
	void setClearDepth(GzReal depth);

	void drawPoint(const GzVertex& v, const GzColor& c, GzFunctional status);

	void drawTriangle(GzVertex *vertices, GzColor *colors, GzFunctional status);

private:
	//Put any variables and private functions for your implementation here

	GzInt Width;
	GzInt Height;

	GzColor ClearColor;
	GzColor* ColorBuffer;

	GzReal ViewDistance;
	GzReal* DepthBuffer;
};
//----------------------------------------------------------------------------

#endif
