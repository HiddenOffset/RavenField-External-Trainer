#include "overlay.h"

#include <cmath>

#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "User32.lib")

namespace
{
    // This class name is registered once with Win32 for the ESP overlay window.
    constexpr wchar_t kOverlayClassName[] = L"RavenfieldDotEspOverlay";

    // Pure black is treated as transparent by the layered-window color key.
    constexpr COLORREF kTransparentColor = RGB(0, 0, 0);

    // The first ESP version intentionally draws only a small solid red dot.
    constexpr COLORREF kDotColor = RGB(255, 0, 0);

    // This controls the requested dot radius in client-area pixels.
    constexpr int kDotRadius = 5;
}

Overlay::~Overlay()
{
    // This makes sure the native overlay HWND is released when the trainer exits.
    Destroy();
}

bool Overlay::Create(HWND gameWindow)
{
    // An invalid Ravenfield HWND cannot be followed by the overlay.
    if (gameWindow == nullptr || !IsWindow(gameWindow))
        return false;

    // If an overlay already exists for this same game window, no second window is needed.
    if (overlayWindow_ != nullptr && IsWindow(overlayWindow_) && gameWindow_ == gameWindow)
        return true;

    // This removes any previous overlay before attaching to a replacement Ravenfield HWND.
    Destroy();

    // This stores the Ravenfield window that UpdateBounds() will follow.
    gameWindow_ = gameWindow;

    // This gets the executable module handle required for Win32 window-class registration.
    HINSTANCE instance = GetModuleHandleW(nullptr);

    // This describes the lightweight native window class used only by the overlay.
    WNDCLASSEXW windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = Overlay::WindowProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.lpszClassName = kOverlayClassName;

    // This registers the class unless an earlier Overlay instance already registered it.
    if (RegisterClassExW(&windowClass) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
    {
        gameWindow_ = nullptr;
        return false;
    }

    // These extended styles make the overlay transparent, topmost, click-through, hidden from Alt-Tab, and non-activating.
    const DWORD extendedStyle = WS_EX_LAYERED |
                                WS_EX_TRANSPARENT |
                                WS_EX_TOPMOST |
                                WS_EX_TOOLWINDOW |
                                WS_EX_NOACTIVATE;

    // This creates a borderless popup; UpdateBounds() will immediately size it to Ravenfield's client area.
    overlayWindow_ = CreateWindowExW(extendedStyle,
                                    kOverlayClassName,
                                    L"Ravenfield Dot ESP",
                                    WS_POPUP,
                                    0,
                                    0,
                                    1,
                                    1,
                                    nullptr,
                                    nullptr,
                                    instance,
                                    this);

    // A failed CreateWindowExW leaves the ESP usable without crashing the rest of the trainer.
    if (overlayWindow_ == nullptr)
    {
        gameWindow_ = nullptr;
        return false;
    }

    // This makes every pure-black overlay pixel transparent while leaving the red dots visible.
    if (!SetLayeredWindowAttributes(overlayWindow_, kTransparentColor, 0, LWA_COLORKEY))
    {
        Destroy();
        return false;
    }

    // The overlay starts hidden and becomes visible only while the ESP has valid foreground game bounds.
    ShowWindow(overlayWindow_, SW_HIDE);

    // The Win32 overlay is now ready to receive dots.
    return true;
}

void Overlay::Destroy()
{
    // DestroyWindow is called only when a live overlay HWND currently exists.
    if (overlayWindow_ != nullptr && IsWindow(overlayWindow_))
        DestroyWindow(overlayWindow_);

    // These members are reset so Create() can safely attach to a new Ravenfield window later.
    overlayWindow_ = nullptr;
    gameWindow_ = nullptr;
    dots_.clear();
}

bool Overlay::IsCreated() const
{
    // This confirms that the stored overlay handle still refers to a live Win32 window.
    return overlayWindow_ != nullptr && IsWindow(overlayWindow_);
}

bool Overlay::UpdateBounds(int& clientWidth, int& clientHeight)
{
    // The output dimensions are reset before any Win32 calls so callers never use stale values.
    clientWidth = 0;
    clientHeight = 0;

    // Both the game and overlay windows must still exist for alignment to be meaningful.
    if (!IsCreated() || gameWindow_ == nullptr || !IsWindow(gameWindow_))
        return false;

    // A minimized game has no useful visible client area, so its overlay is hidden.
    if (IsIconic(gameWindow_))
    {
        Hide();
        return false;
    }

    // This keeps the ESP from remaining visible above unrelated applications after an Alt-Tab.
    HWND foreground = GetForegroundWindow();
    HWND foregroundRoot = foreground != nullptr ? GetAncestor(foreground, GA_ROOT) : nullptr;
    if (foregroundRoot != gameWindow_)
    {
        Hide();
        return false;
    }

    // This reads Ravenfield's current client dimensions instead of assuming any fixed resolution.
    RECT clientRect{};
    if (!GetClientRect(gameWindow_, &clientRect))
    {
        Hide();
        return false;
    }

    // These dimensions exclude Ravenfield's title bar and window borders.
    clientWidth = clientRect.right - clientRect.left;
    clientHeight = clientRect.bottom - clientRect.top;

    // Empty client dimensions can occur during resize/minimize transitions and should not be projected into.
    if (clientWidth <= 0 || clientHeight <= 0)
    {
        Hide();
        clientWidth = 0;
        clientHeight = 0;
        return false;
    }

    // This converts Ravenfield client coordinate (0,0) into desktop screen coordinates for overlay placement.
    POINT clientTopLeft{ 0, 0 };
    if (!ClientToScreen(gameWindow_, &clientTopLeft))
    {
        Hide();
        clientWidth = 0;
        clientHeight = 0;
        return false;
    }

    // This moves and resizes the overlay to exactly cover the live Ravenfield client area without taking focus.
    if (!SetWindowPos(overlayWindow_,
                      HWND_TOPMOST,
                      clientTopLeft.x,
                      clientTopLeft.y,
                      clientWidth,
                      clientHeight,
                      SWP_NOACTIVATE | SWP_SHOWWINDOW))
    {
        Hide();
        clientWidth = 0;
        clientHeight = 0;
        return false;
    }

    // Valid current dimensions are now ready for the WorldToScreen aspect-ratio calculation.
    return true;
}

void Overlay::SetDots(const std::vector<Vec2>& dots)
{
    // This copies the current frame's projected enemy points for Paint().
    dots_ = dots;

    // Invalidating the client area requests a new frame from the Win32 paint system.
    if (IsCreated())
    {
        InvalidateRect(overlayWindow_, nullptr, FALSE);
        UpdateWindow(overlayWindow_);
    }
}

void Overlay::Clear()
{
    // This removes every enemy point from the renderer's current state.
    dots_.clear();

    // This refreshes the window immediately so stale dots disappear as soon as ESP data becomes invalid.
    if (IsCreated())
    {
        InvalidateRect(overlayWindow_, nullptr, FALSE);
        UpdateWindow(overlayWindow_);
    }
}

void Overlay::Hide()
{
    // Hiding preserves the overlay object so it can be shown again without recreating the HWND every Alt-Tab.
    if (IsCreated())
        ShowWindow(overlayWindow_, SW_HIDE);
}

void Overlay::PumpMessages()
{
    // This processes only messages that are currently waiting, so the trainer's main loop never blocks here.
    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
}

LRESULT CALLBACK Overlay::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // WM_NCCREATE contains the Overlay pointer passed through CreateWindowExW's lpParam argument.
    if (message == WM_NCCREATE)
    {
        CREATESTRUCTW* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
        Overlay* overlay = static_cast<Overlay*>(create->lpCreateParams);

        // This stores the C++ object pointer on the HWND so future Win32 messages can reach the same instance.
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(overlay));

        // This records the HWND early because Win32 sends creation messages before CreateWindowExW returns.
        if (overlay != nullptr)
            overlay->overlayWindow_ = hwnd;
    }

    // This recovers the Overlay object associated with the native window.
    Overlay* overlay = reinterpret_cast<Overlay*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    // Instance messages are delegated to the member handler when the object has been attached.
    if (overlay != nullptr)
        return overlay->HandleMessage(hwnd, message, wParam, lParam);

    // Messages received before the object is attached use the normal Win32 default handler.
    return DefWindowProcW(hwnd, message, wParam, lParam);
}

LRESULT Overlay::HandleMessage(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        // WM_PAINT redraws the transparent frame and all dots using the newest collected positions.
        Paint();
        return 0;

    case WM_ERASEBKGND:
        // Paint() already redraws the entire back buffer, so default background erasing would only add flicker.
        return 1;

    case WM_NCHITTEST:
        // HTTRANSPARENT makes mouse hit-testing pass through the overlay instead of blocking Ravenfield input.
        return HTTRANSPARENT;

    default:
        // All unrelated Win32 messages use standard popup-window behavior.
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }
}

void Overlay::Paint()
{
    // A missing overlay HWND means there is nothing to paint.
    if (!IsCreated())
        return;

    // BeginPaint validates the update region and supplies the destination device context.
    PAINTSTRUCT paint{};
    HDC paintDc = BeginPaint(overlayWindow_, &paint);

    // This reads the overlay's current local client dimensions after UpdateBounds() positioned it.
    RECT clientRect{};
    GetClientRect(overlayWindow_, &clientRect);
    const int width = clientRect.right - clientRect.left;
    const int height = clientRect.bottom - clientRect.top;

    // Zero-sized transition frames are simply validated without creating GDI resources.
    if (width <= 0 || height <= 0)
    {
        EndPaint(overlayWindow_, &paint);
        return;
    }

    // This memory DC is the off-screen back buffer used to avoid visible GDI flicker.
    HDC memoryDc = CreateCompatibleDC(paintDc);

    // This bitmap is the same size and format as the visible overlay client area.
    HBITMAP backBuffer = CreateCompatibleBitmap(paintDc, width, height);

    // Resource-allocation failures are handled by ending the paint without crashing the trainer.
    if (memoryDc == nullptr || backBuffer == nullptr)
    {
        if (backBuffer != nullptr)
            DeleteObject(reinterpret_cast<HGDIOBJ>(backBuffer));
        if (memoryDc != nullptr)
            DeleteDC(memoryDc);
        EndPaint(overlayWindow_, &paint);
        return;
    }

    // This selects the off-screen bitmap into the memory DC and saves the previous bitmap for cleanup.
    HGDIOBJ oldBitmap = SelectObject(memoryDc, reinterpret_cast<HGDIOBJ>(backBuffer));

    // Pure black becomes transparent because Create() configured it as the layered-window color key.
    FillRect(memoryDc, &clientRect, reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));

    // This brush fills every requested ESP marker with solid red.
    HBRUSH dotBrush = CreateSolidBrush(kDotColor);

    // A null stock pen prevents GDI from drawing a separate outline around each dot.
    HGDIOBJ oldBrush = SelectObject(memoryDc, reinterpret_cast<HGDIOBJ>(dotBrush));
    HGDIOBJ oldPen = SelectObject(memoryDc, GetStockObject(NULL_PEN));

    // This draws one simple filled circle at every projected living-enemy position.
    for (const Vec2& dot : dots_)
    {
        // Rounding converts the floating-point projection result into stable GDI pixel coordinates.
        const int x = static_cast<int>(std::lround(dot.x));
        const int y = static_cast<int>(std::lround(dot.y));

        // This draws the requested five-pixel-radius dot centered on the WorldToScreen result.
        Ellipse(memoryDc,
                x - kDotRadius,
                y - kDotRadius,
                x + kDotRadius + 1,
                y + kDotRadius + 1);
    }

    // This copies the fully composed off-screen frame to the visible layered window in one operation.
    BitBlt(paintDc, 0, 0, width, height, memoryDc, 0, 0, SRCCOPY);

    // These calls restore the previous GDI objects before deleting the resources created for this frame.
    SelectObject(memoryDc, oldPen);
    SelectObject(memoryDc, oldBrush);
    SelectObject(memoryDc, oldBitmap);

    // This releases the per-frame brush, bitmap, and memory DC after the final blit is complete.
    DeleteObject(reinterpret_cast<HGDIOBJ>(dotBrush));
    DeleteObject(reinterpret_cast<HGDIOBJ>(backBuffer));
    DeleteDC(memoryDc);

    // EndPaint completes the Win32 paint cycle and validates the update region.
    EndPaint(overlayWindow_, &paint);
}
