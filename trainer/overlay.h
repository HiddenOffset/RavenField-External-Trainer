#pragma once

#include <Windows.h>
#include <vector>
#include "math_types.h"

// Overlay owns a transparent click-through Win32 window positioned over Ravenfield's client area.
class Overlay
{
public:
    Overlay() = default;
    ~Overlay();

    // Copying the HWND-owning overlay would create double-destroy problems, so copying is disabled.
    Overlay(const Overlay&) = delete;
    Overlay& operator=(const Overlay&) = delete;

    // This creates the transparent overlay for the supplied Ravenfield top-level window.
    bool Create(HWND gameWindow);

    // This destroys the overlay window if it currently exists.
    void Destroy();

    // This reports whether Create() currently owns a valid overlay HWND.
    bool IsCreated() const;

    // This follows Ravenfield's live client-area position and returns its current pixel dimensions.
    bool UpdateBounds(int& clientWidth, int& clientHeight);

    // This replaces the points drawn by the next overlay paint.
    void SetDots(const std::vector<Vec2>& dots);

    // This removes every currently stored dot and refreshes the overlay.
    void Clear();

    // This hides the overlay without destroying it.
    void Hide();

    // This dispatches pending Win32 messages for the overlay window.
    void PumpMessages();

private:
    // This is the Win32 callback that forwards messages to the correct Overlay object.
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    // This handles messages after WindowProc has recovered the Overlay instance pointer.
    LRESULT HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    // This paints the transparent background and all current dots using a double-buffered GDI surface.
    void Paint();

    // This is the transparent topmost window owned by this class.
    HWND overlayWindow_ = nullptr;

    // This is Ravenfield's top-level window that the overlay follows.
    HWND gameWindow_ = nullptr;

    // These are the screen-space enemy points from the most recent ESP frame.
    std::vector<Vec2> dots_;
};
