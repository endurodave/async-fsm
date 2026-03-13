# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Configure (Windows - Visual Studio)
cmake -G "Visual Studio 17 2022" -A Win32 -B Build -S .

# Configure (Linux)
cmake -G "Unix Makefiles" -B Build -S .

# Build
cmake --build Build

# Run
./Build/StateMachineApp        # Linux
Build\Debug\StateMachineApp.exe  # Windows
```

There is no separate test runner — `StateMachineApp` runs all example/test state machines on launch and outputs results to stdout.

## Architecture

This project is a compact, table-driven C++ finite state machine (FSM) framework targeting both embedded and PC systems. The output binary is ~448 bytes of FSM code.

### Core FSM Framework (`StateMachine.h` / `StateMachine.cpp`)

`StateMachine` is the base class for all state machines. Subclasses define:

1. **State functions** — `STATE_DECLARE`/`STATE_DEFINE` macros generate member functions with signature `void ST_Foo(EventData*)`.
2. **State map** — `BEGIN_STATE_MAP`/`END_STATE_MAP` (basic) or `BEGIN_STATE_MAP_EX`/`END_STATE_MAP_EX` (with guards/entry/exit). This array maps each state enum value to its handler.
3. **Transition maps** — `BEGIN_TRANSITION_MAP`/`END_TRANSITION_MAP` inside each external event method. Each row maps current state → next state (or `EVENT_IGNORED` / `CANNOT_HAPPEN`).

External events call `ExternalEvent()`. States may call `InternalEvent()` to chain transitions without returning to the caller. `StateEngine()` drives execution.

**Two state map modes (pick one per class):**
- `StateMapRow` — state function only
- `StateMapRowEx` — state function + optional `GuardCondition`, `EntryAction`, `ExitAction`

Guard conditions (`GUARD_DECLARE`/`GUARD_DEFINE`, `GD_` prefix) return `bool` and block the transition if `false`. Entry (`EN_`) and exit (`EX_`) actions run on state entry/exit.

### Event Data

All event data structs inherit from `EventData`. States with no data use `NoEventData`. Ownership of heap-allocated `EventData` passes to `ExternalEvent()`; the framework deletes it after the transition.

### Example State Machines

| Class | File | States | Demonstrates |
|---|---|---|---|
| `Motor` | `Motor.cpp` | 4 | Basic transitions, `MotorData` event struct |
| `MotorNM` | `MotorNM.cpp` | 4 | Same as Motor but without macros |
| `Player` | `Player.cpp` | 5 | Multiple valid transitions, internal events |
| `SelfTest` | `SelfTest.cpp` | 3 | Reusable base class, `PARENT_TRANSITION` macro |
| `CentrifugeTest` | `CentrifugeTest.cpp` | 8 | Inheritance from `SelfTest`, guards, entry/exit, polling |

### DelegateMQ Submodule (`delegate-mq/`)

An optional asynchronous delegate library included as a subdirectory. The main FSM framework does **not** depend on it — it is provided for extending state machines with async/threaded event dispatch. Include via `delegate-mq/DelegateMQ.h`.

Key layers:
- `delegate/` — unicast, multicast, async, async-wait, and remote delegate templates
- `predef/os/` — threading abstractions for stdlib, Win32, FreeRTOS, ThreadX, Zephyr, Qt
- `predef/serialize/` — pluggable serialization (msgpack, cereal, bitsery, rapidjson)
- `predef/transport/` — network/IPC transports (UDP, TCP, pipes, serial, MQTT)
- `predef/allocator/` — `xallocator` fixed-block allocator for embedded use
