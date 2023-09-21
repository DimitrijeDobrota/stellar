
# Stellar

UCI bitboard chess engine written in C++20

## Description

The goal of the project is experimentation. I want to see how far I can push the engine to play better while learning various techniques of optimization.

To see the games played or play a game on your own checkout the profile on [lichess](https://lichess.org/@/StellarBOT), bullet, blitz and rapid challenges, ranked or casual, are accepted.

## Getting Started

### Dependencies

* CMake 3.25.2 or latter
* Compiler with C++20 support

### Installing

* Clone the repo
* Make a build folder and cd into it
* Run `cmake -DCMAKE_BUILD_TYPE=Release <path to cloned repo>`

### Executing program

* Run the engine by running: `./bin/engine`

* The engine accepts commands on the standard input and produces results to the standard output
* To communicate with the engine use UCI command. Reference for UCI protocol can be found [here](http://download.shredderchess.com/div/uci.zip)

## Help

* To see the options for additional tools run one of the following commands form the build directory:
	```
	./bin/perft -h
	```
* Changes to the move generation can be tested with `ctest` on a predefined set of positions


## Version History

* 1.0
    * Initial Release

## License

This project is licensed under the MIT License - see the LICENSE.md file for details

## Acknowledgments

Inspiration, code snippets, etc.
* [Chess Programming Wiki](https://www.chessprogramming.org/)
* [Code Monkey King](https://github.com/maksimKorzh)
* [Lichess-Bot](https://github.com/lichess-bot-devs/lichess-bot)

