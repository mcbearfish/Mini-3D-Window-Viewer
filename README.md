# AR Viewer

*Order for Version 1 has been placed. Will post Updates on here when boards are recieved.

# Table of Contents
-[About](#about)<br />
-[Project Description](#Projecct-Description)<br />
-[Circuit Design](#Circuit-Design)<br />
-[Code](#Code)<br />
-[Reflections](#Reflections)

## About
This is a developmental project intended to refresh my skills on designing and programming an STM32 microcontroller board. I chose a project scope that I believed would provide difficult circuit and programming challenges. I will document the development process in this ReadMe.<br />

If you stumble across this project feel free to use it for educationl purposes. If you find errors or have questions about the circuit design or code feel free to reach out. Please cite this repository as a source if you plan to use it.<br />

## Project Description
This project is the Alternate Reality Viewer.<br />

When placed on a flat surface, the device on startup will create a window into an alternate reality. The device will have a screen that a user can hold and move around the starting point to view a 3D object. As the user moves around it will be like the object was placed right in front of them.<br />

## Circuit Design
### Version 1
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

#### Sensors

#### Notes
I found Google's Gemini AI tool to be extremely helpful during  this design process. Especially with mapping the pinout of the display to STM32. I used a cheap knockoff with no documentation and poor pinout labels. In the future may need to remap pins if display fails to work.See ARSensors project for 2nd Board meant to connect sensors to this main board.

#### Main PCB
<img width="682" height="646" alt="image" src="https://github.com/user-attachments/assets/9e0afbe1-164e-45d2-b050-79085eeee85b" /><br />

<img width="924" height="828" alt="image" src="https://github.com/user-attachments/assets/3d0f7357-d871-41f3-9d02-1b833b7dc475" /><br />

<img width="924" height="828" alt="image" src="https://github.com/user-attachments/assets/b82c7a2e-9e28-4dab-b74b-71838d88a736" /><br />

<img width="924" height="828" alt="image" src="https://github.com/user-attachments/assets/83dcfbf9-5c89-470f-a855-a063bfe16aad" /><br />

#### Sensors PCB


## Code
* Waiting for Version 1 boards to arrive.<br />

## Reflections
### Circuit Design Version 1
Designing my first board in almost 4 years was a great experience. In the workspace I have been living in a more software heavy environment. I found this project to be a great refresher of my skills.<br />

I found the inital research phase to be quite enjoyable and straightforward. I easily identified the types of sensors, display, and STM32 chip I thought would be capable for the performance I wanted. Now that I identified the key components it was time to begin creating the schematic.<br />

I quickly realized this was a larger challenge than my senior design project. Although I took a significant part in the circuit design, I realized how benefical it was having a team of 4 people in multiple disciplines. The components I selected for this project were much more sophicated than what was chosen for the senior design project. I was honestly a little overwhelmed at the start. I had to take my time and started to break compoennts down. I started with the most important part, the STM32 chip. <br />

For the STM32 I first connected any power related pins and decoupling capacitors. Then I used the STM32CubeMX tool to assign pinouts. I knew between the display and other peripherals I needed a lot of pins, so I started by making sure the chip was capable of supoorting them all. Once I verfied the chip was capable I began connecting components. The most critical was the USB-C connector, which flash code onto the chip. The USB-C will also provide power to the whole system. I spent quite a bit of time reseaching possible power deisngs. The USB-C provides 5V and I need 3.3V for the STM32 and sensors. I eventually decided on a synchronous buck converter because it would provide an effiencent and stable step down from 5V to 3.3V. The display needs 5V, so I decided on a simple low pass filter because it would provide a clean signal and I have experience with designing those on circuit boards in the past.<br />

Next I added the sensors. I found the datasheets to be extremely helpful. The design process was made much easier by following the typical application circuits that they detailed. Then I added the display, button, and some LEDs. The extra LEDs and button will be useful at the start of programming to make sure the board can properly run code and debug. Lastly I added a connector for debug pins.<br />

Now that I had a schematic to follow I moved onto the PCB layout. This portion was much more difficult than expected. It was at this point I had to take into account physical and cost restrictions. JLCPCB was my choice of board manufacturer. To minimize costs I decided on a 2 layer board, which gave me a lot of physical trace constraints.  Also, since the sensors are a bit costly, I decided to move them to a second circuit board to save on future costs. With these new challenges I had to go back to my schematic and STM32CubeMX. Unfortunately I was not able to place my STM32 pinouts more conviently to minimize trace crossovers. At his point I decided to go with a more generic pin layout that I can wire with some 2.54mm connectors to minimize my trace lengths and crossovers. In the future I will look at more layered boards or other STM32 chips as potential solutions.<br />

To minmize costs on JLCPCB I tried to idently basic parts in the JLCPCB library. Not all parts selected were in that and drove up the cost (~$3 per part not in the basic library). In the future I will place more emphasis on this to lower my costs.<br />

Major Credit to Phil's Lab PBC Design video: https://www.youtube.com/watch?v=aVUqaB0IMh4. This was a great refresher on circuit board design and I highly recommend his channel.<br />
