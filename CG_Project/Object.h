#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <GL/glew.h>

class Object
{
protected:
	float hp;
	float speed;
	int dirX;  // 좌우 이동 방향
	int dirY;  // 상하 이동 방향
	glm::vec3 position;
	glm::vec3 cameraPosition;
	glm::vec3 cameraTarget;
	glm::vec3 cameraUp;
	
public:
	void update();
	virtual void render(GLuint& shaderProgramID, GLuint& VAO, GLuint& VBO, std::vector<float>& vertices) = 0;
	void setPosition(float x, float y, float z)
	{
		position = glm::vec3(x, y, z);
	}
};



class Player : public Object
{
private:
	float power;
public:
	Player()
	{
		// 기본값 설정
		hp = 100.0f;
		speed = 0.1f;
		dirX = 0;
		dirY = 0;
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		power = 10.0f;
	}
	void move(float dx, float dy);
	void render(GLuint& shaderProgramID, GLuint& VAO, GLuint& VBO, std::vector<float>& vertices) override;
	void damaged(float damage);  // 데미지 입음
};



class Bullet : public Object
{
private:
	float damage;
public:
	void render(GLuint& shaderProgramID, GLuint& VAO, GLuint& VBO, std::vector<float>& vertices) override {
		// Bullet의 렌더링 구현
	}
};

