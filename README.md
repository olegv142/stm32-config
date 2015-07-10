stm32-config
============

### Framework for reliable storing of configuration data in STM32 and MSP430 embedded flash

It supports storing of fixed size data in non-volatile storage using 2 flash sectors with atomic updates.
The data is protected by CRC16 checksum. Additional measures are taken to ensure the durability of updates in
the presence of power failures during flash writing and erasing.

#### Major sources

    common\cfg_pool.c
        Configuration data pool over single sector

    common\cfg_storage.c
        Configuration data storage using 2 pools to provide strong consistency

    common\flash_sec.h
        Generic API for flash sector manipulation.
        It abstracts the storage implementation from the platform-specific
        flash write/erase code.

    stm32\Src\cfg_test.c
        Automated tests for configuration storages on STM32 platform

    stm32\Src\flash.c
    stm32\Src\flash_sec.c
        Flash write/erase implementation for STM32 platform

    stm32\Src\cli.c
        Command line interface over USB CDC with plain echo implementation

    stm32\EWARM
        Project for IAR Embedded Workbench for ARM compiler

    stm32\config.ioc
        Project for STM32CubeMX

    msp430\cfg_test.c
        Automated tests for configuration storages on MSP430 platform

    msp430\flash.h
    msp430\flash_sec.c
        Flash write/erase implementation for MSP430 platform

    msp430\cfg_test.eww
        Project for IAR Embedded Workbench for MSP430 compiler

    tests\echo.py
        USB CDC echo test

    doc\cfg_storage.pdf
        Storage design description (in Russian)

The STM32 project is created for STM32-H405 board with STM32F405RGT6 MCU
https://www.olimex.com/Products/ARM/ST/STM32-H405/

The MSP430 project is created for Launch Pad board with MSP430G2553 MCU
http://www.ti.com/tool/msp-exp430g2

