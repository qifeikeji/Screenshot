#include "stdafx.h"
#include "AppSettings.h"
#include "StartupUtil.h"

#include "SettingsDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QKeySequenceEdit>
#include <QKeyCombination>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

namespace {

QKeySequence KeySequenceFromAppHotkey(UINT mod, UINT vk)
{
	Qt::KeyboardModifiers qmod = Qt::NoModifier;
	if (mod & MOD_CONTROL)
		qmod |= Qt::ControlModifier;
	if (mod & MOD_ALT)
		qmod |= Qt::AltModifier;
	if (mod & MOD_SHIFT)
		qmod |= Qt::ShiftModifier;
	const Qt::Key key = static_cast<Qt::Key>(vk);
	return QKeySequence(QKeyCombination(qmod, key));
}

bool ModVkFromKeySequence(const QKeySequence& seq, UINT& mod, UINT& vk)
{
	if (seq.isEmpty())
		return false;
	const QKeyCombination comb = seq[0];
	const Qt::KeyboardModifiers qmod = comb.keyboardModifiers();
	const Qt::Key key = comb.key();

	mod = 0;
	if (qmod & Qt::ShiftModifier)
		mod |= MOD_SHIFT;
	if (qmod & Qt::ControlModifier)
		mod |= MOD_CONTROL;
	if (qmod & Qt::AltModifier)
		mod |= MOD_ALT;

	const int k = static_cast<int>(key);
	if (k == 0)
		return false;
	vk = static_cast<UINT>(k);
	return true;
}

QString CStringToQString(const CString& s)
{
	return QString::fromWCharArray((LPCWSTR)s, s.GetLength());
}

CString QStringToCString(const QString& s)
{
	return CString(s.toStdWString().c_str());
}

} // namespace

SettingsDialog::SettingsDialog(QWidget* parent)
	: QDialog(parent)
{
	setWindowTitle(QStringLiteral("\u8bbe\u7f6e"));
	setMinimumWidth(360);

	const AppSettings& s = GetAppSettings();

	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(16, 16, 16, 16);
	layout->setSpacing(10);

	layout->addWidget(new QLabel(QStringLiteral("\u622a\u56fe\u5feb\u6377\u952e"), this));
	m_hotkeyEdit = new QKeySequenceEdit(this);
	m_hotkeyEdit->setKeySequence(KeySequenceFromAppHotkey(s.hotkeyModifiers, s.hotkeyVk));
	m_hotkeyEdit->setClearButtonEnabled(true);
	layout->addWidget(m_hotkeyEdit);

	layout->addWidget(new QLabel(QStringLiteral("\u4fdd\u5b58\u56fe\u7247\u8def\u5f84"), this));
	auto* pathRow = new QHBoxLayout();
	m_saveDirEdit = new QLineEdit(this);
	m_saveDirEdit->setText(CStringToQString(s.saveDirectory));
	const QString defaultDir = CStringToQString(GetDefaultSaveDirectory());
	m_saveDirEdit->setPlaceholderText(
		QStringLiteral("\u7559\u7a7a\u5219\u4f7f\u7528\u9ed8\u8ba4\uff1a %1").arg(defaultDir));
	auto* browseBtn = new QPushButton(QStringLiteral("\u6d4f\u89c8..."), this);
	pathRow->addWidget(m_saveDirEdit, 1);
	pathRow->addWidget(browseBtn);
	layout->addLayout(pathRow);

	m_checkCopyExit = new QCheckBox(
		QStringLiteral("\u6846\u9009\u540e\u76f4\u63a5\u590d\u5236\u5230\u526a\u8d34\u677f\u3001\u4fdd\u5b58\u5230\u4fdd\u5b58\u8def\u5f84\u5e76\u9000\u51fa\u622a\u56fe"), this);
	m_checkCopyExit->setChecked(s.copyAndExitAfterSelect != FALSE);
	layout->addWidget(m_checkCopyExit);

	m_checkSaveOnEnter = new QCheckBox(
		QStringLiteral("\u6846\u9009\u540e\u6309\u56de\u8f66\u81ea\u52a8\u4fdd\u5b58\u5230\u4fdd\u5b58\u8def\u5f84"), this);
	m_checkSaveOnEnter->setChecked(s.saveToFileOnEnterAfterSelect != FALSE);
	layout->addWidget(m_checkSaveOnEnter);

	m_checkSingleMonitor = new QCheckBox(
		QStringLiteral("\u5355\u663e\u793a\u5668\u622a\u56fe\uff08\u5feb\u6377\u952e\u65f6\u53ea\u622a\u9f20\u6807\u6240\u5728\u663e\u793a\u5668\uff09"), this);
	m_checkSingleMonitor->setChecked(s.singleMonitorCapture != FALSE);
	layout->addWidget(m_checkSingleMonitor);

	m_checkLaunchAtStartup = new QCheckBox(QStringLiteral("\u5f00\u673a\u81ea\u52a8\u542f\u52a8"), this);
	m_checkLaunchAtStartup->setChecked(s.launchAtStartup != FALSE);
	layout->addWidget(m_checkLaunchAtStartup);

	m_checkStartMinimized = new QCheckBox(
		QStringLiteral("\u542f\u52a8\u65f6\u9690\u85cf\u5230\u901a\u77e5\u533a\u57df\uff08\u5173\u95ed\u7a97\u53e3\u540c\u6837\u9690\u85cf\uff0c\u53cc\u51fb\u6258\u76d8\u6062\u590d\uff09"), this);
	m_checkStartMinimized->setChecked(s.startMinimizedToTaskbar != FALSE);
	layout->addWidget(m_checkStartMinimized);

	auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	auto* openFolderBtn = new QPushButton(QStringLiteral("\u6253\u5f00\u6587\u4ef6\u5939"), this);
	buttons->addButton(openFolderBtn, QDialogButtonBox::ActionRole);
	layout->addWidget(buttons);

	connect(browseBtn, &QPushButton::clicked, this, &SettingsDialog::onBrowseSaveDir);
	connect(openFolderBtn, &QPushButton::clicked, this, &SettingsDialog::onOpenSaveFolder);
	connect(buttons, &QDialogButtonBox::accepted, this, &SettingsDialog::onAccept);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void SettingsDialog::onBrowseSaveDir()
{
	const QString dir = QFileDialog::getExistingDirectory(
		this, QStringLiteral("\u9009\u62e9\u4fdd\u5b58\u76ee\u5f55"), m_saveDirEdit->text());
	if (!dir.isEmpty())
		m_saveDirEdit->setText(dir);
}

void SettingsDialog::onOpenSaveFolder()
{
	OpenFolderInExplorer(GetAppSettings().GetEffectiveSaveDirectory());
}

void SettingsDialog::onAccept()
{
	UINT mod = 0;
	UINT vk = 0;
	if (!ModVkFromKeySequence(m_hotkeyEdit->keySequence(), mod, vk))
	{
		QMessageBox::warning(this, QStringLiteral("\u8bbe\u7f6e"),
			QStringLiteral("\u8bf7\u8bbe\u7f6e\u6709\u6548\u7684\u5feb\u6377\u952e\u3002"));
		return;
	}
	if (mod == 0)
	{
		QMessageBox::warning(this, QStringLiteral("\u8bbe\u7f6e"),
			QStringLiteral("\u5feb\u6377\u952e\u9700\u5305\u542b Ctrl\u3001Alt \u6216 Shift \u7b49\u7ec4\u5408\u952e\u3002"));
		return;
	}

	AppSettings& cfg = GetAppSettings();
	cfg.hotkeyModifiers = mod;
	cfg.hotkeyVk = vk;
	cfg.copyAndExitAfterSelect = m_checkCopyExit->isChecked() ? TRUE : FALSE;
	cfg.saveToFileOnEnterAfterSelect = m_checkSaveOnEnter->isChecked() ? TRUE : FALSE;
	cfg.singleMonitorCapture = m_checkSingleMonitor->isChecked() ? TRUE : FALSE;
	cfg.launchAtStartup = m_checkLaunchAtStartup->isChecked() ? TRUE : FALSE;
	cfg.startMinimizedToTaskbar = m_checkStartMinimized->isChecked() ? TRUE : FALSE;
	cfg.saveDirectory = QStringToCString(m_saveDirEdit->text().trimmed());
	cfg.Clamp();
	cfg.Save();
	ApplyLaunchAtStartupSetting(cfg.launchAtStartup);
	accept();
}
