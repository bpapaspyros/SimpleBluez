#pragma once

#include <simpledbus/advanced/Interface.h>
#include <simpledbus/external/kvn_safe_callback.hpp>

#include <simplebluez/Types.h>

#include <string>

namespace SimpleBluez {

class GattDescriptor1 : public SimpleDBus::Interface {
  public:
    GattDescriptor1(std::shared_ptr<SimpleDBus::Connection> conn, std::string path);
    virtual ~GattDescriptor1();

    // ----- METHODS -----
    template <typename T>
    void WriteValue(const T& value) {
        SimpleDBus::Holder value_data = SimpleDBus::Holder::create_array();
        for (size_t i = 0; i < value.size(); i++) {
            value_data.array_append(SimpleDBus::Holder::create_byte(value[i]));
        }

        SimpleDBus::Holder options = SimpleDBus::Holder::create_dict();

        auto msg = create_method_call("WriteValue");
        msg.append_argument(value_data, "ay");
        msg.append_argument(options, "a{sv}");
        _conn->send_with_reply_and_block(msg);
    }

    ByteStrArray ReadValue();
    ByteArray ReadValueBytes();

    // ----- PROPERTIES -----
    std::string UUID();
    ByteStrArray Value();
    ByteArray ValueBytes();

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
