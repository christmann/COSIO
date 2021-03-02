#include "utility.h"

Utility* Utility::_instance = NULL;

Utility::Utility() {

}

String Utility::encryptionTypeStr(uint8_t authmode) {
  switch (authmode) {
    case ENC_TYPE_NONE:
      return "NONE";
    case ENC_TYPE_WEP:
      return "WEP";
    case ENC_TYPE_TKIP:
      return "TKIP";
    case ENC_TYPE_CCMP:
      return "CCMP";
    case ENC_TYPE_AUTO:
      return "AUTO";
    default:
      return "?";
  }
}
