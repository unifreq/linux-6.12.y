/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __LINUX_USB_PCI_QUIRKS_MESON_H
#define __LINUX_USB_PCI_QUIRKS_MESON_H
#if 0
#ifdef CONFIG_USB_PCI
void aml_uhci_reset_hc(struct pci_dev *pdev, unsigned long base);
int aml_uhci_check_and_reset_hc(struct pci_dev *pdev, unsigned long base);
int aml_usb_hcd_amd_remote_wakeup_quirk(struct pci_dev *pdev);
bool aml_usb_amd_hang_symptom_quirk(void);
bool aml_usb_amd_prefetch_quirk(void);
void aml_usb_amd_dev_put(void);
bool aml_usb_amd_quirk_pll_check(void);
void aml_usb_amd_quirk_pll_disable(void);
void aml_usb_amd_quirk_pll_enable(void);
void aml_usb_asmedia_modifyflowcontrol(struct pci_dev *pdev);
void aml_usb_enable_intel_xhci_ports(struct pci_dev *xhci_pdev);
void aml_usb_disable_xhci_ports(struct pci_dev *xhci_pdev);
void aml_sb800_prefetch(struct device *dev, int on);
bool aml_usb_amd_pt_check_port(struct device *device, int port);
#else
#endif
#endif
struct pci_dev;
static inline void aml_usb_amd_quirk_pll_disable(void) {}
static inline void aml_usb_amd_quirk_pll_enable(void) {}
static inline void aml_usb_asmedia_modifyflowcontrol(struct pci_dev *pdev) {}
static inline void aml_usb_amd_dev_put(void) {}
static inline void aml_usb_disable_xhci_ports(struct pci_dev *xhci_pdev) {}
static inline void aml_sb800_prefetch(struct device *dev, int on) {}
static inline bool aml_usb_amd_pt_check_port(struct device *device, int port)
{
	return false;
}
//#endif  /* CONFIG_USB_PCI */

#endif  /*  __LINUX_USB_PCI_QUIRKS_H  */
