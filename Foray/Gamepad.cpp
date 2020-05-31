#include "Gamepad.h"

Gamepad::Gamepad(const int controllerNumber)
{
	this->controllerNumber = controllerNumber;
}

XINPUT_STATE Gamepad::GetState()
{
	ZeroMemory(&controllerState, sizeof(XINPUT_STATE));

	XInputGetState(controllerNumber, &controllerState);

	return controllerState;
}

bool Gamepad::IsConnected()
{
	ZeroMemory(&controllerState, sizeof(XINPUT_STATE));

	const auto result = XInputGetState(controllerNumber, &controllerState);

	if (result == ERROR_SUCCESS)
		return true;
	else
		return false;
}

void Gamepad::Vibrate(const int leftVal, const int rightVal)
{
	XINPUT_VIBRATION vibration;

	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));

	vibration.wLeftMotorSpeed = leftVal;
	vibration.wRightMotorSpeed = rightVal;

	XInputSetState(controllerNumber, &vibration);
}