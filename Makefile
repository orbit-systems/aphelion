all: build-all

CMP_INPATH = ./tools/aas/src
CMP_FLAGS = -o:size -out:./tools/aas/bin/aas.exe

AAS_INPATH = ./tools/aas/test/test.aphel
AAS_FLAGS = -debug -out:tools/aas/test/out.amg

build-all:
	@odin build $(CMP_INPATH) $(CMP_FLAGS)

test-aas: build-all
	@./tools/aas/bin/aas $(AAS_INPATH) $(AAS_FLAGS)
