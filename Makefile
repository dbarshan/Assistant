# ==========================================================================
#                  Assistant Application Makefile (Qt6 C++)
# ==========================================================================

CXX = g++

# Directories
SRC_DIR = src
HEADER_DIR = header
UI_DIR = ui
BUILD_DIR = build

# Include paths
INCLUDES = -I. -I$(HEADER_DIR) \
           -I/usr/include/x86_64-linux-gnu/qt6 \
           -I/usr/include/x86_64-linux-gnu/qt6/QtWidgets \
           -I/usr/include/x86_64-linux-gnu/qt6/QtGui \
           -I/usr/include/x86_64-linux-gnu/qt6/QtCore \
           -I/usr/include/x86_64-linux-gnu/qt6/QtWebEngineWidgets \
           -I/usr/include/x86_64-linux-gnu/qt6/QtWebEngineCore \
           -I/usr/include/x86_64-linux-gnu/qt6/QtWebChannel \
           -I/usr/include/x86_64-linux-gnu/qt6/QtNetwork \
           -I/usr/include/x86_64-linux-gnu/qt6/QtPositioning

CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -fPIC $(INCLUDES)

LIBS = -L/usr/lib/x86_64-linux-gnu \
       -lQt6WebEngineWidgets -lQt6WebEngineCore -lQt6Widgets -lQt6Gui -lQt6Core -lQt6WebChannel -lQt6Network

# Dynamically locate the Qt6 Meta-Object Compiler (MOC) and User Interface Compiler (UIC)
MOC = $(shell which moc-qt6 2>/dev/null || which moc 2>/dev/null || echo /usr/lib/qt6/libexec/moc)
UIC = $(shell which uic-qt6 2>/dev/null || which uic 2>/dev/null || echo /usr/lib/qt6/libexec/uic)

# Target executable inside build folder
TARGET = $(BUILD_DIR)/Assistant

# Object files inside build folder
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/MainWindow.o $(BUILD_DIR)/moc_MainWindow.o

all: $(BUILD_DIR) $(TARGET)

# Ensure the build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS)
	@echo "Linking application: $(TARGET)..."
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
	@echo "Build successful! Target is located at: $(TARGET)"

# Compilation rule for C++ sources in src/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADER_DIR)/ui_MainWindow.h
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run Qt UIC to create ui_MainWindow.h inside header/
$(HEADER_DIR)/ui_MainWindow.h: $(UI_DIR)/MainWindow.ui
	@echo "Running Qt User Interface Compiler (UIC) on $<..."
	mkdir -p $(HEADER_DIR)
	$(UIC) $(UI_DIR)/MainWindow.ui -o $(HEADER_DIR)/ui_MainWindow.h

# Run Qt MOC to generate moc_MainWindow.cpp inside build/
$(BUILD_DIR)/moc_MainWindow.cpp: $(HEADER_DIR)/MainWindow.h
	@echo "Running Qt Meta-Object Compiler (MOC) on $<..."
	mkdir -p $(BUILD_DIR)
	$(MOC) $(HEADER_DIR)/MainWindow.h -o $(BUILD_DIR)/moc_MainWindow.cpp

# Compile the generated moc file
$(BUILD_DIR)/moc_MainWindow.o: $(BUILD_DIR)/moc_MainWindow.cpp
	@echo "Compiling generated MOC file $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR) $(HEADER_DIR)/ui_MainWindow.h
	@echo "Clean completed!"

.PHONY: all clean
