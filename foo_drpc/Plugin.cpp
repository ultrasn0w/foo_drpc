#include <cmath>
#include "Plugin.h"


DECLARE_COMPONENT_VERSION(
"foo_drpc",
"0.1",
"© 2017 - ultrasn0w");

static initquit_factory_t<foo_drpc> foo_interface;
static std::chrono::time_point<std::chrono::high_resolution_clock> lastT;
static std::chrono::time_point<std::chrono::high_resolution_clock> req;
static bool errored; // Still kind of unused
static bool connected;
static bool first;

foo_drpc::foo_drpc()
{
	errored = false;
	connected = true;
	first = true;
}

foo_drpc::~foo_drpc()
{
}

void foo_drpc::on_init()
{
	static_api_ptr_t<play_callback_manager> pcm;
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

void foo_drpc::on_quit()
{
	Discord_Shutdown();
	static_api_ptr_t<play_callback_manager>()->unregister_callback(this);
}

void foo_drpc::on_playback_starting(play_control::t_track_command command, bool pause)
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
		case play_control::track_command_play:
		case play_control::track_command_resume:
		case play_control::track_command_settrack:
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
	// updateDiscordPresence();
}

void foo_drpc::on_playback_stop(play_control::t_stop_reason reason)
{
	if (!connected) return;

	switch (reason)
	{
	case play_control::stop_reason_user:
	case play_control::stop_reason_eof:
	case play_control::stop_reason_shutting_down:
		discordPresence.state = "Stopped";
		discordPresence.smallImageKey = "stop";
		break;
	}
	updateDiscordPresence();
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

		if (format.get_length() + 1 <= 128) {
			static char nya[128];
			size_t destination_size = sizeof(nya);
			strncpy_s(nya, format.get_ptr(), destination_size);
			nya[destination_size - 1] = '\0';

			discordPresence.state = "Listening";
			discordPresence.smallImageKey = "play";
			discordPresence.details = nya;
			updateDiscordPresence();
		}
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
	discordPresence.details = "topkek";
	discordPresence.largeImageKey = "logo";
	discordPresence.smallImageKey = "stop";
	// discordPresence.partyId = "party1234";
	// discordPresence.partySize = 1;
	// discordPresence.partyMax = 6;

	updateDiscordPresence();
}

void foo_drpc::updateDiscordPresence()
{
	if (first) {
		lastT = std::chrono::high_resolution_clock::now();
		first = false;
		Discord_UpdatePresence(&discordPresence);
	}
	else {
		req = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = req - lastT;
		// spam protection
		if (elapsed.count() > 1.0) {
			Discord_UpdatePresence(&discordPresence);
			lastT = std::chrono::high_resolution_clock::now();
		}
	}
}

void connectedF()
{
	connected = true;
}

void disconnectedF(int errorCode, const char* message)
{
	connected = false;
}

void erroredF(int errorCode, const char* message)
{
	errored = true;
}

void foo_drpc::discordInit()
{
	memset(&handlers, 0, sizeof(handlers));
	handlers.ready = connectedF;
	handlers.disconnected = disconnectedF;
	handlers.errored = erroredF;
	// handlers.joinGame = [](const char* joinSecret) {};
	// handlers.spectateGame = [](const char* spectateSecret) {};
	// handlers.joinRequest = [](const DiscordJoinRequest* request) {};
	Discord_Initialize(APPLICATION_ID, &handlers, 1, NULL);
}

// thx SuperKoko (unused)
LPSTR foo_drpc::UnicodeToAnsi(LPCWSTR s)
{
	if (s == NULL) return NULL;
	int cw = lstrlenW(s);
	if (cw == 0) { CHAR *psz = new CHAR[1]; *psz = '\0'; return psz; }
	int cc = WideCharToMultiByte(CP_UTF8, 0, s, cw, NULL, 0, NULL, NULL);
	if (cc == 0) return NULL;
	CHAR *psz = new CHAR[cc + 1];
	cc = WideCharToMultiByte(CP_UTF8, 0, s, cw, psz, cc, NULL, NULL);
	if (cc == 0) { delete[] psz; return NULL; }
	psz[cc] = '\0';
	return psz;
}
