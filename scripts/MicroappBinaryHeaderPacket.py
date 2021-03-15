from crownstone_core.packets.BasePacket import BasePacket
from crownstone_core.util.BufferFiller import BufferFiller
from crownstone_core.util.BufferReader import BufferReader


class MicroappBinaryHeaderPacket():
	"""
	/**
	* Header of a microapp binary.
	*
	* Has to match section .firmware_header in linker file nrf_common.ld of the microapp repo.
	*/
	struct __attribute__((__packed__)) microapp_binary_header_t {
		uint8_t sdkVersionMajor;   // Similar to microapp_sdk_version_t
		uint8_t sdkVersionMinor;
		uint16_t size;             // Size of the binary, including this header.

		uint16_t checksum;         // Checksum of the binary, after this header.
		uint16_t checksumHeader;   // Checksum of this header, with this field set to 0.

		uint32_t appBuildVersion;  // Build version of this microapp.

		uint16_t startOffset;      // Offset in bytes of the first instruction to execute.
		uint16_t reserved;         // Reserved for future use, must be 0 for now.

		uint32_t reserved2;        // Reserved for future use, must be 0 for now.
	};
	"""
	def __init__(self):
		self.sdkVersionMajor = 0
		self.sdkVersionMinor = 0
		self.size = 0

		self.checksum = 0
		self.checksumHeader = 0

		self.appBuildVersion = 0

		self.startOffset = 0
		self.reserved = 0

		self.reserved2 = 0

	def toBuffer(self) -> list:
		bufferFiller = BufferFiller()

		bufferFiller.putUInt8(self.sdkVersionMajor)
		bufferFiller.putUInt8(self.sdkVersionMinor)
		bufferFiller.putUInt16(self.size)

		bufferFiller.putUInt16(self.checksum)
		bufferFiller.putUInt16(self.checksumHeader)

		bufferFiller.putUInt32(self.appBuildVersion)

		bufferFiller.putUInt16(self.startOffset)
		bufferFiller.putUInt16(self.reserved)

		bufferFiller.putUInt32(self.reserved2)

		return bufferFiller.getBuffer()

	def fromBuffer(self, buf: list):
		reader = BufferReader(buf)

		self.sdkVersionMajor = reader.getUInt8()
		self.sdkVersionMinor = reader.getUInt8()
		self.size =            reader.getUInt16()

		self.checksum =        reader.getUInt16()
		self.checksumHeader =  reader.getUInt16()

		self.appBuildVersion = reader.getUInt32()

		self.startOffset =     reader.getUInt16()
		self.reserved =        reader.getUInt16()

		self.reserved2 =       reader.getUInt32()

	def __str__(self) -> str:
		return f"MicroappBinaryHeaderPacket(" \
		       f"sdkVersion={self.sdkVersionMajor}.{self.sdkVersionMinor}, " \
		       f"size={self.size}, " \
		       f"checksum={self.checksum}, " \
		       f"checksumHeader={self.checksumHeader}, " \
		       f"appBuildVersion={self.appBuildVersion}, " \
		       f"startOffset={self.startOffset}, " \
		       f"reserved={self.reserved}, " \
		       f"reserved2={self.reserved2})"
