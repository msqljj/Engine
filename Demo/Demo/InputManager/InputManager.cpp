#include "InputManager/InputManager.h"

Demo::InputManager::InputManager()
{
	mouse = new Mouse;
	keyboard = new Keyboard;
}

Demo::InputManager::~InputManager()
{
	delete mouse;
	mouse = 0;

	delete keyboard;
	keyboard = 0;
}

void Demo::InputManager::setKeyCallback(void callback(uint32_t, uint32_t))
{
	keyboard->setCallback(callback);
}

void Demo::InputManager::setMouseKeyCallback(void callback(uint32_t, uint32_t))
{
	mouse->setKeyCallback(callback);
}

void Demo::InputManager::setMouseMoveCallback(void callback(int32_t, int32_t))
{
	mouse->setMoveCallback(callback);
}

void Demo::InputManager::setMouseScrollCallback(void callback(int32_t))
{
	mouse->setScrollCallback(callback);
}
