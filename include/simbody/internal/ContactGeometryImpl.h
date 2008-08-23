#ifndef SimTK_SIMBODY_CONTACT_GEOMETRY_IMPL_H_
#define SimTK_SIMBODY_CONTACT_GEOMETRY_IMPL_H_

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


#include "simbody/internal/ContactGeometry.h"

namespace SimTK {

class SimTK_SIMBODY_EXPORT ContactGeometryImpl {
public:
    ContactGeometryImpl(const std::string& type);
    virtual ~ContactGeometryImpl() {
        clearMyHandle();
    }
    const std::string& getType() const {
        return type;
    }
    int getTypeIndex() const {
        return typeIndex;
    }
    static int getIndexForType(std::string type);
    virtual ContactGeometryImpl* clone() const = 0;
    ContactGeometry* getMyHandle() {
        return myHandle;
    }
    void setMyHandle(ContactGeometry& h) {
        myHandle = &h;
    }
    void clearMyHandle() {
        myHandle = 0;
    }
protected:
    ContactGeometry* myHandle;
    const std::string& type;
    int typeIndex;
};

class ContactGeometry::HalfSpaceImpl : public ContactGeometryImpl {
public:
    HalfSpaceImpl() : ContactGeometryImpl(Type()) {
    }
    ContactGeometryImpl* clone() const {
        return new HalfSpaceImpl();
    }
    static const std::string Type() {
        static std::string type = "halfspace";
        return type;
    }
};

class ContactGeometry::SphereImpl : public ContactGeometryImpl {
public:
    SphereImpl(Real radius) : ContactGeometryImpl(Type()), radius(radius) {
    }
    ContactGeometryImpl* clone() const {
        return new SphereImpl(radius);
    }
    Real getRadius() const {
        return radius;
    }
    void setRadius(Real r) {
        radius = r;
    }
    static const std::string Type() {
        static std::string type = "sphere";
        return type;
    }
private:
    Real radius;
};

} // namespace SimTK

#endif // SimTK_SIMBODY_CONTACT_GEOMETRY_IMPL_H_
