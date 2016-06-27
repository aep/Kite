#include <stdio.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include "dessert/dessert.hpp"

#include "Unixbus.hpp"

desserts("loopback") {

    class TestServer: public Kite::Unixbus
    {
    public:
        TestServer(std::weak_ptr<Kite::EventLoop> ev)
            : Kite::Unixbus(ev)
        {
            bool ok = bind("session:kite:test");
            dessert(ok);
        }
        virtual void onBusMessage(const std::string &msg, int address)
        {
            dessert(msg == "yo!");
            ev()->exit(0);
        }
    };


    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::shared_ptr<TestServer>         ls(new TestServer(ev));
    dessert(1);
    bool ok = Kite::Unixbus::invoke("session:kite:test", "yo!");
    dessert(ok);
    int r = ev->exec();
    dessert(r == 0);
}


int main(){}
