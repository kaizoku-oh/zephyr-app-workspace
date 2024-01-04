## 💡 Class Design
When it comes to class design for embedded systems and specifically for microcontrollers there are different types of strategies.

### Model / Data holder classes
- These classes are used to describe and store object data, getters and setters, they are simply data holders that organize data in a single object replacing the old C data structuring.

### BSP / Driver classes
- These classes are used to create an object that interact directly with hardware like a sensor (temperature) or a peripheral (UART), they are generally used as wrapper classes on top of the drivers.

- Those classes are generally based on manufacturer C HAL drivers that interact directly with hardware registers.

- The manufacturer C HAL drivers generally use the concept of hardware instances (ex: USART1, USART2...)

- Should those hardware instances be mapped to software objects? ex: USART1 -> Serial1

- Should the user specify to the object constructor which hardware instance to use? Aren't we breaking the abstraction there?

- Should we use a namespace instead of a class, ex: SERIAL::USART1::write()

### Apps / Service / Active Object classes
- There are active object stateful classes that manage their own state machine internally and have their own threads of execution.

- These classes can receive requests from user in an asynchronous manner and process them in a Run-To-Completion way.

- Aside from receiving requests from user, these classes can notify others/user in an asynchronous manner.

