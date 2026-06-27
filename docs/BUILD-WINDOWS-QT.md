# Windows 构建（Qt 主界面 + MFC 截图）

主界面与设置使用 **Qt 6 Widgets**；全屏截图、框选、标注仍为原有 **MFC** `CCatchScreenDlg`。

## 环境

- Windows 10+
- Visual Studio 2022（含「使用 C++ 的桌面开发」与 **MFC**）
- [Qt 6.5+](https://www.qt.io/download)：**MSVC 2019 64-bit** 或 **MSVC 2022 64-bit** 套件（与 VS 位数一致，推荐 x64）

安装 Qt 后设置环境变量，例如：

```powershell
$env:CMAKE_PREFIX_PATH = "C:\Qt\6.7.3\msvc2022_64"
```

## 编译

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

输出目录：`build\Release\`（含 `Screenshot.exe` 及 `windeployqt` 复制的 Qt DLL）。

## 分发（绿色版）

将 `build\Release\` 整个文件夹打成 zip 发给用户即可，**无需安装 Qt**。若构建时未自动执行 `windeployqt`，可手动：

```powershell
& "$env:CMAKE_PREFIX_PATH\bin\windeployqt.exe" "build\Release\Screenshot.exe" --no-translations
```

Release 使用 **/MD** 与 **动态 Qt**，体积比原先单文件静态 MFC 大，但 UI 稳定、易维护。

## 旧版 MSBuild

根目录 `Screenshot.sln` 仍为纯 MFC 工程，**未链接 Qt**。日常开发请使用本 CMake 流程。
