#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/proc_ns.h>
#include <linux/moduleparam.h>

MODULE_AUTHOR("William Liu (wliu1@mit.edu)");	
MODULE_DESCRIPTION("Disable CAP_SYS_ADMIN requirement for /proc/self/pagemap for rowhammer purposes");
MODULE_LICENSE("GPL");

#define FILE_NS_CAPABLE "file_ns_capable"
#define PAGEMAP_READ "pagemap_read"

static uint threshold = 0x200;
module_param(threshold,uint,S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(threshold, "Threshold for instruction patch pinpointing");

static uint64_t addr_of_pagemap_read;
static bool final_probe_injected = false;

static int __kprobes file_ns_capable_pre(struct kprobe *p, struct pt_regs *regs);
static void __kprobes file_ns_capable_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags);
static int __kprobes target_pre(struct kprobe *p, struct pt_regs *regs);
static void __kprobes target_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags);

static struct kprobe rh_kp = {
    .symbol_name    = FILE_NS_CAPABLE,
    .pre_handler = file_ns_capable_pre,
    .post_handler = file_ns_capable_post
};

static struct kprobe target_kp = {
    .pre_handler = target_pre,
    .post_handler = target_post,
};

static int __kprobes target_pre(struct kprobe *p, struct pt_regs *regs) { return 0; }

static void __kprobes target_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags) 
{
    // pr_info("bypassing CAP_SYS_ADMIN check for pagemap_read");
    regs->ax = 1;
}

static int __kprobes file_ns_capable_pre(struct kprobe *p, struct pt_regs *regs)
{
    uint64_t ret_addr = *(uint64_t *)regs->sp;
    if (!final_probe_injected)
    {
        if (ret_addr > addr_of_pagemap_read && ret_addr < addr_of_pagemap_read + threshold) {
            pr_info("potential hit near addr of pagemap_read: %llx", ret_addr);
            // extra heuristics to basically confirm it for us
            // 0x15 == CAP_SYS_ADMIN
            if ((uint32_t)regs->dx == 0x15) {
                if ((uint64_t)regs->si == (uint64_t)&init_user_ns) {
                    pr_info("found highly highly likely candidate: %llx", ret_addr);
                    target_kp.addr = (kprobe_opcode_t *)ret_addr;
                    if (register_kprobe(&target_kp) < 0) {
                        pr_warn("failed target kprobe registration");
                    } else {
                        final_probe_injected = true;
                    }
                }
            }
        }
    }
    return 0;
}

static void __kprobes file_ns_capable_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags) {}

static int find_pagemap_read(void)
{
    int ret;
    struct kprobe pmr_kp = {
        .symbol_name = PAGEMAP_READ
    };
    ret = register_kprobe(&pmr_kp);
    if (ret < 0) {
        return ret;
    }
    addr_of_pagemap_read = (uint64_t)pmr_kp.addr;
    pr_info("address of %s: %pK", PAGEMAP_READ, pmr_kp.addr);
    unregister_kprobe(&pmr_kp);
    if (addr_of_pagemap_read == 0) {
        pr_warn("failed to find %s", PAGEMAP_READ);
        return -1;
    }
    return 0;
}

static int __init rh_driver_init(void)
{
    int ret;

    ret = find_pagemap_read();
    if (ret < 0) {
        pr_err("failed to find the kallsyms lookup table");
        return ret;
    }
    ret = register_kprobe(&rh_kp);
    if (ret < 0) {
        pr_err("register_kprobe failed, returned %d\n", ret);
        return ret;
    }
    pr_info("address of %s: %pK\n", FILE_NS_CAPABLE, rh_kp.addr);
    return 0;
}

static void __exit rh_driver_exit(void)
{
    unregister_kprobe(&rh_kp);

    if (final_probe_injected)
        unregister_kprobe(&target_kp);
}

module_init(rh_driver_init)
module_exit(rh_driver_exit)

