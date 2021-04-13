#include "GzFrameBuffer.h"

//Put your implementation here------------------------------------------------
#include <climits>
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

void GzFrameBuffer::shadeModel(const GzInt model) {
	curShadeModel=model;
}

void GzFrameBuffer::material(GzReal _kA, GzReal _kD, GzReal _kS, GzReal _s) {
	kA=_kA;
	kD=_kD;
	kS=_kS;
	s=_s;
}

void GzFrameBuffer::loadLightTrans(GzMatrix& mat) {
	GzMatrix rotMat;
	rotMat.resize(3, 3);
	for (int i=0; i<3; i++)
		for (int j=0; j<3; j++) rotMat[i][j]=mat[i][j];
	lightTrans=(rotMat.inverse3x3()).transpose();
}
	
void GzFrameBuffer::addLight(const GzVector& v, const GzColor& c) {
	GzVector vn(-v[0], -v[1], -v[2]);
	vn.normalize();
	lightDir.push_back(vn);
	lightColor.push_back(c);
}

void GzFrameBuffer::drawPointWLight(const GzVertex& v, const GzColor& c, const GzVector& n, GzFunctional status) {
	GzInt x=(GzInt)v[X];
	GzInt y=image.sizeH()-(GzInt)v[Y]-1;
	if ((x<0)||(y<0)||(x>=image.sizeW())||(y>=image.sizeH())) return;
	GzColor cl=colorWLight(c, n);
	if (status&GZ_DEPTH_TEST) {
		if (v[Z]>=depthBuffer[x][y]) {
			image.set(x, y, cl);
			depthBuffer[x][y]=v[Z];
		}
	} else {
		image.set(x, y, cl);
		depthBuffer[x][y]=v[Z];
	}
}

void GzFrameBuffer::drawTriangleWLight(vector<GzVertex>& v, vector<GzColor>& c, vector<GzVector>& n, GzFunctional status) {
	if (curShadeModel==GZ_GOURAUD) {
		vector<GzColor> cl;
		for (int i=0; i!=c.size(); i++) cl.push_back(colorWLight(c[i], n[i]));
		drawTriangle(v, cl, status);
	}
	if (curShadeModel==GZ_PHONG) {
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
			drawRasLineWLight(y, xMin, zMin, cMin, nMin, xMax-1e-3, zMax, cMax, nMax, status);
		}
	}
}

void GzFrameBuffer::drawRasLineWLight(GzInt y, GzReal xMin, GzReal zMin, GzColor& cMin, GzVector& nMin, GzReal xMax, GzReal zMax, GzColor& cMax, GzVector& nMax, GzFunctional status) {
	if ((y<0)||(y>=image.sizeH())) return;
	if ((GzInt)floor(xMin)==(GzInt)floor(xMax)) {
		if (zMin>zMax) drawPointWLight(GzVertex(floor(xMin), y, zMin), cMin, nMin, status);
			else drawPointWLight(GzVertex(floor(xMin), y, zMax), cMax, nMax, status);
	} else {
		GzReal z;
		GzColor c;
		GzVector n;
		y=image.sizeH()-y-1;
		int w=image.sizeW();
		if (status&GZ_DEPTH_TEST) {
			for (int x=max(0, (GzInt)floor(xMin)); x<=min(w-1, (GzInt)floor(xMax)); x++) {
				realInterpolate(xMin, zMin, xMax, zMax, x, z);
				if (z>=depthBuffer[x][y]) {
					colorInterpolate(xMin, cMin, xMax, cMax, x, c);
					normalInterpolate(xMin, nMin, xMax, nMax, x, n);
					image.set(x, y, colorWLight(c, n));
					depthBuffer[x][y]=z;
				}
			}
		} else {
			for (int x=max(0, (GzInt)floor(xMin)); x<=min(w-1, (GzInt)floor(xMax)); x++) {
				realInterpolate(xMin, zMin, xMax, zMax, x, z);
				colorInterpolate(xMin, cMin, xMax, cMax, x, c);
				normalInterpolate(xMin, nMin, xMax, nMax, x, n);
				image.set(x, y, colorWLight(c, n));
				depthBuffer[x][y]=z;
			}
		}
	}
}

void GzFrameBuffer::normalInterpolate(GzReal key1, GzVector& val1, GzReal key2, GzVector& val2, GzReal key, GzVector& val) {
	GzReal k=(key-key1)/(key2-key1);
	for (GzInt i=0; i<3; i++) val[i]=val1[i]+(val2[i]-val1[i])*k;
}

GzColor GzFrameBuffer::colorWLight(GzColor c, GzVector n) {
	GzColor res;
	//Ambient
	for (int i=0; i<3; i++) res[i]=kA*c[i];

	for (int l=0; l!=lightDir.size(); l++) {
		//Diffuse
		GzReal nl=dotProduct(n, lightDir[l]);
		for (int i=0; i<3; i++) res[i]+=kD*lightColor[l][i]*nl;

		GzVector r=2*fabs(nl)*n-lightDir[l];
		GzMatrix rm;
		rm.resize(3, 1);
		rm[0][0]=r[0]; rm[1][0]=r[1]; rm[2][0]=r[2];
		rm=lightTrans*rm;
		r[0]=rm[0][0]; r[1]=rm[1][0]; r[2]=rm[2][0];
		r.normalize();
		GzVector e=GzVector(0, 0, 1);
		GzReal re=dotProduct(r, e);
		//Specular
		for (int i=0; i<3; i++) res[i]+=kS*lightColor[l][i]*pow(re, s);
	}

	//Cutoff
	for (int i=0; i<3; i++) res[i]=min(res[i], double(1));
	return res;
}

