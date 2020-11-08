#pragma once

class Gamepad
{
private:
    XINPUT_STATE controllerState;
    int controllerNumber{ 0 };
public:
    Gamepad(const int playerNumber);
    XINPUT_STATE GetState();
    bool IsConnected();
    void Vibrate(const int leftVal, const int rightVal);
};
