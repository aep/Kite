#include "MqttClient.hpp"
#include "Timer.hpp"
#include <iostream>

#define MQTT_PROTO_MAJOR 3
#define MQTT_PROTO_MINOR 1
#define MQTT_PROTOCOL_VERSION "MQTT/3.1"

#define PROTOCOL_MAGIC "MQIsdp"

#define FLAG_CLEANSESS(F, C)(F | ((C) << 1))
#define FLAG_WILL(F, W)(F | ((W) << 2))
#define FLAG_WILLQOS(F, Q)(F | ((Q) << 3))
#define FLAG_WILLRETAIN(F, R) (F | ((R) << 5))
#define FLAG_PASSWD(F, P)(F | ((P) << 6))
#define FLAG_USERNAME(F, U)(F | ((U) << 7))

using namespace Kite;

class Frame
{
public:
    enum MessageType
    {
        //0,  //Reserved
        CONNECT     = 1,  //Client request to connect to Server
        CONNACK     = 2,  //Connect Acknowledgment
        PUBLISH     = 3,  //Publish message
        PUBACK      = 4,  //Publish Acknowledgment
        PUBREC      = 5,  //Publish Received (assured delivery part 1)
        PUBREL      = 6,  //Publish Release (assured delivery part 2)
        PUBCOMP     = 7,  //Publish Complete (assured delivery part 3)
        SUBSCRIBE   = 8,  //Client Subscribe request
        SUBACK      = 9,  //Subscribe Acknowledgment
        UNSUBSCRIBE = 10, //Client Unsubscribe request
        UNSUBACK11  = 11, //Unsubscribe Acknowledgment
        PINGREQ     = 12, //PING Request
        PINGRESP    = 13, //PING Response
        DISCONNECT  = 14, //Client is Disconnecting
        //15  // Reserved
    };

    Frame(MessageType type = MessageType(0), int qos = 0, bool retain = false, bool dup = false)
        : expectedSize(0)
        , type(type)
        , qos(qos)
        , retain(retain)
        , dup(dup)
    { }
    int expectedSize;
    MessageType type;
    uint8_t qos;
    bool retain;
    bool dup;
    //TODO std::string is an inefficient buffer
    std::string data;

    Frame(uint8_t header, const std::string &data);

    int          size() const { return data.size(); }
    uint16_t     readInt();
    uint8_t      readByte();
    std::string readString();

    void writeInt(uint16_t i);
    void writeByte(uint8_t c);
    void writeString (const std::string& string);
    void writeRawData(const std::string& data);


    void parcel(Kite::IO *io) const;
};

///////////////////////////////////////////////////////////////////////////////

struct Kite::MqttClientPrivate
{
    Kite::MqttClient *p;
    std::string clientId;
    std::string username;
    std::string password;
    int keepAlive;
    Timer seenServer;
    std::vector<char> buffer;
    int expectedSize;
    uint8_t nextHeader;
    int readPayloadSize()
    {
        int8_t byte;
        int len = 0;
        int mul = 1;
        do {
            //TODO we hope we got the whole header in one tcp package
            //otherwise we crash
            byte = buffer[0];
            buffer.erase(buffer.begin());
            len += (byte & 127) * mul;
            mul *= 128  ;
        } while ((byte & 128) != 0);
        return len;
    }
    uint16_t nextMessageId;

    void onCONNACK (Frame &frame);
    void onPUBACK  (Frame &frame);
    void onSUBACK  (Frame &frame);
    void onPUBLISH (Frame &frame);
};

///////////////////////////////////////////////////////////////////////////////

MqttClient::MqttClient(std::weak_ptr<Kite::EventLoop> ev)
    : Kite::SecureSocket(ev)
    , p(new MqttClientPrivate)
{
    p->p = this;
    p->keepAlive = 3;
    p->clientId  = "kitemqtt";
    p->expectedSize = 0;
    p->nextMessageId = 1;
}

MqttClient::~MqttClient()
{
    delete p;
}

void MqttClient::setClientId(const std::string &v)
{
    p->clientId = v;
}

void MqttClient::setKeepAlive(int v)
{
    p->keepAlive = v;
}

void MqttClient::onActivated(int)
{
    int osize = p->buffer.size();
    p->buffer.resize(osize + 201);
    int r = read(p->buffer.data() + osize, 200);

    if (r < 1) {
        //AGAIN
        //TODO:
        //this happens when the fd gets ready but ssl frames arent complete
        //don't re-buffer to remove the problem.
        //it's ok if the higher layer gets an EAGAIN,
        //but we need an abstract concept
        //at least enum or something, not ints
        if (r == -1) {
            p->buffer.resize(osize);
            return;
        }

        //TODO
        fprintf(stderr, "ssl read error: %d\n", r);
        ev()->exit(0);
        return;
    }
    p->buffer.resize(osize + r);

    while (p->buffer.size() > 2) {
        if (p->expectedSize == 0) {
            p->nextHeader = p->buffer[0];
            p->buffer.erase(p->buffer.begin());
            p->expectedSize = p->readPayloadSize();
        }


        std::string data;
        if (p->expectedSize != 0) {
            if (p->buffer.size() < p->expectedSize)
                return;

            data = std::string(p->buffer.begin(), p->buffer.begin() + p->expectedSize);
            p->buffer.erase(p->buffer.begin(), p->buffer.begin() + p->expectedSize);
            p->expectedSize = 0;
        }


        Frame frame(p->nextHeader, data);
        std::cerr << "msg type" << frame.type << std::endl;
        switch (frame.type) {
            case Frame::CONNACK:
                p->onCONNACK(frame);
                break;
            case Frame::PUBACK:
                p->onPUBACK(frame);
                break;
            case Frame::SUBACK:
                p->onSUBACK(frame);
                break;
            case Frame::PUBLISH:
                p->onPUBLISH(frame);
                break;
            case Frame::PINGRESP:
                break;
            default:
                std::cerr  << "unhandled message type " << frame.type << std::endl;
        }
    }
}

void MqttClient::onConnected() {
    fprintf(stderr, "connected!\n");
    bool    cleansess = false;

    Frame frame(Frame::CONNECT, 0);
    frame.writeString(PROTOCOL_MAGIC);
    frame.writeByte(MQTT_PROTO_MAJOR);

    uint8_t flags = 0;
    //flags
    flags = FLAG_CLEANSESS(flags, cleansess ? 1 : 0 );
    /*
       flags = FLAG_WILL(flags, will ? 1 : 0);
       if (!willTopic.isEmpty()) {
       flags = FLAG_WILLQOS(flags, will->qos());
       flags = FLAG_WILLRETAIN(flags, will->retain() ? 1 : 0);
       }
       */
    if(!p->username.empty()) {
        flags = FLAG_USERNAME(flags, 1);
    }
    if (!p->password.empty()) {
        flags = FLAG_PASSWD(flags, 1);
    }
    frame.writeByte(flags);
    frame.writeInt(p->keepAlive);
    frame.writeString(p->clientId);
    /*
       if(will != NULL) {
       frame.writeString(will->topic());
       frame.writeString(will->message());
       }
       */
    if (!p->username.empty()) {
        frame.writeString(p->username);
    }
    if (!p->password.empty()) {
        frame.writeString(p->password);
    }


    frame.parcel(this);
    flush();
}



void MqttClient::publish(const std::string &topic, const std::string &message, int qos)
{
    Frame frame(Frame::PUBLISH, qos);
    frame.writeString(topic);
    if (++(p->nextMessageId) == 0) p->nextMessageId = 1;
    frame.writeInt(p->nextMessageId);
    frame.writeRawData(message);
    frame.parcel(this);
    flush();
}
void MqttClient::subscribe(const std::string &topic, int qos)
{
    Frame frame(Frame::SUBSCRIBE, 1);
    if (++(p->nextMessageId) == 0) p->nextMessageId = 1;
    frame.writeInt(p->nextMessageId);
    frame.writeString(topic);
    frame.writeByte(qos);
    frame.parcel(this);
    flush();
}

void MqttClientPrivate::onCONNACK (Frame &frame)
{
    frame.readByte();
    int status = frame.readByte();
    std::cerr << "connection status: " << status << std::endl;
    if (status != 0) {
        p->onProtocolError(MqttClient::ProtocolError(status));
        p->disconnect();
        return;
    }
    p->onMqttConnected();
}

void MqttClientPrivate::onPUBACK  (Frame &frame)  {
    int id = frame.readInt();
    std::cerr<< "ackd publish " << id << std::endl;
}
void MqttClientPrivate::onSUBACK  (Frame &frame)  {
    int id = frame.readInt();
    std::cerr<< "ackd sub " << id << std::endl;
}
void MqttClientPrivate::onPUBLISH (Frame &frame)  {
    std::string topic = frame.readString();

    std::string m  = frame.data;
    p->onPublished(topic, m);

    if (frame.qos == 1) {
        auto id = frame.readInt();
        Frame frame(Frame::PUBACK, 0);
        frame.writeInt(id);
        frame.parcel(p);
        p->flush();
    } else if (frame.qos == 2) {
        std::cerr << "QOS2 not implemented\n";
        //TODO
    }
}


///////////////////////////////////////////////////////////////////////////////

#define LSB(A) (uint8_t)(A & 0x00FF)
#define MSB(A) (uint8_t)((A & 0xFF00) >> 8)


Frame::Frame(uint8_t header, const std::string &data)
    : data(data)
{
    type   = Frame::MessageType(header >> 4);
    dup    = header & (1 << 3);
    qos    = (header >> 1) & 3;
    retain = header & 1;
}

uint8_t Frame::readByte()
{
    uint8_t c = data[0];
    data.erase(0, 1);
    return c;
}

uint16_t Frame::readInt()
{
    uint8_t msb = data[0];
    uint8_t lsb = data[1];
    data.erase(0, 2);
    //TODO: this doesnt look right. will this convert to a wider type before shift?
    return (msb << 8) | lsb;
}

std::string Frame::readString()
{
    int len = readInt();
    std::string s = std::string(data.substr(0, len));
    data.erase(0, len);
    return s;
}

//TODO this is BS. just write the 16bit int in big endian
void Frame::writeInt(uint16_t i)
{
    data += MSB(i);
    data += LSB(i);
}

void Frame::writeString(const std::string &string)
{
    writeInt(string.size());
    data.append(string);
}

void Frame::writeByte(uint8_t c)
{
    data += char(c);
}

void Frame::writeRawData(const std::string &d)
{
    data.append(d);
}

void Frame::parcel(Kite::IO *io) const
{
    uint8_t header = type << 4;
    if (dup)
        header |= 1 << 3;
    header |= qos << 1;
    if (retain)
        header  |= 1;

    io->write((char*)&header, 1);

    if (data.size() == 0) {
        io->write("\0", 1);
        return;
    }


    //length
    std::vector<char> r;
    int length = data.size();
    uint8_t d;
    do {
        d = length % 128;
        length /= 128;
        if (length > 0) {
            d |= 0x80;
        }
        r.push_back(d);
    } while (length > 0);

    io->write(r.data(), r.size());
    io->write(data.c_str(), data.size());
    return;
}
