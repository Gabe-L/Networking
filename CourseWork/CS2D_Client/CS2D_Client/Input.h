#pragma once
class Input
{
private:
	struct Mouse
	{
		int x, y;
		bool left = false, right = false;
	};

public:
	void setKeyDown(int key);
	void setKeyUp(int key);
	bool isKeyDown(int key);

	void setLmbDown();
	void setLmbUp();
	bool isLmbDown();

	void setRmbDown();
	void setRmbUp();
	bool isRmbDown();

	void setMouseX(int lx);
	void setMouseY(int ly);
	void setMousePosition(int lx, int ly);
	int getMouseX();
	int getMouseY();

private:
	bool keys[256]{ false };
	Mouse mouse;
};