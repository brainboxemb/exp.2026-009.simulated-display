# Development setup

This experiment deliberately uses two toolchains:

- **Java 21 / JavaFX / Maven** for the desktop simulator;
- **C11 / CMake** for the native display implementation.

On Windows it is perfectly reasonable to use **NetBeans for the JavaFX application** and **Visual Studio Code for the native C code**. Both IDEs can open the same repository; neither owns the build.

## Recommended Windows setup

## Fastest Windows installation with Chocolatey

If Chocolatey is already installed, open **PowerShell as Administrator** and install the native toolchain explicitly:

```powershell
choco install visualstudio2026buildtools -y
choco install visualstudio2026-workload-vctools -y
choco install cmake -y
```

The important second package is `visualstudio2026-workload-vctools`: the Build Tools package by itself installs only the basic build-tools product, while the VCTools workload adds the Visual C/C++ compiler and recommended native build components.

Optionally install VS Code through Chocolatey as well:

```powershell
choco install vscode -y
```

After installation, close and reopen VS Code and PowerShell. Verify:

```powershell
cmake --version
```

You do not need `cl` to work from an ordinary PowerShell prompt for VS Code/CMake Tools to use MSVC. CMake Tools can activate the Visual Studio developer environment itself.

### Java side

Install:

- JDK 21;
- Apache NetBeans.

Open the repository root as a Maven project in NetBeans. JavaFX dependencies are resolved by Maven.

Verify:

```powershell
java -version
mvn -version
```

If NetBeans uses its bundled Maven integration, a separately installed global Maven is optional for IDE use. A command-line Maven installation is useful for the terminal commands below.

### Native C side with VS Code

Install Visual Studio Code and add these Microsoft extensions:

- **C/C++**;
- **CMake Tools**.

The extensions provide editing, code navigation, configuration and build integration. They do **not** by themselves provide a C compiler.

### C compiler and CMake

The recommended Windows compiler is **MSVC**, installed through **Visual Studio Build Tools**. The Chocolatey commands above are the recommended installation route for this experiment. The Visual Studio Installer route below is the equivalent manual alternative.

In the Visual Studio Installer, select the workload:

```text
Desktop development with C++
```

Make sure the workload includes:

- the MSVC C/C++ compiler;
- a Windows SDK;
- CMake tools for Windows.

This is enough to compile this experiment; the full Visual Studio IDE is not required.

If `cmake` is not available on the command line after installing the Build Tools workload, install the normal Kitware CMake package separately and make sure it is added to `PATH`.

Verify the native tools in a new terminal:

```powershell
cmake --version
cl
```

For `cl`, a normal PowerShell window may not contain the Visual Studio compiler environment. VS Code CMake Tools normally locates the installed Visual Studio toolchain itself. For direct manual `cl` use, launch a Visual Studio Developer PowerShell / Developer Command Prompt.

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
