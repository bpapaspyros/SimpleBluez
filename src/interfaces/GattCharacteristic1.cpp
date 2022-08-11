#include "simplebluez/interfaces/GattCharacteristic1.h"

using namespace SimpleBluez;

GattCharacteristic1::GattCharacteristic1(std::shared_ptr<SimpleDBus::Connection> conn, std::string path)
    : SimpleDBus::Interface(conn, "org.bluez", path, "org.bluez.GattCharacteristic1") {}

GattCharacteristic1::~GattCharacteristic1() { OnValueChanged.unload(); }

void GattCharacteristic1::StartNotify() {
    auto msg = create_method_call("StartNotify");
    _conn->send_with_reply_and_block(msg);
}

void GattCharacteristic1::StopNotify() {
    auto msg = create_method_call("StopNotify");
    _conn->send_with_reply_and_block(msg);
}

ByteStrArray GattCharacteristic1::ReadValue() {
    auto msg = create_method_call("ReadValue");

    // NOTE: ReadValue requires an additional argument, which currently is not supported
    SimpleDBus::Holder options = SimpleDBus::Holder::create_dict();
    msg.append_argument(options, "a{sv}");

    SimpleDBus::Message reply_msg = _conn->send_with_reply_and_block(msg);
    SimpleDBus::Holder value = reply_msg.extract();
    update_value(value);

    return Value();
}

ByteArray GattCharacteristic1::ReadValueBytes() {
    auto msg = create_method_call("ReadValue");

    // NOTE: ReadValue requires an additional argument, which currently is not supported
    SimpleDBus::Holder options = SimpleDBus::Holder::create_dict();
    msg.append_argument(options, "a{sv}");

    SimpleDBus::Message reply_msg = _conn->send_with_reply_and_block(msg);
    SimpleDBus::Holder value = reply_msg.extract();
    update_value(value);

    return ValueBytes();
}

std::string GattCharacteristic1::UUID() {
    // As the UUID property doesn't change, we can cache it
    std::scoped_lock lock(_property_update_mutex);
    return _uuid;
}

ByteStrArray GattCharacteristic1::Value() {
    std::scoped_lock lock(_property_update_mutex);
    return _value;
}

ByteArray GattCharacteristic1::ValueBytes() {
    std::scoped_lock lock(_property_update_mutex);
    return _value_b;
}

bool GattCharacteristic1::Notifying(bool refresh) {
    if (refresh) {
        property_refresh("Notifying");
    }

    std::scoped_lock lock(_property_update_mutex);
    return _properties["Notifying"].get_boolean();
}

void GattCharacteristic1::property_changed(std::string option_name) {
    if (option_name == "UUID") {
        std::scoped_lock lock(_property_update_mutex);
        _uuid = _properties["UUID"].get_string();
    } else if (option_name == "Value") {
        update_value(_properties["Value"]);
        OnValueChanged();
    }
}

void GattCharacteristic1::update_value(SimpleDBus::Holder& new_value) {
    std::scoped_lock lock(_property_update_mutex);
    auto value_array = new_value.get_array();

    char* value_data = new char[value_array.size()];
<<<<<<< HEAD
    _value_b.resize(value_array.size());
    for (unsigned int i = 0; i < value_array.size(); i++) {
=======
    for (std::size_t i = 0; i < value_array.size(); i++) {
>>>>>>> upstream/feature/next
        value_data[i] = value_array[i].get_byte();
        _value_b[i] = value_array[i].get_byte();
    }
    _value = ByteStrArray(value_data, value_array.size());
    delete[] value_data;
}
