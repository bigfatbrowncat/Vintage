all:
	../vintage/release/out/vintage-asm.exe test.viasm test.bin test.dbg
clean:
	rm test.bin test.dbg
run: all
	(cd ../vintage/release/out; ./vintage-vm ../../../TestVintage/test.bin ../../../TestVintage/test.dbg)
run-no-debug:
	(cd ../vintage/release/out; ./vintage-vm ../../../TestVintage/test.bin)
