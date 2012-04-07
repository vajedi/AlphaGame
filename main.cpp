/*-------------------------------------------------------------------------------------*\
|                                                                                       |
|                      Hej                                                              |
|                                                                                       |
\*-------------------------------------------------------------------------------------*/

// include the basic windows header files and the Direct3D header file
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

#define SAFE_RELEASE(x) if( x ) { (x)->Release(); (x) = NULL; }
#define SAFE_DELETE(x) if( x ) { delete(x); (x) = NULL; }
#define SAFE_DELETE_ARRAY(x) if( x ) { delete [] (x); (x) = NULL; }
#define SHOWERROR(s,f,l) char buf[1024]; sprintf( buf, "File: %s\nLine: %d\n%s",f,l,s); MessageBox( 0, buf, "Error", 0 );

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

LPDIRECT3D9 d3d;
LPDIRECT3DDEVICE9 d3ddev;
LPDIRECT3DVERTEXBUFFER9 v_buffer = NULL;

struct CUSTOMVERTEX {FLOAT X, Y, Z; DWORD COLOR;};
#define CUSTOMFVF (D3DFVF_XYZ | D3DFVF_DIFFUSE)

INT64 m_ticksPerSecond;
INT64 m_currentTime;
float m_timeElapsed = 0;
INT64 m_lastTime;
float m_runningTime;
float m_fps;
int m_numFrames = 0;
INT64 m_lastFPSUpdate = 0;
INT64 m_FPSUpdateInterval = 0;
LPD3DXFONT m_font;

void DisplaySomeText()
{
    D3DCOLOR fontColor = D3DCOLOR_ARGB(255,255,255,255);    

    RECT rct;
    rct.left = 20; rct.right = 780;
    rct.top = 10; rct.bottom = rct.top + 20;
    char* str = new char[30];
 
    sprintf(str, "%.5g", m_fps );

    m_font->DrawText(NULL, "FPS:", -1, &rct, 0, fontColor );
    rct.left = 80; rct.right = 780;
    rct.top = 10; rct.bottom = rct.top + 20;
    m_font->DrawText(NULL, str, -1, &rct, 0, fontColor );
}

void initGraphics(void)
{
    CUSTOMVERTEX vertices[] = 
    {
        { 3.0f, -3.0f, 0.0f, D3DCOLOR_XRGB(0, 0, 255), },
        { 0.0f, 3.0f, 0.0f, D3DCOLOR_XRGB(0, 255, 0), },
        { -3.0f, -3.0f, 0.0f, D3DCOLOR_XRGB(255, 0, 0), },
    };

    d3ddev->CreateVertexBuffer(3*sizeof(CUSTOMVERTEX),
                               0,
                               CUSTOMFVF,
                               D3DPOOL_MANAGED,
                               &v_buffer,
                               NULL);

    VOID* pVoid;

    v_buffer->Lock(0, 0, (void**)&pVoid, 0);
    memcpy(pVoid, vertices, sizeof(vertices));
    v_buffer->Unlock();
}

void initD3D(HWND hWnd)
{
    d3d = Direct3DCreate9(D3D_SDK_VERSION);

    D3DPRESENT_PARAMETERS d3dpp;

    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = hWnd;
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
    d3dpp.BackBufferWidth = SCREEN_WIDTH;
    d3dpp.BackBufferHeight = SCREEN_HEIGHT;

    d3d->CreateDevice(D3DADAPTER_DEFAULT,
                      D3DDEVTYPE_HAL,
                      hWnd,
                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                      &d3dpp,
                      &d3ddev);

    initGraphics();

    d3ddev->SetRenderState(D3DRS_LIGHTING, FALSE);
}

void initFont()
{
    D3DXCreateFont( d3ddev, 15, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &m_font );
}

void updateFPS()
{
    QueryPerformanceCounter( (LARGE_INTEGER *)&m_currentTime );
    
    m_timeElapsed = (float)(m_currentTime - m_lastTime)/(float)m_ticksPerSecond;
    m_runningTime += m_timeElapsed;

    m_numFrames++;
    //if ( m_currentTime - m_lastFPSUpdate >= m_FPSUpdateInterval )
    //{
        float currentTime = (float)m_currentTime / (float)m_ticksPerSecond;
        float lastTime = (float)m_lastFPSUpdate / (float)m_ticksPerSecond;
        m_fps = (float)m_numFrames / (float)(currentTime - lastTime);

        m_lastFPSUpdate = m_currentTime;
        m_numFrames = 0;
    //}

    m_lastTime = m_currentTime;
}

void update()
{
    updateFPS();
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            } break;
    }

    return DefWindowProc (hWnd, message, wParam, lParam);
}

void render(void)
{
    d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);DisplaySomeText();
    d3ddev->BeginScene();

    d3ddev->SetFVF(CUSTOMFVF);
    
    D3DXMATRIX matRotateY;
    static float index = 0.0f; index+=0.05f;
    D3DXMatrixRotationY(&matRotateY, index);
    d3ddev->SetTransform(D3DTS_WORLD, &matRotateY);

    D3DXMATRIX matView;
    D3DXMatrixLookAtLH(&matView,
                       &D3DXVECTOR3 (0.0f, 0.0f, 10.0f),    // the camera position
                       &D3DXVECTOR3 (0.0f, 0.0f, 0.0f),     // the look-at position
                       &D3DXVECTOR3 (0.0f, 1.0f, 0.0f));    // the up direction
    d3ddev->SetTransform(D3DTS_VIEW, &matView);

    D3DXMATRIX matProjection;                                              // the projection transform matrix
    D3DXMatrixPerspectiveFovLH(&matProjection,
                               D3DXToRadian(45),                           // the horizontal field of view
                               (FLOAT)SCREEN_WIDTH / (FLOAT)SCREEN_HEIGHT, // aspect ratio
                               1.0f,                                       // the near view-plane
                               100.0f);                                    // the far view-plane
    d3ddev->SetTransform(D3DTS_PROJECTION, &matProjection);

    d3ddev->SetStreamSource(0, v_buffer, 0, sizeof(CUSTOMVERTEX));
    d3ddev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);

    d3ddev->EndScene();
    d3ddev->Present(NULL, NULL, NULL, NULL);
}

void clean(void)
{
    /*SAFE_RELEASE( m_pTextSprite );
    SAFE_RELEASE( m_pFont );*/m_font->Release();
    v_buffer->Release();
    d3ddev->Release();
    d3d->Release();
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    HWND hWnd;
    WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "WindowClass";

    RegisterClassEx(&wc);

    hWnd = CreateWindowEx(NULL, "WindowClass", "The Direct3D Program",
                          WS_OVERLAPPEDWINDOW, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                          NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);

    initD3D(hWnd);
    initFont();
    QueryPerformanceFrequency( (LARGE_INTEGER *)&m_ticksPerSecond );
    
    MSG msg;
    while(TRUE)
    {
        
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
			if ( msg.message == WM_QUIT)
            {
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
		else
		{
            update();
	        render();
		}
    }
        
    clean();
    return msg.wParam;
}