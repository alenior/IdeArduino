#pragma once

#include <Arduino.h>
#include "esp_camera.h"
#include "esp_err.h"

namespace CameraService {

struct CaptureStatistics {
    uint32_t totalRequests;
    uint32_t successfulCaptures;
    uint32_t failedCaptures;
    size_t lastFrameSize;
    uint32_t lastCaptureTimeMs;
};

bool begin();
bool isInitialized();

camera_fb_t* acquireFrame();
void releaseFrame(camera_fb_t*& frame);

sensor_t* getSensor();
uint16_t getSensorPid();

const CaptureStatistics& getStatistics();

}  // namespace CameraService