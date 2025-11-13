// pci_example.c
#include <linux/module.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/io.h>

#define DRIVER_NAME "pci_example"
#define MY_VENDOR_ID 0x1234   // replace with your PCI vendor ID
#define MY_DEVICE_ID 0x5678   // replace with your PCI device ID

struct pci_example_dev {
    struct pci_dev *pdev;
    void __iomem *bar0_virt;
    resource_size_t bar0_len;
    int irq;
};

/* Interrupt Service Routine (ISR) */
static irqreturn_t pci_example_isr(int irq, void *dev_id)
{
    struct pci_example_dev *edev = dev_id;
    pr_info("%s: IRQ %d triggered\n", DRIVER_NAME, irq);

    if (edev->bar0_virt) {
        u32 status = ioread32(edev->bar0_virt + 0x0);
        pr_info("%s: status register = 0x%08x\n", DRIVER_NAME, status);
        /* Normally clear the interrupt in device register */
    }
    return IRQ_HANDLED;
}

static int pci_example_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
    int err;
    struct pci_example_dev *edev;
    u32 val;

    pr_info("%s: probe start for %04x:%04x\n", DRIVER_NAME, id->vendor, id->device);

    err = pci_enable_device(pdev);
    if (err) {
        dev_err(&pdev->dev, "pci_enable_device failed: %d\n", err);
        return err;
    }

    pci_set_master(pdev);

    edev = devm_kzalloc(&pdev->dev, sizeof(*edev), GFP_KERNEL);
    if (!edev) {
        err = -ENOMEM;
        goto err_disable;
    }

    edev->pdev = pdev;
    pci_set_drvdata(pdev, edev);

    /* Request BAR0 */
    err = pci_request_region(pdev, 0, DRIVER_NAME);
    if (err) {
        dev_err(&pdev->dev, "pci_request_region failed: %d\n", err);
        goto err_disable;
    }

    edev->bar0_len = pci_resource_len(pdev, 0);
    edev->bar0_virt = pci_iomap(pdev, 0, 0);
    if (!edev->bar0_virt) {
        dev_err(&pdev->dev, "pci_iomap failed\n");
        err = -EIO;
        goto err_release_region;
    }

    edev->irq = pdev->irq;
    dev_info(&pdev->dev, "Using legacy IRQ %d\n", edev->irq);

    /* Request legacy IRQ */
    err = request_irq(edev->irq, pci_example_isr, IRQF_SHARED, DRIVER_NAME, edev);
    if (err) {
        dev_err(&pdev->dev, "request_irq failed: %d\n", err);
        goto err_unmap;
    }

    /* Example MMIO read */
    val = ioread32(edev->bar0_virt + 0x0);
    dev_info(&pdev->dev, "Initial reg0 = 0x%08x\n", val);

    dev_info(&pdev->dev, "probe successful\n");
    return 0;

err_unmap:
    pci_iounmap(pdev, edev->bar0_virt);
err_release_region:
    pci_release_region(pdev, 0);
err_disable:
    pci_disable_device(pdev);
    return err;
}

static void pci_example_remove(struct pci_dev *pdev)
{
    struct pci_example_dev *edev = pci_get_drvdata(pdev);

    pr_info("%s: remove\n", DRIVER_NAME);

    if (edev) {
        free_irq(edev->irq, edev);
        if (edev->bar0_virt)
            pci_iounmap(pdev, edev->bar0_virt);
    }

    pci_release_region(pdev, 0);
    pci_disable_device(pdev);
}

static const struct pci_device_id pci_example_id_table[] = {
    { PCI_DEVICE(MY_VENDOR_ID, MY_DEVICE_ID) },
    { 0, }
};
MODULE_DEVICE_TABLE(pci, pci_example_id_table);

static struct pci_driver pci_example_driver = {
    .name = DRIVER_NAME,
    .id_table = pci_example_id_table,
    .probe = pci_example_probe,
    .remove = pci_example_remove,
};

module_pci_driver(pci_example_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("afreen");
MODULE_DESCRIPTION("Simple PCI example driver (legacy BAR + IRQ)");

