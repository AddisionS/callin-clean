<p align="center">
  <img src="assets/icon.png" alt="Call-In Clean icon" width="128">
</p>

<h1 align="center">Call-In Clean</h1>

<p align="center">
  <em>Clean your keyboard and mouse without messing up your Windows.</em>
</p>

Call-In Clean is a tiny Windows tray utility that temporarily blocks keyboard and/or mouse input, so you can wipe your peripherals without typing gibberish, clicking things, or closing windows by accident. It is inspired by Apple's Keyboard Cleaning Mode.

> **Status:** early development. It works, but it has not been tested on a wide range of hardware yet. See [Known limitations](#known-limitations).

## Features

- **Three lock modes:** keyboard only, mouse only, or both.
- **Auto-unlock:** input always comes back when the timer ends (5 minutes by default).
- **Early unlock:** hold a gesture for 5 seconds to end the lock (see below).
- **Fullscreen overlay** with a countdown, kept on top and covering every monitor.
- **Tray app:** lives in the system tray, started from a menu.
- **Optional "Start with Windows"**, off by default.
- **Lightweight:** one small native `.exe`, no runtime, no installer, no admin rights.

## Using it

1. Run `CallInClean.exe`. A tray icon appears (on Windows 11 it may be under the `^` overflow arrow).
2. Click the icon and choose **Lock keyboard**, **Lock mouse**, or **Lock keyboard + mouse**.
3. Clean your hardware. The overlay shows the time left.
4. The lock ends by itself when the countdown reaches zero, or earlier with the unlock gesture.

### Unlock gestures

Each gesture uses a device that is *not* locked (or, in full lock, a deliberate chord), and must be held for **5 seconds**. Releasing early restarts the timer.

| Mode | Locked | Unlock gesture (hold 5 s) |
|---|---|---|
| Keyboard | keyboard | Right mouse button |
| Mouse | mouse | `Esc` + `Alt` |
| Both | keyboard + mouse | `Esc` + `Alt` + Right mouse button |

If anything ever goes wrong, `Ctrl+Alt+Del` is never blocked by this app, and ending `CallInClean.exe` in Task Manager restores input immediately.

## How it works

Call-In Clean uses the standard Windows low-level input hooks (`WH_KEYBOARD_LL` and `WH_MOUSE_LL`) from user mode. No driver is installed and no administrator rights are needed.

- Hooks are installed **only while a lock is active**. When idle, the app is just a tray icon.
- The hook callbacks check the deadline themselves, so a lock cannot outlive its timer even if the rest of the app misbehaves.
- If the process crashes or is killed, Windows removes its hooks automatically and input returns.
- **No keystrokes are logged, stored, or sent anywhere.** The hooks only decide whether to pass an event on or discard it, and watch for the unlock gesture. The source is short, so you can verify this yourself.

## Known limitations

A user-mode hook cannot block everything. These are not blocked, by design or by Windows:

- `Ctrl+Alt+Del`, `Win+L`, and anything on the secure desktop (UAC prompts, the lock screen).
- The power and sleep buttons, and some laptop `Fn` keys handled by firmware.
- Some precision-touchpad gestures (for example, three-finger swipes) and touchscreen input.
- Input to windows running as administrator may not be blocked, because Windows isolates elevated windows from non-elevated hooks.
- Exclusive-fullscreen games may draw above the overlay.

On a laptop touchpad you usually can't press left and right buttons at once, which is why the single-mouse gestures use only the right button.

### Antivirus and SmartScreen

Because the app installs keyboard hooks, some antivirus tools or Windows SmartScreen may flag an unsigned build. This is a false positive caused by how hooks work, but you should not take that on trust: the full source is in this repository, and you are welcome to build it yourself.

## Building

Requirements: a Windows C++ toolchain with `g++`, `mingw32-make` (or `make`), and `windres` (all included with MinGW / MinGW-w64).

```
git clone https://github.com/AddisionS/callin-clean.git
cd callin-clean
mingw32-make
```

This produces `CallInClean.exe` in the project root. `mingw32-make clean` removes build output.

### Project layout

```
assets/        icon files
src/main.cpp   tray icon, menu, and app lifecycle
src/hook.cpp   input hooks, lock timer, and unlock gestures
src/overlay.cpp  fullscreen countdown window
src/autostart.cpp  optional "Start with Windows" (per-user registry key)
src/resources.rc   embeds the icon in the executable
Makefile
```

## Roadmap

- Release held keys and buttons cleanly when a lock starts, to avoid stuck modifiers.
- End the lock cleanly on session lock, sleep, and resume.
- Hardware test matrix (laptops, desktops, touchpads, multi-monitor, elevated windows).
- On-screen "hold to unlock" button.
- Configurable timeout and signed releases.

## License

Released under the [MIT License](LICENSE).

## Download 

See the [latest release](https://github.com/AddisionS/callin-clean/releases/latest).
