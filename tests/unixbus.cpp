#include <stdio.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include "dessert/dessert.hpp"

#include "Unixbus.hpp"

desserts("loopback") {

    static int closed = 0;
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
            close();
        }
        virtual void onBusClosed() {
            closed++;
            bool ok = sendBusMessage("yo!");
            dessert(!ok);
        }
    };


    class TestClient : public Kite::Unixbus
    {
    public:
        TestClient(std::weak_ptr<Kite::EventLoop> ev)
            : Kite::Unixbus(ev)
        {
            bool ok;
            ok = connect("session:kite:test");
            dessert(ok);
            ok = sendBusMessage("yo!");
            dessert(ok);
        }
        virtual void onBusClosed() {
            closed++;
            ev()->exit(0);
        }
    };


    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::shared_ptr<TestServer>         ls(new TestServer(ev));
    dessert(1);
    bool ok = Kite::Unixbus::invoke("session:kite:test", "yo!");
    dessert(ok);
    std::shared_ptr<TestClient>         ll(new TestClient(ev));
    int r = ev->exec();
    dessert(r == 0);
    dessert(closed == 2);
}


int main(){}
