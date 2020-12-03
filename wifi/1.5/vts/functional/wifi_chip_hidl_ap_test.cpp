/*
 * Copyright (C) 2020 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <VtsHalHidlTargetCallbackBase.h>
#include <android-base/logging.h>

#undef NAN  // NAN is defined in bionic/libc/include/math.h:38

#include <android/hardware/wifi/1.4/IWifiChipEventCallback.h>
#include <android/hardware/wifi/1.5/IWifi.h>
#include <android/hardware/wifi/1.5/IWifiApIface.h>
#include <android/hardware/wifi/1.5/IWifiChip.h>
#include <gtest/gtest.h>
#include <hidl/GtestPrinter.h>
#include <hidl/ServiceManagement.h>

#include "wifi_hidl_call_util.h"
#include "wifi_hidl_test_utils.h"

using ::android::sp;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::wifi::V1_0::ChipModeId;
using ::android::hardware::wifi::V1_0::IfaceType;
using ::android::hardware::wifi::V1_0::IWifiIface;
using ::android::hardware::wifi::V1_0::WifiDebugRingBufferStatus;
using ::android::hardware::wifi::V1_0::WifiStatus;
using ::android::hardware::wifi::V1_0::WifiStatusCode;
using ::android::hardware::wifi::V1_4::IWifiChipEventCallback;
using ::android::hardware::wifi::V1_5::IWifiApIface;
using ::android::hardware::wifi::V1_5::IWifiChip;

/**
 * Fixture for IWifiChip tests that are conditioned on SoftAP support.
 */
class WifiChipHidlTest : public ::testing::TestWithParam<std::string> {
   public:
    virtual void SetUp() override {
        // Make sure to start with a clean state
        stopWifi(GetInstanceName());

        wifi_chip_ = IWifiChip::castFrom(getWifiChip(GetInstanceName()));
        ASSERT_NE(nullptr, wifi_chip_.get());
    }

    virtual void TearDown() override { stopWifi(GetInstanceName()); }

   protected:
    // Helper function to configure the Chip in one of the supported modes.
    // Most of the non-mode-configuration-related methods require chip
    // to be first configured.
    ChipModeId configureChipForIfaceType(IfaceType type, bool expectSuccess) {
        ChipModeId mode_id;
        EXPECT_EQ(expectSuccess,
                  configureChipToSupportIfaceType(wifi_chip_, type, &mode_id));
        return mode_id;
    }

    WifiStatusCode createApIface(sp<IWifiApIface>* ap_iface) {
        configureChipForIfaceType(IfaceType::AP, true);
        const auto& status_and_iface = HIDL_INVOKE(wifi_chip_, createApIface);
        *ap_iface = IWifiApIface::castFrom(status_and_iface.second);
        return status_and_iface.first.code;
    }

    WifiStatusCode createBridgedApIface(sp<IWifiApIface>* ap_iface) {
        configureChipForIfaceType(IfaceType::AP, true);
        const auto& status_and_iface =
            HIDL_INVOKE(wifi_chip_, createBridgedApIface);
        *ap_iface = status_and_iface.second;
        return status_and_iface.first.code;
    }

    sp<IWifiChip> wifi_chip_;

   private:
    std::string GetInstanceName() { return GetParam(); }
};

// TODO: b/173999527. Add test for bridged API.

/*
 * resetToFactoryMacAddress
 */
TEST_P(WifiChipHidlTest, resetToFactoryMacAddressTest) {
    sp<IWifiApIface> wifi_ap_iface;
    const auto& status_code = createApIface(&wifi_ap_iface);
    if (status_code != WifiStatusCode::SUCCESS) {
        EXPECT_EQ(WifiStatusCode::ERROR_NOT_SUPPORTED, status_code);
    }
    const auto& status = HIDL_INVOKE(wifi_ap_iface, resetToFactoryMacAddress);
    EXPECT_EQ(WifiStatusCode::SUCCESS, status.code);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(WifiChipHidlTest);
INSTANTIATE_TEST_SUITE_P(
    PerInstance, WifiChipHidlTest,
    testing::ValuesIn(android::hardware::getAllHalInstanceNames(
        ::android::hardware::wifi::V1_5::IWifi::descriptor)),
    android::hardware::PrintInstanceNameToString);
