#include "WindowManager/WindowManager.h"

Demo::WindowManager::WindowManager(InputManager* inputManager, VulkanManager* vulkanManager)
{
    inputManagerRef = inputManager;
    vulkanManagerRef = vulkanManager;
    std::cout << "WindowManager::ctor()" << std::endl;
}

Demo::WindowManager::WindowManager()
{
    std::cout << "WindowManager::ctor()" << std::endl;
}

Demo::WindowManager::~WindowManager()
{
    std::cout << "WindowManager::dtor()" << std::endl;
}

bool Demo::WindowManager::windowShouldClose(DemoWindow window)
{
    if (!exitFlag)
    {
        return window.windowShouldClose;
    }
    else
    {
        return true;
    }
}

Demo::DemoWindow Demo::WindowManager::createStandardWindow(int width, int height, const char* applicationName)
{
	DemoWindow window;
	window.width = width;
	window.height = height;
	window.windowTitle = applicationName;
	window.windowClassName = applicationName;
	window.hInstance = GetModuleHandle(NULL);

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wcex.lpfnWndProc = WndProcRelay;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = window.hInstance;
	wcex.hIcon = LoadIcon(NULL, IDI_WINLOGO);;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = window.windowClassName;
	wcex.hIconSm = LoadIcon(NULL, IDI_WINLOGO);;

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, "Call to RegisterClassEx failed! When creating the window, you have to specify a unique application name. After Window creation, it can be changed to whatever you like", "ERROR!", NULL);
	}

	DWORD currentScreenWidth = ::GetSystemMetrics(SM_CXFULLSCREEN);
	DWORD currentScreenHeight = ::GetSystemMetrics(SM_CYFULLSCREEN);
	int x = (currentScreenWidth - width) / 2;
	int y = (currentScreenHeight - height) / 2;
	window.posX = x;
	window.posY = y;

	window.hWnd = CreateWindowExA(0,//DWEX style
		window.windowClassName,		//Window class name
		window.windowTitle,			//Window Title
		WS_OVERLAPPEDWINDOW,		//Style
		window.posX,				//x
		window.posY,				//y
		window.width,				//width
		window.height,				//height
		NULL,						//Parent-Window
		NULL,						//HMenu
		NULL,			            //hInstance
		this);						//lParam

	if (!window.hWnd)
	{
		MessageBox(NULL, "Call to CreateWindow failed!", "ERROR!", NULL);
	}


	UpdateWindow(window.hWnd);
	ShowWindow(window.hWnd, SW_SHOW);
	SetForegroundWindow(window.hWnd);
	SetFocus(window.hWnd);


	return window;
}

LRESULT Demo::WindowManager::WndProcRelay(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static WindowManager* wndmgr = NULL;
	if (uMsg == WM_CREATE)
	{
		wndmgr = (WindowManager*)((LPCREATESTRUCT)lParam)->lpCreateParams;
	}
	if (wndmgr)
		return wndmgr->WindowProc(hwnd, uMsg, wParam, lParam);
	else
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT Demo::WindowManager::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_LBUTTONDOWN:
    {
        if (inputManagerRef != nullptr)
        {
            if (inputManagerRef->mouse != nullptr)
            {
                inputManagerRef->mouse->registerKey(uMsg, wParam);
            }
        }
        break;
    }
    case WM_LBUTTONUP:
    {
        if (inputManagerRef != nullptr)
        {
            if (inputManagerRef->mouse != nullptr)
            {
                inputManagerRef->mouse->registerKey(uMsg, wParam);
            }
        }
        break;
    }
    case WM_MBUTTONDOWN:
    {
        if (inputManagerRef != nullptr)
        {
            if (inputManagerRef->mouse != nullptr)
            {
                inputManagerRef->mouse->registerKey(uMsg, wParam);
            }
        }
        break;
    }
    case WM_MBUTTONUP:
    {
        if (inputManagerRef != nullptr)
        {
            if (inputManagerRef->mouse != nullptr)
            {
                inputManagerRef->mouse->registerKey(uMsg, wParam);
            }
        }
        break;
    }
    case WM_RBUTTONDOWN:
    {
        if (inputManagerRef != nullptr)
        {
            if (inputManagerRef->mouse != nullptr)
            {
                inputManagerRef->mouse->registerKey(uMsg, wParam);
            }
        }
        break;
    }
    case WM_RBUTTONUP:
    {
        if (inputManagerRef != nullptr)
        {
            if (inputManagerRef->mouse != nullptr)
            {
                inputManagerRef->mouse->registerKey(uMsg, wParam);
            }
        }
        break;
    }
    case WM_CREATE:
    {
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
        // All painting occurs here, between BeginPaint and EndPaint.
        FillRect(hdc, &ps.rcPaint, brush);
        EndPaint(hWnd, &ps);
        if (vulkanManagerRef != nullptr)
        {
            if (vulkanManagerRef->prepared)
            {
                vulkanManagerRef->render();
            }
        }
        break;
    }
    case WM_CLOSE:
    {
        exitFlag = true;
        if (vulkanManagerRef)
        {
            vulkanManagerRef->exitFlag = true;
        }
        PostQuitMessage(0);
        break;
    }
    case WM_SIZE:
    {
        if (vulkanManagerRef != nullptr)
        {
            int32_t w_width = LOWORD(lParam);
            int32_t w_height = HIWORD(lParam);
            if (w_width == 0 || w_height == 0)
            {
                vulkanManagerRef->prepared = false;
            }
            else
            {
                vulkanManagerRef->onResize(w_width, w_height);
            }
        }
        break;
    }
    case WM_SETCURSOR:
    {
        if (customWindowFrame)
        {
            return TRUE;
        }
        break;
    }
    case WM_GETMINMAXINFO:
    {
        LPMINMAXINFO mmi = (LPMINMAXINFO)lParam;
        mmi->ptMinTrackSize.x = 64;
        mmi->ptMinTrackSize.y = 64;
    }
    case WM_DESTROY:
    {
        break;
    }
    case WM_MOUSEWHEEL:
    {
        if (inputManagerRef != nullptr)
        {
            if (inputManagerRef->mouse != nullptr)
            {
                inputManagerRef->mouse->registerScroll(GET_WHEEL_DELTA_WPARAM(wParam));
            }
        }
        break;
    }
    case WM_MOUSEMOVE:
    {
        int32_t xpos = GET_X_LPARAM(lParam);
        int32_t ypos = GET_Y_LPARAM(lParam);
        if (inputManagerRef != nullptr)
        {
            if (inputManagerRef->mouse != nullptr)
            {
                inputManagerRef->mouse->registerMovement(xpos, ypos);
            }
        }
        break;
    }
    case WM_KEYDOWN:
    {
        bool keyWasDown = ((lParam & (1 << 30)) != 0);
        bool keyWasUp = ((lParam & (1 << 31)) == 0);
        if (keyWasUp && !keyWasDown)
        {
            if (inputManagerRef != nullptr)
            {
                if (inputManagerRef->keyboard != nullptr)
                {
                    inputManagerRef->keyboard->registerKeyDown((uint32_t)wParam);
                }
            }
        }

        break;
    }
    case WM_KEYUP:
    {
        bool keyWasDown = ((lParam & (1 << 30)) != 0);
        bool keyWasUp = ((lParam & (1 << 31)) == 0);
        if (keyWasUp && !keyWasDown)
        {
            if (inputManagerRef != nullptr)
            {
                if (inputManagerRef->keyboard != nullptr)
                {
                    inputManagerRef->keyboard->registerKeyUp((uint32_t)wParam);
                }
            }
        }
        break;
    }
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}