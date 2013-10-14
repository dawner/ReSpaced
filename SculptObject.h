
#include "define.h"
#include <GL/glut.h>
#include <vector>
#include <set>

class SculptObject
{
private:
	// Array for Geometry
	G308_Point* m_pVertexArray;		// Vertex Array
	G308_Normal* m_pNormalArray;	// Normal Array
	G308_Triangle* m_pTriangles;	// Triangle Array
	G308_UVcoord* m_pUVArray;	    // Texture Coordinate Array
	std::vector<int> *v_normal_faces;
	std::vector<int> *v_vertex_faces;
	std::set<int> edited_triangles;

	// Data for Geometry
	int m_nNumPoint;
	int m_nNumUV;
	int m_nNumNormal;
	int m_nNumPolygon;

	//Texture stuff.
	static const int width = 512;
	static const int height = 512;
	float pixels[width*height*3];
	float texture[width*height*3];
	GLuint pick_texture;
	GLuint display_texture;


public:
	float current_colour[4];

	SculptObject(void);
	~SculptObject(void);

	void ReadOBJ();
	void LoadTexture(char* filename);

	void SaveOBJ(char* filename);
	void SaveTexture(char* filename);

	void MouseDrag(int x, int y, float strength, float distance, int mode);
	void Sculpt(int poly, float strength, float max_dist, float cur_dist);
	void Paint(int pixel, int distance);
	void FillColour();
	void SetCurrentColour(float r, float g, float b, float a);

	void RenderGeometry(int mode);     // mode : G308_SHADE_POLYGON, G308_SHADE_WIREFRAME
	void calculateVertexNormal(int vertex);
	float calculateDistance(int v1, int v2);
	G308_Point calculateFaceNormal(int face);

};
