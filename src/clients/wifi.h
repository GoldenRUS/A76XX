#ifndef A76XX_WIFI_CLIENT_H_
#define A76XX_WIFI_CLIENT_H_

class A76XXWIFIClient : public A76XXBaseClient {

private:
    WifiCommands     _wifi_cmd;

public:
    /*
        @brief
        @param
        @param
        @param
        @param
        @param
    */
    A76XXWIFIClient(A76XX& modem)
        : A76XXBaseClient(modem)
        , _wifi_cmd(_serial)
      {}


    bool isShowSignal(bool& status) {
        int8_t retcode = _wifi_cmd.isShowSignal(status);
        A76XX_CLIENT_RETCODE_ASSERT_BOOL(retcode);
        return true;
    }

    bool setShowSignal(bool status) {
        int8_t retcode = _wifi_cmd.setShowSignal(status);
        A76XX_CLIENT_RETCODE_ASSERT_BOOL(retcode);
        return true;
    }

    template<size_t N>
    bool scanWifi(WifiNetwork_t(&networks)[N], size_t& found_count) {
        int8_t retcode = _wifi_cmd.scanWifi(networks, found_count);
        A76XX_CLIENT_RETCODE_ASSERT_BOOL(retcode);
        return true;
    }
};

#endif A76XX_WIFI_CLIENT_H_
