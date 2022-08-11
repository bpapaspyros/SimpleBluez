#include "simplebluez/interfaces/GattDescriptor1.h"

using namespace SimpleBluez;

GattDescriptor1::GattDescriptor1(std::shared_ptr<SimpleDBus::Connection> conn, std::string path)
    : SimpleDBus::Interface(conn, "org.bluez", path, "org.bluez.GattDescriptor1") {}

GattDescriptor1::~GattDescriptor1() { OnValueChanged.unload(); }

ByteStrArray GattDescriptor1::ReadValue() {
    auto msg = create_method_call("ReadValue");

    // NOTE: ReadValue requires an additional argument, which currently is not supported
    SimpleDBus::Holder options = SimpleDBus::Holder::create_dict();
    msg.append_argument(options, "a{sv}");

    SimpleDBus::Message reply_msg = _conn->send_with_reply_and_block(msg);
    SimpleDBus::Holder value = reply_msg.extract();
    update_value(value);

    return Value();
}

ByteArray GattDescriptor1::ReadValueBytes() {
    auto msg = create_method_call("ReadValue");

    // NOTE: ReadValue requires an additional argument, which currently is not supported
    SimpleDBus::Holder options = SimpleDBus::Holder::create_dict();
    msg.append_argument(options, "a{sv}");

    SimpleDBus::Message reply_msg = _conn->send_with_reply_and_block(msg);
    SimpleDBus::Holder value = reply_msg.extract();
    update_value(value);

    return ValueBytes();
}

std::string GattDescriptor1::UUID() {
    // As the UUID property doesn't change, we can cache it
    std::scoped_lock lock(_property_update_mutex);
    return _uuid;
}

ByteStrArray GattDescriptor1::Value() {
    std::scoped_lock lock(_property_update_mutex);
    return _value;
}

ByteArray GattDescriptor1::ValueBytes() {
    std::scoped_lock lock(_property_update_mutex);
    return _value_b;
}

void GattDescriptor1::property_changed(std::string option_name) {
    if (option_name == "UUID") {
        std::scoped_lock lock(_property_update_mutex);
        _uuid = _properties["UUID"].get_string();
    } else if (option_name == "Value") {
        update_value(_properties["Value"]);
        OnValueChanged();
    }
}

void GattDescriptor1::update_value(SimpleDBus::Holder& new_value) {
    std::scoped_lock lock(_property_update_mutex);
    auto value_array = new_value.get_array();

    char* value_data = new char[value_array.size()];
    _value_b.resize(value_array.size());
    for (std::size_t i = 0; i < value_array.size(); i++) {
        value_data[i] = value_array[i].get_byte();
        _value_b[i] = value_array[i].get_byte();
    }
    _value = ByteStrArray(value_data, value_array.size());
    delete[] value_data;
}
