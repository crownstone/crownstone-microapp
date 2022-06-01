# Header

The binary header has the following information:

```
uint8_t sdkVersionMajor;
uint8_t sdkVersionMinor;
uint16_t size;
uint16_t checksum;
uint16_t checksumHeader;
uint32_t appBuildVersion;
uint16_t startOffset;
uint16_t reserved;
uint32_t reserved2;
```

The first two fields contain the SDK version of the "microapp bluenet SDK" (e.g. 0.2).
The size field contains the size of the microapp.
The checksum is calculated over all bytes of the app except for the header.
The checksumHeader field only over the fields of the header (except for the checksumHeader field itself).
The offset is calculated from the start of the body (after the header of 20 bytes). This can be used to jump to
some other instruction than the first instruction of the binary.
