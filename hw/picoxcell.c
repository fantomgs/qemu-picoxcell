/*
 * Picochip picoxcell PC7302 emulation.
 *
 * Copyright (c) 2011 Picochip Ltd, Jamie Iles
 *
 * This code is licenced under the GPL.
 */

#include "sysbus.h"
#include "arm-misc.h"
#include "primecell.h"
#include "devices.h"
#include "net.h"
#include "sysemu.h"
#include "pci.h"
#include "usb-ohci.h"
#include "boards.h"
#include "blockdev.h"
#include "pc.h"

static struct arm_boot_info pc7302_binfo;

static void pc7302_init(ram_addr_t ram_size,
                        const char *boot_device,
                        const char *kernel_filename,
                        const char *kernel_cmdline,
                        const char *initrd_filename,
                        const char *cpu_model, int board_id,
                        uint16_t device_id)
{
    CPUState *env;
    ram_addr_t ram_offset;
    qemu_irq *cpu_pic;
    qemu_irq vic0[32];
    qemu_irq vic1[32];
    DeviceState *dev;
    int n;

    if (!cpu_model)
        cpu_model = "arm1176";
    env = cpu_init(cpu_model);
    if (!env) {
        fprintf(stderr, "Unable to find CPU definition\n");
        exit(1);
    }
    ram_offset = qemu_ram_alloc(NULL, "pc7302.ram", ram_size);
    cpu_register_physical_memory(0, ram_size, ram_offset | IO_MEM_RAM);

    ram_offset = qemu_ram_alloc(NULL, "picoxcell.sram", 128 * 1024);
    cpu_register_physical_memory(0x20000000, 128 * 1024, ram_offset | IO_MEM_RAM);

    ram_offset = qemu_ram_alloc(NULL, "fuse.ram", 0x10000);
    cpu_register_physical_memory(0, 0x10000, ram_offset | IO_MEM_RAM);

    /*
     * Register the VIC's.  picoxcell actually has 2xPL192's rather than
     * PL190's, but we can use the pl190 model as long as the system doesn't
     * require vectored interrupt support.  The picoxcell Linux port doesn't
     * use this so we're OK for now.
     */
    cpu_pic = arm_pic_init_cpu(env);
    dev = sysbus_create_varargs("pl190", 0x80060000,
                                cpu_pic[0], cpu_pic[1], NULL);
    for (n = 0; n < 32; n++)
        vic0[n] = qdev_get_gpio_in(dev, n);

    dev = sysbus_create_varargs("pl190", 0x80064000,
                                cpu_pic[0], cpu_pic[1], NULL);
    for (n = 0; n < 32; n++)
        vic1[n] = qdev_get_gpio_in(dev, n);

    serial_mm_init(0x80230000, 2, vic1[10], 115200, serial_hds[0], 1, 0);
    serial_mm_init(0x80240000, 2, vic1[9], 115200, serial_hds[1] ?
                   serial_hds[1] : qemu_chr_open("uart2", "null", NULL), 1,
                   0);

    if (device_id == 0x8003 || device_id == 0x8007)
        dev = sysbus_create_varargs("axi2cfg,pc3x2", 0x800A0000, vic0[9], vic0[8],
                                    NULL);
    else
        dev = sysbus_create_varargs("axi2cfg,pc3x3", 0x800A0000, vic0[9], vic0[8],
                                    NULL);
    qdev_prop_set_uint32(dev, "device_id", device_id);

    sysbus_create_varargs("dwapb_timer", 0x80210000, vic0[4], vic0[5], NULL);
    sysbus_create_simple("dwapb_rtc", 0x80200000, NULL);

    pc7302_binfo.ram_size = ram_size;
    pc7302_binfo.kernel_filename = kernel_filename;
    pc7302_binfo.kernel_cmdline = kernel_cmdline;
    pc7302_binfo.initrd_filename = initrd_filename;
    pc7302_binfo.board_id = board_id;
    arm_load_kernel(env, &pc7302_binfo);
}

static void pc7302_pc3x2_init(ram_addr_t ram_size,
                              const char *boot_device,
                              const char *kernel_filename,
                              const char *kernel_cmdline,
                              const char *initrd_filename,
                              const char *cpu_model)
{
    pc7302_init(ram_size, boot_device, kernel_filename, kernel_cmdline,
                initrd_filename, cpu_model, 2220, 0x8003);
}

static QEMUMachine pc7302_pc3x2_machine = {
    .name = "pc7302-pc3x2",
    .desc = "picoxcell pc7302 (ARM1176JZ-S)",
    .init = pc7302_pc3x2_init,
};

static void pc7302_pc3x3_init(ram_addr_t ram_size,
                              const char *boot_device,
                              const char *kernel_filename,
                              const char *kernel_cmdline,
                              const char *initrd_filename,
                              const char *cpu_model)
{
    pc7302_init(ram_size, boot_device, kernel_filename, kernel_cmdline,
                initrd_filename, cpu_model, 2220, 0x0022);
}

static QEMUMachine pc7302_pc3x3_machine = {
    .name = "pc7302-pc3x3",
    .desc = "picoxcell pc7302 (ARM1176JZ-S)",
    .init = pc7302_pc3x3_init,
};

static void picoxcell_machine_init(void)
{
    qemu_register_machine(&pc7302_pc3x2_machine);
    qemu_register_machine(&pc7302_pc3x3_machine);
}
machine_init(picoxcell_machine_init);
