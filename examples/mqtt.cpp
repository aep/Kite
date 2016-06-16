#include <iostream>

#include "EventLoop.hpp"
#include "MqttClient.hpp"
#include "Stdio.hpp"


class MyClient : public Kite::MqttClient
{
public:
    MyClient(std::weak_ptr<Kite::EventLoop> ev)
        : Kite::MqttClient(ev)
    {
    }
protected:
    virtual void onPublished (const std::string &topic, const std::string &message) {
        fprintf(stderr, "%s\n", message.c_str());
    }
    virtual void onMqttConnected() {
        fprintf(stderr, "omg MQTT is totally connected lol \n");
        subscribe("/merpwerf", 0);
    }
    virtual void onDisconnected(SocketState state) {
        fprintf(stderr, "disconnected: %i %s \n", state, errorMessage().c_str());
        ev()->exit(9);
    }
};


class Stdio : public Kite::Stdio
{
public:
    Stdio (std::weak_ptr<Kite::EventLoop> ev, std::weak_ptr<MyClient> mc)
        : Kite::Stdio(ev)
        , mc(mc)
    {
    }
private:
    std::weak_ptr<MyClient> mc;
    std::string lb;
protected:
    void onActivated(int) {
        char buf[1024];
        int r = read(buf, 1204);
        if (r < 1) {
            evRemove(fileno(stdin));
            ev()->exit(0);
            return;
        }
        mc.lock()->publish("/merpwerf", std::string(buf, r), 0);
    }
};

int main(int argc, char **argv)
{
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::shared_ptr<MyClient>        client(new MyClient(ev));
    std::shared_ptr<Stdio>           stdio(new Stdio(ev, client));

    client->setKeepAlive(2);
    client->setClientId("derp" + std::to_string((long)argv));
    client->setUsername("hans");
    client->setPassword("wurst");


    client->connect("localhost", 1883, 5000, false);
    //client->connect("localhost", 1883, 5000, true);

    std::cerr << "lopzing" << std::endl;
    return ev->exec();
}
