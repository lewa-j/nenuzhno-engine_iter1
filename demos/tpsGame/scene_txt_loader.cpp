
#include <string>
#include <fstream>
#include <cstring>
using namespace std;
#include <gtc/matrix_transform.hpp>
#include "log.h"
#include "system/FileSystem.h"
#include "scene/Scene.h"
#include "renderer/Model.h"
#include "resource/ResourceManager.h"
#include "tpsPhysics.h"
#include "tpsObjects.h"

glm::mat4 PosRotToMtx(glm::vec3 pos, glm::vec3 rot){
	glm::mat4 mtx = glm::translate(glm::mat4(1),pos);
	mtx = glm::rotate(mtx,glm::radians(rot.x),glm::vec3(1,0,0));
	mtx = glm::rotate(mtx,glm::radians(rot.y),glm::vec3(0,1,0));
	mtx = glm::rotate(mtx,glm::radians(rot.z),glm::vec3(0,0,1));
	return mtx;

	//vbsp
	/*glm::mat4 m = glm::translate(glm::mat4(1.0), prop->m_Origin);
	m = glm::rotate(m, glm::radians(prop->m_Angles.y), glm::vec3(0,0,1));
	m = glm::rotate(m, glm::radians(prop->m_Angles.x), glm::vec3(0,1,0));
	m = glm::rotate(m, glm::radians(prop->m_Angles.z), glm::vec3(1,0,0));*/
	//star-fleet
	/*	modelMtx = glm::translate(glm::mat4(1),position);
	modelMtx = glm::rotate(modelMtx,glm::radians(angles.y),glm::vec3(0,1,0));
	modelMtx = glm::rotate(modelMtx,glm::radians(angles.x),glm::vec3(1,0,0));*/
	//ME3
	/*	modelMtx = glm::mat4(1.0);
	modelMtx = glm::translate(modelMtx,location);
	modelMtx = glm::rotate(modelMtx,rotation.y,glm::vec3(0,0,1));
	modelMtx = glm::rotate(modelMtx,rotation.x,glm::vec3(0,-1,0));
	modelMtx = glm::rotate(modelMtx,rotation.z,glm::vec3(-1,0,0));
	modelMtx = glm::scale(modelMtx,scale);*/
}

glm::vec3 ReadVec3(ifstream &file){
	glm::vec3 v;
	file >> v.x;
	file >> v.y;
	file >> v.z;
	return v;
}

void LoadTXTSceneV1(ifstream &file, Scene *scene,ResourceManager *resMan,PhysicsSystem *physics);

Scene *LoadTXTScene(const char *fileName,ResourceManager *resMan,PhysicsSystem *physics)
{
	string filePath = "scenes/"+string(fileName)+".txt";
	char path[256];
	g_fs.GetFilePath(filePath.c_str(), path);
	ifstream file(path);
	if(!file){
		Log("Scene file not found: %s\n", path);
		return 0;
	}
	Log("Loading scene (%s)\n",fileName);

	Scene *scene = new Scene();

	int ver = 0;
	file >> ver;
	
	if(ver==1){
		LoadTXTSceneV1(file,scene,resMan,physics);
	}else{
		Log("Unknown scene version %d (%s)\n",ver,fileName);
	}
	file.close();

	return scene;
}

void LoadTXTSceneV1(ifstream &file, Scene *scene,ResourceManager *resMan,PhysicsSystem *physics)
{
	scene->sunDirection = glm::normalize(vec3(0.3,0.4,0.7));
	
	scene->startPos = ReadVec3(file);
	scene->startRot = ReadVec3(file);

	int numObjects = 0;
	file >> numObjects;

	char buff[1024]={0};
	glm::vec3 pos(0),rot(0);
	for(int i=0;i<numObjects;i++){
		file >> buff;
		if(strcmp(buff,"StaticModel")==0){
			file >> buff;
			Model *mdl = resMan->GetModel(buff);
			if(!mdl)
				Log("Error loading scene: Model not loaded (%s)\n",buff);
			pos = ReadVec3(file);
			rot = ReadVec3(file);
			int collType = 0;
			file >> collType;

			StaticModel *obj = new StaticModel();
			obj->mdl = mdl;
			obj->modelMtx = PosRotToMtx(pos,rot);

			if(collType==1){//box
				//boxSize
				physics->AddBox(ReadVec3(file),obj);
			}
			
			scene->AddObject(obj);
		}else if(strcmp(buff,"Collider")==0){
			pos = ReadVec3(file);
			rot = ReadVec3(file);
			int collType = 0;
			file >> collType;

			SceneObject *obj = new SceneObject();
			obj->modelMtx = PosRotToMtx(pos,rot);

			if(collType==1){//box
				//boxSize
				physics->AddBox(ReadVec3(file),obj);
			}
			scene->AddObject(obj);
		}else if(strcmp(buff,"Button")==0){
			pos = ReadVec3(file);
			ButtonObject *obj = new ButtonObject(pos);
			file >> buff;
			obj->name = buff;
			file >> buff;
			obj->text = buff;
			file >> buff;
			if(strcmp(buff,"tp")==0){
				obj->type = BUTTON_TP;
				obj->targetPos = ReadVec3(file);
				obj->targetRot = ReadVec3(file);
			}else{
				obj->type = BUTTON_ACTIVE;
				file >> buff;
				obj->target = (ActivatingObject*)scene->Find(buff);
			}
			obj->mdl = g_mdlBox;
			scene->AddObject(obj);
		}else if(strcmp(buff,"Door")==0){
			pos = ReadVec3(file);
			file >> buff;
			DoorObject *obj = new DoorObject(pos,g_mdlBox,ReadVec3(file));
			obj->name = buff;
			scene->AddObject(obj);
		}else if(strcmp(buff,"Light")==0){
			int type = 0;
			file >> type;
			pos = ReadVec3(file);
			vec3 col = ReadVec3(file);
			float radius = 5;
			file >> radius;
			LightObject *l = new LightObject((eLightType)type,pos,col,radius);
			scene->AddLight(l);
		/*}else if(strcmp(buff,"Plane")==0){
			pos = ReadVec3(file);
			rot = ReadVec3(file);
			*/
		}else{
			Log("Error loading scene: invalid object type (%s)\n",buff);
		}
	}
}
