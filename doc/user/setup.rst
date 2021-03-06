.. _basic-setup:

Basic Setup
============

After installing FRR, some basic configuration must be completed before it is
ready to use.

Daemons File
------------
After a fresh install, starting FRR will do nothing. This is because daemons
must be explicitly enabled by editing a file in your configuration directory.
This file is usually located at :file:`/etc/frr/daemons` and determines which
daemons are activated when issuing a service start / stop command via init or
systemd. The file initially looks like this:

::

   zebra=no
   bgpd=no
   ospfd=no
   ospf6d=no
   ripd=no
   ripngd=no
   isisd=no
   pimd=no
   ldpd=no
   nhrpd=no
   eigrpd=no
   babeld=no
   sharpd=no
   staticd=no
   pbrd=no
   bfdd=no

To enable a particular daemon, simply change the corresponding 'no' to 'yes'.
Subsequent service restarts should start the daemon.

Daemons Configuration File
--------------------------
There is another file that controls the default options passed to daemons when
starting FRR as a service. This file is located in your configuration
directory, usually at :file:`/etc/frr/daemons`.

This file has several parts. Here is an example:

::

   #
   # If this option is set the /etc/init.d/frr script automatically loads
   # the config via "vtysh -b" when the servers are started.
   # Check /etc/pam.d/frr if you intend to use "vtysh"!
   #
   vtysh_enable=yes
   zebra_options=" -s 90000000 --daemon -A 127.0.0.1"
   bgpd_options="   --daemon -A 127.0.0.1"
   ospfd_options="  --daemon -A 127.0.0.1"
   ospf6d_options=" --daemon -A ::1"
   ripd_options="   --daemon -A 127.0.0.1"
   ripngd_options=" --daemon -A ::1"
   isisd_options="  --daemon -A 127.0.0.1"
   pimd_options="  --daemon -A 127.0.0.1"
   ldpd_options="  --daemon -A 127.0.0.1"
   nhrpd_options="  --daemon -A 127.0.0.1"
   eigrpd_options="  --daemon -A 127.0.0.1"
   babeld_options="  --daemon -A 127.0.0.1"
   sharpd_options="  --daemon -A 127.0.0.1"
   staticd_options="  --daemon -A 127.0.0.1"
   pbrd_options="  --daemon -A 127.0.0.1"
   bfdd_options="  --daemon -A 127.0.0.1"

   # The list of daemons to watch is automatically generated by the init script.
   watchfrr_enable=yes
   watchfrr_options=(-d -r /usr/sbin/servicebBfrrbBrestartbB%s -s /usr/sbin/servicebBfrrbBstartbB%s -k /usr/sbin/servicebBfrrbBstopbB%s -b bB)

   # If valgrind_enable is 'yes' the frr daemons will be started via valgrind.
   # The use case for doing so is tracking down memory leaks, etc in frr.
   valgrind_enable=no
   valgrind=/usr/bin/valgrind

Breaking this file down:

::

   vtysh_enable=yes

As the comment says, this causes :ref:`VTYSH <vty-shell>` to apply
configuration when starting the daemons. This is useful for a variety of
reasons touched on in the VTYSH documentation and should generally be enabled.

::

   zebra_options=" -s 90000000 --daemon -A 127.0.0.1"
   bgpd_options="   --daemon -A 127.0.0.1"
   ...

The next set of lines controls what options are passed to daemons when started
from the service script. Usually daemons will have ``--daemon`` and ``-A
<address>`` specified in order to daemonize and listen for VTY commands on a
particular address.

::

   # The list of daemons to watch is automatically generated by the init script.
   watchfrr_enable=yes
   watchfrr_options=(-d -r /usr/sbin/servicebBfrrbBrestartbB%s -s /usr/sbin/servicebBfrrbBstartbB%s -k /usr/sbin/servicebBfrrbBstopbB%s -b bB)

Options for the ``watchfrr``, the watchdog daemon.

::

   valgrind_enable=no
   valgrind=/usr/bin/valgrind

Whether or not to start FRR daemons under Valgrind. This is primarily useful
for gathering information for bug reports and for developers.
``valgrind_enable`` should be ``no`` for production use.

Services
--------
FRR daemons have their own terminal interface or VTY.  After installation, it's
a good idea to setup each daemon's port number to connect to them. To do this
add the following entries to :file:`/etc/services`.

::

   zebrasrv      2600/tcp		  # zebra service
   zebra         2601/tcp		  # zebra vty
   ripd          2602/tcp		  # RIPd vty
   ripngd        2603/tcp		  # RIPngd vty
   ospfd         2604/tcp		  # OSPFd vty
   bgpd          2605/tcp		  # BGPd vty
   ospf6d        2606/tcp		  # OSPF6d vty
   ospfapi       2607/tcp		  # ospfapi
   isisd         2608/tcp		  # ISISd vty
   babeld        2609/tcp                 # BABELd vty
   nhrpd         2610/tcp		  # nhrpd vty
   pimd          2611/tcp		  # PIMd vty
   ldpd          2612/tcp                 # LDPd vty
   eigprd        2613/tcp                 # EIGRPd vty
   bfdd          2617/tcp                 # bfdd vty


If you use a FreeBSD newer than 2.2.8, the above entries are already added to
:file:`/etc/services` so there is no need to add it. If you specify a port
number when starting the daemon, these entries may not be needed.

You may need to make changes to the config files in |INSTALL_PREFIX_ETC|.

systemd
-------
Although not installed when installing from source, FRR provides a service file
for use with ``systemd``. It is located in :file:`tools/frr.service` in the Git
repository. If ``systemctl status frr.service`` indicates that the FRR service
is not found, copy the service file from the Git repository into your preferred
location. A good place is usually ``/etc/systemd/system/``.

After issuing a ``systemctl daemon-reload``, you should be able to start the
FRR service via ``systemctl start frr``. If this fails, or no daemons are
started. check the ``journalctl`` logs for an indication of what went wrong.
