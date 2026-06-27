#pragma once

#include <QDialog>

class QCheckBox;
class QKeySequenceEdit;
class QLineEdit;

class SettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SettingsDialog(QWidget* parent = nullptr);

private slots:
	void onBrowseSaveDir();
	void onAccept();

private:
	QKeySequenceEdit* m_hotkeyEdit = nullptr;
	QLineEdit* m_saveDirEdit = nullptr;
	QCheckBox* m_checkCopyExit = nullptr;
	QCheckBox* m_checkSingleMonitor = nullptr;
};
