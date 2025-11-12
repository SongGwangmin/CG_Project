#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
	void move(float dx, float dy);
	void damaged(float damage);  // 데미지 입음
};



class Bullet : public Object
{
private:
	float damage;
};

