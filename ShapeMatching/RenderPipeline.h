#pragma once
#ifndef _RENDER_PIPELINE_H
#define _RENDER_PIPELINE_H
#include <iostream>

#include <GL\glew.h>
#include <GL\freeglut.h>
#include "imgui.h"
#include "imgui_impl_glut.h"
#include <camera.h>
#include <GLobjects.h>
#include <RenderUtils.h>
#include <CLutils.h>
#include <Eigen_op.h>
#include <ModelLoader.h>

#include <FastGlobalRegistration.h>

using namespace std;
unsigned int screenWidth = 800;
unsigned int screenHeight = 800;
unsigned int pre_screenWidth = 0;
unsigned int pre_screenHeight = 0;
bool show_debug_window = true;
bool show_test_window = false;
bool show_another_window = false;

Camera camera;
std::vector<Vector3f> points0, points1;
std::vector<Vector3f> normals0, normals1;
bool doRegistration = false;
FastGlobalReg fgr;
Eigen::Matrix4f optMat;

GLuint GLPointRenderProgram;
GLmem object0, object1;

//================================
// default draw event
//================================
void DrawCoord() {
	int num = 2 * 7;
	int grad = 50;

	glBegin(GL_LINES);

	for (int z = -num; z <= num; z++) {
		if (z == 0) {
			//x axis
			glColor3f(1, 0, 0);
		}
		else {
			glColor3f(0.2, 0.2, 0.3);
		}
		glVertex3f(-num * grad, 0, z * grad);
		glVertex3f(+num * grad, 0, z * grad);
	}

	for (int x = -num; x <= num; x++) {
		if (x == 0) {
			//z axis
			glColor3f(0, 0, 1);
		}
		else {
			glColor3f(0.2, 0.2, 0.3);
		}
		glVertex3f(x * grad, 0, -num * grad);
		glVertex3f(x * grad, 0, +num * grad);
	}

	glEnd();
}
//=========================================================================
//		Draw methods
//=========================================================================
void DrawScene2D() {
	{
		//glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
		glDisable(GL_DEPTH_TEST);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		// Setup viewport, orthographic projection matrix
		glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0.0f, screenWidth,  0.0f, screenHeight, -1.0f, +1.0f);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		// Draw a triangle at 0.5 z
		glBegin(GL_TRIANGLES);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(50.5, 50.5, 0.5);
		glVertex3f(550.5, 50.5, 0.5);
		glVertex3f(550.0, 150.5, 0.5);
		glEnd();

		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		//glPopAttrib();
	}
}

void DrawScene3D() {
	{
		//glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
		glEnable(GL_DEPTH_TEST);
		// Setup viewport, orthographic projection matrix
		glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		gluPerspective(45.0, (GLdouble)screenWidth / (GLdouble)screenHeight, 1.0, 10000.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMultMatrixf(&camera.Mat()[0][0]);

		// default coord grids
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		{
			glLineWidth(2.0);
			DrawCoord();
			glColor3f(0, 1, 0);
			glVertex3f(0, 0, 0);
			glVertex3f(0, 500, 0);
			glEnd();
		}
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		// Point cloud data
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		if (points0.size()) {
			glUseProgram(GLPointRenderProgram);
			glm::mat4 proj = glm::mat4(1.0f);
			glm::mat4 view = glm::mat4(1.0f);
			glGetFloatv(GL_PROJECTION_MATRIX, glm::value_ptr(proj));
			glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(view));
			glUniformMatrix4fv(glGetUniformLocation(GLPointRenderProgram, "proj"), 1, GL_FALSE, &proj[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(GLPointRenderProgram, "view"), 1, GL_FALSE, &view[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(GLPointRenderProgram, "model"), 1, GL_FALSE, optMat.data());
			glBindVertexArray(object0.vao);
			glDrawArrays(GL_POINTS, 0, object0.m_numVerts);
			glBindVertexArray(0);

			glUseProgram(0);
			/*
			glEnableClientState(GL_VERTEX_ARRAY);
			//glEnableClientState(GL_COLOR_ARRAY);
			glColor3f(1.0f, 0.0f, 0.0f);
			glVertexPointer(3, GL_FLOAT, sizeof(GLfloat) * 3, points0.data());
			//glColorPointer(3, GL_FLOAT, sizeof(GLfloat) * 3, pc0.normals.data());
			glDrawArrays(GL_POINTS, 0, points0.size());

			//glDisableClientState(GL_COLOR_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);
			//*/
		}
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		if (!points1.size()) {
			glEnableClientState(GL_VERTEX_ARRAY);
			//glEnableClientState(GL_COLOR_ARRAY);
			glColor3f(0.0f, 1.0f, 0.0f);
			glVertexPointer(3, GL_FLOAT, sizeof(GLfloat) * 3, points1.data());
			//glColorPointer(3, GL_FLOAT, sizeof(GLfloat) * 3, pc1.normals.data());
			glDrawArrays(GL_POINTS, 0, points1.size());

			//glDisableClientState(GL_COLOR_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);
		}
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		//glPopAttrib();
	}
}

void drawGUI()
{
	ImGui_ImplGLUT_NewFrame(screenWidth, screenHeight);

	// 1. Show a simple window
	// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
	if(show_debug_window)
	{	
		ImGui::SetWindowSize(ImVec2(200, 150), ImGuiSetCond_FirstUseEver);
		ImGui::SetWindowPos(ImVec2(0, screenHeight-150));
		static float f = 0.0f;
		ImGui::Text("Debug information");
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		if (ImGui::Button("Test Window")) show_test_window ^= 1;
		if (ImGui::Button("Another Window")) show_another_window ^= 1;
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	// 2. Show another simple window, this time using an explicit Begin/End pair
	if (show_another_window)
	{
		ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2(150, 20), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Camera information");
		glm::vec3 campos = camera.GetPos();
		ImGui::Text("Camera position: (%.3f, %.3f, %.3f)", campos.x, campos.y, campos.z);
		glm::vec3 camdir = camera.GetDir();
		ImGui::Text("Camera direction: (%.3f, %.3f, %.3f)", camdir.x, camdir.y, camdir.z);
		glm::vec3 camright = glm::cross(camdir, glm::vec3(0.0f, 1.0f, 0.0f));
		ImGui::Text("Camera right: (%.3f, %.3f, %.3f)", camright.x, camright.y, camright.z);
		ImGui::End();
	}

	// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if (show_test_window)
	{
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}

	ImGui::Render();
}

//=========================================================================
//		Render
//=========================================================================
void Render(void)
{
	// Get Back to the Modelview
	glClearColor(0.15f, 0.15f, 0.18f, 1.0f);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_DEPTH_TEST);

	if (1) {
		// draw 3D scene
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		DrawScene3D();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	} 
	if (1) {
		// draw 2D scene
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		DrawScene2D();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}

	if(1){
		// draw gui, seems unnecessary to push/pop matrix and attributes
		//glPushAttrib(GL_ALL_ATTRIB_BITS);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		drawGUI();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		//glPopAttrib();
	}

	glFlush();
	glutSwapBuffers();
	glutPostRedisplay();
}
//=========================================================================
//		Reshape
//=========================================================================
void Reshape(int w, int h)
{
	// update screen width and height for imgui new frames
	screenWidth = w;
	screenHeight = h;

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
		h = 1;
	float ratio = 1.0f* w / h;

	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);

	// Reset Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	//gluPerspective(45, ratio, 1.0f, 10000.0f);
	//glMatrixMode(GL_PROJECTION);
	//glViewport(0, 0, width, height);
	//glLoadIdentity();
	//glOrtho(0.f, w, 0.f, h, -1.f, 1.f);
	//glOrtho(-2.0, 2.0, -2.0, 2.0, -2.0, 2.0);
	//glOrtho(0.f, w, 0.f, h, -1.f, 1.f);
	glutPostRedisplay();
}

//=========================================================================
//		Update
//=========================================================================
void Update(void) {
	if (doRegistration) {
		fgr.NormalizePoints();
		fgr.OptimizePairwise(false, 4);
		optMat = fgr.GetRes();
		AffineTransfomrPointsFromMat(points0, optMat.inverse());
		doRegistration = false;
	}
}

//=========================================================================
//		keyboard & mouse callback
//=========================================================================
bool keyboardEvent(unsigned char nChar, int nX, int nY)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharacter(nChar);

	if (nChar == 27) { //Esc-key
		glutLeaveMainLoop();
	}
	if (nChar == '+') {
		camera.ChangeAperture(0.02f);
	}
	if (nChar == '-') {
		camera.ChangeAperture(-0.02f);
	}
	if (nChar == ']') {
		camera.ChangeFocalDistance(10.0f);
	}
	if (nChar == '[') {
		camera.ChangeFocalDistance(-10.0f);
	}
	if (nChar == 't' || nChar == 'T') {
		screenWidth = 1920;
		screenHeight = 1080;
	}
	if (nChar == 'D' || nChar == 'd') {
		show_debug_window = !show_debug_window;
	}
	if (nChar == 'r') {
		doRegistration = !doRegistration;
	}

	return true;
}

void KeyboardSpecial(int key, int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharacter(key);
}

void keyboardCallback(unsigned char nChar, int x, int y)
{
	if (keyboardEvent(nChar, x, y))
	{
		glutPostRedisplay();
	}
}

bool mouseEvent(int button, int state, int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2((float)x, (float)y);

	if (state == GLUT_DOWN && (button == GLUT_LEFT_BUTTON))
		io.MouseDown[0] = true;
	else
		io.MouseDown[0] = false;

	if (state == GLUT_DOWN && (button == GLUT_RIGHT_BUTTON))
		io.MouseDown[1] = true;
	else
		io.MouseDown[1] = false;

	if (state == GLUT_DOWN && (button == GLUT_MIDDLE_BUTTON))
		io.MouseDown[2] = true;
	else
		io.MouseDown[2] = false;

	return true;
}

void MouseWheel(int button, int dir, int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2((float)x, (float)y);
	if (dir > 0)
	{
		// Zoom in
		io.MouseWheel = 1.0;
		camera.Zoom(dir * 5.0f);
	}
	else if (dir < 0)
	{
		// Zoom out
		io.MouseWheel = -1.0;
		camera.Zoom(dir * 5.0f);
	}
}

void MouseCallback(int button, int state, int x, int y)
{
	if (mouseEvent(button, state, x, y))
	{

	}
}

void MouseDragCallback(int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2((float)x, (float)y);
	float dy = (io.MousePos.y - io.MousePosPrev.y);
	float dx = (io.MousePos.x - io.MousePosPrev.x);
	if (io.MouseDown[0]) {
		camera.Rot(-dy * 0.20f, -dx * 0.20f);
	}
	if (io.MouseDown[2]) {
		camera.Move(-dx * 1.0f, dy * 1.0f);
	}
	glutPostRedisplay();
}

void MouseMoveCallback(int x, int y)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2((float)x, (float)y);

	glutPostRedisplay();
}


//=========================================================================
//		Initialize platforms
//=========================================================================
void Init_GLshader(void) {
	std::vector<char> vert, frag;
	LoadGLShaderFromFile(vert, "Shaders/PointRender.vs");
	LoadGLShaderFromFile(frag, "Shaders/PointRender.fs");
	GLPointRenderProgram = CompileGLShader("PointRender", vert.data(), frag.data());
}
// initialize ogl and imgui
void Init_OpenGL(int argc, char **argv, const char* title)
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 0);
	//glutInitContextFlags(GLUT_FORWARD_COMPATIBLE); // cause error
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_MULTISAMPLE);

	glutInitWindowSize(screenWidth, screenHeight);
	glutInitWindowPosition(200, 200);
	glutCreateWindow(title);
	fprintf(stdout, "INFO: OpenGL Version: %s\n", glGetString(GL_VERSION));

	// glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		getchar();
		return;
	}

	// shaders
	Init_GLshader();

	// virtual camera
	{
		camera.focus = glm::vec3(0.0f, 0.0f, 0.0);
		camera.angle = glm::vec2(-30.0f, 0.0f);
		camera.fov = glm::vec2(45.0f, 45.0f);
		camera.apertureRadius = 0.01f;
		camera.focalDistance = 100.0f;
		camera.radius = 100.0f;
	}

	// callback
	glutDisplayFunc(Render);
	glutReshapeFunc(Reshape);
	glutIdleFunc(Update);
	glutKeyboardFunc(keyboardCallback);
	glutSpecialFunc(KeyboardSpecial);
	glutMouseFunc(MouseCallback);
	glutMouseWheelFunc(MouseWheel);
	glutMotionFunc(MouseDragCallback);
	glutPassiveMotionFunc(MouseMoveCallback);
}

void Init_RenderScene(void) {
	PLYModelLoader pc0, pc1;
	pc0.LoadModel("Depth_0000.ply");
	pc1.LoadModel("Depth_0000.ply");
	pc0.CopyToBuffer(points0, normals0);
	pc1.CopyToBuffer(points1, normals1);
	ScalePoints(points0, 50.0f);
	ScalePoints(points1, 50.0f);
	AffineTransformPointsFromAngle(points0, Eigen::Vector3f(20.0f, -50.0f, 70.0f), Eigen::Vector3f(-50.5f, +50.2f, -50.7f));

	CreateGLmem(object0, points0, normals0);
	CreateGLmem(object1, points1, normals1);

	fgr.LoadPoints(points0, points1);
	fgr.LoadCorrespondence(points0);
	optMat = Eigen::Matrix4f::Identity();
}

void Init_OpenCL(void) {


}

void Init_Imgui(void) {
	//glClearColor(0.447f, 0.565f, 0.604f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplGLUT_Init();
}

//=========================================================================
//		Run Mainloop
//=========================================================================
void Run_Render(int argc, char **argv, const char* title){ 
	Init_OpenGL(argc, argv, title);
	Init_RenderScene();
	Init_OpenCL();
	Init_GLshader();
	Init_Imgui();

	glutMainLoop();
	ImGui_ImplGLUT_Shutdown();
}


#endif // !_RENDER_PIPELINE_H