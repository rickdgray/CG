#include "GzFrameBuffer.h"

//Put your implementation here------------------------------------------------

void GzFrameBuffer::initFrameSize(GzInt width, GzInt height)
{
	Width = width;
	Height = height;
	ColorBuffer = new GzColor[Width * Height];
}

GzImage GzFrameBuffer::toImage()
{
	return GzImage();
}

void GzFrameBuffer::clear(GzFunctional buffer)
{
	if (buffer == GZ_COLOR_BUFFER)
	{
		for (int y = 0; y < Height; y++)
			for (int x = 0; x < Width; x++)
				ColorBuffer[x + (y * Width)] = ClearColor;
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

