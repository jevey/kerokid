KEROKID: the KErnel ROotKit Detector                                      
==============

usage:
----------------------
```
make linux-x86 // for i386
make linux-x86_64 // for amd64
make linux-x86 DEBUG=y // or
make linux-x86_64 DEBUG=y // for debugging
sudo insmod kerokid.ko
dmesg // to see results
```
features
----------------------
- detects syscall table entries directing to module space: determines if module is present at destination or not
- detects inline hooking of syscall table entries: determines if module is present at destination or not
- detects notifier entries to module space: detects module structure of hidden modules in memory [in progress]
 
successfully tested with
----------------------
x86
- Debian-7.7.0-i386 3.2.0-4-486
- Fedora-Desktop-i686-20-1 3.11.10-301.fc20.i686
- Ubuntu 14.04 i686 3.13.0-32-generic

x86_64
- recent vanilla kernels
- Debian 7.7.0 3.16-0.bpo.2-amd64
- Mageia-4.1 3.12.21-desktop-2.mga4
- OpenSUSE 13.2 3.16.6-2-2-desktop
- Ubuntu 14.10 3.16.0-23-generic
- Ubuntu 14.04 3.13.0-generic

known issues
----------------------
- notifiers registered with highest priority cannot be found
- Debugging hooks as described in: http://phrack.org/issues/65/8.html
- Interupt Descriptor Table (IDT) manipulation
- unknown inline hook codes
- advanced inline hooks -> somewhere in the function, calculated jump address, jump from subroutine
- issues with some kernels. confirmed for:
  - Debian 7.7.0 3.2.0-4-amd64 (memory issue)
  - Fedora Desktop 3.11.10.301-fc20.x86_64 (memory issue)
  - Kali Linux 1.0.9a x86_64 3.14.5-1kali1 (memory issue)
  - Ubuntu 12.04.5 LTS x86_64 3.2.0-33-generic (memory issue)
  - CentOS-7.0-1406-x86_64 3.10.0-123.9.3.el7.x86_64 (compiles but kernel panic)
