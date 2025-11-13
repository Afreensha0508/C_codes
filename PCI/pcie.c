// pcie_example.c
#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/errno.h>

#define DRIVER_NAME "pcie_example"
#define MY_VENDOR_ID 0x1234  // replace with your vendor/device ids
#define MY_DEVICE_ID 0x11e8

struct pcie_example_dev {
    struct pci_dev *pdev;
    void __iomem *bar0_virt;
    resource_size_t bar0_len;
    int irq;
};

static irqreturn_t pcie_example_isr(int irq, void *dev_id)
{
    struct pcie_example_dev *edev = dev_id;
    pr_info("%s: IRQ %d triggered\n", DRIVER_NAME, irq);
    /* Read device status register (example offset 0x00) */
    if (edev->bar0_virt) {
        u32 status = ioread32(edev->bar0_virt + 0x0);
        pr_info("%s: device status reg = 0x%08x\n", DRIVER_NAME, status);
        /* clear or acknowledge as device requires */
    }
    return IRQ_HANDLED;
}

static int pcie_example_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
    int err;
    struct pcie_example_dev *edev;
    u32 val;

    pr_info("%s: probe start for %04x:%04x\n", DRIVER_NAME, id->vendor, id->device);

    err = pci_enable_device_mem(pdev);
    if (err) {
        dev_err(&pdev->dev, "pci_enable_device_mem failed: %d\n", err);
        return err;
    }

    pci_set_master(pdev); /* enable bus-master for DMA */

    edev = devm_kzalloc(&pdev->dev, sizeof(*edev), GFP_KERNEL);
    if (!edev) {
        err = -ENOMEM;
        goto err_disable;
    }
    edev->pdev = pdev;
    pci_set_drvdata(pdev, edev);

    /* Request MMIO BAR0 regions */
    if (!(pci_resource_flags(pdev, 0) & IORESOURCE_MEM)) {
        dev_warn(&pdev->dev, "BAR0 is not memory mapped\n");
    }
    err = pci_request_region(pdev, 0, DRIVER_NAME);
    if (err) {
        dev_err(&pdev->dev, "pci_request_region(BAR0) failed: %d\n", err);
        goto err_disable;
    }

    edev->bar0_len = pci_resource_len(pdev, 0);
    edev->bar0_virt = pci_iomap(pdev, 0, 0); /* map full BAR */
    if (!edev->bar0_virt) {
        dev_err(&pdev->dev, "pci_iomap failed\n");
        err = -EIO;
        goto err_release_region;
    }

    /* Try to enable MSI (optional). If it fails, fall back to legacy IRQ */
    err = pci_enable_msi(pdev);
    if (err == 0) {
        dev_info(&pdev->dev, "MSI enabled\n");
    } else {
        dev_warn(&pdev->dev, "MSI not enabled (%d), using legacy IRQ\n", err);
    }

    edev->irq = pdev->irq;
    dev_info(&pdev->dev, "Using IRQ %d\n", edev->irq);

    /* request IRQ (use devm so it's freed on remove) */
    err = devm_request_irq(&pdev->dev, edev->irq, pcie_example_isr, IRQF_SHARED,
                           DRIVER_NAME, edev);
    if (err) {
        dev_err(&pdev->dev, "request_irq failed: %d\n", err);
        goto err_disable_msi;
    }

    /* Example read/write to MMIO: read register at offset 0x0 */
    val = ioread32(edev->bar0_virt + 0x0);
    dev_info(&pdev->dev, "Initial reg0 = 0x%08x\n", val);

    /* Example write: iowrite32(0x1, edev->bar0_virt + 0x4); */

    dev_info(&pdev->dev, "probe successful\n");
    return 0;

err_disable_msi:
    if (!err)
        pci_disable_msi(pdev);
    pci_iounmap(pdev, edev->bar0_virt);
err_release_region:
    pci_release_region(pdev, 0);
err_disable:
    pci_disable_device(pdev);
    return err;
}

static void pcie_example_remove(struct pci_dev *pdev)
{
    struct pcie_example_dev *edev = pci_get_drvdata(pdev);

    pr_info("%s: remove\n", DRIVER_NAME);

    /* IRQ freed automatically if devm_request_irq was used */
    /* disable MSI if enabled */
    if (pci_msi_enabled())
        pci_disable_msi(pdev);

    if (edev && edev->bar0_virt)
        pci_iounmap(pdev, edev->bar0_virt);

    pci_release_region(pdev, 0);
    pci_disable_device(pdev);
}

static const struct pci_device_id pcie_example_id_table[] = {
    { PCI_DEVICE(MY_VENDOR_ID, MY_DEVICE_ID) },
    { 0, }
};
MODULE_DEVICE_TABLE(pci, pcie_example_id_table);

static struct pci_driver pcie_example_pci_driver = {
    .name = DRIVER_NAME,
    .id_table = pcie_example_id_table,
    .probe = pcie_example_probe,
    .remove = pcie_example_remove,
};

module_pci_driver(pcie_example_pci_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("afreen");
MODULE_DESCRIPTION("Simple PCIe example driver (BAR mapping, IRQ, MSI attempt)");

