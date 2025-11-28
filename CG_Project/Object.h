#pragma once
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <glew.h>

class Object
{
protected:
	float currentHp;
	float maxHp; 

	float speed;
	glm::vec3 position;
	glm::vec3 cameraPosition;
	glm::vec3 cameraTarget;
	glm::vec3 cameraUp;
	glm::vec3 color;

	glm::vec3 velocity;  // not same speed
	glm::vec3 scale;
	
public:
	void update();
	virtual void render(GLuint& shaderProgramID, GLuint& VAO, GLuint& VBO, std::vector<float>& vertices) = 0;
	void setPosition(glm::vec3 pos)
	{
		position = pos;
	}
	glm::vec3 getPosition()
	{
		return position;
	}
	void setVelocity(glm::vec3 vel)
	{
		velocity = vel;        
		speed = glm::length(vel);
	}
	glm::vec3 getVelocity()
	{
		return velocity;
	}
	void setColor(glm::vec3 color)
	{
		// 색상 설정 (필요시 구현)
		this->color = color;
	}
	glm::vec3 getColor()
	{
		return color;
	}
	void setScale(glm::vec3 scale)
	{
		this->scale = scale;
	}
	glm::vec3 getScale()
	{
		return scale;
	}
	float getMaxHp()
	{
		return maxHp;
	}
	float getCurrentHp()
	{
		return currentHp;
	}
	void setCurrentHp(float hp)
	{
		currentHp = hp;
	}
	void setMaxHp(float hp)
	{
		maxHp = hp;
	}
};



class Player : public Object
{
private:
	float power;
	int left_keydown;
	int right_keydown;
	int up_keydown;
	int down_keydown;
public:
	Player()
	{
		// 기본값 설정
		maxHp = 1000.0f;
		currentHp = maxHp;
		speed = 10.0f;
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		power = 10.0f;
		left_keydown = 0;
		right_keydown = 0;
		up_keydown = 0;
		down_keydown = 0;
	}
	void move(float deltaTime);
	void render(GLuint& shaderProgramID, GLuint& VAO, GLuint& VBO, std::vector<float>& vertices) override;
	void damaged(float damage);  // 데미지 입음
	
	// 키 입력 상태를 1로 설정하는 함수들
	void setLeftKeyDown() { left_keydown = 1; }
	void setRightKeyDown() { right_keydown = 1; }
	void setUpKeyDown() { up_keydown = 1; }
	void setDownKeyDown() { down_keydown = 1; }
	
	// 키 입력 상태를 0으로 설정하는 함수들
	void resetLeftKeyDown() { left_keydown = 0; }
	void resetRightKeyDown() { right_keydown = 0; }
	void resetUpKeyDown() { up_keydown = 0; }
	void resetDownKeyDown() { down_keydown = 0; }
	
	// 키 입력 상태를 가져오는 함수들
	int getLeftKeyDown() const { return left_keydown; }
	int getRightKeyDown() const { return right_keydown; }
	int getUpKeyDown() const { return up_keydown; }
	int getDownKeyDown() const { return down_keydown; }
	
	// 바운딩 박스 정보 반환 (모델링 좌표계)
	// 3개의 바운딩 박스: 하단, 중단, 상단
	struct BoundingBox {
		glm::vec2 min; // (x_min, y_min)
		glm::vec2 max; // (x_max, y_max)
	};
	
	std::vector<BoundingBox> getBoundingBoxes() const {
		return {
			{{-0.189874f, -2.768519f}, {0.189874f, 1.6254f}},   // 하단 박스 -1.6254 m
			{{-4.250606f, 0.247049f}, {4.250606f, 2.768519f}},     // 중단 박스
			{{-3.591504f, 2.768519f}, {3.591504f, 4.215462f}}      // 상단 박스
		};
	}
};



class Bullet : public Object
{
private:
	float damage;
	float vy = 0.0f; // y축 속도 (중력 적용용)
	static int sharedMeshCount;  // 공유 메쉬의 vertex count

public:
	Bullet()
	{
		damage = 10.0f;
		vy = 0.0f;
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		color = glm::vec3(1.0f, 1.0f, 1.0f);
		scale = glm::vec3(1.5f, 1.5f, 1.5f);
	}

	// 모든 Bullet이 공유할 메쉬 정보 설정
	static void SetSharedMesh(int count)
	{
		sharedMeshCount = count;
	}

	void render(GLuint& shaderProgramID, GLuint& VAO, GLuint& VBO, std::vector<float>& vertices) override;
	void update_first_paze(float deltaTime);
	void update_second_paze(float deltaTime);
	bool collide(const glm::mat4& view, const glm::mat4& proj, Player& player);
};

