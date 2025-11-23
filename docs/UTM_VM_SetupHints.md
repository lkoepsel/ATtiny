# UTM/VM Setup Hints
## Create New VM in UTM
*When downloading an ISO, start with minimal version (no Desktop) and install what is needed upon installation.*
1. Go to [cdimage.debian.org/debian-cd/current/arm64/iso-cd/](https://cdimage.debian.org/debian-cd/current/arm64/iso-cd/) [^1]
2. Download `debian-13.x.x-arm64-netinst.iso` (approximately 400-500 MB)
3. Open UTM and click **Create a New Virtual Machine**
4. Choose **Virtualize** (not Emulate)
5. Select **Linux** as the Operating System
6. Click **Browse** and select your downloaded Debian ISO
7. Configure the VM:
   - **Architecture**: aarch64 (ARM64)
   - **System**: QEMU 8.2 ARM Virtual Machine (virt-8.2)
   - **Memory**: 4096 MB recommended
   - **Storage**: 20 GB minimum 
   - **CPU Cores**: 4-6 cores
### Additional VM Settings
Before starting the VM, configure these settings:
1. **Network**:
   - Mode: Shared Network (for internet access during installation)
2. **Drives**:
   - Ensure the ISO is attached as a CD/DVD drive
3. **Input:**
   * Ensure **USB Sharing** is checked
## SSH Access from macOS

1. enable SSH access:
```bash
# On Debian VM
systemctl enable ssh
systemctl start ssh

# Get IP address
ip addr show
```
2. In Mac Terminal:
```bash
ssh username@vm-ip-address
```
## Create public key for Github
```
ssh-keygen -t rsa -b 4096
cat ~/.ssh/id_rsa.pub
# select and cmd-c
# Add to Settings->SSH Keys on github.com
ssh -T git@github.com
```
## Change passwords
```bash
# run passwd command
passwd
```
## If You've Forgotten the Root Password
[How to Reset the Forgotten Root Password in Debian 11](https://thelinuxcode.com/reset-forgotten-root-password-debian/)
If you cannot access root at all, you'll need to boot into recovery mode: [^2][^1]

1. Reboot the system
2. At GRUB menu, press `e` to edit boot parameters
3. Find the line starting with `linux` and add `init=/bin/bash` at the end
4. Press `Ctrl+X` or `F10` to boot
5. Remount filesystem as read-write:
   ```bash
   mount -o remount,rw /
   ```
6. Change the password:
   ```bash
   passwd
   ```
7. Reboot:
   ```bash
   exec /sbin/init
   ```

## Fixing "sudo: command not found"
This error occurs because **sudo is not installed by default** on minimal Debian installations. 
```bash
su -                           # Become root
apt update && apt install sudo -y
usermod -aG sudo <yourusername>  # Add user to sudo group
exit                           # Exit root
logout                         # Log out completely
```
## If VM continues to boot to install
**Clear ISO While VM is Running**
1. In the UTM toolbar while the VM window is active
2. Click the **CD/DVD icon** in the top menu bar
3. Select **Clear** or **Eject** 
4. Restart the VM
