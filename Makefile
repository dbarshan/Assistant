# ==========================================================================
#                  Assistant Application Makefile (Qt6 C++)
# ==========================================================================

CXX = g++

# Directories
SRC_DIR = src
HEADER_DIR = header
UI_DIR = ui
BUILD_DIR = build

# Define the Qt modules you need
QT_MODULES = Qt6Widgets Qt6WebEngineWidgets Qt6WebEngineCore Qt6WebChannel Qt6Network Qt6Positioning

# Include paths
INCLUDES = -I. -I$(HEADER_DIR)

# Dynamically fetch the correct include paths and compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -fPIC $(INCLUDES) $(shell pkg-config --cflags $(QT_MODULES))

# Dynamically fetch the correct linker flags
LIBS = $(shell pkg-config --libs $(QT_MODULES))

# Dynamically locate the Qt6 MOC, UIC, and RCC tools
QT_LIBEXEC = $(shell pkg-config --variable=libexecdir Qt6Core)
MOC = $(shell which moc-qt6 2>/dev/null || which moc 2>/dev/null || echo $(QT_LIBEXEC)/moc)
UIC = $(shell which uic-qt6 2>/dev/null || which uic 2>/dev/null || echo $(QT_LIBEXEC)/uic)
RCC = $(shell which rcc-qt6 2>/dev/null || which rcc 2>/dev/null || echo $(QT_LIBEXEC)/rcc)

# Target executable inside build folder
TARGET = $(BUILD_DIR)/Assistant

# Object files inside build folder
OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/MainWindow.o $(BUILD_DIR)/moc_MainWindow.o $(BUILD_DIR)/qrc_resources.o

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

# Run Qt RCC to compile resources/resources.qrc into qrc_resources.cpp inside build/
$(BUILD_DIR)/qrc_resources.cpp: resources/resources.qrc resources/icon.png resources/stylesheet.qss
	@echo "Running Qt Resource Compiler (RCC) on $<..."
	mkdir -p $(BUILD_DIR)
	$(RCC) resources/resources.qrc -o $(BUILD_DIR)/qrc_resources.cpp

# Compile the generated QRC C++ file
$(BUILD_DIR)/qrc_resources.o: $(BUILD_DIR)/qrc_resources.cpp
	@echo "Compiling generated QRC file $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR) $(HEADER_DIR)/ui_MainWindow.h
	@echo "Clean completed!"

.PHONY: all clean
