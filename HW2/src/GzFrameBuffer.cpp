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

void GzFrameBuffer::drawTriangle(GzVertex *vertices, GzColor *colors, GzFunctional status)
{
	double denominator = ((vertices[1][Y] - vertices[2][Y]) * (vertices[0][X] - vertices[2][X])) + ((vertices[2][X] - vertices[1][X]) * (vertices[0][Y] - vertices[2][Y]));

	double w1 = ((vertices[1][Y] - vertices[2][Y]) * (vertices[0][X] - vertices[2][X])) + ((vertices[2][X] - vertices[1][X]) * (vertices[0][Y] - vertices[2][Y]));
	if (w1 < 0)
		return;
	
	double w2 = ((vertices[2][Y] - vertices[0][Y]) * (vertices[0][X] - vertices[2][X])) + ((vertices[0][X] - vertices[2][X]) * (vertices[0][Y] - vertices[2][Y]));
	if (w2 < 0)
		return;
	
	double w3 = 1 - w1 - w2;
	if (w3 < 0)
		return;
	
	//this->drawPoint()
}