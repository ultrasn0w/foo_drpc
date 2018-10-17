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

	discordInit();
	initDiscordPresence();
}

void foo_drpc::discordInit()
{
	memset(&handlers, 0, sizeof(handlers));
	handlers.ready = connectedF;
	handlers.disconnected = disconnectedF;
	handlers.errored = erroredF;

	Discord_Initialize(APPLICATION_ID, &handlers, 0, NULL);
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
		discordPresence.state = "Paused";
		discordPresence.smallImageKey = "pause";
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
			discordPresence.state = "Listening";
			discordPresence.smallImageKey = "play";
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
		discordPresence.state = "Stopped";
		discordPresence.smallImageKey = "stop";
		updateDiscordPresence();
		break;
	}
}

void foo_drpc::on_playback_pause(bool pause)
{
	if (!connected) return;

	discordPresence.state = (pause ? "Paused" : "Listening");
	discordPresence.smallImageKey = (pause ? "pause" : "play");
	updateDiscordPresence();
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

		discordPresence.state = "Listening";
		discordPresence.smallImageKey = "play";
		discordPresence.details = details;

		updateDiscordPresence();
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

void foo_drpc::initDiscordPresence()
{
	memset(&discordPresence, 0, sizeof(discordPresence));
	discordPresence.state = "Initialized";
	discordPresence.details = "Waiting ...";
	discordPresence.largeImageKey = "logo";
	discordPresence.smallImageKey = "stop";

	updateDiscordPresence();
}

void foo_drpc::updateDiscordPresence()
{
	Discord_UpdatePresence(&discordPresence);

#ifdef DISCORD_DISABLE_IO_THREAD
	Discord_UpdateConnection();
#endif
	Discord_RunCallbacks();

	DEBUG_CONSOLE_PRINTF("Ran Discord presence update: %s, %s, %s, %s", discordPresence.state, discordPresence.details, discordPresence.largeImageKey, discordPresence.smallImageKey);
}

void connectedF(const DiscordUser* request)
{
	foo_interface.get_static_instance().connected = true;
	DEBUG_CONSOLE_PRINTF("Connected to %s.", request->username);
}

void disconnectedF(int errorCode, const char* message)
{
	foo_interface.get_static_instance().connected = false;
	DEBUG_CONSOLE_PRINTF("Disconnected (%i): %s.", errorCode, message);
}

void erroredF(int errorCode, const char* message)
{
	console::printf("*** Error %i: %s.", errorCode, message);
}
