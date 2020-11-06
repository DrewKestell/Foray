#pragma once

constexpr auto CLIENT_WIDTH = 1400.0f;
constexpr auto CLIENT_HEIGHT = 900.0f;
constexpr auto UPDATE_FREQUENCY = 0.00833333333f;
constexpr auto PLAYER_HORIZONTAL_MOVE_SPEED = 2.5f;
constexpr auto PLAYER_MAX_FALL_VELOCITY = 7.0f;
constexpr auto PLAYER_JUMP_VELOCITY = -8.0f;
constexpr auto PLAYER_DOWNWARD_ACCELERATION = 0.2f;

constexpr auto ARIAL_FONT_FAMILY{ L"Arial" };
constexpr auto TREBUCHET_FONT_FAMILY{ L"Trebuchet" };
constexpr auto LOCALE{ L"en-US" };
constexpr auto BULLET_TEXT{ L"⮡" };

constexpr XMFLOAT2 VECTOR_RIGHT{ 1.0f, 0.0f };
constexpr XMFLOAT2 VECTOR_LEFT{ -1.0f, 0.0f };