$(IMAGE): $(BOOTLOADER_BIN) $(KERNEL_PLAM)
	dd if=/dev/zero of=$@ bs=1M count=64
	dd if=$(BOOTLOADER_BIN) of=$@ conv=notrunc
	dd if=$(KERNEL_PLAM) of=$@ seek=2048 conv=notrunc

$(LIVE_ISO): $(BOOTLOADER_BIN) $(KERNEL_PLAM)
	@mkdir -p iso_root/EFI/BOOT
	cp $(BOOTLOADER_BIN) iso_root/EFI/BOOT/BOOTX64.EFI
	cp $(KERNEL_PLAM) iso_root/kernel.plam
	xorriso -as mkisofs \
		-quiet \
		-iso-level 3 \
		-full-iso9660-filenames \
		-volid "PLUMOS_UEFI" \
		-eltorito-boot /EFI/BOOT/BOOTX64.EFI \
		-no-emul-boot \
		-boot-load-size 1 \
		-boot-info-table \
		-efi-boot /EFI/BOOT/BOOTX64.EFI \
		-efi-boot-part \
		-efi-boot-image \
		-o $@ \
		iso_root/
	@rm -rf iso_root