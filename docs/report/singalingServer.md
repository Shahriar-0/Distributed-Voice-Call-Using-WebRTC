# Project Report: SignalingServer Class

## Overview

The `SignalingServer` class is a TCP server implemented with the Qt framework, designed to manage connections between clients. This server acts as a signaling server, handling incoming client connections, processing messages, and enabling peer-to-peer communication through unique client identifiers. By listening on a specified IP and port, the server manages data routing based on client IDs, facilitating data exchange between connected clients.

## Attributes

- **`clients`**: `QMap<QString, QTcpSocket*>`
  - A mapping between each client ID (`QString`) and its associated socket (`QTcpSocket`). This attribute allows the server to identify and communicate with connected clients directly.

---

## Methods

### `SignalingServer(QObject *parent = nullptr)`

- **Description**: Constructor for the `SignalingServer` class. Sets up a connection to trigger `onNewConnection` whenever a new client connects.
- **Code Explanation**:
  - In the constructor, `QTcpServer`'s `newConnection` signal is connected to the `onNewConnection` slot. This ensures that each new client triggers the `onNewConnection` handler, where further connection handling is established.

### `bool startServer(const QHostAddress &address, quint16 port)`

- **Description**: Starts the server on the specified IP address and port.
- **Code Explanation**:
  - This method calls `listen(address, port)`, which is a `QTcpServer` function that opens the server on the given network parameters. If successful, the server is ready to accept incoming connections and returns `true`; otherwise, `false` indicates an error.

### `void incomingConnection(qintptr socketDescriptor)`

- **Description**: Accepts a new incoming connection using a socket descriptor, setting up a new `QTcpSocket` for each client.
- **Code Explanation**:
  - This overridden method first creates a new `QTcpSocket` instance for the incoming client and assigns its `socketDescriptor`.
  - The method connects two signals from this socket: `readyRead`, which triggers `onReadyRead` to handle incoming data, and `disconnected`, which triggers `onDisconnected` when the client disconnects.
  - The `socketDescriptor` uniquely identifies each connection, allowing the server to manage individual client connections efficiently.

### `void onNewConnection()`

- **Description**: Triggered when a new client connection is accepted, registering the client with the server.
- **Code Explanation**:
  - This method calls `nextPendingConnection()` to retrieve the socket for the new client connection. This socket can then be used for direct communication with the client, and additional handling can be implemented if needed.

### `void onReadyRead()`

- **Description**: Processes incoming data from the connected client, extracting JSON information to route messages to the appropriate target client.
- **Code Explanation**:
  - First, the `onReadyRead` method casts the sender to `QTcpSocket*`, representing the client’s socket. This allows the server to read data from the specific client.
  - Data is read from the socket as `QByteArray` and converted to a `QJsonDocument` object, which is then parsed to `QJsonObject`.
  - The JSON object contains fields such as `clientId`, `sourceId`, `clientType`, `sdp`, and `targetId`. The server uses these fields to determine the source and destination of the message.
  - If the message contains a valid `clientId`, the server updates the `clients` map with this client’s information.
  - If a `targetId` is specified and exists in the `clients` map, the message is forwarded to the target client. A new `QJsonObject` response is created, containing the source details (`sourceId`, `clientType`, `sdp`), and sent to the target client’s socket using `write`.

### `void onDisconnected()`

- **Description**: Manages client disconnection, removing the client from the server’s `clients` map to free resources.
- **Code Explanation**:
  - The method identifies the disconnected client and removes it from the `clients` map.
  - It deletes the socket for cleanup and outputs a debug message, indicating the disconnection.

### `QTcpSocket* getClientSocketById(const QString &clientId)`

- **Description**: Retrieves the `QTcpSocket` associated with a specific `clientId`.
- **Code Explanation**:
  - This helper function checks if a `clientId` exists in the `clients` map. If the client is found, the method returns the corresponding socket. If not, it returns `nullptr`.
  - This allows other methods to access a client’s socket efficiently by ID, facilitating targeted message delivery.

### `bool hasThisClient(const QString &clientId)`

- **Description**: Checks if a client with a specified `clientId` is connected to the server.
- **Code Explanation**:
  - The method looks up `clientId` in the `clients` map. It returns `true` if the client is connected, otherwise `false`.
  - This check is useful for determining if a target client is available before attempting message routing.
