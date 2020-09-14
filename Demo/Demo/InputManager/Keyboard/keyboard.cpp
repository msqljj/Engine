#include "InputManager/Keyboard/keyboard.h"

Demo::Keyboard::Keyboard()
{

}

Demo::Keyboard::~Keyboard()
{
	callbackEnabled = false;
	std::cout << "Demo::Keyboard::Destructor()" << std::endl;
}

void Demo::Keyboard::setCallback(void callback(uint32_t, uint32_t))
{
	if (callback != 0)
	{
		callbackEnabled = true;
		keyCallback = callback;
	}
	else
	{
		std::cout << "Demo::Keyboard: Error registering callback function!!" << std::endl;
		callbackEnabled = false;
	}

}


void Demo::Keyboard::registerKeyUp(uint32_t scancode)
{
	if (this)
	{
		if (callbackEnabled)
		{
			keyCallback(KEY_ACTION_UP, scancode);
		}
	}

}

void Demo::Keyboard::registerKeyDown(uint32_t scancode)
{
	if (this)
	{
		if (callbackEnabled)
		{
			keyCallback(KEY_ACTION_DOWN, scancode);
		}
	}

}



