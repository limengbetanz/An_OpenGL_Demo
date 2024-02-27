//  =============================================================================
//  COSC363: Computer Graphics (2020) Assigment 1;  
//  University of Canterbury.
//
//  FILE NAME: OpenGLMuseum
//
//  DESCRIPTION: The Program displays an OpenGL museum that houses a few  
//  interesting exhibits created using OpenGL models.
//
//  INSTRUCTIONS: 
//  up arrow key - move the camera forward
//  down arrow key - move the camera backward
//  left arrow key - change the direction of motion towards left  
//                   by a certain angle
//  right arrow key - change the direction of motion towards right  
//                    by a certain angle
//  ============================================================================= 
 
#include <iostream>
#include <fstream>
#include <climits>
#include <math.h> 
#include <GL/freeglut.h>
#include <stdlib.h>
#include <cstring>

#include "imageLoader.h"

using namespace std;

const float FRAME_RATE = 30.0f;

/*
 *********************************
 ******* Model definition ********
 *********************************
 */
 
class Model {
	
	public:
		Model(const char* fileName);
		Model();
		~Model();

		void draw();
		void setColor(float r, float g, float b, float a = 1.0f);
		void loadTextureFile(const char* fname, bool useRepetition, bool useModulate);
		void generateSphere(float radius,  int stacksCount, int slicesCount);

	private:
		float *vertices_x;
		float *vertices_y;
		float *vertices_z;

		float *normal_x;
		float *normal_y;
		float *normal_z;

		int *indices_1;
		int *indices_2;
		int *indices_3;

		float *texture_s;
		float *texture_t;
		
		int nvrt;
		int ntri;
		
		float r = 1.0f;
		float g = 1.0f;
		float b = 1.0f;
		float a = 1.0f;
		
		GLuint textureId = 0;
		bool useModulate;
		
		void loadMeshFile(const char* fname);
		void calculate_normal();
};

Model::Model()
{
	
}

Model::Model(const char* fileName)
{
	loadMeshFile(fileName);
	calculate_normal();
}

Model::~Model() 
{
	delete vertices_x;
	delete vertices_y;
	delete vertices_z;
	delete normal_x;
	delete normal_y;
	delete normal_z;
	delete indices_1;
	delete indices_2;
	delete indices_3;
	
	if (texture_s != NULL) {
		delete texture_s;
	}

	if (texture_t != NULL) {
		delete texture_t;
	} 
}

void Model::generateSphere(float radius, int stacksCount, int slicesCount)
{
	int stacks = stacksCount;
	int slices = slicesCount;

	nvrt = (stacks + 1) * (slices + 1);

	vertices_x = new float[nvrt];                    
	vertices_y = new float[nvrt];
	vertices_z = new float[nvrt];

	float x, y, z, xz;

	float slicesStep = 2 * M_PI / slices;
	float stackStep = M_PI / stacks;
	float sliceAngle, stackAngle;
	
	int index = 0;
	
	for(int i = 0; i <= stacks; ++i)
	{
		stackAngle = M_PI / 2 - i * stackStep;
		xz = radius * cosf(stackAngle);
		y = radius * sinf(stackAngle); 

		for(int j = 0; j <= slices; ++j)
		{
			sliceAngle = j * slicesStep;

			x = xz * cosf(sliceAngle);
			z = xz * sinf(sliceAngle);
			vertices_x[index] = x;
			vertices_y[index] = y;
			vertices_z[index++] = -z;
		}
	}

	float s, t;      

	ntri = (stacks - 2) * slices * 2 +  2 * slices;

	indices_1 = new int[ntri];
	indices_2 = new int[ntri];
	indices_3 = new int[ntri];
	
	texture_s = new float[ntri * 3];
	texture_t = new float[ntri * 3];
	
	index = 0;
	int currentTextureIndex = 0;
	
	int k1, k2;
	for(int i = 0; i < stacks; ++i)
	{
		k1 = i * (slices + 1);
		k2 = k1 + slices + 1;

		for(int j = 0; j < slices; ++j, ++k1, ++k2)
		{
			if(i != 0)
			{
				indices_1[index] = k1;
				indices_2[index] = k2;
				indices_3[index++] = k1 + 1;

				s = (float)j / slices;
				t = (float)i / stacks;
				texture_s[currentTextureIndex] = s;
				texture_t[currentTextureIndex++] = 1.0 - t;

				s = (float)j / slices;
				t = (float)(i + 1) / stacks;
				texture_s[currentTextureIndex] = s;
				texture_t[currentTextureIndex++] = 1.0 - t;
				
				s = (float)(j + 1) / slices;
				t = (float)i / stacks;
				texture_s[currentTextureIndex] = s;
				texture_t[currentTextureIndex++] = 1.0 - t;
			}

			if(i != stacks-1)
			{
				indices_1[index] = k1 + 1;
				indices_2[index] = k2;
				indices_3[index++] = k2 + 1;

				s = (float)(j + 1) / slices;
				t = (float)i / stacks;
				texture_s[currentTextureIndex] = s;
				texture_t[currentTextureIndex++] = 1.0 - t;

				s = (float)j / slices;
				t = (float)i / stacks;
				texture_s[currentTextureIndex] = s;
				texture_t[currentTextureIndex++] = 1.0 - t;
				
				s = (float)(j + 1) / slices;
				t = (float)(i + 1) / stacks;
				texture_s[currentTextureIndex] = s;
				texture_t[currentTextureIndex++] = 1.0 - t;
			}
		}
	}
	
	calculate_normal();
}

void Model::draw() {

	glColor4f(r, g, b, a);

	glPushMatrix();
		if(textureId != 0) 
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, textureId);
			if(useModulate)
			{
				glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
			}
			else
			{
				glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
			}
		}

		//Construct the object model here using triangles read from OFF file
		glBegin(GL_TRIANGLES);
		for(int tindx = 0; tindx < ntri; tindx++)
		{
			glNormal3f(normal_x[tindx], normal_y[tindx], normal_z[tindx]);
			
			if(textureId != 0) 
			{
				int textureIndex = tindx * 3;
				glTexCoord2f(texture_s[textureIndex], texture_t[textureIndex]);
				glVertex3d(vertices_x[indices_1[tindx]], vertices_y[indices_1[tindx]], vertices_z[indices_1[tindx]]);

				glTexCoord2f(texture_s[textureIndex + 1], texture_t[textureIndex + 1]);
				glVertex3d(vertices_x[indices_2[tindx]], vertices_y[indices_2[tindx]], vertices_z[indices_2[tindx]]);

				glTexCoord2f(texture_s[textureIndex + 2], texture_t[textureIndex + 2]);
				glVertex3d(vertices_x[indices_3[tindx]], vertices_y[indices_3[tindx]], vertices_z[indices_3[tindx]]);
			}
			else
			{
				glVertex3d(vertices_x[indices_1[tindx]], vertices_y[indices_1[tindx]], vertices_z[indices_1[tindx]]);
				glVertex3d(vertices_x[indices_2[tindx]], vertices_y[indices_2[tindx]], vertices_z[indices_2[tindx]]);
				glVertex3d(vertices_x[indices_3[tindx]], vertices_y[indices_3[tindx]], vertices_z[indices_3[tindx]]);
			}
		}
		glEnd();

		if(textureId != 0) 
		{
			glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
			glDisable(GL_TEXTURE_2D);
		}
	glPopMatrix();
}

void Model::setColor(float r, float g, float b, float a) 
{
	this->r = r;
	this->g = g;
	this->b = b;
	this->a = a;
}

void Model::loadTextureFile(const char* fname, bool useRepetition, bool useModulate)
{
	this->useModulate = useModulate;

	glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    const char* imageExtension = (const char*)(fname + strlen(fname) - 3);
    if(strncmp(imageExtension, "bmp", 3) == 0)
    {
		loadBMP(fname);
	}
	else if(strncmp(imageExtension, "tga", 3) == 0)
	{
		loadTGA(fname);
	}

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    if(useRepetition)
    {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
}

void Model::loadMeshFile(const char* fname) {
	
	ifstream fp_in;
	int num, ne;
	int nvrt;
	int ntc;

	fp_in.open(fname, ios::in);
	if(!fp_in.is_open())
	{
		cout << fname << " Error opening mesh file" << endl;
		exit(1);
	}

	// ignore first line
	fp_in.ignore(INT_MAX, '\n');	
	
	// read number of vertices, polygons, edges			
	fp_in >> nvrt >> ntri >> ne;			    

	// create arrays
	vertices_x = new float[nvrt];                    
	vertices_y = new float[nvrt];
	vertices_z = new float[nvrt];

	indices_1 = new int[ntri];
	indices_2 = new int[ntri];
	indices_3 = new int[ntri];

	// read vertex list 
	for(int i=0; i < nvrt; i++)        
	{
		fp_in >> vertices_x[i] >> vertices_y[i] >> vertices_z[i];
	}

	// read polygon list
	for(int i=0; i < ntri; i++){
		fp_in >> num >> indices_1[i] >> indices_2[i] >> indices_3[i];
		if(num != 3) {
			// not a triangle!!
			cout << "ERROR: Polygon with index " << i  << " is not a triangle." << endl;  
			exit(1);
		}
	}

	fp_in >> ntc;
	if(ntc > 0) 
	{
		texture_s = new float[ntc];
		texture_t = new float[ntc];
		for(int i = 0; i < ntc; i++)  
		{
			fp_in >> texture_s[i] >> texture_t[i];
		}
	}

	fp_in.close();
	cout << " File successfully read." << endl;
}

void Model::calculate_normal() {

	normal_x = new float[ntri];                        
	normal_y = new float[ntri];
	normal_z = new float[ntri];

	for(int tindx = 0; tindx < ntri; tindx++) 
	{
		float x1 = vertices_x[indices_1[tindx]], x2 = vertices_x[indices_2[tindx]], x3 = vertices_x[indices_3[tindx]];
		float y1 = vertices_y[indices_1[tindx]], y2 = vertices_y[indices_2[tindx]], y3 = vertices_y[indices_3[tindx]];
		float z1 = vertices_z[indices_1[tindx]], z2 = vertices_z[indices_2[tindx]], z3 = vertices_z[indices_3[tindx]];
		float nx, ny, nz;

		nx = y1*(z2-z3) + y2*(z3-z1) + y3*(z1-z2);
		ny = z1*(x2-x3) + z2*(x3-x1) + z3*(x1-x2);
		nz = x1*(y2-y3) + x2*(y3-y1) + x3*(y1-y2);

		normal_x[tindx] = nx;
		normal_y[tindx] = ny;
		normal_z[tindx] = nz;
	}
}

/*
 ***********************************
 ******* Particle definition *******
 ***********************************
 */
class Particle
{
	public:
		Particle(float size);
		~Particle();

		int lifetime = 0;

		void move();
		void draw();
		void fade();

		float x = 0;
		float y = 0;
		float z = 0;

		float r = 1.0f;
		float g = 1.0f;
		float b = 1.0f;

		float speedx = 0;
		float speedy = 0;
		float speedz = 0;

	private:
		float diameter;
};

Particle::Particle(float size)
{
	diameter = size;
}

Particle::~Particle()
{
}

void Particle::move()
{
	x += speedx;
	y += speedy;
	z += speedz;
}

void Particle::draw()
{
	glColor3f(r, g, b);

	glPushMatrix();
		glTranslatef(x, y, z);
		glutSolidCube(diameter);
	glPopMatrix();
}

void Particle::fade()
{
	float newColor = r * 0.9;
	r = newColor < 0.8 ? 0.8 : newColor;
	
	newColor = g * 0.9;
	g = newColor < 0.8 ? 0.8 : newColor;
	
	newColor = b * 0.9;
	b = newColor < 0.8 ? 0.8 : newColor;
}

/*
 ***************************************************
 ******* ParticleSystem definition *******
 ***************************************************
 */

class ParticleSystem
{
	public:
		ParticleSystem();
		~ParticleSystem();

		void emit();

	private:
		const static int PARTICAL_COUNT = 1200;
		Particle* particles[PARTICAL_COUNT] = {0};
		float radius;
		float height;
		float angle;

		bool needGenereteNewParticle = true;

		void generateParticles();
};

ParticleSystem::ParticleSystem()
{
	radius = 0.08f;
	height = 2.5f;
	angle = 15.0f;

	for(int i = 0; i < PARTICAL_COUNT; ++i)
	{
		particles[i] = new Particle(0.003);
	}

	generateParticles();
}

ParticleSystem::~ParticleSystem()
{
	for(int i = 0; i < PARTICAL_COUNT; ++i)
	{
		if(particles[i] != NULL)
		{
			delete particles[i];
		}
	}
}

void ParticleSystem::emit()
{
	for(int i = 0; i < PARTICAL_COUNT; ++i)
	{
		if(particles[i]->lifetime > 0)
		{
			particles[i]->move();
			particles[i]->draw();
			particles[i]->fade();
			particles[i]->lifetime -= 1;
		}
		else
		{
			needGenereteNewParticle = true;
		}
	}

	generateParticles();
}

void ParticleSystem::generateParticles()
{
	if(!needGenereteNewParticle)
	{
		return;
	}
	
	float unitRadian = M_PI / 180.0;
	for(int i = 0; i < PARTICAL_COUNT; ++i)
	{
		if(particles[i]->lifetime == 0)
		{
			particles[i]->lifetime = (int)(10.0 * rand() / RAND_MAX);

			float randonAngle = angle * (float)rand() / RAND_MAX;
			float speed = height / 10.0 * (float)rand() / RAND_MAX;
			float x = cos(unitRadian * randonAngle) * speed;
			float y = sin(unitRadian * randonAngle) * speed;

			particles[i]->speedx = 0;
			particles[i]->speedy = y;
			particles[i]->speedz = x;

			particles[i]->x = 0;
			particles[i]->y = 0;
			particles[i]->z = 0;
		}
	}
	
	needGenereteNewParticle = false;
}

/*
 *********************************
 ******* Musuem definition *******
 *********************************
 */
class Museum {
	
	const static int INDEX_MAIN_FLOOR = 0;
	const static int INDEX_MAIN_CEIL = 1;
	const static int INDEX_MAIN_WALL = 2;
	const static int INDEX_MAIN_TOP = 3;
	const static int INDEX_LEFT_WING = 4;
	const static int INDEX_RIGHT_WING = 5;
	const static int INDEX_LEFT_WING_CEIL = 6;
	const static int INDEX_RIGHT_WING_CEIL = 7;
	const static int INDEX_LEFT_GATE = 8;
	const static int INDEX_RIGHT_GATE = 9;
	
	const static int INDEX_WINDOW_1 = 10;
	const static int INDEX_WINDOW_2 = 11;
	const static int INDEX_WINDOW_3 = 12;
	const static int INDEX_WINDOW_4 = 13;
	const static int INDEX_WINDOW_5 = 14;
	const static int INDEX_WINDOW_6 = 15;
	const static int INDEX_WINDOW_7 = 16;
	const static int INDEX_WINDOW_8 = 17;
	const static int INDEX_WINDOW_9 = 18;
	const static int INDEX_WINDOW_10 = 19;
	const static int INDEX_WINDOW_11 = 20;
	const static int INDEX_WINDOW_12 = 21;
	const static int INDEX_WINDOW_13 = 22;
	
	const static int INDEX_EXHIBIT_BASE_1 = 23;
	const static int INDEX_EXHIBIT_BASE_2 = 24;
	const static int INDEX_EXHIBIT_BASE_3 = 25;
	
	const static int INDEX_TITLE = 26;
	
	const static int INDEX_GATE_LEFT_HANDLE = 27;
	const static int INDEX_GATE_RIGHT_HANDLE = 28;
	
	const static int INDEX_PLANET_BIG = 29;
	const static int INDEX_PLANET_MIDDLE = 30;
	const static int INDEX_PLANET_SMALL = 31;
	const static int INDEX_PLANET_COMET = 32;

	const static int INDEX_INTERIOR_WALL = 33;

	const static int INDEX_DUMMY = 34;
	
	const static int MESH_COUNT = INDEX_DUMMY;
	
	public:
		Museum();
		~Museum();

		void drawExterior();
		void drawInterior();
		void draw();
		void setGateRotateAngle(GLfloat angle);

	private:
		GLfloat gateRotateAngle = 0;
		Model* meshes[MESH_COUNT] = {0};
		GLuint titleTextureId = 0;
		ParticleSystem* comet;
		int emitCountdown = FRAME_RATE / 2;

		void loadExteriorMeshes();
		void loadInteriorMeshes();

		void loadExteriorTextures();
		void loadInteriorTextures();

		void drawPlanets();
		void drawLighthouse();
		void drawEmitBall();
		void drawFloor();
		
		void initialise();
		void drawEmitBallCoreCodes(bool drawShadow);
};

Museum::Museum() 
{
	initialise();
}

Museum::~Museum() 
{
	for(int i = 0; i < MESH_COUNT; ++i) 
	{
		if(meshes[i] != NULL) 
		{
			delete meshes[i];
		}
	}
}

void Museum::initialise()
{
	loadExteriorMeshes();
	loadInteriorMeshes();
	
	loadExteriorTextures();
	loadInteriorTextures();

	float grey[4] = {0.2, 0.2, 0.2, 1.0};
    float white[4]  = {1.0, 1.0, 1.0, 1.0};
    //float black[4]  = {0.0, 0.0, 0.0, 1.0};

	// Used for inside of museum
	glLightfv(GL_LIGHT1, GL_AMBIENT, grey);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, white);
    glLightfv(GL_LIGHT1, GL_SPECULAR, white);

	// used for solar system
    glLightfv(GL_LIGHT2, GL_AMBIENT, grey);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, white);
    glLightfv(GL_LIGHT2, GL_SPECULAR, white);

	// used for light house
    glLightfv(GL_LIGHT3, GL_AMBIENT, grey);
    glLightfv(GL_LIGHT3, GL_DIFFUSE, white);
    //glLightfv(GL_LIGHT3, GL_SPECULAR, white);

    glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, 10.0);
    glLightf(GL_LIGHT3, GL_SPOT_EXPONENT, 30);
    
	// Used for emiting ball
	glLightfv(GL_LIGHT4, GL_AMBIENT, grey);
	glLightfv(GL_LIGHT4, GL_DIFFUSE, white);
    glLightfv(GL_LIGHT4, GL_SPECULAR, white);

	//glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black);
	//glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 100.0);

	comet = new ParticleSystem();
}

void Museum::setGateRotateAngle(GLfloat angle)
{
	gateRotateAngle = angle;
}

void Museum::loadExteriorMeshes() 
{
	meshes[INDEX_MAIN_CEIL] = new Model("./Meshes/museum_exterior_main_ceil.off");
    meshes[INDEX_MAIN_CEIL]->setColor(0.5, 0.5, 0.5);
    
    meshes[INDEX_MAIN_WALL] = new Model("./Meshes/museum_exterior_main_wall.off");
    meshes[INDEX_MAIN_WALL]->setColor(0.5, 0.5, 0.5);
 
    meshes[INDEX_MAIN_TOP] = new Model("./Meshes/museum_exterior_main_top.off");
    meshes[INDEX_MAIN_TOP]->setColor(0.4, 0.2, 0.0);

    meshes[INDEX_LEFT_WING] = new Model("./Meshes/museum_exterior_wing.off");
    meshes[INDEX_LEFT_WING]->setColor(0.4, 0.2, 0.0);

    meshes[INDEX_RIGHT_WING] = new Model("./Meshes/museum_exterior_wing.off");
    meshes[INDEX_RIGHT_WING]->setColor(0.4, 0.2, 0.0);
 
    meshes[INDEX_LEFT_WING_CEIL] = new Model("./Meshes/museum_exterior_wing_ceil.off");
    meshes[INDEX_LEFT_WING_CEIL]->setColor(0.4, 0.4, 0.4);
 
    meshes[INDEX_RIGHT_WING_CEIL] = new Model("./Meshes/museum_exterior_wing_ceil.off");
    meshes[INDEX_RIGHT_WING_CEIL]->setColor(0.4, 0.4, 0.4);

    meshes[INDEX_LEFT_GATE] = new Model("./Meshes/museum_gate.off");
    meshes[INDEX_LEFT_GATE]->setColor(0.2, 0.2, 0.2);

    meshes[INDEX_RIGHT_GATE] = new Model("./Meshes/museum_gate.off");
    meshes[INDEX_RIGHT_GATE]->setColor(0.2, 0.2, 0.2);
 
    for(int i = INDEX_WINDOW_1; i < INDEX_WINDOW_1 + 13; i++) 
    {
		meshes[i] = new Model("./Meshes/museum_window.off");
    }

    meshes[INDEX_TITLE] = new Model("./Meshes/museum_title.off");

    meshes[INDEX_GATE_LEFT_HANDLE] = new Model("./Meshes/museum_gate_handle.off");
    meshes[INDEX_GATE_LEFT_HANDLE]->setColor(0.6, 0.6, 0.6);
    meshes[INDEX_GATE_RIGHT_HANDLE] = new Model("./Meshes/museum_gate_handle.off");
    meshes[INDEX_GATE_RIGHT_HANDLE]->setColor(0.6, 0.6, 0.6);
}

void Museum::loadInteriorMeshes() 
{
    meshes[INDEX_INTERIOR_WALL] = new Model("./Meshes/museum_interior_main_wall.off");
    meshes[INDEX_INTERIOR_WALL]->setColor(0.5, 0.5, 0.5);
 
    for(int i = INDEX_EXHIBIT_BASE_1; i < INDEX_EXHIBIT_BASE_1 + 3; i++)
    {
		meshes[i] = new Model("./Meshes/museum_interior_exhibit_base.off");
		meshes[i]->setColor(0.7, 0.7, 0.7);
    }

    Model *newMesh = new Model();
    newMesh->generateSphere(1.2, 100, 100);
    meshes[INDEX_PLANET_BIG] = newMesh;
    
    newMesh = new Model();
	newMesh->generateSphere(0.8, 100, 100);
    meshes[INDEX_PLANET_MIDDLE] = newMesh;

    newMesh = new Model();
    newMesh->generateSphere(0.3, 30, 30);
    meshes[INDEX_PLANET_SMALL] = newMesh;

    newMesh = new Model();
    newMesh->generateSphere(0.01, 5, 5);
    meshes[INDEX_PLANET_COMET] = newMesh;
}

void Museum::loadExteriorTextures()
{
	meshes[INDEX_TITLE]->loadTextureFile("./Textures/museum_title.bmp", false, true);

	for(int i = INDEX_WINDOW_1; i < INDEX_WINDOW_13 + 1; i++)
	{
		meshes[i]->loadTextureFile("./Textures/window.bmp", false, true);
	}
}

void Museum::loadInteriorTextures()
{
	meshes[INDEX_PLANET_BIG]->loadTextureFile("./Textures/sun.tga", false, false);
	meshes[INDEX_PLANET_MIDDLE]->loadTextureFile("./Textures/earth.bmp", false, true);
	meshes[INDEX_PLANET_SMALL]->loadTextureFile("./Textures/moon.bmp", false, true);
}

void Museum::draw()
{
	drawExterior();
	drawInterior();
}

void Museum::drawExterior() 
{
	glPushMatrix();
	    glTranslatef(0, 0, 15);
	    meshes[INDEX_MAIN_CEIL]->draw();
	    meshes[INDEX_MAIN_WALL]->draw();
	    meshes[INDEX_MAIN_TOP]->draw();
	glPopMatrix();

	glPushMatrix();
	    glTranslatef(-37.5, 0, 15);
	    meshes[INDEX_LEFT_WING]->draw();
	glPopMatrix();

	glPushMatrix();
	    glTranslatef(37.5, 0, 15);
	    meshes[INDEX_RIGHT_WING]->draw();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(-39, 0, 15);
	    meshes[INDEX_LEFT_WING_CEIL]->draw();
	glPopMatrix();

	glPushMatrix();
	    glTranslatef(39, 0, 15);
	    glRotatef(180, 0, 1, 0);
	    meshes[INDEX_RIGHT_WING_CEIL]->draw();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(-3.05, 0, 30.06);
		glTranslatef(-3, 0, 0);
		glRotatef(gateRotateAngle, 0, 1, 0);
		glTranslatef(3, 0, 0);
	    meshes[INDEX_LEFT_GATE]->draw();
	    
	    glTranslatef(2.75, 2.5, 0.04);
	    meshes[INDEX_GATE_LEFT_HANDLE]->draw();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(3.05, 0, 30.06);
		glTranslatef(3, 0, 0);
		glRotatef(-gateRotateAngle, 0, 1, 0);
		glTranslatef(-3, 0, 0);
	    meshes[INDEX_RIGHT_GATE]->draw();
	    
	    glTranslatef(-2.75, 2.5, 0.04);
	    meshes[INDEX_GATE_RIGHT_HANDLE]->draw();
	glPopMatrix();
	
	for(int i = INDEX_WINDOW_1; i < INDEX_WINDOW_1 + 13; i++) {
		glPushMatrix();
			if (i < INDEX_WINDOW_1 + 3) {
				glTranslatef(-52 + (i - INDEX_WINDOW_1) * 14, 12, 30.06);
			} else if (i < INDEX_WINDOW_1 + 6) {
				glTranslatef(-52 + (i - 3 - INDEX_WINDOW_1) * 14, 1, 30.06);
			} else if (i < INDEX_WINDOW_1 + 9) {
				glTranslatef(52 - (i - 6 - INDEX_WINDOW_1) * 14, 12, 30.06);
			} else if (i < INDEX_WINDOW_1 + 12) {
				glTranslatef(52 - (i - 9 - INDEX_WINDOW_1) * 14, 1, 30.06);
			} else {
				glTranslatef(0, 32, 30.06);
			}
			meshes[i]->draw();
		glPopMatrix();
    }

    glPushMatrix();
		glTranslatef(0, 15, 30.06);
	    meshes[INDEX_TITLE]->draw();
	glPopMatrix();
}

void Museum::drawPlanets() 
{
	glDisable(GL_LIGHT1);
	
	float light2_pos[4] = {0.0f, 2.3f, 5.0f, 1.0f};
    glLightfv(GL_LIGHT2, GL_POSITION, light2_pos);
    
	glEnable(GL_LIGHT2);

	static float autorotatingAngleSun = 0;
    static float autorotatingAngleEarth = 0;
    static float autorotatingAngleMoon = 0;

    autorotatingAngleSun += 0.2;
    autorotatingAngleEarth += 1;
    autorotatingAngleMoon += 4;

    static float rotatingAngleEarth = 0;
    static float rotatingAngleMoon = 0;
    static float rotatingAngleComet = 0;

    rotatingAngleEarth += 0.3;
    rotatingAngleMoon += 0.1;
    rotatingAngleComet += 0.5;

	glPushMatrix();
	glTranslatef(0, 1.001, 5);
	glColor3f(255.0 / 255, 165.0 / 255, 0);
	glNormal3f(0.0, 1.0, 0.0);
	glBegin(GL_QUADS);
	for(int i = -80; i < 80; i++)
	{
		for(int j = -50;  j < 50; j++)
		{
			glVertex3f(0.05 * i, 0.01, j * 0.05);
			glVertex3f(0.05 * i, 0.01, j * 0.05 + 0.05);
			glVertex3f(0.05 * i + 0.05, 0.01, j * 0.05 + 0.05);
			glVertex3f(0.05 * i + 0.05, 0.01, j * 0.05);
		}
	}
	glEnd();
	glPopMatrix();

	// draw sun
	glPushMatrix();
		glTranslatef(0, 2.3, 5);
		glRotatef(autorotatingAngleSun, 0, 1, 0);
		meshes[INDEX_PLANET_BIG]->setColor(1, 1, 0);
		meshes[INDEX_PLANET_BIG]->draw();

		// draw a comet
		glDisable(GL_LIGHTING);
		glPushMatrix();
			glRotatef(-20, 0, 0, 1);
			glRotatef(rotatingAngleComet, 0, 1, 0);
			glTranslatef(4, 0, 0);
			meshes[INDEX_PLANET_COMET]->draw();
			comet->emit();
		glPopMatrix();
		glEnable(GL_LIGHTING);

		// draw earth
		glRotatef(rotatingAngleEarth, 0, 1, 0);
		glTranslatef(4, 0, 0);
		glRotatef(autorotatingAngleEarth, 0, 1, 0);
		meshes[INDEX_PLANET_MIDDLE]->draw();

		// draw moon
		glRotatef(rotatingAngleMoon, 0, 1, 0);
		glTranslatef(2, 0, 0);
		glRotatef(autorotatingAngleMoon, 0, 1, 0);
		meshes[INDEX_PLANET_SMALL]->draw();

	glPopMatrix();
	
	glDisable(GL_LIGHT2);
	glEnable(GL_LIGHT1);
}

void Museum::drawLighthouse() 
{	
	static float lightHouseRotatingAngle = 0;

	GLUquadric *q = gluNewQuadric();
	gluQuadricDrawStyle(q, GLU_FILL);

	glPushMatrix();
		glTranslatef(-11.25, 1, 6);

		glPushMatrix();
			glColor3f(1.0, 1.0, 1.0);
			glRotatef(-90, 1, 0, 0);
			gluCylinder(q, 0.6, 0.45, 0.4, 20, 5);
		glPopMatrix();
		
		glPushMatrix();
			glColor3f(205.0 / 255, 92.0 / 255, 92.0 / 255);
			glTranslatef(0, 0.4, 0);
			glRotatef(-90, 1, 0, 0);
			gluCylinder(q, 0.45, 0.35, 1, 20, 5);
		glPopMatrix();

		glPushMatrix();
			glColor3f(1.0, 1.0, 1.0);
			glTranslatef(0, 1.4, 0);
			glRotatef(-90, 1, 0, 0);
			gluCylinder(q, 0.35, 0.3, 1, 20, 5);
		glPopMatrix();
		
		glPushMatrix();
			glColor3f(205.0 / 255, 92.0 / 255, 92.0 / 255);
			glTranslatef(0, 2.4, 0);
			glRotatef(-90, 1, 0, 0);
			gluCylinder(q, 0.5, 0.5, 0.2, 20, 5);
		glPopMatrix();
		
		glPushMatrix();
			glColor3f(1.0, 1.0, 1.0);
			glTranslatef(0, 2.6, 0);
			glRotatef(-90, 1, 0, 0);
			gluCylinder(q, 0.3, 0.3, 0.4, 20, 5);
		glPopMatrix();
		
		glPushMatrix();
			glColor3f(205.0 / 255, 92.0 / 255, 92.0 / 255);
			glTranslatef(0, 3, 0);
			glutSolidSphere(0.28, 20, 20);
		glPopMatrix();
		
		glPushMatrix();
			glColor3f(205.0 / 255, 92.0 / 255, 92.0 / 255);
			glTranslatef(0, 3.25, 0);
			glRotatef(-90, 1, 0, 0);
			gluCylinder(q, 0.15, 0.15, 0.1, 20, 5);
		glPopMatrix();
		
		glPushMatrix();
			glColor3f(0.3, 0.3, 0.3);
			glRotatef(lightHouseRotatingAngle, 0, 1, 0);
			glTranslatef(0, 2.5, 0.505);
			gluDisk(q, 0.0, 0.08, 20, 3);
		glPopMatrix();
		
		glPushMatrix();
			glColor3f(1, 1, 1);
			glRotatef(lightHouseRotatingAngle, 0, 1, 0);
			glTranslatef(0, 2.5, 0.506);
			gluDisk(q, 0.0, 0.05, 20, 3);
			
			glEnable(GL_LIGHT3);
	
			float spot_pos[]={0, 0, 0.01, 1.0};
			glLightfv(GL_LIGHT3, GL_POSITION, spot_pos);

			float spotdir[]={0, -1, 0.5};
			glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, spotdir);
 
		glPopMatrix();
		
		glColor3f(135.0 / 255, 206.0 / 255, 235.0 / 255);
		glNormal3f(0.0, 1.0, 0.0);
		glBegin(GL_QUADS);
		for(int i = -80; i < 80; i++)
		{
			for(int j = -50;  j < 50; j++)
			{
				glVertex3f(0.05 * i, 0.01, j * 0.05);
				glVertex3f(0.05 * i, 0.01, j * 0.05 + 0.05);
				glVertex3f(0.05 * i + 0.05, 0.01, j * 0.05 + 0.05);
				glVertex3f(0.05 * i + 0.05, 0.01, j * 0.05);
			}
		}
		glEnd();

	glPopMatrix();
	
	lightHouseRotatingAngle += 3;

	glDisable(GL_LIGHT3);
}

void Museum::drawEmitBallCoreCodes(bool drawShadow)
{
	static float speedGravity = 0;
	static float speed = 0;
	
	static float accumulatedForwadDistance = 0;
	static float accumulatedDownDistance = 0;

	static int emitCountdown = 10;

	glPushMatrix();
		glPushMatrix();
			if(drawShadow)
			{
				glColor3f(0.2, 0.2, 0.2);
			}
			else
			{
				glColor3f(139.0/255, 58.0 / 255, 58.0 / 255);
			}
			glTranslatef(-2.6, 0.7, 0);
			glRotatef(30, 0, 0, 1);
			glScalef(3, 0.1, 0.5);
			glutSolidCube(1);
		glPopMatrix();

		glPushMatrix();
			if(drawShadow)
			{
				glColor3f(0.2, 0.2, 0.2);
			}
			else
			{
				glColor3f(139.0/255, 101.0 / 255, 255.0 / 255);
			}
			glTranslatef(-3.7, 0.25, 0);
			glRotatef(30, 0, 0, 1);
			glScalef(1.5, 1, 1);
			glutSolidCube(0.25);
		glPopMatrix();

		if(emitCountdown <= 10 && emitCountdown > 0)
		{
			glPushMatrix();
				if(drawShadow)
				{
					glColor3f(0.2, 0.2, 0.2);
				}
				else
				{
					glColor3f(0.0/255, 111.0 / 255, 255.0 / 255);
				}
				glTranslatef(-3.4, 0.35, 0);
				glRotatef(30, 0, 0, 1);
				glScalef(0, 0, 0);
				glutSolidCube(0.1);
			glPopMatrix();

			glPushMatrix();
				if(drawShadow)
				{
					glColor3f(0.2, 0.2, 0.2);
				}
				else
				{
					glColor3f(255.0/255, 165.0 / 255, 0.0 / 255);
				}
				glTranslatef(-3.4, 0.4, 0);
				glutSolidSphere(0.1, 30, 30);
			glPopMatrix();
		}
		else if(emitCountdown == 0)
		{
			glPushMatrix();
				if(drawShadow)
				{
					glColor3f(0.2, 0.2, 0.2);
				}
				else
				{
					glColor3f(0.0/255, 111.0 / 255, 255.0 / 255);
				}
				
				glTranslatef(-3.4, 0.35, 0);
				glRotatef(30, 0, 0, 1);
				glScalef(1.5, 1, 1);
				glutSolidCube(0.1);
			glPopMatrix();

			glPushMatrix();
				if(drawShadow)
				{
					glColor3f(0.2, 0.2, 0.2);
				}
				else
				{
					glColor3f(255.0/255, 165.0 / 255, 0.0 / 255);
				}
				glTranslatef(-3.3, 0.45, 0);
				glutSolidSphere(0.1, 30, 30);
			glPopMatrix();

			speed = 4.0f;

			accumulatedForwadDistance = 0;
			accumulatedDownDistance = 0;
			speedGravity = 0;
		}
		else if(emitCountdown < 0)
		{
			glPushMatrix();
				if(drawShadow)
				{
					glColor3f(0.2, 0.2, 0.2);
				}
				else
				{
					glColor3f(0.0/255, 111.0 / 255, 255.0 / 255);
				}
				glTranslatef(-3.4, 0.35, 0);
				glRotatef(30, 0, 0, 1);
				glScalef(1.5, 1, 1);
				glutSolidCube(0.1);
			glPopMatrix();

			float time = 1.0 / FRAME_RATE;

			if(accumulatedForwadDistance > 3.0)
			{
				float oldGravitySpeed = speedGravity;
				speedGravity += 9.8 * time;
				float sg = oldGravitySpeed * time + 0.5 * 9.8 * time * time;
				accumulatedDownDistance += sg;

				accumulatedForwadDistance += speed * time;
				float sx = accumulatedForwadDistance * cos(30 * M_PI / 180.0f);
				float sy = accumulatedForwadDistance * sin(30 * M_PI / 180.0f);

				float deltaY = sy - accumulatedDownDistance;

				if(deltaY <= 0.1)
				{
					emitCountdown = 25;
				}
				else
				{
					glPushMatrix();
						if(drawShadow)
						{
							glColor3f(0.2, 0.2, 0.2);
						}
						else
						{
							glColor3f(255.0/255, 165.0 / 255, 0.0 / 255);
						}
						glTranslatef(-3.3 + sx, 0.45 + deltaY, 0);
						glutSolidSphere(0.1, 30, 30);
					glPopMatrix();	
				}
			}
			else
			{
				accumulatedForwadDistance += speed * time;

				float sx = accumulatedForwadDistance * cos(30 * M_PI / 180.0f);
				float sy = accumulatedForwadDistance * sin(30 * M_PI / 180.0f);

				glPushMatrix();
					if(drawShadow)
					{
						glColor3f(0.2, 0.2, 0.2);
					}
					else
					{
						glColor3f(255.0/255, 165.0 / 255, 0.0 / 255);
					}
					glTranslatef(-3.3 + sx, 0.45 + sy, 0);
					glutSolidSphere(0.1, 30, 30);
				glPopMatrix();
			}
		}
		else
		{
			glPushMatrix();
				if(drawShadow)
				{
					glColor3f(0.2, 0.2, 0.2);
				}
				else
				{
					glColor3f(0.0/255, 111.0 / 255, 255.0 / 255);
				}
				glTranslatef(-3.4, 0.35, 0);
				glRotatef(30, 0, 0, 1);
				glScalef(1.5, 1, 1);
				glutSolidCube(0.1);
			glPopMatrix();
			
			float sx = accumulatedForwadDistance * cos(30 * M_PI / 180.0f);

			glPushMatrix();
				if(drawShadow)
				{
					glColor3f(0.2, 0.2, 0.2);
				}
				else
				{
					glColor3f(255.0/255, 165.0 / 255, 0.0 / 255);
				}
				glTranslatef(-3.3 + sx, 0.1, 0);
				glutSolidSphere(0.1, 30, 30);
			glPopMatrix();
		}
		emitCountdown--;

	glPopMatrix();
}

void Museum::drawEmitBall() 
{
	float lightx = -5;
	float lighty = 10;
	float lightz = 0;
	
	float light4_pos[4] = {lightx, lighty, lightz, 1.0f};
    glLightfv(GL_LIGHT4, GL_POSITION, light4_pos);

	float shadowMat[16] = {lighty, 0, 0, 0,  -lightx, 0, -lightz, -1,  0, 0, lighty, 0,  0, 0, 0, lighty};
	
	glPushMatrix();
		glTranslatef(11.25, 1.0001, 6);
		glColor3f(152.0 / 255, 251.0 / 255, 152.0 / 255);
		glNormal3f(0.0, 1.0, 0.0);

		glBegin(GL_QUADS);
		for(int i = -80; i < 80; i++)
		{
			for(int j = -50;  j < 50; j++)
			{
				glVertex3f(0.05 * i, 0, j * 0.05);
				glVertex3f(0.05 * i, 0, j * 0.05 + 0.05);
				glVertex3f(0.05 * i + 0.05, 0, j * 0.05 + 0.05);
				glVertex3f(0.05 * i + 0.05, 0, j * 0.05);
			}
		}
		glEnd();
	glPopMatrix();

	glDisable(GL_LIGHTING);
	glPushMatrix();
		glTranslatef(11.25, 1.002, 6);
		glMultMatrixf(shadowMat);
		drawEmitBallCoreCodes(true);
	glPopMatrix();
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT4);
	glPushMatrix();
		glTranslatef(11.25, 1, 6);
		drawEmitBallCoreCodes(false);
	glPopMatrix();

	glDisable(GL_LIGHT4);
}

void Museum::drawFloor()
{
    bool flag = false;

    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    
    for(int i = -15; i < 15; i += 2)
    {
        for(int j = 0;  j < 30; j += 2)
        {
            if(flag)
            {
				glColor3f(0.3, 0.3, 0);
			}
            else
            {
				glColor3f(0, 0.3, 0.3);
			}
	
            glVertex3f(i, 0.001, j);
            glVertex3f(i, 0.001, j+2);
            glVertex3f(i+2, 0.001, j+2);
            glVertex3f(i+2, 0.001, j);
            flag = !flag;
        }
    }
    glEnd();
}

void Museum::drawInterior() 
{
	glDisable(GL_LIGHT0);
	
	float light1_pos[4] = {0.0f, 30.0f, 30.0f, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    
	glEnable(GL_LIGHT1);

	drawFloor();

	glPushMatrix();
	    glTranslatef(0, 0, 15);
		meshes[INDEX_INTERIOR_WALL]->draw();
	glPopMatrix();
	
	glPushMatrix();
		glTranslatef(0, 0, 5);
		meshes[INDEX_EXHIBIT_BASE_1]->draw();
	glPopMatrix();

    glPushMatrix();
		glTranslatef(-11.25, 0, 6);
		meshes[INDEX_EXHIBIT_BASE_2]->draw();
	glPopMatrix();

    glPushMatrix();
		glTranslatef(11.25, 0, 6);
		meshes[INDEX_EXHIBIT_BASE_2]->draw();
	glPopMatrix();

	drawPlanets();
	drawLighthouse();
	drawEmitBall();

	glDisable(GL_LIGHT1);
	glEnable(GL_LIGHT0);
}

//--Globals ---------------------------------------------------------------
const GLfloat LOOK_RANGE = 1600;
const float CAMERA_Y = 2.5;
float fov = 65;
float camera_positions[] = {0, CAMERA_Y, 120.0f};
float look_at_positions[] = {0, CAMERA_Y, -LOOK_RANGE};
float view_angle = 0.0f;
float model_rotate_angle = 0.0f;
GLfloat halfSideLenght = 1500;

Museum *museum = NULL;
GLuint floorTextureId = 0;
GLuint skyBoxTextureId[6] = {0};
GLfloat skyBoxOffset = -1000;

void drawSkyBox()
{
	glEnable(GL_TEXTURE_2D);

    glPushMatrix();
    glTranslatef(0, skyBoxOffset, 0);
    
    glBindTexture(GL_TEXTURE_2D, skyBoxTextureId[0]);
    glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex3f(halfSideLenght, 0, -halfSideLenght);
		glTexCoord2f(1, 0);
        glVertex3f(halfSideLenght, 0, halfSideLenght);
        glTexCoord2f(1, 1);
        glVertex3f(halfSideLenght, halfSideLenght * 2, halfSideLenght);
        glTexCoord2f(0, 1);
        glVertex3f(halfSideLenght, halfSideLenght* 2, -halfSideLenght);
    glEnd();
    
    glBindTexture(GL_TEXTURE_2D, skyBoxTextureId[1]);
    glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex3f(-halfSideLenght, 0, halfSideLenght);
		glTexCoord2f(1, 0);
        glVertex3f(-halfSideLenght, 0, -halfSideLenght);
        glTexCoord2f(1, 1);
        glVertex3f(-halfSideLenght, halfSideLenght * 2, -halfSideLenght);
        glTexCoord2f(0, 1);
        glVertex3f(-halfSideLenght, halfSideLenght* 2, halfSideLenght);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, skyBoxTextureId[2]);
    glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex3f(-halfSideLenght, halfSideLenght * 2, -halfSideLenght);
		glTexCoord2f(1, 0);
        glVertex3f(halfSideLenght, halfSideLenght * 2, -halfSideLenght);
        glTexCoord2f(1, 1);
        glVertex3f(halfSideLenght, halfSideLenght * 2, halfSideLenght);
        glTexCoord2f(0, 1);
        glVertex3f(-halfSideLenght, halfSideLenght * 2, halfSideLenght);
	glEnd();
	glPopMatrix();

	glBindTexture(GL_TEXTURE_2D, skyBoxTextureId[3]);
    glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex3f(-halfSideLenght, 0, halfSideLenght);
		glTexCoord2f(1, 0);
        glVertex3f(halfSideLenght, 0, halfSideLenght);
        glTexCoord2f(1, 1);
        glVertex3f(halfSideLenght, 0, -halfSideLenght);
        glTexCoord2f(0, 1);
        glVertex3f(-halfSideLenght, 0, -halfSideLenght);
    glEnd();
	
	glPushMatrix();
	glTranslatef(0, skyBoxOffset, 0);

	glBindTexture(GL_TEXTURE_2D, skyBoxTextureId[4]);
    glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex3f(-halfSideLenght, 0, -halfSideLenght);
		glTexCoord2f(1, 0);
        glVertex3f(halfSideLenght, 0, -halfSideLenght);
        glTexCoord2f(1, 1);
        glVertex3f(halfSideLenght, halfSideLenght * 2, -halfSideLenght);
        glTexCoord2f(0, 1);
        glVertex3f(-halfSideLenght, halfSideLenght * 2, -halfSideLenght);
	glEnd();
	
	glBindTexture(GL_TEXTURE_2D, skyBoxTextureId[5]);
    glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex3f(halfSideLenght, 0, halfSideLenght);
		glTexCoord2f(1, 0);
        glVertex3f(-halfSideLenght, 0, halfSideLenght);
        glTexCoord2f(1, 1);
        glVertex3f(-halfSideLenght, halfSideLenght * 2, halfSideLenght);
        glTexCoord2f(0, 1);
        glVertex3f(halfSideLenght, halfSideLenght * 2, halfSideLenght);
    glEnd();
    glPopMatrix();
    
    glDisable(GL_TEXTURE_2D);
}

void rotatePlanetTimerCallback(int value) 
{
	glutPostRedisplay();
    glutTimerFunc(1000.0 / FRAME_RATE, rotatePlanetTimerCallback, 0);
    museum->draw();
}

void display()  
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(camera_positions[0], camera_positions[1], camera_positions[2], 
		look_at_positions[0], look_at_positions[1], look_at_positions[2], 
		0, 1, 0);

	drawSkyBox();
	
	glRotatef(model_rotate_angle, 0, 1, 0);
	
	museum->draw();

	glutSwapBuffers();
}

void initialiseSkyBox() 
{
	glGenTextures(6, skyBoxTextureId);
	
	const char* imagePaths[6] = {"./Textures/SkyBox/right.tga", "./Textures/SkyBox/left.tga", "./Textures/SkyBox/up.tga", 
		"./Textures/SkyBox/down.tga", "./Textures/SkyBox/front.tga", "./Textures/SkyBox/back.tga"};

	for(int i = 0; i < 6; i++)
	{
		glBindTexture(GL_TEXTURE_2D, skyBoxTextureId[i]);
		loadTGA(imagePaths[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	}
}

void initialize()
{
    float grey[4] = {0.2, 0.2, 0.2, 1.0};
    float white[4]  = {1.0, 1.0, 1.0, 1.0};
	
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glEnable(GL_LIGHTING);
 	glEnable(GL_COLOR_MATERIAL);
 	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_SMOOTH);

	// Used in outside of museum
    glLightfv(GL_LIGHT0, GL_AMBIENT, grey);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white);

	float light0_pos[4] = {100.0f, 100.0f, 100.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
    glEnable(GL_LIGHT0);

	//float light1_pos[4] = {14, 30.0f, 0.0f, 1.0f};
    //glLightfv(GL_LIGHT1, GL_POSITION, light1_pos);
    
    // load all the meshes beforehand
    museum = new Museum();
    initialiseSkyBox();

	// The camera view volume
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, 16.0/9.0, 1, 1000);
}

//------------ Special key event callback ---------------------------------
// To enable the use of left and right arrow keys to rotate the scene
void special(int key, int x, int y)
{
    const float WALKING_STEP = 1;
    const float sightRange = halfSideLenght;

    switch(key) {
		case GLUT_KEY_LEFT:
			view_angle -= 1;
			look_at_positions[0] = camera_positions[0] + LOOK_RANGE * sin(view_angle * M_PI / 180.0);
			look_at_positions[2] = camera_positions[2] - LOOK_RANGE * cos(view_angle * M_PI / 180.0);
			break;
		case GLUT_KEY_RIGHT:
			view_angle += 1;
			look_at_positions[0] = camera_positions[0] + LOOK_RANGE * sin(view_angle * M_PI / 180.0);
			look_at_positions[2] = camera_positions[2] - LOOK_RANGE * cos(view_angle * M_PI / 180.0);
			break;
		case GLUT_KEY_UP:
			{
				float nextCameraX = camera_positions[0] + WALKING_STEP * sin(view_angle * M_PI / 180.0);
				float nextCameraZ = camera_positions[2] - WALKING_STEP * cos(view_angle * M_PI / 180.0);
				
				if(nextCameraZ > 0 && nextCameraZ < 30 && nextCameraX > -15 && nextCameraX < 15)
				{
					// In the museum internal
					if(nextCameraZ > 12 && nextCameraX > -13 && nextCameraX < 13)
					{
						camera_positions[0] = nextCameraX;
						camera_positions[2] = nextCameraZ;
					}
				}
				else if(nextCameraZ > -2 && nextCameraZ < 32 && nextCameraX > -62 && nextCameraX < 62)
				{
					if(nextCameraX > -6 && nextCameraX < 6)
					{
						camera_positions[0] = nextCameraX;
						camera_positions[2] = nextCameraZ;
					}
				}
				else if (nextCameraZ > 0 && nextCameraZ < 45 && nextCameraX > -62 && nextCameraX < 62)
				{
					float angle = 10 * (45 - nextCameraZ);
					angle = angle > 90 ? 90 : angle;
					museum->setGateRotateAngle(-angle);

					camera_positions[0] = nextCameraX;
					camera_positions[2] = nextCameraZ;
				}
				else if(nextCameraZ > -sightRange + 5 && nextCameraZ < sightRange - 5 && nextCameraX > -sightRange + 5 && nextCameraX < sightRange - 5)
				{
					camera_positions[0] = nextCameraX;
					camera_positions[2] = nextCameraZ;
				}
			}
			break;
		case GLUT_KEY_DOWN:
			{
				float nextCameraX = camera_positions[0] - WALKING_STEP * sin(view_angle * M_PI / 180.0);
				float nextCameraZ = camera_positions[2] + WALKING_STEP * cos(view_angle * M_PI / 180.0);

				if(nextCameraZ > 0 && nextCameraZ < 28 && nextCameraX > -15 && nextCameraX < 15)
				{
					// In the museum internal
					if(nextCameraZ < 28 && nextCameraZ > 12 && nextCameraX > -13 && nextCameraX < 13)
					{
						camera_positions[0] = nextCameraX;
						camera_positions[2] = nextCameraZ;
					}
				}
				else if(nextCameraZ > 28 && nextCameraZ < 30 && nextCameraX > -62 && nextCameraX < 62)
				{
					if(nextCameraX > -6 && nextCameraX < 6)
					{
						camera_positions[0] = nextCameraX;
						camera_positions[2] = nextCameraZ;
					}
				}
				else if (nextCameraZ > 36 && nextCameraZ < 46 && nextCameraX > -62 && nextCameraX < 62)
				{
					float angle = 10 * (nextCameraZ - 45);
					angle = angle > 0 ? 0 : angle;
					museum->setGateRotateAngle(angle);

					camera_positions[0] = nextCameraX;
					camera_positions[2] = nextCameraZ;
				}
				else if(nextCameraZ > -sightRange + 5 && nextCameraZ < sightRange - 5 && nextCameraX > -sightRange + 5 && nextCameraX < sightRange - 5)
				{
					camera_positions[0] = nextCameraX;
					camera_positions[2] = nextCameraZ;
				}
			}
			break;
		case GLUT_KEY_F1:
			camera_positions[1] += 1;
			break;
		case GLUT_KEY_F2:
			camera_positions[1] -= 1;
			break;
		case GLUT_KEY_F3:
			model_rotate_angle += 1;
			break;
		case GLUT_KEY_F4:
			model_rotate_angle -= 1;
			break;
		case GLUT_KEY_F5:
			look_at_positions[1] += 2;
			break;
		case GLUT_KEY_F6:
			look_at_positions[1] -= 2;
			break;
		default:
			break;
	}

	glutPostRedisplay();
}

void reshapeWindow(int width, int height)
{
	float ratio = (float)width / height;
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, ratio, 1, 5000);
}

//  ------- Main: Initialize glut window and register call backs -----------
int main(int argc, char** argv)
{	
	glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize (1200, 1200 * 9 / 16); 
    glutInitWindowPosition (10, 10);
    glutCreateWindow ("OpenGL Museum");
    glutReshapeFunc(reshapeWindow);
    
    initialize();

    glutDisplayFunc(display);
    glutSpecialFunc(special);
	glutTimerFunc(1000.0 / FRAME_RATE, rotatePlanetTimerCallback, 0);

    glutMainLoop();

	delete museum;

    return 0;
}
