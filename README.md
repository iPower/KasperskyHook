# KasperskyHook
Hook system calls on Windows by using Kaspersky's hypervisor



## How does it work?

Kaspersky utilizes its hypervisor when hardware virtualization is supported for additional protection. It hooks system calls by changing `IA32_LSTAR` to point to its own syscall handler (which is basically a copy of `KiSystemCall64`) so it dispatches system calls to its own handlers (while doing initialization, it builds its own dispatch table).

This project loads klhk.sys (Kaspersky's hypervisor module) and a custom driver which interfaces with it to subvert the system and hook system calls.



## Why did you write this?

While researching Kaspersky components, I thought it was an interesting idea to write a custom project that lets me hook system calls by using Kaspersky's hypervisor to take a closer look at what it is doing.



## Build steps - how to use it

* Download [Visual Studio 2019](https://visualstudio.microsoft.com/pt-br/downloads/), [WDK](https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk), clone this repository and build the solution.
* Make sure `KasperskyHook.sys` and `KasperskyHookLoader.exe` are in the same folder. Copy `klhk.sys` to `\Windows\System32\drivers`
* Execute `KasperskyHookLoader.exe` and have fun :D


## Troubleshooting

If you're seeing `C00000A3` or `C000090B` from `kaspersky::hvm_init()`, it usually means the hypervisor component of the driver couldn't start. Here are the most common causes and how to fix them:

- **Run on bare metal** – The driver refuses to load inside a virtual machine. Make sure you're on a physical machine.
- **Enable hardware virtualization** – Check that VT-x (Intel) or AMD-V (AMD) is enabled in your BIOS/UEFI.
- **Avoid conflicts with other hypervisors** – Windows Hyper-V, Virtualization-Based Security (VBS), or other antivirus software that uses hardware-assisted virtualization can block `klhk.sys`. Disable them if they're active.
- **Match the Windows build** – klhk.sys contains a hardcoded offset for `ExGetPreviousMode` that must match your specific Windows version. If the pattern check fails, HVM won't start to avoid a crash. In this case, you should upgrade/downgrade the `klhk.sys` driver.

If none of that helps, try a newer version of `klhk.sys` – there's more info and updates in [this GitHub issue](https://github.com/iPower/KasperskyHook/issues/4).

For those curious about what the driver checks internally:  
- It verifies the Windows build number and version – some older or specific builds are intentionally blocked (flags like `0x100`, `0x400`, or `0x2` get set).  
- It looks for signatures of other hypervisors by checking CPUID leaves – e.g., `"VMXh"` for VMware, `"Microsoft Hv"` for Hyper-V, or vendor strings for KVM/VirtualBox.  
- It also reads a registry value `UseHvm`; if that's not present, HVM stays disabled. This value is normally set by the KasperskyHookLoader – if you're writing your own loader, you'll need to replicate that.

----

**MAKE SURE TO ENABLE TEST MODE TO TEST THIS PROJECT. IF YOU WISH TO USE IT OUTSIDE TEST MODE, USE YOUR CUSTOM DRIVER LOADER OR SIGN THE DRIVER.**

**NOTE: THIS ISN'T MEANT TO BE AN EASY-TO-PASTE-DETECTION-PROOF PROJECT. I JUST WROTE THIS FOR EDUCATIONAL PURPOSES SO I WON'T BE ADDING ANY HV-HARDENING OR ANTI-DETECTION CODE.**



## Demo

![Demo](demo.gif)
