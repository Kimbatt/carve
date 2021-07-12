
#include "carve.h"
#include "custom_collector.h"

#include <vector>

#include <include/carve.hpp>
#include <include/csg.hpp>
#include <include/csg_triangulator.hpp>
#include <include/input.hpp>
#include <include/interpolator.hpp>

CSGMesh::CSGMesh()
{
}

CSGMesh::~CSGMesh()
{
}

void CSGMesh::stealVertices(std::vector<float>& vertices)
{
    m_vertices.swap(vertices);
}

void CSGMesh::stealTriangles(std::vector<int>& triangles)
{
    m_triangles.swap(triangles);
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

class FaceCollector : public carve::csg::CSG::Hook
{
public:
    FaceCollector() : carve::csg::CSG::Hook()
    {
        _vertexCount = 0;
    }

    std::vector<float>& getVertices()
    {
        return _uniqueVertices;
    }

    std::vector<int>& getTriangles()
    {
        return _triangles;
    }

protected:
    virtual void resultFace(const carve::csg::CSG::meshset_t::face_t* newFace, const carve::csg::CSG::meshset_t::face_t* originalFace, bool flipped) override
    {
        using Edge = carve::mesh::Edge<3>;

        const Edge* startEdge = newFace->edge;
        int startVertexIndex = tryAddVertex(startEdge->vert);

        const Edge* currentEdge = startEdge->next;
        int currentVertexIndex = tryAddVertex(currentEdge->vert);

        while (currentEdge->next != startEdge)
        {
            const Edge* nextEdge = currentEdge->next;
            int nextVertexIndex = tryAddVertex(nextEdge->vert);

            if (flipped)
            {
                _triangles.push_back(startVertexIndex);
                _triangles.push_back(nextVertexIndex);
                _triangles.push_back(currentVertexIndex);
            }
            else
            {
                _triangles.push_back(startVertexIndex);
                _triangles.push_back(currentVertexIndex);
                _triangles.push_back(nextVertexIndex);
            }

            currentEdge = nextEdge;
            currentVertexIndex = nextVertexIndex;
        }
    }

    virtual void resultNumFaces(size_t numFaces) override
    {
        _triangles.reserve(numFaces * 3);
        _uniqueVertices.reserve(numFaces * 3); // guess
        _vertexIndexMap.reserve(numFaces); // guess
    }

private:
    int tryAddVertex(carve::mesh::Vertex<3>* vertex)
    {
        auto vertexIndexIt = _vertexIndexMap.insert({ vertex, _vertexCount });
        if (vertexIndexIt.second)
        {
            ++_vertexCount;
            _uniqueVertices.push_back((float)vertex->v.x);
            _uniqueVertices.push_back((float)vertex->v.y);
            _uniqueVertices.push_back((float)vertex->v.z);
        }

        return vertexIndexIt.first->second;
    }

private:
    int _vertexCount;
    robin_hood::unordered_flat_map<carve::mesh::Vertex<3>*, int> _vertexIndexMap;
    std::vector<float> _uniqueVertices;
    std::vector<int> _triangles;
};

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

EXPORT const float* STDCALL leoCSGMeshGetVertexPointer(const CSGMesh* mesh)
{
    return mesh->getVertices();
}

EXPORT const int* STDCALL leoCSGMeshGetTrianglePointer(const CSGMesh* mesh)
{
    return mesh->getTriangles();
}


EXPORT CSGMesh* STDCALL leoPerformCSG(const CSGMesh* meshA, const CSGMesh* meshB, CSGOp op, char* errorMessage, int errorMessageLength)
{
    auto emptyMesh = [op](const CSGMesh* mesh, bool isA) {
        CSGMesh* newMesh = new CSGMesh();

        switch (op)
        {
        case CSGOp::Union:
        case CSGOp::SymmetricDifference:
            // just return the non-empty mesh
            newMesh->setVertices(mesh->getVertexCount(), mesh->getVertices());
            newMesh->setTriangles(mesh->getTriangleCount(), mesh->getTriangles());
            break;
        case CSGOp::Intersection:
        default:
            // no overlap, return an empty mesh
            break;
        case CSGOp::AMinusB:
            // return the non-empty mesh if we are subtracting an empty mesh from it
            if (isA)
            {
                newMesh->setVertices(mesh->getVertexCount(), mesh->getVertices());
                newMesh->setTriangles(mesh->getTriangleCount(), mesh->getTriangles());
            }
            break;
        case CSGOp::BMinusA:
            if (!isA)
            {
                newMesh->setVertices(mesh->getVertexCount(), mesh->getVertices());
                newMesh->setTriangles(mesh->getTriangleCount(), mesh->getTriangles());
            }
            break;
        }

        return newMesh;
    };

    if (meshA->getTriangleCount() == 0)
    {
        return emptyMesh(meshB, false);
    }
    else if (meshB->getTriangleCount() == 0)
    {
        return emptyMesh(meshA, true);
    }

    auto setErrorMessage = [=](const char* errorMsg) {
        if (errorMessage != nullptr)
        {
#if _WIN32
            strncpy_s(errorMessage, strlen(errorMsg) + 1, errorMsg, errorMessageLength);
#else
            strncpy(errorMessage, errorMsg, errorMessageLength);
#endif
        }
    };

    try
    {
        using Meshset = carve::mesh::MeshSet<3>;
        using Vertex_t = Meshset::vertex_t;
        using Face_t = Meshset::face_t;

        Meshset* models[2] = { nullptr, nullptr };
        carve::csg::CSG csg;

        try
        {
            for (int i = 0; i < 2; ++i)
            {
                const CSGMesh* mesh = i == 0 ? meshA : meshB;

                carve::input::PolyhedronData data;
                data.reserveVertices(mesh->getVertexCount());
                data.reserveFaces(mesh->getTriangleCount(), 3);

                int numVertexValues = mesh->getVertexCount() * 3;
                const float* meshVerts = mesh->getVertices();
                for (int vi = 0; vi < numVertexValues; vi += 3)
                {
                    float x = meshVerts[vi];
                    float y = meshVerts[vi + 1];
                    float z = meshVerts[vi + 2];
                    data.addVertex(carve::geom::VECTOR(x, y, z));
                }

                int numIndices = mesh->getTriangleCount() * 3;
                const int* meshTris = mesh->getTriangles();
                for (int ti = 0; ti < numIndices; ti += 3)
                {
                    int tri1 = meshTris[ti];
                    int tri2 = meshTris[ti + 1];
                    int tri3 = meshTris[ti + 2];

                    data.addFace(tri1, tri2, tri3);
                }

                models[i] = data.createMesh(carve::input::Options());
            }
        }
        catch (...)
        {
            delete models[0];
            delete models[1];
            setErrorMessage("Cannot construct polyhedron");
            return nullptr;
        }

        csg.hooks.registerHook(new carve::csg::CarveTriangulatorWithImprovement(), carve::csg::CSG::Hooks::PROCESS_OUTPUT_FACE_BIT);

        FaceCollector* faceCollector = new FaceCollector(); // deleted in csg's destructor
        csg.hooks.registerHook(faceCollector, carve::csg::CSG::Hooks::RESULT_FACE_BIT);

        carve::csg::CSG::meshset_t* meshA = models[0];
        carve::csg::CSG::meshset_t* meshB = models[1];

        carve::csg::CSG::Collector* csgCollector = nullptr;
        switch (op)
        {
        case CSGOp::Union:
            csgCollector = new UnionCollectorWithoutResultMeshset(meshA, meshB);
            break;
        case CSGOp::Intersection:
            csgCollector = new IntersectionCollectorWithoutResultMeshset(meshA, meshB);
            break;
        case CSGOp::AMinusB:
            csgCollector = new AMinusBCollectorWithoutResultMeshset(meshA, meshB);
            break;
        case CSGOp::BMinusA:
            csgCollector = new BMinusACollectorWithoutResultMeshset(meshA, meshB);
            break;
        case CSGOp::SymmetricDifference:
            csgCollector = new SymmetricDifferenceCollectorWithoutResultMeshset(meshA, meshB);
            break;
        default:
            // unknown op
            return nullptr;
        }

        csg.compute(meshA, meshB, *csgCollector);

        delete csgCollector;

        delete meshA;
        delete meshB;

        CSGMesh* mesh = new CSGMesh();
        mesh->stealVertices(faceCollector->getVertices());
        mesh->stealTriangles(faceCollector->getTriangles());
        return mesh;
    }
    catch (carve::exception& ex)
    {
        setErrorMessage(ex.str().c_str());
        return nullptr;
    }
}
