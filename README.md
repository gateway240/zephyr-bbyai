# Zephyr BeagleBoardY-AI R5 Development

> NOTE: [This guide](https://opencoursehub.cs.sfu.ca/bfraser/grav-cms/cmpt433/guides/files_byai/R5Guide.pdf) is the basis for much of this code and many of these directions. Thanks Brian Fraser!

## Setting up Development environment
Install toolchain
```bash
cd ~
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.17.0/zephyr-sdk-0.17.0_linux-x86_64_minimal.tar.xz
tar xvf zephyr-sdk-0.17.0_linux-x86_64_minimal.tar.xz
rm zephyr-sdk-0.17.0_linux-x86_64_minimal.tar.xz

cd ~/zephyr-sdk-0.17.0
wget https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v0.17.0/toolchain_linux-x86_64_arm-zephyr-eabi.tar.xz
tar xvf toolchain_linux-x86_64_arm-zephyr-eabi.tar.xz
rm toolchain_linux-x86_64_arm-zephyr-eabi.tar.xz
```
Setup workspace
```bash
cd ~/zephyr-bbyai
python -m venv .venv
source .venv/bin/activate
pip install west
west init -l manifest
west update # Installs 1.5GB of modules.
west zephyr-export
pip install -r zephyr/scripts/requirements.txt
source zephyr/zephyr-env.sh
west sdk install -t arm-zephyr-eabi
```

## Building Code
Blinky example
```bash
west build --pristine -b beagley_ai/j722s/mcu_r5f0_0 blinky
```

Potentiometer ADC example
```bash
west build --pristine -b beagley_ai/j722s/mcu_r5f0_0 potentiometer-ADC-R5
```

RSC table for main processor
```bash
cd zephyr
west build  -b beagley_ai/j722s/main_r5f0_0 --p always samples/subsys/ipc/openamp_rsc_table
```

copy the file

```bash
scp ./build/zephyr/zephyr.elf <USER>@192.168.7.2:~/firmware/zephyr_mcu.elf
```
ssh to the board

ON THE BEAGLEBOARD:

Need to configure GPIO in Linux for now
```bash
gpioset gpiochip0 0=0
gpioget gpiochip0 9
gpioset gpiochip0 10=1
```

For MCU Domain:
```bash
sudo cp ~/firmware/zephyr_mcu.elf /lib/firmware
sudo echo zephyr_mcu.elf | sudo tee /sys/class/remoteproc/remoteproc2/firmware
echo start | sudo tee /sys/class/remoteproc/remoteproc2/state
```


For Main Domain:
```bash
sudo cp ~/firmware/zephyr_main.elf /lib/firmware
sudo echo zephyr_main.elf | sudo tee /sys/class/remoteproc/remoteproc4/firmware
echo start | sudo tee /sys/class/remoteproc/remoteproc4/state
```