# Kea with YANG interface using Sysrepo plugin

This file contains instruction how to set up an environment that
allows doing experiments with Sysrepo plugin that is able to receive
configuration from Sysrepo and send it to Kea.

## Install and setup Kea

1. Download experimental Kea code that has set-config method
implemented.

```bash
git clone https://github.com/isc-projects/kea-yang
autoreconf -i
./configure
make
```

Refer to Kea User's Guide to details.

2. Run Kea with a config file.
You may use one of the examples from kea-configs/kea-cfg* as a
template. One item necessary for Sysrepo integration to work is to
have control socket defined. For example:

```json
      "control-socket": {
          "socket-type": "unix",
          "socket-name": "/tmp/kea-dhcp6-ctrl.sock"
      },
```

This is the location where Kea will create its unix domain socket and
will await commands. This has to match to whatever is specified in
sysrepo-plugin-kea.

3. Run Kea with:
```bash
cd src/bin/dhcp6
./kea-dhcp6 -c your-kea-config-file.json
```

## Install and setup Sysrepo

1. Install sysrepo
Refer to sysrepo documentation for details.

2. Update paths
See plugin-kea.c. Edit KEA_CONTROL_SOCKET to match whatever
you configured in socket-name in step 2. Edit KEA_CONTROL_CLIENT
to point to ctrl-client-cli binary.
(obsolete: cmake should do this)

3. Build sysrepo-plugin-kea
```bash
mkdir build
cmake ..
make
```

4. Copy libplugin-kea.ko to /usr/local/lib/sysrepo/plugins
```bash
cp ~/devel/sysrepo-plugin-kea/build/libplugin-kea.so /usr/local/lib/sysrepo/plugins
```

5. List all yang modules
```bash
sysrepoctl -l
```

6. Install Kea YANG module into the sysrepo 
```bash
sysrepoctl --install --yang=ietf-kea-dhcpv6@2016-07-16.yang
```
7. Verify it's actually installed
```bash
sysrepoctl -l
```

8. Set the configuration. There are three ways to do it:
8.1: Manually (see step 11)
8.2: Run basic_config is a tool that sets first two leafs in the model.
     ./basic_config
8.3: There is an example in kea-configs/kea-cfg1-basic.xml. It can be
loaded with:

```bash
sysrepocfg --import=kea-cfg1-basic.xml --datastore=startup ietf-kea-dhcpv6
```

9. Start sysrepo daemon
```bash
sysrepo -l 4 -d
```
(-l 4 is logging level 4, the most verbose, -d means it stays in the
foreground, so you can see what's going on)

10. Start sysrepo-plugind - daemon that loads plugins from /usr/local/lib/sysrepo/plugins

```bash
sysrepo-plugind -l 4 -d
```
(make sure it prints that the plugin-kea is loaded) The callback
installed with sr_module_change_subscribe should be called.  This call
back should apply configuration to Kea.

At this stage the sysrepo-plugind should print out messages from
libplugin-kea that retrieves the configuration and sends it over the
unix socket to Kea.

Kea should print out messages about receiving
set-config command:

```
2016-07-17 15:36:56.699 INFO  [kea-dhcp6.commands/14236] COMMAND_RECEIVED Received command 'set-config'
```

## Extra steps

The following steps are not needed to have the whole setup
operational. They're useful to conduct certain additional
tasks.

11. Configuration can be inspected and edited with with:
```bash
sysrepocfg --editor=emacs --datastore=running ietf-kea-dhcpv6
```

If the configuration changes, the plugin should be notified.
The callback should get the new configuration and send it
to Kea.

12. Export current model configuration to a file:
```bash
sysrepocfg --export=/tmp/backup.json --format=json --datastore=startup  ietf-kea-dhcpv6
```

---------------------

Tools that may be useful to look at:
Yang Explorer - GUI tool for browsing and editing YANG,
requires flash and it's tricky to set up.
