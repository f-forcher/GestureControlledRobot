
# Gesture controlled robot

My high school ("Liceo Scientifico") project for the final exams, made in June 2013. An [Arduino](https://www.arduino.cc) robot 
controlled remotely (through Bluetooth) by the movements of the hands, recognized from an Android 
application created using the [OpenCV](http://opencv.org) computer vision library. It uses a Machine Learning technique calle Support Vector Machine, modified from face recognition to recognize a closed hand and his position in the camera field of view, and the coordinates are mapped to robot commands (ie Hand in the upper screen means move forward).
The robot however has proximity sensors and obstacle-avoiding capabilities: if the current command would make it collide, it turns around.

The other app instead controls all aspects of the robot, like the lights or the cooling.

# Images
## The robot
It features bluetooth, a speaker, a cooling fan, lights and infrared proximity sensors. 
![The robot](https://github.com/FrancescoForcher/GestureControlledRobot/blob/master/ArduinoRobot.png "The robot")

## The circuit
![The circuit](https://github.com/FrancescoForcher/GestureControlledRobot/blob/master/CIRCUITO3.jpg "The circuit")
