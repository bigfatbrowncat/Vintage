################### Main part ###################

CC = gcc $(COMMON_CFLAGS) $(CFLAGS) $(CDEFINES)
CPP = g++ $(COMMON_CPPFLAGS) $(CPPFLAGS) $(CPPDEFINES)
COMMON_CPPFLAGS += $(COMMON_CFLAGS)
CPPFLAGS += $(CFLAGS)
CPPDEFINES += $(CDEFINES)

OSTYPE := $(shell uname)

TARGET_BIN = $(TARGET)/bin
TARGET_OBJ = $(TARGET)/obj

EXECUTABLE = $(TARGETNAME).exe
TESTER = $(TESTERNAME).exe

OBJECT_FILES_WITH_PATH = $(addprefix $(TARGET_OBJ)/,$(addsuffix .o,$(OBJECTS)))
TARGET_OBJECT_FILES_WITH_PATH = $(addprefix $(TARGET_OBJ)/,$(addsuffix .o,$(TARGET_OBJECTS)))
TESTER_OBJECT_FILES_WITH_PATH = $(addprefix $(TARGET_OBJ)/,$(addsuffix .o,$(TESTER_OBJECTS)))

SOURCE_HEADERS_WITH_PATH = $(addprefix $(SOURCE)/,$(shell cd $(SOURCE); find . -name \*.h))

all: executable tester tests external_bin

clean:
	@echo "[$(PROJ)] Removing target executable..."
	rm -f $(TARGET_BIN)/$(EXECUTABLE)

	@echo "[$(PROJ)] Removing tester executable..."
	rm -f $(TARGET_BIN)/$(TESTER)

	@echo "[$(PROJ)] Removing object files..."
	rm -f $(OBJECT_FILES_WITH_PATH)
	rm -f $(TESTER_OBJECT_FILES_WITH_PATH)
	rm -f $(TARGET_OBJECT_FILES_WITH_PATH)

	@echo "[$(PROJ)] Removing empty directories..."
	find $(TARGET) -depth -empty -type d -exec rmdir {} \;

executable: $(TARGET_BIN)/$(EXECUTABLE)
tester: $(TARGET_BIN)/$(TESTER)

################### Folders ###################

ENSURE_BIN = if [ ! -d "$(TARGET_BIN)" ]; then mkdir -p "$(TARGET_BIN)"; fi
ENSURE_OBJ = if [ ! -d "$(TARGET_OBJ)" ]; then mkdir -p "$(TARGET_OBJ)"; fi

################### Objects ###################

$(OBJECT_FILES_WITH_PATH) : $(TARGET_OBJ)/%.o : $(SOURCE)/%.cpp $(HEADERS_WITH_PATH) $(SOURCE_HEADERS_WITH_PATH)
	@echo "[$(PROJ)] Compiling $@ ..."
	$(ENSURE_OBJ)
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi
	$(CPP) -c $< -o $@ $(INCLUDE)

$(TARGET_OBJECT_FILES_WITH_PATH) : $(TARGET_OBJ)/%.o : $(SOURCE)/%.cpp $(HEADERS_WITH_PATH) $(SOURCE_HEADERS_WITH_PATH)
	@echo "[$(PROJ)] Compiling $@ ..."
	$(ENSURE_OBJ)
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi
	$(CPP) -c $< -o $@ $(INCLUDE)

$(TESTER_OBJECT_FILES_WITH_PATH) : $(TARGET_OBJ)/%.o : $(SOURCE)/%.cpp $(HEADERS_WITH_PATH) $(SOURCE_HEADERS_WITH_PATH)
	@echo "[$(PROJ)] Compiling $@ ..."
	$(ENSURE_OBJ)
	if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi
	$(CPP) -c $< -o $@ $(INCLUDE)

################### Targets ###################

$(TARGET_BIN)/$(EXECUTABLE): $(OBJECT_FILES_WITH_PATH) $(TARGET_OBJECT_FILES_WITH_PATH) 
	@echo "[$(PROJ)] Building executable $@ ..."
	$(ENSURE_BIN)
	$(CPP) $(OBJECT_FILES_WITH_PATH) $(TARGET_OBJECT_FILES_WITH_PATH) $(LIBS) -o $@

$(TARGET_BIN)/$(TESTER): $(OBJECT_FILES_WITH_PATH) $(TESTER_OBJECT_FILES_WITH_PATH) 
	@echo "[$(PROJ)] Building executable $@ ..."
	$(ENSURE_BIN)
	$(CPP) $(OBJECT_FILES_WITH_PATH) $(TESTER_OBJECT_FILES_WITH_PATH) $(LIBS) -o $@

################ Running tests ################

tests: tester
	$(TARGET_BIN)/$(TESTER)

external_bin:
	@echo "[$(PROJ)] Copying external binaries to target folder ..."
	cp -f $(EXTERNAL_BIN)/* $(TARGET_BIN)/

.PHONY: all executable external_bin tester tests clean
.SILENT: