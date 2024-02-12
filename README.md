# Virtual Cord Protocol 

## Project goals 

The primary goal is the implementation of the Virtual Cord Protocol which is used for routing within AD-Hoc WLAN networks. 

## Progress status

This sections briefly describes the current progress status of the VCP-Algorithm implementation. 

### This works

* Sending / receiving broadcasted messages 
* Sending / receiving unicasted messages
* On esp-now sdk level
  * Adding new peers (no message encryption)
* Message routing through different tasks
  * Receiver Callback-Task pushes data into the receiver queue
  * Main VCP-Task reads receiver queue, processes it and pushes data into the sender queue 
  * Sender-Task calls esp-idf sdk functions to send byte-stream using esp-now
  * Sender error queue receives messages from a Sender error Callback-Task (Indication if data could be send or not using esp-now)
* VCP Algorithm
  * Using a message structure which allows us to
    * Send hello messages after joining the cord in order to broadcast the respective own position, predecessor, successor
    * Send update messages to pre- or successor which tells them that their position has changed if a new node has been added
    * Send data (adaptive byte-wise length) through the cord using greedy-routing (ascending- and descending order)

## This does not work

* There is no proper mechanism to remove nodes from the peers if they are not anymore in the network
* No ACK Messages are sent which lets us determine if the message sent to pre- or successor has arrived properly. 
* There is no CRC calculated for the payload, so we do not exactly know if the received data is received correctly
* If a node gets disconnected there is no mechanism that removes the node from the neighbors array and handles a redirection of the data packet through the cord 

We do know what is missing to get this algorithm going properly, but there was no time to implement the stated functionality that is missing! 

## Git structure

To clone the project and fetch all branches, use the following commands:

```shell
git clone URL
cd REPOSITORY-NAME
git fetch --all
```

### How to use PlatformIO

First, make sure your board is listed within the [board-database](https://docs.platformio.org/en/latest/boards/index.html#espressif-32). If not you may want to add your board, just follow [these](https://docs.platformio.org/en/latest/platforms/creating_board.html) instructions. That might drive you insane, but it's worth a try :D. 

Next you can use the branch `platformIO` from this project and create a new branch for your code. After cloning, make sure to add the following lines to `.gitignore` 

```
.vscode/extensions.json
CMakeLists.txt
platformio.ini
sdkconfig.*
```X 