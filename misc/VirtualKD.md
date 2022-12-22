# VirtualKD

> These are prelimiary notes about VirtualKD, it is an alternative to the
> vagrant-windbg setup.

VirtualKD is a tool that improves kernel debugging performance when using VMWare
or VirtualBox. It integrates with WinDbg and reduces debugging latency. To use
it, run the Virtual Machine Monitor, select a VM, and press "Run debugger". A
WinDbg window will appear, and you can start debugging the VM.

## Prerequisites

* Visual Studio 2022
* Newest SDK
* Newest WDK

Follow
https://learn.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk to
ensure all prerequisites are met.

## Setup

Follow the full setup from guest and host. Start the debugging through the use
of `vmmon64` (it will start WinDbg). The VM will initially "hang" at the boot
loading fase because it is stuck at an initial breakpoint. Go to `WinDbg` ->
`Debug` and press `Go` if that happens.

### Guest

1) Download VirtualKD-Redux from
   [https://github.com/4d61726b/VirtualKD-Redux](https://github.com/4d61726b/VirtualKD-Redux),
   remember to follow its
   [tutorial](https://github.com/4d61726b/VirtualKD-Redux/blob/master/VirtualKD-Redux/Docs/Tutorial.md).
   Otherwise, follow along right below.

2) Copy the target folder to the guest VM running inside VMware Workstation. If
   the Guest VM is running a 32-bit OS, copy "target32". Otherwise, if it's
   running a 64-bit OS, then copy "target64" instead.

3) Run "vminstall.exe" in the guest VM. If using Windows 10, ensure that
   "Replace kdcom.dll" is checked. Once the installer is complete, allow the
   guest VM to restart.

4) At the boot manager prompt, ensure the "VKD-Redux" entry is selected then
   press F8 for advanced options.

5) Select "Disable Driver Signature Enforcement" and boot the OS.

### Host

6) Ensure the certificate is signed:

    6.1) Click Start, click Start Search, type mmc, and then press ENTER.

    6.2) Click Yes if you get the UAC screen shown below.

    6.3) On the File menu, click Add/Remove Snap-in.

    6.4) Under Available snap-ins, click Certificates, and then click Add.

    6.5) Under This snap-in will always manage certificates for, click Computer
    account, and then click Next. 

    6.6) Click Local computer, and click Finish.

    6.7) If you have no more snap-ins to add to the console, click OK to return
    to the following screen.

    6.8) In the console tree, double-click Certificates.

    6.9) Right-click the Trusted Root Certification Authorities store.

    6.10) Click Import to import the certificates and follow the steps in
    the Certificate Import Wizard.

7) Close VirtualBox if it is running. Run "VirtualBoxIntegration.exe" and click
   "Enable" for each virtual machine that you want to use VirtualKD-Redux with.
   Once completed, you can now run VirtualBox.

> You can use virtual machine snapshots to avoid having to perform these steps
> every time. Once the final step is completed and the OS has successfully
> booted, make a snapshot that can later be restored. It is important to note
> that if a newer version of VirtualKD-Redux is installed on the host, then the
> guest VM must also be updated and vice versa. It is strongly recommended to
> make a new snapshot after upgrading and rebooting the guest VM.
