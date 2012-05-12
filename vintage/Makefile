PTHREADS_BIN_PATH =		/mingw/bin
PTHREADS_DLL =			pthreadGC2.dll

SDL_INCLUDE_PATH =		../../SDL-1.2.14/include/SDL
SDL_LIBS_PATH =			../../SDL-1.2.14/lib
SDL_BIN_PATH =			../../SDL-1.2.14/bin
SDL_DLL	= 				SDL.dll

SRC_PATH = 			src

ifdef DEBUG
	CXXFLAGS =			-O0 -g2 -Wall -Wno-write-strings -fmessage-length=0 -static-libgcc -static-libstdc++ -DDEBUG
	
	OUT_PATH = 			debug/out
	OBJ_PATH =			debug/obj
else
	CXXFLAGS =			-O3 -g0 -Wall -Wno-write-strings -fmessage-length=0 -static-libgcc -static-libstdc++
	
	OUT_PATH = 			release/out
	OBJ_PATH =			release/obj
endif


VM_OBJS =			$(OBJ_PATH)/SDLConsole.o $(OBJ_PATH)/SDLScreen.o $(OBJ_PATH)/vm_main.o \
					$(OBJ_PATH)/HardwareDevice.o \
					$(OBJ_PATH)/CPU.o $(OBJ_PATH)/globals.o $(OBJ_PATH)/Debugger.o
					
ASM_OBJS =			$(OBJ_PATH)/asm.o $(OBJ_PATH)/asm_main.o \
					$(OBJ_PATH)/globals.o
					
VM_HEADERS = 		$(SRC_PATH)/SDLConsole.h $(SRC_PATH)/SDLScreen.h \
					$(SRC_PATH)/../../FontEditor/include/Font.h \
					$(SRC_PATH)/HardwareDevice.h \
					$(SRC_PATH)/CPU.h $(SRC_PATH)/instructions.h \
					$(SRC_PATH)/chain.h $(SRC_PATH)/globals.h $(SRC_PATH)/Debugger.h

ASM_HEADERS = 		$(SRC_PATH)/asm.h $(SRC_PATH)/instructions.h \
					$(SRC_PATH)/chain.h $(SRC_PATH)/globals.h 
					
VM_LIBS =			-lmingw32 -lSDLmain -lSDL -lpthread.dll

ASM_LIBS =			-lpthread.dll

LIBS_PATH =			-L$(SDL_LIBS_PATH)
INCLUDES_PATH = 	-I$(SDL_INCLUDE_PATH)

VM_TARGET =			vintage-vm
ASM_TARGET =		vintage-asm

all: vm-target asm-target resources

##### Folders #####

$(OUT_PATH):
	mkdir -p $(OUT_PATH)

$(OBJ_PATH):
	mkdir -p $(OBJ_PATH)

##### Objects #####

$(OBJ_PATH)/SDLConsole.o : $(SRC_PATH)/SDLConsole.cpp $(VM_HEADERS) $(OBJ_PATH)
	$(CXX) $(CXXFLAGS) -c -o $@ $(INCLUDES_PATH) $(LIBS_PATH) $(VM_LIBS) $(SRC_PATH)/SDLConsole.cpp 

$(OBJ_PATH)/SDLScreen.o : $(SRC_PATH)/SDLScreen.cpp $(VM_HEADERS) $(OBJ_PATH)
	$(CXX) $(CXXFLAGS) -c -o $@ $(INCLUDES_PATH) $(LIBS_PATH) $(VM_LIBS) $(SRC_PATH)/SDLScreen.cpp 

$(OBJ_PATH)/Debugger.o : $(SRC_PATH)/Debugger.cpp $(VM_HEADERS) $(OBJ_PATH)
	$(CXX) $(CXXFLAGS) -c -o $@ $(INCLUDES_PATH) $(LIBS_PATH) $(VM_LIBS) $(SRC_PATH)/Debugger.cpp 

$(OBJ_PATH)/HardwareDevice.o : $(SRC_PATH)/HardwareDevice.cpp $(VM_HEADERS) $(OBJ_PATH)
	$(CXX) $(CXXFLAGS) -c -o $@ $(INCLUDES_PATH) $(LIBS_PATH) $(VM_LIBS) $(SRC_PATH)/HardwareDevice.cpp 

$(OBJ_PATH)/vm_main.o : $(SRC_PATH)/vm_main.cpp $(VM_HEADERS) $(OBJ_PATH)
	$(CXX) $(CXXFLAGS) -c -o $@ $(INCLUDES_PATH) $(LIBS_PATH) $(VM_LIBS) $(SRC_PATH)/vm_main.cpp 

$(OBJ_PATH)/CPU.o : $(SRC_PATH)/CPU.cpp $(VM_HEADERS) $(OBJ_PATH)
	$(CXX) $(CXXFLAGS) -c -o $@ $(INCLUDES_PATH) $(LIBS_PATH) $(VM_LIBS) $(SRC_PATH)/CPU.cpp 

$(OBJ_PATH)/globals.o : $(SRC_PATH)/globals.cpp $(VM_HEADERS) $(OBJ_PATH)
	$(CXX) $(CXXFLAGS) -c -o $@ $(INCLUDES_PATH) $(LIBS_PATH) $(VM_LIBS) $(SRC_PATH)/globals.cpp 

$(OBJ_PATH)/asm.o : $(SRC_PATH)/asm.cpp $(ASM_HEADERS) $(OBJ_PATH)
	$(CXX) $(CXXFLAGS) -c -o $@ $(INCLUDES_PATH) $(LIBS_PATH) $(ASM_LIBS) $(SRC_PATH)/asm.cpp 

$(OBJ_PATH)/asm_main.o : $(SRC_PATH)/asm_main.cpp $(ASM_HEADERS) $(OBJ_PATH)
	$(CXX) $(CXXFLAGS) -c -o $@ $(INCLUDES_PATH) $(LIBS_PATH) $(ASM_LIBS) $(SRC_PATH)/asm_main.cpp 


##### Target #####

vm-target: $(OUT_PATH)/$(VM_TARGET) externals

$(OUT_PATH)/$(VM_TARGET): $(VM_OBJS) $(OUT_PATH)
	$(CXX) $(CXXFLAGS) -mwindows $(VM_OBJS) -o $@ $(LIBS_PATH) $(VM_LIBS) 

asm-target: $(OUT_PATH)/$(ASM_TARGET)

$(OUT_PATH)/$(ASM_TARGET): $(ASM_OBJS) $(OUT_PATH)
	$(CXX) $(CXXFLAGS) $(ASM_OBJS) -o $@ $(LIBS_PATH) $(ASM_LIBS) 

##### Externals #####

externals: $(OUT_PATH)/$(SDL_DLL) $(OUT_PATH)/$(PTHREADS_DLL) 

$(OUT_PATH)/$(SDL_DLL): $(OUT_PATH) $(SDL_BIN_PATH)/$(SDL_DLL)
	cp -f $(SDL_BIN_PATH)/$(SDL_DLL) $(OUT_PATH)/

$(OUT_PATH)/$(PTHREADS_DLL): $(OUT_PATH) $(PTHREADS_BIN_PATH)/$(PTHREADS_DLL)
	cp -f $(PTHREADS_BIN_PATH)/$(PTHREADS_DLL) $(OUT_PATH)/

##### Res #####
resources: res $(OUT_PATH)
	cp -rf res $(OUT_PATH)/

##### Cleaning #####

clean:
	rm -f $(VM_OBJS) \
	      $(ASM_OBJS) \
	      $(OUT_PATH)/$(VM_TARGET) \
	      $(OUT_PATH)/$(ASM_TARGET) \
	      $(OUT_PATH)/$(SDL_DLL) \
	      $(OUT_PATH)/$(PTHREADS_DLL) \
	rm -rf $(OUT_PATH)/res
