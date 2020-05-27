
#include <fstream>
#include <regex>
#include "3DFileReader.h"

Mesh* StlReader::loadFromFile(std::string path)
{
    std::ifstream file(path, std::ios::binary);
    if (file.fail() || file.eof())
    {
        file.close();
        return nullptr;
    }

    file.seekg(0, std::ios::ios_base::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::ios_base::beg);

    std::vector<unsigned char> data;
    data.resize(fileSize);

    file.read((char*)data.data(), fileSize);
    file.close();

    bool isBinary = false;
    if (data.size() >= 84)
    {
        // check if binary
        if (data[0] != 's' || data[1] != 'o' || data[2] != 'l' || data[3] != 'i' || data[4] != 'd' || data[5] != ' ')
        {
            // not starting with "solid ", binary for sure
            isBinary = true;
        }
        else
        {
            uint32_t numFacets = ((uint32_t)(data[83]) << 24) | ((uint32_t)(data[82]) << 16) | ((uint32_t)(data[81]) << 8) | (uint32_t)(data[80]);
            if ((size_t)numFacets * 50 == (size_t)fileSize - 84)
            {
                // binary
                isBinary = true;
            }
        }
    }

    return isBinary ? loadBinary(data) : loadAscii(data);
}

Mesh* StlReader::loadBinary(std::vector<unsigned char>& data)
{
    uint32_t numFacets = ((uint32_t)(data[83]) << 24) | ((uint32_t)(data[82]) << 16) | ((uint32_t)(data[81]) << 8) | (uint32_t)(data[80]);

    Mesh* mesh = new Mesh();
    mesh->vertices.resize(numFacets * 3);
    mesh->indices.resize(numFacets * 3);

    auto readFloat = [](unsigned char* ptr)
    {
        float f;
        std::memcpy(&f, ptr, 4);
        return f;
    };

    unsigned char* dataPtr = data.data();

    for (int i = 0; i < numFacets; ++i)
    {
        int arrayIndex = 84 + 50 * i;

        int meshIndex = i * 3;
        mesh->vertices[meshIndex] = float3(readFloat(dataPtr + arrayIndex + 12), readFloat(dataPtr + arrayIndex + 16), readFloat(dataPtr + arrayIndex + 20));
        mesh->vertices[meshIndex + 1] = float3(readFloat(dataPtr + arrayIndex + 24), readFloat(dataPtr + arrayIndex + 28), readFloat(dataPtr + arrayIndex + 32));
        mesh->vertices[meshIndex + 2] = float3(readFloat(dataPtr + arrayIndex + 36), readFloat(dataPtr + arrayIndex + 40), readFloat(dataPtr + arrayIndex + 44));

        mesh->indices[meshIndex] = meshIndex;
        mesh->indices[meshIndex + 1] = meshIndex + 1;
        mesh->indices[meshIndex + 2] = meshIndex + 2;
    }

    return mesh;
}

Mesh* StlReader::loadAscii(std::vector<unsigned char>& data)
{
    std::string fileContents(data.begin(), data.end());

    const static std::string numberRegex = "(-?\\d+(\\.\\d+)?(e(\\+|\\-)?\\d+)?)";
    constexpr int captureGroupsPerNumber = 4;
    const static std::string regexString = "facet normal " + numberRegex + " " + numberRegex + " " + numberRegex + "\\s+outer loop\\s+vertex " + numberRegex + " " + numberRegex + " " + numberRegex + "\\s+vertex " + numberRegex + " " + numberRegex + " " + numberRegex + "\\s+vertex " + numberRegex + " " + numberRegex + " " + numberRegex + "\\s+endloop\\s+endfacet";
    const static std::regex regexp(regexString);
    
    std::string::const_iterator searchStart = fileContents.cbegin();

    Mesh* mesh = new Mesh();

    std::smatch regexMatch;
    while (std::regex_search(searchStart, fileContents.cend(), regexMatch, regexp))
    {
        try
        {
            float v0x = std::stof(regexMatch[captureGroupsPerNumber * 3 + 1].str());
            float v0y = std::stof(regexMatch[captureGroupsPerNumber * 4 + 1].str());
            float v0z = std::stof(regexMatch[captureGroupsPerNumber * 5 + 1].str());

            float v1x = std::stof(regexMatch[captureGroupsPerNumber * 6 + 1].str());
            float v1y = std::stof(regexMatch[captureGroupsPerNumber * 7 + 1].str());
            float v1z = std::stof(regexMatch[captureGroupsPerNumber * 8 + 1].str());

            float v2x = std::stof(regexMatch[captureGroupsPerNumber * 9 + 1].str());
            float v2y = std::stof(regexMatch[captureGroupsPerNumber * 10 + 1].str());
            float v2z = std::stof(regexMatch[captureGroupsPerNumber * 11 + 1].str());

            mesh->vertices.push_back(float3(v0x, v0y, v0z));
            mesh->vertices.push_back(float3(v1x, v1y, v1z));
            mesh->vertices.push_back(float3(v2x, v2y, v2z));

			int index = mesh->indices.size();
			mesh->indices.push_back(index);
			mesh->indices.push_back(index + 1);
			mesh->indices.push_back(index + 2);
        }
        catch (...)
        {
            delete mesh;
            return nullptr;
        }

        searchStart = regexMatch.suffix().first;
    }

    return mesh;
}

CSGMesh* Mesh::makeNewCSGMesh() const
{
    CSGMesh* csgMesh = leoCreateCSGMesh();
    std::vector<float> floatVertices;
    floatVertices.reserve(vertices.size() * 3);
    for (int i = 0; i < vertices.size(); ++i)
    {
        float3 vertex = vertices[i];
        floatVertices.push_back(vertex.x);
        floatVertices.push_back(vertex.y);
        floatVertices.push_back(vertex.z);
    }

    leoCSGMeshSetVertices(csgMesh, vertices.size(), floatVertices.data());
    leoCSGMeshSetTriangles(csgMesh, indices.size() / 3, indices.data());

    return csgMesh;
}

Mesh* Mesh::fromCSGMesh(CSGMesh* csgMesh)
{
    int numVertices = leoCSGMeshGetVertexCount(csgMesh);
    int numIndices = leoCSGMeshGetTriangleCount(csgMesh) * 3;

    std::vector<float> floatVertices;
    floatVertices.resize(numVertices * 3);
    leoCSGMeshGetVertices(csgMesh, floatVertices.data());

    Mesh* mesh = new Mesh();
    mesh->vertices.resize(numVertices);

    for (int i = 0; i < numVertices; ++i)
    {
        mesh->vertices[i] = float3(
            floatVertices[i * 3],
            floatVertices[i * 3 + 1],
            floatVertices[i * 3 + 2]
        );
    }

    mesh->indices.resize(numIndices);
    leoCSGMeshGetTriangles(csgMesh, mesh->indices.data());

    return mesh;
}

void Mesh::scale(float3 sc)
{
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        vertices[i] = vertices[i] * sc;
    }
}

void Mesh::translate(float3 tr)
{
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        vertices[i] = vertices[i] + tr;
    }
}

void StlWriter::writeToFile(const Mesh* mesh, std::string fileName)
{
    std::ofstream file(fileName, std::ios::binary, std::ios::trunc);
    if (file.fail())
    {
        return;
    }

    uint32_t numTriangles = mesh->indices.size() / 3;
    std::vector<unsigned char> fileData;
    fileData.resize(numTriangles * 50 + 84);

    fileData[80] = numTriangles & 255;
    fileData[81] = (numTriangles >> 8) & 255;
    fileData[82] = (numTriangles >> 16) & 255;
    fileData[83] = (numTriangles >> 24) & 255;

    auto writeFloat = [](unsigned char* ptr, float value)
    {
        std::memcpy(ptr, &value, 4);
    };

    unsigned char* dataPtr = fileData.data();

    for (uint32_t i = 0; i < numTriangles; ++i)
    {
        int arrayIndex = 84 + 50 * i;

        int meshIndex = i * 3;

        float3 v0 = mesh->vertices[meshIndex];
        float3 v1 = mesh->vertices[meshIndex + 1];
        float3 v2 = mesh->vertices[meshIndex + 2];

        float3 normal = ((v1 - v0).cross(v2 - v1)).normalized();

        writeFloat(dataPtr + arrayIndex, normal.x);
        writeFloat(dataPtr + arrayIndex + 4, normal.y);
        writeFloat(dataPtr + arrayIndex + 8, normal.z);

        writeFloat(dataPtr + arrayIndex + 12, v0.x);
        writeFloat(dataPtr + arrayIndex + 16, v0.y);
        writeFloat(dataPtr + arrayIndex + 20, v0.z);

        writeFloat(dataPtr + arrayIndex + 24, v1.x);
        writeFloat(dataPtr + arrayIndex + 28, v1.y);
        writeFloat(dataPtr + arrayIndex + 32, v1.z);

        writeFloat(dataPtr + arrayIndex + 36, v2.x);
        writeFloat(dataPtr + arrayIndex + 40, v2.y);
        writeFloat(dataPtr + arrayIndex + 44, v2.z);
    }

    file.write((char*)fileData.data(), fileData.size());
    file.close();
}
