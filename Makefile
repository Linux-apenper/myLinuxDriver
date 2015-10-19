
SUBDIRS := driver/globalfifo driverTest
.PHONY :all
all:
	@for dir in $(SUBDIRS) ;\
		do make -C $${dir} ;\
		done

.PHONY:clean
clean:
	@for dir in $(SUBDIRS);\
		do make clean -C $${dir};\
		done

.PHONY:execute
exe:
	./insmodDriver.sh
	./driverTest/pollmonitor
