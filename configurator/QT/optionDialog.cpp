#include "optionDialog.h"

OptionDialog::OptionDialog(Module* module, QWidget *parent) : QDialog(parent) {
	QGridLayout *layout = new QGridLayout;
	Parameter* params = module->parameters;
	QPushButton *okButton = new QPushButton("&OK", this);
	QPushButton *cancelButton = new QPushButton("&Cancel", this);
	okButton->setAutoDefault(true);
	okButton->setDefault(true);

	setWindowModality(Qt::ApplicationModal);

	//layout->setColumnStretch(1, 1);
	layout->setColumnMinimumWidth(1, 250);

	while (params != NULL) {
		QLabel *label = new QLabel;
		QWidget *value;
		
		if (params->comment == NULL) {
			printf("Warning: module %s does have a parameter %s without comment\n", module->name, params->name);
			label->setText(params->name);
		} else {
			label->setText(params->comment);
		}
		layout->addWidget(label);
		
		switch (params->type) {
			case NUMERIC:
			case NUMERIC_CONST:
			case STRING:
			case CHAR:
				value = new QLineEdit;
				break;
			case BOOL:
				value = new QCheckBox;
				break;
			default:
				value = new QLabel;
				printf("Unhandled param type\n");
		}
		layout->addWidget(value);

		params = params->next;
	}

	layout->addWidget(okButton);
	layout->addWidget(cancelButton);

	setLayout(layout);
	setWindowTitle(QString(module->name).append(" options"));

	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}
