all: run-debug

ifdef DEBUG
  PROFILE = debug
else
  PROFILE = release
endif

compile:
	../vintage/$(PROFILE)/out/vintage-asm.exe test.viasm test.bin test.dbg
clean:
	rm test.bin test.dbg
run-debug: compile
	(cd ../vintage/$(PROFILE)/out; ./vintage-vm ../../../TestVintage/test.bin ../../../TestVintage/test.dbg)
run-no-debug: compile
	(cd ../vintage/$(PROFILE)/out; ./vintage-vm ../../../TestVintage/test.bin)
