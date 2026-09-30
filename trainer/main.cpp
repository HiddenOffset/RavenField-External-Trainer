// main.cpp defines the entry point to access RavenField.exe.

#include <array>
#include <cmath>
#include <iostream>
#include <vector>
#include <unordered_map>
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
    bool noRecoilEnabled,
    bool overHeatEnabled,
    bool ignorePlayerEnabled,
    bool weaponBobbingEnabled,
    float speedMultiValue,
    bool espEnabled,
    bool balanceEnabled)
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
        << "[NUMPAD 5] No Recoil: "
        << (noRecoilEnabled ? "ON" : "OFF")
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

    // F2 controls the player's balance and maximum balance.
    std::cout
        << "[F2]       Infinite Balance: "
        << (balanceEnabled ? "ON" : "OFF")
        << std::endl;

    // Aim assist activates only while Left Alt is physically held.
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

namespace
{
    // These are the Weapon.Configuration float fields that together control
    // recoil, spread, snap, and rattle.
    constexpr std::array<uintptr_t, 18> kNoRecoilConfigurationOffsets = {
        GameOffsets::RECOIL_KICKBACK,
        GameOffsets::RECOIL_RANDOM_KICK,
        GameOffsets::RECOIL_SPREAD,
        GameOffsets::RECOIL_FOLLOWUP_SPREAD_GAIN,
        GameOffsets::RECOIL_FOLLOWUP_MAX_SPREAD_HIP,
        GameOffsets::RECOIL_FOLLOWUP_MAX_SPREAD_AIM,
        GameOffsets::RECOIL_FOLLOWUP_SPREAD_STAY_TIME,
        GameOffsets::RECOIL_FOLLOWUP_SPREAD_DISSIPATE_TIME,
        GameOffsets::RECOIL_SNAP_MAGNITUDE,
        GameOffsets::RECOIL_SNAP_DURATION,
        GameOffsets::RECOIL_SNAP_FREQUENCY,
        GameOffsets::RECOIL_RATTLE_MAGNITUDE,
        GameOffsets::RECOIL_RATTLE_DURATION,
        GameOffsets::RECOIL_RATTLE_FREQUENCY,
        GameOffsets::RECOIL_KICKBACK_PRONE_MULTIPLIER,
        GameOffsets::RECOIL_SPREAD_PRONE_MULTIPLIER,
        GameOffsets::RECOIL_FOLLOWUP_SPREAD_PRONE_MULTIPLIER,
        GameOffsets::RECOIL_SNAP_PRONE_MULTIPLIER
    };

    // This stores an untouched copy of every recoil-related float
    // from one configuration object.
    struct RecoilConfigurationBackup
    {
        std::array<float, kNoRecoilConfigurationOffsets.size()> values{};
    };

    // This safely reads one pointer-sized value from Ravenfield.
    bool ReadPointerValue(
        HANDLE hProcess,
        uintptr_t address,
        uintptr_t& outValue)
    {
        outValue = 0;

        SIZE_T bytesRead = 0;

        const BOOL success =
            ReadProcessMemory(
                hProcess,
                reinterpret_cast<LPCVOID>(
                    address),
                &outValue,
                sizeof(outValue),
                &bytesRead);

        return success &&
            bytesRead == sizeof(outValue);
    }

    // This safely reads one float from Ravenfield.
    bool ReadFloatValue(
        HANDLE hProcess,
        uintptr_t address,
        float& outValue)
    {
        outValue = 0.0f;

        SIZE_T bytesRead = 0;

        const BOOL success =
            ReadProcessMemory(
                hProcess,
                reinterpret_cast<LPCVOID>(
                    address),
                &outValue,
                sizeof(outValue),
                &bytesRead);

        return success &&
            bytesRead == sizeof(outValue);
    }

    // This safely writes one float to Ravenfield.
    bool WriteFloatValue(
        HANDLE hProcess,
        uintptr_t address,
        float value)
    {
        SIZE_T bytesWritten = 0;

        const BOOL success =
            WriteProcessMemory(
                hProcess,
                reinterpret_cast<LPVOID>(
                    address),
                &value,
                sizeof(value),
                &bytesWritten);

        return success &&
            bytesWritten == sizeof(value);
    }

    // This captures the complete original recoil block before
    // that configuration is modified.
    bool ReadRecoilConfiguration(
        HANDLE hProcess,
        uintptr_t configuration,
        RecoilConfigurationBackup& outBackup)
    {
        if (configuration == 0)
        {
            return false;
        }

        for (size_t i = 0;
            i < kNoRecoilConfigurationOffsets.size();
            ++i)
        {
            if (!ReadFloatValue(
                hProcess,
                configuration +
                kNoRecoilConfigurationOffsets[i],
                outBackup.values[i]))
            {
                return false;
            }
        }

        return true;
    }

    // This writes zero to every recoil/spread field in one configuration object.
    void ZeroRecoilConfiguration(
        HANDLE hProcess,
        uintptr_t configuration)
    {
        constexpr float zero =
            0.0f;

        for (const uintptr_t offset :
        kNoRecoilConfigurationOffsets)
        {
            WriteFloatValue(
                hProcess,
                configuration + offset,
                zero);
        }
    }

    // This restores one configuration object to the exact values captured
    // before No Recoil touched it.
    void RestoreRecoilConfiguration(
        HANDLE hProcess,
        uintptr_t configuration,
        const RecoilConfigurationBackup& backup)
    {
        for (size_t i = 0;
            i < kNoRecoilConfigurationOffsets.size();
            ++i)
        {
            WriteFloatValue(
                hProcess,
                configuration +
                kNoRecoilConfigurationOffsets[i],
                backup.values[i]);
        }
    }

    // This resolves the Weapon object currently equipped by the local player.
    bool GetCurrentWeapon(
        HANDLE hProcess,
        uintptr_t moduleBase,
        uintptr_t& outWeapon)
    {
        outWeapon = 0;

        // ResolveAddress returns the address of Actor.activeWeapon,
        // so one final pointer read obtains the actual Weapon object.
        const uintptr_t activeWeaponField =
            ResolveAddress(
                hProcess,
                moduleBase,
                GameAddresses::ACTIVE_WEAPON_FIELD);

        if (activeWeaponField == 0)
        {
            return false;
        }

        return ReadPointerValue(
            hProcess,
            activeWeaponField,
            outWeapon) &&
            outWeapon != 0;
    }

    // This applies No Recoil to the currently equipped weapon while
    // preserving original values per Weapon and per Configuration object.
    void ApplyNoRecoilToCurrentWeapon(
        HANDLE hProcess,
        uintptr_t moduleBase,
        std::unordered_map<uintptr_t, float>& runtimeSpreadBackups,
        std::unordered_map<uintptr_t, RecoilConfigurationBackup>&
        configurationBackups)
    {
        // This resolves the currently held Weapon object.
        uintptr_t weapon = 0;

        if (!GetCurrentWeapon(
            hProcess,
            moduleBase,
            weapon))
        {
            return;
        }

        // Weapon + 0x40 points to the Configuration object containing
        // the recoil values discovered in Cheat Engine.
        uintptr_t configuration = 0;

        if (!ReadPointerValue(
            hProcess,
            weapon +
            GameOffsets::WEAPON_CONFIGURATION,
            configuration) ||
            configuration == 0)
        {
            return;
        }

        // The old No Gun Spread feature used Weapon + 0x1A0.
        //
        // Save this once for each distinct Weapon object encountered.
        if (runtimeSpreadBackups.find(
            weapon) ==
            runtimeSpreadBackups.end())
        {
            float originalRuntimeSpread =
                0.0f;

            if (!ReadFloatValue(
                hProcess,
                weapon +
                GameOffsets::WEAPON_RUNTIME_SPREAD,
                originalRuntimeSpread))
            {
                return;
            }

            runtimeSpreadBackups.emplace(
                weapon,
                originalRuntimeSpread);
        }

        // Configuration objects are also backed up only once.
        //
        // This matters if two Weapon objects happen to reference the
        // same Configuration object: we must never back up an already-zeroed copy.
        if (configurationBackups.find(
            configuration) ==
            configurationBackups.end())
        {
            RecoilConfigurationBackup backup{};

            if (!ReadRecoilConfiguration(
                hProcess,
                configuration,
                backup))
            {
                return;
            }

            configurationBackups.emplace(
                configuration,
                backup);
        }

        // This keeps the old known-working No Gun Spread behavior.
        constexpr float noRuntimeSpread =
            -1.0f;

        WriteFloatValue(
            hProcess,
            weapon +
            GameOffsets::WEAPON_RUNTIME_SPREAD,
            noRuntimeSpread);

        // This removes configuration-level kickback, random kick,
        // spread, snap, rattle, and prone recoil multipliers.
        ZeroRecoilConfiguration(
            hProcess,
            configuration);
    }

    // This restores every Weapon and Configuration touched while
    // No Recoil was enabled.
    void RestoreNoRecoilBackups(
        HANDLE hProcess,
        std::unordered_map<uintptr_t, float>& runtimeSpreadBackups,
        std::unordered_map<uintptr_t, RecoilConfigurationBackup>&
        configurationBackups)
    {
        // Restore each Configuration object exactly once.
        for (const auto& entry :
            configurationBackups)
        {
            RestoreRecoilConfiguration(
                hProcess,
                entry.first,
                entry.second);
        }

        // Restore the original runtime spread for every Weapon encountered.
        for (const auto& entry :
            runtimeSpreadBackups)
        {
            WriteFloatValue(
                hProcess,
                entry.first +
                GameOffsets::WEAPON_RUNTIME_SPREAD,
                entry.second);
        }

        // A future enable cycle should capture fresh game values.
        configurationBackups.clear();
        runtimeSpreadBackups.clear();
    }
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

    uintptr_t balanceAddr =
        ResolveAddress(
            hProcess,
            moduleBase,
            GameAddresses::BALANCE);

    uintptr_t maxBalanceAddr =
        ResolveAddress(
            hProcess,
            moduleBase,
            GameAddresses::MAX_BALANCE);

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
        balanceAddr == 0 ||
        maxBalanceAddr == 0 ||
        ammoAddr == 0 ||
        ammoReserveAddr == 0 ||
        yAxisAddr == 0 ||
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
    float storedBalance = 0.0f;
    float storedMaxBalance = 0.0f;
    int storedAmmo = 0;
    int storedAmmoReserve = 0;
    float storedOverHeat = 0.0f;
    bool storedIgnorePlayer = false;

    // These are the existing trainer feature states.
    bool healthEnabled = false;
    bool balanceEnabled = false;
    bool ammoEnabled = false;
    bool ammoReserveEnabled = false;
    bool noRecoilEnabled = false;
    bool overHeatEnabled = false;
    bool ignorePlayerEnabled = false;
    bool weaponBobbingEnabled = false;

    // F1 controls only whether the red ESP dots are displayed.
    bool espEnabled = false;

    // These maps preserve the original recoil values for every object
    // touched while No Recoil is enabled.
    std::unordered_map<uintptr_t, float>
        noRecoilRuntimeSpreadBackups;

    std::unordered_map<uintptr_t, RecoilConfigurationBackup>
        noRecoilConfigurationBackups;

    // These remember previous displayed states so the menu is not constantly redrawn.
    bool lastHealthState = false;
    bool lastBalanceState = false;
    bool lastAmmoState = false;
    bool lastAmmoReserveState = false;
    bool lastNoRecoilState = false;
    bool lastOverHeatState = false;
    bool lastIgnorePlayerState = false;
    bool lastWeaponBobbingState = false;
    bool lastEspState = false;
    float lastSpeedMultiValue = 1.0f;

    // This tracks one-shot NUMPAD 4 presses.
    bool lastYAxisPress = false;

    // This tracks one-shot NUMPAD 5 presses so No Recoil toggles only once per physical press.
    bool lastNoRecoilPress = false;

    // This tracks one-shot NUMPAD 9 presses.
    bool lastSpeedPress = false;

    // This tracks one-shot F1 presses.
    bool lastEspPress = false;

    // This tracks one-shot F2 presses.
    bool lastBalancePress = false;

    // This stores the active movement-speed multiplier.
    float speedMultiValue = 1.0f;

    // This draws the initial trainer menu.
    PrintMenu(
        healthEnabled,
        ammoEnabled,
        ammoReserveEnabled,
        noRecoilEnabled,
        overHeatEnabled,
        ignorePlayerEnabled,
        weaponBobbingEnabled,
        speedMultiValue,
        espEnabled,
        balanceEnabled);

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

        // F2 toggles Infinite Balance once per physical key press.
        const bool balancePress =
            (GetAsyncKeyState(VK_F2) &
                0x8000) != 0;

        if (balancePress &&
            !lastBalancePress)
        {
            balanceEnabled =
                !balanceEnabled;

            if (balanceEnabled)
            {
                // Re-resolve both fields before saving their current values.
                uintptr_t newBalanceAddr =
                    ResolveAddress(
                        hProcess,
                        moduleBase,
                        GameAddresses::BALANCE);

                uintptr_t newMaxBalanceAddr =
                    ResolveAddress(
                        hProcess,
                        moduleBase,
                        GameAddresses::MAX_BALANCE);

                if (newBalanceAddr != 0)
                {
                    balanceAddr =
                        newBalanceAddr;
                }

                if (newMaxBalanceAddr != 0)
                {
                    maxBalanceAddr =
                        newMaxBalanceAddr;
                }

                // Save the normal game values so they can be restored later.
                ReadProcessMemory(
                    hProcess,
                    reinterpret_cast<LPCVOID>(
                        balanceAddr),
                    &storedBalance,
                    sizeof(storedBalance),
                    nullptr);

                ReadProcessMemory(
                    hProcess,
                    reinterpret_cast<LPCVOID>(
                        maxBalanceAddr),
                    &storedMaxBalance,
                    sizeof(storedMaxBalance),
                    nullptr);

                // Both values are immediately raised to the same very large value.
                float balanceValue =
                    9999.0f;

                WriteProcessMemory(
                    hProcess,
                    reinterpret_cast<LPVOID>(
                        balanceAddr),
                    &balanceValue,
                    sizeof(balanceValue),
                    nullptr);

                WriteProcessMemory(
                    hProcess,
                    reinterpret_cast<LPVOID>(
                        maxBalanceAddr),
                    &balanceValue,
                    sizeof(balanceValue),
                    nullptr);
            }
            else
            {
                // Turning the feature off restores the values captured when enabled.
                WriteProcessMemory(
                    hProcess,
                    reinterpret_cast<LPVOID>(
                        balanceAddr),
                    &storedBalance,
                    sizeof(storedBalance),
                    nullptr);

                WriteProcessMemory(
                    hProcess,
                    reinterpret_cast<LPVOID>(
                        maxBalanceAddr),
                    &storedMaxBalance,
                    sizeof(storedMaxBalance),
                    nullptr);
            }
        }

        lastBalancePress =
            balancePress;

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

        // NUMPAD 5 toggles the combined No Recoil feature once per physical key press.
        const bool noRecoilPress =
            (GetAsyncKeyState(VK_NUMPAD5) &
                0x8000) != 0;

        if (noRecoilPress &&
            !lastNoRecoilPress)
        {
            noRecoilEnabled =
                !noRecoilEnabled;

            // Turning No Recoil off restores every weapon/configuration
            // touched during this enable cycle.
            if (!noRecoilEnabled)
            {
                RestoreNoRecoilBackups(
                    hProcess,
                    noRecoilRuntimeSpreadBackups,
                    noRecoilConfigurationBackups);
            }
        }

        lastNoRecoilPress =
            noRecoilPress;

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

        // This continuously re-resolves and enforces both balance values.
        if (balanceEnabled)
        {
            uintptr_t newBalanceAddr =
                ResolveAddress(
                    hProcess,
                    moduleBase,
                    GameAddresses::BALANCE);

            if (newBalanceAddr != 0 &&
                newBalanceAddr != balanceAddr)
            {
                balanceAddr =
                    newBalanceAddr;
            }

            uintptr_t newMaxBalanceAddr =
                ResolveAddress(
                    hProcess,
                    moduleBase,
                    GameAddresses::MAX_BALANCE);

            if (newMaxBalanceAddr != 0 &&
                newMaxBalanceAddr != maxBalanceAddr)
            {
                maxBalanceAddr =
                    newMaxBalanceAddr;
            }

            float balanceValue =
                9999.0f;

            WriteProcessMemory(
                hProcess,
                reinterpret_cast<LPVOID>(
                    balanceAddr),
                &balanceValue,
                sizeof(balanceValue),
                nullptr);

            WriteProcessMemory(
                hProcess,
                reinterpret_cast<LPVOID>(
                    maxBalanceAddr),
                &balanceValue,
                sizeof(balanceValue),
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

        // This continuously applies No Recoil to whichever weapon
        // is currently equipped.
        if (noRecoilEnabled)
        {
            ApplyNoRecoilToCurrentWeapon(
                hProcess,
                moduleBase,
                noRecoilRuntimeSpreadBackups,
                noRecoilConfigurationBackups);
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
                    IsRavenfieldForeground(
                        gameWindow))
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
                // ActorManager is re-resolved every frame just like
                // the existing ESP implementation.
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
                        IsRavenfieldForeground(
                            gameWindow))
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
            balanceEnabled != lastBalanceState ||
            ammoEnabled != lastAmmoState ||
            ammoReserveEnabled != lastAmmoReserveState ||
            noRecoilEnabled != lastNoRecoilState ||
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
                noRecoilEnabled,
                overHeatEnabled,
                ignorePlayerEnabled,
                weaponBobbingEnabled,
                speedMultiValue,
                espEnabled,
                balanceEnabled);

            lastHealthState =
                healthEnabled;

            lastBalanceState =
                balanceEnabled;

            lastAmmoState =
                ammoEnabled;

            lastAmmoReserveState =
                ammoReserveEnabled;

            lastNoRecoilState =
                noRecoilEnabled;

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

    // Restore balance values if the trainer exits while Infinite Balance is active.
    if (balanceEnabled)
    {
        WriteProcessMemory(
            hProcess,
            reinterpret_cast<LPVOID>(
                balanceAddr),
            &storedBalance,
            sizeof(storedBalance),
            nullptr);

        WriteProcessMemory(
            hProcess,
            reinterpret_cast<LPVOID>(
                maxBalanceAddr),
            &storedMaxBalance,
            sizeof(storedMaxBalance),
            nullptr);
    }

    // Restore any weapon values that are still modified if the trainer exits
    // while No Recoil is enabled.
    RestoreNoRecoilBackups(
        hProcess,
        noRecoilRuntimeSpreadBackups,
        noRecoilConfigurationBackups);

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