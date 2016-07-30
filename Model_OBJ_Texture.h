#pragma once
#include<vector>
#include<iostream>
//#include<glew.h>
#include<gl/glut.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include<string>
using namespace std;
class Model_OBJ_Texture
{
public:
	/*float* normals;		//stores the normal
	float* Faces_Triangles;	//stores the triangles
	float* Faces_Triangles_vertex_normal;
	float* Faces_Triangles_uv;
	float* Faces_Quads;
	float* Faces_Quads_vertex_normal;
	float* Faces_Quads_uv;
	float* vertexBuffer;	//stores the points which make the object
	float* normalBuffer;	//stroes the normal which make the object
	float* uvBuffer;
	long TotalConnectedPoints;	//stores the total number of connected verteces
	long TotalConnectedQuads;
	long TotalConnectedTriangles; //stores the total numer of connected triangles
	long TotalConnectedVertexNormals;
	long TotalConnectedUvs;*/
	class Vertex {
	public:
		float x;
		float y;
		float z;
		Vertex();
		~Vertex();
		void operator =(Vertex v);
	};
	class UV {
	public:
		float x;
		float y;
		UV();
		~UV();
	};

	class Face {
	public:
		vector<Vertex> face_vList;
		vector<Vertex> face_nList;
		vector<UV> face_uvList;
		Face();
		~Face();
	};
	class SubMesh {
	public:
		vector<Face> subMesh_fList;
		float Kd[4], Ks[4], Ka[4];
		string loadFile;
		GLuint sub_texture;
		SubMesh();
		~SubMesh();
		void setMaterial();
	};
	vector<SubMesh> Mesh_subList;
	vector<Vertex> vList;
	vector<Vertex> nList;
	vector<UV> uvList;
	vector<string> texFileList;
	GLuint subTexArr[20];
	int subTex_arrIdx;

	int numOfV, numOfN, numOfF, numOfSub, numOfUv;
	string MtlFileName;

	Model_OBJ_Texture();
	~Model_OBJ_Texture();
	int Load(char *filename);
	void Draw();
	void Release();
};

bool LoadMeshFromFile(string loadFile, GLuint &tex_id);