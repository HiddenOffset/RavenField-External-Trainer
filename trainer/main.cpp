// main.cpp defines the entry point to access RavenField.exe.

#include <cmath>
#include <iostream>
#include <vector>
#include <Windows.h>

#include "process.h"
#include "addresses.h"
#include "esp.h"
#include "overlay.h"

namespace
{
    // Higher values make the mouse move toward the target more slowly.
    //
    // 1.0f would attempt to move the entire screen-space difference immediately.
    constexpr float kAimSmoothing = 1.0f;

    // Very tiny movements are ignored to reduce shaking once the crosshair reaches the target.
    constexpr float kAimDeadzone = 1.0f;

    // This checks whether Ravenfield currently owns the foreground window.
    bool IsRavenfieldForeground(HWND gameWindow)
    {
        // An invalid Ravenfield HWND cannot own the foreground.
        if (gameWindow == nullptr ||
            !IsWindow(gameWindow))
        {
            return false;
        }

        // This reads the currently focused window.
        HWND foreground =
            GetForegroundWindow();

        // No foreground window means we should not send mouse input.
        if (foreground == nullptr)
        {
            return false;
        }

        // This converts child windows into their top-level root window.
        HWND foregroundRoot =
            GetAncestor(
                foreground,
                GA_ROOT);

        // If GetAncestor fails, use the original foreground HWND.
        if (foregroundRoot == nullptr)
        {
            foregroundRoot =
                foreground;
        }

        // This returns true only while Ravenfield's top-level HWND owns focus.
        return foregroundRoot ==
            gameWindow;
    }

    // This moves the mouse toward one screen-space enemy target.
    void MoveMouseTowardTarget(
        const Vec2& target,
        int clientWidth,
        int clientHeight)
    {
        // Invalid client dimensions cannot produce a meaningful screen center.
        if (clientWidth <= 0 ||
            clientHeight <= 0)
        {
            return;
        }

        // This calculates the center of Ravenfield's client area.
        const float centerX =
            static_cast<float>(clientWidth) *
            0.5f;

        const float centerY =
            static_cast<float>(clientHeight) *
            0.5f;

        // This calculates how far the enemy is from the crosshair.
        const float differenceX =
            target.x -
            centerX;

        const float differenceY =
            target.y -
            centerY;

        // Tiny movements are ignored to prevent unnecessary jitter at the target.
        if (std::fabs(differenceX) <= kAimDeadzone &&
            std::fabs(differenceY) <= kAimDeadzone)
        {
            return;
        }

        // This divides the movement to produce a smoother pull toward the target.
        const float smoothedX =
            differenceX /
            kAimSmoothing;

        const float smoothedY =
            differenceY /
            kAimSmoothing;

        // SendInput expects relative mouse movement as integer values.
        const LONG mouseX =
            static_cast<LONG>(
                std::lround(
                    smoothedX));

        const LONG mouseY =
            static_cast<LONG>(
                std::lround(
                    smoothedY));

        // If rounding produced no movement, there is nothing useful to send.
        if (mouseX == 0 &&
            mouseY == 0)
        {
            return;
        }

        // This prepares one relative mouse movement event.
        INPUT input{};

        input.type =
            INPUT_MOUSE;

        input.mi.dx =
            mouseX;

        input.mi.dy =
            mouseY;

        input.mi.dwFlags =
            MOUSEEVENTF_MOVE;

        // This submits the synthetic mouse movement to Windows.
        SendInput(
            1,
            &input,
            sizeof(INPUT));
    }
}

void PrintWelcome()
{
    HANDLE hConsole =
        GetStdHandle(
            STD_OUTPUT_HANDLE);

    // Red text for "Raven".
    SetConsoleTextAttribute(
        hConsole,
        FOREGROUND_RED |
        FOREGROUND_INTENSITY);

    std::cout
        << "Raven";

    // White text for "field Trainer EA39".
    SetConsoleTextAttribute(
        hConsole,
        FOREGROUND_RED |
        FOREGROUND_GREEN |
        FOREGROUND_BLUE);

    std::cout
        << "field Trainer EA39, by hiddenOffset"
        << std::endl;

    std::cout
        << std::endl;

    // This briefly displays the welcome screen.
    Sleep(
        1000);

    // This clears the console before displaying the main trainer menu.
    system(
        "cls");
}

void PrintMenu(
    bool healthEnabled,
    bool ammoEnabled,
    bool ammoReserveEnabled,
    bool gunSpreadEnabled,
    bool overHeatEnabled,
    bool ignorePlayerEnabled,
    bool weaponBobbingEnabled,
    float speedMultiValue,
    bool espEnabled)
{
    // This clears the old menu before drawing the newest feature states.
    system(
        "cls");

    std::cout
        << "============== version 1.3  ==========="
        << std::endl;

    std::cout
        << std::endl;

    std::cout
        << "Features:"
        << std::endl;

    std::cout
        << "[NUMPAD 1] Health: "
        << (healthEnabled ? "ON" : "OFF")
        << std::endl;

    std::cout
        << "[NUMPAD 2] Ammo: "
        << (ammoEnabled ? "ON" : "OFF")
        << std::endl;

    std::cout
        << "[NUMPAD 3] Ammo Reserve: "
        << (ammoReserveEnabled ? "ON" : "OFF")
        << std::endl;

    std::cout
        << "[NUMPAD 4] Raise Y-Axis by +0.125"
        << std::endl;

    std::cout
        << "[NUMPAD 5] No Gun Spread: "
        << (gunSpreadEnabled ? "ON" : "OFF")
        << std::endl;

    std::cout
        << "[NUMPAD 6] No OverHeat: "
        << (overHeatEnabled ? "ON" : "OFF")
        << std::endl;

    std::cout
        << "[NUMPAD 7] Ignore Player: "
        << (ignorePlayerEnabled ? "ON" : "OFF")
        << std::endl;

    std::cout
        << "[NUMPAD 8] Disable Weapon Bobbing: "
        << (weaponBobbingEnabled ? "ON" : "OFF")
        << std::endl;

    std::cout
        << "[NUMPAD 9] Speed Multiplier: "
        << speedMultiValue
        << "x"
        << std::endl;

    std::cout
        << "[F1]       Bot Dot ESP: "
        << (espEnabled ? "ON" : "OFF")
        << std::endl;

    // The first aim-assist prototype activates only while Left Alt is physically held.
    std::cout
        << "[LEFT ALT] Hold Aim Assist"
        << std::endl;

    std::cout
        << "[INSERT]   Exit"
        << std::endl;

    std::cout
        << std::endl;

    std::cout
        << "========================================"
        << std::endl;
}

uintptr_t ResolveAddress(
    HANDLE hProcess,
    uintptr_t moduleBase,
    const Address& addr)
{
    // This calculates the runtime address of the pointer-chain base.
    uintptr_t dynamicPtrBaseAddr =
        moduleBase +
        addr.baseOffset;

    // This resolves the multi-level pointer chain.
    return findDMAAddy(
        hProcess,
        dynamicPtrBaseAddr,
        addr.offsets);
}

int main()
{
    // Step 1: Display the trainer welcome screen.
    PrintWelcome();

    // Step 2: Find Ravenfield's process ID.
    DWORD procId =
        GetProcessId(
            L"ravenfield.exe");

    if (procId == 0)
    {
        std::cout
            << "Failed to get process ID. Make sure RavenField.exe is running."
            << std::endl;

        Sleep(
            2000);

        return 1;
    }

    // This clears the console before the next startup stage.
    system(
        "cls");

    // This finds UnityPlayer.dll inside Ravenfield.
    uintptr_t moduleBase =
        GetModuleBaseAddress(
            procId,
            L"UnityPlayer.dll");

    if (moduleBase == 0)
    {
        std::cout
            << "Failed to get module base address."
            << std::endl;

        Sleep(
            2000);

        return 1;
    }

    // This opens Ravenfield so trainer features can read and write its memory.
    HANDLE hProcess =
        OpenProcess(
            PROCESS_ALL_ACCESS,
            FALSE,
            procId);

    if (hProcess == nullptr)
    {
        std::cout
            << "Failed to open process."
            << std::endl;

        Sleep(
            2000);

        return 1;
    }

    // This locates Ravenfield's largest visible top-level window.
    HWND gameWindow =
        FindMainWindow(
            procId);

    // This object owns the transparent ESP overlay.
    Overlay overlay;

    // This object owns enemy enumeration, WorldToScreen, and aim-target selection.
    EspSystem esp(
        hProcess);

    // This vector is reused for projected enemy ESP positions.
    std::vector<Vec2> espDots;

    // Resolve all existing trainer addresses.
    uintptr_t healthAddr =
        ResolveAddress(
            hProcess,
            moduleBase,
            GameAddresses::HEALTH);

    uintptr_t ammoAddr =
        ResolveAddress(
            hProcess,
            moduleBase,
            GameAddresses::AMMO);

    uintptr_t ammoReserveAddr =
        ResolveAddress(
            hProcess,
            moduleBase,
            GameAddresses::AMMO_RESERVE);

    uintptr_t yAxisAddr =
        ResolveAddress(
            hProcess,
            moduleBase,
            GameAddresses::Y_AXIS);

    uintptr_t gunSpreadAddr =
        ResolveAddress(
            hProcess,
            moduleBase,
            GameAddresses::GUN_SPREAD);

    uintptr_t overHeatAddr =
        ResolveAddress(
            hProcess,
            moduleBase,
            GameAddresses::NO_OVERHEAT);

    uintptr_t ignorePlayerAddr =
        ResolveAddress(
            hProcess,
            moduleBase,
            GameAddresses::IGNORE_PLAYER);

    uintptr_t walkBobMultiAddr =
        ResolveAddress(
            hProcess,
            moduleBase,
            GameAddresses::WALK_BOB_MULTI);

    uintptr_t sprintBobMultiAddr =
        ResolveAddress(
            hProcess,
            moduleBase,
            GameAddresses::SPRINT_BOB_MULTI);

    uintptr_t proneBobMultiAddr =
        ResolveAddress(
            hProcess,
            moduleBase,
            GameAddresses::PRONE_BOB_MULTI);

    uintptr_t speedMultiAddr =
        ResolveAddress(
            hProcess,
            moduleBase,
            GameAddresses::SPEED_MULTI);

    // This makes sure every original trainer feature resolved successfully.
    if (healthAddr == 0 ||
        ammoAddr == 0 ||
        ammoReserveAddr == 0 ||
        yAxisAddr == 0 ||
        gunSpreadAddr == 0 ||
        overHeatAddr == 0 ||
        ignorePlayerAddr == 0 ||
        walkBobMultiAddr == 0 ||
        sprintBobMultiAddr == 0 ||
        proneBobMultiAddr == 0 ||
        speedMultiAddr == 0)
    {
        std::cout
            << "Failed to resolve one or more addresses."
            << std::endl;

        CloseHandle(
            hProcess);

        return 1;
    }

    // This clears the console before displaying the main menu.
    system(
        "cls");

    // These values remember original game values for toggle restoration.
    float storedHealth = 0.0f;
    int storedAmmo = 0;
    int storedAmmoReserve = 0;
    float storedGunSpread = 0.0f;
    float storedOverHeat = 0.0f;
    bool storedIgnorePlayer = false;

    // These are the existing trainer feature states.
    bool healthEnabled = false;
    bool ammoEnabled = false;
    bool ammoReserveEnabled = false;
    bool gunSpreadEnabled = false;
    bool overHeatEnabled = false;
    bool ignorePlayerEnabled = false;
    bool weaponBobbingEnabled = false;

    // F1 controls only whether the red ESP dots are displayed.
    bool espEnabled = false;

    // These remember the previous displayed state so the menu is not constantly redrawn.
    bool lastHealthState = false;
    bool lastAmmoState = false;
    bool lastAmmoReserveState = false;
    bool lastGunSpreadState = false;
    bool lastOverHeatState = false;
    bool lastIgnorePlayerState = false;
    bool lastWeaponBobbingState = false;
    bool lastEspState = false;
    float lastSpeedMultiValue = 1.0f;

    // This tracks one-shot NUMPAD 4 presses.
    bool lastYAxisPress = false;

    // This tracks one-shot NUMPAD 9 presses.
    bool lastSpeedPress = false;

    // This stores the active movement-speed multiplier.
    float speedMultiValue = 1.0f;

    // This tracks one-shot F1 presses.
    bool lastEspPress = false;

    // This draws the initial trainer menu.
    PrintMenu(
        healthEnabled,
        ammoEnabled,
        ammoReserveEnabled,
        gunSpreadEnabled,
        overHeatEnabled,
        ignorePlayerEnabled,
        weaponBobbingEnabled,
        speedMultiValue,
        espEnabled);

    // This is the trainer's main update loop.
    while (true)
    {
        // This dispatches pending overlay messages without blocking the trainer.
        overlay.PumpMessages();

        // INSERT exits the trainer.
        if (GetAsyncKeyState(VK_INSERT) &
            0x8000)
        {
            break;
        }

        // Left Alt is intentionally hold-to-activate rather than a toggle.
        const bool aimbotHeld =
            (GetAsyncKeyState(VK_LMENU) &
                0x8000) != 0;

        // F1 toggles the visual Bot Dot ESP once per physical key press.
        const bool espPress =
            (GetAsyncKeyState(VK_F1) &
                0x8000) != 0;

        if (espPress &&
            !lastEspPress)
        {
            espEnabled =
                !espEnabled;

            // Turning ESP off immediately removes and hides previous markers.
            if (!espEnabled)
            {
                overlay.Clear();
                overlay.Hide();
            }
        }

        lastEspPress =
            espPress;

        // NUMPAD 1 toggles health.
        if (GetAsyncKeyState(VK_NUMPAD1) &
            0x8000)
        {
            healthEnabled =
                !healthEnabled;

            if (healthEnabled)
            {
                ReadProcessMemory(
                    hProcess,
                    reinterpret_cast<LPCVOID>(
                        healthAddr),
                    &storedHealth,
                    sizeof(storedHealth),
                    nullptr);

                float healthValue =
                    9999.0f;

                WriteProcessMemory(
                    hProcess,
                    reinterpret_cast<LPVOID>(
                        healthAddr),
                    &healthValue,
                    sizeof(healthValue),
                    nullptr);
            }
            else
            {
                WriteProcessMemory(
                    hProcess,
                    reinterpret_cast<LPVOID>(
                        healthAddr),
                    &storedHealth,
                    sizeof(storedHealth),
                    nullptr);
            }

            Sleep(
                200);
        }

        // NUMPAD 2 toggles ammunition.
        if (GetAsyncKeyState(VK_NUMPAD2) &
            0x8000)
        {
            ammoEnabled =
                !ammoEnabled;

            if (ammoEnabled)
            {
                ReadProcessMemory(
                    hProcess,
                    reinterpret_cast<LPCVOID>(
                        ammoAddr),
                    &storedAmmo,
                    sizeof(storedAmmo),
                    nullptr);

                int ammoValue =
                    9999;

                WriteProcessMemory(
                    hProcess,
                    reinterpret_cast<LPVOID>(
                        ammoAddr),
                    &ammoValue,
                    sizeof(ammoValue),
                    nullptr);
            }
            else
            {
                WriteProcessMemory(
                    hProcess,
                    reinterpret_cast<LPVOID>(
                        ammoAddr),
                    &storedAmmo,
                    sizeof(storedAmmo),
                    nullptr);
            }

            Sleep(
                200);
        }

        // NUMPAD 3 toggles reserve ammunition.
        if (GetAsyncKeyState(VK_NUMPAD3) &
            0x8000)
        {
            ammoReserveEnabled =
                !ammoReserveEnabled;

            if (ammoReserveEnabled)
            {
                ReadProcessMemory(
                    hProcess,
                    reinterpret_cast<LPCVOID>(
                        ammoReserveAddr),
                    &storedAmmoReserve,
                    sizeof(storedAmmoReserve),
                    nullptr);

                int ammoReserveValue =
                    9999;

                WriteProcessMemory(
                    hProcess,
                    reinterpret_cast<LPVOID>(
                        ammoReserveAddr),
                    &ammoReserveValue,
                    sizeof(ammoReserveValue),
                    nullptr);
            }
            else
            {
                WriteProcessMemory(
                    hProcess,
                    reinterpret_cast<LPVOID>(
                        ammoReserveAddr),
                    &storedAmmoReserve,
                    sizeof(storedAmmoReserve),
                    nullptr);
            }

            Sleep(
                200);
        }

        // NUMPAD 4 raises the Y-axis by +0.125 once per press.
        const bool yPress =
            (GetAsyncKeyState(VK_NUMPAD4) &
                0x8000) != 0;

        if (yPress &&
            !lastYAxisPress)
        {
            uintptr_t newYAxisAddr =
                ResolveAddress(
                    hProcess,
                    moduleBase,
                    GameAddresses::Y_AXIS);

            if (newYAxisAddr != 0 &&
                newYAxisAddr != yAxisAddr)
            {
                yAxisAddr =
                    newYAxisAddr;
            }

            float currentY =
                0.0f;

            if (ReadProcessMemory(
                hProcess,
                reinterpret_cast<LPCVOID>(
                    yAxisAddr),
                &currentY,
                sizeof(currentY),
                nullptr))
            {
                currentY +=
                    0.125f;

                WriteProcessMemory(
                    hProcess,
                    reinterpret_cast<LPVOID>(
                        yAxisAddr),
                    &currentY,
                    sizeof(currentY),
                    nullptr);
            }
        }

        lastYAxisPress =
            yPress;

        // NUMPAD 5 toggles no gun spread.
        if (GetAsyncKeyState(VK_NUMPAD5) &
            0x8000)
        {
            gunSpreadEnabled =
                !gunSpreadEnabled;

            if (gunSpreadEnabled)
            {
                ReadProcessMemory(
                    hProcess,
                    reinterpret_cast<LPCVOID>(
                        gunSpreadAddr),
                    &storedGunSpread,
                    sizeof(storedGunSpread),
                    nullptr);

                float spreadValue =
                    -1.0f;

                WriteProcessMemory(
                    hProcess,
                    reinterpret_cast<LPVOID>(
                        gunSpreadAddr),
                    &spreadValue,
                    sizeof(spreadValue),
                    nullptr);
            }
            else
            {
                WriteProcessMemory(
                    hProcess,
                    reinterpret_cast<LPVOID>(
                        gunSpreadAddr),
                    &storedGunSpread,
                    sizeof(storedGunSpread),
                    nullptr);
            }

            Sleep(
                200);
        }

        // NUMPAD 6 toggles no overheat.
        if (GetAsyncKeyState(VK_NUMPAD6) &
            0x8000)
        {
            overHeatEnabled =
                !overHeatEnabled;

            if (overHeatEnabled)
            {
                ReadProcessMemory(
                    hProcess,
                    reinterpret_cast<LPCVOID>(
                        overHeatAddr),
                    &storedOverHeat,
                    sizeof(storedOverHeat),
                    nullptr);

                float overHeatValue =
                    0.0f;

                WriteProcessMemory(
                    hProcess,
                    reinterpret_cast<LPVOID>(
                        overHeatAddr),
                    &overHeatValue,
                    sizeof(overHeatValue),
                    nullptr);
            }
            else
            {
                WriteProcessMemory(
                    hProcess,
                    reinterpret_cast<LPVOID>(
                        overHeatAddr),
                    &storedOverHeat,
                    sizeof(storedOverHeat),
                    nullptr);
            }

            Sleep(
                200);
        }

        // NUMPAD 7 toggles Ignore Player.
        if (GetAsyncKeyState(VK_NUMPAD7) &
            0x8000)
        {
            ignorePlayerEnabled =
                !ignorePlayerEnabled;

            ReadProcessMemory(
                hProcess,
                reinterpret_cast<LPCVOID>(
                    ignorePlayerAddr),
                &storedIgnorePlayer,
                sizeof(storedIgnorePlayer),
                nullptr);

            const bool value =
                ignorePlayerEnabled
                ? true
                : storedIgnorePlayer;

            WriteProcessMemory(
                hProcess,
                reinterpret_cast<LPVOID>(
                    ignorePlayerAddr),
                &value,
                sizeof(value),
                nullptr);

            Sleep(
                200);
        }

        // NUMPAD 8 toggles the weapon-bobbing multipliers.
        if (GetAsyncKeyState(VK_NUMPAD8) &
            0x8000)
        {
            weaponBobbingEnabled =
                !weaponBobbingEnabled;

            if (weaponBobbingEnabled)
            {
                float bobValue =
                    0.0f;

                WriteProcessMemory(
                    hProcess,
                    reinterpret_cast<LPVOID>(
                        sprintBobMultiAddr),
                    &bobValue,
                    sizeof(bobValue),
                    nullptr);

                WriteProcessMemory(
                    hProcess,
                    reinterpret_cast<LPVOID>(
                        proneBobMultiAddr),
                    &bobValue,
                    sizeof(bobValue),
                    nullptr);

                WriteProcessMemory(
                    hProcess,
                    reinterpret_cast<LPVOID>(
                        walkBobMultiAddr),
                    &bobValue,
                    sizeof(bobValue),
                    nullptr);
            }
            else
            {
                float bobValue =
                    1.0f;

                WriteProcessMemory(
                    hProcess,
                    reinterpret_cast<LPVOID>(
                        walkBobMultiAddr),
                    &bobValue,
                    sizeof(bobValue),
                    nullptr);

                WriteProcessMemory(
                    hProcess,
                    reinterpret_cast<LPVOID>(
                        sprintBobMultiAddr),
                    &bobValue,
                    sizeof(bobValue),
                    nullptr);

                WriteProcessMemory(
                    hProcess,
                    reinterpret_cast<LPVOID>(
                        proneBobMultiAddr),
                    &bobValue,
                    sizeof(bobValue),
                    nullptr);
            }

            Sleep(
                200);
        }

        // NUMPAD 9 cycles the speed multiplier from 1x to 2x to 3x and back to 1x.
        const bool speedPress =
            (GetAsyncKeyState(VK_NUMPAD9) &
                0x8000) != 0;

        if (speedPress &&
            !lastSpeedPress)
        {
            uintptr_t newSpeedAddr =
                ResolveAddress(
                    hProcess,
                    moduleBase,
                    GameAddresses::SPEED_MULTI);

            if (newSpeedAddr != 0 &&
                newSpeedAddr != speedMultiAddr)
            {
                speedMultiAddr =
                    newSpeedAddr;
            }

            speedMultiValue +=
                1.0f;

            if (speedMultiValue > 3.0f)
            {
                speedMultiValue =
                    1.0f;
            }

            WriteProcessMemory(
                hProcess,
                reinterpret_cast<LPVOID>(
                    speedMultiAddr),
                &speedMultiValue,
                sizeof(speedMultiValue),
                nullptr);
        }

        lastSpeedPress =
            speedPress;

        // This continuously re-resolves and enforces infinite health while enabled.
        if (healthEnabled)
        {
            uintptr_t newHealthAddr =
                ResolveAddress(
                    hProcess,
                    moduleBase,
                    GameAddresses::HEALTH);

            if (newHealthAddr != 0 &&
                newHealthAddr != healthAddr)
            {
                healthAddr =
                    newHealthAddr;
            }

            float healthValue =
                9999.0f;

            WriteProcessMemory(
                hProcess,
                reinterpret_cast<LPVOID>(
                    healthAddr),
                &healthValue,
                sizeof(healthValue),
                nullptr);
        }

        // This continuously re-resolves and enforces ammunition while enabled.
        if (ammoEnabled)
        {
            uintptr_t newAmmoAddr =
                ResolveAddress(
                    hProcess,
                    moduleBase,
                    GameAddresses::AMMO);

            if (newAmmoAddr != 0 &&
                newAmmoAddr != ammoAddr)
            {
                ammoAddr =
                    newAmmoAddr;
            }

            int ammoValue =
                9999;

            WriteProcessMemory(
                hProcess,
                reinterpret_cast<LPVOID>(
                    ammoAddr),
                &ammoValue,
                sizeof(ammoValue),
                nullptr);
        }

        // This continuously re-resolves and enforces reserve ammunition while enabled.
        if (ammoReserveEnabled)
        {
            uintptr_t newAmmoReserveAddr =
                ResolveAddress(
                    hProcess,
                    moduleBase,
                    GameAddresses::AMMO_RESERVE);

            if (newAmmoReserveAddr != 0 &&
                newAmmoReserveAddr != ammoReserveAddr)
            {
                ammoReserveAddr =
                    newAmmoReserveAddr;
            }

            int ammoReserveValue =
                9999;

            WriteProcessMemory(
                hProcess,
                reinterpret_cast<LPVOID>(
                    ammoReserveAddr),
                &ammoReserveValue,
                sizeof(ammoReserveValue),
                nullptr);
        }

        // This continuously enforces no spread while enabled.
        if (gunSpreadEnabled)
        {
            uintptr_t newGunSpreadAddr =
                ResolveAddress(
                    hProcess,
                    moduleBase,
                    GameAddresses::GUN_SPREAD);

            if (newGunSpreadAddr != 0 &&
                newGunSpreadAddr != gunSpreadAddr)
            {
                gunSpreadAddr =
                    newGunSpreadAddr;
            }

            float spreadValue =
                -1.0f;

            WriteProcessMemory(
                hProcess,
                reinterpret_cast<LPVOID>(
                    gunSpreadAddr),
                &spreadValue,
                sizeof(spreadValue),
                nullptr);
        }

        // This continuously enforces zero overheat while enabled.
        if (overHeatEnabled)
        {
            uintptr_t newOverHeatAddr =
                ResolveAddress(
                    hProcess,
                    moduleBase,
                    GameAddresses::NO_OVERHEAT);

            if (newOverHeatAddr != 0 &&
                newOverHeatAddr != overHeatAddr)
            {
                overHeatAddr =
                    newOverHeatAddr;
            }

            float overHeatValue =
                0.0f;

            WriteProcessMemory(
                hProcess,
                reinterpret_cast<LPVOID>(
                    overHeatAddr),
                &overHeatValue,
                sizeof(overHeatValue),
                nullptr);
        }

        // This continuously keeps the player ignored while enabled.
        if (ignorePlayerEnabled)
        {
            uintptr_t newIgnoreAddr =
                ResolveAddress(
                    hProcess,
                    moduleBase,
                    GameAddresses::IGNORE_PLAYER);

            if (newIgnoreAddr != 0 &&
                newIgnoreAddr != ignorePlayerAddr)
            {
                ignorePlayerAddr =
                    newIgnoreAddr;
            }

            bool value =
                true;

            WriteProcessMemory(
                hProcess,
                reinterpret_cast<LPVOID>(
                    ignorePlayerAddr),
                &value,
                sizeof(value),
                nullptr);
        }

        // This continuously keeps all three weapon-bobbing multipliers at zero.
        if (weaponBobbingEnabled)
        {
            uintptr_t newWalkAddr =
                ResolveAddress(
                    hProcess,
                    moduleBase,
                    GameAddresses::WALK_BOB_MULTI);

            if (newWalkAddr != 0 &&
                newWalkAddr != walkBobMultiAddr)
            {
                walkBobMultiAddr =
                    newWalkAddr;
            }

            uintptr_t newSprintAddr =
                ResolveAddress(
                    hProcess,
                    moduleBase,
                    GameAddresses::SPRINT_BOB_MULTI);

            if (newSprintAddr != 0 &&
                newSprintAddr != sprintBobMultiAddr)
            {
                sprintBobMultiAddr =
                    newSprintAddr;
            }

            uintptr_t newProneAddr =
                ResolveAddress(
                    hProcess,
                    moduleBase,
                    GameAddresses::PRONE_BOB_MULTI);

            if (newProneAddr != 0 &&
                newProneAddr != proneBobMultiAddr)
            {
                proneBobMultiAddr =
                    newProneAddr;
            }

            float bobValue =
                0.0f;

            WriteProcessMemory(
                hProcess,
                reinterpret_cast<LPVOID>(
                    walkBobMultiAddr),
                &bobValue,
                sizeof(bobValue),
                nullptr);

            WriteProcessMemory(
                hProcess,
                reinterpret_cast<LPVOID>(
                    sprintBobMultiAddr),
                &bobValue,
                sizeof(bobValue),
                nullptr);

            WriteProcessMemory(
                hProcess,
                reinterpret_cast<LPVOID>(
                    proneBobMultiAddr),
                &bobValue,
                sizeof(bobValue),
                nullptr);
        }

        // The selected speed multiplier is continuously enforced.
        uintptr_t newSpeedAddr =
            ResolveAddress(
                hProcess,
                moduleBase,
                GameAddresses::SPEED_MULTI);

        if (newSpeedAddr != 0 &&
            newSpeedAddr != speedMultiAddr)
        {
            speedMultiAddr =
                newSpeedAddr;
        }

        WriteProcessMemory(
            hProcess,
            reinterpret_cast<LPVOID>(
                speedMultiAddr),
            &speedMultiValue,
            sizeof(speedMultiValue),
            nullptr);

        // Enemy projection data is needed whenever either ESP or aim assist is active.
        if (espEnabled ||
            aimbotHeld)
        {
            // Ravenfield can recreate its top-level window, so a dead handle is rediscovered.
            if (gameWindow == nullptr ||
                !IsWindow(gameWindow))
            {
                overlay.Destroy();

                gameWindow =
                    FindMainWindow(
                        procId);
            }

            // These dimensions are needed by WorldToScreen and target selection.
            int clientWidth = 0;
            int clientHeight = 0;

            // This tracks whether we successfully obtained Ravenfield's current client size.
            bool haveClientDimensions =
                false;

            // When visual ESP is enabled, the overlay already provides accurate live dimensions.
            if (espEnabled)
            {
                // This lazily creates the visual overlay.
                if (gameWindow != nullptr &&
                    !overlay.IsCreated())
                {
                    overlay.Create(
                        gameWindow);
                }

                // UpdateBounds obtains Ravenfield's current client dimensions.
                if (overlay.IsCreated())
                {
                    haveClientDimensions =
                        overlay.UpdateBounds(
                            clientWidth,
                            clientHeight);
                }
            }
            else
            {
                // Aim assist can operate without displaying the visual ESP.
                //
                // In that case we obtain the client dimensions directly from Ravenfield.
                if (gameWindow != nullptr &&
                    IsWindow(gameWindow) &&
                    IsRavenfieldForeground(gameWindow))
                {
                    RECT clientRect{};

                    if (GetClientRect(
                        gameWindow,
                        &clientRect))
                    {
                        clientWidth =
                            clientRect.right -
                            clientRect.left;

                        clientHeight =
                            clientRect.bottom -
                            clientRect.top;

                        haveClientDimensions =
                            clientWidth > 0 &&
                            clientHeight > 0;
                    }
                }
            }

            // Projection only runs when the game has usable client dimensions.
            if (haveClientDimensions)
            {
                // ActorManager is re-resolved every frame just like the existing ESP implementation.
                uintptr_t actorManager =
                    ResolveAddress(
                        hProcess,
                        moduleBase,
                        GameAddresses::ACTOR_MANAGER);

                // This runs the existing enemy filtering and WorldToScreen calculations.
                const bool collectionSucceeded =
                    actorManager != 0 &&
                    esp.CollectEnemyDots(
                        actorManager,
                        clientWidth,
                        clientHeight,
                        espDots);

                if (collectionSucceeded)
                {
                    // F1 controls whether those projected points are actually drawn.
                    if (espEnabled)
                    {
                        overlay.SetDots(
                            espDots);
                    }

                    // Left Alt activates mouse movement only while Ravenfield is foreground.
                    if (aimbotHeld &&
                        IsRavenfieldForeground(gameWindow))
                    {
                        // This receives the enemy closest to the crosshair inside the aim FOV.
                        Vec2 aimTarget{};

                        // No mouse input is sent when no eligible enemy exists.
                        if (esp.GetClosestAimTarget(
                            aimTarget))
                        {
                            MoveMouseTowardTarget(
                                aimTarget,
                                clientWidth,
                                clientHeight);
                        }
                    }
                }
                else
                {
                    // A failed Actor/camera frame removes stale ESP dots.
                    if (espEnabled)
                    {
                        overlay.Clear();
                    }
                }
            }
            else
            {
                // Invalid window dimensions should not leave stale visual dots.
                if (espEnabled)
                {
                    overlay.Clear();
                }
            }
        }
        else
        {
            // When neither feature needs projection data, the visual overlay can remain hidden.
            overlay.Hide();
        }

        // The menu is redrawn only when an existing toggle changes.
        if (healthEnabled != lastHealthState ||
            ammoEnabled != lastAmmoState ||
            ammoReserveEnabled != lastAmmoReserveState ||
            gunSpreadEnabled != lastGunSpreadState ||
            overHeatEnabled != lastOverHeatState ||
            ignorePlayerEnabled != lastIgnorePlayerState ||
            weaponBobbingEnabled != lastWeaponBobbingState ||
            speedMultiValue != lastSpeedMultiValue ||
            espEnabled != lastEspState)
        {
            PrintMenu(
                healthEnabled,
                ammoEnabled,
                ammoReserveEnabled,
                gunSpreadEnabled,
                overHeatEnabled,
                ignorePlayerEnabled,
                weaponBobbingEnabled,
                speedMultiValue,
                espEnabled);

            lastHealthState =
                healthEnabled;

            lastAmmoState =
                ammoEnabled;

            lastAmmoReserveState =
                ammoReserveEnabled;

            lastGunSpreadState =
                gunSpreadEnabled;

            lastOverHeatState =
                overHeatEnabled;

            lastIgnorePlayerState =
                ignorePlayerEnabled;

            lastWeaponBobbingState =
                weaponBobbingEnabled;

            lastSpeedMultiValue =
                speedMultiValue;

            lastEspState =
                espEnabled;
        }

        // A short sleep prevents the trainer loop from consuming an entire CPU core.
        Sleep(
            5);
    }

    // This removes the transparent Win32 overlay before releasing Ravenfield.
    overlay.Destroy();

    // This closes the process handle opened during startup.
    CloseHandle(
        hProcess);

    // This clears the trainer UI before the final message.
    system(
        "cls");

    std::cout
        << "Exiting Ravenfield Trainer..."
        << std::endl;

    return 0;
}

// New features in next update:
/*
 * - improved speed hack precision
 * - better UI/UX for menu
 * - additional toggles as needed
 * - configurable hotkeys
 * - enhanced stability and performance
 * - thorough testing and validation
 */