# Client Server Subscriber
A simple C++ implementation of client-server-subscriber model.
Clients produce information for the subscribers to consume. The server is the middleman to ensure that the right information is sent to each subscriber.
In this implementation, the client produces headlines. The headline produced has the following format:
```
"<string:category>" : "<string:title>" : <int:priority>
```
Each subscriber informs the server of the category of headlines it is interested in. The server receives headlines from clients and periodically flushes the correct headlines to the respective subscribers. The subscribers receive the headlines in order of highest priority.
 

## How to compile the code
The system uses `Cmake` to compile code and produce targets. This repo has 2 sample implementations of `Client` and `Server` that can be built using:
```
# Building Both Server & Client
cmake --build /path/to/client_server_subscriber/build --target all

# Building only Server
cmake --build /path/to/client_server_subscriber/build --target Sample_Server

# Building only Client
cmake --build /path/to/client_server_subscriber/build --target Sample_Client
```

## How to run the code
#### Server
To run the server it needs a configuration file and an out folder path to output subscriber files. The config file contains the key-value pair of subscriber and their category of interest:
```
<hex-subscriber_id> : "<string-category>"
```
```
# Running the server binary
./build/Sample_Server path/to/config/file path/to/out/dir/
```

#### Client
The client binary needs an input text file filled with headlines of the format:
```
"<string:category>" : "<string:title>" : <int:priority>
```
```
# Running the client binary
./build/Sample_Client path/to/headlines/file
```

There can only be one instance of server running since the socket port is fixed. There can however be as many instances of Client as the system can support. 

## Implementation Overview
The server manages 3 primary threads.
 1. **Listener Thread** - Thread to listen for new Client connections
 2. **Client Manager Thread** -  Thread to manage existing Client connections. Processes incoming messages and client disconnects.
 3. **Publisher Thread** - Thread to manage publishing(writing to disk) messages for subscribers.

The Listener thread upon receiving a new connection, passes on the management of it to the Client Manager Thread. The Client manager thread listens to changes in the socket of this new connection and existing ones, for incoming message or disconnections. 
The client manager thread maintains a list of messages received, and periodically pushes them onto the queue of each subscriber. It is also responsible for sending back ACKs to clients so they can send the next message.

The Publisher thread periodically flushes the priority queue in each subscriber object onto the disk, thus accomplishing the task of publishing the headlines for each subscriber. 

In a real world scenario, the Listener thread, instead of pushing the items to each subscriber's queue, could simply make an RPC/REST call to a service that manages the subscriber queue and its publishing. Services like RabbitMQ's priority queue, which stores the information persistently can be used to scale the system and make if fault tolerant.

## Opportunities for scaling
1. Instead of a single Client Manager thread there can be multiple instances of it, such that when a new client connects, the Listener thread can pass it on to a different Client Manager thread in a Round Robin manner. It can also be smart and see which Client Manager thread is least busy and assign the new client to this least busy thread.
2. The Publisher thread can be scaled with the help of a thread worker pool. Instead of a single thread managing the publishing of items for each subscriber, a pool of worker threads can each pick up a task of publishing to a subscriber. In this case, publishing is writing to a file on disk. Since each subscriber file is separate, this operation can be horizontally scaled quite easily.

