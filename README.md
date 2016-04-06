
Kite is "kind of like Qt"
--------------------------

Ever since i switched to embedded, i missed Qt because of how consistent and well designed it is.
However, Qt in embedded is not a real choice, porting Qt to a new platform is a full time job for a year.

- Qts build system is terrible. It's terrible because Qt isnt well structured.
  And it isnt well structured because the build system is so terrible.

- Qt is obese. For getting QString working, you need 164 other classes, one of which is a json parser.
  Remember Qstring was written back when C++ string basically did not work.

- All the paradigms are for guis. They were genious.... for guis. But no one writes Guis anymore.
  The overhead of string comparison for signal/slots can actually be a deal breaker in evented systems.

Naked C++ isn't a choice for me anymore, it's just too shitty.
Its designed to solve problems made up by scientists, not the kind of real problems i work with.


Unlike Qt, Kite is alot more stl

- uses stl whenever stl does the job (back when qt was designed, stl was basically worthless)
- std::string with no unicode support
- uses stl containers
- c++14

but unlike stl, kite is a lot more Qt

- everything nonblocking
- consistent concious flat api


How to "build" Kite
------------------------------

Kite is a bunch of C++ files that are designed to work on modern unix only.
It has no configure, no premake, no bullshit. Just naked C++. Just type 'make'.

Kite isn't a library, because all distros are garbage.
Probably the best way to use it, is as git submodule in your project.
Then just include Makefile.in in your makefile. It's already prepared with relative paths.

Don't bother picking individual files. It's 2016, the compiler knows better than you.

