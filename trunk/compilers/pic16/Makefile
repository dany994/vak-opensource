SUBDIRS = as cc lib include

all:
	@for subdir in $(SUBDIRS); do (cd $$subdir; \
		echo "=== $$subdir ==="; $(MAKE) all); done

install:
	@for subdir in $(SUBDIRS); do (cd $$subdir; \
		echo "=== $$subdir ==="; $(MAKE) install); done

clean:
	for subdir in $(SUBDIRS); do (cd $$subdir; $(MAKE) clean); done
	rm -f *.b
