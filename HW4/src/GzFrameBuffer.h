#ifndef __GZ_FRAME_BUFFER_H_
#define __GZ_FRAME_BUFFER_H_

#include "GzCommon.h"
#include "GzImage.h"
#include "GzVector.h"
#include "GzMatrix.h"
#include <vector>
#include <cmath>
#include <utility>
#include <algorithm>

using namespace std;

//Frame buffer with Z-buffer -------------------------------------------------
class GzFrameBuffer {
public:

//============================================================================
//Declarations in Assignment #1, 2, 3
//============================================================================
	//The common interface
	void initFrameSize(GzInt width, GzInt height);
	GzImage toImage();

	void clear(GzFunctional buffer);
	void setClearColor(const GzColor& color);
	void setClearDepth(GzReal depth);

	void drawPoint(const GzVertex& v, const GzColor& c, GzFunctional status);
	void drawTriangle(vector<GzVertex>& v, vector<GzColor>& c, GzFunctional status);

private:
	//Put any variables and private functions for your implementation here
	GzImage image;
	vector<vector<GzReal> > depthBuffer;
	GzColor clearColor;
	GzReal clearDepth;

	void realInterpolate(GzReal key1, GzReal val1, GzReal key2, GzReal val2, GzReal key, GzReal& val);
	void colorInterpolate(GzReal key1, GzColor& val1, GzReal key2, GzColor& val2, GzReal key, GzColor& val);
	void drawRasLine(GzInt y, GzReal xMin, GzReal zMin, GzColor& cMin, GzReal xMax, GzReal zMax, GzColor& cMax, GzFunctional status);

//============================================================================
//End of Declarations in Assignment #1, 2, 3
//============================================================================



//============================================================================
//Declarations in Assignment #4
//============================================================================
public:
	void shadeModel(const GzInt model);						//Set the current shade model (curShadeModel)
	void addLight(const GzVector& v, const GzColor& c);		//Add a light source at position p with color c
	void material(GzReal _kA, GzReal _kD, GzReal _kS, GzReal _s);		//Specify the meterial of the object, includes:
																			//	_kA: The ambient coefficients
																			//	_kD: The diffuse coefficients
																			//	_kS: The specular coefficients
																			//	_s: The spec power
	void loadLightTrans(GzMatrix transMatrix);

	void drawPoint(const GzVertex& v, const GzVector& n, const GzColor& c, GzFunctional status);
	void drawTriangle(vector<GzVertex>& v, vector<GzVector>& n, vector<GzColor>& c, GzFunctional status);
private:
	GzInt curShadeModel;
	GzReal kA, kD, kS, s;

	vector<pair<GzVector, GzColor>> lights;
	vector<pair<GzVector, GzColor>> lightTrans;

	GzColor shade(const GzVector& n, const GzColor& c);
//============================================================================
//End of Declarations in Assignment #4
//============================================================================



};
//----------------------------------------------------------------------------

#endif
