#pragma once

#include <QWidget>

class QPushButton;

class MainWindow : public QWidget
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget* parent = nullptr);
	~MainWindow() override;

	static constexpr UINT kHotKeyId = 1;

protected:
	bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
	void closeEvent(QCloseEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;
	void showEvent(QShowEvent* event) override;

private slots:
	void onScreenshot();
	void onSettings();

private:
	void applySizeFromSettings();
	void saveSizeToSettings();
	void registerHotKey();
	void unregisterHotKey();

	QPushButton* m_btnScreenshot = nullptr;
	QPushButton* m_btnSettings = nullptr;
};
