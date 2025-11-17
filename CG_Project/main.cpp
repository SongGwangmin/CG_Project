#include <glew.h>
#include <freeglut.h>
#include <freeglut_ext.h> 

#include <iostream> // 로그 용
#include <vector>

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

// 전역 변수
std::vector<float> vertices;
GLuint VAO, VBO;

Mesh gSphere;  // 구 메쉬

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);  // 깊이 버퍼 추가
	glutInitWindowPosition(100, 50);
	glutInitWindowSize(width, height);
	glutCreateWindow("ComputerGraphics_Prject");

	glewExperimental = GL_TRUE;
	glewInit();

	// 콜백 선언
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);

	glEnable(GL_DEPTH_TEST); // 깊이 테스트 활성화


	make_vertexShaders();
	make_fragmentShaders();
	shaderProgramID = make_shaderProgram();

	// obj 파일 로드
	if (!LoadOBJ_PosNorm_Interleaved("sphere.obj", gSphere))
	{
		std::cerr << "Failed to load sphere.obj\n";
		return 1;
	}

	// 버퍼 세팅
	setupBuffers();

	// 정점 데이터 설정 (삼각형 예제)
	vertices = {
		// 위치              // 색상
		 0.0f,  0.5f, -2.0f,  1.0f, 0.0f, 0.0f,  // 위쪽 정점 (빨강)
		-0.5f, -0.5f, -2.0f,  0.0f, 1.0f, 0.0f,  // 왼쪽 정점 (초록)
		 0.5f, -0.5f, -2.0f,  0.0f, 0.0f, 1.0f   // 오른쪽 정점 (파랑)
	};

	// 메인 루프 진입
	glutMainLoop();

	return 0;
}

// 구 그리는 함수
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
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 셰이더 사용
	glUseProgram(shaderProgramID);

	GLint lightOnLoc = glGetUniformLocation(shaderProgramID, "lightOn");
	glUniform1i(lightOnLoc, 1); // 조명 켜기

	// 조명/객체 색 설정
	GLint lightLoc = glGetUniformLocation(shaderProgramID, "lightColor");
	GLint objLoc = glGetUniformLocation(shaderProgramID, "objectColor");

	glm::vec3 lightBasePos(3.0f, 0.0f, 2.5f);
	//glm::mat4 lightRotate = glm::rotate(glm::mat4(1.0f), glm::radians(angleCenterY), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::vec3 lightPos = glm::vec3(glm::vec4(lightBasePos, 1.0f));

	GLint uLightPos = glGetUniformLocation(shaderProgramID, "lightPos");  // 조명 위치
	GLuint viewPosLoc = glGetUniformLocation(shaderProgramID, "viewPos");    // 카메라 위치
	glUniform3f(lightLoc, 1.0f, 1.0f, 1.0f);      // 흰 조명
	glUniform3f(objLoc, 1.0f, 0.7f, 0.7f);      // 오브젝트 색
	glUniform3f(uLightPos, lightPos.x, lightPos.y, lightPos.z); // 조명 위치

	GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
	GLint projLoc = glGetUniformLocation(shaderProgramID, "projection");
	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);  // 카메라 위치 전달

	glm::mat4 vTransform = glm::mat4(1.0f);
	vTransform = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &vTransform[0][0]);

	glm::mat4 mTransform = glm::mat4(1.0f);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &mTransform[0][0]);

	glm::mat4 pTransform = glm::mat4(1.0f);
	pTransform = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &pTransform[0][0]);

	Player player;  // () 제거
	Object* obj = &player;

	// 중심 구
	glm::mat4 centerM = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f));
	centerM = glm::scale(centerM, glm::vec3(1.5f, 1.5f, 1.5f));
	DrawSphere(gSphere, shaderProgramID, centerM, glm::vec3(0.8f, 0.0f, 0.0f));


	// VBO 데이터 바인딩
	if (!vertices.empty()) {
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
			vertices.data(), GL_STATIC_DRAW);

		//glDrawArrays(GL_TRIANGLES, 0, 3);
		obj->render(shaderProgramID, VAO, VBO, vertices);

		glBindVertexArray(0);
	}

	
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

	// 정점 속성 설정: 위치 (3개) + 색상 (3개) = 총 6개 float
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}
