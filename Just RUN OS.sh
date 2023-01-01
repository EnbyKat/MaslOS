#!/bin/bash

cd kernel

qemu-system-x86_64 -drive file=bin/MaslOS.img -m 256M -cpu qemu64 -drive if=pflash,format=raw,unit=0,file="../ovmfbin/OVMF_CODE-pure-efi.fd",readonly=on -drive if=pflash,format=raw,unit=1,file="../ovmfbin/OVMF_VARS-pure-efi.fd" -net none
