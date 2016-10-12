yet another DSA measurement tool
================================

the responder simply publishes bunch of messages periodically.
DSLink at the responder adds timestamp to the message.
the requester subscribes the messages
and recoreds the timestamp with the time at receiving.
the requester finishes to subscribe until the message counter
reaches the specified number,
or the number of messages specified are received.

## how to build

set the full path to the DSLink SDK directory into DSLINK_SDK_DIR.
clone the smelt library.  Then, make it.

    e.g.
    % export DSLINK_SDK_DIR=/opt/dsa/sdk-dslink-c
    % git clone https://github.com/tanupoo/smelt.git
    % make

## how to run

don't forget to set LD_LIBRARY_PAT like below.

    e.g.
    % export LD_LIBRARY_PATH=$DSLINK_SDK_DIR/build:./smelt

- responder

    % ./pub -h
    Usage: pub [OPTIONS]
        -B,--broker=url     set the broker URL. (required)
                            e.g. http://dsa-broker.example.com/conn
        -D,--dslink=string  set the dslink name of this. (default:pub)
        -l,--log=level      set the log level.  (info,none,..., default:info)
        -s,--size=size      set the size in bytes of the value. (default:40)
        -i,--interval=num   set the interval in msec to update. (default:1000)
        -m,--max-count=num  set the max value of the counter. (default:10)
                            i.e. how many messages to be sent.
        -n,--node=string    set the dslink name of this. (default:test_node)
        -d,--debug          increase verbosity.
        -h,--help           show this help menu.

    e.g.
    % ./pub -B http://1.1.3.142/conn -d

- requester

    % ./sub -h
    Usage: sub [OPTIONS]
        -B,--broker=url     set the broker URL. (required)
                            e.g. http://dsa-broker.example.com/conn
        -D,--dslink=string  set the dslink name of this. (default:sub)
        -l,--log=level      set the log level.  (info,none,..., default:info)
        -m,--max-count=num  set the max value of the counter. (default:10)
                            see -p option also.
        -p,--policy=num     set the policy to decide whether to finish
                            the measurement. (default:1)
                              0: counter in the message reaches.
                              1: the number of received messages.
        -n,--node=string    set the node name to be subscribed.
                            (default:/downstream/pub/test_node)
        -q,--qos=num        set the level of the QoS. (default:1)
        -d,--debug          increase verbosity.
        -h,--help           show this help menu.

    e.g.
    % ./sub -B http://1.1.3.142/conn -d

