#pragma once
#include "InputManager/Mouse/mouse.h"
#include "InputManager/Keyboard/keyboard.h"
#ifndef _INPUTMANAGER_H_
#define _INPUTMANAGER_H_
namespace Demo
{
	class InputManager
	{
	public:
		InputManager();
		~InputManager();
		void setKeyCallback(void callback(uint32_t, uint32_t));
		void setMouseKeyCallback(void callback(uint32_t, uint32_t));
		void setMouseMoveCallback(void callback(int32_t, int32_t));
		void setMouseScrollCallback(void callback(int32_t));
		Keyboard* keyboard = nullptr;
		Mouse* mouse = nullptr;
	};
}
#endif


