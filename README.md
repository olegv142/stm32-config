stm32-config
============

### Framework for reliable storing of configuration data in STM32 embedded flash

It supports storing of fixed size data in non-volatile storage using 2 flash sectors with atomic updates.
The data is protected by CRC16 checksum. Additional measures are taken to ensure the durability of updates in
the presence of power failures during flash writing and erasing.

#### Major sources

    stm32\Src\cfg_pool.c
        Configuration data pool over single sector

    stm32\Src\cfg_storage.c
        Configuration data storage using 2 pools to provide strong consistency

    stm32\Src\cfg_test.c
        Automated tests for the above configuration storages

    stm32\Src\flash_sec.c
        Generic API for flash sector manipulation.
        It abstracts the storage implementation from the platform-specific
        flash write/erase code.

    stm32\Src\flash.c
        Flash write/erase implementation

    stm32\Src\cli.c
        Command line interface over USB CDC with plain echo implementation

    stm32\EWARM
        Project for IAR Embedded Workbench for ARM compiler

    stm32\config.ioc
        Project for STM32CubeMX

    tests\echo.py
        USB CDC echo test

The project is created for STM32-H405 board with STM32F405RGT6 MCU
https://www.olimex.com/Products/ARM/ST/STM32-H405/
