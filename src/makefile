sharelib=stockshare 
plugins=realinfo

all: $(sharelib) market_data $(plugins)
market_data:
	cd ./plugins/$@ && $(MAKE) clean && $(MAKE) 
$(sharelib):
	cd ./plugins/$@ && $(MAKE) clean && $(MAKE)
$(plugins):
	@echo $@
	@echo `pwd`
	cd ./plugins/$@ && $(MAKE)  clean && $(MAKE)
