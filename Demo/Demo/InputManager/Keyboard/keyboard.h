#pragma once
#include <iostream>
#define KEY_ACTION_DOWN 0x100
#define KEY_ACTION_UP 0x101
#define MOUSE_LBUTTON 0x01
#define MOUSE_RBUTTON 0x02
#define MOUSE_MBUTTON 0x04
#define MOUSE_X1BUTTON 0x05
#define MOUSE_X2BUTTON 0x06
#define KEY_ESCAPE VK_ESCAPE 
#define KEY_LEFT 0x25
#define KEY_UP 0x26
#define KEY_RIGHT 0x27
#define KEY_DOWN 0x28
#define KEY_LWIN 0x5B
#define KEY_RWIN 0x5C
#define KEY_DELETE 0x2E
#define KEY_SPACE 0x20
#define KEY_KPADD 0x6B
#define KEY_KPSUB 0x6D
#define KEY_LSHIFT 0xA0
#define KEY_RSHIFT 0xA1
#define KEY_LCONTROL 0xA2
#define KEY_RCONTROL 0xA3
#define KEY_F1 VK_F1
#define KEY_F2 VK_F2
#define KEY_F3 VK_F3
#define KEY_F4 VK_F4
#define KEY_F5 VK_F5
#define KEY_F6 VK_F6
#define KEY_F7 VK_F7
#define KEY_F8 VK_F8
#define KEY_F9 VK_F9
#define KEY_F10 VK_F10
#define KEY_F11 VK_F11
#define KEY_F12 VK_F12
#define KEY_A 0x41
#define KEY_B 0x42
#define KEY_C 0x43
#define KEY_D 0x44
#define KEY_E 0x45
#define KEY_F 0x46
#define KEY_G 0x47
#define KEY_H 0x48
#define KEY_I 0x49
#define KEY_J 0x4A
#define KEY_K 0x4B
#define KEY_L 0x4C
#define KEY_M 0x4D
#define KEY_N 0x4E
#define KEY_O 0x4F
#define KEY_P 0x50
#define KEY_Q 0x51
#define KEY_R 0x52
#define KEY_S 0x53
#define KEY_T 0x54
#define KEY_U 0x55
#define KEY_V 0x56
#define KEY_W 0x57
#define KEY_X 0x58
#define KEY_Y 0x59
#define KEY_Z 0x5A
#define KEY_0 0x30
#define KEY_1 0x31
#define KEY_2 0x32
#define KEY_3 0x33
#define KEY_4 0x34
#define KEY_5 0x35
#define KEY_6 0x36
#define KEY_7 0x37
#define KEY_8 0x38
#define KEY_9 0x39
#define KEY_NUMPAD0 0x60
#define KEY_NUMPAD1 0x61
#define KEY_NUMPAD2 0x62
#define KEY_NUMPAD3 0x63
#define KEY_NUMPAD4 0x64
#define KEY_NUMPAD5 0x65
#define KEY_NUMPAD6 0x66
#define KEY_NUMPAD7 0x67
#define KEY_NUMPAD8 0x68
#define KEY_NUMPAD9 0x69
#define KEY_MULTIPLY 0x6A
#define KEY_ADD 0x6B
#define KEY_SEPARATOR 0x6C
#define KEY_SUBTRACT 0x6D
#define KEY_DECIMAL 0x6E
#define KEY_DIVIDE 0x6F


#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_
namespace Demo
{
	class Keyboard
	{
	public:
		Keyboard();
		~Keyboard();
		void setCallback(void callback(uint32_t, uint32_t));
		void registerKeyUp(uint32_t scancode);
		void registerKeyDown(uint32_t scancode);
	private:
		//Function pointer to a callback func defined in application_name.cpp
		void (*keyCallback)(uint32_t, uint32_t) = 0;
		bool callbackEnabled = false;

	};
}

#endif