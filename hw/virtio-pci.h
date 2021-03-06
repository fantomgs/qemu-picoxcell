/*
 * Virtio PCI Bindings
 *
 * Copyright IBM, Corp. 2007
 * Copyright (c) 2009 CodeSourcery
 *
 * Authors:
 *  Anthony Liguori   <aliguori@us.ibm.com>
 *  Paul Brook        <paul@codesourcery.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2.  See
 * the COPYING file in the top-level directory.
 */

#ifndef QEMU_VIRTIO_PCI_H
#define QEMU_VIRTIO_PCI_H

#include "virtio-net.h"
#include "virtio-serial.h"

typedef struct {
    PCIDevice pci_dev;
    VirtIODevice *vdev;
    uint32_t flags;
    uint32_t addr;
    uint32_t class_code;
    uint32_t nvectors;
    BlockConf block;
    NICConf nic;
    uint32_t host_features;
#ifdef CONFIG_LINUX
    V9fsConf fsconf;
#endif
    virtio_serial_conf serial;
    virtio_net_conf net;
    bool ioeventfd_disabled;
    bool ioeventfd_started;
} VirtIOPCIProxy;

extern void virtio_init_pci(VirtIOPCIProxy *proxy, VirtIODevice *vdev,
                            uint16_t vendor, uint16_t device,
                            uint16_t class_code, uint8_t pif);
#endif
