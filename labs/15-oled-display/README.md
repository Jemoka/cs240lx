### Oled displays  (Irene Geng)

Irene likes displays so much she made two different labs!
  - [ssd1306-display](./ssd1306-display): these are in the 
    smaller rounded corner boxes.
  - [sh1106-display](./sh1106-display/): these are in the larger
    rectangular boxes.

Easy way to tell which you have:
  - Hook up the wires into the Parthiv board in the same header as you used
    for the 6050 MPU using M-to-F jumpers:
     - 3v3 
     - Gnd
     - GPIO 2 for SDA
     - GPIO 3 for SCL.
  - Try running the staff binaries: 
      - `sh1106-0-fill-screen.bin`
      - `ssd1306-0-fill-screen.bin`
  - You will hopefully see a square for one of them.
  - If not: 
    1. Make sure you power cycle after each attempt;
    2. Don't hook right to the pi pins (GPIO 2 and GPIO 3) since 
       it appears this revision leaves those disconnected.

These tiny displays are ubuitious and extremely similar, so it's probably
worth doing both!

Would recommend starting with the SSD1306, as the datasheet
for the SSD1306 is more well-written.

The SH1106 has a couple quirks in comparison:
- Although the display claims to have 132x64 bits of SRAM, it only has 128x64 visible pixels. This might affect your indexing scheme when writing to the display buffer, as pixel (2,0) in SRAM corresponds to pixel (0,0) on the display.
- SH1106 only supports page-addressing mode, which is less convenient if you prefer to update the entire screen on each frame.

If you bit-bang your i2c you can do many of them at once.  A great
extension is having 6 or more making a larger paned display with
interesting animations (e.g., a box rotating and flying between them
tends to get attention :).
