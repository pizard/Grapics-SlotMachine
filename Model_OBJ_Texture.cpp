#include "Model_OBJ_Texture.h"
#include <windows.h>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define VERTEX_PER_TRIANGLE 3
#define VERTEX_PER_QUAD 4
#define VERTEX_PER_FACE 16
#define MAX_NUM_OF_SLASH_PER_FACE 32
using namespace std;

bool LoadMeshFromFile(string loadFile, GLuint &tex_id) {
	const char *texFile = loadFile.c_str();
	FILE *fp = fopen(texFile, "rb");
	if (!fp) {
		printf("ERROR : NO %s.\n fail to bind %d\n", texFile, tex_id);
		return false;
	}

	int width, height, channel;
	unsigned char *image = stbi_load_from_file(fp, &width, &height, &channel, 4);
	fclose(fp);

	//bind
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, GL_MODULATE);
	return true;
}
Model_OBJ_Texture::Model_OBJ_Texture()
{
	this->numOfV = 0;
	this->numOfN = 0;
	this->numOfF = 0;
	this->numOfSub = 0;
	this->numOfUv = 0;
	this->subTex_arrIdx = 0;
}


Model_OBJ_Texture::~Model_OBJ_Texture()
{
}


int Model_OBJ_Texture::Load(char * filename)
{
	string line;
	ifstream objFile(filename);
	if (objFile.is_open()) {
		Vertex temp_v;
		UV temp_uv;
		while (!objFile.eof()) {
			getline(objFile, line);
			if (line.c_str()[0] == 'v' && line.c_str()[1] != 'n' && line.c_str()[1] != 't') {
				line[0] = ' ';
				sscanf(line.c_str(), "%f %f %f ", &temp_v.x, &temp_v.y, &temp_v.z);
				vList.push_back(temp_v);
				numOfV++;
			}
			if (line.c_str()[0] == 'v' && line.c_str()[1] == 'n') {
				line[0] = ' '; line[1] = ' ';
				sscanf(line.c_str(), "%f %f %f ", &temp_v.x, &temp_v.y, &temp_v.z);
				nList.push_back(temp_v);
				numOfN++;
			}
			if (line.c_str()[0] == 'v' && line.c_str()[1] == 't') {
				line[0] = ' '; line[1] = ' ';
				sscanf(line.c_str(), "%f %f ", &temp_uv.x, &temp_uv.y);
				temp_uv.y = 1 - temp_uv.y;
				uvList.push_back(temp_uv);
				numOfUv++;
			}
			if (line.substr(0, 6) == "mtllib") {
				this->MtlFileName = line.substr(7);
			}
			if (line.substr(0, 6) == "usemtl") {
				string mat_name = line.substr(7);
				ifstream mtlFile(MtlFileName);
				SubMesh temp_sub;
				if (mtlFile.is_open()) {
					string mtlLine;
					while (!mtlFile.eof()) {
						getline(mtlFile, mtlLine);
						if (mtlLine.substr(0, 6) == "newmtl" && mtlLine.substr(7) == mat_name) {
							float Kd[4] = { 0 }, Ks[4] = { 0 }, Ka[4] = { 0 };
							while (!mtlFile.eof()) {
								getline(mtlFile, mtlLine);
								if (mtlLine.size() == 0) break;
								if (mtlLine.substr(0, 6) == "newmtl"&& mtlLine.substr(7) != mat_name) break;

								if (mtlLine.substr(0, 2) == "Ka") {
									mtlLine[0] = ' '; mtlLine[1] = ' ';
									sscanf(mtlLine.c_str(), "%f %f %f",
										&Ka[0], &Ka[1], &Ka[2]); Ka[3] = 1;
								}
								if (mtlLine.substr(0, 2) == "Kd") {
									mtlLine[0] = ' '; mtlLine[1] = ' ';
									sscanf(mtlLine.c_str(), "%f %f %f",
										&Kd[0], &Kd[1], &Kd[2]); Kd[3] = 1;
								}
								if (mtlLine.substr(0, 2) == "Ks") {
									mtlLine[0] = ' '; mtlLine[1] = ' ';
									sscanf(mtlLine.c_str(), "%f %f %f",
										&Ks[0], &Ks[1], &Ks[2]); Ks[3] = 1;
								}
								if (mtlLine.substr(0, 6) == "map_Kd") {
									temp_sub.loadFile = mtlLine.substr(7);
								}
							}
							for (int i = 0; i < 4; i++) {
								temp_sub.Ka[i] = Ka[i];
								temp_sub.Ks[i] = Ks[i];
								temp_sub.Kd[i] = Kd[i];
							}
							Mesh_subList.push_back(temp_sub);
							if (Mesh_subList[numOfSub].loadFile.size() != 0) {
								LoadMeshFromFile(Mesh_subList[numOfSub].loadFile, Mesh_subList[numOfSub].sub_texture);
							}
							numOfSub++;
						}
					}
				}
				else {
					cout << "Unable to open mtl file";
				}
			}
			if (line.c_str()[0] == 'f') {
				line[0] = ' ';
				Face temp_f;
				int vertexNumber[VERTEX_PER_FACE] = { 0 };
				int normalNumber[VERTEX_PER_FACE] = { 0 };
				int uvNumber[VERTEX_PER_FACE] = { 0 };
				int slash_cnt = 0;
				int slash_idx[MAX_NUM_OF_SLASH_PER_FACE] = { 0 };
				for (int i = 0; i < line.size(); i++) {
					if (line[i] == '/') {
						slash_idx[slash_cnt] = i;
						slash_cnt++;
					}
				}
				if (slash_cnt == 6) {
					if (slash_idx[1] - slash_idx[0] == 1) {
						sscanf(line.c_str(), "%i//%i %i//%i %i//%i",
							&vertexNumber[0], &normalNumber[0],
							&vertexNumber[1], &normalNumber[1],
							&vertexNumber[2], &normalNumber[2]);
						vertexNumber[0] -= 1; vertexNumber[1] -= 1; vertexNumber[2] -= 1;
						normalNumber[0] -= 1; normalNumber[1] -= 1; normalNumber[2] -= 1;

						for (int i = 0; i < 3; i++) {
							temp_f.face_vList.push_back(vList[vertexNumber[i]]);
							temp_f.face_nList.push_back(nList[normalNumber[i]]);
						}
						Mesh_subList[numOfSub - 1].subMesh_fList.push_back(temp_f);
						numOfF++;
					}
					else if (slash_idx[1] - slash_idx[0] != 1) {
						sscanf(line.c_str(), "%i/%i/%i %i/%i/%i %i/%i/%i",
							&vertexNumber[0], &uvNumber[0], &normalNumber[0],
							&vertexNumber[1], &uvNumber[1], &normalNumber[1],
							&vertexNumber[2], &uvNumber[2], &normalNumber[2]);
						vertexNumber[0] -= 1; vertexNumber[1] -= 1; vertexNumber[2] -= 1;
						uvNumber[0] -= 1; uvNumber[1] -= 1; uvNumber[2] -= 1;
						normalNumber[0] -= 1; normalNumber[1] -= 1; normalNumber[2] -= 1;
						for (int i = 0; i < 3; i++) {
							temp_f.face_vList.push_back(vList[vertexNumber[i]]);
							temp_f.face_nList.push_back(nList[normalNumber[i]]);
							temp_f.face_uvList.push_back(uvList[uvNumber[i]]);
						}
						Mesh_subList[numOfSub - 1].subMesh_fList.push_back(temp_f);
						numOfF++;
					}
				}
				else if (slash_cnt == 8) {
					if (slash_idx[1] - slash_idx[0] == 1) {
						sscanf(line.c_str(), "%i//%i %i//%i %i//%i %i//%i",
							&vertexNumber[0], &normalNumber[0],
							&vertexNumber[1], &normalNumber[1],
							&vertexNumber[2], &normalNumber[2],
							&vertexNumber[3], &normalNumber[3]);
						vertexNumber[0] -= 1; vertexNumber[1] -= 1; vertexNumber[2] -= 1; vertexNumber[3] -= 1;
						normalNumber[0] -= 1; normalNumber[1] -= 1; normalNumber[2] -= 1; vertexNumber[3] -= 1;
						for (int i = 0; i < 4; i++) {
							temp_f.face_vList.push_back(vList[vertexNumber[i]]);
							temp_f.face_nList.push_back(nList[normalNumber[i]]);
						}
						Mesh_subList[numOfSub - 1].subMesh_fList.push_back(temp_f);
						numOfF++;
					}
					else if (slash_idx[1] - slash_idx[0] != 1) {
						sscanf(line.c_str(), "%i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i",
							&vertexNumber[0], &uvNumber[0], &normalNumber[0],
							&vertexNumber[1], &uvNumber[1], &normalNumber[1],
							&vertexNumber[2], &uvNumber[2], &normalNumber[2],
							&vertexNumber[3], &uvNumber[3], &normalNumber[3]);
						vertexNumber[0] -= 1; vertexNumber[1] -= 1; vertexNumber[2] -= 1; vertexNumber[3] -= 1;
						uvNumber[0] -= 1; uvNumber[1] -= 1; uvNumber[2] -= 1; uvNumber[3] -= 1;
						normalNumber[0] -= 1; normalNumber[1] -= 1; normalNumber[2] -= 1; normalNumber[3] -= 1;
						for (int i = 0; i < 4; i++) {
							temp_f.face_vList.push_back(vList[vertexNumber[i]]);
							temp_f.face_nList.push_back(nList[normalNumber[i]]);
							temp_f.face_uvList.push_back(uvList[uvNumber[i]]);
						}
						Mesh_subList[numOfSub - 1].subMesh_fList.push_back(temp_f);
						numOfF++;
					}
				}
				else if (slash_cnt == 16) {
					if (slash_idx[1] - slash_idx[0] != 1) {
						sscanf(line.c_str(), "%i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i",
							&vertexNumber[0], &uvNumber[0], &normalNumber[0],
							&vertexNumber[1], &uvNumber[1], &normalNumber[1],
							&vertexNumber[2], &uvNumber[2], &normalNumber[2],
							&vertexNumber[3], &uvNumber[3], &normalNumber[3],
							&vertexNumber[4], &uvNumber[4], &normalNumber[4],
							&vertexNumber[5], &uvNumber[5], &normalNumber[5],
							&vertexNumber[6], &uvNumber[6], &normalNumber[6],
							&vertexNumber[7], &uvNumber[7], &normalNumber[7]);
						for (int i = 0; i < 8; i++) {
							vertexNumber[i] -= 1;
							uvNumber[i] -= 1;
							normalNumber[i] -= 1;
						}
						for (int i = 0; i < 8; i++) {
							temp_f.face_vList.push_back(vList[vertexNumber[i]]);
							temp_f.face_nList.push_back(nList[normalNumber[i]]);
							temp_f.face_uvList.push_back(uvList[uvNumber[i]]);
						}
						Mesh_subList[numOfSub - 1].subMesh_fList.push_back(temp_f);
						numOfF++;
					}
				}
				else if (slash_cnt == 32) {
					if (slash_idx[1] - slash_idx[0] != 1) {
						sscanf(line.c_str(), "%i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i %i/%i/%i",
							&vertexNumber[0], &uvNumber[0], &normalNumber[0],
							&vertexNumber[1], &uvNumber[1], &normalNumber[1],
							&vertexNumber[2], &uvNumber[2], &normalNumber[2],
							&vertexNumber[3], &uvNumber[3], &normalNumber[3],
							&vertexNumber[4], &uvNumber[4], &normalNumber[4],
							&vertexNumber[5], &uvNumber[5], &normalNumber[5],
							&vertexNumber[6], &uvNumber[6], &normalNumber[6],
							&vertexNumber[7], &uvNumber[7], &normalNumber[7],
							&vertexNumber[8], &uvNumber[8], &normalNumber[8],
							&vertexNumber[9], &uvNumber[9], &normalNumber[9],
							&vertexNumber[10], &uvNumber[10], &normalNumber[10],
							&vertexNumber[11], &uvNumber[11], &normalNumber[11],
							&vertexNumber[12], &uvNumber[12], &normalNumber[12],
							&vertexNumber[13], &uvNumber[13], &normalNumber[13],
							&vertexNumber[14], &uvNumber[14], &normalNumber[14],
							&vertexNumber[15], &uvNumber[15], &normalNumber[15]);
						for (int i = 0; i < 16; i++) {
							vertexNumber[i] -= 1;
							uvNumber[i] -= 1;
							normalNumber[i] -= 1;
						}
						for (int i = 0; i < 16; i++) {
							temp_f.face_vList.push_back(vList[vertexNumber[i]]);
							temp_f.face_nList.push_back(nList[normalNumber[i]]);
							temp_f.face_uvList.push_back(uvList[uvNumber[i]]);
						}
						Mesh_subList[numOfSub - 1].subMesh_fList.push_back(temp_f);
						numOfF++;
					}
				}
			}
		}
		objFile.close();
		//sub mesh texture mapping confirm process
		for (int i = 0; i < Mesh_subList.size(); i++) {
			if (Mesh_subList[i].loadFile.size() != 0) {
				texFileList.push_back(Mesh_subList[i].loadFile);
				subTexArr[subTex_arrIdx++] = Mesh_subList[i].sub_texture;
			}
		}
		//collect texture mapping
		if (texFileList.size() != 0) {
			glGenTextures(subTex_arrIdx, subTexArr);
			for (int i = 0; i < subTex_arrIdx; i++)
				LoadMeshFromFile(texFileList[i], subTexArr[i]);
		}
	}
	else {
		cout << "Unable to open obj file";
	}
	return 0;
}

void Model_OBJ_Texture::Draw()
{
	int arrIdx = 0;
	for (int i = 0; i < Mesh_subList.size(); i++) {
		if (Mesh_subList[i].loadFile.size() != 0) {
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, subTexArr[arrIdx++]);
			Mesh_subList[i].setMaterial();
			for (int j = 0; j < Mesh_subList[i].subMesh_fList.size(); j++) {
				if (Mesh_subList[i].subMesh_fList[j].face_vList.size() == VERTEX_PER_TRIANGLE) {
					glBegin(GL_TRIANGLES);
					for (int k = 0; k < VERTEX_PER_TRIANGLE; k++) {
						if (Mesh_subList[i].subMesh_fList[j].face_uvList.size() > 0) {
							glTexCoord2f(Mesh_subList[i].subMesh_fList[j].face_uvList[k].x, Mesh_subList[i].subMesh_fList[j].face_uvList[k].y);
						}
						glNormal3f(Mesh_subList[i].subMesh_fList[j].face_nList[k].x, Mesh_subList[i].subMesh_fList[j].face_nList[k].y, Mesh_subList[i].subMesh_fList[j].face_nList[k].z);
						glVertex3f(Mesh_subList[i].subMesh_fList[j].face_vList[k].x, Mesh_subList[i].subMesh_fList[j].face_vList[k].y, Mesh_subList[i].subMesh_fList[j].face_vList[k].z);
					}
					glEnd();
				}
				else if (Mesh_subList[i].subMesh_fList[j].face_vList.size() == VERTEX_PER_QUAD) {
					glBegin(GL_QUADS);
					for (int k = 0; k < VERTEX_PER_QUAD; k++) {
						if (Mesh_subList[i].subMesh_fList[j].face_uvList.size() > 0) {
							glTexCoord2f(Mesh_subList[i].subMesh_fList[j].face_uvList[k].x, Mesh_subList[i].subMesh_fList[j].face_uvList[k].y);
						}
						glNormal3f(Mesh_subList[i].subMesh_fList[j].face_nList[k].x, Mesh_subList[i].subMesh_fList[j].face_nList[k].y, Mesh_subList[i].subMesh_fList[j].face_nList[k].z);
						glVertex3f(Mesh_subList[i].subMesh_fList[j].face_vList[k].x, Mesh_subList[i].subMesh_fList[j].face_vList[k].y, Mesh_subList[i].subMesh_fList[j].face_vList[k].z);
					}
					glEnd();
				}
				else {
					glBegin(GL_POLYGON);
					for (int k = 0; k < Mesh_subList[i].subMesh_fList[j].face_vList.size(); k++) {
						if (Mesh_subList[i].subMesh_fList[j].face_uvList.size() > 0) {
							glTexCoord2f(Mesh_subList[i].subMesh_fList[j].face_uvList[k].x, Mesh_subList[i].subMesh_fList[j].face_uvList[k].y);
						}
						glNormal3f(Mesh_subList[i].subMesh_fList[j].face_nList[k].x, Mesh_subList[i].subMesh_fList[j].face_nList[k].y, Mesh_subList[i].subMesh_fList[j].face_nList[k].z);
						glVertex3f(Mesh_subList[i].subMesh_fList[j].face_vList[k].x, Mesh_subList[i].subMesh_fList[j].face_vList[k].y, Mesh_subList[i].subMesh_fList[j].face_vList[k].z);
					}
					glEnd();
				}
			}
			glDisable(GL_TEXTURE_2D);
		}
		else {
			Mesh_subList[i].setMaterial();
			for (int j = 0; j < Mesh_subList[i].subMesh_fList.size(); j++) {
				if (Mesh_subList[i].subMesh_fList[j].face_vList.size() == VERTEX_PER_TRIANGLE) {
					glBegin(GL_TRIANGLES);
					for (int k = 0; k < VERTEX_PER_TRIANGLE; k++) {
						glNormal3f(Mesh_subList[i].subMesh_fList[j].face_nList[k].x, Mesh_subList[i].subMesh_fList[j].face_nList[k].y, Mesh_subList[i].subMesh_fList[j].face_nList[k].z);
						glVertex3f(Mesh_subList[i].subMesh_fList[j].face_vList[k].x, Mesh_subList[i].subMesh_fList[j].face_vList[k].y, Mesh_subList[i].subMesh_fList[j].face_vList[k].z);
					}
					glEnd();
				}
				else if (Mesh_subList[i].subMesh_fList[j].face_vList.size() == VERTEX_PER_QUAD) {
					glBegin(GL_QUADS);
					for (int k = 0; k < VERTEX_PER_QUAD; k++) {
						glNormal3f(Mesh_subList[i].subMesh_fList[j].face_nList[k].x, Mesh_subList[i].subMesh_fList[j].face_nList[k].y, Mesh_subList[i].subMesh_fList[j].face_nList[k].z);
						glVertex3f(Mesh_subList[i].subMesh_fList[j].face_vList[k].x, Mesh_subList[i].subMesh_fList[j].face_vList[k].y, Mesh_subList[i].subMesh_fList[j].face_vList[k].z);
					}
					glEnd();
				}
				else {
					glBegin(GL_POLYGON);
					for (int k = 0; k < Mesh_subList[i].subMesh_fList[j].face_vList.size(); k++) {
						glNormal3f(Mesh_subList[i].subMesh_fList[j].face_nList[k].x, Mesh_subList[i].subMesh_fList[j].face_nList[k].y, Mesh_subList[i].subMesh_fList[j].face_nList[k].z);
						glVertex3f(Mesh_subList[i].subMesh_fList[j].face_vList[k].x, Mesh_subList[i].subMesh_fList[j].face_vList[k].y, Mesh_subList[i].subMesh_fList[j].face_vList[k].z);
					}
					glEnd();
				}
			}
		}
	}
}

void Model_OBJ_Texture::Release()
{
}

Model_OBJ_Texture::Face::Face()
{
}

Model_OBJ_Texture::Face::~Face()
{
}

Model_OBJ_Texture::SubMesh::SubMesh()
{
	for (int i = 0; i < 4; i++) {
		Ka[i] = 0;
		Ks[i] = 0;
		Kd[i] = 0;
	}
}

Model_OBJ_Texture::SubMesh::~SubMesh()
{
}

void Model_OBJ_Texture::SubMesh::setMaterial()
{
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, this->Ka);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, this->Kd);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, this->Ks);
}


Model_OBJ_Texture::Vertex::Vertex()
{
	this->x = 0;
	this->y = 0;
	this->z = 0;
}
Model_OBJ_Texture::Vertex::~Vertex()
{
}

void Model_OBJ_Texture::Vertex::operator=(Vertex v)
{
	this->x = v.x;
	this->y = v.y;
	this->z = v.z;
}

Model_OBJ_Texture::UV::UV()
{
	this->x = 0;
	this->y = 0;
}

Model_OBJ_Texture::UV::~UV()
{
}