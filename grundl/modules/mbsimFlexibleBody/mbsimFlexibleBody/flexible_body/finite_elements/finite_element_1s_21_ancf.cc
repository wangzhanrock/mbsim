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
 * Contact: thorsten.schindler@mytum.de
 */

#include <config.h>
#include "mbsimFlexibleBody/flexible_body/finite_elements/finite_element_1s_21_ancf.h"
#include <mbsim/utils/eps.h>

using namespace std;
using namespace fmatvec;
using namespace MBSim;

namespace MBSimFlexibleBody {

  SqrMat differentiate_normalized_vector_respective_vector(const Vec &vector) {
    double norm = nrm2(vector);
    return static_cast<SqrMat>((SqrMat(vector.size(),EYE) - vector*vector.T()/pow(norm,2.))/norm).copy();
  }

  FiniteElement1s21ANCF::FiniteElement1s21ANCF(double sl0, double sArho, double sEA, double sEI, Vec sg) :l0(sl0), Arho(sArho), EA(sEA), EI(sEI), wss0(0.), depsilon(0.), g(sg), M(8,INIT,0.), h(8,INIT,0.), Damp(8,INIT,0.), Dhq(8,INIT,0.), Dhqp(8,INIT,0.) {}

  FiniteElement1s21ANCF::~FiniteElement1s21ANCF() {
  }

  void FiniteElement1s21ANCF::setCurlRadius(double R) {
    if (fabs(R) < epsroot()) throw MBSimError("CurlRadius must not be 0!\n");
    wss0 = 1/R;
  }

  void FiniteElement1s21ANCF::setMaterialDamping(double depsilons) {
    throw MBSim::MBSimError("Error(FiniteElement1s21ANCF::setMaterialDamping): Not implemented");
    //  depsilon  += depsilons;
    //  Damp(3,3) += -depsilon;
  }

  void FiniteElement1s21ANCF::setLehrDamping(double D) {
    throw MBSim::MBSimError("Error(FiniteElement1s21ANCF::setLehrDamping): Not implemented");
    //  // Longitudinaleifenfrequenz
    //  double weps = sqrt(12.*EA/(Arho*l0h2));
    //  // Biegeeigenfrequenz
    //  double wbL  = sqrt(1260.*128.*EI/(197.*Arho*l0h4));

    //  depsilon  = 1.0 * D * weps * (Arho*l0h3/12.);
    //  Damp(3,3) = -depsilon;
  }

  void FiniteElement1s21ANCF::initM() {
    M(0,0) = Arho*l0*(1.3E1/3.5E1);
    M(0,2) = Arho*(l0*l0)*(1.1E1/2.1E2);
    M(0,4) = Arho*l0*(9.0/7.0E1);
    M(0,6) = Arho*(l0*l0)*(-1.3E1/4.2E2);
    M(1,1) = Arho*l0*(1.3E1/3.5E1);
    M(1,3) = Arho*(l0*l0)*(1.1E1/2.1E2);
    M(1,5) = Arho*l0*(9.0/7.0E1);
    M(1,7) = Arho*(l0*l0)*(-1.3E1/4.2E2);
    M(2,2) = Arho*(l0*l0*l0)*(1.0/1.05E2);
    M(2,4) = Arho*(l0*l0)*(1.3E1/4.2E2);
    M(2,6) = Arho*(l0*l0*l0)*(-1.0/1.4E2);
    M(3,3) = Arho*(l0*l0*l0)*(1.0/1.05E2);
    M(3,5) = Arho*(l0*l0)*(1.3E1/4.2E2);
    M(3,7) = Arho*(l0*l0*l0)*(-1.0/1.4E2);
    M(4,4) = Arho*l0*(1.3E1/3.5E1);
    M(4,6) = Arho*(l0*l0)*(-1.1E1/2.1E2);
    M(5,5) = Arho*l0*(1.3E1/3.5E1);
    M(5,7) = Arho*(l0*l0)*(-1.1E1/2.1E2);
    M(6,6) = Arho*(l0*l0*l0)*(1.0/1.05E2);
    M(7,7) = Arho*(l0*l0*l0)*(1.0/1.05E2);
  }

  void FiniteElement1s21ANCF::computeh(const Vec& qElement, const Vec& qpElement) {
    // Coordinates
    const double & x1 = qElement(0);    const double & y1 = qElement(1);
    const double &dx1 = qElement(2);    const double &dy1 = qElement(3);
    const double & x2 = qElement(4);    const double & y2 = qElement(5);
    const double &dx2 = qElement(6);    const double &dy2 = qElement(7);

    // Gravitation
    double gx = g(0);
    double gy = g(1);

    Vec hgrav(8,NONINIT);
    hgrav(0) = Arho*gx*l0*(1.0/2.0);
    hgrav(1) = Arho*gy*l0*(1.0/2.0);
    hgrav(2) = Arho*gx*(l0*l0)*(1.0/1.2E1);
    hgrav(3) = Arho*gy*(l0*l0)*(1.0/1.2E1);
    hgrav(4) = Arho*gx*l0*(1.0/2.0);
    hgrav(5) = Arho*gy*l0*(1.0/2.0);
    hgrav(6) = Arho*gx*(l0*l0)*(-1.0/1.2E1);
    hgrav(7) = Arho*gy*(l0*l0)*(-1.0/1.2E1);

    // Interior forces
    // Local coordinate system
    Vec e1(2,NONINIT);
    e1(0) = x2-x1; e1(1) = y2-y1;
    e1 /= nrm2(e1);
    const double &e1x = e1(0); const double &e1y = e1(1);

    Mat dvec1dq(2,8,INIT,0.);
    dvec1dq(0,0) = -1.; dvec1dq(0,4) = 1.;
    dvec1dq(1,1) = -1.; dvec1dq(1,5) = 1.;

    Mat de1dq = differentiate_normalized_vector_respective_vector(e1)*dvec1dq;

    Vec e2(2,NONINIT);
    e2(0) = -e1(1); e2(1) = e1(0);
    const double &e2x = e2(0); const double &e2y = e2(1);

    Mat dvec2dq(2,8,INIT,0.);
    dvec2dq(0,1) = 1.; dvec2dq(0,5) = -1.;
    dvec2dq(1,0) = -1.; dvec2dq(1,4) = 1.;

    Mat de2dq = differentiate_normalized_vector_respective_vector(e2)*dvec2dq;

    // Strain
    Vec hst_q(8,NONINIT);
    hst_q(0) = (EA*e1x*(l0*1.0E1+e1x*x1*1.2E1-e1x*x2*1.2E1+e1y*y1*1.2E1-e1y*y2*1.2E1+dx1*e1x*l0+dx2*e1x*l0+dy1*e1y*l0+dy2*e1y*l0)*(-1.0/1.0E1))/l0;
    hst_q(1) = (EA*e1y*(l0*1.0E1+e1x*x1*1.2E1-e1x*x2*1.2E1+e1y*y1*1.2E1-e1y*y2*1.2E1+dx1*e1x*l0+dx2*e1x*l0+dy1*e1y*l0+dy2*e1y*l0)*(-1.0/1.0E1))/l0;
    hst_q(2) = EA*e1x*(e1x*x1*3.0-e1x*x2*3.0+e1y*y1*3.0-e1y*y2*3.0+dx1*e1x*l0*4.0-dx2*e1x*l0+dy1*e1y*l0*4.0-dy2*e1y*l0)*(-1.0/3.0E1);
    hst_q(3) = EA*e1y*(e1x*x1*3.0-e1x*x2*3.0+e1y*y1*3.0-e1y*y2*3.0+dx1*e1x*l0*4.0-dx2*e1x*l0+dy1*e1y*l0*4.0-dy2*e1y*l0)*(-1.0/3.0E1);
    hst_q(4) = (EA*e1x*(l0*1.0E1+e1x*x1*1.2E1-e1x*x2*1.2E1+e1y*y1*1.2E1-e1y*y2*1.2E1+dx1*e1x*l0+dx2*e1x*l0+dy1*e1y*l0+dy2*e1y*l0)*(1.0/1.0E1))/l0;
    hst_q(5) = (EA*e1y*(l0*1.0E1+e1x*x1*1.2E1-e1x*x2*1.2E1+e1y*y1*1.2E1-e1y*y2*1.2E1+dx1*e1x*l0+dx2*e1x*l0+dy1*e1y*l0+dy2*e1y*l0)*(1.0/1.0E1))/l0;
    hst_q(6) = EA*e1x*(e1x*x1*3.0-e1x*x2*3.0+e1y*y1*3.0-e1y*y2*3.0-dx1*e1x*l0+dx2*e1x*l0*4.0-dy1*e1y*l0+dy2*e1y*l0*4.0)*(-1.0/3.0E1);
    hst_q(7) = EA*e1y*(e1x*x1*3.0-e1x*x2*3.0+e1y*y1*3.0-e1y*y2*3.0-dx1*e1x*l0+dx2*e1x*l0*4.0-dy1*e1y*l0+dy2*e1y*l0*4.0)*(-1.0/3.0E1);

    Vec hst_e1(2,NONINIT);
    hst_e1(0) = (EA*(l0*x1*3.0E1-l0*x2*3.0E1+e1x*(x1*x1)*3.6E1+e1x*(x2*x2)*3.6E1+(dx1*dx1)*e1x*(l0*l0)*4.0+(dx2*dx2)*e1x*(l0*l0)*4.0-e1x*x1*x2*7.2E1+e1y*x1*y1*3.6E1-e1y*x1*y2*3.6E1-e1y*x2*y1*3.6E1+e1y*x2*y2*3.6E1+dx1*e1x*l0*x1*6.0-dx1*e1x*l0*x2*6.0+dx2*e1x*l0*x1*6.0-dx2*e1x*l0*x2*6.0+dy1*e1y*l0*x1*3.0-dy1*e1y*l0*x2*3.0+dy2*e1y*l0*x1*3.0-dy2*e1y*l0*x2*3.0+dx1*e1y*l0*y1*3.0-dx1*e1y*l0*y2*3.0+dx2*e1y*l0*y1*3.0-dx2*e1y*l0*y2*3.0-dx1*dx2*e1x*(l0*l0)*2.0+dx1*dy1*e1y*(l0*l0)*4.0-dx1*dy2*e1y*(l0*l0)-dx2*dy1*e1y*(l0*l0)+dx2*dy2*e1y*(l0*l0)*4.0)*(-1.0/3.0E1))/l0;
    hst_e1(1) = (EA*(l0*y1*3.0E1-l0*y2*3.0E1+e1y*(y1*y1)*3.6E1+e1y*(y2*y2)*3.6E1+(dy1*dy1)*e1y*(l0*l0)*4.0+(dy2*dy2)*e1y*(l0*l0)*4.0+e1x*x1*y1*3.6E1-e1x*x1*y2*3.6E1-e1x*x2*y1*3.6E1+e1x*x2*y2*3.6E1-e1y*y1*y2*7.2E1+dy1*e1x*l0*x1*3.0-dy1*e1x*l0*x2*3.0+dy2*e1x*l0*x1*3.0-dy2*e1x*l0*x2*3.0+dx1*e1x*l0*y1*3.0-dx1*e1x*l0*y2*3.0+dx2*e1x*l0*y1*3.0-dx2*e1x*l0*y2*3.0+dy1*e1y*l0*y1*6.0-dy1*e1y*l0*y2*6.0+dy2*e1y*l0*y1*6.0-dy2*e1y*l0*y2*6.0+dx1*dy1*e1x*(l0*l0)*4.0-dx1*dy2*e1x*(l0*l0)-dx2*dy1*e1x*(l0*l0)+dx2*dy2*e1x*(l0*l0)*4.0-dy1*dy2*e1y*(l0*l0)*2.0)*(-1.0/3.0E1))/l0;

    // Bending
    Vec hbe_q(8,NONINIT);
    hbe_q(0) = EI*e2x*1.0/(l0*l0*l0)*(e2x*x1*2.0-e2x*x2*2.0+e2y*y1*2.0-e2y*y2*2.0+dx1*e2x*l0+dx2*e2x*l0+dy1*e2y*l0+dy2*e2y*l0)*-6.0;
    hbe_q(1) = EI*e2y*1.0/(l0*l0*l0)*(e2x*x1*2.0-e2x*x2*2.0+e2y*y1*2.0-e2y*y2*2.0+dx1*e2x*l0+dx2*e2x*l0+dy1*e2y*l0+dy2*e2y*l0)*-6.0;
    hbe_q(2) = -EI*e2x*1.0/(l0*l0)*(e2x*x1*6.0-e2x*x2*6.0+e2y*y1*6.0-e2y*y2*6.0+wss0*(l0*l0)+dx1*e2x*l0*4.0+dx2*e2x*l0*2.0+dy1*e2y*l0*4.0+dy2*e2y*l0*2.0);
    hbe_q(3) = -EI*e2y*1.0/(l0*l0)*(e2x*x1*6.0-e2x*x2*6.0+e2y*y1*6.0-e2y*y2*6.0+wss0*(l0*l0)+dx1*e2x*l0*4.0+dx2*e2x*l0*2.0+dy1*e2y*l0*4.0+dy2*e2y*l0*2.0);
    hbe_q(4) = EI*e2x*1.0/(l0*l0*l0)*(e2x*x1*2.0-e2x*x2*2.0+e2y*y1*2.0-e2y*y2*2.0+dx1*e2x*l0+dx2*e2x*l0+dy1*e2y*l0+dy2*e2y*l0)*6.0;
    hbe_q(5) = EI*e2y*1.0/(l0*l0*l0)*(e2x*x1*2.0-e2x*x2*2.0+e2y*y1*2.0-e2y*y2*2.0+dx1*e2x*l0+dx2*e2x*l0+dy1*e2y*l0+dy2*e2y*l0)*6.0;
    hbe_q(6) = -EI*e2x*1.0/(l0*l0)*(e2x*x1*6.0-e2x*x2*6.0+e2y*y1*6.0-e2y*y2*6.0-wss0*(l0*l0)+dx1*e2x*l0*2.0+dx2*e2x*l0*4.0+dy1*e2y*l0*2.0+dy2*e2y*l0*4.0);
    hbe_q(7) = -EI*e2y*1.0/(l0*l0)*(e2x*x1*6.0-e2x*x2*6.0+e2y*y1*6.0-e2y*y2*6.0-wss0*(l0*l0)+dx1*e2x*l0*2.0+dx2*e2x*l0*4.0+dy1*e2y*l0*2.0+dy2*e2y*l0*4.0);

    Vec hbe_e2(2,NONINIT);
    hbe_e2(0) = -EI*1.0/(l0*l0*l0)*(x1*6.0-x2*6.0+dx1*l0*4.0+dx2*l0*2.0)*(e2x*x1*6.0-e2x*x2*6.0+e2y*y1*6.0-e2y*y2*6.0+wss0*(l0*l0)+dx1*e2x*l0*4.0+dx2*e2x*l0*2.0+dy1*e2y*l0*4.0+dy2*e2y*l0*2.0)+EI*1.0/(l0*l0*l0)*(x1*2.0-x2*2.0+dx1*l0*2.0)*(e2x*x1*2.0-e2x*x2*2.0+e2y*y1*2.0-e2y*y2*2.0+dx1*e2x*l0+dx2*e2x*l0+dy1*e2y*l0+dy2*e2y*l0)*3.0+EI*1.0/(l0*l0*l0)*(x1*2.0-x2*2.0+dx1*l0+dx2*l0)*(e2x*x1*2.0-e2x*x2*2.0+e2y*y1*2.0-e2y*y2*2.0+wss0*(l0*l0)+dx1*e2x*l0*2.0+dy1*e2y*l0*2.0)*3.0;
    hbe_e2(1) = -EI*1.0/(l0*l0*l0)*(y1*6.0-y2*6.0+dy1*l0*4.0+dy2*l0*2.0)*(e2x*x1*6.0-e2x*x2*6.0+e2y*y1*6.0-e2y*y2*6.0+wss0*(l0*l0)+dx1*e2x*l0*4.0+dx2*e2x*l0*2.0+dy1*e2y*l0*4.0+dy2*e2y*l0*2.0)+EI*1.0/(l0*l0*l0)*(y1*2.0-y2*2.0+dy1*l0*2.0)*(e2x*x1*2.0-e2x*x2*2.0+e2y*y1*2.0-e2y*y2*2.0+dx1*e2x*l0+dx2*e2x*l0+dy1*e2y*l0+dy2*e2y*l0)*3.0+EI*1.0/(l0*l0*l0)*(y1*2.0-y2*2.0+dy1*l0+dy2*l0)*(e2x*x1*2.0-e2x*x2*2.0+e2y*y1*2.0-e2y*y2*2.0+wss0*(l0*l0)+dx1*e2x*l0*2.0+dy1*e2y*l0*2.0)*3.0;

    h = hgrav + hst_q + de1dq.T()*hst_e1 + hbe_q + de2dq.T()*hbe_e2; 

    // // Daempfung------------------------------------------------------------------
    // //    hdLokal.init(0.0); beim Initialisieren
    //     hdLokal(3) = - depsilon * epsp; // eps

    //     // Impliziten Integratoren
    //     if(implicit)
    //     {
    // 	static Mat Dhz(2*8,8,fmatvec::INIT,0.0);
    // 	Dhz   = hFullJacobi(qElement,qpElement,qLokal,qpLokal,Jeg,Jegp,MLokal,hZwischen);
    // 	Dhq   = static_cast<SqrMat>(Dhz(0,0, 8-1,8-1));
    // 	Dhqp  = static_cast<SqrMat>(Dhz(8,0,2*8-1,8-1));
    // 	Dhqp += trans(Jeg)*Damp*Jeg;
    //     }

    //     cout << "qElement = " << trans(qElement);
    //     cout << "h        = " << trans(h)        << endl;
    //     throw 1;
  }

  Vec FiniteElement1s21ANCF::computePosition(const Vec& qElement, const ContourPointData& cp) {
    Vec pos(3,NONINIT);

    const double &s = cp.getLagrangeParameterPosition()(0);

    Mat S = GlobalShapeFunctions(s).T();

    Vec tmp = S*qElement;

    pos(0) = tmp(0);
    pos(1) = tmp(1);
    pos(2) = 0.;

    return pos.copy();
  }

  SqrMat FiniteElement1s21ANCF::computeOrientation(const Vec& qElement, const ContourPointData& cp) {
    SqrMat rotation(3,INIT,0.);

    const double & x1 = qElement(0);    const double & y1 = qElement(1);
    const double &dx1 = qElement(2);    const double &dy1 = qElement(3);
    const double & x2 = qElement(4);    const double & y2 = qElement(5);
    const double &dx2 = qElement(6);    const double &dy2 = qElement(7);

    const double &s = cp.getLagrangeParameterPosition()(0);

    // tangent
    Vec t = tangent(qElement, s);
    rotation(0,0) = t(0);
    rotation(1,0) = t(1);

    // normal
    rotation(0,1) = 1.0/sqrt(pow(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0)+pow(fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0))*(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0));
    rotation(1,1) = -1.0/sqrt(pow(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0)+pow(fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0))*(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0));

    // binormal
    rotation(2,2) = 1.;

    return rotation.copy();
  }

  Vec FiniteElement1s21ANCF::computeVelocity(const Vec& qElement, const Vec& qpElement, const ContourPointData& cp) {
    Vec vel(3,NONINIT);

    const double &s = cp.getLagrangeParameterPosition()(0);

    Mat S = GlobalShapeFunctions(s).T();

    Vec tmp = S*qpElement;

    vel(0) = tmp(0);
    vel(1) = tmp(1);
    vel(2) = 0.;

    return vel.copy();
  }

  Vec FiniteElement1s21ANCF::computeAngularVelocity(const Vec& qElement, const Vec& qpElement, const ContourPointData& cp) {
    Vec ang(3,INIT,0.);

    const double & x1 = qElement(0);    const double & y1 = qElement(1);
    const double &dx1 = qElement(2);    const double &dy1 = qElement(3);
    const double & x2 = qElement(4);    const double & y2 = qElement(5);
    const double &dx2 = qElement(6);    const double &dy2 = qElement(7);

    const double & x1p = qpElement(0);    const double & y1p = qpElement(1);
    const double &dx1p = qpElement(2);    const double &dy1p = qpElement(3);
    const double & x2p = qpElement(4);    const double & y2p = qpElement(5);
    const double &dx2p = qpElement(6);    const double &dy2p = qpElement(7);

    const double &s = cp.getLagrangeParameterPosition()(0);

    ang(2) = (1.0/sqrt(pow(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0)+pow(fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0))*(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0))*(y1p*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2p*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2p*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1p*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0))-1.0/sqrt(pow(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0)+pow(fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0))*(x1p*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2p*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2p*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1p*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0))*(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)))/max(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)));

    return ang.copy();
  }

  Vec FiniteElement1s21ANCF::LocateBalken(const Vec& qElement, const double& s) {
    Vec coordinates = computePosition(qElement, ContourPointData(s));

    Vec t = tangent(qElement,s);

    coordinates(2) = atan2(t(1),t(0));

    return coordinates.copy();
  }

  Vec FiniteElement1s21ANCF::StateBalken(const Vec& qElement, const Vec& qpElement, const double& s) {
    Mat J = JGeneralized(qElement,s).T();
    Vec X(6);
    X(Index(0,2)) = J* qElement;
    X(Index(3,5)) = J*qpElement;

    Vec t = tangent(qElement,s);

    X(2) = atan2(t(1),t(0));

    return X.copy();
  }

  Mat FiniteElement1s21ANCF::JGeneralized(const Vec& qElement, const double& s) {
    Mat J(8,3,INIT,0.);

    const double & x1 = qElement(0);    const double & y1 = qElement(1);
    const double &dx1 = qElement(2);    const double &dy1 = qElement(3);
    const double & x2 = qElement(4);    const double & y2 = qElement(5);
    const double &dx2 = qElement(6);    const double &dy2 = qElement(7);

    // calculate matrix of global shape functions
    Mat S = GlobalShapeFunctions(s);

    // Jacobian
    // first two colums, i.e. Jacobian of translation, correlate with matrix of global shape functions S
    J(Index(0,7),Index(0,1)) = S(Index(0,7),Index(0,1)).copy();

    // third column is the Jacobian of rotation
    J(0,2) = -(1.0/sqrt(pow(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0)+pow(fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0))*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)*(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)))/max(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)));
    J(1,2) = (1.0/sqrt(pow(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0)+pow(fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0))*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)*(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)))/max(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)));
    J(2,2) = (l0*1.0/sqrt(pow(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0)+pow(fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0))*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)*(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)))/max(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)));
    J(3,2) = -(l0*1.0/sqrt(pow(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0)+pow(fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0))*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)*(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)))/max(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)));
    J(4,2) = (1.0/sqrt(pow(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0)+pow(fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0))*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)*(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)))/max(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)));
    J(5,2) = -(1.0/sqrt(pow(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0)+pow(fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0))*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)*(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)))/max(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)));
    J(6,2) = -(l0*1.0/sqrt(pow(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0)+pow(fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0))*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)*(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)))/max(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)));
    J(7,2) = (l0*1.0/sqrt(pow(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0)+pow(fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0))*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)*(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)))/max(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)));

    return J.copy();
  }

  Mat FiniteElement1s21ANCF::GlobalShapeFunctions(const double& s) {
    Mat S(8,2,INIT,0.);

    S(0,0) = 1.0/(l0*l0)*(s*s)*-3.0+1.0/(l0*l0*l0)*(s*s*s)*2.0+1.0;
    S(2,0) = l0*(s/l0-1.0/(l0*l0)*(s*s)*2.0+1.0/(l0*l0*l0)*(s*s*s));
    S(4,0) = 1.0/(l0*l0)*(s*s)*3.0-1.0/(l0*l0*l0)*(s*s*s)*2.0;
    S(6,0) = -l0*(1.0/(l0*l0)*(s*s)-1.0/(l0*l0*l0)*(s*s*s));
    S(1,1) = 1.0/(l0*l0)*(s*s)*-3.0+1.0/(l0*l0*l0)*(s*s*s)*2.0+1.0;
    S(3,1) = l0*(s/l0-1.0/(l0*l0)*(s*s)*2.0+1.0/(l0*l0*l0)*(s*s*s));
    S(5,1) = 1.0/(l0*l0)*(s*s)*3.0-1.0/(l0*l0*l0)*(s*s*s)*2.0;
    S(7,1) = -l0*(1.0/(l0*l0)*(s*s)-1.0/(l0*l0*l0)*(s*s*s));

    return S.copy();
  }

  Vec FiniteElement1s21ANCF::tangent(const Vec& qElement, const double& s) {
    Vec t(3,INIT,0.);

    const double & x1 = qElement(0);    const double & y1 = qElement(1);
    const double &dx1 = qElement(2);    const double &dy1 = qElement(3);
    const double & x2 = qElement(4);    const double & y2 = qElement(5);
    const double &dx2 = qElement(6);    const double &dy2 = qElement(7);

    t(0) = -1.0/sqrt(pow(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0)+pow(fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0))*(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0));
    t(1) = -1.0/sqrt(pow(fabs(x1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-x2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dx2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dx1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0)+pow(fabs(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0)),2.0))*(y1*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)-y2*(1.0/(l0*l0)*s*6.0-1.0/(l0*l0*l0)*(s*s)*6.0)+dy2*l0*(1.0/(l0*l0)*s*2.0-1.0/(l0*l0*l0)*(s*s)*3.0)-dy1*l0*(1.0/(l0*l0)*s*-4.0+1.0/l0+1.0/(l0*l0*l0)*(s*s)*3.0));

    return t.copy();
  }
}

