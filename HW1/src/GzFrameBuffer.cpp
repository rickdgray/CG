#include <math.h>
#include "GzFrameBuffer.h"

//Put your implementation here------------------------------------------------

void GzFrameBuffer::initFrameSize(GzInt width, GzInt height)
{
	Width = width;
	Height = height;
	ColorBuffer = new GzColor[Width * Height];
	DepthBuffer = new GzReal[Width * Height];
}

GzImage GzFrameBuffer::toImage()
{
	GzImage image(Width, Height);

	for (GzInt y = 0; y < Height; y++)
		for (GzInt x = 0; x < Width; x++)
			image.set(x, y, ColorBuffer[x + (y * Width)]);

	return image;
}

void GzFrameBuffer::clear(GzFunctional buffer)
{
	if (buffer & GZ_COLOR_BUFFER)
	{
		for (GzInt y = 0; y < Height; y++)
			for (GzInt x = 0; x < Width; x++)
				ColorBuffer[x + (y * Width)] = ClearColor;
	}

	if (buffer & GZ_DEPTH_BUFFER)
	{
		for (GzInt y = 0; y < Height; y++)
			for (GzInt x = 0; x < Width; x++)
				DepthBuffer[x + (y * Width)] = ViewDistance;
	}
}

void GzFrameBuffer::setClearColor(const GzColor& color)
{
	ClearColor = color;
}

void GzFrameBuffer::setClearDepth(GzReal depth)
{
	ViewDistance = depth;
}

void GzFrameBuffer::drawPoint(const GzVertex& v, const GzColor& c, GzFunctional status)
{
	GzInt x = round(v[0]);
	if (x < 0 || x >= Width)
		return;

	GzInt y = round(v[1]);
	if (y < 0 || y >= Height)
		return;

	if (status & GZ_DEPTH_TEST)
	{
		GzReal z = v[2];
		if (z >= DepthBuffer[x + (y * Width)])
		{
			DepthBuffer[x + (y * Width)] = z;
			ColorBuffer[x + (y * Width)] = c;
		}

		return;
	}
	
	ColorBuffer[x + y * Width] = c;
}
