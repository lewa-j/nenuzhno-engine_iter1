
#include <fstream>
#include <vector>
#include "stdlib.h"
#include "string.h"
using namespace std;
#include <vec3.hpp>
#include <vec2.hpp>
#include <geometric.hpp>
#include "log.h"
#include "system/FileSystem.h"
#include "graphics/platform_gl.h"
#include "renderer/mesh.h"

glm::vec3 atovec3(const char *str)
{
	glm::vec3 out(0);
	if(str[0]=='[')
		str++;
	if(str[0]==' ')
		str++;
	/*
	out.x = atof(str);
	str = strstr(str, " ")+1;
	out.y = atof(str);
	str = strstr(str, " ")+1;
	out.z = atof(str);
	*/
	sscanf(str, "%f %f %f", &out.x, &out.y, &out.z);

	return out;
}

glm::vec2 atovec2(char *str)
{
	glm::vec2 out(0);
	if(str[0]=='[')
		str++;
	if(str[0]==' ')
		str++;
	/*
	out.x = atof(str);
	str = strstr(str, " ")+1;
	out.y = atof(str);
	*/
	sscanf(str, "%f %f", &out.x, &out.y);

	return out;
}

struct vert_t
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;
};

Mesh* LoadObjFile(const char* fileName, bool vbo)
{
	string filePath = "models/"+string(fileName)+".obj";
	IFile *file = g_fs.Open(filePath.c_str());
	if(!file){
		//Log("File: %s not found!\n", path);
		return 0;
	}
	
	vector<glm::vec3> verts;
	vector<glm::vec3> normals;
	vector<glm::vec2> uvs;
	vector<int> faces;
	
	string line;
	while(!file->eof())
	{
		file->GetLine(line);
		if(line[0]=='#')
			continue;
		if(line.empty())
			continue;
		
		if(line[0]=='v')
		{
			if(line[1]==' ')
			{
				verts.push_back(atovec3(&line[2]));
				continue;
			}
			if(line[1]=='n')
			{
				normals.push_back(atovec3(&line[3]));
				continue;
			}
			if(line[1]=='t')
			{
				uvs.push_back(atovec2(&line[3]));
				continue;
			}
		}
		
		if(line[0]=='f')
		{
			int ofs=2;
			int nf=0;
			int f1v,f1t,f1n, f2v,f2t,f2n, f3v,f3t,f3n;
			while((unsigned int)ofs-1!=string::npos)
			{
				int attribs = sscanf(line.c_str()+ofs,"%d/%d/%d",&f1v,&f1t,&f1n);
				if(attribs<2){
					attribs = sscanf(line.c_str()+ofs,"%d//%d",&f1v,&f1n);
					f1t=0;
				}
				ofs=line.find(' ',ofs)+1;
				if(nf==0)
				{
					f2v=f1v;
					f2t=f1t;
					f2n=f1n;
				}
				if(nf>2)
				{
					faces.push_back(f2v);
					faces.push_back(f2t);
					faces.push_back(f2n);
					faces.push_back(f3v);
					faces.push_back(f3t);
					faces.push_back(f3n);
					//Log("nf %d ",nf);
				}
				faces.push_back(f1v);
				faces.push_back(f1t);
				faces.push_back(f1n);
				f3v=f1v;
				f3n=f1n;
				f3t=f1t;
				nf++;
			}
			continue;
		}
		
		//Log("%s\n",line.c_str());
	}
	g_fs.Close(file);
	Log("load %s: %d verts %d normals %d uvs %d faces\n",fileName,verts.size(),normals.size(),uvs.size(),faces.size()/9);

	bool genNormals = (normals.size()==0);
	int numVerts = faces.size()/3;
	int numTris = numVerts/3;
	float *outVerts = new float[numVerts*8];
	//Log("outVerts = %p\n",outVerts);
	glm::vec3 normal(0.0f);
	for(int t=0; t<numTris; t++)
	{
		//Log("Gen tri %d\n",t);
		if(genNormals)
		{
			normal = glm::normalize(glm::cross( verts[faces[(t*3+0)*3]-1] - verts[faces[(t*3+1)*3]-1],
												verts[faces[(t*3+0)*3]-1] - verts[faces[(t*3+2)*3]-1]));
		}
		for(int i=0; i<3; i++)
		{
			vert_t *v = (vert_t*)(outVerts+((t*3+i)*8));
			//Log("out vert = %p\n",v);

			v->pos = verts[faces[(t*3+i)*3]-1];
			//Log("Set pos (%f %f %f)\n",verts[faces[(t*3+i)*3]-1].x,verts[faces[(t*3+i)*3]-1].y,verts[faces[(t*3+i)*3]-1].z);
			if(genNormals)
				v->normal = normal;
			else
				v->normal = glm::normalize(normals[faces[(t*3+i)*3+2]-1]);
			//Log("Set normal (%f %f %f)\n",normals[faces[(t*3+i)*3+1]-1].x,normals[faces[(t*3+i)*3+1]-1].y,normals[faces[(t*3+i)*3+1]-1].z);

			int uvId = faces[(t*3+i)*3+1]-1;
			if(uvId>0)
				v->uv = uvs[uvId];
			else
				v->uv = glm::vec2(0);
			//Log("Set uv (%f %f)\n",uvs[faces[(t*3+i)*3+2]-1].x,uvs[faces[(t*3+i)*3+2]-1].y);

			//if((t*3+i)<24)
			//	Log("vert %d %f %f %f\n",(t*3+i),v->pos.x,v->pos.y,v->pos.z);
		}
	}
	
	return new Mesh_N3_T2(outVerts, numVerts, GL_TRIANGLES);
}

