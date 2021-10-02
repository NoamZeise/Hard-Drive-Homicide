#pragma once
#ifndef SIMPLE_INPUT_H
#define SIMPLE_INPUT_H

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>

#include <map>
#include <array>

enum SimpleButton
{
	Up,
	Down,
	Left,
	Right,
	A,
	B,
	X,
	Y,
	Select,
	Start
};

class SimpleInput
{
public:
	SimpleInput()
	{
		for (uint32_t i = SimpleButton::Up; i <= SimpleButton::Start; i++)
			pressed.insert(std::pair<SimpleButton, bool>(static_cast<SimpleButton>(i), false));
	}
	void Update(bool* keys)
	{
		for (uint32_t i = SimpleButton::Up; i <= SimpleButton::Start; i++)
			pressed[static_cast<SimpleButton>(i)] = false;

		test(keys[GLFW_KEY_W],		SimpleButton::Up);
		test(keys[GLFW_KEY_A],		SimpleButton::Left);
		test(keys[GLFW_KEY_S],		SimpleButton::Down);
		test(keys[GLFW_KEY_D],		SimpleButton::Right);
		test(keys[GLFW_KEY_SPACE],	SimpleButton::A);
		test(keys[GLFW_KEY_COMMA], SimpleButton::B);
		test(keys[GLFW_KEY_PERIOD], SimpleButton::X);
		test(keys[GLFW_KEY_PERIOD], SimpleButton::Y);
		test(keys[GLFW_KEY_ENTER],  SimpleButton::Select);
		test(keys[GLFW_KEY_ESCAPE], SimpleButton::Start);
	}
	void Reset(bool val)
	{
		for (uint32_t i = SimpleButton::Up; i <= SimpleButton::Start; i++)
			pressed[static_cast<SimpleButton>(i)] = val;
	}

	bool Up() { return pressed[SimpleButton::Up]; }
	bool Left() { return pressed[SimpleButton::Left]; }
	bool Right() { return pressed[SimpleButton::Right]; }
	bool Down() { return pressed[SimpleButton::Down]; }
	bool B() { return pressed[SimpleButton::B]; }
	bool A() { return pressed[SimpleButton::A]; }
	bool X() { return pressed[SimpleButton::X]; }
	bool Y() { return pressed[SimpleButton::Y]; }
	bool Select() { return pressed[SimpleButton::Select]; }
	bool Start() { return pressed[SimpleButton::Start]; }
private:
	std::map<SimpleButton, bool> pressed;

	void test(bool state, SimpleButton btn)
	{
		pressed[btn] = state;
	}
};

struct Btn
{
	SimpleInput press;
	SimpleInput prev;
	bool Up() { return press.Up() && !prev.Up(); }
	bool Left() { return press.Left() && !prev.Left(); }
	bool Right() { return press.Right() && !prev.Right(); }
	bool Down() { return press.Down() && !prev.Down(); }
	bool B() { return press.B() && !prev.B(); }
	bool A() { return press.A() && !prev.A(); }
	bool X() { return press.X() && !prev.X(); }
	bool Y() { return press.Y() && !prev.Y(); }
	bool Select() { return press.Select() && !prev.Select(); }
	bool Start() { return press.Start() && !prev.Start(); }
};










#endif