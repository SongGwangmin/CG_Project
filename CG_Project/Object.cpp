#include <glew.h>
#include <freeglut.h>
#include <freeglut_ext.h>
#include <vector>
#include <limits>
#include <algorithm>

#include <iostream>

#include "Object.h"

void Object::update()
{

}

void Player::move(float deltaTime)
{
	// 키 상태에 따라 방향 계산
	float dirx = 0.0f;
	float diry = 0.0f;

	if (left_keydown) dirx -= 1.0f;
	if (right_keydown) dirx += 1.0f;
	if (up_keydown) diry += 1.0f;
	if (down_keydown) diry -= 1.0f;

	// 이동 실행 (deltaTime 기반)
	if (dirx != 0.0f || diry != 0.0f)
	{
		position.x += dirx * speed * deltaTime;
		position.y += diry * speed * deltaTime;
	}
}

void Player::damaged(float damage)
{
	if (currentHp - damage < 0)
		currentHp = 0;

	if (currentHp >= 0)
		currentHp -= damage;
}

void Player::render(GLuint& shaderProgramID, GLuint& VAO, GLuint& VBO, std::vector<float>& vertices)
{
	// Transform 설정 - scale 멤버 변수 사용
	glm::mat4 modelTransform = glm::mat4(1.0f);
	modelTransform = glm::translate(modelTransform, position);
	modelTransform = glm::scale(modelTransform, scale);
	modelTransform = glm::rotate(modelTransform, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // x축 기준 -90도 회전
	GLint modelLoc = glGetUniformLocation(shaderProgramID, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &modelTransform[0][0]);

	// 색상 설정
	GLint colorLoc = glGetUniformLocation(shaderProgramID, "objectColor");
	glUniform3f(colorLoc, color.x, color.y, color.z);

	// VAO와 VBO는 이미 main.cpp에서 바인드되어 있으므로
	// 바로 그리기만 하면 됨
	// gPlayer.count를 외부에서 받아야 하지만, 
	// Mesh 구조를 보면 airplane.obj는 대략 8448개 정점
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glDrawArrays(GL_TRIANGLES, 0, 8448);
	glBindVertexArray(0);
}

void Bullet::update_first_paze(float deltaTime)
{
	// z축 이동
	if (position.z < -40.0f)
		position.z += 5.0f * deltaTime;

	// 중력 적용
	vy += -9.8f * deltaTime; // 9.8f is gravity acceleration

	// y 위치 업데이트
	position.y += vy * deltaTime;

	// 바닥 충돌 체크 (완전탄성 충돌)
	if (position.y <= -20.0f) { // groundY = -20.0f
		position.y = -20.0f;  // 바닥 위치로 보정
		vy *= -1.0f;  // 속도 반전 (완전탄성)
		if (vy < 29.2f) {
			vy = 29.2f; // 최소 반발 속도 설정
		}
	}

	if (-40.0f <= position.z) {
		position.z = -90.0f; // reset position
	}
}

void Bullet::update_second_paze(float deltaTime)
{
	position.z += 50.0f;

	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(20.0f * deltaTime), glm::vec3(1.0f, 0.0f, 0.0f));
	glm::vec4 rotatedPos = rotationMatrix * glm::vec4(position, 1.0f);
	rotatedPos.z -= 50.0f;
	position = glm::vec3(rotatedPos);
}


void Bullet::render(GLuint& shaderProgramID, GLuint& VAO, GLuint& VBO, std::vector<float>& vertices) // 렌더링 할 때 넘겨줘야 하는 값들 - shaderProgramID, VAO, VBO, vertices, 정점 개수
{
	// shpere's radius = 1.0f, scale = 1.5f -> actual radius = 1.5f
	unsigned int modelLocation = glGetUniformLocation(shaderProgramID, "model");
	glm::mat4 modelTransform = glm::mat4(1.0f);
	modelTransform = glm::translate(modelTransform, position);
	//glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]);


	// bulletModel
	glm::mat4 bulletModel = glm::translate(glm::mat4(1.0f), position);  // sphere position
	bulletModel = glm::scale(bulletModel, scale);   // sphere scale

	modelTransform = bulletModel;
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &modelTransform[0][0]);

	unsigned int colorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
	glUniform3f(colorLocation, color.x, color.y, color.z);


	glDrawArrays(GL_TRIANGLES, 0, 960);

}

bool Bullet::collide(const glm::mat4& view, const glm::mat4& proj, Player& player)
{
	// -------------------------------------------------------
	// 1. 총알 (Bullet) 투영 -> 화면상 영역(타원) 계산
	// -------------------------------------------------------
	glm::vec4 bulletPos = glm::vec4(position, 1.0f);
	glm::vec4 bulletViewPos = view * bulletPos;

	// 카메라 뒤에 있거나 너무 가까우면 무시
	if (bulletViewPos.z >= -0.1f) return false;

	float bulletDepth = -bulletViewPos.z; // 양수 깊이

	// 총알 중심점 NDC 변환
	float bx_ndc = bulletViewPos.x * proj[0][0] / bulletDepth;
	float by_ndc = bulletViewPos.y * proj[1][1] / bulletDepth;

	// 총알의 화면상 반지름 (Radius) 계산
	float b_radius_x_ndc = scale.x * proj[0][0] / bulletDepth / 2;
	float b_radius_y_ndc = scale.y * proj[1][1] / bulletDepth / 2;

	// -------------------------------------------------------
	// 2. 플레이어 바운딩 박스들을 NDC 좌표로 변환
	// -------------------------------------------------------
	glm::vec3 playerPosWorld = player.getPosition();
	glm::vec3 playerScale = player.getScale();
	
	// 플레이어의 모델 변환 행렬 계산
	glm::mat4 playerModelMatrix = glm::mat4(1.0f);
	playerModelMatrix = glm::translate(playerModelMatrix, playerPosWorld);
	playerModelMatrix = glm::scale(playerModelMatrix, playerScale);
	playerModelMatrix = glm::rotate(playerModelMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // x축 기준 90도 회전
	
	// 바운딩 박스들 가져오기
	auto boundingBoxes = player.getBoundingBoxes();
	
	// -------------------------------------------------------
	// 3. 각 바운딩 박스와 충돌 검사
	// -------------------------------------------------------
	for (const auto& box : boundingBoxes)
	{
		// 바운딩 박스의 4개 꼭지점을 월드 좌표로 변환
		glm::vec4 corners[4] = {
			playerModelMatrix * glm::vec4(box.min.x, box.min.y, 0.0f, 1.0f), // 좌하단
			playerModelMatrix * glm::vec4(box.max.x, box.min.y, 0.0f, 1.0f), // 우하단
			playerModelMatrix * glm::vec4(box.min.x, box.max.y, 0.0f, 1.0f), // 좌상단
			playerModelMatrix * glm::vec4(box.max.x, box.max.y, 0.0f, 1.0f)  // 우상단
		};
		
		// 4개 꼭지점을 NDC로 변환
		float box_min_x_ndc = std::numeric_limits<float>::max();
		float box_max_x_ndc = std::numeric_limits<float>::lowest();
		float box_min_y_ndc = std::numeric_limits<float>::max();
		float box_max_y_ndc = std::numeric_limits<float>::lowest();
		
		bool allBehindCamera = true;
		
		for (int i = 0; i < 4; ++i)
		{
			glm::vec4 cornerView = view * corners[i];
			
			// 카메라 앞에 있는지 확인
			if (cornerView.z < -0.1f)
			{
				allBehindCamera = false;
				float cornerDepth = -cornerView.z;
				
				// NDC 변환
				float cx_ndc = cornerView.x * proj[0][0] / cornerDepth;
				float cy_ndc = cornerView.y * proj[1][1] / cornerDepth;
				
				// AABB 계산
				box_min_x_ndc = std::min(box_min_x_ndc, cx_ndc);
				box_max_x_ndc = std::max(box_max_x_ndc, cx_ndc);
				box_min_y_ndc = std::min(box_min_y_ndc, cy_ndc);
				box_max_y_ndc = std::max(box_max_y_ndc, cy_ndc);
			}
		}
		
		// 모든 꼭지점이 카메라 뒤에 있으면 다음 박스로
		if (allBehindCamera) continue;
		
		// -------------------------------------------------------
		// 4. 원(타원)과 AABB 충돌 검사
		// -------------------------------------------------------
		// 원의 중심에서 박스까지의 가장 가까운 점 찾기
		float closest_x = std::max(box_min_x_ndc, std::min(bx_ndc, box_max_x_ndc));
		float closest_y = std::max(box_min_y_ndc, std::min(by_ndc, box_max_y_ndc));
		
		// 원의 중심에서 가장 가까운 점까지의 거리 계산 (타원 방정식 적용)
		float dx = bx_ndc - closest_x;
		float dy = by_ndc - closest_y;
		
		float x_term = dx / b_radius_x_ndc;
		float y_term = dy / b_radius_y_ndc;
		
		// 타원 내부에 가장 가까운 점이 있으면 충돌
		if ((x_term * x_term) + (y_term * y_term) <= 1.0f)
		{
			std::cout << "Collision detected with bounding box!" << std::endl;
			return true;
		}
	}

	return false;
}