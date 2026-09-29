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
    //
    // While collecting the dots, this also remembers the enemy closest to the
    // center of the screen for the Left-Alt aim-assist prototype.
    bool CollectEnemyDots(
        uintptr_t actorManager,
        int clientWidth,
        int clientHeight,
        std::vector<Vec2>& outDots);

    // This returns the closest currently valid aim target discovered during
    // the most recent CollectEnemyDots() call.
    //
    // The function returns false when no enemy is inside the configured aim FOV.
    bool GetClosestAimTarget(Vec2& outTarget) const;

private:
    // This transforms a world position into the local camera coordinate system.
    static Vec3 TransformPoint(
        const UnityMatrix4x4& matrix,
        const Vec3& worldPosition);

    // This projects a world position into Ravenfield client-area pixel coordinates.
    static bool WorldToScreen(
        const Vec3& worldPosition,
        const UnityMatrix4x4& worldToLocal,
        float verticalFov,
        int clientWidth,
        int clientHeight,
        Vec2& screenPosition,
        Vec3* cameraPosition = nullptr);

    // This is the Ravenfield process handle created by main.cpp.
    HANDLE hProcess_ = nullptr;

    // This stores the closest screen-space target found during the most recent ESP scan.
    Vec2 closestAimTarget_{};

    // This records whether closestAimTarget_ currently contains a usable enemy target.
    bool hasClosestAimTarget_ = false;

    // This throttles optional ESP diagnostic logging so the console is not spammed.
    ULONGLONG lastDebugPrint_ = 0;
};