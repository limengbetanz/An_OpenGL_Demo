/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The object class
*  This is a generic type for storing objects in the scene
*  Sphere, Plane etc. must be defined as subclasses of Object.
*  Being an abstract class, this class cannot be instantiated.
-------------------------------------------------------------*/

#include "SceneObject.h"

glm::vec3 SceneObject::getColor()
{
    return color_;
}

glm::vec3 SceneObject::lighting(glm::vec3 lightPos, glm::vec3 viewVec, glm::vec3 hit)
{
    float ambientTerm = 0.2;
    float specularTerm = 0;
    glm::vec3 normalVec = normal(hit);
    glm::vec3 lightVec = lightPos - hit;
    lightVec = glm::normalize(lightVec);
    float lDotn = glm::dot(lightVec, normalVec);
    if (spec_)
    {
        glm::vec3 reflVec = glm::reflect(-lightVec, normalVec);
        float rDotv = glm::dot(reflVec, viewVec);
        if (rDotv > 0) specularTerm = pow(rDotv, shin_);
    }
    glm::vec3 colorSum = ambientTerm * color_ + lDotn * color_ + specularTerm * glm::vec3(1);
    colorSum.x = colorSum.x < 0 ? 0 : colorSum.x;
    colorSum.y = colorSum.y < 0 ? 0 : colorSum.y;
    colorSum.z = colorSum.z < 0 ? 0 : colorSum.z;
    return colorSum;
}

glm::vec3 SceneObject::doubleLighting(glm::vec3 lightPos1, glm::vec3 lightPos2, glm::vec3 viewVec, glm::vec3 hit)
{
    float ambientTerm = 0.2;
    
    float specularTerm1 = 0;
    glm::vec3 normalVec = normal(hit);
    glm::vec3 lightVec = lightPos1 - hit;
    lightVec = glm::normalize(lightVec);
    float lDotn1 = glm::dot(lightVec, normalVec);
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

glm::vec3 SceneObject::shadow()
{
    float ambientTerm = 0.2;
    glm::vec3 colorSum = ambientTerm * color_;
    return colorSum;
}

float SceneObject::getReflectionCoeff()
{
    return reflc_;
}

float SceneObject::getRefractionCoeff()
{
    return refrc_;
}

float SceneObject::getTransparencyCoeff()
{
    return tranc_;
}

float SceneObject::getRefractiveIndex()
{
    return refri_;
}

float SceneObject::getShininess()
{
    return shin_;
}

bool SceneObject::isReflective()
{
    return refl_;
}

bool SceneObject::isRefractive()
{
    return refr_;
}


bool SceneObject::isSpecular()
{
    return spec_;
}


bool SceneObject::isTransparent()
{
    return tran_;
}

void SceneObject::setColor(glm::vec3 col)
{
    color_ = col;
}

void SceneObject::setReflectivity(bool flag)
{
    refl_ = flag;
}

void SceneObject::setReflectivity(bool flag, float refl_coeff)
{
    refl_ = flag;
    reflc_ = refl_coeff;
}

void SceneObject::setRefractivity(bool flag)
{
    refr_ = flag;
}

void SceneObject::setRefractivity(bool flag, float refr_coeff, float refr_index)
{
    refr_ = flag;
    refrc_ = refr_coeff;
    refri_ = refr_index;
}

void SceneObject::setShininess(float shininess)
{
    shin_ = shininess;
}

void SceneObject::setSpecularity(bool flag)
{
    spec_ = flag;
}

void SceneObject::setTransparency(bool flag)
{
    tran_ = flag;
}

void SceneObject::setTransparency(bool flag, float tran_coeff)
{
    tran_ = flag;
    tranc_ = tran_coeff;
}
