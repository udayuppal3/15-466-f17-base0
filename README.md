# Game0

This is Game0 as implemented by Uday Uppal, through a fork of the Base0 code by Jim McCann.

## The Game

Tennis For One (Default), see http://graphics.cs.cmu.edu/courses/15-466-f17/game0-design/

As described in the design document, this is a single player game in which the player must direct a ball into the target area on the left of the screen, using a paddle on the right of the screen, controllable by the mouse. The ball starts at the center of the screen, and begins moving to the right with some random vertical velocity when the mouse is clicked. The player must then bounce the ball off the paddle and the top, left, and bottom walls in a way to get the ball into the target area.
If the ball hits the target area, the score goes up by one, and the next stage is loaded. Each stage gets progressively harder as the target area gets smaller. The player can begin the next stage by clicking the mouse button. 
If the ball goes past the paddle on the right, the player loses a life. Upon losing three lives, the game ends and the loss screen is displayed with the player score. If instead the player manages to score 99 points without losing 3 lives, the game is won and the win screen is displayed with the end score of 99 points.
During gameplay, the score is displayed in blue in the top left of the screen, and the number of lives remaining is in red in the top right.
The game can be quit by closing the application, hitting escape, or mouse clicking on a game over screen (win or loss).

## Building

There is a Makefile included that is used to build the game. It is the same as the original Makefile from the Base0 fork, however it includes one extra command line option for OS X, that adds usr/local/include to the list of include directories checked. Besides that, the game is built by just using the make command.
