#include "SculptObject.h"
//#include "G308_ImageLoader.h"
#include <stdio.h>
#include <math.h>
#include "define.h"

SculptObject::SculptObject(void) {
	m_pVertexArray = NULL;
	m_pNormalArray = NULL;
	m_pUVArray = NULL;
	m_pTriangles = NULL;

	v_normal_faces = NULL;

	m_nNumPoint = m_nNumUV = m_nNumPolygon = 0;
}

SculptObject::~SculptObject(void) {
	if (m_pVertexArray != NULL)
		delete[] m_pVertexArray;
	if (m_pNormalArray != NULL)
		delete[] m_pNormalArray;
	if (m_pUVArray != NULL)
		delete[] m_pUVArray;
	if (m_pTriangles != NULL)
		delete[] m_pTriangles;
	//free(texture);
}

//-------------------------------------------------------
// Read in the OBJ (Note: fails quietly, so take care)
//--------------------------------------------------------
void SculptObject::ReadOBJ() {
	char filename[] = "sculpt_sphere.obj";
	FILE* fp;
	char mode, vmode;
	char str[200];
	int v1, v2, v3, n1, n2, n3, t1, t2, t3;
	int numVert, numNorm, numUV, numFace;
	float x, y, z;
	float u, v;

	numVert = numNorm = numUV = numFace = 0;

	fp = fopen(filename, "r");
	if (fp == NULL)
		printf("Error reading %s file\n", filename);
	else
		printf("Reading %s file\n", filename);

	// Check number of vertex, normal, uvCoord, and Face
	while (fgets(str, 200, fp) != NULL) {
		sscanf(str, "%c%c", &mode, &vmode);
		switch (mode) {
		case 'v': /* vertex, uv, normal */
			if (vmode == 't') // uv coordinate
				numUV++;
			else if (vmode == 'n') // normal
				numNorm++;
			else if (vmode == ' ') // vertex
				numVert++;
			break;
		case 'f': /* faces */
			numFace++;
			break;
		}
	}

	m_nNumPoint = numVert;
	m_nNumUV = numUV;
	m_nNumPolygon = numFace;
	m_nNumNormal = numNorm;

	printf("Number of Point %d, UV %d, Normal %d, Face %d\n", numVert, numUV,
			numNorm, numFace);
	//-------------------------------------------------------------
	//	Allocate memory for array
	//-------------------------------------------------------------

	if (m_pVertexArray != NULL)
		delete[] m_pVertexArray;
	m_pVertexArray = new G308_Point[m_nNumPoint];

	if (m_pNormalArray != NULL)
		delete[] m_pNormalArray;
	m_pNormalArray = new G308_Normal[m_nNumNormal];

	if (m_pUVArray != NULL)
		delete[] m_pUVArray;
	m_pUVArray = new G308_UVcoord[m_nNumUV];

	if (m_pTriangles != NULL)
		delete[] m_pTriangles;
	m_pTriangles = new G308_Triangle[m_nNumPolygon];

	if (v_normal_faces != NULL)
		delete[] v_normal_faces;
	v_normal_faces = new std::vector<int>[m_nNumNormal];

	//-----------------------------------------------------------
	//	Read obj file
	//-----------------------------------------------------------
	numVert = numNorm = numUV = numFace = 0;

	fseek(fp, 0L, SEEK_SET);
	while (fgets(str, 200, fp) != NULL) {
		sscanf(str, "%c%c", &mode, &vmode);
		switch (mode) {
		case 'v': /* vertex, uv, normal */
			if (vmode == 't') // uv coordinate
					{
				sscanf(str, "vt %f %f", &u, &v);
				m_pUVArray[numUV].u = u;
				m_pUVArray[numUV].v = v;
				numUV++;
			} else if (vmode == 'n') // normal
					{
				sscanf(str, "vn %f %f %f", &x, &y, &z);
				m_pNormalArray[numNorm].x = x;
				m_pNormalArray[numNorm].y = y;
				m_pNormalArray[numNorm].z = z;
				numNorm++;
			} else if (vmode == ' ') // vertex
					{
				sscanf(str, "v %f %f %f", &x, &y, &z);
				m_pVertexArray[numVert].x = x;
				m_pVertexArray[numVert].y = y;
				m_pVertexArray[numVert].z = z;
				numVert++;
			}
			break;
		case 'f': /* faces : stored value is index - 1 since our index starts from 0, but obj starts from 1 */
			if (numNorm > 0 && numUV > 0) {
				sscanf(str, "f %d/%d/%d %d/%d/%d %d/%d/%d", &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3);
			} else if(numNorm > 0 && numUV ==0){
				sscanf(str, "f %d//%d %d//%d %d//%d", &v1, &n1, &v2, &n2, &v3, &n3);
			} else if(numUV > 0 && numNorm==0){
				sscanf(str, "f %d/%d %d/%d %d/%d", &v1, &t1, &v2, &t2, &v3, &t3);
			} else if(numUV==0 && numNorm==0){
				sscanf(str, "f %d %d %d", &v1, &v2, &v3);
			}
			// Vertex indicies for triangle:
			if (numVert != 0) {
				m_pTriangles[numFace].v1 = v1 - 1;
				m_pTriangles[numFace].v2 = v2 - 1;
				m_pTriangles[numFace].v3 = v3 - 1;
			}

			// Normal indicies for triangle
			if (numNorm != 0) {
				m_pTriangles[numFace].n1 = n1 - 1;
				m_pTriangles[numFace].n2 = n2 - 1;
				m_pTriangles[numFace].n3 = n3 - 1;

				v_normal_faces[n1-1].push_back(numFace);
				v_normal_faces[n2-1].push_back(numFace);
				v_normal_faces[n3-1].push_back(numFace);
			}

			// UV indicies for triangle
			if (numUV != 0) {
				m_pTriangles[numFace].t1 = t1 - 1;
				m_pTriangles[numFace].t2 = t2 - 1;
				m_pTriangles[numFace].t3 = t3 - 1;
			}

			numFace++;
			break;
		default:
			break;
		}
	}

	fclose(fp);
	printf("Reading OBJ file is DONE.\n");

}

//--------------------------------------------------------------
// [Assignment4]
// Create a 2D GL texture from the file given
//--------------------------------------------------------------
/*void SculptObject::ReadTexture(char* filename) {
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glShadeModel(GL_FLAT);
	glEnable(GL_DEPTH_TEST);

	TextureInfo t;
	loadTextureFromJPEG(filename, &t);

	//Init the texture storage, and set some parameters.
	//(I high recommend reading up on these commands)
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//Only useful for PNG files, since JPEG doesn't support alpha
	if (t.hasAlpha) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t.width, t.height, 0, GL_RGBA,
				GL_UNSIGNED_BYTE, t.textureData);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t.width, t.height, 0, GL_RGB,
				GL_UNSIGNED_BYTE, t.textureData);
	}
	//Once the texture has been loaded by GL, we don't need this anymore.
	free(t.textureData);

}*/


void SculptObject::MouseDrag(int x, int y, float strength) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_FOG);
	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	RenderGeometry(true);
	unsigned char pixel[3];
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glReadPixels(x, viewport[3] - y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
	if (pixel[0] + pixel[1] + pixel[2] != 0) {
		int n = (pixel[0] - 1) + pixel[1] * 256 + pixel[2] * 65536;
		Sculpt(n, strength, 5.0f);
		
	}
}

void SculptObject::Sculpt(int poly, float strength, float distance){
	if (distance > 0.0) {
		m_pVertexArray[m_pTriangles[poly].v1].x += m_pNormalArray[m_pTriangles[poly].n1].x*strength;
		m_pVertexArray[m_pTriangles[poly].v1].y += m_pNormalArray[m_pTriangles[poly].n1].y*strength;
		m_pVertexArray[m_pTriangles[poly].v1].z += m_pNormalArray[m_pTriangles[poly].n1].z*strength;

		m_pVertexArray[m_pTriangles[poly].v2].x += m_pNormalArray[m_pTriangles[poly].n2].x*strength;
		m_pVertexArray[m_pTriangles[poly].v2].y += m_pNormalArray[m_pTriangles[poly].n2].y*strength;
		m_pVertexArray[m_pTriangles[poly].v2].z += m_pNormalArray[m_pTriangles[poly].n2].z*strength;

		m_pVertexArray[m_pTriangles[poly].v3].x += m_pNormalArray[m_pTriangles[poly].n3].x*strength;
		m_pVertexArray[m_pTriangles[poly].v3].y += m_pNormalArray[m_pTriangles[poly].n3].y*strength;
		m_pVertexArray[m_pTriangles[poly].v3].z += m_pNormalArray[m_pTriangles[poly].n3].z*strength;

		calculateVertexNormal(m_pTriangles[poly].n1);
		calculateVertexNormal(m_pTriangles[poly].n2);
		calculateVertexNormal(m_pTriangles[poly].n3);

		for (int i = 0; i < v_normal_faces[m_pTriangles[poly].v1].size(); i++){
			int v1 = m_pTriangles[poly].v1;
			int v2 = v_normal_faces[m_pTriangles[poly].v1][i];
			if (v2 != v1) {
				Sculpt(v2, strength, distance - calculateDistance(v1, v2)); 
			}
		}

		for (int i = 0; i < v_normal_faces[m_pTriangles[poly].v2].size(); i++){
			int v1 = m_pTriangles[poly].v2;
			int v2 = v_normal_faces[m_pTriangles[poly].v2][i];
			if (v2 != v1) {
				Sculpt(v2, strength, distance - calculateDistance(v1, v2)); 
			}
		}

		for (int i = 0; i < v_normal_faces[m_pTriangles[poly].v3].size(); i++){
			int v1 = m_pTriangles[poly].v3;
			int v2 = v_normal_faces[m_pTriangles[poly].v3][i];
			if (v2 != v1) {
				Sculpt(v2, strength, distance - calculateDistance(v1, v2)); 
			}
		}

	}
}

float SculptObject::calculateDistance(int v1, int v2) {
	return fabs(m_pVertexArray[v1].x - m_pVertexArray[v2].x) + fabs(m_pVertexArray[v1].y - m_pVertexArray[v2].y) + fabs(m_pVertexArray[v1].z - m_pVertexArray[v2].z);
}

void SculptObject::calculateVertexNormal(int vertex) {
	G308_Normal average = {0.0, 0.0, 0.0};

	for (int i = 0; i < v_normal_faces[vertex].size(); i++){
		G308_Point face_n = calculateFaceNormal(v_normal_faces[vertex][i]);
		average.x += face_n.x;
		average.y += face_n.y;
		average.z += face_n.z;
	}

	average.x /= v_normal_faces[vertex].size();
	average.y /= v_normal_faces[vertex].size();
	average.z /= v_normal_faces[vertex].size();

	m_pNormalArray[vertex] = average;
}

G308_Point SculptObject::calculateFaceNormal(int face) {

	G308_Point normal;
	float Ux, Uy, Uz, Vx, Vy, Vz;
	double l;

	Ux = m_pVertexArray[m_pTriangles[face].v2].x - m_pVertexArray[m_pTriangles[face].v1].x;
	Uy = m_pVertexArray[m_pTriangles[face].v2].y - m_pVertexArray[m_pTriangles[face].v1].y;
	Uz = m_pVertexArray[m_pTriangles[face].v2].z - m_pVertexArray[m_pTriangles[face].v1].z;
	Vx = m_pVertexArray[m_pTriangles[face].v3].x - m_pVertexArray[m_pTriangles[face].v1].x;
	Vy = m_pVertexArray[m_pTriangles[face].v3].y - m_pVertexArray[m_pTriangles[face].v1].y;
	Vz = m_pVertexArray[m_pTriangles[face].v3].z - m_pVertexArray[m_pTriangles[face].v1].z;
	normal.x = (Uy * Vz) - (Uz * Vy);
	normal.y = (Uz * Vx) - (Ux * Vz);
	normal.z = (Ux * Vy) - (Uy * Vx);
	l = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
	normal.x /= l;
	normal.y /= l;
	normal.z /= l;
	return normal;
}

void SculptObject::RenderGeometry(bool mode) {

	int n;
	for (n = 0; n < m_nNumPolygon; n++){
		if (mode) {
			GLubyte pixel[3] = {n % 256, (n / 256) % 256, (n / 65536) % 256};
			glColor3f((n % 256)/255.0f, ((n / 256) % 256)/255.0f, ((n / 65536) % 256)/255.0f);
		}
		glBegin(GL_TRIANGLES);

				//glTexCoord2f(m_pUVArray[m_pTriangles[n].t1].u, m_pUVArray[m_pTriangles[n].t1].v);
				glNormal3f(m_pNormalArray[m_pTriangles[n].n1].x, m_pNormalArray[m_pTriangles[n].n1].y, m_pNormalArray[m_pTriangles[n].n1].z);
				glVertex3f(m_pVertexArray[m_pTriangles[n].v1].x, m_pVertexArray[m_pTriangles[n].v1].y, m_pVertexArray[m_pTriangles[n].v1].z);

				//glTexCoord2f(m_pUVArray[m_pTriangles[n].t2].u, m_pUVArray[m_pTriangles[n].t2].v);
				glNormal3f(m_pNormalArray[m_pTriangles[n].n2].x, m_pNormalArray[m_pTriangles[n].n2].y, m_pNormalArray[m_pTriangles[n].n2].z);
				glVertex3f(m_pVertexArray[m_pTriangles[n].v2].x, m_pVertexArray[m_pTriangles[n].v2].y, m_pVertexArray[m_pTriangles[n].v2].z);

				//glTexCoord2f(m_pUVArray[m_pTriangles[n].t3].u, m_pUVArray[m_pTriangles[n].t3].v);
				glNormal3f(m_pNormalArray[m_pTriangles[n].n3].x, m_pNormalArray[m_pTriangles[n].n3].y, m_pNormalArray[m_pTriangles[n].n3].z);
				glVertex3f(m_pVertexArray[m_pTriangles[n].v3].x, m_pVertexArray[m_pTriangles[n].v3].y, m_pVertexArray[m_pTriangles[n].v3].z);

		glEnd();
	}
}
