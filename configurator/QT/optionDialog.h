#ifndef OPTIONDIALOG_H_
#define OPTIONDIALOG_H_

#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QGridLayout>
#include <QPushButton>

#include "../miximConfiguratorCommon.h"

class OptionDialog : public QDialog {
	Q_OBJECT
	
	public:
		OptionDialog(Module* module, QWidget *parent = 0);
};

#endif /*OPTIONDIALOG_H_*/
