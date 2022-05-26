#pragma once

#include <simpledbus/advanced/Interface.h>
#include <simpledbus/external/kvn_safe_callback.hpp>

#include <simplebluez/Types.h>

#include <string>

#define MAX_BYTEARRAY_SIZE 20

namespace SimpleBluez {

class GattCharacteristic1 : public SimpleDBus::Interface {
  public:
    typedef enum { REQUEST = 0, COMMAND } WriteType;

    GattCharacteristic1(std::shared_ptr<SimpleDBus::Connection> conn, std::string path);
    virtual ~GattCharacteristic1();

    // ----- METHODS -----
    void StartNotify();
    void StopNotify();

    template <typename T>
    void WriteValue(const T& value, WriteType type) {
        SimpleDBus::Holder value_data = SimpleDBus::Holder::create_array();
        for (size_t i = 0; i < value.size(); i++) {
            value_data.array_append(SimpleDBus::Holder::create_byte(value[i]));
        }

        SimpleDBus::Holder options = SimpleDBus::Holder::create_dict();
        if (type == WriteType::REQUEST) {
            options.dict_append(SimpleDBus::Holder::Type::STRING, "type", SimpleDBus::Holder::create_string("request"));
        } else if (type == WriteType::COMMAND) {
            options.dict_append(SimpleDBus::Holder::Type::STRING, "type", SimpleDBus::Holder::create_string("command"));
        }

        auto msg = create_method_call("WriteValue");
        msg.append_argument(value_data, "ay");
        msg.append_argument(options, "a{sv}");

        if (type == WriteType::REQUEST) {
            _conn->send_with_reply_and_block(msg);

        } else {
            _conn->send(msg);
        }
    }

    ByteStrArray ReadValue();
    ByteArray ReadValueBytes();

    // ----- PROPERTIES -----
    std::string UUID();
    ByteStrArray Value();
    ByteArray ValueBytes();
    bool Notifying(bool refresh = true);

    // ----- CALLBACKS -----
    kvn::safe_callback<void()> OnValueChanged;

  protected:
    void property_changed(std::string option_name) override;
    void update_value(SimpleDBus::Holder& new_value);

    std::string _uuid;
    ByteStrArray _value;
    ByteArray _value_b;
};

}  // namespace SimpleBluez
