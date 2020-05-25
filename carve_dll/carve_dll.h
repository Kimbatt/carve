
#include <iostream>
#include <vector>

enum class CSGOp : int
{
    Union,
    Intersection,
    AMinusB,
    BMinusA,
    SymmetricDifference
};

class ILeoCSGMesh
{
public:
    virtual void setVertices(int vertexCount, const float* vertices) = 0;
    virtual void setTriangles(int triangleCount, const int* triangles) = 0;
    virtual int getVertexCount() const = 0;
    virtual int getTriangleCount() const = 0;
    virtual const float* getVertices() const = 0;
    virtual const int* getTriangles() const = 0;

protected:
    ILeoCSGMesh() {};
    virtual ~ILeoCSGMesh() {};
};


class CSGMesh : public ILeoCSGMesh
{
public:
    CSGMesh();
    virtual ~CSGMesh();

    virtual void setVertices(int vertexCount, const float* vertices) override;
    virtual void setTriangles(int triangleCount, const int* triangles) override;
    virtual int getVertexCount() const override;
    virtual int getTriangleCount() const override;
    virtual const float* getVertices() const override;
    virtual const int* getTriangles() const override;

    void log(std::ostream& stream) const;

private:
    std::vector<float> m_vertices;
    std::vector<int> m_triangles;
};


#define EXPORT __declspec(dllexport)
#define STDCALL __stdcall

extern "C"
{
    EXPORT ILeoCSGMesh* STDCALL leoCreateCSGMesh();
    EXPORT void STDCALL leoDestroyCSGMesh(const ILeoCSGMesh* mesh);
    EXPORT void STDCALL leoCSGMeshSetVertices(ILeoCSGMesh* mesh, int vertexCount, const float* vertices);
    EXPORT void STDCALL leoCSGMeshSetTriangles(ILeoCSGMesh* mesh, int triangleCount, const int* triangles);
    EXPORT int STDCALL leoCSGMeshGetVertexCount(const ILeoCSGMesh* mesh);
    EXPORT int STDCALL leoCSGMeshGetTriangleCount(const ILeoCSGMesh* mesh);
    EXPORT void STDCALL leoCSGMeshGetVertices(const ILeoCSGMesh* mesh, float* dstBuffer);
    EXPORT void STDCALL leoCSGMeshGetTriangles(const ILeoCSGMesh* mesh, int* dstBuffer);
    EXPORT ILeoCSGMesh* STDCALL leoPerformCSG(const ILeoCSGMesh* meshA, const ILeoCSGMesh* meshB, CSGOp op, char* errorMessage, int errorMessageLength = 0);
}
