#include "ComputeMain.h"



ComputeMain::ComputeMain(int argc, char **argv)
{
	// Initialize window with freeglut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(800, 600);
	glutCreateWindow("simFluid");

	glutMainLoop();
}


ComputeMain::~ComputeMain()
{
}

int main(int argc, char **argv) {
	// Initialize
	ComputeMain computeMain(argc, argv);

	return 0;
}