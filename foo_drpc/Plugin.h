#ifndef FOODRPC_PLUGIN_H_
#define FOODRPC_PLUGIN_H_

#include "../../SDK/foobar2000.h"
#include "discord-rpc.h"

#define FOODRPC_NAME "foo_drpc"

#if defined(_DEBUG) || defined(_FOODRPC_WITH_CONSOLE_LOGGING)
	#define FOODRPC_CONSOLE_HEADER FOODRPC_NAME ": "
	#define DEBUG_CONSOLE_PRINTF(...) console::printf(FOODRPC_CONSOLE_HEADER __VA_ARGS__)
#else
	#define DEBUG_CONSOLE_PRINTF(...) {}
#endif

class foo_drpc :
	public initquit,
	public play_callback
{
public:
	foo_drpc();
	virtual ~foo_drpc();

	DiscordEventHandlers handlers;
	DiscordRichPresence discordPresence;

	// Otherwise known as CLIENT_ID by Discord documentation
	const char* APPLICATION_ID = "FILL_IN_HERE";

	bool connected; // If Discord integrator is connected to a running Discord instance

	// Foobar2000 component setup and teardown
	void on_init();
	void on_quit();

	// Foobar2000 callback functions
	void on_playback_starting(play_control::t_track_command command, bool paused);
	void on_playback_stop(play_control::t_stop_reason reason);
	void on_playback_pause(bool state);
	void on_playback_new_track(metadb_handle_ptr track);
	void on_playback_edited(metadb_handle_ptr track) { on_playback_new_track(track); }
	void on_playback_dynamic_info_track(const file_info& info);

	// Foobar2000 callback stubs
	void on_playback_time(double time) {}
	void on_playback_seek(double time) {}
	void on_playback_dynamic_info(file_info const& info) {}
	void on_volume_change(float p_new_val) {}

	// Discord integration helpers
	void discordInit();
	void initDiscordPresence();
	void updateDiscordPresence();
};

// Discord callback functions for notifications of changes
void connectedF(const DiscordUser* request);
void disconnectedF(int errorCode, const char* message);
void erroredF(int errorCode, const char* message);

#endif
