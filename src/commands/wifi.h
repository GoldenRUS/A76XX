#ifndef A76XX_WIFI_CMDS_H_
#define A76XX_WIFI_CMDS_H_


/*
    @brief Commands in section 25 of the AT command manual version 1.06

    Command  | Implemented | Method | Function(s)
    ---------- | ----------- | ------ |-----------------------
    CWSTASCAN  |      -      |  R/W/E | isShowSignal, setShowSignal, scan
    CWSTASCANEX|      -      |        |

*/


struct WifiNetwork_t {
    uint8_t bssid[6];
    uint8_t channel;
    int8_t signal;
};


class WifiCommands {
public:
    ModemSerial& _serial;

    WifiCommands(ModemSerial& serial)
        : _serial(serial) {}

    int8_t isShowSignal(bool &status) {
        _serial.sendCMD("AT+CWSTASCAN?");
        Response_t rsp = _serial.waitResponse("+CWSTASCAN: ");
        switch (rsp) {
            case Response_t::A76XX_RESPONSE_MATCH_1ST: {
                status = _serial.parseIntClear() == 1;
                return A76XX_OPERATION_SUCCEEDED;
            }
            case Response_t::A76XX_RESPONSE_TIMEOUT: {
                return A76XX_OPERATION_TIMEDOUT;
            }
            default: {
                return A76XX_GENERIC_ERROR;
            }
        }
    }


    int8_t setShowSignal(bool status) {
        _serial.sendCMD("AT+CWSTASCAN=", status ? "1" : "0");
        A76XX_RESPONSE_PROCESS(_serial.waitResponse())
    }
    template<size_t N>
    int8_t scanWifi(WifiNetwork_t(&networks)[N], size_t& found_count) {
        _serial.sendCMD("AT+CWSTASCAN");
        Response_t rsp = _serial.waitResponse("+CWSTASCAN:");

        switch (rsp) {
        case Response_t::A76XX_RESPONSE_MATCH_1ST: {
            bool hasError = false;
            found_count = 0;
            char buffer[64];
            uint32_t timeout = millis() + 1000;

            while (_serial.available() > 0 && found_count < N && millis() < timeout) {

                size_t len = _serial.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
                if (len == 0) continue;

                if (len > 0 && buffer[len - 1] == '\r') {
                    len--;
                }

                buffer[len] = '\0';

                if (len < 10) continue;

                if (parseWifiNetwork(buffer, &networks[found_count])) {
                    found_count++;
                }
                else {
                    hasError = true;
                }
            }

            return hasError ? A76XX_WIFI_HAS_LOST_NETWORKS : A76XX_OPERATION_SUCCEEDED;
        }

        case Response_t::A76XX_RESPONSE_TIMEOUT: {
            return A76XX_OPERATION_TIMEDOUT;
        }

        default: {
            return A76XX_GENERIC_ERROR;
        }
        }
    }


    /**
 * ������ ������ � ����������� � WiFi ���� � ��������� ���������
 * ������ ������: "50:FA:84:AF:C8:B9,11,-61"
 *
 * @param input ������� ������ ��� ��������
 * @param result ��������� �� ��������� ��� ����������
 * @return true - �������� �������, false - ������
 */
    bool parseWifiNetwork(const char* input, WifiNetwork_t* result) {
        if (input == nullptr || result == nullptr) {
            return false;
        }

        // ������ MAC-����� (BSSID)
        const char* macStart = input;
        const char* commaPos = strchr(input, ',');
        if (commaPos == nullptr) {
            return false;
        }

        // ��������� ����� MAC-������ (17 ��������: 6 ���� �� 2 ������� + 5 ������������)
        size_t macLength = commaPos - macStart;
        if (macLength != 17) {
            return false;
        }

        // ������ ������ ���� MAC-������
        for (int i = 0; i < 6; i++) {
            // ���������� ��������� (����� ������� �����)
            if (i > 0) {
                if (*macStart != ':') {
                    return false;
                }
                macStart++;
            }

            // ���������, ��� ���� ������� 2 ������� ��� �����
            if (macStart + 2 > commaPos) {
                return false;
            }

            // ������������ ��� hex ������� � ����
            char byteStr[3] = { macStart[0], macStart[1], '\0' };
            char* endPtr;
            unsigned long byteValue = strtoul(byteStr, &endPtr, 16);

            if (endPtr != byteStr + 2 || byteValue > 0xFF) {
                return false;
            }

            result->bssid[i] = static_cast<uint8_t>(byteValue);
            macStart += 2;
        }

        // ������ �����
        const char* channelStart = commaPos + 1;
        commaPos = strchr(channelStart, ',');

        // ���� ��� ������ �������, ������ ���� ������ MAC � �����
        if (commaPos == nullptr) {
            // ������ ����� �� ����� ������
            char* endPtr;
            unsigned long channel = strtoul(channelStart, &endPtr, 10);

            // ���������, ��� ����� �� ����� ������ ��� ����������� ��������
            if (*endPtr != '\0' && *endPtr != '\r' && *endPtr != '\n') {
                return false;
            }
            if (channel > 0xFF) {
                return false;
            }

            result->channel = static_cast<uint8_t>(channel);
            result->signal = 0; // �������� �� ��������� ��� �������������� �������
            return true;
        }

        // ������ ����� (���� ���� ������� ��� �������)
        char* endPtr;
        unsigned long channel = strtoul(channelStart, &endPtr, 10);
        if (endPtr != commaPos || channel > 0xFF) {
            return false;
        }
        result->channel = static_cast<uint8_t>(channel);

        // ������ ������� �������
        const char* signalStart = commaPos + 1;
        long signal = strtol(signalStart, &endPtr, 10);

        // ��������� ����������� ������� ����� ������
        if (*endPtr != '\0' && *endPtr != '\r' && *endPtr != '\n') {
            return false;
        }
        if (signal < -128 || signal > 127) {
            return false;
        }
        result->signal = static_cast<int8_t>(signal);

        return true;
    }
};

#endif A76XX_WIFI_CMDS_H_