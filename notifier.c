/*
 * The KErnel ROotKIt Detector (KEROKID)
 *
 *      (c) 2014 Fraunhofer FKIE
 *
 * This file contains notifier stuff
 */

#include <linux/keyboard.h>
#include <linux/netdevice.h>
#include <net/netevent.h>
#include <linux/usb.h>
#include <linux/inetdevice.h>
#include <linux/netlink.h>

#include "kerokid.h"
#include "addressAnalysis.h"
#include "proc_file.h"


#define MAX_NOTIFIER_NAME_LENGTH 20
#define NUMBER_OF_NOTIFIERS 7

struct notifier_functions{
	char name[MAX_NOTIFIER_NAME_LENGTH];
	int(*registerFunction)(struct notifier_block*);
	int(*unregisterFunction)(struct notifier_block*);
};

int numberOfInterestingNotifiers;
struct notifier_functions *interestingNotifiers;
char notifier_proc_message[NUMBER_OF_NOTIFIERS][MAX_PROC_MESSAGE_LEN];
char *current_notifier;


struct notifier_functions init_a_notifier(char* name,int(*registerFunction)(struct notifier_block*),int(*unregisterFunction)(struct notifier_block*) ){
		struct notifier_functions n = {
				.registerFunction = registerFunction,
				.unregisterFunction = unregisterFunction
		};
		memcpy(n.name,name,MAX_NOTIFIER_NAME_LENGTH);
		return n;
};


void init_notifiers(void){
	numberOfInterestingNotifiers = 7;	// increase this number if you add a notifier
	interestingNotifiers = vmalloc(numberOfInterestingNotifiers * sizeof(struct notifier_functions));

// ----- add new notifiers here -----
	//keyboard
	interestingNotifiers[0]= init_a_notifier("keyboard",&register_keyboard_notifier,&unregister_keyboard_notifier);
	interestingNotifiers[1]= init_a_notifier("module",&register_module_notifier,&unregister_module_notifier);
	interestingNotifiers[2]= init_a_notifier("netdevice",&register_netdevice_notifier,&unregister_netdevice_notifier);
	interestingNotifiers[3]= init_a_notifier("netevent",&register_netevent_notifier,&unregister_netevent_notifier);
	interestingNotifiers[4]= init_a_notifier("inetaddress",&register_inetaddr_notifier,&unregister_inetaddr_notifier);
	interestingNotifiers[5]= init_a_notifier("netlink",&netlink_register_notifier,&netlink_unregister_notifier);
	interestingNotifiers[6]= init_a_notifier("usb",(int (*)(struct notifier_block *))&usb_register_notify,
											(int (*)(struct notifier_block *))&usb_unregister_notify);
}

int blank(struct notifier_block *nblock, unsigned long code, void *param)
{
	return NOTIFY_OK;
}

struct notifier_block nb = {
	.notifier_call = blank,
	.priority = INT_MAX
};

void notifier_init(struct notifier_functions n){
	n.registerFunction(&nb);
}

void notifier_clean(struct notifier_functions n){
	n.unregisterFunction(&nb);
}

void check_blocks(struct notifier_block *nblock, int i){
	struct notifier_block *current_block = nblock;
	int code, block_nr = 0;
	if (current_block == NULL)
		concatenate_if_not_too_long(notifier_proc_message[i], formats("proc chain is empty\n"), MAX_PROC_MESSAGE_LEN);
	while (current_block != NULL) {
		block_nr++;
#if DEBUG
		printk(KERN_INFO"KEROKID: --> current block in chain: %lx \n",(unsigned long)current_block);
#endif
		code = analyze_address((psize*)current_block->notifier_call);
		if (code == 0) {
			concatenate_if_not_too_long(notifier_proc_message[i], formats("nothing found in block %d\n", block_nr), MAX_PROC_MESSAGE_LEN);
		} else {
			concatenate_if_not_too_long(notifier_proc_message[i], formats("found jump in block %d to module:\n", block_nr), MAX_PROC_MESSAGE_LEN);
			concatenate_if_not_too_long(notifier_proc_message[i], get_unhidden_module_info(), MAX_PROC_MESSAGE_LEN);
			finds.notifiers++;
		}
		current_block = current_block->next;
	}
}

void check_notifier_subscriptions(void){
	int i;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	struct proc_dir_entry* notifier_proc_folder = create_proc_folder("notifiers", get_proc_parent());
	cat_proc_message("notifiers:\n");
#endif
	init_notifiers();
	for (i = 0; i < numberOfInterestingNotifiers; i++){
		current_notifier = interestingNotifiers[i].name;
		printk(KERN_INFO"KEROKID: -> Check %s notifier chain... \n", current_notifier);
		notifier_init(interestingNotifiers[i]);
		check_blocks(nb.next, i);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
		create_proc_file_with_data(current_notifier, notifier_proc_folder, notifier_proc_message[i]);
#endif
		notifier_clean(interestingNotifiers[i]);
	}
	if (!finds.notifiers)
			cat_proc_message("nothing found\n");
	return;
}
