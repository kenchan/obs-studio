#include "system-tray-icon.hpp"
#include "window-basic-main.hpp"

#include <QAction>

OBSSystemTrayIcon::OBSSystemTrayIcon(QWidget *parent) : QSystemTrayIcon(parent)
{
#ifdef __APPLE__
	QIcon trayIconFile = QIcon(":/res/images/obs_macos.svg");
	trayIconFile.setIsMask(true);
#else
	QIcon trayIconFile = QIcon(":/res/images/obs.png");
#endif
	setIcon(QIcon::fromTheme("obs-tray", trayIconFile));
	setToolTip("OBS Studio");

	OBSBasic *main = OBSBasic::Get();
	menu.reset(new QMenu());

	showHide = menu->addAction(main->isVisible()
					   ? QTStr("Basic.SystemTray.Hide")
					   : QTStr("Basic.SystemTray.Show"),
				   this, &OBSSystemTrayIcon::ToggleVisibility);
	menu->addSeparator();

	previewProjector = new QMenu(QTStr("PreviewProjector"));
	studioProgramProjector = new QMenu(QTStr("StudioProgramProjector"));

	main->AddProjectorMenuMonitors(previewProjector, main,
				       &OBSBasic::OpenPreviewProjector);
	main->AddProjectorMenuMonitors(studioProgramProjector, main,
				       &OBSBasic::OpenStudioProgramProjector);
	menu->addMenu(previewProjector);
	menu->addMenu(studioProgramProjector);
	menu->addSeparator();

	stream = menu->addAction(obs_frontend_streaming_active()
					 ? QTStr("Basic.Main.StopStreaming")
					 : QTStr("Basic.Main.StartStreaming"),
				 main, &OBSBasic::on_streamButton_clicked);
	record = menu->addAction(obs_frontend_recording_active()
					 ? QTStr("Basic.Main.StopRecording")
					 : QTStr("Basic.Main.StartRecording"),
				 main, &OBSBasic::on_recordButton_clicked);
	replay = menu->addAction(
		obs_frontend_replay_buffer_active()
			? QTStr("Basic.Main.StopReplayBuffer")
			: QTStr("Basic.Main.StartReplayBuffer"));
	virtualCam =
		menu->addAction(obs_frontend_virtualcam_active()
					? QTStr("Basic.Main.StopVirtualCam")
					: QTStr("Basic.Main.StartVirtualCam"));
	menu->addSeparator();
	menu->addAction(QTStr("Exit"), main, &OBSBasic::close);

	setContextMenu(menu.data());

	UpdateReplayBuffer();

	OBSOutputAutoRelease vcamOutput = obs_frontend_get_virtualcam_output();
	virtualCam->setEnabled(!!vcamOutput);

	if (Active())
		OnActivate(true);

	connect(this, &QSystemTrayIcon::activated, this,
		&OBSSystemTrayIcon::IconActivated);

	obs_frontend_add_event_callback(OnEvent, this);
	show();
}

OBSSystemTrayIcon::~OBSSystemTrayIcon()
{
	delete previewProjector;
	delete studioProgramProjector;

	obs_frontend_remove_event_callback(OnEvent, this);
}

void OBSSystemTrayIcon::OnEvent(enum obs_frontend_event event, void *data)
{
	OBSSystemTrayIcon *sysTray = static_cast<OBSSystemTrayIcon *>(data);

	switch (event) {
	// Streaming
	case OBS_FRONTEND_EVENT_STREAMING_STARTING:
		sysTray->StreamStarting();
		break;
	case OBS_FRONTEND_EVENT_STREAMING_STARTED:
		sysTray->StreamStarted();
		break;
	case OBS_FRONTEND_EVENT_STREAMING_STOPPING:
		sysTray->StreamStopping();
		break;
	case OBS_FRONTEND_EVENT_STREAMING_STOPPED:
		sysTray->StreamStopped();
		break;

	// Recording
	case OBS_FRONTEND_EVENT_RECORDING_STARTED:
		sysTray->RecordingStarted();
		break;
	case OBS_FRONTEND_EVENT_RECORDING_STOPPING:
		sysTray->RecordingStopping();
		break;
	case OBS_FRONTEND_EVENT_RECORDING_STOPPED:
		sysTray->RecordingStopped();
		break;
	case OBS_FRONTEND_EVENT_RECORDING_PAUSED:
		sysTray->RecordingPaused();
		break;
	case OBS_FRONTEND_EVENT_RECORDING_UNPAUSED:
		sysTray->RecordingUnpaused();
		break;

	// Replay Buffer
	case OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTED:
		sysTray->ReplayStarted();
		break;
	case OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPING:
		sysTray->ReplayStopping();
		break;
	case OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPED:
		sysTray->ReplayStopped();
		break;

	// Virtual Camera
	case OBS_FRONTEND_EVENT_VIRTUALCAM_STARTED:
		sysTray->VirtualCamStarted();
		break;
	case OBS_FRONTEND_EVENT_VIRTUALCAM_STOPPED:
		sysTray->VirtualCamStopped();
		break;

	default:
		break;
	}
}

bool OBSSystemTrayIcon::Active()
{
	return obs_frontend_streaming_active() ||
	       obs_frontend_recording_active() ||
	       obs_frontend_replay_buffer_active() ||
	       obs_frontend_virtualcam_active();
}

void OBSSystemTrayIcon::OnActivate(bool force)
{
	if (!force && Active())
		return;

#ifdef __APPLE__
	QIcon trayIconFile = QIcon(":/res/images/tray_active_macos.svg");
	trayIconFile.setIsMask(true);
#else
	QIcon trayIconFile = QIcon(":/res/images/tray_active.png");
#endif
	setIcon(QIcon::fromTheme("obs-tray-active", trayIconFile));
}

void OBSSystemTrayIcon::OnDeactivate()
{
	if (!Active())
		return;

#ifdef __APPLE__
	QIcon trayIconFile = QIcon(":/res/images/obs_macos.svg");
	trayIconFile.setIsMask(true);
#else
	QIcon trayIconFile = QIcon(":/res/images/obs.png");
#endif
	setIcon(QIcon::fromTheme("obs-tray", trayIconFile));
}

void OBSSystemTrayIcon::ToggleVisibility()
{
	OBSBasic *main = OBSBasic::Get();
	main->ToggleShowHide();

	QTimer::singleShot(0, [this, main] {
		showHide->setText(main->isVisible()
					  ? QTStr("Basic.SystemTray.Hide")
					  : QTStr("Basic.SystemTray.Show"));
	});
}

// Streaming
void OBSSystemTrayIcon::StreamStarting()
{
	stream->setText(QTStr("Basic.Main.Connecting"));
	stream->setEnabled(false);
}

void OBSSystemTrayIcon::StreamStarted()
{
	stream->setEnabled(true);
	stream->setText(QTStr("Basic.Main.StopStreaming"));
	OnActivate();
}

void OBSSystemTrayIcon::StreamStopping()
{
	stream->setText(QTStr("Basic.Main.StoppingStreaming"));
}

void OBSSystemTrayIcon::StreamStopped()
{
	stream->setText(QTStr("Basic.Main.StartStreaming"));
	OnDeactivate();
}

// Recording
void OBSSystemTrayIcon::RecordingStarted()
{
	record->setText(QTStr("Basic.Main.StopRecording"));
	OnActivate();
}

void OBSSystemTrayIcon::RecordingStopping()
{
	record->setText(QTStr("Basic.Main.StoppingRecording"));
}

void OBSSystemTrayIcon::RecordingStopped()
{
	record->setText(QTStr("Basic.Main.StartRecording"));
	OnDeactivate();
}

void OBSSystemTrayIcon::RecordingPaused()
{
#ifdef __APPLE__
	QIcon trayIconFile = QIcon(":/res/images/obs_paused_macos.svg");
	trayIconFile.setIsMask(true);
#else
	QIcon trayIconFile = QIcon(":/res/images/obs_paused.png");
#endif
	setIcon(QIcon::fromTheme("obs-tray-paused", trayIconFile));
}

void OBSSystemTrayIcon::RecordingUnpaused()
{
#ifdef __APPLE__
	QIcon trayIconFile = QIcon(":/res/images/tray_active_macos.svg");
	trayIconFile.setIsMask(true);
#else
	QIcon trayIconFile = QIcon(":/res/images/tray_active.png");
#endif
	setIcon(QIcon::fromTheme("obs-tray-active", trayIconFile));
}

// Replay Buffer
void OBSSystemTrayIcon::UpdateReplayBuffer()
{
	OBSOutputAutoRelease replayOutput =
		obs_frontend_get_replay_buffer_output();
	replay->setEnabled(!!replayOutput);
}

void OBSSystemTrayIcon::ReplayStarted()
{
	replay->setText(QTStr("Basic.Main.StopReplayBuffer"));
	OnActivate();
}

void OBSSystemTrayIcon::ReplayStopping()
{
	replay->setText(QTStr("Basic.Main.StoppingReplayBuffer"));
}

void OBSSystemTrayIcon::ReplayStopped()
{
	replay->setText(QTStr("Basic.Main.StartReplayBuffer"));
	OnDeactivate();
}

// Virtual Camera
void OBSSystemTrayIcon::VirtualCamStarted()
{
	virtualCam->setText(QTStr("Basic.Main.StopVirtualCam"));
	OnActivate();
}

void OBSSystemTrayIcon::VirtualCamStopped()
{
	virtualCam->setText(QTStr("Basic.Main.StartVirtualCam"));
	OnDeactivate();
}

void OBSSystemTrayIcon::IconActivated(QSystemTrayIcon::ActivationReason reason)
{
	OBSBasic *main = OBSBasic::Get();

	// Refresh projector list
	previewProjector->clear();
	studioProgramProjector->clear();
	main->AddProjectorMenuMonitors(previewProjector, main,
				       &OBSBasic::OpenPreviewProjector);
	main->AddProjectorMenuMonitors(studioProgramProjector, main,
				       &OBSBasic::OpenStudioProgramProjector);

#ifdef __APPLE__
	UNUSED_PARAMETER(reason);
#else
	if (reason == QSystemTrayIcon::Trigger)
		ToggleVisibility();
#endif
}

void OBSSystemTrayIcon::ShowNotification(const QString &text,
					 QSystemTrayIcon::MessageIcon n)
{
	if (!supportsMessages())
		return;

	showMessage("OBS Studio", text, n, 10000);
}
