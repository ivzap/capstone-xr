# capstone-xr
stream microscope frames to metaquest clients.

## System Design
The high-level design of the streamer can be seen below. The streamer acts as a server that accepts connections from metaquest clients and streams frames to any that are currently connected. The transport protocol used to stream the frames to the clients is UDP. We decide who to send the packets to based off another process that actively maintains a list of valid clients i.e those that are currently connected, we will simply query this updated list. To maintain the tcp connection between the client and server application we will send periodic heartbeats to the client and expect a acknowledgment, if one is not found, it is assumed the client has been dropped, and is thuse removed from the active list.

![Capstone-Streamer-Design drawio (2)](https://github.com/user-attachments/assets/eda963b5-9c1c-4b7f-bcc0-7f6be7c8b912)
