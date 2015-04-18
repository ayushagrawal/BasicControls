
2014-cs101
TEAM ID 406
TEAM MEMBERS:
   Ayush Agrawal      14d070042
   Mohak Goyal        140070029
   Raman Preet Singh  140070059
PROJECT TITLE:  Arena Protecting Bot

INSTRUCTIONS TO SETUP THE PROJECT:


1.You will initially have a bot with USB cam attatched to it.
2.Download the softwares whose links are provided.
3.Fix the X-bee module in the X-bee adapter which will connect the X-Bee with PC(Caution: Connecting the X-Bee module in opposite direction can damage the XBee module.)Make the connection between laptop and XBee module using a USB cable.
(
	When the connection is established, if the connection is correct, the following will happen:
	(i) Power LED on the X-Bee Adapter will be ON and
	(ii) Associate LED on the X-Bee Adapter will blink
)
4.After connecting XBee to the PC, check whether the necessary communication (COM) port is assigned to the XBee. This can be done using the Device Manager on your PC.
(If the com port is not detected in the Device Manager, install driver for CP2102 USB to serial converter.(Drivers can be downloaded from following link: https://www.silabs.com/products/mcu/Pages/USBtoUARTBridgeVCPDrivers.aspx))
5.Open X-CTU application and click on the "Test/Query" button.If the window with serial port etc. pops up, you can infer that the XBee module is working.
6.Open Modem Configuration tab on the X-CTU window and read the XBee module configuration by clicking the Read button.Now note down the following values from the Modem Configuration tab:
CH-Channel
ID-PAN ID
DH-Destination Address High
DL- Destination Address Low
SH-Serial Number High
SL-Serial Number Low
7.Connect the second XBee to the PC and repeat the steps 3 to 6.
8.Set the Destination address (DH and DL) of Receiver with the Source address (SH and SL).Now write the configuration by clicking the Write button in the configuration tab.
9.Remove the Receiver XBee module from adapter and keep it aside. This XBee module must be attached to the Fire Bird V XBee port.
10.Now attach the Transmitter XBee module again to set the network address.Set the Destination address (DH and DL) of Transmitter XBee module with the Source address (SH and SL) of Receiver XBee module.
11.Connect the Receiver XBee module to the Fire Bird V robot.Load the hex file from xbee_testing_verification_code/default folder in xbee_module_testing folder.Connect the Transmitter XBee module to PC using XBee adapter.Connect the Fire Bird to PC using USB cable.
12.Now,the two XBee's are configured to communicate with each other.their communication can be verified by:
	i.open two X-CTU applications. X-CTU window must show the two Serial Ports for Transmitter XBee module and Fire Bird V.
	ii.Now to see the output, open the terminal window in both X-CTU. Type something in window, which gets reflected back in other. The transmitted data appears in blue while the received data appears in red.
	iii.the XBee modules are configured to communicate with each other. Now you are ready to proceed with the future tasks.
13.Open the Codeblocks and open the given code.Now, compile the code.
14.Your bot is now ready to eliminate the red blocks from the arena.

Youtube Link to video of project:
