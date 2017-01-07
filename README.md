![logo](/docs/Kite.png?raw=true)


Kite is "kind of like Qt" for embedded
-------------------------------------

Also kind of like node.js. Except fast, compiled, and available to most embedded platforms.
(embedded are cheap plastic boxes that can barely run a linux kernel, not raspberry pi, doh)

Kite is alot more stl than Qt

- uses stl whenever stl does the job (back when qt was designed, stl was basically broken)
- std::string with no unicode support
- uses stl containers
- c++14

Kite is a lot more nodejs than stl

- single eventloop (optimal for io-bound stuff)
- nonblocking by default

Kite is a lot more Qt than stl

- consistent concious flat api
- templates only where useful
- designed for practical use rather than theorical computer science

Kite is somewhat more nodejs than Qt
- callbacks and promises rather than signal/slots


Basic Concepts
-----------------------------

1. Memory Managment is left to stl. There is no Qt-like refcounting. use shared_ptr for that
2. Kite::EventLoop is the base of it all. it works just like QEventLoop
3. Everything that is a subclass of Kite::Evented runs on the EventLoop
4. Events either trigger a virtual override such as onActivated(), like it would in Qt
5. Or a Promise, which works like node.js


Hello World (callback style)
------------------------------

```C++
#include <Kite/EventLoop.hpp>
#include <Kite/Timer.hpp>

int main(int argc, char **argv)
{
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    Kite::Timer::later(ev, [ev]() { // call this lambda on the eventloop
        printf("hello world\n");
        ev->exit(0);                // stop the eventloop and make it return 0
        return false;               // stop the timer
    }, 100);                        // in 100ms
    return ev->exec();              // start the eventloop
}
```

Hello  World (observer style)
-------------------------------
```C++
#include <Kite/EventLoop.hpp>
#include <Kite/Timer.hpp>

class Clock: public Kite::Timer
{
public:
    Clock(std::weak_ptr<Kite::EventLoop> ev) : Kite::Timer(ev) { }
protected:
    virtual bool onExpired() override {
        fprintf(stderr, "tick\n");
        return true;
    }
};
int main()
{
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::shared_ptr<Clock>        clock(new Clock(ev));
    clock->reset(500);
    return ev->exec();
}

```


Hello  World (promise style)
-------------------------------
```C++
#include <Kite/EventLoop.hpp>
#include <Kite/Timer.hpp>

int main(int argc, char **argv)
{
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    Kite::Timer::later(ev, 100).then([](){
        printf("hello world\n");
        ev->exit(0);                // stop the eventloop and make it return 0
        return false;               // stop the timer
    }, 100);                        // in 100ms
    return ev->exec();              // start the eventloop
}
```




How to "build" Kite
------------------------------

Kite is a bunch of C++ files that are designed to work on modern unix only.
It has no configure, no premake, no bullshit. Just naked C++. Just type 'make'.

Kite isn't shipped as linkable so file. The best way to use it, is as git submodule in your project.
Then just include Makefile.in in your makefile. It's already prepared with relative paths.

Don't bother picking individual files. It's 2016, the compiler knows better than you.

