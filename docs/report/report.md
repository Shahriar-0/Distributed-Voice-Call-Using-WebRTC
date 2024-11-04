*The very detailed explaination of function, methods, attributes and signals are in each class' markdown. Here, we explain the reason and functionality of eaach class breifly.*



**1. Signaling Server:**

- to learn more about QT and signals, we decided to write signaling server with QT
- The signalig server is a TCP socket which clients can connect to it.
- after clients connect to the signaling server, they send their "id" to it, so when a client want to make a call to an id, signaling server send the sdp to that id and vice versa.

CHALLENGE: there was a bug we faced in the sending messages. We should wait until socket connect, then send a message. it was the first time we understood the reason behing signal. we send message just right after we call the `connectToServer` function, so it was not working well.

- the signaling server exchange the sdps of clients.
- The id of client who connect to server will be logged, also disconnections will be logged.


**2. WebRTC:**

- webrtc implemented just right it was asked to.
- There was some functions and variables which were not needed, `m_peerConnections` for example.
- The flow of program is: the client who wants to make a call, call the `offerCall` function, then an offerer sdp will be created, and will be passed to the signaling server. the signaling server send the sdp to the given ID(which is connected to the signaling server and we stored the socket of it in `clients` in `signalingServer`). after that the second client receive the sdp and call the `answerCall` function, we generate an answerer sdp for local client(second one) and we set the first client's sdp as its remote. after that we send back the sdp of second client and we set as the first client's remote sdp in the `confirmCall` function. finally two clients now has a p2p connection to each other.


**3. Client:**

- we create an instance of client in the `main.cpp` and connect to the server.
- the ip and port of the socket which clients try to connect must be same as the server ip and server port, please pay attention that they are hardcoded to `localhost` and `9000`.
- The signals and slots of client are so important, we explain them:
    - three slots, `onConnected`, `onReadyRead` and `onDisconnected` are for socket connection.
    - `offerIsReady` and `answerIsReady` from webrtc connect to the same-named slots, which client send its sdp to the other client right after it.
    - `incommingPacket` from `webrtc` is connected to `packetRecieved`, which pass the packet to audio classes to play it.
    - `newAudioData` from `audioinput` is connected to `packetReady` which pass it to the webrtc to send the voice packet to other client.
    - `p2pConnected` and `p2pDisConnected` are for start/stop audio.

**4. Audio:**

- for ease of use, all hyperparameters are hard-coded.
- the `AudioInput` record start recording the data and when a packet is ready, write it to the buffer and emit the `newAudioData` signal.
- `AudioOutput` use a queue to play all the queued packets.(we do not batch them, we send/play them real time) the packets will be given to it using `addData`, which be called after receiving `incommingPacket` in `packetReady` slot.


**5. UI:**

- we hardcoded the callerId in the `main.cpp` and `main.qml` (just for showing it).
- The caller enter the id of the client who they want to call, after that it press the green button.
- when p2p connection is set, the button will be red for both client.
- both client can end the call.
- ending connection will remove the peer connection and stop the audio functions.
- pressing the red button will result in end p2p connection for both side. both buttons will be green again and both client now can use the app again for calling. 
- There is no limitation for a client to be offerer or answerer. a client can be offerer or answerer, even in one run of the app.