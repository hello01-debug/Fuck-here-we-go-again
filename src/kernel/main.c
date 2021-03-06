#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "multiboot2.h"
#include "utils/debug.h"
#include "cpu/gdt.h"
#include "cpu/tss.h"
#include "cpu/idt.h"
#include "memory/pmm.h"
#include "memory/vmm.h"
#include "cpu/exceptions.h"
#include "cpu/pit.h"
#include "cpu/rtc.h"
#include "system/framebuffer.h"

int kernel_main(uint32_t addr, uint32_t magic)
{
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
        return -1;
    
    struct multiboot_tage_basic_meminfo *multiboot_meminfo;
    struct multiboot_tag_mmap *multiboot_mmap;
	struct multiboot_tag_framebuffer *multiboot_framebuffer;

	struct multiboot_tag *tag;
	for (tag = (struct multiboot_tag *)(addr + 8);
		 tag->type != MULTIBOOT_TAG_TYPE_END;
		 tag = (struct multiboot_tag *)((multiboot_uint8_t *)tag + ((tag->size + 7) & ~7)))
	{
		switch (tag->type)
		{
		case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
			multiboot_meminfo = (struct multiboot_tag_basic_meminfo *)tag;
			break;
		case MULTIBOOT_TAG_TYPE_MMAP:
		{
			multiboot_mmap = (struct multiboot_tag_mmap *)tag;
			break;
		}
		case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
		{
			multiboot_framebuffer = (struct multiboot_tag_framebuffer *)tag;
			break;
		}
		}
	}
    
    debug_init();

    gdt_init();
	install_tss(5, 0x10, 0);

	idt_init();

	pmm_init(multiboot_meminfo, multiboot_mmap);
	vmm_init();

	exception_init();

	rtc_init();
	pit_init();

	framebuffer_init(multiboot_framebuffer);

	asm volatile("sti");

	

    for (;;)
        ;

    return 0;
}