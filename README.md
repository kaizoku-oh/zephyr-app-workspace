# Standalone Golioth Zephyr application

This repo is a playground for experimenting with a standalone Zephyr application for STM32 using Golioth SDK.

## 🚀 Getting started

```bash
# Initialize workspace for the app (main branch)
$ west init -m https://github.com/kaizoku-oh/golioth-zephyr-app --mr main workspace
$ cd workspace

# Update Zephyr modules
$ west update

# Move vscode workspace file from the app to the outer workspace directory
$ Copy-Item –Path app/zephyr-windows.code-workspace -Destination .

# Build app
$ west build app -d app/build -b nucleo_f767zi

# Turn on compilation database
$ west config build.cmake-args -- -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Retrieve `ZEPHYR_SDK_INSTALL_DIR`
$ cmake -P deps/zephyr/cmake/verify-toolchain.cmake

# Open vscode workspace
$ code zephyr-windows.code-workspace

# Open prj.conf and your Golioth PSK credentials

# Build app
$ west build app -d app/build -b nucleo_f767zi

# Flash the app to the board
$ STM32_Programmer_CLI -c port=swd mode=UR -w app/build/zephyr/zephyr.bin 0x08000000
```

Once the app is flashed open a serial monitor like PuTTy and reset your board.

```bash
# Check that you gor an IP address from the network
uart:~$ net iface
Interface 0x20020dc0 (Ethernet) [1]
===================================
Link addr : 00:80:E1:32:32:39
MTU       : 1500
Flags     : AUTO_START,IPv4
Ethernet capabilities supported:
        10 Mbits
        100 Mbits
IPv4 unicast addresses (max 1):
        192.168.137.183 DHCP preferred
IPv4 multicast addresses (max 1):
        <none>
IPv4 gateway : 192.168.137.1
IPv4 netmask : 255.255.255.0
DHCPv4 lease time : 604800
DHCPv4 renew time : 302400
DHCPv4 server     : 192.168.137.1
DHCPv4 requested  : 192.168.137.183
DHCPv4 state      : bound
DHCPv4 attempts   : 1
```
### 🔨 Build footprint

| Memory region | Used Size   | Region Size | %age Used   |
| -----------   | ----------- | ----------- | ----------- |
| FLASH         | 232428 B    | 2   MB      | 11.08%      |
| RAM           | 68956  B    | 384 KB      | 17.54%      |
| QSPI          | 0      GB   | 256 MB      | 0.00%       |
| DTCM          | 12544  B    | 128 KB      | 9.57%       |
| IDT_LIST      | 0      GB   | 2   KB      | 0.00%       |

## ✅ TODO

- [x] Blink board LED

- [x] Print via USB

- [x] Shell over USB

- [x] Add golioth sdk

- [ ] Set Golioth credentials via shell settings

- [ ] Add Github workflow for CI

- [ ] Update the README.md Build footprint automatically when running CI workflow

## 💳 Credits
This project is generated from the [zephyr-vscode-example](https://github.com/beriberikix/zephyr-vscode-example) template by [Jonathan Beri](https://github.com/beriberikix).

It is following the [Golioth](https://github.com/golioth) recommended way to structure a zephyr app using manifest files, more on this in this [blog post](https://blog.golioth.io/improving-zephyr-project-structure-with-manifest-files/).