/*
 * Copyright 2017-2018, Rubicon Communications, LLC.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 */

#include <zebra.h>

#ifdef HAVE_LIBVPPMGMT

#include "if.h"
#include "prefix.h"
#include "connected.h"
#include "table.h"
#include "memory.h"
#include "rib.h"
#include "thread.h"
#include "privs.h"

#include "zebra/zserv.h"
#include "zebra/rt.h"
#include "zebra/redistribute.h"
#include "zebra/interface.h"
#include "zebra/debug.h"

#include <vppinfra/mem.h>

#include <vppmgmt/vpp_mgmt_api.h>

#include "zebra/rt_vpp.h"


unsigned int debug;		/* FIXME -- remove form libvppmgmt */

extern struct zebra_privs_t zserv_privs;

sw_interface_event_t *vpp_intf_events;
struct connected *ifc_add_events;
struct connected *ifc_del_events;
int vpp_event_fds[2];


static int kernel_read(struct thread *thread)
{
	ssize_t ret;
	char *buf[8];

	while (1) {
		ret = read(vpp_event_fds[0], buf, sizeof(buf));
		if (ret < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				break;
			}

			zlog_err("Unable to read from VPP event fd %d",
				 vpp_event_fds[0]);
			return -1;
		}
	}

	if (vec_len(vpp_intf_events) > 0) {
		vmgmt_intf_refresh_all();

		sw_interface_event_t *event;

		vec_foreach(event, vpp_intf_events) {
			vpp_intf_events_process(event);
		}

		vec_reset_length(vpp_intf_events);
	}

	if (vec_len(ifc_add_events) > 0) {
		struct connected *ifc;

		vec_foreach(ifc, ifc_add_events) {
			vpp_ifc_events_process(ifc, /* is_del */ 0);
		}

		vec_reset_length(ifc_add_events);
	}

	if (vec_len(ifc_del_events) > 0) {
		struct connected *ifc;

		vec_foreach(ifc, ifc_del_events) {
			vpp_ifc_events_process(ifc, /* is_del */ 1);
		}

		vec_reset_length(ifc_del_events);
	}

	struct zebra_ns *zns = (struct zebra_ns *)THREAD_ARG(thread);

	thread_add_read(zebrad.master, kernel_read, zns, vpp_event_fds[0], 0);

	return 0;
}


void kernel_init(struct zebra_ns *zns)
{
	int ret;

	clib_mem_init(NULL, 64 << 20);

	frr_elevate_privs(&zserv_privs) {

		ret = vmgmt_init((char *) "route_daemon", 1);
		if (ret < 0) {
			zlog_err("vmgmt_init failed with status %d", ret);
		} else {
			zlog_info("vmgmt_init success");
			vmgmt_intf_refresh_all();
		}
		vmgmt_intf_event_register(vpp_link_change);

	}

	vpp_intf_events = NULL;

	ret = pipe2(vpp_event_fds, O_NONBLOCK);
	if (ret < 0) {
		zlog_err("pipe2 failed with status %d", ret);
	} else {
		thread_add_read(zebrad.master, kernel_read, zns,
				vpp_event_fds[0], 0);
	}
}


void kernel_terminate(struct zebra_ns *zns)
{
	vec_free(vpp_intf_events);
	close(vpp_event_fds[0]);
	close(vpp_event_fds[1]);

	frr_elevate_privs(&zserv_privs) {
		vmgmt_disconnect();
		zlog_info("vmgmt_disconnect success");
		return;
	}
	zlog_info("vmgmt_disconnect failed to obtain priv escalation");
}

#endif
