/* -------------------------------------------------------------------------- *
 *                      SimTK Core: SimTK Simbody(tm)                         *
 * -------------------------------------------------------------------------- *
 * This is part of the SimTK Core biosimulation toolkit originating from      *
 * Simbios, the NIH National Center for Physics-Based Simulation of           *
 * Biological Structures at Stanford, funded under the NIH Roadmap for        *
 * Medical Research, grant U54 GM072970. See https://simtk.org.               *
 *                                                                            *
 * Portions copyright (c) 2008 Stanford University and the Authors.           *
 * Authors: Peter Eastman                                                     *
 * Contributors:                                                              *
 *                                                                            *
 * Permission is hereby granted, free of charge, to any person obtaining a    *
 * copy of this software and associated documentation files (the "Software"), *
 * to deal in the Software without restriction, including without limitation  *
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,   *
 * and/or sell copies of the Software, and to permit persons to whom the      *
 * Software is furnished to do so, subject to the following conditions:       *
 *                                                                            *
 * The above copyright notice and this permission notice shall be included in *
 * all copies or substantial portions of the Software.                        *
 *                                                                            *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    *
 * THE AUTHORS, CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,    *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR      *
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE  *
 * USE OR OTHER DEALINGS IN THE SOFTWARE.                                     *
 * -------------------------------------------------------------------------- */

#include "SimTKcommon.h"

#include "simbody/internal/common.h"
#include "simbody/internal/Contact.h"
#include "simbody/internal/GeneralContactSubsystem.h"
#include "simbody/internal/MobilizedBody.h"

#include "HuntCrossleyForceImpl.h"

using std::vector;

namespace SimTK {

SimTK_INSERT_DERIVED_HANDLE_DEFINITIONS(HuntCrossleyForce, HuntCrossleyForceImpl, Force);

HuntCrossleyForce::HuntCrossleyForce(GeneralForceSubsystem& forces, GeneralContactSubsystem& contacts, ContactSetIndex set) :
        Force(new HuntCrossleyForceImpl(contacts, set)) {
    updImpl().setForceIndex(forces.adoptForce(*this));
}

void HuntCrossleyForce::setBodyParameters(int bodyIndex, Real stiffness, Real dissipation) {
    updImpl().setBodyParameters(bodyIndex, stiffness, dissipation);
}

HuntCrossleyForceImpl::HuntCrossleyForceImpl(GeneralContactSubsystem& subsystem, ContactSetIndex set) : 
        subsystem(subsystem), set(set) {
}

void HuntCrossleyForceImpl::setBodyParameters(int bodyIndex, Real stiffness, Real dissipation) {
    updParameters(bodyIndex) = Parameters(stiffness, dissipation);
}

const HuntCrossleyForceImpl::Parameters& HuntCrossleyForceImpl::getParameters(int bodyIndex) const {
    assert(bodyIndex >= 0 && bodyIndex < subsystem.getNumBodies(set));
    if (bodyIndex >= parameters.size())
        const_cast<vector<Parameters>&>(parameters).resize(bodyIndex+1); // This fills in the default values which the missing entries implicitly had already.
    return parameters[bodyIndex];
}

HuntCrossleyForceImpl::Parameters& HuntCrossleyForceImpl::updParameters(int bodyIndex) {
    assert(bodyIndex >= 0 && bodyIndex < subsystem.getNumBodies(set));
    if (bodyIndex >= parameters.size())
        parameters.resize(bodyIndex+1);
    return parameters[bodyIndex];
}

void HuntCrossleyForceImpl::calcForce(const State& state, Vector_<SpatialVec>& bodyForces, Vector_<Vec3>& particleForces, Vector& mobilityForces) const {
    const vector<Contact>& contacts = subsystem.getContacts(state, set);
    for (int i = 0; i < contacts.size(); i++) {
        const Parameters& param1 = getParameters(contacts[i].getFirstBody());
        const Parameters& param2 = getParameters(contacts[i].getSecondBody());
        
        // Adjust the contact location based on the relative stiffness of the two materials.
        
        const Real s1 = param2.stiffness/(param1.stiffness+param2.stiffness);
        const Real s2 = 1-s1;
        const Real depth = contacts[i].getDepth();
        const Vec3& normal = contacts[i].getNormal();
        const Vec3 location = contacts[i].getLocation()+(depth*(0.5-s1))*normal;
        
        // Calculate the Hertz force.

        const Real k = param1.stiffness*s1;
        const Real c = param1.dissipation*s1 + param2.dissipation*s2;
        const Real radius = contacts[i].getRadius();
        const Real curvature = radius*radius/depth;
        const Real fH = (4.0/3.0)*k*depth*std::sqrt(curvature*k*depth);
        
        // Calculate the relative velocity of the two bodies at the contact point.
        
        const MobilizedBody& body1 = subsystem.getBody(set, contacts[i].getFirstBody());
        const MobilizedBody& body2 = subsystem.getBody(set, contacts[i].getSecondBody());
        const Vec3 station1 = body1.findStationAtGroundPoint(state, location);
        const Vec3 station2 = body2.findStationAtGroundPoint(state, location);
        const Vec3 v1 = body1.findStationVelocityInGround(state, station1);
        const Vec3 v2 = body2.findStationVelocityInGround(state, station2);
        const Vec3 v = v1-v2;
        
        // Calculate the Hunt-Crossley force and apply it to the bodies.
        
        const Real f = fH*(1+1.5*c*dot(v, normal));
        if (f > 0) {
            body1.applyForceToBodyPoint(state, station1, -f*normal, bodyForces);
            body2.applyForceToBodyPoint(state, station2, f*normal, bodyForces);
        }
    }
}

Real HuntCrossleyForceImpl::calcPotentialEnergy(const State& state) const {
    return 0;
}

} // namespace SimTK

