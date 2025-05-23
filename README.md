# Stellar

UCI bitboard chess engine written in C++20


## Description

The goal of the project is experimentation. I want to see how far I can push
the engine to play better while learning various techniques of optimization.

To see the games played or play a game on your own checkout the profile on
[lichess](https://lichess.org/@/StellarBOT), bullet, blitz and rapid challenges, ranked or casual, are accepted.

Check out the games carried out by [CCLR](http://computerchess.org.uk/ccrl/404/)
for up to date rating against other engines.


## Getting Started

### Dependencies

- Linux
    * CMake 3.25.2 or latter
    * Compiler with C++20 support (tested: clang 16.0.5, gcc 13.2.0)

- Windows
    * Visual Studio (tested: Community 2022 17.9.2)


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

- 1.4
    * Better time management
    * Pawn hash table

- 1.3
    * Build engine on Windows using Visual Studio
    * Improve compilation time

- 1.2
    * Improved evaluation using interpolation
    * Incremental sorting
    * Fix timeouts

- 1.1
    * Add Arena

- 1.0
    * Initial Release

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE.md) file for details


## Acknowledgments

Big thanks to [Gabor Szots](https://github.com/SzotsGabor) from CCRL testing
group, for taking an interest in this project by allowing Stellar to compete
with other engines.

Inspiration, code snippets, etc.
* [Chess Programming Wiki](https://www.chessprogramming.org/)
* [Code Monkey King](https://github.com/maksimKorzh)
* [Lichess-Bot](https://github.com/lichess-bot-devs/lichess-bot)

