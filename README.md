# Standalone STM32 Zephyr application

This repo can be used as a template and a playground for experimenting with a standalone Zephyr application for STM32.

## 🚀 Getting started

```bash
# Initialize workspace for the app (main branch)
$ west init -m https://github.com/kaizoku-oh/stm32-zephyr-app --mr main workspace
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

![image](https://github.com/kaizoku-oh/stm32-zephyr-app/assets/22129291/b1eca6ce-78d9-469e-8675-fe2e84a79f1e)

You can run the tasks in the following order clean -> build -> erase -flash

![image](https://github.com/kaizoku-oh/stm32-zephyr-app/assets/22129291/318b48ac-2423-480a-a4a7-7e031f2e9456)

Once the app is flashed open a serial monitor like PuTTy and reset your board.

![image](https://github.com/kaizoku-oh/stm32-zephyr-app/assets/22129291/ccf05b52-55c3-415d-a4ec-01410bd57744)

## 🔨 Build footprint for NUCLEO-L476RG

| Memory region | Used Size   | Region Size | %age Used   |
| ------------- | ----------- | ----------- | ----------- |
| FLASH         | 41388  B    | 1   MB      | 3.95%       |
| RAM           | 12568  B    | 96  KB      | 12.78%      |
| IDT_LIST      | 0      GB   | 2   KB      | 0.00%       |

## ✅ ToDo

- [x] Blink board LED

- [x] Shell over UART

- [ ] Fix vscode launch with cortex-debug

<<<<<<< HEAD
- [x] Add multithreading code
=======
- [ ] Add multithreading code
>>>>>>> b5a03695368ac9f4f7326c1eac9bd9ec61a37b60

- [ ] Add application events manager

- [ ] Add die temp sensor app

- [ ] Add Github CI workflow

- [ ] Update the README.md Build footprint table automatically when running CI workflow

- [ ] Add GitHub badges to readme

- [ ] Add OTA update example

- [ ] Enable C++20

## 💳 Credits
This project is generated from the [zephyr-vscode-example](https://github.com/beriberikix/zephyr-vscode-example) template by [Jonathan Beri](https://github.com/beriberikix).

It is following the [Golioth](https://github.com/golioth) recommended way to structure a zephyr app using manifest files, more on this in this [blog post](https://blog.golioth.io/improving-zephyr-project-structure-with-manifest-files/).