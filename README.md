## Speeduino Minus
Speeduino Minus is a modular fork of Speeduino focused on loading only the features you actually need. The goal is simple: keep the firmware lean, make the codebase easier to navigate, and let each build pull in only the modules required for that setup.

The original idea for the name was `Speeduino+`, to signal a configurable, feature-driven variant of the project. During repository setup, the `+` could not be kept as the GitHub project name, so the project became `Speeduino Minus`. The name also matches the design goal: Speeduino, with less stuff by default.

In practice, that means the firmware is split into a small orchestration core plus optional modules. You can build a minimal firmware, a street-focused firmware, or a more complete motorsport configuration without carrying unrelated code in every binary.

## What This Project Is
This project keeps the Speeduino engine management stack but reorganizes it so the firmware can be composed by feature. The code now exposes a smaller core, a clear module layer, and build-time feature selection so the final firmware can be tailored to the vehicle and use case.

## Documentation
The Speeduino online manual can be found at: https://wiki.speeduino.com

For Speeduino Minus-specific build selection and module composition, see the repository README, the `platformio.ini` profiles, and the feature config files in the root of the project.

## Where to Buy
[Pre-made Speeduino units/boards are available from a number of official resellers](https://speeduino.com/home/where-to-buy)

These resellers all contribute a portion of sales back to the project to allow for continued development to take place and we encourage sales through them whenever possible. 
 
Of course, being open source, you are free to use the design files provided here to create your own hardware! 

## Support
In addition to the manual referenced above, Speeduino has a large and very vibrant community of people to help out with your setup or any questions you might have.

* [Discord](https://discord.gg/YWCEexaNDe)
* [Speeduino Forum](https://speeduino.com/forum) 
* [Facebook](https://www.facebook.com/groups/191918764521976/)

## Contributors

This project exists thanks to all the people who contribute, both in terms of code and testing provided. If you'd like to get involved, please have a read through [Contributing](contributing.md) and then jump on Discord to discuss things further.
