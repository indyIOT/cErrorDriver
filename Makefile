ifeq ($(OS),Windows_NT)
	RM_DIR = rmdir /s /q
	MKDIR  = mkdir
else
	RM_DIR = rm -rf
	MKDIR  = mkdir -p
endif

prepare:
	-$(RM_DIR) build
	$(MKDIR) build
