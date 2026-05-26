# Unified AI Assistant Desktop Client

A premium, secure, native C++ desktop client built with the **Qt6** framework and powered by the **Chromium** rendering engine (`QWebEngineView`). This high-performance client unifies **Google Gemini**, **OpenAI ChatGPT**, and **xAI Grok** inside a single, beautifully styled window with clean session isolation and local persistence.

---

## 🌟 Key Features

* **Sleek Minimalistic Header**: A custom-engineered title bar styled in a slim `44px` height with a premium glassmorphic border matching light and dark environments.
* **Interactive Dropdown Selector**: No messy buttons or sidebar tabs. Switch between Google Gemini, OpenAI ChatGPT, and xAI Grok by simply clicking the interactive **Assistant** title on the left side of the top bar.
* **Custom Window Controls**: Frameless window layouts include custom, highly responsive window management controls placed on the far right:
  * **Minimize (`—`)**
  * **Maximize / Restore (`⛶`)**
  * **Close (`✕`)** with a dynamic red hover visual highlight
* **Glassmorphic Dragging**: Drag and position the frameless client window dynamically on your screen by clicking and moving any blank space inside the top bar.
* **Persistent Sessions**: Powered by Qt WebEngine's cookie managers, the app forces local session caching inside isolated subfolders within `.qt_profiles/`. Once you sign in, **you stay signed in**!
* **Portability**: All cookies, cached layouts, and local storage configurations stay inside the application workspace rather than polluting system paths, allowing you to move or copy the directory without losing your logins.

---

## 📂 Project Directory Structure

The repository is structured following professional C++ directory layouts:

```
Assistant/
├── Makefile                # Build orchestration file (compiles MOC/UIC targets on-the-fly)
├── src/                    # Implementation C++ source files
│   ├── main.cpp            # App entrypoint
│   └── MainWindow.cpp      # Main window design & interaction logic
├── header/                 # Class declaration headers
│   ├── MainWindow.h        # Main window class declaration
│   └── ui_MainWindow.h     # UI layout header (compiled by UIC dynamically)
├── ui/                     # XML Designer layouts
│   └── MainWindow.ui       # Layout designer descriptor
├── resources/              # Theme stylesheets and branding assets
│   ├── icon.png            # Desktop logo icon
│   └── stylesheet.qss      # Application design system (QSS stylesheet)
└── build/                  # Dynamic build workspace (generated on compile)
    ├── main.o              # main.cpp intermediate object
    ├── MainWindow.o        # MainWindow.cpp intermediate object
    ├── moc_MainWindow.cpp  # Meta-object code (compiled by MOC dynamically)
    ├── moc_MainWindow.o    # Meta-object compiled binary
    └── Assistant           # Executable compiled target binary
```

---

## 🛠️ Build and Installation

### Dependencies
Before compiling, ensure you have a standard C++ compiler and the **Qt6 WebEngine Widgets development packages** installed on your Linux system.

On Debian/Ubuntu systems:
```bash
sudo apt update
sudo apt install build-essential qt6-base-dev qt6-webengine-dev
```

### Compiling
To compile the application, navigate to the project directory and run the `Makefile`:
```bash
# Clean previous artifacts and execute compilation
make clean && make
```
*The build will automatically create the `build/` folder, run the User Interface Compiler (UIC), run the Meta-Object Compiler (MOC), compile all binaries, and link the final executable.*

### Running the Application
Launch the unified client directly from the workspace root:
```bash
build/Assistant
```

---

## 💡 Key Architectural Details

### The `.qt_profiles/` Folder
Browser sessions require databases to cache cookies and caches. We explicitly point the browser engines to write into `.qt_profiles/gemini`, `.qt_profiles/chatgpt`, and `.qt_profiles/grok` locally. To log out or clear all cached caches, simply run:
```bash
rm -rf .qt_profiles
```

### The `moc_` (Meta-Object Compiler) Files
Qt relies on MOC to parse header files containing the `Q_OBJECT` macro to generate signal/slot C++ structures. The `moc_MainWindow.cpp` file is dynamically generated during the build step and compiles into `build/moc_MainWindow.o`. They are intermediate compiler artifacts and are automatically excluded from git tracking.
