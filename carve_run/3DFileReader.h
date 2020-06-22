#ifndef THREED_FILE_READER_H
#define THREED_FILE_READER_H

#include "../carve/carve.h"
#include <string>
#include <vector>

template <typename T> struct Vector3
{
    T x, y, z;

    Vector3() : x(0), y(0), z(0)
    {
    }
    Vector3(T x, T y, T z) : x(x), y(y), z(z)
    {
    }

    T dot(Vector3 other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    T lengthsq() const
    {
        return dot(*this);
    }

    T length() const
    {
        return sqrt(lengthsq());
    }

    Vector3 cross(Vector3 other)
    {
        return Vector3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.z);
    }

    Vector3 operator+(const Vector3& other)
    {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }

    Vector3 operator-(const Vector3& other)
    {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }

    Vector3 operator*(const Vector3& other)
    {
        return Vector3(x * other.x, y * other.y, z * other.z);
    }

    Vector3 operator/(const Vector3& other)
    {
        return Vector3(x / other.x, y / other.y, z / other.z);
    }

    Vector3 operator*(const T& other)
    {
        return Vector3(x * other, y * other, z * other);
    }

    Vector3 operator/(const T& other)
    {
        return Vector3(x / other, y / other, z / other);
    }

    bool operator==(const Vector3& other)
    {
        return x == other.x && y == other.y && z == other.z;
    }

    bool operator!=(const Vector3& other)
    {
        return !(operator==(other));
    }

    Vector3 normalized()
    {
        T len = length();
        return Vector3(x / len, y / len, z / len);
    }
};

using float3 = Vector3<float>;
using double3 = Vector3<double>;

struct Mesh
{
    std::vector<float3> vertices;
    std::vector<int> indices;

    CSGMesh* makeNewCSGMesh() const;
    static Mesh* fromCSGMesh(CSGMesh* csgMesh);

    void scale(float3 sc);
    void translate(float3 tr);
    void rotate(float angleInDegrees, float3 axisDirection, float3 axisPoint);
    void weldVertices();
};

class StlReader
{
public:
    static Mesh* loadFromFile(std::string path);

private:
    static Mesh* loadBinary(std::vector<unsigned char>& data);
    static Mesh* loadAscii(std::vector<unsigned char>& data);
};

class StlWriter
{
public:
    static void writeToFile(const Mesh* mesh, std::string fileName);
};

class ObjWriter
{
public:
    static void writeToFile(const Mesh* mesh, std::string fileName);
};

#endif
