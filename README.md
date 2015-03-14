# Screen_UP_DOWN
Arduino project, videoprojector framed screen lifter. also control a LED tape light with 4 timers and fans for cooling your electronic stuff

/*---------------[SCREEN LIFTER ver 2.3 BY BENJAMIN DAVOULT]-----------------
This program has 3 purposes:

-lift up and down my wood framed screen in my home theatre room...whitch is also my bedroom.
-control a LED tape with a 10Kohm trimpot to enlight the room just a bit before bedtime with 4
selectable timers.
-cool the NAS server, DUNE HD player, XBOX etc... with 2 computer fans (I use 2 NOCTUA 60mm
they are fantastics).

note: the system will use the LED tape during the lifting of the screen to act as a safety light
like a "guys, be carefull there is a geek inside this room who make crazy things!"... 
You know what I mean

my screen wheigt about 4kg, and to lift it I use the Hitec HS-805 converted for continuous
rotation (a lot of tuto for this on the internet, its quite easy).
with a 1cm radius spool, it can lift up to 20kg, so it is usually called "the monster".

I started arduino (and programmation in general) about 1 month ago, so I am sure that my program
is terrible and that it is possible to make it FAR more simple...But you know waht? it works
just fine so its good enough for me!
but of course if you have some ideas to make it better, simpler or whatever, please let me know!

The runningMedian library is the one found on the arduino playground at
http://playground.arduino.cc/Main/RunningMedian
the credit goes to robtillaart.
there is no download file on the page, so you have to copy the code of the .h and .cpp,
create files and put it inside.

I dedicate this program to the community, so no copyright.
Do what you want with it.

---------------------[Benn25 (atmk) gmail.com]-----------------------
