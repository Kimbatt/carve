
#include "carve_dll.h"

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
    m_vertices.resize(vertexCount * 3);
    memcpy(m_vertices.data(), vertices, vertexCount * 3 * sizeof(float));
}

void CSGMesh::setTriangles(int triangleCount, const int* triangles)
{
    m_triangles.resize(triangleCount * 3);
    memcpy(m_triangles.data(), triangles, triangleCount * 3 * sizeof(int));
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
    int k = 0;
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

EXPORT ILeoCSGMesh* STDCALL leoCreateCSGMesh()
{
    return new CSGMesh();
}

EXPORT void STDCALL leoDestroyCSGMesh(const ILeoCSGMesh* mesh)
{
    const CSGMesh* csgMesh = static_cast<const CSGMesh*>(mesh);
    delete csgMesh;
}

EXPORT void STDCALL leoCSGMeshSetVertices(ILeoCSGMesh* mesh, int vertexCount, const float* vertices)
{
    mesh->setVertices(vertexCount, vertices);
}

EXPORT void STDCALL leoCSGMeshSetTriangles(ILeoCSGMesh* mesh, int triangleCount, const int* triangles)
{
    mesh->setTriangles(triangleCount, triangles);
}

EXPORT int STDCALL leoCSGMeshGetVertexCount(const ILeoCSGMesh* mesh)
{
    return mesh->getVertexCount();
}

EXPORT int STDCALL leoCSGMeshGetTriangleCount(const ILeoCSGMesh* mesh)
{
    return mesh->getTriangleCount();
}

EXPORT void STDCALL leoCSGMeshGetVertices(const ILeoCSGMesh* mesh, float* dstBuffer)
{
    memcpy(dstBuffer, mesh->getVertices(), mesh->getVertexCount() * 3 * sizeof(float));
}

EXPORT void STDCALL leoCSGMeshGetTriangles(const ILeoCSGMesh* mesh, int* dstBuffer)
{
    memcpy(dstBuffer, mesh->getTriangles(), mesh->getTriangleCount() * 3 * sizeof(int));
}


typedef carve::poly::Polyhedron Poly;

EXPORT ILeoCSGMesh* STDCALL leoPerformCSG(const ILeoCSGMesh* meshA, const ILeoCSGMesh* meshB, CSGOp op, char* errorMessage, int errorMessageLength)
{
    try
    {
        Poly* models[2];
        carve::csg::CSG csg;

        for (int i = 0; i < 2; ++i)
        {
            const ILeoCSGMesh* mesh = i == 0 ? meshA : meshB;
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

        csg.hooks.registerHook(new carve::csg::CarveTriangulatorWithImprovement(), carve::csg::CSG::Hooks::PROCESS_OUTPUT_FACE_BIT);

        carve::csg::CSG::meshset_t* res = nullptr;
        carve::csg::CSG::meshset_t* meshA = carve::meshFromPolyhedron(models[0], -1);
        carve::csg::CSG::meshset_t* meshB = carve::meshFromPolyhedron(models[1], -1);

        switch (op)
        {
        case CSGOp::Union:
            res = csg.compute(meshA, meshB, carve::csg::CSG::UNION);
            break;
        case CSGOp::Intersection:
            res = csg.compute(meshA, meshB, carve::csg::CSG::INTERSECTION);
            break;
        case CSGOp::AMinusB:
            res = csg.compute(meshA, meshB, carve::csg::CSG::A_MINUS_B);
            break;
        case CSGOp::BMinusA:
            res = csg.compute(meshA, meshB, carve::csg::CSG::B_MINUS_A);
            break;
        case CSGOp::SymmetricDifference:
            res = csg.compute(meshA, meshB, carve::csg::CSG::SYMMETRIC_DIFFERENCE);
            break;
        }

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
