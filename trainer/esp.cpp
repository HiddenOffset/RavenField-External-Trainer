#include "esp.h"

#include <cmath>
#include <iostream>

#include "addresses.h"
#include "mem.h"

namespace
{
    // This can be enabled temporarily if we need to inspect ESP values in the console.
    constexpr bool kEspDebug = false;

    // Positive camera-space Z is treated as forward.
    constexpr float kCameraForwardSign = 1.0f;

    // This prevents projection extremely close to the camera plane.
    constexpr float kMinimumDepth = 0.01f;

    // This value is used to convert degrees into radians.
    constexpr float kPi = 3.14159265358979323846f;

    // This is the fixed Y offset that we already confirmed visually lands on a standing enemy's head.
    constexpr float kDotVerticalOffset = 1.530000091f;
}

EspSystem::EspSystem(HANDLE hProcess)
    : hProcess_(hProcess)
{
    // The Ravenfield process handle is supplied by main.cpp.
}

Vec3 EspSystem::TransformPoint(
    const UnityMatrix4x4& matrix,
    const Vec3& worldPosition)
{
    // This stores the position after converting it from world space into camera space.
    Vec3 cameraPosition{};

    // This calculates camera-space X.
    cameraPosition.x =
        matrix.m00 * worldPosition.x +
        matrix.m01 * worldPosition.y +
        matrix.m02 * worldPosition.z +
        matrix.m03;

    // This calculates camera-space Y.
    cameraPosition.y =
        matrix.m10 * worldPosition.x +
        matrix.m11 * worldPosition.y +
        matrix.m12 * worldPosition.z +
        matrix.m13;

    // This calculates camera-space Z.
    cameraPosition.z =
        matrix.m20 * worldPosition.x +
        matrix.m21 * worldPosition.y +
        matrix.m22 * worldPosition.z +
        matrix.m23;

    // This returns the final camera-relative position.
    return cameraPosition;
}

bool EspSystem::WorldToScreen(
    const Vec3& worldPosition,
    const UnityMatrix4x4& worldToLocal,
    float verticalFov,
    int clientWidth,
    int clientHeight,
    Vec2& screenPosition,
    Vec3* cameraPosition)
{
    // Invalid client dimensions cannot be used for projection.
    if (clientWidth <= 0 || clientHeight <= 0)
    {
        return false;
    }

    // This rejects invalid FOV values.
    if (!std::isfinite(verticalFov) ||
        verticalFov <= 1.0f ||
        verticalFov >= 179.0f)
    {
        return false;
    }

    // This transforms the enemy position from world space into camera space.
    const Vec3 camera =
        TransformPoint(
            worldToLocal,
            worldPosition);

    // This optionally returns the camera-space position for debugging.
    if (cameraPosition != nullptr)
    {
        *cameraPosition = camera;
    }

    // This converts camera-space Z into usable forward depth.
    const float depth =
        camera.z * kCameraForwardSign;

    // This rejects enemies behind the camera or extremely close to the camera plane.
    if (!std::isfinite(depth) ||
        depth <= kMinimumDepth)
    {
        return false;
    }

    // This converts the current vertical FOV from degrees into radians.
    const float fovRadians =
        verticalFov *
        (kPi / 180.0f);

    // This calculates the perspective projection scale.
    const float tanHalfFov =
        std::tan(
            fovRadians * 0.5f);

    // This rejects invalid projection values.
    if (!std::isfinite(tanHalfFov) ||
        std::fabs(tanHalfFov) < 0.0001f)
    {
        return false;
    }

    // This calculates Ravenfield's current client aspect ratio.
    const float aspect =
        static_cast<float>(clientWidth) /
        static_cast<float>(clientHeight);

    // This projects camera-space X into normalized screen coordinates.
    const float ndcX =
        camera.x /
        (depth * tanHalfFov * aspect);

    // This projects camera-space Y into normalized screen coordinates.
    const float ndcY =
        camera.y /
        (depth * tanHalfFov);

    // This rejects invalid projection results.
    if (!std::isfinite(ndcX) ||
        !std::isfinite(ndcY))
    {
        return false;
    }

    // This skips enemies whose projected point is outside the visible screen.
    if (ndcX < -1.0f ||
        ndcX > 1.0f ||
        ndcY < -1.0f ||
        ndcY > 1.0f)
    {
        return false;
    }

    // This converts normalized X into a client-area pixel coordinate.
    screenPosition.x =
        (ndcX + 1.0f) *
        0.5f *
        static_cast<float>(clientWidth);

    // This converts normalized Y into a client-area pixel coordinate.
    screenPosition.y =
        (1.0f - ndcY) *
        0.5f *
        static_cast<float>(clientHeight);

    // The enemy position was successfully projected.
    return true;
}

bool EspSystem::CollectEnemyDots(
    uintptr_t actorManager,
    int clientWidth,
    int clientHeight,
    std::vector<Vec2>& outDots)
{
    // This clears last frame's dots.
    outDots.clear();

    // This prevents processing if ActorManager is unavailable.
    if (actorManager == 0)
    {
        return false;
    }

    // This reads the local player's Actor pointer.
    uintptr_t localActor = 0;

    if (!mem::Read(
        hProcess_,
        actorManager + GameOffsets::ACTOR_MANAGER_PLAYER,
        localActor) ||
        localActor == 0)
    {
        return false;
    }

    // This reads the local player's current team.
    int localTeam = 0;

    if (!mem::Read(
        hProcess_,
        localActor + GameOffsets::ACTOR_TEAM,
        localTeam))
    {
        return false;
    }

    // This reads the local player's controller pointer.
    uintptr_t controller = 0;

    if (!mem::Read(
        hProcess_,
        localActor + GameOffsets::ACTOR_CONTROLLER,
        controller) ||
        controller == 0)
    {
        return false;
    }

    // This reads the camera WorldToLocal matrix once for the entire frame.
    UnityMatrix4x4 worldToLocal{};

    if (!mem::Read(
        hProcess_,
        controller + GameOffsets::CONTROLLER_WORLD_TO_LOCAL,
        worldToLocal))
    {
        return false;
    }

    // This reads fpParent, which contains the FOV information.
    uintptr_t fpParent = 0;

    if (!mem::Read(
        hProcess_,
        controller + GameOffsets::CONTROLLER_FP_PARENT,
        fpParent) ||
        fpParent == 0)
    {
        return false;
    }

    // This reads the current ADS interpolation ratio.
    float fovRatio = 0.0f;

    if (!mem::Read(
        hProcess_,
        fpParent + GameOffsets::FP_PARENT_FOV_RATIO,
        fovRatio))
    {
        return false;
    }

    // This reads the normal hip-fire FOV.
    float normalFov = 0.0f;

    if (!mem::Read(
        hProcess_,
        fpParent + GameOffsets::FP_PARENT_NORMAL_FOV,
        normalFov))
    {
        return false;
    }

    // This reads the current weapon's fully zoomed ADS FOV.
    float zoomFov = 0.0f;

    if (!mem::Read(
        hProcess_,
        fpParent + GameOffsets::FP_PARENT_ZOOM_FOV,
        zoomFov))
    {
        return false;
    }

    // This validates the basic FOV values.
    if (!std::isfinite(fovRatio) ||
        !std::isfinite(normalFov) ||
        normalFov <= 1.0f ||
        normalFov >= 179.0f)
    {
        return false;
    }

    // This clamps the ADS ratio to its expected range.
    if (fovRatio < 0.0f)
    {
        fovRatio = 0.0f;
    }
    else if (fovRatio > 1.0f)
    {
        fovRatio = 1.0f;
    }

    // Hip-fire uses the normal FOV.
    float activeFov =
        normalFov;

    // ADS smoothly transitions from normalFov toward zoomFov.
    if (fovRatio > 0.0001f)
    {
        // This validates the weapon's zoom FOV.
        if (!std::isfinite(zoomFov) ||
            zoomFov <= 1.0f ||
            zoomFov >= 179.0f)
        {
            return false;
        }

        // This calculates the actual FOV currently displayed during ADS.
        activeFov =
            normalFov +
            (zoomFov - normalFov) *
            fovRatio;
    }

    // This validates the final FOV used for projection.
    if (!std::isfinite(activeFov) ||
        activeFov <= 1.0f ||
        activeFov >= 179.0f)
    {
        return false;
    }

    // This reads ActorManager.actors.
    uintptr_t actorList = 0;

    if (!mem::Read(
        hProcess_,
        actorManager + GameOffsets::ACTOR_MANAGER_ACTORS,
        actorList) ||
        actorList == 0)
    {
        return false;
    }

    // This reads the List<Actor> backing array.
    uintptr_t items = 0;

    if (!mem::Read(
        hProcess_,
        actorList + GameOffsets::LIST_ITEMS,
        items) ||
        items == 0)
    {
        return false;
    }

    // This reads the actual number of Actors currently stored in the list.
    int actorCount = 0;

    if (!mem::Read(
        hProcess_,
        actorList + GameOffsets::LIST_SIZE,
        actorCount))
    {
        return false;
    }

    // This prevents obviously corrupt Actor counts from being used.
    if (actorCount <= 0 ||
        actorCount > 512)
    {
        return false;
    }

    // This gets the current time for optional rate-limited debugging.
    const ULONGLONG now =
        GetTickCount64();

    // Debug information prints roughly once per second.
    const bool shouldPrintDebug =
        kEspDebug &&
        (now - lastDebugPrint_ >= 1000);

    // This keeps debug mode from printing every enemy at once.
    bool printedEnemyDebug = false;

    // This prints shared frame information when debug mode is enabled.
    if (shouldPrintDebug)
    {
        std::cout
            << "[ESP DEBUG] ActorManager=0x"
            << std::hex
            << actorManager
            << " LocalActor=0x"
            << localActor
            << std::dec
            << " Count="
            << actorCount
            << " LocalTeam="
            << localTeam
            << " FovRatio="
            << fovRatio
            << " NormalFov="
            << normalFov
            << " ZoomFov="
            << zoomFov
            << " ActiveFov="
            << activeFov
            << std::endl;
    }

    // This loops over every Actor in the managed list.
    for (int i = 0; i < actorCount; ++i)
    {
        // This calculates the address containing the current Actor pointer.
        const uintptr_t actorPointerAddress =
            items +
            GameOffsets::MANAGED_ARRAY_FIRST_ELEMENT +
            static_cast<uintptr_t>(i) *
            sizeof(uintptr_t);

        // This reads the Actor pointer.
        uintptr_t actor = 0;

        if (!mem::Read(
            hProcess_,
            actorPointerAddress,
            actor) ||
            actor == 0)
        {
            continue;
        }

        // This prevents the ESP from drawing the local player.
        if (actor == localActor)
        {
            continue;
        }

        // This reads the Actor's team.
        int actorTeam = 0;

        if (!mem::Read(
            hProcess_,
            actor + GameOffsets::ACTOR_TEAM,
            actorTeam))
        {
            continue;
        }

        // This removes teammates from the ESP.
        if (actorTeam == localTeam)
        {
            continue;
        }

        // This reads the Actor's health.
        float health = 0.0f;

        if (!mem::Read(
            hProcess_,
            actor + GameOffsets::ACTOR_HEALTH,
            health))
        {
            continue;
        }

        // This removes dead Actors.
        if (!std::isfinite(health) ||
            health <= 0.0f)
        {
            continue;
        }

        // This reads the enemy's cached root position.
        Vec3 worldPosition{};

        if (!mem::Read(
            hProcess_,
            actor + GameOffsets::ACTOR_CACHED_POSITION,
            worldPosition))
        {
            continue;
        }

        // This validates the enemy's world coordinates.
        if (!std::isfinite(worldPosition.x) ||
            !std::isfinite(worldPosition.y) ||
            !std::isfinite(worldPosition.z))
        {
            continue;
        }

        // This copies the enemy's root position.
        Vec3 espTargetPosition =
            worldPosition;

        // This raises the dot by the fixed standing head-height value that already worked.
        espTargetPosition.y +=
            kDotVerticalOffset;

        // This stores the final projected screen position.
        Vec2 screenPosition{};

        // This is only populated when debugging is enabled.
        Vec3 cameraPosition{};

        // This performs the world-to-screen conversion.
        const bool onScreen =
            WorldToScreen(
                espTargetPosition,
                worldToLocal,
                activeFov,
                clientWidth,
                clientHeight,
                screenPosition,
                shouldPrintDebug
                ? &cameraPosition
                : nullptr);

        // This optionally prints one enemy's projection information.
        if (shouldPrintDebug &&
            !printedEnemyDebug)
        {
            std::cout
                << "[ESP DEBUG] Enemy=0x"
                << std::hex
                << actor
                << std::dec
                << " Root=("
                << worldPosition.x
                << ", "
                << worldPosition.y
                << ", "
                << worldPosition.z
                << ")"
                << " Target=("
                << espTargetPosition.x
                << ", "
                << espTargetPosition.y
                << ", "
                << espTargetPosition.z
                << ")"
                << " Camera=("
                << cameraPosition.x
                << ", "
                << cameraPosition.y
                << ", "
                << cameraPosition.z
                << ")"
                << " Screen=("
                << screenPosition.x
                << ", "
                << screenPosition.y
                << ")"
                << " OnScreen="
                << (onScreen ? "true" : "false")
                << std::endl;

            printedEnemyDebug = true;
        }

        // This skips enemies that could not be projected onto the visible screen.
        if (!onScreen)
        {
            continue;
        }

        // This sends the final dot position to the overlay.
        outDots.push_back(
            screenPosition);
    }

    // This updates the debug timer.
    if (shouldPrintDebug)
    {
        lastDebugPrint_ = now;
    }

    // The Actor list was processed successfully.
    return true;
}