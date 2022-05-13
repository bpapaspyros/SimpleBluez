#pragma once

#include <simpledbus/advanced/Proxy.h>

#include <simplebluez/Types.h>
#include <simplebluez/interfaces/GattCharacteristic1.h>

#include <cstdlib>

namespace SimpleBluez {

class Characteristic : public SimpleDBus::Proxy {
  public:
    Characteristic(std::shared_ptr<SimpleDBus::Connection> conn, const std::string& bus_name, const std::string& path);
    virtual ~Characteristic();

    // ----- METHODS -----
    ByteStrArray read();
    void write_request(ByteStrArray value);
    void write_command(ByteStrArray value);
    void start_notify();
    void stop_notify();

    // ----- PROPERTIES -----
    std::string uuid();
    ByteStrArray value();
    bool notifying();

    // ----- CALLBACKS -----
    void set_on_value_changed(std::function<void(ByteStrArray new_value)> callback);
    void clear_on_value_changed();

  private:
    std::shared_ptr<SimpleDBus::Interface> interfaces_create(const std::string& interface_name) override;

    std::shared_ptr<GattCharacteristic1> gattcharacteristic1();
};

}  // namespace SimpleBluez
