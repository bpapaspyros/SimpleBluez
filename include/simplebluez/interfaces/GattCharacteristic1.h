#pragma once

#include <simpledbus/advanced/Interface.h>
#include <simpledbus/external/kvn_safe_callback.hpp>

#include <simplebluez/Types.h>

#include <string>

namespace SimpleBluez {

class GattCharacteristic1 : public SimpleDBus::Interface {
  public:
    typedef enum { REQUEST = 0, COMMAND } WriteType;

    GattCharacteristic1(std::shared_ptr<SimpleDBus::Connection> conn, std::string path);
    virtual ~GattCharacteristic1();

    // ----- METHODS -----
    void StartNotify();
    void StopNotify();
    void WriteValue(const ByteStrArray& value, WriteType type);
    ByteStrArray ReadValue();

    // ----- PROPERTIES -----
    std::string UUID();
    ByteStrArray Value();
    bool Notifying(bool refresh = true);

    // ----- CALLBACKS -----
    kvn::safe_callback<void()> OnValueChanged;

  protected:
    void property_changed(std::string option_name) override;
    void update_value(SimpleDBus::Holder& new_value);

    std::string _uuid;
    ByteStrArray _value;
};

}  // namespace SimpleBluez
