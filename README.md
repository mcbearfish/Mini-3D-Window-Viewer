# AR Viewer

*Order for Version 1 has been placed. Will post Updates on here when boards are recieved.

# Table of Contents
-[About](#about)<br />
-[Project Scope](#Projecct-Scope)<br />
-[Circuit Design](#Circuit-Design)<br />
-[Code](#Code)<br />
-[Reflections](#Reflections)

## About
This is a developmental project intended to refresh my skills on designing and programming an STM32 microcontroller board. I chose a project scope that I believed would provide difficult circuit and programming challenges. I will document the development process in this ReadMe.<br />

If you stumble across this project feel free to use it for educationl purposes. If you find errors or have questions about the circuit design or code feel free to reach out. Please cite this repository as a source if you plan to use it.<br />

## Project Scope
This project is the Alternate Reality Viewer.<br />

## Circuit Design
### Version 1
#### 

## Code
* Waiting for Version 1 boards to arrive.<br />

## Reflections
### Circuit Design Version 1
Designing my first board in almost 4 years was a great experience. In the workspace I have been living in a more software heavy environment. I found this project to be a great refresher of my skills.<br />

I found the inital research phase to be quite enjoyable and straightforward. I easily identified the types of sensors, display, and STM32 chip I thought would be capable for the performance I wanted. Now that I identified the key components it was time to begin creating the schematic.<br />

I quickly realized this was a larger challenge than my senior design project. Although I took a significant part in the circuit design, I realized how benefical it was having a team of 4 people in multiple disciplines. The components I selected for this project were much more sophicated than what was chosen for the senior design project. I was honestly a little overwhelmed at the start. I had to take my time and started to break compoennts down. I started with the most important part, the STM32 chip. <br />

For the STM32 I first connected any power related pins and decoupling capacitors. Then I used the STM32CubeMX tool to assign pinouts. I knew between the display and other peripherals I needed a lot of pins, so I started by making sure the chip was capable of supoorting them all. Once I verfied the chip was capable I began connecting components. The most critical was the USB-C connector, which flash code onto the chip. The USB-C will also provide power to the whole system. I spent quite a bit of time reseaching possible power deisngs. The USB-C provides 5V and I need 3.3V for the STM32 and sensors. I eventually decided on a synchronous buck converter because it would provide an effiencent and stable step down from 5V to 3.3V. The display needs 5V, so I decided on a simple low pass filter because it would provide a clean signal and I have experience with designing those on circuit boards in the past.<br />

Next I added the sensors. I found the datasheets to be extremely helpful. The design process was made much easier by following the typical application circuits that they detailed. Then I added the display, button, and some LEDs. The extra LEDs and button will be useful at the start of programming to make sure the board can properly run code and debug. Lastly I added a connector for debug pins.<br />

Now that I had a schematic to follow I moved onto the PCB layout. This portion was much more difficult than expected. It was at this point I had to take into account physical and cost restrictions. JLCPCB was my choice of board manufacturer. To minimize costs I decided on a 2 layer board, which gave me a lot of physical trace constraints.  Also, since the sensors are a bit costly, I decided to move them to a second circuit board to save on future costs. With these new challenges I had to go back to my schematic and STM32CubeMX. Unfortunately I was not able to place my STM32 pinouts more conviently to minimize trace crossovers. At his point I decided to go with a more generic pin layout that I can wire with some 2.54mm connectors to minimize my trace lengths and crossovers.<br />

To minmize costs on JLCPCB I tried to idently basic parts in the JLCPCB library. Not all parts selected were in that and drove up the cost (~$3 per part not in the basic library). In the future I will place more emphasis on this to lower my costs.<br />

Major Credit to Phil's Lab PBC Design video: https://www.youtube.com/watch?v=aVUqaB0IMh4. This was a great refresher on circuit board design and I highly recommend his channel.<br />
