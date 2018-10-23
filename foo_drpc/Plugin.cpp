#include "Plugin.h"

// This tells Foobar2000 users what this component does
DECLARE_COMPONENT_VERSION(
"foo_drpc",
"0.3",
"Foobar2000 music status for Discord Rich Presence! (c) 2018 - ultrasn0w et al");

// This tells Foobar2000 what the file really is even if the user renames it (so only one is loaded)
VALIDATE_COMPONENT_FILENAME(FOODRPC_NAME".dll");

static initquit_factory_t<foo_drpc> foo_interface;

foo_drpc::foo_drpc()
{
	// This starts at true because 
	//	1) Discord will not call its connected callback if you start this plugin and it's already running and
	//	2) it costs us very little to write updates into the void
	connected = true;
}

foo_drpc::~foo_drpc()
{
}

void foo_drpc::on_init()
{
	static_api_ptr_t<play_callback_manager> pcm;

	DEBUG_CONSOLE_PRINTF("Initializing");

	pcm->register_callback(
		this,
		play_callback::flag_on_playback_starting |
		play_callback::flag_on_playback_stop |
		play_callback::flag_on_playback_pause |
		play_callback::flag_on_playback_new_track |
		play_callback::flag_on_playback_edited |
		play_callback::flag_on_playback_dynamic_info_track,
		false);

	discord_init();
}

void foo_drpc::discord_init()
{
	memset(&handlers, 0, sizeof(handlers));
	handlers.ready = callback_discord_connected;
	handlers.disconnected = callback_discord_disconnected;
	handlers.errored = callback_discord_errored;

	Discord_Initialize(APPLICATION_ID, &handlers, 0, NULL);

	init_discord_presence();
}

void foo_drpc::init_discord_presence()
{
	memset(&discord_presence, 0, sizeof(discord_presence));
	discord_presence.state = "Initialized";
	discord_presence.details = "Waiting ...";
	discord_presence.largeImageKey = "logo";
	discord_presence.smallImageKey = "stop";

	update_discord_presence();
}

void foo_drpc::on_quit()
{
	DEBUG_CONSOLE_PRINTF("Unloading");

	Discord_ClearPresence();
	Discord_Shutdown();

	static_api_ptr_t<play_callback_manager>()->unregister_callback(this);
}

void foo_drpc::on_playback_starting(playback_control::t_track_command command, bool pause)
{
	if (!connected) return;

	if (pause)
	{
		discord_presence.state = "Paused";
		discord_presence.smallImageKey = "pause";
	}
	else
	{
		switch (command)
		{
		case playback_control::track_command_play:
		case playback_control::track_command_next:
		case playback_control::track_command_prev:
		case playback_control::track_command_resume:
		case playback_control::track_command_rand:
		case playback_control::track_command_settrack:
			discord_presence.state = "Listening";
			discord_presence.smallImageKey = "play";
			break;
		}
	}

	metadb_handle_ptr track;
	static_api_ptr_t<playback_control> pbc;
	if (pbc->get_now_playing(track))
	{
		on_playback_new_track(track);
	}
}

void foo_drpc::on_playback_stop(playback_control::t_stop_reason reason)
{
	if (!connected) return;

	switch (reason)
	{
	case playback_control::stop_reason_user:
	case playback_control::stop_reason_eof:
	case playback_control::stop_reason_shutting_down:
		discord_presence.state = "Stopped";
		discord_presence.smallImageKey = "stop";
		update_discord_presence();
		break;
	}
}

void foo_drpc::on_playback_pause(bool pause)
{
	if (!connected) return;

	discord_presence.state = (pause ? "Paused" : "Listening");
	discord_presence.smallImageKey = (pause ? "pause" : "play");
	update_discord_presence();
}

void foo_drpc::on_playback_new_track(metadb_handle_ptr track)
{
	if (!connected) return;

	service_ptr_t<titleformat_object> script;
	pfc::string8 format = "%artist% - %title%";

	if (static_api_ptr_t<titleformat_compiler>()->compile(script, format))
	{
		static_api_ptr_t<playback_control> pbc;

		pbc->playback_format_title_ex(
			track,
			nullptr,
			format,
			script,
			nullptr,
			playback_control::display_level_titles);

		// If the details size is bigger than MAX_DETAILS_LENGTH chars, truncate it
		const size_t MAX_DETAILS_LENGTH = 128;

		size_t details_length = min(format.get_length(), MAX_DETAILS_LENGTH-1); // -1 to give us room for the '\0' in the longest case
		static char details[MAX_DETAILS_LENGTH];

		strncpy_s(details, format.get_ptr(), details_length);
		details[details_length] = '\0';

		discord_presence.state = (pbc->is_paused() ? "Paused" : "Listening");
		discord_presence.smallImageKey = (pbc->is_paused() ? "pause" : "play");
		discord_presence.details = details;

		update_discord_presence();
	}
}

void foo_drpc::on_playback_dynamic_info_track(const file_info& info)
{
	metadb_handle_ptr track;
	static_api_ptr_t<playback_control> pbc;
	if (pbc->get_now_playing(track))
	{
		on_playback_new_track(track);
	}
}

void foo_drpc::update_discord_presence()
{
	Discord_UpdatePresence(&discord_presence);

#ifdef DISCORD_DISABLE_IO_THREAD
	Discord_UpdateConnection();
#endif
	Discord_RunCallbacks();

	DEBUG_CONSOLE_PRINTF("Ran Discord presence update: %s, %s, %s, %s", discord_presence.state, discord_presence.details, discord_presence.largeImageKey, discord_presence.smallImageKey);
}

void callback_discord_connected(const DiscordUser* request)
{
	foo_interface.get_static_instance().connected = true;
	DEBUG_CONSOLE_PRINTF("Connected to %s.", request->username);
}

void callback_discord_disconnected(int errorCode, const char* message)
{
	foo_interface.get_static_instance().connected = false;
	DEBUG_CONSOLE_PRINTF("Disconnected (%i): %s.", errorCode, message);
}

void callback_discord_errored(int errorCode, const char* message)
{
	console::printf("*** Error %i: %s.", errorCode, message);
}
