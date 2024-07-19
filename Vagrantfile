require 'socket'

Vagrant.configure("2") do |config|
  config.vm.guest = :windows

  # Sync the current directory to /kernel-debugging in the VM
  config.vm.synced_folder ".", "/kernel-debugging", owner: "vagrant", group: "vagrant", mount_options: ["dmode=777", "fmode=777"], create: true, automount: true

  # Configure WinRM for communication
  config.vm.communicator = "winrm"
  config.winrm.password = "vagrant"
  config.winrm.username = "vagrant"

  # Obtain the host IP address
  host_ip = Socket.ip_address_list.detect { |intf| intf.ipv4_private? }.ip_address
  puts "Host IP Address: #{host_ip}"

  config.vm.define "win10" do |win10|
    win10.vm.box = "win10"

    # Run bcdedit commands directly in the Vagrantfile using the obtained host IP
    win10.vm.provision "shell", privileged: true, inline: <<-SHELL
      echo "Running bcdedit commands" >> C:\\kernel-debugging\\provision.log
      bcdedit /debug on >> C:\\kernel-debugging\\provision.log 2>&1
      bcdedit /set nointegritychecks on >> C:\\kernel-debugging\\provision.log 2>&1
      bcdedit /set testsigning on >> C:\\kernel-debugging\\provision.log 2>&1
      bcdedit /dbgsettings net hostip:#{host_ip} port:49152 key:1.1.1.1 >> C:\\kernel-debugging\\provision.log 2>&1
      echo "Completed bcdedit commands" >> C:\\kernel-debugging\\provision.log

      echo "Rebooting the system" >> C:\\kernel-debugging\\provision.log
      shutdown /r /t 0 >> C:\\kernel-debugging\\provision.log 2>&1
    SHELL

    # Provision to run the PowerShell script after reboot
    win10.vm.provision "shell", privileged: true, inline: <<-SHELL
      echo "Running PowerShell script" >> C:\\kernel-debugging\\provision.log
      powershell -ExecutionPolicy Bypass -File C:\\kernel-debugging\\Install-Driver.ps1 -ArgumentList 'input.sys' >> C:\\kernel-debugging\\provision.log 2>&1
      echo "Completed PowerShell script" >> C:\\kernel-debugging\\provision.log
    SHELL

    win10.vm.network :forwarded_port, guest: 3389, host: 53389
    win10.vm.network :forwarded_port, guest: 49152, host: 49152
  end
end
