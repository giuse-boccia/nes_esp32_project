# Virtual Cord Protocol 

## Project goals 

The primary goal is the implementation of the Virtual Cord Protocol which is used for routing within AD-Hoc WLAN networks. 

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
