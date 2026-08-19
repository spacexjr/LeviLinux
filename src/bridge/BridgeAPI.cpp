#include "BridgeAPI.hpp"

namespace levi {

BridgeAPI& BridgeAPI::instance() {
    static BridgeAPI inst;
    return inst;
}

bool BridgeAPI::initialize(LeviLogCallback log_cb, void* user_data) {
    log_cb_.callback = log_cb;
    log_cb_.user_data = user_data;
    initialized_.store(true);
    return true;
}

void BridgeAPI::shutdown() {
    initialized_.store(false);
}

const LeviVersionList* BridgeAPI::versionsRefresh(LeviProgressCallback, void*) { return nullptr; }
const LeviVersionInfo* BridgeAPI::versionsGetInfo(const char*) { return nullptr; }
bool BridgeAPI::versionsIsValid(const char*) { return false; }
LeviErrorCode BridgeAPI::versionsDelete(const char*) { return LEVI_ERROR_GENERIC; }
LeviErrorCode BridgeAPI::versionsExtract(const char*, const char*, LeviProgressCallback, void*) { return LEVI_ERROR_GENERIC; }
LeviErrorCode BridgeAPI::versionsSetArgs(const char*, const char*) { return LEVI_ERROR_GENERIC; }

LeviErrorCode BridgeAPI::modsSetVersion(const char*) { return LEVI_ERROR_GENERIC; }
const LeviModList* BridgeAPI::modsGetList() { return nullptr; }
const LeviModInfo* BridgeAPI::modsGetInfo(const char*) { return nullptr; }
LeviErrorCode BridgeAPI::modsSetEnabled(const char*, bool) { return LEVI_ERROR_GENERIC; }
LeviErrorCode BridgeAPI::modsReorder(const char**, size_t) { return LEVI_ERROR_GENERIC; }
LeviErrorCode BridgeAPI::modsDelete(const char*) { return LEVI_ERROR_GENERIC; }
LeviErrorCode BridgeAPI::modsInstall(const char*, LeviProgressCallback, void*) { return LEVI_ERROR_GENERIC; }
void BridgeAPI::modsSetEventCallback(LeviModEventCallback cb, void* ud) { mod_event_cb_.callback = cb; mod_event_cb_.user_data = ud; }

LeviErrorCode BridgeAPI::authLogin(LeviAuthCallback, void*) { return LEVI_ERROR_GENERIC; }
LeviErrorCode BridgeAPI::authLogout() { return LEVI_ERROR_GENERIC; }
const LeviAccountInfo* BridgeAPI::authGetAccount() { return &account_cache_; }
const LeviAuthTokens* BridgeAPI::authGetTokens() { return &tokens_cache_; }
bool BridgeAPI::authIsLoggedIn() { return false; }

LeviErrorCode BridgeAPI::launchGame(const LeviLaunchConfig*, LeviGameStateCallback, void*) { return LEVI_ERROR_GENERIC; }
bool BridgeAPI::gameIsRunning() { return false; }
int BridgeAPI::gameGetPid() { return 0; }
LeviErrorCode BridgeAPI::gameTerminate() { return LEVI_ERROR_GENERIC; }
void BridgeAPI::gameSetLogCallback(LeviLogCallback cb, void* ud) { log_cb_.callback = cb; log_cb_.user_data = ud; }

LeviErrorCode BridgeAPI::settingsLoad() { return LEVI_OK; }
LeviErrorCode BridgeAPI::settingsSave() { return LEVI_OK; }
const LeviSettings* BridgeAPI::settingsGet() { return &settings_cache_; }
LeviErrorCode BridgeAPI::settingsUpdate(const LeviSettings*) { return LEVI_OK; }

LeviErrorCode BridgeAPI::openDirectory(const char*) { return LEVI_ERROR_GENERIC; }
LeviErrorCode BridgeAPI::openUrl(const char*) { return LEVI_ERROR_GENERIC; }
LeviErrorCode BridgeAPI::cleanupCache() { return LEVI_OK; }

void BridgeAPI::syncVersionsFromService() {}
void BridgeAPI::syncModsFromService() {}
void BridgeAPI::syncAuthFromService() {}

} // namespace levi
