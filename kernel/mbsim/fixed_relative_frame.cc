/* Copyright (C) 2004-2014 MBSim Development Team
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

#include <config.h>
#include "mbsim/fixed_relative_frame.h"

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  void FixedRelativeFrame::init(InitStage stage) {
    if(stage==resolveXMLPath) {
      if(saved_frameOfReference!="")
        setFrameOfReference(getByPath<Frame>(saved_frameOfReference));
      Frame::init(stage);
    }
    else
      Frame::init(stage);
  }

  void FixedRelativeFrame::initializeUsingXML(DOMElement *element) {
    Frame::initializeUsingXML(element);
    DOMElement *ec=element->getFirstElementChild();
    ec=E(element)->getFirstElementChildNamed(MBSIM%"frameOfReference");
    if(ec) setFrameOfReference(E(ec)->getAttribute("ref"));
    ec=E(element)->getFirstElementChildNamed(MBSIM%"relativePosition");
    if(ec) setRelativePosition(getVec3(ec));
    ec=E(element)->getFirstElementChildNamed(MBSIM%"relativeOrientation");
    if(ec) setRelativeOrientation(getSqrMat3(ec));
  }

  DOMElement* FixedRelativeFrame::writeXMLFile(DOMNode *parent) {
    DOMElement *ele0 = Frame::writeXMLFile(parent);
//     if(getFrameOfReference()) {
//        DOMElement *ele1 = new DOMElement( MBSIM%"frameOfReference" );
//        string str = string("../Frame[") + getFrameOfReference()->getName() + "]";
//        ele1->SetAttribute("ref", str);
//        ele0->LinkEndChild(ele1);
//      }
//     addElementText(ele0,MBSIM%"relativePosition",getGeneralizedRelativePosition());
//     addElementText(ele0,MBSIM%"relativeOrientation",getRelativeOrientation());
   return ele0;
  }

  void FixedRelativeFrame::resetUpToDate() {
    Frame::resetUpToDate();
  }
  void FixedRelativeFrame::resetPositionsUpToDate() {
    Frame::resetPositionsUpToDate();
  }

  const Vec3& FixedRelativeFrame::getGlobalRelativePosition(double t) {
    if(updatePos) updatePositions(t);
    return WrRP; 
  }

  void FixedRelativeFrame::updatePositions(double t) { 
    setOrientation(R->getOrientation(t)*ARP);
    WrRP = R->getOrientation(t)*RrRP; 
    setPosition(R->getPosition(t) + WrRP);
    updatePos = false;
  }

  void FixedRelativeFrame::updateVelocities(double t) { 
    setAngularVelocity(R->getAngularVelocity(t));
    setVelocity(R->getVelocity(t) + crossProduct(R->getAngularVelocity(), getGlobalRelativePosition(t))); 
    updateVel = false;
  }

  void FixedRelativeFrame::updateAccelerations(double t) { 
    setAngularAcceleration(R->getAngularAcceleration(t));
    setAcceleration(R->getAcceleration(t) + crossProduct(R->getAngularAcceleration(), getGlobalRelativePosition(t)) + crossProduct(R->getAngularVelocity(t), crossProduct(R->getAngularVelocity(t), getGlobalRelativePosition(t)))); 
    updateAcc = true;
  }

  void FixedRelativeFrame::updateJacobians(double t, int j) {
    setJacobianOfTranslation(R->getJacobianOfTranslation(t,j) - tilde(getGlobalRelativePosition(t))*R->getJacobianOfRotation(t,j),j);
    setJacobianOfRotation(R->getJacobianOfRotation(j),j);
    updateJac[j] = false;
  }

  void FixedRelativeFrame::updateGyroscopicAccelerations(double t) {
    setGyroscopicAccelerationOfTranslation(R->getGyroscopicAccelerationOfTranslation(t) + crossProduct(R->getGyroscopicAccelerationOfRotation(t),getGlobalRelativePosition(t)) + crossProduct(R->getAngularVelocity(t),crossProduct(R->getAngularVelocity(t),WrRP)));
    setGyroscopicAccelerationOfRotation(R->getGyroscopicAccelerationOfRotation());
    updateGA = false;
  }

}

