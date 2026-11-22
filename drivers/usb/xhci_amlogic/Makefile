# ported from BSP (khadas-vims-5.15.y) single-module layout, as built-in
ccflags-y += -I$(src)
ccflags-y += -I$(srctree)/drivers/usb/host/
ccflags-y += -I$(srctree)/drivers/usb/xhci_amlogic/

obj-y += xhci-meson.o
xhci-meson-y += xhci-meson.o xhci-mem-meson.o xhci-ext-caps-meson.o
xhci-meson-y += xhci-ring-meson.o xhci-hub-meson.o
xhci-meson-y += xhci-trace-meson.o
xhci-meson-y += xhci-dbg-meson.o
xhci-meson-y += xhci-debugfs-meson.o
xhci-meson-y += xhci-plat-meson.o
