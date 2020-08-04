# Notes

This project uses a different approach, instead of to store all the HTML and assets (CSS and JS scripts) on the ESP8266, it's only stored a minimal HTML page. The assets are externally loaded from the web (in this case GitHub servers) and the JS script injects all the HTML code to the page.

**Advantages:**
- Save flash space
- Easily changed. You don't need to flash the firmware each time you change the web page
- Consumes fewer resources on the uC (serve large files can be a high demand task for an ESP8266)