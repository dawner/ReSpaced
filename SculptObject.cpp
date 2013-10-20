#include "SculptObject.h"
#include <stdio.h>
#include <math.h>
#include "define.h"

SculptObject::SculptObject(void) {
	m_pVertexArray = NULL;
	m_pNormalArray = NULL;
	m_pUVArray = NULL;
	m_pTriangles = NULL;

	v_normal_faces = NULL;
	v_vertex_faces = NULL;


	m_nNumPoint = m_nNumUV = m_nNumPolygon = 0;

	//Texture for pixel picking.
	glGenTextures(1, &pick_texture);
	glBindTexture(GL_TEXTURE_2D, pick_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	for (int i = 0; i < (width*height*3)-2; i+=3){
		pixels[i] = ((i) % 256)/255.0f;
		pixels[i+1] = ((i / 256) % 256)/255.0f;
		pixels[i+2] = ((i / 65536) % 256)/255.0f;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, pixels);

	//Texture for displaying.
	glGenTextures(1, &display_texture);
	glBindTexture(GL_TEXTURE_2D, display_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	SetCurrentColour(1.0, 1.0, 1.0, 1.0);
	FillColour();

	SetCurrentColour(0.0, 0.5, 0.5, 0.5);

	geometry_changed = false;
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
void SculptObject::ReadOBJ(char* filename) {
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

	if (v_vertex_faces != NULL)
		delete[] v_vertex_faces;
	v_vertex_faces = new std::vector<int>[m_nNumPoint];

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

				v_vertex_faces[v1-1].push_back(numFace);
				v_vertex_faces[v2-1].push_back(numFace);
				v_vertex_faces[v3-1].push_back(numFace);
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

void SculptObject::LoadTexture(char* filename) {
	FILE* fp;

	fp = fopen(filename, "r");
	if (fp == NULL)
		printf("Error reading %s file\n", filename);
	else
		printf("Reading %s file\n", filename);

	for (int i = 0; i < (width*height*3); i++){
		fscanf(fp, "%f ", &texture[i]);
	}
	glBindTexture(GL_TEXTURE_2D, display_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, texture);
	printf("Reading texture file is DONE.\n");
}

void SculptObject::SaveOBJ(char* filename){
	FILE* fp;

	fp = fopen(filename, "w");
	if (fp == NULL)
		printf("Error writing %s file\n", filename);
	else
		printf("Writing %s file\n", filename);
	int i;
	for (i = 0; i < m_nNumPoint; i++){
		fprintf(fp, "v %f %f %f\n", m_pVertexArray[i].x, m_pVertexArray[i].y, m_pVertexArray[i].z);
	}
	for (i = 0; i < m_nNumUV; i++){
		fprintf(fp, "vt %f %f\n", m_pUVArray[i].u, m_pUVArray[i].v);
	}
	for (i = 0; i < m_nNumNormal; i++){
		fprintf(fp, "vn %f %f %f\n", m_pNormalArray[i].x, m_pNormalArray[i].y, m_pNormalArray[i].z);
	}
	for (i = 0; i < m_nNumPolygon; i++){
		fprintf(fp, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", m_pTriangles[i].v1+1, m_pTriangles[i].t1+1, m_pTriangles[i].n1+1, m_pTriangles[i].v2+1, m_pTriangles[i].t2+1, m_pTriangles[i].n2+1, m_pTriangles[i].v3+1, m_pTriangles[i].t3+1, m_pTriangles[i].n3+1);
	}
	fclose(fp);
	printf("Writing OBJ file is DONE.\n");
}


void SculptObject::SaveTexture(char* filename){
	FILE* fp;

	fp = fopen(filename, "w");
	if (fp == NULL)
		printf("Error writing %s file\n", filename);
	else
		printf("Writing %s file\n", filename);

	for (int i = 0; i < (width*height*3); i++){
		fprintf(fp, "%f ", texture[i]);
	}
	printf("Writing texture file is DONE.\n");
}


void SculptObject::MouseDrag(int x, int y, float strength, float distance, int mode) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_FOG);
	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	RenderGeometry(mode);

	unsigned char pixel[3];
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glReadPixels(x, viewport[3] - y, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
	if (pixel[0] + pixel[1] + pixel[2] != 0) {
		if (mode == 1) {
			int n = (pixel[0] - 1) + pixel[1] * 256 + pixel[2] * 65536;
			edited_triangles.clear();
			Sculpt(n, strength, distance, 0.0f);
		}
		else if (mode == 2) {
			int n = (pixel[0]) + pixel[1] * 256 + pixel[2] * 65536;
			Paint(n, distance*3);
		}

	}
}

void SculptObject::Sculpt(int poly, float strength, float max_dist, float cur_dist){
	//Moves the vertices of the selected poly along their normals by the strength,
	//recurses to surrounding faces and then recalcuates normals.
	if (cur_dist < max_dist) {
		if (edited_triangles.count(poly) != 1) {
			edited_triangles.insert(poly);
			float cur_strength = strength * (1 - cur_dist/max_dist);

			m_pVertexArray[m_pTriangles[poly].v1].x += m_pNormalArray[m_pTriangles[poly].n1].x*cur_strength;
			m_pVertexArray[m_pTriangles[poly].v1].y += m_pNormalArray[m_pTriangles[poly].n1].y*cur_strength;
			m_pVertexArray[m_pTriangles[poly].v1].z += m_pNormalArray[m_pTriangles[poly].n1].z*cur_strength;

			m_pVertexArray[m_pTriangles[poly].v2].x += m_pNormalArray[m_pTriangles[poly].n2].x*cur_strength;
			m_pVertexArray[m_pTriangles[poly].v2].y += m_pNormalArray[m_pTriangles[poly].n2].y*cur_strength;
			m_pVertexArray[m_pTriangles[poly].v2].z += m_pNormalArray[m_pTriangles[poly].n2].z*cur_strength;

			m_pVertexArray[m_pTriangles[poly].v3].x += m_pNormalArray[m_pTriangles[poly].n3].x*cur_strength;
			m_pVertexArray[m_pTriangles[poly].v3].y += m_pNormalArray[m_pTriangles[poly].n3].y*cur_strength;
			m_pVertexArray[m_pTriangles[poly].v3].z += m_pNormalArray[m_pTriangles[poly].n3].z*cur_strength;

			for (int i = 0; i < v_vertex_faces[m_pTriangles[poly].v1].size(); i++){
				int v1 = m_pTriangles[poly].v1;
				int v2 = v_vertex_faces[m_pTriangles[poly].v1][i];
				if (v2 != v1) {
					Sculpt(v2, strength, max_dist, cur_dist + calculateDistance(v1, v2));
				}
			}

			for (int i = 0; i < v_vertex_faces[m_pTriangles[poly].v2].size(); i++){
				int v1 = m_pTriangles[poly].v2;
				int v2 = v_vertex_faces[m_pTriangles[poly].v2][i];
				if (v2 != v1) {
					Sculpt(v2, strength, max_dist, cur_dist + calculateDistance(v1, v2));
				}
			}

			for (int i = 0; i < v_vertex_faces[m_pTriangles[poly].v3].size(); i++){
				int v1 = m_pTriangles[poly].v3;
				int v2 = v_vertex_faces[m_pTriangles[poly].v3][i];
				if (v2 != v1) {
					Sculpt(v2, strength, max_dist, cur_dist + calculateDistance(v1, v2));
				}
			}

			calculateVertexNormal(m_pTriangles[poly].n1);
			calculateVertexNormal(m_pTriangles[poly].n2);
			calculateVertexNormal(m_pTriangles[poly].n3);
			geometry_changed = true;
		}
	}
}

void SculptObject::Paint(int pixel, int distance){
	//Fills every pixel within the distance of the pixel selected with the current colour
	//at the current alpha level.
	for (int x = -distance; x < distance; x++){
		for (int y = -distance; y < distance; y++){
			if (sqrt(x*x + y*y) < distance){
				int pos = pixel + (x*3) + (y*3*width);
				if (pos >= 0 && pos <= (width*height*3)-2){
					texture[pos] = current_colour[0]*current_colour[3] + texture[pos]*(1.0f-current_colour[3]);
					texture[pos + 1] = current_colour[1]*current_colour[3] + texture[pos+1]*(1.0f-current_colour[3]);
					texture[pos + 2] = current_colour[2]*current_colour[3] + texture[pos+2]*(1.0f-current_colour[3]);
				}
			}
		}

	}
	glBindTexture(GL_TEXTURE_2D, display_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, texture);
}

void SculptObject::FillColour(){
	//Fills every pixel of the texture with the current colour.
	for (int i = 0; i < (width*height*3)-2; i+=3){
		texture[i] = current_colour[0]*current_colour[3] + texture[i]*(1.0f-current_colour[3]);
		texture[i+1] = current_colour[1]*current_colour[3] + texture[i+1]*(1.0f-current_colour[3]);
		texture[i+2] = current_colour[2]*current_colour[3] + texture[i+2]*(1.0f-current_colour[3]);
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, texture);
}

void SculptObject::SetCurrentColour(float r, float g, float b, float a){
	current_colour[0] = r;
	current_colour[1] = g;
	current_colour[2] = b;
	current_colour[3] = a;
}

float SculptObject::calculateDistance(int v1, int v2) {
	return fabs(m_pVertexArray[v1].x - m_pVertexArray[v2].x) + fabs(m_pVertexArray[v1].y - m_pVertexArray[v2].y) + fabs(m_pVertexArray[v1].z - m_pVertexArray[v2].z);
}

void SculptObject::calculateVertexNormal(int vertex) {
	//Calculates the face normals and then averages them to get the vertex normal.
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
	G308_Point u;
	G308_Point v;
	double l;

	u.x = m_pVertexArray[m_pTriangles[face].v2].x - m_pVertexArray[m_pTriangles[face].v1].x;
	u.y = m_pVertexArray[m_pTriangles[face].v2].y - m_pVertexArray[m_pTriangles[face].v1].y;
	u.z = m_pVertexArray[m_pTriangles[face].v2].z - m_pVertexArray[m_pTriangles[face].v1].z;
	v.x = m_pVertexArray[m_pTriangles[face].v3].x - m_pVertexArray[m_pTriangles[face].v1].x;
	v.y = m_pVertexArray[m_pTriangles[face].v3].y - m_pVertexArray[m_pTriangles[face].v1].y;
	v.z = m_pVertexArray[m_pTriangles[face].v3].z - m_pVertexArray[m_pTriangles[face].v1].z;
	normal.x = (u.y * v.z) - (u.z * v.y);
	normal.y = (u.z * v.x) - (u.x * v.z);
	normal.z = (u.x * v.y) - (u.y * v.x);
	l = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
	normal.x /= l;
	normal.y /= l;
	normal.z /= l;
	return normal;
}

void SculptObject::RenderGeometry(int mode) {
	glColor3f(1.0f, 1.0f, 1.0f);
	//Texture point picking mode.
	if (mode == 2) {
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, pick_texture);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, GL_REPLACE);
	}
	//Normal display mode.
	else if (mode == 0) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, display_texture);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, GL_REPLACE);
	}
	int n;
	for (n = 0; n < m_nNumPolygon; n++){
		//Poly picking mode.
		if (mode == 1) {
			glColor3f(((n+1) % 256)/255.0f, ((n / 256) % 256)/255.0f, ((n / 65536) % 256)/255.0f);
		}
		glBegin(GL_TRIANGLES);

				glTexCoord2f(m_pUVArray[m_pTriangles[n].t1].u, m_pUVArray[m_pTriangles[n].t1].v);
				glNormal3f(m_pNormalArray[m_pTriangles[n].n1].x, m_pNormalArray[m_pTriangles[n].n1].y, m_pNormalArray[m_pTriangles[n].n1].z);
				glVertex3f(m_pVertexArray[m_pTriangles[n].v1].x, m_pVertexArray[m_pTriangles[n].v1].y, m_pVertexArray[m_pTriangles[n].v1].z);

				glTexCoord2f(m_pUVArray[m_pTriangles[n].t2].u, m_pUVArray[m_pTriangles[n].t2].v);
				glNormal3f(m_pNormalArray[m_pTriangles[n].n2].x, m_pNormalArray[m_pTriangles[n].n2].y, m_pNormalArray[m_pTriangles[n].n2].z);
				glVertex3f(m_pVertexArray[m_pTriangles[n].v2].x, m_pVertexArray[m_pTriangles[n].v2].y, m_pVertexArray[m_pTriangles[n].v2].z);

				glTexCoord2f(m_pUVArray[m_pTriangles[n].t3].u, m_pUVArray[m_pTriangles[n].t3].v);
				glNormal3f(m_pNormalArray[m_pTriangles[n].n3].x, m_pNormalArray[m_pTriangles[n].n3].y, m_pNormalArray[m_pTriangles[n].n3].z);
				glVertex3f(m_pVertexArray[m_pTriangles[n].v3].x, m_pVertexArray[m_pTriangles[n].v3].y, m_pVertexArray[m_pTriangles[n].v3].z);

		glEnd();

	}
	glDisable(GL_TEXTURE_2D);
}
