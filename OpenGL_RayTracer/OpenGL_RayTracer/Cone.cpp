/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The Cone class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cone.h"
#include <math.h>

/**
* Sphere's intersection method.  The input is a ray. 
*/
float Cone::intersect(glm::vec3 p0, glm::vec3 dir)
{
    float tan = (radius * radius) / (height * height);
    float a = pow(dir.x, 2) + pow(dir.z, 2) - tan * pow(dir.y, 2);
    float b = 2 * dir.z * (p0.z - center.z) + 2 * dir.x * (p0.x - center.x)
    + 2 * dir.y * tan * (height - p0.y + center.y);
    float c = (p0.x - center.x) * (p0.x - center.x) + (p0.z - center.z) * (p0.z - center.z)
    - tan * (height - p0.y + center.y) * (height - p0.y + center.y);
    
    double t, t1, t2;
    double q = b * b - 4 * a * c;
    if(fabs(q) < 1e-6)
    {
        return -1.0;
    }
    
    if(q < 0.0)
    {
        return -1.0;
    }

    t1 = (-b - sqrt(q)) / (2 * a);
    t2 = (-b + sqrt(q)) / (2 * a);
    t = t1 > t2 ? t2 : t1;
    
    float intersectionY1 = p0.y + t * dir.y;
    if (intersectionY1 > center.y && intersectionY1 < center.y + height)
    {
        return t;
    }
    else
    {
        return -1;
    }
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the sphere.
*/
glm::vec3 Cone::normal(glm::vec3 p)
{
    float a = atan((p.x - center.x) / (p.z - center.z));
    float b = atan(radius / height);
    glm::vec3 n = glm::vec3(sinf(a) * cosf(b), sinf(b), cosf(a) * cosf(b));
    return n;
}
