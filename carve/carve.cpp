
#include "carve.h"

#include <vector>

#include <carve/carve.hpp>
#include <carve/csg.hpp>
#include <carve/input.hpp>
#include <carve/csg_triangulator.hpp>
#include <carve/interpolator.hpp>

CSGMesh::CSGMesh()
{

}

CSGMesh::~CSGMesh()
{
}

void CSGMesh::setVertices(int vertexCount, const float* vertices)
{
    m_vertices.resize((size_t)vertexCount * 3);
    memcpy(m_vertices.data(), vertices, (size_t)vertexCount * 3 * sizeof(float));
}

void CSGMesh::setTriangles(int triangleCount, const int* triangles)
{
    m_triangles.resize((size_t)triangleCount * 3);
    memcpy(m_triangles.data(), triangles, (size_t)triangleCount * 3 * sizeof(int));
}

int CSGMesh::getVertexCount() const
{
    return (int)(m_vertices.size() / 3);
}

int CSGMesh::getTriangleCount() const
{
    return (int)(m_triangles.size() / 3);
}

const float* CSGMesh::getVertices() const
{
    return m_vertices.data();
}

const int* CSGMesh::getTriangles() const
{
    return m_triangles.data();
}


void CSGMesh::log(std::ostream& stream) const
{
    size_t k = 0;
    stream << "vertices: " << m_vertices.size() / 3 << std::endl;
    for (size_t i = 0; i < m_vertices.size() / 3; ++i)
    {
        stream << m_vertices[k] << ", " << m_vertices[k + 1] << ", " << m_vertices[k + 2] << ", " << std::endl;
        k += 3;
    }
    stream << std::endl;

    k = 0;
    stream << "triangles: " << m_triangles.size() / 3 << std::endl;
    for (size_t i = 0; i < m_triangles.size() / 3; ++i)
    {
        stream << m_triangles[k] << ", " << m_triangles[k + 1] << ", " << m_triangles[k + 2] << ", " << std::endl;
        k += 3;
    }
    stream << std::endl;
}

EXPORT CSGMesh* STDCALL leoCreateCSGMesh()
{
    return new CSGMesh();
}

EXPORT void STDCALL leoDestroyCSGMesh(const CSGMesh* mesh)
{
    const CSGMesh* csgMesh = static_cast<const CSGMesh*>(mesh);
    delete csgMesh;
}

EXPORT void STDCALL leoCSGMeshSetVertices(CSGMesh* mesh, int vertexCount, const float* vertices)
{
    mesh->setVertices(vertexCount, vertices);
}

EXPORT void STDCALL leoCSGMeshSetTriangles(CSGMesh* mesh, int triangleCount, const int* triangles)
{
    mesh->setTriangles(triangleCount, triangles);
}

EXPORT int STDCALL leoCSGMeshGetVertexCount(const CSGMesh* mesh)
{
    return mesh->getVertexCount();
}

EXPORT int STDCALL leoCSGMeshGetTriangleCount(const CSGMesh* mesh)
{
    return mesh->getTriangleCount();
}

EXPORT void STDCALL leoCSGMeshGetVertices(const CSGMesh* mesh, float* dstBuffer)
{
    memcpy(dstBuffer, mesh->getVertices(), (size_t)mesh->getVertexCount() * 3 * sizeof(float));
}

EXPORT void STDCALL leoCSGMeshGetTriangles(const CSGMesh* mesh, int* dstBuffer)
{
    memcpy(dstBuffer, mesh->getTriangles(), (size_t)mesh->getTriangleCount() * 3 * sizeof(int));
}

const float* STDCALL leoCSGMeshGetVertexPointer(const CSGMesh* mesh)
{
    return mesh->getVertices();
}

const int* STDCALL leoCSGMeshGetTrianglePointer(const CSGMesh* mesh)
{
    return mesh->getTriangles();
}


typedef carve::poly::Polyhedron Poly;

EXPORT CSGMesh* STDCALL leoPerformCSG(const CSGMesh* meshA, const CSGMesh* meshB, CSGOp op, char* errorMessage, int errorMessageLength)
{
    try
    {
        Poly* models[2];
        carve::csg::CSG csg;

        for (int i = 0; i < 2; ++i)
        {
            const CSGMesh* mesh = i == 0 ? meshA : meshB;
            Poly*& model = models[i];

            std::vector<Poly::vertex_t> vertices;
            int numVerts = mesh->getVertexCount();
            int numComps = numVerts * 3;
            const float* meshVerts = mesh->getVertices();

            vertices.reserve(numVerts);

            for (int vi = 0; vi < numComps; vi += 3)
            {
                float x = meshVerts[vi];
                float y = meshVerts[vi + 1];
                float z = meshVerts[vi + 2];
                vertices.push_back(Poly::vertex_t(carve::geom::VECTOR(x, y, z)));
            }

            std::vector<Poly::face_t> faces;
            int numTris = mesh->getTriangleCount();
            numComps = numTris * 3;
            const int* meshTris = mesh->getTriangles();

            faces.reserve(numTris);

            for (int ti = 0; ti < numComps; ti += 3)
            {
                int tri1 = meshTris[ti];
                int tri2 = meshTris[ti + 1];
                int tri3 = meshTris[ti + 2];

                faces.push_back(Poly::face_t(
                    &vertices[tri1],
                    &vertices[tri2],
                    &vertices[tri3]));
            }

            model = new Poly(faces);
        }

        auto* triangulator = new carve::csg::CarveTriangulatorWithImprovement();
        csg.hooks.registerHook(triangulator, carve::csg::CSG::Hooks::PROCESS_OUTPUT_FACE_BIT);

        carve::csg::CSG::meshset_t* res = nullptr;
        carve::csg::CSG::meshset_t* meshA = carve::meshFromPolyhedron(models[0], -1);
        carve::csg::CSG::meshset_t* meshB = carve::meshFromPolyhedron(models[1], -1);

        switch (op)
        {
        case CSGOp::Union:
            res = csg.compute(meshA, meshB, carve::csg::CSG::CSG_OP::UNION);
            break;
        case CSGOp::Intersection:
            res = csg.compute(meshA, meshB, carve::csg::CSG::CSG_OP::INTERSECTION);
            break;
        case CSGOp::AMinusB:
            res = csg.compute(meshA, meshB, carve::csg::CSG::CSG_OP::A_MINUS_B);
            break;
        case CSGOp::BMinusA:
            res = csg.compute(meshA, meshB, carve::csg::CSG::CSG_OP::B_MINUS_A);
            break;
        case CSGOp::SymmetricDifference:
            res = csg.compute(meshA, meshB, carve::csg::CSG::CSG_OP::SYMMETRIC_DIFFERENCE);
            break;
        }

        delete triangulator;
        delete meshA;
        delete meshB;
        delete models[0];
        delete models[1];

        if (res == nullptr)
        {
            if (errorMessage != nullptr)
            {
                strncpy(errorMessage, "Cannot perform CSG operation", errorMessageLength);
            }
            return nullptr;
        }

        std::vector<float> verts;
        std::vector<int> tris;

        Poly* resultMesh = carve::polyhedronFromMesh(res, -1);
        verts.reserve(resultMesh->faces.size() * 9);
        tris.reserve(resultMesh->faces.size() * 3);

        int ti = 0;
        for (size_t i = 0; i < resultMesh->faces.size(); ++i)
        {
            const Poly::face_t* face = &(resultMesh->faces[i]);

            size_t n = face->nVertices();
            assert(n == 3);

            for (int j = 0; j < 3; ++j)
            {
                const Poly::vertex_t* vertex = face->vertex(j);

                verts.push_back((float)vertex->v.x);
                verts.push_back((float)vertex->v.y);
                verts.push_back((float)vertex->v.z);
                tris.push_back(ti);
                ++ti;
            }
        }

        delete res;
        delete resultMesh;

        CSGMesh* mesh = new CSGMesh();
        mesh->setVertices((int)(verts.size() / 3), verts.data());
        mesh->setTriangles((int)(tris.size() / 3), tris.data());
        return mesh;
    }
    catch (carve::exception& ex)
    {
        if (errorMessage != nullptr)
        {
            strncpy(errorMessage, ex.str().c_str(), errorMessageLength);
        }

        return nullptr;
    }
}
