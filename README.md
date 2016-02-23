# Spout-UE4
[Spout](http://spout.zeal.co/) Plugin for Unreal Engine

Sender and Receiver only DirectX 11.

# Installation and Use

Put code in folder Plugins (for example "yourproject/Plugins/Spout-UE4")

sure that there is no "spout" running before play. to avoid errors like "memory access Violations" or "pure virtual function being called .."

use "spout close" blueprint to close spouts in runtime

you can change the names at runtime.
I have had crashes if the "SenderName" of the "Spout Sender" has the same name as another "Spout Sender".

# Info

the "spout sender" has two options: 
  * "Game Viewport" that send the image of the viewport 
  * or use a "TextureRenderTarget2D" in this case you should create along with a "SceneCaptureComponent2D"



![CaptureSpout1](http://aledel.github.io/KinectXbox360-UE4/Images/Spout1.jpg)
