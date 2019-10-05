*Modification of original plugin by [AleDel](https://github.com/AleDel/Spout-UE4) with UE versions 4.19+.*

**This was tested with:**
* 4.19
* 4.20
* 4.21
* 4.22
* 4.23

# Spout-UE4
This is a [Spout](http://spout.zeal.co/) Plugin for Unreal Engine. It allows you to send and receive textures using Spout framework.

Sender and Receiver only DirectX 11.

# Installation and Use

1. Open up Epic Games Launcher (make sure it's up to date).

2. Create a new C++ First person Project.
![Image 1](http://L05.github.io/Spout-UE4/images/SpoutExample_Instructions_01.png)

3. You should see it Generating code...
![Image 2](http://L05.github.io/Spout-UE4/images/SpoutExample_Instructions_02.png)

4. The Unreal project will open in the editor, and a Visual Studio project will also open.
![Image 3](http://L05.github.io/Spout-UE4/images/SpoutExample_Instructions_03.png)

5. Close the Unreal project.

6. In the project directory, create a Plugins folder.
![Image 4](http://L05.github.io/Spout-UE4/images/SpoutExample_Instructions_04.png)

7. Download the Spout-UE4 repository (zip file) and put it in the Plugins directory.
![Image 5](http://L05.github.io/Spout-UE4/images/SpoutExample_Instructions_05.png)

8. *(Optional)* Download ExampleSpout.zip (from the project GitHub page). Unzip the contents into the Content folder.
![Image 6](http://L05.github.io/Spout-UE4/images/SpoutExample_Instructions_06.png)

9. In the project directory, open the .uproject file. Press Yes when asked if you'd like to rebuild the SpoutPlugin module.
![Image 7](http://L05.github.io/Spout-UE4/images/SpoutExample_Instructions_07.png)
It will start to build.
![Image 8](http://L05.github.io/Spout-UE4/images/SpoutExample_Instructions_08.png)

10. Once the project opens, go to Settings > Plugins.
![Image 9](http://L05.github.io/Spout-UE4/images/SpoutExample_Instructions_09.png)

11. Make sure the Spout Plugin is enabled. If necessary, restart.
![Image 10](http://L05.github.io/Spout-UE4/images/SpoutExample_Instructions_10.png)

12. *(Optional)* Open the ExampleSpout > Spout project and press Play. Configure the Sender and Receiver names to work with other software.
![Image 11](http://L05.github.io/Spout-UE4/images/SpoutExample_Instructions_11.png)

For video instructions, please refer to [Unreal Engine 4 and Lightact Video Tutorials](https://www.youtube.com/playlist?list=PLcNPGta1d2XDcSsz8zcW0f2lPSawnW3mR), which provides a good step-by-Image walkthrough of how to set up your project for use with the plugin.

## Sending Spout

This is done with the **Spout sender** node which has can send texture either from the Game viewport or from a Render Targert 2D: 
  * **Game Viewport** sends the image of the viewport, but please note that it doesn't work in standalone or packaged game.
  * **TextureRenderTarget2D** in which case you should create a _SceneCaptureComponent2D_ and a *Render target 2D* which you should reference in the node.

use **Close Sender** node to close Spouts. The best way is to connect it to **Event EndPlay** node.

## Install Example

* Create new C++ *First Person* project
* unzip ExampleSpout.zip in the "Content" folder of your project
* unzip code plugin in folder "Plugins" as mentioned above, if there is no "Plugins" folder, create it
* restart project
* load Spout scene
* if you encounter compile errors you have to delete and re-insert identical nodes

[ExampleSpout.zip](http://L05.github.io/Spout-UE4/exampleSpoutUE4/ExampleSpout.zip) *(Updated on 9/22/2019)*

![CaptureSpout2](http://aledel.github.io/Spout-UE4/images/spout2.jpg)
This image corresponds to the "Spout" scene. 

## Packaged game
To make this plugin work in a packaged game you have to disable using 'pak' files. You do that by:
1. going to File->Package project->Packaging settings
2. once there uncheck 'Use Pak File' checkbox

This is only necessary if you are using the *Mat* pin on the *Spout Receiver* node.
