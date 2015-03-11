Kite is "kind of like Qt"

Ever since i switched to embedded, i missed Qt because of how consistent and well designed it is.
However, Qt in embedded is not a real choice, porting Qt to a new platform is a full time job for a year.

- Qts build system is terrible. It's terrible because Qt isnt well structured.
  And it isnt well structured because the build system is so terrible.

- Qt is obese. For getting QString working, you need 164 other classes, one of which is a json parser.
  Remember Qstring was written back when C++ string basically did not work.

- All the paradigms are for guis. They were genious.... for guis. But no one writes Guis anymore.
  The overhead of string comparison for signal/slots can actually be a deal breaker in evented systems.

Naked C++ isn't a choice for me anymore, it's just too complex.
Its designed to solve problems made up by scientists, not the kind of problems i work with.

So while we're waiting for the one high level language to finally work properly on embedded,
i made myself a Kite to get through the dark times of having to stick to C/C++
