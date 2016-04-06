#ifndef KITE_MQTT_CLIENT_HPP_WARP_kanmsdlkamsd
#define KITE_MQTT_CLIENT_HPP_WARP_kanmsdlkamsd

#include "SecureSocket.hpp"
#include "EventLoop.hpp"

namespace Kite {

class MqttClientPrivate;
class MqttClient : public Kite::SecureSocket
{
public:
    enum ProtocolError {
        RefusedProtocolVersion    = 0x1,
        RefusedIdentifier         = 0x2,
        RefusedServerUnavailable  = 0x3,
        RefusedBadLogin           = 0x4,
        RefusedNotAuthorized      = 0x5
    };

    MqttClient(std::weak_ptr<Kite::EventLoop> ev);
    ~MqttClient();

    void setReconnect      (bool re);
    void setKeepAlive      (int seconds);
    void setClientId       (const std::string &id);
    void setUsername       (const std::string &username);
    void setPassword       (const std::string &password);

    void subscribe   (const std::string &topic, int qos = 1);
    void unsubscribe (const std::string &topic, int qos = 1);
    void publish     (const std::string &topic, const std::string &message, int qos = 1);

protected:
    virtual void onMqttConnected () {};
    virtual void onPublished (const std::string &topic, const std::string &message){}
    virtual void onProtocolError(ProtocolError e){}

    void onActivated (int) override final;
    void onConnected () override;

private:
    friend class MqttClientPrivate;
    MqttClientPrivate *p;
};

}
#endif
