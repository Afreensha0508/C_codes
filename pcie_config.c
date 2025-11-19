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

static int pci_mydriver_probe(struct pci_dev *dev,const struct pci_device_id *id){
	u16 vid, did;
	u8 capability_ptr;
	u32 bar0, saved_bar0;

	printk("pci_mydriver - now i am in the probe function\n");
	
	/*Let's read the PCIe VID & DIF */

	if(0 !=pci_read_config_word(dev,0x0,&vid)) {
		printk("pci_mydriver-Error reading from config space\n");
		return -1;
	}

	printk("pci_mydriver-VID: 0x%x\n", vid);

	if(0 !=pci_read_config_word(dev,0x2,&did)) {
		printk("pci_mydriver- Error reading from config space\n");
		return -1;
	}
	printk("pci_mydriver-DID:0x%x\n",did);

	/* read the pci capability pointer */

	printk("pci_mydriver - VID : 0x%x\n", vid);
	if(0 != pci_read_config_byte(dev,0x34,&capability_ptr)) {
		printk("pci_mydriver - Error reading from config space\n");
		return -1;
	}

	if(capability_ptr)
		printk("pci_mydriver - PCI card has capability\n");
	else
		printk("pci_mydriver - PCI card doesn't have capabilities\n");

	if(0 != pci_read_config_dword(dev,0x10,&bar0)) {
		printk("pci_mydriver - Error reading from config space\n");
		return -1;
	}

	saved_bar0=bar0;
        
	if(0 !=pci_write_config_dword(dev,0x10,0xffffffff)) {
		printk("pci_mydriver - error writing to config space \n");
		return -1;
	}

	if(0 !=pci_read_config_dword(dev,0x10,&bar0)) {
		printk("pci_mydriver - Error reading from config space\n");
		return -1;
	}

	if((bar0 & 0x3) ==1)
		printk("pci_mydriver -BAR0 is IO space\n");
	else
		printk("pci_mydriver - BAR0 is memory space\n");

	bar0 &= 0xFFFFFFFD;
	bar0 = ~bar0;
	bar0++;

	printk("pci_mydriver - BAR0 is %d bytes big\n");

	if(0 !=pci_write_config_dword(dev,0x10,saved_bar0)) {
		printk("pci_mydriver - Error writing config space\n");
		return -1;

	}

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



