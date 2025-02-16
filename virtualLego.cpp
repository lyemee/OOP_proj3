#include "d3dUtility.h"
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <Windows.h>
#include <d3dx9.h>

IDirect3DDevice9* Device = NULL; //Direct3D 디바이스 포인터 초기화

// window size
const int Width = 1024;
const int Height = 768;

// There are four balls
// initialize the position (coordinate) of each ball (ball0 ~ ball3) 각 공의 초기 위치 설정
const float spherePos[4][2] = { {-2.7f,0} , {+2.4f,0} , {3.3f,0} , {-2.7f,-0.9f} };
// initialize the color of each ball (ball0 ~ ball3) 각 공의 색상 초기화
const D3DXCOLOR sphereColor[4] = { d3d::RED, d3d::RED, d3d::YELLOW, d3d::WHITE };

// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
D3DXMATRIX g_mWorld; //월드 행렬
D3DXMATRIX g_mView; //뷰 행렬
D3DXMATRIX g_mProj; //투영 행렬

#define M_RADIUS 0.21   // ball radius 공의 반지름
#define PI 3.14159265 //원주율
#define M_HEIGHT 0.01 //공의 높이
#define DECREASE_RATE 0.9982 //속도 감소율(시간에 따라 속도가 줄어듦)

// -----------------------------------------------------------------------------
// CSphere class definition
// -----------------------------------------------------------------------------

class CSphere {
private:
	float center_x, center_y, center_z; //공의 위치
	float m_radius; //공의 반지름
	float m_velocity_x; //공의 x축 속도
	float m_velocity_z; //공의 z축 속도

public:
	CSphere(void)
	{
		D3DXMatrixIdentity(&m_mLocal); //로컬 변환 행렬을 단위 행렬로 초기화
		ZeroMemory(&m_mtrl, sizeof(m_mtrl)); //재질 속성을 0으로 초기화
		m_radius = 0; //반지름 초기화
		m_velocity_x = 0; //x축 속도 초기화
		m_velocity_z = 0; //z축 속도 초기화
		m_pSphereMesh = NULL; //구 메시를 NULL로 초기화
	}
	~CSphere(void) {} //소멸자

public:
	//create 함수:
	//pDevice가 NULL인지 확인하여, 올바른 디바이스가 제공되었는지 검사
	// 메시의 재질을 설정, color 매개변수로 전달된 색상을 사용하여, Ambient, Diffuse, Specular 속성을 모두 설정
	// D3DXCreateSphere를 사용하여 구 메시를 생성하며, 구의 반지름과 분할 정도를 설정, 생성에 실패하면 false 반환
	//
	//주어진 색상으로 구 메시 생성
	bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE)
	{
		if (NULL == pDevice) //pDevice가 NULL이면 실패
			return false;

		m_mtrl.Ambient = color; //주변광 색상 설정
		m_mtrl.Diffuse = color; //확산광 색상 설정
		m_mtrl.Specular = color; //반사광 색상 설정
		m_mtrl.Emissive = d3d::BLACK; //방출광 색상 설정
		m_mtrl.Power = 5.0f; //재질의 반사 강도 설정

		//구 메시 생성 : 반지름, 스플릿, 스택 설정, pDevice는 Direct3D 디바이스 포인터
		if (FAILED(D3DXCreateSphere(pDevice, getRadius(), 50, 50, &m_pSphereMesh, NULL)))
			return false; //메시 생성 실패 시 false 반환
		return true; //메시 생성 성공 시 true 반환
	}

	//destroy 함수:
	//메시가 이미 할당되어 있는 경우, Release를 호출하여 메시 리소스를 해제하고, 포인터를 NULL로 설정
	// 
	//공 메시를 해제하는 함수
	void destroy(void)
	{
		//메시가 NULL이 아닌 경우에만 해제
		if (m_pSphereMesh != NULL) {
			m_pSphereMesh->Release(); //메시 리소스 해제
			m_pSphereMesh = NULL; //메시 포인터를 NULL로 설정
		}
	}

	//draw 함수:
	//pDevice가 NULL인지 확인하여 그릴 수 없으면 함수를 종료
	//SetTransform과 MultiplyTransform을 사용하여 공의 월드 변환 행렬을 설정, 여기서는 공의 위치 및 회전 등의 변환을 적용
	//공의 재질을 설정하고, DrawSubset을 호출하여 구 메시를 화면에 그린다, DrawSubset(0)은 첫 번째 서브셋을 그리는 호출
	//공을 그리는 함수
	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		//pDevice가 NULL이면 그릴 수 없으므로 함수를 종료
		if (NULL == pDevice)
			return;
		pDevice->SetTransform(D3DTS_WORLD, &mWorld); //월드 변환 행렬 설정 (공의 위치와 변화를 설정)
		pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal); //로컬 변환 행렬을 월드 변환 행렬에 곱함 (상대적인 위치 설정)
		pDevice->SetMaterial(&m_mtrl); //공의 재질 설정
		m_pSphereMesh->DrawSubset(0); //구 메시를 그리기 (첫 번째 서브셋을 사용)
	}

	bool hasIntersected(CSphere& ball)
	{
		// check if there is collision between two spheres
		D3DXVECTOR3 myCenter = this->getCenter();
		D3DXVECTOR3 otherCenter = ball.getCenter();

		float dx = myCenter.x - otherCenter.x;
		float dz = myCenter.z - otherCenter.z;

		// 두 구의 중심 간 거리 계산
		float distance = sqrt(dx * dx + dz * dz);

		// 거리가 두 반지름의 합보다 작거나 같으면 충돌
		if (distance <= (this->getRadius() + ball.getRadius())) {
			return true;
		}
		else return false;
	}

	void hitBy(CSphere& ball)
	{
		// what needs to be done if there is collision between two spheres.
		// 두 공의 중심과 속도
		D3DXVECTOR3 myCenter = this->getCenter();
		D3DXVECTOR3 otherCenter = ball.getCenter();

		D3DXVECTOR3 myVelocity = D3DXVECTOR3(this->getVelocity_X(), 0, this->getVelocity_Z());
		D3DXVECTOR3 otherVelocity = D3DXVECTOR3(ball.getVelocity_X(), 0, ball.getVelocity_Z());

		// 두 공 사이의 방향 벡터 계산
		D3DXVECTOR3 collisionNormal = myCenter - otherCenter;
		float distance = D3DXVec3Length(&collisionNormal);

		if (hasIntersected(ball)) {
			// 충돌 감지: 공의 중심 간의 거리가 반지름의 합보다 작거나 같으면 충돌
			// 방향 벡터 정규화
			D3DXVec3Normalize(&collisionNormal, &collisionNormal);

			// 상대 속도 계산
			D3DXVECTOR3 relativeVelocity = myVelocity - otherVelocity;

			// 충돌 방향에서의 상대 속도 스칼라 계산
			float velocityAlongNormal = D3DXVec3Dot(&relativeVelocity, &collisionNormal);

			// 충돌 조건: 상대 속도가 0보다 작을 경우만 처리
			if (velocityAlongNormal > 0) {
				return;
			}

			// 충격량 계산 (비탄성 충돌 공식 적용)
			float e = 0.9f;
			float impulseMagnitude = -(1 + e) * velocityAlongNormal / 2.0f;

			// 충격량 벡터
			D3DXVECTOR3 impulse = impulseMagnitude * collisionNormal;

			// 속도 업데이트
			D3DXVECTOR3 newmyVelocity = myVelocity + impulse;
			D3DXVECTOR3 newotherVelocity = otherVelocity - impulse;

			this->setPower(newmyVelocity.x, newmyVelocity.z);
			ball.setPower(newotherVelocity.x, newotherVelocity.z);
		}
	}

	//ballUpdate 함수:
	//timeDiff를 사용하여 공의 위치를 업데이트, 공이 이동할 때 X, Z 방향으로 위치를 게산하고, 속도가 일정 이하로 떨어지면 속도를 0으로 설정
	//공의 속도 감소를 처리하는 부분에서, DECREASE_RATE에 따라 속도를 점차 줄여 나간다
	//공의 상태를 시간 차이에 따라 업데이트하는 함수
	void ballUpdate(float timeDiff)
	{
		const float TIME_SCALE = 3.3; //시간 스케일을 위한 상수(속도 조정)
		D3DXVECTOR3 cord = this->getCenter(); //공의 현재 위치를 가져온다
		//공의 X, Z 방향 속도 값을 절댓값으로 가져온다
		double vx = abs(this->getVelocity_X());
		double vz = abs(this->getVelocity_Z());

		//공의 속도가 일정 값 이상이면, 공의 위치를 업데이트
		if (vx > 0.01 || vz > 0.01)
		{
			//공의 X, Z 방향의 새로운 위치 계산
			float tX = cord.x + TIME_SCALE * timeDiff * m_velocity_x;
			float tZ = cord.z + TIME_SCALE * timeDiff * m_velocity_z;

			//correction of position of ball
			//공이 벽에 충돌할 때 위치를 보정하는 부분
			// Please uncomment this part because this correction of ball position is necessary when a ball collides with a wall
			//이 부분을 해제하면 벽에 충돌 시 공이 벽을 넘지 않도록 조정한다
			if (tX >= (4.5 - M_RADIUS))
				tX = 4.5 - M_RADIUS;
			else if (tX <= (-4.5 + M_RADIUS))
				tX = -4.5 + M_RADIUS;
			else if (tZ <= (-3 + M_RADIUS))
				tZ = -3 + M_RADIUS;
			else if (tZ >= (3 - M_RADIUS))
				tZ = 3 - M_RADIUS;

			this->setCenter(tX, cord.y, tZ); //새로운 위치를 공의 중심에 설정
		}
		else {
			this->setPower(0, 0); //공이 멈췄을 때 속도를 0으로 설정 (속도가 너무 작으면 멈춘 것으로 간주)
		}

		//this->setPower(this->getVelocity_X() * DECREASE_RATE, this->getVelocity_Z() * DECREASE_RATE);
		double rate = 1 - (1 - DECREASE_RATE) * timeDiff * 400; //공의 속도 감소(속도 감소율을 기반으로 적용), DECREASE_RATE에 따른 속도 감소율 계산

		//속도 감소율이 0보다 작으면 0으로 설정
		if (rate < 0)
			rate = 0;
		this->setPower(getVelocity_X() * rate, getVelocity_Z() * rate); //새로운 속도를 설정
	}

	double getVelocity_X() { return this->m_velocity_x; } //공의 X 방향 속도를 반환하는 함수
	double getVelocity_Z() { return this->m_velocity_z; } //공의 Z 방향 속도를 반환하는 함수

	//공의 속도를 설정하는 함수
	void setPower(double vx, double vz)
	{
		this->m_velocity_x = vx; //X 방향 속도 설정
		this->m_velocity_z = vz; //Z 방향 속도 설정
	}

	//setCenter 함수:
	//설정된 위치를 기준으로 D3DMatrixTranslation을 사용하여 변환 행렬을 생성하고 이를 로컬 변환으로 설정
	//공의 중심 위치를 설정하는 함수
	void setCenter(float x, float y, float z)
	{
		D3DXMATRIX m;
		center_x = x;	center_y = y;	center_z = z; //공의 X, Y, Z 위치 설정
		//위치 변환 행렬을 만들어 로컬 변환 설정
		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}

	float getRadius(void)  const { return (float)(M_RADIUS); } //공의 반지름을 반환하는 함수, M_RADIUS는 공의 고정된 반지름 값을 나타내는 상수
	const D3DXMATRIX& getLocalTransform(void) const { return m_mLocal; } //로컬 변환 행렬을 반환하는 함수
	void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; } //로컬 변환 행렬을 설정하는 함수
	//공의 중심 위치를 반환하는 함수
	D3DXVECTOR3 getCenter(void) const
	{
		//공의 현재 중심 위치를 D3DXVECTOR3로 반환
		D3DXVECTOR3 org(center_x, center_y, center_z);
		return org;
	}

private:
	D3DXMATRIX              m_mLocal;
	D3DMATERIAL9            m_mtrl;
	ID3DXMesh* m_pSphereMesh;
};



// -----------------------------------------------------------------------------
// CWall class definition
// -----------------------------------------------------------------------------

class CWall {

private:

	float m_x; //벽의 X 좌표
	float m_z; //벽의 Z 좌표
	float m_width; //벽의 너비
	float m_depth; //벽의 두께
	float m_height; //벽의 높이

public:
	//생성자: 벽 객체 초기화
	CWall(void)
	{
		D3DXMatrixIdentity(&m_mLocal); //로컬 변환 행렬 초기화(단위 행렬로 설정)
		ZeroMemory(&m_mtrl, sizeof(m_mtrl)); //재질 초기화
		m_width = 0;
		m_depth = 0;
		m_pBoundMesh = NULL; //메쉬 포인터 초기화
	}
	~CWall(void) {} //소멸자
public:
	//벽을 생성하는 함수
	//벽의 크기와 색상을 받아 메쉬를 생성, D3DXCreateBox를 사용하여 박스 형태의 메쉬를 생성하며, 벽의 재질도 설정
	bool create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE)
	{
		if (NULL == pDevice)
			return false; //디바이스가 NULL이면 실패

		//벽의 재질 설정
		m_mtrl.Ambient = color; //주변광 색상
		m_mtrl.Diffuse = color; //확산광 색상
		m_mtrl.Specular = color; //반사광 색상
		m_mtrl.Emissive = d3d::BLACK; //방출광 색상(없음)
		m_mtrl.Power = 5.0f; //재질의 반사력

		//벽의 크기 설정
		m_width = iwidth;
		m_depth = idepth;

		//D3DXCreateBox를 사용해 벽의 메쉬 생성
		if (FAILED(D3DXCreateBox(pDevice, iwidth, iheight, idepth, &m_pBoundMesh, NULL)))
			return false; //메쉬 생성 실패 시 false 생성
		return true; //성공적으로 벽을 생성
	}

	//벽 메쉬를 삭제하고 메모리를 해제하는 함수
	void destroy(void)
	{
		if (m_pBoundMesh != NULL) {
			m_pBoundMesh->Release(); //메쉬 리소스 해제
			m_pBoundMesh = NULL; //메쉬 포인터 NULL로 설정
		}
	}

	//벽을 그리는(렌더링하는) 함수
	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		if (NULL == pDevice)
			return; //디바이스가 NULL이면 그리기 중지
		pDevice->SetTransform(D3DTS_WORLD, &mWorld); //월드 변환 행렬 생성
		pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal); //로컬 변환 행렬을 월드 변환 행렬에 곱하여 적용
		pDevice->SetMaterial(&m_mtrl); //재질을 설정
		m_pBoundMesh->DrawSubset(0); //메쉬를 그린다
	}

	bool hasIntersected(CSphere& ball)
	{
		// check if there is collision between a sphere and a wall
		D3DXVECTOR3 ballCenter = ball.getCenter();
		float ballRadius = ball.getRadius();

		// 벽의 범위
		float wallLeft = m_x - m_width / 2.0f;
		float wallRight = m_x + m_width / 2.0f;
		float wallTop = m_z + m_depth / 2.0f;
		float wallBottom = m_z - m_depth / 2.0f;

		// 공과 벽의 충돌 여부 계산
		bool intersectsX = (ballCenter.x + ballRadius >= wallLeft) && (ballCenter.x - ballRadius <= wallRight);
		bool intersectsZ = (ballCenter.z + ballRadius >= wallBottom) && (ballCenter.z - ballRadius <= wallTop);

		// X축과 Z축 경계 중 하나라도 침범하면 충돌로 간주
		return intersectsX && intersectsZ;
	}

	void hitBy(CSphere& ball)
	{
		// 공의 중심 좌표와 속도 가져오기
		D3DXVECTOR3 ballCenter = ball.getCenter();
		float ballRadius = ball.getRadius();

		D3DXVECTOR3 ballVelocity = D3DXVECTOR3(ball.getVelocity_X(), 0, ball.getVelocity_Z());

		// 벽의 경계 계산
		float wallLeft = m_x - m_width / 2.0f;
		float wallRight = m_x + m_width / 2.0f;
		float wallTop = m_z + m_depth / 2.0f;
		float wallBottom = m_z - m_depth / 2.0f;

		if (hasIntersected(ball)) {
			// 벽과 충돌이 감지됨

			// 충돌 방향을 계산
			D3DXVECTOR3 collisionNormal(0, 0, 0);

			if (ballCenter.x - ballRadius < wallLeft) {
				collisionNormal = D3DXVECTOR3(-1, 0, 0); // 왼쪽 벽
			}
			else if (ballCenter.x + ballRadius > wallRight) {
				collisionNormal = D3DXVECTOR3(1, 0, 0); // 오른쪽 벽
			}
			else if (ballCenter.z - ballRadius < wallBottom) {
				collisionNormal = D3DXVECTOR3(0, 0, -1); // 아래쪽 벽
			}
			else if (ballCenter.z + ballRadius > wallTop) {
				collisionNormal = D3DXVECTOR3(0, 0, 1); // 위쪽 벽
			}

			// 벽 방향의 속도 계산
			float velocityAlongNormal = D3DXVec3Dot(&ballVelocity, &collisionNormal);

			// 충격량 계산 (벽은 정지 상태이므로 공만 영향을 받음)
			float e = 0.8f; // 반발 계수 (탄성 정도)
			float impulseMagnitude = -(1 + e) * velocityAlongNormal;

			// 충격량 벡터
			D3DXVECTOR3 impulse = impulseMagnitude * collisionNormal;

			// 공의 속도 업데이트
			D3DXVECTOR3 newBallVelocity = ballVelocity + impulse;
			ball.setPower(newBallVelocity.x, newBallVelocity.z);

			// 공의 위치 수정: 벽에 박히지 않도록 침투를 방지
			D3DXVECTOR3 correctedPosition = ballCenter;
			if (collisionNormal.x != 0) {
				// X축 충돌
				correctedPosition.x = collisionNormal.x < 0 ? wallLeft - ballRadius : wallRight + ballRadius;
			}
			if (collisionNormal.z != 0) {
				// Z축 충돌
				correctedPosition.z = collisionNormal.z < 0 ? wallBottom - ballRadius : wallTop + ballRadius;
			}

			ball.setCenter(correctedPosition.x, ballCenter.y, correctedPosition.z);
		}
	}


	//벽의 위치를 설정하는 함수
	void setPosition(float x, float y, float z)
	{
		D3DXMATRIX m;
		//벽의 X, Z 좌표를 업데이트
		this->m_x = x;
		this->m_z = z;

		D3DXMatrixTranslation(&m, x, y, z); //지정된 x, y, z 값에 대한 변환 행렬을 생성
		setLocalTransform(m); //생성된 변환 행렬을 로컬 변환 행렬에 설정
	}

	float getHeight(void) const { return M_HEIGHT; } //벽의 높이를 반환하는 함수



private:
	void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; } //로컬 변환 행렬을 설정하는 함수

	D3DXMATRIX              m_mLocal; //벽의 로컬 변환 행렬
	D3DMATERIAL9            m_mtrl; //벽의 재질
	ID3DXMesh* m_pBoundMesh; //벽을 나타내는 메쉬 객체
};

// -----------------------------------------------------------------------------
// CLight class definition
// -----------------------------------------------------------------------------

class CLight {
public:
	CLight(void)
	{
		static DWORD i = 0;
		m_index = i++;
		D3DXMatrixIdentity(&m_mLocal);
		::ZeroMemory(&m_lit, sizeof(m_lit));
		m_pMesh = NULL;
		m_bound._center = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_bound._radius = 0.0f;
	}
	~CLight(void) {}
public:
	bool create(IDirect3DDevice9* pDevice, const D3DLIGHT9& lit, float radius = 0.1f)
	{
		if (NULL == pDevice)
			return false;
		if (FAILED(D3DXCreateSphere(pDevice, radius, 10, 10, &m_pMesh, NULL)))
			return false;

		m_bound._center = lit.Position;
		m_bound._radius = radius;

		m_lit.Type = lit.Type;
		m_lit.Diffuse = lit.Diffuse;
		m_lit.Specular = lit.Specular;
		m_lit.Ambient = lit.Ambient;
		m_lit.Position = lit.Position;
		m_lit.Direction = lit.Direction;
		m_lit.Range = lit.Range;
		m_lit.Falloff = lit.Falloff;
		m_lit.Attenuation0 = lit.Attenuation0;
		m_lit.Attenuation1 = lit.Attenuation1;
		m_lit.Attenuation2 = lit.Attenuation2;
		m_lit.Theta = lit.Theta;
		m_lit.Phi = lit.Phi;
		return true;
	}
	void destroy(void)
	{
		if (m_pMesh != NULL) {
			m_pMesh->Release();
			m_pMesh = NULL;
		}
	}
	bool setLight(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		if (NULL == pDevice)
			return false;

		D3DXVECTOR3 pos(m_bound._center);
		D3DXVec3TransformCoord(&pos, &pos, &m_mLocal);
		D3DXVec3TransformCoord(&pos, &pos, &mWorld);
		m_lit.Position = pos;

		pDevice->SetLight(m_index, &m_lit);
		pDevice->LightEnable(m_index, TRUE);
		return true;
	}

	void draw(IDirect3DDevice9* pDevice)
	{
		if (NULL == pDevice)
			return;
		D3DXMATRIX m;
		D3DXMatrixTranslation(&m, m_lit.Position.x, m_lit.Position.y, m_lit.Position.z);
		pDevice->SetTransform(D3DTS_WORLD, &m);
		pDevice->SetMaterial(&d3d::WHITE_MTRL);
		m_pMesh->DrawSubset(0);
	}

	D3DXVECTOR3 getPosition(void) const { return D3DXVECTOR3(m_lit.Position); }

private:
	DWORD               m_index;
	D3DXMATRIX          m_mLocal;
	D3DLIGHT9           m_lit;
	ID3DXMesh* m_pMesh;
	d3d::BoundingSphere m_bound;
};


// -----------------------------------------------------------------------------
// Global variables
// -----------------------------------------------------------------------------
CWall	g_legoPlane; //바닥을 나타내는 벽
CWall	g_legowall[4]; //4개의 벽
CSphere	g_sphere[4]; //4개의 구체
CSphere	g_target_blueball; //목표 구체(파란색)
CLight	g_light; //광원 객체

double g_camera_pos[3] = { 0.0, 5.0, -8.0 }; //카메라의 초기 위치(x, y, z)

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------


void destroyAllLegoBlock(void)
{
}

bool isGameStarted = false;  // 게임 시작 여부
ID3DXFont* startg_Font = nullptr; // 텍스트 렌더링용 폰트
ID3DXFont* infog_Font = nullptr; // 텍스트 렌더링용 폰트

// initialization
bool Setup()
{
	int i;

	D3DXMatrixIdentity(&g_mWorld); //월드 변환 행렬 초기화
	D3DXMatrixIdentity(&g_mView); //뷰 변환 행렬 초기화
	D3DXMatrixIdentity(&g_mProj); //프로젝션 변환 행렬 초기화

	// create plane and set the position 바닥 생성 및 위치 설정
	//바닥을 생성하고 색상과 크기 설정, 실패 시 false 반환
	if (false == g_legoPlane.create(Device, -1, -1, 9, 0.03f, 6, d3d::GREEN)) return false;
	g_legoPlane.setPosition(0.0f, -0.0006f / 5, 0.0f); //바닥의 위치 설정 (y값은 약간 내려서 바닥에 위치하도록)

	// create walls and set the position. note that there are four walls 벽 생성 및 위치 설정
	if (false == g_legowall[0].create(Device, -1, -1, 9, 0.3f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[0].setPosition(0.0f, 0.12f, 3.06f); //위치 설정
	if (false == g_legowall[1].create(Device, -1, -1, 9, 0.3f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[1].setPosition(0.0f, 0.12f, -3.06f); //위치 설정
	if (false == g_legowall[2].create(Device, -1, -1, 0.12f, 0.3f, 6.24f, d3d::DARKRED)) return false;
	g_legowall[2].setPosition(4.56f, 0.12f, 0.0f); //위치 설정
	if (false == g_legowall[3].create(Device, -1, -1, 0.12f, 0.3f, 6.24f, d3d::DARKRED)) return false;
	g_legowall[3].setPosition(-4.56f, 0.12f, 0.0f); //위치 설정

	// create four balls and set the position 4개의 구체 생성 및 위치 설정
	for (i = 0; i < 4; i++) {
		if (false == g_sphere[i].create(Device, sphereColor[i])) return false; //구체를 생성하고 색상을 지정, 실패 시 false 반환
		g_sphere[i].setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]); //구체의 위치 설정(반지름에 맞춰 y 위치는 M_RADIUS 값 사용)
		g_sphere[i].setPower(0, 0); //구체의 초기 속도 설정(속도 0으로 초기화)
	}

	// create blue ball for set direction
	if (false == g_target_blueball.create(Device, d3d::BLUE)) return false; //파란색 구체를 생성, 실패 시 false 반환
	g_target_blueball.setCenter(.0f, (float)M_RADIUS, 0.0f); //구체의 위치를 설정(중앙에 위치)

	// light setting 
	D3DLIGHT9 lit;
	::ZeroMemory(&lit, sizeof(lit));
	lit.Type = D3DLIGHT_POINT;
	lit.Diffuse = d3d::WHITE;
	lit.Specular = d3d::WHITE * 0.9f;
	lit.Ambient = d3d::WHITE * 0.9f;
	lit.Position = D3DXVECTOR3(0.0f, 3.0f, 0.0f);
	lit.Range = 100.0f;
	lit.Attenuation0 = 0.0f;
	lit.Attenuation1 = 0.9f;
	lit.Attenuation2 = 0.0f;
	if (false == g_light.create(Device, lit))
		return false;

	// Position and aim the camera.
	D3DXVECTOR3 pos(0.0f, 5.0f, -8.0f);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 2.0f, 0.0f);
	D3DXMatrixLookAtLH(&g_mView, &pos, &target, &up);
	Device->SetTransform(D3DTS_VIEW, &g_mView);

	// Set the projection matrix.
	D3DXMatrixPerspectiveFovLH(&g_mProj, D3DX_PI / 4,
		(float)Width / (float)Height, 1.0f, 100.0f);
	Device->SetTransform(D3DTS_PROJECTION, &g_mProj);

	// Set render states.
	Device->SetRenderState(D3DRS_LIGHTING, TRUE);
	Device->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
	Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

	g_light.setLight(Device, g_mWorld);

	// start button 
	HRESULT startHr = D3DXCreateFont(Device, 48, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, "Arial", &startg_Font);
	if (FAILED(startHr))
	{
		::MessageBox(0, "D3DXCreateFont() - FAILED", 0, 0);
		return false;
	}

	// info button
	HRESULT infoHr = D3DXCreateFont(Device, 24, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, "Arial", &infog_Font);
	if (FAILED(infoHr))
	{
		::MessageBox(0, "D3DXCreateFont() - FAILED", 0, 0);
		return false;
	}

	return true;
}

//자원을 해제하는 함수
void Cleanup(void)
{
	g_legoPlane.destroy(); //바닥 객체 해제
	for (int i = 0; i < 4; i++) {
		g_legowall[i].destroy(); //4개의 벽 객체 해제
	}
	destroyAllLegoBlock(); //모든 Lego 블록 객체 해제
	g_light.destroy(); //광원 객체 해제
}


// timeDelta represents the time between the current image frame and the last image frame.
// the distance of moving balls should be "velocity * timeDelta"
//화면을 렌더링하는 역할
//timeDelta는 이전 프레임과 현재 프레임 간의 시간 차이를 나타내며, 이를 사용하여 각 구체의 이동을 조정

// 고정된 버튼
RECT startButtonRect = { 300, 200, 700, 300 };
RECT infoRect = { startButtonRect.left, startButtonRect.top + 100, startButtonRect.right, startButtonRect.bottom + 100 };

bool Display(float timeDelta)
{
	int i = 0;
	int j = 0;

	if (Device) //디바이스가 유효한 경우에만 실행
	{
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0); //화면과 Z 버퍼를 초기화(배경색 설정)
		Device->BeginScene(); //렌더링 시작

		// update the position of each ball. during update, check whether each ball hit by walls.
		//각 구체의 위치 업데이트(시간차(timeDelta)를 고려하여 속도에 맞게 이동)
		for (i = 0; i < 4; i++) {
			g_sphere[i].ballUpdate(timeDelta); //각 구체의 위치를 업데이트
			for (j = 0; j < 4; j++) {
				g_legowall[i].hitBy(g_sphere[j]); //각 구체가 벽에 충돌했는지 확인
			}
		}

		// check whether any two balls hit together and update the direction of balls
		//두 구체가 충돌했는지 확인하고, 충돌 시 구체의 방향 업데이트
		for (i = 0; i < 4; i++) {
			for (j = 0; j < 4; j++) {
				if (i >= j) { continue; } //자기 자신과 비교하지 않도록 방지
				g_sphere[i].hitBy(g_sphere[j]); //두 구체의 충돌 확인
			}
		}

		// draw plane, walls, and spheres 화면에 그릴 객체들
		g_legoPlane.draw(Device, g_mWorld); //바닥 그리기
		for (i = 0; i < 4; i++) {
			g_legowall[i].draw(Device, g_mWorld); //각 벽 그리기
			g_sphere[i].draw(Device, g_mWorld); //각 구체 그리기
		}
		g_target_blueball.draw(Device, g_mWorld); //목표 파란색 구체 그리기
		g_light.draw(Device); //광원 객체 그리기
		
		if (!isGameStarted)
		{
			// Start 버튼과 안내 문구 표시
			D3DRECT fillRect = { startButtonRect.left, startButtonRect.top, startButtonRect.right, startButtonRect.bottom };
			Device->Clear(1, &fillRect, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

			// Start 버튼 텍스트와 안내문 출력
			startg_Font->DrawText(0, "START", -1, &startButtonRect, DT_CENTER | DT_BOTTOM | DT_SINGLELINE, d3d::WHITE);

			D3DRECT rect = { infoRect.left, infoRect.top, infoRect.right, infoRect.bottom };
			Device->Clear(1, &rect, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
			infog_Font->DrawText(0, "press ESC to exit", -1, &infoRect, DT_CENTER | DT_TOP | DT_SINGLELINE, d3d::WHITE);
		}

		Device->EndScene(); //렌더링 끝
		Device->Present(0, 0, 0, 0); //화면을 디스플레이
		Device->SetTexture(0, NULL); //텍스처를 비운다
	}
	return true; //함수가 정상적으로 끝나면 true 반환
}

//WndProc 함수:
//윈도우 프로시저로, 윈도우에서 발생하는 다양한 이벤트(키 입력, 마우스 입력 등)를 처리
//WM_KEYDOWN 메시지에서는 키 입력을 처리하고, VK_ESCAPE, VK_RETURN, VK_SPACE 키에 대해 각각 동작을 정의
//WM_MOUSEMOVE 메시지에서는 마우스 움직임을 처리하고, 마우스 좌표에 따라 3D 객체(구체, 파란색 구체 등)의 위치나 회전을 제어
LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool wire = false; //wireframe 모드 여부를 저장하는 변수
	static bool isReset = true; //마우스를 처음 클릭할 때 초기화 여부
	static int old_x = 0; //이전 마우스 x 좌표
	static int old_y = 0; //이전 마우스 y 좌표
	static enum { WORLD_MOVE, LIGHT_MOVE, BLOCK_MOVE } move = WORLD_MOVE; //객체 이동 모드(기본은 WORLD_MOVE)
	static bool isTargetYellow = false; // 현재 누구 차례인지나타냄
	static bool worldMove = false; // 월드가 움직이는지 여부

	switch (msg) {
	case WM_DESTROY: //창이 닫힐 때
	{
		::PostQuitMessage(0); //종료 메시지 보냄
		break;
	}
	case WM_LBUTTONDOWN:
	{
		POINT cursorPos;
		GetCursorPos(&cursorPos);
		ScreenToClient(hwnd, &cursorPos);

		// Start 버튼 클릭 여부 확인
		if (!isGameStarted &&
			cursorPos.x >= startButtonRect.left && cursorPos.x <= startButtonRect.right &&
			cursorPos.y >= startButtonRect.top && cursorPos.y <= startButtonRect.bottom)
		{
			isGameStarted = true; 
			worldMove = true;
		}
	}
	break;

	case WM_KEYDOWN: //키가 눌렸을 때
	{
		// 게임이 시작되어야만 움직임, 그러나 esc를 누르면 종료
		if (!isGameStarted  && VK_ESCAPE == false) break;
		switch (wParam) {
		case VK_ESCAPE: //ESC 키가 눌리면
			::DestroyWindow(hwnd); //창을 닫음
			break;
		case VK_RETURN: //ENTER 키가 눌리면
			if (NULL != Device) {
				wire = !wire; //wire 모드를 토글
				Device->SetRenderState(D3DRS_FILLMODE,
					(wire ? D3DFILL_WIREFRAME : D3DFILL_SOLID)); //wireframe 모드와 solid 모드 전환
			}
			break;
		case VK_SPACE: //SPACE 키가 눌리면
		{
			int n;
			//타겟과 하얀 공의 위치를 가져와서 두 공 사이의 방향과 거리를 계산
			if (isTargetYellow) {
				n = 2;
			}
			else n = 3;
			isTargetYellow = !isTargetYellow;
			D3DXVECTOR3 targetpos = g_target_blueball.getCenter();
			D3DXVECTOR3	ballpos = g_sphere[n].getCenter();
			double theta = acos(sqrt(pow(targetpos.x - ballpos.x, 2)) / sqrt(pow(targetpos.x - ballpos.x, 2) +
				pow(targetpos.z - ballpos.z, 2)));		//각도를 계산(기본 1 사분면)
			//각 사분면에 따라 방향을 조정
			if (targetpos.z - ballpos.z <= 0 && targetpos.x - ballpos.x >= 0) { theta = -theta; }	//4 사분면
			if (targetpos.z - ballpos.z >= 0 && targetpos.x - ballpos.x <= 0) { theta = PI - theta; } //2 사분면
			if (targetpos.z - ballpos.z <= 0 && targetpos.x - ballpos.x <= 0) { theta = PI + theta; } // 3 사분면
			double distance = sqrt(pow(targetpos.x - ballpos.x, 2) + pow(targetpos.z - ballpos.z, 2)); //두 공 사이의 거리 계산
			g_sphere[n].setPower(distance * cos(theta), distance * sin(theta)); //하얀 공에 속도 설정

			break;
		}
		case VK_BACK:
			worldMove= !worldMove; // 월드 이동 활성화/비활성화 토글
			break;
		}
		break;
	}

	case WM_MOUSEMOVE: //마우스가 이동할 때
	{
		// 게임이 시작되어야만 움직임
		if (!isGameStarted) break;
		int new_x = LOWORD(lParam); //마우스 x 좌표
		int new_y = HIWORD(lParam); //마우스 y 좌표
		float dx; //x 방향 이동 거리
		float dy; //y 방향 이동 거리

		if (LOWORD(wParam) & MK_LBUTTON) { //왼쪽 마우스 버튼을 눌렀을 때
			if (!worldMove) break; // worldmove가 설정되었는지 확인

			if (isReset) {
				isReset = false; //첫 번째 클릭 후 초기화 플래그 해제
			}
			else {
				D3DXVECTOR3 vDist; //이동 벡터
				D3DXVECTOR3 vTrans; //이동 벡터 적용
				D3DXMATRIX mTrans; //변환 행렬
				D3DXMATRIX mX; //x축 회전 행렬
				D3DXMATRIX mY; //y축 회전 행렬

				if(worldMove){
					switch (move) {
					case WORLD_MOVE: //월드 이동 모드
						dx = (old_x - new_x) * 0.01f; //마우스 이동에 따른 x 방향 회전 값 계산
						dy = (old_y - new_y) * 0.01f; //마우스 이동에 따른 y 방향 회전 값 계산
						D3DXMatrixRotationY(&mX, dx); //y축 회전 행렬 생성
						D3DXMatrixRotationX(&mY, dy); //x축 회전 행렬 생성
						g_mWorld = g_mWorld * mX * mY; //기존 월드 행렬에 회전 적용

						break;
					}
				}
			}

			old_x = new_x; //현재 마우스 x좌표 저장
			old_y = new_y; //현재 마우스 y좌표 저장

		}
		else { //마우스 왼쪽 버튼을 떼었을 때
			isReset = true; //초기화 플래그 설정

			if (LOWORD(wParam) & MK_RBUTTON) { //오른쪽 마우스 버튼을 눌렀을 때
				dx = (old_x - new_x);// * 0.01f; x 방향 이동 거리
				dy = (old_y - new_y);// * 0.01f; y 방향 이동 거리

				D3DXVECTOR3 coord3d = g_target_blueball.getCenter(); //파란색 공의 현재 위치 가져오기
				g_target_blueball.setCenter(coord3d.x + dx * (-0.007f), coord3d.y, coord3d.z + dy * 0.007f); //공의 위치 이동
			}
			old_x = new_x; //현재 마우스 x좌표 저장
			old_y = new_y; //현재 마우스 y좌표 저장

			if (worldMove) move = WORLD_MOVE; //기본 이동 모드로 설정
			
		}
		break;
	}
	}

	return ::DefWindowProc(hwnd, msg, wParam, lParam); //기본 윈도우 프로시저 호출
}

//WinMain 함수:
//프로그램의 엔트리 포인트로, Direct3D 초기화, 설정, 메시지 루프, 리소스 정리 등을 처리
//d3d::InitD3D와 Setup 함수에서 Direct3D 환경을 설정하고, d3d::EnterMsgLoop로 메인 메시지 루프를 실행
//Cleanup 함수는 프로그램 종료 시 리소스를 정리하고, Device->Release()로 Direct3D 디바이스를 해제
int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCmd)
{
	srand(static_cast<unsigned int>(time(NULL))); //랜덤 시드 설정

	//Direct3D 초기화
	if (!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0); //초기화 실패 시 메시지 출력
		return 0;
	}
	//Setup 함수 호출
	if (!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0); //설정 실패 시 메시지 출력
		return 0;
	}
	//메시지 루프 실행
	d3d::EnterMsgLoop(Display);
	//리소스 정리
	Cleanup();
	//디바이스 해제
	Device->Release();

	return 0;
}