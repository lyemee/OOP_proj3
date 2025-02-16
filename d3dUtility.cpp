#include "d3dUtility.h"

//Direct3D ����̽��� �ʱ�ȭ
bool d3d::InitD3D(
	HINSTANCE hInstance, //���α׷� �ν��Ͻ� �ڵ�
	int width, int height, //â�� �ʺ�� ����
	bool windowed, //â ���(true) �Ǵ� ��ü ȭ��(false)
	D3DDEVTYPE deviceType, //����̽� ����
	IDirect3DDevice9** device) //���: ������ Direct3D ����̽�
{
	//
	// Create the main application window.
	//

	//WINDCLASS ����ü �ʱ�ȭ(������ Ŭ���� ��Ͽ� ���)
	WNDCLASS wc; //������ Ŭ���� ����ü ����
	wc.style = CS_HREDRAW | CS_VREDRAW; //â �������� �� ����/���� �������� �ٽ� �׸���
	wc.lpfnWndProc = (WNDPROC)d3d::WndProc; //������ ���ν��� �Լ� ������
	wc.cbClsExtra = 0; //Ŭ���� ����ü�� �߰����� ������ ����
	wc.cbWndExtra = 0; //â ����ü�� �߰����� ������ ����
	wc.hInstance = hInstance; //���� ���ø����̼��� �ν��Ͻ� �ڵ�
	wc.hIcon = LoadIcon(0, IDI_APPLICATION); //�⺻ ���ø����̼� ������
	wc.hCursor = LoadCursor(0, IDC_ARROW); //�⺻ ȭ��ǥ Ŀ��
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //��� ��� �귯��
	wc.lpszMenuName = 0; //�޴� ����
	wc.lpszClassName = "Direct3D9App"; //������ Ŭ���� �̸� ����

	//RegisterClass�� ���� ������ Ŭ���� ���
	if (!RegisterClass(&wc))
	{
		::MessageBox(0, "RegisterClass() - FAILED", 0, 0); //������ Ŭ���� ��� ���� �� ���� �޽��� ���
		return false; //�ʱ�ȭ ���� ��ȯ
	}

	//������ �ڵ� ���� �ʱ�ȭ
	HWND hwnd = 0;

	//CreateWindow�� ȣ���� �� ������ ����
	hwnd = ::CreateWindow(
		"Direct3D9App", //��ϵ� ������ Ŭ���� �̸�
		"Virtual Billiard", //������ ����
		WS_EX_TOPMOST, //�ֻ��� â ����
		0, 0, width, height, //â�� ��ġ(0,0) �� ũ��(width, height)
		0 /*parent hwnd*/, //�θ� ������ ����
		0 /* menu */, //�޴� ����
		hInstance, //���ø����̼� �ν��Ͻ� �ڵ�
		0 /*extra*/); //�߰� �Ű����� ����

	//������ ���� ���� �� ���� �޽��� ���
	if (!hwnd)
	{
		::MessageBox(0, "CreateWindow() - FAILED", 0, 0); //���� ���� �޽��� ���
		return false; //�ʱ�ȭ ���� ��ȯ
	}

	//ShowWindow�� ȣ���� â ǥ��
	::ShowWindow(hwnd, SW_SHOW); //â�� ������
	::UpdateWindow(hwnd); //â�� ������Ʈ�Ͽ� �׸���

	//
	// Init D3D:
	//

	//Direct3D �ʱ�ȭ �� �ܰ� ����
	HRESULT hr = 0;

	// Step 1: Create the IDirect3D9 object. IDirect3D9 ��ü ����
	IDirect3D9* d3d9 = 0; //IDirect3D9 ��ü ������ ����
	d3d9 = Direct3DCreate9(D3D_SDK_VERSION); //Direct3D 9 ��ü ����

	//Direct3D ��ü ���� ���� ó��
	if (!d3d9)
	{
		::MessageBox(0, "Direct3DCreate9() - FAILED", 0, 0); //���� �޽��� ���
		return false; //�ʱ�ȭ ���� ��ȯ
	}

	// Step 2: Check for hardware vp. �ϵ���� ��ȯ �� ���� ���� ���� Ȯ��
	D3DCAPS9 caps; //����̽� ��� ĸ��ȭ ����ü ����
	d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, deviceType, &caps); //�⺻ ����Ϳ� ����̽� Ÿ���� ��� ��������

	int vp = 0; //���ؽ� ó�� ������ ����
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING; //�ϵ���� ���ؽ� ó�� ���� �� ����
	else
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING; //�׷��� ������ ����Ʈ���� ���ؽ� ó�� ���

	// Step 3: Fill out the D3DPRESENT_PARAMETERS structure. D3DPRESENT_PARAMETERS ����ü �ʱ�ȭ
	RECT rc; //Ŭ���̾�Ʈ ������ ũ�⸦ �������� ���� �簢�� ����ü
	GetClientRect(hwnd, &rc); //Ŭ���̾�Ʈ ���� ũ�� ��������
	UINT w = rc.right - rc.left; //�ʺ� ���
	UINT h = rc.bottom - rc.top; //���� ���
	D3DPRESENT_PARAMETERS d3dpp; //Direct3D ������Ʈ �Ķ���� ����ü ���� �� �ʱ�ȭ
	d3dpp.BackBufferWidth = w; //�� ���� �ʺ� ����
	d3dpp.BackBufferHeight = h; //�� ���� ���� ����
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8; //32��Ʈ ARGB ���� ����
	d3dpp.BackBufferCount = 1; //�� ���� ���� ����
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE; //��Ƽ ���ø� ����(��Ȱ��ȭ)
	d3dpp.MultiSampleQuality = 0; //��Ƽ ���ø� ǰ�� �ʱ�ȭ
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD; //��ȯ ȿ�� ����
	d3dpp.hDeviceWindow = hwnd; //������ ��� ������ ����
	d3dpp.Windowed = windowed; //â ��� �Ǵ� ��ü ȭ�� ��� ����
	d3dpp.EnableAutoDepthStencil = true; //���� ���� ���(���� ���ٽ� Ȱ��ȭ)
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8; //24��Ʈ ����, 8��Ʈ ���ٽ�(����/���ٽ� ���� ����)
	d3dpp.Flags = 0; //�߰� �÷��� ����
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT; //�⺻ �������� �ӵ�
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; //������ �ӵ� ���� ����

	// Step 4: Create the device. Direct3D ����̽� ����
	hr = d3d9->CreateDevice(
		D3DADAPTER_DEFAULT, // primary adapter �⺻ ���÷��� �����
		deviceType,         // device type ����̽� ����
		hwnd,               // window associated with device ����̽��� ������ ������
		vp,                 // vertex processing ���ؽ� ó�� �÷���
		&d3dpp,             // present parameters ������Ʈ �Ķ����
		device);            // return created device ������ ����̽� ������ ��ȯ

	//����̽� ���� ���� �� ó��
	if (FAILED(hr))
	{
		// try again using a 16-bit depth buffer ����̽� ���� ���� �� 16��Ʈ ���� ���۷� ��õ�
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16; //����/���ٽ� ���� ����

		hr = d3d9->CreateDevice(
			D3DADAPTER_DEFAULT, //�⺻ ���÷��� �����
			deviceType, //����̽� Ÿ��
			hwnd, //����̽��� ������ ������
			vp,	//���ؽ� ó�� �÷���
			&d3dpp, //������Ʈ �Ķ����
			device); //������ ����̽� ������ ��ȯ

		//����̽� ���� ���� �� ���� ó��
		if (FAILED(hr))
		{
			d3d9->Release(); // done with d3d9 object Direct3D 9 ��ü ����
			::MessageBox(0, "CreateDevice() - FAILED", 0, 0); //���� �޽��� ���
			return false; //�ʱ�ȭ ���� ��ȯ
		}
	}

	d3d9->Release(); // done with d3d9 object Direct3D 9 ��ü ����

	return true; //�ʱ�ȭ ���� ��ȯ
}

//�޽��� ���� ó�� �Լ�
int d3d::EnterMsgLoop(bool (*ptr_display)(float timeDelta))
{
	MSG msg; // �޽��� ����ü ����
	::ZeroMemory(&msg, sizeof(MSG)); // �޽��� ����ü �ʱ�ȭ

	static double lastTime = (double)timeGetTime(); // �ð� �ʱ�ȭ

	// �޽��� ���� ����
	while (msg.message != WM_QUIT) {
		// ������� �ʰ� �޽��� ó��
		if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			::TranslateMessage(&msg); // Ű���� �޽��� ��ȯ
			::DispatchMessage(&msg); // �޽��� ó��
		}
		else {
			// ���ø����̼� �� ó��
			double currTime = (double)timeGetTime(); // ���� �ð� ���
			double timeDelta = (currTime - lastTime) * 0.0007; // ���� �ð� ���
			ptr_display((float)timeDelta); // ���÷��� �ݹ� ȣ��

			lastTime = currTime; // ������ �ð� ������Ʈ
		}
	}
	return msg.wParam; // �޽��� ���� ����� ��ȯ
}

//���⼺ ������ �ʱ�ȭ�ϴ� �Լ�
D3DLIGHT9 d3d::InitDirectionalLight(D3DXVECTOR3* direction, D3DXCOLOR* color)
{
	D3DLIGHT9 light; //���� ����ü ����
	::ZeroMemory(&light, sizeof(light)); //����ü �ʱ�ȭ

	light.Type = D3DLIGHT_DIRECTIONAL; //���� ������ ���⼺ �������� ����
	light.Ambient = *color * 0.6f; //�ֺ��� ���� (�� ����*0.6)
	light.Diffuse = *color; //Ȯ�걤 ����
	light.Specular = *color * 0.6f; //�ݻ籤 ����
	light.Direction = *direction; //���� ���� ����

	return light; //�ʱ�ȭ�� ���� ��ȯ
}

//�� ������ �ʱ�ȭ�ϴ� �Լ�
D3DLIGHT9 d3d::InitPointLight(D3DXVECTOR3* position, D3DXCOLOR* color)
{
	D3DLIGHT9 light; //���� ����ü ����
	::ZeroMemory(&light, sizeof(light)); //����ü �ʱ�ȭ

	light.Type = D3DLIGHT_POINT; //���� ������ �� �������� ����
	light.Ambient = *color * 0.6f; //�ֺ��� ���� (�� ����*0.6)
	light.Diffuse = *color; //Ȯ�걤 ����
	light.Specular = *color * 0.6f; //�ݻ籤 ����
	light.Position = *position; //���� ��ġ ����
	light.Range = 1000.0f; //������ �ִ� �Ÿ� ����
	light.Falloff = 1.0f; //���� ��� ����
	light.Attenuation0 = 1.0f; //���� ��� (��� ��) ����
	light.Attenuation1 = 0.0f; //���� ��� (1�� ��) ����
	light.Attenuation2 = 0.0f; //���� ��� (2�� ��) ����

	return light; //�ʱ�ȭ�� ���� ��ȯ
}

//����Ʈ����Ʈ�� �ʱ�ȭ�ϴ� �Լ�
D3DLIGHT9 d3d::InitSpotLight(D3DXVECTOR3* position, D3DXVECTOR3* direction, D3DXCOLOR* color)
{
	D3DLIGHT9 light; //���� ����ü ����
	::ZeroMemory(&light, sizeof(light)); //����ü �ʱ�ȭ

	light.Type = D3DLIGHT_SPOT; //���� ������ ����Ʈ����Ʈ�� ����
	light.Ambient = *color * 0.0f; //�ֺ��� ���� (�� ����*0.6)
	light.Diffuse = *color; //�ݻ籤 ����
	light.Specular = *color * 0.6f; //Ȯ�걤 ����
	light.Position = *position; //����Ʈ����Ʈ ��ġ ����
	light.Direction = *direction; //����Ʈ����Ʈ ���� ����
	light.Range = 1000.0f; //������ �ִ� �Ÿ� ����
	light.Falloff = 1.0f; //���� ��� ����
	light.Attenuation0 = 1.0f; //���� ��� (��� ��) ����
	light.Attenuation1 = 0.0f; //���� ��� (1�� ��) ����
	light.Attenuation2 = 0.0f; //���� ��� (2�� ��) ����
	light.Theta = 0.4f; //���� ���� ����(����) ����
	light.Phi = 0.9f; //�ܺ� ���� ����(����) ����

	return light; //�ʱ�ȭ�� ���� ��ȯ
}

//������ �ʱ�ȭ�ϴ� �Լ�
D3DMATERIAL9 d3d::InitMtrl(D3DXCOLOR a, D3DXCOLOR d, D3DXCOLOR s, D3DXCOLOR e, float p)
{
	D3DMATERIAL9 mtrl; //���� ����ü ����
	mtrl.Ambient = a; //�ֺ��� ����
	mtrl.Diffuse = d; //Ȯ�걤 ����
	mtrl.Specular = s; //�ݻ籤 ����
	mtrl.Emissive = e; //�߱� ����
	mtrl.Power = p; //���õ� ����
	return mtrl; //�ʱ�ȭ�� ���� ��ȯ
}

//BoundingBox ������
d3d::BoundingBox::BoundingBox()
{
	// infinite small  �ּҰ��� ���Ѵ��, �ִ밪�� ���� ���Ѵ�� �����Ͽ� �ʱ�ȭ
	_min.x = INFINITY; //x�� �ּҰ�
	_min.y = INFINITY; //y�� �ּҰ�
	_min.z = INFINITY; //z�� �ּҰ�

	_max.x = -INFINITY; //x�� �ִ밪
	_max.y = -INFINITY; //y�� �ִ밪
	_max.z = -INFINITY; //z�� �ִ밪
}

//�־��� ���� BoundingBox ���ο� �ִ��� Ȯ���ϴ� �Լ�
bool d3d::BoundingBox::isPointInside(D3DXVECTOR3& p)
{
	//���� BoundingBox�� �ּҰ��� �ִ밪 ���̿� ��ġ�ϴ��� Ȯ��
	if (p.x >= _min.x && p.y >= _min.y && p.z >= _min.z &&
		p.x <= _max.x && p.y <= _max.y && p.z <= _max.z)
	{
		return true; //���ο� ����
	}
	else
	{
		return false; //���ο� ����
	}
}

//BoundingSphere ������
d3d::BoundingSphere::BoundingSphere()
{
	_radius = 0.0f; //�������� 0���� �ʱ�ȭ
}
