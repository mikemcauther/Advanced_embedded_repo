# CSSE4011 Advanced Embedded Systems 

# 	Git Repo



###### Author: KO-CHEN CHI, s4527438

 

[TOC]

​	

### Pracs: 

​	[Prac1](### Prac1) (2020/04/03)

​	[Prac2](### Prac2) (2020/04/24)

### Folder Structure:

- root/
  - common/
  - ei-changes/
  - myoslib/
  - pracs/
- add_diff_files_into_ei_changes.sh
- buildenv.sh
- cp_py_into_pyboard.sh
- diff_ei_changes.sh
- overrite_diff_files_of_ei_changes.sh
- overrite_diff_files_of_ei_freertos.sh

### Environment Install Guide:

#### 	Step 1: 

```
./buildenv.sh
```

- ​	Result:  It will clone ei-freertos and generate necessary environment variables.

- ​	Generate file: 
  - ./common/proj_common.mk
    - This is the included header file for the Makefile of each pracs(prac1/Makefile , prac2/Makefile ...) 
  - ./env_set.sh
    - Executing this file to generate global variables for toolchain, STLINK ... etc.		

#### 	Step 2:

```
./ei-freertos/repo_setup.sh
```

#### 	Step 3:

```
. ./env_set.sh
```

​	Set the necessary variables into self shell.

### 

### Pracs: 

#### 	Prac1 (2020/04/03)



#### 	Prac2 (2020/04/24)

- ##### Design Task 1A: SCU Development Environment Installation and Integration

  - (Done)

- ##### Design Task 2A: Basic Example

  - Result files: 
    - myoslib/scu/main.py 
    - myoslib/scu/led.py
  - (Done)

- ##### Design Task 1B: Host Controller Interface (HCI)

  - (Done)

- ##### Design Task 2B: AHU HCI HAL/OS MyOsLib Implementation

  - (Done)

- ##### Design Task 3B: AHU HCI MyOsLib Implementation

  - (Done)

- ##### PART C Tasks

  - (Not Yet)

- ##### Foler Structer View

  - root/
    - common/
    - ei-changes/
    - myoslib/
      - hci/
        - s4527438_cli_hci.c
        - s4527438_cli_hci.h
        - s4527438_hal_hci.c
        - s4527438_hal_hci.h
        - s4527438_hci_packet.h
        - s4527438_os_hci.c
        - s4527438_os_hci.h
      - scu/
        - hci.py
        - led.py
        - main.py
    - pracs/
      - prac2/
        - inc/
        - src/
          - prac2.c
        - Makefile

- ##### Test Command:

  - ```
    i2creg r 3 0x28
    ```

    ```
    i2creg w 5 0x29 0x04
    ```

  - ```
    lsm6dsl r x
    ```

  - ```
    lsm6dsl r y
    ```

    

- ##### Topic: 

  - HCI:

    - Signals

      |      | Argon                      | BL Board        |
      | ---- | -------------------------- | --------------- |
      |      | (P0.08 UART1_RX) -- Pin 10 | CN3 -- D1 --TX  |
      |      | (P0.06 UART1_TX) -- Pin 9  | CN3 -- D0 -- RX |
      |      | GND                        | CN2 -- GND      |

    - Hardware:

      - Argon
      
      - B-L475E-IOT01A1
      
      - Feather Board
      
      - Analog Discovery 2
      
        |                    |                    |       | Feather Board ==> |      |      |      |      |      |      |
        | ------------------ | ------------------ | ----- | ----------------- | ---- | ---- | ---- | ---- | ---- | ---- |
        |                    |                    |       | UART              |      |      |      | D2   |      | A2   |
        |                    |                    |       | TX                | RX   | 3V3  | GND  | D2   | D3   | GND  |
        | Argon              | P0.06 TX -- Pin 9  |       | X                 |      |      |      |      |      |      |
        |                    | P0.08 RX -- Pin 10 |       |                   | X    |      |      |      |      |      |
        |                    | GND                |       |                   |      |      | X    |      |      |      |
        |                    | 3V3                |       |                   |      | X    |      |      |      |      |
        |                    | D2                 |       |                   |      |      |      |      |      |      |
        | Feather (selfloop) | D2                 | D2    |                   | X    |      |      |      |      |      |
        |                    |                    | D3    | X                 |      |      |      |      |      |      |
        |                    |                    | 3V3   |                   |      | X    |      |      |      |      |
        |                    |                    | GND   |                   |      |      | X    |      |      |      |
        |                    | UART               | TX    | =                 |      |      |      |      | x    |      |
        |                    |                    | RX    |                   | =    |      |      | x    |      |      |
        |                    |                    | 3V3   |                   |      | =    |      |      |      |      |
        |                    |                    | GND   |                   |      |      | =    |      |      |      |
        | B-L475E-IOT01A1    | CN3                | D0 RX |                   |      |      |      |      | X    |      |
        |                    |                    | D1 TX |                   |      |      |      | X    |      |      |
        |                    | CN2                | GND   |                   |      |      |      |      |      | X    |
        | Analog Discovery 2 | Channel 0 (DIO 0)  |       |                   |      |      |      |      | X    |      |
        |                    | CHannel 8 (DIO 8)  |       |                   |      |      |      | X    |      |      |
        |                    | GND                |       |                   |      |      |      |      |      | X    |
      
        

  - I2C

    - Data Fields

      |                      | SID      (8-bits) | I2C Address include R/W bit     (8-bits) | Register Address            (8-bits) | Register values (for write) (8-bits) |
      | -------------------- | ----------------- | ---------------------------------------- | ------------------------------------ | ------------------------------------ |
      | Read  LSM6DSL        | 1                 | 0xD5                                     | (don't care)                         | (no field)                           |
      | Write LSM6DSL        | 1                 | 0xD4                                     | (don't care)                         | (don't care)                         |
      | Read LIS3MDL         | 2                 | 0x3D                                     | (don't care)                         | (no field)                           |
      | Write LIS3MDL        | 2                 | 0x3C                                     | (don't care)                         | (don't care)                         |
      | Read LPS22HB         | 3                 | 0xBB                                     | (don't care)                         | (no field)                           |
      | Write LPS22HB        | 3                 | 0xBA                                     | (don't care)                         | (don't care)                         |
      | Read VL53L0X         | 4                 | 0x53                                     | (don't care)                         | (no field)                           |
      | Write VL53L0X        | 4                 | 0x52                                     | (don't care)                         | (don't care)                         |
      | Read HTS221          | 5                 | 0xBF                                     | (don't care)                         | (no field)                           |
      | Write HTS221         | 5                 | 0xBE                                     | (don't care)                         | (don't care)                         |
      | Read LSM6DSL x value | 6                 | 0xD5                                     | 0x00                                 | x                                    |
      | Read LSM6DSL y value | 6                 | 0xD5                                     | 0x00                                 | y                                    |
      | Read LSM6DSL z value | 6                 | 0xD5                                     | 0x00                                 | z                                    |
      |                      |                   |                                          |                                      |                                      |

      



### Reference List:

​	[1] Micropython: http://docs.micropython.org/

​    [2] LSM6DSL register value: https://github.com/hanyazou/nuttx/blob/8e18e8ae54f7faeb4712b615dea59f4a89bdedf2/include/nuttx/sensors/lsm6dsl.h

   

















































[Go to Real Cool Heading section](#real-cool-heading)









