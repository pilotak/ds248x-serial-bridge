# DS248x serial bridge

[![Framework Badge Arduino](https://img.shields.io/badge/framework-arduino-00979C.svg)](https://arduino.cc)
[![build](https://github.com/pilotak/ds248x-serial-bridge/actions/workflows/build.yml/badge.svg)](https://github.com/pilotak/ds248x-serial-bridge/actions/workflows/build.yml)

Simple FW for reading Dallas temperature sensors via DS248x with output as JSON

Output example
```
{"0x28FF5A1A31180210": 23.06, "0x28FF9725311801EE":23.12}
```

> All credits for DS2482 library goes to https://github.com/cybergibbons/DS2482_OneWire/