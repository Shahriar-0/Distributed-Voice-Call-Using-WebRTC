# Project Report: Client Class

## Overview

The `Client` class manages the client-side WebRTC signaling and audio communication for a peer-to-peer connection, leveraging the `WebRTC`, `AudioInput`, and `AudioOutput` classes. The `Client` handles the following key responsibilities:

- Establishing a connection to a signaling server.
- Managing SDP (Session Description Protocol) offers and answers to set up WebRTC communication.
- Transmitting and receiving real-time audio packets between peers.

The `Client` class integrates TCP for signaling communication with a server and WebRTC for P2P media streaming, ensuring a complete setup for real-time communication between users.

## Attributes

### Core Attributes

- **`webrtc`**: `WebRTC`
  - Handles WebRTC functionalities like peer connection, SDP generation, and packet transmission.
- **`audioinput`**: `AudioInput`
  - Manages capturing and encoding of local audio data for transmission.
- **`audiooutput`**: `AudioOutput`
  - Decodes and plays received audio data.
- **`socket`**: `QTcpSocket*`
  - Connects to the signaling server for SDP and ICE (Interactive Connectivity Establishment) exchange.
- **`clientId`**: `QString`
  - Unique identifier for the client, used in server communication.
- **`peerId`**: `QString`
  - Identifier for the current peer connection, initialized as `clientId`.
- **`otherClientId`**: `QString`
  - Stores the ID of the peer to which the client is currently connected.

---

## Methods

### Constructor and Connection Management

#### `Client(QObject* parent = nullptr, const QString &clientId = "", bool isOfferer = false)`

- **Description**: Constructs the `Client` instance, initializes WebRTC, sets up audio, and connects to the signaling server.
- **Code Explanation**:
  - Initializes `clientId` and `peerId` to represent this client in signaling communications.
  - Creates a `QTcpSocket` instance (`socket`) to connect to the signaling server, binding `onConnected`, `onReadyRead`, and `onDisconnected` slots to manage server communication.
  - Calls `webrtc.init` to set up WebRTC configuration, specifying whether this client is an offerer (initiator) or an answerer.
  - Adds `peerId` as a peer for WebRTC communication and connects relevant WebRTC signals (e.g., `offerIsReady`, `answerIsReady`, and `incommingPacket`) to handle SDP readiness and packet reception.
  - Connects `audioinput`’s `newAudioData` signal to the `packetReady` slot, facilitating audio transmission to the peer.

#### `void connectToServer(const QString &serverIp, int serverPort)`

- **Description**: Establishes a TCP connection to the signaling server using the provided IP address and port.
- **Code Explanation**:
  - Calls `socket->connectToHost` with the server IP and port, initiating the TCP connection required for signaling communications.

---

### Call Management

#### `void offerCall(QString answererId)`

- **Description**: Initiates a call by generating an SDP offer for the specified `answererId`.
- **Code Explanation**:
  - Sets `otherClientId` to `answererId`, representing the peer that will receive the offer.
  - Invokes `webrtc.generateOfferSDP` to generate the local SDP offer and begin the WebRTC handshake.

#### `void answerCall(QString offererId, QString offererSdp)`

- **Description**: Responds to an incoming SDP offer by setting the remote description and generating an SDP answer.
- **Code Explanation**:
  - Sets `otherClientId` to `offererId`, indicating the peer that initiated the offer.
  - Calls `webrtc.setRemoteDescription` with `offererSdp` to configure the peer connection, followed by `webrtc.generateAnswerSDP` to generate and send the answer SDP.

#### `void confirmCall(QString answererSdp)`

- **Description**: Confirms the call by setting the answerer’s SDP as the remote description, finalizing the P2P setup.
- **Code Explanation**:
  - Calls `webrtc.setRemoteDescription` with `answererSdp`, completing the SDP exchange required to establish a P2P connection.

#### `void startCall()`

- **Description**: Starts audio input for the call.
- **Code Explanation**:
  - Calls `audioinput.startAudio` to begin capturing audio data. If it fails, a warning is logged.

#### `void endCall()`

- **Description**: Stops audio input, ending the call.
- **Code Explanation**:
  - Calls `audioinput.stopAudio` to halt audio capturing. If it fails, a warning is logged.

---

### Packet Handling

#### `void packetRecieved(const QString& peerId, const QByteArray& data, qint64 len)`

- **Description**: Processes received audio packets by adding them to `audiooutput` for playback.
- **Code Explanation**:
  - Calls `audiooutput.addData` with the `data`, queuing it for playback. Logs the reception of the packet.

#### `void packetReady(const QByteArray& buffer)`

- **Description**: Sends audio packets to the peer via WebRTC.
- **Code Explanation**:
  - Invokes `webrtc.sendTrack` with `peerId` and `buffer`, transmitting the audio data to the connected peer. Logs when a packet is ready for transmission.

---

### SDP Management

#### `void offererIsReady(const QString& peerId, const QString& sdp)`

- **Description**: Sends the generated offer SDP to the signaling server.
- **Code Explanation**:
  - Logs that the offer SDP is ready and calls `sendSdpToServer`, setting `clientType` as "offerer" to distinguish this as the initiating SDP.

#### `void answererIsReady(const QString& peerId, const QString& sdp)`

- **Description**: Sends the generated answer SDP to the signaling server.
- **Code Explanation**:
  - Logs that the answer SDP is ready and calls `sendSdpToServer`, setting `clientType` as "answerer" to distinguish this as the responding SDP.

#### `void sendSdpToServer(const QString &sdp, const QString &targetId, const QString &clientType)`

- **Description**: Transmits the specified SDP to the signaling server along with client type and target information.
- **Code Explanation**:
  - Checks if `socket` is connected. If so, creates a `QJsonObject` message containing `sourceId`, `clientType`, `sdp`, and `targetId`.
  - Converts the JSON object to a `QJsonDocument` and sends it to the signaling server. Logs a message if the connection is not active.

---

### Signaling Server Communication

#### `void onConnected()`

- **Description**: Executes when a connection to the signaling server is established, sending the client ID.
- **Code Explanation**:
  - Logs successful connection and sends a JSON message containing `clientId` to register this client with the server.

#### `void onReadyRead()`

- **Description**: Reads incoming data from the signaling server, processes SDP offers and answers, and invokes appropriate call methods.
- **Code Explanation**:
  - Reads and parses JSON data from the server, extracting `sourceId`, `clientType`, and `sdp` fields.
  - If `clientType` is "offerer," calls `answerCall` with the `sourceId` and SDP; if "answerer," it calls `confirmCall` with the SDP.
  - Emits `sdpReceived` to notify other components of the received SDP.

#### `void onDisconnected()`

- **Description**: Executes when the client disconnects from the signaling server.
- **Code Explanation**:
  - Logs the disconnection event to notify that the client is no longer connected to the server.
