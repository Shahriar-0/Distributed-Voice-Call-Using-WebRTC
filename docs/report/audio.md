# Project Report: AudioOutput and AudioInput Classes

## Overview

The `AudioOutput` and `AudioInput` classes manage audio data transmission and reception, essential for real-time audio streaming. The `AudioInput` class captures raw audio data, encodes it using the Opus codec, and emits it to be sent over the network. The `AudioOutput` class receives encoded audio data, decodes it, and plays it back. Both classes leverage Qt’s audio capabilities and Opus for audio compression, which is highly effective for real-time communication.

## Class Attributes

### Common Opus Settings

Both classes use an **Opus Frame Size** of 960 samples per frame. This frame size balances latency and quality, providing high-quality audio without excessive delay.

### `AudioOutput` Attributes

- **`audioFormat`**: `QAudioFormat`
  - Specifies the audio format for playback, set to 48 kHz, mono channel, and 16-bit integer samples.
- **`audioSink`**: `QAudioSink*`
  - The audio sink for outputting decoded audio data to the device.
- **`audioDevice`**: `QIODevice*`
  - Represents the audio output device for playback.
- **`opusDecoder`**: `OpusDecoder*`
  - The Opus decoder for handling encoded audio data.
- **`audioQueue`**: `QQueue<QByteArray>`
  - Stores encoded audio packets received for playback.
- **`mutex`**: `QMutex`
  - Ensures thread-safe access to `audioQueue`.

### `AudioInput` Attributes

- **`audioSource`**: `QAudioSource*`
  - Represents the audio input device for capturing raw audio data.
- **`opusEncoder`**: `OpusEncoder*`
  - The Opus encoder for compressing raw audio data.

---

## Methods

### `AudioOutput::AudioOutput(QObject* parent = nullptr)`

- **Description**: Initializes the `AudioOutput` instance, setting up the audio format, Opus decoder, and audio sink.
- **Code Explanation**:
  - Sets the `audioFormat` parameters, including sample rate (48 kHz), channel count (1 for mono), and sample format (`QAudioFormat::Int16`).
  - Initializes the Opus decoder with `opus_decoder_create` for 48 kHz, mono audio. If the decoder creation fails, it logs a warning with the error message.
  - Creates `audioSink` with `audioFormat` settings and starts it by assigning `audioDevice`.
  - Connects the `newPacket` signal to the `play` slot, ensuring that playback occurs when new data arrives.

### `AudioOutput::~AudioOutput()`

- **Description**: Cleans up resources, specifically the Opus decoder.
- **Code Explanation**:
  - Calls `opus_decoder_destroy` to release memory allocated for the decoder.

### `void AudioOutput::addData(const QByteArray& data)`

- **Description**: Adds encoded audio data to the queue and signals for playback.
- **Code Explanation**:
  - Uses `QMutexLocker` to safely enqueue `data` into `audioQueue`.
  - Emits the `newPacket` signal, which triggers `play` for playback.

### `void AudioOutput::play()`

- **Description**: Decodes and plays audio data from the `audioQueue`.
- **Code Explanation**:
  - Dequeues encoded data from `audioQueue` if available.
  - Decodes the data using `opus_decode` into an array of `opus_int16` samples. If decoding fails, it logs an error.
  - Writes the decoded data to `audioDevice` for playback.

---

### `AudioInput::AudioInput(QObject* parent = nullptr)`

- **Description**: Initializes the `AudioInput` instance, setting up the Opus encoder and audio format.
- **Code Explanation**:
  - Creates an Opus encoder with `opus_encoder_create`, setting a 48 kHz sample rate, mono channel, and VOIP application mode for optimized voice encoding.
  - Configures encoder settings, including a bitrate of 64 kbps, maximum complexity, and Forward Error Correction (FEC) for network resilience.
  - Defines `audioFormat` for 48 kHz, mono channel, and 16-bit samples, then checks if the default input device supports this format.
  - Creates `audioSource` to capture audio with `audioFormat`.

### `AudioInput::~AudioInput()`

- **Description**: Cleans up resources, specifically the Opus encoder.
- **Code Explanation**:
  - Calls `opus_encoder_destroy` to release memory allocated for the encoder.

### `bool AudioInput::startAudio()`

- **Description**: Starts capturing audio, making the class ready for recording.
- **Code Explanation**:
  - Opens the device and starts `audioSource` if it is not active. Logs a warning if `audioSource` fails to enter the `ActiveState`.

### `bool AudioInput::stopAudio()`

- **Description**: Stops capturing audio, ending the recording process.
- **Code Explanation**:
  - Stops `audioSource` and logs a warning if it fails to enter the `StoppedState`.

### `qint64 AudioInput::writeData(const char* data, qint64 len)`

- **Description**: Encodes raw audio data using Opus and emits it as `newAudioData`.
- **Code Explanation**:
  - Encodes `data` with `opus_encode`, storing the compressed result in `encodedData`.
  - Emits `newAudioData` with the encoded audio, logging the size of raw and encoded audio.

### `qint64 AudioInput::readData(char* data, qint64 len)`

- **Description**: Unused method required by `QIODevice`.

---

## Challenges in Encoding and Decoding

1. **Latency and Frame Size Optimization**:
Balancing latency and quality was challenging. Low latency is critical for real-time applications, but using too small a frame size can lead to poor audio quality and increased packet overhead. A frame size of 960 samples was chosen as a compromise, offering good audio quality while keeping latency manageable.

2. **Network Variability**:
Real-time audio transmission is vulnerable to network issues such as packet loss and jitter. To address this, FEC was enabled in the Opus encoder, allowing the decoder to handle some lost packets and maintain quality during brief network disruptions.

3. **Audio Quality and Bitrate Trade-offs**:
Choosing an optimal bitrate is essential; lower bitrates reduce network load but can affect audio quality. A bitrate of 64 kbps was set as it provides clear audio quality suitable for voice transmission while minimizing bandwidth usage.

4. **Error Handling in Encoding and Decoding**:
The Opus encoder and decoder can return errors when processing data. For example, invalid packets or corrupted data can cause decoding errors, leading to dropped frames. Proper error handling with warnings and checks helped identify and handle such issues effectively.

## Why This Format Works

Using a 48 kHz sample rate, mono channel, and 16-bit depth was essential for compatibility with standard audio hardware and maintaining high voice quality. The Opus codec was selected for its efficiency and quality at low bitrates, making it ideal for real-time communication over variable networks. Opus also provides features like FEC and variable bitrate, which enhance quality during network issues. By setting a frame size of 960 samples, the design balances latency with audio quality, providing a smooth, clear audio experience essential for real-time applications.
