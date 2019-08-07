
#include <stdint.h>
#include "log.h"
#include "renderer/renderer.h"
#include "scene/Scene.h"

Scene::Scene():objects(),dynamicObjects(),skyBox(0){
	
}

void Scene::Update(float delta){
	for(uint32_t i=0;i<dynamicObjects.size();i++){
		dynamicObjects[i]->Update(delta);
	}
}

void Scene::Draw(IRenderer *r){
	for(uint32_t i=0;i<objects.size();i++){
		objects[i]->Draw(r);
	}
	
	for(uint32_t i=0;i<dynamicObjects.size();i++){
		dynamicObjects[i]->Draw(r);
	}
}

void Scene::AddObject(SceneObject *obj){
	if(obj->dynamic)
		dynamicObjects.push_back(obj);
	else
		objects.push_back(obj);
}

void Scene::RemoveObject(SceneObject *obj){
	for(std::vector<SceneObject*>::iterator it = dynamicObjects.begin();it!=dynamicObjects.end();it++){
		if(*it==obj){
			dynamicObjects.erase(it);
			return;
		}
	}
	Log("RemoveObject(%p) can't find\n",obj);
}

SceneObject *Scene::Find(const char *name){
	for(std::vector<SceneObject*>::iterator it = dynamicObjects.begin();it!=dynamicObjects.end();it++){
		if((*it)->name==name){
			return *it;
		}
	}
	Log("Find(%s) can't find\n",name);
	return 0;
}

void Scene::AddLight(LightObject *l){
	lights.push_back(l);
}

StaticModel::StaticModel():SceneObject(){
	dynamic = false;
	mdl = 0;
}

void StaticModel::Draw(IRenderer *r){
	if(!mdl)
		return;
	r->DrawModel(mdl,modelMtx);
}
