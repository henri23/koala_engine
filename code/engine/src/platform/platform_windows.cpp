#include "platform/platform.hpp"

// Windows platform layer.
#if ENGINE_PLATFORM_WINDOWS

#include "core/logger.hpp"
#include "core/input.hpp"
#include "core/event.hpp"

#include <windows.h>
#include <windowsx.h>  // param input extraction
#include <stdlib.h>

#include "containers/auto_array.hpp"

#include "renderer/vulkan/vulkan_types.hpp"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

struct Internal_State {
    HINSTANCE h_instance;
    HWND hwnd;
};

// Clock
static f64 clock_frequency;
static LARGE_INTEGER start_time;

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param);

b8 platform_startup(
    Platform_State *plat_state,
    const char *application_name,
    s32 x,
    s32 y,
    s32 width,
    s32 height) {
    plat_state->internal_state = malloc(sizeof(Internal_State));
    Internal_State *state = (Internal_State *)plat_state->internal_state;

    state->h_instance = GetModuleHandleA(0);

    // Setup and register window class.
    HICON icon = LoadIcon(state->h_instance, IDI_APPLICATION);
    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.style = CS_DBLCLKS;  // Get double-clicks
    wc.lpfnWndProc = win32_process_message;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = state->h_instance;
    wc.hIcon = icon;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);  // NULL; // Manage the cursor manually
    wc.hbrBackground = NULL;                   // Transparent
    wc.lpszClassName = "koala_window_class";

    if (!RegisterClassA(&wc)) {
        MessageBoxA(0, "Window registration failed", "Error", MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }

    // Create window
    u32 client_x = x;
    u32 client_y = y;
    u32 client_width = width;
    u32 client_height = height;

    u32 window_x = client_x;
    u32 window_y = client_y;
    u32 window_width = client_width;
    u32 window_height = client_height;

    u32 window_style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
    u32 window_ex_style = WS_EX_APPWINDOW;

    window_style |= WS_MAXIMIZEBOX;
    window_style |= WS_MINIMIZEBOX;
    window_style |= WS_THICKFRAME;

    // Obtain the size of the border.
    RECT border_rect = {0, 0, 0, 0};
    AdjustWindowRectEx(&border_rect, window_style, 0, window_ex_style);

    // In this case, the border rectangle is negative.
    window_x += border_rect.left;
    window_y += border_rect.top;

    // Grow by the size of the OS border.
    window_width += border_rect.right - border_rect.left;
    window_height += border_rect.bottom - border_rect.top;

    HWND handle = CreateWindowExA(
        window_ex_style, "koala_window_class", application_name,
        window_style, window_x, window_y, window_width, window_height,
        0, 0, state->h_instance, 0);

    if (handle == 0) {
        MessageBoxA(NULL, "Window creation failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);

        ENGINE_FATAL("Window creation failed!");
        return FALSE;
    } else {
        state->hwnd = handle;
    }

    // Show the window
    b32 should_activate = 1;  // TODO: if the window should not accept input, this should be false.
    s32 show_window_command_flags = should_activate ? SW_SHOW : SW_SHOWNOACTIVATE;
    // If initially minimized, use SW_MINIMIZE : SW_SHOWMINNOACTIVE;
    // If initially maximized, use SW_SHOWMAXIMIZED : SW_MAXIMIZE
    ShowWindow(state->hwnd, show_window_command_flags);

    // Clock setup
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clock_frequency = 1.0 / (f64)frequency.QuadPart;
    QueryPerformanceCounter(&start_time);

    return TRUE;
}

void platform_shutdown(Platform_State *plat_state) {
    // Simply cold-cast to the known type.
    Internal_State *state = (Internal_State *)plat_state->internal_state;

    if (state->hwnd) {
        DestroyWindow(state->hwnd);
        state->hwnd = 0;
    }
}

b8 platform_message_pump(Platform_State *plat_state) {
    MSG message;
    while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    return TRUE;
}

void *platform_allocate(u64 size, b8 aligned) {
    return malloc(size);
}

void platform_free(void *block, b8 aligned) {
    free(block);
}

void *platform_zero_memory(void *block, u64 size) {
    return memset(block, 0, size);
}

void *platform_copy_memory(void *dest, const void *source, u64 size) {
    return memcpy(dest, source, size);
}

void* platform_move_memory(void* dest, const void* source, u64 size) {
    return memmove(dest, source, size);
}

void *platform_set_memory(void *dest, s32 value, u64 size) {
    return memset(dest, value, size);
}

void platform_console_write(const char *message, u8 colour) {
    HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[colour]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, number_written, 0);
}
// Vulkan platform specific definitions
void platform_get_required_extensions(Auto_Array<const char*>* required_extensions) {
    ENGINE_INFO("Attaching WIN32 surface for Windows platform");
    required_extensions->add("VK_KHR_win32_surface");
}

b8 platform_create_vulkan_surface(Vulkan_Context* context, Platform_State* plat_state) {

    ENGINE_INFO("Creating Vulkan WIN32 surface...");
    Internal_State* state = static_cast<Internal_State*>(plat_state->internal_state);

    VkWin32SurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = state->hwnd;
    createInfo.hinstance = state->h_instance;

    VK_ENSURE_SUCCESS(vkCreateWin32SurfaceKHR(context->instance, &createInfo, nullptr, &context->surface));

    ENGINE_INFO("Vulkan WIN32 surface created.");

    return TRUE;
}
void platform_console_write_error(const char *message, u8 colour) {
    HANDLE console_handle = GetStdHandle(STD_ERROR_HANDLE);
    // FATAL,ERROR,WARN,INFO,DEBUG,TRACE
    static u8 levels[6] = {64, 4, 6, 2, 1, 8};
    SetConsoleTextAttribute(console_handle, levels[colour]);
    OutputDebugStringA(message);
    u64 length = strlen(message);
    LPDWORD number_written = 0;
    WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, number_written, 0);
}

f64 platform_get_absolute_time() {
    LARGE_INTEGER now_time;
    QueryPerformanceCounter(&now_time);
    return (f64)now_time.QuadPart * clock_frequency;
}

void platform_sleep(u64 ms) {
    Sleep(ms);
}

LRESULT CALLBACK win32_process_message(HWND hwnd, u32 msg, WPARAM w_param, LPARAM l_param) {
    switch (msg) {
        case WM_ERASEBKGND:
            // Notify the OS that erasing will be handled by the application to prevent flicker.
            return 1;
        case WM_CLOSE: {
			Event_Context data = {};
			event_fire(Event_Code::APPLICATION_QUIT, nullptr, data);
			return TRUE;
		}
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE: {
            // Get the updated size.
            RECT r;
            GetClientRect(hwnd, &r);
            u32 width = r.right - r.left;
            u32 height = r.bottom - r.top;

			Event_Context context;
			context.data.u16[0] = (u16)width;
			context.data.u16[1] = (u16)height;
			event_fire(Event_Code::RESIZED, nullptr, context);
        } break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            // Key pressed/released
            b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
            Keyboard_Key key = static_cast<Keyboard_Key>(w_param);

            // No need to translate windows keycodes since we are using
            // windows standard
            // TODO: Read modifier mask for windows
            input_process_key(key, 0, pressed);
        } break;
        case WM_MOUSEMOVE: {
            // Mouse move
            s32 x_position = GET_X_LPARAM(l_param);
            s32 y_position = GET_Y_LPARAM(l_param);
            
            // Pass over to the input subsystem.
            input_process_mouse_move(x_position, y_position);
        } break;
        case WM_MOUSEWHEEL: {
            s32 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
            if (z_delta != 0) {
                // Flatten the input to an OS-independent (-1, 1)
                z_delta = (z_delta < 0) ? -1 : 1;
                input_process_mouse_wheel_move(z_delta);
            }
        } break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            b8 pressed = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
            Mouse_Button mouse_button = Mouse_Button::MAX_BUTTONS;
            switch (msg) {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                    mouse_button = Mouse_Button::LEFT;
                    break;
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                    mouse_button = Mouse_Button::MIDDLE;
                    break;
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                    mouse_button = Mouse_Button::RIGHT;
                    break;
            }

            // Pass over to the input subsystem.
            if (mouse_button != Mouse_Button::MAX_BUTTONS) {
                input_process_button(mouse_button, pressed);
            }
        } break;
    }

    return DefWindowProcA(hwnd, msg, w_param, l_param);
}

#endif
