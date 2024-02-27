/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The Cylinder class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cylinder.h"
#include <math.h>
#include <iostream>

/**
* Sphere's intersection method.  The input is a ray. 
*/
glm::vec3 Cylinder::doubleLighting(glm::vec3 lightPos1, glm::vec3 lightPos2, glm::vec3 viewVec, glm::vec3 hit)
{
    float ambientTerm = 0.2;
    
    float specularTerm1 = 0;
    glm::vec3 normalVec = normal(hit);
    glm::vec3 lightVec = lightPos1 - hit;
    lightVec = glm::normalize(lightVec);
    float lDotn1 = glm::dot(lightVec, normalVec);
    if(lDotn1 < 0)
    {
        lDotn1 = glm::dot(lightVec, -normalVec);
    }

    if (spec_)
    {
        glm::vec3 reflVec = glm::reflect(-lightVec, normalVec);
        float rDotv = glm::dot(reflVec, viewVec);
        if (rDotv > 0) specularTerm1 = pow(rDotv, shin_);
    }
    
    float specularTerm2 = 0;
    normalVec = normal(hit);
    lightVec = lightPos2 - hit;
    lightVec = glm::normalize(lightVec);
    float lDotn2 = glm::dot(lightVec, normalVec);
    if(lDotn2 < 0)
    {
        lDotn2 = glm::dot(lightVec, -normalVec);
    }

    if (spec_)
    {
        glm::vec3 reflVec = glm::reflect(-lightVec, normalVec);
        float rDotv = glm::dot(reflVec, viewVec);
        if (rDotv > 0) specularTerm2 = pow(rDotv, shin_);
    }
    
    glm::vec3 colorSum = ambientTerm * color_ + lDotn1 * color_ + specularTerm1 * glm::vec3(1) + lDotn2 * color_ + specularTerm2 * glm::vec3(1);

    colorSum.x = colorSum.x < 0 ? 0 : colorSum.x;
    colorSum.y = colorSum.y < 0 ? 0 : colorSum.y;
    colorSum.z = colorSum.z < 0 ? 0 : colorSum.z;
    
    colorSum.x = colorSum.x > 1 ? 1 : colorSum.x;
    colorSum.y = colorSum.y > 1 ? 1 : colorSum.y;
    colorSum.z = colorSum.z > 1 ? 1 : colorSum.z;
    
    return colorSum;
}

float Cylinder::intersect(glm::vec3 p0, glm::vec3 dir)
{
    // dx^2 + dz^2
    float a = pow(dir.x, 2) + pow(dir.z, 2);
    
    // 2 * {dx(x0 - xc) + dz(z0 - zc)}
    float b = 2 * (dir.x * (p0.x - center.x) + dir.z * (p0.z - center.z));
    
    // (x0 - xc) ^ 2 + (z0 - zc) ^ 2 - r ^ 2
    float c = pow((p0.x - center.x), 2) + pow((p0.z - center.z), 2) - pow(radius, 2);
    
    double t1, t2;
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
    if (t1 > t2)
    {
        double temp = t2;
        t2 = t1;
        t1 = temp;
    }

    double intersectionY1 = p0.y + dir.y * t1;
    double intersectionY2 = p0.y + dir.y * t2;
    if (intersectionY1 > center.y && intersectionY1 < center.y + height)
    {
        return t1;
    }
    else
    {
        if (intersectionY2 > center.y && intersectionY2 < center.y + height)
        {
            return t2;
        }
        else
        {
            return -1;
        }
    }
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the sphere.
*/
glm::vec3 Cylinder::normal(glm::vec3 p)
{
    glm::vec3 n = glm::vec3((p.x - center.x) / radius, 0, (p.z - center.z) / radius);
    return n;
}

glm::vec2 Cylinder::textureCoords(glm::vec3 p)
{
    float s = atan((p.z - center.z) / (center.x - p.x)) / (2 * M_PI);
    float t = (p.y - center.y) / height;
    glm::vec2 coords = glm::vec2(s, t);
    return coords;
}
