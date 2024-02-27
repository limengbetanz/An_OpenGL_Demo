cmake_minimum_required(VERSION 2.8)

project(OpenGLRayTracer)

add_executable(OpenGLRayTracer.out OpenGLRayTracer.cpp Cone.cpp Cylinder.cpp Plane.cpp Ray.cpp SceneObject.cpp Sphere.cpp TextureBMP.cpp)

find_package(OpenGL REQUIRED)

find_package(GLUT REQUIRED)

include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )

target_link_libraries( OpenGLRayTracer.out ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} )
