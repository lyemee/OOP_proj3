#include "d3dUtility.h"

//Direct3D 디바이스를 초기화
bool d3d::InitD3D(
	HINSTANCE hInstance, //프로그램 인스턴스 핸들
	int width, int height, //창의 너비와 높이
	bool windowed, //창 모드(true) 또는 전체 화면(false)
	D3DDEVTYPE deviceType, //디바이스 유형
	IDirect3DDevice9** device) //출력: 생성된 Direct3D 디바이스
{
	//
	// Create the main application window.
	//

	//WINDCLASS 구조체 초기화(윈도우 클래스 등록에 사용)
	WNDCLASS wc; //윈도우 클래스 구조체 선언
	wc.style = CS_HREDRAW | CS_VREDRAW; //창 리사이즈 시 가로/세로 방향으로 다시 그리기
	wc.lpfnWndProc = (WNDPROC)d3d::WndProc; //윈도우 프로시저 함수 포인터
	wc.cbClsExtra = 0; //클래스 구조체에 추가적인 데이터 없음
	wc.cbWndExtra = 0; //창 구조체에 추가적인 데이터 없음
	wc.hInstance = hInstance; //현재 애플리케이션의 인스턴스 핸들
	wc.hIcon = LoadIcon(0, IDI_APPLICATION); //기본 애플리케이션 아이콘
	wc.hCursor = LoadCursor(0, IDC_ARROW); //기본 화살표 커서
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); //흰색 배경 브러시
	wc.lpszMenuName = 0; //메뉴 없음
	wc.lpszClassName = "Direct3D9App"; //윈도우 클래스 이름 지정

	//RegisterClass를 통해 윈도우 클래스 등록
	if (!RegisterClass(&wc))
	{
		::MessageBox(0, "RegisterClass() - FAILED", 0, 0); //윈도우 클래스 등록 실패 시 에러 메시지 출력
		return false; //초기화 실패 반환
	}

	//윈도우 핸들 변수 초기화
	HWND hwnd = 0;

	//CreateWindow를 호출해 새 윈도우 생성
	hwnd = ::CreateWindow(
		"Direct3D9App", //등록된 윈도우 클래스 이름
		"Virtual Billiard", //윈도우 제목
		WS_EX_TOPMOST, //최상위 창 설정
		0, 0, width, height, //창의 위치(0,0) 및 크기(width, height)
		0 /*parent hwnd*/, //부모 윈도우 없음
		0 /* menu */, //메뉴 없음
		hInstance, //애플리케이션 인스턴스 핸들
		0 /*extra*/); //추가 매개변수 없음

	//윈도우 생성 실패 시 에러 메시지 출력
	if (!hwnd)
	{
		::MessageBox(0, "CreateWindow() - FAILED", 0, 0); //생성 실패 메시지 출력
		return false; //초기화 실패 반환
	}

	//ShowWindow를 호출해 창 표시
	::ShowWindow(hwnd, SW_SHOW); //창을 보여줌
	::UpdateWindow(hwnd); //창을 업데이트하여 그리기

	//
	// Init D3D:
	//

	//Direct3D 초기화 및 단계 시작
	HRESULT hr = 0;

	// Step 1: Create the IDirect3D9 object. IDirect3D9 객체 생성
	IDirect3D9* d3d9 = 0; //IDirect3D9 객체 포인터 선언
	d3d9 = Direct3DCreate9(D3D_SDK_VERSION); //Direct3D 9 객체 생성

	//Direct3D 객체 생성 실패 처리
	if (!d3d9)
	{
		::MessageBox(0, "Direct3DCreate9() - FAILED", 0, 0); //에러 메시지 출력
		return false; //초기화 실패 반환
	}

	// Step 2: Check for hardware vp. 하드웨어 변환 및 조명 지원 여부 확인
	D3DCAPS9 caps; //디바이스 기능 캡슐화 구조체 선언
	d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, deviceType, &caps); //기본 어댑터와 디바이스 타입의 기능 가져오기

	int vp = 0; //버텍스 처리 츨래그 변수
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING; //하드웨어 버텍스 처리 지원 시 설정
	else
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING; //그렇지 않으면 소프트웨어 버텍스 처리 사용

	// Step 3: Fill out the D3DPRESENT_PARAMETERS structure. D3DPRESENT_PARAMETERS 구조체 초기화
	RECT rc; //클라이언트 영역의 크기를 가져오기 위한 사각형 구조체
	GetClientRect(hwnd, &rc); //클라이언트 영역 크기 가져오기
	UINT w = rc.right - rc.left; //너비 계산
	UINT h = rc.bottom - rc.top; //높이 계산
	D3DPRESENT_PARAMETERS d3dpp; //Direct3D 프레젠트 파라미터 구조체 선언 및 초기화
	d3dpp.BackBufferWidth = w; //백 버퍼 너비 설정
	d3dpp.BackBufferHeight = h; //백 버퍼 높이 설정
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8; //32비트 ARGB 색상 포맷
	d3dpp.BackBufferCount = 1; //백 버퍼 개수 설정
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE; //멀티 샘플링 없음(비활성화)
	d3dpp.MultiSampleQuality = 0; //멀티 샘플링 품질 초기화
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD; //교환 효과 설정
	d3dpp.hDeviceWindow = hwnd; //렌더링 대상 윈도우 설정
	d3dpp.Windowed = windowed; //창 모드 또는 전체 화면 모드 설정
	d3dpp.EnableAutoDepthStencil = true; //깊이 버퍼 사용(깊이 스텐실 활성화)
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8; //24비트 깊이, 8비트 스텐실(깊이/스텐실 포맷 설정)
	d3dpp.Flags = 0; //추가 플래그 없음
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT; //기본 리프레시 속도
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE; //프레임 속도 제한 없음

	// Step 4: Create the device. Direct3D 디바이스 생성
	hr = d3d9->CreateDevice(
		D3DADAPTER_DEFAULT, // primary adapter 기본 디스플레이 어댑터
		deviceType,         // device type 디바이스 유형
		hwnd,               // window associated with device 디바이스를 연결할 윈도우
		vp,                 // vertex processing 버텍스 처리 플래그
		&d3dpp,             // present parameters 프레젠트 파라미터
		device);            // return created device 생성된 디바이스 포인터 반환

	//디바이스 생성 실패 시 처리
	if (FAILED(hr))
	{
		// try again using a 16-bit depth buffer 디바이스 생성 실패 시 16비트 깊이 버퍼로 재시도
		d3dpp.AutoDepthStencilFormat = D3DFMT_D16; //깊이/스텐실 포맷 변경

		hr = d3d9->CreateDevice(
			D3DADAPTER_DEFAULT, //기본 디스플레이 어댑터
			deviceType, //디바이스 타입
			hwnd, //디바이스를 연결할 윈도우
			vp,	//버텍스 처리 플래그
			&d3dpp, //프레젠트 파라미터
			device); //생성된 디바이스 포인터 반환

		//디바이스 생성 실패 시 에러 처리
		if (FAILED(hr))
		{
			d3d9->Release(); // done with d3d9 object Direct3D 9 객체 해제
			::MessageBox(0, "CreateDevice() - FAILED", 0, 0); //에러 메시지 출력
			return false; //초기화 실패 반환
		}
	}

	d3d9->Release(); // done with d3d9 object Direct3D 9 객체 해제

	return true; //초기화 성공 반환
}

//메시지 루프 처리 함수
int d3d::EnterMsgLoop(bool (*ptr_display)(float timeDelta))
{
	MSG msg; // 메시지 구조체 선언
	::ZeroMemory(&msg, sizeof(MSG)); // 메시지 구조체 초기화

	static double lastTime = (double)timeGetTime(); // 시간 초기화

	// 메시지 루프 시작
	while (msg.message != WM_QUIT) {
		// 대기하지 않고 메시지 처리
		if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			::TranslateMessage(&msg); // 키보드 메시지 변환
			::DispatchMessage(&msg); // 메시지 처리
		}
		else {
			// 애플리케이션 논리 처리
			double currTime = (double)timeGetTime(); // 현재 시간 계산
			double timeDelta = (currTime - lastTime) * 0.0007; // 지난 시간 계산
			ptr_display((float)timeDelta); // 디스플레이 콜백 호출

			lastTime = currTime; // 마지막 시간 업데이트
		}
	}
	return msg.wParam; // 메시지 루프 종료시 반환
}

//방향성 조명을 초기화하는 함수
D3DLIGHT9 d3d::InitDirectionalLight(D3DXVECTOR3* direction, D3DXCOLOR* color)
{
	D3DLIGHT9 light; //조명 구조체 선언
	::ZeroMemory(&light, sizeof(light)); //구조체 초기화

	light.Type = D3DLIGHT_DIRECTIONAL; //조명 유형을 방향성 조명으로 설정
	light.Ambient = *color * 0.6f; //주변광 설정 (주 색상*0.6)
	light.Diffuse = *color; //확산광 설정
	light.Specular = *color * 0.6f; //반사광 설정
	light.Direction = *direction; //조명 방향 설정

	return light; //초기화된 조명 반환
}

//점 조명을 초기화하는 함수
D3DLIGHT9 d3d::InitPointLight(D3DXVECTOR3* position, D3DXCOLOR* color)
{
	D3DLIGHT9 light; //조명 구조체 선언
	::ZeroMemory(&light, sizeof(light)); //구조체 초기화

	light.Type = D3DLIGHT_POINT; //조명 유형을 점 조명으로 설정
	light.Ambient = *color * 0.6f; //주변광 설정 (주 색상*0.6)
	light.Diffuse = *color; //확산광 설정
	light.Specular = *color * 0.6f; //반사광 설정
	light.Position = *position; //조명 위치 설정
	light.Range = 1000.0f; //조명의 최대 거리 설정
	light.Falloff = 1.0f; //감쇠 계수 설정
	light.Attenuation0 = 1.0f; //감쇠 계수 (상수 항) 설정
	light.Attenuation1 = 0.0f; //감쇠 계수 (1차 항) 설정
	light.Attenuation2 = 0.0f; //감쇠 계수 (2차 항) 설정

	return light; //초기화된 조명 반환
}

//스포트라이트를 초기화하는 함수
D3DLIGHT9 d3d::InitSpotLight(D3DXVECTOR3* position, D3DXVECTOR3* direction, D3DXCOLOR* color)
{
	D3DLIGHT9 light; //조명 구조체 선언
	::ZeroMemory(&light, sizeof(light)); //구조체 초기화

	light.Type = D3DLIGHT_SPOT; //조명 유형을 스포트라이트로 설정
	light.Ambient = *color * 0.0f; //주변광 설정 (주 색상*0.6)
	light.Diffuse = *color; //반사광 설정
	light.Specular = *color * 0.6f; //확산광 설정
	light.Position = *position; //스포트라이트 위치 설정
	light.Direction = *direction; //스포트라이트 방향 설정
	light.Range = 1000.0f; //조명의 최대 거리 설정
	light.Falloff = 1.0f; //감쇠 계수 설정
	light.Attenuation0 = 1.0f; //감쇠 계수 (상수 항) 설정
	light.Attenuation1 = 0.0f; //감쇠 계수 (1차 항) 설정
	light.Attenuation2 = 0.0f; //감쇠 계수 (2차 항) 설정
	light.Theta = 0.4f; //내부 원뿔 각도(라디안) 설정
	light.Phi = 0.9f; //외부 원뿔 각도(라디안) 설정

	return light; //초기화된 조명 반환
}

//재질을 초기화하는 함수
D3DMATERIAL9 d3d::InitMtrl(D3DXCOLOR a, D3DXCOLOR d, D3DXCOLOR s, D3DXCOLOR e, float p)
{
	D3DMATERIAL9 mtrl; //재질 구조체 선언
	mtrl.Ambient = a; //주변광 설정
	mtrl.Diffuse = d; //확산광 설정
	mtrl.Specular = s; //반사광 설정
	mtrl.Emissive = e; //발광 설정
	mtrl.Power = p; //광택도 설정
	return mtrl; //초기화된 재질 반환
}

//BoundingBox 생성자
d3d::BoundingBox::BoundingBox()
{
	// infinite small  최소값을 무한대로, 최대값을 음의 무한대로 설정하여 초기화
	_min.x = INFINITY; //x축 최소값
	_min.y = INFINITY; //y축 최소값
	_min.z = INFINITY; //z축 최소값

	_max.x = -INFINITY; //x축 최대값
	_max.y = -INFINITY; //y축 최대값
	_max.z = -INFINITY; //z축 최대값
}

//주어진 점이 BoundingBox 내부에 있는지 확인하는 함수
bool d3d::BoundingBox::isPointInside(D3DXVECTOR3& p)
{
	//점이 BoundingBox의 최소값과 최대값 사이에 위치하는지 확인
	if (p.x >= _min.x && p.y >= _min.y && p.z >= _min.z &&
		p.x <= _max.x && p.y <= _max.y && p.z <= _max.z)
	{
		return true; //내부에 있음
	}
	else
	{
		return false; //내부에 없음
	}
}

//BoundingSphere 생성자
d3d::BoundingSphere::BoundingSphere()
{
	_radius = 0.0f; //반지름을 0으로 초기화
}
