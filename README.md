# Spout-UE4
[Spout](http://spout.zeal.co/) Plugin for Unreal Engine

Sender and Receiver only DirectX 11.

# Installation and Use

Put code in folder Plugins (for example "yourproject/Plugins/Spout-UE4")

# Info

the "spout sender" has two options: 
  * "Game Viewport" that send the image of the viewport 
  * or use a "TextureRenderTarget2D" in this case you should create along with a "SceneCaptureComponent2D"

use "spout close" blueprint to close spouts in runtime

you can change the names at runtime.

I have had crashes if the "SenderName" of the "Spout Sender" has the same name as another "Spout Sender".

![CaptureSpout1](http://aledel.github.io/KinectXbox360-UE4/Images/Spout1.jpg)
Pictured above corresponds to the scene "simpleSpout" is a good example.

# Install Example

* Create new c++ First Person project
* unzip example in the "Content" folder of your project
* unzip code plugin in folder "Plugins" as mentioned above, if there is no "Plugins" folder, create it
* restart project
* load Spout scene

[ExampleSpout.zip](http://aledel.github.io/Spout-UE4/exampleSpoutUE4/ExampleSpout.zip)

![CaptureSpout2](http://aledel.github.io/Spout-UE4/images/spout2.jpg)
This image corresponds to the "Spout" scene. It works much better.
and as a curiosity when I print screen. you can see what it took to complete the shared memory in that frame, the balls were really very fast

