#include <stdio.h>
#include <pci/pci.h>

int main() {
    struct pci_access *pacc;
    struct pci_dev *dev;
    char namebuf[1024], classbuf[1024];

    // Initialize PCI access structure
    pacc = pci_alloc();
    pci_init(pacc);
    pci_scan_bus(pacc);

    // Loop through devices
    for (dev = pacc->devices; dev; dev = dev->next) {
        pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS);

        printf("Device %02x:%02x.%d: %04x:%04x\n",
               dev->bus, dev->dev, dev->func,
               dev->vendor_id, dev->device_id);

        printf("  Class: %s\n",
               pci_lookup_name(pacc, classbuf, sizeof(classbuf),
                               PCI_LOOKUP_CLASS, dev->device_class));

        printf("  Vendor:Device = %s\n",
               pci_lookup_name(pacc, namebuf, sizeof(namebuf),
                               PCI_LOOKUP_VENDOR | PCI_LOOKUP_DEVICE,
                               dev->vendor_id, dev->device_id));
    }

    pci_cleanup(pacc); // Free memory
    return 0;
}

