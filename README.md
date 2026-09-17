# PixelPost

A 16x8 LED pixel canvas that syncs live with a drawable web page. Draw on the site, and the physical grid updates in real time over WiFi. Built as a small, personal gift.

<img width="1600" height="1200" alt="WhatsApp Image 2026-09-17 at 2 18 32 AM" src="https://github.com/user-attachments/assets/281e948a-64fa-48bd-86b0-7f6505216370" />
*The physical 16x8 grid*

## Demo



https://github.com/user-attachments/assets/6039af84-c5e6-4b6a-b971-4fd00be2ca80



*Drawing on the web canvas and watching the physical grid update in real time*

## Overview

- **Display:** 16x8 LED matrix, built from two chained 8x8 modules, driven via the `MD_MAX72xx` library
- **Controller:** ESP32 (currently DevKitC, moving to ESP32-C3)
- **Sync layer:** Firebase Realtime Database - streaming constantly for current frame
- **Input:** A web page with a clickable/paintable 16x8 grid. Drawing on it pushes the frame straight to the database
- **Auth:** Firebase Authentication gates who can draw on the grid (could be expanded to multiple users, currently gatekeeping for myself)  
- The frame is packed as 16 bytes (one per column) and base64-encoded before being written to the database.
- The ESP32 keeps a persistent stream subscription on `/frame`, decodes each update, and repaints the matrix column-by-column.
- The web page mirrors the same encode/decode scheme, so either side can push or load the current frame.

## Hardware

| Component | Notes |
|---|---|
| Microcontroller | ESP32 DevKitC, migrating to ESP32-C3 |
| Display | 2x 8x8 LED matrix modules (MAX7219-based), chained into a 16x8 grid |

## Software

- **Firmware:** Arduino framework, `MD_MAX72xx` for display driving, `FirebaseClient` for the realtime stream, `mbedtls` for base64 decoding
- **Web app:** Vanilla JS + Firebase JS SDK (Realtime Database + Auth), canvas implemented as a CSS grid of clickable cells with click-and-drag painting

## Roadmap

- [ ] Migrate controller from ESP32 DevKitC to ESP32-C3

## Background

A one-off, personal build - a small LED display that a partner can "draw" on remotely and see appear on a physical device in real time.

---
