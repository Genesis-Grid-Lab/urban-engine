#include "Core/Input.h"
#include "Core/Log.h"

namespace UE {

    bool Input::s_Keys[512] = {};
	bool Input::s_KeysLastFrame[512] = {};
	bool Input::s_MouseButtons[32] = {};

	glm::vec2 Input::s_MousePos = { 0,0 };
	glm::vec2 Input::s_LastMousePos = { 0,0 };
	glm::vec2 Input::s_MouseDelta = { 0,0 };

	std::function<void(bool)>        Input::s_SetCursorVisible = nullptr;
	std::function<void(float, float)> Input::s_SetCursorPos = nullptr;

	bool Input::IsKeyPressed(KeyCode key) {
		return s_Keys[(int)key];
	}

	bool Input::IsKeyJustPressed(KeyCode key) {
		int k = (int)key;
		return s_Keys[k] && !s_KeysLastFrame[k];
	}

	bool Input::IsMouseButtonPressed(MouseCode button) {
		return s_MouseButtons[(int)button];
	}

	glm::vec2 Input::GetMousePosition() {
		return s_MousePos;
	}

	glm::vec2 Input::GetMouseDelta() {
		return s_MouseDelta;
	}

	float Input::GetMouseX() { return s_MousePos.x; }
	float Input::GetMouseY() { return s_MousePos.y; }

	void Input::HideCursor(bool hide) {
		if (s_SetCursorVisible)
			s_SetCursorVisible(!hide);
	}

	void Input::SetCursorPos(float x, float y) {
		if (s_SetCursorPos)
			s_SetCursorPos(x, y);
	}

	void Input::SetCursorPos(const glm::vec2& pos) {
		if (s_SetCursorPos)
			s_SetCursorPos(pos.x, pos.y);
	}

	void Input::Update() {
		memcpy(s_KeysLastFrame, s_Keys, sizeof(s_Keys));
		s_MouseDelta = s_MousePos - s_LastMousePos;
		s_LastMousePos = s_MousePos;
	}

}