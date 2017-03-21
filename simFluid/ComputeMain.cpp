#include "ComputeMain.h"

using namespace std;

extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

ComputeMain::ComputeMain(int argc, char **argv)
{
	// Initialize window with freeglut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(800, 600);
	glutCreateWindow("simFluid");

	cout << glGetString(GL_VENDOR) << endl;
	cout << glGetString(GL_RENDERER) << endl;
	
	glewInit();
	if (glewIsSupported("GL_VERSION_4_3"))
	{
		cout << "GL version is supported.\n";
	}
	else
	{
		cout << "GL version is not supported. Compute shader requires the minimum of OpenGL version 4.3. \n";
	}

	glEnable(GL_DEPTH_TEST);

	glutDisplayFunc(update);

	// Main loop to keep the window running
	glutMainLoop();
}


ComputeMain::~ComputeMain()
{
}

void ComputeMain::update(void)
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0);

	glutSwapBuffers();
}

int main(int argc, char **argv) {
	// Initialize
	ComputeMain computeMain(argc, argv);

	return 0;
}