/* Copyright (C) 2004-2010 MBSim Development Team
 *
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 *  
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * Lesser General Public License for more details. 
 *  
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Contact: martin.o.foerg@googlemail.com
 */

#ifndef _RIGID_BODY_H_
#define _RIGID_BODY_H_

#include "mbsim/body.h"
#include "fmatvec/fmatvec.h"
#include "fmatvec/function.h"
#include "utils/function_library.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
namespace OpenMBV {
  class RigidBody;
  class Arrow;
}
#endif

namespace MBSim {

  class Frame;
  class Contour;
  class FixedRelativeFrame;
  class CompoundContour;
  class Constraint;

  /**
   * \brief rigid bodies with arbitrary kinematics 
   * \author Martin Foerg
   * \date 2009-04-08 some comments (Thorsten Schindler)
   * \date 2009-07-16 splitted link / object right hand side (Thorsten Schindler)
   * \date 2009-12-14 revised inverse kinetics (Martin Foerg)
   * \date 2010-04-24 class can handle constraints on generalized coordinates (Martin Foerg) 
   * \date 2010-06-20 add getter for Kinematics; revision on doxygen comments (Roland Zander)
   * \todo kinetic energy TODO
   * \todo Euler parameter TODO
   * \todo check if inertial system for performance TODO
   *
   * rigid bodies have a predefined canonic Frame 'C' in their centre of gravity 
   */
  class RigidBody : public Body {
    public:
      /**
       * \brief constructor
       * \param name name of rigid body
       */
      RigidBody(const std::string &name="");

      /**
       * \brief destructor
       */
      virtual ~RigidBody();

      void addDependency(Constraint* constraint_) {
        //body.push_back(body_); 
        constraint = constraint_;
      }

      virtual void updatedq(double t, double dt);
      virtual void updateqd(double t); 
      virtual void updateT(double t);
      virtual void updateh(double t, int j=0);
      virtual void updateh0Fromh1(double t);
      virtual void updateW0FromW1(double t);
      virtual void updateV0FromV1(double t);
      virtual void updatehInverseKinetics(double t, int j=0);
      virtual void updateStateDerivativeDependentVariables(double t);
      virtual void updateM(double t, int i=0) { (this->*updateM_)(t,i); }
      virtual void updateStateDependentVariables(double t) { 
	updateKinematicsForSelectedFrame(t); 
	updateKinematicsForRemainingFramesAndContours(t); 
      }
      virtual void updateJacobians(double t, int j=0) { (this->*updateJacobians_[j])(t); }
      void updateJacobians0(double t) { 
	updateJacobiansForSelectedFrame0(t); 
	updateJacobiansForRemainingFramesAndContours(t,0); 
      }
      void updateJacobians1(double t) { 
        updateJacobiansForRemainingFramesAndContours1(t); 
      }
      virtual void calcqSize();
      virtual void calcuSize(int j=0);

      /* INHERITED INTERFACE OF OBJECT */
      virtual void updateqRef(const fmatvec::Vec& ref);
      virtual void updateuRef(const fmatvec::Vec& ref);
      virtual void init(InitStage stage);
      virtual void initz();
      virtual void facLLM(int i=0) { (this->*facLLM_)(i); }
      virtual void setUpInverseKinetics();
      /*****************************************************/

      /* INHERITED INTERFACE OF ELEMENT */
      virtual std::string getType() const { return "RigidBody"; }
      virtual void plot(double t, double dt=1);
      /*****************************************************/

      /* INTERFACE FOR DERIVED CLASSES */
      /**
       * \brief updates kinematics of kinematic Frame starting from reference Frame
       */
      virtual void updateKinematicsForSelectedFrame(double t);
      /**
       * \brief updates JACOBIAN for kinematics starting from reference Frame
       */
      virtual void updateJacobiansForSelectedFrame0(double t); 

      /**
       * \brief updates kinematics for remaining Frames starting with and from cog Frame
       */
      virtual void updateKinematicsForRemainingFramesAndContours(double t);

      /**
       * \brief updates remaining JACOBIANS for kinematics starting with and from cog Frame
       */
      virtual void updateJacobiansForRemainingFramesAndContours(double t, int j=0);
      virtual void updateJacobiansForRemainingFramesAndContours1(double t);

      /* GETTER / SETTER */

      // NOTE: we can not use a overloaded setTranslation here due to restrictions in XML but define them for convinience in c++
      /*!
       * \brief set Kinematic for genral translational motion
       * \param fPrPK translational kinematic description
       */
      void setGeneralTranslation(fmatvec::Function<fmatvec::Vec3(fmatvec::VecV, double)> *fPrPK_) {
        fPrPK = fPrPK_;
      }
      /*!
       * \brief set Kinematic for only time dependent translational motion
       * \param fPrPK translational kinematic description
       */
      void setTimeDependentTranslation(fmatvec::Function<fmatvec::Vec3(double)> *fPrPK_) {
        setGeneralTranslation(new TimeDependentFunction<fmatvec::Vec3>(fPrPK_));
      }
      /*!
       * \brief set Kinematic for only state dependent translational motion
       * \param fPrPK translational kinematic description
       */
      void setStateDependentTranslation(fmatvec::Function<fmatvec::Vec3(fmatvec::VecV)> *fPrPK_) {
        setGeneralTranslation(new StateDependentFunction<fmatvec::Vec3>(fPrPK_));
      }
      void setTranslation(fmatvec::Function<fmatvec::Vec3(fmatvec::VecV, double)> *fPrPK_) { setGeneralTranslation(fPrPK_); }
      void setTranslation(fmatvec::Function<fmatvec::Vec3(double)> *fPrPK_) { setTimeDependentTranslation(fPrPK_); }
      void setTranslation(fmatvec::Function<fmatvec::Vec3(fmatvec::VecV)> *fPrPK_) { setStateDependentTranslation(fPrPK_); }

      // NOTE: we can not use a overloaded setRotation here due to restrictions in XML but define them for convinience in c++
      /*!
       * \brief set Kinematic for general rotational motion
       * \param fAPK rotational kinematic description
       */
      void setGeneralRotation(fmatvec::Function<fmatvec::RotMat3(fmatvec::VecV, double)>* fAPK_, bool dep=false, bool ct=true) { 
        fAPK = fAPK_; 
        translationDependentRotation = dep; 
        coordinateTransformation = ct;
      }
      /*!
       * \brief set Kinematic for only time dependent rotational motion
       * \param fAPK rotational kinematic description
       */
      void setTimeDependentRotation(fmatvec::Function<fmatvec::RotMat3(double)>* fAPK_, bool dep=false, bool ct=true) { 
        setGeneralRotation(new TimeDependentFunction<fmatvec::RotMat3>(fAPK_), dep, ct);
      }
      /*!
       * \brief set Kinematic for only state dependent rotational motion
       * \param fAPK rotational kinematic description
       */
      void setStateDependentRotation(fmatvec::Function<fmatvec::RotMat3(fmatvec::VecV)>* fAPK_, bool dep=false, bool ct=true) { 
        setGeneralRotation(new StateDependentFunction<fmatvec::RotMat3>(fAPK_), dep, ct);
      }
      void setRotation(fmatvec::Function<fmatvec::RotMat3(fmatvec::VecV, double)>* fAPK_, bool dep=false, bool ct=true) { setGeneralRotation(fAPK_,dep,ct); }
      void setRotation(fmatvec::Function<fmatvec::RotMat3(double)>* fAPK_, bool dep=false, bool ct=true) { setTimeDependentRotation(fAPK_,dep,ct); }
      void setRotation(fmatvec::Function<fmatvec::RotMat3(fmatvec::VecV)>* fAPK_, bool dep=false, bool ct=true) { setStateDependentRotation(fAPK_,dep,ct); }

      /*!
       * \brief get Kinematic for translational motion
       * \return translational kinematic description
       */
      fmatvec::Function<fmatvec::Vec3(fmatvec::VecV, double)>* getTranslation() { return fPrPK; }
      /*!
       * \brief get Kinematic for rotational motion
       * \return rotational kinematic description
       */
      fmatvec::Function<fmatvec::RotMat3(fmatvec::VecV, double)>* getRotation() { return fAPK; }

      void setMass(double m_) { m = m_; }
      double getMass() const { return m; }
      FixedRelativeFrame* getFrameForKinematics() { return K; };
      FixedRelativeFrame* getFrameC() { return C; };

      /**
       * \param RThetaR  inertia tensor
       * \param frame optional reference Frame of inertia tensor, otherwise cog-Frame will be used as reference
       */
      void setInertiaTensor(const fmatvec::SymMat3& RThetaR, Frame *frame=0) {
        SThetaS = RThetaR;
        frameForInertiaTensor = frame;
      }

      const fmatvec::SymMat3& getInertiaTensor() const {return SThetaS;}
      fmatvec::SymMat3& getInertiaTensor() {return SThetaS;}

      void addFrame(FixedRelativeFrame *frame); 

      using Body::addContour;

//      /**
//       * \param frame        specific Frame to add
//       * \param RrRF         constant relative vector from reference Frame to specific Frame in reference system
//       * \param ARF          constant relative rotation from specific Frame to reference Frame
//       * \param refFrameName reference Frame name
//       */
//      void addFrame(Frame *frame, const fmatvec::Vec3 &RrRF, const fmatvec::SqrMat3 &ARF, const std::string& refFrameName); 

      /**
       * \param frame        specific Frame to add
       * \param RrRF         constant relative vector from reference Frame to specific Frame in reference system
       * \param ARF          constant relative rotation from specific Frame to reference Frame
       * \param refFrameName optional reference Frame, otherwise cog-Frame will be used as reference
       */
      void addFrame(Frame *frame, const fmatvec::Vec3 &RrRF, const fmatvec::SqrMat3 &ARF, const Frame* refFrame=0);

      /**
       * \param str          name of Frame to add
       * \param RrRF         constant relative vector from reference Frame to specific Frame in reference system
       * \param ARF          constant relative rotation from specific Frame to reference Frame
       * \param refFrameName optional reference Frame, otherwise cog-Frame will be used as reference
       */
      void addFrame(const std::string &str, const fmatvec::Vec3 &RrRF, const fmatvec::SqrMat3 &ARF, const Frame* refFrame=0);

//      /**
//       * \param contour      specific contour to add
//       * \param RrRC         constant relative vector from reference Frame to specific contour in reference system
//       * \param ARC          constant relative rotation from specific contour to reference Frame
//       * \param refFrameName reference Frame name
//       */
//      void addContour(Contour* contour, const fmatvec::Vec3 &RrRC, const fmatvec::SqrMat3 &ARC, const std::string& refFrameName);

      /**
       * \param contour      specific contour to add
       * \param RrRC         constant relative vector from reference Frame to specific contour in reference system
       * \param ARC          constant relative rotation from specific contour to reference Frame
       * \param refFrameName optional reference Frame, otherwise cog-Frame will be used as reference
       */
      void addContour(Contour* contour, const fmatvec::Vec3 &RrRC, const fmatvec::SqrMat3 &ARC, const Frame* refFrame=0);

      /**
       * \param frame Frame to be used for kinematical description depending on reference Frame and generalised positions / velocities
       */
      void setFrameForKinematics(Frame *frame);

#ifdef HAVE_OPENMBVCPPINTERFACE
      void setOpenMBVRigidBody(OpenMBV::RigidBody* body);
      void setOpenMBVFrameOfReference(Frame * frame) {openMBVFrame=frame; }
      const Frame* getOpenMBVFrameOfReference() const {return openMBVFrame; }

      /** \brief Visualize the weight */
      void setOpenMBVWeightArrow(OpenMBV::Arrow *arrow) { FWeight = arrow; }

      /** \brief Visualize the joint force */
      void setOpenMBVJointForceArrow(OpenMBV::Arrow *arrow) { FArrow = arrow; }

      /** \brief Visualize the joint moment */
      void setOpenMBVJointMomentArrow(OpenMBV::Arrow *arrow) { MArrow = arrow; }
#endif

      virtual void initializeUsingXML(MBXMLUtils::TiXmlElement *element);
      virtual MBXMLUtils::TiXmlElement* writeXMLFile(MBXMLUtils::TiXmlNode *element);

      virtual void updatePositionAndOrientationOfFrame(double t, Frame *P);
      virtual void updateAccelerations(double t, Frame *P);
      virtual void updateRelativeJacobians(double t, Frame *P);
      virtual void updateRelativeJacobians(double t, Frame *P, fmatvec::Mat3xV &WJTrel, fmatvec::Mat3xV &WJRrel);
      const fmatvec::Mat3xV& getWJTrel() const {return WJTrel;}
      const fmatvec::Mat3xV& getWJRrel() const {return WJRrel;}
      fmatvec::Mat3xV& getWJTrel() {return WJTrel;}
      fmatvec::Mat3xV& getWJRrel() {return WJRrel;}
      fmatvec::Mat& getJRel(int i=0) {return JRel[i];}
      fmatvec::Vec& getjRel() {return jRel;}
      fmatvec::Vec& getqRel() {return qRel;}
      fmatvec::Vec& getuRel() {return uRel;}
      // void setqRel(const fmatvec::Vec &q) {qRel0 = q;}
      // void setuRel(const fmatvec::Vec &u) {uRel0 = u;}
      fmatvec::Mat3xV& getPJT(int i=0) {return PJT[i];}
      fmatvec::Mat3xV& getPJR(int i=0) {return PJR[i];}

      int getqRelSize() const {return nq;}
      int getuRelSize(int i=0) const {return nu[i];}

    protected:
      /**
       * \brief mass
       */
      double m;

      /**
       * \brief inertia tensor with respect to centre of gravity in centre of gravity and world Frame
       */
      fmatvec::SymMat3 SThetaS, WThetaS;

      FixedRelativeFrame *K;

      /**
       * \brief TODO
       */
      fmatvec::SymMat Mbuf;

      /**
       * \brief boolean to use body fixed Frame for rotation
       */
      bool coordinateTransformation;

      /**
       * JACOBIAN of translation, rotation and their derivatives in parent system
       */
      fmatvec::Mat3xV PJT[2], PJR[2];

      fmatvec::Vec3 PjhT, PjhR, PjbT, PjbR;

      /**
       * \brief rotation matrix from kinematic Frame to parent Frame
       */
      fmatvec::SqrMat3 APK;

      /**
       * \brief translation from parent to kinematic Frame in parent and world system
       */
      fmatvec::Vec3 PrPK, WrPK;

      /**
       * \brief translational and angular velocity from parent to kinematic Frame in world system
       */
      fmatvec::Vec3 WvPKrel, WomPK;

      fmatvec::Function<fmatvec::MatV(fmatvec::VecV)> *fTR;

      /**
       * \brief translation from parent Frame to kinematic Frame in parent system
       */
      fmatvec::Function<fmatvec::Vec3(fmatvec::VecV, double)> *fPrPK;

      /**
       * \brief rotation from kinematic Frame to parent Frame
       */
      fmatvec::Function<fmatvec::RotMat3(fmatvec::VecV, double)> *fAPK;

      /**
       * \brief function pointer to update mass matrix
       */
      void (RigidBody::*updateM_)(double t, int i);

      /**
       * \brief update constant mass matrix
       */
      void updateMConst(double t, int i=0);

      /**
       * \brief update time dependend mass matrix
       */
      void updateMNotConst(double t, int i=0); 

      /**
       * \brief function pointer for Cholesky decomposition of mass matrix
       */
      void (RigidBody::*facLLM_)(int i);

      /**
       * \brief Cholesky decomposition of constant mass matrix
       */
      void facLLMConst(int i=0) {};

      /**
       * \brief Cholesky decomposition of time dependent mass matrix
       */
      void facLLMNotConst(int i=0) { Object::facLLM(i); }

      void (RigidBody::*updateJacobians_[2])(double t); 

      /** a pointer to Frame "C" */
      FixedRelativeFrame *C;

      fmatvec::Vec aT, aR;

      fmatvec::Vec qRel, uRel;
      fmatvec::Mat JRel[2];
      fmatvec::Vec jRel;

      fmatvec::VecV qTRel, qRRel, uTRel, uRRel;
      fmatvec::Mat3xV WJTrel, WJRrel, PJTT, PJRR;

      Constraint *constraint;

      int nu[2], nq;

      Frame *frameForJacobianOfRotation;

      std::vector<FixedRelativeFrame*> RBF;
      std::vector<CompoundContour*> RBC;

      Frame *frameForInertiaTensor;

      fmatvec::Range<fmatvec::Var,fmatvec::Var> iqT, iqR, iuT, iuR;

      bool translationDependentRotation, constJT, constJR, constjT, constjR;

    private:
#ifdef HAVE_OPENMBVCPPINTERFACE
      /**
       * \brief Frame of reference for drawing openMBVBody
       */
      Frame * openMBVFrame;
      OpenMBV::Arrow *FWeight, *FArrow, *MArrow;
#endif
  };

}

#endif
