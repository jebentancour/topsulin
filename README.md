# Topsulin

![topsulin](images/topsulin.png)

## Software

### BLE Services and Characteristics

|GATT Services |GATT Characteristics            |Description                              |
|--------------|--------------------------------|-----------------------------------------|
|**Glucose**   |0x1808                          |Servicio estándar para glucómetros       |
|0x2A18        |**Glucose Measurement**         |Valores de glucemia                      |
|0x2A34        |**Glucose Measurement Context** |Información adicional (CHO e insulina)   |
|0x2A51        |**Glucose Feature**             |Información sobre features soportadas    |
|0x2A52        |**Record Access Control Point** |Punto de control de acceso del registro  |
|**Topsulin**  |0xF65D                          |Servicio específico de Topsulin          |
|0xF65E        |**Configuration**               |Configuración de preferencias de usuario |
|0xF65F        |**Name**                        |Nombre para mostrar                      |
|0xF660        |**Time**                        |Fecha y hora                             |
|0xF661        |**Calculator**                  |Parámetros pra el calculador de bolos    |
|0xF662        |**Insulin**                     |Tipo, capacidad y cantidad restante      |

Base UUID para Topsulin Services y Characteristics 0x3419-XXXX-1505-31A7-EC4D-449B-0752-1104

#### Glucose Service

Según el servicio estándar [Glucose Service Specification](https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.service.glucose.xml)

#### Topsulin Service

|Characteristic                                      |Format |Value                              |
|----------------------------------------------------|-------|-----------------------------------|
|**Configuration**                                   |       |                                   |
|Registro de glucemia (si o no)                      |bit0   |0 No, 1 Si                         |
|Registro de insulina (si o no)                      |bit1   |0 No, 1 Si                         |
|Registro de CHO (si o no)                           |bit2   |0 No, 1 Si                         |
|Invertir los colores de la pantalla                 |bit3   |0 Fondo blanco, 1 Fondo negro      |
|Unidades de glucemia (kg/L o mol/L)                 |bit4   |0 kg/L, 1 mol/L                    |
|Calculador de bolo (activado o desactivado)         |bit5   |0 Desactivado, 1 Activado          |
|Orientación de pantalla (diestro o zurdo)           |bit6   |0 Diestro, 1 Zurdo                 |
|Unidades de CHO (gramos o porciones)                |bit7   |0 CHO en g, 1 CHO en porciones     |
|Equivalencia entre gramos y porciones               |uint16 |Peso en g de una porción           |
|**Name**                                            |       |                                   |
|Nombre para mostrar                                 |uint8  |String de largo máximo 20 UTF-8    |
|**Time**                                            |       |                                   |
|Hora y fecha del dispositivo                        |       |[Time Characteristic](https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.date_time.xml) |
|**Insulin**                                         |       |                                   |
|Tipo de insulina                                    |uint8  |Medication ID from Glucose Service |
|Capacidad de la lapicera                            |uint16 |U de insulina en una lapicera nueva|
|Capacidad restante de la lapicera                   |uint16 |U de insulina restantes            |


## Board

![pinout](images/Core51822-B-pin.jpg)

|nRF pin|Use  |
|-------|-----|
|30     |A    |
|01     |B    |
|03     |SCL  |
|05     |SDA  |
|09     |SW   |
|21     |LED  |
|20     |MISO |
|19     |MOSI |
|18     |CLK  |
|22     |CS   |
|23     |DC   |
|24     |RST  |
|25     |BUSY |

## SoC

SoC [nRF51822](https://www.nordicsemi.com/eng/nordic/download_resource/62726/14/39584073/13358) QFACA10 (256kB flash, 32 kB RAM)

[INFOCENTER](http://infocenter.nordicsemi.com)

|ID |Base address |Peripheral  |Instance    |Description                                  |Used by           | 
|---|-------------|------------|------------|---------------------------------------------|------------------|
|0  |0x40000000   |POWER       |POWER       |Power Control.                               |SoftDevice        |
|0  |0x40000000   |CLOCK       |CLOCK       |Clock Control.                               |SoftDevice        |
|0  |0x40000000   |MPU         |MPU         |Memory Protection Unit.                      |SoftDevice        |
|1  |0x40001000   |RADIO       |RADIO       |2.4 GHz Radio.                               |SoftDevice        |
|2  |0x40002000   |UART        |UART0       |Universal Asynchronous Receiver/Transmitter. |                  |
|3  |0x40003000   |SPI         |SPI0        |SPI Master.                                  |EPD               |
|3  |0x40003000   |TWI         |TWI0        |I2C compatible Two-Wire Interface 0.         |EPD               |
|4  |0x40004000   |SPIS        |SPIS1       |SPI Slave.                                   |                  |
|4  |0x40004000   |SPI         |SPI1        |SPI Master.                                  |                  |
|4  |0x40004000   |TWI         |TWI1        |I2C compatible Two-Wire Interface 1.         |                  |
|6  |0x40006000   |GPIOTE      |GPIOTE      |GPIO Task and Events.                        |                  |
|7  |0x40007000   |ADC         |ADC         |Analog to Digital Converter.                 |                  |
|8  |0x40008000   |TIMER       |TIMER0      |Timer/Counter 0.                             |SoftDevice        |
|9  |0x40009000   |TIMER       |TIMER1      |Timer/Counter 1.                             |                  |
|10 |0x4000A000   |TIMER       |TIMER2      |Timer/Counter 2.                             |                  |
|11 |0x4000B000   |RTC         |RTC0        |Real Time Counter 0.                         |SoftDevice        |
|12 |0x4000C000   |TEMP        |TEMP        |Temperature Sensor.                          |SoftDevice        |
|13 |0x4000D000   |RNG         |RNG         |Random Number Generator.                     |SoftDevice        |
|14 |0x4000E000   |ECB         |ECB         |Crypto AES ECB.                              |SoftDevice        |
|15 |0x4000F000   |CCM         |CCM         |AES Crypto CCM.                              |SoftDevice        |
|15 |0x4000F000   |AAR         |AAR         |Accelerated Address Resolver.                |SoftDevice        |
|16 |0x40010000   |WDT         |WDT         |Watchdog Timer.                              |                  |
|17 |0x40011000   |RTC         |RTC1        |Real Time Counter 1.                         |Timer library     |
|18 |0x40012000   |QDEC        |QDEC        |Quadrature Decoder.                          |                  |
|19 |0x40013000   |LPCOMP      |LPCOMP      |Low Power Comparator.                        |                  |
|20 |0x40014000   |SWI         |SWI0        |Software interrupt.                          |Timer library     |
|21 |0x40015000   |SWI         |SWI1        |Software interrupt.                          |SoftDevice        |
|22 |0x40016000   |SWI         |SWI2        |Software interrupt                           |SoftDevice        |
|23 |0x40017000   |SWI         |SWI3        |Software interrupt.                          |                  |
|24 |0x40018000   |SWI         |SWI4        |Software interrupt.                          |SoftDevice        |
|25 |0x40019000   |SWI         |SWI5        |Software interrupt.                          |SoftDevice        |
|30 |0x4001E000   |NVMC        |NVMC        |Non-Volatile Memory Controller.              |SoftDevice        |
|31 |0x4001F000   |PPI         |PPI         |Programmable Peripheral Interconnect.        |                  |
|NA |0x50000000   |GPIO        |GPIO        |General Purpose Input and Output.            |GPIO              |
|NA |0x10000000   |FICR        |FICR        |Factory Information Configuration Registers. |SoftDevice        |
|NA |0x10001000   |UICR        |UICR        |User Information Configuration Registers.    |SoftDevice        |
