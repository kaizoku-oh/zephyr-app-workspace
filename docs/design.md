## 💡 Class Design
When it comes to class design for embedded systems and specifically for microcontrollers there are different types of strategies and design patterns.

### Model / Data holder classes
- These classes are used to describe and store object data and expose getter and setter methods, they are simply data holders that organize data in a single object replacing the old C data structuring.

### BSP / Device classes
- These classes are generally based on zephyr device tree devices.

- These classes are used to create an object that interact directly with hardware like a sensor (temperature) or a peripheral (UART), they are generally used as wrapper classes on top of the drivers.

### App / Service / Active Object classes
- These are active object stateful classes that manage their own state machine internally and may have their own threads of execution.

- They can be used as wrappers around subsystems

- These classes can receive requests from user in an asynchronous way and process them in a Run-To-Completion way.

- Aside from receiving requests from user, these classes can notify others/user in an asynchronous way via sending events on the event channel or raising a registered callback.

