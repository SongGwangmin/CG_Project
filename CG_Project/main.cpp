#include <glew.h>
#include <freeglut.h>
#include <freeglut_ext.h> 

#include <iostream> // Î°úÍ∑∏ ??
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

// ?ÑÏó≠ Î≥Ä??
std::vector<float> vertices;
GLuint VAO, VBO;

Mesh gSphere;  // Íµ?Î©îÏâ¨

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);  // ÍπäÏù¥ Î≤ÑÌçº Ï∂îÍ?
	glutInitWindowPosition(100, 50);
	glutInitWindowSize(width, height);
	glutCreateWindow("ComputerGraphics_Prject");

	glewExperimental = GL_TRUE;
	glewInit();

	// ÏΩúÎ∞± ?†Ïñ∏
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);

	glEnable(GL_DEPTH_TEST); // ÍπäÏù¥ ?åÏä§???úÏÑ±??


	make_vertexShaders();
	make_fragmentShaders();
	shaderProgramID = make_shaderProgram();

	// obj ?åÏùº Î°úÎìú
	if (!LoadOBJ_PosNorm_Interleaved("sphere.obj", gSphere))
	{
		std::cerr << "Failed to load sphere.obj\n";
		return 1;
	}

	// Î≤ÑÌçº ?∏ÌåÖ
	setupBuffers();

	// ?ïÏ†ê ?∞Ïù¥???§Ï†ï (?ºÍ∞Å???àÏ†ú)
	vertices = {
		// ?ÑÏπò              // ?âÏÉÅ
		 0.0f,  0.5f, -2.0f,  1.0f, 0.0f, 0.0f,  // ?ÑÏ™Ω ?ïÏ†ê (Îπ®Í∞ï)
		-0.5f, -0.5f, -2.0f,  0.0f, 1.0f, 0.0f,  // ?ºÏ™Ω ?ïÏ†ê (Ï¥àÎ°ù)
		 0.5f, -0.5f, -2.0f,  0.0f, 0.0f, 1.0f   // ?§Î•∏Ï™??ïÏ†ê (?åÎûë)
	};

	// Î©îÏù∏ Î£®ÌîÑ ÏßÑÏûÖ
	glutMainLoop();

	return 0;
}

// Íµ?Í∑∏Î¶¨???®Ïàò
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

	// ?∞Ïù¥???¨Ïö©
	glUseProgram(shaderProgramID);

	GLint lightOnLoc = glGetUniformLocation(shaderProgramID, "lightOn");
	glUniform1i(lightOnLoc, 1); // Ï°∞Î™Ö ÏºúÍ∏∞

	// Ï°∞Î™Ö/Í∞ùÏ≤¥ ???§Ï†ï
	GLint lightLoc = glGetUniformLocation(shaderProgramID, "lightColor");
	GLint objLoc = glGetUniformLocation(shaderProgramID, "objectColor");

	glm::vec3 lightBasePos(3.0f, 0.0f, 2.5f);
	glm::mat4 lightRotate = glm::rotate(glm::mat4(1.0f), glm::radians(-40.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec3 lightPos = glm::vec3(lightRotate* glm::vec4(lightBasePos, 1.0f));

	GLint uLightPos = glGetUniformLocation(shaderProgramID, "lightPos");  // Ï°∞Î™Ö ?ÑÏπò
	GLuint viewPosLoc = glGetUniformLocation(shaderProgramID, "viewPos");    // Ïπ¥Î©î???ÑÏπò
	glUniform3f(lightLoc, 1.0f, 1.0f, 1.0f);      // ??Ï°∞Î™Ö
	glUniform3f(objLoc, 1.0f, 0.7f, 0.7f);      // ?§Î∏å?ùÌä∏ ??
	glUniform3f(uLightPos, lightPos.x, lightPos.y, lightPos.z); // Ï°∞Î™Ö ?ÑÏπò

	GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
	GLint projLoc = glGetUniformLocation(shaderProgramID, "projection");
	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);  // Ïπ¥Î©î???ÑÏπò ?ÑÎã¨

	// x√‡ ±‚¡ÿ -40µµ »∏¿¸ ( ¿ßø°º≠ æ∆∑°∑Œ ∫∏¥¬ ∞¢µµ )
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(-40.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	cameraPos = glm::vec3(rotation * glm::vec4(cameraPos - cameraDirection, 1.0f)) + cameraDirection;

	glm::mat4 vTransform = glm::mat4(1.0f);
	vTransform = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &vTransform[0][0]);

	glm::mat4 mTransform = glm::mat4(1.0f);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &mTransform[0][0]);

	glm::mat4 pTransform = glm::mat4(1.0f);
	pTransform = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, &pTransform[0][0]);

	Player player;  // () ?úÍ±∞
	Object* obj = &player;

	// Ï§ëÏã¨ Íµ?
	glm::mat4 centerM = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f));
	centerM = glm::scale(centerM, glm::vec3(1.5f, 1.5f, 1.5f));
	DrawSphere(gSphere, shaderProgramID, centerM, glm::vec3(0.8f, 0.0f, 0.0f));


	// VBO ?∞Ïù¥??Î∞îÏù∏??
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

	// ?ïÏ†ê ?çÏÑ± ?§Ï†ï: ?ÑÏπò (3Í∞? + ?âÏÉÅ (3Í∞? = Ï¥?6Í∞?float
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}
