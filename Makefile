all: build

AAS_BUILD_INPATH = ./tools/aas/src
AAS_BUILD_FLAGS = -o:speed -out:./tools/aas/bin/aas.exe

AAS_STRESSTEST_INPATH = ./tools/aas/test/twomil.aphel
AAS_TEST_INPATH = ./tools/aas/test/test.aphel
AAS_TEST_FLAGS = -out:tools/aas/test/out.amg -debug -no-color # apparently make has trouble displaying ansi codes

build:
	@odin build $(AAS_BUILD_INPATH) $(AAS_BUILD_FLAGS)

test-aas: build
	@./tools/aas/bin/aas.exe $(AAS_TEST_INPATH) $(AAS_TEST_FLAGS)

stresstest-aas: build
	@./tools/aas/bin/aas.exe $(AAS_STRESSTEST_INPATH) $(AAS_TEST_FLAGS)

# odin build ./tools/aas/src -o:speed -out:./tools/aas/bin/aas.exe; ./tools/aas/bin/aas.exe ./tools/aas/test/test.aphel -debug -out:tools/aas/test/out.amg
