# Standalone Zephyr application

[![GitHub Build workflow status](https://github.com/kaizoku-oh/zephyr-app-workspace/workflows/Build/badge.svg)](https://github.com/kaizoku-oh/zephyr-app-workspace/actions/workflows/build.yaml)
[![GitHub release](https://img.shields.io/github/v/release/kaizoku-oh/zephyr-app-workspace)](https://github.com/kaizoku-oh/zephyr-app-workspace/releases)
[![GitHub issues](https://img.shields.io/github/issues/kaizoku-oh/zephyr-app-workspace)](https://github.com/kaizoku-oh/zephyr-app-workspace/issues)
![GitHub top language](https://img.shields.io/github/languages/top/kaizoku-oh/zephyr-app-workspace)
[![License](https://img.shields.io/github/license/kaizoku-oh/zephyr-app-workspace)](https://github.com/kaizoku-oh/zephyr-app-workspace/blob/main/LICENSE)

This repo can be used as a template and a playground for experimenting with a standalone Zephyr application.

## 🚀 Getting started

```bash
# Initialize workspace for the app (main branch)
$ west init -m https://github.com/kaizoku-oh/zephyr-app-workspace --mr main workspace
$ cd workspace

# Update Zephyr modules
$ west update

# Copy vscode workspace file from the app to the outer workspace directory
$ Copy-Item –Path app/zephyr-windows.code-workspace -Destination .

# Build the app
$ west build app -d app/build -b nucleo_f767zi

# Turn on compilation database
$ west config build.cmake-args -- -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Retrieve `ZEPHYR_SDK_INSTALL_DIR`
$ cmake -P deps/zephyr/cmake/verify-toolchain.cmake

# Open vscode workspace
$ code zephyr-windows.code-workspace
```
Once vscode is open you can run your workspace tasks like the following

![image](https://github.com/kaizoku-oh/zephyr-app-workspace/assets/22129291/b1eca6ce-78d9-469e-8675-fe2e84a79f1e)

You can run the tasks in the following order Clean => Build => Erase => Flash

![image](https://github.com/kaizoku-oh/zephyr-app-workspace/assets/22129291/08cda574-0ea5-4c34-8598-d53e3c5c96de)

Once the app is flashed open a serial monitor like PuTTY and reset your board.

![image](https://github.com/kaizoku-oh/zephyr-app-workspace/assets/22129291/d1f073a3-197b-40f9-8e73-649a705cd287)

## 🔨 Build footprint for NUCLEO-F767ZI

| Memory region | Used Size   | Region Size | %age Used   |
| ------------- | ----------- | ----------- | ----------- |
| FLASH         | 149460 B    | 2   MB      | 7.13%       |
| RAM           | 42592  B    | 384 KB      | 10.83%      |
| QSPI          | 0     GB    | 256 MB      | 0.00%       |
| DTCM          | 12544  B    | 128 KB      | 9.57%       |
| IDT_LIST      | 0     GB    | 2   KB      | 0.00%       |

## ✅ ToDo

- [x] Blink board LED

- [x] Shell over UART

- [ ] Add a script to download and setup the project (intellisence, structure, venv...)

- [x] Fix vscode launch with cortex-debug

- [x] Add multithreading code

- [ ] Add application events manager

- [x] Add die temp sensor class

- [x] Add Github CI workflow

- [ ] Update the README.md Build footprint table automatically as a post build

- [x] Add GitHub badges to readme

- [ ] Add OTA update example

- [x] Enable C++20

- [ ] Add lambda callback to Button class

- [x] Add lambda callback to Serial class

- [x] Add a simple Network manager class

- [ ] Add a usage of the active objects pattern

- [ ] Add a usage of a state machine using SMF

- [ ] Add Local class for managing local time and geo-location

- [ ] Add HTTP client class

- [ ] Add MQTT client class

- [ ] Add NVS config class

- [ ] Add an out-of-tree driver

## 💳 Credits
This project is generated from the [zephyr-vscode-example](https://github.com/beriberikix/zephyr-vscode-example) template by [Jonathan Beri](https://github.com/beriberikix).

It is following the [Golioth](https://github.com/golioth) recommended way to structure a zephyr app using manifest files, more on this in this [blog post](https://blog.golioth.io/improving-zephyr-project-structure-with-manifest-files/).
