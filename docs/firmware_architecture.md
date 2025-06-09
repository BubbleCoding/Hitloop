# Firmware Architecture

This document provides a high-level overview of the firmware architecture for the HitLoop Scanner device.

## Core Design Philosophy

The firmware is designed around two core principles:

1.  **Modularity:** Each major piece of functionality (e.g., managing Wi-Fi, handling BLE scans, controlling LEDs) is encapsulated in its own "Manager" class.
2.  **Decoupling:** Instead of Managers calling each other directly, they communicate using an **event-driven system**. A central `EventManager` is used to broadcast events, and managers subscribe to the events they care about. This reduces dependencies and makes the system easier to modify and test.

All managers inherit from a base `Process` class, which ensures they have a common `setup()` and `update()` interface that is called by the main `Scanner.ino` sketch.

## System Components and Data Flow

The diagram below illustrates the primary components of the system and how they interact. The `EventManager` is the central hub through which all communication flows.

```mermaid
graph TD
    subgraph "Main Loop (Scanner.ino)"
        direction LR
        loop(Loop) -.-> BleManager
        loop -.-> WifiManager
        loop -.-> LedManager
        loop -.-> VibrationManager
    end

    subgraph "Hardware/Logic Managers (Processes)"
        BleManager
        DataManager
        HTTPManager
        BehaviorManager
        WifiManager
    end

    subgraph "Actuator Managers"
        LedManager
        VibrationManager
    end

    subgraph "Event System"
        EventManager((EventManager))
    end

    BleManager -- Publishes --> ScanCompleteEvent
    ScanCompleteEvent -- Notifies --> EventManager
    EventManager -- Subscribed --> DataManager

    DataManager -- Publishes --> DataReadyForHttpEvent
    DataReadyForHttpEvent -- Notifies --> EventManager
    EventManager -- Subscribed --> HTTPManager

    HTTPManager -- Publishes --> HttpResponseEvent
    HttpResponseEvent -- Notifies --> EventManager
    EventManager -- Subscribed --> BehaviorManager

    HTTPManager -- Publishes --> ServerDisconnectedEvent
    ServerDisconnectedEvent -- Notifies --> EventManager
    EventManager -- Subscribed --> BehaviorManager

    WifiManager -- Publishes --> WifiConnectedEvent
    WifiConnectedEvent -- Notifies --> EventManager
    EventManager -- Subscribed --> BehaviorManager

    BehaviorManager -- Controls --> LedManager
    BehaviorManager -- Controls --> VibrationManager

    style loop fill:#f9f,stroke:#333,stroke-width:2px
    style EventManager fill:#cff,stroke:#333,stroke-width:2px
```

### Data Flow Example: A Full Cycle

1.  `BleManager`'s timer fires, and it initiates a BLE scan.
2.  When the scan completes, `BleManager` publishes a `ScanCompleteEvent` containing the results.
3.  `DataManager`, which is subscribed to this event, receives it. It processes the raw scan data, combines it with IMU data, and formats it into a JSON payload.
4.  `DataManager` then publishes a `DataReadyForHttpEvent` containing the JSON payload.
5.  `HTTPManager` receives this event, opens a connection to the server, and POSTs the data.
6.  When the server responds, `HTTPManager` publishes an `HttpResponseEvent` with the server's payload (or a `ServerDisconnectedEvent` on failure).
7.  `BehaviorManager` receives the response event. It parses the payload for any behavior commands (`led_behavior`, `vibration_behavior`) or synchronization data (`wait_ms`).
8.  If there are behavior commands, `BehaviorManager` tells the appropriate manager (`LedManager` or `VibrationManager`) which behavior to use from its pool.
9.  The `LedManager` or `VibrationManager` then runs the `update()` loop for that behavior on its own, independent of the main loop, ensuring smooth animations.

## The Behavior Pattern

The firmware uses a "Behavior" pattern to define how the LEDs and vibration motor act. This makes it easy to add new animations or effects.

-   **Base Class:** A base class (`LedBehavior` or `VibrationBehavior`) defines a common interface with `setup()`, `update()`, and `updateParams()` methods.
-   **Concrete Classes:** Specific effects like `SolidBehavior`, `HeartBeatBehavior`, or `BurstVibrationBehavior` inherit from the base class and implement the logic for that effect.
-   **BehaviorManager:** This manager holds a "pool" of all available behavior objects. When it receives a command from the server, it looks up the requested behavior in its pool, updates its parameters (e.g., color, frequency), and tells the relevant `LedManager` or `VibrationManager` to use it.
-   **Actuator Managers:** The `LedManager` and `VibrationManager` are simple. They only hold a pointer to the *current* active behavior and are responsible for calling its `update()` method. The `LedManager` uses a `Ticker` to do this at a fixed interval, ensuring animations are smooth.

### LED Behavior Class Diagram

```mermaid
classDiagram
    direction LR
    class LedBehavior {
        <<Interface>>
        +type: const char*
        +setup(pixels) void
        +update() void
        +updateParams(params) void
    }

    class LedsOffBehavior {
    }
    class SolidBehavior {
        +color: uint32_t
    }
    class BreathingBehavior {
        +color: uint32_t
    }
    class HeartBeatBehavior {
        +color: uint32_t
        +pulse_duration: ulong
        +pulse_interval: ulong
        +setParams()
    }
    class CycleBehavior {
        +color: uint32_t
        +delay: int
    }

    LedBehavior <|-- LedsOffBehavior
    LedBehavior <|-- SolidBehavior
    LedBehavior <|-- BreathingBehavior
    LedBehavior <|-- HeartBeatBehavior
    LedBehavior <|-- CycleBehavior
``` 