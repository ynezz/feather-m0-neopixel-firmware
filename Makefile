PKG_VERSION ?= 1.8.3
BOSSAC_VERSION ?= 1.7.0
TOOLCHAIN_VERSION ?= 4.8.3-2014q1
CMSIS_VERSION ?= 4.5.0
CMSIS_ATMEL_VERSION ?= 1.1.0

JOBS := $(shell nproc)
TOPDIR := ${CURDIR}
PKG_URL := http://ynezz.ibawizard.net/private
PKG_SUM := 6c5b403f4ad30d7eff2f9a9f19fc255f7bb33790ff4a2365e8df77913c464e9d
PKG_TAR := arduino_packages_v$(PKG_VERSION).tar
TOOLS_DIR ?= $(TOPDIR)/toolchain-$(PKG_VERSION)
BUILD_DIR ?= build$(if $(TARGET_DEVICE),-$(shell echo $(TARGET_DEVICE) | tr '[:upper:]' '[:lower:]'),)

.PHONY: clean build

build: $(TOOLS_DIR)/.prepared config.local.cmake
	@cmake --version
	-rm -fr $(BUILD_DIR)
	mkdir $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. && make -j$(JOBS) VERBOSE=99

$(TOOLS_DIR)/.prepared: $(TOOLS_DIR)/.downloaded
	tar xf $(TOOLS_DIR)/$(PKG_TAR) -C $(TOOLS_DIR)
	tar xjf $(TOOLS_DIR)/CMSIS-$(CMSIS_VERSION).tar.bz2 -C $(TOOLS_DIR)
	tar xjf $(TOOLS_DIR)/CMSIS-Atmel-$(CMSIS_ATMEL_VERSION).tar.bz2 -C $(TOOLS_DIR)
	tar xzf $(TOOLS_DIR)/bossac-$(BOSSAC_VERSION)-x86_64-linux-gnu.tar.gz -C $(TOOLS_DIR)
	tar xzf $(TOOLS_DIR)/gcc-arm-none-eabi-$(TOOLCHAIN_VERSION)-linux64.tar.gz -C $(TOOLS_DIR)
	touch $(TOOLS_DIR)/.prepared

$(TOOLS_DIR)/.downloaded:
	[ -d $(TOOLS_DIR) ] || mkdir -p $(TOOLS_DIR)
	wget -c $(PKG_URL)/$(PKG_TAR) -O $(TOOLS_DIR)/$(PKG_TAR)
	echo "$(PKG_SUM)  $(TOOLS_DIR)/$(PKG_TAR)" | sha256sum -c
	touch $(TOOLS_DIR)/.downloaded

config.local.cmake:
	[ -f config.local.cmake ] && touch config.local.cmake || \
		echo "set(ARDUINO_PACKAGES_PATH \"$(TOOLS_DIR)\")" > config.local.cmake; \
		echo "set(ADAFRUIT_SAMD_PATH \"$(TOPDIR)/3rdparty/ArduinoCore-samd\")" >> config.local.cmake; \
		echo "set(ARDUINO_TOOLS_PATH \"$(TOOLS_DIR)\")" >> config.local.cmake; \
		echo "set(ARDUINO_CMSIS_PATH \"$(TOOLS_DIR)/CMSIS\")" >> config.local.cmake; \
		echo "set(ARDUINO_CMSIS_ATMEL_PATH \"$(TOOLS_DIR)/CMSIS-Atmel\")" >> config.local.cmake; \
		echo "set(ARDUINO_BOSSAC_PATH \"$(TOOLS_DIR)/bossac-$(BOSSAC_VERSION)\")" >> config.local.cmake; \
		echo "set(ARDUINO_TOOLCHAIN_PATH \"$(TOOLS_DIR)/gcc-arm-none-eabi-$(TOOLCHAIN_VERSION)\")" >> config.local.cmake; \
		echo "# set(SERIAL_DEV \"/dev/feather_m0\")" >> config.local.cmake

clean:
	-rm -fr build*

mrproper:
	make clean
	-rm -fr $(TOOLS_DIR)
	rm config.local.cmake
