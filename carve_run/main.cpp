
#include "../carve/carve.h"
#include "3DFileReader.h"

#include <iostream>
#include <chrono>

void logMesh(CSGMesh* mesh)
{
    
    int numTris = leoCSGMeshGetTriangleCount(mesh);
    int numVerts = leoCSGMeshGetVertexCount(mesh);

    const float* verts = leoCSGMeshGetVertexPointer(mesh);
    const int* tris = leoCSGMeshGetTrianglePointer(mesh);

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

    CSGMesh* meshA = leoCreateCSGMesh();
    leoCSGMeshSetVertices(meshA, 5, vertsA);
    leoCSGMeshSetTriangles(meshA, 6, tris);

    CSGMesh* meshB = leoCreateCSGMesh();
    leoCSGMeshSetVertices(meshB, 5, vertsB);
    leoCSGMeshSetTriangles(meshB, 6, tris);

    CSGMesh* result = leoPerformCSG(meshA, meshB, CSGOp::Union, nullptr, 0);
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

    CSGMesh* meshA = leoCreateCSGMesh();
    leoCSGMeshSetVertices(meshA, 8, vertsA);
    leoCSGMeshSetTriangles(meshA, 12, tris);

    CSGMesh* meshB = leoCreateCSGMesh();
    leoCSGMeshSetVertices(meshB, 8, vertsB);
    leoCSGMeshSetTriangles(meshB, 12, tris);

    CSGMesh* result = leoPerformCSG(meshA, meshB, CSGOp::Union, nullptr, 0);
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
        1.07f, -1.5f, 1.5f,
        -1.93f, -1.5f, 1.5f,
        1.07f, 1.5f, 1.5f,
        -1.93f, 1.5f, 1.5f,
        1.07f, 1.5f, -1.5f,
        -1.93f, 1.5f, -1.5f,
        1.07f, -1.5f, -1.5f,
        -1.93f, -1.5f, -1.5f,
        1.07f, 1.5f, 1.5f,
        -1.93f, 1.5f, 1.5f,
        1.07f, 1.5f, -1.5f,
        -1.93f, 1.5f, -1.5f,
        1.07f, -1.5f, -1.5f,
        1.07f, -1.5f, 1.5f,
        -1.93f, -1.5f, 1.5f,
        -1.93f, -1.5f, -1.5f,
        -1.93f, -1.5f, 1.5f,
        -1.93f, 1.5f, 1.5f,
        -1.93f, 1.5f, -1.5f,
        -1.93f, -1.5f, -1.5f,
        1.07f, -1.5f, -1.5f,
        1.07f, 1.5f, -1.5f,
        1.07f, 1.5f, 1.5f,
        1.07f, -1.5f, 1.5f,
    };

    const float vertsB[] =
    {
        7.28f, -2.0f, 2.0f,
        3.28f, -2.0f, 2.0f,
        7.28f, 2.0f, 2.0f,
        3.28f, 2.0f, 2.0f,
        7.28f, 2.0f, -2.0f,
        3.28f, 2.0f, -2.0f,
        7.28f, -2.0f, -2.0f,
        3.28f, -2.0f, -2.0f,
        7.28f, 2.0f, 2.0f,
        3.28f, 2.0f, 2.0f,
        7.28f, 2.0f, -2.0f,
        3.28f, 2.0f, -2.0f,
        7.28f, -2.0f, -2.0f,
        7.28f, -2.0f, 2.0f,
        3.28f, -2.0f, 2.0f,
        3.28f, -2.0f, -2.0f,
        3.28f, -2.0f, 2.0f,
        3.28f, 2.0f, 2.0f,
        3.28f, 2.0f, -2.0f,
        3.28f, -2.0f, -2.0f,
        7.28f, -2.0f, -2.0f,
        7.28f, 2.0f, -2.0f,
        7.28f, 2.0f, 2.0f,
        7.28f, -2.0f, 2.0f,
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

    CSGMesh* meshA = leoCreateCSGMesh();
    leoCSGMeshSetVertices(meshA, 24, vertsA);
    leoCSGMeshSetTriangles(meshA, 12, tris);

    CSGMesh* meshB = leoCreateCSGMesh();
    leoCSGMeshSetVertices(meshB, 24, vertsB);
    leoCSGMeshSetTriangles(meshB, 12, tris);

    CSGMesh* result = leoPerformCSG(meshA, meshB, CSGOp::Union, nullptr, 0);
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
    // from https://www.ozeki.hu/attachments/16/Stanford_Bunny_sample.stl
    Mesh* meshA = StlReader::loadFromFile("../stl_test/Stanford_Bunny_sample.stl");
    Mesh* meshB = StlReader::loadFromFile("../stl_test/Stanford_Bunny_sample.stl");

    if (meshA == nullptr)
    {
        std::cerr << "Cannot load meshA" << std::endl;
        return 1;
    }
    else if (meshB == nullptr)
    {
        std::cerr << "Cannot load meshB" << std::endl;
        return 1;
    }

    //meshB->scale(float3(20, 20, 20));
    //meshB->translate(float3(0, 0, 40));
    meshB->rotate(90.0f, float3(0.0f, 1.0f, 1.0f), float3(0.0f, -100.0f, 0.0f));
    meshB->translate(float3(70.0f, 0.0f, 0.0f));
    meshA->weldVertices();
    meshB->weldVertices();

    CSGMesh* csgMeshA = meshA->makeNewCSGMesh();
    CSGMesh* csgMeshB = meshB->makeNewCSGMesh();

    constexpr int iterationCount = 5;
    for (size_t i = 0; i < iterationCount; i++)
    {
        char errorMsg[2048];
        auto start = std::chrono::steady_clock::now();
        CSGMesh* result = leoPerformCSG(csgMeshA, csgMeshB, CSGOp::Union, errorMsg, 2048);
        auto end = std::chrono::steady_clock::now();
        auto elapsedMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        if (result != nullptr)
        {
            Mesh* resultMesh = Mesh::fromCSGMesh(result);

            StlWriter::writeToFile(resultMesh, "../stl_test/csg_result.stl");
            //ObjWriter::writeToFile(resultMesh, "../stl_test/csg_result.obj");

            delete resultMesh;
            leoDestroyCSGMesh(result);
            std::cout << "CSG OK" << std::endl;
        }
        else
        {
            std::cout << errorMsg << std::endl;
        }

        std::cout << "Iteration " << i << " finished in " << elapsedMilliseconds << "ms" << std::endl;
    }

    leoDestroyCSGMesh(csgMeshA);
    leoDestroyCSGMesh(csgMeshB);

    delete meshA;
    delete meshB;

    //testCubes();
    //testPyramids();
    //testCustom();
    return 0;
}
