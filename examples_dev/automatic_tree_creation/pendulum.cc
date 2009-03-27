#include "pendulum.h"
#include "mbsim/rigid_body.h"
#include "special_classes.h"

#ifdef HAVE_AMVIS
#include "objobject.h"

using namespace AMVis;
#endif
#ifdef HAVE_AMVISCPPINTERFACE
#include "amviscppinterface/objobject.h"
#endif

Pendulum::Pendulum(const string &projectName) : MultiBodySystem(projectName) {

  setProjectDirectory("plot");

  Vec grav(3);
  grav(1)=-9.81;
  setAccelerationOfGravity(grav);

  SpecialGroup *tree = new SpecialGroup("MehrfachPendel"); 
  addSubsystem(tree,Vec(3),SqrMat(3,EYE));

  double mStab = 0.2;
  double lStab = 0.3;
  double JStab = 1.0/12.0 * mStab * lStab * lStab; 
  double a1 = 0.15*lStab;
  double a2 = 0.15*lStab;

  Vec KrRP(3), KrCR(3);
  SymMat Theta(3);

  SpecialRigidBody* stab1 = new SpecialRigidBody("Stab1");
  tree->addObject(stab1);
  //Node* node = tree->addObject(0,stab1);
  KrCR(0) = a1;

  stab1->addFrame("R",KrCR,SqrMat(3,EYE));

  stab1->setFrameOfReference(getFrame("I"));
  stab1->setFrameForKinematics(stab1->getFrame("R"));

  stab1->setMass(mStab);
  Theta(2,2) = JStab;
  stab1->setInertiaTensor(Theta);
  stab1->setRotation(new RotationAboutFixedAxis(Vec("[0;0;1]")));

#ifdef HAVE_AMVISCPPINTERFACE
  AMVis::ObjObject *obj=new AMVis::ObjObject;
  obj->setObjFileName("objects/pendel1.obj");
  obj->setScaleFactor(0.1*0.3);
  obj->setInitialRotation(0,0,M_PI/2);
  obj->setNormals(AMVis::ObjObject::smoothIfLessBarrier);
  obj->setEpsVertex(1e-5);
  obj->setEpsNormal(1e-5);
  obj->setSmoothBarrier(M_PI*2/9);
  stab1->setAMVisRigidBody(obj);
#endif

  SpecialRigidBody* stab2 = new SpecialRigidBody("Stab2");
  tree->addObject(stab2);
  //tree->addObject(node,stab2);
  KrRP(0) = lStab/2;
  KrRP(2) = 0.006;
  stab1->addFrame("P",KrRP,SqrMat(3,EYE),stab1->getFrame("R"));
  KrCR(0) = a2;
  stab2->addFrame("R",-KrCR,SqrMat(3,EYE));
  stab2->setFrameOfReference(stab1->getFrame("P"));
  stab2->setFrameForKinematics(stab2->getFrame("R"));
  stab2->setMass(mStab);
  Theta(2,2) = JStab;
  stab2->setInertiaTensor(Theta,stab2->getFrame("C"));
  stab2->setRotation(new RotationAboutFixedAxis(Vec("[0;0;1]")));
  stab2->setq0(Vec("[-1.6]"));

#ifdef HAVE_AMVISCPPINTERFACE
  obj=new AMVis::ObjObject;
  obj->setObjFileName("objects/pendel2.obj");
  obj->setScaleFactor(0.1*0.3);
  obj->setInitialRotation(0,0,M_PI/2);
  obj->setNormals(AMVis::ObjObject::smoothIfLessBarrier);
  obj->setEpsVertex(1e-5);
  obj->setEpsNormal(1e-5);
  obj->setSmoothBarrier(M_PI*2/9);
  stab2->setAMVisRigidBody(obj);
#endif

  SpecialRigidBody* stab3 = new SpecialRigidBody("Stab3");
  tree->addObject(stab3);
  //tree->addObject(node,stab3);
  KrRP(0) = lStab/2;
  KrRP(2) = 0.006;
  stab2->addFrame("P",KrRP,SqrMat(3,EYE),stab2->getFrame("R"));
  KrCR(0) = a2;
  stab3->addFrame("R",-KrCR,SqrMat(3,EYE));
  stab3->setFrameOfReference(stab2->getFrame("P"));
  stab3->setFrameForKinematics(stab3->getFrame("R"));
  stab3->setMass(mStab);
  Theta(2,2) = JStab;
  stab3->setInertiaTensor(Theta,stab3->getFrame("C"));
  stab3->setRotation(new RotationAboutFixedAxis(Vec("[0;0;1]")));
  stab3->setq0(Vec("[-1.6]"));

#ifdef HAVE_AMVISCPPINTERFACE
  obj=new AMVis::ObjObject;
  obj->setObjFileName("objects/pendel2.obj");
  obj->setScaleFactor(0.1*0.3);
  obj->setInitialRotation(0,0,M_PI/2);
  obj->setNormals(AMVis::ObjObject::smoothIfLessBarrier);
  obj->setEpsVertex(1e-5);
  obj->setEpsNormal(1e-5);
  obj->setSmoothBarrier(M_PI*2/9);
  stab3->setAMVisRigidBody(obj);
#endif

  SpecialRigidBody* stab4 = new SpecialRigidBody("Stab4");
  tree->addObject(stab4);
  //tree->addObject(node,stab4);
  KrRP(0) = lStab/2;
  KrRP(2) = 0.006;
  KrCR(0) = a2;
  stab4->setFrameOfReference(getFrame("I"));
  stab4->setFrameForKinematics(stab4->getFrame("C"));
  stab4->setMass(mStab);
  Theta(2,2) = JStab;
  stab4->setInertiaTensor(Theta,stab4->getFrame("C"));
  stab4->setRotation(new RotationAboutFixedAxis(Vec("[0;0;1]")));
  stab4->setu0(Vec("[-1.6]"));

#ifdef HAVE_AMVISCPPINTERFACE
  obj=new AMVis::ObjObject;
  obj->setObjFileName("objects/pendel2.obj");
  obj->setScaleFactor(0.1*0.3);
  obj->setInitialRotation(0,0,M_PI/2);
  obj->setNormals(AMVis::ObjObject::smoothIfLessBarrier);
  obj->setEpsVertex(1e-5);
  obj->setEpsNormal(1e-5);
  obj->setSmoothBarrier(M_PI*2/9);
  stab4->setAMVisRigidBody(obj);
#endif


}

