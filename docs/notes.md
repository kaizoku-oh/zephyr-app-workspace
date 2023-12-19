### Class design
When it comes to class design for embedded systems and specifically microcontrollers there are different types of strategies.

There are BSP classes that are used to create an object that interact directly with hardware like a sensor or a peripheral, these are generally wrapper classes on top of the drivers.

There are Model classes that are used to describe and store object data, these are simply data holders that organize data in a single object replacing the old C data structuring.

There are Service classes that are used to create more complex objects that have their own threads of execution.

