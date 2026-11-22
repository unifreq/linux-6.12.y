# uwe5622 Linux Driver

Linux kernel drivers for the Unisoc/Spreadtrum uwe5622 wireless connectivity chipset.

## Overview

This repository provides out-of-tree kernel drivers for the uwe5622 wireless chipset, commonly found on ARM-based single-board computers from Allwinner and Rockchip platforms (e.g., Orange Pi Zero 2, Orange Pi 3 LTS, Orange Pi 4).

The uwe5622 is a combo chipset that provides:
- **WiFi** (802.11b/g/n, 2.4 GHz)
- **Bluetooth** (BLE + Classic)
- **GNSS** (GPS/GLONASS/BeiDou/Galileo)

## Project Structure

```
uwe5622/
├── unisocwcn/      # Wireless Connectivity Network core
│   ├── boot/       # Firmware/boot loader
│   ├── gnss/       # GNSS driver components
│   ├── sdio/       # SDIO interface
│   ├── usb/        # USB interface
│   ├── pcie/       # PCIe interface
│   └── ...
├── unisocwifi/     # WiFi driver (cfg80211-based)
├── tty-sdio/       # TTY over SDIO driver (for Bluetooth)
└── reference/      # Historical patches and documentation
```

## Features

- **WiFi driver**: Full cfg80211-based implementation supporting:
  - Station and SoftAP modes
  - IBSS (Ad-hoc) with WPA2 support
  - WMM AC certification
  - NAN (Neighbor Awareness Network)
  - RTT (Round Trip Time)
  - DFS (Dynamic Frequency Selection)

- **Bluetooth**: TTY over SDIO for Bluetooth HCI communication

- **GNSS**: Support for multiple satellite navigation systems

- **Power Management**: Platform-specific power saving and sleep modes

## Status

This is community-maintained software derived from vendor BSPs. It is actively developed to support newer kernel versions as they are released.

## Kernel Compatibility

The drivers are maintained to work with modern kernel versions. Patches for specific kernel versions are available in the `reference/` directory.

Supported kernel versions include:
- Linux 6.1
- Linux 6.12
- Linux 6.18
- Linux 7.0
- Linux 7.1

## Building

### In an Armbian image

The driver is built automatically and included in images created with the Armbian
Build Framework.

### Out of tree with DKMS

A `dkms.conf` is provided, so the driver can be built against the kernel the
system is actually running and reinstalled automatically on kernel updates:

```sh
sudo apt install dkms
sudo cp -r . /usr/src/uwe5622-1.0
sudo dkms add -m uwe5622 -v 1.0
sudo dkms install -m uwe5622 -v 1.0
```

This builds and installs `uwe5622_bsp_sdio`, `sprdwl_ng` and `sprdbt_tty` under
`/lib/modules/$(uname -r)/updates/dkms/`, and runs `depmod`.

Notes:

* The default `dkms.conf` builds the **Allwinner** variant
  (`CONFIG_AW_WIFI_DEVICE_UWE5622=y`). On Rockchip boards replace it with
  `CONFIG_RK_WIFI_DEVICE_UWE5622=y` in `MAKE[0]`.
* DKMS only provides the kernel modules. The **device tree** has to describe the
  chip already (the SDIO bus and the `mmc-pwrseq` that powers/resets it, plus the
  `uwe-bsp` node when the driver is built with `CONFIG_WCN_PARSE_DTS`), and the
  **firmware** has to be in place — see below.
* Verified with DKMS 3.2.2: builds against Linux 7.2 and installs correctly
  (cross-built for arm64, so `strip` warns about the architecture; a native build
  on the board does not).


## Platform Support

Tested on:
- **Orange Pi Zero 2** (Allwinner H616)
- **Orange Pi 3 LTS** (Allwinner H6)
- **Orange Pi 4** (Rockchip RK3399)
- Similar boards with UWE5622 chipset

## Firmware

The WCN core looks for `wcnmodem.bin` in `/lib/firmware/uwe5622/` first and in
`/lib/firmware/` as a fallback: the first path comes from `UNISOC_FW_PATH_CONFIG`
in the top-level Makefile, the second is the hardcoded entry in
`unisocwcn/platform/wcn_boot.c`. Either location works, and the firmware shipped
with the boards is normally in `/lib/firmware/`:

- `wcnmodem.bin` - main firmware binary
- `nvm.bin` - NV configuration

The WiFi driver additionally reads its per-board configuration
(`wifi_2355b001_1ant.ini` and similar) from the default firmware path, so
`/lib/firmware/` is the location that covers both.

## Contributing

This is community-supported software. Contributions are certainly appreciated.

## License

GPL-2.0

## References

- Original driver source: Spreadtrum Communications Inc.
- Armbian integration: https://github.com/armbian/build
