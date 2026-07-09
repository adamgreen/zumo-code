# My Zumo 2040 Challenge Bots
A place to store code for my entrant(s) in [The Zumo 2040 Challenge](https://github.com/adamgreen/zumo-2040-challenge#readme).

![Pololu Zumo 2040](images/20260607-PololuZumo2040.png)

## My Pololu Zumo 2040 Bots
### Bot Order
![Pololu Order](images/20260607-PololuOrder.png)

I ordered enough parts from Pololu on June 7, 2026 to build two [Zumo 2040 robots](https://www.pololu.com/product/5010) with the challenge required [75:1 Micro Metal HP Gearmotors](https://www.pololu.com/product/2215). This will allow me to conduct test matches with my various firmware variations.

I also ordered some additional motors with alternate gear ratios to experiment with those in the future as well:
* [50:1 Micro Metal HP Gearmotors](https://www.pololu.com/product/2213)
* [100:1 Micro Metal HP Gearmotors](https://www.pololu.com/product/2214)

![Pololo order received](images/20260611-Zumo2040_Boxes.jpg)

### Bot Build
![Just finished building first bot](images/20260612-ZumoBuild.jpg)

I finished building the first Zumo robot kit (shown above) on June 12th, 2026, the day after I received my order from Pololu.

I later built the second Zumo robot kit (shown below) on June 26th, 2026. This robot was initially constructed without the blade attached to the front of the robot to allow the [Balancing Example](https://github.com/adamgreen/zumo-2040-arduino-library/blob/main/examples/Balancing/Balancing.ino) to perform its magic. I later finished the build for this bot on July 5th and attached its front sensor array and blade as well. Now I can compete one bot against the other.

![Second bot balancing on its nose](images/20260629-Balancing.jpg)

## Zumo 2040 Arduino Library
I ported my [3π+ 2040 Arduino Library](https://github.com/adamgreen/pololu-3pi-plus-2040-arduino-library) to the **Zumo 2040**. This required:
* Removing support for the 3π+'s Bumper Sensors.
* Adding support for the Zumo's Proximity Sensors.
* Updating the various [examples](https://github.com/adamgreen/zumo-2040-arduino-library/tree/main/examples) to work on my Zumo 2040 bots.

This port can be found here: [Zumo 2040 Arduino Library](https://github.com/adamgreen/zumo-2040-arduino-library)

**Balancing.ino**<br>
![Balancing](images/20260629-Balancing.gif)

**SumoCollisionDetection.ino**<br>
![Sumo Collision Detection](images/20260629-SumoCollisionDetection.gif)

## Reading List
| | |
|-|-|
| ![Robot Sumo Book Cover](images/20260609-RobotSumo_BookCover.png) | [Robot Sumo by Pete Miles](https://www.amazon.com/Robot-Sumo-Official-Pete-Miles/dp/007222617X) |
| ![Sumo Bot Manual Cover](images/20260614-SumoBot_ManualCover.jpg) | [Parallax SumoBot - Mini-Sumo Robotics](https://www.parallax.com/package/sumobot-manual-downloads/) |
