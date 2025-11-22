#include <glew.h>
#include <freeglut.h>
#include <freeglut_ext.h> 

#include <iostream>
#include <vector>
#include <ctime>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "filetobuf.h"
#include "shaderMaker.h"
#include "Object.h"

#include "sphere_obj_load.h"


//void make_vertexShaders();
//void make_fragmentShaders();
//GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
void setupBuffers();
void TimerFunction(int value);

// ?
std::vector<float> vertices;
GLuint VAO, VBO;

Mesh gSphere;  // sphere obj

std::vector<Object*> objects; // object list

glm::vec3 spherePosition(-40.0f, 25.0f, -90.0f);
float vy = 0.0f;  // y 방향 속도
clock_t lastTime;  // 이전 프레임 시간
const float gravity = -9.8f * 2;  // 중력 가속도
const float groundY = -20.0f;  // 바닥 위치

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);  // using depth buffer
	glutInitWindowPosition(100, 50);
	glutInitWindowSize(width, height);
	glutCreateWindow("ComputerGraphics_Prject");

	glewExperimental = GL_TRUE;
	glewInit();

	// callback 
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutTimerFunc(16, TimerFunction, 0);  // ~60 FPS (16ms)

	glEnable(GL_DEPTH_TEST); // depth buffer


	make_vertexShaders();
	make_fragmentShaders();
	shaderProgramID = make_shaderProgram();

	// obj load
	if (!LoadOBJ_PosNorm_Interleaved("sphere.obj", gSphere))
	{
		std::cerr << "Failed to load sphere.obj\n";
		return 1;
	}

	// buffer setting
	setupBuffers();

	// vertex data setting
	vertices = {
		// position             // color
		 0.0f,  0.5f, -2.0f,  1.0f, 0.0f, 0.0f,  // top vertex (red)
		-0.5f, -0.5f, -2.0f,  0.0f, 1.0f, 0.0f,  // left vertex (green)
		 0.5f, -0.5f, -2.0f,  0.0f, 0.0f, 1.0f   // right vertex (blue)
	};

	Player* player = new Player();
	//objects.push_back(player);

	Bullet* bullet = new Bullet();
	bullet->setPosition(-40.0f, 25.0f, -90.0f);
	bullet->setMesh(gSphere);
	objects.push_back(bullet);
	// 시간 초기화
	lastTime = clock();

	glutMainLoop();

	return 0;
}

// draw sphere
void DrawSphere(const Mesh& mesh, GLuint shaderProgram, const glm::mat4& model, const glm::vec3& color)
{
	GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);

	GLint objLoc = glGetUniformLocation(shaderProgram, "objectColor");
	glUniform3fv(objLoc, 1, &color[0]);

	glBindVertexArray(mesh.vao);
	glDrawArrays(GL_TRIANGLES, 0, mesh.count);
	glBindVertexArray(0);
}

GLvoid drawScene()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// using shader program
	glUseProgram(shaderProgramID);

	GLint lightOnLoc = glGetUniformLocation(shaderProgramID, "lightOn");
	glUniform1i(lightOnLoc, 1); // 1 light on / 0 light off

	GLint lightLoc = glGetUniformLocation(shaderProgramID, "lightColor");
	GLint objLoc = glGetUniformLocation(shaderProgramID, "objectColor");

	glm::vec3 lightBasePos(3.0f, 0.0f, 2.5f);
	glm::mat4 lightRotate = glm::rotate(glm::mat4(1.0f), glm::radians(-40.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec3 lightPos = glm::vec3(lightRotate * glm::vec4(lightBasePos, 1.0f));

	GLint uLightPos = glGetUniformLocation(shaderProgramID, "lightPos");     // light position
	GLuint viewPosLoc = glGetUniformLocation(shaderProgramID, "viewPos");    // camera position
	// transfer to shader
	glUniform3f(lightLoc, 1.0f, 1.0f, 1.0f);      // light color
	glUniform3f(objLoc, 1.0f, 0.7f, 0.7f);        // object color
	glUniform3f(uLightPos, lightPos.x, lightPos.y, lightPos.z); // light position

	GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
	GLint projLoc = glGetUniformLocation(shaderProgramID, "projection");
	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);  // camera position to shader

	// x축 기준 -40도 회전 ( 위에서 아래로 보는 각도 ) << 직각이 되어야 보기 편할 거 같아요
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	cameraPos = glm::vec3(rotation * glm::vec4(cameraPos - cameraDirection, 1.0f)) + cameraDirection;

	glm::mat4 vTransform = glm::mat4(1.0f);
	vTransform = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &vTransform[0][0]);

	glm::mat4 mTransform = glm::mat4(1.0f);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &mTransform[0][0]);

	glm::mat4 pTransform = glm::mat4(1.0f);
	pTransform = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &pTransform[0][0]);

	

	// center sphere
	glm::mat4 centerM = glm::translate(glm::mat4(1.0f), spherePosition);
	centerM = glm::scale(centerM, glm::vec3(1.5f, 1.5f, 1.5f));
	//DrawSphere(gSphere, shaderProgramID, centerM, glm::vec3(0.8f, 0.0f, 0.0f));

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
		vertices.data(), GL_STATIC_DRAW);



	for (auto obj : objects){
		obj->render(shaderProgramID, VAO, VBO, vertices);
	}
	

	glBindVertexArray(0);
	glutSwapBuffers();
}

GLvoid Reshape(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, w, h);
}

void setupBuffers()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void TimerFunction(int value)
{
	// 델타 타임 계산
	clock_t currentTime = clock();
	float deltaTime = (float)(currentTime - lastTime) / CLOCKS_PER_SEC;
	lastTime = currentTime;

	/*// z축 이동
	if (spherePosition.z < -40.0f)
		spherePosition.z += 5.0f * deltaTime;

	// 중력 적용
	vy += gravity * deltaTime;

	// y 위치 업데이트
	spherePosition.y += vy * deltaTime;

	// 바닥 충돌 체크 (완전탄성 충돌)
	if (spherePosition.y <= groundY) {
		spherePosition.y = groundY;  // 바닥 위치로 보정
		vy *= -1.0f;  // 속도 반전 (완전탄성)
	}*/
	for (auto obj : objects) {
		obj->update(deltaTime);
	}

	glutPostRedisplay();
	glutTimerFunc(16, TimerFunction, 0);  // 다음 타이머 등록
}
