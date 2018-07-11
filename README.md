# Int64-Parametric-Plotter
Plots a subset of valid arithmetic and bitwise expressions in C

I ran this on Ubuntu 16.04. Uses C++11.

Uses FLTK for the display.

This program is incomplete.

To be added:
1) Ability to zoom
2) All numbers accepted are in hexadecimal.
   Decimal should be allowed as well with 0x preceding hex numbers.
   As of now, there is no "0x" ("10" is 16).
3) The space to the right in the screenshots was intended to have control widgets
   for scaling, etc...
4) There is no axis indicator on the entry fields. The top is x and the bottom is y.
5) Overflow check needs to be added

How to use:

Type parametric equation in fields using "t" as parameter (see screenshots for example).

Hover mouse to see unscaled coordinates.


![Alt text](screenshot1.png?raw=true "Screenshot1")


![Alt text](screenshot2.png?raw=true "Screenshot2")


Supported:

binary C operators:
\+ \- \* \/ \% << >> \& \| \^

unary C operators:
\- \~ (limitation: there cannot be consecutive unary tokens)

---

Type expression 

To Install FLTK:

    apt-get install libx11-dev
    apt-get install libfltk1.3-dev

To compile and run:

    Copy Makefile and main.cpp in some directory and type make. Run "./primarygui"
