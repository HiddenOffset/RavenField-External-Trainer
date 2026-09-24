#pragma once

#include <Windows.h>
#include <cstdint>
#include <vector>
#include "math_types.h"

// EspSystem reads Ravenfield's Actor list and converts enemy world positions to screen pixels.
class EspSystem
{
public:
    // This stores the already-open Ravenfield process handle used for read-only ESP memory access.
    explicit EspSystem(HANDLE hProcess);

    // This collects one screen-space dot for every living enemy Actor that projects on screen.
    bool CollectEnemyDots(uintptr_t actorManager, int clientWidth, int clientHeight, std::vector<Vec2>& outDots);

private:
    // This transforms a world position into the local camera coordinate system.
    static Vec3 TransformPoint(const UnityMatrix4x4& matrix, const Vec3& worldPosition);

    // This projects a world position into Ravenfield client-area pixel coordinates.
    static bool WorldToScreen(const Vec3& worldPosition,
                              const UnityMatrix4x4& worldToLocal,
                              float verticalFov,
                              int clientWidth,
                              int clientHeight,
                              Vec2& screenPosition,
                              Vec3* cameraPosition = nullptr);

    // This is the Ravenfield process handle created by main.cpp.
    HANDLE hProcess_ = nullptr;

    // This throttles optional ESP diagnostic logging so the console is not spammed.
    ULONGLONG lastDebugPrint_ = 0;
};
