cat ../elks-gh/elks/arch/i86/boot/system-nm.map | sed -e '/&/d; /!/d' | sort > file
cc convmap.c -o convmap
./convmap > mapfile.c
rm file
