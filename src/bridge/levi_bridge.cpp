#include "levi_bridge.h"
#include "BridgeAPI.hpp"

static levi::BridgeAPI& api() { return levi::BridgeAPI::instance(); }

extern "C" {

LeviErrorCode levi_init(LeviLogCallback log_cb, void* user_data) {
    return api().initialize(log_cb, user_data) ? LEVI_OK : LEVI_ERROR_GENERIC;
}

void levi_shutdown(void) { api().shutdown(); }

const LeviLinuxVersion* levi_get_version(void) {
    static const LeviLinuxVersion v = {1,0,0, "dev", __DATE__};
    return &v;
}

const char* levi_get_last_error(void) { return "not implemented"; }

LeviErrorCode levi_settings_load(void) { return api().settingsLoad(); }
LeviErrorCode levi_settings_save(void) { return api().settingsSave(); }
const LeviSettings* levi_settings_get(void) { return api().settingsGet(); }
LeviErrorCode levi_settings_update(const LeviSettings* s) { return api().settingsUpdate(s); }

LeviErrorCode levi_versions_refresh(LeviProgressCallback cb, void* ud) { api().versionsRefresh(cb, ud); return LEVI_OK; }
const LeviVersionList* levi_versions_get_list(void) { return api().versionsRefresh(nullptr, nullptr); }
const LeviVersionInfo* levi_versions_get_info(const char* id) { return api().versionsGetInfo(id); }
bool levi_versions_is_valid(const char* id) { return api().versionsIsValid(id); }
LeviErrorCode levi_versions_delete(const char* id) { return api().versionsDelete(id); }
LeviErrorCode levi_versions_extract(const char* a, const char* b, LeviProgressCallback cb, void* ud) { return api().versionsExtract(a,b,cb,ud); }
LeviErrorCode levi_versions_set_args(const char* a, const char* b) { return api().versionsSetArgs(a,b); }

LeviErrorCode levi_mods_set_version(const char* id) { return api().modsSetVersion(id); }
const LeviModList* levi_mods_get_list(void) { return api().modsGetList(); }
const LeviModInfo* levi_mods_get_info(const char* id) { return api().modsGetInfo(id); }
LeviErrorCode levi_mods_set_enabled(const char* id, bool e) { return api().modsSetEnabled(id,e); }
LeviErrorCode levi_mods_reorder(const char** ids, size_t n) { return api().modsReorder(ids,n); }
LeviErrorCode levi_mods_delete(const char* id) { return api().modsDelete(id); }
LeviErrorCode levi_mods_install(const char* p, LeviProgressCallback cb, void* ud) { return api().modsInstall(p,cb,ud); }
void levi_mods_set_event_callback(LeviModEventCallback cb, void* ud) { api().modsSetEventCallback(cb,ud); }

LeviErrorCode levi_auth_login(LeviAuthCallback cb, void* ud) { return api().authLogin(cb,ud); }
LeviErrorCode levi_auth_logout(void) { return api().authLogout(); }
const LeviAccountInfo* levi_auth_get_account(void) { return api().authGetAccount(); }
const LeviAuthTokens* levi_auth_get_tokens(void) { return api().authGetTokens(); }
bool levi_auth_is_logged_in(void) { return api().authIsLoggedIn(); }

LeviErrorCode levi_launch_game(const LeviLaunchConfig* c, LeviGameStateCallback cb, void* ud) { return api().launchGame(c,cb,ud); }
bool levi_game_is_running(void) { return api().gameIsRunning(); }
int levi_game_get_pid(void) { return api().gameGetPid(); }
LeviErrorCode levi_game_terminate(void) { return api().gameTerminate(); }
void levi_game_set_log_callback(LeviLogCallback cb, void* ud) { api().gameSetLogCallback(cb,ud); }

LeviErrorCode levi_open_directory(const char* p) { return api().openDirectory(p); }
LeviErrorCode levi_open_url(const char* u) { return api().openUrl(u); }
LeviErrorCode levi_cleanup_cache(void) { return api().cleanupCache(); }

}
