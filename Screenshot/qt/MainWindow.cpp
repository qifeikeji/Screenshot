#include "stdafx.h"
#include "AppSettings.h"

#include "MainWindow.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QHBoxLayout>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QResizeEvent>
#include <QShowEvent>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QVBoxLayout>

#include "ScreenshotBridge.h"
#include "SettingsDialog.h"
#include "StartupUtil.h"

MainWindow::MainWindow(QWidget* parent)
	: QWidget(parent)
{
	setWindowTitle(QStringLiteral("\u622a\u56fe\u5de5\u5177"));
	setMinimumSize(360, 72);

	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(16, 16, 16, 16);

	auto* row = new QHBoxLayout();
	m_btnScreenshot = new QPushButton(QStringLiteral("\u622a\u56fe"), this);
	m_btnSettings = new QPushButton(QStringLiteral("\u8bbe\u7f6e"), this);
	m_btnOpenFolder = new QPushButton(QStringLiteral("\u6253\u5f00\u6587\u4ef6\u5939"), this);
	m_btnScreenshot->setMinimumHeight(32);
	m_btnSettings->setMinimumHeight(32);
	m_btnOpenFolder->setMinimumHeight(32);
	row->addWidget(m_btnScreenshot);
	row->addWidget(m_btnSettings);
	row->addWidget(m_btnOpenFolder);
	layout->addLayout(row);

	connect(m_btnScreenshot, &QPushButton::clicked, this, &MainWindow::onScreenshot);
	connect(m_btnSettings, &QPushButton::clicked, this, &MainWindow::onSettings);
	connect(m_btnOpenFolder, &QPushButton::clicked, this, &MainWindow::onOpenSaveFolder);

	setupTrayIcon();
	applySizeFromSettings();

	if (useSystemTrayBehavior())
		m_startupTrayPending = true;
}

MainWindow::~MainWindow()
{
	unregisterHotKey();
	if (m_trayIcon)
		m_trayIcon->hide();
}

bool MainWindow::useSystemTrayBehavior() const
{
	return GetAppSettings().startMinimizedToTaskbar != FALSE;
}

void MainWindow::setupTrayIcon()
{
	if (m_trayReady)
		return;
	if (!QSystemTrayIcon::isSystemTrayAvailable())
		return;

	m_trayIcon = new QSystemTrayIcon(this);
	m_trayIcon->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
	m_trayIcon->setToolTip(QStringLiteral("\u622a\u56fe\u5de5\u5177"));

	m_trayMenu = new QMenu(this);
	auto* showAction = m_trayMenu->addAction(QStringLiteral("\u663e\u793a\u4e3b\u7a97\u53e3"));
	auto* shotAction = m_trayMenu->addAction(QStringLiteral("\u622a\u56fe"));
	auto* settingsAction = m_trayMenu->addAction(QStringLiteral("\u8bbe\u7f6e"));
	m_trayMenu->addSeparator();
	auto* quitAction = m_trayMenu->addAction(QStringLiteral("\u9000\u51fa"));

	connect(showAction, &QAction::triggered, this, &MainWindow::showFromTray);
	connect(shotAction, &QAction::triggered, this, &MainWindow::onScreenshot);
	connect(settingsAction, &QAction::triggered, this, &MainWindow::onSettings);
	connect(quitAction, &QAction::triggered, this, [this]() {
		saveSizeToSettings();
		unregisterHotKey();
		QApplication::quit();
	});

	m_trayIcon->setContextMenu(m_trayMenu);
	connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayActivated);
	m_trayIcon->show();
	m_trayReady = true;
}

void MainWindow::showFromTray()
{
	showNormal();
	raise();
	activateWindow();
	registerHotKey();
}

void MainWindow::onTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
	if (reason == QSystemTrayIcon::DoubleClick)
		showFromTray();
}

void MainWindow::applySizeFromSettings()
{
	const AppSettings& s = GetAppSettings();
	resize(s.windowWidth, s.windowHeight);
}

void MainWindow::saveSizeToSettings()
{
	AppSettings& s = GetAppSettings();
	s.windowWidth = width();
	s.windowHeight = height();
	s.Clamp();
	s.Save();
}

void MainWindow::registerHotKey()
{
	const HWND hwnd = reinterpret_cast<HWND>(winId());
	RegisterGlobalScreenshotHotKey(hwnd, kHotKeyId);
}

void MainWindow::unregisterHotKey()
{
	const HWND hwnd = reinterpret_cast<HWND>(winId());
	UnregisterGlobalScreenshotHotKey(hwnd, kHotKeyId);
}

bool MainWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
	if (eventType == QByteArrayLiteral("windows_generic_MSG")
		|| eventType == QByteArrayLiteral("windows_dispatcher_MSG"))
	{
		const MSG* msg = static_cast<const MSG*>(message);
		if (msg->message == WM_HOTKEY && msg->wParam == kHotKeyId)
		{
			onScreenshot();
			if (result)
				*result = 0;
			return true;
		}
	}
	return QWidget::nativeEvent(eventType, message, result);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (useSystemTrayBehavior() && m_trayReady)
	{
		event->ignore();
		hide();
		if (m_trayIcon)
		{
			m_trayIcon->showMessage(
				QStringLiteral("\u622a\u56fe\u5de5\u5177"),
				QStringLiteral("\u5df2\u9690\u85cf\u5230\u901a\u77e5\u533a\u57df\uff0c\u53cc\u51fb\u6258\u76d8\u56fe\u6807\u53ef\u6062\u590d\u3002"),
				QSystemTrayIcon::Information, 2000);
		}
		return;
	}

	saveSizeToSettings();
	unregisterHotKey();
	QWidget::closeEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);
}

void MainWindow::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	registerHotKey();

	if (m_startupTrayPending && useSystemTrayBehavior())
	{
		m_startupTrayPending = false;
		QTimer::singleShot(0, this, [this]() {
			hide();
			if (m_trayIcon)
			{
				m_trayIcon->showMessage(
					QStringLiteral("\u622a\u56fe\u5de5\u5177"),
					QStringLiteral("\u5df2\u542f\u52a8\u5230\u901a\u77e5\u533a\u57df\uff0c\u53cc\u51fb\u6258\u76d8\u56fe\u6807\u53ef\u6253\u5f00\u3002"),
					QSystemTrayIcon::Information, 2500);
			}
		});
	}
}

void MainWindow::onScreenshot()
{
	if (!isVisible())
		showFromTray();
	StartScreenshotCapture(reinterpret_cast<HWND>(winId()));
}

void MainWindow::onSettings()
{
	if (!isVisible())
		showFromTray();
	SettingsDialog dlg(this);
	if (dlg.exec() == QDialog::Accepted)
	{
		unregisterHotKey();
		registerHotKey();
	}
}

void MainWindow::onOpenSaveFolder()
{
	OpenFolderInExplorer(GetAppSettings().GetEffectiveSaveDirectory());
}
