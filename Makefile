all: build-all

CMP_INPATH = ./tools/aas/src
CMP_FLAGS = -o:speed -out:./tools/aas/bin/aas.exe

AAS_INPATH = ./tools/aas/test/test.aphel
AAS_FLAGS = -out:tools/aas/test/out.amg -debug

build-all:
	@odin build $(CMP_INPATH) $(CMP_FLAGS)

test-aas: build-all
	@./tools/aas/bin/aas.exe $(AAS_INPATH) $(AAS_FLAGS)

stresstest-aas: build-all
	@./tools/aas/bin/aas.exe ./tools/aas/test/twomil.aphel $(AAS_FLAGS)

# odin build ./tools/aas/src -o:speed -out:./tools/aas/bin/aas.exe; ./tools/aas/bin/aas.exe ./tools/aas/test/test.aphel -debug -out:tools/aas/test/out.amg
