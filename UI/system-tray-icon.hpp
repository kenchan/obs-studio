#pragma once

#include <QSystemTrayIcon>
#include <QPointer>
#include <obs-frontend-api.h>

class QAction;

class OBSSystemTrayIcon : public QSystemTrayIcon {
	Q_OBJECT

private:
	QScopedPointer<QMenu> menu;
	QPointer<QAction> showHide;
	QPointer<QAction> stream;
	QPointer<QAction> record;
	QPointer<QAction> replay;
	QPointer<QAction> virtualCam;
	QPointer<QMenu> previewProjector;
	QPointer<QMenu> studioProgramProjector;

	static void OnEvent(enum obs_frontend_event event, void *data);
	bool Active();
	void OnActivate(bool force = false);
	void OnDeactivate();

	void StreamStarting();
	void StreamStarted();
	void StreamStopping();
	void StreamStopped();

	void RecordingStarted();
	void RecordingStopping();
	void RecordingStopped();
	void RecordingPaused();
	void RecordingUnpaused();

	void ReplayStarted();
	void ReplayStopping();
	void ReplayStopped();

	void VirtualCamStarted();
	void VirtualCamStopped();

private slots:
	void ToggleVisibility();
	void IconActivated(QSystemTrayIcon::ActivationReason reason);

public:
	OBSSystemTrayIcon(QWidget *parent = nullptr);
	~OBSSystemTrayIcon();

	void ShowNotification(const QString &text,
			      QSystemTrayIcon::MessageIcon n);
	void UpdateReplayBuffer();
};
