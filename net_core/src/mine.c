#include <linux/types.h>
#include "../../booter/src/booter.h"

CL_MINE(bpf_iter_get_info)
CL_MINE(bpf_iter_reg_target)
CL_MINE(bpf_iter_run_prog)
CL_MINE(bpf_link_cleanup)
CL_MINE(bpf_link_init)
CL_MINE(bpf_link_prime)
CL_MINE(bpf_link_settle)
CL_MINE(bpf_prog_change_xdp)
CL_MINE(bpf_prog_get_type_dev)
CL_MINE(bpf_prog_inc)
CL_MINE(bpf_prog_put)
CL_MINE(bpf_sk_storage_clone)
CL_MINE(bpf_sk_storage_free)
CL_MINE(bpf_warn_invalid_xdp_action)
CL_MINE(__cant_sleep)
CL_MINE(copy_bpf_fprog_from_user)
CL_MINE(cpu_map_prog_allowed)
CL_MINE(csum_ipv6_magic)
CL_MINE(datagram_poll)
CL_MINE(dev_activate)
CL_MINE(dev_addr_flush)
CL_MINE(dev_addr_init)
CL_MINE(dev_deactivate_many)
CL_MINE(device_rename)
CL_MINE(dev_init_scheduler)
CL_MINE(dev_map_can_have_prog)
CL_MINE(dev_mc_add_excl)
CL_MINE(dev_mc_del)
CL_MINE(dev_mc_flush)
CL_MINE(dev_mc_init)
CL_MINE(dev_proc_init)
CL_MINE(dev_qdisc_change_tx_queue_len)
CL_MINE(dev_shutdown)
CL_MINE(dev_uc_add_excl)
CL_MINE(dev_uc_del)
CL_MINE(dev_uc_flush)
CL_MINE(dev_uc_init)
CL_MINE(dql_init)
CL_MINE(dst_release)
CL_MINE(ethtool_check_ops)
CL_MINE(eth_type_trans)
CL_MINE(kernel_sendmsg)
CL_MINE(kernel_sendmsg_locked)
CL_MINE(kernel_sendpage_locked)
CL_MINE(kmem_cache_free_bulk)
CL_MINE(linkwatch_fire_event)
CL_MINE(linkwatch_forget_dev)
CL_MINE(linkwatch_init_dev)
CL_MINE(linkwatch_run_queue)
CL_MINE(metadata_dst_alloc)
CL_MINE(netdev_change_owner)
CL_MINE(netdev_kobject_init)
CL_MINE(netdev_queue_update_kobjects)
CL_MINE(netdev_register_kobject)
CL_MINE(netdev_unregister_kobject)
CL_MINE(__netdev_watchdog_up)
CL_MINE(netif_carrier_off)
CL_MINE(netif_carrier_on)
CL_MINE(net_ratelimit)
CL_MINE(net_rx_queue_update_kobjects)
CL_MINE(nla_find)
CL_MINE(nla_memcpy)
CL_MINE(__nla_parse)
CL_MINE(nla_put)
CL_MINE(nla_put_64bit)
CL_MINE(nla_reserve)
CL_MINE(nla_reserve_64bit)
CL_MINE(nla_strdup)
CL_MINE(nla_strlcpy)
CL_MINE(__nla_validate)
CL_MINE(page_frag_alloc)
CL_MINE(page_frag_free)
CL_MINE(put_cmsg)
CL_MINE(qdisc_reset)
CL_MINE(__qdisc_run)
CL_MINE(refcount_dec_and_mutex_lock)
CL_MINE(reuseport_detach_prog)
CL_MINE(reuseport_detach_sock)
CL_MINE(sch_direct_xmit)
CL_MINE(__scm_destroy)
CL_MINE(scm_detach_fds)
CL_MINE(__scm_send)
CL_MINE(send_sigurg)
CL_MINE(sk_attach_bpf)
CL_MINE(sk_attach_filter)
CL_MINE(skb_copy_datagram_iter)
CL_MINE(skb_free_datagram)
CL_MINE(__skb_get_hash)
CL_MINE(skb_recv_datagram)
CL_MINE(sk_detach_filter)
CL_MINE(sk_filter_charge)
CL_MINE(sk_filter_trim_cap)
CL_MINE(sk_filter_uncharge)
CL_MINE(sk_get_filter)
CL_MINE(sk_reuseport_attach_bpf)
CL_MINE(sk_reuseport_attach_filter)
CL_MINE(sock_create_lite)
CL_MINE(sock_diag_broadcast_destroy)
CL_MINE(sock_edemux)
CL_MINE(sock_from_file)
CL_MINE(sock_gen_cookie)
CL_MINE(sock_gen_put)
CL_MINE(sock_is_registered)
CL_MINE(__sock_recv_timestamp)
CL_MINE(__sock_recv_wifi_status)
CL_MINE(sock_register)
CL_MINE(sock_release)
CL_MINE(sock_wake_async)
CL_MINE(splice_to_pipe)
CL_MINE(static_key_slow_dec)
CL_MINE(tcp_ca_get_name_by_key)
CL_MINE(tcp_get_timestamping_opt_stats)
CL_MINE(tcp_wfree)
CL_MINE(xdp_do_generic_redirect)
CL_MINE(xdp_rxq_info_reg)
CL_MINE(xdp_rxq_info_unreg)
CL_MINE(yield)
CL_MINE(__zerocopy_sg_from_iter)
