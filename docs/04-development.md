# Development setup

This experiment deliberately uses two toolchains:

- **Java 21 / JavaFX / Maven** for the desktop simulator;
- **C11 / CMake** for the native display implementation.

On Windows it is perfectly reasonable to use **NetBeans for the JavaFX application** and **Visual Studio Code for the native C code**. Both IDEs can open the same repository; neither owns the build.

## Recommended Windows setup

## Install the Windows native toolchain

Choose **one** of the following installation methods. They install the same MSVC/C++ toolchain required by this experiment.

### Option A — Chocolatey

This is the recommended route when Chocolatey is already installed.

Open **PowerShell as Administrator**:

```powershell
choco install visualstudio2026buildtools -y
choco install visualstudio2026-workload-vctools -y
choco install cmake -y
```

The important second package is `visualstudio2026-workload-vctools`. It adds the Visual C/C++ compiler and native build workload to Visual Studio Build Tools.

You do **not** need to open Visual Studio Installer and manually select `Desktop development with C++` after using these Chocolatey commands.

Optionally install VS Code through Chocolatey as well:

```powershell
choco install vscode -y
```

### Option B — Visual Studio Installer

Use this only when you prefer the graphical installer instead of Chocolatey.

Install **Visual Studio Build Tools 2026** and select the workload:

```text
Desktop development with C++
```

Make sure the workload includes:

- the MSVC C/C++ compiler;
- a Windows SDK;
- CMake tools for Windows.

The full Visual Studio IDE is not required.

If `cmake` is still unavailable afterwards, install Kitware CMake separately and add it to `PATH`.

### Verify

Close and reopen PowerShell and VS Code after installation.

```powershell
cmake --version
```

A normal PowerShell window does not necessarily expose `cl.exe` directly. That is fine: VS Code CMake Tools can activate the installed Visual Studio developer environment automatically.

For direct manual `cl` use, open a Visual Studio Developer PowerShell / Developer Command Prompt.

## VS Code workflow

Open the repository root in VS Code.

CMake Tools should detect `native/CMakeLists.txt`. If it asks for a kit/compiler, select the installed Visual Studio/MSVC toolchain.

The equivalent terminal workflow is:

```powershell
cmake -S native -B native/build
cmake --build native/build --config Release
ctest --test-dir native/build -C Release --output-on-failure
```

Expected Windows executable:

```text
native/build/Release/display-host.exe
```

Depending on the selected CMake generator, a single-configuration build may instead place it at:

```text
native/build/display-host.exe
```

The Java simulator accepts both locations.

## Run the complete simulator

After the native build is green, start the JavaFX application from NetBeans with **Run Project**.

The configured main class is `SimulatedDisplayApplication`. This is deliberately a **plain Java launcher** which calls `Application.launch(SimulatedDisplayFxApplication.class, args)`. Do not configure NetBeans to run the JavaFX `Application` subclass directly: doing so through Maven `exec` can produce:

```text
Error: JavaFX runtime components are missing, and are required to run this application
```

If NetBeans previously remembered a different main class, use **Clean and Build**, reload the Maven project, and select `io.github.brainboxemb.experimental.display.SimulatedDisplayApplication` as the main class.

Or from a terminal:

```powershell
mvn javafx:run
```

The helper script performs the native build/test first and then starts JavaFX:

```powershell
.\run.ps1
```

The startup path is therefore:

```text
CMake
  -> display-host.exe
       |
       | binary process pipe
       v
Java 21 / JavaFX
  -> simulated 90x35 red LED display
```

## Suggested IDE split

A practical setup for this experiment is:

| Work | IDE |
| --- | --- |
| JavaFX UI, Maven, Java debugging | NetBeans |
| C parser, renderer, CMake, C debugging | VS Code |
| Repository / Git | either |
| End-to-end run | NetBeans or `run.ps1` |

Using two IDEs does not create two project formats. Maven and CMake remain the actual build authorities.

## Troubleshooting

### `cmake` is not recognized

CMake is not installed or is not on `PATH`. Install CMake support through Visual Studio Build Tools or install Kitware CMake separately, then open a new terminal.

### VS Code reports that no compiler is available

Installing the C/C++ extension is not enough. Install Visual Studio Build Tools with **Desktop development with C++**, then use **CMake: Select a Kit** / the compiler selection offered by CMake Tools.

### `cl` is not recognized, but VS Code builds successfully

That can be normal. CMake Tools can activate the Visual Studio developer environment internally. Use a Developer PowerShell if you want to call `cl` manually.

### NetBeans prints `Unsupported JavaFX configuration`

When NetBeans starts the plain launcher through Maven `exec`, JavaFX may print:

```text
WARNING: Unsupported JavaFX configuration: classes were loaded from 'unnamed module ...'
```

That warning is caused by the classpath-based NetBeans/Maven launch and is not the reason for an application failure. The experiment currently accepts this warning for the simple non-modular PoP setup.

A failure such as `Unable to construct Application instance` is different and must not be ignored; the JavaFX application class must be public and expose a public no-argument constructor. This contract is covered by an automated test.

### JavaFX says that `display-host` is missing

Build the native project first:

```powershell
cmake -S native -B native/build
cmake --build native/build --config Release
```

Or specify the native executable explicitly:

```powershell
mvn -Ddisplay.native="C:\path\to\display-host.exe" javafx:run
```

## Alternative compilers

Clang or MinGW can also compile portable C11 code, but they are not needed for the initial Windows experiment. MSVC is the recommended baseline because it integrates directly with Windows, CMake Tools and GitHub's Windows qualification runner.

ESP32 development will later use the ESP-IDF toolchain separately; that toolchain is not required for the current 90x35 desktop simulator.
