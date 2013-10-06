
#include "define.h"
#include <GL/glut.h>
#include <vector>

class SculptObject
{
private:
	// Array for Geometry
	G308_Point* m_pVertexArray;		// Vertex Array
	G308_Normal* m_pNormalArray;	// Normal Array
	G308_Triangle* m_pTriangles;	// Triangle Array
	G308_UVcoord* m_pUVArray;	    // Texture Coordinate Array
	std::vector<int> *v_normal_faces;

	// Data for Geoemetry
	int m_nNumPoint;
	int m_nNumUV;
	int m_nNumNormal;
	int m_nNumPolygon;


public:
	SculptObject(void);
	~SculptObject(void);

	void ReadOBJ();
	void ReadTexture(char* filename);

	void MouseDrag(int x, int y, float strength);

	void RenderGeometry(bool mode);     // mode : G308_SHADE_POLYGON, G308_SHADE_WIREFRAME
	void calculateVertexNormal(int vertex);
	G308_Point calculateFaceNormal(int face);

};
