/* Copyright (C) 2004-2009 MBSim Development Team
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
 * Contact: mfoerg@users.berlios.de
 *          rzander@users.berlios.de
 */

#include <config.h>
#include "mbsim/contour.h"
#include "mbsim/object.h"
#include "mbsim/mbsim_event.h"
#include "mbsim/utils/rotarymatrices.h"
#include "mbsim/contours/point.h"
#include "mbsim/utils/contact_utils.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
#include <object_interface.h>
#include <openmbvcppinterface/group.h>
#include <openmbvcppinterface/rigidbody.h>
#include <openmbvcppinterface/frustum.h>
#include <openmbvcppinterface/sphere.h>
#endif

using namespace fmatvec;
using namespace std;

namespace MBSim {

  /* Contour */
  Contour::Contour(const string &name) : Element(name), parent(0), R("R") {
    // no canonic output...
    hSize[0] = 0;
    hSize[1] = 0;
    hInd[0] = 0;
    hInd[1] = 0;
  }

  Contour::~Contour() {}

  void Contour::init(InitStage stage) {
    if(stage==unknownStage) {
      getFrame()->getJacobianOfTranslation(0).resize(3,hSize[0]);
      getFrame()->getJacobianOfRotation(0).resize(3,hSize[0]);
      getFrame()->getJacobianOfTranslation(1).resize(3,hSize[1]);
      getFrame()->getJacobianOfRotation(1).resize(3,hSize[1]);
    }
    else if(stage==MBSim::plot) {
      updatePlotFeatures(parent);

      if(getPlotFeature(plotRecursive)==enabled) {
        Element::init(stage, parent);
      }
    }
    else
      Element::init(stage, parent);
  }

  void Contour::resizeJacobians(int j) {
    getFrame()->getJacobianOfTranslation().resize(3,hSize[j]);
    getFrame()->getJacobianOfRotation().resize(3,hSize[j]);
  }

  /* Rigid Contour */
  RigidContour::~RigidContour() {
# ifdef HAVE_OPENMBVCPPINTERFACE
    if(openMBVRigidBody) delete openMBVRigidBody;
# endif
  }

  void RigidContour::updateKinematicsForFrame(ContourPointData &cp, FrameFeature ff) {
    if(ff==velocity || ff==velocities) {
      Vec WrPC = cp.getFrameOfReference().getPosition() - R.getPosition();
      cp.getFrameOfReference().setVelocity(R.getVelocity() + crossProduct(R.getAngularVelocity(),WrPC));
    }
    if(ff==angularVelocity || ff==velocities)
      cp.getFrameOfReference().setAngularVelocity(R.getAngularVelocity());
    if(ff!=velocity && ff!=angularVelocity && ff!=velocities) throw MBSimError("ERROR (RigidContour::updateKinematicsForFrame): FrameFeature not implemented!");
  }

  void RigidContour::updateJacobiansForFrame(ContourPointData &cp, int j) {
    Vec WrPC = cp.getFrameOfReference().getPosition() - R.getPosition();
    Mat tWrPC = tilde(WrPC);

    cp.getFrameOfReference().setJacobianOfTranslation(R.getJacobianOfTranslation(j) - tWrPC*R.getJacobianOfRotation(j),j);
    cp.getFrameOfReference().setJacobianOfRotation(R.getJacobianOfRotation(j),j);
    cp.getFrameOfReference().setGyroscopicAccelerationOfTranslation(R.getGyroscopicAccelerationOfTranslation() - tWrPC*R.getGyroscopicAccelerationOfRotation() + crossProduct(R.getAngularVelocity(),crossProduct(R.getAngularVelocity(),WrPC)));
    cp.getFrameOfReference().setGyroscopicAccelerationOfRotation(R.getGyroscopicAccelerationOfRotation());

    // adapt dimensions if necessary
    if(cp.getFrameOfReference().getJacobianOfTranslation(j).rows() == 0) cp.getFrameOfReference().getJacobianOfTranslation(j).resize(3,R.getJacobianOfTranslation(j).cols());
    if(cp.getFrameOfReference().getJacobianOfRotation(j).rows() == 0) cp.getFrameOfReference().getJacobianOfRotation(j).resize(3,R.getJacobianOfRotation(j).cols());
  }

  void RigidContour::init(InitStage stage) {
    if(stage==unknownStage)
      Contour::init(stage);
    else if(stage==MBSim::plot) {
      updatePlotFeatures(parent);

      if(getPlotFeature(plotRecursive)==enabled) {
#ifdef HAVE_OPENMBVCPPINTERFACE
        if(getPlotFeature(openMBV)==enabled && openMBVRigidBody) {
          openMBVRigidBody->setName(name);
          parent->getOpenMBVGrp()->addObject(openMBVRigidBody);
        }
#endif
        Contour::init(stage);
      }
    }
    else
      Contour::init(stage);
  }

  void RigidContour::plot(double t, double dt) {
    if(getPlotFeature(plotRecursive)==enabled) {
#ifdef HAVE_OPENMBVCPPINTERFACE
      if(getPlotFeature(openMBV)==enabled && openMBVRigidBody) {
        vector<double> data;
        data.push_back(t);
        data.push_back(R.getPosition()(0));
        data.push_back(R.getPosition()(1));
        data.push_back(R.getPosition()(2));
        Vec cardan=AIK2Cardan(R.getOrientation());
        data.push_back(cardan(0));
        data.push_back(cardan(1));
        data.push_back(cardan(2));
        data.push_back(0);
        openMBVRigidBody->append(data);
      }
#endif
      Element::plot(t,dt);
    }
  }

  ContactKinematics * RigidContour::findContactPairingWith(std::string type0, std::string type1) {
    return findContactPairingRigidRigid(type0.c_str(), type1.c_str());
  }

}
