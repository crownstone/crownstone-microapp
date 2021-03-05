from crownstone_core.packets.BasePacket import BasePacket
from crownstone_core.util.BufferFiller import BufferFiller

class MicroappBinaryHeaderPacket():
	"""
	/**
	* Header of a microapp binary.
	*
	* Has to match section .firmware_header in linker file nrf_common.ld of the microapp repo.
	*/
	typedef struct __attribute__((__packed__)) microapp_binary_header_t {
		uint32_t startAddress;   // Address of first function to call.

		uint8_t sdkVersionMajor; // Similar to microapp_sdk_version_packet_t
		uint8_t sdkVersionMinor;
		uint16_t size;           // Size of the binary, including this header.

		uint16_t checksum;       // Checksum of the binary, after this header.
		uint16_t checksumHeader; // Checksum of this header, with this field set to 0.

		uint32_t appBuildVersion;  // Build version of this microapp.

		uint32_t reserved;       // Reserved for future use, must be 0 for now.
	};
	"""
	def __init__(self):
		self.startAddress = 0

		self.sdkVersionMajor = 0
		self.sdkVersionMinor = 0
		self.size = 0

		self.checksum = 0
		self.checksumHeader = 0

		self.appBuildVersion = 0

		self.reserved = 0

	def toBuffer(self) -> list:
		bufferFiller = BufferFiller()

		bufferFiller.putUInt32(self.startAddress)

		bufferFiller.putUInt8(self.sdkVersionMajor)
		bufferFiller.putUInt8(self.sdkVersionMinor)
		bufferFiller.putUInt16(self.size)

		bufferFiller.putUInt16(self.checksum)
		bufferFiller.putUInt16(self.checksumHeader)

		bufferFiller.putUInt32(self.appBuildVersion)

		bufferFiller.putUInt32(self.reserved)

		return bufferFiller.getBuffer()

	def __str__(self) -> str:
		return f"MicroappBinaryHeaderPacket(" \
		       f"startAddress={self.startAddress}, " \
		       f"sdkVersion={self.sdkVersionMajor}.{self.sdkVersionMinor}, " \
		       f"size={self.size}, " \
		       f"checksum={self.checksum}, " \
		       f"checksumHeader={self.checksumHeader}, " \
		       f"appBuildVersion={self.appBuildVersion}, " \
		       f"reserved={self.reserved})"
