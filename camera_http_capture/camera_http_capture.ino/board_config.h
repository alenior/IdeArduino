#pragma once

namespace BoardPins {

constexpr int CAM_PWDN  = -1;
constexpr int CAM_RESET = -1;

constexpr int CAM_SIOD = 4;
constexpr int CAM_SIOC = 5;

constexpr int CAM_VSYNC = 6;
constexpr int CAM_HREF  = 7;
constexpr int CAM_XCLK  = 15;
constexpr int CAM_PCLK  = 13;

constexpr int CAM_D0 = 11;  // Y2
constexpr int CAM_D1 = 9;   // Y3
constexpr int CAM_D2 = 8;   // Y4
constexpr int CAM_D3 = 10;  // Y5
constexpr int CAM_D4 = 12;  // Y6
constexpr int CAM_D5 = 18;  // Y7
constexpr int CAM_D6 = 17;  // Y8
constexpr int CAM_D7 = 16;  // Y9

constexpr int STATUS_LED = 2;
constexpr int WS2812_LED = 48;

}  // namespace BoardPins