#include "esp.h"

#include <cmath>
#include <iostream>
#include "addresses.h"
#include "mem.h"

namespace
{
    // Set this to true temporarily when diagnosing matrix, FOV, or list-layout problems.
    constexpr bool kEspDebug = false;

    // A positive value means the initial implementation treats positive camera-space Z as forward.
    // Change this to -1.0f only if runtime logging proves enemies in front have negative camera Z.
    constexpr float kCameraForwardSign = 1.0f;

    // This prevents division by zero when an Actor is extremely close to the camera plane.
    constexpr float kMinimumDepth = 0.01f;

    // This is pi as a float for converting the vertical FOV from degrees to radians.
    constexpr float kPi = 3.14159265358979323846f;
}

EspSystem::EspSystem(HANDLE hProcess)
    : hProcess_(hProcess)
{
    // The constructor only stores the process handle because main.cpp already owns process setup.
}

Vec3 EspSystem::TransformPoint(const UnityMatrix4x4& matrix, const Vec3& worldPosition)
{
    // This multiplies the world-space point by Unity's world-to-local camera matrix.
    Vec3 cameraPosition{};

    // This calculates left/right camera-space position and includes the translation term m03.
    cameraPosition.x = matrix.m00 * worldPosition.x +
                       matrix.m01 * worldPosition.y +
                       matrix.m02 * worldPosition.z +
                       matrix.m03;

    // This calculates up/down camera-space position and includes the translation term m13.
    cameraPosition.y = matrix.m10 * worldPosition.x +
                       matrix.m11 * worldPosition.y +
                       matrix.m12 * worldPosition.z +
                       matrix.m13;

    // This calculates camera-space depth and includes the translation term m23.
    cameraPosition.z = matrix.m20 * worldPosition.x +
                       matrix.m21 * worldPosition.y +
                       matrix.m22 * worldPosition.z +
                       matrix.m23;

    // This returns the calculated camera-relative XYZ values.
    return cameraPosition;
}

bool EspSystem::WorldToScreen(const Vec3& worldPosition,
                              const UnityMatrix4x4& worldToLocal,
                              float verticalFov,
                              int clientWidth,
                              int clientHeight,
                              Vec2& screenPosition,
                              Vec3* cameraPosition)
{
    // Invalid client dimensions cannot be used for perspective projection.
    if (clientWidth <= 0 || clientHeight <= 0)
        return false;

    // Invalid FOV values usually mean the camera object is not ready during a load or transition.
    if (!std::isfinite(verticalFov) || verticalFov <= 1.0f || verticalFov >= 179.0f)
        return false;

    // This transforms the enemy from world coordinates into camera-relative coordinates.
    const Vec3 camera = TransformPoint(worldToLocal, worldPosition);

    // This optionally returns camera-space XYZ for the compile-time debug output.
    if (cameraPosition != nullptr)
        *cameraPosition = camera;

    // This applies the currently assumed forward-axis sign to the camera-space Z value.
    const float depth = camera.z * kCameraForwardSign;

    // This rejects Actors that are behind the camera or too close to the camera plane.
    if (!std::isfinite(depth) || depth <= kMinimumDepth)
        return false;

    // This converts the current vertical FOV from degrees into radians.
    const float fovRadians = verticalFov * (kPi / 180.0f);

    // This is the vertical perspective scale needed by the pinhole-camera projection formula.
    const float tanHalfFov = std::tan(fovRadians * 0.5f);

    // A zero or invalid tangent would make the projection divide by an invalid value.
    if (!std::isfinite(tanHalfFov) || std::fabs(tanHalfFov) < 0.0001f)
        return false;

    // This calculates the live Ravenfield client area's width-to-height ratio.
    const float aspect = static_cast<float>(clientWidth) / static_cast<float>(clientHeight);

    // This projects camera-space X into normalized device coordinates using the vertical FOV and aspect ratio.
    const float ndcX = camera.x / (depth * tanHalfFov * aspect);

    // This projects camera-space Y into normalized device coordinates using the vertical FOV.
    const float ndcY = camera.y / (depth * tanHalfFov);

    // Invalid normalized coordinates indicate unusable camera or position data for this frame.
    if (!std::isfinite(ndcX) || !std::isfinite(ndcY))
        return false;

    // This skips off-screen Actors instead of drawing dots outside the overlay client area.
    if (ndcX < -1.0f || ndcX > 1.0f || ndcY < -1.0f || ndcY > 1.0f)
        return false;

    // This converts normalized X from [-1, 1] into client-area pixel coordinates.
    screenPosition.x = (ndcX + 1.0f) * 0.5f * static_cast<float>(clientWidth);

    // This flips normalized Y because Windows pixel coordinates increase downward from the top edge.
    screenPosition.y = (1.0f - ndcY) * 0.5f * static_cast<float>(clientHeight);

    // The Actor produced a valid on-screen pixel coordinate.
    return true;
}

bool EspSystem::CollectEnemyDots(uintptr_t actorManager,
                                 int clientWidth,
                                 int clientHeight,
                                 std::vector<Vec2>& outDots)
{
    // This removes the previous frame's points before collecting fresh enemy positions.
    outDots.clear();

    // A null ActorManager means Ravenfield is probably loading or not currently inside a match.
    if (actorManager == 0)
        return false;

    // This reads ActorManager.player, which is the local player's Actor reference.
    uintptr_t localActor = 0;
    if (!mem::Read(hProcess_, actorManager + GameOffsets::ACTOR_MANAGER_PLAYER, localActor) || localActor == 0)
        return false;

    // This reads the local player's team so enemy filtering does not assume team zero or team one.
    int localTeam = 0;
    if (!mem::Read(hProcess_, localActor + GameOffsets::ACTOR_TEAM, localTeam))
        return false;

    // This reads the local Actor's controller reference, which owns the active camera data.
    uintptr_t controller = 0;
    if (!mem::Read(hProcess_, localActor + GameOffsets::ACTOR_CONTROLLER, controller) || controller == 0)
        return false;

    // The active world-to-local Matrix4x4 is inline data at Controller + 0x120, not another pointer.
    UnityMatrix4x4 worldToLocal{};
    if (!mem::Read(hProcess_, controller + GameOffsets::CONTROLLER_WORLD_TO_LOCAL, worldToLocal))
        return false;

    // This reads the fpParent reference that owns Ravenfield's hip-fire and ADS FOV state.
    uintptr_t fpParent = 0;
    if (!mem::Read(hProcess_, controller + GameOffsets::CONTROLLER_FP_PARENT, fpParent) || fpParent == 0)
        return false;

    // This reads how far the current camera has transitioned from hip-fire into ADS.
    float fovRatio = 0.0f;
    if (!mem::Read(hProcess_, fpParent + GameOffsets::FP_PARENT_FOV_RATIO, fovRatio))
        return false;

    // This reads the normal hip-fire FOV that Ravenfield uses before aiming down sights.
    float normalFov = 0.0f;
    if (!mem::Read(hProcess_, fpParent + GameOffsets::FP_PARENT_NORMAL_FOV, normalFov))
        return false;

    // This reads the current weapon's fully zoomed ADS FOV, which changes between weapon types.
    float zoomFov = 0.0f;
    if (!mem::Read(hProcess_, fpParent + GameOffsets::FP_PARENT_ZOOM_FOV, zoomFov))
        return false;

    // An invalid ratio or normal FOV usually means the camera object is temporarily unavailable.
    if (!std::isfinite(fovRatio) || !std::isfinite(normalFov) || normalFov <= 1.0f || normalFov >= 179.0f)
        return false;

    // This clamps the ADS transition value so temporary overshoot cannot create a broken projection.
    if (fovRatio < 0.0f)
        fovRatio = 0.0f;
    else if (fovRatio > 1.0f)
        fovRatio = 1.0f;

    // Hip-fire can safely use normalFov even if a weapon does not currently expose a useful zoomFov.
    float activeFov = normalFov;

    // Once ADS begins, the weapon-specific zoom FOV must be valid before interpolation is attempted.
    if (fovRatio > 0.0001f)
    {
        // This rejects an invalid weapon zoom value instead of feeding bad projection data into WorldToScreen.
        if (!std::isfinite(zoomFov) || zoomFov <= 1.0f || zoomFov >= 179.0f)
            return false;

        // This linearly interpolates from hip-fire FOV to the weapon's zoom FOV as ADS progresses.
        activeFov = normalFov + (zoomFov - normalFov) * fovRatio;
    }

    // This final check guarantees WorldToScreen receives a sane current FOV.
    if (!std::isfinite(activeFov) || activeFov <= 1.0f || activeFov >= 179.0f)
        return false;

    // This reads ActorManager.actors, which points to the managed List<Actor> object.
    uintptr_t actorList = 0;
    if (!mem::Read(hProcess_, actorManager + GameOffsets::ACTOR_MANAGER_ACTORS, actorList) || actorList == 0)
        return false;

    // This reads List<Actor>._items, which points to the managed Actor[] backing array.
    uintptr_t items = 0;
    if (!mem::Read(hProcess_, actorList + GameOffsets::LIST_ITEMS, items) || items == 0)
        return false;

    // This reads List<Actor>._size, which is the real number of valid list entries.
    int actorCount = 0;
    if (!mem::Read(hProcess_, actorList + GameOffsets::LIST_SIZE, actorCount))
        return false;

    // This guards against invalid list data during loading, map changes, or bad pointer resolution.
    if (actorCount <= 0 || actorCount > 512)
        return false;

    // Debug output is limited to about once per second when kEspDebug is manually enabled.
    const ULONGLONG now = GetTickCount64();
    const bool shouldPrintDebug = kEspDebug && (now - lastDebugPrint_ >= 1000);

    // This tracks whether the optional debug output already printed one enemy this cycle.
    bool printedEnemyDebug = false;

    // This prints the shared camera/list state once before the Actor loop when debugging is enabled.
    if (shouldPrintDebug)
    {
        std::cout << "[ESP DEBUG] ActorManager=0x" << std::hex << actorManager
                  << " LocalActor=0x" << localActor << std::dec
                  << " Count=" << actorCount
                  << " LocalTeam=" << localTeam
                  << " FovRatio=" << fovRatio
                  << " NormalFov=" << normalFov
                  << " ZoomFov=" << zoomFov
                  << " ActiveFov=" << activeFov << std::endl;
    }

    // This iterates only the valid List<Actor> entries reported by _size.
    for (int i = 0; i < actorCount; ++i)
    {
        // Managed arrays begin their first reference at +0x20 and each x64 reference is sizeof(uintptr_t).
        const uintptr_t actorPointerAddress = items + GameOffsets::MANAGED_ARRAY_FIRST_ELEMENT +
                                              static_cast<uintptr_t>(i) * sizeof(uintptr_t);

        // This reads the Actor reference stored in the current managed-array slot.
        uintptr_t actor = 0;
        if (!mem::Read(hProcess_, actorPointerAddress, actor) || actor == 0)
            continue;

        // This prevents a dot from being drawn for the local player's own Actor.
        if (actor == localActor)
            continue;

        // This reads the current Actor's team value.
        int actorTeam = 0;
        if (!mem::Read(hProcess_, actor + GameOffsets::ACTOR_TEAM, actorTeam))
            continue;

        // This skips teammates and leaves only Actors whose team differs from the local player.
        if (actorTeam == localTeam)
            continue;

        // This reads the Actor's current health so dead Actors can be excluded.
        float health = 0.0f;
        if (!mem::Read(hProcess_, actor + GameOffsets::ACTOR_HEALTH, health))
            continue;

        // This skips dead Actors and also rejects invalid floating-point health values.
        if (!std::isfinite(health) || health <= 0.0f)
            continue;

        // This reads cachedPosition.x/y/z in one RPM call because UnityEngine.Vector3 is contiguous.
        Vec3 worldPosition{};
        if (!mem::Read(hProcess_, actor + GameOffsets::ACTOR_CACHED_POSITION, worldPosition))
            continue;

        // Invalid position values cannot produce a reliable screen-space projection.
        if (!std::isfinite(worldPosition.x) || !std::isfinite(worldPosition.y) || !std::isfinite(worldPosition.z))
            continue;

        // This receives the calculated client-area position for the enemy dot.
        Vec2 screenPosition{};

        // This receives camera-space XYZ only when debug output needs to show the transform result.
        Vec3 cameraPosition{};

        // This transforms and projects the enemy using the one camera matrix and FOV read for this frame.
        const bool onScreen = WorldToScreen(worldPosition,
                                            worldToLocal,
                                            activeFov,
                                            clientWidth,
                                            clientHeight,
                                            screenPosition,
                                            shouldPrintDebug ? &cameraPosition : nullptr);

        // This prints one enemy's world/camera/screen values to make first-run matrix debugging straightforward.
        if (shouldPrintDebug && !printedEnemyDebug)
        {
            std::cout << "[ESP DEBUG] Enemy Actor=0x" << std::hex << actor << std::dec
                      << " World=(" << worldPosition.x << ", " << worldPosition.y << ", " << worldPosition.z << ")"
                      << " Camera=(" << cameraPosition.x << ", " << cameraPosition.y << ", " << cameraPosition.z << ")"
                      << " Screen=(" << screenPosition.x << ", " << screenPosition.y << ")"
                      << " OnScreen=" << (onScreen ? "true" : "false") << std::endl;
            printedEnemyDebug = true;
        }

        // Off-screen or behind-camera Actors do not get a dot this frame.
        if (!onScreen)
            continue;

        // This stores the final screen-space position for the overlay renderer.
        outDots.push_back(screenPosition);
    }

    // This updates the debug timer only when a diagnostic cycle was actually printed.
    if (shouldPrintDebug)
        lastDebugPrint_ = now;

    // Reaching this point means the required list and camera state were read successfully.
    return true;
}
