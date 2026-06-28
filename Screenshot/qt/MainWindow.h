#pragma once

#include <QSystemTrayIcon>
#include <QWidget>

class QCloseEvent;
class QMenu;
class QPushButton;
class QResizeEvent;
class QShowEvent;

class MainWindow : public QWidget
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget* parent = nullptr);
	~MainWindow() override;

	static constexpr unsigned kHotKeyId = 1;

protected:
	bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
	void closeEvent(QCloseEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;
	void showEvent(QShowEvent* event) override;

private slots:
	void onScreenshot();
	void onSettings();
	void onOpenSaveFolder();
	void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
	void showFromTray();

private:
	void applySizeFromSettings();
	void saveSizeToSettings();
	void registerHotKey();
	void unregisterHotKey();
	void setupTrayIcon();
	bool useSystemTrayBehavior() const;

	QPushButton* m_btnScreenshot = nullptr;
	QPushButton* m_btnSettings = nullptr;
	QPushButton* m_btnOpenFolder = nullptr;
	QSystemTrayIcon* m_trayIcon = nullptr;
	QMenu* m_trayMenu = nullptr;
	bool m_trayReady = false;
	bool m_startupTrayPending = false;
};
