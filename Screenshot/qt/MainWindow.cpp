#include "stdafx.h"
#include "AppSettings.h"

#include "MainWindow.h"

#include <QCloseEvent>
#include <QHBoxLayout>
#include <QPushButton>
#include <QResizeEvent>
#include <QShowEvent>
#include <QVBoxLayout>

#include "ScreenshotBridge.h"
#include "SettingsDialog.h"

MainWindow::MainWindow(QWidget* parent)
	: QWidget(parent)
{
	setWindowTitle(QStringLiteral("\u622a\u56fe\u5de5\u5177"));
	setMinimumSize(220, 72);

	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(16, 16, 16, 16);

	auto* row = new QHBoxLayout();
	m_btnScreenshot = new QPushButton(QStringLiteral("\u622a\u56fe"), this);
	m_btnSettings = new QPushButton(QStringLiteral("\u8bbe\u7f6e"), this);
	m_btnScreenshot->setMinimumHeight(32);
	m_btnSettings->setMinimumHeight(32);
	row->addWidget(m_btnScreenshot);
	row->addWidget(m_btnSettings);
	layout->addLayout(row);

	connect(m_btnScreenshot, &QPushButton::clicked, this, &MainWindow::onScreenshot);
	connect(m_btnSettings, &QPushButton::clicked, this, &MainWindow::onSettings);

	applySizeFromSettings();
}

MainWindow::~MainWindow()
{
	unregisterHotKey();
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
	if (GetAppSettings().startMinimizedToTaskbar)
		showMinimized();
}

void MainWindow::onScreenshot()
{
	StartScreenshotCapture(reinterpret_cast<HWND>(winId()));
}

void MainWindow::onSettings()
{
	SettingsDialog dlg(this);
	if (dlg.exec() == QDialog::Accepted)
	{
		unregisterHotKey();
		registerHotKey();
	}
}
