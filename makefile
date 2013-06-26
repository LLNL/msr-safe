# Stolen from qib
VERSION=$(shell grep Version META | sed -e "s/Version:[ \t]*\(.*\)/\1/")
RELEASE=test
KERNVER=$(shell uname -r)
KERNDIR=/usr/src/kernels/$(KERNVER)

default:
	make RELEASE=$(VERSION)-$(RELEASE)$(KERNVER) -C $(KERNDIR) M=`pwd`

clean:
	make -C /usr/src/kernels/`uname -r` M=`pwd` clean
	rm -f *.tgz

dist: clean
	mkdir -p /tmp/msr-safe-$(VERSION); \
	sed -e 's/#VERSION#/'$(VERSION)'/g' msr-safe.spec.in > /tmp/msr-safe-$(VERSION)/msr-safe.spec ; \
	cp -a * /tmp/msr-safe-$(VERSION); \
	sed -e 's/#VERSION#/'$(VERSION)'/g' msr-safe.spec.in > msr-safe.spec ; \
	pushd /tmp; \
	rm -f ./msr-safe-$(VERSION)/msr-safe.spec.in ./msr-safe-$(VERSION)/cscope*; \
	tar czf msr-safe-$(VERSION).tgz ./msr-safe-$(VERSION); \
	popd; \
	mv /tmp/msr-safe-$(VERSION).tgz .

install:
	mkdir -p $(RPM_BUILD_ROOT)/lib/modules/$(KERNVER)/updates
	install msr-safe.ko $(RPM_BUILD_ROOT)/lib/modules/$(KERNVER)/updates
