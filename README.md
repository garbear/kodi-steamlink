# Kodi on the Steam Link

This repo contains the Steam Link port of Kodi.

## Installation

1. Download the latest release from https://github.com/garbear/kodi-steamlink/releases.
2. Copy archive to the folder `steamlink/apps/` on a USB drive (FAT32 or EXT4).
3. Insert the drive into the Steam Link and power-cycle. It will take approximately 2 minutes to install the 80MB archive.
4. When the Steam Link boots, Kodi should appear on the home screen.

## Building Kodi

To build Kodi, follow the steps at: https://github.com/ValveSoftware/steamlink-sdk/tree/master/examples/kodi

## Debugging

Kodi may crash when launched from the Steam Link homescreen due to a PulseAudio conflict. Until this is fixed, Kodi must be run from the command line.

#### Enable SSH

1. Create a non-empty file on the USB drive at `steamlink/config/system/enable_ssh.txt`
2. Insert the drive and power on the Steam Link
3. SSH into the Steam Link as root using an SSH client, e.g. `ssh root@10.0.0.103`
4. The password is `steamlink`

#### Changing Steam Link builds

If you need to test Kodi on a build different from the latest public beta, follow these steps:

1. Place the build number (e.g. `600`) in a text file on a USB drive at `steamlink/config/system/update_branch.txt`
2. Insert the drive and power on the Steam Link

To revert back to the latest public beta build:

1. Place the word `beta` in a text file on a USB drive at `steamlink/config/system/update_branch.txt`
2. Insert the drive and power on the Steam Link

#### Kill the Steam Link launcher and launch Kodi

If you are on the **public build (566)**, enter the commands:

```bash
killall powermanager.sh powermanager app_run.sh shell

cd /home/apps/kodi && ./kodi.sh
```

If you are on the **beta build (597 or later)**, enter the commands:

```bash
killall powermanager.sh powermanager shell.sh shell

cd /home/apps/kodi && ./kodi.sh
```

#### Viewing the log

If you launched Kodi from the **Steam Link launcher**, use the command:

```bash
tail -F /home/apps/kodi/.home/.kodi/temp/kodi.log
```

If you launched Kodi from the **command line**, use the command:

```bash
tail -F /home/steam/.kodi/temp/kodi.log
```

#### Remote debugging via GDB

To get a stack trace, run `gdbserver` on the Steam Link and connect via local `gdb`. This is explained here: https://github.com/ValveSoftware/steamlink-sdk

`build_steamlink.sh` can be modified to make this easier. See [this commit](https://github.com/garbear/steamlink-sdk/commit/kodi-debug%5E) and [this commit](https://github.com/garbear/steamlink-sdk/commit/kodi-debug).

Before building Kodi, modify the startup script to run `gdbserver`. Find the line with the Kodi command and prefix it with `gdbserver :8080 ` like [this](https://github.com/garbear/kodi-steamlink/commit/steamlink-gdb).

#### Debugging locally using Valgrind

To track down a memory corruption bug, perform a "depends" build of Kodi. This will use the same libraries as the Steam Link instead of relying on system libraries.

First, create a writable folder `/opt/kodi-deps`. Then clone this repo and run:

```bash
cd tools/depends
./bootstrap
./configure --with-toolchain=/usr --prefix=/opt/kodi-deps --host=x86_64-linux-gnu
make
cd ../..
```

Before compiling Kodi, modify the startup script to launch Kodi under Valgrind. You should prefix the Kodi command with `valgrind --leak-check=yes ` like [this](https://github.com/garbear/kodi-steamlink/commit/steamlink-valgrind).

Next, compile Kodi via CMake:

```bash
mkdir build
cd build
/opt/kodi-deps/x86_64-linux-gnu-native/bin/cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_TOOLCHAIN_FILE=/opt/kodi-deps/x86_64-linux-gnu/share/Toolchain.cmake \
    -DCMAKE_INSTALL_PREFIX=$HOME/kodi-steamlink \
    ../project/cmake
make
make install
```

You can substitute the installation folder (`$HOME/kodi-steamlink`) for a prefix of your choice.

Finally, run the resulting startup script at `$HOME/kodi-steamlink/bin/kodi`.
