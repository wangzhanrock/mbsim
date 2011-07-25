/* Copyright (C) 2004-2009 MBSim Development Team
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
 */

#include <config.h>
#include "mbsim/dynamic_system.h"
#include "mbsim/modelling_interface.h"
#include "mbsim/extra_dynamic.h"
#include "mbsim/link.h"
#include "mbsim/contour.h"
#include "mbsim/object.h"
#include "mbsim/frame.h"
#include "mbsim/contact.h"
#include "mbsim/dynamic_system_solver.h"
#include "hdf5serie/fileserie.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
#include "openmbvcppinterface/group.h"
#endif

//#ifdef _OPENMP
//#include <omp.h>
//#endif

#include<algorithm>

using namespace std;
using namespace fmatvec;

namespace MBSim {

  DynamicSystem::DynamicSystem(const string &name) : Element(name), parent(0), frameParent(0), PrPF(Vec(3,INIT,0.)), APF(SqrMat(3,EYE)), q0(0), u0(0), x0(0), qSize(0), qInd(0), xSize(0), xInd(0), gSize(0), gInd(0), gdSize(0), gdInd(0), laSize(0), laInd(0), rFactorSize(0), rFactorInd(0), svSize(0), svInd(0), LinkStatusSize(0), LinkStatusInd(0)
#ifdef HAVE_OPENMBVCPPINTERFACE                      
                                                     , openMBVGrp(0)
#endif
                                                     {
                                                       uSize[0] = 0;
                                                       uSize[1] = 0;
                                                       uInd[0] = 0;
                                                       uInd[1] = 0;
                                                       hSize[0] = 0;
                                                       hSize[1] = 0;
                                                       hInd[0] = 0;
                                                       hInd[1] = 0;

                                                       I=new Frame("I");
                                                       addFrame(I);

                                                       IrOF.push_back(Vec(3,INIT,0.));
                                                       AIF.push_back(SqrMat(3,EYE));
                                                     }

  DynamicSystem::~DynamicSystem() {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      delete *i;
    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i)
      delete *i;
    for(vector<Link*>::iterator i = link.begin(); i != link.end(); ++i)
      delete *i;
    for(vector<ExtraDynamic*>::iterator i = extraDynamic.begin(); i != extraDynamic.end(); ++i)
      delete *i;
    for(vector<Frame*>::iterator i = frame.begin(); i != frame.end(); ++i)
      delete *i;
  }

  void DynamicSystem::updateT(double t) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (**i).updateT(t);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (**i).updateT(t);
  }

  void DynamicSystem::updateh(double t, int j) {
//#pragma omp parallel for schedule(static) shared(t) default(none)
    for(int i=0; i<(int)dynamicsystem.size(); i++) {
      try { dynamicsystem[i]->updateh(t,j); }
      catch(MBSimError error) { error.printExceptionMessage(); throw; }
    }

//#pragma omp parallel for num_threads(omp_get_max_threads()-1) schedule(dynamic, max(1,(int)object.size()/(10*omp_get_num_threads()))) shared(t) default(none) if((int)object.size()>30) 
    for(int i=0; i<(int)object.size(); i++) {
      try { object[i]->updateh(t,j); }
      catch(MBSimError error) { error.printExceptionMessage(); throw; }
    }

//#pragma omp parallel for num_threads(omp_get_max_threads()-1) schedule(dynamic, max(1,(int)linkSingleValued.size()/(10*omp_get_num_threads()))) shared(t) default(none) if((int)linkSingleValued.size()>30) 
    for(int i=0; i<(int)linkSingleValued.size(); i++) {
      try { linkSingleValued[i]->updateh(t,j); }
      catch(MBSimError error) { error.printExceptionMessage(); throw; }
    }

//#pragma omp parallel for num_threads(omp_get_max_threads()-1) schedule(dynamic, max(1,(int)linkSetValuedNotActiveWithSmoothPart.size()/(10*omp_get_num_threads()))) shared(t) default(none) if((int)linkSetValuedNotActiveWithSmoothPart.size()>30) 
    for(int i=0; i<(int)linkSetValuedNotActiveWithSmoothPart.size(); i++) {
      try { linkSetValuedNotActiveWithSmoothPart[i]->updateh(t,j); }
      catch(MBSimError error) { error.printExceptionMessage(); throw; }
    }
  }

  void DynamicSystem::updateh0Fromh1(double t) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (**i).updateh0Fromh1(t);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (**i).updateh0Fromh1(t);
  }

  void DynamicSystem::updateW0FromW1(double t) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (**i).updateW0FromW1(t);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (**i).updateW0FromW1(t);
  }

  void DynamicSystem::updateV0FromV1(double t) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (**i).updateV0FromV1(t);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (**i).updateV0FromV1(t);
  }

  void DynamicSystem::updatehInverseKinetics(double t, int j) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (**i).updatehInverseKinetics(t,j);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (**i).updatehInverseKinetics(t,j);
  }

  void DynamicSystem::updateStateDerivativeDependentVariables(double t) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (**i).updateStateDerivativeDependentVariables(t);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (**i).updateStateDerivativeDependentVariables(t);
  }

  void DynamicSystem::updatedhdz(double t) {
//#pragma omp parallel for schedule(static) shared(t) default(none)
    for(int i=0; i<(int)dynamicsystem.size(); i++) {
      try { dynamicsystem[i]->updatedhdz(t); }
      catch(MBSimError error) { error.printExceptionMessage(); throw; }
    }

//#pragma omp parallel for num_threads(omp_get_max_threads()-1) schedule(dynamic, max(1,(int)object.size()/(10*omp_get_num_threads()))) shared(t) default(none) if((int)object.size()>30) 
    for(int i=0; i<(int)object.size(); i++) {
      try { object[i]->updatedhdz(t); }
      catch(MBSimError error) { error.printExceptionMessage(); throw; }
    }

    for(vector<Link*>::iterator i = linkSingleValued.begin(); i != linkSingleValued.end(); ++i) 
      (**i).updatedhdz(t);

    for(vector<Link*>::iterator i = linkSetValuedNotActiveWithSmoothPart.begin(); i != linkSetValuedNotActiveWithSmoothPart.end(); ++i) 
      (**i).updatedhdz(t);
  }

  void DynamicSystem::updateM(double t, int j) {
    for(int i=0; i<(int)dynamicsystem.size(); i++) {
      try { dynamicsystem[i]->updateM(t,j); }
      catch(MBSimError error) { error.printExceptionMessage(); throw; }
    }

//#pragma omp parallel for num_threads(omp_get_max_threads()-1) schedule(dynamic, max(1,(int)object.size()/(10*omp_get_num_threads()))) shared(t) default(none) if((int)object.size()>30) 
    for(int i=0; i<(int)object.size(); i++) {
      try { object[i]->updateM(t,j); }
      catch(MBSimError error) { error.printExceptionMessage(); throw; }
    }
  }

  void DynamicSystem::updatedq(double t, double dt) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i)
      (**i).updatedq(t,dt);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i)
      (**i).updatedq(t,dt);
  }

  void DynamicSystem::sethSize(int hSize_, int j) {
    hSize[j] = hSize_;

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) {
      (*i)->sethSize((*i)->getuSize(j),j);
      (*i)->sethInd((*i)->getuInd(j),j);
    }

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) {
      (*i)->sethSize((*i)->getuSize(j),j);
      (*i)->sethInd((*i)->getuInd(j),j);
    }
  }

  void DynamicSystem::calcqSize() {
    qSize = 0;

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) {
      (*i)->calcqSize();
      (*i)->setqInd(qSize);
      qSize += (*i)->getqSize();
    }
    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) {
      (*i)->calcqSize();
      (*i)->setqInd(qSize);
      qSize += (*i)->getqSize();
    }
  }

  void DynamicSystem::calcuSize(int j) {
    uSize[j] = 0;

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) {
      (*i)->calcuSize(j);
      (*i)->setuInd(uSize[j],j);
      uSize[j] += (*i)->getuSize(j);
    }
    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) {
      (*i)->calcuSize(j);
      (*i)->setuInd(uSize[j],j);
      uSize[j] += (*i)->getuSize(j);
    }
  }

  int DynamicSystem::gethInd(DynamicSystem* sys, int i) {
    return (this == sys) ? 0 : hInd[i] + parent->gethInd(sys,i);
  }

  int DynamicSystem::getuInd(DynamicSystem* sys, int i) {
    return (this == sys) ? 0 : uInd[i] + parent->getuInd(sys,i);
  }

  int DynamicSystem::getqInd(DynamicSystem* sys) {
    return (this == sys) ? 0 : qInd + parent->getqInd(sys);
  }

#ifdef HAVE_OPENMBVCPPINTERFACE
  OpenMBV::Group* DynamicSystem::getOpenMBVGrp() { return openMBVGrp; }
#endif

  void DynamicSystem::updatewb(double t, int j) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updatewb(t,j);

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i)
      (**i).updatewb(t,j);
  }

  void DynamicSystem::updateW(double t, int j) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (**i).updateW(t,j);

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i)
      (**i).updateW(t,j);
  }

  void DynamicSystem::updateJacobiansInverseKinetics(double t, int j) {

    for(vector<Link*>::iterator i = inverseKineticsLink.begin(); i != inverseKineticsLink.end(); ++i)
      (**i).updateJacobians(t,j);
  }

  void DynamicSystem::updateWInverseKinetics(double t, int j) {
    WInverseKinetics.init(0);

    for(vector<Link*>::iterator i = inverseKineticsLink.begin(); i != inverseKineticsLink.end(); ++i)
      (**i).updateW(t,j);
  }

  void DynamicSystem::updateV(double t, int j) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (**i).updateV(t,j);

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i)
      (**i).updateV(t,j);
  }

  void DynamicSystem::updateg(double t) {
//#pragma omp parallel for schedule(static) shared(t) default(none) 
    for(int i=0; i<(int)dynamicsystem.size(); i++) {
      try { dynamicsystem[i]->updateg(t); }
      catch(MBSimError error) { error.printExceptionMessage(); throw; }
    }

//#pragma omp parallel for schedule(dynamic, max(1,(int)linkSingleValued.size()/(10*omp_get_num_threads()))) shared(t) default(none) if((int)linkSingleValued.size()>30) 
    for(int i=0; i<(int)linkSingleValued.size(); i++) {
      try { linkSingleValued[i]->updateg(t); }
      catch(MBSimError error) { error.printExceptionMessage(); throw; }
    }

//#pragma omp parallel for schedule(dynamic, max(1,(int)linkSetValued.size()/(10*omp_get_num_threads()))) shared(t) default(none) if((int)linkSetValued.size()>30) 
    for(int i=0; i<(int)linkSetValued.size(); i++) {
      try { linkSetValued[i]->updateg(t); }
      catch(MBSimError error) { error.printExceptionMessage(); throw; }
    }
  }

  void DynamicSystem::updategInverseKinetics(double t) {
//#pragma omp parallel for schedule(static) shared(t) default(none) 
    for(int i=0; i<(int)dynamicsystem.size(); i++) {
      try { dynamicsystem[i]->updategInverseKinetics(t); }
      catch(MBSimError error) { error.printExceptionMessage(); throw; }
    }

//#pragma omp parallel for schedule(dynamic, max(1,(int)inverseKineticsLink.size()/(10*omp_get_num_threads()))) shared(t) default(none) if((int)inverseKineticsLink.size()>30) 
    for(int i=0; i<(int)inverseKineticsLink.size(); i++) {
      try { inverseKineticsLink[i]->updateg(t); }
      catch(MBSimError error) { error.printExceptionMessage(); throw; }
    }
  }

  void DynamicSystem::updategd(double t) {
//#pragma omp parallel for schedule(static) shared(t) default(none)
    for(int i=0; i<(int)dynamicsystem.size(); i++) {
      try { dynamicsystem[i]->updategd(t); }
      catch(MBSimError error) { error.printExceptionMessage(); throw; }
    }

//#pragma omp parallel for schedule(static) shared(t) default(none) if((int)linkSingleValued.size()>30)
    for(int i=0; i<(int)linkSingleValued.size(); i++) {
      try { linkSingleValued[i]->updategd(t); }
      catch(MBSimError error) { error.printExceptionMessage(); throw; }
    }

//#pragma omp parallel for schedule(static) shared(t) default(none) if((int)linkSetValuedNotActiveWithSmoothPart.size()>30)
    for(int i=0; i<(int)linkSetValuedNotActiveWithSmoothPart.size(); i++) { 
      try { linkSetValuedNotActiveWithSmoothPart[i]->updategd(t); }
      catch(MBSimError error) { error.printExceptionMessage(); throw; }
    }

//#pragma omp parallel for schedule(static) shared(t) default(none) if((int)linkSetValuedActive.size()>30)
    for(int i=0; i<(int)linkSetValuedActive.size(); i++) { 
      try { linkSetValuedActive[i]->updategd(t); }
      catch(MBSimError error) { error.printExceptionMessage(); throw; }
    }
  }

  void DynamicSystem::updategdInverseKinetics(double t) {
//#pragma omp parallel for schedule(static) shared(t) default(none)
    for(int i=0; i<(int)dynamicsystem.size(); i++) {
      try { dynamicsystem[i]->updategdInverseKinetics(t); }
      catch(MBSimError error) { error.printExceptionMessage(); throw; }
    }

//#pragma omp parallel for schedule(static) shared(t) default(none) if((int)inverseKineticsLink.size()>30)
    for(int i=0; i<(int)inverseKineticsLink.size(); i++) { 
      try { inverseKineticsLink[i]->updategd(t); }
      catch(MBSimError error) { error.printExceptionMessage(); throw; }
    }
  }

  void DynamicSystem::updatedx(double t, double dt) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (**i).updatedx(t,dt);

    for(vector<Link*>::iterator i = link.begin(); i != link.end(); ++i)
      (**i).updatedx(t,dt);

    for(vector<ExtraDynamic*>::iterator i = extraDynamic.begin(); i!= extraDynamic.end(); ++i) 
      (**i).updatedx(t,dt);

  }

  void DynamicSystem::updateqd(double t) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updateqd(t);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (*i)->updateqd(t);
  }

  void DynamicSystem::updatexd(double t) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (**i).updatexd(t);

    for(vector<Link*>::iterator i = link.begin(); i != link.end(); ++i)
      (**i).updatexd(t);

    for(vector<ExtraDynamic*>::iterator i = extraDynamic.begin(); i!= extraDynamic.end(); ++i) 
      (**i).updatexd(t);
  }

  void DynamicSystem::updateStopVector(double t) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updateStopVector(t);
    for(vector<Link*>::iterator i = linkSetValued.begin(); i != linkSetValued.end(); ++i) 
      (*i)->updateStopVector(t); 
  }  
 
  void DynamicSystem::updateLinkStatus(double t) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updateLinkStatus(t);
    for(vector<Link*>::iterator i = linkSetValued.begin(); i != linkSetValued.end(); ++i) 
      (*i)->updateLinkStatus(t); 
  } 

  void DynamicSystem::setDynamicSystemSolver(DynamicSystemSolver* sys) {
    Element::setDynamicSystemSolver(sys);
    for(unsigned i=0; i<dynamicsystem.size(); i++)
      dynamicsystem[i]->setDynamicSystemSolver(sys);

    for(unsigned i=0; i<object.size(); i++)
      object[i]->setDynamicSystemSolver(sys);

    for(unsigned i=0; i<link.size(); i++)
      link[i]->setDynamicSystemSolver(sys);

    for (unsigned i=0; i<extraDynamic.size(); i++)
      extraDynamic[i]->setDynamicSystemSolver(sys);
  }

  void DynamicSystem::plot(double t, double dt) {
    if(getPlotFeature(plotRecursive)==enabled) {
      for(unsigned i=0; i<dynamicsystem.size(); i++)
        dynamicsystem[i]->plot(t,dt);
      for(unsigned i=0; i<object.size(); i++)
        object[i]->plot(t,dt);
      for(unsigned i=0; i<link.size(); i++)
        link[i]->plot(t,dt);
      for(unsigned i=0; i<extraDynamic.size(); i++)
        extraDynamic[i]->plot(t,dt);
      for(unsigned i=0; i<frame.size(); i++)
        frame[i]->plot(t,dt);
      for(unsigned i=0; i<inverseKineticsLink.size(); i++)
        inverseKineticsLink[i]->plot(t,dt);
    }
  }

  void DynamicSystem::closePlot() {
    if(getPlotFeature(plotRecursive)==enabled) {
      for(unsigned i=0; i<dynamicsystem.size(); i++)
        dynamicsystem[i]->closePlot();
      for(unsigned i=0; i<object.size(); i++)
        object[i]->closePlot();
      for(unsigned i=0; i<link.size(); i++)
        link[i]->closePlot();
      for(unsigned i=0; i<extraDynamic.size(); i++)
        extraDynamic[i]->closePlot();
      for(unsigned i=0; i<frame.size(); i++)
        frame[i]->closePlot();

      if(getPlotFeature(separateFilePerGroup)==enabled)
        delete (H5::FileSerie*)plotGroup;
      else
        delete (H5::Group*)plotGroup;
    }

#ifdef HAVE_OPENMBVCPPINTERFACE
    if(openMBVGrp) { delete openMBVGrp; openMBVGrp=0; }
#endif
  }

  void DynamicSystem::init(InitStage stage) {
    if(stage==relativeFrameContourLocation) {
      // This outer loop is nessesary because the frame hierarchy must not be in the correct order!
      for(size_t k=0; k<saved_refFrameF.size(); k++)
        for(size_t j=0; j<saved_refFrameF.size(); j++) {
          int i = 0;
          if(saved_refFrameF[j]!="") i = frameIndex(getFrame(saved_refFrameF[j]));

          IrOF[j+1]=IrOF[i] + AIF[i]*saved_RrRF[j];
          AIF[j+1]=AIF[i]*saved_ARF[j];
        }
      for(size_t j=0; j<saved_refFrameC.size(); j++) {
        int i = 0;
        if(saved_refFrameC[j]!="") i = frameIndex(getFrame(saved_refFrameC[j]));

        IrOC[j]=IrOF[i] + AIF[i]*saved_RrRC[j];
        AIC[j]=AIF[i]*saved_ARC[j];
      }
    }
    else if(stage==worldFrameContourLocation) {
      if(frameParent) {
        I->setPosition(frameParent->getPosition() + frameParent->getOrientation()*PrPF);
        I->setOrientation(frameParent->getOrientation()*APF);
      }
      else {
        if(parent) {
          I->setPosition(parent->getFrameI()->getPosition() + parent->getFrameI()->getOrientation()*PrPF);
          I->setOrientation(parent->getFrameI()->getOrientation()*APF);
        }
        else {
          I->setPosition(getFrameI()->getPosition() + getFrameI()->getOrientation()*PrPF);
          I->setOrientation(getFrameI()->getOrientation()*APF);
        }
      }
      for(unsigned int i=1; i<frame.size(); i++) { // kinematics of other frames can be updates from frame I 
        frame[i]->setPosition(I->getPosition() + I->getOrientation()*IrOF[i]);
        frame[i]->setOrientation(I->getOrientation()*AIF[i]);
      }
      for(unsigned int i=0; i<contour.size(); i++) { // kinematics of other contours can be updates from frame I
        contour[i]->setReferencePosition(I->getPosition() + I->getOrientation()*IrOC[i]);
        contour[i]->setReferenceOrientation(I->getOrientation()*AIC[i]);
      }
    }
    else if(stage==MBSim::plot) {
      if(parent)
        updatePlotFeatures(parent);

      if(getPlotFeature(plotRecursive)==enabled) {
        if(getPlotFeature(separateFilePerGroup)==enabled) {
          // create symbolic link in parent plot file if exist
          if(parent) H5Lcreate_external((getPath()+".mbsim.h5").c_str(), "/",
              parent->getPlotGroup()->getId(), name.c_str(),
              H5P_DEFAULT, H5P_DEFAULT);
          // create new plot file (cast needed because of the inadequacy of the HDF5 C++ interface?)
          plotGroup=(H5::Group*)new H5::FileSerie(getPath()+".mbsim.h5", H5F_ACC_TRUNC);
        }
        else
          plotGroup=new H5::Group(parent->getPlotGroup()->createGroup(name));

        H5::SimpleAttribute<string>::setData(*plotGroup, "Description", "Object of class: "+getType());
        plotVectorSerie=NULL;

#ifdef HAVE_OPENMBVCPPINTERFACE
        openMBVGrp=new OpenMBV::Group();
        openMBVGrp->setName(name);
        if(parent) parent->openMBVGrp->addObject(openMBVGrp);
        if(getPlotFeature(separateFilePerGroup)==enabled)
          openMBVGrp->setSeparateFile(true);
#endif

        plotGroup->flush(H5F_SCOPE_GLOBAL);
      }
    }

    for(unsigned i=0; i<frame.size(); i++)
      frame[i]->init(stage);
    for(unsigned int i=0; i<contour.size(); i++)
      contour[i]->init(stage);
    for(unsigned int i=0; i<dynamicsystem.size(); i++)
      dynamicsystem[i]->init(stage);
    for(unsigned i=0; i<object.size(); i++)
      object[i]->init(stage);
    for(unsigned i=0; i<link.size(); i++)
      link[i]->init(stage);
    for(unsigned i=0; i<extraDynamic.size(); i++)
      extraDynamic[i]->init(stage);
    for(unsigned i=0; i<model.size(); i++)
      model[i]->init(stage);
    for(unsigned i=0; i<inverseKineticsLink.size(); i++)
      inverseKineticsLink[i]->init(stage);
  }

  int DynamicSystem::solveConstraintsFixpointSingle() {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->solveConstraintsFixpointSingle(); 

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i) 
      (*i)->solveConstraintsFixpointSingle();

    return 0;
  }

  int DynamicSystem::solveImpactsFixpointSingle(double dt) {
//#pragma omp parallel for schedule(static) shared(dt) default(none)
    for(int i=0; i<(int)dynamicsystem.size(); i++) {
      try { dynamicsystem[i]->solveImpactsFixpointSingle(dt); }
      catch(MBSimError error) { error.printExceptionMessage(); throw; }
    }

//#pragma omp parallel for num_threads(omp_get_max_threads()-1) schedule(dynamic, max(1,(int)linkSetValuedActive.size()/(10*omp_get_num_threads()))) shared(dt) default(none) if((int)linkSetValuedActive.size()>30) 
    for(int i=0; i<(int)linkSetValuedActive.size(); i++) {
      try { linkSetValuedActive[i]->solveImpactsFixpointSingle(dt); }
      catch(MBSimError error) { error.printExceptionMessage(); throw; }
    }

    return 0;
  }

  int DynamicSystem::solveConstraintsGaussSeidel() {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->solveConstraintsGaussSeidel(); 

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i) 
      (*i)->solveConstraintsGaussSeidel();

    return 0;
  }

  int DynamicSystem::solveImpactsGaussSeidel(double dt) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->solveImpactsGaussSeidel(dt); 

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i)
      (*i)->solveImpactsGaussSeidel(dt);

    return 0;
  }

  int DynamicSystem::solveConstraintsRootFinding() {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->solveConstraintsRootFinding(); 

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i) 
      (*i)->solveConstraintsRootFinding();

    return 0;
  }

  int DynamicSystem::solveImpactsRootFinding(double dt) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->solveImpactsRootFinding(dt);

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i) 
      (*i)->solveImpactsRootFinding(dt);

    return 0;
  }

  int DynamicSystem::jacobianConstraints() {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->jacobianConstraints(); 

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i) 
      (*i)->jacobianConstraints();

    return 0;
  }

  int DynamicSystem::jacobianImpacts() {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->jacobianImpacts(); 

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i) 
      (*i)->jacobianImpacts();

    return 0;
  }

  void DynamicSystem::checkConstraintsForTermination() {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->checkConstraintsForTermination(); 

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i)
      (**i).checkConstraintsForTermination();
  }

  void DynamicSystem::checkImpactsForTermination(double dt) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->checkImpactsForTermination(dt);

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i)
      (**i).checkImpactsForTermination(dt);
  }

  void DynamicSystem::updaterFactors() {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updaterFactors(); 

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i)
      (**i).updaterFactors();
  }

  Frame* DynamicSystem::getFrame(const string &name, bool check) {
    unsigned int i;
    for(i=0; i<frame.size(); i++) {
      if(frame[i]->getName() == name)
        return frame[i];
    }
    if(check) {
      if(!(i<frame.size()))
        throw MBSimError("The object \""+name+"\" comprises no frame \""+name+"\"!");
      assert(i<frame.size());
    }
    return NULL;
  }

  Contour* DynamicSystem::getContour(const string &name, bool check) {
    unsigned int i;
    for(i=0; i<contour.size(); i++) {
      if(contour[i]->getName() == name)
        return contour[i];
    }
    if(check) {
      if(!(i<contour.size()))
        throw MBSimError("The object \""+name+"\" comprises no contour \""+name+"\"!");
      assert(i<contour.size());
    }
    return NULL;
  }

  void DynamicSystem::updateqRef(const Vec &qParent) {
    q >> qParent(qInd,qInd+qSize-1);

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (**i).updateqRef(q);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (**i).updateqRef(q);
  }

  void DynamicSystem::updateqdRef(const Vec &qdParent) {
    qd >> qdParent(qInd,qInd+qSize-1);

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (**i).updateqdRef(qd);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (**i).updateqdRef(qd);
  }

  void DynamicSystem::updateuRef(const Vec &uParent) {
    u >> uParent(uInd[0],uInd[0]+uSize[0]-1);

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (**i).updateuRef(u);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (**i).updateuRef(u);
  }

  void DynamicSystem::updateuallRef(const Vec &uParent) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (**i).updateuallRef(u);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (**i).updateuallRef(u);
  }

  void DynamicSystem::updateudRef(const Vec &udParent, int j) {
    ud[j] >> udParent(uInd[j],uInd[j]+uSize[j]-1);

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (**i).updateudRef(ud[j],j);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (**i).updateudRef(ud[j],j);
  }

  void DynamicSystem::updateudallRef(const Vec &udParent, int j) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (**i).updateudallRef(ud[j],j);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (**i).updateudallRef(ud[j],j);
  }

  void DynamicSystem::updatexRef(const Vec &xParent) {
    x >> xParent(xInd,xInd+xSize-1);

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (**i).updatexRef(x);

    for(vector<Link*>::iterator i = link.begin(); i != link.end(); ++i)
      (**i).updatexRef(x);

    for(vector<ExtraDynamic*>::iterator i = extraDynamic.begin(); i!= extraDynamic.end(); ++i) 
      (**i).updatexRef(x);
  }

  void DynamicSystem::updatexdRef(const Vec &xdParent) {
    xd >> xdParent(xInd,xInd+xSize-1);

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (**i).updatexdRef(xd);

    for(vector<Link*>::iterator i = link.begin(); i != link.end(); ++i)
      (**i).updatexdRef(xd);

    for(vector<ExtraDynamic*>::iterator i = extraDynamic.begin(); i!= extraDynamic.end(); ++i) 
      (**i).updatexdRef(xd);
  }

  void DynamicSystem::updatehRef(const Vec &hParent, int j) {
    h[j].resize() >> hParent(hInd[j],hInd[j]+hSize[j]-1);

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (**i).updatehRef(h[j],j);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (**i).updatehRef(h[j],j);

    for(vector<Link*>::iterator i = linkSingleValued.begin(); i != linkSingleValued.end(); ++i)
      (**i).updatehRef(h[j],j);

    for(vector<Link*>::iterator i = linkSetValued.begin(); i != linkSetValued.end(); ++i)
      if ((**i).hasSmoothPart())
        (**i).updatehRef(h[j],j);
  }

  void DynamicSystem::updatedhdqRef(const Mat &dhdqObjectParent, const Mat &dhdqLinkParent, int j) {
    dhdqObject.resize() >> dhdqObjectParent(Index(hInd[j],hInd[j]+hSize[j]-1),Index(qInd,qInd+qSize-1));
    dhdqLink.  resize() >> dhdqLinkParent  (Index(hInd[j],hInd[j]+hSize[j]-1),Index(qInd,qInd+qSize-1));

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i)
      (**i).updatedhdqRef(dhdqObject,dhdqLink,j);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (**i).updatedhdqRef(dhdqObject,j);

    for(vector<Link*>::iterator i = linkSingleValued.begin(); i != linkSingleValued.end(); ++i)
      (**i).updatedhdqRef(dhdqLink);

    for(vector<Link*>::iterator i = linkSetValuedNotActiveWithSmoothPart.begin(); i != linkSetValuedNotActiveWithSmoothPart.end(); ++i)
      (**i).updatedhdqRef(dhdqLink);
  }

  void DynamicSystem::updatedhduRef(const SqrMat &dhduObjectParent, const SqrMat &dhduLinkParent, int j) {
    dhduObject.resize() >> dhduObjectParent(Index(hInd[j],hInd[j]+hSize[j]-1),Index(uInd[0],uInd[0]+uSize[0]-1));
    dhduLink.  resize() >> dhduLinkParent  (Index(hInd[j],hInd[j]+hSize[j]-1),Index(uInd[0],uInd[0]+uSize[0]-1));

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i)
      (**i).updatedhduRef(dhduObject,dhduLink,j);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (**i).updatedhduRef(dhduObject,j);

    for(vector<Link*>::iterator i = linkSingleValued.begin(); i != linkSingleValued.end(); ++i)
      (**i).updatedhduRef(dhduLink);

    for(vector<Link*>::iterator i = linkSetValuedNotActiveWithSmoothPart.begin(); i != linkSetValuedNotActiveWithSmoothPart.end(); ++i)
      (**i).updatedhduRef(dhduLink);
  }

  void DynamicSystem::updatedhdtRef(const Vec &dhdtObjectParent, const Vec &dhdtLinkParent, int j) {
    dhdtObject.resize() >> dhdtObjectParent(hInd[j],hInd[j]+hSize[j]-1);
    dhdtLink.  resize() >> dhdtLinkParent  (hInd[j],hInd[j]+hSize[j]-1);

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i)
      (**i).updatedhdtRef(dhdtObject,dhdtLink,j);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (**i).updatedhdtRef(dhdtObject,j);

    for(vector<Link*>::iterator i = linkSingleValued.begin(); i != linkSingleValued.end(); ++i)
      (**i).updatedhdtRef(dhdtLink);

    for(vector<Link*>::iterator i = linkSetValuedNotActiveWithSmoothPart.begin(); i != linkSetValuedNotActiveWithSmoothPart.end(); ++i)
      (**i).updatedhdtRef(dhdtLink);
  }

  void DynamicSystem::updatefRef(const Vec &fParent) {
    f >> fParent(xInd,xInd+xSize-1);

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (**i).updatefRef(f);

    for(vector<Link*>::iterator i = link.begin(); i != link.end(); ++i) 
      (**i).updatefRef(f);
  }

  void DynamicSystem::updaterRef(const Vec &rParent, int j) {
    r[j].resize() >> rParent(hInd[j],hInd[j]+hSize[j]-1);

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (**i).updaterRef(r[j],j);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (**i).updaterRef(r[j],j);

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i)
      (**i).updaterRef(r[j],j);
  }

  void DynamicSystem::updateTRef(const Mat& TParent) {
    T >> TParent(Index(qInd,qInd+qSize-1),Index(uInd[0],uInd[0]+uSize[0]-1));

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updateTRef(T);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (*i)->updateTRef(T);
  }

  void DynamicSystem::updateMRef(const SymMat& MParent, int j) {
    M[j].resize() >> MParent(Index(hInd[j],hInd[j]+hSize[j]-1));

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updateMRef(M[j],j);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (*i)->updateMRef(M[j],j);
  }

  void DynamicSystem::updateLLMRef(const SymMat& LLMParent, int j) {
    LLM[j].resize() >> LLMParent(Index(hInd[j],hInd[j]+hSize[j]-1));

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updateLLMRef(LLM[j],j);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (*i)->updateLLMRef(LLM[j],j);
  }

  void DynamicSystem::updategRef(const Vec& gParent) {
    g.resize() >> gParent(gInd,gInd+gSize-1);

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updategRef(g);

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i) 
      (**i).updategRef(g);
  }

  void DynamicSystem::updategdRef(const Vec& gdParent) {
    gd.resize() >> gdParent(gdInd,gdInd+gdSize-1);

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updategdRef(gd);

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i) 
      (**i).updategdRef(gd);
  }

  void DynamicSystem::updatelaRef(const Vec &laParent) {
    la.resize() >> laParent(laInd,laInd+laSize-1);

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updatelaRef(la);

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i) 
      (**i).updatelaRef(la);

    for(vector<Link*>::iterator i = linkSetValuedNotActiveWithSmoothPart.begin(); i != linkSetValuedNotActiveWithSmoothPart.end(); ++i) 
      (**i).deletelaRef();
  }

  void DynamicSystem::updatelaInverseKineticsRef(const Vec &laParent) {
    laInverseKinetics.resize() >> laParent(0,laInverseKineticsSize-1);

    //for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
    //  (*i)->updatelaRefSpecial(la);

    for(vector<Link*>::iterator i = inverseKineticsLink.begin(); i != inverseKineticsLink.end(); ++i) 
      (**i).updatelaRef(laInverseKinetics);
  }

  void DynamicSystem::updatewbRef(const Vec &wbParent) {
    wb.resize() >> wbParent(laInd,laInd+laSize-1);

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updatewbRef(wb);

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i) 
      (**i).updatewbRef(wb);
  }

  void DynamicSystem::updateWRef(const Mat &WParent, int j) {
    W[j].resize() >> WParent(Index(hInd[j],hInd[j]+hSize[j]-1),Index(laInd,laInd+laSize-1));

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updateWRef(W[j],j);

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i) 
      (**i).updateWRef(W[j],j);
  }

  void DynamicSystem::updateWnVRefObjects() {

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updateWnVRefObjects();

    // TODO: Prüfen ob sauber
    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) {
      (**i).updateWRef(ds->getW(0),0);
      (**i).updateVRef(ds->getV(0),0);
      (**i).updateWRef(ds->getW(1),1);
      (**i).updateVRef(ds->getV(1),1);
    }
  }

  void DynamicSystem::updateWInverseKineticsRef(const Mat &WParent) {
    WInverseKinetics.resize() >> WParent(Index(hInd[1],hInd[1]+hSize[1]-1),Index(0,laInverseKineticsSize-1));

    for(vector<Link*>::iterator i = inverseKineticsLink.begin(); i != inverseKineticsLink.end(); ++i) 
      (**i).updateWRef(WInverseKinetics,1);
  }

  void DynamicSystem::updateVRef(const Mat &VParent, int j) {
    V[j].resize() >> VParent(Index(hInd[j],hInd[j]+hSize[j]-1),Index(laInd,laInd+laSize-1));

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updateVRef(V[j],j);

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i) 
      (**i).updateVRef(V[j],j);
  }

  void DynamicSystem::updatesvRef(const Vec &svParent) {
    sv >> svParent(svInd,svInd+svSize-1);

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updatesvRef(sv);

    for(vector<Link*>::iterator i = link.begin(); i != link.end(); ++i) 
      (**i).updatesvRef(sv);
  }

  void DynamicSystem::updatejsvRef(const Vector<int> &jsvParent) {
    jsv >> jsvParent(svInd,svInd+svSize-1);

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updatejsvRef(jsv);

    for(vector<Link*>::iterator i = link.begin(); i != link.end(); ++i) 
      (**i).updatejsvRef(jsv);
  }

  void DynamicSystem::updateresRef(const Vec &resParent) {
    res.resize() >> resParent(laInd,laInd+laSize-1);

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updateresRef(res);

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i) 
      (**i).updateresRef(res);
  }

  void DynamicSystem::updaterFactorRef(const Vec &rFactorParent) {
    rFactor.resize() >> rFactorParent(rFactorInd,rFactorInd+rFactorSize-1);

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updaterFactorRef(rFactor);

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i) 
      (**i).updaterFactorRef(rFactor);
  }

  void DynamicSystem::updateLinkStatusRef(const Vector<int> &LinkStatusParent) {
    LinkStatus.resize() >> LinkStatusParent(LinkStatusInd,LinkStatusInd+LinkStatusSize-1);

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updateLinkStatusRef(LinkStatus);

    for(vector<Link*>::iterator i = linkSetValued.begin(); i != linkSetValued.end(); ++i) 
      (**i).updateLinkStatusRef(LinkStatus);
  }

  void DynamicSystem::initz() {
    for(unsigned i=0; i<dynamicsystem.size(); i++)
      dynamicsystem[i]->initz();
    for(unsigned i=0; i<object.size(); i++)
      object[i]->initz();
    for(unsigned i=0; i<link.size(); i++)
      link[i]->initz();
    for(unsigned i=0; i<extraDynamic.size(); i++)
      extraDynamic[i]->initz();
  }

  void DynamicSystem::buildListOfObjects(vector<Object*> &obj, bool recursive) {
    for(unsigned int i=0; i<object.size(); i++) 
      obj.push_back(object[i]);
    if(recursive)
      for(unsigned int i=0; i<dynamicsystem.size(); i++)
        dynamicsystem[i]->buildListOfObjects(obj,recursive);
  }

  void DynamicSystem::buildListOfLinks(vector<Link*> &lnk, bool recursive) {
    for(unsigned int i=0; i<link.size(); i++)
      lnk.push_back(link[i]);
    if(recursive)
      for(unsigned int i=0; i<dynamicsystem.size(); i++)
        dynamicsystem[i]->buildListOfLinks(lnk,recursive);
  }

  void DynamicSystem::buildListOfSetValuedLinks(vector<Link*> &lnk, bool recursive) {
    for(unsigned int i=0; i<link.size(); i++)
      if(link[i]->isSetValued()) lnk.push_back(link[i]);
    if(recursive)
      for(unsigned int i=0; i<dynamicsystem.size(); i++)
        dynamicsystem[i]->buildListOfSetValuedLinks(lnk,recursive);
  }

  void DynamicSystem::buildListOfFrames(vector<Frame*> &frm, bool recursive) {
    for(unsigned int i=0; i<frame.size(); i++)
      frm.push_back(frame[i]);
    if(recursive)
      for(unsigned int i=0; i<dynamicsystem.size(); i++)
        dynamicsystem[i]->buildListOfFrames(frm,recursive);
  }

  void DynamicSystem::buildListOfContours(vector<Contour*> &cnt, bool recursive) {
    for(unsigned int i=0; i<contour.size(); i++)
      cnt.push_back(contour[i]);
    if(recursive)
      for(unsigned int i=0; i<dynamicsystem.size(); i++)
        dynamicsystem[i]->buildListOfContours(cnt,recursive);
  }

  void DynamicSystem::buildListOfExtraDynamic(vector<ExtraDynamic*> &ed, bool recursive) {
    for(unsigned int i=0; i<extraDynamic.size(); i++)
      ed.push_back(extraDynamic[i]);
    if(recursive)
      for(unsigned int i=0; i<dynamicsystem.size(); i++)
        dynamicsystem[i]->buildListOfExtraDynamic(ed,recursive);
  }

  void DynamicSystem::buildListOfModels(std::vector<ModellingInterface*> &modelList, bool recursive) {
    for(unsigned int i=0; i<model.size(); i++)
      modelList.push_back(model[i]);
    if(recursive)
      for(unsigned int i=0; i<dynamicsystem.size(); i++)
        dynamicsystem[i]->buildListOfModels(modelList,recursive);
  }

  void DynamicSystem::updateCondition() {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->updateCondition();

    for(vector<Link*>::iterator i = linkSetValued.begin(); i != linkSetValued.end(); ++i)
      (*i)->updateCondition();
  }

  void DynamicSystem::checkState() {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->checkState();

    for(vector<Link*>::iterator i = linkSetValued.begin(); i != linkSetValued.end(); ++i)
      (*i)->checkState();
  }

  void DynamicSystem::resizeJacobians(int j) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->resizeJacobians(j);

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (*i)->resizeJacobians(j);

    for(vector<Link*>::iterator i = link.begin(); i != link.end(); ++i) 
      (*i)->resizeJacobians(j);

    for(vector<Link*>::iterator i = inverseKineticsLink.begin(); i != inverseKineticsLink.end(); ++i) 
      (*i)->resizeJacobians(j);
  }

  void DynamicSystem::checkForConstraints() {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->checkForConstraints();

    for(vector<Object*>::iterator i = object.begin(); i != object.end(); ++i) 
      (*i)->checkForConstraints();
  }

  void DynamicSystem::setUpLinks() {
    for(unsigned int i=0; i<dynamicsystem.size(); i++) 
      dynamicsystem[i]->setUpLinks();

    // clear container first, because setUpLinks in called twice from InitStage resize (before and after the reorganization)
    linkSetValued.clear();
    linkSetValuedActive.clear();
    linkSetValuedNotActiveWithSmoothPart.clear();
    linkSingleValued.clear();
    for(unsigned int i=0; i<link.size(); i++) {
      if(link[i]->isSetValued()) {
        linkSetValued.push_back(link[i]);
        linkSetValuedActive.push_back(link[i]);
      }
      else 
        linkSingleValued.push_back(link[i]);
    }
  }

  bool DynamicSystem::gActiveChanged() {
    bool changed = false;

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      if ((*i)->gActiveChanged())
        changed = true;

    for(vector<Link*>::iterator i = linkSetValued.begin(); i != linkSetValued.end(); ++i) 
      if ((*i)->gActiveChanged())
        changed = true;

    return changed;
  }

  void DynamicSystem::calcxSize() {
    xSize = 0;

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) {
      (*i)->calcxSize();
      (*i)->setxInd(xSize);
      xSize += (*i)->getxSize();
    }

    for(vector<Link*>::iterator i = link.begin(); i != link.end(); ++i) {
      (*i)->calcxSize();
      (*i)->setxInd(xSize);
      xSize += (*i)->getxSize();
    }

    for(vector<ExtraDynamic*>::iterator i = extraDynamic.begin(); i != extraDynamic.end(); ++i) {
      (*i)->calcxSize();
      (*i)->setxInd(xSize);
      xSize += (*i)->getxSize();
    }
  }

  void DynamicSystem::calcLinkStatusSize() {
    LinkStatusSize = 0;

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) {
      (*i)->calcLinkStatusSize();
      (*i)->setLinkStatusInd(LinkStatusSize);
      LinkStatusSize += (*i)->getLinkStatusSize();
    }
    for(vector<Link*>::iterator i = linkSetValued.begin(); i != linkSetValued.end(); ++i) {
      (*i)->calcLinkStatusSize();
      (*i)->setLinkStatusInd(LinkStatusSize);
      LinkStatusSize += (*i)->getLinkStatusSize();
    }
  }

  void DynamicSystem::calcsvSize() {
    svSize = 0;

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) {
      (*i)->calcsvSize();
      (*i)->setsvInd(svSize);
      svSize += (*i)->getsvSize();
    }
    for(vector<Link*>::iterator i = link.begin(); i != link.end(); ++i) {
      (*i)->calcsvSize();
      (*i)->setsvInd(svSize);
      svSize += (*i)->getsvSize();
    }
  }

  void DynamicSystem::calclaSize() {
    laSize = 0;

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) {
      (*i)->calclaSize();
      (*i)->setlaInd(laSize);
      laSize += (*i)->getlaSize();
    }

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i) {
      (*i)->calclaSize();
      (*i)->setlaInd(laSize);
      laSize += (*i)->getlaSize();
    }
  }

  void DynamicSystem::calclaInverseKineticsSize() {
    laInverseKineticsSize = 0;

    for(vector<Link*>::iterator i = inverseKineticsLink.begin(); i != inverseKineticsLink.end(); ++i) {
      (*i)->calclaSize();
      (*i)->setlaInd(laInverseKineticsSize);
      laInverseKineticsSize += (*i)->getlaSize();
    }
  }

  void DynamicSystem::calclaSizeForActiveg() {
    laSize = 0;

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) {
      (*i)->calclaSizeForActiveg();
      (*i)->setlaInd(laSize);
      laSize += (*i)->getlaSize();
    }

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i) {
      (*i)->calclaSizeForActiveg();
      (*i)->setlaInd(laSize);
      laSize += (*i)->getlaSize();
    }
  }

  void DynamicSystem::calcgSize() {
    gSize = 0;

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) {
      (*i)->calcgSize();
      (*i)->setgInd(gSize);
      gSize += (*i)->getgSize();
    }

    for(vector<Link*>::iterator i = linkSetValued.begin(); i != linkSetValued.end(); ++i) {
      (*i)->calcgSize();
      (*i)->setgInd(gSize);
      gSize += (*i)->getgSize();
    }
  }

  void DynamicSystem::calcgSizeActive() {
    gSize = 0;

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) {
      (*i)->calcgSizeActive();
      (*i)->setgInd(gSize);
      gSize += (*i)->getgSize();
    }

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i) {
      (*i)->calcgSizeActive();
      (*i)->setgInd(gSize);
      gSize += (*i)->getgSize();
    }
  }

  void DynamicSystem::calcgdSize() {
    gdSize = 0;

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) {
      (*i)->calcgdSize();
      (*i)->setgdInd(gdSize);
      gdSize += (*i)->getgdSize();
    }

    for(vector<Link*>::iterator i = linkSetValued.begin(); i != linkSetValued.end(); ++i) {
      (*i)->calcgdSize();
      (*i)->setgdInd(gdSize);
      gdSize += (*i)->getgdSize();
    }
  }

  void DynamicSystem::calcgdSizeActive() {
    gdSize = 0;

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) {
      (*i)->calcgdSizeActive();
      (*i)->setgdInd(gdSize);
      gdSize += (*i)->getgdSize();
    }

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i) {
      (*i)->calcgdSizeActive();
      (*i)->setgdInd(gdSize);
      gdSize += (*i)->getgdSize();
    }
  }

  void DynamicSystem::calcrFactorSize() {
    rFactorSize = 0;

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) {
      (*i)->calcrFactorSize();
      (*i)->setrFactorInd(rFactorSize);
      rFactorSize += (*i)->getrFactorSize();
    }

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i) {
      (*i)->calcrFactorSize();
      (*i)->setrFactorInd(rFactorSize);
      rFactorSize += (*i)->getrFactorSize();
    }
  }

  void DynamicSystem::checkActiveLinks() {

    linkSetValuedActive.clear();
    linkSetValuedNotActiveWithSmoothPart.clear();

    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->checkActiveLinks();

    for(vector<Link*>::iterator i = linkSetValued.begin(); i != linkSetValued.end(); ++i) {
      if((*i)->isActive())
        linkSetValuedActive.push_back(*i);
      else if((*i)->hasSmoothPart())
        linkSetValuedNotActiveWithSmoothPart.push_back(*i);
    }
  }

  void DynamicSystem::checkActiveg() {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->checkActiveg();

    for(vector<Link*>::iterator i = linkSetValued.begin(); i != linkSetValued.end(); ++i)
      (*i)->checkActiveg();
  }

  void DynamicSystem::checkActivegd() {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->checkActivegd();

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i)
      (*i)->checkActivegd();
  }

  void DynamicSystem::checkActivegdn() {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->checkActivegdn();

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i)
      (*i)->checkActivegdn();
  }

  void DynamicSystem::checkActivegdd() {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->checkActivegdd();

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i)
      (*i)->checkActivegdd();
  }

  void DynamicSystem::checkAllgd() {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i != dynamicsystem.end(); ++i) 
      (*i)->checkAllgd();

    for(vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i)
      (*i)->checkAllgd();
  }

  void DynamicSystem::setgTol(double tol) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i!= dynamicsystem.end(); ++i)
      (**i).setgTol(tol);
    for(vector<Link*>::iterator i = link.begin(); i!= link.end(); ++i)
      (**i).setgTol(tol);
  }

  void DynamicSystem::setgdTol(double tol) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i!= dynamicsystem.end(); ++i)
      (**i).setgdTol(tol);
    for(vector<Link*>::iterator i = link.begin(); i!= link.end(); ++i)
      (**i).setgdTol(tol);
  }

  void DynamicSystem::setgddTol(double tol) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i!= dynamicsystem.end(); ++i)
      (**i).setgddTol(tol);
    for(vector<Link*>::iterator i = link.begin(); i!= link.end(); ++i)
      (**i).setgddTol(tol);
  }

  void DynamicSystem::setlaTol(double tol) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i!= dynamicsystem.end(); ++i)
      (**i).setlaTol(tol);
    for(vector<Link*>::iterator i = link.begin(); i!= link.end(); ++i)
      (**i).setlaTol(tol);
  }

  void DynamicSystem::setLaTol(double tol) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i!= dynamicsystem.end(); ++i)
      (**i).setLaTol(tol);
    for(vector<Link*>::iterator i = link.begin(); i!= link.end(); ++i)
      (**i).setLaTol(tol);
  }

  void DynamicSystem::setrMax(double rMax) {
    for(vector<DynamicSystem*>::iterator i = dynamicsystem.begin(); i!= dynamicsystem.end(); ++i)
      (**i).setrMax(rMax);
    for(vector<Link*>::iterator i = link.begin(); i!= link.end(); ++i)
      (**i).setrMax(rMax);
  }

  void DynamicSystem::addFrame(Frame* cosy) {
    if(getFrame(cosy->getName(),false)) { 
      throw MBSimError("The DynamicSystem \""+name+"\" can only comprises one Frame by the name \""+name+"\"!");
      assert(getFrame(cosy->getName(),false)==NULL);
    }
    frame.push_back(cosy);
    cosy->setParent(this);
  }

  void DynamicSystem::addFrame(Frame* cosy, const Vec &RrRF, const SqrMat &ARF, const string& refFrameName) {
    addFrame(cosy);

    saved_refFrameF.push_back(refFrameName);
    saved_RrRF.push_back(RrRF.copy()); // use .copy() because the copy constructor of fmatvec is a reference
    saved_ARF.push_back(ARF.copy()); // use .copy() because the copy constructor of fmatvec is a reference
    IrOF.push_back(Vec(3));
    AIF.push_back(SqrMat(3));
  }

  void DynamicSystem::addFrame(Frame *frame_, const fmatvec::Vec &RrRF, const fmatvec::SqrMat &ARF, const Frame* refFrame) {
    addFrame(frame_, RrRF, ARF, refFrame?refFrame->getName():"I");
  }

  void DynamicSystem::addFrame(const string &str, const Vec &RrRF, const SqrMat &ARF, const Frame* refFrame) {
    addFrame(new Frame(str),RrRF,ARF,refFrame);
  }

  void DynamicSystem::addContour(Contour* contour_) {
    if(getContour(contour_->getName(),false)) { 
      throw MBSimError("The DynamicSystem \""+name+"\" can only comprise one Contour by the name \""+name+"\"!");
      assert(getContour(contour_->getName(),false)==NULL);
    }
    contour.push_back(contour_);
    contour_->setParent(this);
  }

  void DynamicSystem::addContour(Contour* contour, const Vec &RrRC, const SqrMat &ARC, const string& refFrameName) {
    addContour(contour);

    saved_refFrameC.push_back(refFrameName);
    saved_RrRC.push_back(RrRC.copy()); // use .copy() because the copy constructor of fmatvec is a reference
    saved_ARC.push_back(ARC.copy()); // use .copy() because the copy constructor of fmatvec is a reference
    IrOC.push_back(Vec(3));
    AIC.push_back(SqrMat(3));
  }

  void DynamicSystem::addContour(Contour* contour, const fmatvec::Vec &RrRC, const fmatvec::SqrMat &ARC, const Frame* refFrame) {
    addContour(contour, RrRC, ARC, refFrame?refFrame->getName():"I");
  }

  int DynamicSystem::frameIndex(const Frame *frame_) const {
    for(unsigned int i=0; i<frame.size(); i++) {
      if(frame_==frame[i])
        return i;
    }
    return -1;
  }

  DynamicSystem* DynamicSystem::getGroup(const string &name, bool check) {
    unsigned int i;
    for(i=0; i<dynamicsystem.size(); i++) {
      if(dynamicsystem[i]->getName() == name)
        return dynamicsystem[i];
    }
    if(check){
      if(!(i<dynamicsystem.size()))
        throw MBSimError("The DynamicSystem \""+this->name+"\" comprises no DynamicSystem \""+name+"\"!");
      assert(i<dynamicsystem.size());
    }
    return NULL;
  }

  Object* DynamicSystem::getObject(const string &name, bool check) {
    unsigned int i;
    for(i=0; i<object.size(); i++) {
      if(object[i]->getName() == name)
        return object[i];
    }
    if(check){
      if(!(i<object.size()))
        throw MBSimError("The DynamicSystem \""+this->name+"\" comprises no Object \""+name+"\"!");
      assert(i<object.size());
    }
    return NULL;
  }

  void DynamicSystem::addLink(Link *lnk) {
    if(getLink(lnk->getName(),false)) {
      throw MBSimError("The DynamicSystem \""+name+"\" can only comprise one Link by the name \""+lnk->getName()+"\"!");
      assert(getLink(lnk->getName(),false) == NULL);
    }

    link.push_back(lnk);
    lnk->setParent(this);
  }

  void DynamicSystem::addInverseKineticsLink(Link *lnk) {
    //if(getLink(lnk->getName(),false)) {
    //  cout << "ERROR (DynamicSystem: addLink): The DynamicSystem " << name << " can only comprise one Link by the name " <<  lnk->getName() << "!" << endl;
    //  assert(getLink(lnk->getName(),false) == NULL);
    //}
    inverseKineticsLink.push_back(lnk);
    lnk->setParent(this);
  }

  Link* DynamicSystem::getLink(const string &name, bool check) {
    unsigned int i;
    for(i=0; i<link.size(); i++) {
      if(link[i]->getName() == name)
        return link[i];
    }
    if(check){
      if(!(i<link.size()))
        throw MBSimError("The DynamicSystem \""+this->name+"\" comprises no Link \""+name+"\"!");
      assert(i<link.size());
    }
    return NULL;
  }

  void DynamicSystem::addExtraDynamic(ExtraDynamic *ed_) {
    if(getExtraDynamic(ed_->getName(),false)) {
        throw MBSimError("The DynamicSystem \""+name+"\" can only comprise one ExtraDynamic by the name \""+ed_->getName()+"\"!");
      assert(getExtraDynamic(ed_->getName(),false) == NULL);
    }
    extraDynamic.push_back(ed_);
    ed_->setParent(this);
  }

  ExtraDynamic* DynamicSystem::getExtraDynamic(const string &name, bool check) {
    unsigned int i;
    for(i=0; i<extraDynamic.size(); i++) {
      if(extraDynamic[i]->getName() == name)
        return extraDynamic[i];
    }
    if(check){
      if(!(i<extraDynamic.size()))
        throw MBSimError("The DynamicSystem \""+this->name+"\" comprises no ExtraDynamic \""+name+"\"!");
      assert(i<extraDynamic.size());
    }
    return NULL;
  }

  void DynamicSystem::addModel(ModellingInterface *model_) {
    if(getModel(model_->getName(),false)) {
      throw MBSimError("The DynamicSystem \""+name+"\" can only comprise one model by the name \""+model_->getName()+"\"!");
      assert(getModel(model_->getName(),false) == NULL); 
    }
    model.push_back(model_);
    model_->setParent(this);
  }

  ModellingInterface* DynamicSystem::getModel(const string &name, bool check) {
    unsigned int i;
    for(i=0; i<model.size(); i++) {
      if(model[i]->getName() == name)
        return model[i];
    }
    if(check){
      if(!(i<model.size()))
        throw MBSimError("The DynamicSystem \""+this->name+"\" comprises no model \""+name+"\"!");
      assert(i<model.size());
    }
    return NULL;
  }

  void DynamicSystem::addGroup(DynamicSystem *sys) {
    if(getGroup(sys->getName(),false)) {
      throw MBSimError("The DynamicSystem \""+name+"\" can only comprise one DynamicSystem by the name \""+sys->getName()+"\"!");
      assert(getGroup(sys->getName(),false) == NULL); 
    }
    dynamicsystem.push_back(sys);
    sys->setParent(this);
  }

  void DynamicSystem::addObject(Object *obj) {
    if(getObject(obj->getName(),false)) {
      throw MBSimError("The DynamicSystem \""+name+"\" can only comprise one Object by the name \""+obj->getName()+"\"!");
      assert(getObject(obj->getName(),false) == NULL); 
    }
    object.push_back(obj);
    obj->setParent(this);
  }

  Element * DynamicSystem::getByPathSearch(string path) {
    if (path.substr(0, 1)=="/") { // absolut path
      if(parent)
        return parent->getByPathSearch(path);
      else
        return getByPathSearch(path.substr(1));
    }
    else if (path.substr(0, 3)=="../") // relative path
      return parent->getByPathSearch(path.substr(3));
    else { // local path
      size_t pos0=path.find_first_of("[");
      string container=path.substr(0, pos0);
      size_t pos1=path.find_first_of("]", pos0);
      string searched_name=path.substr(pos0+1, pos1-pos0-1);
      if(path.length()>pos1+1) { // weiter absteigen
        string rest=path.substr(pos1+2);
        if (container=="Object")
          return getObject(searched_name)->getByPathSearch(rest);
        else if (container=="Link")
          return getLink(searched_name)->getByPathSearch(rest);
        else if (container=="ExtraDynamic")
          return getExtraDynamic(searched_name)->getByPathSearch(rest);
        else if (container=="Group")
          return getGroup(searched_name)->getByPathSearch(rest);
        else
          throw MBSimError("Unknown name of container");
      }
      else {
        if (container=="Object")
          return getObject(searched_name);
        else if (container=="Link")
          return getLink(searched_name);
        else if (container=="ExtraDynamic")
          return getExtraDynamic(searched_name);
        else if (container=="Group")
          return getGroup(searched_name);
        else if (container=="Frame")
          return getFrame(searched_name);
        else if (container=="Contour")
          return getContour(searched_name);
        else
          throw MBSimError("Unknown name of container");
      }
    }
  }
}
