SYSLIBPATH = /mingw/lib
SYSBINPATH = /mingw/bin
PTHREAD_BIN = $(shell echo `dlltool -I $(SYSLIBPATH)/libpthread.a`)

SDL_INCLUDE_PATH =		../../SDL-1.2.14/include/SDL
SDL_LIBS_PATH =			../../SDL-1.2.14/lib
SDL_BIN_PATH =			../../SDL-1.2.14/bin
SDL_DLL	= 				SDL.dll

GLEW_INCLUDE_PATH =		../../glew-1.7.0/include
GLEW_LIBS_PATH =		../../glew-1.7.0/lib
GLEW_DLL = 				glew32.dll 
GLEW_MX_DLL =			glew32mx.dll

SRC_PATH = 				src

ifdef DEBUG
	CXXFLAGS =			-O0 -g2 -Weffc++ -Wall -Wno-write-strings -fmessage-length=0 -static-libgcc -static-libstdc++ -DDEBUG
	
	OUT_PATH = 			debug/out
	OBJ_PATH =			debug/obj
else
	CXXFLAGS =			-O3 -g0 -Wall -Wno-write-strings -fmessage-length=0 -static-libgcc -static-libstdc++
	
	OUT_PATH = 			release/out
	OBJ_PATH =			release/obj
endif


VM_OBJS =			$(OBJ_PATH)/SDLTerminal.o $(OBJ_PATH)/SDLScreen.o $(OBJ_PATH)/vm_main.o \
					$(OBJ_PATH)/HardwareDevice.o \
					$(OBJ_PATH)/HardwareTimer.o \
					$(OBJ_PATH)/CPUKeyboardController.o \
					$(OBJ_PATH)/DebuggerKeyboardController.o \
					$(OBJ_PATH)/CPU.o $(OBJ_PATH)/Console.o $(OBJ_PATH)/globals.o $(OBJ_PATH)/Debugger.o
					
ASM_OBJS =			$(OBJ_PATH)/asm.o $(OBJ_PATH)/asm_main.o \
					$(OBJ_PATH)/globals.o
					
VM_HEADERS = 		$(SRC_PATH)/SDLTerminal.h $(SRC_PATH)/SDLScreen.h $(SRC_PATH)/MessageContext.h \
					$(SRC_PATH)/../../FontEditor/include/Font.h \
					$(SRC_PATH)/HardwareDevice.h \
					$(SRC_PATH)/CPU.h $(SRC_PATH)/instructions.h \
					$(SRC_PATH)/chain.h $(SRC_PATH)/globals.h $(SRC_PATH)/Debugger.h

ASM_HEADERS = 		$(SRC_PATH)/asm.h $(SRC_PATH)/instructions.h \
					$(SRC_PATH)/chain.h $(SRC_PATH)/globals.h 
					
DEPLOYED_RESOURCES = res/curfont.txt res/font.txt res/term.frag res/term.vert
					
VM_LIBS =			-lmingw32 -lSDLmain -lSDL -lopengl32 -lglu32 -lglew32 -lglew32mx -lpthread.dll

ASM_LIBS =			-lpthread.dll

LIBS_PATH =			-L$(SDL_LIBS_PATH) -L$(GLEW_LIBS_PATH)
INCLUDES_PATH = 	-I$(SDL_INCLUDE_PATH) -I$(GLEW_INCLUDE_PATH)

VM_TARGET =			vintage-vm
ASM_TARGET =		vintage-asm

all: vm-target asm-target resources

##### Folders #####

$(OUT_PATH):
	mkdir -p $(OUT_PATH)

$(OBJ_PATH):
	mkdir -p $(OBJ_PATH)

##### Objects #####

$(VM_OBJS) : $(OBJ_PATH)/%.o : $(SRC_PATH)/%.cpp $(VM_HEADERS)
	mkdir -p $(OBJ_PATH)
	$(CXX) $(CXXFLAGS) -c -o $@ $(INCLUDES_PATH) $(LIBS_PATH) $(VM_LIBS) $< 

$(ASM_OBJS) : $(OBJ_PATH)/%.o : $(SRC_PATH)/%.cpp $(ASM_HEADERS)
	mkdir -p $(OBJ_PATH)
	$(CXX) $(CXXFLAGS) -c -o $@ $(INCLUDES_PATH) $(LIBS_PATH) $(ASM_LIBS) $< 

##### Target #####

vm-target: $(OUT_PATH)/$(VM_TARGET) externals resources

$(OUT_PATH)/$(VM_TARGET): $(VM_OBJS) $(OUT_PATH) $(OUT_PATH)/res/vintage-vm.res
	$(CXX) $(CXXFLAGS) -mwindows $(VM_OBJS) -o $@ $(LIBS_PATH) $(VM_LIBS) $(OUT_PATH)/res/vintage-vm.res

asm-target: $(OUT_PATH)/$(ASM_TARGET)

$(OUT_PATH)/$(ASM_TARGET): $(ASM_OBJS) $(OUT_PATH)
	$(CXX) $(CXXFLAGS) $(ASM_OBJS) -o $@ $(LIBS_PATH) $(ASM_LIBS) 

##### Externals #####

externals: $(OUT_PATH)/$(SDL_DLL) $(OUT_PATH)/$(GLEW_DLL) $(OUT_PATH)/$(GLEW_MX_DLL) $(OUT_PATH)/$(PTHREAD_BIN)

$(OUT_PATH)/$(GLEW_DLL): $(GLEW_LIBS_PATH)/$(GLEW_DLL)
	cp -f $(GLEW_LIBS_PATH)/$(GLEW_DLL) $(OUT_PATH)/

$(OUT_PATH)/$(GLEW_MX_DLL): $(GLEW_LIBS_PATH)/$(GLEW_MX_DLL)
	cp -f $(GLEW_LIBS_PATH)/$(GLEW_MX_DLL) $(OUT_PATH)/

$(OUT_PATH)/$(SDL_DLL): $(SDL_BIN_PATH)/$(SDL_DLL)
	cp -f $(SDL_BIN_PATH)/$(SDL_DLL) $(OUT_PATH)/

$(OUT_PATH)/$(PTHREAD_BIN): $(SYSBINPATH)/$(PTHREAD_BIN)
	cp -f $(SYSBINPATH)/$(PTHREAD_BIN) $(OUT_PATH)/

##### Res #####

$(OUT_PATH)/res/vintage-vm.res: res-src/vintage-vm.rc
	mkdir -p $(OUT_PATH)/res
	windres res-src/vintage-vm.rc -O coff -o $(OUT_PATH)/res/vintage-vm.res

resources: $(DEPLOYED_RESOURCES)
	mkdir -p $(OUT_PATH)
	cp -rf $(DEPLOYED_RESOURCES) $(OUT_PATH)/res/

##### Cleaning #####

clean:
	rm -f $(VM_OBJS) \
	      $(ASM_OBJS) \
	      $(OUT_PATH)/$(VM_TARGET) \
	      $(OUT_PATH)/$(ASM_TARGET) \
	      $(OUT_PATH)/$(SDL_DLL) \
	      $(OUT_PATH)/$(PTHREAD_BIN); \
	rm -rf $(OUT_PATH)/res
