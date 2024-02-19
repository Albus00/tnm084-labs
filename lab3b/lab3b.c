// Lab 3b
// Terrain generation

// Current contents:
// Terrain being generated on CPU (MakeTerrain). Simple terrain as example: Flat surface with a bump.
// Note the constants kTerrainSize and kPolySize for changing the number of vertices and polygon size!

// Things to do:
// Generate a terrain on CPU
// Generate a terrain on GPU, in the vertex shader.
// Generate textures for the surface (fragment shader).

// If you want to use Perlin noise, use the code from Lab 1.

#define MAIN
#include "MicroGlut.h"
#include "GL_utilities.h"
#include "VectorUtils4.h"
#include "LittleOBJLoader.h"
#include "LoadTGA.h"
#include <math.h>
#include "noise1234.h"
#include "simplexnoise1234.h"
#include "cellular.h"

mat4 projectionMatrix;
Model *floormodel;
GLuint grasstex;

// Reference to shader programs
GLuint phongShader, texShader;

#define kTerrainSize 512
#define kPolySize 0.2

// Terrain data. To be intialized in MakeTerrain or in the shader
vec3 vertices[kTerrainSize*kTerrainSize];
vec2 texCoords[kTerrainSize*kTerrainSize];
vec3 normals[kTerrainSize*kTerrainSize];
GLuint indices[(kTerrainSize-1)*(kTerrainSize-1)*3*2];

// These are considered unsafe, but with most C code, write with caution.
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

float fbm(vec2 st, int octaves){
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < octaves; ++i)
    {
        //printf("%f \n", noise2(0.4*st.x, 0.4*st.y));

        value += amplitude * noise2(0.4*st.x,0.4*st.y);

        st.x *= 2.0;
        st.y *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

void MakeTerrain()
{
	// TO DO: This is where your terrain generation goes if on CPU.
	for (int x = 0; x < kTerrainSize; x++)
	for (int z = 0; z < kTerrainSize; z++)
	{
		int ix = z * kTerrainSize + x;
		vec3 god_vector, v1, v2, v3, p1, p2, p3, normal;

		//#define bumpHeight 0.5
		//#define bumpWidth 2.0

		// squared distance to center
		//float h = ( (x - kTerrainSize/2)/bumpWidth * (x - kTerrainSize/2)/bumpWidth +  (z - kTerrainSize/2)/bumpWidth * (z - kTerrainSize/2)/bumpWidth );
		//float y = MAX(0, 3 - h) * bumpHeight;

		float rand = noise3(0.4*x,0,0.4*z)*1.2*kPolySize;
		rand += noise3(0.1*x,0,0.1*z)*2;

        rand += fbm(SetVec2((float)x,(float)z), 10);

		if (rand < 0) {
            rand = 0;
		}


		vertices[ix] = SetVec3(x * kPolySize, rand, z * kPolySize);
		texCoords[ix] = SetVec2(x, z);
        normals[ix] = SetVec3(0,1,0);

	}

	// Make indices
	// You don't need to change this.
	for (int x = 0; x < kTerrainSize-1; x++)
	for (int z = 0; z < kTerrainSize-1; z++)
	{
		// Quad count
		int q = (z*(kTerrainSize-1)) + (x);
		// Polyon count = q * 2
		// Indices
		indices[q*2*3] = x + z * kTerrainSize; // top left
		indices[q*2*3+1] = x+1 + z * kTerrainSize;
		indices[q*2*3+2] = x + (z+1) * kTerrainSize;
		indices[q*2*3+3] = x+1 + z * kTerrainSize;
		indices[q*2*3+4] = x+1 + (z+1) * kTerrainSize;
		indices[q*2*3+5] = x + (z+1) * kTerrainSize;
	}

	// Make normal vectors
	// TO DO: This is where you calculate normal vectors
	for (int x = 0; x < kTerrainSize; x++)
	for (int z = 0; z < kTerrainSize; z++)
	{
	    int ix = z * kTerrainSize + x;
	    vec3 god_vector, v1, v2, p1, p2, p3, p4, normal;

	    if (x == 0) {
            p1 = vertices[ix];
            p2 = vertices[ix+1];
        }
        else if (x == kTerrainSize -1) {
            p1 = vertices[ix-1];
            p2 = vertices[ix];
        }
        else {
            p1 = vertices[ix-1];
            p2 = vertices[ix+1];
        }

        if (z == 0) {
            p3 = vertices[ix];
            p4 = vertices[ix+kTerrainSize];
        }
        else if (z == kTerrainSize-1) {
            p3 = vertices[ix-kTerrainSize];
            p4 = vertices[ix];
        }
        else {
            p3 = vertices[ix-kTerrainSize];
            p4 = vertices[ix+kTerrainSize];
        }

        // Create vectors from p1 to p2 and p3 to p4
        v1 = SetVec3(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
        v2 = SetVec3(p4.x - p3.x, p4.y - p3.y, p4.z - p3.z);;

        // Compute the cross product of v1 and v2
        normal = cross(v1, v2);

		normals[z * kTerrainSize + x] = normalize(normal);
	}
}

void init(void)
{
	// GL inits
	glClearColor(0.4,0.6,0.8,0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	printError("GL inits");

	projectionMatrix = frustum(-0.15 * 640/480, 0.1 * 640/480, -0.1, 0.1, 0.2, 300.0);

	// Load and compile shader
	texShader = loadShaders("textured.vert", "textured.frag");
	printError("init shader");

	// Upload geometry to the GPU:
	MakeTerrain();
	floormodel = LoadDataToModel(vertices, normals, texCoords, NULL,
			indices, kTerrainSize*kTerrainSize, (kTerrainSize-1)*(kTerrainSize-1)*2*3);

	printError("LoadDataToModel");

// Important! The shader we upload to must be active!
	glUseProgram(texShader);
	glUniformMatrix4fv(glGetUniformLocation(texShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);


	glUniform1i(glGetUniformLocation(texShader, "tex"), 0); // Texture unit 0


	LoadTGATextureSimple("grass.tga", &grasstex);
	glBindTexture(GL_TEXTURE_2D, grasstex);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);

	printError("init arrays");
}

vec3 campos = {kTerrainSize*kPolySize/4, 1.5, kTerrainSize*kPolySize/4};
vec3 forward = {8, 0, 8};
vec3 up = {0, 1, 0};

void display(void)
{
	printError("pre display");

	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 worldToView, m;

	if (glutKeyIsDown('a'))
		forward = MultMat3Vec3(mat4tomat3(Ry(0.03)), forward);
	if (glutKeyIsDown('d'))
		forward = MultMat3Vec3(mat4tomat3(Ry(-0.03)), forward);
	if (glutKeyIsDown('w'))
		campos = VectorAdd(campos, ScalarMult(forward, 0.01));
	if (glutKeyIsDown('s'))
		campos = VectorSub(campos, ScalarMult(forward, 0.01));
	if (glutKeyIsDown('q'))
	{
		vec3 side = CrossProduct(forward, SetVector(0,1,0));
		campos = VectorSub(campos, ScalarMult(side, 0.01));
	}
	if (glutKeyIsDown('e'))
	{
		vec3 side = CrossProduct(forward, SetVector(0,1,0));
		campos = VectorAdd(campos, ScalarMult(side, 0.01));
	}

	// Move up/down
	if (glutKeyIsDown('z'))
		campos = VectorAdd(campos, ScalarMult(SetVector(0,1,0), 0.01));
	if (glutKeyIsDown('c'))
		campos = VectorSub(campos, ScalarMult(SetVector(0,1,0), 0.01));

	// NOTE: Looking up and down is done by making a side vector and rotation around arbitrary axis!
	if (glutKeyIsDown('+'))
	{
		vec3 side = CrossProduct(forward, SetVector(0,1,0));
		mat4 m = ArbRotate(side, 0.05);
		forward = MultMat3Vec3(mat4tomat3(m), forward);
	}
	if (glutKeyIsDown('-'))
	{
		vec3 side = CrossProduct(forward, SetVector(0,1,0));
		mat4 m = ArbRotate(side, -0.05);
		forward = MultMat3Vec3(mat4tomat3(m), forward);
	}

	worldToView = lookAtv(campos, VectorAdd(campos, forward), up);

	glBindTexture(GL_TEXTURE_2D, grasstex); // The texture is not used but provided as example
	// Floor
	glUseProgram(texShader);
	m = worldToView;
	glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
	DrawModel(floormodel, texShader, "inPosition", "inNormal", "inTexCoord");

	printError("display");

	glutSwapBuffers();
}

void keys(unsigned char key, int x, int y)
{
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 2);
	glutInitWindowSize(640,360);
	glutCreateWindow ("Lab 3b");
	glutRepeatingTimer(20);
	glutDisplayFunc(display);
	glutKeyboardFunc(keys);
	init ();
	glutMainLoop();
}
