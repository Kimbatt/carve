
#ifndef CARVE_DLL_CUSTOM_COLLECTOR_H
#define CARVE_DLL_CUSTOM_COLLECTOR_H

#include <include/csg.hpp>

// This file is copied from csg_collector.cpp 
// It contains all collectors from that file, but without constructing the final meshset
// It can save a lot of time if we only care about the vertices and the triangles of the result mesh

namespace
{
    class BaseCollectorWithoutResultMeshset : public carve::csg::CSG::Collector
    {
        BaseCollectorWithoutResultMeshset();
        BaseCollectorWithoutResultMeshset(const BaseCollectorWithoutResultMeshset&);

    protected:
        struct face_data_t
        {
            carve::mesh::MeshSet<3>::face_t* face;
            const carve::mesh::MeshSet<3>::face_t* orig_face;
            bool flipped;
            face_data_t(carve::mesh::MeshSet<3>::face_t* _face, const carve::mesh::MeshSet<3>::face_t* _orig_face, bool _flipped)
                : face(_face), orig_face(_orig_face), flipped(_flipped) {};

            face_data_t() = default;
        };

        std::vector<carve::small_vector_on_stack<face_data_t, 3>> tmpFaces;
        std::vector<face_data_t> faces;

        const carve::mesh::MeshSet<3>* src_a;
        const carve::mesh::MeshSet<3>* src_b;

        BaseCollectorWithoutResultMeshset(const carve::mesh::MeshSet<3>* _src_a, const carve::mesh::MeshSet<3>* _src_b) : carve::csg::CSG::Collector(), src_a(_src_a), src_b(_src_b)
        {
        }

        virtual ~BaseCollectorWithoutResultMeshset()
        {
            for (face_data_t& data : faces)
            {
                delete data.face;
            }
        }

        void processFace(const carve::mesh::MeshSet<3>::face_t* orig_face, const std::vector<carve::mesh::MeshSet<3>::vertex_t*>& vertices,
                carve::geom3d::Vector /* normal */, bool /* poly_a */, carve::csg::FaceClass face_class, carve::csg::CSG::Hooks& hooks, bool reversed, size_t index)
        {
            carve::small_vector_on_stack<carve::mesh::MeshSet<3>::face_t*, 16> new_faces;
            new_faces.push_back(orig_face->create(vertices.begin(), vertices.end(), reversed));
            hooks.processOutputFace(new_faces, orig_face, reversed);

            carve::small_vector_on_stack<face_data_t, 3>& resultFaces = tmpFaces[index];
            size_t count = new_faces.size();
            resultFaces.resize(count);
            for (size_t i = 0; i < count; ++i)
            {
                resultFaces[i] = face_data_t(new_faces[i], orig_face, reversed);
            }
        }

        virtual void collect(const carve::mesh::MeshSet<3>::face_t* orig_face, const std::vector<carve::mesh::MeshSet<3>::vertex_t*>& vertices,
            carve::geom3d::Vector normal, bool poly_a, carve::csg::FaceClass face_class, carve::csg::CSG::Hooks& hooks, size_t index) = 0;

        virtual void collect(carve::csg::FaceLoopGroup* grp, carve::csg::CSG::Hooks& hooks) override
        {
            std::list<carve::csg::ClassificationInfo>& cinfo = (grp->classification);

            if (cinfo.size() == 0)
            {
                std::cerr << "WARNING! group " << grp << " has no classification info!" << std::endl;
                return;
            }

            carve::csg::FaceClass fc = carve::csg::FaceClass::FACE_UNCLASSIFIED;

            unsigned fc_closed_bits = 0;
            unsigned fc_open_bits = 0;
            unsigned fc_bits = 0;

            for (std::list<carve::csg::ClassificationInfo>::const_iterator i = grp->classification.begin(), e = grp->classification.end(); i != e; ++i)
            {
                if ((*i).intersected_mesh == NULL)
                {
                    // classifier only returns global info
                    fc_closed_bits = class_to_class_bit((*i).classification);
                    break;
                }

                if ((*i).classification == carve::csg::FaceClass::FACE_UNCLASSIFIED)
                    continue;
                if ((*i).intersectedMeshIsClosed())
                {
                    fc_closed_bits |= class_to_class_bit((*i).classification);
                }
                else
                {
                    fc_open_bits |= class_to_class_bit((*i).classification);
                }
            }

            if (fc_closed_bits)
            {
                fc_bits = fc_closed_bits;
            }
            else
            {
                fc_bits = fc_open_bits;
            }

            fc = carve::csg::class_bit_to_class(fc_bits);

            // handle the complex cases where a group is classified differently with respect to two or more closed manifolds.
            if (fc == carve::csg::FaceClass::FACE_UNCLASSIFIED)
            {
                unsigned inout_bits = fc_bits & (unsigned)carve::csg::FaceClassBit::FACE_NOT_ON_BIT;
                unsigned on_bits = fc_bits & (unsigned)carve::csg::FaceClassBit::FACE_ON_BIT;

                // both in and out. indicates an invalid manifold embedding.
                if (inout_bits == ((unsigned)carve::csg::FaceClassBit::FACE_IN_BIT | (unsigned)carve::csg::FaceClassBit::FACE_OUT_BIT))
                    goto out;

                // on, both orientations. could be caused by two manifolds touching at a face.
                if (on_bits == ((unsigned)carve::csg::FaceClassBit::FACE_ON_ORIENT_IN_BIT | (unsigned)carve::csg::FaceClassBit::FACE_ON_ORIENT_OUT_BIT))
                    goto out;

                // in or out, but also on (with orientation). the on classification takes precedence.
                fc = carve::csg::class_bit_to_class(on_bits);
            }

        out:

            if (fc == carve::csg::FaceClass::FACE_UNCLASSIFIED)
            {
                std::cerr << "group " << grp << " is unclassified!" << std::endl;
                return;
            }

            bool is_poly_a = grp->src == src_a;

            auto process = [this, is_poly_a, fc, &hooks](const auto& data)
            {
                const carve::csg::FaceLoop* f = data.first;
                size_t idx = data.second;
                collect(f->orig_face, f->vertices, f->orig_face->plane.N, is_poly_a, fc, hooks, idx);
            };

            tmpFaces.resize(grp->face_loops.count);
            faces.reserve(std::max(faces.capacity(), grp->face_loops.count * 2 + faces.size()));

            carve::util::forEachParallel(grp->face_loops, process);

            for (auto& it : tmpFaces)
            {
                for (auto& f : it)
                {
                    faces.push_back(f);
                }
            }

            tmpFaces.clear();
        }

        virtual carve::mesh::MeshSet<3>* done(carve::csg::CSG::Hooks& hooks) override
        {
            if (hooks.hasHook(carve::csg::CSG::Hooks::RESULT_FACE_HOOK))
            {
                hooks.resultNumFaces(faces.size());
                for (std::vector<face_data_t>::iterator i = faces.begin(); i != faces.end(); ++i)
                {
                    hooks.resultFace((*i).face, (*i).orig_face, (*i).flipped);
                }
            }

            return NULL;
        }
    };


    class UnionCollectorWithoutResultMeshset : public BaseCollectorWithoutResultMeshset
    {
    public:
        UnionCollectorWithoutResultMeshset(const carve::mesh::MeshSet<3>* _src_a, const carve::mesh::MeshSet<3>* _src_b) : BaseCollectorWithoutResultMeshset(_src_a, _src_b)
        {
        }
        virtual ~UnionCollectorWithoutResultMeshset()
        {
        }
        virtual void collect(const carve::mesh::MeshSet<3>::face_t* orig_face, const std::vector<carve::mesh::MeshSet<3>::vertex_t*>& vertices,
            carve::geom3d::Vector normal, bool poly_a, carve::csg::FaceClass face_class, carve::csg::CSG::Hooks& hooks, size_t index) override
        {
            if (face_class == carve::csg::FaceClass::FACE_OUT || (poly_a && face_class == carve::csg::FaceClass::FACE_ON_ORIENT_OUT))
            {
                processFace(orig_face, vertices, normal, poly_a, face_class, hooks, false, index);
            }
        }
    };


    class IntersectionCollectorWithoutResultMeshset : public BaseCollectorWithoutResultMeshset
    {
    public:
        IntersectionCollectorWithoutResultMeshset(const carve::mesh::MeshSet<3>* _src_a, const carve::mesh::MeshSet<3>* _src_b) : BaseCollectorWithoutResultMeshset(_src_a, _src_b)
        {
        }
        virtual ~IntersectionCollectorWithoutResultMeshset()
        {
        }
        virtual void collect(const carve::mesh::MeshSet<3>::face_t* orig_face, const std::vector<carve::mesh::MeshSet<3>::vertex_t*>& vertices,
            carve::geom3d::Vector normal, bool poly_a, carve::csg::FaceClass face_class, carve::csg::CSG::Hooks& hooks, size_t idx) override
        {
            if (face_class == carve::csg::FaceClass::FACE_IN || (poly_a && face_class == carve::csg::FaceClass::FACE_ON_ORIENT_OUT))
            {
                processFace(orig_face, vertices, normal, poly_a, face_class, hooks, false, idx);
            }
        }
    };


    class SymmetricDifferenceCollectorWithoutResultMeshset : public BaseCollectorWithoutResultMeshset
    {
    public:
        SymmetricDifferenceCollectorWithoutResultMeshset(const carve::mesh::MeshSet<3>* _src_a, const carve::mesh::MeshSet<3>* _src_b) : BaseCollectorWithoutResultMeshset(_src_a, _src_b)
        {
        }
        virtual ~SymmetricDifferenceCollectorWithoutResultMeshset()
        {
        }
        virtual void collect(const carve::mesh::MeshSet<3>::face_t* orig_face, const std::vector<carve::mesh::MeshSet<3>::vertex_t*>& vertices,
            carve::geom3d::Vector normal, bool poly_a, carve::csg::FaceClass face_class, carve::csg::CSG::Hooks& hooks, size_t idx) override
        {
            if (face_class == carve::csg::FaceClass::FACE_OUT)
            {
                processFace(orig_face, vertices, normal, poly_a, face_class, hooks, false, idx);
            }
            else if (face_class == carve::csg::FaceClass::FACE_IN)
            {
                processFace(orig_face, vertices, normal, poly_a, face_class, hooks, true, idx);
            }
        }
    };


    class AMinusBCollectorWithoutResultMeshset : public BaseCollectorWithoutResultMeshset
    {
    public:
        AMinusBCollectorWithoutResultMeshset(const carve::mesh::MeshSet<3>* _src_a, const carve::mesh::MeshSet<3>* _src_b) : BaseCollectorWithoutResultMeshset(_src_a, _src_b)
        {
        }
        virtual ~AMinusBCollectorWithoutResultMeshset()
        {
        }
        virtual void collect(const carve::mesh::MeshSet<3>::face_t* orig_face, const std::vector<carve::mesh::MeshSet<3>::vertex_t*>& vertices,
            carve::geom3d::Vector normal, bool poly_a, carve::csg::FaceClass face_class, carve::csg::CSG::Hooks& hooks, size_t idx) override
        {
            if ((face_class == carve::csg::FaceClass::FACE_OUT || face_class == carve::csg::FaceClass::FACE_ON_ORIENT_IN) && poly_a)
            {
                processFace(orig_face, vertices, normal, poly_a, face_class, hooks, false, idx);
            }
            else if (face_class == carve::csg::FaceClass::FACE_IN && !poly_a)
            {
                processFace(orig_face, vertices, normal, poly_a, face_class, hooks, true, idx);
            }
        }
    };


    class BMinusACollectorWithoutResultMeshset : public BaseCollectorWithoutResultMeshset
    {
    public:
        BMinusACollectorWithoutResultMeshset(const carve::mesh::MeshSet<3>* _src_a, const carve::mesh::MeshSet<3>* _src_b) : BaseCollectorWithoutResultMeshset(_src_a, _src_b)
        {
        }
        virtual ~BMinusACollectorWithoutResultMeshset()
        {
        }
        virtual void collect(const carve::mesh::MeshSet<3>::face_t* orig_face, const std::vector<carve::mesh::MeshSet<3>::vertex_t*>& vertices,
            carve::geom3d::Vector normal, bool poly_a, carve::csg::FaceClass face_class, carve::csg::CSG::Hooks& hooks, size_t idx) override
        {
            if ((face_class == carve::csg::FaceClass::FACE_OUT || face_class == carve::csg::FaceClass::FACE_ON_ORIENT_IN) && !poly_a)
            {
                processFace(orig_face, vertices, normal, poly_a, face_class, hooks, false, idx);
            }
            else if (face_class == carve::csg::FaceClass::FACE_IN && poly_a)
            {
                processFace(orig_face, vertices, normal, poly_a, face_class, hooks, true, idx);
            }
        }
    };
} 

carve::csg::CSG::Collector* makeCollectorWithoutResultMeshset(carve::csg::CSG::CSG_OP op, const carve::mesh::MeshSet<3>* poly_a, const carve::mesh::MeshSet<3>* poly_b)
{
    switch (op)
    {
    case carve::csg::CSG::CSG_OP::UNION:
        return new UnionCollectorWithoutResultMeshset(poly_a, poly_b);
    case carve::csg::CSG::CSG_OP::INTERSECTION:
        return new IntersectionCollectorWithoutResultMeshset(poly_a, poly_b);
    case carve::csg::CSG::CSG_OP::A_MINUS_B:
        return new AMinusBCollectorWithoutResultMeshset(poly_a, poly_b);
    case carve::csg::CSG::CSG_OP::B_MINUS_A:
        return new BMinusACollectorWithoutResultMeshset(poly_a, poly_b);
    case carve::csg::CSG::CSG_OP::SYMMETRIC_DIFFERENCE:
        return new SymmetricDifferenceCollectorWithoutResultMeshset(poly_a, poly_b);
    }
    return NULL;
}

#endif
