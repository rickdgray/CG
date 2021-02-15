#include "GzFrameBuffer.h"

//Put your implementation here------------------------------------------------

void GzFrameBuffer::initFrameSize(GzInt width, GzInt height)
{
	Width = width;
	Height = height;
}

GzImage GzFrameBuffer::toImage()
{
	return GzImage();
}

void GzFrameBuffer::clear(GzFunctional buffer)
{
	if (buffer == GZ_COLOR_BUFFER)
	{
		for (int h = 0; h < Height; h++)
			for (int w = 0; w < Width; w++)
				ColorBuffer[w + (h * Width)] = ClearColor;
	}
}

void GzFrameBuffer::setClearColor(const GzColor& color)
{
	ClearColor = color;
}

void GzFrameBuffer::setClearDepth(GzReal depth)
{
}

void GzFrameBuffer::drawPoint(const GzVertex& v, const GzColor& c, GzFunctional status)
{
}

