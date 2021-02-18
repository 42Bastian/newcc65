# makefile for the complete archive

all	: info

info: 
	@echo use :
	@echo "'make install' to install the package"
	@echo "'make demos'   to make the demos"
	@echo "'make _src'    to zip the sources"
	@echo "'make _bin'    to zip the binaries"       

install: 
	@cd cc65 ;\
	$(MAKE) install ;\
	cd ..
	@cd ra65 ;\
	$(MAKE) install ;\
	cd ..
	@cd runtime;\
	$(MAKE) install;\
	cd ..
	@cd libsrc ;\
	$(MAKE) install ;\
	cd ..
	@cd libsrc/lynx;\
	$(MAKE) install;\
	cd ../..
demos:
	@cd examples.c65;\
	$(MAKE);\
	cd demos;\
	$(MAKE);\
	cd ../..

clean:
	@cd cc65;\
	$(MAKE) clean;\
	cd ..
	cd ra65;\
	$(MAKE) clean;\
	cd ..
	@cd libsrc;\
	$(MAKE) clean;\
	cd ..
	@cd libsrc/lynx;\
	$(MAKE) clean;\
	cd ../..
	@cd examples.c65/demos;\
	$(MAKE) clean;\
	cd ..;\
	$(MAKE) clean;\
	cd ..
	cd include;\
	rm -f *.bak;\
	cd ..

_src:
	@touch bin/dummy
	@touch lib/dummy
	@cd ..;\
	zip -r -u newcc65src newcc65/cc65/ newcc65/ra65 newcc65/libsrc \
               newcc65/examples.c65 newcc65/include newcc65/bin/dummy \
               newcc65/doc newcc65/lib/dummy;\
	zip -r -u newcc65src\
               newcc65/readme.txt newcc65/makefile newcc65/addenum.txt newcc65/changes* newcc65/install.txt \
               newcc65/runtime;\
	cd newcc65
	@rm bin/dummy lib/dummy

_bin:
	@cd ..;\
	zip -r newcc65bin newcc65/bin newcc65/lib;\
	cd newcc65
