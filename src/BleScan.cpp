#include <BleScan.h>

ble_ad_t BleScan::localName(const uint8_t* scanData, uint8_t scanSize) {
	ble_ad_t localName;
	if (findAdvertisementDataType(scanData, scanSize, GapAdvType::CompleteLocalName, &localName)) {
		return localName; // filled
	}
	else if (findAdvertisementDataType(scanData, scanSize, GapAdvType::ShortenedLocalName, &localName)) {
		return localName; // filled
	}
	else {
		return localName; // empty
	}
}

bool BleScan::findAdvertisementDataType(const uint8_t* scanData, uint8_t scanSize, GapAdvType type, ble_ad_t* foundData) {
	uint8_t i       = 0;
	foundData->type = 0;
	foundData->data = nullptr;
	foundData->len  = 0;
	while (i < scanSize - 1) {
		uint8_t fieldLen  = scanData[i];
		uint8_t fieldType = scanData[i + 1];
		if (fieldLen == 0 || i + 1 + fieldLen > scanSize) {
			return false;
		}
		if (fieldType == type) {
			foundData->data = &scanData[i + 2];
			foundData->len  = fieldLen - 1;
			foundData->type = (uint8_t)type;
			return true;
		}
		i += fieldLen + 1;
	}
	return false;
}

bool BleScan::hasServiceUuid(const uint8_t* scanData, uint8_t scanSize, uuid16_t uuid) {
	GapAdvType serviceUuidListTypes[2] = {
			GapAdvType::IncompleteList16BitServiceUuids,
			GapAdvType::CompleteList16BitServiceUuids};
	ble_ad_t ad;
	for (uint8_t i = 0; i < sizeof(serviceUuidListTypes) / sizeof(serviceUuidListTypes[0]); i++) {
		if (findAdvertisementDataType(scanData, scanSize, serviceUuidListTypes[i], &ad)) {
			// uuid == 0 means any uuid
			if (uuid == 0 && ad.len >= sizeof(uuid)) {
				return true;
			}
			// check ad for uuid
			uuid16_t scanUuid;
			for (uint8_t j = 0; j < ad.len; j += sizeof(uuid)) {
				scanUuid = ((ad.data[j + 1] << 8) | ad.data[j]);
				if (uuid == scanUuid) {
					return true;
				}
			}
		}
	}
	return false;
}

uint8_t BleScan::serviceUuidCount(const uint8_t* scanData, uint8_t scanSize) {
	GapAdvType serviceUuidListTypes[2] = {
			GapAdvType::IncompleteList16BitServiceUuids,
			GapAdvType::CompleteList16BitServiceUuids};
	ble_ad_t ad;
	uint8_t count = 0;
	for (uint8_t i = 0; i < sizeof(serviceUuidListTypes) / sizeof(serviceUuidListTypes[0]); i++) {
		if (findAdvertisementDataType(scanData, scanSize, serviceUuidListTypes[i], &ad)) {
			count += ad.len / UUID_16BIT_BYTE_LENGTH;
		}
	}
	return count;
}

uuid16_t BleScan::serviceUuid(const uint8_t* scanData, uint8_t scanSize, uint8_t index) {
	GapAdvType serviceUuidListTypes[2] = {
			GapAdvType::IncompleteList16BitServiceUuids,
			GapAdvType::CompleteList16BitServiceUuids};
	ble_ad_t ad;
	uint8_t count = 0;
	for (uint8_t i = 0; i < sizeof(serviceUuidListTypes) / sizeof(serviceUuidListTypes[0]); i++) {
		if (findAdvertisementDataType(scanData, scanSize, serviceUuidListTypes[i], &ad)) {
			for (uint8_t j = 0; j < ad.len; j += 2) {
				if (count == index) {
					return ((ad.data[j + 1] << 8) | ad.data[j]);
				}
				count++;
			}
		}
	}
	return 0;
}
