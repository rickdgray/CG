#include "GzFrameBuffer.h"

//Put your implementation here------------------------------------------------
void GzFrameBuffer::initFrameSize(GzInt width, GzInt height) {
	image.resize(width, height);
	depthBuffer=vector<vector<GzReal> >(width, vector<GzReal>(height, clearDepth));
}

GzImage GzFrameBuffer::toImage() {
	return image;
}

void GzFrameBuffer::clear(GzFunctional buffer) {
	if (buffer&GZ_COLOR_BUFFER) image.clear(clearColor);
	if (buffer&GZ_DEPTH_BUFFER)
		for (GzInt x=0; x!=depthBuffer.size(); x++) 
			fill(depthBuffer[x].begin(), depthBuffer[x].end(), clearDepth);
}

void GzFrameBuffer::setClearColor(const GzColor& color) {
	clearColor=color;
}

void GzFrameBuffer::setClearDepth(GzReal depth) {
	clearDepth=depth;
}

void GzFrameBuffer::drawPoint(const GzVertex& v, const GzColor& c, GzFunctional status) {
	GzInt x=(GzInt)v[X];
	GzInt y=image.sizeH()-(GzInt)v[Y]-1;
	if ((x<0)||(y<0)||(x>=image.sizeW())||(y>=image.sizeH())) return;
	if (status&GZ_DEPTH_TEST) {
		if (v[Z]>=depthBuffer[x][y]) {
			image.set(x, y, c);
			depthBuffer[x][y]=v[Z];
		}
	} else {
		image.set(x, y, c);
		depthBuffer[x][y]=v[Z];
	}
}

void GzFrameBuffer::drawTriangle(vector<GzVertex>& v, vector<GzColor>& c, GzFunctional status) {
	GzInt yMin, yMax;
	GzReal xMin, xMax, zMin, zMax;
	GzColor cMin, cMax;

	v.push_back(v[0]);
	c.push_back(c[0]);

	yMin=INT_MAX;
	yMax=-INT_MAX;

	for (GzInt i=0; i<3; i++) {
		yMin=min((GzInt)floor(v[i][Y]), yMin);
		yMax=max((GzInt)floor(v[i][Y]-1e-3), yMax);
	}
		
	for (GzInt y=yMin; y<=yMax; y++) {
		xMin=INT_MAX;
		xMax=-INT_MAX;
		for (GzInt i=0; i<3; i++) {
			if ((GzInt)floor(v[i][Y])==y) {
				if (v[i][X]<xMin) {
					xMin=v[i][X];
					zMin=v[i][Z];
					cMin=c[i];
				}
				if (v[i][X]>xMax) {
					xMax=v[i][X];
					zMax=v[i][Z];
					cMax=c[i];
				}
			}
			if ((y-v[i][Y])*(y-v[i+1][Y])<0) {
				GzReal x;
				realInterpolate(v[i][Y], v[i][X], v[i+1][Y], v[i+1][X], y, x);
				if (x<xMin) {
					xMin=x;
					realInterpolate(v[i][Y], v[i][Z], v[i+1][Y], v[i+1][Z], y, zMin);
					colorInterpolate(v[i][Y], c[i], v[i+1][Y], c[i+1], y, cMin);
				}
				if (x>xMax) {
					xMax=x;
					realInterpolate(v[i][Y], v[i][Z], v[i+1][Y], v[i+1][Z], y, zMax);
					colorInterpolate(v[i][Y], c[i], v[i+1][Y], c[i+1], y, cMax);
				}
			}
		}
		drawRasLine(y, xMin, zMin, cMin, xMax-1e-3, zMax, cMax, status);
	}
}

void GzFrameBuffer::drawRasLine(GzInt y, GzReal xMin, GzReal zMin, GzColor& cMin, GzReal xMax, GzReal zMax, GzColor& cMax, GzFunctional status) {
	if ((y<0)||(y>=image.sizeH())) return;
	if ((GzInt)floor(xMin)==(GzInt)floor(xMax)) {
		if (zMin>zMax) drawPoint(GzVertex(floor(xMin), y, zMin), cMin, status);
			else drawPoint(GzVertex(floor(xMin), y, zMax), cMax, status);
	} else {
		GzReal z;
		GzColor c;
		y=image.sizeH()-y-1;
		int w=image.sizeW();
		if (status&GZ_DEPTH_TEST) {
			for (int x=max(0, (GzInt)floor(xMin)); x<=min(w-1, (GzInt)floor(xMax)); x++) {
				realInterpolate(xMin, zMin, xMax, zMax, x, z);
				if (z>=depthBuffer[x][y]) {
					colorInterpolate(xMin, cMin, xMax, cMax, x, c);
					image.set(x, y, c);
					depthBuffer[x][y]=z;
				}
			}
		} else {
			for (int x=max(0, (GzInt)floor(xMin)); x<=min(w-1, (GzInt)floor(xMax)); x++) {
				realInterpolate(xMin, zMin, xMax, zMax, x, z);
				colorInterpolate(xMin, cMin, xMax, cMax, x, c);
				image.set(x, y, c);
				depthBuffer[x][y]=z;
			}
		}
	}
}

void GzFrameBuffer::realInterpolate(GzReal key1, GzReal val1, GzReal key2, GzReal val2, GzReal key, GzReal& val) {
	val=val1+(val2-val1)*(key-key1)/(key2-key1);
}

void GzFrameBuffer::colorInterpolate(GzReal key1, GzColor& val1, GzReal key2, GzColor& val2, GzReal key, GzColor& val) {
	GzReal k=(key-key1)/(key2-key1);
	for (GzInt i=0; i<4; i++) val[i]=val1[i]+(val2[i]-val1[i])*k;
}

void GzFrameBuffer::normalInterpolate(GzReal key1, GzVector& val1, GzReal key2, GzVector& val2, GzReal key, GzVector& val)
{
    GzReal k=(key-key1)/(key2-key1);

    for (GzInt i = 0; i < 3; i++)
        val[i]=val1[i]+(val2[i]-val1[i])*k;

    val.normalize();
}

void GzFrameBuffer::shadeModel(const GzInt model) {
	curShadeModel=model;
}

void GzFrameBuffer::material(GzReal _kA, GzReal _kD, GzReal _kS, GzReal _s) {
	kA=_kA;
	kD=_kD;
	kS=_kS;
	s=_s;
}

void GzFrameBuffer::addLight(const GzVector& v, const GzColor& c) {
	lights.push_back(make_pair(v, c));
	lightTrans.push_back(make_pair(v, c));
}

void GzFrameBuffer::loadLightTrans(GzMatrix transMatrix)
{
	GzMatrix mat;
	mat.resize(3, 3);

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++ )
			mat[i][j] = transMatrix[i][j];

	mat = mat.inverse3x3().transpose();

	for (int i = 0; i < lights.size(); i++)
	{
		GzVector transedVector;
		for (int j = 0; j < 3; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				transedVector[j] += mat[j][k] * lights[i].first[k];
			}
		}

		lightTrans[i].first = transedVector;
	}
}

void GzFrameBuffer::drawPoint(const GzVertex& v, const GzVector& n, const GzVector& e, const GzColor& c, GzFunctional status)
{
	GzInt x = round(v[X]);
	if (x < 0 || x >= image.sizeW())
		return;

	GzInt y = round(v[Y]);
	if (y < 0 || y >= image.sizeH())
		return;

	if (status & GZ_DEPTH_TEST)
	{
		GzReal z = v[Z];
		if (z >= depthBuffer[x][y])
		{
			image.set(x, y, shade(n, e, c));
			depthBuffer[x][y] = v[Z];
		}

		return;
	}
	
	image.set(x, y, shade(n, e, c));
}

void GzFrameBuffer::drawTriangle(vector<GzVertex>& v, vector<GzVector>& n, const GzVector& e, vector<GzColor>& c, GzFunctional status)
{
	if (curShadeModel == GZ_GOURAUD)
	{
		vector<GzColor> shadedColors(3);
        for (int i = 0; i < 3; i++)
        {
            shadedColors[i] = shade(n[i], e, c[i]);
        }

        drawTriangle(v, shadedColors, status);
	}
	else if (curShadeModel == GZ_PHONG)
	{
		GzInt yMin, yMax;
		GzReal xMin, xMax, zMin, zMax;
		GzColor cMin, cMax;
		GzVector nMin, nMax;

		v.push_back(v[0]);
		c.push_back(c[0]);
		n.push_back(n[0]);

		yMin=INT_MAX;
		yMax=-INT_MAX;

		for (GzInt i=0; i<3; i++) {
			yMin=min((GzInt)floor(v[i][Y]), yMin);
			yMax=max((GzInt)floor(v[i][Y]-1e-3), yMax);
		}

		for (GzInt y=yMin; y<=yMax; y++) {
			xMin=INT_MAX;
			xMax=-INT_MAX;
			for (GzInt i=0; i<3; i++) {
				if ((GzInt)floor(v[i][Y])==y) {
					if (v[i][X]<xMin) {
						xMin=v[i][X];
						zMin=v[i][Z];
						cMin=c[i];
						nMin=n[i];
					}
					if (v[i][X]>xMax) {
						xMax=v[i][X];
						zMax=v[i][Z];
						cMax=c[i];
						nMax=n[i];
					}
				}
				if ((y-v[i][Y])*(y-v[i+1][Y])<0) {
					GzReal x;
					realInterpolate(v[i][Y], v[i][X], v[i+1][Y], v[i+1][X], y, x);
					if (x<xMin) {
						xMin=x;
						realInterpolate(v[i][Y], v[i][Z], v[i+1][Y], v[i+1][Z], y, zMin);
						colorInterpolate(v[i][Y], c[i], v[i+1][Y], c[i+1], y, cMin);
						normalInterpolate(v[i][Y], n[i], v[i+1][Y], n[i+1], y, nMin);
					}
					if (x>xMax) {
						xMax=x;
						realInterpolate(v[i][Y], v[i][Z], v[i+1][Y], v[i+1][Z], y, zMax);
						colorInterpolate(v[i][Y], c[i], v[i+1][Y], c[i+1], y, cMax);
						normalInterpolate(v[i][Y], n[i], v[i+1][Y], n[i+1], y, nMax);
					}
				}
			}

			drawRasLine(y, xMin, zMin, cMin, nMin, xMax-1e-3, zMax, cMax, nMax, e, status);
		}
	}
}

GzColor GzFrameBuffer::shade(const GzVector& n, const GzVector& e, const GzColor& c)
{
	//ambient light
	GzColor shadedColor;
	for (int i = 0; i < 4; i++)
	{
		shadedColor[i] = max(kA * c[i], 0.0);
	}

	for (int i = 0; i < lights.size(); i++)
	{
		GzVector lightDirection = lights[i].first;
		GzColor lightColor = lights[i].second;

		//normalized light direction
		GzVector L = GzVector() - lightDirection;
		L.normalize();

		//lambertian reflectance
		for (int j = 0; j < 4; j++)
		{
			shadedColor[j] += max(kD * lightColor[j] * dotProduct(n, L), 0.0);
		}

		//view vector
		GzVector V = e;
		V.normalize();

		//halfway vector
		GzVector H = L + V;
		H.normalize();

		//normal vector
		GzVector N = n;

		//blinn-phong specular highlights
		for (int j = 0; j < 4; j++)
		{
			shadedColor[j] += max(kS * lightColor[j] * pow(max(dotProduct(n, H), 0.0), s), 0.0);
		}

		//check shaded color bounds
		for (int j = 0; j < 4; j++)
		{
			shadedColor[j] = min(shadedColor[j], 1.0);
		}
	}

	return shadedColor;
}

void GzFrameBuffer::drawRasLine(GzInt y, GzReal xMin, GzReal zMin, GzColor& cMin, GzVector& nMin, GzReal xMax, GzReal zMax, GzColor& cMax, GzVector& nMax, const GzVector& e, GzFunctional status)
{
	if ((y<0)||(y>=image.sizeH())) return;
	if ((GzInt)floor(xMin)==(GzInt)floor(xMax)) {
		if (zMin>zMax) drawPoint(GzVertex(floor(xMin), y, zMin), nMin, e, cMin, status);
			else drawPoint(GzVertex(floor(xMin), y, zMax), nMax, e, cMax, status);
	} else {
		GzReal z;
		GzVector n;
		GzColor c;
		y=image.sizeH()-y-1;
		int w=image.sizeW();
		if (status&GZ_DEPTH_TEST) {
			for (int x=max(0, (GzInt)floor(xMin)); x<=min(w-1, (GzInt)floor(xMax)); x++) {
				realInterpolate(xMin, zMin, xMax, zMax, x, z);
				if (z>=depthBuffer[x][y]) {
					colorInterpolate(xMin, cMin, xMax, cMax, x, c);
					normalInterpolate(xMin, nMin, xMax, nMax, x, n);
					image.set(x, y, shade(n, e, c));
					depthBuffer[x][y]=z;
				}
			}
		} else {
			for (int x=max(0, (GzInt)floor(xMin)); x<=min(w-1, (GzInt)floor(xMax)); x++) {
				realInterpolate(xMin, zMin, xMax, zMax, x, z);
				colorInterpolate(xMin, cMin, xMax, cMax, x, c);
				normalInterpolate(xMin, nMin, xMax, nMax, x, n);
				image.set(x, y, shade(n, e, c));
				depthBuffer[x][y]=z;
			}
		}
	}
}