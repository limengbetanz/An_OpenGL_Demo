//  =============================================================================
//  COSC363: Computer Graphics (2020) Assigment 2;
//  University of Canterbury.
//
//  FILE NAME: OpenGLRayTracer
//
//  =============================================================================

#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "SceneObject.h"
#include "Ray.h"
#include <GL/freeglut.h>
#include "Plane.h"
#include "TextureBMP.h"
#include "Cylinder.h"
#include "Cone.h"

const int CELL_COUNT = 800;
const float Z_NEAR = 40.0;
const float Z_FAR = 200.0;
const int MAX_STEPS = 5;

const float VIEW_WIDTH = 20.0;
const float VIEW_HEIGHT = 20.0;
const float X_MIN = -VIEW_WIDTH * 0.5;
const float X_MAX =  VIEW_WIDTH * 0.5;
const float Y_MIN = -VIEW_HEIGHT * 0.5;
const float Y_MAX =  VIEW_HEIGHT * 0.5;

vector<SceneObject*> sceneObjects;
TextureBMP wallTexture;
TextureBMP cylinderTexture;

const int PROCEDURAL_PATTEN_HEIGHT = 500;
const int PROCEDURAL_PATTEN_WIDTH = 500;
const int PROCEDURAL_PATTEN_COLOR_DEPTH = 3;
float proceduralPatternTexture[PROCEDURAL_PATTEN_WIDTH * PROCEDURAL_PATTEN_HEIGHT * PROCEDURAL_PATTEN_COLOR_DEPTH] = {0};

glm::vec3 trace(Ray ray, int step)
{
    glm::vec3 backgroundColor(0);
    glm::vec3 lightPosRight(15, 30, 10);
    glm::vec3 lightPosLeft(-15, 30, 10);
    glm::vec3 color(0);
    SceneObject* obj;

    ray.closestPt(sceneObjects);
    if(ray.index == -1)
    {
        return backgroundColor;
    }

    obj = sceneObjects[ray.index];

    switch(ray.index)
    {
        // Floor
        case 0:
        {
            int blockLength = 5;
            int iz = (ray.hit.z) / blockLength;
            int ix = (ray.hit.x) / blockLength;
            int mz = iz % 2;
            int mx = ix % 2;

            if (mz == 0)
            {
                if (ray.hit.x <= 0)
                {
                    if(mx == 0)
                    {
                        color = glm::vec3(0, 1, 1);
                    }
                    else
                    {
                        color = glm::vec3(1, 1, 0);
                    }
                }
                else
                {
                    if(mx == 0)
                    {
                        color = glm::vec3(1, 1, 0);
                    }
                    else
                    {
                        color = glm::vec3(0, 1, 1);
                    }
                }
            }
            else
            {
                if (ray.hit.x <= 0)
                {
                    if(mx == 0)
                    {
                        color = glm::vec3(1, 1, 0);
                    }
                    else
                    {
                        color = glm::vec3(0, 1, 1);
                    }
                }
                else
                {
                    if(mx == 0)
                    {
                        color = glm::vec3(0, 1, 1);
                    }
                    else
                    {
                        color = glm::vec3(1, 1, 0);
                    }
                }
            }
            obj->setColor(color);
            break;
        }
        // Wall
        case 1:
        {
            int repeatTimes = 15;
            float texcoords = (ray.hit.x + 60) / repeatTimes - int((ray.hit.x + 60) / repeatTimes);
            float texcoordt = (ray.hit.y + 60) / repeatTimes - int((ray.hit.y + 60) / repeatTimes);
            color = wallTexture.getColorAt(texcoords, texcoordt);
            obj->setColor(color);
            break;
        }
        case 3:
        {
            // Procedural patten
            int texcoordsIndex = (int)((10 + ray.hit.x) / 4.0 * PROCEDURAL_PATTEN_WIDTH);
            int texcoordtIndex = (int)((10 + ray.hit.y) / 4.0 * PROCEDURAL_PATTEN_HEIGHT);
            int index = (texcoordtIndex * PROCEDURAL_PATTEN_WIDTH + texcoordsIndex) * PROCEDURAL_PATTEN_COLOR_DEPTH;
            color.r = proceduralPatternTexture[index];
            color.g = proceduralPatternTexture[index + 1];
            color.b = proceduralPatternTexture[index + 2];
            
            color.r = color.r * 0.6;
            color.g = color.g * 0.6;
            color.b = color.b * 0.6;
            
            obj->setColor(color);
            break;
        }
        // Cylinder
        case 9:
        {
            Cylinder *c = (Cylinder*)obj;
            glm::vec2 texCoord = c->textureCoords(ray.hit);
            texCoord.x = texCoord.x * 2 * 2 * M_PI / 3;

            if(texCoord.x > 0)
            {
                while (texCoord.x > 1.0)
                {
                    texCoord.x -= 1;
                }
            }
            else
            {
                while (texCoord.x < 0)
                {
                    texCoord.x += 1;
                }
            }

            color = cylinderTexture.getColorAt(texCoord.x, texCoord.y);
            color.r = color.r * 0.6;
            color.g = color.g * 0.6;
            color.b = color.b * 0.6;
            
            obj->setColor(color);
            break;
        }
        default:
        {
            break;
        }
    }

    
    if(obj->type == 2)
    {
        Cylinder *c = (Cylinder *)obj;
        color = c->doubleLighting(lightPosLeft, lightPosRight, -ray.dir, ray.hit);
    }
    else
    {
        color = obj->doubleLighting(lightPosLeft, lightPosRight, -ray.dir, ray.hit);
    }
    
    glm::vec3 lightVecRight = lightPosRight - ray.hit;
    Ray shadowRayRight(ray.hit, lightVecRight);
    shadowRayRight.closestPt(sceneObjects);

    glm::vec3 lightVecLeft = lightPosLeft - ray.hit;
    Ray shadowRayLeft(ray.hit, lightVecLeft);
    shadowRayLeft.closestPt(sceneObjects);

    bool hasLeftShadow = shadowRayLeft.index > -1 && shadowRayLeft.dist < glm::length(lightVecLeft);
    bool hasRightShadow = shadowRayRight.index > -1 && shadowRayRight.dist < glm::length(lightVecRight);

    if(shadowRayLeft.hitSceneObject == obj)
    {
        hasLeftShadow = false;
    }

    if(shadowRayRight.hitSceneObject == obj)
    {
        hasRightShadow = false;
    }
    
    if(obj->type == 1)
    {
        hasLeftShadow = false;
        hasRightShadow = false;
    }

    float factor = 1.46;
    if(hasLeftShadow && hasRightShadow)
    {
        color = obj->shadow();
        SceneObject* shadowObj = sceneObjects[shadowRayRight.index];
        if (shadowObj->isRefractive() || shadowObj->isTransparent())
        {
            glm::vec3 color1 = obj->lighting(lightPosRight, -ray.dir, ray.hit);
            glm::vec3 color2 = obj->lighting(lightPosLeft, -ray.dir, ray.hit);
            
            color.r = (color1.r + color2.r) * factor * 0.45;
            color.g = (color1.g + color2.g) * factor * 0.45;
            color.b = (color1.b + color2.b) * factor * 0.45;
        }
    }
    else if(!hasLeftShadow && hasRightShadow)
    {
        color = obj->lighting(lightPosRight, -ray.dir, ray.hit);
        SceneObject* shadowObj = sceneObjects[shadowRayRight.index];
        if (shadowObj->isRefractive() || shadowObj->isTransparent())
        {
            color.r = color.r * factor > 1 ? 1 : color.r * factor;
            color.g = color.g * factor > 1 ? 1 : color.g * factor;
            color.b = color.b * factor > 1 ? 1 : color.b * factor;
        }
    }
    else if(hasLeftShadow && !hasRightShadow)
    {
        color = obj->lighting(lightPosLeft, -ray.dir, ray.hit);
        SceneObject* shadowObj = sceneObjects[shadowRayLeft.index];
        if (shadowObj->isRefractive() || shadowObj->isTransparent())
        {
            color.r = color.r * factor > 1 ? 1 : color.r * factor;
            color.g = color.g * factor > 1 ? 1 : color.g * factor;
            color.b = color.b * factor > 1 ? 1 : color.b * factor;
        }
    }

    if (obj->isReflective() && step < MAX_STEPS)
    {
        float rho = obj->getReflectionCoeff();
        glm::vec3 normalVec = obj->normal(ray.hit);
        glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
        Ray reflectedRay(ray.hit, reflectedDir);
        glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
        color = color + (rho * reflectedColor);
    }

    if (obj->isTransparent() && step < MAX_STEPS)
    {
        float factor = obj->getTransparencyCoeff();
        Ray reflectedRay(ray.hit, ray.dir);
        glm::vec3 backgroundColor = trace(reflectedRay, step + 1);
        color = (1 - factor) * color + factor * backgroundColor;
    }

    if (obj->isRefractive() && step < MAX_STEPS)
    {
        float eta = 0.992;
        glm::vec3 n = obj->normal(ray.hit);
        glm::vec3 g = glm::refract(ray.dir, n, eta);
        Ray refrRayInward(ray.hit, g);
        refrRayInward.closestPt(sceneObjects);
        glm::vec3 m = obj->normal(refrRayInward.hit);
        glm::vec3 h = glm::refract(g, -m, 1.0f/eta);

        Ray refrRayOurward(refrRayInward.hit, h);
        glm::vec3 refractiveColor = trace(refrRayOurward, step + 1);
        color = refractiveColor;
    }

    return color;
}

void display()
{
    float xp, yp;
    float cellX = (X_MAX - X_MIN) / CELL_COUNT;
    float cellY = (Y_MAX - Y_MIN) / CELL_COUNT;

    glm::vec3 eye(0., 0., 0.);

    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBegin(GL_QUADS);
    for(int i = 0; i < CELL_COUNT; i++)
    {
        xp = X_MIN + i * cellX;
        for(int j = 0; j < CELL_COUNT; j++)
        {
            yp = Y_MIN + j * cellY;

            int antiAliasingFactor = 2;
            int subCellCount = antiAliasingFactor;
            float subCellX = cellX / float(antiAliasingFactor);
            float subCellY = cellY / float(antiAliasingFactor);

            vector<glm::vec3> colors;
            for(int k = 0; k < subCellCount; k++)
            {
                float subxp = xp + k * subCellX;
                for(int h = 0; h < subCellCount; h++)
                {
                    float subyp = yp + h * subCellY;

                    glm::vec3 dir(subxp + 0.5 * subCellX, subyp + 0.5 * subCellY, -Z_NEAR);
                    Ray ray = Ray(eye, dir);
                    glm::vec3 col = trace(ray, 1);
                    colors.push_back(col);
                }
            }

            glm::vec3 color = glm::vec3(0.0);
            for (uint m = 0; m < colors.size(); m++)
            {
                glm::vec3 col = colors[m];
                color.r += col.r;
                color.g += col.g;
                color.b += col.b;
            }

            color.r /= colors.size();
            color.g /= colors.size();
            color.b /= colors.size();

            glColor3f(color.r, color.g, color.b);
            glVertex2f(xp, yp);
            glVertex2f(xp + cellX, yp);
            glVertex2f(xp + cellX, yp + cellY);
            glVertex2f(xp, yp + cellY);
        }
    }
    glEnd();
    glFlush();
}

void initialize()
{
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(X_MIN, X_MAX, Y_MIN, Y_MAX);

    glClearColor(0, 0, 0, 1);

    // Floor
    Plane *floor = new Plane (glm::vec3(-60.0, -10, -Z_NEAR + 20),
                              glm::vec3(60.0, -10, -Z_NEAR + 20),
                              glm::vec3(60.0, -10, -Z_FAR),
                              glm::vec3(-60.0, -10, -Z_FAR));
    floor->setSpecularity(false);
    sceneObjects.push_back(floor);

    // Wall
    Plane *wall = new Plane (glm::vec3(-60.0, -10, -Z_FAR),
                             glm::vec3(60.0, -10, -Z_FAR),
                             glm::vec3(60.0, 70, -Z_FAR),
                             glm::vec3(-60.0, 70, -Z_FAR));
    wall->setSpecularity(false);
    sceneObjects.push_back(wall);

    // Box
    float side = 4;
    float left = -10;
    float right = left + side;
    float down = -10;
    float up = down + side;
    float front = -60;
    float back = front - side;

    Plane *boxUp = new Plane (glm::vec3(left, up, front),
                              glm::vec3(right, up, front),
                              glm::vec3(right, up, back),
                              glm::vec3(left, up, back));
    boxUp->setSpecularity(false);
    boxUp->setColor(glm::vec3(1, 0, 0));
    boxUp->type = 1;
    sceneObjects.push_back(boxUp);

    Plane *boxFront = new Plane (glm::vec3(left, down, front),
                                glm::vec3(right, down, front),
                                glm::vec3(right, up, front),
                                glm::vec3(left, up, front));
    boxFront->setSpecularity(false);
    boxFront->setColor(glm::vec3(0, 1, 0));
    boxFront->type = 1;
    sceneObjects.push_back(boxFront);

    Plane *boxLeft = new Plane (glm::vec3(left, down, back),
                                glm::vec3(left, down, front),
                                glm::vec3(left, up, front),
                                glm::vec3(left, up, back));
    boxLeft->setSpecularity(false);
    boxLeft->setColor(glm::vec3(0, 1, 0));
    boxLeft->type = 1;
    sceneObjects.push_back(boxLeft);

    Plane *boxBack = new Plane (glm::vec3(right, down, back),
                                glm::vec3(left, down, back),
                                glm::vec3(left, up, back),
                                glm::vec3(right, up, back));
    boxBack->setSpecularity(false);
    boxBack->setColor(glm::vec3(1, 1, 0));
    boxBack->type = 1;
    sceneObjects.push_back(boxBack);

    Plane *boxRight = new Plane (glm::vec3(right, down, front),
                                glm::vec3(right, down, back),
                                glm::vec3(right, up, back),
                                glm::vec3(right, up, front));
    boxRight->setSpecularity(false);
    boxRight->setColor(glm::vec3(0, 1, 0));
    boxRight->type = 1;
    sceneObjects.push_back(boxRight);

    Sphere *transparentSphere = new Sphere(glm::vec3(0.5, 5.0, -80.0), 10.0);
    transparentSphere->setColor(glm::vec3(1, 1, 1));
    transparentSphere->setReflectivity(true, 0.8);
    transparentSphere->setTransparency(true, 0.8);
    sceneObjects.push_back(transparentSphere);

    Sphere *refractiveSphere = new Sphere(glm::vec3(7.0, -2.0, -60.0), 3.0);
    refractiveSphere->setColor(glm::vec3(0.0 / 255, 100.0 / 255, 100.0 / 255));
    refractiveSphere->setRefractivity(true);
    sceneObjects.push_back(refractiveSphere);

    Cylinder *cylinder = new Cylinder(glm::vec3(10, -10.0, -60.0), 2.0, 3.0);
    cylinder->setColor(glm::vec3(1, 1, 1));
    sceneObjects.push_back(cylinder);

    Cone *cone = new Cone(glm::vec3(0, -10.0, -60.0), 2.0, 4.0);
    cone->setColor(glm::vec3(100.0 / 255, 100.0 / 255, 0.0));
    sceneObjects.push_back(cone);

    wallTexture = TextureBMP("Wall.bmp");
    cylinderTexture = TextureBMP("VaseTexture.bmp");
}

void generetaProceduralPatternTexture()
{
    for(int i = 0; i < PROCEDURAL_PATTEN_WIDTH * PROCEDURAL_PATTEN_HEIGHT * PROCEDURAL_PATTEN_COLOR_DEPTH; i += 3)
    {
        proceduralPatternTexture[i] = 0.5;
        proceduralPatternTexture[i + 1] = 0.5;
        proceduralPatternTexture[i + 2] = 0.5;
    }

    vector<glm::vec2> colors;

    for(int i = 0; i < PROCEDURAL_PATTEN_WIDTH; i++)
    {
        float radian = 2 * M_PI * (float)i / PROCEDURAL_PATTEN_WIDTH;
        float sina = sinf(radian + M_PI);

        int targetPixelIndex = 0;
        if(sina > 0)
        {
            targetPixelIndex = (int)(PROCEDURAL_PATTEN_HEIGHT * 0.5 - sina * PROCEDURAL_PATTEN_WIDTH * 0.5);
        }
        else
        {
            targetPixelIndex = (int)(abs(sina) * PROCEDURAL_PATTEN_WIDTH * 0.5 + PROCEDURAL_PATTEN_HEIGHT * 0.5);
        }

        colors.push_back(glm::vec2(i, targetPixelIndex));
    }

    for(uint i = 0; i < colors.size(); ++i)
    {
        glm::vec2 point1 = colors[i];

        int min = point1.y;
        int max = PROCEDURAL_PATTEN_WIDTH * 0.5;
        if(max < min)
        {
            min = PROCEDURAL_PATTEN_WIDTH * 0.5;
            max = point1.y;
        }

        for(int j = min; j < max; j++)
        {
            proceduralPatternTexture[(j * PROCEDURAL_PATTEN_WIDTH + i) * 3] = 1;
            proceduralPatternTexture[(j * PROCEDURAL_PATTEN_WIDTH + i) * 3 + 1] = 164 / 255.0;
            proceduralPatternTexture[(j * PROCEDURAL_PATTEN_WIDTH + i) * 3 + 2] = 0;
        }
    }
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(1000, 1000);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("OpenGL Ray Tracer");
    generetaProceduralPatternTexture();
    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}
