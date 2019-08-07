
#include "tpsPhysics.h"
#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <BulletCollision/BroadphaseCollision/btCollisionAlgorithm.h>
#include <LinearMath/btIDebugDraw.h>
#include "log.h"
#include "graphics/platform_gl.h"
#include "renderer/renderer.h"

using glm::vec3;

void neMotionState::getWorldTransform(btTransform &centerOfMassWorldTrans) const{
	if(!obj)
		return;
	centerOfMassWorldTrans.setFromOpenGLMatrix((float*)&obj->modelMtx);
}

void neMotionState::setWorldTransform(const btTransform &centerOfMassWorldTrans){
	if(!obj)
		return;
	centerOfMassWorldTrans.getOpenGLMatrix((float*)&obj->modelMtx);
}


class neDebugDraw: public btIDebugDraw
{
	int mode;
	IRenderer *renderer;

public:
	neDebugDraw(IRenderer *rend):mode(0),renderer(rend){}
	void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color){
		renderer->SetModelMtx(glm::mat4(1.0));

		float verts[6] = {from.x(),from.y(),from.z(), to.x(), to.y(), to.z()};
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, verts);
		renderer->SetColor(color.x(),color.y(),color.z(),1);
		glDrawArrays(GL_LINES,0,2);
	}
	void drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color){
	}
	void reportErrorWarning(const char *warningString){
		Log("Physics: %s\n",warningString);
	}
	void draw3dText(const btVector3 &location, const char *textString){
		//TODO
		Log("Physics 3dtext: %s\n",textString);
	}
	void setDebugMode(int debugMode){
		mode = debugMode;
	}
	int getDebugMode() const{
		return mode;
	}
};

void PhysicsSystem::Init(IRenderer *rend){
	collisionConfig = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfig);
	broadphase = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();
	
	world = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfig);
	world->setGravity(btVector3(0, -9.8, 0));
	if(rend){
		world->setDebugDrawer(new neDebugDraw(rend));
		world->getDebugDrawer()->setDebugMode(1);
	}
	Log("PhysicsSystem Init\n");
}

void PhysicsSystem::Update(float deltaTime){
	world->stepSimulation(deltaTime);
}

void PhysicsSystem::AddBox(vec3 size, SceneObject *so,float mass){
	btCollisionShape* shape = new btBoxShape(*((btVector3*)&size.x));
	shapes.push_back(shape);

	btVector3 inertia(0, 0, 0);
	//if(mass>0)
		shape->calculateLocalInertia(mass, inertia);

	btMotionState* motionState = new neMotionState(so);
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, motionState, shape, inertia);
	btRigidBody *body = new btRigidBody(rigidBodyCI);

	world->addRigidBody(body);
}

PlayerPhysics *PhysicsSystem::CreatePlayer(float height, float radius,glm::mat4 mtx){
	btCapsuleShape *shape = new btCapsuleShape(radius,height);
	//btBoxShape *shape = new btBoxShape(btVector3(radius,height/2+radius,radius));
	shapes.push_back(shape);

	btTransform transform;
	transform.setFromOpenGLMatrix((float*)&mtx);

	btPairCachingGhostObject *charGhostObj = new btPairCachingGhostObject();
	charGhostObj->setWorldTransform(transform);
	charGhostObj->setCollisionShape(shape);
	charGhostObj->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);

	PlayerPhysics *player = new PlayerPhysics(shape, charGhostObj, 50, mtx);

	broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
	world->addCollisionObject(charGhostObj);
	world->addAction(player);

	return player;
}

//=============================Player=============================

vec3 PlayerPhysics::getPosition() const
{
	return *((vec3*)&curPos);
}

void PlayerPhysics::updateAction(btCollisionWorld *collisionWorld, btScalar deltaTimeStep)
{
	preStep(collisionWorld);
	playerStep(collisionWorld, deltaTimeStep);
}

void PlayerPhysics::setWalkDirection(const btVector3 &walkDirection)
{
	walkDir = walkDirection;
}

void PlayerPhysics::debugDraw(btIDebugDraw *debugDrawer)
{
	//Log("PlayerPhysics::debugDraw\n");
}

void PlayerPhysics::setVelocityForTimeInterval(const btVector3 &velocity, btScalar timeInterval)
{
	Log("PlayerPhysics::setVelocityForTimeInterval\n");
}

void PlayerPhysics::reset(btCollisionWorld *collisionWorld)
{
	Log("PlayerPhysics::reset\n");
}

void PlayerPhysics::warp(const btVector3 &origin){
	btTransform xform;
	xform.setIdentity();
	xform.setOrigin(origin);
	ghostObj->setWorldTransform(xform);
}

btVector3 computeReflectionDirection(const btVector3 & direction, const btVector3 & normal){
	return direction - (btScalar(2) * direction.dot(normal)) * normal;
}
btVector3 parallelComponent(const btVector3 & direction, const btVector3 & normal){
	btScalar magnitude = direction.dot(normal);
	return normal * magnitude;
}
btVector3 perpindicularComponent(const btVector3 & direction, const btVector3 & normal){
	return direction - parallelComponent(direction, normal);
}

void PlayerPhysics::stepUp(btCollisionWorld *collisionWorld){

	targetPos = curPos + btVector3(0,stepHeight+(vertOffs > 0 ? vertOffs : 0),0);

	btTransform start, end;
	start.setIdentity();
	end.setIdentity();
	start.setOrigin(curPos);
	end.setOrigin(targetPos);

	ClosestNotMeConvexResultCallback callback(ghostObj,btVector3(0,1,0),0.4);
	callback.m_collisionFilterGroup = ghostObj->getBroadphaseHandle()->m_collisionFilterGroup;
	callback.m_collisionFilterMask = ghostObj->getBroadphaseHandle()->m_collisionFilterMask;

	ghostObj->convexSweepTest(shape,start,end,callback,collisionWorld->getDispatchInfo().m_allowedCcdPenetration);

	if(callback.hasHit()){
		Log("Player hit ceiling\n");
		if (callback.m_hitNormalWorld.dot(btVector3(0,1,0)) > 0){
			curPos.setInterpolate3(curPos, targetPos, callback.m_closestHitFraction);
			curStepOffs = stepHeight * callback.m_closestHitFraction;
		}
		vertOffs = 0;
		vertVel = 0;
	}else{
		curPos = targetPos;
		curStepOffs = stepHeight;
	}

}

void PlayerPhysics::stepForward(btCollisionWorld *collisionWorld, btScalar dt){

	targetPos = curPos + walkDir*dt;

	btTransform start, end;
	start.setIdentity();
	end.setIdentity();

	btScalar fraction = 1.0;
	//btScalar distance2 = (curPos - targetPos).length2();

	/*if (touchingContact){
		//broke stairs
	//	if (walkDir.normalize().dot(touchingNormal) > 0)
	//		updateTargetPositionBasedOnCollision(touchingNormal);
	}*/

	//int maxIter = 10;
	//while (fraction > 0.01 && maxIter-- > 0)
	{
		start.setOrigin(curPos);
		end.setOrigin(targetPos);

		ClosestNotMeConvexResultCallback callback(ghostObj,curPos-targetPos,0);
		callback.m_collisionFilterGroup = ghostObj->getBroadphaseHandle()->m_collisionFilterGroup;
		callback.m_collisionFilterMask = ghostObj->getBroadphaseHandle()->m_collisionFilterMask;

		ghostObj->convexSweepTest(shape,start,end,callback,collisionWorld->getDispatchInfo().m_allowedCcdPenetration);

		fraction -= callback.m_closestHitFraction;

		if(callback.hasHit()){
			updateTargetPositionBasedOnCollision(callback.m_hitNormalWorld);
/*
			btVector3 currentDir = targetPos - curPos;
			distance2 = currentDir.length2();

			if (distance2 > SIMD_EPSILON)
			{
				currentDir.normalize();

				if (currentDir.dot(walkDir.normalize()) <= 0)
					break;
			}
			else
				break;*/
		}else{
			curPos = targetPos;
		}
	}
}

void PlayerPhysics::stepDown(btCollisionWorld *collisionWorld, btScalar dt){

	float downVel = (vertVel < 0 ? -vertVel : 0) * dt;

	if (downVel > 0 && downVel < stepHeight && (bOnGround || !jumping))
		downVel = stepHeight;

	targetPos -= btVector3(0,curStepOffs+downVel,0);

	btTransform start, end;
	start.setIdentity();
	end.setIdentity();
	start.setOrigin(curPos);
	end.setOrigin(targetPos);

	ClosestNotMeConvexResultCallback callback(ghostObj,btVector3(0,1,0),btCos(btRadians(85)));
	callback.m_collisionFilterGroup = ghostObj->getBroadphaseHandle()->m_collisionFilterGroup;
	callback.m_collisionFilterMask = ghostObj->getBroadphaseHandle()->m_collisionFilterMask;

	ghostObj->convexSweepTest(shape,start,end,callback,collisionWorld->getDispatchInfo().m_allowedCcdPenetration);

	if(callback.hasHit()){
		curPos.setInterpolate3(curPos, targetPos, callback.m_closestHitFraction);
		vertVel = 0;
		vertOffs = 0;
		jumping = false;
	}else{
		curPos = targetPos;
	}
}

void PlayerPhysics::preStep(btCollisionWorld *collisionWorld)
{
	int numPenetrationLoops = 0;
	touchingContact = false;

	while(recoverFromPenetration(collisionWorld))
	{
		numPenetrationLoops++;
		touchingContact = true;

		if(numPenetrationLoops > 4)
			break;
	}

	curPos = ghostObj->getWorldTransform().getOrigin();
	targetPos = curPos;
}

void PlayerPhysics::playerStep(btCollisionWorld *collisionWorld, btScalar dt)
{
	bOnGround = onGround();

	//broken slower movement
	//setRBForceImpulseBasedOnCollision(dt);

	vertVel -= 9.8*dt;//gravity

	//TODO clamp vertVel (jump, fall)

	vertOffs = vertVel*dt;

	stepUp(collisionWorld);

	stepForward(collisionWorld,dt);

	stepDown(collisionWorld,dt);

	btTransform transf = ghostObj->getWorldTransform();
	transf.setOrigin(curPos);
	ghostObj->setWorldTransform(transf);

}

bool PlayerPhysics::canJump() const
{
	return onGround();
}

void PlayerPhysics::jump(const btVector3 &dir)
{
	if(!canJump())
		return;
	vertVel = 5;
	jumping = true;
}

bool PlayerPhysics::onGround() const
{
	return vertVel == 0 && vertOffs == 0;
}

void PlayerPhysics::setUpInterpolate(bool value)
{
	Log("PlayerPhysics::setUpInterpolate\n");
}

void PlayerPhysics::updateTargetPositionBasedOnCollision(const btVector3 & hitNormal, btScalar tangentMag, btScalar normalMag)
{
	btVector3 movementDirection = targetPos - curPos;
	btScalar movementLenght = movementDirection.length();

	if (movementLenght > SIMD_EPSILON)
	{
		movementDirection.normalize();

		btVector3 reflectDir = computeReflectionDirection(movementDirection, hitNormal);

		btVector3 parallelDir, perpindicularDir;

		parallelDir = parallelComponent(reflectDir, hitNormal);
		perpindicularDir = perpindicularComponent(reflectDir, hitNormal);

		targetPos = curPos;

		if (normalMag != 0)
		{
			btVector3 perpComponent = perpindicularDir * btScalar(normalMag * movementLenght);
			targetPos += perpComponent;
		}
	}
}

void PlayerPhysics::setRBForceImpulseBasedOnCollision(float dt)
{
	if (!walkDir.isZero())
	{
		for (int i = 0; i < ghostObj->getOverlappingPairCache()->getNumOverlappingPairs(); i++)
		{
			btBroadphasePair * collisionPair = &ghostObj->getOverlappingPairCache()->getOverlappingPairArray()[i];

			btRigidBody * rb = (btRigidBody*)collisionPair->m_pProxy1->m_clientObject;

			if (mass > rb->getInvMass())
			{
				btScalar resultMass = mass - rb->getInvMass();
				btVector3 reflection = computeReflectionDirection(walkDir * resultMass * dt, walkDir.normalize());
				rb->applyCentralImpulse(reflection * -1);
			}
		}

	}
}

bool PlayerPhysics::recoverFromPenetration(btCollisionWorld *collisionWorld)
{
	bool penetration = false;

	collisionWorld->getDispatcher()->dispatchAllCollisionPairs(ghostObj->getOverlappingPairCache(), collisionWorld->getDispatchInfo(), collisionWorld->getDispatcher());

	curPos = ghostObj->getWorldTransform().getOrigin();

	//btScalar maxPen = 0;

	btManifoldArray mManifoldArray;
	for (int i = 0; i < ghostObj->getOverlappingPairCache()->getNumOverlappingPairs(); i++)
	{
		mManifoldArray.resize(0);

		btBroadphasePair * collisionPair = &ghostObj->getOverlappingPairCache()->getOverlappingPairArray()[i];

		if (collisionPair->m_algorithm)
			collisionPair->m_algorithm->getAllContactManifolds(mManifoldArray);

		for (int j = 0; j < mManifoldArray.size(); j++)
		{
			btPersistentManifold * manifold = mManifoldArray[j];
			btScalar directionSign = manifold->getBody0() == ghostObj ? btScalar(-1) : btScalar(1);

			for (int p = 0; p < manifold->getNumContacts(); p++)
			{
				const btManifoldPoint & pt = manifold->getContactPoint(p);

				btScalar dist = pt.getDistance();

				if (dist < 0)
				{
					//maxPen = dist;
					touchingNormal = pt.m_normalWorldOnB * directionSign;
					penetration = true;
				}

				curPos += pt.m_normalWorldOnB * directionSign * dist * btScalar(0.2);


			}
		}
	}

	btTransform newTrans = ghostObj->getWorldTransform();
	newTrans.setOrigin(curPos);
	ghostObj->setWorldTransform(newTrans);

	return penetration;
}
