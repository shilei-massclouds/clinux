/* automatically generated by rust-bindgen 0.56.0 */

extern "C" {
    #[link_name="rust_helper_BUG"]
    pub fn BUG();
}
extern "C" {
    #[link_name="rust_helper_clk_disable_unprepare"]
    pub fn clk_disable_unprepare(clk: *mut clk);
}
extern "C" {
    #[link_name="rust_helper_clk_prepare_enable"]
    pub fn clk_prepare_enable(clk: *mut clk) -> core::ffi::c_int;
}
extern "C" {
    #[link_name="rust_helper_copy_from_user"]
    pub fn copy_from_user(
        to: *mut core::ffi::c_void,
        from: *const core::ffi::c_void,
        n: core::ffi::c_ulong,
    ) -> core::ffi::c_ulong;
}
extern "C" {
    #[link_name="rust_helper_copy_to_user"]
    pub fn copy_to_user(
        to: *mut core::ffi::c_void,
        from: *const core::ffi::c_void,
        n: core::ffi::c_ulong,
    ) -> core::ffi::c_ulong;
}
extern "C" {
    #[link_name="rust_helper_clear_user"]
    pub fn clear_user(
        to: *mut core::ffi::c_void,
        n: core::ffi::c_ulong,
    ) -> core::ffi::c_ulong;
}
extern "C" {
    #[link_name="rust_helper_ioremap"]
    pub fn ioremap(
        offset: resource_size_t,
        size: core::ffi::c_ulong,
    ) -> *mut core::ffi::c_void;
}
extern "C" {
    #[link_name="rust_helper_readb"]
    pub fn readb(addr: *const core::ffi::c_void) -> u8_;
}
extern "C" {
    #[link_name="rust_helper_readw"]
    pub fn readw(addr: *const core::ffi::c_void) -> u16_;
}
extern "C" {
    #[link_name="rust_helper_readl"]
    pub fn readl(addr: *const core::ffi::c_void) -> u32_;
}
extern "C" {
    #[link_name="rust_helper_readq"]
    pub fn readq(addr: *const core::ffi::c_void) -> u64_;
}
extern "C" {
    #[link_name="rust_helper_writeb"]
    pub fn writeb(value: u8_, addr: *mut core::ffi::c_void);
}
extern "C" {
    #[link_name="rust_helper_writew"]
    pub fn writew(value: u16_, addr: *mut core::ffi::c_void);
}
extern "C" {
    #[link_name="rust_helper_writel"]
    pub fn writel(value: u32_, addr: *mut core::ffi::c_void);
}
extern "C" {
    #[link_name="rust_helper_writeq"]
    pub fn writeq(value: u64_, addr: *mut core::ffi::c_void);
}
extern "C" {
    #[link_name="rust_helper_readb_relaxed"]
    pub fn readb_relaxed(addr: *const core::ffi::c_void) -> u8_;
}
extern "C" {
    #[link_name="rust_helper_readw_relaxed"]
    pub fn readw_relaxed(addr: *const core::ffi::c_void) -> u16_;
}
extern "C" {
    #[link_name="rust_helper_readl_relaxed"]
    pub fn readl_relaxed(addr: *const core::ffi::c_void) -> u32_;
}
extern "C" {
    #[link_name="rust_helper_readq_relaxed"]
    pub fn readq_relaxed(addr: *const core::ffi::c_void) -> u64_;
}
extern "C" {
    #[link_name="rust_helper_writeb_relaxed"]
    pub fn writeb_relaxed(value: u8_, addr: *mut core::ffi::c_void);
}
extern "C" {
    #[link_name="rust_helper_writew_relaxed"]
    pub fn writew_relaxed(value: u16_, addr: *mut core::ffi::c_void);
}
extern "C" {
    #[link_name="rust_helper_writel_relaxed"]
    pub fn writel_relaxed(value: u32_, addr: *mut core::ffi::c_void);
}
extern "C" {
    #[link_name="rust_helper_writeq_relaxed"]
    pub fn writeq_relaxed(value: u64_, addr: *mut core::ffi::c_void);
}
extern "C" {
    #[link_name="rust_helper_memcpy_fromio"]
    pub fn memcpy_fromio(
        to: *mut core::ffi::c_void,
        from: *const core::ffi::c_void,
        count: core::ffi::c_long,
    );
}
extern "C" {
    #[link_name="rust_helper___spin_lock_init"]
    pub fn __spin_lock_init(
        lock: *mut spinlock_t,
        name: *const core::ffi::c_char,
        key: *mut lock_class_key,
    );
}
extern "C" {
    #[link_name="rust_helper_spin_lock"]
    pub fn spin_lock(lock: *mut spinlock_t);
}
extern "C" {
    #[link_name="rust_helper_spin_unlock"]
    pub fn spin_unlock(lock: *mut spinlock_t);
}
extern "C" {
    #[link_name="rust_helper_spin_lock_irqsave"]
    pub fn spin_lock_irqsave(lock: *mut spinlock_t) -> core::ffi::c_ulong;
}
extern "C" {
    #[link_name="rust_helper_spin_unlock_irqrestore"]
    pub fn spin_unlock_irqrestore(lock: *mut spinlock_t, flags: core::ffi::c_ulong);
}
extern "C" {
    #[link_name="rust_helper__raw_spin_lock_init"]
    pub fn _raw_spin_lock_init(
        lock: *mut raw_spinlock_t,
        name: *const core::ffi::c_char,
        key: *mut lock_class_key,
    );
}
extern "C" {
    #[link_name="rust_helper_raw_spin_lock"]
    pub fn raw_spin_lock(lock: *mut raw_spinlock_t);
}
extern "C" {
    #[link_name="rust_helper_raw_spin_unlock"]
    pub fn raw_spin_unlock(lock: *mut raw_spinlock_t);
}
extern "C" {
    #[link_name="rust_helper_raw_spin_lock_irqsave"]
    pub fn raw_spin_lock_irqsave(lock: *mut raw_spinlock_t) -> core::ffi::c_ulong;
}
extern "C" {
    #[link_name="rust_helper_raw_spin_unlock_irqrestore"]
    pub fn raw_spin_unlock_irqrestore(
        lock: *mut raw_spinlock_t,
        flags: core::ffi::c_ulong,
    );
}
extern "C" {
    #[link_name="rust_helper_init_wait"]
    pub fn init_wait(wq_entry: *mut wait_queue_entry);
}
extern "C" {
    #[link_name="rust_helper_init_waitqueue_func_entry"]
    pub fn init_waitqueue_func_entry(
        wq_entry: *mut wait_queue_entry,
        func: wait_queue_func_t,
    );
}
extern "C" {
    #[link_name="rust_helper_signal_pending"]
    pub fn signal_pending(t: *mut task_struct) -> core::ffi::c_int;
}
extern "C" {
    #[link_name="rust_helper_alloc_pages"]
    pub fn alloc_pages(gfp_mask: gfp_t, order: core::ffi::c_uint) -> *mut page;
}
extern "C" {
    #[link_name="rust_helper_kmap"]
    pub fn kmap(page: *mut page) -> *mut core::ffi::c_void;
}
extern "C" {
    #[link_name="rust_helper_kunmap"]
    pub fn kunmap(page: *mut page);
}
extern "C" {
    #[link_name="rust_helper_cond_resched"]
    pub fn cond_resched() -> core::ffi::c_int;
}
extern "C" {
    #[link_name="rust_helper_copy_from_iter"]
    pub fn copy_from_iter(
        addr: *mut core::ffi::c_void,
        bytes: usize,
        i: *mut iov_iter,
    ) -> usize;
}
extern "C" {
    #[link_name="rust_helper_copy_to_iter"]
    pub fn copy_to_iter(
        addr: *const core::ffi::c_void,
        bytes: usize,
        i: *mut iov_iter,
    ) -> usize;
}
extern "C" {
    #[link_name="rust_helper_IS_ERR"]
    pub fn IS_ERR(ptr: *const core::ffi::c_void) -> bool_;
}
extern "C" {
    #[link_name="rust_helper_PTR_ERR"]
    pub fn PTR_ERR(ptr: *const core::ffi::c_void) -> core::ffi::c_long;
}
extern "C" {
    #[link_name="rust_helper_errname"]
    pub fn errname(err: core::ffi::c_int) -> *const core::ffi::c_char;
}
extern "C" {
    #[link_name="rust_helper_mutex_lock"]
    pub fn mutex_lock(lock: *mut mutex);
}
extern "C" {
    #[link_name="rust_helper_amba_set_drvdata"]
    pub fn amba_set_drvdata(dev: *mut amba_device, data: *mut core::ffi::c_void);
}
extern "C" {
    #[link_name="rust_helper_amba_get_drvdata"]
    pub fn amba_get_drvdata(dev: *mut amba_device) -> *mut core::ffi::c_void;
}
extern "C" {
    #[link_name="rust_helper_platform_get_drvdata"]
    pub fn platform_get_drvdata(pdev: *const platform_device)
        -> *mut core::ffi::c_void;
}
extern "C" {
    #[link_name="rust_helper_platform_set_drvdata"]
    pub fn platform_set_drvdata(
        pdev: *mut platform_device,
        data: *mut core::ffi::c_void,
    );
}
extern "C" {
    #[link_name="rust_helper_REFCOUNT_INIT"]
    pub fn REFCOUNT_INIT(n: core::ffi::c_int) -> refcount_t;
}
extern "C" {
    #[link_name="rust_helper_refcount_inc"]
    pub fn refcount_inc(r: *mut refcount_t);
}
extern "C" {
    #[link_name="rust_helper_refcount_dec_and_test"]
    pub fn refcount_dec_and_test(r: *mut refcount_t) -> bool_;
}
extern "C" {
    #[link_name="rust_helper_rb_link_node"]
    pub fn rb_link_node(
        node: *mut rb_node,
        parent: *mut rb_node,
        rb_link: *mut *mut rb_node,
    );
}
extern "C" {
    #[link_name="rust_helper_get_current"]
    pub fn get_current() -> *mut task_struct;
}
extern "C" {
    #[link_name="rust_helper_get_task_struct"]
    pub fn get_task_struct(t: *mut task_struct);
}
extern "C" {
    #[link_name="rust_helper_put_task_struct"]
    pub fn put_task_struct(t: *mut task_struct);
}
extern "C" {
    #[link_name="rust_helper_security_binder_set_context_mgr"]
    pub fn security_binder_set_context_mgr(mgr: *const cred) -> core::ffi::c_int;
}
extern "C" {
    #[link_name="rust_helper_security_binder_transaction"]
    pub fn security_binder_transaction(
        from: *const cred,
        to: *const cred,
    ) -> core::ffi::c_int;
}
extern "C" {
    #[link_name="rust_helper_security_binder_transfer_binder"]
    pub fn security_binder_transfer_binder(
        from: *const cred,
        to: *const cred,
    ) -> core::ffi::c_int;
}
extern "C" {
    #[link_name="rust_helper_security_binder_transfer_file"]
    pub fn security_binder_transfer_file(
        from: *const cred,
        to: *const cred,
        file: *mut file,
    ) -> core::ffi::c_int;
}
extern "C" {
    #[link_name="rust_helper_get_file"]
    pub fn get_file(f: *mut file) -> *mut file;
}
extern "C" {
    #[link_name="rust_helper_rcu_read_lock"]
    pub fn rcu_read_lock();
}
extern "C" {
    #[link_name="rust_helper_rcu_read_unlock"]
    pub fn rcu_read_unlock();
}
extern "C" {
    #[link_name="rust_helper_synchronize_rcu"]
    pub fn synchronize_rcu();
}
extern "C" {
    #[link_name="rust_helper_dev_get_drvdata"]
    pub fn dev_get_drvdata(dev: *mut device) -> *mut core::ffi::c_void;
}
extern "C" {
    #[link_name="rust_helper_dev_name"]
    pub fn dev_name(dev: *const device) -> *const core::ffi::c_char;
}
extern "C" {
    #[link_name="rust_helper___seqcount_init"]
    pub fn __seqcount_init(
        s: *mut seqcount_t,
        name: *const core::ffi::c_char,
        key: *mut lock_class_key,
    );
}
extern "C" {
    #[link_name="rust_helper_read_seqcount_begin"]
    pub fn read_seqcount_begin(s: *mut seqcount_t) -> core::ffi::c_uint;
}
extern "C" {
    #[link_name="rust_helper_read_seqcount_retry"]
    pub fn read_seqcount_retry(
        s: *mut seqcount_t,
        start: core::ffi::c_uint,
    ) -> core::ffi::c_int;
}
extern "C" {
    #[link_name="rust_helper_write_seqcount_begin"]
    pub fn write_seqcount_begin(s: *mut seqcount_t);
}
extern "C" {
    #[link_name="rust_helper_write_seqcount_end"]
    pub fn write_seqcount_end(s: *mut seqcount_t);
}
extern "C" {
    #[link_name="rust_helper_irq_set_handler_locked"]
    pub fn irq_set_handler_locked(data: *mut irq_data, handler: irq_flow_handler_t);
}
extern "C" {
    #[link_name="rust_helper_irq_data_get_irq_chip_data"]
    pub fn irq_data_get_irq_chip_data(d: *mut irq_data) -> *mut core::ffi::c_void;
}
extern "C" {
    #[link_name="rust_helper_irq_desc_get_chip"]
    pub fn irq_desc_get_chip(desc: *mut irq_desc) -> *mut irq_chip;
}
extern "C" {
    #[link_name="rust_helper_irq_desc_get_handler_data"]
    pub fn irq_desc_get_handler_data(desc: *mut irq_desc) -> *mut core::ffi::c_void;
}
extern "C" {
    #[link_name="rust_helper_chained_irq_enter"]
    pub fn chained_irq_enter(chip: *mut irq_chip, desc: *mut irq_desc);
}
extern "C" {
    #[link_name="rust_helper_chained_irq_exit"]
    pub fn chained_irq_exit(chip: *mut irq_chip, desc: *mut irq_desc);
}
extern "C" {
    #[link_name="rust_helper_get_cred"]
    pub fn get_cred(cred: *const cred) -> *const cred;
}
extern "C" {
    #[link_name="rust_helper_put_cred"]
    pub fn put_cred(cred: *const cred);
}
extern "C" {
    #[link_name="rust_helper_of_match_device"]
    pub fn of_match_device(
        matches: *const of_device_id,
        dev: *const device,
    ) -> *const of_device_id;
}
extern "C" {
    #[link_name="rust_helper_init_completion"]
    pub fn init_completion(c: *mut completion);
}
extern "C" {
    #[link_name="rust_helper_skb_get"]
    pub fn skb_get(skb: *mut sk_buff) -> *mut sk_buff;
}
extern "C" {
    #[link_name="rust_helper_skb_headlen"]
    pub fn skb_headlen(skb: *const sk_buff) -> core::ffi::c_uint;
}
extern "C" {
    #[link_name="rust_helper_dev_hold"]
    pub fn dev_hold(dev: *mut net_device);
}
extern "C" {
    #[link_name="rust_helper_dev_put"]
    pub fn dev_put(dev: *mut net_device);
}
extern "C" {
    #[link_name="rust_helper_get_net"]
    pub fn get_net(net: *mut net) -> *mut net;
}
extern "C" {
    #[link_name="rust_helper_put_net"]
    pub fn put_net(net: *mut net);
}
extern "C" {
    #[link_name="rust_helper_NF_QUEUE_NR"]
    pub fn NF_QUEUE_NR(n: core::ffi::c_uint) -> core::ffi::c_uint;
}
extern "C" {
    #[link_name="rust_helper___INIT_WORK_WITH_KEY"]
    pub fn __INIT_WORK_WITH_KEY(
        work: *mut work_struct,
        func: work_func_t,
        on_stack: bool_,
        key: *mut lock_class_key,
    );
}
extern "C" {
    #[link_name="rust_helper_dget"]
    pub fn dget(dentry: *mut dentry) -> *mut dentry;
}
extern "C" {
    #[link_name="rust_helper_lockdep_register_key"]
    pub fn lockdep_register_key(key: *mut lock_class_key);
}
extern "C" {
    #[link_name="rust_helper_lockdep_unregister_key"]
    pub fn lockdep_unregister_key(key: *mut lock_class_key);
}
extern "C" {
    #[link_name="rust_helper_fs_parse"]
    pub fn fs_parse(
        fc: *mut fs_context,
        desc: *const fs_parameter_spec,
        param: *mut fs_parameter,
        result: *mut fs_parse_result,
    ) -> core::ffi::c_int;
}
