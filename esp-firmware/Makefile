

PROGRAM=upnp-control

EXTRA_CFLAGS= -I./fsdata -DLWIP_HTTPD_CGI=1 -DLWIP_HTTPD_SSI=1 -DLWIP_HTTPD_SUPPORT_POST=1

#
# Enable debugging
#
EXTRA_CFLAGS+=-DLWIP_DEBUG=1
# -DHTTPD_DEBUG=LWIP_DBG_O

EXTRA_COMPONENTS=extras/mbedtls extras/httpd

include $(ESP_OPEN_RTOS_SDK_DIR)/common.mk

html:
	@echo "Generating fsdata.."
	cd fsdata && ./makefsdata
