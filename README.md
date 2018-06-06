# Spout-UE4
[Spout](http://spout.zeal.co/) Plugin for Unreal Engine

Sender and Receiver only DirectX 11.

# Installation and Use

Put code in folder Plugins (for example "yourproject/Plugins/SpoutUE4")

# Info

the "spout sender" has two options: 
  * "Game Viewport" that send the image of the viewport (no work in standalone game) 
  * or use a "TextureRenderTarget2D" in this case you should create along with a "SceneCaptureComponent2D"

use "spout close" blueprint to close spouts

![CaptureSpout2](http://aledel.github.io/Spout-UE4/images/10senders.jpg)
test sending 10 sender to Touchdesigner 1024x768 either one, the performance is good.

# Install Example

* Create new c++ First Person project
* unzip example in the "Content" folder of your project
* unzip code plugin in folder "Plugins" as mentioned above, if there is no "Plugins" folder, create it
* restart project
* load Spout scene

[ExampleSpout.zip](http://aledel.github.io/Spout-UE4/exampleSpoutUE4/ExampleSpout.zip)

![CaptureSpout2](http://aledel.github.io/Spout-UE4/images/spout2.jpg)
This image corresponds to the "Spout" scene. 

