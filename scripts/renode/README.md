# Generate a REPL file for your board

```bash
# Generate flatened dts for nucleo_f767zi board
(zephyr-venv) $ gcc -H -E -P -x assembler-with-cpp \
    -I ../../../deps/zephyr/include \
    -I ../../../deps/zephyr/dts \
    -I ../../../deps/zephyr/dts/arm \
    -I ../../../deps/zephyr/dts/common \
    -I ../../../deps/modules/hal/stm32/dts \
    ../../../deps/zephyr/boards/arm/nucleo_f767zi/nucleo_f767zi.dts 1>flat.dts 2>includes.txt

# Install dts2repl
(zephyr-venv) $ pip3 install git+https://github.com/antmicro/dts2repl.git

# Generate repl file
(zephyr-venv) $ dts2repl flat.dts --output nucleo_f767zi.repl
```
