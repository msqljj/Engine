#pragma once
#include <iostream>
#include <Windows.h>
#define MOUSE_LBUTTONDOWN                  0x201
#define MOUSE_LBUTTONUP                    0x202
#define MOUSE_LBUTTONDBLCLK                0x203
#define MOUSE_RBUTTONDOWN                  0x204
#define MOUSE_RBUTTONUP                    0x205
#define MOUSE_RBUTTONDBLCLK                0x206
#define MOUSE_MBUTTONDOWN                  0x207
#define MOUSE_MBUTTONUP                    0x208
#define MOUSE_MBUTTONDBLCLK                0x209
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))



#ifndef _MOUSE_H_
#define _MOUSE_H_
namespace Demo
{
	class Mouse
	{
	public:
		Mouse();
		~Mouse();
		bool exitFlag = false;
		void setHandleResize(bool handleResizeFlag);
		//Callback-based-only-works-inside-window-------------------------
		void setKeyCallback(void callback(uint32_t, uint32_t));
		void setMoveCallback(void callback(int32_t, int32_t));
		void setScrollCallback(void callback(int32_t));
		void registerScroll(int32_t delta);
		void registerMovement(int32_t xpos, int32_t ypos);
		void registerKey(uint32_t uMsg, uint32_t wParam);
		void setScrollStep(uint32_t stepsize);
		int getDeltaX();
		int getDeltaY();
		int getMouseX();
		int getMouseY();
		int getDeltaStepX();
		int getDeltaStepY();
		bool isLButtonDown();
		bool isMButtonDown();
		bool isRButtonDown();
		//----------------------------------------------------------------

		//-----------------------------------------------------------------
	private:
		//Function pointer to a callback func defined in application_name.cpp
		void (*keyCallback)(uint32_t, uint32_t) = 0;
		void (*moveCallback)(int32_t, int32_t) = 0;
		void (*scrollCallback)(int32_t) = 0;
		bool moveCallbackEnabled = false;
		bool keyCallbackEnabled = false;
		bool scrollCallbackEnabled = false;
		bool deltaCaptureEnabled = false;
		bool scrollStepSet = false;
		uint32_t scrollStep = 0;
		uint32_t prevmouse_x = 0;
		uint32_t prevmouse_y = 0;
		uint32_t deltastep_x = 0;
		uint32_t deltastep_y = 0;
		uint32_t mouse_x = 0;
		uint32_t mouse_y = 0;
		uint32_t delta_x = 0;
		uint32_t delta_y = 0;
		bool l_button_down = false;
		bool m_button_down = false;
		bool r_button_down = false;
	};

}

#endif