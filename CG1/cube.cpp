//
// Display a color cube
//
// Colors are assigned to each vertex and then the rasterizer interpolates
//   those colors across the triangles.  We us an orthographic projection
//   as the default projetion.

#include "cube.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

#include <iostream>
#include "glm/gtx/string_cast.hpp"

glm::mat4 projectMat;
glm::mat4 viewMat;

GLuint pvmMatrixID; // vshader의 mPVM에 넘겨줄 uniform variable

#define arm_xlen 0.2f
#define arm_ylen 0.3f
#define arm_zlen 0.15f
#define forearm_xlen 0.2f
#define forearm_ylen 0.3f
#define forearm_zlen 0.15f
#define leg_xlen 0.2f
#define leg_ylen 0.3f
#define leg_zlen 0.2f
#define lowerleg_xlen 0.2f
#define lowerleg_ylen 0.4f
#define lowerleg_zlen 0.2f

// left angle
float left_arm_angle = glm::radians(90.0f);
float left_forearm_angle = glm::radians(0.0f);
float left_leg_angle = glm::radians(-90.0f);
float left_lowerleg_angle = glm::radians(0.0f);

// right angle
float right_arm_angle = glm::radians(-90.0f);
float right_forearm_angle = glm::radians(180.0f);
float right_leg_angle = glm::radians(45.0f);
float right_lowerleg_angle = glm::radians(45.0f);
int level = 1; // 단계: 1 ~ 6

typedef glm::vec4  color4;
typedef glm::vec4  point4;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)

point4 points[NumVertices];
color4 colors[NumVertices];

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[8] = {
	point4(-0.5, -0.5, 0.5, 1.0),
	point4(-0.5, 0.5, 0.5, 1.0),
	point4(0.5, 0.5, 0.5, 1.0),
	point4(0.5, -0.5, 0.5, 1.0),
	point4(-0.5, -0.5, -0.5, 1.0),
	point4(-0.5, 0.5, -0.5, 1.0),
	point4(0.5, 0.5, -0.5, 1.0),
	point4(0.5, -0.5, -0.5, 1.0)
};

// RGBA colors
color4 vertex_colors[8] = {
	color4(0.0, 0.0, 0.0, 1.0),  // black
	color4(0.0, 1.0, 1.0, 1.0),   // cyan
	color4(1.0, 0.0, 1.0, 1.0),  // magenta
	color4(1.0, 1.0, 0.0, 1.0),  // yellow
	color4(1.0, 0.0, 0.0, 1.0),  // red
	color4(0.0, 1.0, 0.0, 1.0),  // green
	color4(0.0, 0.0, 1.0, 1.0),  // blue
	color4(1.0, 1.0, 1.0, 1.0)  // white
};

//----------------------------------------------------------------------------

// quad generates two triangles for each face and assigns colors
//    to the vertices
int Index = 0;
void
quad(int a, int b, int c, int d)
{
	colors[Index] = vertex_colors[a]; points[Index] = vertices[a];  Index++;
	colors[Index] = vertex_colors[b]; points[Index] = vertices[b];  Index++;
	colors[Index] = vertex_colors[c]; points[Index] = vertices[c];  Index++;
	colors[Index] = vertex_colors[a]; points[Index] = vertices[a];  Index++;
	colors[Index] = vertex_colors[c]; points[Index] = vertices[c];  Index++;
	colors[Index] = vertex_colors[d]; points[Index] = vertices[d];  Index++;
}

//----------------------------------------------------------------------------

// generate 12 triangles: 36 vertices and 36 colors
void
colorcube()
{
	quad(1, 0, 3, 2);
	quad(2, 3, 7, 6);
	quad(3, 0, 4, 7);
	quad(6, 5, 1, 2);
	quad(4, 5, 6, 7);
	quad(5, 4, 0, 1);
}

//----------------------------------------------------------------------------

// OpenGL initialization
void
init()
{
	colorcube();

	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);

	// Load shaders and use the resulting shader program
	GLuint program = InitShader("src/vshader.glsl", "src/fshader.glsl"); // shader를 load해서 compile해서 연결
	glUseProgram(program);

	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(points)));

	pvmMatrixID = glGetUniformLocation(program, "mPVM");

	projectMat = glm::perspective(glm::radians(65.0f), 1.0f, 0.1f, 100.0f);
	viewMat = glm::lookAt(glm::vec3(2, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); // 2, 0, 0에서 원점을 바라보고 있으며, up vector가 0, 1, 0

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
}

//----------------------------------------------------------------------------
glm::vec3 body_locate(0.0, 0.0, 0.0);

void drawMan(glm::mat4 manMat) {
	glm::mat4 modelMat, leftarm, leftforearm, rightarm, rightforearm, leftleg, leftlowerleg, rightleg, rightlowerleg, pvmMat;

	// man body 
	modelMat = glm::translate(manMat, body_locate);
	modelMat = glm::scale(modelMat, glm::vec3(0.4, 0.6, 0.2));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// man head
	modelMat = glm::translate(manMat, body_locate + glm::vec3(0, 0.4, 0));
	modelMat = glm::scale(modelMat, glm::vec3(0.2, 0.2, 0.2));
	pvmMat = projectMat * viewMat * modelMat;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	//left Arm
	leftarm = glm::translate(manMat, body_locate + glm::vec3(1 * 0.25, 0.3, 0));
	leftarm = glm::rotate(leftarm, left_arm_angle, glm::vec3(1, 0, 0));
	leftarm = glm::translate(leftarm, glm::vec3(0, -1 * arm_ylen / 2, 0));
	leftarm = glm::scale(leftarm, glm::vec3(arm_xlen, arm_ylen, arm_zlen));
	pvmMat = projectMat * viewMat * leftarm;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	glm::vec4 left_arm_coord = leftarm * glm::vec4(0, 0, 0, 1); // 왼쪽팔 좌표 받기
	left_arm_coord = left_arm_coord - glm::vec4(0, arm_ylen / 2 * cos(left_arm_angle), arm_ylen / 2 * sin(left_arm_angle), 0);

	//right arm
	rightarm = glm::translate(manMat, body_locate + glm::vec3(-1 * 0.25, 0.3, 0));
	rightarm = glm::rotate(rightarm, right_arm_angle, glm::vec3(1, 0, 0));
	rightarm = glm::translate(rightarm, glm::vec3(0, -1 * arm_ylen / 2, 0));
	rightarm = glm::scale(rightarm, glm::vec3(arm_xlen, arm_ylen, arm_zlen));
	pvmMat = projectMat * viewMat * rightarm;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	glm::vec4 right_arm_coord = rightarm * glm::vec4(0, 0, 0, 1); // 오른쪽팔 좌표 받기
	right_arm_coord = right_arm_coord - glm::vec4(0, arm_ylen / 2 * cos(right_arm_angle), arm_ylen / 2 * sin(right_arm_angle), 0);

	//left fore arm
	leftforearm = glm::translate(manMat, glm::vec3(left_arm_coord[0], left_arm_coord[1], left_arm_coord[2])); // 왼쪽팔 위치로
	leftforearm = glm::rotate(leftforearm, left_forearm_angle, glm::vec3(1, 1, 0));
	leftforearm = glm::translate(leftforearm, glm::vec3(0, -1 * forearm_ylen / 2, 0));
	leftforearm = glm::scale(leftforearm, glm::vec3(forearm_xlen, forearm_ylen, forearm_zlen));
	pvmMat = projectMat * viewMat * leftforearm;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	//right fore arm
	rightforearm = glm::translate(manMat, glm::vec3(right_arm_coord[0], right_arm_coord[1], right_arm_coord[2]));// 오른쪽팔 위치로
	rightforearm = glm::rotate(rightforearm, right_forearm_angle, glm::vec3(1, -1, 0));
	rightforearm = glm::translate(rightforearm, glm::vec3(0, -1 * forearm_ylen / 2, 0));
	rightforearm = glm::scale(rightforearm, glm::vec3(forearm_xlen, forearm_ylen, forearm_zlen));
	pvmMat = projectMat * viewMat * rightforearm;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// left leg
	leftleg = glm::translate(manMat, body_locate + glm::vec3(1 * 0.1, -0.3, 0));
	leftleg = glm::rotate(leftleg, left_leg_angle, glm::vec3(1, 0, 0));
	leftleg = glm::translate(leftleg, glm::vec3(0, -1 * leg_ylen / 2, 0));
	leftleg = glm::scale(leftleg, glm::vec3(leg_xlen, leg_ylen, leg_zlen));
	pvmMat = projectMat * viewMat * leftleg;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	glm::vec4 left_leg_coord = leftleg * glm::vec4(0, 0, 0, 1); 
	left_leg_coord = left_leg_coord - glm::vec4(0, leg_ylen / 2 * cos(left_leg_angle), leg_ylen / 2 * sin(left_leg_angle), 0);

	// right leg
	rightleg = glm::translate(manMat, body_locate + glm::vec3(-1 * 0.1, -0.3, 0));
	rightleg = glm::rotate(rightleg, right_leg_angle, glm::vec3(1, 0, 0));
	rightleg = glm::translate(rightleg, glm::vec3(0, -1 * leg_ylen / 2, 0));
	rightleg = glm::scale(rightleg, glm::vec3(leg_xlen, leg_ylen, leg_zlen));
	pvmMat = projectMat * viewMat * rightleg;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	glm::vec4 right_leg_coord = rightleg * glm::vec4(0, 0, 0, 1); 
	right_leg_coord = right_leg_coord - glm::vec4(0, leg_ylen / 2 * cos(right_leg_angle), leg_ylen / 2 * sin(right_leg_angle), 0);

	// left lowerlegs
	leftlowerleg = glm::translate(manMat, glm::vec3(left_leg_coord[0], left_leg_coord[1], left_leg_coord[2])); // left leg로 이동
	leftlowerleg = glm::rotate(leftlowerleg, left_lowerleg_angle, glm::vec3(1, 0, 0));
	leftlowerleg = glm::translate(leftlowerleg, glm::vec3(0, -1 * lowerleg_ylen / 2, 0));
	leftlowerleg = glm::scale(leftlowerleg, glm::vec3(lowerleg_xlen, lowerleg_ylen, lowerleg_zlen));
	pvmMat = projectMat * viewMat * leftlowerleg;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// right lowerlegs
	rightlowerleg = glm::translate(manMat, glm::vec3(right_leg_coord[0], right_leg_coord[1], right_leg_coord[2])); // right leg로 이동
	rightlowerleg = glm::rotate(rightlowerleg, right_lowerleg_angle, glm::vec3(1, 0, 0));
	rightlowerleg = glm::translate(rightlowerleg, glm::vec3(0, -1 * lowerleg_ylen / 2, 0));
	rightlowerleg = glm::scale(rightlowerleg, glm::vec3(lowerleg_xlen, lowerleg_ylen, lowerleg_zlen));
	pvmMat = projectMat * viewMat * rightlowerleg;
	glUniformMatrix4fv(pvmMatrixID, 1, GL_FALSE, &pvmMat[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	
}

void display(void)
{
	glm::mat4 worldMat, pvmMat;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// indentity matrix로 선언
	worldMat = glm::mat4(1.0f);
	drawMan(worldMat);

	glutSwapBuffers();
}

//----------------------------------------------------------------------------

void idle()
{
	static int prevTime = glutGet(GLUT_ELAPSED_TIME); // 프로그램이 시작한 이후로 얼마만큼의 시간이 지났는지 ms 단위로 return
	int currTime = glutGet(GLUT_ELAPSED_TIME);

	float d_t = 1000.f;
	glm::vec3 d_body(0.0, 0.01, 0.0);

	if (abs(currTime - prevTime) >= 20) // 20ms마다 실행하도록 설정. 이걸 안하면 컴퓨터 속도에 따라 달라지기 때문에 시간을 지정하도록 한다.
	{
		float t = abs(currTime - prevTime); // 저번과 실행한 이후로 얼마나 시간이 지났는지...
		if (level == 1) {
			body_locate -= d_body;
			std::cout << "level 1: " << glutGet(GLUT_ELAPSED_TIME) << std::endl;

			left_arm_angle -= glm::radians(t * 480.0f / d_t);
			right_arm_angle += glm::radians(t * 480.0f / d_t);

			left_forearm_angle -= glm::radians(t * 480.0f / d_t);
			right_forearm_angle += glm::radians(t * 480.0f / d_t);

			left_leg_angle += glm::radians(t * 360.0f / d_t);
			right_leg_angle -= glm::radians(t * 360.0f / d_t);

			right_lowerleg_angle += glm::radians(t * 360.0f / d_t);

			if (right_lowerleg_angle >= glm::radians(90.0f)) {
				level = 2;
			}
		}
		else if (level == 2) {
			body_locate -= d_body;
			std::cout << "level 2: " << glutGet(GLUT_ELAPSED_TIME) << std::endl;

			left_arm_angle -= glm::radians(t * 480.0f / d_t);
			right_arm_angle += glm::radians(t * 480.0f / d_t);

			left_forearm_angle -= glm::radians(t * 480.0f / d_t);
			right_forearm_angle += glm::radians(t * 480.0f / d_t);

			left_leg_angle += glm::radians(t * 360.0f / d_t);
			right_leg_angle -= glm::radians(t * 360.0f / d_t);

			right_lowerleg_angle -= glm::radians(t * 360.0f / d_t);

			if (left_leg_angle >= glm::radians(0.0f)) {
				level = 3;
			}
		}
		else if (level == 3) {
			body_locate += d_body;
			body_locate += d_body;
			std::cout << "level 3: " << glutGet(GLUT_ELAPSED_TIME) << std::endl;

			left_arm_angle -= glm::radians(t * 480.0f / d_t);
			right_arm_angle += glm::radians(t * 480.0f / d_t);

			left_forearm_angle -= glm::radians(t * 480.0f / d_t);
			right_forearm_angle += glm::radians(t * 480.0f / d_t);

			left_leg_angle += glm::radians(t * 360.0f / d_t);
			right_leg_angle -= glm::radians(t * 360.0f / d_t);

			left_lowerleg_angle += glm::radians(t * 360.0f / d_t);
			right_lowerleg_angle -= glm::radians(t * 360.0f / d_t);

			if (left_leg_angle >= glm::radians(45.0f)) {
				level = 4;
			}
		}
		else if (level == 4) {
			body_locate -= d_body;
			std::cout << "level 4: " << glutGet(GLUT_ELAPSED_TIME) << std::endl;

			left_arm_angle += glm::radians(t * 480.0f / d_t);
			right_arm_angle -= glm::radians(t * 480.0f / d_t);

			left_forearm_angle += glm::radians(t * 480.0f / d_t);
			right_forearm_angle -= glm::radians(t * 480.0f / d_t);

			left_leg_angle -= glm::radians(t * 360.0f / d_t);
			right_leg_angle += glm::radians(t * 360.0f / d_t);

			left_lowerleg_angle += glm::radians(t * 360.0f / d_t);

			if (left_lowerleg_angle >= glm::radians(90.0f)) {
				level = 5;
			}
		}
		else if (level == 5) {
			body_locate -= d_body;
			std::cout << "level 5: " << glutGet(GLUT_ELAPSED_TIME) << std::endl;

			left_arm_angle += glm::radians(t * 480.0f / d_t);
			right_arm_angle -= glm::radians(t * 480.0f / d_t);

			left_forearm_angle += glm::radians(t * 480.0f / d_t);
			right_forearm_angle -= glm::radians(t * 480.0f / d_t);

			left_leg_angle -= glm::radians(t * 360.0f / d_t);
			right_leg_angle += glm::radians(t * 360.0f / d_t);

			left_lowerleg_angle -= glm::radians(t * 360.0f / d_t);

			if (right_leg_angle >= glm::radians(0.0f)) {
				level = 6;
			}
		}
		else if (level == 6) {
			body_locate += d_body;
			body_locate += d_body;
			std::cout << "level 6: " << glutGet(GLUT_ELAPSED_TIME) << std::endl;

			left_arm_angle += glm::radians(t * 480.0f / d_t);
			right_arm_angle -= glm::radians(t * 480.0f / d_t);

			left_forearm_angle += glm::radians(t * 480.0f / d_t);
			right_forearm_angle -= glm::radians(t * 480.0f / d_t);

			left_leg_angle -= glm::radians(t * 360.0f / d_t);
			right_leg_angle += glm::radians(t * 360.0f / d_t);

			left_lowerleg_angle -= glm::radians(t * 360.0f / d_t);
			right_lowerleg_angle += glm::radians(t * 360.0f / d_t);

			if (right_leg_angle >= glm::radians(45.0f)) {
				body_locate = glm::vec3(0, 0, 0);
				level = 1;
			}
		}
		else {
			// error
		}
		prevTime = currTime;
		glutPostRedisplay();
	}
}

//----------------------------------------------------------------------------

void
keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case '1':
		viewMat = glm::lookAt(glm::vec3(2, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); // 2, 0, 0에서 원점을 바라보고 있으며, up vector가 0, 1, 0
		break;
	case '2':
		viewMat = glm::lookAt(glm::vec3(0, 0, -2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); // 0, 0, -2에서 원점을 바라보고 있으며, up vector가 0, 1, 0
		break;
	case '3':
		viewMat = glm::lookAt(glm::vec3(0, 0, 2), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); // 0, 0, 2에서 원점을 바라보고 있으며, up vector가 0, 1, 0
		break;
	}
}

//----------------------------------------------------------------------------

void resize(int w, int h)
{
	float ratio = (float)w / (float)h;
	glViewport(0, 0, w, h);

	// window size가 변하더라도 aspect ratio를 유지하기 위해 사용
	// near, far는 고정적일지 몰라도 ratio값이 변할 수 있음. w와 h가 바뀌기 때문에
	projectMat = glm::perspective(glm::radians(65.0f), ratio, 0.1f, 100.0f); 

	glutPostRedisplay();
}

//----------------------------------------------------------------------------

int
main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutInitContextVersion(3, 2);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow("Man Running");

	glewInit();

	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard); // keyboard input 받을 때마다 keyboard 호출
	glutReshapeFunc(resize); // window size가 바뀔 때마다 resize를 호출
	glutIdleFunc(idle);

	glutMainLoop();
	return 0;
}
