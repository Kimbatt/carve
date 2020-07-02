
#ifndef CARVE_DLL_H
#define CARVE_DLL_H

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

class CSGMesh
{
public:
    CSGMesh();
    ~CSGMesh();

    void stealVertices(std::vector<float>& vertices);
    void stealTriangles(std::vector<int>& triangles);
    void setVertices(int vertexCount, const float* vertices);
    void setTriangles(int triangleCount, const int* triangles);
    int getVertexCount() const;
    int getTriangleCount() const;
    const float* getVertices() const;
    const int* getTriangles() const;

    void log(std::ostream& stream) const;

private:
    std::vector<float> m_vertices;
    std::vector<int> m_triangles;
};

#if __EMSCRIPTEN__
#define EXPORT
#define STDCALL
#else
#define EXPORT __declspec(dllexport)
#define STDCALL __stdcall
#endif

extern "C"
{
    EXPORT CSGMesh* STDCALL leoCreateCSGMesh();
    EXPORT void STDCALL leoDestroyCSGMesh(const CSGMesh* mesh);
    EXPORT void STDCALL leoCSGMeshSetVertices(CSGMesh* mesh, int vertexCount, const float* vertices);
    EXPORT void STDCALL leoCSGMeshSetTriangles(CSGMesh* mesh, int triangleCount, const int* triangles);
    EXPORT int STDCALL leoCSGMeshGetVertexCount(const CSGMesh* mesh);
    EXPORT int STDCALL leoCSGMeshGetTriangleCount(const CSGMesh* mesh);
    EXPORT void STDCALL leoCSGMeshGetVertices(const CSGMesh* mesh, float* dstBuffer);
    EXPORT void STDCALL leoCSGMeshGetTriangles(const CSGMesh* mesh, int* dstBuffer);
    EXPORT const float* STDCALL leoCSGMeshGetVertexPointer(const CSGMesh* mesh);
    EXPORT const int* STDCALL leoCSGMeshGetTrianglePointer(const CSGMesh* mesh);
    EXPORT CSGMesh* STDCALL leoPerformCSG(const CSGMesh* meshA, const CSGMesh* meshB, CSGOp op, char* errorMessage, int errorMessageLength = 0);
}

#endif
