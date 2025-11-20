#include<linux/module.h>
#include<linux/init.h>
#include<linux/pci.h>
#include<linux/io.h>
#include<linux/bitops.h>


#define PCI_MYDRIVER_VENDOR_ID 0x8086
#define PCI_MYDRIVER_DEVICE_ID 0x5182
#define PCI_MYDRIVER_OFFSET_IO_STATE 0xFC
#define PCI_MYDRIVER_OFFSET_DIRECTION 0xF8

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AFREEN");
MODULE_DESCRIPTION("Driver for the PCI gpio card");

static struct pci_device_id pci_mydriver_ids[]={
	{ PCI_DEVICE(PCI_MYDRIVER_VENDOR_ID,PCI_MYDRIVER_DEVICE_ID) },
	{ }
};
MODULE_DEVICE_TABLE(pci,pci_mydriver_ids);

/**
 * @brief fun is called,when a pci device is registerd
 *
 * @param dev    Pointer to the PCI device
 * @param id     Pointer to the corresponding id table's entry
 *
 * return        0 on success
 *               negative error is failure 
 */

static int pci_mydriver_probe(struct pci_dev *dev,const struct pci_device_id *id){
        int status;
	void __iomem *ptr_bar0;

	printk("pci_mydriver - now i am in the probe function\n");
	
	status= pci_resource_len(dev, 0); // find the len of the bar0
    
	    	printk("pci_mydriver - Size of BAR0 is %d bytes\n", status);
        
        	if(status != 256) {
	 
			printk("pci_mydriver -Error BAR0 has wrong size\n");
       
		    	return -1;
	}
 
	 	printk("pci_mydriver - BAR0 is mapped to 0x%llx\n",pci_resource_start(dev,0));
	
		 status =pcim_enable_device(dev);
	
		 if(status<0) {
	
		 	 printk("pci_mydriver - Error could not enable the device\n");
		
			 return status;
	}


	 	 status=pcim_iomap_regions(dev,BIT(0),KBUILD_MODNAME);

	 	 if(status<0) {
	
		 	 printk("pci_mydriver - Error BAR0 is already in use\n");
		
			 return status;
	}
	

	 	 ptr_bar0=pcim_iomap_table(dev)[0];

	 	 if(ptr_bar0==NULL) {
	
		 	 printk("pci_mydriver - Error Inavlid pointer for BAR0\n");
		
			 return -1;
	}


	 	 printk("pci_mydriver -GPIO state Dword: 0x%x\n",ioread32(ptr_bar0 + PCI_MYDRIVER_OFFSET_IO_STATE));
	 	 iowrite8(0x1, ptr_bar0 + PCI_MYDRIVER_OFFSET_DIRECTION);

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



