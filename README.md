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
