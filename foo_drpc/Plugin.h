#ifndef FOODRPC_PLUGIN_H_
#define FOODRPC_PLUGIN_H_

#include "../../SDK/foobar2000.h"
#include "discord-rpc.h"
#include <chrono>

class foo_drpc :
	public initquit,
	public play_callback
{
public:
	foo_drpc();
	~foo_drpc();

	DiscordEventHandlers handlers;
	DiscordRichPresence discordPresence;
	// Censored on GitHub :)
	const char* APPLICATION_ID = "FILL_IN_HERE";

	void on_init();
	void on_quit();

	void discordInit();
	void initDiscordPresence();
	void updateDiscordPresence();

	LPSTR UnicodeToAnsi(LPCWSTR s);

	void on_playback_starting(play_control::t_track_command command, bool paused);
	void on_playback_stop(play_control::t_stop_reason reason);
	void on_playback_pause(bool state);
	void on_playback_new_track(metadb_handle_ptr track);
	void on_playback_edited(metadb_handle_ptr track) { on_playback_new_track(track); }
	void on_playback_dynamic_info_track(const file_info& info);
	void on_playback_time(double time) {}
	void on_playback_seek(double time) {}
	void on_playback_dynamic_info(file_info const& info) {}
	void on_volume_change(float p_new_val) {}
};

#endif
