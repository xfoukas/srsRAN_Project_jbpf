srsRAN Project with jbpf extensions
===================================

This repo provides extensions to build srsRAN with support for the [jbpf](https://github.com/microsoft/jbpf/) userspace eBPF framework.

To build, simply run the script `init_modules.sh` and then follow the regular build process for srsRAN found [here](./srsRAN_README.md), while setting the additional cmake flag `-DENABLE_JBPF=on".

