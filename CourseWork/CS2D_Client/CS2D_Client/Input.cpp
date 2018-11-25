#include "pch.h"

#include "Input.h"

void Input::setKeyDown(int key)
{
	keys[key] = true;
}

void Input::setKeyUp(int key)
{
	keys[key] = false;
}

bool Input::isKeyDown(int key)
{
	return keys[key];
}

void Input::setMouseX(int lx)
{
	mouse.x = lx;
}

void Input::setLmbDown()
{
	mouse.left = true;
}

void Input::setLmbUp()
{
	mouse.left = false;
}

bool Input::isLmbDown()
{
	return mouse.left;
}

void Input::setRmbDown()
{
	mouse.right = true;
}

void Input::setRmbUp()
{
	mouse.right = false;
}

bool Input::isRmbDown()
{
	return mouse.right;
}

void Input::setMouseY(int ly)
{
	mouse.y = ly;
}

void Input::setMousePosition(int lx, int ly)
{
	mouse.x = lx;
	mouse.y = ly;
}

int Input::getMouseX()
{
	return mouse.x;
}

int Input::getMouseY()
{
	return mouse.y;
}