#include <stdio.h>
#include <string>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <arpa/inet.h>

#include "EventLoop.hpp"
#include "UdpServer.hpp"
#include "Timer.hpp"



class MqttSnProtocol
{
public:
    enum MessageType {
        ADVERTISE     = 0x0,
        SEARCHGW      = 0x1,
        GWINFO        = 0x2,
        CONNECT       = 0x4,
        CONNACK       = 0x5,
        WILLTOPICREQ  = 0x06,
        WILLTOPIC     = 0x07,
        WILLMSGREQ    = 0x08,
        WILLMSG       = 0x09,
        REGISTER      = 0x0A,
        REGACK        = 0x0B,
        PUBLISH       = 0x0C,
        PUBACK        = 0x0D,
        PUBCOMP       = 0x0E,
        PUBREC        = 0x0F,
        PUBREL        = 0x10,
        SUBSCRIBE     = 0x12,
        SUBACK        = 0x13,
        UNSUBSCRIBE   = 0x14,
        UNSUBACK      = 0x15,
        PINGREQ       = 0x16,
        PINGRESP      = 0x17,
        DISCONNECT    = 0x18,
        WILLTOPICUPD  = 0x1A,
        WILLTOPICRESP = 0x1B,
        WILLMSGUPD    = 0x1C,
        WILLMSGRESP   = 0x1D
    };

    enum TopicType {
        FullTopic    = 0x0,
        NumericTopic = 0x1,
        ShortTopic   = 0x2,
        ReservedTopicType = 0x3
    };

    struct Flags {
        TopicType topic_type  : 2;
        uint8_t clean  : 1;
        uint8_t will   : 1;
        uint8_t retain : 1;
        uint8_t qos    : 2;
        uint8_t dup    : 1;
    };

    enum ReturnCode {
        ACCEPT               = 0x0,
        REJECT_CONGESTED     = 0x1,
        REJECT_INVALID_TOPIC = 0x2,
        REJECT_UNSUPPORTED   = 0x3
    };

    MqttSnProtocol();
    virtual ~MqttSnProtocol();

protected:
    virtual void onActivated(int);
    virtual int send    (const char *data, int len, const InternetAddress &address) = 0;
    virtual int receive (char *data, int len,  InternetAddress *address) = 0;

    virtual void onConnect   (const Kite::InternetAddress &remote, const Flags &flags, uint8_t protocol, uint8_t duration, const std::string &name) = 0;
    virtual void onPublish   (const Kite::InternetAddress &remote, const Flags &flags, uint16_t topic, uint16_t mid, const std::string &data) = 0;
    virtual void onRegister  (const Kite::InternetAddress &remote, uint16_t topic, uint16_t mid, const std::string &name) = 0;
    virtual void onSubscribe (const Kite::InternetAddress &remote, const Flags &flags, uint16_t mid, const std::string &topic_name, uint16_t topic_id) = 0;


    void sendConnack  (const Kite::InternetAddress &remote, ReturnCode code);
    void sendRegack   (const Kite::InternetAddress &remote, uint16_t mid, uint16_t tid, ReturnCode code);
    void sendPuback   (const Kite::InternetAddress &remote, uint16_t mid, uint16_t tid, ReturnCode code);
    void sendSuback   (const Kite::InternetAddress &remote, uint16_t mid, uint16_t tid, ReturnCode code);
    void sendRegister (const Kite::InternetAddress &remote, uint16_t mid, uint16_t tid, const char *buf, int len);
    void sendPublish  (const Kite::InternetAddress &remote, uint16_t mid, uint16_t tid, const char *buf, int len);
};



void MqttSnProtocol::onActivated(int)
{
    Kite::InternetAddress src;
    char mem[1025];
    char *buf = mem;
    int len = receive(buf, 1025, &src);

    if (buf[0] == 0x01) {
        fprintf(stderr, "%s:%d  => large frames not supported\n", src.address().c_str(), src.port());
        return;
    }
    MessageType type = MessageType(buf[1]);
    buf += 2;
    len -= 2;
    switch (type) {
        case CONNECT:
            {
                Flags flags(*(Flags*)buf);
                uint8_t  protocol = buf[1];
                uint16_t duration = ntohs(*(uint16_t*)(buf + 2));
                buf += 4;
                len -= 4;
                std::string name(buf, len);
                onConnect(src,  flags, protocol, duration, name);
                break;
            }
        case REGISTER:
            {
                uint16_t  tid   = ntohs(*(uint16_t*)(buf + 0));
                uint16_t  mid   = ntohs(*(uint16_t*)(buf + 2));
                buf += 4;
                len -= 4;
                std::string topic(buf, len);
                onRegister (src, tid, mid, topic);
                break;
            }
        case PUBLISH:
            {
                Flags flags(*(Flags*)buf);
                uint16_t  tid   = ntohs(*((uint16_t*)(buf + 1)));
                uint16_t  mid   = ntohs(*((uint16_t*)(buf + 3)));
                buf += 5;
                len -= 5;
                std::string message(buf, len);
                onPublish (src, flags, tid, mid, message);
                break;
            }
        case SUBSCRIBE:
            {
                Flags flags(*(Flags*)buf);
                uint16_t  mid   = ntohs(*(uint16_t*)(buf + 1));
                buf += 3;
                len -= 3;

                Topic topic;

                uint16_t topic_id = 0;
                std::string topic_name;

                if (flags.topic_type == FullTopic) {
                    topic_name = std::string(buf, len);
                } else if(flags.topic_type == NumericTopic) {
                    topic_id = ntohs(*((uint16_t*)(buf)));
                } else if(flags.topic_type == ShortTopic) {
                    topic_name = std::string(buf, len);
                }

                onSubscribe (src, flags, mid, topic_name, topic_id);
                break;
            }
        default:
            fprintf(stderr, "%s:%d => unhandled message type 0x%x\n", src.address().c_str(), src.port(), type);
            return;
    };
}


void MqttSnProtocol::sendCONNACK(const Kite::InternetAddress &remote, ReturnCode code)
{
    const char data[] = {3, CONNACK, code};
    send(data, sizeof(data), remote);
}

void MqttSnProtocol::sendREGACK (const Kite::InternetAddress &remote, uint16_t mid, uint16_t tid, ReturnCode code)
{
    const char data[] = {7, REGACK, ((char*)&tid)[1], ((char*)&tid)[0],  ((char*)&mid)[1], ((char*)&mid)[0], code };
    send(data, sizeof(data), remote);
}

void MqttSnProtocol::sendPUBACK (const Kite::InternetAddress &remote, uint16_t mid, uint16_t tid, ReturnCode code)
{
    const char data[] = {7, PUBACK, ((char*)&tid)[1], ((char*)&tid)[0],  ((char*)&mid)[1], ((char*)&mid)[0], code };
    send(data, sizeof(data), remote);
}

void MqttsnServer::sendSUBACK (const Kite::InternetAddress &remote, uint16_t mid, uint16_t tid, ReturnCode code)
{
    const char data[] = {8, SUBACK, 0, ((char*)&tid)[1], ((char*)&tid)[0],  ((char*)&mid)[1], ((char*)&mid)[0], code };
    send(data, sizeof(data), remote);
}

void MqttSnProtocol::sendREGISTER (const Kite::InternetAddress &remote, uint16_t mid, uint16_t tid, const char *buf, int len)
{
    const uint8_t const_len = 6;
    const uint8_t final_len = const_len + len;
    char data[final_len] = {const_len, REGISTER, ((char*)&tid)[1], ((char*)&tid)[0],  ((char*)&mid)[1], ((char*)&mid)[0]};

    memcpy(data + const_len, buf, len);
    data[0] = final_len;
    send(data, final_len, remote);
}

void MqttSnProtocol::sendPUBLISH  (const Kite::InternetAddress &remote, uint16_t mid, uint16_t tid, const char *buf, int len)
{
    char flags_d = 0;
    Flags *flags ((Flags*)&flags_d);
    flags->ttype = 1;
    const uint8_t const_len = 7;
    const uint8_t final_len = const_len + len;

    char data[final_len] = {const_len, PUBLISH, flags_d, ((char*)&tid)[1], ((char*)&tid)[0],  ((char*)&mid)[1], ((char*)&mid)[0]};

    memcpy(data + const_len, buf, len);
    data[0] = final_len;
    send(data, final_len, remote);
}













class MqttSnServer: public Kite::UdpServer, public MqttSnProtocol
{
public:
    std::map<uint16_t, std::string> topic_map;

    MqttsnServer(std::weak_ptr<Kite::EventLoop> ev)
        : Kite::UdpServer(ev)
    {
        //test
        Kite::Timer::later(ev, [this]() {
            for (auto it = clients.begin(); it != clients.end(); ++it ) {
                sendPUBLISH(it->second->address, 666, 1, "yolo", 4);
            }
            return true;
        }, 1000);
    }

protected:
private:
    class Client
    {
    public:
        Client()
            : pending_resend_registered_after_subscribe(false)
        {
        }
        std::string id;
        bool pending_resend_registered_after_subscribe;
        Kite::InternetAddress address;
        std::unordered_set<uint16_t> subscriptions;
    };
    std::map<std::string, Client*> clients;
    Client *client_by_address(const Kite::InternetAddress &address);

    virtual void onConnect   (const Kite::InternetAddress &remote, const Flags &flags, uint8_t protocol, uint8_t duration, const std::string &name) override;
    virtual void onPublish   (const Kite::InternetAddress &remote, const Flags &flags, uint16_t topic, uint16_t mid, const std::string &data) override;
    virtual void onRegister  (const Kite::InternetAddress &remote, uint16_t topic, uint16_t mid, const std::string &name) override;
    virtual void onSubscribe (const Kite::InternetAddress &remote, const Flags &flags, uint16_t mid, const std::string &topic_name, uint16_t topic_id) override;

    uint16_t topic_id_for_name(const std::string &name);
};


void MqttsnServer::onConnect (const Kite::InternetAddress &remote, const Flags &flags, uint8_t protocol, uint8_t duration, const std::string &name)
{
    bool was_reconnect = false;

    Client *client = NULL;
    auto idx = clients.find(name);
    if (idx != clients.end()) {
        client = idx->second;
        was_reconnect = true;
    }

    if (client == NULL) {
        client = new Client;
        clients.insert(std::make_pair(name, client));
    }
    client->address = remote;
    client->id      = name;

    fprintf(stderr, "%s:%d => connected as %.*s v%d qos%d %s\n", remote.address().c_str(),
            remote.port(), name.size(), name.c_str(), protocol, flags.qos, was_reconnect ? "(reconnected)" : "");

    sendCONNACK(remote, ACCEPT);

    if (was_reconnect) {
        client->pending_resend_registered_after_subscribe = true;
    }
}

void MqttsnServer::handleREGISTER(char *buf, int len, const Kite::InternetAddress &remote)
{
    uint16_t  mid   = ntohs(*(uint16_t*)(buf + 2));
    buf += 4;
    len -= 4;

    std::string topic(buf, len);
    uint16_t id = topic_id_for_name(topic);

    fprintf(stderr, "%s:%d => register topic %s as %d \n", remote.address().c_str(), remote.port(), topic.c_str(), id);

    sendREGACK(remote, mid, id, ACCEPT);
}

void MqttsnServer::handlePUBLISH   (char *buf, int len, const Kite::InternetAddress &remote)
{
    Flags flags(*(Flags*)buf);
    uint16_t  tid   = ntohs(*((uint16_t*)(buf + 1)));
    uint16_t  mid   = ntohs(*((uint16_t*)(buf + 3)));
    buf += 5;
    len -= 5;

    auto idx = topic_map.find(tid);
    if (idx == topic_map.end()) {
        fprintf(stderr, "%s:%d => no such topic id: %u\n", remote.address().c_str(), remote.port(), tid);
        sendPUBACK(remote, mid, tid, REJECT_INVALID_TOPIC);
        return;
    }

    fprintf(stderr, "%s:%d => %s : %.*s\n", remote.address().c_str(), remote.port(), idx->second.c_str(), len, buf);
}

void MqttsnServer::handleSUBSCRIBE (char *buf, int len, const Kite::InternetAddress &remote)
{
    Flags flags(*(Flags*)buf);

    uint16_t  mid   = ntohs(*(uint16_t*)(buf + 1));
    buf += 3;
    len -= 3;

    uint16_t topic_id;

    if (flags.ttype == 0) { // normal
        topic_id   = topic_id_for_name(std::string(buf, len));

    } else if(flags.ttype == 1) { // predefined
        topic_id = ntohs(*(uint16_t*)(buf));
        if (topic_map.find(topic_id) == topic_map.end()) {
            sendSUBACK(remote, mid, topic_id, REJECT_INVALID_TOPIC);
            return;
        }

    } else {
        fprintf(stderr, "%s:%d => not handling topic type: %d\n",  remote.address().c_str(), remote.port(), flags.ttype);
        sendSUBACK(remote, mid, 0, REJECT_UNSUPPORTED);
        return;
    }

    Client *client = client_by_address(remote);
    if (!client) {
        sendSUBACK(remote, mid, 0, REJECT_INVALID_TOPIC);
        return;
    }


    client->subscriptions.insert(topic_id);
    sendSUBACK(remote, mid, topic_id, ACCEPT);

    //TODO: fugly hack.
    //udp might be out of order on the real internet
    //and shitty clients will crash if they get register before suback
    //TODO: actually check for suback
    if (client->pending_resend_registered_after_subscribe) {
        client->pending_resend_registered_after_subscribe = false;
        int mid =0;
        for (auto it = client->subscriptions.begin(); it != client->subscriptions.end(); ++it ) {
            sendREGISTER(remote, mid++, *it, topic_map[*it].data(), topic_map[*it].size());
        }
    }
}

uint16_t MqttsnServer::topic_id_for_name(const std::string &name)
{
    for (auto it = topic_map.begin(); it != topic_map.end(); ++it )
        if (it->second == name)
            return it->first;
    uint16_t id = topic_map.size() + 1;
    topic_map.insert(std::make_pair(id, name));
    return id;
}

MqttsnServer::Client *MqttsnServer::client_by_address(const Kite::InternetAddress &address)
{
    for (auto it = clients.begin(); it != clients.end(); ++it ) {
        if (it->second->address == address)
            return it->second;
    }
    return NULL;
}


int main()
{
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::shared_ptr<MqttsnServer>   mqs(new MqttsnServer(ev));
    mqs->listen(Kite::InternetAddress(Kite::InternetAddress::Any, 1883));

    return ev->exec();
}
