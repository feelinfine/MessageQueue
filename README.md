# MessageQueue

A simple library, that provides you to create a queue of popup message windows in your qt-project.

Threadsafe.

Example 1:

![](https://github.com/feelinfine/MessageQueue/blob/dev/22.gif)


Example 2:

![](https://github.com/feelinfine/MessageQueue/blob/dev/33.gif)

MessageQueue is a singleton class.

You can use it as follow:
```c++
MessageQueue& instance = MessageQueue::instance();

//no ownership
//messages will be binded to the right bottom corner of base_widget (QWidget*) 
//installs an event filter to this widget
instance.set_base_widget(base_widget);	

//how long message will be displayed (closes automatically)
instance.set_msg_close_time(5000);  //msec

//how many messages can be displayed at the same time
instance.set_active_size_limit(3);

//pointer to output device (maybe a log file). All messages will be also written to this device if exists
QFile* out = new QFile("log.txt");
out->open(QIODevice::WriteOnly | QIODevice::Text);
instance.set_output_device(out);

MessageQueue::instance() << WarnMessage("Some warning message");
MessageQueue::instance() << ErrorMessage("Some error message");
```
