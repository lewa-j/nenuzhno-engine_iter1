
#pragma once

#include <vector>
#include <LinearMath/btMotionState.h>
#include <LinearMath/btTransform.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletCollision/CollisionDispatch/btCollisionDispatcher.h>
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletDynamics/Character/btCharacterControllerInterface.h>
#include <mat4x4.hpp>
#include <vec3.hpp>
#include "scene/Scene.h"

class IRenderer;

class neMotionState: public btMotionState{
public:
	SceneObject *obj;
	
	neMotionState():obj(0){
	}
	neMotionState(SceneObject *so):obj(so){
	}

	virtual void getWorldTransform(btTransform& centerOfMassWorldTrans ) const;
	virtual void setWorldTransform(const btTransform& centerOfMassWorldTrans);
};

class PlayerPhysics;

class PhysicsSystem{
public:
	PhysicsSystem(){}

	void Init(IRenderer *rend);
	void Update(float deltaTime);
	void AddBox(glm::vec3 size, SceneObject *so,float mass=0);
	btRigidBody *CreateRigidBody(float mass, btTransform tr, btCollisionShape *shape);
	PlayerPhysics *CreatePlayer(float height,float radius, glm::mat4 mtx);
	
	btCollisionConfiguration *collisionConfig;
	btDispatcher *dispatcher;
	btBroadphaseInterface *broadphase;
	btConstraintSolver *solver;
	btDynamicsWorld *world;
	
	std::vector<btCollisionShape*> shapes;
};

class PlayerPhysics: public btCharacterControllerInterface{
public:
	PlayerPhysics(btConvexShape *shp, btPairCachingGhostObject *go, float aMass, glm::mat4 m):
		shape(shp),ghostObj(go),mtx(m),stepHeight(0.5f),mass(aMass),walkDir(0,0,0),vertVel(0),
		vertOffs(0),curStepOffs(0),bOnGround(false),jumping(false){ curPos=*((btVector3*)(&m[3])); }

	btConvexShape *shape;
	btPairCachingGhostObject *ghostObj;
	glm::mat4 mtx;

	glm::vec3 getPosition() const;
	bool recoverFromPenetration(btCollisionWorld * collisionWorld);
	void updateTargetPositionBasedOnCollision(const btVector3 & hitNormal, btScalar tangentMag = 0, btScalar normalMag = 1);
	void setRBForceImpulseBasedOnCollision(float dt);

	void stepUp(btCollisionWorld *collisionWorld);
	void stepForward(btCollisionWorld *collisionWorld, btScalar dt);
	void stepDown(btCollisionWorld *collisionWorld, btScalar dt);

	void updateAction(btCollisionWorld *collisionWorld, btScalar deltaTimeStep);
	void debugDraw(btIDebugDraw *debugDrawer);
	void setWalkDirection(const btVector3 &walkDirection);
	void setVelocityForTimeInterval(const btVector3 &velocity, btScalar timeInterval);
	void reset(btCollisionWorld *collisionWorld);
	void warp(const btVector3 &origin);
	void preStep(btCollisionWorld *collisionWorld);
	void playerStep(btCollisionWorld *collisionWorld, btScalar dt);
	bool canJump() const;
	void jump(const btVector3 &dir);
	bool onGround() const;
	void setUpInterpolate(bool value);

	float stepHeight;
	float mass;
	btVector3 curPos;
	btVector3 targetPos;
	btVector3 walkDir;
	bool touchingContact;
	btVector3 touchingNormal;
	float vertVel;
	float vertOffs;
	float curStepOffs;
	bool bOnGround;
	bool jumping;

	class ClosestNotMeConvexResultCallback : public btCollisionWorld::ClosestConvexResultCallback
	{
		private:
			btCollisionObject * mMe;
			const btVector3 mUp;
			btScalar mMinSlopeDot;

		public:
			ClosestNotMeConvexResultCallback(btCollisionObject * me, const btVector3 & up, btScalar minSlopeDot) :
				btCollisionWorld::ClosestConvexResultCallback(btVector3(0, 0, 0), btVector3(0, 0, 0)),
				mMe(me),
				mUp(up),
				mMinSlopeDot(minSlopeDot)
			{}

			btScalar addSingleResult(btCollisionWorld::LocalConvexResult & convexResult, bool normalInWorldSpace)
			{
				if (convexResult.m_hitCollisionObject == mMe)
					return 1.0;

				btVector3 hitNormalWorld;

				if (normalInWorldSpace)
					hitNormalWorld = convexResult.m_hitNormalLocal;
				else
					hitNormalWorld = convexResult.m_hitCollisionObject->getWorldTransform().getBasis() * convexResult.m_hitNormalLocal;

				btScalar dotUp = mUp.dot(hitNormalWorld);

				if (dotUp < mMinSlopeDot)
					return 1.0;

				return btCollisionWorld::ClosestConvexResultCallback::addSingleResult(convexResult, normalInWorldSpace);
			}
	};
};

