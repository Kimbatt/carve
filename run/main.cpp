
#include "../carve_dll/carve_dll.h"

#include <iostream>

void logMesh(ILeoCSGMesh* mesh)
{
    int numTris = mesh->getTriangleCount();
    int numVerts = mesh->getVertexCount();

    const float* verts = mesh->getVertices();
    const int* tris = mesh->getTriangles();

    int k = 0;
    std::cout << "vertices: " << numVerts << std::endl;
    for (int i = 0; i < numVerts; ++i)
    {
        std::cout << i << ": " << verts[k] << ", " << verts[k + 1] << ", " << verts[k + 2] << std::endl;
        k += 3;
    }
    std::cout << std::endl;

    k = 0;
    std::cout << "triangles: " << numTris << std::endl;
    for (int i = 0; i < numTris; ++i)
    {
        std::cout << i << ": " << tris[k] << ", " << tris[k + 1] << ", " << tris[k + 2] << std::endl;
        k += 3;
    }
    std::cout << std::endl;
}


void testPyramids()
{
    int k1 = sizeof(int);
    int k2 = sizeof(3);

    const float vertsA[] =
    {
        -1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        -1.0f, 0.0f, -1.0f,
        1.0f, 0.0f, -1.0f,
        0.0f, 1.0f, 0.0f
    };

    const float vertsB[] =
    {
        1.0f, 1.5f, 1.0f,
        -1.0f, 1.5f, 1.0f,
        1.0f, 1.5f, -1.0f,
        -1.0f, 1.5f, -1.0f,
        0.0f, 0.5f, 0.0f
    };

    const int tris[] =
    {
        0, 1, 4,
        1, 3, 4,
        3, 2, 4,
        2, 0, 4,
        0, 2, 1,
        1, 2, 3
    };

    ILeoCSGMesh* meshA = leoCreateCSGMesh();
    meshA->setVertices(5, vertsA);
    meshA->setTriangles(6, tris);

    ILeoCSGMesh* meshB = leoCreateCSGMesh();
    meshB->setVertices(5, vertsB);
    meshB->setTriangles(6, tris);

    ILeoCSGMesh* result = leoPerformCSG(meshA, meshB, CSGOp::Union, nullptr, 0);
    if (result != nullptr)
    {
        logMesh(result);
    }
    else
    {
        std::cout << "leoPerformCSG failed\n";
    }
}


void testCubes()
{
    const float vertsA[] =
    {
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f
    };

    const float vertsB[] =
    {
        2.0f, -1.2f, -0.8f,
        2.0f, -1.2f, 0.8f,
        0.0f, -1.2f, 0.8f,
        0.0f, -1.2f, -0.8f,
        2.0f, 1.2f, -0.8f,
        2.0f, 1.2f, 0.8f,
        0.0f, 1.2f, 0.8f,
        0.0f, 1.2f, -0.8f
    };

    const int tris[] =
    {
        0, 1, 2,
        0, 2, 3,
        4, 7, 6,
        4, 6, 5,
        0, 4, 5,
        0, 5, 1,
        1, 5, 6,
        1, 6, 2,
        2, 6, 7,
        2, 7, 3,
        4, 0, 3,
        4, 3, 7
    };

    ILeoCSGMesh* meshA = leoCreateCSGMesh();
    meshA->setVertices(8, vertsA);
    meshA->setTriangles(12, tris);

    ILeoCSGMesh* meshB = leoCreateCSGMesh();
    meshB->setVertices(8, vertsB);
    meshB->setTriangles(12, tris);

    ILeoCSGMesh* result = leoPerformCSG(meshA, meshB, CSGOp::Union, nullptr, 0);
    if (result != nullptr)
    {
        logMesh(result);
    }
    else
    {
        std::cout << "leoPerformCSG failed\n";
    }
}


void testCustom()
{
    const float vertsA[] =
    {
        1.07, -1.5, 1.5,
        -1.93, -1.5, 1.5,
        1.07, 1.5, 1.5,
        -1.93, 1.5, 1.5,
        1.07, 1.5, -1.5,
        -1.93, 1.5, -1.5,
        1.07, -1.5, -1.5,
        -1.93, -1.5, -1.5,
        1.07, 1.5, 1.5,
        -1.93, 1.5, 1.5,
        1.07, 1.5, -1.5,
        -1.93, 1.5, -1.5,
        1.07, -1.5, -1.5,
        1.07, -1.5, 1.5,
        -1.93, -1.5, 1.5,
        -1.93, -1.5, -1.5,
        -1.93, -1.5, 1.5,
        -1.93, 1.5, 1.5,
        -1.93, 1.5, -1.5,
        -1.93, -1.5, -1.5,
        1.07, -1.5, -1.5,
        1.07, 1.5, -1.5,
        1.07, 1.5, 1.5,
        1.07, -1.5, 1.5,
    };

    const float vertsB[] =
    {
        7.28, -2, 2,
        3.28, -2, 2,
        7.28, 2, 2,
        3.28, 2, 2,
        7.28, 2, -2,
        3.28, 2, -2,
        7.28, -2, -2,
        3.28, -2, -2,
        7.28, 2, 2,
        3.28, 2, 2,
        7.28, 2, -2,
        3.28, 2, -2,
        7.28, -2, -2,
        7.28, -2, 2,
        3.28, -2, 2,
        3.28, -2, -2,
        3.28, -2, 2,
        3.28, 2, 2,
        3.28, 2, -2,
        3.28, -2, -2,
        7.28, -2, -2,
        7.28, 2, -2,
        7.28, 2, 2,
        7.28, -2, 2,
    };

    const int tris[] =
    {
        0, 3, 2,
        0, 1, 3,
        8, 5, 4,
        8, 9, 5,
        10, 7, 6,
        10, 11, 7,
        12, 14, 13,
        12, 15, 14,
        16, 18, 17,
        16, 19, 18,
        20, 22, 21,
        20, 23, 22,
    };

    ILeoCSGMesh* meshA = leoCreateCSGMesh();
    meshA->setVertices(24, vertsA);
    meshA->setTriangles(12, tris);

    ILeoCSGMesh* meshB = leoCreateCSGMesh();
    meshB->setVertices(24, vertsB);
    meshB->setTriangles(12, tris);

    ILeoCSGMesh* result = leoPerformCSG(meshA, meshB, CSGOp::Union, nullptr, 0);
    if (result != nullptr)
    {
        logMesh(result);
    }
    else
    {
        std::cout << "leoPerformCSG failed\n";
    }
}

int main()
{
    testCubes();
    testPyramids();
    //testCustom();
    return 0;
}
