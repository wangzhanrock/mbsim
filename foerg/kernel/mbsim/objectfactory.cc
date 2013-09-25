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
 * Contact: friedrich.at.gc@googlemail.com
 */

#include <mbsim/objectfactory.h>

// NOTE: We can skip this code completely if everything is derived from e.g. 
// class Atom.

// NOTE: On Windows each static ObjectFactory<BaseType>::instance() function must only be defined ones
// (in one DLL). If not instance() will not return the same singleton object across DLLs.
// Hence we write the template defintion of instance() here and explicitly instantiate all available BaseType's here.
// If a MBSim modules defines a modules specific BaseType then the same must be done in a objectfactory.cc file in
// this MBSim module.

// create an singleton instance of the object factory.
// Defition for all BaseType here
#include <mbsim/objectfactory_instance.h>
//#include "mbsim/utils/function.h"

using namespace fmatvec;

namespace fmatvec {
  template<typename Sig>
    class Function;
}

namespace MBSim {

// forward declaration of all classed uses as BaseType
class Element;
class Environment;
class Integrator;
class GeneralizedForceLaw;
class GeneralizedImpactLaw;
class FrictionForceLaw;
class FrictionImpactLaw;
class ContourFunction1s;
class Translation;
class Rotation;

// explicit instantiation for all BaseType's here
template ObjectFactory<Function<double(double)>   >& ObjectFactory<Function<double(double)>   >::instance();
template ObjectFactory<Function<Vec(double)>   >& ObjectFactory<Function<Vec(double)>   >::instance();
template ObjectFactory<Function<VecV(double)>   >& ObjectFactory<Function<VecV(double)>   >::instance();
template ObjectFactory<Function<Vec3(double)>   >& ObjectFactory<Function<Vec3(double)>   >::instance();
template ObjectFactory<Function<Vec(Vec)>   >& ObjectFactory<Function<Vec(Vec)>   >::instance();
template ObjectFactory<Function<VecV(VecV)>   >& ObjectFactory<Function<VecV(VecV)>   >::instance();
template ObjectFactory<Function<VecV(Vec3)>   >& ObjectFactory<Function<VecV(Vec3)>   >::instance();
template ObjectFactory<Function<Vec3(VecV)>   >& ObjectFactory<Function<Vec3(VecV)>   >::instance();
template ObjectFactory<Function<Vec3(Vec3)>   >& ObjectFactory<Function<Vec3(Vec3)>   >::instance();
template ObjectFactory<Function<double(Vec)>   >& ObjectFactory<Function<double(Vec)>   >::instance();
template ObjectFactory<Function<double(VecV)>   >& ObjectFactory<Function<double(VecV)>   >::instance();
template ObjectFactory<Function<double(Vec3)>   >& ObjectFactory<Function<double(Vec3)>   >::instance();

template ObjectFactory<Function<double(double,double)>   >& ObjectFactory<Function<double(double,double)>   >::instance();
template ObjectFactory<Function<Vec(Vec,double)>   >& ObjectFactory<Function<Vec(Vec,double)>   >::instance();
template ObjectFactory<Function<VecV(VecV,double)>   >& ObjectFactory<Function<VecV(VecV,double)>   >::instance();
template ObjectFactory<Function<VecV(Vec3,double)>   >& ObjectFactory<Function<VecV(Vec3,double)>   >::instance();
template ObjectFactory<Function<Vec3(VecV,double)>   >& ObjectFactory<Function<Vec3(VecV,double)>   >::instance();
template ObjectFactory<Function<Vec3(Vec3,double)>   >& ObjectFactory<Function<Vec3(Vec3,double)>   >::instance();
template ObjectFactory<Function<Vec(Vec,Vec)>   >& ObjectFactory<Function<Vec(Vec,Vec)>   >::instance();
template ObjectFactory<Function<double(Vec2,Vec2)>   >& ObjectFactory<Function<double(Vec2,Vec2)>   >::instance();

template ObjectFactory<Element             >& ObjectFactory<Element             >::instance();
template ObjectFactory<Environment         >& ObjectFactory<Environment         >::instance();
template ObjectFactory<Integrator          >& ObjectFactory<Integrator          >::instance();
template ObjectFactory<GeneralizedForceLaw >& ObjectFactory<GeneralizedForceLaw >::instance();
template ObjectFactory<GeneralizedImpactLaw>& ObjectFactory<GeneralizedImpactLaw>::instance();
template ObjectFactory<FrictionForceLaw    >& ObjectFactory<FrictionForceLaw    >::instance();
template ObjectFactory<FrictionImpactLaw   >& ObjectFactory<FrictionImpactLaw   >::instance();
template ObjectFactory<ContourFunction1s   >& ObjectFactory<ContourFunction1s   >::instance();
template ObjectFactory<Translation         >& ObjectFactory<Translation         >::instance();
template ObjectFactory<Rotation            >& ObjectFactory<Rotation            >::instance();

}