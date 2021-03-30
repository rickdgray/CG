#include "Gz.h"



//============================================================================
//Implementations in Assignment #1
//============================================================================
GzImage Gz::toImage() {
	return frameBuffer.toImage();
}

void Gz::clear(GzFunctional buffer) {
	frameBuffer.clear(buffer);
}

void Gz::clearColor(const GzColor& color) {
	frameBuffer.setClearColor(color);
}

void Gz::clearDepth(GzReal depth) {
	frameBuffer.setClearDepth(depth);
}

void Gz::enable(GzFunctional f) {
	status=status|f;
}

void Gz::disable(GzFunctional f) {
	status=status&(~f);
}

GzBool Gz::get(GzFunctional f) {
	if (status&f) return true; else return false;
}

void Gz::begin(GzPrimitiveType p) {
	currentPrimitive=p;
}

void Gz::addVertex(const GzVertex& v) {
	vertexQueue.push(v);
}

void Gz::addColor(const GzColor& c) {
	colorQueue.push(c);
}
//============================================================================
//End of Implementations in Assignment #1
//============================================================================



//============================================================================
//Implementations in Assignment #3
//============================================================================

void Gz::initFrameSize(GzInt width, GzInt height) {
	//This function need to be updated since we have introduced the viewport.
	//The viewport size is set to the size of the frame buffer.
	wViewport=(GzReal)width;
	hViewport=(GzReal)height;
	frameBuffer.initFrameSize(width, height);
	prjMatrix = Identity(4);
	transMatrix = Identity(4);
	viewport(0, 0);			//Default center of the viewport 
}

void Gz::end() {
	switch (currentPrimitive) {
		case GZ_POINTS: {
			GzMatrix primativeM;
			GzVertex v;
			GzColor c;

			while ( (vertexQueue.size()>=1) && (colorQueue.size()>=1) )
			{
				primativeM.fromVertex(vertexQueue.front());
				primativeM = prjMatrix*transMatrix*primativeM;
				v = primativeM.toVertex();
				affineTransform(v);
				vertexQueue.pop();

				c = colorQueue.front();
				colorQueue.pop();
						
				frameBuffer.drawPoint(v,c,status);
			}
		} break;
		case GZ_TRIANGLES: {
			while (vertexQueue.size() >= 1 && colorQueue.size() >= 1)
			{
				vector<GzColor> clist;
				GzMatrix primativeM;
				vector<GzVertex> vlist;

				for (int i = 0; i < 3; i++)
				{
					primativeM.fromVertex(vertexQueue.front());
					primativeM = prjMatrix*transMatrix*primativeM;
					vlist.push_back(primativeM.toVertex());
					affineTransform(vlist[i]);
					vertexQueue.pop();

					clist.push_back(colorQueue.front());
					colorQueue.pop();

				}

				frameBuffer.drawTriangle(vlist,clist,status);
			}
		}
	}
}

void Gz::viewport(GzInt x, GzInt y) {
	//This function only updates xViewport and yViewport.
	//Viewport calculation will be done in different function, e.g. Gz::end().
	//See http://www.opengl.org/sdk/docs/man/xhtml/glViewport.xml
	//Or google: glViewport
	xViewport=x;
	yViewport=y;
}

//Transformations-------------------------------------------------------------
void Gz::lookAt(GzReal eyeX, GzReal eyeY, GzReal eyeZ, GzReal centerX, GzReal centerY, GzReal centerZ, GzReal upX, GzReal upY, GzReal upZ) {
	prjMatrix = Identity(4);
    transMatrix = Identity(4);

    GzReal f[] = {centerX - eyeX, centerY - eyeY, centerZ - eyeZ};

    eucledianNorm(f);
    GzReal up[] = {upX, upY, upZ};
    eucledianNorm(up);
    GzReal s[3];
	GzReal u[3];

    crossProduct(s,f,up);
    crossProduct(u,s,f);

    GzReal arrayM[] = { s[0], s[1], s[2], 0,
                        u[0], u[1], u[2], 0,
                       -f[0],-f[1],-f[2],0,
                        0, 0, 0, 1};
    GzMatrix M;
    arrayToMatrix(arrayM,M,4,4);

    multMatrix(M);;
    translate(-eyeX,-eyeY, -eyeZ);
}

void Gz::translate(GzReal x, GzReal y, GzReal z) {
	GzReal arrayM[] = {1, 0, 0, x,
                       0, 1, 0, y,
                       0, 0, 1, z,
                       0, 0, 0, 1};

    GzMatrix M;
    arrayToMatrix(arrayM,M,4,4);

    multMatrix(M);
}

void Gz::rotate(GzReal angle, GzReal x, GzReal y, GzReal z) {
	angle = angle*Pi()/180;
    GzReal c = cos(angle);
    GzReal s = sin(angle);

    GzReal v[] = {x, y, z};
    eucledianNorm(v);

    GzReal arrayM[] =
    {v[X]*v[X]*(1-c) + c, v[X]*v[Y]*(1-c) - v[Z]*s, v[X]*v[Z]*(1-c) + v[Y]*s, 0,
     v[Y]*v[X]*(1-c) + v[Z]*s, v[Y]*v[Y]*(1-c) + c, v[Y]*v[Z]*(1-c) - v[X]*s, 0,
     v[X]*v[Z]*(1-c) - v[Y]*s, v[Y]*v[Z]*(1-c) + v[X]*s, v[Z]*v[Z]*(1-c) + c, 0,
     0, 0, 0, 1};

    GzMatrix M;
    arrayToMatrix(arrayM,M,4,4);

    multMatrix(M);
}

void Gz::scale(GzReal x, GzReal y, GzReal z) {
	GzReal arrayM[] = {x, 0, 0, 0,
                       0, y, 0, 0,
                       0, 0, z, 0,
                       0, 0, 0, 1};

    GzMatrix M;
    arrayToMatrix(arrayM,M,4,4);

    multMatrix(M);
}

//This function was updated on September 26, 2010
void Gz::multMatrix(GzMatrix mat) {
	//Multiply transMatrix by the matrix mat
	transMatrix=mat*transMatrix;
}
//End of Transformations------------------------------------------------------

//Projections-----------------------------------------------------------------
void Gz::perspective(GzReal fovy, GzReal aspect, GzReal zNear, GzReal zFar) {
	fovy = fovy*Pi()/180;
    GzReal f = cotan(fovy/2);
    GzReal arrayM[] = {f/aspect, 0, 0, 0,
                       0, f, 0, 0,
                       0, 0, -(zFar+zNear)/(zNear - zFar), -2*(zFar*zNear)/(zNear-zFar),
                       0, 0, -1, 0};

    GzMatrix M;
    arrayToMatrix(arrayM,M,4,4);
	
    prjMatrix = M;
}

void Gz::orthographic(GzReal left, GzReal right, GzReal bottom, GzReal top, GzReal nearVal, GzReal farVal) {
	GzReal tx = -(right + left)/(right - left);
    GzReal ty = -(top + bottom)/(top - bottom);
    GzReal tz = (farVal + nearVal)/(farVal - nearVal);

    GzReal arrayM[] =  {2/(right - left), 0, 0, tx,
                        0, 2/(top - bottom), 0, ty,
                        0, 0, 2/(farVal-nearVal), tz,
                        0, 0, 0, 1};

    GzMatrix M;
    arrayToMatrix(arrayM,M,4,4);

    prjMatrix = M;
}
//End of Projections----------------------------------------------------------

void Gz::eucledianNorm(GzReal v[])
{
    GzReal norm = 0;

    for (int i = 0; i < 3; i ++)
        norm += v[i]*v[i];

    norm = sqrt(norm);

    for (int j = 0; j < 3; j++)
        v[j] = v[j]/norm;
}

void Gz::crossProduct(GzReal result[], GzReal v1[], GzReal v2[])
{
    result[0] = v1[1]*v2[2] - v1[2]*v2[1];
    result[1] = -(v1[0]*v2[2] - v1[2]*v2[0]);
    result[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

void Gz::arrayToMatrix(GzReal a[], GzMatrix& m, int nRow, int nCol)
{
    m.resize(nRow,nCol);

    for (int i = 0; i < nRow*nCol; i++)
    {
        m.at(i/nRow)[i%nRow] = a[i];
    }
}

void Gz::affineTransform(GzVertex &dCoord)
{
    dCoord[X] = (dCoord[X] + 1)*(wViewport)/2 + xViewport;
    dCoord[Y] = (dCoord[Y] + 1)*(hViewport)/2 + yViewport;
    dCoord[Z] = (dCoord[Z] + 1)/2;
}

//============================================================================
//End of Implementations in Assignment #3
//============================================================================
