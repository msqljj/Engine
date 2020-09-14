#include "InputManager/Mouse/mouse.h"

Demo::Mouse::Mouse()
{
}

Demo::Mouse::~Mouse()
{
	exitFlag = true;
	keyCallback = nullptr;
	scrollCallback = nullptr;
	moveCallback = nullptr;
	scrollCallbackEnabled = false;
	keyCallbackEnabled = false;
	moveCallbackEnabled = false;
	std::cout << "Demo::Mouse::Destructor()" << std::endl;
}

void Demo::Mouse::setKeyCallback(void callback(uint32_t, uint32_t))
{
	if (callback != 0)
	{
		keyCallbackEnabled = true;
		keyCallback = callback;
	}
	else
	{
		std::cout << "Demo::Mouse: Error registering callback function!" << std::endl;
		keyCallbackEnabled = false;
	}
}

void Demo::Mouse::setMoveCallback(void callback(int32_t, int32_t))
{
	if (callback != 0)
	{
		moveCallbackEnabled = true;
		moveCallback = callback;
	}
	else
	{
		std::cout << "Demo::Mouse: Error registering callback function!" << std::endl;
		moveCallbackEnabled = false;
	}
}

void Demo::Mouse::setScrollCallback(void callback(int32_t))
{
	if (callback != 0)
	{
		scrollCallbackEnabled = true;
		scrollCallback = callback;
	}
	else
	{
		std::cout << "Demo::Mouse: Error registering callback function!" << std::endl;
		scrollCallbackEnabled = false;
	}
}

void Demo::Mouse::registerScroll(int32_t delta)
{
	if (this)
	{
		if (scrollCallbackEnabled)
		{
			if (scrollStepSet == false)
			{
				scrollCallback(delta);
			}
			else
			{
				if (delta < 0)
				{
					int32_t temp = scrollStep * -1;
					scrollCallback(temp);
				}
				else if (delta > 0)
				{
					scrollCallback(scrollStep);
				}
			}
		}
	}

}

void Demo::Mouse::registerMovement(int32_t xpos, int32_t ypos)
{
	if (this)
	{
		mouse_x = xpos;
		mouse_y = ypos;
		deltastep_x = mouse_x - prevmouse_x;
		deltastep_y = mouse_y - prevmouse_y;
		prevmouse_x = mouse_x;
		prevmouse_y = mouse_y;
		if (moveCallbackEnabled)
		{
			moveCallback(xpos, ypos);
		}
	}
}

void Demo::Mouse::registerKey(uint32_t uMsg, uint32_t wParam)
{
	if (this != 0)
	{
		if (uMsg == MOUSE_LBUTTONDOWN)
		{
			l_button_down = true;
			deltaCaptureEnabled = true;
			delta_x = mouse_x;
			delta_y = mouse_y;
		}
		if (uMsg == MOUSE_LBUTTONUP)
		{
			l_button_down = false;
			deltaCaptureEnabled = false;
			delta_x = 0;
			delta_y = 0;
		}
		if (uMsg == MOUSE_MBUTTONDOWN)
		{

			m_button_down = true;
			deltaCaptureEnabled = true;
			delta_x = mouse_x;
			delta_y = mouse_y;
		}
		if (uMsg == MOUSE_MBUTTONUP)
		{
			m_button_down = false;
			deltaCaptureEnabled = false;
			delta_x = 0;
			delta_y = 0;
		}
		if (uMsg == MOUSE_RBUTTONDOWN)
		{
			r_button_down = true;
			deltaCaptureEnabled = true;
			delta_x = mouse_x;
			delta_y = mouse_y;
		}
		if (uMsg == MOUSE_RBUTTONUP)
		{
			r_button_down = false;
			deltaCaptureEnabled = false;
			delta_x = 0;
			delta_y = 0;
		}

		if (keyCallbackEnabled)
		{
			if(keyCallback != nullptr)
			keyCallback(uMsg, wParam);
		}
	}

}

void Demo::Mouse::setScrollStep(uint32_t stepsize)
{
	scrollStepSet = true;
	scrollStep = stepsize;
}

int Demo::Mouse::getDeltaX()
{
	if (delta_x == 0)
	{
		return delta_x;
	}
	else
	{
		return mouse_x - delta_x;
	}
}

int Demo::Mouse::getDeltaY()
{
	if (delta_y == 0)
	{
		return delta_y;
	}
	else
	{
		return mouse_y - delta_y;
	}
}

int Demo::Mouse::getMouseX()
{
	return mouse_x;
}

int Demo::Mouse::getMouseY()
{
	return mouse_y;
}

int Demo::Mouse::getDeltaStepX()
{
	return deltastep_x;
}

int Demo::Mouse::getDeltaStepY()
{
	return deltastep_y;
}

bool Demo::Mouse::isLButtonDown()
{
	return l_button_down;
}

bool Demo::Mouse::isMButtonDown()
{
	return m_button_down;
}

bool Demo::Mouse::isRButtonDown()
{
	return r_button_down;
}

