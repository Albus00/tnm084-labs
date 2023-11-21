// Fractal tree generation

#define MAIN
#include "MicroGlut.h"
#include "GL_utilities.h"
#include "VectorUtils4.h"
#include "LittleOBJLoader.h"
#include "LoadTGA.h"
#include "glugg.h"

// uses framework OpenGL
// uses framework Cocoa

void MakeCylinderAlt(int aSlices, float height, float topwidth, float bottomwidth)
{
	gluggMode(GLUGG_TRIANGLE_STRIP);
	vec3 top = SetVector(0,height,0);
	vec3 center = SetVector(0,0,0);
	vec3 bn = SetVector(0,-1,0); // Bottom normal
	vec3 tn = SetVector(0,1,0); // Top normal

	for (float a = 0.0; a < 2.0*M_PI+0.0001; a += 2.0*M_PI / aSlices)
	{
		float a1 = a;

		vec3 p1 = SetVector(topwidth * cos(a1), height, topwidth * sin(a1));
		vec3 p2 = SetVector(bottomwidth * cos(a1), 0, bottomwidth * sin(a1));
		vec3 pn = SetVector(cos(a1), 0, sin(a1));

// Done making points and normals. Now create polygons!
		gluggNormalv(pn);
	    gluggTexCoord(height, a1/M_PI);
	    gluggVertexv(p2);
	    gluggTexCoord(0, a1/M_PI);
	    gluggVertexv(p1);
	}

	// Then walk around the top and bottom with fans
	gluggMode(GLUGG_TRIANGLE_FAN);
	gluggNormalv(bn);
	gluggVertexv(center);
	// Walk around edge
	for (float a = 0.0; a <= 2.0*M_PI+0.001; a += 2.0*M_PI / aSlices)
	{
		vec3 p = SetVector(bottomwidth * cos(a), 0, bottomwidth * sin(a));
	    gluggVertexv(p);
	}
	// Walk around edge
	gluggMode(GLUGG_TRIANGLE_FAN); // Reset to new fan
	gluggNormalv(tn);
	gluggVertexv(top);
	for (float a = 2.0*M_PI; a >= -0.001; a -= 2.0*M_PI / aSlices)
	{
		vec3 p = SetVector(topwidth * cos(a), height, topwidth * sin(a));
	    gluggVertexv(p);
	}
}


mat4 projectionMatrix;

Model *floormodel;
GLuint grasstex, barktex;

// Reference to shader programs
GLuint phongShader, texShader;

// Floor quad
GLfloat vertices2[] = {	-20.5,0.0,-20.5,
						20.5,0.0,-20.5,
						20.5,0.0,20.5,
						-20.5,0.0,20.5};
GLfloat normals2[] = {	0,1.0,0,
						0,1.0,0,
						0,1.0,0,
						0,1.0,0};
GLfloat texcoord2[] = {	50.0f, 50.0f,
						0.0f, 50.0f,
						0.0f, 0.0f,
						50.0f, 0.0f};
GLuint indices2[] = {	0,3,2, 0,2,1};

// THIS IS WHERE YOUR WORK GOES!

bool checkForRoot(int currentDepth)
{
    return (currentDepth == 0);
}

gluggModel MakeTree()
{
	gluggSetPositionName("inPosition");
	gluggSetNormalName("inNormal");
	gluggSetTexCoordName("inTexCoord");

	gluggBegin(GLUGG_TRIANGLES);

	// Between gluggBegin and gluggEnd, call MakeCylinderAlt plus glugg transformations
	// to create a tree.

	MakeBranch(4, 0, 3, 1);

	return gluggBuildModel(0);
}

void MakeBranch(const int maxDepth, int currentDepth, const int maxSporuts, int currentSprout) {
    gluggPushMatrix();

    if (currentDepth > 0) {
        // Branch
        float height = (float)rand()/(float)(RAND_MAX/2.5);
        MakeCylinderAlt(3, height, 0.1, 0.15);
        gluggTranslate(0, height, 0);
    }
    else {
        // Stem
        MakeCylinderAlt(3, 2, 0.1, 0.15);
        gluggTranslate(0, 2, 0);
    }
        printf("Rotation: %f deg, sprout: %i, depth: %i\n", (360/(float)maxSporuts) * ((float)currentSprout-1), currentSprout, currentDepth);

        float rotation = (float)rand()/(float)(RAND_MAX/(2*3.14));
        gluggRotate(rotation, 0, 1, 0);

        float tilt = (float)rand()/(float)(RAND_MAX/(3.14/4));
        gluggRotate(tilt, 0, 0, 1);
        const float scaling = 0.7;
        gluggScale(scaling, scaling, scaling);

    printf("%i , %i \n", currentDepth, currentSprout);


    if (currentDepth == maxDepth) {
        // Max depth
        if (currentSprout == maxSporuts) {
            // Max depth at last sprout
            printf("Last at max depth\n");
            printf("POP\n");
            gluggPopMatrix();
            return;
        }
        else {
            // Max depth at random sprout
            printf("POP\n");
            gluggPopMatrix();
            currentSprout++;
            MakeBranch(maxDepth, currentDepth, maxSporuts, currentSprout);
        }
    }
    else {
        // Random depth, build branch
        currentDepth++;
        MakeBranch(maxDepth, currentDepth, maxSporuts, 1);
        currentDepth--;

        if (currentSprout < maxSporuts) {
            // This is not the last sprout, build more sprouts
            printf("POP\n");
            gluggPopMatrix();
            currentSprout++;
            MakeBranch(maxDepth, currentDepth, maxSporuts, currentSprout);
        }
        else {
            // This is the last branch at a random depth
            printf("POP\n");
            gluggPopMatrix();
            return;
        }
    }
}

gluggModel tree;

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);

	// Set the clipping volume
	projectionMatrix = perspective(45,1.0f*w/h,1,1000);
	glUseProgram(phongShader);
	glUniformMatrix4fv(glGetUniformLocation(phongShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUseProgram(texShader);
	glUniformMatrix4fv(glGetUniformLocation(texShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
}

void init(void)
{
	// GL inits
	glClearColor(0.4,0.6,0.8,0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	printError("GL inits");

	projectionMatrix = frustum(-0.1, 0.1, -0.1, 0.1, 0.2, 300.0);

	// Load and compile shader
	phongShader = loadShaders("phong.vert", "phong.frag");
	texShader = loadShaders("textured.vert", "textured.frag");
	printError("init shader");

	// Upload geometry to the GPU:
	floormodel = LoadDataToModel((vec3 *)vertices2, (vec3 *)normals2, (vec2 *)texcoord2, NULL,
			indices2, 4, 6);

// Important! The shader we upload to must be active!
	glUseProgram(phongShader);
	glUniformMatrix4fv(glGetUniformLocation(phongShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUseProgram(texShader);
	glUniformMatrix4fv(glGetUniformLocation(texShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);

	glUniform1i(glGetUniformLocation(texShader, "tex"), 0); // Texture unit 0

	LoadTGATextureSimple("grass.tga", &grasstex);
	glBindTexture(GL_TEXTURE_2D, grasstex);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);

	LoadTGATextureSimple("bark2.tga", &barktex);

	tree = MakeTree();

	printError("init arrays");
}

GLfloat a = 0.0;
vec3 campos = {0, 1.5, 10};
vec3 forward = {0, 0, -4};
vec3 up = {0, 1, 0};

void display(void)
{
//	int i, j;

	printError("pre display");

	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 worldToView, m; // m1, m2, m, tr, scale;

	if (glutKeyIsDown('a'))
		forward = MultMat3Vec3(mat4tomat3(Ry(0.05)), forward);
	if (glutKeyIsDown('d'))
		forward = MultMat3Vec3(mat4tomat3(Ry(-0.05)), forward);
	if (glutKeyIsDown('w'))
		campos = VectorAdd(campos, ScalarMult(forward, 0.1));
	if (glutKeyIsDown('s'))
		campos = VectorSub(campos, ScalarMult(forward, 0.1));
	if (glutKeyIsDown('q'))
	{
		vec3 side = CrossProduct(forward, SetVector(0,1,0));
		campos = VectorSub(campos, ScalarMult(side, 0.05));
	}
	if (glutKeyIsDown('e'))
	{
		vec3 side = CrossProduct(forward, SetVector(0,1,0));
		campos = VectorAdd(campos, ScalarMult(side, 0.05));
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

	a += 0.1;

	glBindTexture(GL_TEXTURE_2D, grasstex);
	// Floor
	glUseProgram(texShader);
	m = worldToView;
	glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
	DrawModel(floormodel, texShader, "inPosition", "inNormal", "inTexCoord");

	// Draw the tree, as defined on MakeTree
	glBindTexture(GL_TEXTURE_2D, barktex);
	glUseProgram(texShader);
    m = Mult(worldToView, T(0, 0, 0));
    glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
	gluggDrawModel(tree, texShader);

	printError("display");

	glutSwapBuffers();
}

void keys(unsigned char key, int x, int y)
{
	switch (key)
	{
		case ' ':
			forward.y = 0;
			forward = ScalarMult(normalize(forward), 4.0);
			break;
	}
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 2);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(640,360);
	glutCreateWindow ("Fractal tree lab");
	glutRepeatingTimer(20);
	glutDisplayFunc(display);
	glutKeyboardFunc(keys);
	glutReshapeFunc(reshape);
	init ();
	glutMainLoop();
}
