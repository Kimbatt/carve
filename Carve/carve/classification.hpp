// Begin License:
// Copyright (C) 2006-2011 Tobias Sargeant (tobias.sargeant@gmail.com).
// All rights reserved.
//
// This file is part of the Carve CSG Library (http://carve-csg.com/)
//
// This file may be used under the terms of the GNU General Public
// License version 2.0 as published by the Free Software Foundation
// and appearing in the file LICENSE.GPL2 included in the packaging of
// this file.
//
// This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
// INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE.
// End:


#pragma once

#include <carve/carve.hpp>
#include <carve/collection_types.hpp>

namespace carve {
  namespace csg {

    enum class FaceClass : int {
      FACE_UNCLASSIFIED = -3,
      FACE_ON_ORIENT_OUT = -2,
      FACE_OUT = -1,
      FACE_ON = 0,
      FACE_IN = +1,
      FACE_ON_ORIENT_IN = +2
    };

    enum class FaceClassBit : unsigned {
      FACE_ON_ORIENT_OUT_BIT = 0x01,
      FACE_OUT_BIT           = 0x02,
      FACE_IN_BIT            = 0x04,
      FACE_ON_ORIENT_IN_BIT  = 0x08,

      FACE_ANY_BIT           = 0x0f,
      FACE_ON_BIT            = 0x09,
      FACE_NOT_ON_BIT        = 0x06
    };

    static inline FaceClass class_bit_to_class(unsigned i) {
      if (i & (unsigned)FaceClassBit::FACE_ON_ORIENT_OUT_BIT) return FaceClass::FACE_ON_ORIENT_OUT;
      if (i & (unsigned)FaceClassBit::FACE_OUT_BIT) return FaceClass::FACE_OUT;
      if (i & (unsigned)FaceClassBit::FACE_IN_BIT) return FaceClass::FACE_IN;
      if (i & (unsigned)FaceClassBit::FACE_ON_ORIENT_IN_BIT) return FaceClass::FACE_ON_ORIENT_IN;
      return FaceClass::FACE_UNCLASSIFIED;
    }

    static inline unsigned class_to_class_bit(FaceClass f) {
      switch (f) {
      case FaceClass::FACE_ON_ORIENT_OUT: return (unsigned)FaceClassBit::FACE_ON_ORIENT_OUT_BIT;
      case FaceClass::FACE_OUT: return (unsigned)FaceClassBit::FACE_OUT_BIT;
      case FaceClass::FACE_ON: return (unsigned)FaceClassBit::FACE_ON_BIT;
      case FaceClass::FACE_IN: return (unsigned)FaceClassBit::FACE_IN_BIT;
      case FaceClass::FACE_ON_ORIENT_IN: return (unsigned)FaceClassBit::FACE_ON_ORIENT_IN_BIT;
      case FaceClass::FACE_UNCLASSIFIED: return (unsigned)FaceClassBit::FACE_ANY_BIT;
      default: return 0;
      }
    }

    enum class EdgeClass : int {
      EDGE_UNK = -2,
      EDGE_OUT = -1,
      EDGE_ON = 0,
      EDGE_IN = 1
    };



    const char *ENUM(FaceClass f);
    const char *ENUM(PointClass p);



    struct ClassificationInfo {
      const carve::mesh::Mesh<3> *intersected_mesh;
      FaceClass classification;

      ClassificationInfo() : intersected_mesh(NULL), classification(FaceClass::FACE_UNCLASSIFIED) { }
      ClassificationInfo(const carve::mesh::Mesh<3> *_intersected_mesh,
                         FaceClass _classification) :
          intersected_mesh(_intersected_mesh),
          classification(_classification) {
      }
      bool intersectedMeshIsClosed() const {
        return intersected_mesh->isClosed();
      }
    };



    struct EC2 {
      EdgeClass cls[2];
      EC2() { cls[0] = cls[1] = EdgeClass::EDGE_UNK; }
      EC2(EdgeClass a, EdgeClass b) { cls[0] = a; cls[1] = b; }
    };

    struct PC2 {
      PointClass cls[2];
      PC2() { cls[0] = cls[1] = PointClass::POINT_UNK; }
      PC2(PointClass a, PointClass b) { cls[0] = a; cls[1] = b; }
    };

    typedef std::unordered_map<std::pair<const carve::mesh::MeshSet<3>::vertex_t *, const carve::mesh::MeshSet<3>::vertex_t *>,
                               EC2> EdgeClassification;

    typedef std::unordered_map<const carve::mesh::Vertex<3> *, PC2> VertexClassification;

  }
}

