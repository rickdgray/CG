#include "GzMatrix.h"



//============================================================================
//Implementations in Assignment #3
//============================================================================


//Class represents for matrices-----------------------------------------------
GzVertex GzMatrix::toVertex() {
	assert((nRow()==4)&&(nCol()==1));

	GzReal x = at(0)[0]/at(3)[0];
	GzReal y = at(1)[0]/at(3)[0];
	GzReal z = at(2)[0]/at(3)[0];

	return GzVertex(x,y,z);
}

void GzMatrix::fromVertex(const GzVertex& v) {
	this->resize(4,1);

    for(int i = 0; i < 3; i++)
    {
        at(i)[0] = v[i];
    }
	
    at(3)[0] = 1;
}

void GzMatrix::resize(GzInt _nRow, GzInt _nCol) {
	vector<vector<GzReal> >::resize(_nRow);
	for (GzInt i=0; i<_nRow; i++) 
		at(i).resize(_nCol);
}

GzInt GzMatrix::nRow() const {
	return (GzInt)size();
}

GzInt GzMatrix::nCol() const {
	if (size()==0) return 0;
	return at(0).size();
}
//----------------------------------------------------------------------------


//Matrix operators------------------------------------------------------------
GzMatrix operator + (const GzMatrix& a, const GzMatrix& b) {
	assert((a.nRow()==b.nRow())&&(a.nCol()==b.nCol()));
	int m=a.nRow();
	int n=a.nCol();
	GzMatrix c;
	c.resize(m, n);
	for (int i=0; i<m; i++)
		for (int j=0; j<n; j++) c[i][j]=a[i][j]+b[i][j];
	return c;
}

GzMatrix operator - (const GzMatrix& a, const GzMatrix& b) {
	assert((a.nRow()==b.nRow())&&(a.nCol()==b.nCol()));
	int m=a.nRow();
	int n=a.nCol();
	GzMatrix c;
	c.resize(m, n);
	for (int i=0; i<m; i++)
		for (int j=0; j<n; j++) c[i][j]=a[i][j]-b[i][j];
	return c;
}

GzMatrix operator * (const GzMatrix& a, const GzMatrix& b) {
	assert(a.nCol()==b.nRow());
	int m=a.nRow();
	int n=a.nCol();
	int p=b.nCol();
	GzMatrix c;
	c.resize(m, p);
	for (int i=0; i<m; i++)
		for (int j=0; j<p; j++) {
			c[i][j]=0;
			for (int k=0; k<n; k++) c[i][j]+=a[i][k]*b[k][j];
		}
	return c;
}
//----------------------------------------------------------------------------


//Some default matrices-------------------------------------------------------
GzMatrix Zeros(GzInt _n) {
	GzMatrix a;
	a.resize(_n, _n);
	for (int i=0; i<_n; i++)
		for (int j=0; j<_n; j++) a[i][j]=0;
	return a;
}

GzMatrix Identity(GzInt _n) {
	GzMatrix a=Zeros(_n);
	for (int i=0; i<_n; i++) a[i][i]=1;
	return a;
}
//----------------------------------------------------------------------------


//============================================================================
//End of Implementations in Assignment #3
//============================================================================
