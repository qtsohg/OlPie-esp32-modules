# MQTT Interface Refactoring

## Overview

The `NetOtaMqtt` class has been refactored to use an interface-based design pattern instead of being tightly coupled to the `ToothbrushController`. This change makes the network module more flexible and reusable across different applications.

## Changes Made

### 1. Created IMqttCommandHandler Interface

A new interface `IMqttCommandHandler` was added to `/lib/olpie-esp32-modules/include/espmods/network.hpp`:

```cpp
class IMqttCommandHandler {
 public:
  virtual ~IMqttCommandHandler() = default;
  virtual void handleMqttCommand(const String &topic, const String &payload) = 0;
};
```

### 2. Updated NetOtaMqtt Class

- Changed `setController(ToothbrushController *controller)` to `setController(IMqttCommandHandler *controller)`
- Updated the private member `controller_` to use the interface type
- This allows any class that implements `IMqttCommandHandler` to work with `NetOtaMqtt`

### 3. Updated ToothbrushController

- Made `ToothbrushController` inherit from `IMqttCommandHandler`
- Added `override` keyword to `handleMqttCommand()` method
- No functional changes to the existing behavior

## Benefits

1. **Loose Coupling**: NetOtaMqtt is no longer dependent on ToothbrushController specifically
2. **Reusability**: The network module can now be used with any controller that implements the interface
3. **Testability**: Easier to create mock controllers for testing
4. **Extensibility**: New controllers can easily integrate with the MQTT functionality

## Usage Examples

### Existing Usage (ToothbrushController)
```cpp
NetOtaMqtt netModule;
ToothbrushController controller(/* parameters */);
netModule.setController(&controller);  // Still works!
```

### New Generic Controller
```cpp
NetOtaMqtt netModule;
GenericController controller;  // Any class implementing IMqttCommandHandler
netModule.setController(&controller);
```

## Example Implementation

See `/lib/olpie-esp32-modules/examples/generic_mqtt_controller/` for a complete example of how to create a new controller that works with the NetOtaMqtt module.

## Interface Contract

Any controller that wants to work with NetOtaMqtt must:

1. Inherit from `IMqttCommandHandler`
2. Implement the `handleMqttCommand(const String &topic, const String &payload)` method
3. Handle MQTT commands appropriately for their specific use case

This design follows the **Dependency Inversion Principle** from SOLID principles, making the code more maintainable and flexible.