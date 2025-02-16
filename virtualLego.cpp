#include "d3dUtility.h"
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <Windows.h>
#include <d3dx9.h>

IDirect3DDevice9* Device = NULL; //Direct3D ����̽� ������ �ʱ�ȭ

// window size
const int Width = 1024;
const int Height = 768;

// There are four balls
// initialize the position (coordinate) of each ball (ball0 ~ ball3) �� ���� �ʱ� ��ġ ����
const float spherePos[4][2] = { {-2.7f,0} , {+2.4f,0} , {3.3f,0} , {-2.7f,-0.9f} };
// initialize the color of each ball (ball0 ~ ball3) �� ���� ���� �ʱ�ȭ
const D3DXCOLOR sphereColor[4] = { d3d::RED, d3d::RED, d3d::YELLOW, d3d::WHITE };

// -----------------------------------------------------------------------------
// Transform matrices
// -----------------------------------------------------------------------------
D3DXMATRIX g_mWorld; //���� ���
D3DXMATRIX g_mView; //�� ���
D3DXMATRIX g_mProj; //���� ���

#define M_RADIUS 0.21   // ball radius ���� ������
#define PI 3.14159265 //������
#define M_HEIGHT 0.01 //���� ����
#define DECREASE_RATE 0.9982 //�ӵ� ������(�ð��� ���� �ӵ��� �پ��)

// -----------------------------------------------------------------------------
// CSphere class definition
// -----------------------------------------------------------------------------

class CSphere {
private:
	float center_x, center_y, center_z; //���� ��ġ
	float m_radius; //���� ������
	float m_velocity_x; //���� x�� �ӵ�
	float m_velocity_z; //���� z�� �ӵ�

public:
	CSphere(void)
	{
		D3DXMatrixIdentity(&m_mLocal); //���� ��ȯ ����� ���� ��ķ� �ʱ�ȭ
		ZeroMemory(&m_mtrl, sizeof(m_mtrl)); //���� �Ӽ��� 0���� �ʱ�ȭ
		m_radius = 0; //������ �ʱ�ȭ
		m_velocity_x = 0; //x�� �ӵ� �ʱ�ȭ
		m_velocity_z = 0; //z�� �ӵ� �ʱ�ȭ
		m_pSphereMesh = NULL; //�� �޽ø� NULL�� �ʱ�ȭ
	}
	~CSphere(void) {} //�Ҹ���

public:
	//create �Լ�:
	//pDevice�� NULL���� Ȯ���Ͽ�, �ùٸ� ����̽��� �����Ǿ����� �˻�
	// �޽��� ������ ����, color �Ű������� ���޵� ������ ����Ͽ�, Ambient, Diffuse, Specular �Ӽ��� ��� ����
	// D3DXCreateSphere�� ����Ͽ� �� �޽ø� �����ϸ�, ���� �������� ���� ������ ����, ������ �����ϸ� false ��ȯ
	//
	//�־��� �������� �� �޽� ����
	bool create(IDirect3DDevice9* pDevice, D3DXCOLOR color = d3d::WHITE)
	{
		if (NULL == pDevice) //pDevice�� NULL�̸� ����
			return false;

		m_mtrl.Ambient = color; //�ֺ��� ���� ����
		m_mtrl.Diffuse = color; //Ȯ�걤 ���� ����
		m_mtrl.Specular = color; //�ݻ籤 ���� ����
		m_mtrl.Emissive = d3d::BLACK; //���Ɽ ���� ����
		m_mtrl.Power = 5.0f; //������ �ݻ� ���� ����

		//�� �޽� ���� : ������, ���ø�, ���� ����, pDevice�� Direct3D ����̽� ������
		if (FAILED(D3DXCreateSphere(pDevice, getRadius(), 50, 50, &m_pSphereMesh, NULL)))
			return false; //�޽� ���� ���� �� false ��ȯ
		return true; //�޽� ���� ���� �� true ��ȯ
	}

	//destroy �Լ�:
	//�޽ð� �̹� �Ҵ�Ǿ� �ִ� ���, Release�� ȣ���Ͽ� �޽� ���ҽ��� �����ϰ�, �����͸� NULL�� ����
	// 
	//�� �޽ø� �����ϴ� �Լ�
	void destroy(void)
	{
		//�޽ð� NULL�� �ƴ� ��쿡�� ����
		if (m_pSphereMesh != NULL) {
			m_pSphereMesh->Release(); //�޽� ���ҽ� ����
			m_pSphereMesh = NULL; //�޽� �����͸� NULL�� ����
		}
	}

	//draw �Լ�:
	//pDevice�� NULL���� Ȯ���Ͽ� �׸� �� ������ �Լ��� ����
	//SetTransform�� MultiplyTransform�� ����Ͽ� ���� ���� ��ȯ ����� ����, ���⼭�� ���� ��ġ �� ȸ�� ���� ��ȯ�� ����
	//���� ������ �����ϰ�, DrawSubset�� ȣ���Ͽ� �� �޽ø� ȭ�鿡 �׸���, DrawSubset(0)�� ù ��° ������� �׸��� ȣ��
	//���� �׸��� �Լ�
	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		//pDevice�� NULL�̸� �׸� �� �����Ƿ� �Լ��� ����
		if (NULL == pDevice)
			return;
		pDevice->SetTransform(D3DTS_WORLD, &mWorld); //���� ��ȯ ��� ���� (���� ��ġ�� ��ȭ�� ����)
		pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal); //���� ��ȯ ����� ���� ��ȯ ��Ŀ� ���� (������� ��ġ ����)
		pDevice->SetMaterial(&m_mtrl); //���� ���� ����
		m_pSphereMesh->DrawSubset(0); //�� �޽ø� �׸��� (ù ��° ������� ���)
	}

	bool hasIntersected(CSphere& ball)
	{
		// check if there is collision between two spheres
		D3DXVECTOR3 myCenter = this->getCenter();
		D3DXVECTOR3 otherCenter = ball.getCenter();

		float dx = myCenter.x - otherCenter.x;
		float dz = myCenter.z - otherCenter.z;

		// �� ���� �߽� �� �Ÿ� ���
		float distance = sqrt(dx * dx + dz * dz);

		// �Ÿ��� �� �������� �պ��� �۰ų� ������ �浹
		if (distance <= (this->getRadius() + ball.getRadius())) {
			return true;
		}
		else return false;
	}

	void hitBy(CSphere& ball)
	{
		// what needs to be done if there is collision between two spheres.
		// �� ���� �߽ɰ� �ӵ�
		D3DXVECTOR3 myCenter = this->getCenter();
		D3DXVECTOR3 otherCenter = ball.getCenter();

		D3DXVECTOR3 myVelocity = D3DXVECTOR3(this->getVelocity_X(), 0, this->getVelocity_Z());
		D3DXVECTOR3 otherVelocity = D3DXVECTOR3(ball.getVelocity_X(), 0, ball.getVelocity_Z());

		// �� �� ������ ���� ���� ���
		D3DXVECTOR3 collisionNormal = myCenter - otherCenter;
		float distance = D3DXVec3Length(&collisionNormal);

		if (hasIntersected(ball)) {
			// �浹 ����: ���� �߽� ���� �Ÿ��� �������� �պ��� �۰ų� ������ �浹
			// ���� ���� ����ȭ
			D3DXVec3Normalize(&collisionNormal, &collisionNormal);

			// ��� �ӵ� ���
			D3DXVECTOR3 relativeVelocity = myVelocity - otherVelocity;

			// �浹 ���⿡���� ��� �ӵ� ��Į�� ���
			float velocityAlongNormal = D3DXVec3Dot(&relativeVelocity, &collisionNormal);

			// �浹 ����: ��� �ӵ��� 0���� ���� ��츸 ó��
			if (velocityAlongNormal > 0) {
				return;
			}

			// ��ݷ� ��� (��ź�� �浹 ���� ����)
			float e = 0.9f;
			float impulseMagnitude = -(1 + e) * velocityAlongNormal / 2.0f;

			// ��ݷ� ����
			D3DXVECTOR3 impulse = impulseMagnitude * collisionNormal;

			// �ӵ� ������Ʈ
			D3DXVECTOR3 newmyVelocity = myVelocity + impulse;
			D3DXVECTOR3 newotherVelocity = otherVelocity - impulse;

			this->setPower(newmyVelocity.x, newmyVelocity.z);
			ball.setPower(newotherVelocity.x, newotherVelocity.z);
		}
	}

	//ballUpdate �Լ�:
	//timeDiff�� ����Ͽ� ���� ��ġ�� ������Ʈ, ���� �̵��� �� X, Z �������� ��ġ�� �Ի��ϰ�, �ӵ��� ���� ���Ϸ� �������� �ӵ��� 0���� ����
	//���� �ӵ� ���Ҹ� ó���ϴ� �κп���, DECREASE_RATE�� ���� �ӵ��� ���� �ٿ� ������
	//���� ���¸� �ð� ���̿� ���� ������Ʈ�ϴ� �Լ�
	void ballUpdate(float timeDiff)
	{
		const float TIME_SCALE = 3.3; //�ð� �������� ���� ���(�ӵ� ����)
		D3DXVECTOR3 cord = this->getCenter(); //���� ���� ��ġ�� �����´�
		//���� X, Z ���� �ӵ� ���� �������� �����´�
		double vx = abs(this->getVelocity_X());
		double vz = abs(this->getVelocity_Z());

		//���� �ӵ��� ���� �� �̻��̸�, ���� ��ġ�� ������Ʈ
		if (vx > 0.01 || vz > 0.01)
		{
			//���� X, Z ������ ���ο� ��ġ ���
			float tX = cord.x + TIME_SCALE * timeDiff * m_velocity_x;
			float tZ = cord.z + TIME_SCALE * timeDiff * m_velocity_z;

			//correction of position of ball
			//���� ���� �浹�� �� ��ġ�� �����ϴ� �κ�
			// Please uncomment this part because this correction of ball position is necessary when a ball collides with a wall
			//�� �κ��� �����ϸ� ���� �浹 �� ���� ���� ���� �ʵ��� �����Ѵ�
			if (tX >= (4.5 - M_RADIUS))
				tX = 4.5 - M_RADIUS;
			else if (tX <= (-4.5 + M_RADIUS))
				tX = -4.5 + M_RADIUS;
			else if (tZ <= (-3 + M_RADIUS))
				tZ = -3 + M_RADIUS;
			else if (tZ >= (3 - M_RADIUS))
				tZ = 3 - M_RADIUS;

			this->setCenter(tX, cord.y, tZ); //���ο� ��ġ�� ���� �߽ɿ� ����
		}
		else {
			this->setPower(0, 0); //���� ������ �� �ӵ��� 0���� ���� (�ӵ��� �ʹ� ������ ���� ������ ����)
		}

		//this->setPower(this->getVelocity_X() * DECREASE_RATE, this->getVelocity_Z() * DECREASE_RATE);
		double rate = 1 - (1 - DECREASE_RATE) * timeDiff * 400; //���� �ӵ� ����(�ӵ� �������� ������� ����), DECREASE_RATE�� ���� �ӵ� ������ ���

		//�ӵ� �������� 0���� ������ 0���� ����
		if (rate < 0)
			rate = 0;
		this->setPower(getVelocity_X() * rate, getVelocity_Z() * rate); //���ο� �ӵ��� ����
	}

	double getVelocity_X() { return this->m_velocity_x; } //���� X ���� �ӵ��� ��ȯ�ϴ� �Լ�
	double getVelocity_Z() { return this->m_velocity_z; } //���� Z ���� �ӵ��� ��ȯ�ϴ� �Լ�

	//���� �ӵ��� �����ϴ� �Լ�
	void setPower(double vx, double vz)
	{
		this->m_velocity_x = vx; //X ���� �ӵ� ����
		this->m_velocity_z = vz; //Z ���� �ӵ� ����
	}

	//setCenter �Լ�:
	//������ ��ġ�� �������� D3DMatrixTranslation�� ����Ͽ� ��ȯ ����� �����ϰ� �̸� ���� ��ȯ���� ����
	//���� �߽� ��ġ�� �����ϴ� �Լ�
	void setCenter(float x, float y, float z)
	{
		D3DXMATRIX m;
		center_x = x;	center_y = y;	center_z = z; //���� X, Y, Z ��ġ ����
		//��ġ ��ȯ ����� ����� ���� ��ȯ ����
		D3DXMatrixTranslation(&m, x, y, z);
		setLocalTransform(m);
	}

	float getRadius(void)  const { return (float)(M_RADIUS); } //���� �������� ��ȯ�ϴ� �Լ�, M_RADIUS�� ���� ������ ������ ���� ��Ÿ���� ���
	const D3DXMATRIX& getLocalTransform(void) const { return m_mLocal; } //���� ��ȯ ����� ��ȯ�ϴ� �Լ�
	void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; } //���� ��ȯ ����� �����ϴ� �Լ�
	//���� �߽� ��ġ�� ��ȯ�ϴ� �Լ�
	D3DXVECTOR3 getCenter(void) const
	{
		//���� ���� �߽� ��ġ�� D3DXVECTOR3�� ��ȯ
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

	float m_x; //���� X ��ǥ
	float m_z; //���� Z ��ǥ
	float m_width; //���� �ʺ�
	float m_depth; //���� �β�
	float m_height; //���� ����

public:
	//������: �� ��ü �ʱ�ȭ
	CWall(void)
	{
		D3DXMatrixIdentity(&m_mLocal); //���� ��ȯ ��� �ʱ�ȭ(���� ��ķ� ����)
		ZeroMemory(&m_mtrl, sizeof(m_mtrl)); //���� �ʱ�ȭ
		m_width = 0;
		m_depth = 0;
		m_pBoundMesh = NULL; //�޽� ������ �ʱ�ȭ
	}
	~CWall(void) {} //�Ҹ���
public:
	//���� �����ϴ� �Լ�
	//���� ũ��� ������ �޾� �޽��� ����, D3DXCreateBox�� ����Ͽ� �ڽ� ������ �޽��� �����ϸ�, ���� ������ ����
	bool create(IDirect3DDevice9* pDevice, float ix, float iz, float iwidth, float iheight, float idepth, D3DXCOLOR color = d3d::WHITE)
	{
		if (NULL == pDevice)
			return false; //����̽��� NULL�̸� ����

		//���� ���� ����
		m_mtrl.Ambient = color; //�ֺ��� ����
		m_mtrl.Diffuse = color; //Ȯ�걤 ����
		m_mtrl.Specular = color; //�ݻ籤 ����
		m_mtrl.Emissive = d3d::BLACK; //���Ɽ ����(����)
		m_mtrl.Power = 5.0f; //������ �ݻ��

		//���� ũ�� ����
		m_width = iwidth;
		m_depth = idepth;

		//D3DXCreateBox�� ����� ���� �޽� ����
		if (FAILED(D3DXCreateBox(pDevice, iwidth, iheight, idepth, &m_pBoundMesh, NULL)))
			return false; //�޽� ���� ���� �� false ����
		return true; //���������� ���� ����
	}

	//�� �޽��� �����ϰ� �޸𸮸� �����ϴ� �Լ�
	void destroy(void)
	{
		if (m_pBoundMesh != NULL) {
			m_pBoundMesh->Release(); //�޽� ���ҽ� ����
			m_pBoundMesh = NULL; //�޽� ������ NULL�� ����
		}
	}

	//���� �׸���(�������ϴ�) �Լ�
	void draw(IDirect3DDevice9* pDevice, const D3DXMATRIX& mWorld)
	{
		if (NULL == pDevice)
			return; //����̽��� NULL�̸� �׸��� ����
		pDevice->SetTransform(D3DTS_WORLD, &mWorld); //���� ��ȯ ��� ����
		pDevice->MultiplyTransform(D3DTS_WORLD, &m_mLocal); //���� ��ȯ ����� ���� ��ȯ ��Ŀ� ���Ͽ� ����
		pDevice->SetMaterial(&m_mtrl); //������ ����
		m_pBoundMesh->DrawSubset(0); //�޽��� �׸���
	}

	bool hasIntersected(CSphere& ball)
	{
		// check if there is collision between a sphere and a wall
		D3DXVECTOR3 ballCenter = ball.getCenter();
		float ballRadius = ball.getRadius();

		// ���� ����
		float wallLeft = m_x - m_width / 2.0f;
		float wallRight = m_x + m_width / 2.0f;
		float wallTop = m_z + m_depth / 2.0f;
		float wallBottom = m_z - m_depth / 2.0f;

		// ���� ���� �浹 ���� ���
		bool intersectsX = (ballCenter.x + ballRadius >= wallLeft) && (ballCenter.x - ballRadius <= wallRight);
		bool intersectsZ = (ballCenter.z + ballRadius >= wallBottom) && (ballCenter.z - ballRadius <= wallTop);

		// X��� Z�� ��� �� �ϳ��� ħ���ϸ� �浹�� ����
		return intersectsX && intersectsZ;
	}

	void hitBy(CSphere& ball)
	{
		// ���� �߽� ��ǥ�� �ӵ� ��������
		D3DXVECTOR3 ballCenter = ball.getCenter();
		float ballRadius = ball.getRadius();

		D3DXVECTOR3 ballVelocity = D3DXVECTOR3(ball.getVelocity_X(), 0, ball.getVelocity_Z());

		// ���� ��� ���
		float wallLeft = m_x - m_width / 2.0f;
		float wallRight = m_x + m_width / 2.0f;
		float wallTop = m_z + m_depth / 2.0f;
		float wallBottom = m_z - m_depth / 2.0f;

		if (hasIntersected(ball)) {
			// ���� �浹�� ������

			// �浹 ������ ���
			D3DXVECTOR3 collisionNormal(0, 0, 0);

			if (ballCenter.x - ballRadius < wallLeft) {
				collisionNormal = D3DXVECTOR3(-1, 0, 0); // ���� ��
			}
			else if (ballCenter.x + ballRadius > wallRight) {
				collisionNormal = D3DXVECTOR3(1, 0, 0); // ������ ��
			}
			else if (ballCenter.z - ballRadius < wallBottom) {
				collisionNormal = D3DXVECTOR3(0, 0, -1); // �Ʒ��� ��
			}
			else if (ballCenter.z + ballRadius > wallTop) {
				collisionNormal = D3DXVECTOR3(0, 0, 1); // ���� ��
			}

			// �� ������ �ӵ� ���
			float velocityAlongNormal = D3DXVec3Dot(&ballVelocity, &collisionNormal);

			// ��ݷ� ��� (���� ���� �����̹Ƿ� ���� ������ ����)
			float e = 0.8f; // �ݹ� ��� (ź�� ����)
			float impulseMagnitude = -(1 + e) * velocityAlongNormal;

			// ��ݷ� ����
			D3DXVECTOR3 impulse = impulseMagnitude * collisionNormal;

			// ���� �ӵ� ������Ʈ
			D3DXVECTOR3 newBallVelocity = ballVelocity + impulse;
			ball.setPower(newBallVelocity.x, newBallVelocity.z);

			// ���� ��ġ ����: ���� ������ �ʵ��� ħ���� ����
			D3DXVECTOR3 correctedPosition = ballCenter;
			if (collisionNormal.x != 0) {
				// X�� �浹
				correctedPosition.x = collisionNormal.x < 0 ? wallLeft - ballRadius : wallRight + ballRadius;
			}
			if (collisionNormal.z != 0) {
				// Z�� �浹
				correctedPosition.z = collisionNormal.z < 0 ? wallBottom - ballRadius : wallTop + ballRadius;
			}

			ball.setCenter(correctedPosition.x, ballCenter.y, correctedPosition.z);
		}
	}


	//���� ��ġ�� �����ϴ� �Լ�
	void setPosition(float x, float y, float z)
	{
		D3DXMATRIX m;
		//���� X, Z ��ǥ�� ������Ʈ
		this->m_x = x;
		this->m_z = z;

		D3DXMatrixTranslation(&m, x, y, z); //������ x, y, z ���� ���� ��ȯ ����� ����
		setLocalTransform(m); //������ ��ȯ ����� ���� ��ȯ ��Ŀ� ����
	}

	float getHeight(void) const { return M_HEIGHT; } //���� ���̸� ��ȯ�ϴ� �Լ�



private:
	void setLocalTransform(const D3DXMATRIX& mLocal) { m_mLocal = mLocal; } //���� ��ȯ ����� �����ϴ� �Լ�

	D3DXMATRIX              m_mLocal; //���� ���� ��ȯ ���
	D3DMATERIAL9            m_mtrl; //���� ����
	ID3DXMesh* m_pBoundMesh; //���� ��Ÿ���� �޽� ��ü
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
CWall	g_legoPlane; //�ٴ��� ��Ÿ���� ��
CWall	g_legowall[4]; //4���� ��
CSphere	g_sphere[4]; //4���� ��ü
CSphere	g_target_blueball; //��ǥ ��ü(�Ķ���)
CLight	g_light; //���� ��ü

double g_camera_pos[3] = { 0.0, 5.0, -8.0 }; //ī�޶��� �ʱ� ��ġ(x, y, z)

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------


void destroyAllLegoBlock(void)
{
}

bool isGameStarted = false;  // ���� ���� ����
ID3DXFont* startg_Font = nullptr; // �ؽ�Ʈ �������� ��Ʈ
ID3DXFont* infog_Font = nullptr; // �ؽ�Ʈ �������� ��Ʈ

// initialization
bool Setup()
{
	int i;

	D3DXMatrixIdentity(&g_mWorld); //���� ��ȯ ��� �ʱ�ȭ
	D3DXMatrixIdentity(&g_mView); //�� ��ȯ ��� �ʱ�ȭ
	D3DXMatrixIdentity(&g_mProj); //�������� ��ȯ ��� �ʱ�ȭ

	// create plane and set the position �ٴ� ���� �� ��ġ ����
	//�ٴ��� �����ϰ� ����� ũ�� ����, ���� �� false ��ȯ
	if (false == g_legoPlane.create(Device, -1, -1, 9, 0.03f, 6, d3d::GREEN)) return false;
	g_legoPlane.setPosition(0.0f, -0.0006f / 5, 0.0f); //�ٴ��� ��ġ ���� (y���� �ణ ������ �ٴڿ� ��ġ�ϵ���)

	// create walls and set the position. note that there are four walls �� ���� �� ��ġ ����
	if (false == g_legowall[0].create(Device, -1, -1, 9, 0.3f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[0].setPosition(0.0f, 0.12f, 3.06f); //��ġ ����
	if (false == g_legowall[1].create(Device, -1, -1, 9, 0.3f, 0.12f, d3d::DARKRED)) return false;
	g_legowall[1].setPosition(0.0f, 0.12f, -3.06f); //��ġ ����
	if (false == g_legowall[2].create(Device, -1, -1, 0.12f, 0.3f, 6.24f, d3d::DARKRED)) return false;
	g_legowall[2].setPosition(4.56f, 0.12f, 0.0f); //��ġ ����
	if (false == g_legowall[3].create(Device, -1, -1, 0.12f, 0.3f, 6.24f, d3d::DARKRED)) return false;
	g_legowall[3].setPosition(-4.56f, 0.12f, 0.0f); //��ġ ����

	// create four balls and set the position 4���� ��ü ���� �� ��ġ ����
	for (i = 0; i < 4; i++) {
		if (false == g_sphere[i].create(Device, sphereColor[i])) return false; //��ü�� �����ϰ� ������ ����, ���� �� false ��ȯ
		g_sphere[i].setCenter(spherePos[i][0], (float)M_RADIUS, spherePos[i][1]); //��ü�� ��ġ ����(�������� ���� y ��ġ�� M_RADIUS �� ���)
		g_sphere[i].setPower(0, 0); //��ü�� �ʱ� �ӵ� ����(�ӵ� 0���� �ʱ�ȭ)
	}

	// create blue ball for set direction
	if (false == g_target_blueball.create(Device, d3d::BLUE)) return false; //�Ķ��� ��ü�� ����, ���� �� false ��ȯ
	g_target_blueball.setCenter(.0f, (float)M_RADIUS, 0.0f); //��ü�� ��ġ�� ����(�߾ӿ� ��ġ)

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

//�ڿ��� �����ϴ� �Լ�
void Cleanup(void)
{
	g_legoPlane.destroy(); //�ٴ� ��ü ����
	for (int i = 0; i < 4; i++) {
		g_legowall[i].destroy(); //4���� �� ��ü ����
	}
	destroyAllLegoBlock(); //��� Lego ��� ��ü ����
	g_light.destroy(); //���� ��ü ����
}


// timeDelta represents the time between the current image frame and the last image frame.
// the distance of moving balls should be "velocity * timeDelta"
//ȭ���� �������ϴ� ����
//timeDelta�� ���� �����Ӱ� ���� ������ ���� �ð� ���̸� ��Ÿ����, �̸� ����Ͽ� �� ��ü�� �̵��� ����

// ������ ��ư
RECT startButtonRect = { 300, 200, 700, 300 };
RECT infoRect = { startButtonRect.left, startButtonRect.top + 100, startButtonRect.right, startButtonRect.bottom + 100 };

bool Display(float timeDelta)
{
	int i = 0;
	int j = 0;

	if (Device) //����̽��� ��ȿ�� ��쿡�� ����
	{
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00afafaf, 1.0f, 0); //ȭ��� Z ���۸� �ʱ�ȭ(���� ����)
		Device->BeginScene(); //������ ����

		// update the position of each ball. during update, check whether each ball hit by walls.
		//�� ��ü�� ��ġ ������Ʈ(�ð���(timeDelta)�� ����Ͽ� �ӵ��� �°� �̵�)
		for (i = 0; i < 4; i++) {
			g_sphere[i].ballUpdate(timeDelta); //�� ��ü�� ��ġ�� ������Ʈ
			for (j = 0; j < 4; j++) {
				g_legowall[i].hitBy(g_sphere[j]); //�� ��ü�� ���� �浹�ߴ��� Ȯ��
			}
		}

		// check whether any two balls hit together and update the direction of balls
		//�� ��ü�� �浹�ߴ��� Ȯ���ϰ�, �浹 �� ��ü�� ���� ������Ʈ
		for (i = 0; i < 4; i++) {
			for (j = 0; j < 4; j++) {
				if (i >= j) { continue; } //�ڱ� �ڽŰ� ������ �ʵ��� ����
				g_sphere[i].hitBy(g_sphere[j]); //�� ��ü�� �浹 Ȯ��
			}
		}

		// draw plane, walls, and spheres ȭ�鿡 �׸� ��ü��
		g_legoPlane.draw(Device, g_mWorld); //�ٴ� �׸���
		for (i = 0; i < 4; i++) {
			g_legowall[i].draw(Device, g_mWorld); //�� �� �׸���
			g_sphere[i].draw(Device, g_mWorld); //�� ��ü �׸���
		}
		g_target_blueball.draw(Device, g_mWorld); //��ǥ �Ķ��� ��ü �׸���
		g_light.draw(Device); //���� ��ü �׸���
		
		if (!isGameStarted)
		{
			// Start ��ư�� �ȳ� ���� ǥ��
			D3DRECT fillRect = { startButtonRect.left, startButtonRect.top, startButtonRect.right, startButtonRect.bottom };
			Device->Clear(1, &fillRect, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

			// Start ��ư �ؽ�Ʈ�� �ȳ��� ���
			startg_Font->DrawText(0, "START", -1, &startButtonRect, DT_CENTER | DT_BOTTOM | DT_SINGLELINE, d3d::WHITE);

			D3DRECT rect = { infoRect.left, infoRect.top, infoRect.right, infoRect.bottom };
			Device->Clear(1, &rect, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
			infog_Font->DrawText(0, "press ESC to exit", -1, &infoRect, DT_CENTER | DT_TOP | DT_SINGLELINE, d3d::WHITE);
		}

		Device->EndScene(); //������ ��
		Device->Present(0, 0, 0, 0); //ȭ���� ���÷���
		Device->SetTexture(0, NULL); //�ؽ�ó�� ����
	}
	return true; //�Լ��� ���������� ������ true ��ȯ
}

//WndProc �Լ�:
//������ ���ν�����, �����쿡�� �߻��ϴ� �پ��� �̺�Ʈ(Ű �Է�, ���콺 �Է� ��)�� ó��
//WM_KEYDOWN �޽��������� Ű �Է��� ó���ϰ�, VK_ESCAPE, VK_RETURN, VK_SPACE Ű�� ���� ���� ������ ����
//WM_MOUSEMOVE �޽��������� ���콺 �������� ó���ϰ�, ���콺 ��ǥ�� ���� 3D ��ü(��ü, �Ķ��� ��ü ��)�� ��ġ�� ȸ���� ����
LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool wire = false; //wireframe ��� ���θ� �����ϴ� ����
	static bool isReset = true; //���콺�� ó�� Ŭ���� �� �ʱ�ȭ ����
	static int old_x = 0; //���� ���콺 x ��ǥ
	static int old_y = 0; //���� ���콺 y ��ǥ
	static enum { WORLD_MOVE, LIGHT_MOVE, BLOCK_MOVE } move = WORLD_MOVE; //��ü �̵� ���(�⺻�� WORLD_MOVE)
	static bool isTargetYellow = false; // ���� ���� ����������Ÿ��
	static bool worldMove = false; // ���尡 �����̴��� ����

	switch (msg) {
	case WM_DESTROY: //â�� ���� ��
	{
		::PostQuitMessage(0); //���� �޽��� ����
		break;
	}
	case WM_LBUTTONDOWN:
	{
		POINT cursorPos;
		GetCursorPos(&cursorPos);
		ScreenToClient(hwnd, &cursorPos);

		// Start ��ư Ŭ�� ���� Ȯ��
		if (!isGameStarted &&
			cursorPos.x >= startButtonRect.left && cursorPos.x <= startButtonRect.right &&
			cursorPos.y >= startButtonRect.top && cursorPos.y <= startButtonRect.bottom)
		{
			isGameStarted = true; 
			worldMove = true;
		}
	}
	break;

	case WM_KEYDOWN: //Ű�� ������ ��
	{
		// ������ ���۵Ǿ�߸� ������, �׷��� esc�� ������ ����
		if (!isGameStarted  && VK_ESCAPE == false) break;
		switch (wParam) {
		case VK_ESCAPE: //ESC Ű�� ������
			::DestroyWindow(hwnd); //â�� ����
			break;
		case VK_RETURN: //ENTER Ű�� ������
			if (NULL != Device) {
				wire = !wire; //wire ��带 ���
				Device->SetRenderState(D3DRS_FILLMODE,
					(wire ? D3DFILL_WIREFRAME : D3DFILL_SOLID)); //wireframe ���� solid ��� ��ȯ
			}
			break;
		case VK_SPACE: //SPACE Ű�� ������
		{
			int n;
			//Ÿ�ٰ� �Ͼ� ���� ��ġ�� �����ͼ� �� �� ������ ����� �Ÿ��� ���
			if (isTargetYellow) {
				n = 2;
			}
			else n = 3;
			isTargetYellow = !isTargetYellow;
			D3DXVECTOR3 targetpos = g_target_blueball.getCenter();
			D3DXVECTOR3	ballpos = g_sphere[n].getCenter();
			double theta = acos(sqrt(pow(targetpos.x - ballpos.x, 2)) / sqrt(pow(targetpos.x - ballpos.x, 2) +
				pow(targetpos.z - ballpos.z, 2)));		//������ ���(�⺻ 1 ��и�)
			//�� ��и鿡 ���� ������ ����
			if (targetpos.z - ballpos.z <= 0 && targetpos.x - ballpos.x >= 0) { theta = -theta; }	//4 ��и�
			if (targetpos.z - ballpos.z >= 0 && targetpos.x - ballpos.x <= 0) { theta = PI - theta; } //2 ��и�
			if (targetpos.z - ballpos.z <= 0 && targetpos.x - ballpos.x <= 0) { theta = PI + theta; } // 3 ��и�
			double distance = sqrt(pow(targetpos.x - ballpos.x, 2) + pow(targetpos.z - ballpos.z, 2)); //�� �� ������ �Ÿ� ���
			g_sphere[n].setPower(distance * cos(theta), distance * sin(theta)); //�Ͼ� ���� �ӵ� ����

			break;
		}
		case VK_BACK:
			worldMove= !worldMove; // ���� �̵� Ȱ��ȭ/��Ȱ��ȭ ���
			break;
		}
		break;
	}

	case WM_MOUSEMOVE: //���콺�� �̵��� ��
	{
		// ������ ���۵Ǿ�߸� ������
		if (!isGameStarted) break;
		int new_x = LOWORD(lParam); //���콺 x ��ǥ
		int new_y = HIWORD(lParam); //���콺 y ��ǥ
		float dx; //x ���� �̵� �Ÿ�
		float dy; //y ���� �̵� �Ÿ�

		if (LOWORD(wParam) & MK_LBUTTON) { //���� ���콺 ��ư�� ������ ��
			if (!worldMove) break; // worldmove�� �����Ǿ����� Ȯ��

			if (isReset) {
				isReset = false; //ù ��° Ŭ�� �� �ʱ�ȭ �÷��� ����
			}
			else {
				D3DXVECTOR3 vDist; //�̵� ����
				D3DXVECTOR3 vTrans; //�̵� ���� ����
				D3DXMATRIX mTrans; //��ȯ ���
				D3DXMATRIX mX; //x�� ȸ�� ���
				D3DXMATRIX mY; //y�� ȸ�� ���

				if(worldMove){
					switch (move) {
					case WORLD_MOVE: //���� �̵� ���
						dx = (old_x - new_x) * 0.01f; //���콺 �̵��� ���� x ���� ȸ�� �� ���
						dy = (old_y - new_y) * 0.01f; //���콺 �̵��� ���� y ���� ȸ�� �� ���
						D3DXMatrixRotationY(&mX, dx); //y�� ȸ�� ��� ����
						D3DXMatrixRotationX(&mY, dy); //x�� ȸ�� ��� ����
						g_mWorld = g_mWorld * mX * mY; //���� ���� ��Ŀ� ȸ�� ����

						break;
					}
				}
			}

			old_x = new_x; //���� ���콺 x��ǥ ����
			old_y = new_y; //���� ���콺 y��ǥ ����

		}
		else { //���콺 ���� ��ư�� ������ ��
			isReset = true; //�ʱ�ȭ �÷��� ����

			if (LOWORD(wParam) & MK_RBUTTON) { //������ ���콺 ��ư�� ������ ��
				dx = (old_x - new_x);// * 0.01f; x ���� �̵� �Ÿ�
				dy = (old_y - new_y);// * 0.01f; y ���� �̵� �Ÿ�

				D3DXVECTOR3 coord3d = g_target_blueball.getCenter(); //�Ķ��� ���� ���� ��ġ ��������
				g_target_blueball.setCenter(coord3d.x + dx * (-0.007f), coord3d.y, coord3d.z + dy * 0.007f); //���� ��ġ �̵�
			}
			old_x = new_x; //���� ���콺 x��ǥ ����
			old_y = new_y; //���� ���콺 y��ǥ ����

			if (worldMove) move = WORLD_MOVE; //�⺻ �̵� ���� ����
			
		}
		break;
	}
	}

	return ::DefWindowProc(hwnd, msg, wParam, lParam); //�⺻ ������ ���ν��� ȣ��
}

//WinMain �Լ�:
//���α׷��� ��Ʈ�� ����Ʈ��, Direct3D �ʱ�ȭ, ����, �޽��� ����, ���ҽ� ���� ���� ó��
//d3d::InitD3D�� Setup �Լ����� Direct3D ȯ���� �����ϰ�, d3d::EnterMsgLoop�� ���� �޽��� ������ ����
//Cleanup �Լ��� ���α׷� ���� �� ���ҽ��� �����ϰ�, Device->Release()�� Direct3D ����̽��� ����
int WINAPI WinMain(HINSTANCE hinstance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCmd)
{
	srand(static_cast<unsigned int>(time(NULL))); //���� �õ� ����

	//Direct3D �ʱ�ȭ
	if (!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0); //�ʱ�ȭ ���� �� �޽��� ���
		return 0;
	}
	//Setup �Լ� ȣ��
	if (!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0); //���� ���� �� �޽��� ���
		return 0;
	}
	//�޽��� ���� ����
	d3d::EnterMsgLoop(Display);
	//���ҽ� ����
	Cleanup();
	//����̽� ����
	Device->Release();

	return 0;
}