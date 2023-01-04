# IDA Remote Debugging guide

The following has been tested running a REMnux VM inside a FlareVM.

## Host settings

### VM Settings 

* Network: Host-Only adapter
* Nested virtualization: Enabled

### IDA settings

* Remote Linux/Windos debugger
* Debug process options:
  * Application: `/path/to/app`
  * Input file: `/path/to/app`
  * Directory: `/path/to/`

  * Hostname: Found by running `ip addr` on guest machine, then retrieve `inet` (e.g.: `inet 192.168.56.102/24 brd 192.168.56.255 scope global dynamic enp0s3`)
  * Port: `23946`

### Transfer files from host to guest
  * Transfer IDA debug server files to guest machine. Find them at `C:\path\to\IDA\dbgsrv`
  * Transfer file to be analyzed to guest machine.

### Guest VM settings

* Start server. Example below where an amd64 .elf file is being ran on the guest machine:  

```
$ cd ~/Documents && chmod +x linux_server64 && ./linux_server64 -v
IDA Linux 64-bit remote debug server(ST) v1.22. Hex-Rays (c) 2004-2017
Listening on 0.0.0.0:23946...
```

* Give file to be analyzed execute permission: `chmod +x file-to-be-analyzed`

## References 

* https://hex-rays.com/products/ida/support/freefiles/remotedbg.pdf
* https://eviatargerzi.medium.com/remote-debugging-with-ida-from-windows-to-linux-4a98d7095215
