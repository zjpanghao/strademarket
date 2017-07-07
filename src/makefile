sharelib=stockshare 
plugins=realinfo

all: $(sharelib)  $(plugins)
$(sharelib):
	cd ./plugins/$@ && $(MAKE) clean && $(MAKE)
$(plugins):
	@echo $@
	@echo `pwd`
	cd ./plugins/$@ && $(MAKE)  clean && $(MAKE)
