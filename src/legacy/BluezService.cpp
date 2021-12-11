#include "BluezService.h"

#include <algorithm>

BluezService::BluezService()
    : _conn(std::make_shared<SimpleDBus::Connection>(DBUS_BUS_SYSTEM)),
      object_manager(_conn, "org.bluez", "/"),
      Introspectable{_conn, "org.bluez", "/"} {
    object_manager.InterfacesAdded = [&](std::string path, SimpleDBus::Holder options) { add_path(path, options); };
    object_manager.InterfacesRemoved = [&](std::string path, SimpleDBus::Holder options) {
        remove_path(path, options);
    };
}

BluezService::~BluezService() { _conn->remove_match("type='signal',sender='org.bluez'"); }

void BluezService::init() {
    _conn->init();
    object_manager.GetManagedObjects(true);  // Feed the objects via callback.

    _conn->add_match("type='signal',sender='org.bluez'");
}

void BluezService::run_async() {
    _conn->read_write();
    SimpleDBus::Message message = _conn->pop_message();
    while (message.is_valid()) {
        switch (message.get_type()) {
            case SimpleDBus::Message::Type::SIGNAL:
                process_received_signal(message);
                break;
            default:
                break;
        }
        message = _conn->pop_message();
    }
}

void BluezService::process_received_signal(SimpleDBus::Message& message) {
    std::string path = message.get_path();

    if (path == "/org/freedesktop/DBus") return;

    if (object_manager.process_received_signal(message)) return;

    for (auto& [path, adapter] : adapters) {
        if (adapter->process_received_signal(message)) return;
    }
}

void BluezService::add_path(std::string path, SimpleDBus::Holder options) {
    int path_elements = std::count(path.begin(), path.end(), '/');

    switch (path_elements) {
        case 2:
            agent.reset(new BluezAgent(path, options));
            break;
        case 3:
            adapters.emplace(std::make_pair(path, new BluezAdapter(_conn, path, options)));
            break;
        default:
            // Propagate the paths downwards until someone claims it.
            for (auto& [adapter_path, adapter] : adapters) {
                if (adapter->add_path(path, options)) return;
            }
            break;
    }
}

void BluezService::remove_path(std::string path, SimpleDBus::Holder options) {
    int path_elements = std::count(path.begin(), path.end(), '/');

    switch (path_elements) {
        case 2:
            break;
        case 3: {
            // HACK: Due to the library architecture, the only way to make sure if one
            // of the underlying objects has to be deleted is to check if the options
            // list contains `org.bluez.Adapter1`, `org.freedesktop.DBus.Introspectable`,
            // or `org.freedesktop.DBus.Properties`.
            // Eventually this structure will have to be revisited.
            std::vector<std::string> interface_list;
            for (auto& option : options.get_array()) {
                interface_list.push_back(option.get_string());
            }
            bool must_erase = std::any_of(interface_list.begin(), interface_list.end(), [](const std::string& str) {
                return str == "org.freedesktop.DBus.Properties" || str == "org.freedesktop.DBus.Introspectable" ||
                       str == "org.bluez.Adapter1";
            });
            if (must_erase) {
                adapters.erase(path);
            }
            break;
        }
        default:
            // Propagate the paths downwards until someone claims it.
            for (auto& [adapter_path, adapter] : adapters) {
                if (adapter->remove_path(path, options)) return;
            }
            break;
    }
}

std::shared_ptr<BluezAdapter> BluezService::get_first_adapter() {
    std::shared_ptr<BluezAdapter> return_value = nullptr;
    if (!adapters.empty()) {
        return_value = adapters.begin()->second;
    }

    return return_value;
}

std::shared_ptr<BluezAdapter> BluezService::get_adapter(std::string adapter_name) {
    std::shared_ptr<BluezAdapter> return_value = nullptr;
    std::string expected_path = "/org/bluez/" + adapter_name;

    for (auto& [adapter_path, adapter] : adapters) {
        if (adapter_path == expected_path) {
            return_value = adapter;
            break;
        }
    }
    return return_value;
}

std::vector<std::shared_ptr<BluezAdapter>> BluezService::get_all_adapters() {
    std::vector<std::shared_ptr<BluezAdapter>> adapter_list;
    for (auto& [adapter_path, adapter] : adapters) {
        adapter_list.push_back(adapter);
    }
    return adapter_list;
}
