#include<linux/module.h>
#include<linux/init.h>
#include<linux/pci.h>

#define PCI_MYDRIVER_VENDOR_ID 0x8086
#define PCI_MYDRIVER_DEVICE_ID 0x5182

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AFREEN");
MODULE_DESCRIPTION("Driver for the PCI gpio card");

static struct pci_device_id pci_mydriver_ids[]={
	{ PCI_DEVICE(PCI_MYDRIVER_VENDOR_ID,PCI_MYDRIVER_DEVICE_ID) },
	{ }
};
MODULE_DEVICE_TABLE(pci,pci_mydriver_ids);

/**
 * @bried fun is called,when a pci device is registerd
 *
 * @param dev    Pointer to the PCI device
 * @param id     Pointer to the corresponding id table's entry
 *
 * return        0 on success
 *               negative error is failure 
 */

static int pci_mydriver_probe(struct pci_dev *Dev,const struct pci_device_id *id){
	printk("pci_mydriver - now i am in the probe function\n");
	return 0;
}

/**
 * @brief fun is called ,when a PCI device is unregisterd
 *
 * #param dev  pointer to the PCI device
 */

static void pci_mydriver_remove(struct pci_dev *dev) {
	printk("pci_mydriver- now i am in remove function\n");
}

/*PCI DRIVER STRUCTURE*/

static struct pci_driver pci_mydriver_driver={
	.name="pci_mydriver",
	.id_table=pci_mydriver_ids,
	.probe=pci_mydriver_probe,
	.remove=pci_mydriver_remove,
};

static int __init my_init(void){
	printk("pci_mydriver-registering the pci device\n");
	return pci_register_driver(&pci_mydriver_driver);
}

static void __exit my_exit(void) {
	printk("pci_mydriver-unregistering the pci device\n");
	pci_unregister_driver(&pci_mydriver_driver);
}

module_init(my_init);
module_exit(my_exit);




