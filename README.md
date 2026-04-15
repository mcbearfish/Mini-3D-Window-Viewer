# 3D Window Project
# Table of Contents
-[About](#about)<br />
-[Project Description](#Project-Description)<br />
-[Demo Video](#Demo-Video)<br />
-[Circuit Design](#Circuit-Design)<br />
-[Box Design](#Box-Design)<br />
-[Code Design](#Code)<br />
-[Reflections](#Reflections)

## About
This is a developmental project intended to refresh my skills on designing and programming an STM32 microcontroller board. I chose a project scope that I believed would provide difficult circuit and programming challenges. I will document the development process in this ReadMe.<br />

If you stumble across this project feel free to use it for educationl purposes. If you find errors or have questions about the circuit design or code feel free to reach out. Please cite this repository as a source if you plan to use it.<br />

## Project Description
This project is the 3D Window Viewer.<br />

When placed on a flat surface, the device on startup will create a window into a 3D Scene of a hammer (Boiler Up and Hail Purdue!). Moving and tilting the device will change the camera angle of the hammer in an anchored position .<br />

This project uses accelerometer, gyroscope, and magnetometer IMU sensors (I2C) to simulate the camera view of the 3D object where moving/tilting the box changes camera view direction realistically, but position is anchored (C Code). I Prototyped IMU-only position tracking, identified drift as limiting factor and redesigned around constrained motion. <br />

I designed my own custom STM32 development board and sensor board using KiCad Schematic and PCB Design Software. <br />

## Demo Video
[![Everything Is AWESOME](https://img.youtube.com/vi/Y7tffPQoLfw/hqdefault.jpg)](https://www.youtube.com/watch?v=Y7tffPQoLfw "Everything Is AWESOME")

## Circuit Design
#### STM32 Circuit
<img width="568" height="762" alt="image" src="https://github.com/user-attachments/assets/303fddad-d7d7-4e21-b84f-9480f5f74530" /><br />
This STM32 chip was chosen for its large size of pinouts and processing capabilities.

#### Button
<img width="395" height="284" alt="image" src="https://github.com/user-attachments/assets/5bae4b5d-869c-40eb-963d-ae59cf6c7e4d" /><br />
Simple Button circuit to test user inputs. Will be used later to 'zero' location of 3D object in virtual space.

#### LEDs
<img width="230" height="294" alt="image" src="https://github.com/user-attachments/assets/852c60a7-4242-4d3a-8030-189382736d24" /><br />
Will be used to for inital testing and debugging.

#### Crystal Oscciallator
<img width="437" height="283" alt="image" src="https://github.com/user-attachments/assets/d24f10df-7c6e-48a1-9470-f4daef24d1ee" /><br />
Provide a highly stable, accurate, and precise frequency reference.

#### USB-C Connector
<img width="532" height="365" alt="image" src="https://github.com/user-attachments/assets/a12726bb-a4ad-47a6-8967-1e452fd64ac1" /><br />
Provides power to board.

#### Boot Switch
<img width="235" height="160" alt="image" src="https://github.com/user-attachments/assets/f280e3e0-6fc7-4074-b2d5-349e042cced1" /><br />
Reset switch.

#### Mounting Holes
<img width="216" height="184" alt="image" src="https://github.com/user-attachments/assets/3dc0a0be-8cf3-4cd5-a20a-b994358961eb" /><br />
Grounded pads to use for mounting and probe testing.

#### Pin Connectors
<img width="692" height="404" alt="image" src="https://github.com/user-attachments/assets/e253e8ae-4975-4d4d-a1a5-3d6586c90698" /><br />

#### Buck Converter
<img width="657" height="296" alt="image" src="https://github.com/user-attachments/assets/c418947c-6b7b-45a5-82bc-9ea7d88593ff" /><br />
Chosen to smooth 5V to 3.3V for STM32 chip and sensors.

#### Accelerometer and Gyroscope
<img width="751" height="352" alt="image" src="https://github.com/user-attachments/assets/b40b9d26-4685-462b-91da-61180e5c4c62" /><br />
Chosen to track real time postion of board as it moves around in space.

#### Magnetometer
<img width="786" height="343" alt="image" src="https://github.com/user-attachments/assets/e504e925-99b6-4897-b088-bd03f4eafa38" /><br />
Chosen to help track real time postion of board as it moves around in space. Will help to correct any drift when running accelermeter and gyroscope for long times.

#### Pin Connector
<img width="611" height="399" alt="image" src="https://github.com/user-attachments/assets/15865bf0-0dd7-4b5c-87a7-4108525f2891" /><br />

#### Mounting Holes
<img width="315" height="238" alt="image" src="https://github.com/user-attachments/assets/3162fb25-4d3a-4de8-ade6-60f83bad134b" /><br />

#### Notes
I found Google's Gemini AI tool to be extremely helpful during this design process. Especially with mapping the pinout of the display to STM32. I used a cheap knockoff with no documentation and poor pinout labels. In the future may need to remap pins if display fails to work.See ARSensors project for 2nd Board meant to connect sensors to this main board.<br />

Manfucaturing files can be found for each board in this repository.<br />

#### Main PCB
<img width="682" height="646" alt="image" src="https://github.com/user-attachments/assets/9e0afbe1-164e-45d2-b050-79085eeee85b" /><br />

<img width="924" height="828" alt="image" src="https://github.com/user-attachments/assets/3d0f7357-d871-41f3-9d02-1b833b7dc475" /><br />

<img width="924" height="828" alt="image" src="https://github.com/user-attachments/assets/b82c7a2e-9e28-4dab-b74b-71838d88a736" /><br />

<img width="924" height="828" alt="image" src="https://github.com/user-attachments/assets/83dcfbf9-5c89-470f-a855-a063bfe16aad" /><br />

#### Sensors PCB
<img width="476" height="511" alt="image" src="https://github.com/user-attachments/assets/01913261-0ae3-470a-b881-d836e78dffa2" /><br />

<img width="712" height="757" alt="image" src="https://github.com/user-attachments/assets/bd685a8c-0c97-4eeb-9e0f-ecbf7dc5b131" /><br />

<img width="712" height="757" alt="image" src="https://github.com/user-attachments/assets/cf4fe37a-f3e2-44ea-a555-c659f44f0613" /><br />

<img width="712" height="757" alt="image" src="https://github.com/user-attachments/assets/0e8da07b-4ea0-4321-ae73-0175a2d32cb0" /><br />

## Code

#### Code Flowchart


## Reflections
### Circuit Design Version 1
#### Design Process
Designing my first board in almost 4 years was a great experience. In the workspace I have been living in a more software heavy environment. I found this project to be a great refresher of my skills.<br />

I found the inital research phase to be quite enjoyable and straightforward. I easily identified the types of sensors, display, and STM32 chip I thought would be capable for the performance I wanted. Now that I identified the key components it was time to begin creating the schematic.<br />

I quickly realized this was a larger challenge than my senior design project. Although I took a significant part in the circuit design, I realized how benefical it was having a team of 4 people in multiple disciplines. The components I selected for this project were much more sophicated than what was chosen for the senior design project. I was honestly a little overwhelmed at the start. I had to take my time and started to break compoennts down. I started with the most important part, the STM32 chip. <br />

For the STM32 I first connected any power related pins and decoupling capacitors. Then I used the STM32CubeMX tool to assign pinouts. I knew between the display and other peripherals I needed a lot of pins, so I started by making sure the chip was capable of supoorting them all. Once I verfied the chip was capable I began connecting components. The most critical was the USB-C connector, which flash code onto the chip. The USB-C will also provide power to the whole system. I spent quite a bit of time reseaching possible power deisngs. The USB-C provides 5V and I need 3.3V for the STM32 and sensors. I eventually decided on a synchronous buck converter because it would provide an effiencent and stable step down from 5V to 3.3V. The display needs 5V, so I decided on a simple low pass filter because it would provide a clean signal and I have experience with designing those on circuit boards in the past.<br />

Next I added the sensors. I found the datasheets to be extremely helpful. The design process was made much easier by following the typical application circuits that they detailed. Then I added the display, button, and some LEDs. The extra LEDs and button will be useful at the start of programming to make sure the board can properly run code and debug. Lastly I added a connector for debug pins.<br />

Now that I had a schematic to follow I moved onto the PCB layout. This portion was much more difficult than expected. It was at this point I had to take into account physical and cost restrictions. JLCPCB was my choice of board manufacturer. To minimize costs I decided on a 2 layer board, which gave me a lot of physical trace constraints.  Also, since the sensors are a bit costly, I decided to move them to a second circuit board to save on future costs. With these new challenges I had to go back to my schematic and STM32CubeMX. Unfortunately I was not able to place my STM32 pinouts more conviently to minimize trace crossovers. At his point I decided to go with a more generic pin layout that I can wire with some 2.54mm connectors to minimize my trace lengths and crossovers. In the future I will look at more layered boards or other STM32 chips as potential solutions.<br />

To minmize costs on JLCPCB I tried to idently basic parts in the JLCPCB library. Not all parts selected were in that and drove up the cost (~$3 per part not in the basic library). In the future I will place more emphasis on this to lower my costs.<br />

Major Credit to Phil's Lab PBC Design video: https://www.youtube.com/watch?v=aVUqaB0IMh4. This was a great refresher on circuit board design and I highly recommend his channel.<br />

#### Soldering and Assembly
When ordering the board I decided that to save on costs to I would hand solder some of the components. This involved things the STM32 chip, USB-C connector, async buck convertor, buttons, and pins. I felt this was a great skill refresher. I took it slow and steady. I started with easier items like the pins and buttons to get back into the flow of things. The picture below is from right before I went to solder the STM chip.<br />
<img width="608" height="569" alt="image" src="https://github.com/user-attachments/assets/7197c4ce-4d4f-4999-8ae8-337b23b1696f" /><br />

I bought a cheap soldering station and found it pretty useful when adding the smaller components. In the picture below you can see a cheaper soldering iron. I found this ok for the larger components but, I decided it was safer to spend the money and get a proper iron for the delicate items like the USB-C and STM32 chip.<br />
<img width="665" height="530" alt="image" src="https://github.com/user-attachments/assets/71d7bfaa-2d15-4635-99d2-65914d4bc8ba" /><br />

### Box Design
I bought a simple project box and placed the main board inside the box. I left the sensor board on top along with the display to keep them on the same level.
<img width="527" height="649" alt="image" src="https://github.com/user-attachments/assets/b8e64388-a0e3-476a-846f-721dc418218f" />

### Code Design

#### Inital Project Scope

#### Limitations of IMU Sensors

#### Redesign

#### Extra Notes
